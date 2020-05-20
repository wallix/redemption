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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#pragma once

#include "core/RDP/channels/rdpdr.hpp"
#include "core/channel_names.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/std_stream_proto.hpp"
#include "utils/sugar/movable_noncopyable.hpp"

#include <vector>
#include <string>
#include <array>


class ChannelsAuthorizations
{
public:
    ChannelsAuthorizations() = default;

    ChannelsAuthorizations(std::string const & allow, std::string const & deny);

    [[nodiscard]] bool is_authorized(CHANNELS::ChannelNameId id) const noexcept;

    [[nodiscard]] bool rdpdr_type_all_is_authorized() const noexcept;
    [[nodiscard]] bool rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP DeviceType) const noexcept;
    [[nodiscard]] bool rdpdr_drive_read_is_authorized() const noexcept;
    [[nodiscard]] bool rdpdr_drive_write_is_authorized() const noexcept;

    [[nodiscard]] bool rdpsnd_audio_output_is_authorized() const noexcept;

    REDEMPTION_FRIEND_OSTREAM(out, ChannelsAuthorizations const & auth);

    [[nodiscard]] bool cliprdr_up_is_authorized() const noexcept;
    [[nodiscard]] bool cliprdr_down_is_authorized() const noexcept;
    [[nodiscard]] bool cliprdr_file_is_authorized() const noexcept;

public:
    static constexpr std::array<chars_view, 3> cliprde_list()
    {
        return {{
            cstr_array_view("cliprdr_up,"),
            cstr_array_view("cliprdr_down,"),
            cstr_array_view("cliprdr_file,"),
        }};
    }
    static constexpr std::array<chars_view, 5> rdpdr_list()
    {
        return {{
            cstr_array_view("rdpdr_printer,"),
            cstr_array_view("rdpdr_port,"),
            cstr_array_view("rdpdr_drive_read,"),
            cstr_array_view("rdpdr_drive_write,"),
            cstr_array_view("rdpdr_smartcard,"),
        }};
    }
    static constexpr std::array<chars_view, 1> rdpsnd_list()
    {
        return {{
            cstr_array_view("rdpsnd_audio_output,"),
        }};
    }

    // Boolean structures moved around in other parts of the code
    // could merely be restricted to what we have below
    // See equivalent fields in : core/file_system_virtual_channel_params.hpp
    // and core/clipboard_virtual_channels_params.hpp
    
//    struct {
//        bool up;    // client to server
//        bool down;  // server to client
//        bool file;  // file copy
//    } rdpsnd_acl;

//    struct {
//        bool up;    // client to server
//        bool down;  // server to client
//        bool file;  // file copy
//    } cliprdr_acl;

//    struct {
//        bool printer;
//        bool port;
//        bool drive_read;
//        bool drive_write;
//        bool smartcard;
//    } rdpdr_acl;

    std::vector<CHANNELS::ChannelNameId> allow_and_deny_;
    size_t allow_and_deny_pivot_;
    bool all_allow_ = false;
    bool all_deny_ = false;
    // TODO array<bool, n> -> bitset<n> | uint8_t | enum
    std::array<bool, decltype(rdpdr_list())().size()> rdpdr_restriction_ {{}};
    std::array<bool, decltype(cliprde_list())().size()> cliprdr_restriction_ {{}};
    std::array<bool, decltype(rdpsnd_list())().size()> rdpsnd_restriction_ {{}};
};

inline static std::pair<std::string,std::string> update_authorized_channels(std::string allow, std::string deny, std::string proxy_opt)
{
    auto remove = [](std::string & str, std::string_view pattern) -> bool {
        bool removed = false;
        size_t pos = 0;
        while ((pos = str.find(pattern, pos)) != std::string::npos) {
            str.erase(pos, pattern.size());
            removed = true;
        }

        return removed;
    };

    std::string expanded_proxy_opt = proxy_opt;
    while (!expanded_proxy_opt.empty() && expanded_proxy_opt.back() == ',') {
        expanded_proxy_opt.pop_back();
    }
    expanded_proxy_opt += ',';
    if (remove(expanded_proxy_opt, "RDP_DRIVE,")) {
        expanded_proxy_opt += "RDP_DRIVE_READ,RDP_DRIVE_WRITE";
    }
    if (!expanded_proxy_opt.empty() && expanded_proxy_opt.back() == ',') {
        expanded_proxy_opt.pop_back();
    }

    allow += ',';
    deny += ',';

    struct ref_string {
        std::string & s;
        std::string & get() { return this->s; }
        operator std::string & () { return this->s; }
    };

    std::array<ref_string, 2> ret{{{allow}, {deny}}};

    for (std::string & s : ret) {
        remove(s, "cliprdr,");
        remove(s, "rdpdr,");
        remove(s, "rdpsnd,");
        for (auto str : ChannelsAuthorizations::cliprde_list()) {
            remove(s, {str.data(), str.size()});
        }
        for (auto str : ChannelsAuthorizations::rdpdr_list()) {
            remove(s, {str.data(), str.size()});
        }
        for (auto str : ChannelsAuthorizations::rdpsnd_list()) {
            remove(s, {str.data(), str.size()});
        }
        if (!s.empty() && s.back() == ',') {
            s.pop_back();
        }
    }

    constexpr struct {
        const char * opt;
        const char * channel;
    } opts_channels[] {
        {"RDP_CLIPBOARD_UP",   ",cliprdr_up"          },
        {"RDP_CLIPBOARD_DOWN", ",cliprdr_down"        },
        {"RDP_CLIPBOARD_FILE", ",cliprdr_file"        },

        {"RDP_PRINTER",        ",rdpdr_printer"       },
        {"RDP_COM_PORT",       ",rdpdr_port"          },
        {"RDP_DRIVE_READ",     ",rdpdr_drive_read"    },
        {"RDP_DRIVE_WRITE",    ",rdpdr_drive_write"   },
        {"RDP_SMARTCARD",      ",rdpdr_smartcard"     },

        {"RDP_AUDIO_OUTPUT",   ",rdpsnd_audio_output" }
    };

    static_assert(
        decltype(ChannelsAuthorizations::cliprde_list())().size()
        + decltype(ChannelsAuthorizations::rdpdr_list())().size()
        + decltype(ChannelsAuthorizations::rdpsnd_list())().size()
    == std::extent<decltype(opts_channels)>::value
    , "opts_channels.size() error");

    for (auto & x : opts_channels) {
        ret[(expanded_proxy_opt.find(x.opt) != std::string::npos) ? 0 : 1].get() += x.channel;
    }

    for (std::string & s : ret) {
        if (!s.empty() && s.front() == ',') {
            s.erase(0,1);
        }
    }
    return {allow,deny};
}

