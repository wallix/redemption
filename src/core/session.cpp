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
   Copyright (C) Wallix 2010-2012
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou, Meng Tan
*/

#include "acl/module_manager/mod_factory.hpp"
#include "core/RDP/tpdu_buffer.hpp"
#include "core/session.hpp"
#include "core/session_events.hpp"
#include "core/session_verbose.hpp"
#include "core/pid_file.hpp"
#include "core/listen.hpp"
#include "core/guest_ctx.hpp"

#include "acl/session_inactivity.hpp"
#include "acl/acl_serializer.hpp"
#include "acl/mod_pack.hpp"
#include "acl/session_logfile.hpp"

#include "capture/capture.hpp"
#include "configs/config.hpp"
#include "front/front.hpp"
#include "mod/mod_api.hpp"
#include "mod/rdp/rdp_api.hpp"
#include "transport/socket_transport.hpp"
#include "transport/ws_transport.hpp"
#include "utils/genrandom.hpp"
#include "utils/invalid_socket.hpp"
#include "utils/netutils.hpp"
#include "utils/select.hpp"
#include "utils/log_siem.hpp"
#include "utils/redirection_info.hpp"
#include "utils/verbose_flags.hpp"
#include "utils/local_err_msg.hpp"
#include "utils/monotonic_clock.hpp"
#include "utils/to_timeval.hpp"

#include <vector>

#include <cassert>
#include <cerrno>
#include <cstring>
#include <cstdlib>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <unistd.h>


using namespace std::chrono_literals;

namespace
{

struct FinalSocketTransport final : ::SocketTransport
{
    using ::SocketTransport::SocketTransport;
};

enum class SessionVerbose : uint32_t
{
    Event   = 0x02,
    Acl     = 0x04,
    Trace   = 0x08,
};

SessionVerbose operator & (SessionVerbose x, SessionVerbose y) noexcept
{
    using int_type = uint32_t;
    return SessionVerbose(int_type(x) & int_type(y));
}

class Session
{
    struct AclReport final : AclReportApi
    {
        AclReport(Inifile& ini) : ini(ini) {}

        void report(const char * reason, const char * message) override
        {
            char report[1024];
            snprintf(report, sizeof(report), "%s:%s:%s", reason,
                this->ini.get<cfg::globals::target_device>().c_str(), message);
            this->ini.set_acl<cfg::context::reporting>(report);
        }

    private:
        Inifile& ini;
    };

    struct SecondarySession final : private SessionLogApi
    {
        enum class Type { RDP, VNC, };

        SecondarySession(
            Inifile& ini,
            CryptoContext& cctx,
            Random& rnd,
            gdi::CaptureProbeApi& probe_api,
            TimeBase const& time_base)
        : ini(ini)
        , probe_api(probe_api)
        , time_base(time_base)
        , cctx(cctx)
        , log_file(cctx, rnd, [&ini](Error const& error){
            if (error.errnum == ENOSPC) {
                // error.id = ERR_TRANSPORT_WRITE_NO_ROOM;
                AclReport{ini}.report("FILESYSTEM_FULL", "100|unknown");
            }
        })
        {
            auto has_drive = bool(ini.get<cfg::video::disable_file_system_log>() & FileSystemLogFlags::wrm);
            auto has_clipboard = bool(ini.get<cfg::video::disable_clipboard_log>() & ClipboardLogFlags::wrm);

            this->dont_log |= (has_drive ? LogCategoryId::Drive : LogCategoryId::None);
            this->dont_log |= (has_clipboard ? LogCategoryId::Clipboard : LogCategoryId::None);
        }

        [[nodiscard]]
        SessionLogApi& open_secondary_session(Type session_type)
        {
            assert(!this->is_open());

            switch (session_type)
            {
                case Type::RDP: this->session_type = "RDP"; break;
                case Type::VNC: this->session_type = "VNC"; break;
            }

            this->cctx.set_master_key(this->ini.get<cfg::crypto::encryption_key>());
            this->cctx.set_hmac_key(this->ini.get<cfg::crypto::sign_key>());
            this->cctx.set_trace_type(this->ini.get<cfg::globals::trace_type>());

            auto const& subdir = this->ini.get<cfg::capture::record_subdirectory>();
            auto const& record_dir = this->ini.get<cfg::video::record_path>();
            auto const& hash_dir = this->ini.get<cfg::video::hash_path>();
            auto const& filebase = this->ini.get<cfg::capture::record_filebase>();

            std::string record_path = str_concat(record_dir.as_string(), subdir, '/');
            std::string hash_path = str_concat(hash_dir.as_string(), subdir, '/');

            for (auto* s : {&record_path, &hash_path}) {
                if (recursive_create_directory(s->c_str(), S_IRWXU | S_IRGRP | S_IXGRP) != 0) {
                    LOG(LOG_ERR,
                        "Session::open_secondary_session: Failed to create directory: \"%s\"", *s);
                }
            }

            std::string basename = str_concat(filebase, ".log");
            record_path += basename;
            hash_path += basename;

            this->log_file.open_session_log(
                record_path.c_str(), hash_path.c_str(),
                this->ini.get<cfg::video::file_permissions>(), /*derivator=*/basename);

            return *this;
        }

        void close_secondary_session()
        {
            assert(this->is_open());
            this->session_type = {};
            this->log_file.close_session_log();
        }

        [[nodiscard]]
        SessionLogApi& get_secondary_session_log()
        {
            assert(this->is_open());
            return *this;
        }

        void report(const char * reason, const char * message) override
        {
            AclReport{this->ini}.report(reason, message);
        }

    private:
        #ifndef NDEBUG
        bool is_open() const
        {
            return !this->session_type.empty();
        }
        #endif

        void log6(LogId id, KVLogList kv_list) override
        {
            timespec tp;
            clock_gettime(CLOCK_REALTIME, &tp);

            this->log_file.log(tp.tv_sec, this->ini, this->session_type, id, kv_list);

            if (this->dont_log.test(detail::log_id_category_map[underlying_cast(id)])) {
                return ;
            }

            this->probe_api.session_update(this->time_base.monotonic_time, id, kv_list);
        }

        void set_control_owner_ctx(chars_view name) override
        {
            this->log_file.set_control_owner_ctx(name);
        }

        Inifile& ini;
        gdi::CaptureProbeApi& probe_api;
        TimeBase const& time_base;
        CryptoContext & cctx;
        std::string_view session_type;
        LogCategoryFlags dont_log {};
        SessionLogFile log_file;
        std::string sharing_ctx_extra_log;
    };

    struct Select
    {
        Select()
        {
            io_fd_zero(this->rfds);
        }

        int select(timeval* delay)
        {
            return ::select(
                max + 1,
                &rfds,
                want_write ? &wfds : nullptr,
                nullptr, delay);
        }

        bool is_set_for_writing(int fd) const
        {
            assert(this->want_write);
            return io_fd_isset(fd, this->wfds);
        }

        bool is_set_for_reading(int fd) const
        {
            bool ret = io_fd_isset(fd, this->rfds);
            assert(!this->want_write || !ret);
            return ret;
        }

        void set_read_sck(int fd)
        {
            assert(fd != INVALID_SOCKET);
            io_fd_set(fd, this->rfds);
            this->max = std::max(this->max, fd);
        }

