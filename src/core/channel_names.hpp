/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/


#pragma once

#include "utils/sugar/std_stream_proto.hpp"

#include <cstdint>

namespace CHANNELS
{
    struct ChannelNameId
    {
        enum class Id : uint64_t;

        constexpr ChannelNameId(Id id = Id{}) noexcept
          : id(id)
        {}

        template<std::size_t n>
        constexpr explicit ChannelNameId(char const (&channel_name)[n])
          : id(Id(
            #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
                (n > 0 ? (lower(channel_name[0]) << 0 ) : uint64_t{})
              | (n > 1 ? (lower(channel_name[1]) << 8 ) : uint64_t{})
              | (n > 2 ? (lower(channel_name[2]) << 16) : uint64_t{})
              | (n > 3 ? (lower(channel_name[3]) << 24) : uint64_t{})
              | (n > 4 ? (lower(channel_name[4]) << 32) : uint64_t{})
              | (n > 5 ? (lower(channel_name[5]) << 40) : uint64_t{})
              | (n > 6 ? (lower(channel_name[6]) << 48) : uint64_t{})
            #else
                (n > 0 ? (lower(channel_name[0]) << 55) : uint64_t{})
              | (n > 1 ? (lower(channel_name[1]) << 48) : uint64_t{})
              | (n > 2 ? (lower(channel_name[2]) << 40) : uint64_t{})
              | (n > 3 ? (lower(channel_name[3]) << 32) : uint64_t{})
              | (n > 4 ? (lower(channel_name[4]) << 24) : uint64_t{})
              | (n > 5 ? (lower(channel_name[5]) << 16) : uint64_t{})
              | (n > 6 ? (lower(channel_name[6]) << 8 ) : uint64_t{})
            #endif
          ))
        {}

        // for switch
        constexpr operator uint64_t () const noexcept
        {
            return uint64_t(this->id);
        }

        auto c_str() const noexcept
          -> char const (&)[8]
        {
            return reinterpret_cast<char const (&)[8]>(
                *reinterpret_cast<char const*>(&this->id)
            );
        }

        constexpr bool operator == (ChannelNameId const & other) const noexcept
        {
            return this->id == other.id;
        }

        constexpr bool operator != (ChannelNameId const & other) const noexcept
        {
            return this->id != other.id;
        }

    private:
        Id id;

        static constexpr uint64_t lower(char c)
        {
            return uint64_t(('A' <= c && c <= 'Z') ? c - 'A' + 'a' : c);
        }
    };

    inline char const * log_value(ChannelNameId const & channel_name)
    {
        return channel_name.c_str();
    }

    REDEMPTION_OSTREAM(out, ChannelNameId const & channel_name)
    {
        return out << channel_name.c_str();
    }
}

#define DEF_NAME(name) constexpr CHANNELS::ChannelNameId name = CHANNELS::ChannelNameId(#name)
namespace channel_names
{
    DEF_NAME(cliprdr);
    DEF_NAME(rdpdr);
    DEF_NAME(rdpsnd);
    DEF_NAME(drdynvc);
    DEF_NAME(rail);
    DEF_NAME(sespro);
}
#undef DEF_NAME

namespace CHANNELS {
    namespace channel_names = ::channel_names;
}

//         DEF_NAME(dynamique = "drdynvc";
//         DEF_NAME(plug_and_play_devices = "pnpdr";
//         DEF_NAME(video_redirection = "tsmf";
//         DEF_NAME(printer_ticket = "tsvctkt";
//         DEF_NAME(printer_driver = "xpsrd";
//         DEF_NAME(drive_letters = "wmsdl";
//         DEF_NAME(audio_level = "wmsaud";
//         DEF_NAME(audio_input = "audio_input";
//         DEF_NAME(file_system = "rdpdr";
//         DEF_NAME(clipboard = "cliprdr";
//         DEF_NAME(sound = "rdpsnd";
//         DEF_NAME(echo = "echo";
//         DEF_NAME(usb = "urbdrc";

