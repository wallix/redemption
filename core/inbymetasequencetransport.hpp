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

#ifndef _REDEMPTION_CORE_INBYMETASEQUENCETRANSPORT_HPP_
#define _REDEMPTION_CORE_INBYMETASEQUENCETRANSPORT_HPP_

#include "transport.hpp"

TODO("This readline function could probably move into stream to give some level of support for text oriented files")
static inline bool readline(int fd, char ** begin, char **end, char **eol, char buffer[], size_t len)
{
    for (char * p = *begin; p < *end; p++){
        if (*p == '\n'){
            *eol = p+1;
            return true;
        }
    }
    size_t trailing_space = buffer + len - *end;
    // reframe buffer if no trailing space left
    if (trailing_space == 0){
        size_t used_len = *end - *begin;
        memmove(buffer, *begin, used_len);
        *end = buffer + used_len;
        *begin = buffer;
    }
    ssize_t rcvd = 0;
    do {
        rcvd = ::read(fd, *end, buffer + len - *end);
    } while (rcvd < 0 && errno == EINTR);
    if (rcvd < 0){
        throw Error(ERR_TRANSPORT_READ_FAILED, 0);
    }
    if (rcvd == 0){
        if (*begin != *end) {
            *eol = *end;
            return false;
        }
        throw Error(ERR_TRANSPORT_READ_FAILED, 0);
    }
    *end += rcvd;
    for (char * p = *begin; p < *end; p++){
        if (*p == '\n'){
            *eol = p+1;
            return true;
        }
    }
    *eol = *end;
    return false;
}

class InByMetaSequenceTransport : public InFileTransport {
    char meta_filename[2048];
    int meta_fd;
    char buffer[2048];
    char * begin;
    char * end;
public:
    char path[2048];
    uint32_t begin_chunk_time;
    uint32_t end_chunk_time;
    unsigned chunk_num;

    InByMetaSequenceTransport(const char * meta_filename)
    : InFileTransport(-1)
    , meta_fd(-1)
    , begin(this->buffer)
    , end(this->buffer)
    , begin_chunk_time(0)
    , end_chunk_time(0)
    , chunk_num(0)
    {
        this->path[0] = 0;
        strcpy(this->meta_filename, meta_filename);
        this->reset_meta();
    }

    ~InByMetaSequenceTransport()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
        if (this->meta_fd != -1){
            ::close(this->meta_fd);
            this->meta_fd = -1;
        }
    }

    void reset_meta(){
        if (this->meta_fd != -1){
            ::close(this->meta_fd);
        }
        this->begin = this->end = this->buffer;
        printf("opening %s\n", this->meta_filename);
        this->meta_fd = ::open(this->meta_filename, O_RDONLY);
        char * eol = NULL;
        if(!readline(this->meta_fd, &this->begin, &this->end, &eol, this->buffer, sizeof(this->buffer))){
            LOG(LOG_INFO, "InByMetaSequenceTransport recv failed with error %s reading meta file line 0", strerror(errno));
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        };
        this->begin = eol;
        if(!readline(this->meta_fd, &this->begin, &this->end, &eol, this->buffer, sizeof(this->buffer))){
            LOG(LOG_INFO, "InByMetaSequenceTransport recv failed with error %s reading meta file line 1", strerror(errno));
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        };
        this->begin = eol;
        if(!readline(this->meta_fd, &this->begin, &this->end, &eol, this->buffer, sizeof(this->buffer))){
            LOG(LOG_INFO, "InByMetaSequenceTransport recv failed with error %s reading meta file line 2", strerror(errno));
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        };
        this->begin = eol;
        this->chunk_num = 0;
    }

    void next_chunk_info()
    {
        char * eol = NULL;
        bool res = readline(this->meta_fd, &this->begin, &this->end, &eol, this->buffer, sizeof(this->buffer));
        if (!res) {
            LOG(LOG_INFO, "InByMetaSequenceTransport recv failed with error %s reading meta file", strerror(errno));
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }
        char *eol2 = strchrnul(this->begin, ' ');
        if (eol2){
            memcpy(this->path, this->begin, eol2 - this->begin);
            this->path[eol2 - this->begin] = 0;
            this->begin = eol;
        }
        else {
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }

        TODO("Make this code harder, input sanity is not checked")
        this->begin_chunk_time = atol(eol2+1);
        char *eol3 = strchrnul(eol2+1, ' ');
        this->end_chunk_time = atol(eol3+1);

        this->chunk_num++;
    }

    TODO("Code below looks insanely complicated for what it is doing. I should probably stop at some point"
         "and *THINK* about the API that transport objects should really provide."
         "For instance I strongly suspect that it should be allowed to stop returning only part of the asked datas"
         "like the file system transport objects. There should also be an easy way to combine several layers of"
         "transports (using templates ?) and clearly define the properties of objects providing the sources of datas"
         "(some abstraction above file ?). The current sequences are easy to use, but somewhat limited")
    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        size_t remaining_len = len;
        while (remaining_len > 0){
            if (this->fd == -1){
                this->next_chunk_info();
                printf("opening new source WRM %s\n", this->path);
                this->fd = ::open(this->path, O_RDONLY);
                if (this->fd == -1){
                    LOG(LOG_INFO, "InByMetaSequence transport '%s' recv failed with error : %s", this->path, strerror(errno));
                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                }
            }
            char * oldpbuffer = *pbuffer;
            try {
                InFileTransport::recv(pbuffer, remaining_len);
                // if recv returns it has read everything asked for, otherwise it will raise some exception
                remaining_len = 0;
            }
            catch (const Error & e) {
                if (e.id == ERR_TRANSPORT_NO_MORE_DATA){
                    remaining_len -= *pbuffer - oldpbuffer;
                    this->next();
                }
                else {
                    throw;
                }
            };
        }
    }

    virtual bool next()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
        this->InFileTransport::next();
        return true;
    }
};


class InByMetaSequenceTransport2 : public Transport {
public:
    char path[1024];
    unsigned begin_chunk_time;
    unsigned end_chunk_time;
    unsigned chunk_num;


    InByMetaSequenceTransport2(const char * meta_filename)
    : Transport()
    {
        memset(this->path, 0, sizeof(path));
        this->begin_chunk_time = 0;
        this->end_chunk_time = 0;
    }

    ~InByMetaSequenceTransport2()
    {
    }

    void reset_meta(){
    }

    void next_chunk_info()
    {
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) 
    {
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV, 0);
    }

    virtual bool next()
    {
        return true;
    }
};

#endif
