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
    Copyright (C) Wallix 201&
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/flags.hpp"
#include "utils/sugar/zstring_view.hpp"


enum class LogCategoryId : uint8_t
{
    None,
    Clipboard,
    Widget,
    Drive,
    FileVerification,
    ServerCertificate,
    TitleBar,
    Web,
    PatternDetected,
    OutboundConnection,
    Application,
    Session,
    Probe,
    Kbd,
    Protocol,
    AccountManipulation,
    BestSafe,
    DynamicChannel,
    SessionSharing,
    count
};

template<>
struct utils::enum_as_flag<LogCategoryId>
{
    static const std::size_t max = std::size_t(LogCategoryId::count);
};

using LogCategoryFlags = utils::flags_t<LogCategoryId>;

#define X_LOG_ID(f)                                                          \
    f(BUTTON_CLICKED, Widget, _)                                             \
    f(CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION, Clipboard, _)             \
    f(CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION_EX, Clipboard, DEPRECATED) \
    f(CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION, Clipboard, _)               \
    f(CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX, Clipboard, DEPRECATED)   \
    f(CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION, Clipboard, _)             \
    f(CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION, Clipboard, _)               \
    f(CERTIFICATE_CHECK_SUCCESS, ServerCertificate, _)                       \
    f(CHECKBOX_CLICKED, Widget, _)                                           \
    f(CLIENT_EXECUTE_REMOTEAPP, Protocol, _)                                 \
    f(COMPLETED_PROCESS, Application, _)                                     \
    f(CONNECTION_FAILED, Protocol, _)                                        \
    f(DRIVE_REDIRECTION_DELETE, Drive, _)                                    \
    f(DRIVE_REDIRECTION_READ, Drive, _)                                      \
    f(DRIVE_REDIRECTION_READ_EX, Drive, DEPRECATED)                          \
    f(DRIVE_REDIRECTION_RENAME, Drive, _)                                    \
    f(DRIVE_REDIRECTION_USE, Drive, _)                                       \
    f(DRIVE_REDIRECTION_WRITE, Drive, _)                                     \
    f(DRIVE_REDIRECTION_WRITE_EX, Drive, DEPRECATED)                         \
    f(EDIT_CHANGED, Widget, _)                                               \
    f(FILE_VERIFICATION, FileVerification, _)                                \
    f(FILE_VERIFICATION_ERROR, FileVerification, _)                          \
    f(FOREGROUND_WINDOW_CHANGED, TitleBar, _)                                \
    f(GROUP_MEMBERSHIP, Probe, _)                                            \
    f(INPUT_LANGUAGE, Kbd, _)                                                \
    f(KBD_INPUT, Kbd, _)                                                     \
    f(KERBEROS_TICKET_CREATION, Application, _)                              \
    f(KERBEROS_TICKET_DELETION, Application, _)                              \
    f(KILL_PATTERN_DETECTED, PatternDetected, _)                             \
    f(NEW_PROCESS, Application, _)                                           \
    f(NOTIFY_PATTERN_DETECTED, PatternDetected, _)                           \
    f(OUTBOUND_CONNECTION_BLOCKED, OutboundConnection, _)                    \
    f(OUTBOUND_CONNECTION_BLOCKED_2, OutboundConnection, DEPRECATED)         \
    f(OUTBOUND_CONNECTION_DETECTED, OutboundConnection, _)                   \
    f(OUTBOUND_CONNECTION_DETECTED_2, OutboundConnection, DEPRECATED)        \
    f(PASSWORD_TEXT_BOX_GET_FOCUS, Widget, _)                                \
    f(PROCESS_BLOCKED, Application, _)                                       \
    f(PROCESS_DETECTED, Application, _)                                      \
    /* internal, used from front to capture for enabled ocr */               \
    f(PROBE_STATUS, Probe, _)                                                \
    f(SERVER_CERTIFICATE_ERROR, ServerCertificate, _)                        \
    f(SERVER_CERTIFICATE_MATCH_FAILURE, ServerCertificate, _)                \
    f(SERVER_CERTIFICATE_MATCH_SUCCESS, ServerCertificate, _)                \
    f(SERVER_CERTIFICATE_NEW, ServerCertificate, _)                          \
    f(SESSION_CREATION_FAILED, Session, _)                                   \
    f(SESSION_DISCONNECTION, Session, _)                                     \
    f(SESSION_ENDING_IN_PROGRESS, Probe, _)                                  \
    f(SESSION_ESTABLISHED_SUCCESSFULLY, Session, _)                          \
    f(STARTUP_APPLICATION_FAIL_TO_RUN, Application, _)                       \
    f(STARTUP_APPLICATION_FAIL_TO_RUN_2, Application, DEPRECATED)            \
    f(TITLE_BAR, TitleBar, _)                                                \
    f(UAC_PROMPT_BECOME_VISIBLE, Widget, _)                                  \
    f(UNIDENTIFIED_INPUT_FIELD_GET_FOCUS, Widget, _)                         \
    f(WEB_ATTEMPT_TO_PRINT, Web, _)                                          \
    f(WEB_BEFORE_NAVIGATE, Web, _)                                           \
    f(WEB_DOCUMENT_COMPLETE, Web, _)                                         \
    f(WEB_ENCRYPTION_LEVEL_CHANGED, Web, _)                                  \
    f(WEB_NAVIGATE_ERROR, Web, _)                                            \
    f(WEB_NAVIGATION, Web, _)                                                \
    f(WEB_PRIVACY_IMPACTED, Web, _)                                          \
    f(WEB_THIRD_PARTY_URL_BLOCKED, Web, _)                                   \
    f(ACCOUNT_MANIPULATION_BLOCKED, AccountManipulation, _)                  \
    f(ACCOUNT_MANIPULATION_DETECTED, AccountManipulation, _)                 \
    f(TEXT_VERIFICATION, FileVerification, _)                                \
    f(BESTSAFE_SERVICE_LOG, BestSafe, _)                                     \
    f(DYNAMIC_CHANNEL_CREATION_ALLOWED, DynamicChannel, _)                   \
    f(DYNAMIC_CHANNEL_CREATION_REJECTED, DynamicChannel, _)                  \
    f(FILE_BLOCKED, FileVerification, _)                                     \
    f(SESSION_LOCKED, Session, _)                                            \
    f(EDIT_CHANGED_2, Widget, DEPRECATED)                                    \
    f(SELECT_CHANGED, Widget, _)                                             \
    f(SESSION_EVENT, Session, _)                                             \
    f(SESSION_INVITE_GUEST_CONNECTION, SessionSharing, _)                    \
    f(SESSION_INVITE_GUEST_CONNECTION_REJECTED, SessionSharing, _)           \
    f(SESSION_INVITE_GUEST_DISCONNECTION, SessionSharing, _)                 \
    f(SESSION_INVITE_CONTROL_OWNERSHIP_CHANGED, SessionSharing, _)           \
    f(SESSION_INVITE_GUEST_KILLED, SessionSharing, _)                        \
    f(SESSION_INVITE_GUEST_VIEW_CHANGED, SessionSharing, _)


