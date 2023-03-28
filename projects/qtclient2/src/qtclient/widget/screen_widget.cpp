/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qtclient/widget/screen_widget.hpp"
#include "qtclient/inputs/send_scancode.hpp"
#include "qtclient/inputs/send_mouse_event.hpp"
#include "core/callback.hpp"
#include "keyboard/keymap.hpp"
#include "utils/log.hpp"

#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>


namespace
{

// TODO remove that
const Keymap keymap(KeyLayout::null_layout());

inline void use_cached_pointer(
    QWidget& widget, gdi::CachePointerIndex cache_idx, qtclient::CursorCache cursor_cache, QCursor shaped_cursor)
{
    QCursor const* cursor;

    if (!cache_idx.is_predefined_pointer()) {
        cursor = cursor_cache.get_cursor(cache_idx.cache_index());
    }
    else {
        cursor = qtclient::set_predefined_pointer(shaped_cursor, cache_idx.as_predefined_pointer());
    }

    if (!cursor) {
        LOG(LOG_ERR, "Unknown pointer id %d (is_predefined=%d)",
            cache_idx.cache_index(), cache_idx.is_predefined_pointer());
        return;
    }

    widget.setCursor(*cursor);
}

inline void set_pointer(
    gdi::CachePointerIndex cache_idx, RdpPointerView const& pointer, qtclient::CursorCache cursor_cache)
{
    if (!cache_idx.is_predefined_pointer()) {
        cursor_cache.set_rdp_pointer(cache_idx.cache_index(), pointer);
    }
}

}

namespace qtclient
{

ScreenWidget::ScreenWidget(QPoint target)
: _rdp_input(nullptr)
, _target(target)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

void ScreenWidget::wheelEvent(QWheelEvent* event)
{
    assert(_rdp_input);
    auto p = event->angleDelta();
    send_mouse_wheel(*_rdp_input, MOUSE_FLAG_WHEEL, p.x());
    send_mouse_wheel(*_rdp_input, MOUSE_FLAG_HWHEEL, p.y());
}

void ScreenWidget::keyPressEvent(QKeyEvent* event)
{
    assert(_rdp_input);
    x11_send_scancode(*_rdp_input, kbdtypes::KbdFlags::NoFlags, keymap, event->nativeScanCode());
}

void ScreenWidget::keyReleaseEvent(QKeyEvent* event)
{
    assert(_rdp_input);
    if (!event->isAutoRepeat()) {
        x11_send_scancode(*_rdp_input, kbdtypes::KbdFlags::Release, keymap, event->nativeScanCode());
    }
}

void ScreenWidget::mousePressEvent(QMouseEvent* event)
{
    assert(_rdp_input);
    send_mouse_button(*_rdp_input, MOUSE_FLAG_DOWN, event->button(), event->x(), event->y());
}

void ScreenWidget::mouseReleaseEvent(QMouseEvent* event)
{
    assert(_rdp_input);
    send_mouse_button(*_rdp_input, 0, event->button(), event->x(), event->y());
}

void ScreenWidget::mouseMoveEvent(QMouseEvent* event)
{
    assert(_rdp_input);
    _rdp_input->rdp_input_mouse(MOUSE_FLAG_MOVE, checked_int(event->x()), checked_int(event->y()));
}

void ScreenWidget::resizeEvent(QResizeEvent* event)
{
    auto size = event->size();
    Graphics::resize(size.width(), size.height());
}

void ScreenWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.drawPixmap(event->rect().topLeft(), get_pixmap(), event->rect());
}

void ScreenWidget::new_pointer(gdi::CachePointerIndex cache_idx, RdpPointerView const& pointer)
{
    set_pointer(cache_idx, pointer, _cursor_cache);
}

void ScreenWidget::cached_pointer(gdi::CachePointerIndex cache_idx)
{
    use_cached_pointer(*this, cache_idx, _cursor_cache, _shaped_cursor);
}

}
