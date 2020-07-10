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

#include "mod/mod_api.hpp"
#include "mod/rdp/channels/cliprdr_channel.hpp"
#include "mod/rdp/channels/sespro_channel.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"
#include "mod/rdp/channels/sespro_clipboard_based_launcher.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "core/channel_names.hpp"
#include "utils/timebase.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/RDP/clipboard/format_list_serialize.hpp"
#include "acl/auth_api.hpp"

class SessionProbeClipboardBasedLauncher final : public SessionProbeLauncher {
public:
    using Params = SessionProbeClipboardBasedLauncherParams;

    private:
    enum class State {
        START,
        DELAY,
        RUN,
        WAIT,
        STOP
    } state = State::START;

    Params params;

    private:
    mod_api& mod;

    const std::string alternate_shell;

    bool drive_ready = false;
    bool drive_redirection_initialized = false;
    bool image_readed = false;
    bool clipboard_initialized = false;
    bool clipboard_initialized_by_proxy = false;
    bool clipboard_monitor_ready = false;

    bool format_data_requested = false;

    int event_id = 0;

    SessionProbeVirtualChannel* sesprob_channel = nullptr;
    ClipboardVirtualChannel*    cliprdr_channel = nullptr;

    float   delay_coefficient = 1.0f;

    unsigned int copy_paste_loop_counter = 0;

    time_t  delay_end_time = 0;
    bool    delay_executed = false;
    bool    delay_format_list_received = false;
    bool    delay_wainting_clipboard_response = false;

    TimeBase& time_base;
    EventContainer& events;

    const RDPVerbose verbose;

    static long long ms2ll(std::chrono::milliseconds const& ms)
    {
        return ms.count();
    }

