/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "core/wait_obj.hpp"
#include "mod/internal/client_execute.hpp"
#include "mod/internal/internal_mod.hpp"

struct LocallyIntegrableMod : public InternalMod {
    ClientExecute & client_execute;

    bool alt_key_pressed = false;

    uint16_t front_width;
    uint16_t front_height;

    enum {
        DCSTATE_WAIT,
        DCSTATE_FIRST_CLICK_DOWN,
        DCSTATE_FIRST_CLICK_RELEASE,
        DCSTATE_SECOND_CLICK_DOWN
    };

    int dc_state = DCSTATE_WAIT;

    wait_obj secondary_event;

    LocallyIntegrableMod(FrontAPI & front,
                         uint16_t front_width, uint16_t front_height,
                         Font const & font, ClientExecute & client_execute,
                         Theme const & theme = Theme())
    : InternalMod(front, front_width, front_height, font, theme)
    , client_execute(client_execute)
    , front_width(front_width)
    , front_height(front_height) {}

    ~LocallyIntegrableMod() override {
        this->client_execute.reset();
    }

    wait_obj * get_secondary_event() override {
        if (!this->secondary_event.object_and_time)
            return nullptr;

        return &this->secondary_event;
    }

    void rdp_input_invalidate(const Rect& r) override {
        InternalMod::rdp_input_invalidate(r);

        this->client_execute.input_invalidate(r);
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override {
        InternalMod::rdp_input_mouse(device_flags, x, y, keymap);

        //LOG(LOG_INFO, "device_flags=0x%X", device_flags);

        this->client_execute.input_mouse(device_flags, x, y);

        switch (this->dc_state) {
            case DCSTATE_WAIT:
                if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    this->dc_state = DCSTATE_FIRST_CLICK_DOWN;

                    this->secondary_event.set(1000000);

                    this->secondary_event.object_and_time  = true;
                    this->secondary_event.waked_up_by_time = false;
                }
            break;

            case DCSTATE_FIRST_CLICK_DOWN:
                if (device_flags == SlowPath::PTRFLAGS_BUTTON1) {
                    this->dc_state = DCSTATE_FIRST_CLICK_RELEASE;
                }
                else if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                }
                else {
                    this->cancel_double_click_detection();
                }
            break;

            case DCSTATE_FIRST_CLICK_RELEASE:
                if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    this->dc_state = DCSTATE_SECOND_CLICK_DOWN;
                }
                else {
                    this->cancel_double_click_detection();
                }
            break;

            case DCSTATE_SECOND_CLICK_DOWN:
                if (device_flags == SlowPath::PTRFLAGS_BUTTON1) {
                    this->dc_state = DCSTATE_WAIT;

                    this->client_execute.input_mouse(PTRFLAGS_EX_DOUBLE_CLICK, x, y);

                    this->cancel_double_click_detection();
                }
                else if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                }
                else {
                    this->cancel_double_click_detection();
                }
            break;

            default:
                REDASSERT(false);

                this->cancel_double_click_detection();
            break;
        }
    }

    void rdp_input_scancode(long param1, long param2, long param3, long param4,
            Keymap2 * keymap) override {
        InternalMod::rdp_input_scancode(param1, param2, param3, param4, keymap);

        if (!this->alt_key_pressed) {
            if ((param1 == 56) && !param3) {
                this->alt_key_pressed = true;
            }
        }
        else {
            if ((param1 == 56) && (param3 == (SlowPath::KBDFLAGS_DOWN | SlowPath::KBDFLAGS_RELEASE))) {
                this->alt_key_pressed = false;
            }
            else if ((param1 == 62) && !param3) {
                LOG(LOG_INFO, "Close by user (Alt+F4)");
                throw Error(ERR_WIDGET);    // F4 key pressed
            }
        }
    }

    void draw_event(time_t, gdi::GraphicApi &) override {
        if (!this->client_execute && this->event.waked_up_by_time) {
            this->client_execute.ready(*this, this->front_width, this->front_height, this->font());
        }

        if (this->secondary_event.object_and_time &&
            this->secondary_event.waked_up_by_time) {
            this->cancel_double_click_detection();
        }
    }

    void send_to_mod_channel(const char * front_channel_name, InStream& chunk, size_t length, uint32_t flags) override {
        if (this->client_execute && !strcmp(front_channel_name, CHANNELS::channel_names::rail)) {
            this->client_execute.send_to_mod_rail_channel(length, chunk, flags);
        }
    }

private:
    void cancel_double_click_detection() {
        this->secondary_event.reset();

        this->secondary_event.object_and_time  = false;
        this->secondary_event.waked_up_by_time = false;

        this->dc_state = DCSTATE_WAIT;
    }
};
