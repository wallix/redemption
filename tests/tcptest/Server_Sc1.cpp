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
        close(incoming_sck);
        switch (pid){
        case -1:
            return Server::START_FAILED;
        case 0: // Child
            // Actual Server code : this one is a simple generator, always sending the same message
            const char * message = "The quick brown fox jump over the lazy dog\n";
            unsigned len = strlen(message);
            while(1) {
                unsigned sent = 0;
                while (sent < len){
                    int res = write(this->sck, &message[sent], len-sent);
                    if (res < 0){
                        break;
                    }
                    sent += len;
                }
            }
            _exit(0);
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
    
    Listen listener(Generator, 0, 5000, true, 25);  // 25 seconds to connect, or timeout
    listener.run();
}
