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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier
*/

#if !defined(__CAPTURE_NATIVECAPTURE_HPP__)
#define __CAPTURE_NATIVECAPTURE_HPP__

#include <errno.h>
#include "rdtsc.hpp"
#include "bitmap.hpp"
#include "rect.hpp"
#include "constants.hpp"
#include <time.h>
#include "difftimeval.hpp"

#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"

#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"

#include "error.hpp"
#include "config.hpp"
#include "RDP/caches/bmpcache.hpp"
#include "colors.hpp"
#include "stream.hpp"

#include "GraphicToFile.hpp"
#include "png.hpp"
#include "auto_buffer.hpp"
#include "cipher_transport.hpp"

#include "zlib.hpp"

class NativeCapture : public RDPGraphicDevice
{
public:
    int width;
    int height;
    int bpp;

    OutFileTransport trans;

    OutCipherTransport cipher_trans;
    const EVP_CIPHER * cipher_mode;
    const unsigned char* cipher_key;
    const unsigned char* cipher_iv;
    ENGINE* cipher_impl;

    BStream stream;
    GraphicsToFile recorder;
    char filename[1024];
    unsigned int filename_len;
    unsigned int basepath_len;
    uint32_t nb_file;

    char * meta_name;
    uint32_t meta_name_len;
    FILE* meta_file;

private:
    void next_filename()
    {
        this->filename_len = this->basepath_len + sprintf(this->filename + this->basepath_len, "%u.wrm", this->nb_file++);
    }

    void open_file()
    {
        LOG(LOG_INFO, "Open to file : %s", this->filename);
        this->trans.fd = open(this->filename, O_WRONLY|O_CREAT, 0666);
        if (this->trans.fd < 0){
            LOG(LOG_ERR, "Error opening native capture file : %s", strerror(errno));
            throw Error(ERR_NATIVE_CAPTURE_OPEN_FAILED);
        }
    }

    void send_meta_path()
    {
        this->recorder.chunk_type = WRMChunk::META_FILE;
        this->recorder.order_count = 1;
        {
            this->stream.out_uint32_le(this->meta_name_len);
            this->stream.out_copy_bytes(this->meta_name, this->meta_name_len);
        }
        this->recorder.flush();
    }

    bool _start_cipher()
    {
        return this->cipher_trans.start(this->cipher_mode,
                                        this->cipher_key,
                                        this->cipher_iv,
                                        this->cipher_impl);
    }

public:
    /**
     * @attention not copy \p key, \p iv and \p impl
     */
    NativeCapture(int width, int height, const char * path,
                  const char * meta_filename = 0,
                  CipherMode::enum_t e = CipherMode::NO_MODE,
                  const unsigned char* key = 0,
                  const unsigned char* iv = 0,
                  ENGINE* impl = 0)
    : width(width)
    , height(height)
    , bpp(24)
    , trans(-1)
    , cipher_trans(&trans)
    , cipher_mode(CipherMode::to_evp_cipher(e))
    , cipher_key(key)
    , cipher_iv(iv)
    , cipher_impl(impl)
    , stream(65536)
    , recorder(this->cipher_mode
               ? (Transport*)&this->cipher_trans : &this->trans,
               &this->stream, NULL, 24, 8192, 768, 8192, 3072, 8192, 12288)
    , nb_file(0)
    {
        if (e && !this->cipher_mode)
        {
            LOG(LOG_ERR, "Error selected cipher mode (%d) in NativeCapture", e);
            throw Error(ERR_CIPHER_START);
        }
        if (this->cipher_mode && !this->_start_cipher())
        {
            LOG(LOG_ERR, "Error cipher start in NativeCapture");
            throw Error(ERR_CIPHER_START);
        }
        this->basepath_len = sprintf(this->filename, "%s-%u-", path, getpid());
        this->next_filename();
        this->open_file();

        if (meta_filename) {
            this->meta_name_len = strlen(meta_filename);
            this->meta_name = (char*)malloc(this->meta_name_len + 1);
            memcpy(this->meta_name, meta_filename, this->meta_name_len + 1);
        }
        else {
            this->meta_name_len = this->basepath_len + 4;
            this->meta_name = (char*)malloc(this->meta_name_len + 1);
            memcpy(this->meta_name, this->filename, this->basepath_len - 1);
            memcpy(this->meta_name + this->basepath_len - 1, ".mwrm", 6);
        }
        this->send_meta_path();

        this->meta_file = fopen(this->meta_name, "w+");
        if (!this->meta_file) {
            free(this->meta_name);
            LOG(LOG_ERR, "error open meta: %s", strerror(errno));
            throw Error(ERR_NATIVE_CAPTURE_OPEN_FAILED);
        }

        fprintf(this->meta_file, "%d %d\n%d",
                this->width, this->height, e);
        if (this->cipher_is_active())
        {
            if (iv)
            {
                fputs(" ", this->meta_file);
                for (int i = 0, n = EVP_CIPHER_CTX_iv_length(&this->cipher_trans.ctx()); i < n ; ++i)
                {
                    fprintf(this->meta_file, "%x%x", iv[i] >> 4, iv[i] & 0xf);
                }
            }
            else
            {
                fputs(" 00000000000000000000000000000000", this->meta_file);
            }
        }
        fputs("\n\n", this->meta_file);
    }

