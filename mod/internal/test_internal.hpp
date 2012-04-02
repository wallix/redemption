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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Use (implemented) basic RDP orders to draw some known test pattern

*/

#if !defined(__TEST_INTERNAL_HPP__)
#define __TEST_INTERNAL_HPP__

#include "GraphicToFile.hpp"
#include "RDP/RDPGraphicDevice.hpp"

struct test_internal_mod : public internal_mod {
    test_internal_mod(
        wait_obj * event,
        ModContext & context, FrontAPI & front, uint16_t width, uint16_t height):
            internal_mod(front, width, height)
    {
        this->front.set_mod_bpp(24);
        this->event = event;
        this->event->set();
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
        this->event->reset();
        const char * movie = "/tmp/replay.wrm";
        int fd = ::open(movie, O_RDONLY);
        assert(fd > 0);
        InFileTransport in_trans(fd);
        RDPUnserializer reader(&in_trans, &this->front, this->get_screen_rect());
        this->front.send_global_palette();
        this->front.begin_update();
        while (reader.next()){
        }
        this->front.end_update();
        return BACK_EVENT_NONE;
    }
};

#endif
