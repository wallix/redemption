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

#include "core/channel_names.hpp"
#include "utils/sugar/array_view.hpp"

#include <vector>
#include <string>
#include <string_view>
#include <array>

namespace rdpdr
{
    enum RDPDR_DTYP : uint32_t;
}

class ChannelsAuthorizations
{
public:
    ChannelsAuthorizations() = default;

    ChannelsAuthorizations(std::string_view allow, std::string_view deny);

    [[nodiscard]] bool is_authorized(CHANNELS::ChannelNameId id) const noexcept;

    [[nodiscard]] bool rdpdr_type_all_is_authorized() const noexcept;
    [[nodiscard]] bool rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP DeviceType) const noexcept;
    [[nodiscard]] bool rdpdr_drive_read_is_authorized() const noexcept;
    [[nodiscard]] bool rdpdr_drive_write_is_authorized() const noexcept;

    [[nodiscard]] bool rdpsnd_audio_output_is_authorized() const noexcept;
    [[nodiscard]] bool rdpsnd_audio_input_is_authorized() const noexcept;

    [[nodiscard]] bool cliprdr_up_is_authorized() const noexcept;
    [[nodiscard]] bool cliprdr_down_is_authorized() const noexcept;
    [[nodiscard]] bool cliprdr_file_is_authorized() const noexcept;

public:
    static constexpr std::array<chars_view, 3> cliprde_list()
    {
        return {{
            "cliprdr_up,"_av,
            "cliprdr_down,"_av,
            "cliprdr_file,"_av,
        }};
    }

    static constexpr std::array<chars_view, 5> rdpdr_list()
    {
        return {{
            "rdpdr_printer,"_av,
            "rdpdr_port,"_av,
            "rdpdr_drive_read,"_av,
            "rdpdr_drive_write,"_av,
            "rdpdr_smartcard,"_av,
        }};
    }

    static constexpr std::array<chars_view, 1> rdpsnd_list()
    {
        return {{
            "rdpsnd_audio_output,"_av,
        }};
    }

    static constexpr std::array<chars_view, 1> rdpcap_list()
    {
        return {{
            "rdpcap_audio_input,"_av,
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

private:
    std::vector<CHANNELS::ChannelNameId> allow_and_deny_;
    size_t allow_and_deny_pivot_;
    bool all_allow_ = false;
    bool all_deny_ = false;
    // TODO array<bool, n> -> bitset<n> | uint8_t | enum
    std::array<bool, decltype(rdpdr_list())().size()> rdpdr_restriction_ {{}};
    std::array<bool, decltype(cliprde_list())().size()> cliprdr_restriction_ {{}};
    std::array<bool, decltype(rdpsnd_list())().size()> rdpsnd_restriction_ {{}};
    std::array<bool, decltype(rdpcap_list())().size()> rdpcap_restriction_ {{}};
};

std::pair<std::string,std::string>
compute_authorized_channels(
    std::string_view original_allow, std::string_view original_deny,
    std::string proxy_opt);