    ~NativeCapture(){
        this->recorder.flush();
        if (this->cipher_is_active())
        {
            this->cipher_trans.stop();
        }
        close(this->trans.fd);
        fclose(this->meta_file);
        free(this->meta_name);
    }

    bool cipher_is_active() const
    {
        return this->cipher_mode;
    }

    virtual void flush()
    {}

    virtual void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->recorder.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

private:
    void send_rect(const Rect& rect)
    {
        this->stream.out_uint16_le(rect.x);
        this->stream.out_uint16_le(rect.y);
        this->stream.out_uint16_le(rect.cx);
        this->stream.out_uint16_le(rect.cy);
    }

    void send_brush(const RDPBrush& brush)
    {
        this->stream.out_uint8(brush.org_x);
        this->stream.out_uint8(brush.org_y);
        this->stream.out_uint8(brush.style);
        this->stream.out_uint8(brush.hatch);
        this->stream.out_uint8(brush.extra[0]);
        this->stream.out_uint8(brush.extra[1]);
        this->stream.out_uint8(brush.extra[2]);
        this->stream.out_uint8(brush.extra[3]);
        this->stream.out_uint8(brush.extra[4]);
        this->stream.out_uint8(brush.extra[5]);
        this->stream.out_uint8(brush.extra[6]);
    }

    void send_pen(const RDPPen pen)
    {
        this->stream.out_uint32_le(pen.color);
        this->stream.out_uint8(pen.style);
        this->stream.out_uint8(pen.width);
    }

private:
    void send_time_start_order(const timeval& now)
    {
        this->recorder.chunk_type = WRMChunk::TIME_START;
        this->recorder.order_count = 1;
        this->stream.out_uint64_be(now.tv_sec);
        this->stream.out_uint64_be(now.tv_usec);
        this->recorder.flush();
    }

public:
    void write_start_in_meta(const timeval& now)
    {
        fprintf(this->meta_file, "%s, %ld %ld\n",
                this->filename, now.tv_sec, now.tv_usec);
    }

    void send_time_start(const timeval& now)
    {
        this->send_time_start_order(now);
        this->write_start_in_meta(now);
    }

