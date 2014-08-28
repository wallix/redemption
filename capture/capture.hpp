/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#ifndef _REDEMPTION_CAPTURE_CAPTURE_HPP_
#define _REDEMPTION_CAPTURE_CAPTURE_HPP_

#include "client_info.hpp"
#include "out_meta_sequence_transport.hpp"
#include "crypto_out_meta_sequence_transport.hpp"
#include "out_filename_sequence_transport.hpp"
#include "RDP/caches/pointercache.hpp"
#include "staticcapture.hpp"
#include "nativecapture.hpp"

#include "wait_obj.hpp"
#include "RDP/pointer.hpp"

#include "auth_api.hpp"

class Capture : public RDPGraphicDevice, public RDPCaptureDevice {
public:
    const bool capture_wrm;
    const bool capture_drawable;
    const bool capture_png;

    const bool enable_file_encryption;

    OutFilenameSequenceTransport * png_trans;
    StaticCapture                * psc;

    Transport                    * wrm_trans;
private:
    BmpCache      * pnc_bmp_cache;
    NativeCapture * pnc;

    RDPDrawable * drawable;

    PointerCache ptr_cache;

public:
    wait_obj capture_event;

private:
    redemption::string png_path;
    redemption::string basename;

    CryptoContext crypto_ctx;

private:
    RDPGraphicDevice * gd;

    timeval last_now;
    int     last_x;
    int     last_y;

public:
    Capture( const timeval & now, int width, int height, const char * wrm_path
           , const char * png_path, const char * hash_path, const char * basename
           , bool clear_png, bool no_timestamp, auth_api * authentifier, Inifile & ini)
    : capture_wrm(ini.video.capture_wrm)
    , capture_drawable(ini.video.capture_wrm||(ini.video.png_limit > 0))
    , capture_png(ini.video.png_limit > 0)
    , enable_file_encryption(ini.globals.enable_file_encryption.get())
    , png_trans(NULL)
    , psc(NULL)
    , wrm_trans(NULL)
    , pnc_bmp_cache(NULL)
    , pnc(NULL)
    , drawable(NULL)
    , capture_event(wait_obj(NULL))
    , png_path(png_path)
    , basename(basename)
    , gd(NULL)
    , last_now(now)
    , last_x(width / 2)
    , last_y(height / 2)
    {
        if (this->capture_drawable) {
            this->drawable = new RDPDrawable(width, height);
        }

        if (this->capture_png) {
            if (recursive_create_directory(png_path, S_IRWXU|S_IRWXG, ini.video.capture_groupid) != 0) {
                LOG(LOG_ERR, "Failed to create directory: \"%s\"", png_path);
            }

            this->png_trans = new OutFilenameSequenceTransport( FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, png_path
                                                              , basename, ".png", ini.video.capture_groupid, authentifier);
            this->psc = new StaticCapture( now, *this->png_trans, this->png_trans->seqgen(), width, height
                                         , clear_png, ini, this->drawable->drawable);
        }

        if (this->capture_wrm) {
            if (recursive_create_directory( wrm_path
                                          , S_IRWXU | S_IRGRP | S_IXGRP, ini.video.capture_groupid) != 0) {
                LOG(LOG_ERR, "Failed to create directory: \"%s\"", wrm_path);
            }

            if (recursive_create_directory( hash_path
                                          , S_IRWXU | S_IRGRP | S_IXGRP, ini.video.capture_groupid) != 0) {
                LOG(LOG_ERR, "Failed to create directory: \"%s\"", hash_path);
            }

            memset(&this->crypto_ctx, 0, sizeof(this->crypto_ctx));
            memcpy(this->crypto_ctx.crypto_key, ini.crypto.key0, sizeof(this->crypto_ctx.crypto_key));
            memcpy(this->crypto_ctx.hmac_key,   ini.crypto.key1, sizeof(this->crypto_ctx.hmac_key  ));

            TODO("there should only be one outmeta, not two. Capture code should not really care if file is encrypted or not."
                 "Here is not the right level to manage anything related to encryption.")
            TODO("Also we may wonder why we are encrypting wrm and not png"
                 "(This is related to the path split between png and wrm)."
                 "We should stop and consider what we should actually do")
            this->pnc_bmp_cache = new BmpCache( BmpCache::Recorder, 24, 3, false,
                                                BmpCache::CacheOption(600, 768, false),
                                                BmpCache::CacheOption(300, 3072, false),
                                                BmpCache::CacheOption(262, 12288, false));
            if (this->enable_file_encryption) {
                this->wrm_trans = new CryptoOutMetaSequenceTransport( &this->crypto_ctx, wrm_path, hash_path, basename, now
                                                                    , width, height, ini.video.capture_groupid
                                                                    , authentifier);
            }
            else {
                this->wrm_trans = new OutMetaSequenceTransport( wrm_path, basename, now
                                                              , width, height, ini.video.capture_groupid, authentifier);
            }
            this->pnc = new NativeCapture( now, *this->wrm_trans, width, height
                                         , *this->pnc_bmp_cache, *this->drawable, ini);
            this->pnc->recorder.send_input = true;
        }

        Pointer pointer0(Pointer::POINTER_CURSOR0);
        this->ptr_cache.add_pointer_static(pointer0, 0);
        if (this->drawable) {
            this->drawable->send_pointer(0, pointer0);
        }
        Pointer pointer1(Pointer::POINTER_CURSOR1);
        this->ptr_cache.add_pointer_static(pointer1, 1);
        if (this->drawable) {
            this->drawable->send_pointer(1, pointer1);
        }

        if (this->capture_wrm) {
            this->gd = this->pnc;
        }
        else if (this->capture_drawable) {
            this->gd = this->drawable;
        }
    }

