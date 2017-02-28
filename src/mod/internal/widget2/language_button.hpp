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

#pragma once

#include "utils/sugar/splitter.hpp"
#include "flat_button.hpp"

#include "utils/sugar/algostring.hpp"
#include "gdi/graphic_api.hpp"

class LanguageButton : public WidgetFlatButton
{
    static constexpr size_t locale_name_len = 5;
    struct Loc {
        char const * locale_name;
        int LCID;
    };
    std::vector<Loc> locales;
    unsigned selected_language = 0;
    FrontAPI & front;
    Widget2 & parent;

    public:
    LanguageButton(
            std::string const & enable_locales,
            Widget2 & parent,
            gdi::GraphicApi & drawable,
            FrontAPI & front,
            Font const & font,
            Theme const & theme
        )
        : WidgetFlatButton(drawable, *this, this, nullptr, -1,
                           theme.global.fgcolor.to_u32(), theme.global.bgcolor.to_u32(),
                           theme.global.focus_color.to_u32(), 2, font, 7, 7)
        , front(front)
        , parent(parent)
    {
        using std::begin;
        using std::end;

        auto LCID = front.get_keylayout();

        {
            auto it = std::find_if(begin(keylayouts), end(keylayouts), [&](Keylayout const * k){
                return k->LCID == LCID;
            });
            if (it == end(keylayouts)) {
                LCID = keylayout_x00000409.LCID;
                this->locales.push_back({keylayout_x00000409.locale_name, keylayout_x00000409.LCID});
            }
            else {
                this->locales.push_back({(*it)->locale_name, (*it)->LCID});
            }
        }


        for (auto && r : get_split(enable_locales, ',')) {
            auto const trimmed_range = trim(r);
            auto cstr = begin(trimmed_range).base();
            auto cend = end(trimmed_range).base();

            auto it = std::find_if(begin(keylayouts), end(keylayouts), [&](Keylayout const * k){
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

    void notify(Widget2* widget, NotifyApi::notify_event_t event) override {
        (void)widget;
        if (event == NOTIFY_SUBMIT || event == MOUSE_FLAG_BUTTON1) {
            Rect rect = this->get_rect();

            this->selected_language = (this->selected_language + 1) % this->locales.size();
            this->set_text(this->locales[this->selected_language].locale_name);

            Dimension dim = this->get_optimal_dim();
            this->set_wh(dim);

            rect.cx = std::max(rect.cx, this->cx());
            rect.cy = std::max(rect.cy, this->cy());
            this->parent.rdp_input_invalidate(rect);

            front.set_keylayout(this->locales[this->selected_language].LCID);
        }
    }
};
