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

    wait_obj first_click_down_event;

    class FirstClickDownEventHandler : public EventHandler::CB {
        LocallyIntegrableMod& mod_;

    public:
        FirstClickDownEventHandler(LocallyIntegrableMod& mod)
        : mod_(mod)
        {}

        void operator()(time_t now, wait_obj* event, gdi::GraphicApi& drawable) override {
            this->mod_.process_first_click_down_event(now, event, drawable);
        }
    } first_click_down_event_handler;

    LocallyIntegrableMod(FrontAPI & front,
                         uint16_t front_width, uint16_t front_height,
                         Font const & font, ClientExecute & client_execute,
                         Theme const & theme)
    : InternalMod(front, front_width, front_height, font, theme, false)
    , client_execute(client_execute)
    , front_width(front_width)
    , front_height(front_height)
    , first_click_down_event_handler(*this) {}

    ~LocallyIntegrableMod() override {
        this->client_execute.reset(true);
    }

    void get_event_handlers(std::vector<EventHandler>& out_event_handlers) override {
        if (this->first_click_down_event.object_and_time) {
            out_event_handlers.emplace_back(
                    &this->first_click_down_event,
                    &this->first_click_down_event_handler,
                    INVALID_SOCKET
                );
        }

        InternalMod::get_event_handlers(out_event_handlers);
    }

    void process_first_click_down_event(time_t, wait_obj* /*event*/, gdi::GraphicApi&) {
        if (this->first_click_down_event.object_and_time &&
            this->first_click_down_event.waked_up_by_time) {
            this->cancel_double_click_detection();
        }
    }

    void rdp_input_invalidate(Rect r) override {
        InternalMod::rdp_input_invalidate(r);

        this->client_execute.input_invalidate(r);
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override {
        InternalMod::rdp_input_mouse(device_flags, x, y, keymap);

        //LOG(LOG_INFO, "device_flags=0x%X", device_flags);

        if (this->client_execute.input_mouse(device_flags, x, y)) {
            this->screen.current_over = nullptr;
        }

        switch (this->dc_state) {
            case DCSTATE_WAIT:
                if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    this->dc_state = DCSTATE_FIRST_CLICK_DOWN;

                    this->first_click_down_event.set(1000000);

                    this->first_click_down_event.object_and_time  = true;
                    this->first_click_down_event.waked_up_by_time = false;
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

    void refresh(Rect r) override {
        InternalMod::refresh(r);

        this->client_execute.input_invalidate(r);
    }

    void draw_event(time_t, gdi::GraphicApi &) override {
        if (!this->client_execute && this->event.waked_up_by_time) {
            this->client_execute.ready(*this, this->front_width, this->front_height, this->font());
        }
    }

    void send_to_mod_channel(const char * front_channel_name, InStream& chunk, size_t length, uint32_t flags) override {
        if (this->client_execute && !strcmp(front_channel_name, CHANNELS::channel_names::rail)) {
            this->client_execute.send_to_mod_rail_channel(length, chunk, flags);
        }
    }

private:
    void cancel_double_click_detection() {
        this->first_click_down_event.reset();

        this->first_click_down_event.object_and_time  = false;
        this->first_click_down_event.waked_up_by_time = false;

        this->dc_state = DCSTATE_WAIT;
    }
};
