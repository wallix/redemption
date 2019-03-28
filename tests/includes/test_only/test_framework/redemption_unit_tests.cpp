/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#ifdef IN_IDE_PARSER
# define REDEMPTION_UNIT_TEST_CPP
#endif

#include "./redemption_unit_tests.hpp"

#include <algorithm>
#include <ostream>


namespace redemption_unit_test__
{
    bool xarray::operator == (xarray const & other) const noexcept
    {
        return sig.size() == other.sig.size()
            && std::equal(sig.begin(), sig.end(), other.sig.begin());
    }

    bool xarray_color::operator == (xarray_color const & other) const noexcept
    {
        this->res = std::mismatch(sig.begin(), sig.end(), other.sig.begin(), other.sig.end()).first - sig.begin();
        return this->res == sig.size() && this->sig.size() == other.sig.size();
    }

    std::ostream & operator<<(std::ostream & out, xarray_color const & x)
    {
        if (x.size() == 0){
            return out << "\"\"\n";
        }
        char const * hex_table = "0123456789abcdef";
        size_t q = 0;
        size_t split = 16;
        uint8_t tmpbuf[16];
        size_t i = 0;
        for (unsigned c : x.sig) {
            if (q%split == 0){
                if (x.sig.size()>split){
                    out << "\n\"";
                }
                else {
                    out << "\"";
                }
            }
            if (q++ == x.res){ out << "\x1b[31m";}
            out << "\\x" << hex_table[c >> 4] << hex_table[c & 0xf];
            tmpbuf[i++] = c;
            if (q%split == 0){
                if (x.sig.size()>split) {
                    out << "\" //";
                    for (size_t v = 0 ; v < i ; v++){
                        if ((tmpbuf[v] >= 0x20) && (tmpbuf[v] < 127)) {
                            out << char(tmpbuf[v]);
                        }
                        else {
                            out << ".";
                        }
                    }
                    out << " !";
                    i = 0;
                }
                else {
                    out << "\"";
                }
            }
        }
        if (q%split != 0){
            if (x.sig.size()>split) {
                out << "\" //";
                for (size_t v = 0 ; v < i ; v++){
                    if ((tmpbuf[v] >= 0x20) && (tmpbuf[v] <= 127)) {
                        out << char(tmpbuf[v]);
                    }
                    else {
                        out << ".";
                    }
                }
                out << " !";
            }
            else {
                out << "\"";
            }
        }
        return out << "\x1b[0m";
    }

    std::ostream & operator<<(std::ostream & out, xarray const & x)
    {
        out << "\"";
        char const * hex_table = "0123456789abcdef";
        for (unsigned c : x.sig) {
            out << "\\x" << hex_table[c >> 4] << hex_table[c & 0xf];
        }
        return out << "\"";
    }


    bool xsarray::operator == (xsarray const & other) const noexcept
    {
        return sig.size() == other.sig.size()
            && std::equal(sig.begin(), sig.end(), other.sig.begin());
    }

    std::ostream & operator<<(std::ostream & out, xsarray const & x)
    {
        out << "\"";
        char const * hex_table = "0123456789abcdef";
        for (unsigned c : x.sig) {
            if ((c >= 0x20) && (c <= 127)) {
                out << char(c);
            }
            else {
                out << "\\x" << hex_table[c >> 4] << hex_table[c & 0xf];
            }
        }
        return out << "\"";
    }

    namespace
    {
#ifdef __clang__
        constexpr std::size_t start_type_name = 43;
        constexpr std::size_t end_type_name = 1;
#elif defined(__GNUC__)
        constexpr std::size_t start_type_name = 48;
        constexpr std::size_t end_type_name = 34;
#endif
    }

    array_view_const_char Enum::get_type_name(char const* s, std::size_t n) noexcept
    {
        return {s + start_type_name, n - start_type_name - end_type_name};
    }
} // namespace redemption_unit_test__


void RED_TEST_PRINT_TYPE_STRUCT_NAME<redemption_unit_test__::int_variation>::operator()(
    std::ostream& out, redemption_unit_test__::int_variation const & x) const
{
    if (x.left == x.right) {
        out << x.left;
    }
    else {
        out << x.value << "+-" << x.variant << (x.is_percent ? "%" : "")
            << " [" << x.left << ", " << x.right << "]";
    }
}


std::ostream& operator<<(std::ostream& out, redemption_unit_test__::Enum const& e)
{
    out.write(e.name.data(), e.name.size()) << "{";
    if (e.is_signed) {
        out << e.x;
    }
    else {
        out << static_cast<unsigned long long>(e.x);
    }
    return out << "}";
}
