/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2018
*   Author(s): Clément Moroldo
*/

#pragma once

#include "mod/metrics.hpp"


class VNCMetrics
{

//-> minimum: data quantity (up/down)
//-> copy-paste: up/down
//-> redrawn surface
//-> mouse metrics same as RDP (displacement, clicks)
//-> data quantity transfered per data type (codec)
//-> keyboard metrics (special characters, total characters)


private:
    enum : int {
        data_from_client,     // number of byte sent from client to server
        data_from_server,     // number of byte sent from server to client

        mouse_displacement,
        keys_pressed,
        right_click,
        left_click,

        COUNT_FIELD
    };

    const char * vnc_metrics_name(int index) {

        switch (index) {
            case data_from_client:   return "data_from_client";
            case data_from_server:   return "data_from_server";
            case mouse_displacement: return "mouse_displacement";
            case keys_pressed:       return "keys_pressed";
            case right_click:        return "right_click";
            case left_click:         return "left_click";
            case COUNT_FIELD: break;
        }

        return "unknow_vnc_metrics_name";
    }

    const char * vnc_protocol_name = "vnc";

    Metrics metrics;


    // VNC context Info
    int last_x = -1;
    int last_y = -1;



public:

    VNCMetrics( const bool activate                         // do nothing if false
              , std::string path
              , std::string session_id
              , array_view_const_char primary_user_sig      // clear primary user account
              , array_view_const_char account_sig           // secondary account
              , array_view_const_char target_service_sig    // clear target service name + clear device name
              , array_view_const_char session_info_sig      // source_host + client info
              , const std::chrono::seconds now              // time at beginning of metrics
              , const std::chrono::hours file_interval      // daily rotation of filename (hours)
              , const std::chrono::seconds log_delay        // delay between 2 logs
              )
        : metrics(/*this->rdp_field_version*/"v1.0", this->vnc_protocol_name,
            activate, COUNT_FIELD, std::move(path), std::move(session_id),
            primary_user_sig, account_sig, target_service_sig, session_info_sig,
            now, file_interval, log_delay)
    {}

    bool active() const {
        return this->metrics.active_;
    }

    void log(timeval const& now) {
        this->metrics.log(now);
    }

    void mouse_mouve(const int x, const int y) {
        if (this->last_x >= 0 && this->last_y >= 0) {
            int x_shift = x - this->last_x;
            if (x_shift < 0) {
                x_shift *=  -1;
            }
            int y_shift = y - this->last_y;
            if (y_shift < 0) {
                y_shift *=  -1;
            }
            this->metrics.add_to_current_data(mouse_displacement, x_shift + y_shift);
        }
        this->last_x = x;
        this->last_y = y;
    }

    void key_pressed() {
        this->metrics.add_to_current_data(keys_pressed, 1);
    }

    void right_click_pressed() {
        this->metrics.add_to_current_data(right_click, 1);
    }

    void left_click_pressed() {
        this->metrics.add_to_current_data(left_click, 1);
    }


};
