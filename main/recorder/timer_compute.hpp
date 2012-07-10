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

#if !defined(__MAIN_RECORDER_TIMER_COMPUTE__)
#define __MAIN_RECORDER_TIMER_COMPUTE__

#include "wrm_recorder.hpp"

class TimerCompute {
    uint64_t micro_sec;

public:
    static const uint64_t coeff_sec_to_usec = 1000000;

public:
    WRMRecorder& recorder;
    uint64_t chunk_time_value;

public:
    TimerCompute(WRMRecorder& recorder)
    : micro_sec(0)
    , recorder(recorder)
    , chunk_time_value(0)
    {}

    void reset()
    {
        this->micro_sec = 0;
    }

    uint64_t& usec()
    { return this->micro_sec; }

    uint64_t sec()
    { return this->micro_sec / coeff_sec_to_usec; }

    void interpret_time()
    {
        this->chunk_time_value = this->recorder.reader.stream.in_uint64_be();
        this->micro_sec += this->chunk_time_value;
        --this->recorder.remaining_order_count();
    }

    bool interpret_is_time_chunk()
    {
        if (recorder.chunk_type() == WRMChunk::TIMESTAMP)
        {
            //std::cout << this->micro_sec << " -> ";
            this->interpret_time();
            //std::cout << this->micro_sec << " (" << this->chunk_time_value << ")\n";
            return true;
        }
        return false;
    }

    timeval start()
    {
        while (this->recorder.selected_next_order())
        {
            if (this->recorder.chunk_type() == WRMChunk::TIME_START)
            {
                return this->recorder.get_start_time_order();
            }
            if (this->recorder.chunk_type() == WRMChunk::TIMESTAMP)
            {
                this->interpret_time();
                timeval ret = {0,0};
                return ret;
            }
            this->recorder.interpret_order();
        }
        timeval ret = {0,0};
        return ret;
    }

    uint64_t advance_usecond(uint msec)
    {
        if (!msec)
            return 0;

        if (this->micro_sec >= msec)
        {
            uint64_t tmp = this->micro_sec;
            this->reset();
            return tmp;
        }

        while (this->recorder.selected_next_order())
        {
            if (this->interpret_is_time_chunk())
            {
                if (this->micro_sec >= msec)
                {
                    uint64_t tmp = this->micro_sec;
                    this->reset();
                    return tmp;
                }
            }
            else
                this->recorder.interpret_order();
        }
        return 0;
    }

    uint64_t advance_second(uint second)
    {
        return this->advance_usecond(coeff_sec_to_usec * second);
    }
};

#endif