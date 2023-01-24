/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <memory>

#include "core/RDP/RDPDrawable.hpp"
#include "gdi/resize_api.hpp"
#include "gdi/graphic_api_forwarder.hpp"


struct HeadlessGraphics final : gdi::GraphicApiForwarder<RDPDrawable>, gdi::ResizeApi
{
    static constexpr std::size_t pointer_cache_entries = gdi::CachePointerIndex::MAX_POINTER_COUNT;

    HeadlessGraphics();
    ~HeadlessGraphics();

    Drawable& drawable()
    {
        return sink.impl();
    }

    void resize(uint16_t width, uint16_t height) override
    {
        sink.resize(width, height);
    }

    void dump_png(zstring_view filename, uint16_t mouse_x, uint16_t mouse_y);

    void cached_pointer(gdi::CachePointerIndex cache_idx) override;

    void new_pointer(gdi::CachePointerIndex cache_idx, RdpPointerView const& cursor) override;

private:
    std::unique_ptr<DrawablePointer[]> pointers;
    DrawablePointer* current_pointer = nullptr;
};
