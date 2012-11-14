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

#if !defined(__CAPTURE_CAPTURE_HPP__)
#define __CAPTURE_CAPTURE_HPP__

#include "staticcapture.hpp"
#include "nativecapture.hpp"

class Capture : public RDPGraphicDevice
{
public:
    FileSequence * meta_sequence;
    FileSequence * wrm_sequence;
    FileSequence * png_sequence;

    OutByFilenameSequenceTransport * png_trans;
    StaticCapture * psc;

    OutByFilenameSequenceWithMetaTransport * wrm_trans;
    BmpCache * pnc_bmp_cache;
    RDPDrawable * drawable;
    NativeCapture * pnc;
    
    Capture(const timeval & now, int width, int height, const char * path, const char * basename, const Inifile & ini) 
      : meta_sequence(NULL)
      , wrm_sequence(NULL)
      , png_sequence(NULL)
      , png_trans(NULL)
      , psc(NULL)
      , wrm_trans(NULL)
      , pnc_bmp_cache(NULL)
      , drawable(NULL)
      , pnc(NULL)
    {
        this->meta_sequence = new FileSequence("path file pid extension", path, basename, "mwrm");
        this->wrm_sequence = new FileSequence("path file pid count extension", path, basename, "wrm");
        this->png_sequence = new FileSequence("path file pid count extension", path, basename, "png");

        this->png_trans = new OutByFilenameSequenceTransport(*this->png_sequence);
        this->psc = new StaticCapture(now, *this->png_trans, *this->png_sequence, width, height, ini);

        this->wrm_trans = new OutByFilenameSequenceWithMetaTransport(*this->meta_sequence, now, width, height, *this->wrm_sequence);
        this->pnc_bmp_cache = new BmpCache(24, 600, 768, 300, 3072, 262, 12288); 
        this->drawable = new RDPDrawable(width, height, true);
        this->pnc = new NativeCapture(now, *this->wrm_trans, width, height, *this->pnc_bmp_cache, this->drawable, ini);
   }

    ~Capture(){
        delete this->psc;
        delete this->png_sequence;
        delete this->png_trans;

        delete this->pnc;
        delete this->wrm_sequence;
        delete this->meta_sequence;
        delete this->wrm_trans;
        delete this->pnc_bmp_cache;
        delete this->drawable;
    }
    
    void update_config(const Inifile & ini){
        this->psc->update_config(ini);
        this->pnc->update_config(ini);
    }

    void snapshot(const timeval & now, int x, int y, bool pointer_already_displayed, bool no_timestamp)
    {
        this->psc->snapshot(now, x, y, pointer_already_displayed, no_timestamp);
        this->drawable->snapshot(now, x, y, pointer_already_displayed, no_timestamp);
        this->pnc->snapshot(now, x, y, pointer_already_displayed, no_timestamp);
    }

    void flush()
    {
        this->psc->flush();
        this->drawable->flush();
        this->pnc->flush();
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        this->psc->draw(cmd, clip);
        this->drawable->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }

    void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        this->psc->draw(cmd, clip);
        this->drawable->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->psc->draw(cmd, clip);
        this->drawable->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->psc->draw(cmd, clip, bmp);
        this->drawable->draw(cmd, clip, bmp);
        this->pnc->draw(cmd, clip, bmp);
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->psc->draw(cmd, clip);
        this->drawable->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }


    void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        this->psc->draw(cmd, clip);
        this->drawable->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
//        this->psc->glyph_index(cmd, clip);
        this->drawable->draw(cmd, clip);
//        this->pnc->glyph_index(cmd, clip);
    }

};

#endif
