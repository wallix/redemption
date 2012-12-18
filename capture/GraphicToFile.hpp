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

#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"
#include "RDP/rdp.hpp"
#include "RDP/sec.hpp"
#include "RDP/lic.hpp"
#include "RDP/RDPSerializer.hpp"
#include "difftimeval.hpp"
#include "png.hpp"
#include "error.hpp"
#include "config.hpp"
#include "RDP/caches/bmpcache.hpp"
#include "colors.hpp"

#include "RDP/RDPDrawable.hpp"

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

template <size_t SZ>
class OutChunkedBufferingTransport : public Transport
{
public:
    Transport * trans;
    size_t max;
    BStream stream;
    OutChunkedBufferingTransport(Transport * trans)
        : trans(trans)
        , max(SZ-8)
        , stream(SZ)
    {
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        // CheckTransport does never receive anything
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error)
    {   
        size_t to_buffer_len = len;
        while (this->stream.size() + to_buffer_len > max){
            BStream header(8);
            WRMChunk_Send chunk(header, PARTIAL_IMAGE_CHUNK, max, 1);
            this->trans->send(header.data, header.size());
            this->trans->send(this->stream.data, this->stream.size());
            size_t to_send = max - this->stream.size();
            this->trans->send(buffer + len - to_buffer_len, to_send);
            to_buffer_len -= to_send;
            this->stream.reset();    
        }
        this->stream.out_copy_bytes(buffer + len - to_buffer_len, to_buffer_len);
        REDOC("Marking end here is necessary for chunking")
        this->stream.mark_end();
    }

    virtual void flush() {
        this->stream.mark_end();
        if (this->stream.size() > 0){
            BStream header(8);
            WRMChunk_Send chunk(header, LAST_IMAGE_CHUNK, this->stream.size(), 1);
            this->trans->send(header.data, header.size());
            this->trans->send(this->stream.data, this->stream.size());
        }
    }
};