        void set_write_sck(int fd)
        {
            assert(fd != INVALID_SOCKET);
            if (!this->want_write) {
                this->want_write = true;
                io_fd_zero(this->wfds);
            }

            io_fd_set(fd, this->wfds);
            this->max = std::max(this->max, fd);
        }

    private:
        int max = 0;
        bool want_write = false;
        fd_set rfds;
        fd_set wfds;
    };

    struct NullAclReport final : AclReportApi
    {
        void report(const char * reason, const char * message) override
        {
            (void)reason;
            (void)message;
        }
    };

    struct SessionFront final : Front
    {
        MonotonicTimePoint target_connection_start_time {};
        Inifile& ini;

        SessionFront(
            EventContainer& events,
            AclReportApi& acl_report,
            Transport& trans,
            Random& gen,
            Inifile& ini,
            CryptoContext& cctx
        )
        : Front(events, acl_report, trans, gen, ini, cctx)
        , ini(ini)
        {}

        // secondary session is ready, set target_connection_time
        bool can_be_start_capture(SessionLogApi& session_log) override
        {
            if (this->target_connection_start_time != MonotonicTimePoint{}) {
                auto elapsed = MonotonicTimePoint::clock::now()
                                - this->target_connection_start_time;
                this->ini.set_acl<cfg::globals::target_connection_time>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(elapsed));
                this->target_connection_start_time = {};
            }

            this->ini.set_acl<cfg::context::sharing_ready>(true);

            if (this->Front::can_be_start_capture(session_log)) {
                // Must be synchronized with Front::can_be_start_capture()

                CaptureFlags const capture_flags
                    = (this->ini.get<cfg::globals::is_rec>()
                    || this->ini.get<cfg::video::allow_rt_without_recording>()
                    )
                    ? this->ini.get<cfg::video::capture_flags>()
                    : CaptureFlags::none;

                if (bool(capture_flags & CaptureFlags::wrm)) {
                    this->ini.set_acl<cfg::context::recording_started>(true);
                }

                if (bool(capture_flags & CaptureFlags::png)
                    && this->ini.get<cfg::video::png_limit>() > 0
                    && !this->ini.get<cfg::context::rt_ready>()
                ){
                    this->ini.set_acl<cfg::context::rt_ready>(true);
                }

                return true;
            }

            return false;
        }
    };

    Inifile & ini;
    PidFile & pid_file;
    SessionVerbose verbose;

private:
    enum class EndSessionResult
    {
        close_box,
        retry,
        reconnection,
        redirection,
    };

    EndSessionResult end_session_exception(Error const& e, Inifile & ini, ModFactory const& mod_factory)
    {
        if (e.id == ERR_RAIL_LOGON_FAILED_OR_WARNING){
            ini.set_acl<cfg::context::session_probe_launch_error_message>(local_err_msg(e, language(ini)));
        }

        if (e.id == ERR_SESSION_PROBE_LAUNCH
         || e.id == ERR_SESSION_PROBE_ASBL_FSVC_UNAVAILABLE
         || e.id == ERR_SESSION_PROBE_ASBL_MAYBE_SOMETHING_BLOCKS
         || e.id == ERR_SESSION_PROBE_ASBL_UNKNOWN_REASON
         || e.id == ERR_SESSION_PROBE_CBBL_FSVC_UNAVAILABLE
         || e.id == ERR_SESSION_PROBE_CBBL_CBVC_UNAVAILABLE
         || e.id == ERR_SESSION_PROBE_CBBL_DRIVE_NOT_READY_YET
         || e.id == ERR_SESSION_PROBE_CBBL_MAYBE_SOMETHING_BLOCKS
         || e.id == ERR_SESSION_PROBE_CBBL_LAUNCH_CYCLE_INTERRUPTED
         || e.id == ERR_SESSION_PROBE_CBBL_UNKNOWN_REASON_REFER_TO_SYSLOG
         || e.id == ERR_SESSION_PROBE_RP_LAUNCH_REFER_TO_SYSLOG
        ) {
            if (ini.get<cfg::session_probe::on_launch_failure>() ==
                    SessionProbeOnLaunchFailure::retry_without_session_probe)
            {
                LOG(LOG_INFO, "Retry connection without session probe");
                ini.set<cfg::session_probe::enable_session_probe>(false);
                return EndSessionResult::retry;
            }
            this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
            return EndSessionResult::close_box;
        }

        if (e.id == ERR_SESSION_PROBE_DISCONNECTION_RECONNECTION) {
            LOG(LOG_INFO, "Retry Session Probe Disconnection Reconnection");
            return EndSessionResult::retry;
        }

        if (e.id == ERR_RAIL_RESIZING_REQUIRED) {
            return EndSessionResult::retry;
        }

        if (e.id == ERR_AUTOMATIC_RECONNECTION_REQUIRED) {
            LOG(LOG_INFO, "Retry Automatic Reconnection Required");
            return EndSessionResult::reconnection;
        }

        if (e.id == ERR_RAIL_NOT_ENABLED) {
            LOG(LOG_INFO, "Retry without native remoteapp capability");
            ini.set<cfg::mod_rdp::use_native_remoteapp_capability>(false);
            return EndSessionResult::retry;
        }

        if (e.id == ERR_RDP_SERVER_REDIR){
            if (ini.get<cfg::mod_rdp::server_redirection_support>()) {
                LOG(LOG_INFO, "Server redirection");
                return EndSessionResult::redirection;
            }
            else {
                LOG(LOG_ERR, "Start Session Failed: forbidden redirection = %s", e.errmsg());
                this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
                return EndSessionResult::close_box;
            }
        }

        if (e.id == ERR_SESSION_CLOSE_ENDDATE_REACHED){
            LOG(LOG_INFO, "Close because disconnection time reached");
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::session_out_time, language(this->ini)));
            return EndSessionResult::close_box;
        }

        if (e.id == ERR_MCS_APPID_IS_MCS_DPUM){
            LOG(LOG_INFO, "Remote Session Closed by User");
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::end_connection, language(this->ini)));
            return EndSessionResult::close_box;
        }

        if (e.id == ERR_SESSION_CLOSE_ACL_KEEPALIVE_MISSED) {
            LOG(LOG_INFO, "Close because of missed ACL keepalive");
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::miss_keepalive, language(this->ini)));
            return EndSessionResult::close_box;
        }

        if (e.id == ERR_SESSION_CLOSE_USER_INACTIVITY) {
            LOG(LOG_INFO, "Close because of user Inactivity");
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::close_inactivity, language(this->ini)));
            return EndSessionResult::close_box;
        }

        if (e.id == ERR_SESSION_CLOSE_MODULE_NEXT) {
            LOG(LOG_INFO, "Acl confirmed user close");
            return EndSessionResult::close_box;
        }

        if ((e.id == ERR_TRANSPORT_WRITE_FAILED || e.id == ERR_TRANSPORT_NO_MORE_DATA)
         && mod_factory.mod_sck_transport()
         && static_cast<uintptr_t>(mod_factory.mod_sck_transport()->get_fd()) == e.data
         && ini.get<cfg::mod_rdp::auto_reconnection_on_losing_target_link>()
         && mod_factory.mod().is_auto_reconnectable()
         && !mod_factory.mod().server_error_encountered()
        ) {
            LOG(LOG_INFO, "Session::end_session_exception: target link exception. %s",
                ERR_TRANSPORT_WRITE_FAILED == e.id
                    ? "ERR_TRANSPORT_WRITE_FAILED"
                    : "ERR_TRANSPORT_NO_MORE_DATA");
            return EndSessionResult::reconnection;
        }

        LOG(LOG_INFO,
            "ModTrans=<%p> Sock=%d AutoReconnection=%s AutoReconnectable=%s ErrorEncountered=%s",
            mod_factory.mod_sck_transport(),
            (mod_factory.mod_sck_transport() ? mod_factory.mod_sck_transport()->get_fd() : -1),
            (ini.get<cfg::mod_rdp::auto_reconnection_on_losing_target_link>() ? "Yes" : "No"),
            (mod_factory.mod().is_auto_reconnectable() ? "Yes" : "No"),
            (mod_factory.mod().server_error_encountered() ? "Yes" : "No")
            );

        this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
        return EndSessionResult::close_box;
    }