    void breakpoint(const uint8_t* data_drawable, uint8_t bpp,
                    uint16_t width, uint16_t height, size_t rowsize,
                    const timeval& now)
    {
        this->recorder.flush();

        this->recorder.chunk_type = WRMChunk::NEXT_FILE_ID;
        this->recorder.order_count = 1;
        this->stream.out_uint32_le(this->nb_file);
        this->next_filename();
        this->recorder.flush();
        if (this->cipher_is_active())
        {
            this->cipher_trans.stop();
        }

        close(this->trans.fd);
        this->open_file();
        if (this->cipher_is_active())
        {
            this->cipher_trans.reset();
            this->_start_cipher();
        }
        this->send_meta_path();

        this->recorder.chunk_type = WRMChunk::BREAKPOINT;
        this->recorder.order_count = 1;
        this->stream.out_uint16_le(this->width);
        this->stream.out_uint16_le(this->height);
        this->stream.out_uint8(this->bpp);
        this->stream.out_uint64_le(this->recorder.timer.sec());
        this->stream.out_uint64_le(this->recorder.timer.usec());
        this->recorder.send_order();

        // write screen
        {
            this->filename[this->filename_len] = '.';
            if (this->cipher_is_active())
            {
                this->filename[this->filename_len+1] = 'c';
                this->filename[this->filename_len+2] = 't';
                this->filename[this->filename_len+3] = 'x';
                this->filename[this->filename_len+4] = 0;
                int fd = ::open(this->filename, O_WRONLY|O_CREAT, 0666);
                if (fd < 0){
                    LOG(LOG_ERR, "Error opening context file : %s", strerror(errno));
                    throw Error(ERR_NATIVE_CAPTURE_OPEN_FAILED);
                }
                /*OutFileTransport file_trans(fd);
                OutCipherTransport context_trans(&file_trans);
                context_trans.start(this->cipher_mode,
                                    this->cipher_key,
                                    this->cipher_iv);
                context_trans.send(data_drawable, rowsize * height);*/
                OutFileTransport file_trans(fd);
                OutCipherTransport context_trans(&file_trans);
                z_stream zstrm;
                zstrm.zalloc = 0;
                zstrm.zfree = 0;
                zstrm.opaque = 0;
                int ret;
                if ((ret = deflateInit(&zstrm, Z_DEFAULT_COMPRESSION)) != Z_OK)
                {
                    LOG(LOG_ERR, "zlib: deflateInit: %d", ret);
                    throw Error(ERR_NATIVE_CAPTURE_ZIP_COMPRESS);
                }
                ZRaiiDeflateEnd deflate_end(zstrm);
                uint8_t buffer[8192];
                context_trans.start(this->cipher_mode,
                                    this->cipher_key,
                                    this->cipher_iv);
                zstrm.next_in = (Bytef *)data_drawable;
                zstrm.avail_in = rowsize * height;
                int flush = Z_FINISH;
                do {
                    zstrm.next_out = buffer;
                    zstrm.avail_out = sizeof buffer;
                    if ((ret = deflate(&zstrm, flush)) != Z_OK)
                    {
                        if (Z_STREAM_END != ret)
                        {
                            LOG(LOG_ERR, "zlib: deflate: %s", zError(ret));
                            throw Error(ERR_NATIVE_CAPTURE_ZIP_COMPRESS);
                        }
                    }
                    context_trans.send(buffer, sizeof buffer - zstrm.avail_out);
                } while (zstrm.avail_out == 0);
                context_trans.stop();
            }
            else
            {
                this->filename[this->filename_len+1] = 'p';
                this->filename[this->filename_len+2] = 'n';
                this->filename[this->filename_len+3] = 'g';
                this->filename[this->filename_len+4] = 0;
                if (std::FILE* fd = std::fopen(this->filename, "w+"))
                {
                    dump_png24(fd, data_drawable, width, height, rowsize);
                    fclose(fd);
                }
                else
                {
                    LOG(LOG_ERR, "Error opening context file : %s", strerror(errno));
                    throw Error(ERR_NATIVE_CAPTURE_OPEN_FAILED);
                }
            }
            this->filename[this->filename_len] = 0;
        }

        this->recorder.init();
        this->recorder.order_count = 1;
        this->recorder.chunk_type = WRMChunk::BREAKPOINT;

        this->stream.out_uint8(this->recorder.common.order);
        this->send_rect(this->recorder.common.clip);

        this->stream.out_uint32_le(this->recorder.opaquerect.color);
        this->send_rect(this->recorder.opaquerect.rect);

        this->stream.out_uint8(this->recorder.destblt.rop);
        this->send_rect(this->recorder.destblt.rect);

        this->stream.out_uint8(this->recorder.patblt.rop);
        this->stream.out_uint32_le(this->recorder.patblt.back_color);
        this->stream.out_uint32_le(this->recorder.patblt.fore_color);
        this->send_brush(this->recorder.patblt.brush);
        this->send_rect(this->recorder.patblt.rect);

        this->stream.out_uint8(this->recorder.scrblt.rop);
        this->stream.out_uint16_le(this->recorder.scrblt.srcx);
        this->stream.out_uint16_le(this->recorder.scrblt.srcy);
        this->send_rect(this->recorder.scrblt.rect);

        this->stream.out_uint8(this->recorder.memblt.rop);
        this->stream.out_uint16_le(this->recorder.memblt.srcx);
        this->stream.out_uint16_le(this->recorder.memblt.srcy);
        this->stream.out_uint16_le(this->recorder.memblt.cache_id);
        this->stream.out_uint16_le(this->recorder.memblt.cache_idx);
        this->send_rect(this->recorder.memblt.rect);

        this->stream.out_uint8(this->recorder.lineto.rop2);
        this->stream.out_uint16_le(this->recorder.lineto.startx);
        this->stream.out_uint16_le(this->recorder.lineto.starty);
        this->stream.out_uint16_le(this->recorder.lineto.endx);
        this->stream.out_uint16_le(this->recorder.lineto.endy);
        this->stream.out_uint8(this->recorder.lineto.back_mode);
        this->stream.out_uint32_le(this->recorder.lineto.back_color);
        this->send_pen(this->recorder.lineto.pen);

        this->stream.out_uint32_le(this->recorder.glyphindex.back_color);
        this->stream.out_uint32_le(this->recorder.glyphindex.fore_color);
        this->stream.out_uint16_le(this->recorder.glyphindex.f_op_redundant);
        this->stream.out_uint16_le(this->recorder.glyphindex.fl_accel);
        this->stream.out_uint16_le(this->recorder.glyphindex.glyph_x);
        this->stream.out_uint16_le(this->recorder.glyphindex.glyph_y);
        this->stream.out_uint16_le(this->recorder.glyphindex.ui_charinc);
        this->stream.out_uint8(this->recorder.glyphindex.cache_id);
        this->stream.out_uint8(this->recorder.glyphindex.data_len);
        this->send_rect(this->recorder.glyphindex.bk);
        this->send_rect(this->recorder.glyphindex.op);
        this->send_brush(this->recorder.glyphindex.brush);
        this->stream.out_copy_bytes(this->recorder.glyphindex.data, this->recorder.glyphindex.data_len);

        this->stream.out_uint16_le(this->recorder.order_count);

        this->stream.out_uint16_le(this->recorder.bmp_cache.small_entries);
        this->stream.out_uint16_le(this->recorder.bmp_cache.small_size);
        this->stream.out_uint16_le(this->recorder.bmp_cache.medium_entries);
        this->stream.out_uint16_le(this->recorder.bmp_cache.medium_size);
        this->stream.out_uint16_le(this->recorder.bmp_cache.big_entries);
        this->stream.out_uint16_le(this->recorder.bmp_cache.big_size);
        this->stream.out_uint32_le(this->recorder.bmp_cache.stamp);

        this->recorder.send_order();

        this->stream.init(14);
        this->recorder.chunk_type = WRMChunk::BREAKPOINT;
        AutoBuffer buffer;
        z_stream zstrm;
        zstrm.zalloc = 0;
        zstrm.zfree = 0;
        zstrm.opaque = 0;
        int ret;
        const int Bpp = 3;
        for (size_t cid = 0; cid != 3 ; ++cid){
            const Bitmap* (&bitmaps)[8192] = this->recorder.bmp_cache.cache[cid];
            const uint32_t (&stamps)[8192] = this->recorder.bmp_cache.stamps[cid];

            for (uint16_t cidx = 0; cidx < 8192 ; ++cidx){
                if (bitmaps[cidx]){
                    this->stream.out_uint16_le(8192 * cid + cidx);
                    this->stream.out_uint32_le(stamps[cidx]);
                    this->stream.out_uint16_le(bitmaps[cidx]->cx);
                    this->stream.out_uint16_le(bitmaps[cidx]->cy);

                    if ((ret = deflateInit(&zstrm, Z_DEFAULT_COMPRESSION)) != Z_OK)
                    {
                        LOG(LOG_ERR, "zlib: deflateInit: %d", ret);
                        throw Error(ERR_NATIVE_CAPTURE_ZIP_COMPRESS);
                    }
                    uint16_t y = 1;
                    int flush;
                    /*const*/ uint8_t *src = bitmaps[cidx]->data_bitmap.get();
                    uint size_x = bitmaps[cidx]->cx * Bpp;
                    uLong destlen = compressBound(size_x * bitmaps[cidx]->cy);
                    buffer.alloc(destlen);

                    do {
                        zstrm.next_in = src;
                        zstrm.avail_in = size_x;
                        zstrm.next_out = buffer.get() + zstrm.total_out;
                        zstrm.avail_out = destlen - zstrm.total_out;
                        flush = y == bitmaps[cidx]->cy ? Z_FINISH : Z_NO_FLUSH;
                        if ((ret = deflate(&zstrm, flush)) == Z_STREAM_ERROR)
                        {
                            deflateEnd(&zstrm);
                            LOG(LOG_ERR, "zlib: deflate: %d", ret);
                            throw Error(ERR_NATIVE_CAPTURE_ZIP_COMPRESS);
                        }
                        ++y;
                        src += bitmaps[cidx]->line_size;
                    } while (flush != Z_FINISH);
                    deflateEnd(&zstrm);
                    this->stream.out_uint32_le(zstrm.total_out);
                    this->recorder.trans->send(this->stream.data, 14);
                    this->recorder.trans->send(buffer.get(), zstrm.total_out);
                    this->stream.p = this->stream.data;
                }
            }
        }
        this->stream.out_uint16_le(8192 * 3 + 1);
        this->stream.out_uint32_le(0);
        this->stream.out_uint16_le(0);
        this->stream.out_uint16_le(0);
        this->stream.out_uint32_le(0);
        this->recorder.trans->send(this->stream.data, 14);

        this->recorder.init();
        this->send_time_start_order(now);
        fprintf(this->meta_file, "%s,%s.%s %ld %ld\n",
                this->filename, this->filename,
                this->cipher_is_active() ? "ctx" : "png",
                now.tv_sec, now.tv_usec);
        this->recorder.chunk_type = RDP_UPDATE_ORDERS;
    }
};

#endif
