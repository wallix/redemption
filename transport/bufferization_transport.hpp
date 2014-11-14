/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef REDEMPTION_TRANSPORT_BUFFERIZATION_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_BUFFERIZATION_TRANSPORT_HPP

#include "transport.hpp"

static const size_t BUFFERIZATION_TRANSPORT_BUFFER_LENGTH = 1024 * 64;

/******************************
* BufferizationInTransport
*/

class BufferizationInTransport : public Transport {
    Transport & source_transport;

    BStream bufferized_data_stream;

    uint32_t verbose;

public:
    BufferizationInTransport(Transport & st, uint32_t verbose = 0)
    : Transport()
    , source_transport(st)
    , bufferized_data_stream(BUFFERIZATION_TRANSPORT_BUFFER_LENGTH)
    , verbose(verbose) {
    }

private:
    virtual void do_recv(char ** pbuffer, size_t len) {
        uint8_t * temp_data        = reinterpret_cast<uint8_t *>(*pbuffer);
        size_t    temp_data_length = len;

        while (temp_data_length) {
            if (this->bufferized_data_stream.in_remain()) {
                const size_t data_length = std::min<size_t>(temp_data_length, this->bufferized_data_stream.in_remain());

                this->bufferized_data_stream.in_copy_bytes(temp_data, data_length);

                temp_data        += data_length;
                temp_data_length -= data_length;
            }
            else {
                this->bufferized_data_stream.reset();

                this->source_transport.recv(
                      &this->bufferized_data_stream.end
                    , 4                                 // bufferized_data_length(4)
                    );

                const size_t bufferized_data_length = this->bufferized_data_stream.in_uint32_le();
                if (this->verbose) {
                    LOG( LOG_INFO, "BufferizationInTransport::do_recv: bufferized_data_length=%u"
                       , bufferized_data_length);
                }

                this->bufferized_data_stream.reset();

                this->source_transport.recv(&this->bufferized_data_stream.end, bufferized_data_length);
            }
        }   // while (temp_data_length)

        (*pbuffer) = (*pbuffer) + len;
    }
};  // class BufferizationInTransport


/******************************
* BufferizationOutTransport
*/

class BufferizationOutTransport : public Transport {
    Transport & target_transport;

    uint8_t bufferized_data[BUFFERIZATION_TRANSPORT_BUFFER_LENGTH];
    size_t  bufferized_data_length;

    uint32_t verbose;

public:
    BufferizationOutTransport(Transport & tt, uint32_t verbose = 0)
    : Transport()
    , target_transport(tt)
    , bufferized_data()
    , bufferized_data_length(0)
    , verbose(verbose) {
    }

    virtual ~BufferizationOutTransport() {
        if (this->bufferized_data_length) {
            this->send_to_target_ex(this->bufferized_data, this->bufferized_data_length);

            this->bufferized_data_length = 0;
        }
    }

private:
    virtual void do_send(const char * const buffer, size_t len) {
        if (this->verbose & 0x4) {
            LOG(LOG_INFO, "BufferizationOutTransport::do_send: len=%u", len);
        }

        const uint8_t * temp_data        = reinterpret_cast<const uint8_t *>(buffer);
        size_t          temp_data_length = len;

        while (temp_data_length) {
            if (this->bufferized_data_length) {
                const size_t data_length = std::min<size_t>(
                      temp_data_length
                    , sizeof(this->bufferized_data) - this->bufferized_data_length
                    );

                ::memcpy(this->bufferized_data + this->bufferized_data_length, temp_data, data_length);

                this->bufferized_data_length += data_length;

                temp_data += data_length;
                temp_data_length -= data_length;

                if (this->bufferized_data_length == sizeof(this->bufferized_data)) {
                    this->send_to_target_ex(this->bufferized_data, this->bufferized_data_length);

                    this->bufferized_data_length = 0;
                }
            }
            else {
                if (temp_data_length >= sizeof(this->bufferized_data)) {
                    this->send_to_target_ex(static_cast<const uint8_t *>(temp_data), sizeof(this->bufferized_data));

                    temp_data += sizeof(this->bufferized_data);
                    temp_data_length -= sizeof(this->bufferized_data);
                }
                else {
                    ::memcpy(this->bufferized_data, temp_data, temp_data_length);

                    this->bufferized_data_length = temp_data_length;

                    temp_data_length = 0;
                }
            }
        }

        if (this->verbose & 0x4) {
            LOG( LOG_INFO, "BufferizationOutTransport::do_send: bufferized_data_length=%u"
               , this->bufferized_data_length);
        }
    }

public:
    virtual bool next() {
        if (this->bufferized_data_length) {
            this->send_to_target_ex(this->bufferized_data, this->bufferized_data_length);

            this->bufferized_data_length = 0;
        }

        return this->target_transport.next();
    }

private:
    void send_to_target_ex(const uint8_t * data, size_t data_length) {
        if (!data_length)
            return;

        BStream buffer_stream(128);

        buffer_stream.out_uint32_le(data_length);
        if (this->verbose) {
            LOG( LOG_INFO
               , "BufferizationOutTransport::send_to_target_ex: data_length=%u"
               , data_length);
        }

        buffer_stream.mark_end();
        this->target_transport.send(buffer_stream);

        this->target_transport.send(data, data_length);
    }

public:
    virtual void timestamp(timeval now) {
        this->target_transport.timestamp(now);
    }
};  // class BufferizationOutTransport


#endif  // #ifndef REDEMPTION_TRANSPORT_BUFFERIZATION_TRANSPORT_HPP