private:
    static bool is_target_module(ModuleName mod)
    {
        return mod == ModuleName::RDP
            || mod == ModuleName::VNC;
    }

    static bool is_close_module(ModuleName mod)
    {
        return mod == ModuleName::close
            || mod == ModuleName::close_back;
    }

    static void set_inactivity_timeout(Inactivity& inactivity, Inifile& ini)
    {
        if (is_target_module(ini.get<cfg::context::module>())) {
            auto const& inactivity_timeout
                = ini.get<cfg::globals::inactivity_timeout>();

            auto timeout = (inactivity_timeout != inactivity_timeout.zero())
                ? inactivity_timeout
                : ini.get<cfg::globals::base_inactivity_timeout>();

            inactivity.start(timeout);
        }
    }

    bool is_first_looping_on_mod_selector = true;
    std::string session_type;

    void next_backend_module(
        ModuleName next_state,
        SecondarySession& secondary_session,
        ModFactory& mod_factory,
        Inactivity& inactivity,
        KeepAlive& keepalive,
        SessionFront& front,
        GuestCtx& guest_ctx,
        EventManager& event_manager)
    {
        LOG_IF(bool(this->verbose & SessionVerbose::Trace),
            LOG_INFO, "Current Mod is %s Previous %s",
            get_module_name(mod_factory.mod_name()),
            get_module_name(next_state)
        );

        if (mod_factory.is_connected()) {
            LOG(LOG_INFO, "Exited from target connection");
            guest_ctx.stop();
            mod_factory.disconnect();
            if (!ini.get<cfg::context::has_more_target>()) {
                front.must_be_stop_capture();
                secondary_session.close_secondary_session();
            }
            else {
                LOG(LOG_INFO, "Keep existing capture & session log.");
            }
        }
        else {
            mod_factory.disconnect();
        }

        if (is_target_module(next_state)) {
            keepalive.start();
            event_manager.set_time_base(current_time_base());
            front.target_connection_start_time = event_manager.get_monotonic_time();
        }
        else {
            keepalive.stop();
            front.target_connection_start_time = MonotonicTimePoint();
        }

        if (next_state == ModuleName::INTERNAL) {
            next_state = get_internal_module_id_from_target(
                this->ini.get<cfg::context::target_host>()
            );
        }

        LOG(LOG_INFO, "New Module: %s", get_module_name(next_state));

        auto open_secondary_session = [&](SecondarySession::Type secondary_session_type){
            log_siem::set_user(this->ini.get<cfg::globals::auth_user>());
            try {
                switch (secondary_session_type)
                {
                    case SecondarySession::Type::RDP:
                    {
                        SessionLogApi& session_log_api =
                              this->ini.get<cfg::context::try_alternate_target>()
                            ? secondary_session.get_secondary_session_log()
                            : secondary_session.open_secondary_session(secondary_session_type);

                        mod_factory.create_rdp_mod(
                            session_log_api,
                            PerformAutomaticReconnection::No);
                        break;
                    }
                    case SecondarySession::Type::VNC:
                        mod_factory.create_vnc_mod(
                            secondary_session.open_secondary_session(secondary_session_type));
                        break;
                }
                this->ini.set<cfg::context::auth_error_message>("");
                set_inactivity_timeout(inactivity, ini);
                return;
            }
            catch (Error const& /*error*/) {
                this->secondary_session_creation_failed(secondary_session, this->ini.get<cfg::context::has_more_target>());
                mod_factory.create_transition_mod();
            }
            catch (...) {
                this->secondary_session_creation_failed(secondary_session, this->ini.get<cfg::context::has_more_target>());
                throw;
            }
        };

        switch (next_state)
        {
        case ModuleName::RDP:
            open_secondary_session(SecondarySession::Type::RDP);
            break;

        case ModuleName::VNC:
            open_secondary_session(SecondarySession::Type::VNC);
            break;

        case ModuleName::close:
            mod_factory.create_close_mod();
            inactivity.stop();
            break;

        case ModuleName::close_back:
            mod_factory.create_close_mod_back_to_selector();
            inactivity.stop();
            break;

        case ModuleName::login:
            log_siem::set_user("");
            inactivity.stop();
            mod_factory.create_login_mod();
            break;

        case ModuleName::waitinfo:
            log_siem::set_user("");
            inactivity.stop();
            mod_factory.create_wait_info_mod();
            break;

        case ModuleName::confirm:
            log_siem::set_user("");
            inactivity.start(this->ini.get<cfg::globals::base_inactivity_timeout>());
            mod_factory.create_display_message_mod();
            break;

        case ModuleName::link_confirm:
            log_siem::set_user("");
            if (auto timeout = this->ini.get<cfg::context::mod_timeout>()
                ; timeout.count() != 0
            ) {
                inactivity.start(timeout);
            }
            else {
                inactivity.start(this->ini.get<cfg::globals::base_inactivity_timeout>());
            }
            mod_factory.create_display_link_mod();
            break;

        case ModuleName::valid:
            log_siem::set_user("");
            inactivity.start(this->ini.get<cfg::globals::base_inactivity_timeout>());
            mod_factory.create_valid_message_mod();
            break;

        case ModuleName::challenge:
            log_siem::set_user("");
            inactivity.start(this->ini.get<cfg::globals::base_inactivity_timeout>());
            mod_factory.create_dialog_challenge_mod();
            break;

        case ModuleName::selector:
            inactivity.start(this->ini.get<cfg::globals::base_inactivity_timeout>());

            if (this->is_first_looping_on_mod_selector) {
                this->is_first_looping_on_mod_selector = false;
                switch (this->ini.get<cfg::translation::language>())
                {
                    case Language::en:
                        this->ini.set_acl<cfg::translation::login_language>(LoginLanguage::EN);
                        break;
                    case Language::fr:
                        this->ini.set_acl<cfg::translation::login_language>(LoginLanguage::FR);
                        break;
                }
            }
            log_siem::set_user(this->ini.get<cfg::globals::auth_user>());
            mod_factory.create_selector_mod();
            break;

        case ModuleName::bouncer2:
            log_siem::set_user(this->ini.get<cfg::globals::auth_user>());
            mod_factory.create_mod_bouncer();
            break;

        case ModuleName::autotest:
            inactivity.stop();
            log_siem::set_user(this->ini.get<cfg::globals::auth_user>());
            mod_factory.create_mod_replay();
            break;

        case ModuleName::widgettest:
            log_siem::set_user(this->ini.get<cfg::globals::auth_user>());
            mod_factory.create_widget_test_mod();
            break;

        case ModuleName::card:
            log_siem::set_user(this->ini.get<cfg::globals::auth_user>());
            mod_factory.create_test_card_mod();
            break;

        case ModuleName::interactive_target:
            log_siem::set_user(this->ini.get<cfg::globals::auth_user>());
            inactivity.start(this->ini.get<cfg::globals::base_inactivity_timeout>());
            mod_factory.create_interactive_target_mod();
            break;

        case ModuleName::transitory:
            log_siem::set_user(this->ini.get<cfg::globals::auth_user>());
            mod_factory.create_transition_mod();
            break;

        case ModuleName::INTERNAL:
        case ModuleName::UNKNOWN:
            LOG(LOG_INFO, "ModuleManager::Unknown backend exception %u", unsigned(next_state));
            throw Error(ERR_SESSION_UNKNOWN_BACKEND);
        }
    }

    void secondary_session_creation_failed(SecondarySession & secondary_session, bool has_other_target_to_try)
    {
        secondary_session.get_secondary_session_log().log6(LogId::SESSION_CREATION_FAILED, {});
        if (!has_other_target_to_try) {
            secondary_session.close_secondary_session();
        }
        else {
            LOG(LOG_INFO, "Keep existing session log.");
        }
        this->ini.set_acl<cfg::context::module>(ModuleName::close);
    }

    bool retry_rdp(
        SecondarySession & secondary_session, ModFactory & mod_factory,
        SessionFront & front, PerformAutomaticReconnection perform_automatic_reconnection)
    {
        LOG(LOG_INFO, "Retry RDP");

        if (mod_factory.mod_name() != ModuleName::RDP) {
            LOG(LOG_ERR, "Previous module is %s, RDP is expected",
                get_module_name(mod_factory.mod_name()));
            throw Error(ERR_SESSION_CLOSE_MODULE_NEXT);
        }

        log_siem::set_user(this->ini.get<cfg::globals::auth_user>());

        mod_factory.disconnect();

        SessionLogApi& session_log = secondary_session.get_secondary_session_log();
        try {
            front.target_connection_start_time = MonotonicTimePoint::clock::now();
            mod_factory.create_rdp_mod(session_log, perform_automatic_reconnection);
            this->ini.set<cfg::context::auth_error_message>("");
            return true;
        }
        catch (Error const& /*error*/) {
            front.must_be_stop_capture();
            this->secondary_session_creation_failed(secondary_session, false);
            mod_factory.create_transition_mod();
        }
        catch (...) {
            front.must_be_stop_capture();
            this->secondary_session_creation_failed(secondary_session, false);
        }

        return false;
    }

    struct NextDelay
    {
        NextDelay(bool nodelay, EventManager& event_manager)
        {
            if (!nodelay) {
                event_manager.get_writable_time_base().monotonic_time
                    = MonotonicTimePoint::clock::now();
                auto timeout = event_manager.next_timeout();
                // 0 means no timeout to trigger
                if (timeout != MonotonicTimePoint{}) {
                    auto now = MonotonicTimePoint::clock::now();
                    this->tv = (now < timeout)
                        ? to_timeval(timeout - now)
                        // no delay
                        : timeval{};
                    this->r = &this->tv;
                }
            }
            else {
                this->tv = {0, 0};
                this->r = &this->tv;
            }
        }

        timeval* timeout() const { return this->r; }

    private:
        timeval tv;
        timeval* r = nullptr;
    };

    static inline void front_process(
        TpduBuffer& buffer, SessionFront& front, InTransport front_trans,
        Callback& callback)
    {
        buffer.load_data(front_trans);
        while (buffer.next(TpduBuffer::PDU)) // or TdpuBuffer::CredSSP in NLA
        {
            bytes_view tpdu = buffer.current_pdu_buffer();
            uint8_t current_pdu_type = buffer.current_pdu_get_type();
            front.incoming(tpdu, current_pdu_type, callback);
        }
    }

    template<class Fn>
    bool internal_front_loop(
        SessionFront& front,
        SocketTransport& front_trans,
        TpduBuffer& rbuf,
        EventManager& event_manager,
        Callback& callback,
        Fn&& stop_event)
    {
        const int sck = front_trans.get_fd();
        const int max = sck;

        fd_set fds;
        io_fd_zero(fds);

        for (;;) {
            bool const has_data_to_write = front_trans.has_data_to_write();
            bool const has_tls_pending_data = front_trans.has_tls_pending_data();

            io_fd_set(sck, fds);

            int const num = ::select(max+1,
                has_data_to_write ? nullptr : &fds,
                has_data_to_write ? &fds : nullptr,
                nullptr, NextDelay(has_tls_pending_data, event_manager).timeout());

            if (num < 0) {
                if (errno != EINTR) {
                    return false;
                }
                continue;
            }

            event_manager.set_time_base(current_time_base());
            event_manager.execute_events(
                [](int /*fd*/){ assert(false); return false; },
                bool(this->verbose & SessionVerbose::Event));

            if (num) {
                assert(io_fd_isset(sck, fds));

                if (has_data_to_write) {
                    front_trans.send_waiting_data();
                }
                else {
                    front_process(rbuf, front, front_trans, callback);
                }
            }
            else if (has_tls_pending_data) {
                front_process(rbuf, front, front_trans, callback);
            }

            if (stop_event()) {
                return true;
            }
        }
    }

    void acl_auth_info(ClientInfo const& client_info)
    {
        LOG(LOG_INFO, "Session: Keyboard Layout = 0x%x", client_info.keylayout);
        this->ini.set_acl<cfg::client::keyboard_layout>(safe_int(client_info.keylayout));

        this->ini.set_acl<cfg::context::opt_width>(client_info.screen_info.width);
        this->ini.set_acl<cfg::context::opt_height>(client_info.screen_info.height);
        this->ini.set_acl<cfg::context::opt_bpp>(safe_int(client_info.screen_info.bpp));

        std::string username = client_info.username;
        std::string_view domain = client_info.domain;
        std::string_view password = client_info.password;
        if (not domain.empty()
         && username.find('@') == std::string::npos
         && username.find('\\') == std::string::npos
        ) {
            str_append(username, '@', domain);
        }

        LOG_IF(bool(this->verbose & SessionVerbose::Trace), LOG_INFO,
            "Session::flush_acl_auth_info(auth_user=%s)", username);

        this->ini.set_acl<cfg::globals::auth_user>(username);
        this->ini.ask<cfg::context::selector>();
        this->ini.ask<cfg::globals::target_user>();
        this->ini.ask<cfg::globals::target_device>();
        this->ini.ask<cfg::context::target_protocol>();
        if (!password.empty()) {
            this->ini.set_acl<cfg::context::password>(password);
        }
    }

    enum class EndLoopState
    {
        ImmediateDisconnection,
        ShowCloseBox
    };

    inline EndLoopState main_loop(
        int auth_sck, EventManager& event_manager,
        CryptoContext& cctx, UdevRandom& rnd,
        TpduBuffer& rbuf, SocketTransport& front_trans,
        SessionFront& front, GuestCtx& guest_ctx,
        ModFactory& mod_factory
    )
    {
        LOG_IF(bool(this->verbose & SessionVerbose::Trace),
            LOG_INFO, "Session: Main loop");

        assert(auth_sck != INVALID_SOCKET);

        FinalSocketTransport auth_trans(
            "Authentifier"_sck_name, unique_fd(auth_sck),
            ini.get<cfg::globals::authfile>(), 0,
            std::chrono::milliseconds(1000),
            std::chrono::milliseconds::zero(),
            std::chrono::milliseconds(1000),
            SocketTransport::Verbose::none);

        auto& events = event_manager.get_events();
        EndSessionWarning end_session_warning(events);

        KeepAlive keepalive(ini, events, ini.get<cfg::globals::keepalive_grace_delay>());
        Inactivity inactivity(events);

        AclSerializer acl_serial(ini, auth_trans);

        SecondarySession secondary_session(ini, cctx, rnd, front, event_manager.get_time_base());

        enum class LoopState
        {
            AclSend,
            Select,
            Front,
            AclReceive,
            AclUpdate,
            EventLoop,
            BackEvent,
            NextMod,
            UpdateOsd,
        };

        ModuleName next_module = ModuleName::UNKNOWN;

        for (;;) {
            assert(front_trans.get_fd() != INVALID_SOCKET);
            assert(auth_trans.get_fd() != INVALID_SOCKET);

            auto loop_state = LoopState::AclSend;

            try {
                acl_serial.send_acl_data();

                loop_state = LoopState::Select;

                auto* pmod_trans = mod_factory.mod_sck_transport();
                const bool front_has_data_to_write     = front_trans.has_data_to_write();
                const bool front_has_tls_pending_data  = front_trans.has_tls_pending_data();
                const bool front2_has_data_to_write    = guest_ctx.has_front()
                                                      && guest_ctx.front_transport().has_data_to_write();
                const bool front2_has_tls_pending_data = guest_ctx.has_front()
                                                      && guest_ctx.front_transport().has_tls_pending_data();
                const bool mod_trans_is_valid          = pmod_trans
                                                      && pmod_trans->get_fd() != INVALID_SOCKET;
                const bool mod_has_data_to_write       = mod_trans_is_valid
                                                      && pmod_trans->has_data_to_write();
                const bool mod_has_tls_pending_data    = mod_trans_is_valid
                                                      && pmod_trans->has_tls_pending_data();

                Select ioswitch;

                // writing pending, do not read anymore (excepted acl)
                if (REDEMPTION_UNLIKELY(mod_has_data_to_write
                                     || front_has_data_to_write
                                     || front2_has_data_to_write)
                ) {
                    if (front_has_data_to_write) {
                        LOG_IF(bool(this->verbose & SessionVerbose::Trace),
                            LOG_INFO, "Session: Front has data to write");
                        ioswitch.set_write_sck(front_trans.get_fd());
                    }

                    if (front2_has_data_to_write) {
                        LOG_IF(bool(this->verbose & SessionVerbose::Trace),
                            LOG_INFO, "Session: Front2 has data to write");
                        ioswitch.set_write_sck(guest_ctx.front_transport().get_fd());
                    }

                    if (mod_has_data_to_write) {
                        LOG_IF(bool(this->verbose & SessionVerbose::Trace),
                            LOG_INFO, "Session: Mod has data to write");
                        ioswitch.set_write_sck(pmod_trans->get_fd());
                    }
                }
                else {
                    ioswitch.set_read_sck(front_trans.get_fd());
                    event_manager.for_each_fd([&](int fd){ ioswitch.set_read_sck(fd); });
                }
                ioswitch.set_read_sck(auth_sck);

                if (ioswitch.select(NextDelay(
                    mod_has_tls_pending_data || front_has_tls_pending_data || front2_has_tls_pending_data,
                    event_manager
                ).timeout()) < 0) {
                    if (errno != EINTR) {
                        // Cope with EBADF, EINVAL, ENOMEM : none of these should ever happen
                        // EBADF: means fd has been closed (by me) or as already returned an error on another call
                        // EINVAL: invalid value in timeout (my fault again)
                        // ENOMEM: no enough memory in kernel (unlikely fort 3 sockets)
                        LOG(LOG_ERR, "Proxy data wait loop raised error %d: %s",
                            errno, strerror(errno));
                        break;
                    }
                    continue;
                }

                if (front2_has_tls_pending_data) {
                    LOG_IF(bool(this->verbose & SessionVerbose::Trace),
                        LOG_INFO, "Session: Front2 has tls pending data");
                    ioswitch.set_read_sck(guest_ctx.front_transport().get_fd());
                }

                if (front_has_tls_pending_data) {
                    LOG_IF(bool(this->verbose & SessionVerbose::Trace),
                        LOG_INFO, "Session: Front has tls pending data");
                    ioswitch.set_read_sck(front_trans.get_fd());
                }

                if (mod_has_tls_pending_data) {
                    LOG_IF(bool(this->verbose & SessionVerbose::Trace),
                        LOG_INFO, "Session: Mod has tls pending data");
                    ioswitch.set_read_sck(pmod_trans->get_fd());
                }

                // update times and synchronize real time
                {
                    auto old_time_base = event_manager.get_time_base();
                    auto new_time_base = current_time_base();

                    event_manager.set_time_base(new_time_base);

                    constexpr auto max_delay = MonotonicTimePoint::duration(1s);
                    MonotonicTimePoint::duration monotonic_elpased
                        = new_time_base.monotonic_time - old_time_base.monotonic_time;
                    MonotonicTimePoint::duration real_elpased
                        = new_time_base.real_time - old_time_base.real_time;

                    if (abs(real_elpased) >= monotonic_elpased + max_delay) {
                        front.must_synchronize_times_capture(new_time_base.monotonic_time, new_time_base.real_time);
                        end_session_warning.add_delay(real_elpased);
                    }
                }

                loop_state = LoopState::Front;

                if (REDEMPTION_UNLIKELY(front_has_data_to_write || front2_has_data_to_write)) {
                    if (front_has_data_to_write) {
                        if (ioswitch.is_set_for_writing(front_trans.get_fd())) {
                            front_trans.send_waiting_data();
                        }
                    }

                    if (front2_has_data_to_write) {
                        if (ioswitch.is_set_for_writing(guest_ctx.front_transport().get_fd())) {
                            front_trans.send_waiting_data();
                        }
                    }
                }
                else if (ioswitch.is_set_for_reading(front_trans.get_fd())) {
                    auto& callback = mod_factory.callback();
                    front_process(rbuf, front, front_trans, callback);

                    // TODO should be replaced by callback.rdp_gdi_up/down() when is_up_and_running changes
                    if (front.front_must_notify_resize) {
                        LOG(LOG_INFO, "Notify resize to front");
                        front.notify_resize(callback);
                    }
                }


                loop_state = LoopState::AclReceive;

                BackEvent_t back_event = BACK_EVENT_NONE;

                if (ioswitch.is_set_for_reading(auth_sck)) {
                    AclFieldMask updated_fields = acl_serial.incoming();

                    loop_state = LoopState::AclUpdate;

                    AclFieldMask owned_fields {};
                    auto has_field = [&](auto field){
                        using Field = decltype(field);
                        owned_fields.set(Field::index);
                        return updated_fields.has<Field>();
                    };

                    if (has_field(cfg::context::session_id())) {
                        this->pid_file.rename(this->ini.get<cfg::context::session_id>());
                    }

                    if (has_field(cfg::context::module())) {
                        if ((ini.get<cfg::context::module>() != mod_factory.mod_name())
                         || ini.get<cfg::context::try_alternate_target>()) {
                            next_module = ini.get<cfg::context::module>();
                            back_event = BACK_EVENT_NEXT;
                        }
                    }

                    if (has_field(cfg::context::keepalive())) {
                        keepalive.keep_alive();
                    }

                    if (has_field(cfg::context::end_date_cnx())) {
                        auto time_base = current_time_base();
                        event_manager.set_time_base(time_base);
                        const auto sys_date = time_base.real_time.time_since_epoch();
                        auto const elapsed = ini.get<cfg::context::end_date_cnx>() - sys_date;
                        auto const new_end_date = time_base.monotonic_time + elapsed;
                        end_session_warning.set_time(new_end_date);
                        mod_factory.set_time_close(new_end_date);
                    }

                    if (has_field(cfg::globals::inactivity_timeout())) {
                        set_inactivity_timeout(inactivity, ini);
                    }

                    if (has_field(cfg::audit::rt_display())) {
                        const Capture::RTDisplayResult rt_status =
                            front.set_rt_display(ini.get<cfg::audit::rt_display>(),
                                                 redis_params_from_ini(ini));

                        if (ini.get<cfg::client::enable_osd_4_eyes>()
                         && rt_status == Capture::RTDisplayResult::Enabled
                        ) {
                            zstring_view msg = TR(trkeys::enable_rt_display, language(ini));
                            mod_factory.display_osd_message(msg.to_sv());
                        }
                    }

                    if (has_field(cfg::context::session_sharing_enable_control())) {
                        GuestCtx::ResultError result = {0xffff, "Sharing not available"};

                        if (front.is_up_and_running()
                         && mod_factory.is_connected()
                         && mod_factory.mod().is_up_and_running()
                        ) {
                            if (guest_ctx.is_started()) {
                                guest_ctx.stop();
                            }

                            result = guest_ctx.start(
                                app_path(AppPath::SessionTmpDir),
                                this->ini.get<cfg::context::session_id>(),
                                events, front, mod_factory.callback(),
                                secondary_session.get_secondary_session_log(),
                                this->ini.get<cfg::context::session_sharing_ttl>(), rnd, ini,
                                this->ini.get<cfg::context::session_sharing_enable_control>()
                            );
                        }

                        this->ini.send<cfg::context::session_sharing_userdata>();
                        this->ini.set_acl<cfg::context::session_sharing_invitation_error_code>(result.errnum);
                        if (result.errnum) {
                            this->ini.set_acl<cfg::context::session_sharing_invitation_error_message>(result.errmsg);
                        }
                        else {
                            this->ini.set_acl<cfg::context::session_sharing_invitation_addr>(guest_ctx.sck_path());
                            this->ini.set_acl<cfg::context::session_sharing_invitation_id>(guest_ctx.sck_password());
                        }
                    }

                    if (has_field(cfg::context::rejected())) {
                        LOG(LOG_ERR, "Connection is rejected by Authentifier! Reason: %s",
                            this->ini.get<cfg::context::rejected>().c_str());
                        this->ini.set<cfg::context::auth_error_message>(
                            this->ini.get<cfg::context::rejected>());
                        next_module = ModuleName::close;
                        back_event = BACK_EVENT_NEXT;
                    }
                    else if (has_field(cfg::context::disconnect_reason())) {
                        this->ini.set<cfg::context::auth_error_message>(
                            this->ini.get<cfg::context::disconnect_reason>());
                        this->ini.set_acl<cfg::context::disconnect_reason_ack>(true);
                        back_event = std::max(BACK_EVENT_NEXT, mod_factory.mod().get_mod_signal());
                    }
                    else if (!back_event) {
                        updated_fields.clear(owned_fields);
                        if (!updated_fields.is_empty()
                         && (next_module == ModuleName::UNKNOWN
                          || next_module == mod_factory.mod_name()
                        )) {
                            auto& mod = mod_factory.mod();
                            mod.acl_update(updated_fields);
                            back_event = std::max(back_event, mod.get_mod_signal());
                        }
                    }
                }


                loop_state = LoopState::EventLoop;

                if (!back_event) {
                    if (REDEMPTION_UNLIKELY(mod_has_data_to_write)) {
                        pmod_trans = mod_factory.mod_sck_transport();
                        if (pmod_trans && ioswitch.is_set_for_writing(pmod_trans->get_fd())) {
                            pmod_trans->send_waiting_data();
                        }
                    }

                    event_manager.execute_events(
                        [&ioswitch](int fd){ return ioswitch.is_set_for_reading(fd); },
                        bool(this->verbose & SessionVerbose::Event));

                    back_event = mod_factory.mod().get_mod_signal();
                }
                else {
                    event_manager.execute_events(
                        [](int /*fd*/){ return false; },
                        bool(this->verbose & SessionVerbose::Event));
                }


                loop_state = LoopState::BackEvent;

                if (REDEMPTION_UNLIKELY(back_event)) {
                    if (back_event == BACK_EVENT_STOP) {
                        LOG(LOG_INFO, "Module asked Front Disconnection");
                        break;
                    }

                    assert(back_event == BACK_EVENT_NEXT);

                    if (next_module == ModuleName::UNKNOWN) {
                        if (mod_factory.is_connected()) {
                            next_module = ModuleName::close;
                            this->ini.set_acl<cfg::context::module>(ModuleName::close);
                        }
                        else {
                            next_module = ModuleName::transitory;
                        }
                    }
                }


                loop_state = LoopState::NextMod;

                if (REDEMPTION_UNLIKELY(next_module != ModuleName::UNKNOWN)) {
                    if (is_close_module(next_module)) {
                        if (!ini.get<cfg::globals::enable_close_box>()) {
                            LOG(LOG_INFO, "Close Box disabled: ending session");
                            break;
                        }
                    }

                    this->next_backend_module(
                        std::exchange(next_module, ModuleName::UNKNOWN),
                        secondary_session, mod_factory, inactivity,
                        keepalive, front, guest_ctx, event_manager);
                }


                if (front.is_up_and_running()) {
                    if (front.has_user_activity) {
                        inactivity.activity();
                        front.has_user_activity = false;
                    }

                    end_session_warning.update_warning([&](std::chrono::minutes minutes){
                        if (ini.get<cfg::globals::enable_osd>()
                         && mod_factory.mod().is_up_and_running()
                        ) {
                            loop_state = LoopState::UpdateOsd;
                            auto lang = language(ini);
                            mod_factory.display_osd_message(str_concat(
                                int_to_decimal_chars(minutes.count()),
                                ' ',
                                TR(trkeys::minute, lang),
                                (minutes.count() > 1) ? "s " : " ",
                                TR(trkeys::before_closing, lang)
                            ));
                        }
                    });
                }
            }
            catch (Error const& e)
            {
                bool run_session = false;

                switch (loop_state)
                {
                case LoopState::AclSend:
                    LOG(LOG_ERR, "ACL SERIALIZER: %s", e.errmsg());
                    ini.set<cfg::context::auth_error_message>(
                        TR(trkeys::acl_fail, language(ini)));
                    auth_trans.disconnect();
                    break;

                case LoopState::AclReceive:
                    LOG(LOG_INFO, "acl_serial.incoming() Session lost");
                    ini.set<cfg::context::auth_error_message>(
                        TR(trkeys::manager_close_cnx, language(ini)));
                    auth_trans.disconnect();
                    break;

                case LoopState::Select:
                    break;

                case LoopState::Front:
                    if (e.id == ERR_TRANSPORT_WRITE_FAILED
                     || e.id == ERR_TRANSPORT_NO_MORE_DATA
                    ) {
                        SocketTransport* socket_transport_ptr = mod_factory.mod_sck_transport();

                        if (socket_transport_ptr
                         && e.data == static_cast<uintptr_t>(socket_transport_ptr->get_fd())
                         && ini.get<cfg::mod_rdp::auto_reconnection_on_losing_target_link>()
                         && mod_factory.mod().is_auto_reconnectable()
                         && !mod_factory.mod().server_error_encountered())
                        {
                            LOG(LOG_INFO, "Session::Session: target link exception. %s",
                                (ERR_TRANSPORT_WRITE_FAILED == e.id)
                                    ? "ERR_TRANSPORT_WRITE_FAILED"
                                    : "ERR_TRANSPORT_NO_MORE_DATA"
                            );

                            run_session = this->retry_rdp(
                                secondary_session, mod_factory,
                                front, PerformAutomaticReconnection::Yes);
                        }
                    }
                    else {
                        // RemoteApp disconnection initiated by user
                        // ERR_DISCONNECT_BY_USER == e.id
                        if (// Can be caused by client disconnect.
                            e.id != ERR_X224_RECV_ID_IS_RD_TPDU
                         && e.id != ERR_MCS_APPID_IS_MCS_DPUM
                         && e.id != ERR_RDP_HANDSHAKE_TIMEOUT
                         // Can be caused by wabwatchdog.
                         && e.id != ERR_TRANSPORT_NO_MORE_DATA
                        ) {
                            LOG(LOG_ERR, "Proxy data processing raised error %u: %s",
                                e.id, e.errmsg(false));
                        }

                        front_trans.disconnect();
                    }

                    break;

                case LoopState::NextMod:
                    break;

                case LoopState::AclUpdate:
                case LoopState::EventLoop:
                case LoopState::BackEvent:
                case LoopState::UpdateOsd:
                    if (!front.is_up_and_running()) {
                        break;
                    }

                    switch (end_session_exception(e, ini, mod_factory))
                    {
                    case EndSessionResult::close_box:
                        if (ini.get<cfg::globals::enable_close_box>()) {
                            if (!is_close_module(mod_factory.mod_name())) {
                                if (mod_factory.is_connected()) {
                                    this->ini.set_acl<cfg::context::module>(ModuleName::close);
                                }
                                this->next_backend_module(
                                    ModuleName::close, secondary_session, mod_factory,
                                    inactivity, keepalive, front, guest_ctx,
                                    event_manager);
                                run_session = true;
                            }
                        }
                        else {
                            LOG(LOG_INFO, "Close Box disabled : ending session");
                        }
                        break;

                    case EndSessionResult::redirection: {
                        // SET new target in ini
                        auto& redir_info = mod_factory.get_redir_info();
                        const char * host = char_ptr_cast(redir_info.host);
                        const char * username = char_ptr_cast(redir_info.username);
                        const char * change_user = "";
                        if (redir_info.dont_store_username && username[0] != 0) {
                            LOG(LOG_INFO, "SrvRedir: Change target username to '%s'", username);
                            ini.set_acl<cfg::globals::target_user>(username);
                            change_user = username;
                        }
                        if (redir_info.password_or_cookie.size())
                        {
                            LOG(LOG_INFO, "SrvRedir: password or cookie");
                            std::vector<uint8_t>& redirection_password_or_cookie =
                                ini.get_mutable_ref<cfg::context::redirection_password_or_cookie>();

                            redirection_password_or_cookie = std::move(redir_info.password_or_cookie);
                        }
                        LOG(LOG_INFO, "SrvRedir: Change target host to '%s'", host);
                        ini.set_acl<cfg::context::target_host>(host);
                        auto message = str_concat(change_user, '@', host);
                        secondary_session.report("SERVER_REDIRECTION", message.c_str());
                    }
                    [[fallthrough]];

                    // TODO: should we put some counter to avoid retrying indefinitely?
                    case EndSessionResult::retry:
                        run_session = this->retry_rdp(
                            secondary_session, mod_factory,
                            front, PerformAutomaticReconnection::No);
                        break;

                    // TODO: should we put some counter to avoid retrying indefinitely?
                    case EndSessionResult::reconnection:
                        run_session = this->retry_rdp(
                            secondary_session, mod_factory,
                            front, PerformAutomaticReconnection::Yes);
                        break;
                    }

                    break;
                }

                if (!run_session) {
                    break;
                }
            }
            catch (...)
            {
                switch (loop_state)
                {
                    case LoopState::AclSend:
                        LOG(LOG_ERR, "ACL SERIALIZER: unknown error");
                        ini.set<cfg::context::auth_error_message>(
                            TR(trkeys::acl_fail, language(ini)));
                        auth_trans.disconnect();
                        break;

                    case LoopState::AclReceive:
                        LOG(LOG_INFO, "acl_serial.incoming() Session lost");
                        ini.set<cfg::context::auth_error_message>(
                            TR(trkeys::manager_close_cnx, language(ini)));
                        auth_trans.disconnect();
                        break;

                    case LoopState::Select:
                        break;

                    case LoopState::Front:
                        LOG(LOG_ERR, "Proxy data processing raised an unknown error");
                        break;

                    case LoopState::NextMod:
                    case LoopState::AclUpdate:
                    case LoopState::EventLoop:
                    case LoopState::BackEvent:
                    case LoopState::UpdateOsd:
                        break;
                }

                break;
            }
        }

        try { acl_serial.send_acl_data(); }
        catch (...) {}

        guest_ctx.stop();

        const bool show_close_box = auth_trans.get_fd() == INVALID_SOCKET;
        if (!show_close_box || mod_factory.mod_name() != ModuleName::close) {
            mod_factory.disconnect();
        }
        front.must_be_stop_capture();

        return show_close_box && mod_factory.mod().get_mod_signal() == BACK_EVENT_NONE
             ? EndLoopState::ShowCloseBox
             : EndLoopState::ImmediateDisconnection;
    }

    static TimeBase current_time_base()
    {
        return TimeBase{
            MonotonicTimePoint::clock::now(),
            std::chrono::system_clock::now()
        };
    }

