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

#include "utils/sugar/noncopyable.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/flags.hpp"
#include <string>

#include <sys/time.h> // timeval

#define X_LOG_ID(f)                                   \
    f(BUTTON_CLICKED)                                 \
    f(CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION)    \
    f(CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION_EX) \
    f(CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION)      \
    f(CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX)   \
    f(CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION)    \
    f(CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION)      \
    f(CERTIFICATE_CHECK_SUCCESS)                      \
    f(CHECKBOX_CLICKED)                               \
    f(CLIENT_EXECUTE_REMOTEAPP)                       \
    f(COMPLETED_PROCESS)                              \
    f(CONNECTION_FAILED)                              \
    f(DRIVE_REDIRECTION_DELETE)                       \
    f(DRIVE_REDIRECTION_READ)                         \
    f(DRIVE_REDIRECTION_READ_EX)                      \
    f(DRIVE_REDIRECTION_RENAME)                       \
    f(DRIVE_REDIRECTION_USE)                          \
    f(DRIVE_REDIRECTION_WRITE)                        \
    f(DRIVE_REDIRECTION_WRITE_EX)                     \
    f(EDIT_CHANGED)                                   \
    f(FILE_VERIFICATION)                              \
    f(FILE_VERIFICATION_ERROR)                        \
    f(FILE_VERIFICATION_SERVER_ERROR)                 \
    f(FOREGROUND_WINDOW_CHANGED)                      \
    f(GROUP_MEMBERSHIP)                               \
    f(INPUT_LANGUAGE)                                 \
    f(KBD_INPUT)                                      \
    f(KERBEROS_TICKET_CREATION)                       \
    f(KERBEROS_TICKET_DELETION)                       \
    f(KILL_PATTERN_DETECTED)                          \
    f(NEW_PROCESS)                                    \
    f(NOTIFY_PATTERN_DETECTED)                        \
    f(OUTBOUND_CONNECTION_BLOCKED)                    \
    f(OUTBOUND_CONNECTION_BLOCKED_2)                  \
    f(OUTBOUND_CONNECTION_DETECTED)                   \
    f(OUTBOUND_CONNECTION_DETECTED_2)                 \
    f(PASSWORD_TEXT_BOX_GET_FOCUS)                    \
    f(PROCESS_BLOCKED)                                \
    f(PROCESS_DETECTED)                               \
    f(SERVER_CERTIFICATE_ERROR)                       \
    f(SERVER_CERTIFICATE_MATCH_FAILURE)               \
    f(SERVER_CERTIFICATE_MATCH_SUCCESS)               \
    f(SERVER_CERTIFICATE_NEW)                         \
    f(SESSION_CREATION_FAILED)                        \
    f(SESSION_DISCONNECTION)                          \
    f(SESSION_ENDING_IN_PROGRESS)                     \
    f(SESSION_ESTABLISHED_SUCCESSFULLY)               \
    f(STARTUP_APPLICATION_FAIL_TO_RUN)                \
    f(TITLE_BAR)                                      \
    f(UAC_PROMPT_BECOME_VISIBLE)                      \
    f(UNIDENTIFIED_INPUT_FIELD_GET_FOCUS)             \
    f(WEB_ATTEMPT_TO_PRINT)                           \
    f(WEB_BEFORE_NAVIGATE)                            \
    f(WEB_DOCUMENT_COMPLETE)                          \
    f(WEB_ENCRYPTION_LEVEL_CHANGED)                   \
    f(WEB_NAVIGATE_ERROR)                             \
    f(WEB_NAVIGATION)                                 \
    f(WEB_PRIVACY_IMPACTED)                           \
    f(WEB_THIRD_PARTY_URL_BLOCKED)


enum class LogId
{
#define f(x) x,
    X_LOG_ID(f)
#undef f
};

namespace detail
{
    constexpr inline array_view_const_char log_id_string_map[]{
        #define f(x) #x ""_av,
        X_LOG_ID(f)
        #undef f
    };

    enum class KVLogCategory : uint8_t
    {
        Siem,
        Arcsight,
        COUNT,
    };
}

#ifndef NOT_UNDEF_X_LOG_ID
# undef X_LOG_ID
#endif

template<>
struct utils::enum_as_flag<detail::KVLogCategory>
{
    static constexpr std::size_t max = unsigned(detail::KVLogCategory::COUNT);
};

struct KVLog
{
    using Category = detail::KVLogCategory;
    using Categories = utils::flags_t<Category>;

    Categories categories;
    array_view_const_char key;
    array_view_const_char value;

    static KVLog siem(array_view_const_char key, array_view_const_char value) noexcept
    {
        return KVLog{Category::Siem, key, value};
    }

    static KVLog arcsight(array_view_const_char key, array_view_const_char value) noexcept
    {
        return KVLog{Category::Arcsight, key, value};
    }

    static KVLog all(array_view_const_char key, array_view_const_char value) noexcept
    {
        return KVLog{Categories{} | Category::Arcsight | Category::Siem, key, value};
    }
};

struct KVList : array_view<KVLog const>
{
    KVList(array_view<KVLog const> kv_list) noexcept
    : array_view<KVLog const>(kv_list)
    {}

    KVList(std::initializer_list<KVLog> kv_list) noexcept
    : array_view<KVLog const>(kv_list)
    {}
};

struct ReportMessageApi : noncopyable
{
    // TODO array_view
    virtual void report(const char * reason, const char * message) = 0;

    virtual void log6(LogId id, const timeval time, KVList kv_list) = 0;

    // TODO other interface
    virtual void update_inactivity_timeout() = 0;

    // TODO other interface
    virtual time_t get_inactivity_timeout() = 0;

    virtual ~ReportMessageApi() = default;
};

struct NullReportMessage : ReportMessageApi
{
    void report(const char * reason, const char * message) override
    {
        (void)reason;
        (void)message;
    }

    void log6(LogId id, const timeval time, KVList kv_list) override
    {
        (void)id;
        (void)time;
        (void)kv_list;
    }


    void update_inactivity_timeout() override { }

    time_t get_inactivity_timeout() override {
        return static_cast<time_t>(0);
    }
};
