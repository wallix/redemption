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
Copyright (C) Wallix 2022
Author(s): Proxies Team
*/

#pragma once

#include <ctime>
#include <cstring>


namespace detail
{
    inline char* push_2digits(char* p, int n) noexcept
    {
        char const* ints2digits =
            "00" "01" "02" "03" "04" "05" "06" "07" "08" "09"
            "10" "11" "12" "13" "14" "15" "16" "17" "18" "19"
            "20" "21" "22" "23" "24" "25" "26" "27" "28" "29"
            "30" "31" "32" "33" "34" "35" "36" "37" "38" "39"
            "40" "41" "42" "43" "44" "45" "46" "47" "48" "49"
            "50" "51" "52" "53" "54" "55" "56" "57" "58" "59"
            "60" "61" "62" "63" "64" "65" "66" "67" "68" "69"
            "70" "71" "72" "73" "74" "75" "76" "77" "78" "79"
            "80" "81" "82" "83" "84" "85" "86" "87" "88" "89"
            "90" "91" "92" "93" "94" "95" "96" "97" "98" "99"
        ;

        memcpy(p, ints2digits + n * 2, 2);
        return p + 2;
    }
}

namespace dateformats
{
    struct YYYY_mm_dd_HH_MM_SS
    {
        static constexpr std::size_t output_length = 19;

        static char* to_chars(char* p, struct tm tm) noexcept
        {
            p = detail::push_2digits(p, (1900 + tm.tm_year) / 100);
            p = detail::push_2digits(p, (1900 + tm.tm_year) % 100);
            *p++ = '-';
            p = detail::push_2digits(p, tm.tm_mon + 1);
            *p++ = '-';
            p = detail::push_2digits(p, tm.tm_mday);
            *p++ = ' ';
            p = detail::push_2digits(p, tm.tm_hour);
            *p++ = ':';
            p = detail::push_2digits(p, tm.tm_min);
            *p++ = ':';
            p = detail::push_2digits(p, tm.tm_sec);

            return p;
        }
    };

    struct MMM_dd_YYYY_HH_MM_SS
    {
        static constexpr std::size_t output_length = 20;

        static char* to_chars(char* p, struct tm tm) noexcept
        {
            char const* months =
                "Jan "
                "Feb "
                "Mar "
                "Apr "
                "May "
                "Jun "
                "Jul "
                "Aug "
                "Sep "
                "Oct "
                "Nov "
                "Dec "
            ;

            memcpy(p, months + tm.tm_mon * 4, 4);
            p += 4;
            p = detail::push_2digits(p, tm.tm_mday);
            *p++ = ' ';
            p = detail::push_2digits(p, (1900 + tm.tm_year) / 100);
            p = detail::push_2digits(p, (1900 + tm.tm_year) % 100);
            *p++ = ' ';
            p = detail::push_2digits(p, tm.tm_hour);
            *p++ = ':';
            p = detail::push_2digits(p, tm.tm_min);
            *p++ = ':';
            p = detail::push_2digits(p, tm.tm_sec);

            return p;
        }
    };
}
