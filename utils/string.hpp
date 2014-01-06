/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_UTILS_STRING_HPP_
#define _REDEMPTION_UTILS_STRING_HPP_

#include "error.hpp"
#include "log.hpp"
#include <string>

namespace redemption {

class string {
private:
    std::string internal_string;

public:
    string() : internal_string() {}

    string(const char * source) : internal_string(source) {}

    string(const string & source) : internal_string(source.internal_string) {}

    string(const char * source, size_t length) : internal_string(source, length) {}

private:
    string & operator=(const char * source);

    string & operator=(const string & source);

public:
    const char * c_str() const {
        return this->internal_string.c_str();
    }

    void concatenate_c_str(const char * source) {
        this->internal_string += source;
    }

    void concatenate_str(const string & source) {
        this->internal_string += source.internal_string;
    }

    void copy_c_str(const char * source) {
        this->internal_string = source;
    }

    void copy_str(const string & source) {
        this->internal_string = source.internal_string;
    }

    void empty() {
        this->internal_string.clear();
    }

    size_t find(const char * s, size_t pos = 0) const
    {
        return this->internal_string.find(s, pos);
    }

    bool is_empty() const {
        return this->internal_string.empty();
    }

    size_t length() const {
        return this->internal_string.length();
    }
};  // class string

}   // namespace redemption

#endif  // #ifndef _REDEMPTION_UTILS_STRING_HPP_
