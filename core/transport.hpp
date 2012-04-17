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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

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
#include "log.hpp"


static inline int connect(const char* ip, int port, const char * name,
             int nbretry = 2, int retry_delai_ms = 1000) throw (Error)
{
    int sck = 0;
    LOG(LOG_INFO, "connecting to %s (%s:%d)\n", name, ip, port);
    // we will try connection several time
    // the trial process include socket opening, hostname resolution, etc
    // because some problems can come from the local endpoint,
    // not necessarily from the remote endpoint.
    for (int trial = 0; ; trial++){
        try {
            sck = socket(PF_INET, SOCK_STREAM, 0);

            /* set snd buffer to at least 32 Kbytes */
            int snd_buffer_size;
            unsigned int option_len = sizeof(snd_buffer_size);
            if (0 == getsockopt(sck, SOL_SOCKET, SO_SNDBUF, &snd_buffer_size, &option_len)) {
                if (snd_buffer_size < 32768) {
                    snd_buffer_size = 32768;
                    if (-1 == setsockopt(sck,
                            SOL_SOCKET,
                            SO_SNDBUF,
                            &snd_buffer_size, sizeof(snd_buffer_size))){
                        LOG(LOG_WARNING, "setsockopt failed with errno=%d", errno);
                        throw Error(ERR_SOCKET_CONNECT_FAILED);
                    }
                }
            }
            else {
                LOG(LOG_WARNING, "getsockopt failed with errno=%d", errno);
                throw Error(ERR_SOCKET_CONNECT_FAILED);
            }

            struct sockaddr_in s;
            memset(&s, 0, sizeof(struct sockaddr_in));
            s.sin_family = AF_INET;
            s.sin_port = htons(port);
            s.sin_addr.s_addr = inet_addr(ip);
            if (s.sin_addr.s_addr == INADDR_NONE) {
            TODO(" gethostbyname is obsolete use new function getnameinfo")
                LOG(LOG_INFO, "Asking ip to DNS for %s\n", ip);
                struct hostent *h = gethostbyname(ip);
                if (!h) {
                    LOG(LOG_ERR, "DNS resolution failed for %s with errno =%d (%s)\n",
                        ip, errno, strerror(errno));
                    throw Error(ERR_SOCKET_GETHOSTBYNAME_FAILED);
                }
                s.sin_addr.s_addr = *((int*)(*(h->h_addr_list)));
            }
            TODO(" we should control and detect timeout instead of relying on default connect behavior. Maybe set O_NONBLOCK and use poll to manage timeouts ?")
            if (-1 == ::connect(sck, (struct sockaddr*)&s, sizeof(s))){
                LOG(LOG_INFO, "Connection to %s failed with errno = %d (%s)",
                    ip, errno, strerror(errno));
                throw Error(ERR_SOCKET_CONNECT_FAILED);
            }
            fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) | O_NONBLOCK);
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
    LOG(LOG_INFO, "connection to %s succeeded : socket %d\n", ip, sck);
    return sck;
}

class Transport {
public:
    uint64_t total_received;
    uint64_t last_quantum_received;
    uint64_t total_sent;
    uint64_t last_quantum_sent;
    uint64_t quantum_count;

    Transport() :
        total_received(0),
        last_quantum_received(0),
        total_sent(0),
        last_quantum_sent(0),
        quantum_count(0)
    {}

    void tick() {
        quantum_count++;
        last_quantum_received = 0;
        last_quantum_sent = 0;
    }

    void recv(uint8_t ** pbuffer, size_t len) throw (Error) {
        this->recv(reinterpret_cast<char **>(pbuffer), len);
    }
    virtual void recv(char ** pbuffer, size_t len) throw (Error) = 0;
    virtual void send(const char * const buffer, size_t len) throw (Error) = 0;
    void send(const uint8_t * const buffer, size_t len) throw (Error) {
        this->send(reinterpret_cast<const char * const>(buffer), len);
    }
};

class GeneratorTransport : public Transport {

    public:
    size_t current;
    char * data;
    size_t len;


    GeneratorTransport(const char * data, size_t len)
        : Transport(), current(0), data(0), len(len)
    {
        this->data = (char *)malloc(len);
        memcpy(this->data, data, len);
    }

