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

#ifndef REDEMPTION_UTILS_FILTERING_CHANNEL_HPP
#define REDEMPTION_UTILS_FILTERING_CHANNEL_HPP

#include "channel_list.hpp"
#include <iterator>
#include <cassert>

#include <iostream>

struct FilteringChannel {
    // The number of requested static virtual channels (the maximum allowed is 31).
    static const size_t max_filtering_channels = 31;
    class ChannelName {
        char name_[8]; /*short name*/

    public:
        ChannelName & operator=(const char * s) {
            char * end = std::end(this->name_) - 1;
            char * beg = this->name_;
            for (; beg != end && *s; ++beg, ++s) {
                *beg = *s;
            }
            assert(!*s);
            *beg = 0;
            return *this;
        }

        bool operator==(const char * s) const {
            const char * beg = this->name_;
            for (; *beg && *s; ++beg, ++s) {
                if (*beg != *s) {
                    return false;
                }
            }
            return !*beg && !*s;
        }

        bool empty() const {
            return !this->name_[0];
        }
    };

    ChannelName names[max_filtering_channels];
    ChannelName * pos = names;

public:
    void add_filter(const char * name) {
        assert(this->end() != this->begin() + this->max_size());
        *this->pos = name;
        ++this->pos;
    }

    bool contains(const char * s) const {
        for (ChannelName const & name : *this) {
            if (name == s) {
                return true;
            }
        }
        return false;
    }

    ChannelName const * begin() const {
        return this->names;
    }

    ChannelName const * end() const {
        return this->pos;
    }

    size_t size() const {
        return this->pos - this->names;
    }

    size_t max_size() const {
        return max_filtering_channels;
    }
};

#endif
