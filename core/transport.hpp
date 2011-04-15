/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Transport layer abstraction

*/


#if !defined(__TRANSPORT_HPP__)
#define __TRANSPORT_HPP__

#include <sys/types.h> // recv, send
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/un.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "error.hpp"

class Transport {

public:
    virtual void recv(char ** pbuffer, int len) throw (Error) = 0;
    virtual void send(const char * buffer, int len) throw (Error) = 0;
    virtual void disconnect() = 0;
    virtual void connect(const char* ip, int port, int nbretry = 0, int retry_delai_ms = 1000000) throw (Error) = 0;

};

class GeneratorTransport : public Transport {

    GeneratorTransport()
    {
    }

    virtual void recv(char ** pbuffer, int len) throw (Error) {
    }
    virtual void send(const char * buffer, int len) throw (Error) {
    }
    virtual void disconnect() {
    }
};


class LoopTransport : public Transport {

    virtual void recv(char ** pbuffer, int len) throw (Error) {
    }
    virtual void send(const char * buffer, int len) throw (Error) {
    }
    virtual void disconnect() {
    }
};

class SocketTransport : public Transport {
    public:
        int sck;
        int sck_closed;
        int (*is_term)(void);

    SocketTransport(int sck, int (*is_term)(void))
    {
        this->sck = sck;
        this->sck_closed = 0;
        this->is_term = is_term;
    }


    SocketTransport(const char* ip, int port, int (*is_term)(void),
            int nbretry = 0, int retry_delai_ms = 1000000)
    {
        this->sck = 0;
        this->sck_closed = 0;
        this->is_term = is_term;
        this->connect(ip, port, nbretry, retry_delai_ms);
    }

    ~SocketTransport(){
        if (!this->sck_closed){
            this->disconnect();
        }
    }
    static bool try_again(int errnum){
        int res = false;
        switch (errno){
            case EAGAIN:
            /* case EWOULDBLOCK: */ // same as EAGAIN on Linux
            case EINPROGRESS:
            case EALREADY:
            case EBUSY:
            case EINTR:
                res = true;
                break;
            default:
                ;
        }
        return res;
    }

    virtual void disconnect(){
        LOG(LOG_INFO, "Socket %d : closing connection\n", this->sck);
        if (this->sck != 0) {
            shutdown(this->sck, 2);
            close(this->sck);
        }
        this->sck = 0;
        this->sck_closed = 1;
    }

    enum direction_t {
        NONE = 0,
        RECV = 1,
        SEND = 2
    };

    void wait_recv_ready(int delay_ms) throw (Error)
    {
        this->wait_ready(RECV, delay_ms);
    }

    void wait_send_ready(int delay_ms) throw (Error)
    {
        this->wait_ready(SEND, delay_ms);
    }

    void wait_ready(direction_t d, int delay_ms) throw (Error)
    {
        fd_set fds;
        struct timeval time;

        time.tv_sec = delay_ms / 1000;
        time.tv_usec = (delay_ms * 1000) % 1000000;
        FD_ZERO(&fds);
        FD_SET(((unsigned int)this->sck), &fds);
        if (select(this->sck + 1,
            (d & RECV)? &fds : 0,
            (d & SEND)? &fds : 0,
            0, &time) > 0) {
            // Test if manager asked for closing current session
            if (this->is_term && (*this->is_term)()) {
                throw Error(ERR_SESSION_TERMINATED);
            }
            int opt = 0;
            unsigned int opt_len = sizeof(opt);
            getsockopt(this->sck, SOL_SOCKET, SO_ERROR, (char*)(&opt), &opt_len);
            // Test if we got a socket error
            if (opt) {
                throw Error(ERR_SESSION_TERMINATED);
            }
        }
    }

    virtual void recv(char ** input_buffer, int total_len) throw (Error)
    {
        int len = total_len;
        char * pbuffer = *input_buffer;

        if (this->sck_closed) {
            LOG(LOG_INFO, "socket allready closed\n");
            throw Error(ERR_SOCKET_ALLREADY_CLOSED);
        }
        while (len > 0) {
            int rcvd = ::recv(this->sck, pbuffer, len, 0);
            switch (rcvd) {
                case -1: /* error, maybe EAGAIN */
                    if (!this->try_again(errno)) {
                        LOG(LOG_INFO, "closing socket on recv\n");
                        this->sck_closed = 1;
                        throw Error(ERR_SOCKET_ERROR, errno);
                    }
                    this->wait_ready(RECV, 10);
                    break;
                case 0: /* no data received, socket closed */
                    LOG(LOG_INFO, "no data received socket closed on recv\n");
                    this->sck_closed = 1;
                    throw Error(ERR_SOCKET_CLOSED);
                default: /* some data received */
                    pbuffer += rcvd;
                    len -= rcvd;
            }
        }
        *input_buffer = pbuffer;

    }

