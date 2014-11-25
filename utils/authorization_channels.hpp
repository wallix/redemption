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
#include <iterator>
#include <cstring>
#include <string>

struct AuthorizationChannels
: public movable_noncopyable
{
    AuthorizationChannels() = default;

    AuthorizationChannels(std::string allow, std::string deny)
    : allow_(normalize(std::move(allow)))
    , deny_(normalize(std::move(deny)))
    {}

    AuthorizationChannels(std::string allow, bool deny = true)
    : allow_(normalize(std::move(allow)))
    , all_deny_(deny)
    {}

    AuthorizationChannels(bool allow, std::string deny)
    : deny_(normalize(std::move(deny)))
    , all_allow_(allow)
    {}

    bool authorized(const char * s) const noexcept {
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

private:
    static std::string normalize(std::string s) {
        constexpr const char * clipboard = "cliprdr";
        if (contains(s, clipboard, strlen(clipboard))) {
            s += ",cliprdr_down,cliprdr_up";
        }
        else {
            constexpr const char * clipboard_up = "cliprdr_up";
            constexpr const char * clipboard_down = "cliprdr_down";
            if (contains(s, clipboard_up, strlen(clipboard_up))
             && contains(s, clipboard_down, strlen(clipboard_down))) {
                s += ",cliprdr";
            }
        }
        return s;
    }

    static bool contains(std::string const & s, const char * search, std::size_t len) noexcept {
        for (auto & r : get_split(s, ',')) {
            if (r.size() == len && std::equal(r.begin(), r.end(), search)) {
                return true;
            }
        }
        return false;
    }

    std::string allow_;
    std::string deny_;
    bool all_allow_ = false;
    bool all_deny_ = false;
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

#endif
