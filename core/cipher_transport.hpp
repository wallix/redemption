/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(__CORE_CIPHER_TRANSPORT__)
#define __CORE_CIPHER_TRANSPORT__

#include "cipher.hpp"
#include "transport.hpp"

class OutCipherTransport
: public Transport
{
    unsigned char buf_data[1024];
    CipherCryptData cipher_data;
    CipherCrypt cipher_crypt;

public:
    Transport * out;

public:
    OutCipherTransport(Transport * out)
    : cipher_data(buf_data)
    , cipher_crypt(CipherCrypt::EncryptConstruct(), &this->cipher_data)
    , out(out)
    {
    }

    ~OutCipherTransport()
    {}

    void reset()
    {
        this->cipher_data.reset();
    }

    bool start(const EVP_CIPHER * mode,
               const unsigned char* key = 0,
               const unsigned char* iv = 0,
               ENGINE* impl = 0)
    {
        return this->cipher_crypt.start(mode, key, iv, impl);
    }

    bool stop()
    {
        if (this->cipher_crypt.stop())
        {
            if (!cipher_data.empty())
                this->_send_data();
            return true;
        }
        return false;
    }

    // recv is not implemented for OutCipherTransport
    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        LOG(LOG_INFO, "OutCipherTransport used for recv");
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_RECV, 0);
    }

    virtual void send(const char * const buffer, size_t len) throw (Error)
    {
        this->do_send((const uint8_t * const)buffer, len);
    }
    void send(const uint8_t * const buffer, size_t len) throw (Error)
    {
        this->do_send(buffer, len);
    }

private:
    void _send_data()
    {
        this->out->send(this->buf_data, this->cipher_data.size());
    }

    void do_send(const uint8_t * const buffer, size_t len) throw (Error)
    {
        std::size_t remaining_len = 0;
        while (len)
        {
            std::size_t size = std::min((sizeof(buf_data) - EVP_MAX_BLOCK_LENGTH), len);
            this->cipher_crypt.update(buffer + remaining_len, size);
            if (!this->cipher_data.empty())
            {
                this->_send_data();
                this->cipher_data.reset();
            }
            len -= size;
            remaining_len += size;
        }
    }
};

class InCipherTransport
: public Transport
{
    unsigned char buf_data[1024];
    CipherCryptData cipher_data;
    CipherCrypt cipher_crypt;
    std::size_t remaining_data;
    unsigned char buf[EVP_MAX_BLOCK_LENGTH];
    std::size_t use_buf;
    bool is_stop;
public:
    Transport * in;

public:
    InCipherTransport(Transport * in)
    : cipher_data(buf_data)
    , cipher_crypt(CipherCrypt::DecryptConstruct(), &this->cipher_data)
    , remaining_data(0)
    , use_buf(0)
    , is_stop(false)
    , in(in)
    {
    }

    ~InCipherTransport()
    {
    }

    void reset()
    {
        this->cipher_data.reset();
        this->remaining_data = 0;
        this->use_buf = 0;
        this->is_stop = false;
    }

    bool start(const EVP_CIPHER * mode,
               const unsigned char* key = 0,
               const unsigned char* iv = 0,
               ENGINE* impl = 0)
    {
        return this->cipher_crypt.start(mode, key, iv, impl);
    }

    bool stop()
    {
        return this->cipher_crypt.stop();
    }

    virtual void recv(char ** pbuffer, size_t len) throw (Error)
    {
        this->do_recv((uint8_t**)pbuffer, len);
    }
    void recv(uint8_t ** pbuffer, size_t len) throw (Error)
    {
        this->do_recv(pbuffer, len);
    }

    // send is not implemented for InCipherTransport
    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        LOG(LOG_INFO, "InCipherTransport used for writing");
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_SEND, 0);
    }

private:
    size_t _copy_data_buf(uint8_t ** pbuffer, size_t len)
    {
        std::size_t size = 0;
        if (!this->cipher_data.empty() && this->cipher_data.size() != this->remaining_data)
        {
            size = std::min(
                this->cipher_data.size() - this->remaining_data, len);
            memcpy(*pbuffer,
                   this->cipher_data.data + this->remaining_data,
                   size);
            *pbuffer += size;
            if (size == len)
                this->remaining_data += size;
            else
                this->remaining_data = 0;
        }
        return size;
    }

    std::size_t _copy_recv(uint8_t ** pbuffer, size_t len) throw (Error)
    {
        unsigned char * const tmp = *pbuffer;
        while (len >= EVP_MAX_BLOCK_LENGTH)
        {
            this->cipher_data.reset();
            this->in->recv(&this->cipher_data.pbuf, (sizeof(buf_data) - EVP_MAX_BLOCK_LENGTH));
            std::size_t size = std::min(this->cipher_data.size(), len);
            CipherCryptData tmp = this->cipher_data;
            this->cipher_data.reset(*pbuffer);
            this->cipher_crypt.update(tmp.data, size);
            *pbuffer = this->cipher_data.pbuf;
            if (0 == this->cipher_data.size())
                throw Error(ERR_TRANSPORT_READ_FAILED);
            len -= this->cipher_data.size();
            this->cipher_data = tmp;
            if (!this->cipher_data.size() == (sizeof(buf_data) - EVP_MAX_BLOCK_LENGTH))
            {
                this->remaining_data = this->cipher_data.size();
                break ;
            }
        }
        return *pbuffer - tmp;
    }

    void _recv_in_buf()
    {
        unsigned char * p = this->buf;
        this->in->recv(&p, sizeof(this->buf));
        this->use_buf = p - this->buf;
        this->cipher_data.reset();
        this->cipher_crypt.update(this->buf, this->use_buf);
        this->remaining_data = 0;
    }

    void do_recv(uint8_t ** pbuffer, size_t len) throw (Error)
    {
        while (len
            && (len -= this->_copy_data_buf(pbuffer, len))
            && (len -= this->_copy_recv(pbuffer, len))
        ) {
            this->_recv_in_buf();
            if (!this->use_buf)
            {
                if (this->is_stop)
                {
                    throw Error(ERR_TRANSPORT_READ_FAILED);
                }
                this->cipher_crypt.stop();
                len -= this->_copy_data_buf(pbuffer, len);
                if (len)
                {
                    throw Error(ERR_TRANSPORT_READ_FAILED);
                }
                this->is_stop = true;
                break;
            }
        }
    }
};


class RaiiOutCipherTransport
{
    OutCipherTransport &ref_trans;

public:
    RaiiOutCipherTransport(OutCipherTransport& trans,
                          const EVP_CIPHER * mode,
                          const unsigned char* key = 0,
                          const unsigned char* iv = 0,
                          ENGINE* impl = 0)
    : ref_trans(trans)
    {
        this->ref_trans.start(mode, key, iv, impl);
    }

    ~RaiiOutCipherTransport()
    {
        this->ref_trans.stop();
    }
};

class RaiiInCipherTransport
{
    InCipherTransport &ref_trans;

public:
    RaiiInCipherTransport(InCipherTransport& trans,
                          const EVP_CIPHER * mode,
                          const unsigned char* key = 0,
                          const unsigned char* iv = 0,
                          ENGINE* impl = 0)
    : ref_trans(trans)
    {
        this->ref_trans.start(mode, key, iv, impl);
    }

    ~RaiiInCipherTransport()
    {
        this->ref_trans.stop();
    }
};

#endif