    virtual ~Capture() {
        delete this->psc;
        delete this->png_trans;

        delete this->pnc;
        delete this->wrm_trans;
        delete this->pnc_bmp_cache;
        delete this->drawable;

        clear_files_flv_meta_png(this->png_path.c_str(), this->basename.c_str());
    }

    const SequenceGenerator * seqgen() const
    {
        return !this->wrm_trans ? 0 : this->enable_file_encryption
        ? static_cast<const CryptoOutMetaSequenceTransport*>(this->wrm_trans)->seqgen()
        : static_cast<const OutMetaSequenceTransport*>(this->wrm_trans)->seqgen();
    }

    void request_full_cleaning()
    {
        this->wrm_trans->request_full_cleaning();
    }

    void pause() {
        if (this->capture_png) {
            struct timeval now = tvtime();
            this->psc->pause_snapshot(now);
        }
    }

    void resume() {
        if (this->capture_wrm){
            this->wrm_trans->next();
            struct timeval now = tvtime();
            this->pnc->recorder.timestamp(now);
            this->pnc->recorder.send_timestamp_chunk(true);
        }
    }

    void update_config(const Inifile & ini) {
        if (this->capture_png) {
            this->psc->update_config(ini);
        }
        if (this->capture_wrm) {
            this->pnc->update_config(ini);
        }
    }

    virtual void set_row(size_t rownum, const uint8_t * data)
    {
        if (this->capture_drawable){
            this->drawable->set_row(rownum, data);
        }
    }

    void snapshot(const timeval & now, int x, int y, bool ignore_frame_in_timeval) {
        this->capture_event.reset();

        if (this->capture_drawable) {
            this->drawable->drawable.set_mouse_cursor_pos(x, y);
        }

        this->last_now = now;
        this->last_x   = x;
        this->last_y   = y;

        if (this->capture_png) {
            this->psc->snapshot(now, x, y, ignore_frame_in_timeval);
            this->capture_event.update(this->psc->time_to_wait);
        }
        if (this->capture_wrm) {
            this->pnc->snapshot(now, x, y, ignore_frame_in_timeval);
            this->capture_event.update(this->pnc->time_to_wait);
        }
    }

    void flush() {
        if (this->capture_wrm) {
            this->pnc->flush();
        }
    }

    void close()
    {
        this->flush();
        this->wrm_trans->disconnect();
    }

