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
 *              Meng Tan, Jennifer Inthavong
 */

#define RED_TEST_MODULE TestFlatWait
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/flat_wait.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TraceFlatWait)
{
    TestGraphic drawable(800, 600);


    // FlatWait is a flat_dialog widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;

    // const char * text =
    //     "The target trucmuch@machinbidule:serv is invalid<br>"
    //     "please enter a valid target<br>"
    //     "You can either return to selector<br>"
    //     "or exit<br>"
    //     "Sorry for the inconvenience";

    // const char * text_timeframe =
    //     "Access to \"trucmuch@machinbidule:serv\" is not allowed because of out of timeframe.<br>"
    //     "It will be available on Oct 4 at 7:00 am.<br>"
    //     "You can either return to selector or exit.";

    const char * text_invalid =
        "Target \"trucmuch@machinbidule:serv\" is not allowed because you either<br>"
        "has no right to access it or it does not exist.<br>"
        "you can either return to selector or exit.";

    // const char * text_pending =
    //     "An approbation demand is currently pending for \"trucmuch@machinbidule:serv\".<br>"
    //     "Please wait for approbator confirmation.<br>"
    //     "Otherwise, you can either return to selector or exit.";

    // const char * text_approb =
    //     "An approbation is required for \"trucmuch@machinbidule:serv\".<br>"
    //     "Please fill following form and enter confirm to ask for an approbation.<br>"
    //     "Otherwise, you can either return to selector or exit.";


    WidgetFlatButton * extra_button = nullptr;
    FlatWait flat_dialog(drawable, 0, 0, 800, 600, parent, notifier, "Invalid Target",
                         text_invalid, 0, extra_button, global_font_deja_vu_14(), colors, Translation::EN);
    // FlatWait flat_dialog(drawable, 800, 600, parent, notifier, "Pending Approbation",
    //                      text_pending, 0, colors);
    // FlatWait flat_dialog(drawable, 800, 600, parent, notifier, "Out of Timeframe",
    //                      text_timeframe, 0, colors);
    // FlatWait flat_dialog(drawable, 800, 600, parent, notifier, "Approbation needed",
    //                      text_approb, 0, colors, true);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());

    // drawable.save_to_png("flat_wait.png");

    RED_CHECK_SIG(drawable, "\x37\x22\xba\x23\x6c\x15\x2f\x46\x0e\x0d\xec\x1e\xf3\xe5\x0b\xa7\x29\xb1\xa7\x40");
}

RED_AUTO_TEST_CASE(TraceFlatWaitWithForm)
{
    TestGraphic drawable(800, 600);


    // FlatWait is a flat_dialog widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;

    // const char * text =
    //     "The target trucmuch@machinbidule:serv is invalid<br>"
    //     "please enter a valid target<br>"
    //     "You can either return to selector<br>"
    //     "or exit<br>"
    //     "Sorry for the inconvenience";

    // const char * text_timeframe =
    //     "Access to \"trucmuch@machinbidule:serv\" is not allowed because of out of timeframe.<br>"
    //     "It will be available on Oct 4 at 7:00 am.<br>"
    //     "You can either return to selector or exit.";

    const char * text_invalid =
        "Target \"trucmuch@machinbidule:serv\" is not allowed because you either<br>"
        "has no right to access it or it does not exist.<br>"
        "you can either return to selector or exit.";

    // const char * text_pending =
    //     "An approbation demand is currently pending for \"trucmuch@machinbidule:serv\".<br>"
    //     "Please wait for approbator confirmation.<br>"
    //     "Otherwise, you can either return to selector or exit.";

    // const char * text_approb =
    //     "An approbation is required for \"trucmuch@machinbidule:serv\".<br>"
    //     "Please fill following form and enter confirm to ask for an approbation.<br>"
    //     "Otherwise, you can either return to selector or exit.";

    WidgetFlatButton * extra_button = nullptr;
    FlatWait flat_dialog(drawable, 0, 0, 800, 600, parent, notifier, "Invalid Target",
                         text_invalid, 0, extra_button, global_font_deja_vu_14(), colors, Translation::EN, true,
                         FlatForm::COMMENT_DISPLAY | FlatForm::COMMENT_MANDATORY |
                            FlatForm::TICKET_DISPLAY | FlatForm::TICKET_MANDATORY |
                            FlatForm::DURATION_DISPLAY);
    // FlatWait flat_dialog(drawable, 800, 600, parent, notifier, "Pending Approbation",
    //                      text_pending, 0, colors);
    // FlatWait flat_dialog(drawable, 800, 600, parent, notifier, "Out of Timeframe",
    //                      text_timeframe, 0, colors);
    // FlatWait flat_dialog(drawable, 800, 600, parent, notifier, "Approbation needed",
    //                      text_approb, 0, colors, true);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());

    // drawable.save_to_png("flat_wait_1.png");

    RED_CHECK_SIG(drawable, "\x83\x4a\x45\xdc\xb6\x7a\x12\x79\x14\x95\xc1\x11\x57\xc7\x05\xa2\x23\x54\xde\x92");
}


