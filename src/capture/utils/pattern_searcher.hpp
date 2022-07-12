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
*   Copyright (C) Wallix 2012-2015
*   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier,
*              Jonathan Poelen, Raphael Zhou, Meng Tan
*/

#pragma once

#include "utils/sugar/array_view.hpp"
#include "capture/utils/pattutils.hpp"


struct PatternSearcher
{
    explicit PatternSearcher(
        array_view<CapturePattern> cap_patterns_kill,
        array_view<CapturePattern> cap_patterns_notify,
        CapturePattern::CaptureType cap_type);

    PatternSearcher(PatternSearcher const&) = delete;
    PatternSearcher& operator=(PatternSearcher const&) = delete;

    ~PatternSearcher();


    bool has_pattern() const;

    void reset_kbd_streams();

    struct PatternFound
    {
        chars_view pattern;
        unsigned id;
        bool is_pattern_kill;
        // size from end of data
        unsigned long long match_len;
    };

    array_view<PatternFound> scan(chars_view str);

private:
    struct D;
    D* d = nullptr;
};
