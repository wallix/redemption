/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "headlessclient/headless_graphics.hpp"
#include "utils/drawable_pointer.hpp"
#include "utils/png.hpp"


HeadlessGraphics::HeadlessGraphics()
: gdi::GraphicApiForwarder<RDPDrawable>(uint16_t(1), uint16_t(1))
, pointers(new DrawablePointer[pointer_cache_entries + 1u])
{
    pointers[0].set_cursor(normal_pointer());
}

HeadlessGraphics::~HeadlessGraphics() = default;

void HeadlessGraphics::dump_png(zstring_view filename, uint16_t mouse_x, uint16_t mouse_y)
{
    DrawablePointer::BufferSaver buffer;
    auto& gd = sink.impl();

    if (current_pointer) {
        current_pointer->set_position(mouse_x, mouse_y);
        current_pointer->trace_mouse(gd, buffer);
    }

    dump_png24(filename, gd, true);

    if (current_pointer) {
        current_pointer->clear_mouse(gd, buffer);
    }
}

void HeadlessGraphics::cached_pointer(gdi::CachePointerIndex cache_idx)
{
    if (!cache_idx.is_predefined_pointer()) {
        auto idx = cache_idx.cache_index();
        if (idx < pointer_cache_entries) {
            current_pointer = &pointers[idx + 1u];
        }
    }
    else {
        if (cache_idx.as_predefined_pointer() == PredefinedPointer::SystemNormal) {
            current_pointer = &pointers[0];
        }
        else if (cache_idx.as_predefined_pointer() == PredefinedPointer::Null) {
            current_pointer = nullptr;
        }
    }
}

void HeadlessGraphics::new_pointer(gdi::CachePointerIndex cache_idx, RdpPointerView const& cursor)
{
    assert(!cache_idx.is_predefined_pointer());
    auto idx = cache_idx.cache_index();
    if (idx < pointer_cache_entries) {
        pointers[idx + 1u].set_cursor(cursor);
    }
}
