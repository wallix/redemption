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
#include "core/session_reactor.hpp"

template<char... cs>
struct string_c
{
    static inline char const value[sizeof...(cs)+1]{cs..., '\0'};
};
#include "cxx/diagnostic.hpp"
REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wgnu-string-literal-operator-template")
template<class C, C... cs>
string_c<cs...> operator ""_c ()
{ return {}; }
REDEMPTION_DIAGNOSTIC_POP

class SessionProbeClipboardBasedLauncher final : public SessionProbeLauncher {
    enum class State {
        START,                          // 0
        RUN,
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

    SessionReactor::BasicTimerPtr event;

    SessionProbeVirtualChannel* sesprob_channel = nullptr;
    ClipboardVirtualChannel*    cliprdr_channel = nullptr;

    const std::chrono::milliseconds clipboard_initialization_delay;
    const std::chrono::milliseconds long_delay;
    const std::chrono::milliseconds short_delay;

    unsigned int copy_paste_loop_counter = 0;

    SessionReactor& session_reactor;

    const RDPVerbose verbose;

public:
    SessionProbeClipboardBasedLauncher(
        SessionReactor& session_reactor,
        mod_api& mod,
        const char* alternate_shell,
        std::chrono::milliseconds clipboard_initialization_delay_ms,
        std::chrono::milliseconds long_delay_ms,
        std::chrono::milliseconds short_delay_ms, RDPVerbose verbose)
    : mod(mod)
    , alternate_shell(alternate_shell)
    , clipboard_initialization_delay(
        std::max(clipboard_initialization_delay_ms, std::chrono::milliseconds(2000)))
    , long_delay(std::max(long_delay_ms, std::chrono::milliseconds(500)))
    , short_delay(std::max(short_delay_ms, std::chrono::milliseconds(50)))
    , session_reactor(session_reactor)
    , verbose(verbose)
    {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher: "
                    "clipboard_initialization_delay_ms=%" PRId64 " "
                    "long_delay_ms=%" PRId64 " "
                    "short_delay_ms=%" PRId64,
                clipboard_initialization_delay_ms.count(), long_delay_ms.count(),
                short_delay_ms.count());
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
            this->event = this->session_reactor.create_timer(std::ref(*this))
            .set_delay(this->clipboard_initialization_delay)
            .on_action(jln::one_shot<&SessionProbeClipboardBasedLauncher::on_event>());
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

    template<auto x>
    static constexpr auto value = std::integral_constant<decltype(x), x>{};

    template<class i, class F>
    struct indexed_type
    {
        using type = F;
    };

    template<std::size_t i, class F>
    using make_indexed = indexed_type<std::integral_constant<std::size_t, i>, F>;

    template<class... Fs>
    struct FunSequence
    {
        template<class Fn>
        FunSequence<make_indexed<0, Fn>> then(Fn) noexcept
        {
            return {};
        }
    };

    template<class i, class F>
    static F get_fun(indexed_type<i, F>)
    {
        return jln::make_lambda<F>();
    }

    template<class i, class Sequenced, class Ctx>
    struct REDEMPTION_CXX_NODISCARD FunSequencerExecutorCtx : Ctx
    {
        constexpr static bool is_final_sequence() noexcept
        {
            return i::value == Sequenced::sequence_size;
        }

        constexpr static size_t index() noexcept
        {
            return i::value;
        }

        jln::ExecutorResult sequence_next() noexcept
        {
            return this->sequence_at<i::value+1>();
        }

        jln::ExecutorResult sequence_previous() noexcept
        {
            return this->sequence_at<i::value-1>();
        }

        template<std::size_t I>
        jln::ExecutorResult sequence_at() noexcept
        {
            return this->next_action(this->get_sequence_at<I>());
        }

        template<class I>
        jln::ExecutorResult sequence_at(I) noexcept
        {
            return this->sequence_at<I::value>();
        }

        template<std::size_t I>
        jln::ExecutorResult exec_sequence_at() noexcept
        {
            return this->exec_action(this->get_sequence_at<I>());
        }

        template<class I>
        jln::ExecutorResult exec_sequence_at(I) noexcept
        {
            return this->sequence_at<I::value>();
        }

        template<std::size_t I>
        auto get_sequence_at() noexcept
        {
            using index = std::integral_constant<std::size_t, I>;
            return [](auto ctx, auto&&... xs){
                return get_fun<index>(Sequenced{})(
                    static_cast<FunSequencerExecutorCtx&>(ctx),
                    static_cast<decltype(xs)&&>(xs)...);
            };
        }

        FunSequencerExecutorCtx set_time(std::chrono::milliseconds ms)
        {
            return static_cast<FunSequencerExecutorCtx&&>(Ctx::set_time(ms));
        }
    };

    template<class i, class Sequenced>
    struct FunSequencerExecutor
    {
        template<class Ctx, class... Ts>
        jln::ExecutorResult operator()(Ctx ctx, Ts&&... xs)
        {
            using NewCtx = FunSequencerExecutorCtx<i, Sequenced, Ctx>;
            return get_fun<i>(Sequenced{})(
                static_cast<NewCtx&>(ctx), static_cast<Ts&&>(xs)...);
        }
    };

