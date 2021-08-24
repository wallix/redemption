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

#include "mod/rdp/channels/sespro_launcher.hpp"
#include "mod/rdp/channels/sespro_channel_params.hpp"
#include "mod/rdp/channels/rdp_session_probe_wrapper.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "core/events.hpp"

#include <memory>


class mod_rdp;


class SessionProbeClipboardBasedLauncher final : public SessionProbeLauncher
{
public:
    using Params = SessionProbeClipboardBasedLauncherParams;

    SessionProbeClipboardBasedLauncher(
        EventContainer& events,
        RdpSessionProbeWrapper rdp,
        const char* alternate_shell,
        Params params,
        RDPVerbose verbose);

    ~SessionProbeClipboardBasedLauncher() = default;

    bool on_client_format_list_rejected() override;

    bool on_clipboard_initialize() override;

    bool on_clipboard_monitor_ready() override;

    bool on_drive_access() override;

    bool on_device_announce_responded(bool bSucceeded) override;

    bool on_drive_redirection_initialize() override;

    bool on_image_read(uint64_t offset, uint32_t length) override;

    bool on_server_format_data_request() override;

    bool on_server_format_list() override;

    bool on_server_format_list_response() override;

    // Returns false to prevent message to be sent to server.
    bool process_client_cliprdr_message(InStream & chunk, uint32_t length, uint32_t flags) override;

    void set_clipboard_virtual_channel(ClipboardVirtualChannel* channel) override;

    void set_remote_programs_virtual_channel(RemoteProgramsVirtualChannel* channel) override;

    void set_session_probe_virtual_channel(SessionProbeVirtualChannel* channel) override;

    void stop(bool bLaunchSuccessful, error_type& id_ref) override;

    [[nodiscard]] bool is_keyboard_sequences_started() const override;

    [[nodiscard]] bool is_stopped() const override;

private:
    MonotonicTimePoint get_short_delay_timeout() const;

    MonotonicTimePoint get_long_delay_timeout() const;

    bool restore_client_clipboard();

    void make_delay_sequencer();

    void make_run_sequencer();

    bool on_event();

    void do_state_start();

    enum class State {
        START,
        DELAY,
        RUN,
        WAIT,
        STOP
    } state = State::START;

    Params params;

    RdpSessionProbeWrapper rdp;

    const std::string alternate_shell;

    bool drive_ready = false;
    bool drive_redirection_initialized = false;
    bool image_readed = false;
    bool clipboard_initialized = false;
    bool clipboard_initialized_by_proxy = false;
    bool clipboard_monitor_ready = false;

    bool format_data_requested = false;

    EventRef event_ref;

    SessionProbeVirtualChannel* sesprob_channel = nullptr;
    ClipboardVirtualChannel*    cliprdr_channel = nullptr;

    float   delay_coefficient = 1.0f;

    unsigned int copy_paste_loop_counter = 0;

    time_t delay_end_time = 0;
    bool   delay_executed = false;
    bool   delay_format_list_received = false;
    bool   delay_wainting_clipboard_response = false;

    EventsGuard events_guard;

    const RDPVerbose verbose;

    std::unique_ptr<uint8_t[]> current_client_format_list_pdu;
    size_t                     current_client_format_list_pdu_length = 0;
    uint32_t                   current_client_format_list_pdu_flags  = 0;

    bool format_list_sent = false;
};  // class SessionProbeClipboardBasedLauncher
