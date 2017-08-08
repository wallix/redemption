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
   Author(s): Christophe Grosjean, Javier Caverni, JOnathan Poelen, Raphael Zhou

   Use (implemented) basic RDP orders to draw some known test pattern
*/

#pragma once

#include "core/defines.hpp"
#include "internal_mod.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "utils/bitmap_from_file.hpp"

class TestCardMod : public InternalMod
{
    BGRPalette const & palette332 = BGRPalette::classic_332();

    Font const & font;

    bool unit_test;

public:
    TestCardMod(FrontAPI & front, uint16_t width, uint16_t height, Font const & font, bool unit_test = true)
    : InternalMod(front, width, height, font, Theme{}, false)
    , font(font)
    , unit_test(unit_test)
    {}

    void rdp_input_invalidate(Rect /*rect*/) override {}

    void rdp_input_mouse(int /*device_flags*/, int /*x*/, int /*y*/, Keymap2 * /*keymap*/) override {}

    void rdp_input_scancode(long /*param1*/, long /*param2*/, long /*param3*/,
                                    long /*param4*/, Keymap2 * keymap) override {
        if (keymap->nb_kevent_available() > 0
         && keymap->get_kevent() == Keymap2::KEVENT_ESC) {
            this->event.signal = BACK_EVENT_STOP;
//            this->event.set();
            this->event.set_trigger_time(wait_obj::NOW);
        }
    }

    void rdp_input_synchronize(uint32_t /*time*/, uint16_t /*device_flags*/,
                                       int16_t /*param1*/, int16_t /*param2*/) override {}

    void refresh(Rect /*rect*/) override {}

