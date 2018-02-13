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
   Copyright (C) Wallix 2018
   Author(s): Christophe Grosjean

   Wrapper code around zlib to compress or decompress
*/


#pragma once


template <size_t outsize = 65536>
class Zcompressor
{
    size_t offset;
    uint8_t out[outsize];
    z_stream z;
    public:
    Zcompressor()
    {
        this->z.zalloc = nullptr; // Z_NULL;
        this->z.zfree = nullptr; // Z_NULL;
        this->z.opaque = nullptr; //Z_NULL;
        deflateInit(&this->z, 9);
        this->offset = 0;
        this->z.next_out = &this->out[this->offset];
        this->z.avail_out = sizeof(this->out) - this->offset;
    }

    ~Zcompressor()
    {
        deflateEnd(&this->z);
    }


    // send more data to compressor
    // returns the amount of data processed
    size_t update(uint8_t * const data, size_t data_size)
    {
        this->z.next_in = data;
        this->z.avail_in = data_size;
        deflate(&z, Z_NO_FLUSH);
        return data_size  - this->z.avail_in;
    }

    // return the amount of compressed data available for emission
    size_t available() const {
        return sizeof(this->out) - this->offset - this->z.avail_out;
    }

    // TODO: we could return a pair buffer/size pointing to inner buffer
    // TODO: we mau also use a vector for that
    size_t flush_ready(uint8_t * data, size_t data_size){
        size_t to_send = data_size >= this->available() ? this->available() : data_size;
        memcpy(data, &this->out[this->offset], to_send);
        if (to_send < this->available()){
            this->offset += to_send;
            return to_send;
        }
        this->offset = 0;
        this->z.avail_out = sizeof(this->out);
        this->z.next_out = &this->out[0];
        return to_send;
    }

    bool full(){
        return this->z.avail_out == 0;
    }

    // finish flushes all buffer
    // we may need to call it several times (and flush ready data) until it returns true
    bool finish()
    {
        return deflate(&this->z, Z_FINISH) == Z_STREAM_END;
    }
};


template <size_t outsize = 131072>
class Zdecompressor
{
    size_t offset;
    uint8_t out[outsize];
    z_stream z;
    public:
    Zdecompressor()
    {
        this->z.zalloc = nullptr; // Z_NULL;
        this->z.zfree = nullptr; // Z_NULL;
        this->z.opaque = nullptr; //Z_NULL;
        inflateInit(&this->z);
        this->offset = 0;
        this->z.next_out = &this->out[this->offset];
        this->z.avail_out = sizeof(this->out) - this->offset;
    }

    ~Zdecompressor()
    {
        inflateEnd(&this->z);
    }


    // send more data to compressor
    // returns the amount of data processed
    size_t update(uint8_t * const data, size_t data_size)
    {
        this->z.next_in = data;
        this->z.avail_in = data_size;
        inflate(&z, Z_NO_FLUSH);
        return data_size  - this->z.avail_in;
    }

    // return the amount of compressed data available for emission
    size_t available() const {
        LOG(LOG_INFO, "available=%zu", sizeof(this->out) - this->offset - this->z.avail_out);
        return sizeof(this->out) - this->offset - this->z.avail_out;
    }

    size_t flush_ready(uint8_t * data, size_t data_size){
        size_t to_send = (data_size >= this->available()) ? this->available() : data_size;
        memcpy(data, &this->out[this->offset], to_send);
        if (to_send < this->available()){
            this->offset += to_send;
            return to_send;
        }
        this->offset = 0;
        this->z.avail_out = sizeof(this->out);
        this->z.next_out = &this->out[0];
        return to_send;
    }

    size_t flush_ready(std::vector<uint8_t> & vec){
        const size_t data_ready = vec.size();
        const size_t to_send = this->available();
        vec.resize(data_ready+to_send);
        memcpy(&vec[data_ready], &this->out[this->offset], to_send);
        this->offset = 0;
        this->z.avail_out = sizeof(this->out);
        this->z.next_out = &this->out[0];
        return to_send;
    }


    bool full(){
        return this->z.avail_out == 0;
    }

    // finish flushes all buffer
    // we may need to call it several times (and flush ready data) until it returns true
    bool finish()
    {
        return inflate(&this->z, Z_FINISH) == Z_STREAM_END;
    }
};

