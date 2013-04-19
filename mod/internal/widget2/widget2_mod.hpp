/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_WIDGET2_MOD_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_WIDGET2_MOD_HPP

// #include "widget.hpp"
#include "front.hpp"
#include "mod_api.hpp"
#include "internal/internal_mod.hpp"
#include "selector.hpp"

class widget2_mod : public internal_mod
{
    class WidgetModApi : public ModApi {
        widget2_mod * api;

    public:
        WidgetModApi(widget2_mod * mod)
        : api(mod)
        {}

        virtual ~WidgetModApi()
        {}

        virtual void begin_update()
        {
            this->api->begin_update();
        }

        virtual void end_update()
        {
            this->api->end_update();
        }

        virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
        {
            this->api->draw(cmd, clip);
        }

        virtual void draw(const RDPScrBlt & cmd, const Rect &clip)
        {
            this->api->draw(cmd, clip);
        }

        virtual void draw(const RDPDestBlt & cmd, const Rect &clip)
        {
            this->api->draw(cmd, clip);
        }

        virtual void draw(const RDPPatBlt & cmd, const Rect &clip)
        {
            this->api->draw(cmd, clip);
        }

        virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
        {
            this->api->draw(cmd, clip, bmp);
        }

        virtual void draw(const RDPLineTo& cmd, const Rect & clip)
        {
            this->api->draw(cmd, clip);
        }

        virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip)
        {
            this->api->draw(cmd, clip);
        }

        virtual void server_draw_text(int x, int y, const char* text, uint32_t fgcolor, const Rect& clip)
        {
            this->api->server_draw_text(x, y, text, WHITE, fgcolor, clip);
            //Front& front = static_cast<Front&>(this->api->front);
            //front.capture->drawable->server_draw_text(x,y,text,fgcolor, clip, front.font);
        }

        virtual void text_metrics(const char * text, int & width, int & height)
        {
            this->api->text_metrics(text, width, height);
            //Front& front = static_cast<Front&>(this->api->front);
            //height = 0;
            //width = 0;
            //uint32_t uni[256];
            //size_t len_uni = UTF8toUnicode(reinterpret_cast<const uint8_t *>(text), uni, sizeof(uni)/sizeof(uni[0]));
            //if (len_uni){
            //    for (size_t index = 0; index < len_uni; index++) {
            //        FontChar *font_item = front.capture->drawable->get_font(front.font, uni[index]);
            //        width += font_item->width + 2;
            //        height = std::max(height, font_item->height);
            //    }
            //    width -= 2;
            //}
            //this->api->text_metrics(text, width, height);
        }
    } widget_mod_api;
    WidgetSelector selector;

public:
    widget2_mod(ModContext& context, Front& front, uint16_t width, uint16_t height)
    : internal_mod(front, width, height)
    , widget_mod_api(this)
    , selector(context, &this->widget_mod_api, "bidule", width, height, NULL)
    {
        this->selector.refresh_context();
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.set_index_list(0);
    }

    virtual ~widget2_mod()
    {}

//     virtual BackEvent_t draw_event()
//     {
//         this->refresh();
//         //this->event.reset();
//         this->event.set(33333); // 0.3s is 30fps
//         return BACK_EVENT_NONE;
//     }

    virtual void rdp_input_invalidate(const Rect& r)
    {
        this->rdp_input_invalidate(r);
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        //this->rdp_input_mouse(device_flags, x, y, keymap);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        this->front.begin_update();
        this->selector.account_device_lines.rdp_input_scancode(param1, param2, param3, param4, keymap);
        this->front.end_update();
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {}

    void refresh()
    {
        this->front.begin_update();
        this->selector.refresh(this->selector.rect);
        this->front.end_update();
    }
};

#endif