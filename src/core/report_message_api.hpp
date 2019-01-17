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
#include <string>

#include <sys/time.h> // timeval


struct ArcsightLogInfo {

    enum : uint32_t {
            EVENT_SIGNATURE_ID_NONE,
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
            EDIT_CHANGED,
            KBD_INPUT,
            TITLE_BAR,
            MATCH_FINDER,
            UNIDENTIFIED_INPUT_FIELD_GET_FOCUS,
    };

    enum :  uint8_t {
        SERVER_SRC,
        SERVER_DST,
        NONE
    };

    ArcsightLogInfo() = default;

    std::string name;
    int signatureID = 0;
    int severity = 5;
    std::string ApplicationProtocol;
    std::string WallixBastionStatus;
    std::string message;
    std::string filePath;
    std::string fileName;
    long fileSize = 0;
    std::string oldFilePath;
    uint64_t endTime = 0;
    std::string WallixBastionSHA256Digest;

    uint8_t direction_flag = NONE;
};


struct ReportMessageApi : noncopyable
{
    virtual void report(const char * reason, const char * message) = 0;

    virtual void log6(const std::string & info, const ArcsightLogInfo & asl_info, const timeval time) = 0;

    virtual void update_inactivity_timeout() = 0;

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

    void log6(const std::string & /*info*/, const ArcsightLogInfo & /*asl_info*/, const timeval /*time*/) override
    {}


    void update_inactivity_timeout() override { }

    time_t get_inactivity_timeout() override {
        return static_cast<time_t>(0);
    }
};
