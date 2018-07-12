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

#include "utils/authorization_channels.hpp"

#include "utils/sugar/splitter.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/array_view.hpp"

#include <algorithm>
#include <vector>
#include <string>
#include <array>

#include <cassert>


namespace
{
    template<std::size_t n>
    auto c_array(char const * p) -> char const(&)[n]
    {
        return reinterpret_cast<char const(&)[n]>(*p);
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
        std::vector<array_view_const_char> & large_ids,
        CHANNELS::ChannelNameId channel_name,
        bool set,
        std::array<bool, N> & values,
        std::array<array_view_const_char, N> restriction_names)
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
            name = {name.data(), name.size()-1u};
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
        array_view<CHANNELS::ChannelNameId const> ids,
        CHANNELS::ChannelNameId id
    ) noexcept
    {
        return std::find(ids.begin(), ids.end(), id) != ids.end();
    }
} // namespace


AuthorizationChannels::AuthorizationChannels(std::string const & allow, std::string const & deny)
{
    std::vector<CHANNELS::ChannelNameId> allow_ids;
    std::vector<CHANNELS::ChannelNameId> deny_ids;
    std::vector<array_view_const_char> allow_large_ids;
    std::vector<array_view_const_char> deny_large_ids;

    auto extract = [](
        array_view_const_char list,
        std::vector<CHANNELS::ChannelNameId> & ids,
        std::vector<array_view_const_char> & large_ids
    ) {
        bool all = false;
        for (auto && r : get_split(list.begin(), list.end(), ',')) {
            auto trimmed = trim(begin(r), end(r));
            if (trimmed.empty()) {
                continue;
            }
            if (trimmed[0] == '*') {
                all = true;
            }
            else switch (trimmed.size()) {
                case 0: break;
                case 1: ids.emplace_back(c_array<1>(trimmed.begin())); break;
                case 2: ids.emplace_back(c_array<2>(trimmed.begin())); break;
                case 3: ids.emplace_back(c_array<3>(trimmed.begin())); break;
                case 4: ids.emplace_back(c_array<4>(trimmed.begin())); break;
                case 5: ids.emplace_back(c_array<5>(trimmed.begin())); break;
                case 6: ids.emplace_back(c_array<6>(trimmed.begin())); break;
                case 7: ids.emplace_back(c_array<7>(trimmed.begin())); break;
                default: large_ids.push_back({trimmed.begin(), trimmed.end()});
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
    }

    auto normalize = [this](
        bool set,
        std::vector<CHANNELS::ChannelNameId> & ids,
        std::vector<array_view_const_char> & large_ids
    ) {
        ::normalize(ids, large_ids, channel_names::cliprdr, set, this->cliprdr_restriction_, cliprde_list());
        ::normalize(ids, large_ids, channel_names::rdpdr, set, this->rdpdr_restriction_, rdpdr_list());
        ::normalize(ids, large_ids, channel_names::rdpsnd, set, this->rdpsnd_restriction_, rdpsnd_list());
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
    this->allow_and_deny_pivot_ = this->allow_and_deny_.data() + allow_ids.size();
}

bool AuthorizationChannels::is_authorized(CHANNELS::ChannelNameId id) const noexcept
{
    if (this->all_deny_) {
        return contains(this->rng_allow(), id);
    }
    if (this->all_allow_) {
        return !contains(this->rng_deny(), id);
    }
    return !contains(this->rng_deny(), id)
        &&  contains(this->rng_allow(), id);
}

bool AuthorizationChannels::rdpdr_type_all_is_authorized() const noexcept
{
    return (this->rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL) &&
            this->rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT) &&
            this->rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM) &&
            this->rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD)
            );
}

bool AuthorizationChannels::rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP DeviceType) const noexcept
{
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
}

bool AuthorizationChannels::rdpdr_drive_read_is_authorized() const noexcept
{
    return this->rdpdr_restriction_[2];
}

bool AuthorizationChannels::rdpdr_drive_write_is_authorized() const noexcept
{
    return this->rdpdr_restriction_[3];
}

bool AuthorizationChannels::cliprdr_up_is_authorized() const noexcept
{
    return this->cliprdr_restriction_[0];
}

bool AuthorizationChannels::cliprdr_down_is_authorized() const noexcept
{
    return this->cliprdr_restriction_[1];
}

bool AuthorizationChannels::cliprdr_file_is_authorized() const noexcept
{
    return this->cliprdr_restriction_[2];
}

bool AuthorizationChannels::rdpsnd_audio_output_is_authorized() const noexcept
{
    return this->rdpsnd_restriction_[0];
}

REDEMPTION_OSTREAM(out, AuthorizationChannels const & auth)
{
    auto p = [&](
        std::vector<CHANNELS::ChannelNameId> const & ids,
        bool all, bool val_ok, const char * name
    ) {
        if (all) {
            out << name << "=*\n";
        }
        else {
            out << name << '=';
            for (size_t i = 0; i < auth.cliprdr_restriction_.size(); ++i) {
                if (auth.cliprdr_restriction_[i] == val_ok) {
                    out << AuthorizationChannels::cliprde_list()[i].data();
                }
            }
            for (size_t i = 0; i < auth.rdpdr_restriction_.size(); ++i) {
                if (auth.rdpdr_restriction_[i] == val_ok) {
                    out << AuthorizationChannels::rdpdr_list()[i].data();
                }
            }
            for (size_t i = 0; i < auth.rdpsnd_restriction_.size(); ++i) {
                if (auth.rdpsnd_restriction_[i] == val_ok) {
                    out << AuthorizationChannels::rdpsnd_list()[i].data();
                }
            }

            for (auto && id : ids) {
                out << id << ',';
            }
            out << '\n';
        }
    };
    p(auth.rng_allow(), auth.all_allow_, true, "allow");
    p(auth.rng_deny(), auth.all_deny_, false, "deny");
    return out;
}

// TODO review
void AuthorizationChannels::update_authorized_channels(
    std::string & allow, std::string & deny, const std::string & proxy_opt)
{
    auto remove = [] (std::string & str, const char * pattern) -> bool {
        bool removed = false;
        size_t pos = 0;
        while ((pos = str.find(pattern, pos)) != std::string::npos) {
            str.erase(pos, strlen(pattern));
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
        for (auto str : AuthorizationChannels::cliprde_list()) {
            remove(s, str.data());
        }
        for (auto str : AuthorizationChannels::rdpdr_list()) {
            remove(s, str.data());
        }
        for (auto str : AuthorizationChannels::rdpsnd_list()) {
            remove(s, str.data());
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
        decltype(AuthorizationChannels::cliprde_list())().size()
        + decltype(AuthorizationChannels::rdpdr_list())().size()
        + decltype(AuthorizationChannels::rdpsnd_list())().size()
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
}

inline array_view<CHANNELS::ChannelNameId const> AuthorizationChannels::rng_allow() const
{
    return {this->allow_and_deny_.data(), this->allow_and_deny_pivot_};
}

inline array_view<CHANNELS::ChannelNameId const> AuthorizationChannels::rng_deny() const
{
    return {this->allow_and_deny_pivot_, this->allow_and_deny_.data() + this->allow_and_deny_.size()};
}
