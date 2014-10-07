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

#include "channel_list.hpp"
#include "splitter.hpp"
#include "string.hpp"
#include "iter.hpp"
#include "log.hpp"
#include <iterator>
#include <cassert>


struct AuthorizationChannels {
    static const size_t max_authorization_channels = CHANNELS::MAX_STATIC_VIRTUAL_CHANNELS;

    class ChannelNameArray
    {
        class ChannelName
        {
            char name_[CHANNELS::ChannelDef::max_size_name+1];

        public:
            void assign(const char * s, std::size_t n) {
                std::copy(s, s + n, std::begin(this->name_));
                this->name_[n] = 0;
            }

            bool operator==(const char * s) const noexcept {
                return !strcmp(this->name_, s);
            }

            bool empty() const noexcept {
                return !this->name_[0];
            }
        };

        ChannelName names[max_authorization_channels];
        ChannelName * pos = names;

        friend class AuthorizationChannels;

        ChannelNameArray() = default;
        ChannelNameArray(const ChannelNameArray &) = delete;
        ChannelNameArray& operator=(const ChannelNameArray &) = delete;

    public:
        void push_back(const char * name, std::size_t n) {
            assert((this->pos != this->names + this->max_size()) && n <= CHANNELS::ChannelDef::max_size_name);
            this->pos->assign(name, n);
            ++this->pos;
        }

        constexpr static size_t max_size() {
            return max_authorization_channels;
        }

        bool contains(const char * s) const {
            for (ChannelName const & name : iter(this->names, this->pos)) {
                if (name == s) {
                    return true;
                }
            }
            return false;
        }

        size_t size() const {
            return this->pos - this->names;
        }
    };

private:
    ChannelNameArray allow_;
    ChannelNameArray deny_;
    bool all_deny_ = false;

public:
    ChannelNameArray & allow() {
        return this->allow_;
    }

    ChannelNameArray & deny() {
        return this->deny_;
    }

    void all_is_deny(bool x) {
        this->all_deny_ = x;
    }

    bool authorized(const char * s) const {
        if (this->all_deny_) {
            if (this->allow_.contains(s)) {
                return true;
            }
            return false;
        }
        if (this->deny_.contains(s)) {
            return false;
        }
        return true;
    }

    constexpr static size_t max_size() {
        return max_authorization_channels;
    }
};


void initalize_authorization_channels(
    AuthorizationChannels & authorization_channels,
    const redemption::string & allow,
    const redemption::string & deny
) {
    struct add_impl {
        static void add(
            AuthorizationChannels::ChannelNameArray & channel_array,
            const redemption::string & s, const char * type_error
        ) {
            for (auto & r : get_split(s.c_str(), s.c_str() + s.length(), ',')) {
                if (r.size() > CHANNELS::ChannelDef::max_size_name) {
                    LOG(LOG_WARNING, "%s: to many channels (max is: %d).",
                        type_error, int(CHANNELS::ChannelDef::max_size_name));
                }
                else if (channel_array.size() == channel_array.max_size()) {
                    LOG(LOG_WARNING, "%s is full", type_error);
                }
                else {
                    channel_array.push_back(r.begin(), r.size());
                }
            }
        }
    };

    if (deny.length() == 1 && deny.c_str()[0] == '*') {
        authorization_channels.all_is_deny(true);
    }
    else {
        add_impl::add(authorization_channels.deny(), deny, "deny_channels");
    }

    if (!(allow.length() == 1 && allow.c_str()[0] == '*')) {
        add_impl::add(authorization_channels.allow(), allow, "allow_channels");
    }
}

#endif
