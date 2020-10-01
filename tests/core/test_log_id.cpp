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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "core/log_id.hpp"

#include <type_traits>


int main()
{
    using int_type = std::underlying_type_t<LogId>;

    // LogId must be stable for Arcsight

#define CASE(id, i) case id: static_assert(int_type(id) == i); break

    switch (LogId())
    {
        CASE(LogId::BUTTON_CLICKED, 0);
        CASE(LogId::CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION, 1);
        CASE(LogId::CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION_EX, 2);
        CASE(LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION, 3);
        CASE(LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX, 4);
        CASE(LogId::CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION, 5);
        CASE(LogId::CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION, 6);
        CASE(LogId::CERTIFICATE_CHECK_SUCCESS, 7);
        CASE(LogId::CHECKBOX_CLICKED, 8);
        CASE(LogId::CLIENT_EXECUTE_REMOTEAPP, 9);
        CASE(LogId::COMPLETED_PROCESS, 10);
        CASE(LogId::CONNECTION_FAILED, 11);
        CASE(LogId::DRIVE_REDIRECTION_DELETE, 12);
        CASE(LogId::DRIVE_REDIRECTION_READ, 13);
        CASE(LogId::DRIVE_REDIRECTION_READ_EX, 14);
        CASE(LogId::DRIVE_REDIRECTION_RENAME, 15);
        CASE(LogId::DRIVE_REDIRECTION_USE, 16);
        CASE(LogId::DRIVE_REDIRECTION_WRITE, 17);
        CASE(LogId::DRIVE_REDIRECTION_WRITE_EX, 18);
        CASE(LogId::EDIT_CHANGED, 19);
        CASE(LogId::FILE_VERIFICATION, 20);
        CASE(LogId::FILE_VERIFICATION_ERROR, 21);
        CASE(LogId::FOREGROUND_WINDOW_CHANGED, 22);
        CASE(LogId::GROUP_MEMBERSHIP, 23);
        CASE(LogId::INPUT_LANGUAGE, 24);
        CASE(LogId::KBD_INPUT, 25);
        CASE(LogId::KERBEROS_TICKET_CREATION, 26);
        CASE(LogId::KERBEROS_TICKET_DELETION, 27);
        CASE(LogId::KILL_PATTERN_DETECTED, 28);
        CASE(LogId::NEW_PROCESS, 29);
        CASE(LogId::NOTIFY_PATTERN_DETECTED, 30);
        CASE(LogId::OUTBOUND_CONNECTION_BLOCKED, 31);
        CASE(LogId::OUTBOUND_CONNECTION_BLOCKED_2, 32);
        CASE(LogId::OUTBOUND_CONNECTION_DETECTED, 33);
        CASE(LogId::OUTBOUND_CONNECTION_DETECTED_2, 34);
        CASE(LogId::PASSWORD_TEXT_BOX_GET_FOCUS, 35);
        CASE(LogId::PROCESS_BLOCKED, 36);
        CASE(LogId::PROCESS_DETECTED, 37);
        CASE(LogId::PROBE_STATUS, 38);
        CASE(LogId::SERVER_CERTIFICATE_ERROR, 39);
        CASE(LogId::SERVER_CERTIFICATE_MATCH_FAILURE, 40);
        CASE(LogId::SERVER_CERTIFICATE_MATCH_SUCCESS, 41);
        CASE(LogId::SERVER_CERTIFICATE_NEW, 42);
        CASE(LogId::SESSION_CREATION_FAILED, 43);
        CASE(LogId::SESSION_DISCONNECTION, 44);
        CASE(LogId::SESSION_ENDING_IN_PROGRESS, 45);
        CASE(LogId::SESSION_ESTABLISHED_SUCCESSFULLY, 46);
        CASE(LogId::STARTUP_APPLICATION_FAIL_TO_RUN, 47);
        CASE(LogId::STARTUP_APPLICATION_FAIL_TO_RUN_2, 48);
        CASE(LogId::TITLE_BAR, 49);
        CASE(LogId::UAC_PROMPT_BECOME_VISIBLE, 50);
        CASE(LogId::UNIDENTIFIED_INPUT_FIELD_GET_FOCUS, 51);
        CASE(LogId::WEB_ATTEMPT_TO_PRINT, 52);
        CASE(LogId::WEB_BEFORE_NAVIGATE, 53);
        CASE(LogId::WEB_DOCUMENT_COMPLETE, 54);
        CASE(LogId::WEB_ENCRYPTION_LEVEL_CHANGED, 55);
        CASE(LogId::WEB_NAVIGATE_ERROR, 56);
        CASE(LogId::WEB_NAVIGATION, 57);
        CASE(LogId::WEB_PRIVACY_IMPACTED, 58);
        CASE(LogId::WEB_THIRD_PARTY_URL_BLOCKED, 59);

        CASE(LogId::ACCOUNT_MANIPULATION_BLOCKED, 60);
        CASE(LogId::ACCOUNT_MANIPULATION_DETECTED, 61);
        CASE(LogId::TEXT_VERIFICATION, 62);
        CASE(LogId::BESTSAFE_SERVICE_LOG, 63);
        CASE(LogId::DYNAMIC_CHANNEL_CREATION_ALLOWED, 64);
        CASE(LogId::DYNAMIC_CHANNEL_CREATION_REJECTED, 65);
        CASE(LogId::FILE_BLOCKED, 66);
        CASE(LogId::SESSION_LOCKED, 67);
    }
}