    std::unique_ptr<uint8_t[]> current_client_format_list_pdu;
    size_t                     current_client_format_list_pdu_length = 0;
    uint32_t                   current_client_format_list_pdu_flags  = 0;

public:
    SessionProbeClipboardBasedLauncher(
        TimeBase& time_base,
        EventContainer& events,
        mod_api& mod,
        const char* alternate_shell,
        Params params,
        RDPVerbose verbose)
    : params(params)
    , mod(mod)
    , alternate_shell(alternate_shell)
    , time_base(time_base)
    , events(events)
    , verbose(verbose|RDPVerbose::sesprobe_launcher)
    {
        this->params.clipboard_initialization_delay_ms = std::max(this->params.clipboard_initialization_delay_ms, std::chrono::milliseconds(2000));
        this->params.long_delay_ms                     = std::max(this->params.long_delay_ms, std::chrono::milliseconds(500));
        this->params.short_delay_ms                    = std::max(this->params.short_delay_ms, std::chrono::milliseconds(50));

        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe_launcher), LOG_INFO,
            "SessionProbeClipboardBasedLauncher: "
                "clipboard_initialization_delay_ms=%lld "
                "start_delay_ms=%lld "
                "long_delay_ms=%lld "
                "short_delay_ms=%lld",
            ms2ll(this->params.clipboard_initialization_delay_ms), ms2ll(this->params.start_delay_ms),
            ms2ll(this->params.long_delay_ms), ms2ll(this->params.short_delay_ms));
    }

    ~SessionProbeClipboardBasedLauncher()
    {
        this->events.end_of_lifespan(this);
    }

    bool on_clipboard_initialize() override {
        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe_launcher), LOG_INFO,
            "SessionProbeClipboardBasedLauncher :=> on_clipboard_initialize");

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
            Event event("SessionProbeClipboardBasedLauncher::on_clipboard_monitor_ready", this);
            this->event_id = this->events.erase_event(this->event_id);
            this->event_id = event.id;
            event.alarm.set_timeout(this->time_base.get_current_time()+this->params.clipboard_initialization_delay_ms);
            event.actions.on_timeout = [this](Event&event)
            {
                this->on_event();
                event.garbage=true;
            };
            this->events.add(std::move(event));
        }

        if (this->sesprob_channel) {
            this->sesprob_channel->give_additional_launch_time();
        }

        return false;
    }

    bool on_drive_access() override {
        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe_launcher), LOG_INFO,
            "SessionProbeClipboardBasedLauncher :=> on_drive_access");

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

    bool on_device_announce_responded(bool bSucceeded) override {
        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe_launcher), LOG_INFO,
            "SessionProbeClipboardBasedLauncher :=> on_device_announce_responded, Succeeded=%s", (bSucceeded ? "Yes" : "No"));

        if (this->state == State::START) {
            this->drive_ready = bSucceeded;

            if (bSucceeded) {
                if (this->sesprob_channel) {
                    this->sesprob_channel->give_additional_launch_time();
                }

                this->do_state_start();
            }
            else {
                this->drive_redirection_initialized = false;

                if (this->sesprob_channel) {
                    this->sesprob_channel->abort_launch();
                }
            }
        }

        return false;
    }

    bool on_drive_redirection_initialize() override {
        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe_launcher), LOG_INFO,
            "SessionProbeClipboardBasedLauncher :=> on_drive_redirection_initialize");

        this->drive_redirection_initialized = true;

        return false;
    }

    bool on_event()
    {
        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe_launcher), LOG_INFO,
            "SessionProbeClipboardBasedLauncher :=> on_event - %d", int(this->state));

        switch (this->state) {
            case State::START:
                if (!this->clipboard_initialized) {
                    LOG_IF(bool(this->verbose & RDPVerbose::sesprobe_launcher), LOG_INFO,
                        "SessionProbeClipboardBasedLauncher :=> launcher managed cliprdr initialization");

                    if (this->cliprdr_channel) {
                        this->cliprdr_channel->disable_to_client_sender();
                    }

                    this->clipboard_initialized_by_proxy = true;

                    // Client Clipboard Capabilities PDU.
                    {
                        RDPECLIP::GeneralCapabilitySet general_cap_set(
                            RDPECLIP::CB_CAPS_VERSION_1,
                            RDPECLIP::CB_USE_LONG_FORMAT_NAMES);

                        RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu(1);

                        RDPECLIP::CliprdrHeader clipboard_header(RDPECLIP::CB_CLIP_CAPS, RDPECLIP::CB_RESPONSE__NONE_,
                            clipboard_caps_pdu.size() + general_cap_set.size());

                        StaticOutStream<1024> out_s;

                        clipboard_header.emit(out_s);
                        clipboard_caps_pdu.emit(out_s);
                        general_cap_set.emit(out_s);

                        InStream in_s(out_s.get_produced_bytes());
                        const size_t totalLength = out_s.get_offset();
                        this->mod.send_to_mod_channel(channel_names::cliprdr,
                                                      in_s,
                                                      totalLength,
                                                        CHANNELS::CHANNEL_FLAG_FIRST
                                                      | CHANNELS::CHANNEL_FLAG_LAST
                                                      | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
                    }

                    // Format List PDU.
                    {
                        StaticOutStream<256> out_s;
                        Cliprdr::format_list_serialize_with_header(
                            out_s,
                            Cliprdr::IsLongFormat(this->cliprdr_channel
                                ? this->cliprdr_channel->use_long_format_names()
                                : false),
                            std::array{Cliprdr::FormatNameRef{RDPECLIP::CF_TEXT, {}}});

                        InStream in_s(out_s.get_produced_bytes());
                        const size_t totalLength = out_s.get_offset();
                        this->mod.send_to_mod_channel(channel_names::cliprdr,
                                                      in_s,
                                                      totalLength,
                                                        CHANNELS::CHANNEL_FLAG_FIRST
                                                      | CHANNELS::CHANNEL_FLAG_LAST
                                                      | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
                    }
                }
            break;

            case State::WAIT:
            case State::STOP:
            default:
                LOG(LOG_WARNING, "SessionProbeClipboardBasedLauncher::on_event: State=%d", this->state);
                assert(false);
            break;
        }   // switch (this->state)

        return (this->state >= State::WAIT);
    }   // bool on_event() override

    bool on_image_read(uint64_t offset, uint32_t length) override {
        (void)offset;
        (void)length;

        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe_launcher), LOG_INFO,
            "SessionProbeClipboardBasedLauncher :=> on_image_read");

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
        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe_launcher), LOG_INFO,
            "SessionProbeClipboardBasedLauncher :=> on_server_format_data_request");

        StaticOutStream<1024> out_s;
        size_t alternate_shell_length = this->alternate_shell.length() + 1;
        RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, alternate_shell_length);
        const RDPECLIP::FormatDataResponsePDU format_data_response_pdu;
        header.emit(out_s);
        format_data_response_pdu.emit(out_s, byte_ptr_cast(this->alternate_shell.c_str()), alternate_shell_length);

        InStream in_s(out_s.get_produced_bytes());
        const size_t totalLength = out_s.get_offset();
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

    bool on_server_format_list() override
    {
        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe_launcher), LOG_INFO,
            "SessionProbeClipboardBasedLauncher :=> on_server_format_list");

        this->delay_format_list_received = true;

        return false;
    }

    static inline std::chrono::milliseconds to_microseconds(std::chrono::milliseconds const& delay, float coefficient) {
        return std::chrono::milliseconds(static_cast<uint64_t>(delay.count() * coefficient));
    }

    timeval get_short_delay_timeout()
    {
        std::chrono::milliseconds delay_ms = this->params.short_delay_ms;
        return this->time_base.get_current_time()+this->to_microseconds(delay_ms, this->delay_coefficient);
    }

    timeval get_long_delay_timeout()
    {
        std::chrono::milliseconds delay_ms = this->params.long_delay_ms;
        return this->time_base.get_current_time()+this->to_microseconds(delay_ms, this->delay_coefficient);
    }

    void make_delay_sequencer()
    {

        LOG(LOG_INFO, "SessionProbeClipboardBasedLauncher make_delay_sequencer()");

        Sequencer chain = {false, 0, bool(this->verbose & RDPVerbose::sesprobe_launcher), {
        { "Windows (down)",
            [this](Event&event,Sequencer&sequencer)
            {
                if (this->delay_format_list_received) {
                    event.alarm.set_timeout(event.alarm.now);
                    return sequencer.next_state("Send format list");
                }
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE,
                        SlowPath::KBDFLAGS_EXTENDED, 91, 0/*param2*/);
                event.alarm.set_timeout(this->get_short_delay_timeout());
            }
        },
        { "r (down)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE, 0, 19, 0/*param2*/);
                event.alarm.set_timeout(this->get_short_delay_timeout());
            }
        },
        { "r (up)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE,
                    SlowPath::KBDFLAGS_RELEASE, 19, 0/*param2*/);
                event.alarm.set_timeout(this->get_short_delay_timeout());
            }
        },
        { "Windows (up)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE,
                    SlowPath::KBDFLAGS_EXTENDED | SlowPath::KBDFLAGS_RELEASE, 91, 0/*param2*/);
                event.alarm.set_timeout(this->get_long_delay_timeout());
            }
        },
        { "Ctrl (down)",
            [this](Event&event,Sequencer&sequencer)
            {
                if (this->delay_format_list_received) {
                    event.alarm.set_timeout(event.alarm.now);
                    return sequencer.next_state("Send format list");
                }
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE, 0, 29, 0/*param2*/);
                event.alarm.set_timeout(this->get_short_delay_timeout());
            }
        },
        { "c (down)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE, 0, 46, 0/*param2*/);
                event.alarm.set_timeout(this->get_short_delay_timeout());
            }
        },
        { "c (up)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE,
                    SlowPath::KBDFLAGS_RELEASE, 46, 0/*param2*/);
                event.alarm.set_timeout(this->get_short_delay_timeout());
            }
        },
        { "Ctrl (up)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE,
                    SlowPath::KBDFLAGS_RELEASE, 29, 0/*param2*/);
                event.alarm.set_timeout(this->get_long_delay_timeout());
            }
        },
        { "Wait",
            [this](Event&event,Sequencer&sequencer)
            {
                if (time(nullptr) < this->delay_end_time){
                    this->delay_coefficient += 0.5f;
                    event.alarm.set_timeout(event.alarm.now);
                    return sequencer.next_state("Windows (down)");
                }
                event.alarm.set_timeout(event.alarm.now);
                return sequencer.next_state("Send format list");
            }
        },
        { "Send format list",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                this->delay_wainting_clipboard_response = true;
                {
                    StaticOutStream<256> out_s;
                    Cliprdr::format_list_serialize_with_header(
                        out_s,
                        Cliprdr::IsLongFormat(this->cliprdr_channel
                            ? this->cliprdr_channel->use_long_format_names()
                            : false),
                        std::array{Cliprdr::FormatNameRef{RDPECLIP::CF_TEXT, {}}});

                    InStream in_s(out_s.get_produced_bytes());
                    const size_t totalLength = out_s.get_offset();
                    this->mod.send_to_mod_channel(
                                        channel_names::cliprdr,
                                         in_s,
                                         totalLength,
                                           CHANNELS::CHANNEL_FLAG_FIRST
                                         | CHANNELS::CHANNEL_FLAG_LAST
                                         | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
                }
                event.alarm.set_timeout(this->get_long_delay_timeout());
            }
        },
        { "Wait format list response",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                event.alarm.set_timeout(this->get_long_delay_timeout());
                event.garbage = true;
            }
        }
        }};

        Event event("SessionProbeClipboardBasedLauncher Event", this);
        this->event_id = this->events.erase_event(this->event_id);
        this->event_id = event.id;
        event.alarm.set_timeout(this->time_base.get_current_time()+this->params.short_delay_ms);
        chain.verbose = true; // bool(this->verbose & RDPVerbose::sesprobe_launcher);
        event.actions.on_timeout = std::move(chain);
        this->events.add(std::move(event));
    }

    void make_run_sequencer()
    {
        LOG(LOG_INFO, "SessionProbeClipboardBasedLauncher make_run_sequencer()");

        Sequencer chain = {false, 0, bool(this->verbose & RDPVerbose::sesprobe_launcher),
        {{ "Windows (down)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                if (this->image_readed) {
                    event.garbage = true;
                    return;
                }
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE,
                    SlowPath::KBDFLAGS_EXTENDED, 91, 0/*param2*/);

                event.alarm.set_timeout(this->get_short_delay_timeout());
            }
        },
        { "r (down)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE,
                    0, 19, 0/*param2*/);
                event.alarm.set_timeout(this->get_short_delay_timeout());
            }
        },
        { "r (up)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE,
                    SlowPath::KBDFLAGS_RELEASE, 19, 0/*param2*/);
                event.alarm.set_timeout(this->get_short_delay_timeout());
            }
        },
        { "Windows (up)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE,
                    SlowPath::KBDFLAGS_EXTENDED |SlowPath::KBDFLAGS_RELEASE, 91, 0/*param2*/);
                event.alarm.set_timeout(this->get_long_delay_timeout());
            }
        },
        { "Ctrl (down)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                if (this->image_readed) {
                    event.garbage = true;
                    return;
                }
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE, 0, 29, 0/*param2*/);
                event.alarm.set_timeout(this->get_short_delay_timeout());
            }
        },
        { "v (down)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE, 0, 47, 0/*param2*/);
                event.alarm.set_timeout(this->get_short_delay_timeout());
            }
        },
        { "v (up)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE,
                    SlowPath::KBDFLAGS_RELEASE, 47, 0/*param2*/);
                event.alarm.set_timeout(this->get_short_delay_timeout());
            }
        },
        { "Ctrl (up)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
               this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE,
                    SlowPath::KBDFLAGS_RELEASE, 29, 0/*param2*/);
               event.alarm.set_timeout(this->get_long_delay_timeout());
            }
        },
        { "Enter (down)",
            [this](Event&event,Sequencer&sequencer)
            {
                ++this->copy_paste_loop_counter;
                if (!this->format_data_requested) {
                    LOG(LOG_INFO, ":=> on_event: Back to begining of the sequence");
                    // Back to the beginning of the sequence
                    sequencer.next_state("Windows (down)");
                    event.alarm.set_timeout(event.alarm.now);
                    return;
                }
                if (this->image_readed) {
                    event.garbage = true;
                    return;
                }
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE, 0, 28, 0/*param2*/);
                event.alarm.set_timeout(this->get_short_delay_timeout());
            }
        },
        { "Enter (up)",
            [this](Event&event,Sequencer&/*sequencer*/)
            {
                this->mod.send_input(0/*time*/, RDP_INPUT_SCANCODE, SlowPath::KBDFLAGS_RELEASE, 28, 0/*param2*/);
                LOG(LOG_INFO, "========= state changed to State::WAIT (%d) ====", this->state);
                this->state = State::WAIT;
                event.alarm.set_timeout(this->get_short_delay_timeout());
                event.garbage = true;
            }
        }}};

        Event event("SessionProbeClipboardBasedLauncher Event", this);
        this->event_id = this->events.erase_event(this->event_id);
        this->event_id = event.id;
        event.alarm.set_timeout(this->time_base.get_current_time()+this->params.short_delay_ms);
        chain.verbose = true; //bool(this->verbose & RDPVerbose::sesprobe_launcher);
        event.actions.on_timeout = std::move(chain);
        this->events.add(std::move(event));
    }

    bool on_server_format_list_response() override
    {
        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe_launcher), LOG_INFO,
            "SessionProbeClipboardBasedLauncher :=> on_server_format_list_response");

        if (this->params.start_delay_ms.count()) {
            if (!this->delay_executed) {
                if (this->state != State::START) {
                    return (this->state < State::WAIT);
                }

                LOG(LOG_INFO, "========= state changed to State::DELAY (%d) ====", this->state);
                this->state = State::DELAY;

                make_delay_sequencer();

                time_t const now = time(nullptr);
                this->delay_end_time = (now + (this->params.start_delay_ms.count() + 999) / 1000);

                this->delay_executed = true;
            }
            else if (this->delay_wainting_clipboard_response) {
                this->delay_wainting_clipboard_response = false;

                LOG(LOG_INFO, "========= state changed to State::RUN (%d) ====", this->state);
                this->state = State::RUN;

                make_run_sequencer();

                this->delay_coefficient = 1.0f;

                return false;
            }

            return true;
        }

        if (this->state != State::START) {
            return (this->state < State::WAIT);
        }

        LOG(LOG_INFO, "========= state changed to State::RUN B (%d) ====", this->state);
        this->state = State::RUN;

        make_run_sequencer();

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
            const uint8_t* current_chunk_pos  = chunk.get_current();
            const size_t   current_chunk_size = chunk.in_remain();

            assert(current_chunk_size == length);

            const uint16_t msgType = chunk.in_uint16_le();
            if (msgType == RDPECLIP::CB_FORMAT_LIST) {
                this->current_client_format_list_pdu_length = current_chunk_size;
                this->current_client_format_list_pdu        =
                    std::make_unique<uint8_t[]>(current_chunk_size);
                ::memcpy(this->current_client_format_list_pdu.get(),
                    current_chunk_pos, current_chunk_size);
                this->current_client_format_list_pdu_flags  = flags;

                StaticOutStream<256> out_s;
                Cliprdr::format_list_serialize_with_header(
                    out_s,
                    Cliprdr::IsLongFormat(this->cliprdr_channel
                                       && this->cliprdr_channel->use_long_format_names()),
                    std::array{Cliprdr::FormatNameRef{RDPECLIP::CF_TEXT, {}}});

                const size_t totalLength = out_s.get_offset();

                this->cliprdr_channel->process_client_message(
                        totalLength,
                          CHANNELS::CHANNEL_FLAG_FIRST
                        | CHANNELS::CHANNEL_FLAG_LAST
                        | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
                        out_s.get_produced_bytes());

                ret = false;
            }
        }

        chunk.rewind(saved_chunk_offset);

        return ret;
    }

    void set_clipboard_virtual_channel(ClipboardVirtualChannel* channel) override {
        this->cliprdr_channel = channel;
    }

    void set_remote_programs_virtual_channel(BaseVirtualChannel* /*channel*/) override {}

    void set_session_probe_virtual_channel(
            BaseVirtualChannel* channel) override {
        this->sesprob_channel = reinterpret_cast<SessionProbeVirtualChannel*>(channel);
    }

    void stop(bool bLaunchSuccessful, error_type& id_ref) override {
        id_ref = NO_ERROR;

        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe_launcher), LOG_INFO,
            "SessionProbeClipboardBasedLauncher :=> stop");

        LOG(LOG_INFO, "========= state changed to State::STOP (%d) ====", this->state);

        this->state = State::STOP;
        this->event_id = this->events.erase_event(this->event_id);

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
            if (!this->clipboard_initialized_by_proxy && bool(this->current_client_format_list_pdu)) {
                // Sends client Format List PDU to server
                this->cliprdr_channel->process_client_message(
                        this->current_client_format_list_pdu_length,
                        this->current_client_format_list_pdu_flags,
                        {this->current_client_format_list_pdu.get(),
                        this->current_client_format_list_pdu_length});
            }
            else {
                this->cliprdr_channel->empty_client_clipboard();
            }
        }
    }

