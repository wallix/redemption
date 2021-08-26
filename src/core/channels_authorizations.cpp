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

#include "core/channels_authorizations.hpp"

#include "core/RDP/channels/rdpdr.hpp"
#include "utils/sugar/split.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/strutils.hpp"
#include "utils/ref.hpp"

#include <algorithm>

#include <cassert>


namespace
{
    template<std::size_t n>
    auto c_array(char const * p) -> char const(&)[n]
    {
        return reinterpret_cast<char const(&)[n]>(*p); /*NOLINT*/
    }

    template<class Cont>
    bool contains_true(Cont const & cont)
    {
        for (bool x : cont) {
            if (x) {
                return true;
            }
        }
        return false;
    }

    template<std::size_t N>
    void normalize(
        std::vector<CHANNELS::ChannelNameId> & ids,
        std::vector<chars_view> & large_ids,
        CHANNELS::ChannelNameId channel_name,
        bool set,
        std::array<bool, N> & values,
        std::array<chars_view, N> restriction_names)
    {
        auto p = std::remove(ids.begin(), ids.end(), channel_name);
        if (p != ids.end()) {
            ids.erase(p, ids.end());
            values.fill(set);
            return;
        }

        if (large_ids.empty()) {
            return;
        }

        auto first = values.begin();
        for (auto name : restriction_names) {
            assert(name.back() == ',');
            name = name.first(name.size()-1u);
            for (auto && av : large_ids) {
                if (av.size() == name.size() && std::equal(av.begin(), av.end(), name.begin())) {
                    *first = set;
                    break;
                }
            }
            ++first;
        }
    }

    bool contains(
        array_view<CHANNELS::ChannelNameId> ids,
        CHANNELS::ChannelNameId id
    ) noexcept
    {
        return std::find(ids.begin(), ids.end(), id) != ids.end();
    }
} // namespace


ChannelsAuthorizations::ChannelsAuthorizations(std::string_view allow, std::string_view deny)
{
    std::vector<CHANNELS::ChannelNameId> allow_ids;
    std::vector<CHANNELS::ChannelNameId> deny_ids;
    std::vector<chars_view> allow_large_ids;
    std::vector<chars_view> deny_large_ids;

    auto extract = [](
        chars_view list,
        std::vector<CHANNELS::ChannelNameId> & ids,
        std::vector<chars_view> & large_ids
    ) {
        bool all = false;
        for (auto r : split_with(list, ',')) {
            auto trimmed = trim(r);
            if (trimmed.empty()) {
                continue;
            }

            if (trimmed[0] == '*') {
                all = true;
            }
            else {
                switch (trimmed.size()) {
                    case 0: break;
                    case 1: ids.emplace_back(c_array<1>(trimmed.begin())); break;
                    case 2: ids.emplace_back(c_array<2>(trimmed.begin())); break;
                    case 3: ids.emplace_back(c_array<3>(trimmed.begin())); break;
                    case 4: ids.emplace_back(c_array<4>(trimmed.begin())); break;
                    case 5: ids.emplace_back(c_array<5>(trimmed.begin())); break;
                    case 6: ids.emplace_back(c_array<6>(trimmed.begin())); break;
                    case 7: ids.emplace_back(c_array<7>(trimmed.begin())); break;
                    default: large_ids.emplace_back(trimmed.begin(), trimmed.end());
                }
            }
        }
        return all;
    };

    this->all_allow_ = extract(allow, allow_ids, allow_large_ids);
    this->all_deny_ = extract(deny, deny_ids, deny_large_ids);

    if (this->all_allow_ && !this->all_deny_) {
        this->rdpdr_restriction_.fill(true);
        this->cliprdr_restriction_.fill(true);
        this->rdpsnd_restriction_.fill(true);
        this->rdpcap_restriction_.fill(true);
    }

    auto normalize = [this](
        bool set,
        std::vector<CHANNELS::ChannelNameId> & ids,
        std::vector<chars_view> & large_ids
    ) {
        ::normalize(ids, large_ids, channel_names::cliprdr, set, this->cliprdr_restriction_, cliprde_list());
        ::normalize(ids, large_ids, channel_names::rdpdr, set, this->rdpdr_restriction_, rdpdr_list());
        ::normalize(ids, large_ids, channel_names::rdpsnd, set, this->rdpsnd_restriction_, rdpsnd_list());
        ::normalize(ids, large_ids, CHANNELS::ChannelNameId(), set, this->rdpcap_restriction_, rdpcap_list());
    };

    normalize(true, allow_ids, allow_large_ids);
    normalize(false, deny_ids, deny_large_ids);

    auto normalize_channel = [&](CHANNELS::ChannelNameId channel_name, bool is_allowed) {
        if (is_allowed) {
            allow_ids.emplace_back(channel_name);
        }
        else {
            deny_ids.emplace_back(channel_name);
        }
    };

    normalize_channel(
        channel_names::cliprdr,
        cliprdr_up_is_authorized() || cliprdr_down_is_authorized());
    bool const is_allowed = (
        contains_true(this->rdpdr_restriction_)
     || contains_true(this->rdpsnd_restriction_));
    normalize_channel(channel_names::rdpdr, is_allowed);
    normalize_channel(channel_names::rdpsnd, is_allowed);

    auto optimize = [](std::vector<CHANNELS::ChannelNameId> & ids) {
        auto cmp = [](CHANNELS::ChannelNameId name1, CHANNELS::ChannelNameId name2){
            return uint64_t(name1) < uint64_t(name2);
        };
        std::sort(ids.begin(), ids.end(), cmp);
        ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
    };

    optimize(allow_ids);
    optimize(deny_ids);

    this->allow_and_deny_.reserve(allow_ids.size() + deny_ids.size());
    this->allow_and_deny_.insert(this->allow_and_deny_.end(), allow_ids.begin(), allow_ids.end());
    this->allow_and_deny_.insert(this->allow_and_deny_.end(), deny_ids.begin(), deny_ids.end());
    this->allow_and_deny_pivot_ = allow_ids.size();
}

