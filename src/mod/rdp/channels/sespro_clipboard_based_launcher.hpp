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
    Copyright (C) Wallix 2016
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef REDEMPTION_MOD_RDP_CHANNELS_SESPROCLIPBOARDBASEDLAUNCHER_HPP
#define REDEMPTION_MOD_RDP_CHANNELS_SESPROCLIPBOARDBASEDLAUNCHER_HPP

#include "core/wait_obj.hpp"
#include "mod/mod_api.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"
#include "mod/rdp/rdp_log.hpp"
#include "utils/virtual_channel_data_sender.hpp"

class SessionProbeClipboardBasedLauncher : public SessionProbeLauncher {
    enum class State {
        START,                          // 0
        RUN,
        RUN_WIN_D_WIN_DOWN,
        RUN_WIN_D_D_DOWN,
        RUN_WIN_D_D_UP,
        RUN_WIN_D_WIN_UP,               // 5
        RUN_WIN_R_WIN_DOWN,
        RUN_WIN_R_R_DOWN,
        RUN_WIN_R_R_UP,
        RUN_WIN_R_WIN_UP,
        CLIPBOARD,                      // 10
        CLIPBOARD_CTRL_A_CTRL_DOWN,
        CLIPBOARD_CTRL_A_A_DOWN,
        CLIPBOARD_CTRL_A_A_UP,
        CLIPBOARD_CTRL_A_CTRL_UP,
        CLIPBOARD_CTRL_V_CTRL_DOWN,     // 15
        CLIPBOARD_CTRL_V_V_DOWN,
        CLIPBOARD_CTRL_V_V_UP,
        CLIPBOARD_CTRL_V_CTRL_UP,
        ENTER,
        ENTER_DOWN,                     // 20
        ENTER_UP,
        STOP
    } state = State::START;

private:
    mod_api& mod;

    const std::string& alternate_shell;

    bool drive_ready = false;
    bool clipboard_initialized = false;

    bool format_data_requested = false;

    wait_obj event;

    uint32_t verbose;

public:
    SessionProbeClipboardBasedLauncher(mod_api& mod,
        const std::string& alternate_shell, uint32_t verbose)
    : mod(mod)
    , alternate_shell(alternate_shell)
    , verbose(verbose) {}

    wait_obj* get_event() override {
        if (this->event.object_and_time) {
            return &this->event;
        }

        return nullptr;
    }