private:
    void do_state_start() {
        assert(State::START == this->state);

        if (!this->drive_ready || !this->clipboard_initialized) {
            return;
        }

        StaticOutStream<256> out_s;
        Cliprdr::format_list_serialize_with_header(
            out_s,
            Cliprdr::IsLongFormat(this->cliprdr_channel
                ? this->cliprdr_channel->use_long_format_names()
                : false),
            std::array{Cliprdr::FormatNameRef{RDPECLIP::CF_TEXT, {}}});

        InStream in_s(out_s.get_produced_bytes());
        const size_t totalLength = out_s.get_offset();
        this->mod.send_to_mod_channel(channel_names::cliprdr,
                                      in_s,
                                      totalLength,
                                        CHANNELS::CHANNEL_FLAG_FIRST
                                      | CHANNELS::CHANNEL_FLAG_LAST
                                      | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
    }

    void rdp_send_scancode(long param1, long param2, long device_flags, long time, Keymap2 * /*unused*/) {
        this->mod.send_input(time, RDP_INPUT_SCANCODE, device_flags, param1, param2);
    }

public:
    [[nodiscard]] bool is_keyboard_sequences_started() const override {
        return (State::START != this->state);
    }

    [[nodiscard]] bool is_stopped() const override {
        return (this->state == State::STOP);
    }
};  // class SessionProbeClipboardBasedLauncher
