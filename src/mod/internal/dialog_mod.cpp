/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen,
 *              Meng Tan, Jennifer Inthavong
 */

#include "configs/config.hpp"
#include "mod/internal/copy_paste.hpp"
#include "mod/internal/dialog_mod.hpp"
#include "mod/internal/widget/edit.hpp"
#include "utils/translation.hpp"

namespace
{

void init_dialog_mod(WidgetScreen& screen, Widget& dialog_widget)
{
    screen.add_widget(dialog_widget, WidgetComposite::HasFocus::Yes);
    screen.init_focus();
    screen.rdp_input_invalidate(screen.get_rect());
}

}

DialogMod::DialogMod(
    DialogModVariables vars,
    gdi::GraphicApi & drawable,
    uint16_t width, uint16_t height,
    Rect const widget_rect, const char * caption, const char * message,
    const char * cancel_text, ClientExecute & rail_client_execute,
    Font const& font, Theme const& theme
)
    : RailInternalModBase(drawable, width, height, rail_client_execute, font, theme, nullptr)
    , dialog_widget(
        drawable, widget_rect,
        cancel_text
            ? WidgetDialog::Events{
                .onsubmit = [this]{
                    this->vars.set_acl<cfg::context::accept_message>(true);
                    this->set_mod_signal(BACK_EVENT_NEXT);
                },
                .oncancel = [this]{
                    this->vars.set_acl<cfg::context::accept_message>(false);
                    this->set_mod_signal(BACK_EVENT_NEXT);
                },
            }
            : WidgetDialog::Events{
                .onsubmit = [this]{
                    this->vars.set_acl<cfg::context::display_message>(true);
                    this->set_mod_signal(BACK_EVENT_NEXT);
                },
                .oncancel = [this]{
                    this->vars.set_acl<cfg::context::display_message>(false);
                    this->set_mod_signal(BACK_EVENT_NEXT);
                },
            },
        caption, message, theme, font,
        TR(trkeys::OK, language(vars)),
        cancel_text)
    , vars(vars)
{
    init_dialog_mod(this->screen, this->dialog_widget);
}


DialogWithChallengeMod::DialogWithChallengeMod(
    DialogWithChallengeModVariables vars,
    gdi::GraphicApi & drawable,
    FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, const char * caption, const char * message,
    ClientExecute & rail_client_execute,
    Font const& font, Theme const& theme, CopyPaste& copy_paste,
    ChallengeOpt challenge
)
    : RailInternalModBase(drawable, width, height, rail_client_execute, font, theme, &copy_paste)
    , language_button(
        vars.get<cfg::client::keyboard_layout_proposals>(), this->dialog_widget,
        drawable, front, font, theme)
    , dialog_widget(
        drawable, widget_rect,
        WidgetDialogWithChallenge::Events{
            .onsubmit = [this]{
                this->vars.set_acl<cfg::context::password>(this->dialog_widget.challenge->get_text());
                this->set_mod_signal(BACK_EVENT_NEXT);
            },
            .oncancel = [this]{
                this->vars.set_acl<cfg::context::password>("");
                this->set_mod_signal(BACK_EVENT_NEXT);
            },
            .onctrl_shift = [this]{ this->language_button.next_layout(); },
        },
        caption, message,
        &this->language_button,
        TR(trkeys::OK, language(vars)),
        font, theme, copy_paste, challenge)
    , vars(vars)
{
    init_dialog_mod(this->screen, this->dialog_widget);
}


WidgetDialogWithCopyableLinkMod::WidgetDialogWithCopyableLinkMod(
    WidgetDialogWithCopyableLinkModVariables vars,
    gdi::GraphicApi & drawable,
    uint16_t width, uint16_t height,
    Rect const widget_rect, const char * caption, const char * message,
    const char * link_value, const char * link_label,
    ClientExecute & rail_client_execute,
    Font const& font, Theme const& theme, CopyPaste& copy_paste
)
    : RailInternalModBase(drawable, width, height, rail_client_execute, font, theme, &copy_paste)
    , dialog_widget(
        drawable, widget_rect,
        {
            .onsubmit = [this]{
                this->vars.set_acl<cfg::context::display_message>(true);
                this->set_mod_signal(BACK_EVENT_NEXT);
            },
            .oncancel = [this]{
                this->vars.set_acl<cfg::context::display_message>(false);
                this->set_mod_signal(BACK_EVENT_NEXT);
            }
        },
        caption, message, link_value, link_label, TR(trkeys::OK, language(vars)),
        font, theme, copy_paste)
    , vars(vars)
{
    init_dialog_mod(this->screen, this->dialog_widget);
}