// RED_AUTO_TEST_CASE(TraceFlatWait2)
// {
//     TestGraphic drawable(800, 600);

//     // FlatWait is a flat_dialog widget of size 100x20 at position 10,100 in it's parent context
//     WidgetScreen parent(drawable, 800, 600);
//     NotifyApi * notifier = nullptr;
//     Theme colors;
//     colors.global.bgcolor = DARK_BLUE_BIS;
//     colors.global.fgcolor = WHITE;
//     FlatWait flat_dialog(drawable, 640, 480, parent, notifier, "test2",
//                            "line 1<br>"
//                            "line 2<br>"
//                            "<br>"
//                            "line 3, blah blah<br>"
//                            "line 4",
//                            0, colors);

//     // ask to widget to redraw at it's current position
//     flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
//                                       0 + flat_dialog.dy(),
//                                       flat_dialog.cx(),
//                                       flat_dialog.cy()));

//     // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog2.png");

//     char message[1024];

//     if (!check_sig(drawable.impl(), message,
//                    "\xbe\xdc\x51\xe5\x3f\x17\x87\xa2\x5e\x57"
//                    "\x57\x77\xc8\xab\x74\xd1\x44\x67\x73\x6a"
//                    )){
//         RED_CHECK_MESSAGE(false, message);
//     }

// }

// RED_AUTO_TEST_CASE(TraceFlatWait3)
// {
//     TestGraphic drawable(800, 600);

//     // FlatWait is a flat_dialog widget of size 100x20 at position -10,500 in it's parent context
//     WidgetScreen parent(drawable, 800, 600);
//     NotifyApi * notifier = nullptr;
//     Theme colors;
//     colors.global.bgcolor = DARK_BLUE_BIS;
//     colors.global.fgcolor = WHITE;
//     FlatWait flat_dialog(drawable, 1280, 1024, parent, notifier, "test3",
//                            "line 1<br>"
//                            "line 2<br>"
//                            "<br>"
//                            "line 3, blah blah<br>"
//                            "line 4",
//                            0, colors);

//     // ask to widget to redraw at it's current position
//     flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
//                                       0 + flat_dialog.dy(),
//                                       flat_dialog.cx(),
//                                       flat_dialog.cy()));

//     // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog3.png");

//     char message[1024];

//     if (!check_sig(drawable.impl(), message,
//                    "\xab\xf6\x15\x4e\xf6\x00\xcf\xb1\xba\x72"
//                    "\xc7\x45\x21\x71\xdc\x87\x99\x29\xcd\xdc"
//                    )){
//         RED_CHECK_MESSAGE(false, message);
//     }

// }

// RED_AUTO_TEST_CASE(TraceFlatWait4)
// {
//     TestGraphic drawable(1280, 1024);

//     // FlatWait is a flat_dialog widget of size 100x20 at position 770,500 in it's parent context
//     WidgetScreen parent(drawable, 1280, 1024);
//     NotifyApi * notifier = nullptr;
//     Theme colors;
//     colors.global.bgcolor = DARK_BLUE_BIS;
//     colors.global.fgcolor = WHITE;
//     FlatWait flat_dialog(drawable, 1280, 1024, parent, notifier, "test4",
//                            "line 1<br>"
//                            "line 2<br>"
//                            "<br>"
//                            "line 3, blah blah<br>"
//                            "line 4",
//                            0, colors);

//     // ask to widget to redraw at it's current position
//     flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
//                                       0 + flat_dialog.dy(),
//                                       flat_dialog.cx(),
//                                       flat_dialog.cy()));

//     // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog4.png");

//     char message[1024];

//     if (!check_sig(drawable.impl(), message,
//                    "\xa4\x3f\xae\x5e\x0c\x62\x18\x35\x12\x4e"
//                    "\x97\xb2\x9d\xf3\x0a\x21\x3b\xaa\x16\xa8"
//                    )){
//         RED_CHECK_MESSAGE(false, message);
//     }

