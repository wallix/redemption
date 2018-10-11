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

#include "capture/video_params.hpp"
#include "capture/kbd_log_params.hpp"
#include "capture/meta_params.hpp"
#include "capture/ocr_params.hpp"
#include "capture/pattern_params.hpp"
#include "capture/wrm_params.hpp"
#include "gdi/screen_info.hpp"


class CryptoContext;
class Fstat;
class Inifile;
class Random;

VideoParams video_params_from_ini(
    uint16_t original_width, uint16_t original_height,
    std::chrono::seconds video_break_interval,
    const Inifile & ini);

OcrParams ocr_params_from_ini(Inifile & ini);

MetaParams meta_params_from_ini(Inifile & ini);

KbdLogParams kbd_log_params_from_ini(Inifile & ini);

PatternParams pattern_params_from_ini(Inifile & ini);

WrmParams wrm_params_from_ini(
    BitsPerPixel capture_bpp, bool remote_app, CryptoContext & cctx, Random & rnd, Fstat & fstat,
    const char * hash_path, Inifile & ini);
