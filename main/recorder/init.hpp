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

#if !defined(__MAIN_RECORDER_INIT__)
#define __MAIN_RECORDER_INIT__

#include <iostream>

#include "wrm_recorder_option.hpp"
#include "output_type.hpp"
#include "input_type.hpp"

/**
 * \brief Init opt, otype and itype and display error.
 * Return 0 if success
 * @{
 */
int parse_command_line(WrmRecoderOption& opt, int argc, char** argv);

int set_iotype(WrmRecoderOption& opt,
               InputType::enum_t& itype, OutputType::enum_t& otype);

int init_opt_and_iotype(WrmRecoderOption& opt, int argc, char** argv,
                        InputType::enum_t& itype, OutputType::enum_t& otype);
//@}

#endif