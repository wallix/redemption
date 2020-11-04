/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Meng Tan, Jonathan Poelen
 */


#pragma once

#include "configs/autogen/enums.hpp" // Language
#include "cxx/diagnostic.hpp"
#include "utils/sugar/zstring_view.hpp"

#include "system/i18n/message_translator_gettext.hpp"

#include <cstdio>

namespace trkeys
{
    struct TrKey { zstring_view translation; };

#define TR_PROTECTED_KV(name, en)                       \
    constexpr struct TrKey##_##name                     \
    { zstring_view translation; } name{en ""_zv}

    TR_PROTECTED_KV(password, "Password");

#undef TR_PROTECTED_KV

    template<class T> struct TrKeyFmt
    {
        zstring_view translation;
    };

#define TR_KV_FMT(name, en)                          \
    struct TrKeyFmt##_##name                         \
    {                                                \
        template<class... Ts>                        \
        static auto check_printf_result(             \
            char* s, std::size_t n, Ts const& ... xs \
        ) {                                          \
            (void)std::snprintf(s, n, en, xs...);    \
            return int();                            \
        }                                            \
    };                                               \
    constexpr TrKeyFmt<TrKeyFmt##_##name> name{en ""_zv}

#define TR_KV(name, en) constexpr TrKey name{en ""_zv}
    TR_KV(login, "Login");
    TR_KV(diagnostic, "Diagnostic");
    TR_KV(connection_closed, "Connection closed");
    TR_KV(OK, "OK");
    TR_KV(cancel, "Cancel");
    TR_KV(help, "Help");
    TR_KV(close, "Close");
    TR_KV(refused, "Refused");
    TR_KV(username, "Username");
    TR_KV(password_expire, "Your Bastion password will expire soon. Please change it.");
    TR_KV(protocol, "Protocol");
    TR_KV(authorization, "Authorization");
    TR_KV(target, "Target");
    TR_KV(description, "Description");
    TR_KV(close_time, "Close Time");
    TR_KV(logout, "Logout");
    TR_KV(apply, "Apply");
    TR_KV(filter, "Filter");
    TR_KV(connect, "Connect");
    TR_KV(timeleft, "Time left");
    TR_KV(second, "second");
    TR_KV(minute, "minute");
    TR_KV(hour, "hour");
    TR_KV(before_closing, "before closing");
    TR_KV(enable_rt_display, "Your session is observed by an administrator");
    TR_KV(disable_rt_display, "Your session is no longer observed by an administrator.");
    TR_KV(manager_close_cnx, "Connection closed by manager.");
    TR_KV(end_connection, "End of connection");
    TR_KV(help_message,
          "The \"Target\" field can be entered with a string labelled in this format:\n"
          "\"Account@Domain@Device:Service:Auth\".\n"
          "The \"Domain\", \"Service\" and \"Auth\" parts are optional.\n"
          "This field is optional and case-sensitive.\n"
          "\n"
          "The \"Login\" field must refer to a user declared on the Bastion.\n"
          "This field is required and not case-sensitive.\n"
          "\n"
          "Contact your system administrator for assistance.");
    TR_KV(selector, "Selector");
    TR_KV(session_out_time, "Session is out of allowed timeframe");
    TR_KV(miss_keepalive, "Missed keepalive from ACL");
    TR_KV(close_inactivity, "Connection closed on inactivity");
    TR_KV(acl_fail, "Authentifier service failed");
    TR_KV(target_fail, "Failed to connect to remote host.");
    TR_KV(authentification_rdp_fail, "Failed to authenticate with remote RDP host.");
    TR_KV(authentification_vnc_fail, "Failed to authenticate with remote VNC host.");
    TR_KV(authentification_x_fail, "Failed to authenticate with remote X host.");
    TR_KV(connection_ended, "Connection to server ended.");
    TR_KV(no_results, "No results found");
    TR_KV(back_selector, "Back to Selector");
    TR_KV(exit, "Exit");
    TR_KV(comment, "Comment");
    TR_KV(comment_r, "Comment *");
    TR_KV(ticket, "Ticket Ref.");
    TR_KV(ticket_r, "Ticket Ref. *");
    TR_KV(duration, "Duration");
    TR_KV(duration_r, "Duration *");
    TR_KV(note_duration_format, "Format: [hours]h[mins]m");
    TR_KV(note_required, "(*) required fields");
    TR_KV(confirm, "Confirm");
    TR_KV_FMT(fmt_field_required, "Error: %s field is required.");
    TR_KV_FMT(fmt_invalid_format, "Error: %s invalid format.");
    TR_KV_FMT(fmt_toohigh_duration, "Error: %s is too high (max: %d minutes).");
    TR_KV(information, "Information");
    TR_KV(authentication_required,  "Authentication Required");
    TR_KV(target_info_required, "Target Information Required");
    TR_KV(device, "Device");
    TR_KV(disable_osd, "(insert key or left click to hide)");
    TR_KV(disconnected_by_otherconnection, "Another user connected to the resource, so your connection was lost.");
    TR_KV_FMT(process_interrupted_security_policies, "The process '%s' was interrupted in accordance with security policies.");
    TR_KV_FMT(account_manipulation_blocked_security_policies, "The account manipulation initiated by process '%s' was rejected in accordance with security policies.");
    TR_KV(session_logoff_in_progress, "Session logoff in progress.");
    TR_KV(starting_remoteapp, "Starting RemoteApp ...");
    TR_KV(closing_remoteapp, "All RemoteApp windows are closed.");
    TR_KV(disconnect_now, "Disconnect Now");
    TR_KV(err_rdp_server_redir, "The computer that you are trying to connect to is redirecting you to another computer!");
    TR_KV(err_nla_authentication_failed, "NLA Authentication Failed!");
    TR_KV(err_transport_tls_certificate_changed, "TLS certificate changed!");
    TR_KV(err_transport_tls_certificate_missed, "TLS certificate missed!");
    TR_KV(err_transport_tls_certificate_corrupted, "TLS certificate corrupted!");
    TR_KV(err_transport_tls_certificate_inaccessible, "TLS certificate  is inaccessible!");
    TR_KV(err_vnc_connection_error, "VNC connection error!");
    TR_KV(err_rdp_unsupported_monitor_layout, "Unsupported client display monitor layout!");
    TR_KV(err_lic, "An error occurred during the licensing protocol!");
    TR_KV(err_rail_client_execute, "The RemoteApp program did not start on the remote computer!");
    TR_KV(err_rail_starting_program, "Cannot start the RemoteApp program!");
    TR_KV(err_rail_unauthorized_program, "The RemoteApp program is not in the list of authorized programs!");
    TR_KV(err_rdp_open_session_timeout, "Logon timer expired!");
    TR_KV(err_session_probe_launch,"Could not launch Session Probe!");
    TR_KV(err_session_probe_asbl_fsvc_unavailable,
          "(ASBL) Could not launch Session Probe! File System Virtual Channel is unavailable. "
          "Please allow the drive redirection in the Remote Desktop Services settings of the target.");
    TR_KV(err_session_probe_asbl_maybe_something_blocks,
          "(ASBL) Could not launch Session Probe! Maybe something blocks it on the target. "
          "Is the target running under Microsoft Server products? "
          "The Command Prompt should be published as the RemoteApp program and accept any command-line parameters. "
          "Please also check the temporary directory to ensure there is enough free space.");
    TR_KV(err_session_probe_asbl_unknown_reason, "(ASBL) Session Probe launch has failed for unknown reason!");
    TR_KV(err_session_probe_cbbl_fsvc_unavailable,
          "(CBBL) Could not launch Session Probe! File System Virtual Channel is unavailable. "
          "Please allow the drive redirection in the Remote Desktop Services settings of the target.");
    TR_KV(err_session_probe_cbbl_cbvc_unavailable,
          "(CBBL) Could not launch Session Probe! Clipboard Virtual Channel is unavailable. "
          "Please allow the clipboard redirection in the Remote Desktop Services settings of the target.");
    TR_KV(err_session_probe_cbbl_drive_not_ready_yet,
          "(CBBL) Could not launch Session Probe! Drive of Session Probe is not ready yet. "
          "Is the target running under Windows Server 2008 R2 or more recent version?");
    TR_KV(err_session_probe_cbbl_maybe_something_blocks,
          "(CBBL) Session Probe is not launched! Maybe something blocks it on the target. "
          "Please also check the temporary directory to ensure there is enough free space.");
    TR_KV(err_session_probe_cbbl_launch_cycle_interrupted,
          "(CBBL) Session Probe launch cycle has been interrupted! "
          "The launch timeout duration may be too short.");
    TR_KV(err_session_probe_cbbl_unknown_reason_refer_to_syslog,
          "(CBBL) Session Probe launch has failed for unknown reason! "
          "Please refer to the syslog file for more detailed information regarding the error condition.");
    TR_KV(err_session_probe_rp_launch_refer_to_syslog,
          "(RP) Could not launch Session Probe! "
          "Please refer to the syslog file for more detailed information regarding the error condition.");
    TR_KV(err_session_unknown_backend, "Unknown backend failure.");
    TR_KV(err_login_password, "Provided login/password is probably incorrect.");
    TR_KV(wait_msg, "Please wait...");
    TR_KV(err_nla_required, "Enable NLA is probably required.");
    TR_KV(err_tls_required, "Enable TLS is probably required.");
    TR_KV(err_server_denied_connection, "Please check provided Load Balance Info.");
    TR_KV(err_mod_rdp_nego, "Fail during TLS security exchange.");
    TR_KV(err_mod_rdp_basic_settings_exchange, "Fail during basic setting exchange.");
    TR_KV(err_mod_rdp_channel_connection_attach_user, "Fail during channels connection.");
    TR_KV(mod_rdp_channel_join_confirme, "Fail during channels connection.");
    TR_KV(mod_rdp_get_license, "Failed while trying to get licence.");
    TR_KV(err_mod_rdp_connected, "Fail while connecting session on the target.");
    TR_KV(file_verification_wait, "File being analyzed: ");
    TR_KV(file_verification_accepted, "Valid file: ");
    TR_KV(file_verification_rejected, "Invalid file: ");
#undef TR_KV
#undef TR_KV_FMT
} // namespace trkeys


