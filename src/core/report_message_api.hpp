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

#include <sys/time.h> // timeval

enum class LogId : unsigned;

struct KVLog
{
    array_view_const_char key;
    array_view_const_char value;

    KVLog() = default;

    KVLog(array_view_const_char key, array_view_const_char value) noexcept
    : key(key)
    , value(value)
    {}
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
