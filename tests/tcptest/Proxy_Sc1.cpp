#define LOGPRINT
#include "utils/log.hpp"
#include "core/server.hpp"
#include "core/listen.hpp"
#include "utils/invalid_socket.hpp"
#include "utils/netutils.hpp"
#include <algorithm>

// Simple proxy is both a Server and a Client
// When it receives an incoming connexion
// it connects to some outbound server
// The proxy is contents neutral and should work
// for any kind of traffic.

class Proxy : public Server {
public:
    int  frontsck;
    char ip_source[256];

    Proxy() : frontsck(0) {
        LOG(LOG_INFO, "Incoming connection to proxy\n");
       this->ip_source[0] = 0;
    }

    Server_status start(int incoming_sck) override {
        union {
            struct sockaddr s;
            struct sockaddr_storage ss;
            struct sockaddr_in s4;
            struct sockaddr_in6 s6;
        } u;
        unsigned int sin_size = sizeof(u);
        memset(&u, 0, sin_size);
        this->frontsck = accept(incoming_sck, &u.s, &sin_size);
        strcpy(this->ip_source, inet_ntoa(u.s4.sin_addr));
        LOG(LOG_INFO, "Incoming socket to %d (ip=%s)\n", this->frontsck, this->ip_source);
        int pid = fork();
        switch (pid){
        case -1:
        {
            return Server::START_FAILED;
        }
        case 0: // Child
        {
            close(incoming_sck);

            // 1) connect to remote target

            const char * ip = "localhost";
            const int port = 6000;
            int targetsck = ip_connect(ip, port, 3, 1000);

            if (targetsck == INVALID_SOCKET){
                LOG(LOG_ERR, "Error connecting socket to %s:%d\n", ip, port);
                _exit(-1);
            }

            // 2) Listen on both sockets
            bool loop = true;
            size_t target_to_front_start = 0;
            size_t target_to_front_end = 0;
            uint8_t front_to_target_buffer[32768] = {};
            size_t front_to_target_start = 0;
            size_t front_to_target_end = 0;
            uint8_t target_to_front_buffer[32768] = {};
            while(loop) {
                struct timeval timeout;
                timeout.tv_sec = 30;
                timeout.tv_usec = 0;

                LOG(LOG_INFO, "front to target [ %d, %d] target to front [ %d, %d ]\n",
                    static_cast<int>(front_to_target_start), static_cast<int>(front_to_target_end),
                    static_cast<int>(target_to_front_start), static_cast<int>(target_to_front_end)
                    );

                // - only read on a socket if the cross write on the other socket is ready
                // (useless to read if we can't send). Another way to achieve that
                // is to only read when a buffer has been sent.
                fd_set rfds, wfds;
                io_fd_zero(rfds);
                io_fd_zero(wfds);
                if (front_to_target_end == 0){
                    io_fd_set(frontsck, rfds);
                }
                else {
                    io_fd_set(targetsck, wfds);
                }
                if (target_to_front_end == 0){
                    io_fd_set(targetsck, rfds);
                }
                else {
                    io_fd_set(frontsck, wfds);
                }
                int select_res = select(std::max(targetsck, frontsck) + 1, &rfds, &wfds, nullptr, &timeout);
                switch (select_res){
                case -1: // error
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)
                     || (errno == EINPROGRESS) || (errno == EINTR)){
                        continue; /* these are not really errors */
                    }
                    loop = false;
                break;
                case 0:  // timeout
                break;
                default:
                {
                    if (io_fd_isset(targetsck, rfds)){
                        LOG(LOG_INFO, "Data to read on targetsck");
                        int res = read(targetsck, &target_to_front_buffer[0], sizeof(target_to_front_buffer)-target_to_front_end);
                        if (res > 0){
                            target_to_front_end += res;
                        }
                        else if (res < 0) {
                            if ((errno != EAGAIN) && (errno != EWOULDBLOCK)
                             && (errno != EINPROGRESS) && (errno != EINTR)){
                             // there is some kind of error
                             // close connection
                             LOG(LOG_INFO, "Error reading target socket : %s", strerror(errno));
                             loop = false;
                            }
                            else {
                                // Nothing really happened
                            }
                        }
                        else { // res == 0
                            // This is end of file. Socket closed ?
                            LOG(LOG_INFO, "End of target socket");
                            loop = false;
                        }
                    }

                    if (io_fd_isset(targetsck, wfds)){
                        LOG(LOG_INFO, "Ready to write on targetsck");
                        int res = write(targetsck,
                                        &front_to_target_buffer[front_to_target_start],
                                        front_to_target_end-front_to_target_start);
                        if (res >= 0){
                            front_to_target_start += res;
                            if (front_to_target_start == front_to_target_end){
                                front_to_target_start = front_to_target_end = 0;
                            }
                        }
                        else if (res < 0) {
                            if ((errno != EAGAIN) && (errno != EWOULDBLOCK)
                             && (errno != EINPROGRESS) && (errno != EINTR)){
                             // there is some kind of error
                             // close connection
                             LOG(LOG_INFO, "Error writing to target socket : %s", strerror(errno));
                             loop = false;
                            }
                            else {
                                // EAGAIN or EINTR:: Nothing really happened
                            }
                        }
                    }
                    if (io_fd_isset(frontsck, rfds)){
                        LOG(LOG_INFO, "Data to read on frontsck");
                        int res = read(frontsck, &front_to_target_buffer[0], sizeof(front_to_target_buffer)-front_to_target_end);
                        if (res > 0){
                            front_to_target_end += res;
                        }
                        else if (res < 0) {
                            if ((errno != EAGAIN) && (errno != EWOULDBLOCK)
                             && (errno != EINPROGRESS) && (errno != EINTR)){
                             // there is some kind of error
                             // close connection
                             LOG(LOG_INFO, "Error reading front socket : %s", strerror(errno));
                             loop = false;
                            }
                            else {
                                // Nothing really happened
                            }
                        }
                        else { // res == 0
                            // This is end of file. Socket closed ?
                            LOG(LOG_INFO, "End of front socket");
                            loop = false;
                        }
                    }

                    if (io_fd_isset(frontsck, wfds)){
                        LOG(LOG_INFO, "Ready to write on frontsck");
                        int res = write(frontsck,
                                        &target_to_front_buffer[target_to_front_start],
                                        target_to_front_end-target_to_front_start);
                        if (res >= 0){
                            target_to_front_start += res;
                            if (target_to_front_start == target_to_front_end){
                                target_to_front_start = target_to_front_end = 0;
                            }
                        }
                        else if (res < 0) {
                            if ((errno != EAGAIN) && (errno != EWOULDBLOCK)
                             && (errno != EINPROGRESS) && (errno != EINTR)){
                             // there is some kind of error, close connection
                             LOG(LOG_INFO, "Error writing to target socket : %s", strerror(errno));
                             loop = false;
                            }
                            else {
                                // EAGAIN or EINTR:: Nothing really happened
                            }
                        }
                    }
                }
                }

            }
            _exit(0);
        }
        default: // Listener
            return Server::START_OK;
        }
    }
};


int main(int argc, char * argv[])
{
    LOG(LOG_INFO, "Server Start\n");
    // TODO: provide server port on command line
    (void)argc; (void)argv;
    Proxy p;

    Listen listener(p, 0, 5000, true, 25);  // 25 seconds to connect, or timeout
    listener.run();
}