/* Need to pass type with template on Translation struct
   for avoid strong dependency but it forces to change a
   lot of file in project */
using MessageTranslator_t = i18n::MessageTranslatorGettext;

class Inifile;

struct Translation
{
    enum language_t : unsigned char
    {
        EN,
        FR,
        MAX_LANG
    };

private:
    language_t lang = EN;
    language_t prev_lang = EN;
    mutable MessageTranslator_t message_translator;

    Translation() = default;

    inline void reset_message_translator_context() const
    {
        if (this->lang != this->prev_lang)
        {
            this->message_translator.clear_context();

            /* check if it's english language for avoid useless context reset
               because text is already in english by default in code */
            if (this->lang != language_t::EN)
            {
                this->message_translator.set_context(to_sv(this->lang));
            }
        }
    }

    [[nodiscard]]
    inline zstring_view _translate(zstring_view text) const
    {
        reset_message_translator_context();
        return this->message_translator.get_translated_text(text);
    }

public:
    Translation(Translation const&) = delete;
    void operator=(Translation const&) = delete;

    static Translation& getInstance()
    {
        static Translation instance;
        return instance;
    }

    static zstring_view to_sv(language_t lang)
    {
        switch (lang)
        {
            case Translation::language_t::EN :
                return "en"_zv;
            case Translation::language_t::FR :
                return "fr"_zv;
            case Translation::language_t::MAX_LANG :
                return "MAX_LANG"_zv;
        }
        assert(false);
        return ""_zv;
    }