    virtual void send(const char * buffer, int len) throw (Error)
    {
        if (this->sck_closed) {
            throw Error(ERR_SOCKET_ALLREADY_CLOSED);
        }
        int total = 0;
        while (total < len) {
            int sent = ::send(this->sck, buffer + total, len - total, 0);
            switch (sent){
            case -1:
                if (!this->try_again(errno)) {
                    LOG(LOG_INFO, "%s sck=%d\n", strerror(errno), this->sck);
                    this->sck_closed = 1;
                    throw Error(ERR_SOCKET_ERROR, errno);
                }
                this->wait_ready(SEND, 10);
                break;
            case 0:
                LOG(LOG_INFO, "socket closed on sending %s sck=%d\n", strerror(errno), this->sck);
                this->sck_closed = 1;
                throw Error(ERR_SOCKET_CLOSED, errno);
            default:
                total = total + sent;
            }
        }
    }

    private:
    void connect(const char* ip, int port,
                 int nbretry = 0, int retry_delai_ms = 1000000) throw (Error)
    {
        LOG(LOG_INFO, "connecting to %s:%d\n", ip, port);
        // we will try connection several time
        // the trial process include socket opening, hostname resolution, etc
        // because some problems can come from the local endpoint,
        // not necessarily from the remote endpoint.
        for (int trial = 0; ; trial++){
            try {
                this->sck = socket(PF_INET, SOCK_STREAM, 0);

                /* reuse same port if a previous daemon was stopped */
                unsigned int option_len;
                int allow_reuse = 1;
                setsockopt(this->sck, SOL_SOCKET, SO_REUSEADDR,
                    (char*)&allow_reuse, sizeof(allow_reuse));

                /* set snd buffer to at least 32 Kbytes */
                int snd_buffer_size;
                option_len = sizeof(snd_buffer_size);
                if (0 == getsockopt(this->sck, SOL_SOCKET, SO_SNDBUF,
                                (char*)&snd_buffer_size, &option_len)) {
                    if (snd_buffer_size < 32768) {
                        snd_buffer_size = 32768;
                        setsockopt(this->sck, SOL_SOCKET, SO_SNDBUF,
                            (char*)&snd_buffer_size, sizeof(snd_buffer_size));
                    }
                }

                struct sockaddr_in s;
                memset(&s, 0, sizeof(struct sockaddr_in));
                s.sin_family = AF_INET;
                s.sin_port = htons(port);
                s.sin_addr.s_addr = inet_addr(ip);
                if (s.sin_addr.s_addr == INADDR_NONE) {
                #warning gethostbyname is obsolete use new function getnameinfo
                    LOG(LOG_INFO, "Asking ip to DNS for %s\n", ip);
                    struct hostent *h = gethostbyname(ip);
                    if (!h) {
                        LOG(LOG_ERR, "DNS resolution failed for %s\n", ip);
                        throw Error(ERR_SOCKET_GETHOSTBYNAME_FAILED);
                    }
                    s.sin_addr.s_addr = *((int*)(*(h->h_addr_list)));
                }
                #warning we should control and detect timeout instead of relying on default connect behavior. Maybe set O_NONBLOCK and use poll to manage timeouts ?
                if (::connect(this->sck, (struct sockaddr*)&s, sizeof(s))){
                    LOG(LOG_INFO, "connection to %s failed\n", ip);
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }
                fcntl(this->sck, F_SETFL, fcntl(this->sck, F_GETFL) | O_NONBLOCK);
                break;
            }
            catch (Error){
                if (trial >= nbretry){
                    LOG(LOG_INFO, "All trials done connecting to %s\n", ip);
                    throw;
                }
            }
            LOG(LOG_INFO, "Will retry connecting to %s in %d ms (trial %d on %d)\n",
                ip, retry_delai_ms, trial, nbretry);
            usleep(retry_delai_ms);
        }
        LOG(LOG_INFO, "connection to %s succeeded : socket %d\n", ip, this->sck);
    }
};

#endif
