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

#include <cstring>
#include <string>
#include <array>
#include <iosfwd>


struct AuthorizationChannels
: public movable_noncopyable
{
    AuthorizationChannels() = default;

    AuthorizationChannels(std::string allow, std::string deny)
    : allow_(std::move(allow))
    , deny_(std::move(deny))
    {
        this->normalize(this->allow_);
        this->normalize(this->deny_);
    }

    AuthorizationChannels(std::string allow, bool deny = true)
    : allow_(std::move(allow))
    , all_deny_(deny)
    , rdpdr_restriction_{{!deny, !deny, !deny, !deny, !deny}}
    , cliprdr_restriction_{{!deny, !deny}}
    {
        this->normalize(this->allow_);
    }

    AuthorizationChannels(bool allow, std::string deny)
    : deny_(std::move(deny))
    , all_allow_(allow)
    , rdpdr_restriction_{{allow, allow, allow, allow, allow}}
    , cliprdr_restriction_{{allow, allow}}
    {
        this->normalize(this->deny_);
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


    template<class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> & os, AuthorizationChannels const & auth) {
        auto p = [&](std::string const & s, bool all, bool val_ok, const char * name) {
            if (all) {
                os << name << "=*\n";
            }
            else {
                os << name << '=';
                for (size_t i = 0; i < auth.cliprdr_restriction_.size(); ++i) {
                    if (auth.cliprdr_restriction_[i] == val_ok) {
                        os << cliprde_list[i];
                    }
                }
                for (size_t i = 0; i < auth.rdpdr_restriction_.size(); ++i) {
                    if (auth.rdpdr_restriction_[i] == val_ok) {
                        os << rdpdr_list[i];
                    }
                }
                os << s << '\n';
            }
        };
        p(auth.allow_, auth.all_allow_, true, "allow");
        p(auth.deny_, auth.all_deny_, false, "deny");
        return os;
    }

private:
    template<class Cont>
    static bool contains_true(Cont const & cont) {
        for (bool x : cont) {
            if (x) {
                return true;
            }
        }
        return false;
    }

    template<std::size_t N>
    std::string normalize(
      std::string & s, bool set, std::array<bool, N> & values,
      const char * channel_name, std::array<char const *, N> restriction_names
    ) {
        bool has = false;
        bool add = false;

        auto pos = s.find(channel_name);
        if (pos != std::string::npos) {
            s.erase(pos, strlen(channel_name));
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

        return s;
    }

    static constexpr const std::array<const char *, 2> cliprde_list {{"cliprdr_up,", "cliprdr_down,"}};
    static constexpr const std::array<const char *, 5> rdpdr_list {{
        "rdpdr_general,", "rdpdr_printer,", "rdpdr_port,", "rdpdr_drive,", "rdpdr_smartcard,"
    }};

    void normalize(std::string & s) {
        const bool set = (&s == &this->allow_);
        if (!s.empty()) {
            s += ',';
            this->normalize(s, set, this->cliprdr_restriction_, "cliprdr,", cliprde_list);
            this->normalize(s, set, this->rdpdr_restriction_, "rdpdr,", rdpdr_list);
            if (!s.empty() && s.front() == ',') {
                s.erase(0, 1);
            }
        }
        if (set == contains_true(this->cliprdr_restriction_)) {
            s += "cliprdr,";
        }
        if (set == contains_true(this->rdpdr_restriction_)) {
            s += "rdpdr,";
        }
        if (!s.empty() && s.back() == ',') {
            s.pop_back();
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
    std::array<bool, 5> rdpdr_restriction_ {};
    std::array<bool, 2> cliprdr_restriction_ {};
};
constexpr decltype(AuthorizationChannels::cliprde_list) AuthorizationChannels::cliprde_list;
constexpr decltype(AuthorizationChannels::rdpdr_list) AuthorizationChannels::rdpdr_list;


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
