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
   Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CAPTURE_CAPTURE_HPP_
#define _REDEMPTION_CAPTURE_CAPTURE_HPP_

#include "staticcapture.hpp"
#include "nativecapture.hpp"
#include "outmetatransport.hpp"
#include "outfilenametransport.hpp"
class Capture : public RDPGraphicDevice
{
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

    TODO("capture_wrm flag should be changed to some configuration parameter in inifile")
    Capture(const timeval & now, int width, int height, const char * wrm_path, const char * png_path, const char * hash_path, const char * basename, bool clear_png, const Inifile & ini)
      : capture_wrm(ini.video.capture_wrm)
      , capture_drawable(ini.video.capture_wrm||(ini.video.png_limit > 0))
      , capture_png(ini.video.png_limit > 0)
      , enable_file_encryption(ini.globals.enable_file_encryption)
      , png_trans(NULL)
      , psc(NULL)
      , wrm_trans(NULL)
      , crypto_wrm_trans(NULL)
      , pnc_bmp_cache(NULL)
      , pnc(NULL)
      , drawable(NULL)
    {
        if (this->capture_drawable){
            this->drawable = new RDPDrawable(width, height, false);
        }

        if (this->capture_png){
            if (recursive_create_directory(png_path, S_IRWXU|S_IRWXG, ini.video.capture_groupid) != 0) {
                LOG(LOG_ERR, "Failed to create directory: \"%s\"", png_path);
            }

            this->png_trans = new OutFilenameTransport(SQF_PATH_FILE_PID_COUNT_EXTENSION, png_path, basename, ".png", ini.video.capture_groupid);
            this->psc = new StaticCapture(now, *this->png_trans, &(this->png_trans->seq), width, height, clear_png, ini, this->drawable->drawable);
        }

        if (this->capture_wrm){
            if (recursive_create_directory(wrm_path, S_IRWXU|S_IRGRP|S_IXGRP, ini.video.capture_groupid) != 0) {
                LOG(LOG_ERR, "Failed to create directory: \"%s\"", wrm_path);
            }

            if (recursive_create_directory(hash_path, S_IRWXU|S_IRGRP|S_IXGRP, ini.video.capture_groupid) != 0) {
                LOG(LOG_ERR, "Failed to create directory: \"%s\"", hash_path);
            }

            TODO("there should only be one outmeta, not two. Capture code should not really care if file is encrypted or not."
                 "Here is not the right level to manage anything related to encryption.")
            TODO("Also we may wonder why we are encrypting wrm and not png"
                 "(This is related to the path split between png and wrm)."
                 "We should stop and consider what we should actually do")
            if (this->enable_file_encryption) {
                this->crypto_wrm_trans = new CryptoOutmetaTransport(wrm_path, hash_path, basename, now, width, height, ini.video.capture_groupid);
                this->pnc_bmp_cache = new BmpCache(24, 600, 768, 300, 3072, 262, 12288);
                this->pnc = new NativeCapture(now, *this->crypto_wrm_trans, width, height, *this->pnc_bmp_cache, this->drawable, ini);
            }
            else
            {
                this->wrm_trans = new OutmetaTransport(wrm_path, basename, now, width, height, ini.video.capture_groupid);
                this->pnc_bmp_cache = new BmpCache(24, 600, 768, 300, 3072, 262, 12288);
                this->pnc = new NativeCapture(now, *this->wrm_trans, width, height, *this->pnc_bmp_cache, this->drawable, ini);
            }
            this->pnc->recorder.send_input = true;
        }
   }

    ~Capture(){
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
    }

    void update_config(const Inifile & ini){
//        if (this->capture_drawable){
//            this->drawable->update_config(ini);
//        }
        if (this->capture_png){ 
            this->psc->update_config(ini);
        }
        if (this->capture_wrm){
            this->pnc->update_config(ini);
        }
    }

    void snapshot(const timeval & now, int x, int y, bool pointer_already_displayed, bool no_timestamp)
    {
        if (this->capture_drawable){
            this->drawable->snapshot(now, x, y, pointer_already_displayed, no_timestamp);
        }
        if (this->capture_png){ 
            this->psc->snapshot(now, x, y, pointer_already_displayed, no_timestamp);
        }
        if (this->capture_wrm){
            this->pnc->snapshot(now, x, y, pointer_already_displayed, no_timestamp);
        }
    }

    void flush()
    {
        if (this->capture_drawable){
            this->drawable->flush();
        }
        if (this->capture_png){ 
            this->psc->flush();
        }
        if (this->capture_wrm){
            this->pnc->flush();
        }
    }

    void input(const timeval & now, Stream & input_data_32) {
        if (this->capture_wrm){
            this->pnc->input(now, input_data_32);
        }
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        if (this->capture_drawable){
            this->drawable->draw(cmd, clip);
        }
        if (this->capture_wrm){
            this->pnc->draw(cmd, clip);
        }
    }

    void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        if (this->capture_drawable){
            this->drawable->draw(cmd, clip);
        }
        if (this->capture_wrm){
            this->pnc->draw(cmd, clip);
        }
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        if (this->capture_drawable){
            this->drawable->draw(cmd, clip);
        }
        if (this->capture_wrm){
            this->pnc->draw(cmd, clip);
        }
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        if (this->capture_drawable){
            this->drawable->draw(cmd, clip, bmp);
        }
        if (this->capture_wrm){
            this->pnc->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        if (this->capture_drawable){
            this->drawable->draw(cmd, clip, bmp);
        }
        if (this->capture_wrm){
            this->pnc->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        if (this->capture_drawable){
            this->drawable->draw(cmd, clip);
        }
        if (this->capture_wrm){
            this->pnc->draw(cmd, clip);
        }
    }

    void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        if (this->capture_drawable){
            this->drawable->draw(cmd, clip);
        }
        if (this->capture_wrm){
            this->pnc->draw(cmd, clip);
        }
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
    }

    virtual void draw( const RDPBitmapData & bitmap_data, const uint8_t * data
                     , size_t size, const Bitmap & bmp) {
        if (this->capture_drawable) {
            this->drawable->draw(bitmap_data, data, size, bmp);
        }
        if (this->capture_wrm) {
            this->pnc->draw(bitmap_data, data, size, bmp);
        }
    }
};

#endif
