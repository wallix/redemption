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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan, Jennifer Inthavong
 */

#pragma once

#include "mod/internal/widget/composite.hpp"
#include "mod/internal/widget/button.hpp"
#include "mod/internal/widget/image.hpp"
#include "mod/internal/widget/label.hpp"
#include "mod/internal/widget/vertical_scroll_text.hpp"
#include "mod/internal/widget/delegated_copy.hpp"
#include "mod/internal/widget/widget_rect.hpp"

class WidgetEdit;
class Theme;
class CopyPaste;

class WidgetDialogBase : public WidgetComposite
{
public:
    struct WidgetLink
    {
        WidgetLabel label;
        WidgetVerticalScrollText show;
        WidgetDelegatedCopy copy;
    };

    struct Events
    {
        WidgetEventNotifier onsubmit;
        WidgetEventNotifier oncancel;
        WidgetEventNotifier onctrl_shift = {};
    };

    WidgetDialogBase(
        gdi::GraphicApi & drawable, Rect widget_rect, Events events,
        const char* caption, const char * text, WidgetButton * extra_button,
        Theme const & theme, Font const & font, const char * ok_text,
        std::unique_ptr<WidgetButton> cancel,
        std::unique_ptr<WidgetEdit> challenge,
        WidgetLink* link);

    ~WidgetDialogBase();

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height);

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override;

private:
    WidgetEventNotifier onctrl_shift;

    WidgetLabel        title;
    WidgetRect         separator;
    WidgetVerticalScrollText dialog;

// TODO private
public:
    WidgetLink* link;
    std::unique_ptr<WidgetEdit> challenge;

    WidgetButton   ok;

public:
    std::unique_ptr<WidgetButton> cancel;

private:
    WidgetImage         img;
    WidgetButton*       extra_button;
    WidgetEventNotifier oncancel;
};


class WidgetDialog : public WidgetDialogBase
{
public:
    struct Events
    {
        WidgetEventNotifier onsubmit;
        WidgetEventNotifier oncancel;
    };

    WidgetDialog(
        gdi::GraphicApi& drawable, Rect widget_rect, Events events,
        const char* caption, const char* text,
        Theme const& theme, Font const& font,
        const char* ok_text, const char* cancel_text);
};


class WidgetDialogWithChallenge : public WidgetDialogBase
{
public:
    using Events = WidgetDialogBase::Events;

    enum class ChallengeOpt { Echo, Hide };

    WidgetDialogWithChallenge(
        gdi::GraphicApi & drawable, Rect widget_rect, Events events,
        const char* caption, const char * text,
        WidgetButton * extra_button,
        const char * ok_text,
        Font const & font, Theme const & theme, CopyPaste & copy_paste,
        ChallengeOpt challenge);
};


class WidgetDialogWithCopyableLink : private WidgetDialogBase::WidgetLink, public WidgetDialogBase
{
public:
    using Events = WidgetDialog::Events;

    WidgetDialogWithCopyableLink(
        gdi::GraphicApi & drawable, Rect widget_rect, Events events,
        const char* caption, const char * text,
        const char * link_value, const char * link_label,
        const char * ok_text,
        Font const & font, Theme const & theme, CopyPaste & copy_paste);

private:
    std::string link_value;
    CopyPaste & copy_paste;
};
