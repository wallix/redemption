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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetTooltip
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/tooltip.hpp"
#include "internal/widget2/screen.hpp"
#include "internal/widget2/label.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
# define FIXTURES_PATH
#endif
#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

struct TestDraw : DrawApi
{
    RDPDrawable gd;
    Font font;

    TestDraw(uint16_t w, uint16_t h)
    : gd(w, h)
    , font(FIXTURES_PATH "/dejavu-sans-10.fv1")
    {}

    virtual void draw(const RDPOpaqueRect& cmd, const Rect& rect)
    {
        this->gd.draw(cmd, rect);
    }

    virtual void draw(const RDPScrBlt&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPDestBlt&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPPatBlt&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPMemBlt& cmd, const Rect& rect, const Bitmap& bmp)
    {
        this->gd.draw(cmd, rect, bmp);
    }

    virtual void draw(const RDPMem3Blt& cmd, const Rect& rect, const Bitmap& bmp)
    {
        this->gd.draw(cmd, rect, bmp);
    }

    virtual void draw(const RDPLineTo&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPGlyphIndex&, const Rect&, const GlyphCache * gly_cache)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPBrushCache&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPColCache&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPGlyphCache&)
    {
        BOOST_CHECK(false);
    }

    virtual void begin_update()
    {}

    virtual void end_update()
    {}

    virtual void server_draw_text(int16_t x, int16_t y, const char* text, uint32_t fgcolor, uint32_t bgcolor, const Rect& clip)
    {
        this->gd.server_draw_text(x, y, text, fgcolor, bgcolor, clip, this->font);
    }

    virtual void text_metrics(const char* text, int& width, int& height)
    {
        height = 0;
        width = 0;
        uint32_t uni[256];
        size_t len_uni = UTF8toUnicode(reinterpret_cast<const uint8_t *>(text), uni, sizeof(uni)/sizeof(uni[0]));
        if (len_uni){
            for (size_t index = 0; index < len_uni; index++) {
                FontChar *font_item = this->gd.get_font(this->font, uni[index]);
                width += font_item->width + 2;
                height = std::max(height, font_item->height);
            }
            width -= 2;
        }
    }

    void save_to_png(const char * filename)
    {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.drawable.data, this->gd.drawable.width,
                   this->gd.drawable.height, this->gd.drawable.rowsize, true);
        fclose(file);
    }
};

BOOST_AUTO_TEST_CASE(TraceWidgetTooltip)
{
    TestDraw drawable(800, 600);

    // WidgetTooltip is a tooltip widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);

    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int16_t x = 10;
    int16_t y = 10;
    const char * tooltiptext = "testémq";

    WidgetTooltip wtooltip(drawable, x, y, parent, notifier, tooltiptext, fg_color, bg_color);

    // ask to widget to redraw
    wtooltip.rdp_input_invalidate(Rect(0, 0, 100, 100));

    drawable.save_to_png(OUTPUT_FILE_PATH "tooltip.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd8\xcd\x65\xae\x47\xc0\x71\xe1\xf4\x1f"
                   "\x30\xd1\x77\xc9\xd2\x0d\x06\x7c\xd3\xff"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap, WidgetScreen * parent, Widget2 * w)
{
    if (device_flags == MOUSE_FLAG_MOVE) {
        Widget2 * wid = parent->widget_at_pos(x, y);
        if (wid == w) {
            parent->show_tooltip("Test tooltip description", x, y);
        }
    }
    parent->rdp_input_mouse(device_flags, x, y, keymap);
};

BOOST_AUTO_TEST_CASE(TraceWidgetTooltipScreen)
{
    TestDraw drawable(800, 600);
    int x = 50;
    int y = 20;
    // WidgetTooltip is a tooltip widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    WidgetLabel label(drawable, x, y, parent, &parent, "TOOLTIPTEST");

    parent.add_widget(&label);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));

    drawable.save_to_png(OUTPUT_FILE_PATH "tooltipscreen1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x3e\x67\x39\xdc\x4c\x58\x4f\x86\xa1\x44"
                   "\xaa\x73\x99\x54\x77\xb2\x8e\xb9\xf2\x89"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    rdp_input_mouse(MOUSE_FLAG_MOVE,
                    label.centerx(), label.centery(),
                    NULL, &parent, &label);
    rdp_input_mouse(MOUSE_FLAG_MOVE,
                    label.centerx(), label.centery() + 1,
                    NULL, &parent, &label);
    parent.rdp_input_invalidate(parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "tooltipscreen2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x49\xb5\xde\x42\x9e\x83\x3a\xb3\x22\xb0"
                   "\xce\x9e\xf4\xbb\xcb\x9b\xd1\x8c\x3a\x9d"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    parent.clear();
}
