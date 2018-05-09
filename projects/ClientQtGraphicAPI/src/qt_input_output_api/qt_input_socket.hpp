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

#include <fstream>

#include "utils/log.hpp"
#include "core/session_reactor.hpp"
#include "client_redemption/client_input_output_api.hpp"

#if REDEMPTION_QT_VERSION == 4
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtGui/name>
#else
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtWidgets/name>
#endif

#include REDEMPTION_QT_INCLUDE_WIDGET(QWidget)
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>


class QtInputSocket : public QObject, public ClientInputSocketAPI
{
    Q_OBJECT

    SessionReactor& session_reactor;

public:
    QSocketNotifier           * _sckListener;

    QTimer timer;


    QtInputSocket(SessionReactor& session_reactor, QWidget * parent)
        : QObject(parent)
        , session_reactor(session_reactor)
        , _sckListener(nullptr)
        , timer(this)
    {}

    ~QtInputSocket() {
        this->disconnect();
    }


    virtual void disconnect() override {
        if (this->_sckListener != nullptr) {
            delete (this->_sckListener);
            this->_sckListener = nullptr;
        }
    }


    virtual bool start_to_listen(int client_sck, mod_api * mod) override {

        this->_callback = mod;

        if (this->_callback) {
            this->_sckListener = new QSocketNotifier(client_sck, QSocketNotifier::Read, this);
            this->QObject::connect(this->_sckListener, SIGNAL(activated(int)), this, SLOT(call_draw_event_data()));
            this->QObject::connect(&(this->timer), SIGNAL(timeout()), this, SLOT(call_draw_event_timer()));

            LOG(LOG_INFO, "start to listen : we have a callback");

            this->prepare_timer_event();

            return true;
        }
        return false;
    }



public Q_SLOTS:
    void call_draw_event_data() {
        //LOG(LOG_INFO, "draw_event_data");
        if (this->client->mod) {
            this->client->callback();
            this->prepare_timer_event();
        }
    }

    void call_draw_event_timer() {
        //LOG(LOG_INFO, "draw_event_timer");
        if (this->client->mod) {
            this->client->callback();
            this->prepare_timer_event();
        }
    }

private:
    void prepare_timer_event() {
        timeval now = tvtime();
        this->session_reactor.set_current_time(now);
        timeval const tv = this->session_reactor.get_next_timeout(
            SessionReactor::EnableGraphics{false});
        if (tv.tv_sec > -1) {
            auto const time_to_wake = std::chrono::duration_cast<std::chrono::milliseconds>( ustime(tv) - ustime(now));
            this->timer.start( time_to_wake );
        }
        else {
            this->timer.stop();
        }
    }
};