struct GraphicToFile : public RDPSerializer
REDOC("To keep things easy all chunks have 8 bytes headers"
      " starting with chunk_type, chunk_size"
      " and order_count (whatever it means, depending on chunks")
{
    Transport * trans;
    BStream buffer_stream;

    timeval last_sent_timer;
    timeval timer;
    const uint16_t width;
    const uint16_t height;
    const uint8_t  bpp;
    RDPDrawable * drawable;

    GraphicToFile(const timeval& now
                , Transport * trans
                , const uint16_t width
                , const uint16_t height
                , const uint8_t  bpp
                , BmpCache & bmp_cache
                , RDPDrawable * drawable
                , const Inifile & ini)
    : RDPSerializer(trans, this->buffer_stream, bpp, bmp_cache, 0, 1, 1, ini)
    , trans(trans)
    , buffer_stream(65536)
    , last_sent_timer() 
    , timer(now)
    , width(width)
    , height(height)
    , bpp(bpp)
    , drawable(drawable)
    {
        last_sent_timer.tv_sec = 0;
        last_sent_timer.tv_usec = 0;
        this->order_count = 0;
        
        this->send_meta_chunk();
        this->send_image_chunk();
    }

    ~GraphicToFile(){
    }

    virtual void timestamp(const timeval& now)
    REDOC("Update timestamp but send nothing, the timestamp will be sent later with the next effective event")
    {
        uint64_t old_timer = this->timer.tv_sec * 1000000ULL + this->timer.tv_usec;
        uint64_t current_timer = now.tv_sec * 1000000ULL + now.tv_usec;
        if (old_timer < current_timer){
            this->flush();
            this->timer = now;
            this->trans->timestamp(now);
        }
    }

    void send_meta_chunk(void)
    {
        BStream header(8);
        BStream payload(20);
        payload.out_uint16_le(1); // WRM FORMAT VERSION1
        payload.out_uint16_le(this->width);
        payload.out_uint16_le(this->height);
        payload.out_uint16_le(this->bpp);
        payload.out_uint16_le(this->bmp_cache.small_entries);
        payload.out_uint16_le(this->bmp_cache.small_size);
        payload.out_uint16_le(this->bmp_cache.medium_entries);
        payload.out_uint16_le(this->bmp_cache.medium_size);
        payload.out_uint16_le(this->bmp_cache.big_entries);
        payload.out_uint16_le(this->bmp_cache.big_size);
        payload.mark_end();

        WRMChunk_Send chunk(header, META_FILE, payload.size(), 1);

        this->trans->send(header.data, header.size());
        this->trans->send(payload.data, payload.size());
    }

    // this one is used to store some embedded image inside WRM
    void send_image_chunk(void)
    {
        OutChunkedBufferingTransport<65536> png_trans(trans);

        ::transport_dump_png24(&png_trans, this->drawable->drawable.data,
                     this->drawable->drawable.width, this->drawable->drawable.height,
                     this->drawable->drawable.rowsize
                    );
    }

    void send_timestamp_chunk(void)
    {
        BStream payload(8);
        payload.out_uint64_le(this->timer.tv_sec * 1000000ULL + this->timer.tv_usec);
        payload.mark_end();

        BStream header(8);
        WRMChunk_Send chunk(header, TIMESTAMP, 8, 1);
        this->trans->send(header.data, header.size());
        this->trans->send(payload.data, payload.size());
    }

    void send_save_state_chunk()
    {
        BStream payload(2048);
        // RDPOrderCommon common;
        payload.out_uint8(this->common.order);
        payload.out_uint16_le(this->common.clip.x);
        payload.out_uint16_le(this->common.clip.y);
        payload.out_uint16_le(this->common.clip.cx);
        payload.out_uint16_le(this->common.clip.cy);
        // RDPDestBlt destblt;
        payload.out_uint16_le(this->destblt.rect.x);
        payload.out_uint16_le(this->destblt.rect.y);
        payload.out_uint16_le(this->destblt.rect.cx);
        payload.out_uint16_le(this->destblt.rect.cy);
        payload.out_uint8(this->destblt.rop);
        // RDPDestBlt destblt;
        payload.out_uint16_le(this->patblt.rect.x);
        payload.out_uint16_le(this->patblt.rect.y);
        payload.out_uint16_le(this->patblt.rect.cx);
        payload.out_uint16_le(this->patblt.rect.cy);
        payload.out_uint8(this->patblt.rop);
        payload.out_uint32_le(this->patblt.back_color);
        payload.out_uint32_le(this->patblt.fore_color);
        payload.out_uint8(this->patblt.brush.org_x);
        payload.out_uint8(this->patblt.brush.org_y);
        payload.out_uint8(this->patblt.brush.style);
        payload.out_uint8(this->patblt.brush.hatch);
        payload.out_copy_bytes(this->patblt.brush.extra, 7);
        // RDPScrBlt scrblt;
        payload.out_uint16_le(this->scrblt.rect.x);
        payload.out_uint16_le(this->scrblt.rect.y);
        payload.out_uint16_le(this->scrblt.rect.cx);
        payload.out_uint16_le(this->scrblt.rect.cy);
        payload.out_uint8(this->scrblt.rop);
        payload.out_uint16_le(this->scrblt.srcx);
        payload.out_uint16_le(this->scrblt.srcy);
        // RDPOpaqueRect opaquerect;
        payload.out_uint16_le(this->opaquerect.rect.x);
        payload.out_uint16_le(this->opaquerect.rect.y);
        payload.out_uint16_le(this->opaquerect.rect.cx);
        payload.out_uint16_le(this->opaquerect.rect.cy);
        payload.out_uint8(this->opaquerect.color);
        payload.out_uint8(this->opaquerect.color >> 8);
        payload.out_uint8(this->opaquerect.color >> 16);
        // RDPMemBlt memblt;
        payload.out_uint16_le(this->memblt.cache_id);
        payload.out_uint16_le(this->memblt.rect.x);
        payload.out_uint16_le(this->memblt.rect.y);
        payload.out_uint16_le(this->memblt.rect.cx);
        payload.out_uint16_le(this->memblt.rect.cy);
        payload.out_uint8(this->memblt.rop);
        payload.out_uint8(this->memblt.srcx);
        payload.out_uint8(this->memblt.srcy);
        payload.out_uint16_le(this->memblt.cache_idx);
        //RDPLineTo lineto;
        payload.out_uint8(this->lineto.back_mode);
        payload.out_uint16_le(this->lineto.startx);
        payload.out_uint16_le(this->lineto.starty);
        payload.out_uint16_le(this->lineto.endx);
        payload.out_uint16_le(this->lineto.endy);
        payload.out_uint32_le(this->lineto.back_color);
        payload.out_uint8(this->lineto.rop2);
        payload.out_uint8(this->lineto.pen.style);
        payload.out_sint8(this->lineto.pen.width);
        payload.out_uint32_le(this->lineto.pen.color);
        // RDPGlyphIndex glyphindex;
        payload.out_uint8(this->glyphindex.cache_id);
        payload.out_sint16_le(this->glyphindex.fl_accel);
        payload.out_sint16_le(this->glyphindex.ui_charinc);
        payload.out_sint16_le(this->glyphindex.f_op_redundant);
        payload.out_uint32_le(this->glyphindex.back_color);
        payload.out_uint32_le(this->glyphindex.fore_color);
        payload.out_uint16_le(this->glyphindex.bk.x);
        payload.out_uint16_le(this->glyphindex.bk.y);
        payload.out_uint16_le(this->glyphindex.bk.cx);
        payload.out_uint16_le(this->glyphindex.bk.cy);
        payload.out_uint16_le(this->glyphindex.op.x);
        payload.out_uint16_le(this->glyphindex.op.y);
        payload.out_uint16_le(this->glyphindex.op.cx);
        payload.out_uint16_le(this->glyphindex.op.cy);
        payload.out_uint8(this->glyphindex.brush.org_x);
        payload.out_uint8(this->glyphindex.brush.org_y);
        payload.out_uint8(this->glyphindex.brush.style);
        payload.out_uint8(this->glyphindex.brush.hatch);
        payload.out_copy_bytes(this->glyphindex.brush.extra, 7);
        payload.out_sint16_le(this->glyphindex.glyph_x);
        payload.out_sint16_le(this->glyphindex.glyph_y);
        payload.out_uint8(this->glyphindex.data_len);
        memset(this->glyphindex.data 
                + this->glyphindex.data_len, 0,
            sizeof(this->glyphindex.data) 
                - this->glyphindex.data_len);
        payload.out_copy_bytes(this->glyphindex.data, 256);

        //------------------------------ missing variable length ---------------
        payload.mark_end();

        BStream header(8);
        WRMChunk_Send chunk(header, SAVE_STATE, payload.size(), 1);
        this->trans->send(header.data, header.size());
        this->trans->send(payload.data, payload.size());
    }    


    void save_bmp_caches()
    {
        for (size_t i = 0; i < this->bmp_cache.small_entries ; i++){
            this->emit_bmp_cache(0, i);
        }
        for (size_t i = 0; i < this->bmp_cache.medium_entries ; i++){
            this->emit_bmp_cache(1, i);
        }
        for (size_t i = 0; i < this->bmp_cache.big_entries ; i++){
            this->emit_bmp_cache(2, i);
        }
    }

    void send_caches_chunk()
    {
        this->save_bmp_caches();
        if (this->order_count > 0){
            this->send_orders_chunk();
        }
    }

    void breakpoint()
    {
        this->flush();
        this->trans->next();
        this->send_meta_chunk();
        this->send_timestamp_chunk();
        this->send_save_state_chunk();
        this->send_image_chunk();
        this->send_caches_chunk();
    }

    virtual void flush()
    {
        if (this->order_count > 0){
            if (this->timer.tv_sec - this->last_sent_timer.tv_sec > 0){
                this->send_timestamp_chunk();
            }
            this->send_orders_chunk();
        }
    }

    void send_orders_chunk()
    {
        this->stream.mark_end();
        BStream header(8);
        WRMChunk_Send chunk(header, RDP_UPDATE_ORDERS, this->stream.size(), this->order_count);
        this->trans->send(header.data, header.size());
        this->trans->send(this->stream.data, this->stream.size());
        this->order_count = 0;
        this->stream.reset();
    }


    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) 
    {
        this->drawable->draw(cmd, clip);
        this->RDPSerializer::draw(cmd, clip);
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect &clip)
    {
        this->drawable->draw(cmd, clip);
        this->RDPSerializer::draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        this->drawable->draw(cmd, clip);
        this->RDPSerializer::draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->drawable->draw(cmd, clip);
        this->RDPSerializer::draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->drawable->draw(cmd, clip, bmp);
        this->RDPSerializer::draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo& cmd, const Rect & clip)
    {
        this->drawable->draw(cmd, clip);
        this->RDPSerializer::draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        this->drawable->draw(cmd, clip);
        this->RDPSerializer::draw(cmd, clip);
    }

};

#endif
