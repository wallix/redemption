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
#include "outmetatransport.hpp"
#include "outfilenametransport.hpp"
#include "RDP/caches/pointercache.hpp"
#include "staticcapture.hpp"
#include "nativecapture.hpp"

#include "wait_obj.hpp"

class Capture : public RDPGraphicDevice {
public:
    const bool capture_wrm;
    const bool capture_drawable;
    const bool capture_png;

    const bool enable_file_encryption;

    OutFilenameTransport * png_trans;
    StaticCapture        * psc;

    OutmetaTransport       * wrm_trans;
    CryptoOutmetaTransport * crypto_wrm_trans;
    BmpCache               * pnc_bmp_cache;
    NativeCapture          * pnc;

    RDPDrawable * drawable;

    PointerCache ptr_cache;

    wait_obj capture_event;

    redemption::string png_path;
    redemption::string basename;

    Capture( const timeval & now, int width, int height, const char * wrm_path
           , const char * png_path, const char * hash_path, const char * basename
           , bool clear_png, Inifile & ini)
            : capture_wrm(ini.video.capture_wrm)
            , capture_drawable(ini.video.capture_wrm||(ini.video.png_limit > 0))
            , capture_png(ini.video.png_limit > 0)
            , enable_file_encryption(ini.globals.enable_file_encryption.get())
            , png_trans(NULL)
            , psc(NULL)
            , wrm_trans(NULL)
            , crypto_wrm_trans(NULL)
            , pnc_bmp_cache(NULL)
            , pnc(NULL)
            , drawable(NULL)
            , capture_event(wait_obj(0))
            , png_path(png_path)
            , basename(basename) {
        if (this->capture_drawable) {
            this->drawable = new RDPDrawable(width, height);
        }

        if (this->capture_png) {
            if (recursive_create_directory(png_path, S_IRWXU|S_IRWXG, ini.video.capture_groupid) != 0) {
                LOG(LOG_ERR, "Failed to create directory: \"%s\"", png_path);
            }

            this->png_trans = new OutFilenameTransport( SQF_PATH_FILE_PID_COUNT_EXTENSION, png_path
                                                      , basename, ".png", ini.video.capture_groupid);
            this->psc = new StaticCapture( now, *this->png_trans, &(this->png_trans->seq), width, height
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

            TODO("there should only be one outmeta, not two. Capture code should not really care if file is encrypted or not."
                 "Here is not the right level to manage anything related to encryption.")
            TODO("Also we may wonder why we are encrypting wrm and not png"
                 "(This is related to the path split between png and wrm)."
                 "We should stop and consider what we should actually do")
            this->pnc_bmp_cache = new BmpCache(24, 600, 768, 300, 3072, 262, 12288);
            if (this->enable_file_encryption) {
                this->crypto_wrm_trans = new CryptoOutmetaTransport( wrm_path, hash_path, basename, now
                                                                   , width, height
                                                                   , ini.video.capture_groupid);
                this->pnc = new NativeCapture( now, *this->crypto_wrm_trans, width, height
                                             , *this->pnc_bmp_cache, *this->drawable, ini);
            }
            else
            {
                this->wrm_trans = new OutmetaTransport( wrm_path, basename, now, width, height
                                                      , ini.video.capture_groupid);
                this->pnc = new NativeCapture( now, *this->wrm_trans, width, height, *this->pnc_bmp_cache
                                             , *this->drawable, ini);
            }
            this->pnc->recorder.send_input = true;
        }

        pointer_item pointer0(POINTER_CURSOR0);
        this->ptr_cache.add_pointer_static(&pointer0, 0);
        if (this->drawable) {
            this->drawable->send_pointer(0,
                                         pointer0.data,
                                         pointer0.mask,
                                         pointer0.x,
                                         pointer0.y);
        }
        pointer_item pointer1(POINTER_CURSOR1);
        this->ptr_cache.add_pointer_static(&pointer1, 1);
        if (this->drawable) {
            this->drawable->send_pointer(1,
                                         pointer1.data,
                                         pointer1.mask,
                                         pointer1.x,
                                         pointer1.y);
        }
    }

    virtual ~Capture() {
        delete this->psc;
        delete this->png_trans;

        delete this->pnc;
        if (this->enable_file_encryption == false) {
            delete this->wrm_trans;
        }
        else {
            delete this->crypto_wrm_trans;
        }
        delete this->pnc_bmp_cache;

        delete this->drawable;

        clear_files_flv_meta_png(this->png_path.c_str(), this->basename.c_str());
    }

    void pause() {
        if (this->capture_png) {
            struct timeval now = tvtime();
            this->psc->pause_snapshot(now);
        }
    }

    void resume() {
        if (this->capture_wrm){
            if (this->enable_file_encryption) {
                this->crypto_wrm_trans->next();
            }
            else {
                this->wrm_trans->next();
            }
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

    void snapshot( const timeval & now, int x, int y, bool pointer_already_displayed
                 , bool no_timestamp, bool ignore_frame_in_timeval) {
        this->capture_event.reset();

        if (this->capture_png) {
            this->psc->snapshot( now, x, y, pointer_already_displayed, no_timestamp
                               , ignore_frame_in_timeval);
            this->capture_event.update(this->psc->time_to_wait);
        }
        if (this->capture_wrm) {
            this->pnc->snapshot( now, x, y, pointer_already_displayed, no_timestamp
                               , ignore_frame_in_timeval);
            this->capture_event.update(this->pnc->time_to_wait);
        }
    }

    void flush() {
        if (this->capture_png) {
            this->psc->flush();
        }
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
        if (this->capture_wrm) {
            this->pnc->draw(cmd, clip);
        }
        else if (this->capture_drawable) {
            this->drawable->draw(cmd, clip);
        }
    }

    void draw(const RDPDestBlt & cmd, const Rect &clip) {
        if (this->capture_wrm) {
            this->pnc->draw(cmd, clip);
        }
        else if (this->capture_drawable) {
            this->drawable->draw(cmd, clip);
        }
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip) {
        if (this->capture_wrm) {
            this->pnc->draw(cmd, clip);
        }
        else if (this->capture_drawable) {
            this->drawable->draw(cmd, clip);
        }
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) {
        if (this->capture_wrm) {
            this->pnc->draw(cmd, clip, bmp);
        }
        else if (this->capture_drawable) {
            this->drawable->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) {
        if (this->capture_wrm) {
            this->pnc->draw(cmd, clip, bmp);
        }
        else if (this->capture_drawable) {
            this->drawable->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip) {
        if (this->capture_wrm) {
            this->pnc->draw(cmd, clip);
        }
        else if (this->capture_drawable) {
            this->drawable->draw(cmd, clip);
        }
    }

    void draw(const RDPLineTo & cmd, const Rect & clip) {
        if (this->capture_wrm) {
            this->pnc->draw(cmd, clip);
        }
        else if (this->capture_drawable) {
            this->drawable->draw(cmd, clip);
        }
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip,
        const GlyphCache * gly_cache) {}

    virtual void draw( const RDPBitmapData & bitmap_data, const uint8_t * data
                     , size_t size, const Bitmap & bmp) {
        if (this->capture_wrm) {
            this->pnc->draw(bitmap_data, data, size, bmp);
        }
        else if (this->capture_drawable) {
            this->drawable->draw(bitmap_data, data, size, bmp);
        }
    }

    virtual void server_set_pointer(int hotspot_x, int hotspot_y,
        const uint8_t * data, const uint8_t * mask) {
        int cache_idx = 0;
        switch (this->ptr_cache.add_pointer(data,
                                            mask,
                                            hotspot_x,
                                            hotspot_y,
                                            cache_idx)) {
        case POINTER_TO_SEND:
            this->send_pointer(cache_idx, data, mask, hotspot_x, hotspot_y);
        break;
        default:
        case POINTER_ALLREADY_SENT:
            this->set_pointer(cache_idx);
        break;
        }
    }

    virtual void send_pointer(int cache_idx, const uint8_t * data,
        const uint8_t * mask, int hotspot_x, int hotspot_y) {
        if (this->capture_wrm) {
            this->pnc->send_pointer(cache_idx, data, mask,
                hotspot_x, hotspot_y);
        }
        else if (this->capture_drawable) {
            this->drawable->send_pointer(cache_idx, data, mask,
                hotspot_x, hotspot_y);
        }
    }

    virtual void set_pointer(int cache_idx) {
        if (this->capture_wrm) {
            this->pnc->set_pointer(cache_idx);
        }
        else if (this->capture_drawable) {
            this->drawable->set_pointer(cache_idx);
        }
    }
};

#endif
