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

#include "splitter.hpp"
#include "string.hpp"
#include <iterator>
#include <string>

struct AuthorizationChannels
{
    AuthorizationChannels() = default;

    AuthorizationChannels(std::string allow, std::string deny)
    : allow_(std::move(allow))
    , deny_(std::move(deny))
    {}

    AuthorizationChannels(std::string allow, bool deny = true)
    : allow_(std::move(allow))
    , all_deny_(deny)
    {}

    AuthorizationChannels(bool allow, std::string deny)
    : deny_(std::move(deny))
    , all_allow_(allow)
    {}

    AuthorizationChannels(AuthorizationChannels &&) = default;
    AuthorizationChannels& operator=(AuthorizationChannels &&) = default;

    AuthorizationChannels(AuthorizationChannels const &) = delete;
    AuthorizationChannels& operator=(AuthorizationChannels const &) = delete;

    bool authorized(const char * s) const {
        if (this->all_deny_) {
            return this->contains(this->allow_, s);
        }
        if (this->all_allow_) {
            return !this->contains(this->deny_, s);
        }
        return !this->contains(this->deny_, s)
            &&  this->contains(this->allow_, s);
    }

private:
    bool contains(std::string const & s, const char * search) const {
        const std::size_t len = strlen(search);
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
    if (deny.length() == 1 && deny.c_str()[0] == '*') {
        return AuthorizationChannels(std::string(allow.c_str(), allow.length()), true);
    }
    else if (allow.length() == 1 && allow.c_str()[0] == '*') {
        return AuthorizationChannels(true, std::string(deny.c_str(), deny.length()));
    }
    else {
        return AuthorizationChannels(
            std::string(allow.c_str(), allow.length())
          , std::string(deny.c_str(), deny.length())
        );
    }
}

#endif
