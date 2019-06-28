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

#include "utils/arcsight_format.hpp"
#include "utils/arcsight.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "main/version.hpp"

#include <string>
#include <string_view>
#include <charconv>


namespace
{
    template<std::size_t N>
    struct StringBuf
    {
        std::string_view sv() const noexcept
        {
            return {buf, len};
        }

        std::size_t capacity() noexcept
        {
            return N;
        }

        char* data() noexcept
        {
            return buf;
        }

        void setsize(std::size_t n) noexcept
        {
            len = n;
        }

    private:
        char buf[N];
        std::size_t len;
    };

    StringBuf<32> from_int(safe_int<uint64_t> n) noexcept
    {
        StringBuf<32> buf;
        auto r = std::to_chars(buf.data(), buf.data() + buf.capacity(), n.underlying());
        assert(!bool(r.ec));
        buf.setsize(r.ptr - buf.data());
        return buf;
    }

    StringBuf<64> from_gmtime(std::time_t time) noexcept
    {
        StringBuf<64> buf;

        struct tm t;
        gmtime_r(&time, &t);

        constexpr char const* months[]{
            "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
        };

        // MMM(text) dd yyyy hh:mm:ss
        int len = snprintf(buf.data(), buf.capacity(), "%s %02d %04d %02d:%02d:%02d",
            months[t.tm_mon], t.tm_mday, 1900 + t.tm_year, t.tm_hour, t.tm_min, t.tm_sec);
        assert(len > 0);
        buf.setsize(len);

        return buf;
    }

    void arcsight_text_formating(std::string& buff, std::string_view text)
    {
        auto* p = text.data();
        auto* pend = text.data() + text.size();
        while (p != pend) {
            auto* curr = p;
            while (curr != pend && *curr != '\\' && *curr != '\n' && *curr != '=')
            {
                ++curr;
            }

            buff.append(p, curr);

            if (*curr == '\\' || *curr == '=') {
                buff += '\\';
                buff += *curr;
                ++curr;
            }
            else if (*curr == '\n') {
                buff += "\\n";
                ++curr;
            }
            // else == pend

            p = curr;
        }
    }
}

void arcsight_format(
    std::string& s,
    ArcsightLogInfo const& asl,
    std::time_t time,
    std::string_view user,
    std::string_view account,
    std::string_view host,
    std::string_view target_ip,
    std::string_view session_id,
    std::string_view session_type)
{
    auto add_str = [&s](std::string_view prefix, std::string_view str) {
        if (!str.empty()) {
            s += prefix;
            arcsight_text_formating(s, str);
        }
    };

    auto add_int = [&s](std::string_view prefix, safe_int<uint64_t> x) {
        if (bool(x.underlying())) {
            str_append(s, prefix, from_int(x).sv());
        }
    };

    auto add_time = [&s](std::string_view prefix, std::time_t t) {
        if (t) {
            str_append(s, prefix, from_gmtime(t).sv());
        }
    };

    s.reserve(s.size() + 256);

    str_append(s,
        from_gmtime(time).sv(),
        " host message CEF:1|Wallix|Bastion|" VERSION "|",
        from_int(asl.signatureID).sv(), '|',
        asl.name, '|',
        from_int(asl.severity).sv(),
        "|WallixBastionUser=", user,
        " WallixBastionAccount=", account,
        " WallixBastionHost=", host,
        " WallixBastionTargetIP=", target_ip,
        " WallixBastionSession_id=", session_id,
        " WallixBastionSessionType=", session_type.empty() ? std::string_view("Neutral") : session_type
    );

    switch (asl.direction_flag)
    {
        case ArcsightLogInfo::Direction::NONE: break;

        case ArcsightLogInfo::Direction::SERVER_DST:
            str_append(s, " suser=", user, " duser=", account, " src=", host, " dst=", target_ip);
            break;

        case ArcsightLogInfo::Direction::SERVER_SRC:
            str_append(s, " suser=", account, " duser=", user, " src=", target_ip, " dst=", host);
            break;
    }

    add_str(" app=", asl.ApplicationProtocol);
    add_str(" WallixBastionStatus=", asl.WallixBastionStatus);
    add_str(" msg=", asl.message);
    add_str(" oldFilePath=", asl.oldFilePath);
    add_str(" filePath=", asl.filePath);
    add_int(" fsize=", asl.fileSize);
    add_time(" end=", asl.endTime);
    add_str(" fname=", asl.fileName);
}
