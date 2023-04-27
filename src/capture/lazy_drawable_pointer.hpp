/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "utils/drawable_pointer.hpp"
#include "core/RDP/caches/pointercache.hpp"


struct LazyDrawablePointer
{
    LazyDrawablePointer(PointerCache::SourcePointersView ptr_cache)
    : cache_idx_(PredefinedPointer::Normal)
    , ptr_cache_(ptr_cache)
    {}

    void set_position(uint16_t x, uint16_t y)
    {
        drawable_pointer_.set_position(x, y);
    }

    void set_cache_id(gdi::CachePointerIndex cache_idx)
    {
        update_pointer = true;
        cache_idx_ = cache_idx;
    }

    DrawablePointer& drawable_pointer()
    {
        if (update_pointer) {
            update_pointer = false;
            drawable_pointer_.set_cursor(ptr_cache_.pointer(cache_idx_));
        }

        return drawable_pointer_;
    }

private:
    bool update_pointer = true;
    gdi::CachePointerIndex cache_idx_;
    PointerCache::SourcePointersView ptr_cache_;
    DrawablePointer drawable_pointer_;
};
