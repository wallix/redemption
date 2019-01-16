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
#include "core/session_reactor.hpp"

class SessionProbeClipboardBasedLauncher final : public SessionProbeLauncher {
    public:
    struct Params {
        std::chrono::milliseconds   clipboard_initialization_delay_ms{};
        std::chrono::milliseconds   start_delay_ms{};
        std::chrono::milliseconds   long_delay_ms{};
        std::chrono::milliseconds   short_delay_ms{};
    };

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

    SessionReactor::TimerPtr event;

    SessionProbeVirtualChannel* sesprob_channel = nullptr;
    ClipboardVirtualChannel*    cliprdr_channel = nullptr;

    float   delay_coefficient = 1.0f;

    unsigned int copy_paste_loop_counter = 0;

    time_t  delay_end_time = 0;
    bool    delay_executed = false;
    bool    delay_format_list_received = false;
    bool    delay_wainting_clipboard_response = false;

    SessionReactor& session_reactor;

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
        SessionReactor& session_reactor,
        mod_api& mod,
        const char* alternate_shell,
        Params params,
        RDPVerbose verbose)
    : params(params)
    , mod(mod)
    , alternate_shell(alternate_shell)
    , session_reactor(session_reactor)
    , verbose(verbose)
    {
        this->params.clipboard_initialization_delay_ms = std::max(this->params.clipboard_initialization_delay_ms, std::chrono::milliseconds(2000));
        this->params.long_delay_ms                     = std::max(this->params.long_delay_ms, std::chrono::milliseconds(500));
        this->params.short_delay_ms                    = std::max(this->params.short_delay_ms, std::chrono::milliseconds(50));

        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher: "
                    "clipboard_initialization_delay_ms=%lld "
                    "start_delay_ms=%lld "
                    "long_delay_ms=%lld "
                    "short_delay_ms=%lld",
                ms2ll(this->params.clipboard_initialization_delay_ms), ms2ll(this->params.start_delay_ms),
                ms2ll(this->params.long_delay_ms), ms2ll(this->params.short_delay_ms));
        }
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
            this->event = this->session_reactor.create_timer()
            .set_delay(this->params.clipboard_initialization_delay_ms)
            .on_action(jln::one_shot([&]{ this->on_event(); }));
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

    bool on_device_announce_responded() override {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> on_device_announce_responded");
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

    bool on_event()
    {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO, "SessionProbeClipboardBasedLauncher :=> on_event - %d",
                static_cast<int>(this->state));
        }

        switch (this->state) {
            case State::START:
                if (!this->clipboard_initialized) {
                    if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
                        LOG(LOG_INFO,
                            "SessionProbeClipboardBasedLauncher :=> launcher managed cliprdr initialization");
                    }

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
                        RDPECLIP::FormatListPDUEx format_list_pdu;
                        format_list_pdu.add_format_name(RDPECLIP::CF_TEXT);

                        const bool use_long_format_names =
                            (this->cliprdr_channel ?
                             this->cliprdr_channel->use_long_format_names() :
                             false);
                        const bool in_ASCII_8 = format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names);

                        RDPECLIP::CliprdrHeader clipboard_header(RDPECLIP::CB_FORMAT_LIST,
                            RDPECLIP::CB_RESPONSE__NONE_ | (in_ASCII_8 ? RDPECLIP::CB_ASCII_NAMES : 0),
                            format_list_pdu.size(use_long_format_names));

                        StaticOutStream<256> out_s;

                        clipboard_header.emit(out_s);
                        format_list_pdu.emit(out_s, use_long_format_names);

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
        size_t alternate_shell_length = this->alternate_shell.length() + 1;
        RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, alternate_shell_length);
        const RDPECLIP::FormatDataResponsePDU format_data_response_pdu;
        header.emit(out_s);
        format_data_response_pdu.emit(out_s, byte_ptr_cast(this->alternate_shell.c_str()), alternate_shell_length);
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

    bool on_server_format_list() override
    {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> on_server_format_list");
        }

        this->delay_format_list_received = true;

        return false;
    }

    static inline std::chrono::milliseconds to_microseconds(std::chrono::milliseconds const& delay, float coefficient) {
        return std::chrono::milliseconds(static_cast<uint64_t>(delay.count() * coefficient));
    }

    void make_delay_sequencer()
    {
        using jln::value;
        using namespace jln::literals;

        auto send_scancode = [](auto param1, auto device_flags, auto check_format_list_received, auto wait_for_short_delay){
            return [](auto ctx, SessionProbeClipboardBasedLauncher& self){
                if (bool(self.verbose & RDPVerbose::sesprobe_launcher)) {
                    LOG(LOG_INFO, "SessionProbeClipboardBasedLauncher :=> on_event - %s",
                        ctx.sequence_name());
                }

                if (decltype(check_format_list_received)::value && self.delay_format_list_received) {
                    return ctx.exec_at("Send format list"_c);
                }

                self.mod.send_input(
                    0/*time*/,
                    RDP_INPUT_SCANCODE,
                    decltype(device_flags)::value,
                    decltype(param1)::value,
                    0/*param2*/
                );

                return ctx.set_delay(
                        self.to_microseconds(
                                (decltype(wait_for_short_delay)::value ? self.params.short_delay_ms : self.params.long_delay_ms),
                                self.delay_coefficient
                            )
                    ).next();
            };
        };

        this->event = this->session_reactor.create_timer(std::ref(*this))
        .set_delay(this->params.short_delay_ms)
        .on_action(jln::sequencer(
            "Windows (down)"_f  (send_scancode(value<91>, value<SlowPath::KBDFLAGS_EXTENDED>, value<true>,  value<true> )),
            "r (down)"_f        (send_scancode(value<19>, value<0>,                           value<false>, value<true> )),
            "r (up)"_f          (send_scancode(value<19>, value<SlowPath::KBDFLAGS_RELEASE>,  value<false>, value<true> )),
            "Windows (up)"_f    (send_scancode(value<91>, value<SlowPath::KBDFLAGS_EXTENDED |
                                                                SlowPath::KBDFLAGS_RELEASE>,  value<false>, value<false>)),
            "Ctrl (down)"_f     (send_scancode(value<29>, value<0>,                           value<true>,  value<true> )),
            "c (down)"_f        (send_scancode(value<46>, value<0>,                           value<false>, value<true> )),
            "c (up)"_f          (send_scancode(value<46>, value<SlowPath::KBDFLAGS_RELEASE>,  value<false>, value<true> )),
            "Ctrl (up)"_f       (send_scancode(value<29>, value<SlowPath::KBDFLAGS_RELEASE>,  value<false>, value<false>)),

            "Wait"_f            ([](auto ctx, SessionProbeClipboardBasedLauncher& self) {
                if (time(nullptr) < self.delay_end_time) {
                    self.delay_coefficient += 0.5f;

                    return ctx.exec_at("Windows (down)"_c);
                }
                return ctx.exec_at("Send format list"_c);
            }),

            "Send format list"_f
                                ([](auto ctx, SessionProbeClipboardBasedLauncher& self) {
                self.delay_wainting_clipboard_response = true;

                {
                    RDPECLIP::FormatListPDUEx format_list_pdu;
                    format_list_pdu.add_format_name(RDPECLIP::CF_TEXT);

                    const bool use_long_format_names =
                        (self.cliprdr_channel ?
                         self.cliprdr_channel->use_long_format_names() :
                         false);
                    const bool in_ASCII_8 = format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names);

                    RDPECLIP::CliprdrHeader clipboard_header(RDPECLIP::CB_FORMAT_LIST,
                        RDPECLIP::CB_RESPONSE__NONE_ | (in_ASCII_8 ? RDPECLIP::CB_ASCII_NAMES : 0),
                        format_list_pdu.size(use_long_format_names));

                    StaticOutStream<256> out_s;

                    clipboard_header.emit(out_s);
                    format_list_pdu.emit(out_s, use_long_format_names);

                    const size_t totalLength = out_s.get_offset();

                    InStream in_s(out_s.get_data(), totalLength);

                    self.mod.send_to_mod_channel(channel_names::cliprdr,
                                                 in_s,
                                                 totalLength,
                                                   CHANNELS::CHANNEL_FLAG_FIRST
                                                 | CHANNELS::CHANNEL_FLAG_LAST
                                                 | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
                }

                return ctx.set_delay(self.to_microseconds(self.params.long_delay_ms, self.delay_coefficient)).next();
            }),
            "Wait format list response"_f
                                ([](auto ctx, SessionProbeClipboardBasedLauncher& self) {
                return ctx.set_delay(self.to_microseconds(self.params.long_delay_ms, self.delay_coefficient)).ready();
            })

        ));
    }

    void make_run_sequencer()
    {
        using jln::value;
        using namespace jln::literals;

        auto send_scancode = [](auto param1, auto device_flags, auto check_image_readed, auto wait_for_short_delay){
            return [](auto ctx, SessionProbeClipboardBasedLauncher& self){
                if (bool(self.verbose & RDPVerbose::sesprobe_launcher)) {
                    LOG(LOG_INFO, "SessionProbeClipboardBasedLauncher :=> on_event - %s",
                        ctx.sequence_name());
                }

                if (decltype(check_image_readed)::value && self.image_readed) {
                    return ctx.terminate();
                }

                self.mod.send_input(
                    0/*time*/,
                    RDP_INPUT_SCANCODE,
                    decltype(device_flags)::value,
                    decltype(param1)::value,
                    0/*param2*/
                );

                if (ctx.is_final_sequence()) {
                    self.state = State::WAIT;
                    return ctx.set_delay(self.to_microseconds(self.params.short_delay_ms, self.delay_coefficient)).at(0).ready();
                }

                return ctx.set_delay(
                    self.to_microseconds(
                        (decltype(wait_for_short_delay)::value ? self.params.short_delay_ms : self.params.long_delay_ms),
                        self.delay_coefficient
                    )
                ).next();
            };
        };

        this->event = this->session_reactor.create_timer(std::ref(*this))
        .set_delay(this->params.short_delay_ms)
        .on_action(jln::sequencer(
            "Windows (down)"_f  (send_scancode(value<91>, value<SlowPath::KBDFLAGS_EXTENDED>, value<true>,  value<true> )),
            "r (down)"_f        (send_scancode(value<19>, value<0>,                           value<false>, value<true> )),
            "r (up)"_f          (send_scancode(value<19>, value<SlowPath::KBDFLAGS_RELEASE>,  value<false>, value<true> )),
            "Windows (up)"_f    (send_scancode(value<91>, value<SlowPath::KBDFLAGS_EXTENDED |
                                                                SlowPath::KBDFLAGS_RELEASE>,  value<false>, value<false>)),
            "Ctrl (down)"_f     (send_scancode(value<29>, value<0>,                           value<true>,  value<true> )),
            "v (down)"_f        (send_scancode(value<47>, value<0>,                           value<false>, value<true> )),
            "v (up)"_f          (send_scancode(value<47>, value<SlowPath::KBDFLAGS_RELEASE>,  value<false>, value<true> )),
            "Ctrl (up)"_f       (send_scancode(value<29>, value<SlowPath::KBDFLAGS_RELEASE>,  value<false>, value<false>)),

            "Enter (down)"_f    ([](auto ctx, SessionProbeClipboardBasedLauncher& self) {
                ++self.copy_paste_loop_counter;
                if (!self.format_data_requested) {
                    return ctx.set_delay(self.to_microseconds(self.params.short_delay_ms, self.delay_coefficient)).exec_at(0);
                }
                return jln::make_lambda<decltype(send_scancode)>()(value<28>, value<0>, value<true>, value<true>)(ctx, self);
            }),
            "Enter (up)"_f      (send_scancode(value<28>, value<SlowPath::KBDFLAGS_RELEASE>,  value<false>, value<true>))
        ));
    }

    bool on_server_format_list_response() override
    {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> on_server_format_list_response");
        }

        if (this->params.start_delay_ms.count()) {
            if (!this->delay_executed) {
                if (this->state != State::START) {
                    return (this->state < State::WAIT);
                }

                this->state = State::DELAY;

                make_delay_sequencer();

                time_t const now = time(nullptr);
                this->delay_end_time = (now + (this->params.start_delay_ms.count() + 999) / 1000);

                this->delay_executed = true;
            }
            else if (this->delay_wainting_clipboard_response) {
                this->delay_wainting_clipboard_response = false;

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

                RDPECLIP::FormatListPDUEx format_list_pdu;
                format_list_pdu.add_format_name(RDPECLIP::CF_TEXT);

                const bool use_long_format_names =
                    (this->cliprdr_channel ?
                     this->cliprdr_channel->use_long_format_names() :
                     false);
                const bool in_ASCII_8 = format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names);

                RDPECLIP::CliprdrHeader clipboard_header(RDPECLIP::CB_FORMAT_LIST,
                    RDPECLIP::CB_RESPONSE__NONE_ | (in_ASCII_8 ? RDPECLIP::CB_ASCII_NAMES : 0),
                    format_list_pdu.size(use_long_format_names));

                StaticOutStream<256> out_s;

                clipboard_header.emit(out_s);
                format_list_pdu.emit(out_s, use_long_format_names);

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

        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> stop");
        }

        this->state = State::STOP;
        this->event.reset();

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
                        this->current_client_format_list_pdu.get(),
                        this->current_client_format_list_pdu_length);
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

        RDPECLIP::FormatListPDUEx format_list_pdu;
        format_list_pdu.add_format_name(RDPECLIP::CF_TEXT);

        const bool use_long_format_names =
            (this->cliprdr_channel ?
             this->cliprdr_channel->use_long_format_names() :
             false);
        const bool in_ASCII_8 = format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names);

        RDPECLIP::CliprdrHeader clipboard_header(RDPECLIP::CB_FORMAT_LIST,
            RDPECLIP::CB_RESPONSE__NONE_ | (in_ASCII_8 ? RDPECLIP::CB_ASCII_NAMES : 0),
            format_list_pdu.size(use_long_format_names));

        StaticOutStream<256> out_s;

        clipboard_header.emit(out_s);
        format_list_pdu.emit(out_s, use_long_format_names);

        const size_t totalLength = out_s.get_offset();
        InStream in_s(out_s.get_data(), totalLength);

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
    bool is_keyboard_sequences_started() const override {
        return (State::START != this->state);
    }

    bool is_stopped() const override {
        return (this->state == State::STOP);
    }
};  // class SessionProbeClipboardBasedLauncher