    bool set_lang(language_t lang)
    {
        if (lang >= MAX_LANG) {
            return false;
        }
        this->prev_lang = this->lang;
        this->lang = lang;
        return true;
    }

    [[nodiscard]] zstring_view translate(trkeys::TrKey_password k) const
    {
        return _translate(k.translation);
    }

    [[nodiscard]] zstring_view translate(trkeys::TrKey k) const
    {
        return _translate(k.translation);
    }

    template<class T, class... Ts>
    auto translate_fmt(char* s, std::size_t n, trkeys::TrKeyFmt<T> k, Ts const&... xs) const
    -> decltype(T::check_printf_result(s, n, xs...))
    {
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
        return std::snprintf(s, n, _translate(k.translation).c_str(), xs...);
        REDEMPTION_DIAGNOSTIC_POP
    }
};


inline zstring_view TR(trkeys::TrKey_password k, Translation::language_t lang)
{
    Translation::getInstance().set_lang(lang);
    return Translation::getInstance().translate(k);
}

inline zstring_view TR(trkeys::TrKey k, Translation::language_t lang)
{
    Translation::getInstance().set_lang(lang);
    return Translation::getInstance().translate(k);
}

template<class T, class... Ts>
int TR_fmt(char* s, std::size_t n, trkeys::TrKeyFmt<T> k, Translation::language_t lang, Ts const&... xs)
{
    Translation::getInstance().set_lang(lang);
    return Translation::getInstance().translate_fmt(s, n, k, xs...);
}

// implementation in config.cpp
Translation::language_t language(Inifile const & ini);

inline Translation::language_t language(Language lang)
{
    return static_cast<Translation::language_t>(lang);
}

LoginLanguage to_login_language(Language lang);

struct Translator
{
    explicit Translator(Translation::language_t lang = Translation::EN) /*NOLINT*/
      : lang(lang)
    {}

    explicit Translator(Inifile const & ini)
      : lang(language(ini))
    {}

    zstring_view operator()(trkeys::TrKey_password const & k) const
    {
        return TR(k, this->lang);
    }

    zstring_view operator()(trkeys::TrKey const & k) const
    {
        return TR(k, this->lang);
    }

    template<class T, class... Ts>
    int fmt(char* s, std::size_t n, trkeys::TrKeyFmt<T> k, Ts const&... xs) const
    {
        return TR_fmt(s, n, k, this->lang, xs...);
    }

private:
    Translation::language_t lang;
};
