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

#include "configs/autogen/enums.hpp"
#include "gdi/screen_info.hpp"

#include <chrono>

#ifdef basename
# undef basename
#endif

class CryptoContext;
class Random;
class Fstat;

struct WrmParams
{
    BitsPerPixel capture_bpp;
    bool remote_app;
    CryptoContext & cctx;
    Random & rnd;
    Fstat & fstat;
    const char * hash_path;
    std::chrono::duration<unsigned int, std::ratio<1, 100>> frame_interval;
    std::chrono::seconds break_interval;
    WrmCompressionAlgorithm wrm_compression_algorithm;
    uint32_t wrm_verbose;

    WrmParams(BitsPerPixel capture_bpp,
              bool remote_app,
              CryptoContext & cctx,
              Random & rnd,
              Fstat & fstat,
              const char * hash_path,
              std::chrono::duration<unsigned int, std::ratio<1, 100>> frame_interval,
              std::chrono::seconds break_interval,
              WrmCompressionAlgorithm wrm_compression_algorithm,
              // TODO Verbose::WrmCapture
              uint32_t wrm_verbose)
    : capture_bpp(capture_bpp)
    , remote_app(remote_app)
    , cctx(cctx)
    , rnd(rnd)
    , fstat(fstat)
    , hash_path(hash_path)
    , frame_interval(frame_interval)
    , break_interval(break_interval)
    , wrm_compression_algorithm(wrm_compression_algorithm)
    , wrm_verbose(wrm_verbose)
    {
    }
};