    void reset(const char * data, size_t len)
    {
        delete this->data;
        current = 0;
        this->len = len;
        this->data = (char *)malloc(len);
        memcpy(this->data, data, len);
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        if (current + len > this->len){
            size_t available_len = this->len - this->current;
            memcpy(*pbuffer, (const char *)(&this->data[this->current]),
                                            available_len);
            *pbuffer += available_len;
            this->current += available_len;
            LOG(LOG_INFO, "Generator transport has no more data");
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
        }
        memcpy(*pbuffer, (const char *)(&this->data[current]), len);
        *pbuffer += len;
        current += len;
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        // send perform like a /dev/null and does nothing in generator transport
    }
};

class CheckTransport : public Transport {

    public:
    bool status;
    size_t current;
    char * data;
    size_t len;


    CheckTransport(const char * data, size_t len)
        : Transport(), status(true), current(0), data(0), len(len)
    {
        this->data = (char *)malloc(len);
        memcpy(this->data, data, len);
    }

    void reset(const char * data, size_t len)
    {
        delete this->data;
        current = 0;
        this->len = len;
        this->data = (char *)malloc(len);
        memcpy(this->data, data, len);
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        // CheckTransport does never receive anything
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_RECV);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        size_t available_len = (this->current + len > this->len)?this->len - this->current:len;
        if (0 != memcmp(buffer, (const char *)(&this->data[this->current]), available_len)){
            // data differs
            this->status = false;
            // find where
            uint32_t differs = 0;
            for (size_t i = 0; i < available_len ; i++){
                if (buffer[i] != ((const char *)(&this->data[this->current]))[i]){
                    differs = i;
                    break;
                }
            }
            LOG(LOG_INFO, "=============== Common Part =======");
            hexdump(buffer, differs);
            LOG(LOG_INFO, "=============== Expected ==========");
            hexdump((const char *)(&this->data[this->current]) + differs, available_len - differs);
            LOG(LOG_INFO, "=============== Got ===============");
            hexdump(buffer+differs, available_len - differs);
        }
        this->current += available_len;
        if (available_len != len){
            LOG(LOG_INFO, "Check transport out of reference data");
            TODO("Maybe we should expose it as a SOCKET CLOSE event ?");
            this->status = false;
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
        }
    }
};

class TestTransport : public Transport {

    GeneratorTransport out;
    CheckTransport in;
    public:
    bool status;
    char name[256];
    uint32_t verbose;

    TestTransport(const char * name, const char * outdata, size_t outlen, const char * indata, size_t inlen, uint32_t verbose = 0)
        : out(outdata, outlen), in(indata, inlen), status(true), verbose(verbose)
    {
        strncpy(this->name, name, 254);
        this->name[255]=0;
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        if (this->status){
            this->out.recv(pbuffer, len);
            if (this->verbose & 0x100){
                LOG(LOG_INFO, "Recv done on %s (Test Data)", this->name);
                hexdump_c(*pbuffer - len, len);
                LOG(LOG_INFO, "Dump done on %s (Test Data)", this->name);
            }
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        if (this->status){
            if (this->verbose & 0x100){
                LOG(LOG_INFO, "Test Transport %s (Test Data) sending %u bytes", this->name, len);
                hexdump_c(buffer, len);
                LOG(LOG_INFO, "Dump done %s (Test Data) sending %u bytes", this->name, len);
            }
            this->in.send(buffer, len);
            this->status = this->in.status;
        }
    }
};

class OutFileTransport : public Transport {

    public:
    int fd;

    OutFileTransport(int fd) : Transport(), fd(fd) {}

    ~OutFileTransport() {}

    // recv is not implemented for OutFileTransport
    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        LOG(LOG_INFO, "OutFileTransport used for recv");
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_RECV, 0);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        ssize_t status = 0;
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            status = ::write(this->fd, buffer + total_sent, remaining_len);
            if (status > 0){
                remaining_len -= status;
                total_sent += status;
            }
            else {
                if (errno == EINTR){
                    continue;
                }
                LOG(LOG_INFO, "Outfile transport write failed with error %s", strerror(errno));
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
    }

};

class InFileTransport : public Transport {

    public:
    int fd;

    InFileTransport(int fd)
        : Transport(), fd(fd)
    {
    }

    ~InFileTransport()
    {
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        size_t status = 0;
        size_t remaining_len = len;
        char * buffer = *pbuffer;
        while (remaining_len) {
            status = ::read(this->fd, buffer, remaining_len);
            if (status > 0){
                remaining_len -= status;
                buffer += status;
            }
            else {
                if (errno == EINTR){
                    continue;
                }
                *pbuffer = buffer;
                LOG(LOG_INFO, "Infile transport read failed with error %s", strerror(errno));
                throw Error(ERR_TRANSPORT_READ_FAILED, 0);
            }
        }
        *pbuffer = buffer;
    }

