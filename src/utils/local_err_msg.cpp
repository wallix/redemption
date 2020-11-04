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
  Copyright (C) Wallix 2020
  Author(s): Wallix Team
*/

#include "utils/local_err_msg.hpp"
#include "core/error.hpp"

zstring_view local_err_msg(Error const& error, Translation::language_t lang, bool with_id) noexcept
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
