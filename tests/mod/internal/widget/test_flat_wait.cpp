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
#include "test_only/test_framework/check_img.hpp"

#include "mod/internal/widget/flat_wait.hpp"
#include "mod/internal/widget/screen.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/wait/"

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
                         text_invalid, 0, extra_button, global_font_deja_vu_14(), colors, Language::en);
    // FlatWait flat_dialog(drawable, 800, 600, parent, notifier, "Pending Approbation",
    //                      text_pending, 0, colors);
    // FlatWait flat_dialog(drawable, 800, 600, parent, notifier, "Out of Timeframe",
    //                      text_timeframe, 0, colors);
    // FlatWait flat_dialog(drawable, 800, 600, parent, notifier, "Approbation needed",
    //                      text_approb, 0, colors, true);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "wait_1.png");
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
                         text_invalid, 0, extra_button, global_font_deja_vu_14(), colors, Language::en, true,
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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "wait_2.png");
}
