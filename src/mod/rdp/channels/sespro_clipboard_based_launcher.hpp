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


#pragma once

#include "core/wait_obj.hpp"
#include "mod/mod_api.hpp"
#include "mod/rdp/channels/cliprdr_channel.hpp"
#include "mod/rdp/channels/sespro_channel.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"
#include "mod/rdp/rdp_log.hpp"
#include "core/channel_names.hpp"

class SessionProbeClipboardBasedLauncher final : public SessionProbeLauncher {
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
        WAIT,
        STOP
    } state = State::START;

    mod_api& mod;

    const std::string alternate_shell;

    bool drive_ready = false;
    bool drive_redirection_initialized = false;
    bool image_readed = false;
    bool clipboard_initialized = false;
    bool clipboard_monitor_ready = false;

    bool format_data_requested = false;

    wait_obj event;

    SessionProbeVirtualChannel* sesprob_channel = nullptr;
    ClipboardVirtualChannel*    cliprdr_channel = nullptr;

    const uint64_t clipboard_initialization_delay  = 2000000;
    const uint64_t long_delay                      = 500000;
    const uint64_t short_delay                     = 50000;

    unsigned int copy_paste_loop_counter = 0;

    const RDPVerbose verbose;

public:
    SessionProbeClipboardBasedLauncher(mod_api& mod,
        const char* alternate_shell, RDPVerbose verbose)
    : mod(mod)
    , alternate_shell(alternate_shell)
    , verbose(verbose)
    {}

    wait_obj* get_event() override {
        if (this->event.is_trigger_time_set()) {
            return &this->event;
        }

        return nullptr;
    }

    bool on_clipboard_initialize() override {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> on_clipboard_initialize");
        }

        if (this->state != State::START) {
            return false;
        }

        this->clipboard_initialized = true;

        if (this->sesprob_channel) {
            this->sesprob_channel->give_additional_launch_time();
        }

        this->do_state_start();

        return false;
    }

    bool on_clipboard_monitor_ready() override {
        this->clipboard_monitor_ready = true;

        if (this->state == State::START) {
            this->event.set_trigger_time(this->clipboard_initialization_delay);
        }

        if (this->sesprob_channel) {
            this->sesprob_channel->give_additional_launch_time();
        }

        return false;
    }

    bool on_drive_access() override {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> on_drive_access");
        }

        if (this->state != State::START) {
            return false;
        }

        if (this->sesprob_channel) {
            this->sesprob_channel->give_additional_launch_time();
        }

        this->drive_ready = true;

        this->do_state_start();

        return false;
    }

    bool on_drive_redirection_initialize() override {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> on_drive_redirection_initialize");
        }

        this->drive_redirection_initialized = true;

        return false;
    }

    bool on_event() override {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO, "SessionProbeClipboardBasedLauncher :=> on_event - %d",
                static_cast<int>(this->state));
        }

        const long     param2 = 0;
        const long     param4 = 0;
              Keymap2* keymap = nullptr;

        switch (this->state) {
            case State::RUN_WIN_D_WIN_DOWN:
                // Windows (down)
                this->rdp_input_scancode(91,
                                         param2,
                                         SlowPath::KBDFLAGS_EXTENDED,
                                         param4,
                                         keymap);

                this->state = State::RUN_WIN_D_D_DOWN;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::RUN_WIN_D_D_DOWN:
                // d (down)
                this->rdp_input_scancode(32,
                                         param2,
                                         0,
                                         param4,
                                         keymap);

                this->state = State::RUN_WIN_D_D_UP;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::RUN_WIN_D_D_UP:
                // d (up)
                this->rdp_input_scancode(32,
                                         param2,
                                         SlowPath::KBDFLAGS_DOWN |
                                             SlowPath::KBDFLAGS_RELEASE,
                                         param4,
                                         keymap);

                this->state = State::RUN_WIN_D_WIN_UP;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::RUN_WIN_D_WIN_UP:
                // Windows (up)
                this->rdp_input_scancode(91,
                                         param2,
                                         SlowPath::KBDFLAGS_EXTENDED |
                                             SlowPath::KBDFLAGS_DOWN |
                                             SlowPath::KBDFLAGS_RELEASE,
                                         param4,
                                         keymap);

                this->state = State::RUN_WIN_R_WIN_DOWN;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::RUN_WIN_R_WIN_DOWN:
                // Windows (down)
                this->rdp_input_scancode(91,
                                         param2,
                                         SlowPath::KBDFLAGS_EXTENDED,
                                         param4,
                                         keymap);

                this->state = State::RUN_WIN_R_R_DOWN;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::RUN_WIN_R_R_DOWN:
                // r (down)
                this->rdp_input_scancode(19,
                                         param2,
                                         0,
                                         param4,
                                         keymap);

                this->state = State::RUN_WIN_R_R_UP;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::RUN_WIN_R_R_UP:
                // r (up)
                this->rdp_input_scancode(19,
                                         param2,
                                         SlowPath::KBDFLAGS_DOWN |
                                             SlowPath::KBDFLAGS_RELEASE,
                                         param4,
                                         keymap);

                this->state = State::RUN_WIN_R_WIN_UP;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::RUN_WIN_R_WIN_UP:
                // Windows (up)
                this->rdp_input_scancode(91,
                                         param2,
                                         SlowPath::KBDFLAGS_EXTENDED |
                                             SlowPath::KBDFLAGS_DOWN |
                                             SlowPath::KBDFLAGS_RELEASE,
                                         param4,
                                         keymap);

                this->state = State::CLIPBOARD;

                this->event.set_trigger_time(this->long_delay);
            break;

            case State::CLIPBOARD:
                this->state = State::CLIPBOARD_CTRL_A_CTRL_DOWN;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::CLIPBOARD_CTRL_A_CTRL_DOWN:
                // Ctrl (down)
                this->rdp_input_scancode(29,
                                         param2,
                                         0,
                                         param4,
                                         keymap);

                this->state = State::CLIPBOARD_CTRL_A_A_DOWN;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::CLIPBOARD_CTRL_A_A_DOWN:
                // a (down)
                this->rdp_input_scancode(16,
                                         param2,
                                         0,
                                         param4,
                                         keymap);

                this->state = State::CLIPBOARD_CTRL_A_A_UP;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::CLIPBOARD_CTRL_A_A_UP:
                // a (up)
                this->rdp_input_scancode(16,
                                         param2,
                                         SlowPath::KBDFLAGS_DOWN |
                                             SlowPath::KBDFLAGS_RELEASE,
                                         param4,
                                         keymap);

                this->state = State::CLIPBOARD_CTRL_A_CTRL_UP;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::CLIPBOARD_CTRL_A_CTRL_UP:
                // Ctrl (up)
                this->rdp_input_scancode(29,
                                         param2,
                                         SlowPath::KBDFLAGS_DOWN |
                                             SlowPath::KBDFLAGS_RELEASE,
                                         param4,
                                         keymap);

                this->state = State::CLIPBOARD_CTRL_V_CTRL_DOWN;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::CLIPBOARD_CTRL_V_CTRL_DOWN:
                // Ctrl (down)
                this->rdp_input_scancode(29,
                                         param2,
                                         0,
                                         param4,
                                         keymap);

                this->state = State::CLIPBOARD_CTRL_V_V_DOWN;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::CLIPBOARD_CTRL_V_V_DOWN:
                // v (down)
                this->rdp_input_scancode(47,
                                         param2,
                                         0,
                                         param4,
                                         keymap);

                this->state = State::CLIPBOARD_CTRL_V_V_UP;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::CLIPBOARD_CTRL_V_V_UP:
                // v (up)
                this->rdp_input_scancode(47,
                                         param2,
                                         SlowPath::KBDFLAGS_DOWN |
                                             SlowPath::KBDFLAGS_RELEASE,
                                         param4,
                                         keymap);

                this->state = State::CLIPBOARD_CTRL_V_CTRL_UP;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::CLIPBOARD_CTRL_V_CTRL_UP:
                // Ctrl (up)
                this->rdp_input_scancode(29,
                                         param2,
                                         SlowPath::KBDFLAGS_DOWN |
                                             SlowPath::KBDFLAGS_RELEASE,
                                         param4,
                                         keymap);

                this->state = State::ENTER;

                this->event.set_trigger_time(this->long_delay);
            break;

            case State::ENTER:
                this->do_state_enter();
            break;

            case State::ENTER_DOWN:
                // Enter (down)
                this->rdp_input_scancode(28,
                                         param2,
                                         0,
                                         param4,
                                         keymap);

                this->state = State::ENTER_UP;

                this->event.set_trigger_time(this->short_delay);
            break;

            case State::ENTER_UP:
                // Enter (up)
                this->rdp_input_scancode(28,
                                         param2,
                                         SlowPath::KBDFLAGS_DOWN |
                                             SlowPath::KBDFLAGS_RELEASE,
                                         param4,
                                         keymap);

                this->state = State::WAIT;

                this->event.reset_trigger_time();
            break;

            case State::START:
                if (!this->clipboard_initialized) {
                    if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
                        LOG(LOG_INFO,
                            "SessionProbeClipboardBasedLauncher :=> launcher managed cliprdr initialization");
                    }

                    if (this->cliprdr_channel) {
                        this->cliprdr_channel->disable_to_client_sender();
                    }

                    // Client Clipboard Capabilities PDU.
                    {
                        RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu(1,
                            RDPECLIP::GeneralCapabilitySet::size());
                        RDPECLIP::GeneralCapabilitySet general_cap_set(
                            RDPECLIP::CB_CAPS_VERSION_1,
                            RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
                        StaticOutStream<1024> out_s;

                        clipboard_caps_pdu.emit(out_s);
                        general_cap_set.emit(out_s);

                        const size_t totalLength = out_s.get_offset();

                        InStream in_s(out_s.get_data(), totalLength);

                        this->mod.send_to_mod_channel(channel_names::cliprdr,
                                                      in_s,
                                                      totalLength,
                                                        CHANNELS::CHANNEL_FLAG_FIRST
                                                      | CHANNELS::CHANNEL_FLAG_LAST
                                                      | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
                    }

                    // Format List PDU.
                    {
                        const bool use_long_format_names =
                            (this->cliprdr_channel ?
                             this->cliprdr_channel->use_long_format_names() :
                             false);

                        RDPECLIP::FormatListPDU format_list_pdu;
                        StaticOutStream<256>    out_s;

                        const bool unicodetext = false;

                        format_list_pdu.emit_2(out_s, unicodetext,
                            use_long_format_names);

                        const size_t totalLength = out_s.get_offset();

                        InStream in_s(out_s.get_data(), totalLength);

                        this->mod.send_to_mod_channel(channel_names::cliprdr,
                                                      in_s,
                                                      totalLength,
                                                        CHANNELS::CHANNEL_FLAG_FIRST
                                                      | CHANNELS::CHANNEL_FLAG_LAST
                                                      | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
                    }
                }

                this->event.reset_trigger_time();
            break;

            case State::RUN:
            case State::WAIT:
            case State::STOP:
            default:
                LOG(LOG_WARNING, "SessionProbeClipboardBasedLauncher::on_event: State=%d", this->state);
                REDASSERT(false);
            break;
        }   // switch (this->state)

        return (this->state >= State::WAIT);
    }   // bool on_event() override

    bool on_image_read(uint64_t offset, uint32_t length) override {
        (void)offset;
        (void)length;

        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> on_image_read");
        }

        this->image_readed = true;

        if (this->state == State::STOP) {
            return false;
        }

        if (this->sesprob_channel) {
            this->sesprob_channel->give_additional_launch_time();
        }

        return true;
    }

    bool on_server_format_data_request() override {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> on_server_format_data_request");
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
                                      | CHANNELS::CHANNEL_FLAG_LAST
                                      | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);

        this->format_data_requested = true;

        if (this->sesprob_channel) {
            this->sesprob_channel->give_additional_launch_time();
        }

        return false;
    }

    bool on_server_format_list_response() override {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> on_server_format_list_response");
        }

        if (this->state != State::START) {
            return (this->state < State::WAIT);
        }

        this->state = State::RUN_WIN_D_WIN_DOWN;

        this->event.set_trigger_time(this->short_delay);

        return false;
    }

    // Returns false to prevent message to be sent to server.
    bool process_client_cliprdr_message(InStream & chunk, uint32_t length, uint32_t flags) override {
        (void)length;


        if (this->state == State::STOP) {
            return true;
        }

        bool ret = true;

        const size_t saved_chunk_offset = chunk.get_offset();

        if ((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) &&
            (chunk.in_remain() >= 8 /* msgType(2) + msgFlags(2) + dataLen(4) */)) {
            const uint16_t msgType = chunk.in_uint16_le();
            chunk.in_skip_bytes(6); // msgFlags(2) + dataLen(4)
            if (msgType == RDPECLIP::CB_FORMAT_LIST) {
                const bool use_long_format_names =
                    (this->cliprdr_channel ?
                     this->cliprdr_channel->use_long_format_names() :
                     false);

                RDPECLIP::FormatListPDU format_list_pdu;
                StaticOutStream<256>    out_s;

                const bool unicodetext = false;

                format_list_pdu.emit_2(out_s, unicodetext, use_long_format_names);

                const size_t totalLength = out_s.get_offset();

                this->cliprdr_channel->process_client_message(
                        totalLength,
                          CHANNELS::CHANNEL_FLAG_FIRST
                        | CHANNELS::CHANNEL_FLAG_LAST
                        | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
                        out_s.get_data(),
                        totalLength);

                ret = false;
            }
        }

        chunk.rewind(saved_chunk_offset);

        return ret;
    }

    void set_clipboard_virtual_channel(
            BaseVirtualChannel* channel) override {
        this->cliprdr_channel = reinterpret_cast<ClipboardVirtualChannel*>(channel);
    }

    void set_remote_programs_virtual_channel(BaseVirtualChannel*) override {}

    void set_session_probe_virtual_channel(
            BaseVirtualChannel* channel) override {
        this->sesprob_channel = reinterpret_cast<SessionProbeVirtualChannel*>(channel);
    }

    void stop(bool bLaunchSuccessful, error_type& id_ref) override {
        id_ref = NO_ERROR;

        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> stop");
        }

        this->state = State::STOP;

        this->event.reset_trigger_time();

        if (!bLaunchSuccessful) {
            if (!this->drive_redirection_initialized) {
                LOG(LOG_ERR,
                    "SessionProbeClipboardBasedLauncher :=> "
                        "File System Virtual Channel is unavailable. "
                        "Please allow the drive redirection in the Remote Desktop Services settings of the target.");
                id_ref = ERR_SESSION_PROBE_CBBL_FSVC_UNAVAILABLE;
            }
            else if (!this->clipboard_initialized) {
                LOG(LOG_ERR,
                    "SessionProbeClipboardBasedLauncher :=> "
                        "Clipboard Virtual Channel is unavailable. "
                        "Please allow the clipboard redirection in the Remote Desktop Services settings of the target.");
                id_ref = ERR_SESSION_PROBE_CBBL_CBVC_UNAVAILABLE;
            }
            else if (!this->drive_ready) {
                LOG(LOG_ERR,
                    "SessionProbeClipboardBasedLauncher :=> "
                        "Drive of Session Probe is not ready yet. "
                        "Is the target running under Windows Server 2008 R2 or more recent version?");
                id_ref = ERR_SESSION_PROBE_CBBL_DRIVE_NOT_READY_YET;
            }
            else if (!this->image_readed) {
                LOG(LOG_ERR,
                    "SessionProbeClipboardBasedLauncher :=> "
                        "Session Probe is not launched. "
                        "Maybe something blocks it on the target. "
                        "Please also check the temporary directory to ensure there is enough free space.");
                id_ref = ERR_SESSION_PROBE_CBBL_MAYBE_SOMETHING_BLOCKS;
            }
            else if (!this->copy_paste_loop_counter) {
                LOG(LOG_ERR,
                    "SessionProbeClipboardBasedLauncher :=> "
                        "Session Probe launch cycle has been interrupted. "
                        "The launch timeout duration may be too short.");
                id_ref = ERR_SESSION_PROBE_CBBL_LAUNCH_CYCLE_INTERRUPTED;
            }
            else {
                LOG(LOG_ERR,
                    "SessionProbeClipboardBasedLauncher :=> "
                        "Session Probe launch has failed for unknown reason. "
                        "clipboard_monitor_ready=%s format_data_requested=%s",
                    (this->clipboard_monitor_ready ? "yes" : "no"),
                    (this->format_data_requested ? "yes" : "no"));
                id_ref = ERR_SESSION_PROBE_CBBL_UNKNOWN_REASON_REFER_TO_SYSLOG;
            }
        }

        if (this->clipboard_initialized) {
            this->cliprdr_channel->empty_client_clipboard();
        }
    }

