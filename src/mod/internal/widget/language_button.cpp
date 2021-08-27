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
 *   Copyright (C) Wallix 2010-2015
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen,
 *              Meng Tan, Jennifer Inthavong
 */

#include "mod/internal/widget/language_button.hpp"
#include "core/front_api.hpp"
#include "keyboard/keylayouts.hpp"
#include "utils/log.hpp"
#include "utils/theme.hpp"
#include "utils/sugar/split.hpp"
#include "utils/strutils.hpp"


LanguageButton::LanguageButton(
    zstring_view enable_locales,
    Widget & parent,
    gdi::GraphicApi & drawable,
    FrontAPI & front,
    Font const & font,
    Theme const & theme
)
    : WidgetButton(drawable, *this, this, nullptr, -1,
                       theme.global.fgcolor, theme.global.bgcolor,
                       theme.global.focus_color, 2, font, 7, 7)
    , front(front)
    , parent_redraw(parent)
    , front_layout(front.get_keylayout())
{
    using std::begin;
    using std::end;

    this->locales.push_back(bool(front_layout.kbdid)
        ? Ref(this->front_layout)
        : Ref(default_layout()));

    auto const layouts = keylayouts();

    for (auto locale : split_with(enable_locales, ',')) {
        auto const name = trim(locale).as<std::string_view>();
        auto const it = std::find_if(begin(layouts), end(layouts), [&](KeyLayout const& k){
            return k.name.to_sv() == name;
        });
        if (it != end(layouts)) {
            if (it->kbdid != front_layout.kbdid) {
                this->locales.push_back(*it);
            }
        }
        else {
            LOG(LOG_WARNING, "Layout \"%.*s\" not found.",
                static_cast<int>(name.size()), name.data());
        }
    }

    this->set_text(this->locales.front().get().name);

    Dimension dim = this->get_optimal_dim();
    this->set_wh(dim);
}

void LanguageButton::notify(Widget& widget, NotifyApi::notify_event_t event)
{
    (void)widget;
    if (event == NOTIFY_SUBMIT || event == MOUSE_FLAG_BUTTON1) {
        Rect rect = this->get_rect();

        this->selected_language = (this->selected_language + 1) % this->locales.size();
        KeyLayout const& layout = this->locales[this->selected_language];
        this->set_text(layout.name);

        Dimension dim = this->get_optimal_dim();
        this->set_wh(dim);

        rect.cx = std::max(rect.cx, this->cx());
        rect.cy = std::max(rect.cy, this->cy());
        this->parent_redraw.rdp_input_invalidate(rect);

        front.set_keylayout(layout);
    }
}