public:
    Session(SocketTransport&& front_trans, MonotonicTimePoint sck_start_time, Inifile& ini, PidFile& pid_file, Font const& font, bool prevent_early_log)
    : ini(ini)
    , pid_file(pid_file)
    , verbose(safe_cast<SessionVerbose>(ini.get<cfg::debug::session>()))
    {
        CryptoContext cctx;
        UdevRandom rnd;

        EventManager event_manager;
        event_manager.set_time_base(current_time_base());

        AclReport acl_report{ini};
        SessionFront front(event_manager.get_events(), acl_report, front_trans, rnd, ini, cctx);

        TpduBuffer rbuf;
        int auth_sck = INVALID_SOCKET;

        LOG_IF(bool(this->verbose & SessionVerbose::Trace),
            LOG_INFO, "Session: Wait front.is_up_and_running()");

        try {
            null_mod no_mod;
            bool is_connected = this->internal_front_loop(
                front, front_trans, rbuf, event_manager, no_mod,
                [&]{
                    return front.is_up_and_running();
                });

            if (is_connected) {
                if (unique_fd client_sck = addr_connect_blocking(
                    ini.get<cfg::globals::authfile>().c_str(),
                    ini.get<cfg::all_target_mod::connection_establishment_timeout>(),
                    prevent_early_log)
                ) {
                    auth_sck = client_sck.release();

                    ini.set_acl<cfg::globals::front_connection_time>(
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            MonotonicTimePoint::clock::now() - sck_start_time));
                }
                else {
                    ini.set<cfg::context::auth_error_message>("No authentifier available");
                }
            }
        }
        catch (Error const& e) {
            // RemoteApp disconnection initiated by user
            // ERR_DISCONNECT_BY_USER == e.id
            if (// Can be caused by client disconnect.
                e.id != ERR_X224_RECV_ID_IS_RD_TPDU
             && e.id != ERR_MCS_APPID_IS_MCS_DPUM
             && e.id != ERR_RDP_HANDSHAKE_TIMEOUT
                // Can be caused by wabwatchdog.
             && e.id != ERR_TRANSPORT_NO_MORE_DATA
            ) {
                LOG(LOG_ERR, "Proxy data processing raised error %u : %s", e.id, e.errmsg(false));
            }
            return ;
        }
        catch (const std::exception & e) {
            LOG(LOG_ERR, "Proxy data processing raised error %s!", e.what());
            return ;
        }
        catch (...) {
            LOG(LOG_ERR, "Proxy data processing raised an unknown error");
            return ;
        }

        if (auth_sck == INVALID_SOCKET
         && (!front.is_up_and_running() || !ini.get<cfg::globals::enable_close_box>())
        ) {
            // silent message for localhost or probe IPs for watchdog
            if (!prevent_early_log) {
                log_siem::disconnection(ini.get<cfg::context::auth_error_message>().c_str());
            }

            return ;
        }

        this->acl_auth_info(front.get_client_info());

        try {
            ModFactory mod_factory(
                event_manager.get_events(), event_manager.get_time_base(),
                front.get_client_info(), front, front, front.get_palette(),
                font, ini, front.keymap, rnd, cctx);

            GuestCtx guest_ctx;

            auto end_loop = EndLoopState::ShowCloseBox;

            if (auth_sck != INVALID_SOCKET) {
                end_loop = this->main_loop(
                    auth_sck, event_manager, cctx, rnd,
                    rbuf, front_trans, front, guest_ctx,
                    mod_factory
                );
            }

            if (end_loop == EndLoopState::ShowCloseBox
             && front.is_up_and_running()
             && ini.get<cfg::globals::enable_close_box>()
            ) {
                if (mod_factory.mod_name() != ModuleName::close) {
                    mod_factory.create_close_mod();
                }

                auto& mod = mod_factory.mod();
                this->internal_front_loop(
                    front, front_trans, rbuf, event_manager, mod,
                    [&]{
                        return mod.get_mod_signal() != BACK_EVENT_NONE
                            || !front.is_up_and_running();
                    });
            }

            if (front.is_up_and_running()) {
                front.disconnect();
            }
        }
        catch (Error const& e) {
            if (e.id != ERR_TRANSPORT_WRITE_FAILED) {
                LOG(LOG_INFO, "Session Init exception %s", e.errmsg());
            }
        }
        catch (const std::exception & e) {
            LOG(LOG_ERR, "Session exception %s!", e.what());
        }
        catch (...) {
            LOG(LOG_ERR, "Session unexpected exception");
        }

        if (!ini.is_asked<cfg::globals::host>()) {
            LOG(LOG_INFO, "Client Session Disconnected");
        }
        log_siem::disconnection(ini.get<cfg::context::auth_error_message>().c_str());

        front.must_be_stop_capture();
    }

    Session(Session const &) = delete;
};