private:
    void do_state_enter() {
        this->copy_paste_loop_counter++;

        if (!this->format_data_requested) {
            this->state = State::RUN_WIN_D_WIN_DOWN;

            this->event.set_trigger_time(this->short_delay);

            return;
        }

        this->state = State::ENTER_DOWN;

        this->event.set_trigger_time(this->short_delay);
    }

    void do_state_start() {
        REDASSERT(State::START == this->state);

        if (!this->drive_ready || !this->clipboard_initialized) {
            return;
        }

        const bool use_long_format_names =
            (this->cliprdr_channel ?
             this->cliprdr_channel->use_long_format_names() :
             false);

        RDPECLIP::FormatListPDU format_list_pdu;
        StaticOutStream<256>    out_s;

        const bool unicodetext = false;

        format_list_pdu.emit_2(out_s, unicodetext, use_long_format_names);

        const size_t totalLength = out_s.get_offset();

        InStream in_s(out_s.get_data(), totalLength);

        this->mod.send_to_mod_channel(channel_names::cliprdr,
                                      in_s,
                                      totalLength,
                                        CHANNELS::CHANNEL_FLAG_FIRST
                                      | CHANNELS::CHANNEL_FLAG_LAST
                                      | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
    }

    void rdp_input_scancode(long param1, long param2, long device_flags, long time, Keymap2 *) {
        this->mod.send_input(time, RDP_INPUT_SCANCODE, device_flags, param1, param2);
    }
};  // class SessionProbeClipboardBasedLauncher

