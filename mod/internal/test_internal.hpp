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


struct test_internal_mod : public internal_mod {
    test_internal_mod(
        wait_obj * event,
        int (& keys)[256], int & key_flags, Keymap * &keymap,
        ModContext & context, Front & front):
            internal_mod(keys, key_flags, keymap, front)
    {
      this->mod_bpp = this->get_front_bpp();
        this->event = event;
        this->event->set();
    }

    virtual ~test_internal_mod()
    {
    }

    // event from front (keyboard or mouse event)
    virtual int mod_event(int msg, long x, long y, long param4, long param5)
    {
        return 0;
    }

    // event from back end (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    virtual int mod_signal()
    {
        this->event->reset();
        this->draw();
        return 0;
    }

    void draw()
    {
        this->front.send_global_palette(this->palette332);

        this->server_begin_update();
//        this->color_cache(this->palette332, 0);
//        this->color_cache(this->palette332, 1);
//        this->color_cache(this->palette332, 2);
//        this->color_cache(this->palette332, 3);
//        this->color_cache(this->palette332, 4);
//        this->color_cache(this->palette332, 5);

//        #include "trace2008.cpp"
        #include "vnctest.cpp"

// {
//     RDPPatBlt cmd(Rect(381, 702, 10, 16), 240, 0xffffff, 0xd6d3ce,
//          RDPBrush(5, 7, 3, 170,            (const uint8_t *)"\x55\xaa\x55\xaa\x55\xaa\x55"));
//     this->front.pat_blt(cmd, Rect(0, 0, 1280, 800));
// }
//     {
//         RDPBrushCache cmd1(9, 1, 8, 8, 0, 8,(const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55");
//         this->front.orders->send(cmd1);
//         RDPBrush brush(5, 7, 0x81, 9);
//         RDPPatBlt cmd(Rect(381, 702, 10, 16), 240, 0xffffff, 0xd6d3ce, brush );
//         this->front.orders->send(cmd, Rect(0, 0, 1280, 800));
//     }
        this->server_end_update();
    }

};

#endif