// }

// RED_AUTO_TEST_CASE(TraceFlatWait5)
// {
//     TestGraphic drawable(640, 480);

//     // FlatWait is a flat_dialog widget of size 100x20 at position -20,-7 in it's parent context
//     WidgetScreen parent(drawable, 640, 480);
//     NotifyApi * notifier = nullptr;
//     Theme colors;
//     colors.global.bgcolor = DARK_BLUE_BIS;
//     colors.global.fgcolor = WHITE;
//     FlatWait flat_dialog(drawable, 640, 480, parent, notifier, "test5",
//                            "line 1<br>"
//                            "line 2<br>"
//                            "<br>"
//                            "line 3, blah blah<br>"
//                            "line 4",
//                            0, colors);

//     // ask to widget to redraw at it's current position
//     flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
//                                       0 + flat_dialog.dy(),
//                                       flat_dialog.cx(),
//                                       flat_dialog.cy()));

//     // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog5.png");

//     char message[1024];

//     if (!check_sig(drawable.impl(), message,
//                    "\x23\x3e\xc8\xac\x9e\x89\x32\x4a\xc7\x13"
//                    "\xb5\xe8\x55\xf9\x55\x0c\x61\xf8\x1f\x56"
//                    )){
//         RED_CHECK_MESSAGE(false, message);
//     }

// }

// RED_AUTO_TEST_CASE(TraceFlatWait6)
// {
//     TestGraphic drawable(350, 500);

//     // FlatWait is a flat_dialog widget of size 100x20 at position 760,-7 in it's parent context
//     WidgetScreen parent(drawable, 300, 600);
//     NotifyApi * notifier = nullptr;
//     Theme colors;
//     colors.global.bgcolor = DARK_BLUE_BIS;
//     colors.global.fgcolor = WHITE;
//     FlatWait flat_dialog(drawable, 350, 500, parent, notifier, "test6",
//                            "line 1<br>"
//                            "line 2<br>"
//                            "<br>"
//                            "line 3, blah blah<br>"
//                            "line 4",
//                            0, colors);

//     // ask to widget to redraw at it's current position
//     flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
//                                       0 + flat_dialog.dy(),
//                                       flat_dialog.cx(),
//                                       flat_dialog.cy()));

//     // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog6.png");

//     char message[1024];

//     if (!check_sig(drawable.impl(), message,
//                    "\x60\x53\x42\x70\xfb\xc5\x95\xd8\xa7\xef"
//                    "\x07\xe8\x5d\xab\xbc\xb1\xf7\x7a\xb6\x6e"
//     )){
//         RED_CHECK_MESSAGE(false, message);
//     }


// }

// RED_AUTO_TEST_CASE(TraceFlatWaitClip)
// {
//     TestGraphic drawable(800, 600);

//     // FlatWait is a flat_dialog widget of size 100x20 at position 760,-7 in it's parent context
//     WidgetScreen parent(drawable, 800, 600);
//     NotifyApi * notifier = nullptr;
//     Theme colors;
//     colors.global.bgcolor = DARK_BLUE_BIS;
//     colors.global.fgcolor = WHITE;
//     FlatWait flat_dialog(drawable, 300, 600, parent, notifier, "test6",
//                            "line 1<br>"
//                            "line 2<br>"
//                            "<br>"
//                            "line 3, blah blah<br>"
//                            "line 4",
//                            0, colors);

//     // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
//     flat_dialog.rdp_input_invalidate(Rect(20 + flat_dialog.dx(),
//                                       0 + flat_dialog.dy(),
//                                       flat_dialog.cx(),
//                                       flat_dialog.cy()));

//     // drawable.save_to_png("flat_dialog7.png");

//     char message[1024];

//     if (!check_sig(drawable.impl(), message,
//                    "\x6f\x5e\xaa\x7b\xaa\x85\x86\x9a\x32\x6c"
//                    "\xd3\x2e\xa2\x2b\xde\x76\x35\xd6\x3b\x50"
//     )){
//         RED_CHECK_MESSAGE(false, message);
//     }

// }

// RED_AUTO_TEST_CASE(TraceFlatWaitClip2)
// {
//     TestGraphic drawable(800, 600);