    void input(const timeval & now, Stream & input_data_32) {
        if (this->capture_wrm) {
            this->pnc->input(now, input_data_32);
        }
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    void draw(const RDPDestBlt & cmd, const Rect &clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    void draw(const RDPMultiDstBlt & cmd, const Rect & clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) {
        if (this->gd) {
            this->gd->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) {
        if (this->gd) {
            this->gd->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    void draw(const RDPLineTo & cmd, const Rect & clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    void draw(const RDPBrushCache & cmd) {
        if (this->gd) {
            this->gd->draw(cmd);
        }
    }

    void draw(const RDPColCache & cmd) {
        if (this->gd) {
            this->gd->draw(cmd);
        }
    }

    void draw(const RDPGlyphCache & cmd) {
        if (this->gd) {
            this->gd->draw(cmd);
        }
    }


    void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache) {
        if (this->gd) {
            this->gd->draw(cmd, clip, gly_cache);
        }
    }

    void draw(const RDPBitmapData & bitmap_data, const uint8_t * data , size_t size, const Bitmap & bmp) {
        if (this->gd) {
            if (bmp.original_bpp == 8) {
                Bitmap bmp_24(24, bmp);

                BStream bmp_stream(65535);
                bmp_24.compress(24, bmp_stream);
                bmp_stream.mark_end();

                RDPBitmapData bitmap_data_24 = bitmap_data;
                bitmap_data_24.bits_per_pixel = 24;
                bitmap_data_24.flags          = BITMAP_COMPRESSION;
                bitmap_data_24.bitmap_length  = bmp_stream.size() + 8;

                bitmap_data_24.cb_comp_main_body_size = bmp_stream.size();
                bitmap_data_24.cb_scan_width          = bmp_24.cx;
                bitmap_data_24.cb_uncompressed_size   = bmp_24.bmp_size;

                this->gd->draw(bitmap_data_24, bmp_stream.get_data(), bmp_stream.size(), bmp_24);
            }
            else {
                if (!(bitmap_data.flags & BITMAP_COMPRESSION)) {
                    BStream bmp_stream(65535);
                    bmp.compress(24, bmp_stream);
                    bmp_stream.mark_end();

                    RDPBitmapData bitmap_data_compressed = bitmap_data;

                    bitmap_data_compressed.flags         = BITMAP_COMPRESSION | NO_BITMAP_COMPRESSION_HDR;
                    bitmap_data_compressed.bitmap_length = bmp_stream.size();

                    this->gd->draw(bitmap_data_compressed, bmp_stream.get_data(), bmp_stream.size(), bmp);
                }
                else {
                    this->gd->draw(bitmap_data, data, size, bmp);
                }
            }
        }
    }

    virtual void draw(const RDP::FrameMarker & order) {
        if (this->gd) {
            this->gd->draw(order);
        }

        if (order.action == RDP::FrameMarker::FrameEnd) {
            if (this->capture_png) {
                this->psc->snapshot(this->last_now, this->last_x, this->last_y, false);
            }
        }
    }

    void draw(const RDPPolygonSC & cmd, const Rect & clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    void draw(const RDPPolygonCB & cmd, const Rect & clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    void draw(const RDPEllipseSC & cmd, const Rect & clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    void draw(const RDPEllipseCB & cmd, const Rect & clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    virtual void server_set_pointer(const Pointer & cursor)
    {
        int cache_idx = 0;
        switch (this->ptr_cache.add_pointer(cursor, cache_idx)) {
        case POINTER_TO_SEND:
            this->send_pointer(cache_idx, cursor);
        break;
        default:
        case POINTER_ALLREADY_SENT:
            this->set_pointer(cache_idx);
        break;
        }
    }

    virtual void send_pointer(int cache_idx, const Pointer & cursor) {
        if (this->gd) {
           this->gd->send_pointer(cache_idx, cursor);
        }
    }

    virtual void set_pointer(int cache_idx) {
        if (this->gd) {
            this->gd->set_pointer(cache_idx);
        }
    }

    virtual void set_pointer_display() {
        if (this->capture_drawable) {
            this->drawable->drawable.dont_show_mouse_cursor = true;
        }
    }
};

#endif
