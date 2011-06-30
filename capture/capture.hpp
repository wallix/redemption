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

#if !defined(__CAPTURE_HPP__)
#define __CAPTURE_HPP__

#include "staticcapture.hpp"
#include "nativecapture.hpp"

class Capture
{
    StaticCapture sc;
    NativeCapture nc;

    public:

    #warning fat interface : ugly, find another way
    Capture(int width, int height, int bpp, char * path, const char * codec_id, const char * video_quality) :
        sc(width, height, bpp, path, codec_id, video_quality),
        nc(width, height, bpp, path)
    {
    }

    ~Capture(){
    }

    #warning fat interface : ugly, find another way
    void snapshot(int x, int y, bool pointer_already_displayed, bool no_timestamp, int timezone)
    {
        this->sc.snapshot(x, y, pointer_already_displayed, no_timestamp, timezone);
        this->nc.snapshot(x, y, pointer_already_displayed, no_timestamp, timezone);
    }

    void bitmap_cache(const RDPBmpCache & cmd)
    {
        this->nc.bitmap_cache(cmd);
    }

    void scr_blt(const RDPScrBlt & cmd, const Rect & clip)
    {
        this->sc.scr_blt(cmd, clip);
        this->nc.scr_blt(cmd, clip);
    }

    void dest_blt(const RDPDestBlt & cmd, const Rect &clip)
    {
        this->sc.dest_blt(cmd, clip);
        this->nc.dest_blt(cmd, clip);
    }

    void pat_blt(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->sc.pat_blt(cmd, clip);
        this->nc.pat_blt(cmd, clip);
    }

    void mem_blt(const RDPMemBlt & cmd, const BitmapCache & bmp_cache, const Rect & clip)
    {
        this->sc.mem_blt(cmd, bmp_cache, clip);
        this->nc.mem_blt(cmd, bmp_cache, clip);
    }

    void opaque_rect(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->sc.opaque_rect(cmd, clip);
        this->nc.opaque_rect(cmd, clip);
    }


    void line_to(const RDPLineTo & cmd, const Rect & clip)
    {
        this->sc.line_to(cmd, clip);
        this->nc.line_to(cmd, clip);

    }

    void glyph_index(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        this->sc.glyph_index(cmd, clip);
        this->nc.glyph_index(cmd, clip);
    }

};

#endif