    // event from back end (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    void draw_event(time_t now, gdi::GraphicApi & drawable) override {
        (void)now;
        this->draw(drawable);
//        this->event.reset();
        this->event.reset_trigger_time();
    }

    bool is_up_and_running() override { return true; }

    void draw(gdi::GraphicApi & drawable)
    {
        this->front.begin_update();

        const Rect clip = this->get_screen_rect();

        auto const color_ctx = gdi::ColorCtx::depth24();

        this->front.draw(RDPOpaqueRect(this->get_screen_rect(), encode_color24()(WHITE)), clip, color_ctx);
        this->front.draw(RDPOpaqueRect(this->get_screen_rect().shrink(5), encode_color24()(RED)), clip, color_ctx);
        this->front.draw(RDPOpaqueRect(this->get_screen_rect().shrink(10), encode_color24()(GREEN)), clip, color_ctx);
        this->front.draw(RDPOpaqueRect(this->get_screen_rect().shrink(15), encode_color24()(BLUE)), clip, color_ctx);
        this->front.draw(RDPOpaqueRect(this->get_screen_rect().shrink(20), encode_color24()(BLACK)), clip, color_ctx);

        Rect winrect = this->get_screen_rect().shrink(30);
        this->front.draw(RDPOpaqueRect(winrect, encode_color24()(WINBLUE)), clip, color_ctx);


        Bitmap bitmap = bitmap_from_file(SHARE_PATH "/" "Philips_PM5544_640.png");

        this->front.draw(RDPMemBlt(0,
            Rect(winrect.x + (winrect.cx - bitmap.cx())/2,
                 winrect.y + (winrect.cy - bitmap.cy())/2,
                 bitmap.cx(), bitmap.cy()),
                 0xCC,
             0, 0, 0), clip, bitmap);

        //  lineTo mix_mode=1 startx=200 starty=1198 endx=200 endy=145 bg_color=0 rop2=13 clip=(200, 145, 1, 110)
        this->front.draw(
            RDPLineTo(1, 200, 1198, 200, 145, RDPColor{}, 13, RDPPen(0, 1, encode_color24()(RED))),
            Rect(200, 145, 1, 110), color_ctx);

        this->front.draw(
            RDPLineTo(1, 200, 145, 200, 1198, RDPColor{}, 13, RDPPen(0, 1, encode_color24()(RED))),
            Rect(200, 145, 1, 110), color_ctx);

        this->front.draw(
            RDPLineTo(1, 201, 1198, 200, 145, RDPColor{}, 13, RDPPen(0, 1, encode_color24()(RED))),
            Rect(200, 145, 1, 110), color_ctx);

        this->front.draw(
            RDPLineTo(1, 200, 145, 201, 1198, RDPColor{}, 13, RDPPen(0, 1, encode_color24()(RED))),
            Rect(200, 145, 1, 110), color_ctx);

        this->front.draw(
            RDPLineTo(1, 1198, 200, 145, 200, RDPColor{}, 13, RDPPen(0, 1, encode_color24()(RED))),
            Rect(145, 200, 110, 1), color_ctx);

        this->front.draw(
            RDPLineTo(1, 145, 200, 1198, 200, RDPColor{}, 13, RDPPen(0, 1, encode_color24()(RED))),
            Rect(145, 200, 110, 1), color_ctx);

        this->front.draw(
            RDPLineTo(1, 1198, 201, 145, 200, RDPColor{}, 13, RDPPen(0, 1, encode_color24()(RED))),
            Rect(145, 200, 110, 1), color_ctx);

        this->front.draw(
            RDPLineTo(1, 145, 200, 1198, 201, RDPColor{}, 13, RDPPen(0, 1, encode_color24()(RED))),
            Rect(145, 200, 110, 1), color_ctx);

        gdi::server_draw_text(drawable, this->font, 30, 30, "White", encode_color24()(WHITE), encode_color24()(BLACK), color_ctx, clip);
        gdi::server_draw_text(drawable, this->font, 30, 50, "Red  ", encode_color24()(RED), encode_color24()(BLACK), color_ctx, clip);
        gdi::server_draw_text(drawable, this->font, 30, 70, "Green", encode_color24()(GREEN), encode_color24()(BLACK), color_ctx, clip);
        gdi::server_draw_text(drawable, this->font, 30, 90, "Blue ", encode_color24()(BLUE), encode_color24()(BLACK), color_ctx, clip);
        gdi::server_draw_text(drawable, this->font, 30, 110, "Black", encode_color24()(BLACK), encode_color24()(WHITE), color_ctx, clip);

        Bitmap card = bitmap_from_file(SHARE_PATH "/" REDEMPTION_LOGO24);
        this->front.draw(RDPMemBlt(0,
            Rect(this->get_screen_rect().cx - card.cx() - 30,
                 this->get_screen_rect().cy - card.cy() - 30, card.cx(), card.cy()),
                 0xCC,
             0, 0, 0), clip, card);

        // Bogus square generating zero width/height tiles if not properly guarded
        uint8_t comp64x64RED[] = {
            0xc0, 0x30, 0x00, 0x00, 0xFF,
            0xf0, 0xc0, 0x0f,
        };

        Bitmap bloc64x64(24, 24, &this->palette332, 64, 64, comp64x64RED, sizeof(comp64x64RED), true);
        this->front.draw(RDPMemBlt(0,
            Rect(0, this->get_screen_rect().cy - 64, bloc64x64.cx(), bloc64x64.cy()), 0xCC,
             32, 32, 0), clip, bloc64x64);

        //Bitmap_PNG logo(SHARE_PATH "/ad8b.bmp");
        Bitmap logo = bitmap_from_file(SHARE_PATH "/ad8b.png");
        this->front.draw(RDPMemBlt(0,
            Rect(100, 100, 26, 32),
            0xCC,
            80, 50, 0), clip, logo);

        if (!unit_test) {
            //this->front.draw(RDPOpaqueRect(this->get_screen_rect(), RED), clip, depth);
            this->front.sync();

            Bitmap wab_logo_blue = bitmap_from_file(SHARE_PATH "/" "wablogoblue.png");


            const uint16_t startx = 5;
            const uint16_t starty = 5;

            const uint16_t tile_width_height = 32;

            for (uint16_t y = 0; y < wab_logo_blue.cy(); y += tile_width_height) {
                uint16_t cy = std::min<uint16_t>(tile_width_height, wab_logo_blue.cy() - y);

                for (uint16_t x = 0; x < wab_logo_blue.cx(); x += tile_width_height) {
                    uint16_t cx = std::min<uint16_t>(tile_width_height, wab_logo_blue.cx() - x);

                    Bitmap tile(wab_logo_blue, Rect(x, y, cx, cy));

                    RDPBitmapData bitmap_data;

                    bitmap_data.dest_left       = startx + x;
                    bitmap_data.dest_top        = starty + y;
                    bitmap_data.dest_right      = bitmap_data.dest_left + cx - 1;
                    bitmap_data.dest_bottom     = bitmap_data.dest_top + cy - 1;
                    bitmap_data.width           = tile.cx();
                    bitmap_data.height          = tile.cy();
                    bitmap_data.bits_per_pixel  = 24;
                    bitmap_data.flags           = 0;
                    bitmap_data.bitmap_length   = tile.bmp_size();

                    bitmap_data.log(LOG_INFO);

                    this->front.draw(bitmap_data, tile);
                }
            }
        }

        this->front.end_update();
    }
};
