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

#include <iostream>

#include <boost/program_options/parsers.hpp>

#include "cipher.hpp"
#include "relative_time_point.hpp"
#include "hexadecimal_option.hpp"

namespace po = boost::program_options;

template<typename _T>
void validate_time_or_throw_invalid_option_value(boost::any& v,
                                                 const std::vector<std::string>& values,
                                                 _T*)
{
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = po::validators::get_single_string(values);
    try
    {
        v = boost::any(_T(s));
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        throw po::validation_error(po::validation_error::invalid_option_value);
    }
}

template<typename _T>
void validate(boost::any& v,
              const std::vector<std::string>& values,
              basic_relative_point<_T>* target_type, int)
{
    validate_time_or_throw_invalid_option_value(v, values, target_type);
}

inline void validate(boost::any& v,
                     const std::vector<std::string>& values,
                     range_time_point* target_type, int)
{
    validate_time_or_throw_invalid_option_value(v, values, target_type);
}

inline void validate(boost::any& v,
                     const std::vector<std::string>& values,
                     time_point* target_type, int)
{
    validate_time_or_throw_invalid_option_value(v, values, target_type);
}

template<std::size_t _N>
void validate(boost::any& v,
              const std::vector<std::string>& values,
              HexadecimalOption<_N>*, int)
{
    // Make sure no previous assignment to 'a' was made.
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = po::validators::get_single_string(values);
    HexadecimalOption<_N> bs;
    if (!bs.parse(s))
        throw po::validation_error(po::validation_error::invalid_option_value);
    v = boost::any(bs);
}


inline CipherMode::enum_t get_enum_mode(boost::any& v,
                                        const std::vector<std::string>& values)
{
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = po::validators::get_single_string(values);

    CipherMode::enum_t mode = CipherMode::NO_MODE;

    if (s == "bf")
        mode = CipherMode::BLOWFISH_CBC;
    else if (s == "bf-cbc")
        mode = CipherMode::BLOWFISH_CBC;
    else if (s == "bf-ecb")
        mode = CipherMode::BLOWFISH_ECB;
    else if (s == "bf-ofb")
        mode = CipherMode::BLOWFISH_OFB;

    else if (s == "cast5")
        mode = CipherMode::CAST5_CBC;
    else if (s == "cast5-cbc")
        mode = CipherMode::CAST5_CBC;
    else if (s == "cast5-cfb")
        mode = CipherMode::CAST5_CFB;
    else if (s == "cast5-ofb")
        mode = CipherMode::CAST5_OFB;
    else if (s == "cast5-ecb")
        mode = CipherMode::CAST5_ECB;

    else if (s == "des")
        mode = CipherMode::DES_CBC;
    else if (s == "des-cbc")
        mode = CipherMode::DES_CBC;
    else if (s == "des-cfb")
        mode = CipherMode::DES_CFB;
    else if (s == "des-ofb")
        mode = CipherMode::DES_OFB;
    else if (s == "des-ecb")
        mode = CipherMode::DES_ECB;

    else if (s == "des3")
        mode = CipherMode::DES_EDE3_CBC;
    else if (s == "des3-ecb")
        mode = CipherMode::DES_EDE3_ECB;
    else if (s == "des-ede3-cbc")
        mode = CipherMode::DES_EDE3_CBC;
    else if (s == "des-ede3-ecb")
        mode = CipherMode::DES_EDE3_ECB;
    else if (s == "des-ede3-cfb-1")
        mode = CipherMode::DES_EDE3_CFB_1;
    else if (s == "des-ede3-cfb-8")
        mode = CipherMode::DES_EDE3_CFB_8;
    else if (s == "des-ede3-cfb-64")
        mode = CipherMode::DES_EDE3_CFB_64;
    else if (s == "des-ede3-ofb")
        mode = CipherMode::DES_EDE3_OFB;

    else if (s == "rc2")
        mode = CipherMode::RC2_CBC;
    else if (s == "rc2-cbc")
        mode = CipherMode::RC2_CBC;
    else if (s == "rc2-cfb")
        mode = CipherMode::RC2_CFB;
    else if (s == "rc2-ecb")
        mode = CipherMode::RC2_ECB;
    else if (s == "rc2-ofb")
        mode = CipherMode::RC2_OFB;
    else if (s == "rc2-64-cbc")
        mode = CipherMode::RC2_64_CBC;
    else if (s == "rc2-40-cbc")
        mode = CipherMode::RC2_40_CBC;

    else if (s == "rc4")
        mode = CipherMode::RC4;
    else if (s == "rc4-40")
        mode = CipherMode::RC4_40;

    else if (s == "aes-128")
        mode = CipherMode::AES_128_CBC;
    else if (s == "aes-128-cbc")
        mode = CipherMode::AES_128_CBC;
    else if (s == "aes-128-cfb")
        mode = CipherMode::AES_128_CFB;
    else if (s == "aes-128-cfb1")
        mode = CipherMode::AES_128_CFB1;
    else if (s == "aes-128-cfb8")
        mode = CipherMode::AES_128_CFB8;
    else if (s == "aes-128-ecb")
        mode = CipherMode::AES_128_ECB;
    else if (s == "aes-128-ofb")
        mode = CipherMode::AES_128_OFB;

    else if (s == "aes-192")
        mode = CipherMode::AES_192_CBC;
    else if (s == "aes-192-cbc")
        mode = CipherMode::AES_192_CBC;
    else if (s == "aes-192-cfb")
        mode = CipherMode::AES_192_CFB;
    else if (s == "aes-192-cfb1")
        mode = CipherMode::AES_192_CFB1;
    else if (s == "aes-192-cfb8")
        mode = CipherMode::AES_192_CFB8;
    else if (s == "aes-192-ecb")
        mode = CipherMode::AES_192_ECB;
    else if (s == "aes-192-ofb")
        mode = CipherMode::AES_192_OFB;

    else if (s == "aes-256")
        mode = CipherMode::AES_256_CBC;
    else if (s == "aes-256-cbc")
        mode = CipherMode::AES_256_CBC;
    else if (s == "aes-256-cfb")
        mode = CipherMode::AES_256_CFB;
    else if (s == "aes-256-cfb1")
        mode = CipherMode::AES_256_CFB1;
    else if (s == "aes-256-cfb8")
        mode = CipherMode::AES_256_CFB8;
    else if (s == "aes-256-ecb")
        mode = CipherMode::AES_256_ECB;
    else if (s == "aes-256-ofb")
        mode = CipherMode::AES_256_OFB;

    if (!mode)
        throw po::validation_error(po::validation_error::invalid_option_value);

    return mode;
}

inline void validate(boost::any& v,
              const std::vector<std::string>& values,
              CipherMode::enum_t* /*mode*/, int)
{
    v = boost::any(get_enum_mode(v, values));
}

inline void validate(boost::any& v,
              const std::vector<std::string>& values,
              const EVP_CIPHER** /*mode*/, int)
{
    v = boost::any(CipherMode::to_evp_cipher(get_enum_mode(v, values)));
}

#endif