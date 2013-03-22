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

#ifndef _REDEMPTION_CORE_CIPHER_TRANSPORT_HPP_
#define _REDEMPTION_CORE_CIPHER_TRANSPORT_HPP_

#include"cipher.hpp"
#include"transport.hpp"

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
    {
        this->cipher_crypt.clean();
    }

    const EVP_CIPHER_CTX& ctx() const
    {
        return this->cipher_crypt.ctx();
    }

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
            this->cipher_crypt.clean();
            return true;
        }
        this->cipher_crypt.clean();
        return false;
    }

    // recv is not implemented for OutCipherTransport
    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        LOG(LOG_INFO, "OutCipherTransport used for recv");
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND, 0);
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
    uint8_t crypt_buf[1024];
    uint8_t uncrypt_buf[EVP_MAX_BLOCK_LENGTH*2];
    CipherCryptData cipher_data;
    CipherCrypt cipher_crypt;
    int status_in;

    struct Buffer
    {
        uint8_t* data;
        uint8_t* pbuf;
        uint8_t* pend;

        Buffer(uint8_t* pbegin, std::size_t size)
        : data(pbegin)
        , pbuf(pbegin)
        , pend(pbegin + size)
        {}

        bool empty() const
        { return this->data == this->pbuf; }

        std::size_t size() const
        { return this->pbuf - this->data; }

        std::size_t remain() const
        { return this->pend - this->pbuf; }
    };

    CipherCryptData crypt;
    Buffer uncrypt;

public:
    Transport * in;

public:
    InCipherTransport(Transport * in)
    : cipher_data(0)
    , cipher_crypt(CipherCrypt::DecryptConstruct(), &this->cipher_data)
    , status_in(0)
    , crypt(this->crypt_buf)
    , uncrypt(this->uncrypt_buf, 0)
    , in(in)
    {
    }

    ~InCipherTransport()
    {
        this->cipher_crypt.clean();
    }

    const EVP_CIPHER_CTX& ctx() const
    {
        return this->cipher_crypt.ctx();
    }

    void reset()
    {
        this->status_in = 0;
        this->crypt.reset(this->crypt_buf);
        this->uncrypt.data = this->uncrypt_buf;
        this->uncrypt.pbuf = this->uncrypt_buf;
        this->uncrypt.pend = this->uncrypt_buf;
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
        bool ret = true;
        if (this->status_in == 0)
        {
            this->cipher_data.reset(this->uncrypt.data);
            ret = this->cipher_crypt.stop();
        }
        this->cipher_crypt.clean();
        return ret;
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
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV, 0);
    }

private:
    std::size_t _uncrypt_move(uint8_t ** pbuffer, size_t len) throw ()
    {
        std::size_t size = std::min(this->uncrypt.remain(), len);
        memcpy(*pbuffer, this->uncrypt.pbuf, size);
        *pbuffer += size;
        this->uncrypt.pbuf += size;
        return size;
    }

    void _decrypt_in_uncrypt() throw ()
    {
        this->uncrypt.pbuf = this->uncrypt_buf;
        this->cipher_data.reset(this->uncrypt.data);
        std::size_t size = std::min<std::size_t>(this->crypt.size(),
                                                 EVP_MAX_BLOCK_LENGTH);
        this->_cipher_update(size);
        this->uncrypt.pbuf = this->uncrypt.data;
        this->uncrypt.pend = this->uncrypt.data + this->cipher_data.size();
    }

    void _cipher_update(std::size_t size) throw (Error)
    {
        if (!this->cipher_crypt.update(this->crypt.data, size))
        {
            throw Error(ERR_CIPHER_UPDATE);
        }
        this->crypt.data += size;
    }

    void _recv()
    {
        this->crypt.reset(this->crypt_buf);
        this->in->recv(&this->crypt.pbuf, sizeof(this->crypt_buf));
    }

    std::size_t _recv_and_uncrypt_block(uint8_t ** pbuffer, std::size_t len)
    {
        this->_recv();
        if (crypt.size() != sizeof(this->crypt_buf))
        {
            this->status_in = 1;
        }
        if (!crypt.empty())
        {
            this->_decrypt_in_uncrypt();
            return this->_uncrypt_move(pbuffer, len);
        }
        return 0;
    }

    std::size_t _status_in_equal_to_1(uint8_t ** pbuffer, std::size_t len)
    {
        this->uncrypt.pbuf = this->uncrypt_buf;
        this->cipher_data.reset(this->uncrypt.data);
        this->cipher_crypt.stop();
        this->uncrypt.pbuf = this->uncrypt.data;
        this->uncrypt.pend = this->uncrypt.data + this->cipher_data.size();
        this->status_in = 2;
        return this->_uncrypt_move(pbuffer, len);
    }

    void do_recv(uint8_t ** pbuffer, size_t len) throw (Error)
    {
        if (!this->uncrypt.empty() && !(len -= _uncrypt_move(pbuffer, len)))
            return ;

        if (!this->crypt.empty())
        {
            if (len > EVP_MAX_BLOCK_LENGTH)
            {
                this->cipher_data.reset(*pbuffer);
                std::size_t size = std::min(this->crypt.size(),
                                            len - EVP_MAX_BLOCK_LENGTH);
                this->_cipher_update(size);
                *pbuffer = this->cipher_data.pbuf;
                if (!(len -= this->cipher_data.size()))
                    return ;
            }
            if (len && !this->crypt.empty())
            {
                this->_decrypt_in_uncrypt();
                if (!(len -= this->_uncrypt_move(pbuffer, len)))
                    return ;
            }
        }

        if (this->status_in != 0)
        {
            if (this->status_in == 2)
                throw Error(ERR_TRANSPORT_READ_FAILED);
            len -= _status_in_equal_to_1(pbuffer, len);
            if (len)
                throw Error(ERR_TRANSPORT_READ_FAILED);
        }

        while (len > EVP_MAX_BLOCK_LENGTH)
        {
            this->_recv();
            if (this->crypt.empty())
                throw Error(ERR_TRANSPORT_READ_FAILED);
            std::size_t size = std::min(this->crypt.size(),
                                        len - EVP_MAX_BLOCK_LENGTH);
            this->cipher_data.reset(*pbuffer);
            this->_cipher_update(size);
            len -= this->cipher_data.size();
            *pbuffer = this->cipher_data.pbuf;
        }

        if (len)
        {
            if (this->crypt.empty())
            {
                switch (this->status_in)
                {
                    case 1:
                        len -= _status_in_equal_to_1(pbuffer, len);
                        break;
                    case 2:
                        throw Error(ERR_TRANSPORT_READ_FAILED);
                        break;
                    default:
                        len -= this->_recv_and_uncrypt_block(pbuffer, len);
                        if (len && this->status_in == 1)
                            len -= _status_in_equal_to_1(pbuffer, len);
                        break;
                }
            }
            else
            {
                this->_decrypt_in_uncrypt();
                len -= this->_uncrypt_move(pbuffer, len);
                if (len && this->crypt.empty())
                {
                    len -= this->_recv_and_uncrypt_block(pbuffer, len);
                    if (len && this->status_in == 1)
                        len -= _status_in_equal_to_1(pbuffer, len);
                }
            }
        }

        if (len)
            throw Error(ERR_TRANSPORT_READ_FAILED);
    }
};

#endif
