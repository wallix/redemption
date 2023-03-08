/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qtclient/graphics/cursor_cache.hpp"

#include <QtGui/QPixmap>
#include <QtGui/QImage>


QCursor qtclient::CursorCache::rdp_pointer_to_cursor(const RdpPointerView& pointer)
{
    auto hotspot = pointer.hotspot();
    auto rgba_cursor = redclient::rdp_pointer_to_rgba8888(pointer);
    QImage cursor_image(
        rgba_cursor.data(),
        rgba_cursor.width,
        rgba_cursor.height,
        int(rgba_cursor.bytes_per_line()),
        QImage::Format_RGBA8888
    );

    return QCursor(QPixmap::fromImage(cursor_image), hotspot.x, hotspot.x);
}

QCursor const* qtclient::set_predefined_pointer(QCursor& out_cursor, PredefinedPointer predefined_pointer)
{
    switch (predefined_pointer) {
        case PredefinedPointer::Normal:
        case PredefinedPointer::SystemNormal:
            out_cursor.setShape(Qt::ArrowCursor);
            return &out_cursor;
        case PredefinedPointer::Edit:
            out_cursor.setShape(Qt::IBeamCursor);
            return &out_cursor;
        case PredefinedPointer::Null:
            out_cursor.setShape(Qt::BlankCursor);
            return &out_cursor;
        case PredefinedPointer::Dot:
            out_cursor.setShape(Qt::CrossCursor);
            return &out_cursor;
        case PredefinedPointer::NS:
            out_cursor.setShape(Qt::SizeVerCursor);
            return &out_cursor;
        case PredefinedPointer::NESW:
            out_cursor.setShape(Qt::SizeFDiagCursor);
            return &out_cursor;
        case PredefinedPointer::NWSE:
            out_cursor.setShape(Qt::SizeBDiagCursor);
            return &out_cursor;
        case PredefinedPointer::WE:
            out_cursor.setShape(Qt::SizeHorCursor);
            return &out_cursor;
        case PredefinedPointer::SlashedCircle:
            out_cursor.setShape(Qt::ForbiddenCursor);
            return &out_cursor;
    }
    return nullptr;
}
