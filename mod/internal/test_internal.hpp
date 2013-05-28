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
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Use (implemented) basic RDP orders to draw some known test pattern
*/

#ifndef _REDEMPTION_MOD_INTERNAL_TEST_INTERNAL_HPP_
#define _REDEMPTION_MOD_INTERNAL_TEST_INTERNAL_HPP_

#include "FileToGraphic.hpp"
#include "GraphicToFile.hpp"
#include "RDP/RDPGraphicDevice.hpp"
#include "inbymetasequencetransport.hpp"

struct test_internal_mod : public internal_mod {

    char movie[1024];

    test_internal_mod( FrontAPI & front
                     , char * replay_path
                     , char * movie
                     , uint16_t width
                     , uint16_t height):
            internal_mod(front, width, height)
    {
        TODO("use canonical_path to manage trailing slash")
        strcpy(this->movie, replay_path);
        strcat(this->movie, movie);
        LOG(LOG_INFO, "Playing %s", this->movie);
    }

    virtual ~test_internal_mod()
    {
    }

    virtual void rdp_input_invalidate(const Rect & rect)
    {
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
    }

    virtual void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap){
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        return;
    }

    // event from back end (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    virtual BackEvent_t draw_event()
    {
        this->event.reset();
        TODO("use system constants for sizes");

        char path[1024];
        char basename[1024];
        char extension[128];
        strcpy(path, "/tmp/"); // default value, actual one should come from movie_path
        strcpy(basename, "replay"); // default value actual one should come from movie_path
        strcpy(extension, ".mwrm"); // extension is currently ignored
        char prefix[4096];

        canonical_path(this->movie, path, sizeof(path), basename, sizeof(basename), extension, sizeof(extension));
        sprintf(prefix, "%s%s", path, basename);

        TODO("RZ: Support encrypted recorded file.")
        InByMetaSequenceTransport in_trans(prefix, extension);
        timeval begin_capture; begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
        timeval end_capture; end_capture.tv_sec = 0; end_capture.tv_usec = 0;
        FileToGraphic reader(&in_trans, begin_capture, end_capture, true, 0);
        reader.add_consumer(&this->front);
        this->front.send_global_palette();
        this->front.begin_update();
        while (reader.next_order()){
            reader.interpret_order();
        }
        this->front.end_update();
        return BACK_EVENT_STOP;
    }
};

#endif