    // send is not implemented for InFileTransport
    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        LOG(LOG_INFO, "InFileTransport used for writing");
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_SEND, 0);
    }

};

TODO("for now loop transport is not yet implemented, it's a null transport")

class LoopTransport : public Transport {
    public:
    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
    }
    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
    }
};

class SocketTransport : public Transport {
    public:
        int sck;
        int sck_closed;
        const char * name;
        uint32_t verbose;

    SocketTransport(const char * name, int sck, uint32_t verbose)
        : Transport(), name(name), verbose(verbose)
    {
        this->sck = sck;
        this->sck_closed = 0;
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

    void disconnect(){
        LOG(LOG_INFO, "Socket %s (%d) : closing connection\n", this->name, this->sck);
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
            int opt = 0;
            unsigned int opt_len = sizeof(opt);
            getsockopt(this->sck, SOL_SOCKET, SO_ERROR, (char*)(&opt), &opt_len);
            // Test if we got a socket error

            if (opt) {
                LOG(LOG_INFO, "Socket error detected on %s : %s", this->name, strerror(errno));
                throw Error(ERR_SESSION_TERMINATED);
            }

        }
    }

    using Transport::recv;
    virtual void recv(char ** input_buffer, size_t total_len) throw (Error)
    {
        if (this->verbose & 0x100){
            LOG(LOG_INFO, "Socket %s (%u) receiving %u bytes", this->name, this->sck, total_len);
        }
        char * start = *input_buffer;
        size_t len = total_len;
        char * pbuffer = *input_buffer;

        if (this->sck_closed) {
            LOG(LOG_INFO, "Socket %s (%u) already closed", this->name, this->sck);
            throw Error(ERR_SOCKET_ALLREADY_CLOSED);
        }

        while (len > 0) {
            ssize_t rcvd = ::recv(this->sck, pbuffer, len, 0);
            switch (rcvd) {
                case -1: /* error, maybe EAGAIN */
                    if (!this->try_again(errno)) {
                        LOG(LOG_INFO, "Closing socket %s (%u) on recv", this->name, this->sck);
                        this->sck_closed = 1;
                        throw Error(ERR_SOCKET_ERROR, errno);
                    }
                    this->wait_ready(RECV, 10);
                    break;
                case 0: /* no data received, socket closed */
                    LOG(LOG_INFO, "No data received. Socket %s (%u) closed on recv", this->name, this->sck);
                    this->sck_closed = 1;
                    throw Error(ERR_SOCKET_CLOSED);
                default: /* some data received */
                    pbuffer += rcvd;
                    len -= rcvd;
            }
        }

        if (this->verbose & 0x100){
            LOG(LOG_INFO, "Recv done on %s (%u)", this->name, this->sck);
            hexdump_c(start, total_len);
            LOG(LOG_INFO, "Dump done on %s (%u)", this->name, this->sck);
        }

        *input_buffer = pbuffer;
        total_received += total_len;
        last_quantum_received += total_len;
    }


    using Transport::send;

    virtual void send(const char * const buffer, size_t len) throw (Error)
    {
        if (this->verbose & 0x100){
            LOG(LOG_INFO, "Socket %s (%u) sending %u bytes", this->name, this->sck, len);
            hexdump_c(buffer, len);
            LOG(LOG_INFO, "Dump done %s (%u) sending %u bytes", this->name, this->sck, len);
        }
        if (this->sck_closed) {
            LOG(LOG_INFO, "Socket already closed on %s (%u)", this->name, this->sck);
            throw Error(ERR_SOCKET_ALLREADY_CLOSED);
        }
        size_t total = 0;
        while (total < len) {
            ssize_t sent = ::send(this->sck, buffer + total, len - total, 0);
            switch (sent){
            case -1:
                if (!this->try_again(errno)) {
                    this->sck_closed = 1;
                    LOG(LOG_INFO, "Socket %s (%u) : %s", this->name, this->sck, strerror(errno));
                    throw Error(ERR_SOCKET_ERROR, errno);
                }
                this->wait_ready(SEND, 10);
                break;
            case 0:
                this->sck_closed = 1;
                LOG(LOG_INFO, "Socket %s (%u) closed on sending : %s", this->name, this->sck, strerror(errno));
                throw Error(ERR_SOCKET_CLOSED, errno);
            default:
                total = total + sent;
            }
        }
        total_sent += len;
        last_quantum_sent += len;
        if (this->verbose & 0x100){
            LOG(LOG_INFO, "Send done on %s (%u)", this->name, this->sck);
        }
    }

    private:

};

#endif
