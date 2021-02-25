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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "mod/internal/widget/delegated_copy.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/delegated_copy/"

namespace
{
    struct TestWidgetCopyNotifier : Widget
    {
        TestWidgetCopyNotifier(gdi::GraphicApi & drawable, Widget & parent)
        : Widget(drawable, parent, nullptr)
        {}

        void notify(Widget * sender, NotifyApi::notify_event_t event) override
        {
            this->last_event = event;
            this->last_widget = sender;
        }

        void rdp_input_invalidate(Rect r) override
        {
            (void)r;
        }

        NotifyApi::notify_event_t last_event = 0;
        Widget const * last_widget = nullptr;
    };
} // anonymous namespace

RED_AUTO_TEST_CASE(TraceWidgetDelegatedCopy)
{
    const uint16_t w = 30;
    const uint16_t h = 30;
    const Font& font = global_font_deja_vu_14();

    TestGraphic gd(w, h);

    WidgetScreen screen(gd, w, h, font, nullptr, Theme{});

    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor focus_color = BLUE;

    TestWidgetCopyNotifier widget(gd, screen);
    TestWidgetCopyNotifier dummy_widget(gd, screen);
    WidgetDelegatedCopy delegated(gd, dummy_widget, widget, fg_color, bg_color, focus_color, font, 2, 2);

    delegated.set_wh(delegated.get_optimal_dim());
    delegated.set_xy(5, 5);

    screen.add_widget(&delegated);
    screen.refresh(screen.get_rect());

    RED_CHECK(widget.last_event == 0);
    RED_CHECK(!widget.last_widget);
    RED_CHECK_IMG(gd, IMG_TEST_PATH "delegated_copy_1.png");

    delegated.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 10, 10, nullptr);

    RED_CHECK(widget.last_event == NOTIFY_COPY);
    RED_CHECK(widget.last_widget == &dummy_widget);
    RED_CHECK_IMG(gd, IMG_TEST_PATH "delegated_copy_2.png");

    delegated.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 10, 10, nullptr);

    RED_CHECK_IMG(gd, IMG_TEST_PATH "delegated_copy_1.png");
}
