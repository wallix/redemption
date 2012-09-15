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
                                                     const char * wrm_filename)
{
    std::cerr << recorder.meta() << '\n'
    << "Chunk META not found in " << wrm_filename
    << "\n. Chunk is " << recorder.chunk_type() << std::endl;
    return 2004;
}

inline static int __wrm_recorder_init_idx_not_found(WRMRecorder& recorder,
                                                    WrmRecorderOption& opt)
{
    std::cerr << "idx " << opt.idx_start << " not found" << std::endl;
    return 2002;
}

inline static void __wrm_recorder_init_set_good_idx(WRMRecorder& recorder,
                                                    WrmRecorderOption& opt)
{
    if (opt.times_in_meta_are_false)
        return ;
    const std::vector<DataFile>& files = recorder.meta().files;
    if (!files[0].start_sec)
        return ;
    const timeval tm = {files[0].start_sec, files[0].start_usec};
    uint64_t time = 0;
    for (uint idx = opt.idx_start + 1; idx != files.size(); ++idx)
    {
        const DataFile& data_file = files[idx];
        if (data_file.start_sec)
        {
            timeval tm2 = {data_file.start_sec, data_file.start_usec};
            uint64_t elapsed = difftimeval(tm2, tm) / 1000000;
            if (elapsed > opt.range.left.time)
            {
                opt.range.left.time -= time;
                break;
            }
            time = elapsed;
            opt.idx_start = idx;
        }
    }
}

inline static bool __wrm_recorder_init_init_crypt(WRMRecorder& recorder,
                                                  WrmRecorderOption& opt)
{
    if (opt.in_crypt_key.size)
    {
        if (!opt.in_crypt_iv.size)
        {
            opt.in_crypt_iv.size = sizeof(opt.in_crypt_iv.data);
            memcpy(opt.in_crypt_iv.data,
                   recorder.meta().crypt_iv,
                   sizeof(opt.in_crypt_iv.data));
        }
        if (!recorder.init_cipher(
            (opt.in_crypt_mode
            ? opt.in_crypt_mode
            : CipherMode::to_evp_cipher(
                (CipherMode::enum_t)recorder.meta().crypt_mode
            )),
            opt.in_crypt_key.data,
            opt.in_crypt_iv.size ? opt.in_crypt_iv.data : 0))
        {
            std::cerr << "Error in the initialization of the encryption" << std::endl;
            return false;
        }
    }
    return true;
}


int wrm_recorder_init(WRMRecorder& recorder, WrmRecorderOption& opt, InputType::enum_t itype)
{
    recorder.set_basepath(opt.base_path);
    recorder.only_filename = opt.ignore_dir_for_meta_in_wrm;

    try
    {
        const char * wrm_filename;
        switch (itype) {
            case InputType::WRM_TYPE:
                if (!__wrm_recorder_init_init_crypt(recorder, opt))
                    return 3000;
                wrm_filename = opt.in_filename.c_str();
                recorder.open_wrm_only(wrm_filename);
                if (!recorder.selected_next_order())
                {
                    std::cerr << opt.in_filename << " is invalid wrm file" << std::endl;
                    return 2001;
                }
                if (!recorder.is_meta_chunk())
                    return __wrm_recorder_init_meta_not_found(recorder, wrm_filename);
                if (!recorder.interpret_meta_chunk())
                {
                    std::cerr << "invalid meta chunck in " << opt.in_filename << std::endl;
                    return 2003;
                }
                if (!recorder.meta().files.empty())
                {
                    if (opt.idx_start >= recorder.meta().files.size())
                        return __wrm_recorder_init_idx_not_found(recorder, opt);
                    __wrm_recorder_init_set_good_idx(recorder, opt);
                }
                else  if (opt.idx_start >= recorder.meta().files.size())
                    return __wrm_recorder_init_idx_not_found(recorder, opt);
                if (opt.idx_start != recorder.idx_file)
                {
                    recorder.next_file(recorder.meta().files[recorder.idx_file].wrm_filename.c_str());
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
                __wrm_recorder_init_set_good_idx(recorder, opt);
                wrm_filename = recorder.get_cpath(
                    recorder.meta()
                    .files[opt.idx_start]
                    .wrm_filename.c_str()
                );
                if (!__wrm_recorder_init_init_crypt(recorder, opt))
                    return 3000;                recorder.open_wrm_only(wrm_filename);
                if (recorder.selected_next_order() && recorder.is_meta_chunk())
                    recorder.ignore_chunks();
                if (!recorder.is_meta_chunk())
                    return __wrm_recorder_init_meta_not_found(recorder, wrm_filename);
                break;
            default:
                std::cerr << "Input type not found" << std::endl;
                return 2000;
        }
        recorder.idx_file = opt.idx_start + 1;
        recorder.force_interpret_breakpoint = opt.force_interpret_breakpoint;
    }
    catch (Error& error)
    {
        std::cerr << "Error " << error.id << ": " << strerror(error.errnum) << std::endl;
        return 100000 + error.errnum;
    }

    return 0;
}

#endif