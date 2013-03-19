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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean
*/

#ifndef _REDEMPTION_CAPTURE_CAPTURE_HPP_
#define _REDEMPTION_CAPTURE_CAPTURE_HPP_

#include "staticcapture.hpp"
#include "nativecapture.hpp"
#include "outmetatransport.hpp"
#include "outbyfilenamesequencetransport.hpp"
class Capture : public RDPGraphicDevice
{
public:
    const bool capture_wrm;
    const bool capture_drawable;
    const bool capture_png;
    FileSequence * png_sequence;

    OutByFilenameSequenceTransport * png_trans;
    StaticCapture * psc;

    OutmetaTransport * wrm_trans;
    BmpCache * pnc_bmp_cache;
    RDPDrawable * drawable;
    NativeCapture * pnc;
    
    TODO("capture_wrm flag should be changed to some configuration parameter in inifile")
    Capture(const timeval & now, int width, int height, const char * path, const char * basename, const Inifile & ini) 
      : capture_wrm(ini.globals.capture_wrm)
      , capture_drawable(ini.globals.capture_wrm)
      , capture_png(ini.globals.png_limit > 0)
      , png_sequence(NULL)
      , png_trans(NULL)
      , psc(NULL)
      , wrm_trans(NULL)
      , pnc_bmp_cache(NULL)
      , drawable(NULL)
      , pnc(NULL)
    {
        if (this->capture_png){
            this->png_sequence = new FileSequence("path file pid count extension", path, basename, ".png");
            this->png_trans = new OutByFilenameSequenceTransport(*this->png_sequence);
            this->psc = new StaticCapture(now, *this->png_trans, *this->png_sequence, width, height, ini);
        }

        if (this->capture_drawable){
            this->drawable = new RDPDrawable(width, height, false);
        }

        if (this->capture_wrm){
            this->wrm_trans = new OutmetaTransport(path, basename, now, width, height);
            this->pnc_bmp_cache = new BmpCache(24, 600, 768, 300, 3072, 262, 12288); 
            this->pnc = new NativeCapture(now, *this->wrm_trans, width, height, *this->pnc_bmp_cache, this->drawable, ini);
            this->pnc->recorder.send_input = true;
        }
   }

    ~Capture(){
        delete this->psc;
        delete this->png_sequence;
        delete this->png_trans;

        delete this->pnc;
        delete this->wrm_trans;
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

    virtual void set_row(size_t rownum, const uint8_t * data) 
    { 
        if (this->capture_drawable){
            this->drawable->set_row(rownum, data);
        }
        if (this->capture_png){ 
            this->psc->set_row(rownum, data);
        }
        if (this->capture_wrm){
            this->pnc->set_row(rownum, data);
        }
    }


    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        if (this->capture_drawable){
            this->drawable->draw(cmd, clip);
        }
        if (this->capture_png){ 
            this->psc->draw(cmd, clip);
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
        if (this->capture_png){ 
            this->psc->draw(cmd, clip);
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
        if (this->capture_png){ 
            this->psc->draw(cmd, clip);
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
        if (this->capture_png){ 
            this->psc->draw(cmd, clip, bmp);
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
        if (this->capture_png){ 
            this->psc->draw(cmd, clip);
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
        if (this->capture_png){ 
            this->psc->draw(cmd, clip);
        }
        if (this->capture_wrm){
            this->pnc->draw(cmd, clip);
        }
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
//        this->psc->glyph_index(cmd, clip);
//        this->drawable->draw(cmd, clip);
//        this->pnc->glyph_index(cmd, clip);
    }

};

#endif