bool ChannelsAuthorizations::is_authorized(CHANNELS::ChannelNameId id) const noexcept
{
    auto rng_allow = array_view<CHANNELS::ChannelNameId>{this->allow_and_deny_.data(), this->allow_and_deny_.data() + this->allow_and_deny_pivot_};
    auto rng_deny = array_view<CHANNELS::ChannelNameId>{this->allow_and_deny_.data() + this->allow_and_deny_pivot_, this->allow_and_deny_.data() + this->allow_and_deny_.size()};
    if (this->all_deny_) {
        return contains(rng_allow, id);
    }
    if (this->all_allow_) {
        return !contains(rng_deny, id);
    }
    return !contains(rng_deny, id) && contains(rng_allow, id);
}

bool ChannelsAuthorizations::rdpdr_type_all_is_authorized() const noexcept
{
    return (this->rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL) &&
            this->rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT) &&
            this->rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM) &&
            this->rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD)
            );
}

bool ChannelsAuthorizations::rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP DeviceType) const noexcept
{
    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
    switch (DeviceType) {
        case rdpdr::RDPDR_DTYP_SERIAL:
        case rdpdr::RDPDR_DTYP_PARALLEL:
            return this->rdpdr_restriction_[1];

        case rdpdr::RDPDR_DTYP_PRINT:
            return this->rdpdr_restriction_[0];

        case rdpdr::RDPDR_DTYP_FILESYSTEM:
            return (this->rdpdr_restriction_[2] || this->rdpdr_restriction_[3]);

        case rdpdr::RDPDR_DTYP_SMARTCARD:
            return this->rdpdr_restriction_[4];

        default:
            assert(!"Unknown RDPDR DeviceType");
            return false;
    }
    REDEMPTION_DIAGNOSTIC_POP()
}

bool ChannelsAuthorizations::rdpdr_drive_read_is_authorized() const noexcept
{
    return this->rdpdr_restriction_[2];
}

bool ChannelsAuthorizations::rdpdr_drive_write_is_authorized() const noexcept
{
    return this->rdpdr_restriction_[3];
}

bool ChannelsAuthorizations::cliprdr_up_is_authorized() const noexcept
{
    return this->cliprdr_restriction_[0];
}

bool ChannelsAuthorizations::cliprdr_down_is_authorized() const noexcept
{
    return this->cliprdr_restriction_[1];
}

bool ChannelsAuthorizations::cliprdr_file_is_authorized() const noexcept
{
    return this->cliprdr_restriction_[2];
}

bool ChannelsAuthorizations::rdpsnd_audio_output_is_authorized() const noexcept
{
    return this->rdpsnd_restriction_[0];
}

bool ChannelsAuthorizations::rdpsnd_audio_input_is_authorized() const noexcept
{
    return this->rdpcap_restriction_[0];
}

std::pair<std::string,std::string>
compute_authorized_channels(
    std::string_view original_allow, std::string_view original_deny,
    std::string proxy_opt)
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

    while (!proxy_opt.empty() && proxy_opt.back() == ',') {
        proxy_opt.pop_back();
    }
    proxy_opt += ',';
    if (remove(proxy_opt, "RDP_DRIVE,")) {
        proxy_opt += "RDP_DRIVE_READ,RDP_DRIVE_WRITE";
    }
    if (!proxy_opt.empty() && proxy_opt.back() == ',') {
        proxy_opt.pop_back();
    }

    auto allow = str_concat(original_allow, ',');
    auto deny = str_concat(original_deny, ',');

    std::array<Ref<std::string>, 2> ret{{{allow}, {deny}}};

    for (std::string & s : ret) {
        remove(s, "cliprdr,");
        remove(s, "rdpdr,");
        remove(s, "rdpsnd,");
        // not name for audio capture because not a static channel
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

    static constexpr struct {
        std::string_view opt;
        std::string_view channel;
    } opts_channels[] {
        {"RDP_CLIPBOARD_UP",   ",cliprdr_up"          },
        {"RDP_CLIPBOARD_DOWN", ",cliprdr_down"        },
        {"RDP_CLIPBOARD_FILE", ",cliprdr_file"        },

        {"RDP_PRINTER",        ",rdpdr_printer"       },
        {"RDP_COM_PORT",       ",rdpdr_port"          },
        {"RDP_DRIVE_READ",     ",rdpdr_drive_read"    },
        {"RDP_DRIVE_WRITE",    ",rdpdr_drive_write"   },
        {"RDP_SMARTCARD",      ",rdpdr_smartcard"     },

        {"RDP_AUDIO_OUTPUT",   ",rdpsnd_audio_output" },

        {"RDP_AUDIO_INPUT",    ",rdpcap_audio_input" },
    };

    static_assert(
        decltype(ChannelsAuthorizations::cliprde_list())().size()
      + decltype(ChannelsAuthorizations::rdpdr_list())().size()
      + decltype(ChannelsAuthorizations::rdpsnd_list())().size()
      + decltype(ChannelsAuthorizations::rdpcap_list())().size()
    == std::extent<decltype(opts_channels)>::value
    , "opts_channels.size() error");

    for (auto & x : opts_channels) {
        ret[(proxy_opt.find(x.opt) != std::string::npos) ? 0 : 1].get() += x.channel;
    }

    for (std::string & s : ret) {
        if (!s.empty() && s.front() == ',') {
            s.erase(0,1);
        }
    }

    return {allow, deny};
}
