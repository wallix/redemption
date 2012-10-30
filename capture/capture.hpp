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
    char log_prefix[256];

    FileSequence * png_sequence;
    OutByFilenameSequenceTransport * png_trans;
    StaticCapture * psc;

    FileSequence * wrm_sequence;
    OutByFilenameSequenceTransport * wrm_trans;
    NativeCapture * pnc;

public:
    Capture(const timeval & now, const Inifile & ini, int width, int height) 
      : png_sequence(NULL)
      , png_trans(NULL)
      , psc(NULL)
    {
        char path[1024];
        char basename[1024];
        strcpy(path, "/tmp/"); 
        strcpy(basename, "redemption"); 
        const char * end_of_path = strrchr(ini.globals.movie_path, '/') + 1;
        if (end_of_path){
            memcpy(path, ini.globals.movie_path, end_of_path - ini.globals.movie_path);
            path[end_of_path - ini.globals.movie_path] = 0;
            const char * start_of_extension = strrchr(end_of_path, '.');
            if (start_of_extension){
                memcpy(basename, end_of_path, start_of_extension - end_of_path);
                basename[start_of_extension - end_of_path] = 0;
            }
            else {
                if (end_of_path[0]){
                    strcpy(basename, end_of_path);
                }
            }
        }
        
        this->png_sequence = new FileSequence("path file pid count extension", path, basename, "png");
        this->png_trans = new OutByFilenameSequenceTransport(*this->png_sequence);
        this->psc = new StaticCapture(now, *this->png_trans, *this->png_sequence, width, height);

        this->wrm_sequence = new FileSequence("path file pid count extension", path, basename, "wrm");
        this->wrm_trans = new OutByFilenameSequenceTransport(*this->wrm_sequence);
        this->pnc = new NativeCapture(now, *this->wrm_trans, width, height);
 
        this->log_prefix[0] = 0;
        
        char buffer[256];
        snprintf(buffer, 256, "type='OCR title bar' "
                              "username='%s' "
                              "client_ip='%s' "
                              "ressource='%s' "
                              "account='%s'", 
                ini.globals.auth_user, 
                ini.globals.host, 
                ini.globals.target_device,
                ini.globals.target_user);
        buffer[255] = 0;
        this->set_prefix(buffer, strlen(buffer));
        this->update_config(now, ini);
   }

    ~Capture(){
        delete this->psc;
        delete this->wrm_sequence;
        delete this->wrm_trans;

        delete this->pnc;
        delete this->png_sequence;
        delete this->png_trans;
    }
    
    void update_config(const timeval & now, const Inifile & ini){
        this->psc->update_config(ini);
        this->pnc->update_config(ini);
    }

    void set_prefix(const char * prefix, size_t len_prefix)
    {
        size_t len = (len_prefix < sizeof(log_prefix))?len_prefix:(sizeof(log_prefix)-1);
        memcpy(this->log_prefix, prefix, len);
        this->log_prefix[len] = 0;
    }

    void snapshot(int x, int y, bool pointer_already_displayed, bool no_timestamp)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        this->psc->snapshot(now, x, y, pointer_already_displayed, no_timestamp);
        this->pnc->snapshot(now, x, y, pointer_already_displayed, no_timestamp);
    }

    void flush()
    {}

//    timeval& timer()
//    {
//        return this->pnc->recorder.timer;
//    }

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        this->psc->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }

    void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        this->psc->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->psc->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->psc->draw(cmd, clip, bmp);
        this->pnc->draw(cmd, clip, bmp);
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->psc->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }


    void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        this->psc->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
//        this->psc->glyph_index(cmd, clip);
//        this->pnc->glyph_index(cmd, clip);
    }

};

#endif
