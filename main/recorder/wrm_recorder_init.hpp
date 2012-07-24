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

#if !defined(__MAIN_RECORDER_WRM_RECORDER_INIT_HPP__)
#define __MAIN_RECORDER_WRM_RECORDER_INIT_HPP__

#include <iostream>

#include <error.hpp>
#include "wrm_recorder.hpp"
#include "wrm_recorder_option.hpp"

inline static int __wrm_recorder_init_meta_not_found(WRMRecorder& recorder,
                                                     WrmRecorderOption& opt)
{
    std::cerr << recorder.chunk_type() << '\n'
    << recorder.meta() << '\n'
    << "Chunk META not found in " << opt.in_filename << std::endl;
    return 2004;
}

inline static int __wrm_recorder_init_idx_not_found(WRMRecorder& recorder,
                                                    WrmRecorderOption& opt)
{
    std::cerr << "idx " << opt.idx_start << " not found" << std::endl;
    return 2002;
}

inline static uint __wrm_recorder_init_get_good_idx(WRMRecorder& recorder,
                                                    WrmRecorderOption& opt)
{
    if (opt.times_in_meta_are_false)
        return opt.idx_start;
    const std::vector<DataFile>& files = recorder.meta().files;
    if (!files[0].start_sec)
        return opt.idx_start;
    const timeval tm = {files[0].start_sec, files[0].start_usec};
    const uint64_t time = opt.range.left.time;
    uint real_idx = opt.idx_start;
    for (uint idx = real_idx + 1; idx != files.size(); ++idx)
    {
        const DataFile& data_file = files[idx];
        if (data_file.start_sec)
        {
            timeval tm2 = {data_file.start_sec, data_file.start_usec};
            uint64_t elapsed = difftimeval(tm2, tm) / 1000000;
            if (elapsed > time)
            {
                opt.range.left.time -= time;
                break;
            }
            real_idx = idx;
        }
    }
    return real_idx;
}

int wrm_recorder_init(WRMRecorder& recorder, WrmRecorderOption& opt, InputType::enum_t itype)
{
    recorder.set_basepath(opt.base_path);
    recorder.only_filename = opt.ignore_dir_for_meta_in_wrm;

    try
    {
        switch (itype) {
            case InputType::WRM_TYPE:
                recorder.open_wrm_only(opt.in_filename.c_str());
                if (!recorder.selected_next_order())
                {
                    std::cerr << opt.in_filename << " is invalid wrm file" << std::endl;
                    return 2001;
                }
                if (!recorder.is_meta_chunk())
                    return __wrm_recorder_init_meta_not_found(recorder, opt);
                if (!recorder.interpret_meta_chunk())
                {
                    std::cerr << "invalid meta chunck in " << opt.in_filename << std::endl;
                    return 2003;
                }
                if (opt.idx_start >= recorder.meta().files.size())
                    return __wrm_recorder_init_idx_not_found(recorder, opt);
                opt.idx_start = __wrm_recorder_init_get_good_idx(recorder, opt);
                if (opt.idx_start != recorder.idx_file)
                {
                    recorder.next_file(recorder.meta().files[recorder.idx_file].wrm_filename.c_str());
                    recorder.load_png_context(recorder.meta().files[recorder.idx_file].png_filename.c_str());
                }
            break;
            case InputType::META_TYPE:
                if (!recorder.open_meta(opt.in_filename.c_str()))
                {
                    std::cerr << "open " << opt.in_filename << ' ' << strerror(errno) << std::endl;
                    return 2005;
                }
                if (opt.idx_start >= recorder.meta().files.size())
                    return __wrm_recorder_init_idx_not_found(recorder, opt);
                opt.idx_start = __wrm_recorder_init_get_good_idx(recorder, opt);
                recorder.open_wrm_only(
                    recorder.get_cpath(
                        recorder.meta()
                        .files[opt.idx_start]
                        .wrm_filename.c_str()
                    )
                );
                if (!recorder.meta().files[opt.idx_start].png_filename.empty())
                    recorder.load_png_context(
                        recorder.meta()
                        .files[opt.idx_start]
                        .png_filename.c_str()
                    );
                if (recorder.selected_next_order() && recorder.is_meta_chunk())
                    recorder.ignore_chunks();
                if (!recorder.is_meta_chunk())
                    return __wrm_recorder_init_meta_not_found(recorder, opt);
                break;
            default:
                std::cerr << "Input type not found" << std::endl;
                return 2000;
        }
        recorder.idx_file = opt.idx_start + 1;
    }
    catch (Error& error)
    {
        std::cerr << "Error " << error.id << ": " << strerror(error.errnum) << std::endl;
        return 100000 + error.errnum;
    }

    return 0;
}

#endif