#define REDEMPTION_UNIT_TEST_CPP 1
#include "system/redemption_unit_tests.hpp"

#include <algorithm>


namespace redemption_unit_test__
{
    bool xarray::operator == (xarray const & other) const
    {
        return sig.size() == other.sig.size()
            && std::equal(sig.begin(), sig.end(), other.sig.begin());
    }

    bool xarray_color::operator == (xarray_color const & other) const
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


    bool xsarray::operator == (xsarray const & other) const
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
} // namespace redemption_unit_test__
