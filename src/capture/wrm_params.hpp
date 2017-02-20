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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Christophe Grosjean
*/


#pragma once
#include "capture/cryptofile.hpp"
#include "configs/autogen/enums.hpp"

struct WrmParams
{
    uint8_t capture_bpp;
    TraceType trace_type;
    CryptoContext & cctx; 
    Random & rnd;
    const char * record_path;
    const char * hash_path;
    const char * basename;
    int groupid;
//    auth_api * authentifier;
//    RDPDrawable & drawable;
    std::chrono::duration<unsigned int, std::ratio<1, 100>> frame_interval;
    std::chrono::seconds break_interval;
    WrmCompressionAlgorithm wrm_compression_algorithm;
    int wrm_verbose;

    WrmParams(uint8_t capture_bpp,
              TraceType trace_type,
              CryptoContext & cctx,
              Random & rnd,
              const char * record_path,
              const char * hash_path,
              const char * basename,
              int groupid,
//              auth_api * authentifier,
//              RDPDrawable & drawable,
              std::chrono::duration<unsigned int, std::ratio<1, 100>> frame_interval,
              std::chrono::seconds break_interval,
              WrmCompressionAlgorithm wrm_compression_algorithm,
              int wrm_verbose)
    : capture_bpp(capture_bpp)
    , trace_type(trace_type)
    , cctx(cctx)
    , rnd(rnd)
    , record_path(record_path)
    , hash_path(hash_path)
    , basename(basename)
    , groupid(groupid)
//    , authentifier(authentifier)
//    , drawable(drawable)
    , frame_interval(frame_interval)
    , break_interval(break_interval)
    , wrm_compression_algorithm(wrm_compression_algorithm)
    , wrm_verbose(wrm_verbose)
    {}
};

struct WrmInputParams
{
};
