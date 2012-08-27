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

#if !defined(__MAIN_RECORDER_VALIDATE_HPP__)
#define __MAIN_RECORDER_VALIDATE_HPP__

#include <vector>
#include <string>
#include <boost/any.hpp>
#include "cipher.hpp"

template<std::size_t>
class HexadecimalOption;
class time_point;
class range_time_point;

void validate(boost::any& v,
              const std::vector<std::string>& values,
              range_time_point* range, int);

void validate(boost::any& v,
              const std::vector<std::string>& values,
              time_point* time, int);

void validate(boost::any& v,
              const std::vector<std::string>& values,
              const EVP_CIPHER** mode, int);

void validate(boost::any& v,
              const std::vector<std::string>& values,
              CipherMode::enum_t* mode, int);

template<std::size_t _N>
void validate(boost::any& v,
              const std::vector<std::string>& values,
              HexadecimalOption<_N>*, int);

#endif