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
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

#include "keyboard/keymap2.hpp"
#include "mod/internal/widget/vertical_scroll_text.hpp"


struct TestScrollCtx
{
    TestGraphic drawable;
    WidgetVerticalScrollText scroll;
    Keymap2 keymap;

    TestScrollCtx(std::string text)
    : drawable(1, 1)
    , scroll(
        this->drawable, this->scroll, nullptr, 0, std::move(text),
        /*fg_color=*/RED, /*bg_color=*/YELLOW, /*focus_color=*/WINBLUE,
        global_font_deja_vu_14(), /*xtext=*/4)
    {
        this->keymap.init_layout(0x040C);
    }

    void set_size(uint16_t w, uint16_t h, int16_t x = 0, int16_t y = 0)
    {
        this->scroll.set_xy(x, y);
        this->scroll.set_wh(w, h);
        this->drawable.resize(this->scroll.cx(), this->scroll.cy());
        this->draw();
    }

    void draw()
    {
        this->scroll.rdp_input_invalidate(this->scroll.get_rect());
    }

    void click(int x, int y)
    {
        this->scroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, x, y, nullptr);
        this->draw();
        this->scroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
    }

    void input_key(Keymap2::KEvent ev)
    {
        this->keymap.push_kevent(ev);
        this->scroll.rdp_input_scancode(0, 0, 0, 0, &this->keymap);
        this->draw();
    }
};

#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/vertical_scroll_text/"

constexpr char const* long_text =
R"(Le pastafarisme (mot-valise faisant référence aux pâtes et au mouvement rastafari) est originellement une parodie de religion1,2,3,4 dont la divinité est le Monstre en spaghetti volant (Flying Spaghetti Monster)5,6 créée en 2005 par Bobby Henderson, alors étudiant de l'université d'État de l'Oregon. Depuis, le pastafarisme a été reconnu administrativement comme religion par certains pays7,8,9,10,11, et rejeté en tant que telle par d'autres12,13,14.

Écrivant une lettre ouverte pour protester contre la décision du Comité d'Éducation de l'État du Kansas d'autoriser l'enseignement du dessein intelligent dans les cours de science au même titre que la théorie de l'évolution15, Henderson professe sa foi en un dieu créateur surnaturel dont l'apparence serait celle d'un plat de spaghetti et de boulettes de viande et demande que le pastafarisme reçoive une durée d'enseignement égale à celle du dessein intelligent et de la théorie de l'évolution.

La lettre devient rapidement un phénomène Internet. Les croyances pastafariennes sont présentées sur le site internet d'Henderson (où il se décrit comme un « prophète ») et dans L'Évangile du monstre en spaghettis volant. La croyance centrale est qu'un Monstre en spaghetti volant, invisible et indétectable, a créé l'univers. Les pirates sont vénérés comme les premiers pastafariens, et les pastafariens affirment que le constant déclin du nombre de pirates au cours des dernières années a entraîné un accroissement significatif de la température mondiale.

Cette parodie, qui peut être rapprochée de la théière de Russell ou de la Licorne rose invisible, est utilisée par des pastafariens qui s'engagent dans des parodies de disputes religieuses afin de faire valoir leur point de vue16.)";


RED_AUTO_TEST_CASE(TestWidgetVerticalScrollTextShortText)
{
    TestScrollCtx ctx("bla bla\nbla bla bla\nbla");

    ctx.set_size(300, 200);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "short1.png");

    auto dim = ctx.scroll.get_optimal_dim();
    RED_CHECK(dim.w == 78);
    RED_CHECK(dim.h == 51);
}

RED_AUTO_TEST_CASE(TestWidgetVerticalScrollTextLongText)
{
    TestScrollCtx ctx(long_text);

    ctx.set_size(300, 200);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "long1.png");

    ctx.click(290, 10);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "long2.png");

    ctx.click(290, 190);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "long3.png");

    ctx.click(290, 190);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "long4.png");

    ctx.click(290, 190);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "long5.png");

    ctx.click(290, 190);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "long6.png");

    ctx.click(290, 190);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "long7.png");

    ctx.click(290, 190);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "long7.png");

    ctx.click(290, 10);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "long6_2.png");

    ctx.click(290, 50);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "long5_2.png");

    ctx.input_key(Keymap2::KEVENT_DOWN_ARROW);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "long6_3.png");

    auto dim = ctx.scroll.get_optimal_dim();
    RED_CHECK(dim.w == 280);
    RED_CHECK(dim.h == 952);
    ctx.set_size(dim.w, dim.h);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "long_opti.png");
}
