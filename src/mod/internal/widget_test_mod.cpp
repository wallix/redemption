/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Meng Tan, Jonathan Poelen, Raphael Zhou
*/

#include "core/font.hpp"
#include "core/front_api.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "core/session_reactor.hpp"
#include "mod/internal/widget_test_mod.hpp"
#include "keyboard/keymap2.hpp"
#include "utils/bitmap.hpp"
#include "utils/bitmap_private_data.hpp"
#include "utils/sugar/update_lock.hpp"

#include <cstring>


// Pimpl
struct WidgetTestMod::WidgetTestModPrivate
{
    WidgetTestModPrivate(SessionReactor& session_reactor, WidgetTestMod& mod)
      : session_reactor(session_reactor)
    {
        LOG(LOG_DEBUG, "WidgetTestModPrivate");
        this->timer = this->session_reactor.create_graphic_timer(std::ref(mod))
        .set_delay(std::chrono::seconds(0))
        .on_action([](auto ctx, gdi::GraphicApi& gd, WidgetTestMod& mod){
            update_lock update_lock{mod.front};
            int y = 10;
            for (auto s : {
                // "/home/jpoelen/rawdisk2/Laksaman_14.rbf",
                // "/home/jpoelen/rawdisk2/Laksaman_15.rbf",
                // "/home/jpoelen/rawdisk2/Laksaman_16.rbf",
                // "/home/jpoelen/rawdisk2/Laksaman_17.rbf",
                "/home/jpoelen/rawdisk2/Lato-Light_14.rbf",
                "/home/jpoelen/rawdisk2/Lato-Light_15.rbf",
                "/home/jpoelen/rawdisk2/Lato-Light_16.rbf",
                "/home/jpoelen/rawdisk2/Lato-Light_17.rbf",
                "/home/jpoelen/rawdisk2/Lato-Thin_14.rbf",
                "/home/jpoelen/rawdisk2/Lato-Thin_15.rbf",
                "/home/jpoelen/rawdisk2/Lato-Thin_16.rbf",
                "/home/jpoelen/rawdisk2/Lato-Thin_17.rbf",
                "/home/jpoelen/rawdisk2/Lohit-Telugu_14.rbf",
                "/home/jpoelen/rawdisk2/Lohit-Telugu_15.rbf",
                "/home/jpoelen/rawdisk2/Lohit-Telugu_16.rbf",
                "/home/jpoelen/rawdisk2/Lohit-Telugu_17.rbf",
                // "/home/jpoelen/rawdisk2/NanumSquareR_14.rbf",
                // "/home/jpoelen/rawdisk2/NanumSquareR_15.rbf",
                // "/home/jpoelen/rawdisk2/NanumSquareR_16.rbf",
                // "/home/jpoelen/rawdisk2/NanumSquareR_17.rbf",
                // "/home/jpoelen/rawdisk2/Suruma_14.rbf",
                // "/home/jpoelen/rawdisk2/Suruma_15.rbf",
                // "/home/jpoelen/rawdisk2/Suruma_16.rbf",
                // "/home/jpoelen/rawdisk2/Suruma_17.rbf",
            }) {
                Font font(s);
                auto * text = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789`!@#$%^&*()_=[]'\",./<>?|:{}¹²³ cl◀◂▸▶▲▼▤▥➜¤€’¥×\\æœúíàéèçùµÉÀð.";
                gdi::server_draw_text(
                    gd, font, 10, y, text,
                    encode_color24()(BGRColor(0xeeb6c1)), encode_color24()(BGRColor(0x747132)),
                    gdi::ColorCtx::depth24(), Rect(10, y-10, gdi::TextMetrics(font, text).width, 600));
                y += font.max_height() + 10;
            }
            // return ctx.set_delay(std::chrono::seconds(10)).ready();
            return ctx.terminate();
        });
    }

    SessionReactor& session_reactor;
    SessionReactor::GraphicTimerPtr timer;
};

WidgetTestMod::WidgetTestMod(
    SessionReactor& session_reactor,
    FrontAPI & front, uint16_t width, uint16_t height, Font const & font)
: InternalMod(front, width, height, font, Theme{})
, d(std::make_unique<WidgetTestModPrivate>(session_reactor, *this))
{
    front.server_resize(width, height, BitsPerPixel{8});
}

WidgetTestMod::~WidgetTestMod()
{
    this->screen.clear();
}

void WidgetTestMod::rdp_input_invalidate(Rect /*r*/)
{}

void WidgetTestMod::rdp_input_mouse(int /*device_flags*/, int /*x*/, int /*y*/, Keymap2 * /*keymap*/)
{}

void WidgetTestMod::rdp_input_scancode(
    long /*param1*/, long /*param2*/, long /*param3*/, long /*param4*/, Keymap2 * keymap)
{
    if (keymap->nb_kevent_available() > 0
        && keymap->get_kevent() == Keymap2::KEVENT_ENTER) {
        this->d->session_reactor.set_next_event(BACK_EVENT_STOP);
    }
}

void WidgetTestMod::refresh(Rect clip)
{
    this->rdp_input_invalidate(clip);
}

void WidgetTestMod::draw_event(time_t /*now*/, gdi::GraphicApi& gd)
{
    update_lock<decltype(this->front)> update_lock{this->front};

    auto mono_palette = [&](BGRColor const& color) {
        BGRColor d[256];
        for (auto& c : d) {
            c = color;
        }
        return BGRPalette{d};
    };

    const auto clip = this->get_screen_rect();
    const auto color_ctx = gdi::ColorCtx::depth8(BGRPalette::classic_332());
    const auto encode_color = encode_color8();
    const auto cx = clip.cx / 2;
    const auto cy = clip.cy / 3;

    auto draw_rect = [&](int x, int y, BGRColor color){
        gd.draw(RDPOpaqueRect(Rect(x*cx, y*cy, cx, cy), encode_color(color)), clip, color_ctx);
    };

    auto draw_text = [&](int x, int y, char const* txt){
        gdi::server_draw_text(gd, this->font(), 10+x*cx, 10+y*cy, txt, encode_color(WHITE), encode_color(BLACK), color_ctx, clip);
    };

    auto draw_img = [&](int x, int y, int col, Bitmap const& bitmap){
        gd.draw(RDPMemBlt(col, Rect(x*cx, y*cy, cx, cy), 0xCC, 0, 0, 0), clip, bitmap);
    };

    auto plain_img = [&](BGRColor const& color){
        Bitmap img;
        Bitmap::PrivateData::Data & data = Bitmap::PrivateData::initialize(img, BitsPerPixel{8}, cx, cy);
        memset(data.get(), encode_color(color).as_bgr().to_u32(), data.line_size() * cy);
        //data.palette() = BGRPalette::classic_332();
        data.palette() = mono_palette(color);
        return img;
    };

    const auto img1 = plain_img(GREEN);
    const auto img2 = plain_img(ANTHRACITE);

    draw_rect(0, 0, BLUE);
    draw_rect(1, 0, RED);
    gd.sync();
    draw_img(0, 1, 0, img1);
    draw_img(1, 1, 1, img2);
    gd.sync();
    gd.draw(RDPColCache(0, mono_palette(WHITE)));
    gd.draw(RDPColCache(1, mono_palette(WHITE)));
    gd.sync();
    const auto img3 = plain_img(DARK_WABGREEN);
    const auto img4 = plain_img(BLUE);
    draw_img(0, 2, 0, img3);
    draw_img(1, 2, 1, img4);
    draw_text(0, 0, "blue");
    draw_text(1, 0, "red");
    draw_text(0, 1, "cyan img");
    draw_text(1, 1, "pink img");
    draw_text(0, 2, "yellow palette");
    draw_text(1, 2, "yellow palette");
}
