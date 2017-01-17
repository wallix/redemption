#define LOGPRINT
#include "utils/log.hpp"
#include "core/server.hpp"
#include "core/listen.hpp"

class Generator : public Server {
public:
    int  sck;
    char ip_source[256];

    Generator() : sck(0) {
        LOG(LOG_INFO, "Server Once Start\n");
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
        this->sck = accept(incoming_sck, &u.s, &sin_size);
        strcpy(this->ip_source, inet_ntoa(u.s4.sin_addr));
        LOG(LOG_INFO, "Incoming socket to %d (ip=%s)\n", this->sck, this->ip_source);
        int pid = fork();
        switch (pid){
        case -1:
        {
            return Server::START_FAILED;
        }
        case 0: // Child
        {
            close(incoming_sck);
            // Actual Server code : this one is a simple generator, always sending the same message
            char message[65536];
            int count = 0;
            unsigned len = 0;
            bool loop = true;
            unsigned sent = 0;

            while(loop) {
                if (sent == 0){
                    snprintf(message, sizeof(message), "The quick brown fox jump over the lazy dog %d\n", count++);
                    len = strlen(message);
                }

                fd_set wfds;
                FD_ZERO(&wfds);
                FD_SET(this->sck, &wfds);
                struct timeval timeout;
                timeout.tv_sec = 300; // test server runs for 5 minutes
                timeout.tv_usec = 0;
                int select_res = select(this->sck + 1, nullptr, &wfds, nullptr, &timeout);
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
                    int res = write(this->sck, &message[sent], len-sent);
                    if (res < 0){
                        loop = false;
                        break;
                    }
                    sent += len;
                    if (sent >= len){
                        sent = 0;
                    }
                }
                break;
                }
            }
            _exit(0);
        }
        break;
        default: // Listener
            return Server::START_OK;
        break;
        }
    }
};


int main(int argc, char * argv[])
{
    LOG(LOG_INFO, "Server Start\n");
    // TODO: provide server port on command line
    (void)argc; (void)argv;
    Generator g;

    Listen listener(g, 0, 6000, true, 25);  // 25 seconds to connect, or timeout
    listener.run();
}
