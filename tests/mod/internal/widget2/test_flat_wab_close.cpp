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
#define BOOST_TEST_MODULE TestFlatWabClose
#include <boost/test/auto_unit_test.hpp>

#undef FIXTURES_PATH
#define FIXTURES_PATH "./tests/fixtures"
#undef SHARE_PATH
#define SHARE_PATH "./tests/fixtures"

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/flat_wab_close.hpp"
#include "internal/widget2/screen.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"


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

    virtual void draw(const RDPPatBlt& cmd, const Rect& rect)
    {
        this->gd.draw(cmd, rect);
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
                // width += font_item->incby;
                width += font_item->width + 2;
                height = std::max(height, font_item->height);
            }
            width -= 1;
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


BOOST_AUTO_TEST_CASE(TraceFlatWabClose)
{
    BOOST_CHECK(1);

    TestDraw drawable(800, 600);

    BOOST_CHECK(1);

    // FlatWabClose is a flat_wab_close widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int id = 0;

    BOOST_CHECK(1);

    try {

        FlatWabClose flat_wab_close(drawable, 800, 600, parent, notifier,
                                    "abc<br>def", id, "rec", "rec");

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close.png");

    } catch (Error & e) {
        LOG(LOG_INFO, "e=%u", e.id);
    };

    BOOST_CHECK(1);

    char message[1024];
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xc4\x46\x5c\xc2\x19\xfd\x24\x7b\x66\xa9"
    //     "\x28\x2d\x3f\xfe\xf6\x7d\x0d\x62\x25\xfa"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\xb7\x12\xc9\xbe\xea\xa1\xc3\x2f\x4e\x5b"
                   "\x8e\x81\xdc\x4f\x79\xe0\xfa\x1d\x36\xa3"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatWabClose2)
{
    TestDraw drawable(800, 600);

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    try {
    FlatWabClose flat_wab_close(drawable, 800, 600, parent, notifier,
        "Lorem ipsum dolor sit amet, consectetur<br>"
        "adipiscing elit. Nam purus lacus, luctus sit<br>"
        "amet suscipit vel, posuere quis turpis. Sed<br>"
        "venenatis rutrum sem ac posuere. Phasellus<br>"
        "feugiat dui eu mauris adipiscing sodales.<br>"
        "Mauris rutrum molestie purus, in tempor lacus<br>"
        "tincidunt et. Sed eu ligula mauris, a rutrum<br>"
        "est. Vestibulum in nunc vel massa condimentum<br>"
        "iaculis nec in arcu. Pellentesque accumsan,<br>"
        "quam sit amet aliquam mattis, odio purus<br>"
        "porttitor tortor, sit amet tincidunt odio<br>"
        "erat ut ligula. Fusce sit amet mauris neque.<br>"
        "Sed orci augue, luctus in ornare sed,<br>"
        "adipiscing et arcu."
    );

    flat_wab_close.rdp_input_invalidate(flat_wab_close.rect);
    }
    catch(Error & e) {
        LOG(LOG_INFO, "error.id=%d", e.id);
        BOOST_CHECK(false);
    };

    // ask to widget to redraw at it's current position

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close2.png");

    char message[1024];
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x6a\x67\xa0\x60\x91\xe2\xcd\x44\x4b\x33"
    //     "\x86\xc0\x99\x3b\x47\x68\x3b\x21\x80\x69"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\xcc\x30\xbf\x17\x9e\x61\x1f\xc7\x31\xac"
                   "\x70\xe8\x71\x55\x7b\x9c\xbe\x4e\x35\x1a"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatWabClose3)
{
    TestDraw drawable(800, 600);

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    FlatWabClose flat_wab_close(drawable, 800, 600, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close3.png");

    char message[1024];
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xbb\xeb\x86\x43\x8e\xe4\xa4\x1f\xd9\x83"
    //     "\x0a\xe4\x6a\xb5\x40\xc2\xa7\x24\x43\xe3"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x24\xe0\x51\xc3\xe4\x28\xcd\x2a\x83\xf9"
                   "\x27\x12\x8d\x7c\x3b\xd9\x70\x55\xf0\xab"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatWabClose4)
{
    TestDraw drawable(800, 600);

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    FlatWabClose flat_wab_close(drawable, 800, 600, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close4.png");

    char message[1024];
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xbb\xeb\x86\x43\x8e\xe4\xa4\x1f\xd9\x83"
    //     "\x0a\xe4\x6a\xb5\x40\xc2\xa7\x24\x43\xe3"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x24\xe0\x51\xc3\xe4\x28\xcd\x2a\x83\xf9"
                   "\x27\x12\x8d\x7c\x3b\xd9\x70\x55\xf0\xab"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatWabClose5)
{
    TestDraw drawable(800, 600);

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    FlatWabClose flat_wab_close(drawable, 800, 600, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close5.png");

    char message[1024];
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xbb\xeb\x86\x43\x8e\xe4\xa4\x1f\xd9\x83"
    //     "\x0a\xe4\x6a\xb5\x40\xc2\xa7\x24\x43\xe3"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x24\xe0\x51\xc3\xe4\x28\xcd\x2a\x83\xf9"
                   "\x27\x12\x8d\x7c\x3b\xd9\x70\x55\xf0\xab"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatWabClose6)
{
    TestDraw drawable(800, 600);

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    FlatWabClose flat_wab_close(drawable, 800, 600, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close6.png");

    char message[1024];
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xbb\xeb\x86\x43\x8e\xe4\xa4\x1f\xd9\x83"
    //     "\x0a\xe4\x6a\xb5\x40\xc2\xa7\x24\x43\xe3"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x24\xe0\x51\xc3\xe4\x28\xcd\x2a\x83\xf9"
                   "\x27\x12\x8d\x7c\x3b\xd9\x70\x55\xf0\xab"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatWabCloseClip)
{
    TestDraw drawable(800, 600);

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    FlatWabClose flat_wab_close(drawable, 800, 600, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_wab_close.rdp_input_invalidate(flat_wab_close.rect.offset(20,0));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close7.png");

    char message[1024];
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x52\x55\x4e\xf9\x74\x66\xef\x2b\x62\x12"
    //     "\x37\xc7\x95\xce\x24\xbe\x23\xb5\x74\xb5"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\xa0\x14\x4c\x4f\x3c\x08\x98\x7f\x62\x08"
                   "\x4b\xc6\xba\x64\x97\xe9\x9d\xc5\xac\x47"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatWabCloseClip2)
{
    TestDraw drawable(800, 600);

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    FlatWabClose flat_wab_close(drawable, 800, 600, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_wab_close.rdp_input_invalidate(Rect(20 + flat_wab_close.dx(),
                                               5 + flat_wab_close.dy(),
                                               30,
                                               10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close8.png");

    char message[1024];
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x31\x82\xdc\x89\xfd\xda\x77\xc1\xf9\xa1"
    //     "\x44\x23\xdb\xc5\x09\xae\xb9\xb7\x2b\x35"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\xe0\xb8\xb4\x24\x90\xd7\xcd\x81\xbb\x28"
                   "\xeb\x85\x43\xe5\x7f\xcd\x37\x5a\x77\x94"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}