    bool on_clipboard_initialize() override {
        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_LAUNCHER) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> on_clipboard_initialize");
        }

        if (this->state != State::START) {
            return false;
        }

        this->clipboard_initialized = true;

        this->do_state_start();

        return false;
    }

    bool on_drive_access() override {
        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_LAUNCHER) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> on_drive_access");
        }

        if (this->state != State::START) {
            return false;
        }

        this->drive_ready = true;

        this->do_state_start();

        return false;
    }

    bool on_event() override {
        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_LAUNCHER) {
            LOG(LOG_INFO, "SessionProbeClipboardBasedLauncher :=> on_event - %d",
                static_cast<int>(this->state));
        }

        const long     param2 = 0;
        const long     param4 = 0;
              Keymap2* keymap = nullptr;

        switch (this->state) {
            case State::RUN_WIN_D_WIN_DOWN:
                // Windows (down)
                this->mod.rdp_input_scancode(91,
                                             param2,
                                             SlowPath::KBDFLAGS_EXTENDED,
                                             param4,
                                             keymap);

                this->state = State::RUN_WIN_D_D_DOWN;
            break;

            case State::RUN_WIN_D_D_DOWN:
                // d (down)
                this->mod.rdp_input_scancode(32,
                                             param2,
                                             0,
                                             param4,
                                             keymap);

                this->state = State::RUN_WIN_D_D_UP;
            break;

            case State::RUN_WIN_D_D_UP:
                // d (up)
                this->mod.rdp_input_scancode(32,
                                             param2,
                                             SlowPath::KBDFLAGS_DOWN |
                                                 SlowPath::KBDFLAGS_RELEASE,
                                             param4,
                                             keymap);

                this->state = State::RUN_WIN_D_WIN_UP;
            break;

            case State::RUN_WIN_D_WIN_UP:
                // Windows (up)
                this->mod.rdp_input_scancode(91,
                                             param2,
                                             SlowPath::KBDFLAGS_EXTENDED |
                                                 SlowPath::KBDFLAGS_DOWN |
                                                 SlowPath::KBDFLAGS_RELEASE,
                                             param4,
                                             keymap);

                this->state = State::RUN_WIN_R_WIN_DOWN;
            break;

            case State::RUN_WIN_R_WIN_DOWN:
                // Windows (down)
                this->mod.rdp_input_scancode(91,
                                             param2,
                                             SlowPath::KBDFLAGS_EXTENDED,
                                             param4,
                                             keymap);

                this->state = State::RUN_WIN_R_R_DOWN;
            break;

            case State::RUN_WIN_R_R_DOWN:
                // r (down)
                this->mod.rdp_input_scancode(19,
                                             param2,
                                             0,
                                             param4,
                                             keymap);

                this->state = State::RUN_WIN_R_R_UP;
            break;

            case State::RUN_WIN_R_R_UP:
                // r (up)
                this->mod.rdp_input_scancode(19,
                                             param2,
                                             SlowPath::KBDFLAGS_DOWN |
                                                 SlowPath::KBDFLAGS_RELEASE,
                                             param4,
                                             keymap);

                this->state = State::RUN_WIN_R_WIN_UP;
            break;

            case State::RUN_WIN_R_WIN_UP:
                // Windows (up)
                this->mod.rdp_input_scancode(91,
                                             param2,
                                             SlowPath::KBDFLAGS_EXTENDED |
                                                 SlowPath::KBDFLAGS_DOWN |
                                                 SlowPath::KBDFLAGS_RELEASE,
                                             param4,
                                             keymap);

                this->state = State::CLIPBOARD;

                this->event.set(1000000);
            break;

            case State::CLIPBOARD:
                this->do_state_clipboard();

                this->event.set(250000);
            break;

            case State::CLIPBOARD_CTRL_A_CTRL_DOWN:
                // Ctrl (down)
                this->mod.rdp_input_scancode(29,
                                             param2,
                                             0,
                                             param4,
                                             keymap);

                this->state = State::CLIPBOARD_CTRL_A_A_DOWN;
            break;

            case State::CLIPBOARD_CTRL_A_A_DOWN:
                // a (down)
                this->mod.rdp_input_scancode(16,
                                             param2,
                                             0,
                                             param4,
                                             keymap);

                this->state = State::CLIPBOARD_CTRL_A_A_UP;
            break;

            case State::CLIPBOARD_CTRL_A_A_UP:
                // a (up)
                this->mod.rdp_input_scancode(16,
                                             param2,
                                             SlowPath::KBDFLAGS_DOWN |
                                                 SlowPath::KBDFLAGS_RELEASE,
                                             param4,
                                             keymap);

                this->state = State::CLIPBOARD_CTRL_A_CTRL_UP;
            break;

            case State::CLIPBOARD_CTRL_A_CTRL_UP:
                // Ctrl (up)
                this->mod.rdp_input_scancode(29,
                                             param2,
                                             0,
                                             SlowPath::KBDFLAGS_DOWN |
                                                 SlowPath::KBDFLAGS_RELEASE,
                                             keymap);

                this->state = State::CLIPBOARD_CTRL_V_CTRL_DOWN;
            break;

            case State::CLIPBOARD_CTRL_V_CTRL_DOWN:
                // Ctrl (down)
                this->mod.rdp_input_scancode(29,
                                             param2,
                                             0,
                                             param4,
                                             keymap);

                this->state = State::CLIPBOARD_CTRL_V_V_DOWN;
            break;

            case State::CLIPBOARD_CTRL_V_V_DOWN:
                // v (down)
                this->mod.rdp_input_scancode(47,
                                             param2,
                                             0,
                                             param4,
                                             keymap);

                this->state = State::CLIPBOARD_CTRL_V_V_UP;
            break;

            case State::CLIPBOARD_CTRL_V_V_UP:
                // v (up)
                this->mod.rdp_input_scancode(47,
                                             param2,
                                             SlowPath::KBDFLAGS_DOWN |
                                                 SlowPath::KBDFLAGS_RELEASE,
                                             param4,
                                             keymap);

                this->state = State::CLIPBOARD_CTRL_V_CTRL_UP;
            break;

            case State::CLIPBOARD_CTRL_V_CTRL_UP:
                // Ctrl (up)
                this->mod.rdp_input_scancode(29,
                                             param2,
                                             0,
                                             SlowPath::KBDFLAGS_DOWN |
                                                 SlowPath::KBDFLAGS_RELEASE,
                                             keymap);

                this->state = State::CLIPBOARD;

                this->event.set(1000000);
            break;

            case State::ENTER:
                this->state = State::ENTER_DOWN;

                this->event.set(250000);
            break;

            case State::ENTER_DOWN:
                // Enter (down)
                this->mod.rdp_input_scancode(28,
                                             param2,
                                             0,
                                             param4,
                                             keymap);

                this->state = State::ENTER_UP;
            break;

            case State::ENTER_UP:
                // Enter (up)
                this->mod.rdp_input_scancode(28,
                                             param2,
                                             SlowPath::KBDFLAGS_DOWN |
                                                 SlowPath::KBDFLAGS_RELEASE,
                                             param4,
                                             keymap);

                this->event.object_and_time = false;

                this->state = State::STOP;
            break;

            case State::START:
            case State::RUN:
            case State::STOP:
            default:
                REDASSERT(false);
            break;
        }

        return (this->state != State::STOP);
    }

    bool on_server_format_data_request() override {
        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_LAUNCHER) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> on_server_format_data_request");
        }

        this->format_data_requested = true;

        if (this->state == State::CLIPBOARD) {
            this->do_state_clipboard();
        }

        return false;
    }

    bool on_server_format_list_response() override {
        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_LAUNCHER) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> on_server_format_list_response");
        }

        if (this->state != State::RUN) {
            return (this->state != State::STOP);
        }

        this->event.object_and_time = true;

        this->event.set(250000);

        this->state = State::RUN_WIN_D_WIN_DOWN;

        return false;
    }

    virtual void stop() override {
        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_LAUNCHER) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> stop");
        }

        this->event.object_and_time = false;

        this->state = State::STOP;
    }

