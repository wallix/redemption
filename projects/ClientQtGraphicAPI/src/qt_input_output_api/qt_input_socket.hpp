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

#include "client_redemption/client_redemption_api.hpp"
#include "core/events.hpp"

#include "redemption_qt_include_widget.hpp"

#include REDEMPTION_QT_INCLUDE_WIDGET(QWidget)

#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>



class QtInputSocket : public QObject, public ClientInputSocketAPI
{
REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Winconsistent-missing-override")
Q_OBJECT
REDEMPTION_DIAGNOSTIC_POP

    TimeBase& time_base;
    EventContainer& events;

    QSocketNotifier * _sckListener;

    QTimer timer;

    ClientRedemptionAPI * client;

public:
    QtInputSocket(
        TimeBase& time_base,
        EventContainer & events,
        ClientRedemptionAPI * client,
        QWidget * parent)
    : QObject(parent)
    , time_base(time_base)
    , events(events)
    , _sckListener(nullptr)
    , timer(this)
    , client(client)
    {}

    ~QtInputSocket() {
        this->disconnect();
    }

    void disconnect() override {
        if (this->_sckListener != nullptr) {
            delete (this->_sckListener);
            this->_sckListener = nullptr;
        }
    }


    bool start_to_listen(int client_sck, mod_api * mod) override {

        this->_callback = mod;

        if (this->_callback) {
            this->_sckListener = new QSocketNotifier(client_sck, QSocketNotifier::Read, this);
            this->QObject::connect(this->_sckListener, SIGNAL(activated(int)), this, SLOT(call_draw_event_data()));
            this->QObject::connect(&(this->timer), SIGNAL(timeout()), this, SLOT(call_draw_event_timer()));

            //LOG(LOG_INFO, "start to listen : we have a callback");

            this->prepare_timer_event();

            return true;
        }
        return false;
    }



public Q_SLOTS:
    void call_draw_event_data() {
        // LOG(LOG_DEBUG, "draw_event_data");
        if (this->_callback && this->client) {
            this->client->callback(false);
            this->prepare_timer_event();
        }
    }

    void call_draw_event_timer() {
        // LOG(LOG_DEBUG, "draw_event_timer");
        if (this->_callback && this->client) {
            this->client->callback(true);
            this->prepare_timer_event();
        }
    }

private:
    void init()
    {
        this->timer.start(0);
    }

    void prepare_timer_event()
    {
        this->timer.start(1);
    }
};
