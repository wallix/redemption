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
        NONE,
        SESSION_CREATION,
        CONNECTION,
        CERTIFICATE_CHECK,
        SERVER_CERTIFICATE_NEW,
        SERVER_CERTIFICATE_MATCH,
        SERVER_CERTIFICATE,
        SESSION_ESTABLISHED,
        SESSION_DISCONNECTION,
        DRIVE_REDIRECTION_USE,
        DRIVE_REDIRECTION_READ_EX,
        DRIVE_REDIRECTION_READ,
        DRIVE_REDIRECTION_WRITE_EX,
        DRIVE_REDIRECTION_WRITE,
        DRIVE_REDIRECTION_DELETE,
        DRIVE_REDIRECTION_RENAME,
        CB_COPYING_PASTING_DATA,
        CB_COPYING_PASTING_DATA_EX,
        CLIENT_EXECUTE_REMOTEAPP,
        SESSION_ENDING_IN_PROGRESS,
        KERBEROS_TICKET,
        PASSWORD_TEXT_BOX_GET_FOCUS,
        UAC_PROMPT_BECOME_VISIBLE,
        INPUT_LANGUAGE,
        PROCESS,
        STARTUP_APPLICATION,
        OUTBOUND_CONNECTION,
        FOREGROUND_WINDOW_CHANGED,
        BUTTON_CLICKED,
        CHECKBOX_CLICKED,
        EDIT_CHANGED,
        KBD_INPUT,
        TITLE_BAR,
        MATCH_FINDER,
        UNIDENTIFIED_INPUT_FIELD_GET_FOCUS,
        FILE_SCAN_RESULT
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
