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
   Author(s): Clément Moroldo

*/

#pragma once

#include <fstream>

#include "utils/log.hpp"
#include "../client_input_output_api.hpp"

#include <QtGui/QWidget>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>


class QtInputSocket : public QObject, public ClientInputSocketAPI
{

Q_OBJECT

public:
    QSocketNotifier           * _sckListener;

    QTimer timer;


    QtInputSocket(QWidget * parent)
        : QObject(parent)
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

        if (this->_callback !=  nullptr) {
            this->_sckListener = new QSocketNotifier(client_sck, QSocketNotifier::Read, this);
            this->QObject::connect(this->_sckListener,   SIGNAL(activated(int)), this,  SLOT(call_draw_event_data()));
            this->QObject::connect(&(this->timer),   SIGNAL(timeout()), this,  SLOT(call_draw_event_timer()));

            if (this->_callback) {
                if (this->_callback->get_event().is_trigger_time_set()) {
                    struct timeval now = tvtime();
                    int time_to_wake = (this->_callback->get_event().get_trigger_time().tv_usec - now.tv_usec) / 1000
                    + (this->_callback->get_event().get_trigger_time().tv_sec - now.tv_sec) * 1000;
//                     this->_callback->get_event().reset_trigger_time();

                    if (time_to_wake < 0) {
                        this->timer.stop();
                    } else {

                        this->timer.start( time_to_wake );
                    }
                }
            } else {
                return false;
            }

        } else {
            return false;
        }
        return true;
    }



public Q_SLOTS:
    void call_draw_event_data() {
        this->_callback->get_event().set_waked_up_by_time(false);
        this->call_draw_event();
    }

    void call_draw_event_timer() {
        this->_callback->get_event().set_waked_up_by_time(true);
        this->call_draw_event();
    }

    void call_draw_event() {
        if (this->client->mod) {

            this->client->callback();

            if (this->_callback) {
                if (this->_callback->get_event().is_trigger_time_set()) {
                    struct timeval now = tvtime();
                    int time_to_wake = ((this->_callback->get_event().get_trigger_time().tv_usec - now.tv_usec) / 1000)
                    + ((this->_callback->get_event().get_trigger_time().tv_sec - now.tv_sec) * 1000);

                    this->_callback->get_event().reset_trigger_time();

                    if (time_to_wake < 0) {
                        this->timer.stop();
//                         LOG(LOG_INFO, "time_to_wake = %d", time_to_wake);
//                         this->timer.start( 40 );
                    } else {
                        this->timer.start( time_to_wake );
                    }
                } else {
                    this->timer.stop();
                }
            }
        }
    }

};

