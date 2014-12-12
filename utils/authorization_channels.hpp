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

#ifndef REDEMPTION_UTILS_AUTHORIZATION_CHANNELS_HPP
#define REDEMPTION_UTILS_AUTHORIZATION_CHANNELS_HPP

#include "movable_noncopyable.hpp"
#include "splitter.hpp"
#include "string.hpp"

#include <functional> // std:ref
#include <iterator>
#include <cstring>
#include <string>
#include <array>


struct AuthorizationChannels
: public movable_noncopyable
{
    AuthorizationChannels() = default;

    AuthorizationChannels(std::string allow, std::string deny)
    : allow_(std::move(allow))
    , deny_(std::move(deny))
    {
        this->normalize();
    }

    AuthorizationChannels(std::string allow, bool deny = true)
    : allow_(std::move(allow))
    , all_deny_(deny)
    {
        this->normalize();
    }

    AuthorizationChannels(bool allow, std::string deny)
    : deny_(std::move(deny))
    , all_allow_(allow)
    {
        this->normalize();
    }

    bool is_authorized(const char * s) const noexcept {
        const std::size_t len = strlen(s);
        if (this->all_deny_) {
            return contains(this->allow_, s, len);
        }
        if (this->all_allow_) {
            return !contains(this->deny_, s, len);
        }
        return !contains(this->deny_, s, len)
            &&  contains(this->allow_, s, len);
    }

    bool rdpdr_type_is_authorized(unsigned type) const noexcept {
        return type - 1u < this->rdpdr_restriction_.size() && this->rdpdr_restriction_[type - 1];
    }

    bool cliprdr_up_is_authorized() const noexcept {
        return this->cliprdr_restriction_[0];
    }

    bool cliprdr_down_is_authorized() const noexcept {
        return this->cliprdr_restriction_[1];
    }

private:
    template<std::size_t N>
    std::string normalize(
      std::string & s, bool set, std::array<bool, N> & values,
      const char * channel_name, std::initializer_list<char const *> restriction_names
    ) {
        bool has = false;
        bool add = false;

        auto pos = s.find(channel_name);
        if (pos != std::string::npos) {
            for (auto & x : values) {
                x = set;
            }
            has = true;
        }

        auto first = values.begin();
        for (auto name : restriction_names) {
            pos = s.find(name);
            if (pos != std::string::npos) {
                s.erase(pos, strlen(name));
                *first = set;
                add = true;
            }
            ++first;
        }

        if (!has && add) {
            s += channel_name;
        }

        return s;
    }

    void normalize() {
        typedef std::initializer_list<const char *> list_t;
        const list_t cliprde {"cliprdr_up,", "cliprdr_down,"};
        const list_t rdpdr {"rdpdr_general,", "rdpdr_printer,", "rdpdr_port,", "rdpdr_drive,", "rdpdr_smartcard,"};

        auto l = {std::ref(this->allow_), std::ref(this->deny_)};
        for (unsigned i = 0; i < l.size(); ++i) {
            std::string & s = l.begin()[i];
            if (!s.empty()) {
                s += ',';
                this->normalize(s, !i, this->cliprdr_restriction_, "cliprdr,", cliprde);
                this->normalize(s, !i, this->rdpdr_restriction_, "rdpdr,", rdpdr);
            }
        }
    }

    static bool contains(std::string const & s, const char * search, std::size_t len) noexcept {
        for (auto & r : get_split(s, ',')) {
            if (r.size() == len && std::equal(r.begin(), r.end(), search)) {
                return true;
            }
        }
        return false;
        //auto pos = s.find(search);
        //return (pos != std::string::npos && s[pos + len] == ',');
    }

    std::string allow_;
    std::string deny_;
    bool all_allow_ = false;
    bool all_deny_ = false;
    std::array<bool, 5> rdpdr_restriction_;
    std::array<bool, 2> cliprdr_restriction_;
};


AuthorizationChannels make_authorization_channels(const redemption::string & allow, const redemption::string & deny) {
    if (deny.length() == 1 && deny.str()[0] == '*') {
        return AuthorizationChannels(allow.str(), true);
    }
    else if (allow.length() == 1 && allow.str()[0] == '*') {
        return AuthorizationChannels(true, deny.str());
    }
    else {
        return AuthorizationChannels(allow.str(), deny.str());
    }
}

std::array<std::string, 2> update_authorized_channels(const std::string & allow,
                                                      const std::string & deny,
                                                      const std::string & proxy_opt) {
    auto remove=[](std::string & str, const char * pattern) {
        auto pos = str.find(pattern);
        if (pos != std::string::npos) {
            str.erase(pos, strlen(pattern));
        }
    };

    std::array<std::string, 2> ret{{allow + ',', deny + ','}};

    for (std::string & s : ret) {
        remove(s, "cliprdr,");
        remove(s, "cliprdr_up,");
        remove(s, "cliprdr_down,");
        remove(s, "rdpdr,");
        remove(s, "rdpdr_general,");
        remove(s, "rdpdr_printer,");
        remove(s, "rdpdr_port,");
        remove(s, "rdpdr_drive,");
        remove(s, "rdpdr_smartcard,");
        if (!s.empty() && s.back() == ',') {
            s.pop_back();
        }
    }

    constexpr struct {
        const char * opt;
        const char * channel;
    } opts_channels[] {
        {"RDP_CLIPBOARD_UP", ",cliprdr_up"},
        {"RDP_CLIPBOARD_DOWN", ",cliprdr_down"},
        {"RDP_DEVICE_REDIRECTION", ",rdpdr"},
    };

    for (auto & x : opts_channels) {
        ret[(proxy_opt.find(x.opt) != std::string::npos) ? 0 : 1] += x.channel;
    }

    for (std::string & s : ret) {
        if (!s.empty() && s.front() == ',') {
            s.erase(0,1);
        }
    }

    return ret;
}

#endif
