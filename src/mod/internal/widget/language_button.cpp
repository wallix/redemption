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
#include "keyboard/keymap2.hpp"
#include "utils/log.hpp"
#include "utils/theme.hpp"
#include "utils/sugar/splitter.hpp"
#include "utils/sugar/algostring.hpp"

LanguageButton::LanguageButton(
    std::string const & enable_locales,
    Widget & parent,
    gdi::GraphicApi & drawable,
    FrontAPI & front,
    Font const & font,
    Theme const & theme
)
    : WidgetFlatButton(drawable, *this, this, nullptr, -1,
                       theme.global.fgcolor, theme.global.bgcolor,
                       theme.global.focus_color, 2, font, 7, 7)
    , front(front)
    , parent_redraw(parent)
{
    using std::begin;
    using std::end;

    auto LCID = front.get_keylayout();

    {
        auto const keylayouts = Keymap2::keylayouts();
        auto const it = std::find_if(begin(keylayouts), end(keylayouts), [&](Keylayout const * k){
            return k->LCID == LCID;
        });
        if (it == end(keylayouts)) {
            auto & default_layout = Keymap2::default_layout();
            LCID = default_layout.LCID;
            this->locales.push_back({default_layout.locale_name, default_layout.LCID});
        }
        else {
            this->locales.push_back({(*it)->locale_name, (*it)->LCID});
        }
    }


    for (auto && r : get_split(enable_locales, ',')) {
        auto const trimmed_range = trim(r);
        auto cstr = begin(trimmed_range).base();
        auto cend = end(trimmed_range).base();

        auto const keylayouts = Keymap2::keylayouts();
        auto const it = std::find_if(begin(keylayouts), end(keylayouts), [&](Keylayout const * k){
            return strncmp(k->locale_name, cstr, cend-cstr) == 0;
        });
        if (it != end(keylayouts)) {
            if ((*it)->LCID != LCID) {
                this->locales.push_back({(*it)->locale_name, (*it)->LCID});
            }
        }
        else {
            LOG(LOG_WARNING, "Layout \"%.*s\" not found.", static_cast<int>(cend - cstr), cstr);
        }
    }

    this->set_text(this->locales[0].locale_name);

    Dimension dim = this->get_optimal_dim();
    this->set_wh(dim);
}

void LanguageButton::notify(Widget* widget, NotifyApi::notify_event_t event)
{
    (void)widget;
    if (event == NOTIFY_SUBMIT || event == MOUSE_FLAG_BUTTON1) {
        Rect rect = this->get_rect();

        this->selected_language = (this->selected_language + 1) % this->locales.size();
        this->set_text(this->locales[this->selected_language].locale_name);

        Dimension dim = this->get_optimal_dim();
        this->set_wh(dim);

        rect.cx = std::max(rect.cx, this->cx());
        rect.cy = std::max(rect.cy, this->cy());
        this->parent_redraw.rdp_input_invalidate(rect);

        front.set_keylayout(this->locales[this->selected_language].LCID);
    }
}
