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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou,
              Jonathan Poelen

   Error exception object
*/

#define NOT_UNDEF_EACH_ERROR
#include "core/error.hpp"
#include "cxx/cxx.hpp"

#if __cplusplus >= REDEMPTION_CXX_STD_17
# include "utils/string_c.hpp"
#else
namespace
{
    template<std::size_t n>
    struct ErrorCbuf
    {
        char buf[n+30];

        ErrorCbuf(char const* s, unsigned e) noexcept
        {
            std::sprintf(buf, "Exception %s no: %u", s, e);
        }
    };
}
#endif

#ifndef NDEBUG
# include <cstring>
# include "utils/log.hpp"
# if __has_include(<boost/stacktrace.hpp>)
//#  include <iostream>
#  include <boost/stacktrace.hpp>
#  define REDEMPTION_ERROR_WITH_STACKTRACE
# endif
#endif

Error::Error(error_type id, int errnum) noexcept
: id(id)
, errnum(errnum)
{
#ifndef NDEBUG
    if (id == NO_ERROR) {
        return;
    }

    if (errnum) {
        LOG(LOG_DEBUG, "Create Error: %s: %s", this->errmsg(), strerror(errnum));
    }
    else {
        LOG(LOG_DEBUG, "Create Error: %s", this->errmsg());
    }

# ifdef REDEMPTION_ERROR_WITH_STACKTRACE
    int i = 0;
    for (auto && frame: boost::stacktrace::stacktrace()){
        if (!frame.empty()){
            auto line = boost::stacktrace::to_string(frame);
            LOG(LOG_DEBUG, "#%d %s", i, line);
            ++i;
        }
    }
//    std::cerr << boost::stacktrace::stacktrace() << std::flush;
# endif
#endif
}

