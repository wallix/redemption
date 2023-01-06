/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Meng Tan, Jonathan Poelen, Raphael Zhou
*/

#include "core/front_api.hpp"
#include "gdi/graphic_api.hpp"
#include "mod/internal/copy_paste.hpp"
#include "mod/internal/widget_test_mod.hpp"
#include "mod/internal/widget/label.hpp"
#include "mod/internal/widget/delegated_copy.hpp"
#include "mod/internal/widget/screen.hpp"
#include "utils/theme.hpp"


// Pimpl
struct WidgetTestMod::WidgetTestModPrivate
{
    WidgetTestModPrivate(
        uint16_t width, uint16_t height, gdi::GraphicApi & gd, EventContainer & /*events*/,
        FrontAPI & front, Font const& font, Theme const & theme)
    : gd(gd)
    , front(front)
    // , events_guard(events)
    , screen(gd, width, height, font, theme)
    , copy_paste(true)
    , label(gd, this->screen, "bla bla", theme.global.fgcolor, theme.global.bgcolor, font)
    , delegated_copy(
        gd, this->label, WidgetEventNotifier(), theme.global.fgcolor, theme.global.bgcolor,
        theme.global.focus_color, font, 2, 2, WidgetDelegatedCopy::MouseButton::Both)
    {
        this->screen.add_widget(&this->label);
        this->screen.add_widget(&this->delegated_copy);

        auto dim1 = this->label.get_optimal_dim();
        auto dim2 = this->delegated_copy.get_optimal_dim();
        this->label.set_xy(40, 10);
        this->label.set_wh(dim1);
        this->delegated_copy.set_xy(10, 10);
        this->delegated_copy.set_wh(dim2);

        this->screen.rdp_input_invalidate(this->screen.get_rect());
    }

    gdi::GraphicApi & gd;
    FrontAPI & front;
    // EventRef timer;
    // EventsGuard events_guard;
    WidgetScreen screen;
    CopyPaste copy_paste;
    WidgetLabel label;
    WidgetDelegatedCopy delegated_copy;
};

WidgetTestMod::WidgetTestMod(
    gdi::GraphicApi & gd,
    EventContainer & events,
    FrontAPI & front, uint16_t width, uint16_t height,
    Font const & font, Theme const & theme)
: d(std::make_unique<WidgetTestModPrivate>(width, height, gd, events, front, font, theme))
{
    (void)front.server_resize({width, height, BitsPerPixel{8}});
}

void WidgetTestMod::init()
{
    this->d->copy_paste.ready(this->d->front);
}

WidgetTestMod::~WidgetTestMod() = default;

void WidgetTestMod::rdp_input_invalidate(Rect clip)
{
    this->d->screen.rdp_input_invalidate(clip);
}

void WidgetTestMod::rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y)
{
    this->d->screen.rdp_input_mouse(device_flags, x, y);
}

void WidgetTestMod::rdp_input_scancode(
    KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap)
{
    if (pressed_scancode(flags, scancode) == Scancode::Esc) {
        this->set_mod_signal(BACK_EVENT_STOP);
    }
    else {
        this->d->screen.rdp_input_scancode(flags, scancode, event_time, keymap);
    }
}

void WidgetTestMod::rdp_input_unicode(KbdFlags flag, uint16_t unicode)
{
    (void)flag;
    (void)unicode;
}

void WidgetTestMod::rdp_input_synchronize(KeyLocks locks)
{
    (void)locks;
}

void WidgetTestMod::send_to_mod_channel(
    CHANNELS::ChannelNameId front_channel_name,
    InStream & chunk, std::size_t /*length*/, uint32_t flags)
{
    if (this->d->copy_paste && front_channel_name == CHANNELS::channel_names::cliprdr) {
        this->d->copy_paste.send_to_mod_channel(chunk, flags);
    }
}
