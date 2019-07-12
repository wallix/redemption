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

#pragma once

#include <string>

struct ArcsightLogInfo
{
    enum class ID : uint16_t
    {
        NONE                               = 0,

        SESSION_CREATION                   = 1,
        CONNECTION                         = 2,
        CERTIFICATE_CHECK                  = 3,
        SERVER_CERTIFICATE_NEW             = 4,
        SERVER_CERTIFICATE_MATCH           = 5,
        SERVER_CERTIFICATE                 = 6,
        SESSION_ESTABLISHED                = 7,
        SESSION_DISCONNECTION              = 8,
        DRIVE_REDIRECTION_USE              = 9,
        DRIVE_REDIRECTION_READ_EX          = 10,
        DRIVE_REDIRECTION_READ             = 11,
        DRIVE_REDIRECTION_WRITE_EX         = 12,
        DRIVE_REDIRECTION_WRITE            = 13,
        DRIVE_REDIRECTION_DELETE           = 14,
        DRIVE_REDIRECTION_RENAME           = 15,
        CB_COPYING_PASTING_DATA            = 16,
        CB_COPYING_PASTING_DATA_EX         = 17,
        CLIENT_EXECUTE_REMOTEAPP           = 18,
        SESSION_ENDING_IN_PROGRESS         = 19,
        KERBEROS_TICKET                    = 20,
        PASSWORD_TEXT_BOX_GET_FOCUS        = 21,
        UAC_PROMPT_BECOME_VISIBLE          = 22,
        INPUT_LANGUAGE                     = 23,
        PROCESS                            = 24,
        STARTUP_APPLICATION                = 25,
        OUTBOUND_CONNECTION                = 26,
        FOREGROUND_WINDOW_CHANGED          = 27,
        BUTTON_CLICKED                     = 28,
        CHECKBOX_CLICKED                   = 29,
        EDIT_CHANGED                       = 30,
        KBD_INPUT                          = 31,
        TITLE_BAR                          = 32,
        MATCH_FINDER                       = 33,
        UNIDENTIFIED_INPUT_FIELD_GET_FOCUS = 34,
        FILE_SCAN_RESULT                   = 35,

        WEB_ATTEMPT_TO_PRINT               = 100,
        WEB_BEFORE_NAVIGATE                = 101,
        WEB_DOCUMENT_COMPLETE              = 102,
        WEB_NAVIGATE_ERROR                 = 103,
        WEB_NAVIGATION                     = 104,
        WEB_PRIVACY_IMPACTED               = 105,
        WEB_ENCRYPTION_LEVEL_CHANGED       = 106,
        WEB_THIRD_PARTY_URL_BLOCKED        = 107
    };

    enum class Direction : uint8_t
    {
        SERVER_SRC,
        SERVER_DST,
        NONE
    };

    std::string name;
    std::string ApplicationProtocol;
    std::string WallixBastionStatus;
    std::string message;
    std::string filePath;
    std::string fileName;
    std::string oldFilePath;

    ID signatureID = ID::NONE;
    Direction direction_flag = Direction::NONE;
    unsigned severity = 5;
    time_t endTime = 0;
    uint64_t fileSize = 0;
};
