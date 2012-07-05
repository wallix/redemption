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

#if !defined(__MAIN_RECORDER_GET_RECORDER_ACTION__)
#define __MAIN_RECORDER_GET_RECORDER_ACTION__

#include <utility>
#include <string>
#include "wrm_recorder.hpp"

template<typename _WrmRecorderOption>
struct recorder_item_traits
{
    typedef void(*action_type)(WRMRecorder&, _WrmRecorderOption&, const char *);
    typedef std::pair<std::string, action_type> recorder_item;
};

template<typename _RecorderAction, typename _ForwardIterator>
_RecorderAction get_recorder_action(_ForwardIterator first,
                                    _ForwardIterator last,
                                    const std::string& extension)
{
    for (; first != last; ++first)
    {
        if (first->first == extension)
            return first->second;
    }
    return 0;
}

/*template<typename _WrmRecorderOption, std::size_t _N>
typename recorder_item_traits<_WrmRecorderOption>::action_type
get_recorder_action(typename recorder_item_traits<
                        _WrmRecorderOption
                    >::recorder_item (&actions)[_N],
                    const std::string& extension)
{
    return get_recorder_action<
        typename recorder_item_traits<_WrmRecorderOption>::action_type
    >(actions + 0, actions + _N, extension);
}*/

#endif