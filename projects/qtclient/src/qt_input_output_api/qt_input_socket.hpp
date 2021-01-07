/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Clément Moroldo, Christophe Grosjean

*/

#pragma once

#include "mod/mod_api.hpp"
#include "core/events.hpp"
#include "utils/timebase.hpp"
#include "utils/difftimeval.hpp"


#include <QtWidgets/QWidget>

#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>



class QtInputSocket : public QObject
{
REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Winconsistent-missing-override")
Q_OBJECT
REDEMPTION_DIAGNOSTIC_POP

    EventContainer& events;

    QSocketNotifier * _sckListener;
    QTimer timer;

public:
    QtInputSocket(QWidget * parent, EventContainer& events)
    : QObject(parent)
    , events(events)
    , _sckListener(nullptr)
    , timer(this)
    {
        this->timer.setSingleShot(true);
    }

    ~QtInputSocket()
    {
        this->disconnect();
    }

    void disconnect()
    {
        if (this->_sckListener != nullptr) {
            delete (this->_sckListener);
            this->_sckListener = nullptr;
        }
    }

    template<class SocketEventFunction, class TimerEventFunction>
    void start_to_listen(
        int client_sck,
        SocketEventFunction sck_event_fn,
        TimerEventFunction timer_event_fn)
    {
        this->_sckListener = new QSocketNotifier(client_sck, QSocketNotifier::Read, this);
        this->QObject::connect(
            this->_sckListener, &QSocketNotifier::activated,
            [this, fn = std::move(sck_event_fn)] {
                fn();
                this->prepare_timer_event();
            });
        this->QObject::connect(
            &this->timer, &QTimer::timeout,
            [this, fn = std::move(timer_event_fn)] {
                fn();
                this->prepare_timer_event();
            });

        this->prepare_timer_event();
    }

private:
    void prepare_timer_event()
    {
        auto tv = this->events.next_timeout();
        if (tv.tv_sec != 0 || tv.tv_usec != 0) {
            auto delay = tv - this->events.get_current_time();
            this->timer.start(std::chrono::duration_cast<std::chrono::milliseconds>(delay));
        }
    }
};
