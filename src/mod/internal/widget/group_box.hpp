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
    Author(s): Christophe Grosjean, Meng Tan, Raphael Zhou
*/

#pragma once

#include "mod/internal/widget/composite.hpp"
#include "mod/internal/widget/language_button.hpp"

class Font;

class WidgetGroupBox : public WidgetParent
{
public:
    WidgetGroupBox( gdi::GraphicApi & drawable, Widget & parent
                  , NotifyApi * notifier, const char * text
                  , BGRColor fgcolor, BGRColor bgcolor, Font const & font);

    ~WidgetGroupBox() override;

    void rdp_input_invalidate(Rect clip) override;

    BGRColor get_bg_color() const override;

    const char * get_text() const;
    void set_text(const char * text);

private:
    static const size_t buffer_size = 256;

    char buffer[buffer_size];

    BGRColor bg_color;
    BGRColor fg_color;

    CompositeArray composite_array;

    Font const & font;
};
