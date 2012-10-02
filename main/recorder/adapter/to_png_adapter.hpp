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

#if !defined(__MAIN_RECORDER_ADAPTER_TO_PNG_ADAPTER_HPP__)
#define __MAIN_RECORDER_ADAPTER_TO_PNG_ADAPTER_HPP__

#include "recorder/adapter.hpp"
#include "recorder/recorder_option.hpp"
#include "recorder/to_png.hpp"

class ToPngAdapter
: public RecorderAdapter
{
    RecorderOption& _option;

public:
    ToPngAdapter(RecorderOption& option)
    : _option(option)
    {}

    virtual void operator()(WRMRecorder& recorder, const char* outfile)
    {
        to_png(recorder, outfile,
               this->_option.range.left.time,
               this->_option.range.right.time,
               this->_option.time.time,
               this->_option.png_scale_width,
               this->_option.png_scale_height,
               this->_option.frame,
               this->_option.screenshot_start,
               this->_option.no_screenshot_stop,
               this->_option.screenshot_all
        );
    }
};

class ToPngListAdapter
: public RecorderAdapter
{
    RecorderOption& _option;

public:
    ToPngListAdapter(RecorderOption& option)
    : _option(option)
    {}

    virtual void operator()(WRMRecorder& recorder, const char* outfile)
    {
        to_png_2(recorder, outfile,
               this->_option.time_list,
               this->_option.png_scale_width,
               this->_option.png_scale_height,
               this->_option.no_screenshot_stop
        );
    }
};

#endif
