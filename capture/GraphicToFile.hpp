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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Jonathan Poelen

   RDPGraphicDevice is an abstract class that describe a device able to
   proceed RDP Drawing Orders. How the drawing will be actually done
   depends on the implementation.
   - It may be sent on the wire,
   - Used to draw on some internal bitmap,
   - etc.

*/

#if !defined(__GRAPHIC_TO_FILE_HPP__)
#define __GRAPHIC_TO_FILE_HPP__

#include <sys/time.h>
#include <ctime>

#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"
#include "RDP/rdp.hpp"
#include "RDP/sec.hpp"
#include "RDP/lic.hpp"
#include "RDP/RDPSerializer.hpp"
#include "difftimeval.hpp"
#include "image_capture.hpp"

class WRMChunk_Send
{
    public:
    WRMChunk_Send(Stream & stream, uint16_t chunktype, uint16_t data_size, uint16_t count)
    {
        stream.out_uint16_le(chunktype);
        stream.out_uint32_le(8 + data_size);
        stream.out_uint16_le(count);
        stream.mark_end();
    } 
};


struct GraphicToFile : public RDPGraphicDevice
REDOC("To keep things easy all chunks have 8 bytes headers"
      " starting with chunk_type, chunk_size"
      " and order_count (whatever it means, depending on chunks")
{
    timeval last_sent_timer;
    timeval timer;
    const uint16_t width;
    const uint16_t height;
    const uint8_t  bpp;
    ImageCapture * image;
    RDPSerializer * serializer;
    Transport * trans;

    GraphicToFile(const timeval& now
                , Transport * trans
                , Stream * pstream
                , const Inifile * ini
                , const uint16_t width
                , const uint16_t height
                , const uint8_t  bpp
                , uint32_t small_entries
                , uint32_t small_size
                , uint32_t medium_entries
                , uint32_t medium_size
                , uint32_t big_entries
                , uint32_t big_size)
    : timer(now)
    , width(width)
    , height(height)
    , bpp(bpp)
    , trans(trans)
    {
        this->serializer = new RDPSerializer(trans, pstream, ini, bpp,
                    small_entries, small_size,
                    medium_entries, medium_size,
                    big_entries, big_size,
                    0, 1, 1);
    
        this->image = new ImageCapture(*trans, width, height, true);

        last_sent_timer.tv_sec = 0;
        last_sent_timer.tv_usec = 0;
        this->serializer->order_count = 0;
        
        this->send_meta_chunk();
    }

    ~GraphicToFile(){
    }

    virtual void timestamp(const timeval& now)
    REDOC("Update timestamp but send nothing, the timestamp will be sent later with the next effective event")
    {
        uint64_t old_timer = this->timer.tv_sec * 1000000ULL + this->timer.tv_usec;
        uint64_t current_timer = now.tv_sec * 1000000ULL + now.tv_usec;
        if (old_timer < current_timer){
            this->timer = now;
        }
    }

    void send_meta_chunk(void)
    {
        BStream stream(8);
        stream.out_uint16_le(this->width);
        stream.out_uint16_le(this->height);
        stream.out_uint16_le(this->bpp);
        stream.out_uint16_le(0);
        stream.mark_end();

        BStream header(8);
        WRMChunk_Send chunk(header, META_FILE, 8, 1);
        this->trans->send(header.data, header.size());
        this->trans->send(stream.data, stream.size());
    }

    // this one is used to store some embedded image inside WRM
    void send_image_chunk(void)
    {
        BStream stream(107);
        stream.out_copy_bytes(
    "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a"                                 //.PNG....
    "\x00\x00\x00\x0d\x49\x48\x44\x52"                                 //....IHDR
    "\x00\x00\x00\x14\x00\x00\x00\x0a\x08\x02\x00\x00\x00"             //.............
    "\x3b\x37\xe9\xb1"                                                 //;7..
    "\x00\x00\x00\x32\x49\x44\x41\x54"                                 //...2IDAT
    "\x28\x91\x63\xfc\xcf\x80\x17\xfc\xff\xcf\xc0\xc8\x88\x4b\x92\x09" //(.c..........K..
    "\xbf\x5e\xfc\x60\x88\x6a\x66\x41\xe3\x33\x32\xa0\x84\xe0\x7f\x54" //.^.`.jfA.32....T
    "\x91\xff\x0c\x28\x81\x37\x70\xce\x66\x1c\xb0\x78\x06\x00\x69\xdc" //...(.7p.f..x..i.
    "\x0a\x12"                                                         //..
    "\x86\x4a\x0c\x44"                                                 //.J.D
    "\x00\x00\x00\x00\x49\x45\x4e\x44"                                 //....IEND
    "\xae\x42\x60\x82"                                                 //.B`.
        , 107);
        stream.mark_end();

        BStream header(8);
        WRMChunk_Send chunk(header, IMAGE_CHUNK, 107, 1);
        this->trans->send(header.data, header.size());
        this->trans->send(stream.data, stream.size());
    }


    void send_timestamp_chunk(void)
    {
        uint64_t old_timer = this->last_sent_timer.tv_sec * 1000000ULL + this->last_sent_timer.tv_usec;
        uint64_t current_timer = this->timer.tv_sec * 1000000ULL + this->timer.tv_usec;
        if (old_timer < current_timer){
            BStream stream(8);
            stream.out_uint64_le(current_timer);
            stream.mark_end();

            BStream header(8);
            WRMChunk_Send chunk(header, TIMESTAMP, 8, 1);
            this->trans->send(header.data, header.size());
            this->trans->send(stream.data, stream.size());
        }
    }

    virtual void flush()
    {
        if (this->serializer->order_count > 0){
            this->send_timestamp_chunk();
            this->send_orders_chunk();
        }
    }

    void send_orders_chunk()
    {
        this->serializer->pstream->mark_end();
        BStream header(8);
        WRMChunk_Send chunk(header, RDP_UPDATE_ORDERS, this->serializer->pstream->size(), this->serializer->order_count);
        this->trans->send(header.data, header.size());
        this->serializer->flush();
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) 
    {
        this->serializer->draw(cmd, clip);
    }
    virtual void draw(const RDPScrBlt & cmd, const Rect &clip)
    {
        this->serializer->draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        this->serializer->draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->serializer->draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->serializer->draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo& cmd, const Rect & clip)
    {
        this->serializer->draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        this->serializer->draw(cmd, clip);
    }

};

#endif