//     // FlatWait is a flat_dialog widget of size 100x20 at position 10,7 in it's parent context
//     WidgetScreen parent(drawable, 800, 600);
//     NotifyApi * notifier = nullptr;
//     Theme colors;
//     colors.global.bgcolor = DARK_BLUE_BIS;
//     colors.global.fgcolor = WHITE;
//     FlatWait flat_dialog(drawable, 800, 600, parent, notifier, "test6",
//                            "line 1<br>"
//                            "line 2<br>"
//                            "<br>"
//                            "line 3, blah blah<br>"
//                            "line 4",
//                            0, colors);

//     // ask to widget to redraw at position 30,12 and of size 30x10.
//     flat_dialog.rdp_input_invalidate(Rect(20 + flat_dialog.dx(),
//                                       5 + flat_dialog.dy(),
//                                       30,
//                                       10));

//     // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog8.png");

//     char message[1024];

//     if (!check_sig(drawable.impl(), message,
//                    "\xb1\x4f\x44\xde\x94\x93\x7c\x72\x48\xc1"
//                    "\x15\x50\x5f\x45\xa5\xb4\xcf\x9f\x21\x73"
//     )){
//         RED_CHECK_MESSAGE(false, message);
//     }

// }

// RED_AUTO_TEST_CASE(EventWidgetOkCancel)
// {
//     ClientInfo info(1, true, true);
//     info.keylayout = 0x040C;
//     info.console_session = 0;
//     info.brush_cache_code = 0;
//     info.bpp = 24;
//     info.width = 800;
//     info.height = 600;

//     TestGraphic drawable(800, 600);

//     WidgetScreen parent(drawable, 800, 600);
//     struct Notify : NotifyApi {
//         Widget2* sender;
//         notify_event_t event;

//         Notify()
//         : sender(0)
//         , event(0)
//         {}

//         virtual void notify(Widget2* sender, notify_event_t event)
//         {
//             this->sender = sender;
//             this->event = event;
//         }
//     } notifier;
//     Theme colors;
//     colors.global.bgcolor = DARK_BLUE_BIS;
//     colors.global.fgcolor = WHITE;
//     FlatWait flat_dialog(drawable, 800, 600, parent, &notifier, "test6",
//                            "line 1<br>"
//                            "line 2<br>"
//                            "<br>"
//                            "line 3, blah blah<br>"
//                            "line 4",
//                            0, colors);

// //    RED_CHECK(notifier.sender == 0);
//     RED_CHECK(notifier.sender == &flat_dialog);
//     RED_CHECK(notifier.event == 0);

//     unsigned x = flat_dialog.ok.rect.x + flat_dialog.ok.rect.cx / 2 ;
//     unsigned y = flat_dialog.ok.rect.y + flat_dialog.ok.rect.cy / 2 ;
//     flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
//     // flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
// //    RED_CHECK(notifier.sender == 0);
//     RED_CHECK(notifier.sender == &flat_dialog);
//     RED_CHECK(notifier.event == 0);

//     flat_dialog.rdp_input_invalidate(flat_dialog.rect);
//     // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-1-button-ok.png");

//     char message[1024];

//     if (!check_sig(drawable.impl(), message,
//                    "\x09\x03\x4f\xf6\x6b\xbf\xe2\x9c\xf0\xee"
//                    "\x89\x5c\x88\x5b\x63\xe8\x39\x4e\xe4\xef"
//     )){
//         RED_CHECK_MESSAGE(false, message);
//     }



//     flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
//     // flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
//     //                                  flat_dialog.ok.dx(), flat_dialog.ok.dy(), nullptr);
//     RED_CHECK(notifier.sender == &flat_dialog);
//     RED_CHECK(notifier.event == NOTIFY_SUBMIT);
//     notifier.sender = 0;
//     notifier.event = 0;

//     flat_dialog.rdp_input_invalidate(flat_dialog.rect);
//     // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-2-button-ok.png");


//     if (!check_sig(drawable.impl(), message,
//                    "\xed\xa6\x0c\x8e\xbc\xab\xdf\x81\x63\xd9"
//                    "\xaf\x19\x24\x80\x7e\x65\x98\xe9\x94\xab"
//     )){
//         RED_CHECK_MESSAGE(false, message);
//     }


//     x = flat_dialog.cancel->rect.x + flat_dialog.cancel->rect.cx / 2 ;
//     y = flat_dialog.cancel->rect.y + flat_dialog.cancel->rect.cy / 2 ;
//     flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
//     // flat_dialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
// //    RED_CHECK(notifier.sender == 0);
//     RED_CHECK(notifier.sender == &flat_dialog);
//     RED_CHECK(notifier.event == 0);