const char * Error::errmsg(bool with_id) const noexcept
{
    switch(this->id) {
    case NO_ERROR:
        return "No error";
    case ERR_SESSION_UNKNOWN_BACKEND:
        return "Unknown Backend";
    case ERR_NLA_AUTHENTICATION_FAILED:
        return "NLA Authentication Failed";
    case ERR_TRANSPORT_OPEN_FAILED:
        return "Open file failed";
    case ERR_TRANSPORT_TLS_CERTIFICATE_CHANGED:
        return "TLS certificate changed";
    case ERR_TRANSPORT_TLS_CERTIFICATE_MISSED:
        return "TLS certificate missed";
    case ERR_TRANSPORT_TLS_CERTIFICATE_CORRUPTED:
        return "TLS certificate corrupted";
    case ERR_TRANSPORT_TLS_CERTIFICATE_INACCESSIBLE:
        return "TLS certificate is inaccessible";
    case ERR_VNC_CONNECTION_ERROR:
        return "VNC connection error.";
    case ERR_WIDGET_INVALID_COMPOSITE_DESTROY:
        return "Composite Widget Destroyed without child list not empty";

    case ERR_SESSION_PROBE_ENDING_IN_PROGRESS:
        return "Session logoff in progress";

    case ERR_RDP_UNSUPPORTED_MONITOR_LAYOUT:
        return "Unsupported client display monitor layout";

    case ERR_LIC:
        return "An error occurred during the licensing protocol";

    case ERR_RAIL_CLIENT_EXECUTE:
        return "The RemoteApp program did not start on the remote computer";

    case ERR_RAIL_STARTING_PROGRAM:
        return "Cannot start the RemoteApp program";

    case ERR_RAIL_UNAUTHORIZED_PROGRAM:
        return "The RemoteApp program is not in the list of authorized programs";

    case ERR_RDP_OPEN_SESSION_TIMEOUT:
        return "Logon timer expired";

    case ERR_RDP_SERVER_REDIR:
        return "The computer that you are trying to connect to is redirecting you to another computer.";

    default:
        #define PP_STRINGIFY_I(x) #x
        #define PP_STRINGIFY(x) PP_STRINGIFY_I(x)
        #define MAKE_CASE_V(e, x) case e:                 \
            return with_id                                \
                ? "Exception " #e " no: " PP_STRINGIFY(x) \
                : "Exception " #e;
#if __cplusplus >= REDEMPTION_CXX_STD_17
        using namespace jln::literals;
        #define MAKE_CASE(e) case e:                         \
            return with_id                                   \
                ? jln::string_c_concat_t<                    \
                    decltype("Exception " #e " no: "_c),     \
                    jln::ull_to_string_c_t<int(e)>>::c_str() \
                : "Exception " #e;
#else
        #define MAKE_CASE(e) case e:                               \
            static ErrorCbuf<sizeof(#e)> buf_##e(#e, unsigned(e)); \
            return with_id ? buf_##e.buf : "Exception " #e;
#endif
        switch (this->id) {
            EACH_ERROR(MAKE_CASE, MAKE_CASE_V)
        }
        #undef MAKE_CASE
        #undef MAKE_CASE_V
        #undef PP_STRINGIFY
        #undef PP_STRINGIFY_I
        return "Unknown Error";
    }
}

const char* local_err_msg(const Error& error, Translation::language_t lang, bool with_id) noexcept
{
    switch (error.id) {
    case ERR_SESSION_UNKNOWN_BACKEND:
        return TR(trkeys::err_session_unknown_backend, lang);

    case ERR_NLA_AUTHENTICATION_FAILED:
        return TR(trkeys::err_nla_authentication_failed, lang);

    case ERR_TRANSPORT_TLS_CERTIFICATE_CHANGED:
        return TR(trkeys::err_transport_tls_certificate_changed, lang);

    case ERR_TRANSPORT_TLS_CERTIFICATE_MISSED:
        return TR(trkeys::err_transport_tls_certificate_missed, lang);

    case ERR_TRANSPORT_TLS_CERTIFICATE_CORRUPTED:
        return TR(trkeys::err_transport_tls_certificate_corrupted, lang);

    case ERR_TRANSPORT_TLS_CERTIFICATE_INACCESSIBLE:
        return TR(trkeys::err_transport_tls_certificate_inaccessible, lang);

    case ERR_VNC_CONNECTION_ERROR:
        return TR(trkeys::err_vnc_connection_error, lang);

    case ERR_SESSION_PROBE_ENDING_IN_PROGRESS:
        return TR(trkeys::session_logoff_in_progress, lang);

    case ERR_RDP_UNSUPPORTED_MONITOR_LAYOUT:
        return TR(trkeys::err_rdp_unsupported_monitor_layout, lang);

    case ERR_LIC:
        return TR(trkeys::err_lic, lang);

    case ERR_RAIL_CLIENT_EXECUTE:
        return TR(trkeys::err_rail_client_execute, lang);

    case ERR_RAIL_STARTING_PROGRAM:
        return TR(trkeys::err_rail_starting_program, lang);

    case ERR_RAIL_UNAUTHORIZED_PROGRAM:
        return TR(trkeys::err_rail_unauthorized_program, lang);

    case ERR_RDP_OPEN_SESSION_TIMEOUT:
        return TR(trkeys::err_rdp_open_session_timeout, lang);

    case ERR_RDP_SERVER_REDIR:
        return TR(trkeys::err_rdp_server_redir, lang);

    case ERR_SESSION_PROBE_LAUNCH:
        return TR(trkeys::err_session_probe_launch, lang);

    case ERR_SESSION_PROBE_ASBL_FSVC_UNAVAILABLE:
        return TR(trkeys::err_session_probe_asbl_fsvc_unavailable, lang);

    case ERR_SESSION_PROBE_ASBL_MAYBE_SOMETHING_BLOCKS:
        return TR(trkeys::err_session_probe_asbl_maybe_something_blocks, lang);

    case ERR_SESSION_PROBE_ASBL_UNKNOWN_REASON:
        return TR(trkeys::err_session_probe_asbl_unknown_reason, lang);

    case ERR_SESSION_PROBE_CBBL_FSVC_UNAVAILABLE:
        return TR(trkeys::err_session_probe_cbbl_fsvc_unavailable, lang);

    case ERR_SESSION_PROBE_CBBL_CBVC_UNAVAILABLE:
        return TR(trkeys::err_session_probe_cbbl_cbvc_unavailable, lang);

    case ERR_SESSION_PROBE_CBBL_DRIVE_NOT_READY_YET:
        return TR(trkeys::err_session_probe_cbbl_drive_not_ready_yet, lang);

    case ERR_SESSION_PROBE_CBBL_MAYBE_SOMETHING_BLOCKS:
        return TR(trkeys::err_session_probe_cbbl_maybe_something_blocks, lang);

    case ERR_SESSION_PROBE_CBBL_LAUNCH_CYCLE_INTERRUPTED:
        return TR(trkeys::err_session_probe_cbbl_launch_cycle_interrupted, lang);

    case ERR_SESSION_PROBE_CBBL_UNKNOWN_REASON_REFER_TO_SYSLOG:
        return TR(trkeys::err_session_probe_cbbl_unknown_reason_refer_to_syslog, lang);

    case ERR_SESSION_PROBE_RP_LAUNCH_REFER_TO_SYSLOG:
        return TR(trkeys::err_session_probe_rp_launch_refer_to_syslog, lang);

    default:
        return error.errmsg(with_id);
    }
}
