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

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/flat_wait.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/test_framework/img_sig.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TraceFlatWait)
{
    TestGraphic drawable(800, 600);


    // FlatWait is a flat_dialog widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;

    // const char * text =
    //     "The target trucmuch@machinbidule:serv is invalid\n"
    //     "please enter a valid target\n"
    //     "You can either return to selector\n"
    //     "or exit\n"
    //     "Sorry for the inconvenience";

    // const char * text_timeframe =
    //     "Access to \"trucmuch@machinbidule:serv\" is not allowed because of out of timeframe.\n"
    //     "It will be available on Oct 4 at 7:00 am.\n"
    //     "You can either return to selector or exit.";

    const char * text_invalid =
        "Target \"trucmuch@machinbidule:serv\" is not allowed because you either\n"
        "has no right to access it or it does not exist.\n"
        "you can either return to selector or exit.";

    // const char * text_pending =
    //     "An approbation demand is currently pending for \"trucmuch@machinbidule:serv\".\n"
    //     "Please wait for approbator confirmation.\n"
    //     "Otherwise, you can either return to selector or exit.";

    // const char * text_approb =
    //     "An approbation is required for \"trucmuch@machinbidule:serv\".\n"
    //     "Please fill following form and enter confirm to ask for an approbation.\n"
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

    RED_CHECK_IMG_SIG(drawable, "\x37\x22\xba\x23\x6c\x15\x2f\x46\x0e\x0d\xec\x1e\xf3\xe5\x0b\xa7\x29\xb1\xa7\x40");
}

RED_AUTO_TEST_CASE(TraceFlatWaitWithForm)
{
    TestGraphic drawable(800, 600);


    // FlatWait is a flat_dialog widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;

    // const char * text =
    //     "The target trucmuch@machinbidule:serv is invalid\n"
    //     "please enter a valid target\n"
    //     "You can either return to selector\n"
    //     "or exit\n"
    //     "Sorry for the inconvenience";

    // const char * text_timeframe =
    //     "Access to \"trucmuch@machinbidule:serv\" is not allowed because of out of timeframe.\n"
    //     "It will be available on Oct 4 at 7:00 am.\n"
    //     "You can either return to selector or exit.";

    const char * text_invalid =
        "Target \"trucmuch@machinbidule:serv\" is not allowed because you either\n"
        "has no right to access it or it does not exist.\n"
        "you can either return to selector or exit.";

    // const char * text_pending =
    //     "An approbation demand is currently pending for \"trucmuch@machinbidule:serv\".\n"
    //     "Please wait for approbator confirmation.\n"
    //     "Otherwise, you can either return to selector or exit.";

    // const char * text_approb =
    //     "An approbation is required for \"trucmuch@machinbidule:serv\".\n"
    //     "Please fill following form and enter confirm to ask for an approbation.\n"
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

    RED_CHECK_IMG_SIG(drawable, "\x83\x4a\x45\xdc\xb6\x7a\x12\x79\x14\x95\xc1\x11\x57\xc7\x05\xa2\x23\x54\xde\x92");
}
