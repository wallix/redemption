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

#include <string>
#include "utils/sugar/noncopyable.hpp"


struct ArcsightLogInfo {

    std::string name;
    int signatureID = 0;
    int severity = 5;
    std::string ApplicationProtocol;
    std::string WallixBastionStatus;
    std::string message;
    std::string filePath;
    std::string oldFilePath;
    std::string fileSize;

};


struct ReportMessageApi : noncopyable
{
    virtual void report(const char * reason, const char * message) = 0;

    virtual void log5(const std::string & info) = 0;

    virtual void log6(const std::string & info, const ArcsightLogInfo & asl_info) = 0;

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

    void log5(const std::string & info) override
    {
        (void)info;
    }

    void log6(const std::string & info, const ArcsightLogInfo & asl_info) override
    {
        (void)info;
        (void)asl_info;
    }


    void update_inactivity_timeout() override { }

    time_t get_inactivity_timeout() override {
        return static_cast<time_t>(0);
    }
};
