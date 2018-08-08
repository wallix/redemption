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

#include <ctime>

extern std::string text_gmdate(const time_t time, char sep);

inline std::string text_gmdate(const time_t time, char sep='-')
{
    struct tm t;
    gmtime_r(&time, &t);
    char buf[11] = {};
    snprintf(buf, sizeof(buf), "%04d%c%02d%c%02d",1900+t.tm_year, sep, 1+t.tm_mon, sep, t.tm_mday);
    return std::string(buf, 10);
}


extern std::string text_gmdatetime(const time_t time);

inline std::string text_gmdatetime(const time_t time) {
    struct tm t;
    gmtime_r(&time, &t);
    char buf[20] = {};
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",1900+t.tm_year, 1+t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    return std::string(buf, 19);
}


extern std::string filename_gmdatetime(const time_t time);

inline std::string filename_gmdatetime(const time_t time) {
    struct tm t;
    gmtime_r(&time, &t);
    char buf[20] = {};
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d_%02d-%02d-%02d",1900+t.tm_year, 1+t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    return std::string(buf, 19);
}
