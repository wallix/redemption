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

#if !defined(__MAIN_RECORDER_HEXADECIMAL_OPTION_HPP__)
#define __MAIN_RECORDER_HEXADECIMAL_OPTION_HPP__

#include <string>

#include <openssl/evp.h>

template<std::size_t N>
struct HexadecimalOption
{
    unsigned char data[N];
    std::size_t size;

    HexadecimalOption()
    : size(0)
    {}

    /**
     * \param s value in hexadecimal base
     */
    bool parse(const std::string& s)
    {
        std::size_t n = s.size() / 2 + (s.size() & 1);
        if (n > N || !transform_string_hex_to_data(s, this->data))
            return false;
        this->size = n;
        while (n != N)
            this->data[n++] = 0;
        return true;
    }

private:
    static bool transform_string_hex_to_data(const std::string& s,
                                             unsigned char * pdata)
    {
        std::string::const_iterator first = s.begin();
        std::string::const_iterator last = s.end();
        char c;
        if (s.size() & 1)
            --last;
        for (; first != last; ++first, ++pdata)
        {
            if (0xf == (*pdata = transform_c_hex_to_c_data(*first)))
                return false;
            if (0xf == (c = transform_c_hex_to_c_data(*++first)))
                return false;
            *pdata = (*pdata << 4) + c;
        }
        if (s.size() & 1)
        {
            if (0xf == (*pdata = transform_c_hex_to_c_data(*first)))
                return false;
            *pdata <<= 4;
        }
        return true;
    }

    static unsigned char transform_c_hex_to_c_data(char c)
    {
        if ('a' <= c && c <= 'f')
            return c - 'a' + 0xa;
        if ('A' <= c && c <= 'F')
            return c - 'A' + 0xa;
        if ('0' > c || c > '9')
            return 0xf;
        return c - '0';
    }
};

typedef HexadecimalOption<EVP_MAX_KEY_LENGTH> HexadecimalKeyOption;
typedef HexadecimalOption<EVP_MAX_IV_LENGTH> HexadecimalIVOption;

#endif