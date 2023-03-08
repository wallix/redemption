/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "gdi/graphic_api.hpp" // gdi::CachePointerIndex
#include "core/RDP/rdp_pointer.hpp"
#include "client/common/rdp_pointer_to_rgba8888.hpp"

#include <array>

#include <QtGui/QCursor>


namespace qtclient
{

class CursorCache
{
public:
    bool set_rdp_pointer(uint16_t idx, RdpPointerView const& pointer)
    {
        if (REDEMPTION_UNLIKELY(idx >= pointer_cache.size())) {
            return false;
        }

        this->pointer_cache[idx] = rdp_pointer_to_cursor(pointer);
        return true;
    }

    QCursor const* get_cursor(uint16_t idx) const
    {
        if (REDEMPTION_UNLIKELY(idx >= pointer_cache.size())) {
            return nullptr;
        }

        return &this->pointer_cache[idx];
    }

private:
    QCursor rdp_pointer_to_cursor(RdpPointerView const& pointer);

    std::array<QCursor, gdi::CachePointerIndex::MAX_POINTER_COUNT> pointer_cache;
};


/// \return \p cursor when transformation is possible, otherwise nullptr
QCursor const* set_predefined_pointer(QCursor& out_cursor, PredefinedPointer predefined_pointer);

}