    template<class F, class... Fs>
    struct FunSequence<F, Fs...> : F, Fs...
    {
        static constexpr std::size_t sequence_size = sizeof...(Fs)+1;

        template<class Fn>
        FunSequence<F, Fs..., make_indexed<1+sizeof...(Fs), Fn>> then(Fn) noexcept
        {
            return {};
        }

        template<class... Ts>
        jln::ExecutorResult operator()(Ts&&... xs)
        {
            return this->to_function()(static_cast<Ts&&>(xs)...);
        }

        auto to_function() noexcept
        {
            return FunSequencerExecutor<std::integral_constant<std::size_t, 0>, FunSequence>{};
        }
    };

    static FunSequence<> sequence() noexcept
    { return {}; }

    bool on_event() override
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

    bool on_server_format_list_response() override
    {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeClipboardBasedLauncher :=> on_server_format_list_response");
        }

        if (this->state != State::START) {
            return (this->state < State::WAIT);
        }

        this->state = State::RUN;

        auto send_scancode = [](auto s, auto param1, auto device_flags){
            return [](auto ctx, SessionProbeClipboardBasedLauncher& self){
                if (bool(self.verbose & RDPVerbose::sesprobe_launcher)) {
                    LOG(LOG_INFO, "SessionProbeClipboardBasedLauncher :=> on_event - %s",
                        decltype(s)::value);
                }

                self.mod.send_input(
                    0/*time*/,
                    RDP_INPUT_SCANCODE,
                    decltype(device_flags)::value,
                    decltype(param1)::value,
                    0/*param2*/
                );

                if constexpr (ctx.is_final_sequence()) {
                    self.state = State::WAIT;
                    return ctx.terminate();
                }
                else {
                    return ctx.set_time(self.short_delay)
                    .sequence_next();
                }
            };
        };

        this->event = this->session_reactor.create_timer(std::ref(*this))
        .set_delay(this->short_delay)
        .on_action(sequence()
        .then(send_scancode("Windows (down)"_c,    value<91>, value<SlowPath::KBDFLAGS_EXTENDED>))
        .then(send_scancode("d (down)"_c,          value<32>, value<0>))
        .then(send_scancode("d (up)"_c,            value<32>, value<SlowPath::KBDFLAGS_RELEASE>))
        .then(send_scancode("Windows (up)"_c,      value<91>, value<SlowPath::KBDFLAGS_EXTENDED |
                                                                    SlowPath::KBDFLAGS_RELEASE>))
        .then(send_scancode("Windows (down)"_c,    value<91>, value<SlowPath::KBDFLAGS_EXTENDED>))
        .then(send_scancode("r (down)"_c,          value<19>, value<0>))
        .then(send_scancode("r (up)"_c,            value<19>, value<SlowPath::KBDFLAGS_RELEASE>))
        .then(send_scancode("Windows (up)"_c,      value<91>, value<SlowPath::KBDFLAGS_EXTENDED |
                                                                    SlowPath::KBDFLAGS_RELEASE>))
        .then(send_scancode("Ctrl (down)"_c,       value<29>, value<0>))
        .then(send_scancode("a (down)"_c,          value<16>, value<0>))
        .then(send_scancode("a (up)"_c,            value<16>, value<SlowPath::KBDFLAGS_RELEASE>))
        .then(send_scancode("Ctrl (up)"_c,         value<29>, value<SlowPath::KBDFLAGS_RELEASE>))
        .then(send_scancode("Ctrl (down)"_c,       value<29>, value<0>))
        .then(send_scancode("v (down)"_c,          value<47>, value<0>))
        .then(send_scancode("v (up)"_c,            value<47>, value<SlowPath::KBDFLAGS_RELEASE>))
        .then(send_scancode("Ctrl (up)"_c,         value<29>, value<SlowPath::KBDFLAGS_RELEASE>))
        .then([](auto ctx, SessionProbeClipboardBasedLauncher& self) {
            if (!self.format_data_requested) {
                return ctx.set_time(self.short_delay)
                .template exec_sequence_at<0>();
            }
            return jln::make_lambda<decltype(send_scancode)>()
                           ("Enter (down)"_c,      value<28>, value<0>)(ctx, self);
        })
        .then(send_scancode("Enter (up)"_c,        value<28>, value<SlowPath::KBDFLAGS_RELEASE>))
        .to_function())
        ;

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
            this->cliprdr_channel->empty_client_clipboard();
        }
    }

private:
    void do_state_start() {
        assert(State::START == this->state);

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

    void rdp_send_scancode(long param1, long param2, long device_flags, long time, Keymap2 *) {
        this->mod.send_input(time, RDP_INPUT_SCANCODE, device_flags, param1, param2);
    }

public:
    bool is_stopped() const override {
        return (this->state == State::STOP);
    }
};  // class SessionProbeClipboardBasedLauncher

