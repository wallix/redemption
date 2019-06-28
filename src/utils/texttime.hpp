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
   Copyright (C) Wallix 2018
   Author(s): Christophe Grosjean, Clément Moroldo
*/

#pragma once

#include <string>
#include <chrono>
#include <ctime>
#include <sstream>

// TODO std::string -> static_string

inline std::string text_gmdate(const timeval tv)
{                                                           // yyyy-MM-dd
    struct tm t;
    time_t time = tv.tv_sec;
    gmtime_r(&time, &t);
    char buf[20];
    int len = snprintf(buf, sizeof(buf), "%04d-%02d-%02d",1900+t.tm_year, 1+t.tm_mon, t.tm_mday);
    return std::string(buf, len);
}


inline std::string text_gmdatetime(const timeval tv)
{                                                           // yyyy-MM-dd hh:mm:ss
    struct tm t;
    time_t time = tv.tv_sec;
    gmtime_r(&time, &t);
    char buf[35];
    int len = snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",1900+t.tm_year, 1+t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    return std::string(buf, len);
}


inline std::string filename_gmdatetime(const timeval tv)
{                                                           // yyyy-MM-dd_hh-mm-ss
    struct tm t;
    time_t time = tv.tv_sec;
    gmtime_r(&time, &t);
    char buf[35];
    int len = snprintf(buf, sizeof(buf), "%04d-%02d-%02d_%02d-%02d-%02d",1900+t.tm_year, 1+t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    return std::string(buf, len);
}

inline std::string get_full_text_sec_and_usec(const timeval tv) { // s_us
    time_t time_sec = tv.tv_sec;
    time_t time_usec = tv.tv_usec;

    std::stringstream s_us;
    s_us << time_sec << "_" << time_usec;

    return s_us.str();
}
