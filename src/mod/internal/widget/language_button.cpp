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
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/front_api.hpp"
#include "core/font.hpp"
#include "keyboard/keylayouts.hpp"
#include "gdi/graphic_api.hpp"
#include "utils/log.hpp"
#include "utils/theme.hpp"
#include "utils/sugar/split.hpp"
#include "utils/strutils.hpp"
#include "utils/txt2d_to_rects.hpp"


namespace
{
    constexpr auto kbd_icon_rects = TXT2D_TO_RECTS(
        // "            ##                                                  ",
        // "            ##                                                  ",
        // "            ########################################            ",
        // "                                                  ##            ",
        // "                                                  ##            ",
        "################################################################",
        "##------------------------------------------------------------##",
        "##------------------------------------------------------------##",
        "##----######----######----######----######----############----##",
        "##----######----######----######----######----############----##",
        "##----######----######----######----######----############----##",
        "##--------------------------------------------------######----##",
        "##--------------------------------------------------######----##",
        "##----############----######----######----######----######----##",
        "##----############----######----######----######----######----##",
        "##----############----######----######----######----######----##",
        "##------------------------------------------------------------##",
        "##------------------------------------------------------------##",
        "##----######----################################----######----##",
        "##----######----################################----######----##",
        "##----######----################################----######----##",
        "##------------------------------------------------------------##",
        "##------------------------------------------------------------##",
        "################################################################",
    );
    constexpr int16_t kbd_icon_cx = kbd_icon_rects.back().cx;
    constexpr int16_t kbd_icon_cy = kbd_icon_rects.back().ebottom();

    int get_space_size(Font const & font)
    {
        FontCharView c = font.glyph_or_unknown(' ');
        int w = c.width + c.incby;
        if (w <= 0) {
            w = 8;
        }
        return w;
    }

    uint16_t kbd_icon_in_space(int w)
    {
        auto spaces = static_cast<uint16_t>((kbd_icon_cx + w - 1) / w + 1);
        return std::min(spaces, uint16_t{16});
    }

    // insert space for draw icon
    struct LanguageButtonText
    {
        char text[128];

        LanguageButtonText(chars_view str, unsigned spaces)
        {
            memset(text, ' ', spaces);

            std::size_t remaining = std::size(text) - spaces - 1;
            std::size_t len = std::min(remaining, str.size());
            memcpy(text + spaces, str.data(), len);
            text[spaces + len] = '\0';
        }
    };

    constexpr uint16_t language_button_border = 2;
    constexpr uint16_t language_button_padding = 7;
}

LanguageButton::LanguageButton(
    zstring_view enable_locales,
    Widget & parent,
    gdi::GraphicApi & drawable,
    FrontAPI & front,
    Font const & font,
    Theme const & theme
)
: WidgetButton(
    drawable, *this, nullptr,
    [this] { return this->next_layout(); },
    theme.global.fgcolor, theme.global.bgcolor,
    theme.global.focus_color, language_button_border, font,
    language_button_padding, language_button_padding)
, icon_size_in_space(kbd_icon_in_space(get_space_size(font)))
, space_size(static_cast<uint16_t>(icon_size_in_space * get_space_size(font)))
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
                this->locales.emplace_back(*it);
            }
        }
        else {
            LOG(LOG_WARNING, "Layout \"%.*s\" not found.",
                static_cast<int>(name.size()), name.data());
        }
    }

    this->set_text(LanguageButtonText(this->locales.front().get().name, icon_size_in_space).text);

    Dimension dim = this->get_optimal_dim();
    this->set_wh(dim);
}

void LanguageButton::next_layout()
{
    Rect rect = this->get_rect();

    this->selected_language = (this->selected_language + 1) % this->locales.size();
    KeyLayout const& layout = this->locales[this->selected_language];
    this->set_text(LanguageButtonText(layout.name, icon_size_in_space).text);

    Dimension dim = this->get_optimal_dim();
    this->set_wh(dim);

    rect.cx = std::max(rect.cx, this->cx());
    rect.cy = std::max(rect.cy, this->cy());
    this->parent_redraw.rdp_input_invalidate(rect);

    front.set_keylayout(layout);
}

void LanguageButton::rdp_input_invalidate(Rect clip)
{
    WidgetButton::rdp_input_invalidate(clip);

    int ox = x() + language_button_border + (space_size - kbd_icon_cy - language_button_padding) / 2;
    int oy = y() + (cy() - kbd_icon_cy) / 2;

    Rect rect_intersect = clip.intersect(Rect(ox, oy, kbd_icon_cx, kbd_icon_cy));
    if (!rect_intersect.isempty()) {
        this->drawable.begin_update();
        for (auto r : kbd_icon_rects) {
            r.x += ox;
            r.y += oy;
            drawable.draw(RDPOpaqueRect(r, fg_color), rect_intersect, gdi::ColorCtx::depth24());
        }
        this->drawable.end_update();
    }
}