enum class LogId : uint16_t
{
#define LOG_ID_ATTR_
#define LOG_ID_ATTRDEPRECATED [[deprecated("only for old trace")]]
#define LOG_ID_ATTR_DISPATCH(attr) LOG_ID_ATTR##attr
#define f(x, cat, attr) x LOG_ID_ATTR_DISPATCH(attr),
    X_LOG_ID(f)
#undef f
#undef LOG_ID_ATTR
#undef LOG_ID_ATTR_
#undef LOG_ID_ATTRDEPRECATED
};

namespace detail
{
    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_EMSCRIPTEN_IGNORE("-Wmissing-variable-declarations")
    constexpr inline zstring_view log_id_string_map[]{
        #define f(x, cat, attr) #x ""_zv,
        X_LOG_ID(f)
        #undef f
    };

    constexpr inline LogCategoryId log_id_category_map[]{
        #define f(x, cat, attr) LogCategoryId::cat,
        X_LOG_ID(f)
        #undef f
    };
    REDEMPTION_DIAGNOSTIC_POP()

#define f(x, cat, attr) +1
    inline constexpr std::size_t log_id_max = 0u X_LOG_ID(f);
#undef f
} // namespace detail

constexpr bool is_valid_log_id(unsigned id) noexcept
{
    return id < detail::log_id_max;
}
