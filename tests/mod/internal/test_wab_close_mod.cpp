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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWabCloseMod
#include <boost/test/auto_unit_test.hpp>

#undef FIXTURES_PATH
#define FIXTURES_PATH "./tests/fixtures"
#undef SHARE_PATH
#define SHARE_PATH "./tests/fixtures"

#define LOGNULL
#include "log.hpp"

#include "internal/wab_close_mod.hpp"

BOOST_AUTO_TEST_CASE(TestWabCloseMod)
{
    ClientInfo info(1, true, true);
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;

    class Front : public FrontAPI {
        public:
        const ClientInfo & info;
        CHANNELS::ChannelDefArray cl;

        virtual void flush() {}
        virtual void draw(const RDPOpaqueRect& cmd, const Rect& clip)
        {
            cmd.log(LOG_INFO, clip);
        }
        virtual void draw(const RDPScrBlt& cmd, const Rect& clip)
        {
            cmd.log(LOG_INFO, clip);
        }
        virtual void draw(const RDPDestBlt& cmd, const Rect& clip)
        {
            cmd.log(LOG_INFO, clip);
        }
        virtual void draw(const RDPPatBlt& cmd, const Rect& clip)
        {
            cmd.log(LOG_INFO, clip);
        }
        virtual void draw(const RDPMemBlt& cmd, const Rect& clip, const Bitmap& bmp)
        {
            cmd.log(LOG_INFO, clip);
        }
        virtual void draw(const RDPMem3Blt& cmd, const Rect& clip, const Bitmap& bmp)
        {
            cmd.log(LOG_INFO, clip);
        }
        virtual void draw(const RDPLineTo& cmd, const Rect& clip)
        {
            cmd.log(LOG_INFO, clip);
        }
        virtual void draw(const RDPGlyphIndex& cmd, const Rect& clip, const GlyphCache * gly_cache)
        {
            cmd.log(LOG_INFO, clip);
        }

        virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const { return cl; }
        virtual void send_to_channel(const CHANNELS::ChannelDef & channel, uint8_t* data, size_t length, size_t chunk_size, int flags)
        {
        }

        virtual void send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y) throw (Error)
        {
            LOG(LOG_INFO, "send_pointer(cache_idx=%d, data=%p, mask=%p, x=%d, y=%d\n",
                cache_idx, data, mask, x, y);
        }
        virtual void send_global_palette() throw (Error)
        {
            LOG(LOG_INFO, "send_global_palette()\n");
        }
        virtual void set_pointer(int cache_idx) throw (Error)
        {
            LOG(LOG_INFO, "set_pointer\n");
        }
        virtual void begin_update()
        {
            LOG(LOG_INFO, "begin_update\n");
        }
        virtual void end_update()
        {
            LOG(LOG_INFO, "end_update\n");
        }
        virtual void color_cache(const BGRPalette & palette, uint8_t cacheIndex)
        {
            LOG(LOG_INFO, "color_cache\n");
        }
        virtual void set_mod_palette(const BGRPalette & palette)
        {
            LOG(LOG_INFO, "set_mod_palette\n");
        }
        virtual void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask)
        {
            LOG(LOG_INFO, "server_set_pointer\n");
        }
        virtual void server_draw_text(int16_t x, int16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
        {
            LOG(LOG_INFO, "server_draw_text %s\n", text);
        }
        virtual void text_metrics(const char * text, int & width, int & height)
        {
            LOG(LOG_INFO, "text_metrics\n");
        }
        virtual int server_resize(int width, int height, int bpp)
        {
             LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d\n", width, height, bpp);
             return 0;
        }

        int mouse_x;
        int mouse_y;
        bool notimestamp;
        bool nomouse;

        Front(const ClientInfo & info) :
              FrontAPI(false, false),
              info(info),
              mouse_x(0),
              mouse_y(0),
              notimestamp(true),
              nomouse(true)
            {}

    } front(info);

    Inifile ini;

    Keymap2 keymap;
    keymap.init_layout(info.keylayout);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);

    WabCloseMod d(ini, front, 800, 600, (time_t)100000);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
}
