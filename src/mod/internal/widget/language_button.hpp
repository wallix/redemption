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

#include "mod/internal/widget/flat_button.hpp"

#include <vector>

class FrontAPI;
class Theme;

class LanguageButton : public WidgetFlatButton
{
public:
    LanguageButton(
        std::string const & enable_locales,
        Widget & parent,
        gdi::GraphicApi & drawable,
        FrontAPI & front,
        Font const & font,
        Theme const & theme
    );

    void notify(Widget* widget, NotifyApi::notify_event_t event) override;

private:
    struct Loc
    {
        char const * locale_name;
        int LCID;
    };
    std::vector<Loc> locales;
    unsigned selected_language = 0;
    FrontAPI & front;
    Widget & parent_redraw;
};
