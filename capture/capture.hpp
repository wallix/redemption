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

#include "out_meta_sequence_transport.hpp"
#include "crypto_out_meta_sequence_transport.hpp"
#include "out_filename_sequence_transport.hpp"

#include "RDP/caches/pointercache.hpp"

#include "nativecapture.hpp"
#include "staticcapture.hpp"

#include "RDP/compress_and_draw_bitmap_update.hpp"

#include "wait_obj.hpp"

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
    GlyphCache    * pnc_gly_cache;
    PointerCache  * pnc_ptr_cache;
    NativeCapture * pnc;

    RDPDrawable * drawable;

public:
    wait_obj capture_event;

private:
    std::string png_path;
    std::string basename;

    CryptoContext crypto_ctx;

private:
    RDPGraphicDevice * gd;

    timeval last_now;
    int     last_x;
    int     last_y;

    bool    clear_png;

    uint8_t order_bpp;
    uint8_t capture_bpp;

    const BGRPalette & mod_palette_rgb = BGRPalette::classic_332_rgb();

public:
    Capture( const timeval & now, int width, int height, int order_bpp, int capture_bpp, const char * wrm_path
           , const char * png_path, const char * hash_path, const char * basename
           , bool clear_png, bool no_timestamp, auth_api * authentifier, Inifile & ini, bool externally_generated_breakpoint = false)
    : capture_wrm(ini.video.capture_wrm)
    , capture_drawable(ini.video.capture_wrm||(ini.video.png_limit > 0))
    , capture_png(ini.video.png_limit > 0)
    , enable_file_encryption(ini.globals.enable_file_encryption.get())
    , png_trans(nullptr)
    , psc(nullptr)
    , wrm_trans(nullptr)
    , pnc_bmp_cache(nullptr)
    , pnc_gly_cache(nullptr)
    , pnc_ptr_cache(nullptr)
    , pnc(nullptr)
    , drawable(nullptr)
    , png_path(png_path)
    , basename(basename)
    , gd(nullptr)
    , last_now(now)
    , last_x(width / 2)
    , last_y(height / 2)
    , clear_png(clear_png)
    , order_bpp(order_bpp)
    , capture_bpp(capture_bpp)
    {
        if (this->capture_drawable) {
            this->drawable = new RDPDrawable(width, height, capture_bpp);
        }

        if (this->capture_png) {
            if (recursive_create_directory(png_path, S_IRWXU|S_IRWXG, ini.video.capture_groupid) != 0) {
                LOG(LOG_ERR, "Failed to create directory: \"%s\"", png_path);
            }

//            this->png_trans = new OutFilenameSequenceTransport( FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, png_path
            this->png_trans = new OutFilenameSequenceTransport( FilenameGenerator::PATH_FILE_COUNT_EXTENSION, png_path
                                                              , basename, ".png", ini.video.capture_groupid, authentifier);
            this->psc = new StaticCapture( now, *this->png_trans, this->png_trans->seqgen(), width, height
                                         , clear_png, ini, this->drawable->impl());
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
            this->pnc_bmp_cache = new BmpCache( BmpCache::Recorder, capture_bpp, 3, false
                                              , BmpCache::CacheOption(600, 768, false)
                                              , BmpCache::CacheOption(300, 3072, false)
                                              , BmpCache::CacheOption(262, 12288, false)
                                              );
            this->pnc_gly_cache = new GlyphCache();
            const int pointerCacheSize = 0x19;
            this->pnc_ptr_cache = new PointerCache(pointerCacheSize);

            if (this->enable_file_encryption) {
                this->wrm_trans = new CryptoOutMetaSequenceTransport( &this->crypto_ctx, wrm_path, hash_path, basename, now
                                                                    , width, height, ini.video.capture_groupid
                                                                    , authentifier);
            }
            else {
                this->wrm_trans = new OutMetaSequenceTransport( wrm_path, basename, now
                                                              , width, height, ini.video.capture_groupid, authentifier);
            }
            this->pnc = new NativeCapture( now, *this->wrm_trans, width, height, capture_bpp
                                         , *this->pnc_bmp_cache, *this->pnc_gly_cache, *this->pnc_ptr_cache
                                         , *this->drawable, ini, externally_generated_breakpoint
                                         , NativeCapture::SendInput::YES);
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

        if (this->pnc) {
            timeval now = tvtime();
            this->pnc->recorder.timestamp(now);
            this->pnc->recorder.send_timestamp_chunk(false);
            delete this->pnc;
        }
        delete this->wrm_trans;
        delete this->pnc_bmp_cache;
        delete this->pnc_gly_cache;
        delete this->pnc_ptr_cache;
        delete this->drawable;

        if (this->clear_png) {
            clear_files_flv_meta_png(this->png_path.c_str(), this->basename.c_str());
        }
    }

    void request_full_cleaning()
    {
        this->wrm_trans->request_full_cleaning();
    }

    void pause() {
        if (this->capture_png) {
            timeval now = tvtime();
            this->psc->pause_snapshot(now);
        }
    }

    void resume() {
        if (this->capture_wrm){
            this->wrm_trans->next();
            timeval now = tvtime();
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
            this->drawable->set_mouse_cursor_pos(x, y);
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
            if (this->capture_bpp != this->order_bpp) {
                RDPMultiOpaqueRect capture_cmd = cmd;

                capture_cmd._Color = color_encode(
                      color_decode_opaquerect(cmd._Color, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );

                this->gd->draw(capture_cmd, clip);
            }
            else {
                this->gd->draw(cmd, clip);
            }
        }
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) {
        if (this->gd) {
            if (this->capture_bpp != this->order_bpp) {
                RDP::RDPMultiPatBlt capture_cmd = cmd;

                capture_cmd.BackColor = color_encode(
                      color_decode_opaquerect(cmd.BackColor, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );
                capture_cmd.ForeColor = color_encode(
                      color_decode_opaquerect(cmd.ForeColor, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );

                this->gd->draw(capture_cmd, clip);
            }
            else {
                this->gd->draw(cmd, clip);
            }
        }
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) {
        if (this->gd) {
            this->gd->draw(cmd, clip);
        }
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip) {
        if (this->gd) {
            if (this->capture_bpp != this->order_bpp) {
                RDPPatBlt capture_cmd = cmd;

                capture_cmd.back_color = color_encode(
                      color_decode_opaquerect(cmd.back_color, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );
                capture_cmd.fore_color = color_encode(
                      color_decode_opaquerect(cmd.fore_color, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );

                this->gd->draw(capture_cmd, clip);
            }
            else {
                this->gd->draw(cmd, clip);
            }
        }
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) {
        if (this->gd) {
            this->gd->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) {
        if (this->gd) {
            if (this->capture_bpp != this->order_bpp) {
                RDPMem3Blt capture_cmd = cmd;

                capture_cmd.back_color = color_encode(
                      color_decode_opaquerect(cmd.back_color, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );
                capture_cmd.fore_color = color_encode(
                      color_decode_opaquerect(cmd.fore_color, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );

                this->gd->draw(capture_cmd, clip, bmp);
            }
            else {
                this->gd->draw(cmd, clip, bmp);
            }
        }
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip) {
        if (this->gd) {
            if (this->capture_bpp != this->order_bpp) {
                RDPOpaqueRect capture_cmd = cmd;

                capture_cmd.color = color_encode(
                      color_decode_opaquerect(cmd.color, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );

                this->gd->draw(capture_cmd, clip);
            }
            else {
                this->gd->draw(cmd, clip);
            }
        }
    }

    void draw(const RDPLineTo & cmd, const Rect & clip) {
        if (this->gd) {
            if (this->capture_bpp != this->order_bpp) {
                RDPLineTo capture_cmd = cmd;

                capture_cmd.back_color = color_encode(
                      color_decode_opaquerect(cmd.back_color, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );
                capture_cmd.pen.color = color_encode(
                      color_decode_opaquerect(cmd.pen.color, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );

                this->gd->draw(capture_cmd, clip);
            }
            else {
                this->gd->draw(cmd, clip);
            }
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

    void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache) {
        if (this->gd) {
            if (this->capture_bpp != this->order_bpp) {
                RDPGlyphIndex capture_cmd = cmd;

                capture_cmd.back_color = color_encode(
                      color_decode_opaquerect(cmd.back_color, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );
                capture_cmd.fore_color = color_encode(
                      color_decode_opaquerect(cmd.fore_color, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );

                this->gd->draw(capture_cmd, clip, gly_cache);
            }
            else {
                this->gd->draw(cmd, clip, gly_cache);
            }
        }
    }

    void draw(const RDPBitmapData & bitmap_data, const uint8_t * data , size_t size, const Bitmap & bmp) {
        if (this->gd) {
            if (this->capture_wrm) {
                if (bmp.bpp() > this->capture_bpp) {
                    // reducing the color depth of image.
                    Bitmap capture_bmp(this->capture_bpp, bmp);

                    ::compress_and_draw_bitmap_update(bitmap_data, capture_bmp, this->capture_bpp, *this->gd);
                }
                else if (!(bitmap_data.flags & BITMAP_COMPRESSION)) {
                    ::compress_and_draw_bitmap_update(bitmap_data, bmp, this->capture_bpp, *this->gd);
                }
                else {
                    this->gd->draw(bitmap_data, data, size, bmp);
                }
            }
            else {
                this->gd->draw(bitmap_data, data, size, bmp);
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
            if (this->capture_bpp != this->order_bpp) {
                RDPPolygonSC capture_cmd = cmd;

                capture_cmd.BrushColor = color_encode(
                      color_decode_opaquerect(cmd.BrushColor, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );

                this->gd->draw(capture_cmd, clip);
            }
            else {
                this->gd->draw(cmd, clip);
            }
        }
    }

    void draw(const RDPPolygonCB & cmd, const Rect & clip) {
        if (this->gd) {
            if (this->capture_bpp != this->order_bpp) {
                RDPPolygonCB capture_cmd = cmd;

                capture_cmd.foreColor = color_encode(
                      color_decode_opaquerect(cmd.foreColor, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );
                capture_cmd.backColor = color_encode(
                      color_decode_opaquerect(cmd.backColor, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );

                this->gd->draw(capture_cmd, clip);
            }
            else {
                this->gd->draw(cmd, clip);
            }
        }
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) {
        if (this->gd) {
            if (this->capture_bpp != this->order_bpp) {
                RDPPolyline capture_cmd = cmd;

                capture_cmd.PenColor = color_encode(
                      color_decode_opaquerect(cmd.PenColor, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );

                this->gd->draw(capture_cmd, clip);
            }
            else {
                this->gd->draw(cmd, clip);
            }
        }
    }

    void draw(const RDPEllipseSC & cmd, const Rect & clip) {
        if (this->gd) {
            if (this->capture_bpp != this->order_bpp) {
                RDPEllipseSC capture_cmd = cmd;

                capture_cmd.color = color_encode(
                      color_decode_opaquerect(cmd.color, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );

                this->gd->draw(capture_cmd, clip);
            }
            else {
                this->gd->draw(cmd, clip);
            }
        }
    }

    void draw(const RDPEllipseCB & cmd, const Rect & clip) {
        if (this->gd) {
            if (this->capture_bpp != this->order_bpp) {
                RDPEllipseCB capture_cmd = cmd;

                capture_cmd.back_color = color_encode(
                      color_decode_opaquerect(cmd.back_color, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );
                capture_cmd.fore_color = color_encode(
                      color_decode_opaquerect(cmd.fore_color, this->order_bpp, this->mod_palette_rgb)
                    , this->capture_bpp
                    );

                this->gd->draw(capture_cmd, clip);
            }
            else {
                this->gd->draw(cmd, clip);
            }
        }
    }

    virtual void server_set_pointer(const Pointer & cursor)
    {
        if (this->gd) {
            this->gd->server_set_pointer(cursor);
        }
    }

    virtual void set_mod_palette(const BGRPalette & palette) {
        if (this->capture_drawable) {
            this->drawable->set_mod_palette(palette);
        }
    }

    virtual void set_pointer_display() {
        if (this->capture_drawable) {
            this->drawable->show_mouse_cursor(false);
        }
    }

    // toggles externally genareted breakpoint.
    virtual void external_breakpoint() {
        if (this->capture_wrm) {
            this->pnc->external_breakpoint();
        }
    }

    virtual void external_time(const timeval & now) {
        if (this->capture_wrm) {
            this->pnc->external_time(now);
        }
    }
};

#endif
