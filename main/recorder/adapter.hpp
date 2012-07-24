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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(__MAIN_RECORDER_ADAPTER_HPP__)
#define __MAIN_RECORDER_ADAPTER_HPP__

#include <utility>
#include <string>

#include "wrm_recorder.hpp"

struct RecorderAdapter
{
    virtual void operator()(WRMRecorder& recorder, const char* outfile) = 0;
};

typedef std::pair<std::string, RecorderAdapter*> RecorderAction;

RecorderAdapter* get_recorder_adapter(RecorderAction* first,
                                          RecorderAction* last,
                                          const std::string& extension)
{
    for (; first != last; ++first)
    {
        if (first->first == extension)
            return first->second;
    }
    return 0;
}

struct RecorderActionStringIterator
{
private:
    RecorderAction* _base;

public:
    RecorderActionStringIterator(RecorderAction* it)
    : _base(it)
    {}

    RecorderActionStringIterator& operator++()
    {
        ++this->_base;
        return *this;
    }

    const std::string& operator*() const
    {
        return this->_base->first;
    }

    bool operator==(const RecorderActionStringIterator& other)
    {
        return this->_base == other._base;
    }

    bool operator!=(const RecorderActionStringIterator& other)
    {
        return !(*this == other);
    }
};

#endif