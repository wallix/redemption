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
  Copyright (C) Wallix 2020
  Author(s): Proxy Team
*/

#include "mod/internal/rail_module_host_mod.hpp"
#include "mod/null/null.hpp"
#include "utils/timebase.hpp"
#include "RAIL/client_execute.hpp"
#include "core/events.hpp"
#include "core/RDP/capabilities/window.hpp"
#include "gdi/graphic_api_forwarder.hpp"

#include "test_only/front/fake_front.hpp"
#include "test_only/core/font.hpp"

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/rail_module_host_mod/"

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "gdi/graphic_api.hpp"

struct TestRectMod : null_mod
{
    // vector of Rect
    uint16_t width;
    uint16_t height;
    gdi::GraphicApi* gd;

    TestRectMod(uint16_t width, uint16_t height, gdi::GraphicApi& gd)
    : width(width)
    , height(height)
    , gd(&gd)
    {}

    void rdp_input_invalidate(Rect rect) override
    {
        this->gd->draw(
            RDPOpaqueRect(rect, encode_color24()(RED)),
            rect,
            gdi::ColorCtx(gdi::Depth::depth24(), nullptr));
    }

    void refresh(Rect clip) override
    {
        this->rdp_input_invalidate(clip);
    }

    Dimension get_dim() const override
    {
        return {width, height};
    }
};

struct TestGd : gdi::GraphicApiForwarder<gdi::GraphicApi&>
{
    using gdi::GraphicApiForwarder<gdi::GraphicApi&>::GraphicApiForwarder;

    void set_pointer(uint16_t cache_idx, const Pointer & cursor, gdi::GraphicApi::SetPointerMode mode) override
    {
        (void)cache_idx;
        (void)mode;
        this->last_cursor = cursor;
    }

    Pointer last_cursor;
};

RED_AUTO_TEST_CASE(TestRailHostMod)
{
    const uint16_t w = 400;
    const uint16_t h = 200;

    ScreenInfo screen_info{w, h, BitsPerPixel::BitsPP16};
    EventContainer events;
    FakeFront front(screen_info);
    WindowListCaps win_caps;
    TestGd gd(front.gd());
    ClientExecute client_execute(events, gd, front, win_caps, false);
    const Theme theme;
    const GCC::UserData::CSMonitor cs_monitor;
    Font const& font = global_font_deja_vu_14();

    auto mod = std::make_unique<TestRectMod>(w, h, gdi::null_gd());
    mod->init();
    // mod is moved in railmo
    auto& mod_ref = *mod;

    front.add_channel(channel_names::rail, 0, 0);
    client_execute.enable_remote_program(true);

    const Rect widget_rect = client_execute.adjust_rect(cs_monitor.get_widget_rect(w, h));

    RailModuleHostMod host_mod(
        events, gd, front, w, h, widget_rect, std::move(mod),
        client_execute, font, theme, cs_monitor, false);
    host_mod.init();

    mod_ref.gd = &host_mod.get_module_host().proxy_gd();

    RED_TEST(w == host_mod.get_dim().w);
    RED_TEST(h == host_mod.get_dim().h);

    host_mod.rdp_input_invalidate(Rect{ 0, 0, w, h });
    RED_CHECK_IMG(front, IMG_TEST_PATH "rail1.png");

    // set pointer mod
    mod_ref.gd->set_pointer(0, normal_pointer(), gdi::GraphicApi::SetPointerMode::Insert);

    // move to top border
    host_mod.rdp_input_mouse(MOUSE_FLAG_MOVE, 200, 19, nullptr);
    RED_TEST((gd.last_cursor == size_NS_pointer()));

    // move to widget
    host_mod.rdp_input_mouse(MOUSE_FLAG_MOVE, 200, 100, nullptr);
    RED_TEST((gd.last_cursor == normal_pointer()));
}