//     flat_dialog.rdp_input_invalidate(flat_dialog.rect);
//     // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-3-button-cancel.png");



//     if (!check_sig(drawable.impl(), message,
//                    "\xee\x26\x6b\x3c\x2b\x33\x01\x18\xf5\xca"
//                    "\x87\x41\xd2\xce\x00\x17\xf9\x2f\xbd\xb2"
//     )){
//         RED_CHECK_MESSAGE(false, message);
//     }


//     flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
//     RED_CHECK(notifier.sender == &flat_dialog);
//     RED_CHECK(notifier.event == NOTIFY_CANCEL);

//     notifier.sender = 0;
//     notifier.event = 0;

//     Keymap2 keymap;
//     keymap.init_layout(info.keylayout);
//     keymap.push_kevent(Keymap2::KEVENT_ESC);
//     flat_dialog.rdp_input_scancode(0, 0, 0, 0, &keymap);
//     RED_CHECK(notifier.sender == &flat_dialog);
//     RED_CHECK(notifier.event == NOTIFY_CANCEL);
// }



// RED_AUTO_TEST_CASE(EventWidgetChallenge)
// {
//     ClientInfo info(1, true, true);
//     info.keylayout = 0x040C;
//     info.console_session = 0;
//     info.brush_cache_code = 0;
//     info.bpp = 24;
//     info.width = 800;
//     info.height = 600;

//     TestGraphic drawable(800, 600);

//     WidgetScreen parent(drawable, 800, 600);
//     struct Notify : NotifyApi {
//         Widget2* sender;
//         notify_event_t event;

//         Notify()
//         : sender(0)
//         , event(0)
//         {}

//         virtual void notify(Widget2* sender, notify_event_t event)
//         {
//             this->sender = sender;
//             this->event = event;
//         }
//     } notifier;
//     Theme colors;
//     colors.global.bgcolor = DARK_BLUE_BIS;
//     colors.global.fgcolor = WHITE;
//     FlatWait flat_dialog(drawable, 800, 600, parent, &notifier, "test6",
//                            "Lorem ipsum dolor sit amet, consectetur<br>"
//                            "adipiscing elit. Nam purus lacus, luctus sit<br>"
//                            "amet suscipit vel, posuere quis turpis. Sed<br>"
//                            "venenatis rutrum sem ac posuere. Phasellus<br>"
//                            "feugiat dui eu mauris adipiscing sodales.<br>"
//                            "Mauris rutrum molestie purus, in tempor lacus<br>"
//                            "tincidunt et. Sed eu ligula mauris, a rutrum<br>"
//                            "est. Vestibulum in nunc vel massa condimentum<br>"
//                            "iaculis nec in arcu. Pellentesque accumsan,<br>"
//                            "quam sit amet aliquam mattis, odio purus<br>"
//                            "porttitor tortor, sit amet tincidunt odio<br>"
//                            "erat ut ligula. Fusce sit amet mauris neque.<br>"
//                            "Sed orci augue, luctus in ornare sed,<br>"
//                            "adipiscing et arcu.", 0, colors,
//                            "Ok", "Cancel", CHALLENGE_ECHO);

// //    RED_CHECK(notifier.sender == 0);
//     RED_CHECK(notifier.sender == &flat_dialog);
//     RED_CHECK(notifier.event == 0);


//     flat_dialog.challenge->set_text("challenge_test");

// //    RED_CHECK(notifier.sender == 0);
//     RED_CHECK(notifier.sender == &flat_dialog);
//     RED_CHECK(notifier.event == 0);

//     flat_dialog.rdp_input_invalidate(flat_dialog.rect);
//     // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-challenge-1.png");

//     char message[1024];

//     if (!check_sig(drawable.impl(), message,
//                    "\xaa\x65\x78\x57\x46\xa8\x97\x24\xf1\xd6"
//                    "\xc2\xff\x28\x92\xbe\xc8\x46\x0c\xbe\x01"
//     )){
//         RED_CHECK_MESSAGE(false, message);
//     }

//     Keymap2 keymap;
//     keymap.init_layout(info.keylayout);
//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     flat_dialog.rdp_input_scancode(0, 0, 0, 0, &keymap);
//     RED_CHECK(notifier.sender == &flat_dialog);
//     RED_CHECK(notifier.event == NOTIFY_SUBMIT);

//     notifier.sender = 0;
//     notifier.event = 0;

// }