private:
    void do_state_clipboard() {
        if (!this->format_data_requested) {
            this->state = State::CLIPBOARD_CTRL_A_CTRL_DOWN;

            return;
        }

        StaticOutStream<1024> out_s;

        const bool response_ok = true;
        const RDPECLIP::FormatDataResponsePDU format_data_response_pdu(
            response_ok);

        size_t alternate_shell_length = this->alternate_shell.length() + 1;
        format_data_response_pdu.emit_ex(out_s, alternate_shell_length);
        out_s.out_copy_bytes(this->alternate_shell.c_str(),
            alternate_shell_length);

        const size_t totalLength = out_s.get_offset();

        InStream in_s(out_s.get_data(), totalLength);

        this->mod.send_to_mod_channel(channel_names::cliprdr,
                                      in_s,
                                      totalLength,
                                        CHANNELS::CHANNEL_FLAG_FIRST
                                      | CHANNELS::CHANNEL_FLAG_LAST);

        this->state = State::ENTER;

        this->event.set(1000000);
    }

    void do_state_start() {
        REDASSERT(State::START == this->state);

        if (!this->drive_ready || !this->clipboard_initialized) {
            return;
        }

        RDPECLIP::FormatListPDU format_list_pdu;
        StaticOutStream<256>    out_s;

        const bool unicodetext = false;

        format_list_pdu.emit_ex(out_s, unicodetext);

        const size_t totalLength = out_s.get_offset();

        InStream in_s(out_s.get_data(), totalLength);

        this->mod.send_to_mod_channel(channel_names::cliprdr,
                                      in_s,
                                      totalLength,
                                        CHANNELS::CHANNEL_FLAG_FIRST
                                      | CHANNELS::CHANNEL_FLAG_LAST);

        this->state = State::RUN;
    }
};

#endif  // #ifndef REDEMPTION_MOD_RDP_CHANNELS_SESPROCLIPBOARDBASEDLAUNCHER_HPP