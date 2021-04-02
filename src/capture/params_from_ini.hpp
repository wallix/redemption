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

#include "capture/kbd_log_params.hpp"
#include "capture/meta_params.hpp"
#include "capture/ocr_params.hpp"
#include "capture/pattern_params.hpp"
#include "capture/wrm_params.hpp"
#include "gdi/screen_info.hpp"


class CryptoContext;
class Inifile;
class Random;

OcrParams ocr_params_from_ini(const Inifile & ini);

MetaParams meta_params_from_ini(const Inifile & ini);

KbdLogParams kbd_log_params_capture_from_ini(const Inifile & ini);
KbdLogParams kbd_log_params_video_from_ini(const Inifile & ini);

PatternParams pattern_params_from_ini(const Inifile & ini);

WrmParams wrm_params_from_ini(
    BitsPerPixel capture_bpp, bool remote_app, CryptoContext & cctx, Random & rnd,
    const char * hash_path, const Inifile & ini);
