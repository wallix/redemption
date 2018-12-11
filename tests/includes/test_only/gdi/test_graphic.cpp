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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#include "test_only/gdi/test_graphic.hpp"

#include "core/RDP/RDPDrawable.hpp"

struct TestGraphic::D
{
    RDPDrawable gd;
};

TestGraphic::TestGraphic(uint16_t w, uint16_t h)
  : d(new D{{w, h}})
{}

TestGraphic::~TestGraphic() = default;

uint16_t TestGraphic::width() const
{
    return this->d->gd.width();
}

uint16_t TestGraphic::height() const
{
    return this->d->gd.height();
}

TestGraphic::operator ConstImageDataView() const
{
    return this->d->gd;
}

TestGraphic::operator gdi::GraphicApi&()
{
    return this->d->gd;
}

TestGraphic::operator gdi::ImageFrameApi&()
{
    return this->d->gd;
}

gdi::GraphicApi* TestGraphic::operator->()
{
    return &this->d->gd;
}

void TestGraphic::resize(uint16_t w, uint16_t h)
{
    this->d->gd.resize(w, h);
}

void TestGraphic::trace_mouse()
{
    this->d->gd.trace_mouse();
}

void TestGraphic::clear_mouse()
{
    this->d->gd.clear_mouse();
}

void TestGraphic::show_mouse_cursor(bool display)
{
    this->d->gd.show_mouse_cursor(display);
}

void TestGraphic::set_mouse_cursor_pos(int x, int y)
{
    this->d->gd.set_mouse_cursor_pos(x, y);
}

MutableImageDataView TestGraphic::get_mutable_image_view()
{
    return gdi::get_mutable_image_view(this->d->gd);
}