template<class SocketType, class... Args>
void session_start_sck(
    SocketTransport::Name name, unique_fd&& sck,
    MonotonicTimePoint sck_start_time, Inifile& ini,
    PidFile& pid_file, Font const& font, bool prevent_early_log,
    Args&&... args)
{
    auto const watchdog_verbosity = prevent_early_log
        ? SocketTransport::Verbose::watchdog
        : SocketTransport::Verbose();
    auto const sck_verbosity = safe_cast<SocketTransport::Verbose>(
        ini.get<cfg::debug::sck_front>());

    Session session(
        SocketType(
            name, std::move(sck), ""_av, 0,
            ini.get<cfg::all_target_mod::connection_establishment_timeout>(),
            ini.get<cfg::all_target_mod::tcp_user_timeout>(),
            ini.get<cfg::client::recv_timeout>(),
            static_cast<Args&&>(args)..., sck_verbosity | watchdog_verbosity
        ),
        sck_start_time, ini, pid_file, font, prevent_early_log
    );
}

} // anonymous namespace

void session_start_tls(unique_fd sck, MonotonicTimePoint sck_start_time, Inifile& ini, PidFile& pid_file, Font const& font, bool prevent_early_log)
{
    session_start_sck<FinalSocketTransport>(
        "RDP Client"_sck_name, std::move(sck), sck_start_time, ini, pid_file, font, prevent_early_log);
}

void session_start_ws(unique_fd sck, MonotonicTimePoint sck_start_time, Inifile& ini, PidFile& pid_file, Font const& font, bool prevent_early_log)
{
    session_start_sck<WsTransport>(
        "RDP Ws Client"_sck_name, std::move(sck), sck_start_time, ini, pid_file, font, prevent_early_log,
        WsTransport::UseTls::No, WsTransport::TlsOptions());
}

void session_start_wss(unique_fd sck, MonotonicTimePoint sck_start_time, Inifile& ini, PidFile& pid_file, Font const& font, bool prevent_early_log)
{
    session_start_sck<WsTransport>(
        "RDP Wss Client"_sck_name, std::move(sck), sck_start_time, ini, pid_file, font, prevent_early_log,
        WsTransport::UseTls::Yes, WsTransport::TlsOptions{
            ini.get<cfg::globals::certificate_password>(),
            ini.get<cfg::client::ssl_cipher_list>(),
            ini.get<cfg::client::tls_min_level>(),
            ini.get<cfg::client::tls_max_level>(),
            ini.get<cfg::client::show_common_cipher_list>(),
        });
}
