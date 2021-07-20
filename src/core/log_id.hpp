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
#include "core/log_id.hpp"

enum class LogCategoryId
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
    count
};

template<>
struct utils::enum_as_flag<LogCategoryId>
{
    static const std::size_t max = std::size_t(LogCategoryId::count);
};

using LogCategoryFlags = utils::flags_t<LogCategoryId>;

#define X_LOG_ID(f)                                              \
    f(BUTTON_CLICKED, Widget)                                    \
    f(CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION, Clipboard)    \
    f(CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION_EX, Clipboard) \
    f(CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION, Clipboard)      \
    f(CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX, Clipboard)   \
    f(CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION, Clipboard)    \
    f(CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION, Clipboard)      \
    f(CERTIFICATE_CHECK_SUCCESS, ServerCertificate)              \
    f(CHECKBOX_CLICKED, Widget)                                  \
    f(CLIENT_EXECUTE_REMOTEAPP, Protocol)                        \
    f(COMPLETED_PROCESS, Application)                            \
    f(CONNECTION_FAILED, Protocol)                               \
    f(DRIVE_REDIRECTION_DELETE, Drive)                           \
    f(DRIVE_REDIRECTION_READ, Drive)                             \
    f(DRIVE_REDIRECTION_READ_EX, Drive)                          \
    f(DRIVE_REDIRECTION_RENAME, Drive)                           \
    f(DRIVE_REDIRECTION_USE, Drive)                              \
    f(DRIVE_REDIRECTION_WRITE, Drive)                            \
    f(DRIVE_REDIRECTION_WRITE_EX, Drive)                         \
    f(EDIT_CHANGED, Widget)                                      \
    f(FILE_VERIFICATION, FileVerification)                       \
    f(FILE_VERIFICATION_ERROR, FileVerification)                 \
    f(FOREGROUND_WINDOW_CHANGED, TitleBar)                       \
    f(GROUP_MEMBERSHIP, Probe)                                   \
    f(INPUT_LANGUAGE, Kbd)                                       \
    f(KBD_INPUT, Kbd)                                            \
    f(KERBEROS_TICKET_CREATION, Application)                     \
    f(KERBEROS_TICKET_DELETION, Application)                     \
    f(KILL_PATTERN_DETECTED, PatternDetected)                    \
    f(NEW_PROCESS, Application)                                  \
    f(NOTIFY_PATTERN_DETECTED, PatternDetected)                  \
    f(OUTBOUND_CONNECTION_BLOCKED, OutboundConnection)           \
    f(OUTBOUND_CONNECTION_BLOCKED_2, OutboundConnection)         \
    f(OUTBOUND_CONNECTION_DETECTED, OutboundConnection)          \
    f(OUTBOUND_CONNECTION_DETECTED_2, OutboundConnection)        \
    f(PASSWORD_TEXT_BOX_GET_FOCUS, Widget)                       \
    f(PROCESS_BLOCKED, Application)                              \
    f(PROCESS_DETECTED, Application)                             \
    f(PROBE_STATUS, Probe)                                       \
    f(SERVER_CERTIFICATE_ERROR, ServerCertificate)               \
    f(SERVER_CERTIFICATE_MATCH_FAILURE, ServerCertificate)       \
    f(SERVER_CERTIFICATE_MATCH_SUCCESS, ServerCertificate)       \
    f(SERVER_CERTIFICATE_NEW, ServerCertificate)                 \
    f(SESSION_CREATION_FAILED, Session)                          \
    f(SESSION_DISCONNECTION, Session)                            \
    f(SESSION_ENDING_IN_PROGRESS, Probe)                         \
    f(SESSION_ESTABLISHED_SUCCESSFULLY, Session)                 \
    f(STARTUP_APPLICATION_FAIL_TO_RUN, Application)              \
    f(STARTUP_APPLICATION_FAIL_TO_RUN_2, Application)            \
    f(TITLE_BAR, TitleBar)                                       \
    f(UAC_PROMPT_BECOME_VISIBLE, Widget)                         \
    f(UNIDENTIFIED_INPUT_FIELD_GET_FOCUS, Widget)                \
    f(WEB_ATTEMPT_TO_PRINT, Web)                                 \
    f(WEB_BEFORE_NAVIGATE, Web)                                  \
    f(WEB_DOCUMENT_COMPLETE, Web)                                \
    f(WEB_ENCRYPTION_LEVEL_CHANGED, Web)                         \
    f(WEB_NAVIGATE_ERROR, Web)                                   \
    f(WEB_NAVIGATION, Web)                                       \
    f(WEB_PRIVACY_IMPACTED, Web)                                 \
    f(WEB_THIRD_PARTY_URL_BLOCKED, Web)                          \
    f(ACCOUNT_MANIPULATION_BLOCKED, AccountManipulation)         \
    f(ACCOUNT_MANIPULATION_DETECTED, AccountManipulation)        \
    f(TEXT_VERIFICATION, FileVerification)                       \
    f(BESTSAFE_SERVICE_LOG, BestSafe)                            \
    f(DYNAMIC_CHANNEL_CREATION_ALLOWED, DynamicChannel)          \
    f(DYNAMIC_CHANNEL_CREATION_REJECTED, DynamicChannel)         \
    f(FILE_BLOCKED, FileVerification)                            \
    f(SESSION_LOCKED, Session)                                   \
    f(EDIT_CHANGED_2, Widget)                                    \
    f(SELECT_CHANGED, Widget)


enum class LogId : unsigned
{
#define f(x, cat) x,
    X_LOG_ID(f)
#undef f
};

inline bool is_valid_log_id(unsigned id)
{
    switch (id) {
#define f(x, cat) case unsigned(LogId::x):
    X_LOG_ID(f)
#undef f
        return true;
    }
    return false;
}

namespace detail
{
    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_EMSCRIPTEN_IGNORE("-Wmissing-variable-declarations")
    constexpr inline zstring_view log_id_string_map[]{
        #define f(x, cat) #x ""_zv,
        X_LOG_ID(f)
        #undef f
    };

    constexpr inline LogCategoryId log_id_category_map[]{
        #define f(x, cat) LogCategoryId::cat,
        X_LOG_ID(f)
        #undef f
    };
    REDEMPTION_DIAGNOSTIC_POP()
} // namespace detail
