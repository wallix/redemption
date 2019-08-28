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

#include <boost/test/results_collector.hpp>
// #include <boost/test/results_reporter.hpp>
#include <boost/test/framework.hpp>

#include <algorithm>
#include <ostream>


namespace redemption_unit_test__
{
    unsigned long current_count_error()
    {
        using boost::unit_test::results_collector;
        using boost::unit_test::framework::current_test_case;
        using boost::unit_test::test_case;
        using boost::unit_test::test_results;
        return results_collector.results(current_test_case().p_id).p_assertions_failed.get();
    }

    // std::ostream& current_stream()
    // {
    //     return boost::unit_test::results_reporter::get_stream();
    // }

    bool compare_bytes(size_t& pos, bytes_view b, bytes_view a) noexcept
    {
        pos = std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first - a.begin();
        return pos == a.size() && a.size() == b.size();
    }

    namespace
    {
        constexpr uint8_t utf8_byte_size_table[] {
            // 0xxx x[xxx]
            1, 1, 1, 1,
            1, 1, 1, 1,
            1, 1, 1, 1,
            1, 1, 1, 1,
            // 10xx x[xxx]  invalid value
            0, 0, 0, 0,
            0, 0, 0, 0,
            // 110x x[xxx]
            2, 2, 2, 2,
            // 1110 x[xxx]
            3, 3,
            // 1111 0[xxx]
            4,
            // 1111 1[xxx]  invalid value
            0,
        };

        using N0 = std::integral_constant<std::size_t, 0>;
        using N1 = std::integral_constant<std::size_t, 1>;
        using N2 = std::integral_constant<std::size_t, 2>;
        using N3 = std::integral_constant<std::size_t, 3>;
        using N4 = std::integral_constant<std::size_t, 4>;

        template<class Size, class F>
        void utf8_char_process(byte_ptr v, Size size, F&& f)
        {
            switch (utf8_byte_size_table[v[0] >> 3]) {
                case 0: return f(N0{});
                case 1: return f(N1{});
                case 2: return size >= 2 && (v[1] >> 6) == 0b10 ? f(N2{}) : f(N1{});
                case 3:
                    if (size < 3 || (v[2] >> 6) != 0b10) {
                        return f(N0{});
                    }
                    switch (v[0] & 0b1111) {
                        case 0: return (v[1] >> 5) == 0b101 ? f(N3{}) : f(N0{});
                        case 0b1101: return (v[1] >> 5) == 0b100 ? f(N3{}) : f(N0{});
                        default: return (v[1] >> 6) == 0b10 ? f(N3{}) : f(N0{});
                    }
                case 4:
                    if (size < 4 || (v[2] >> 6) != 0b10 || (v[3] >> 6) != 0b10) {
                        return f(N0{});
                    }
                    switch (v[0] & 0b111) {
                        case 0: return (v[1] >> 4) != 0b1000 ? f(N4{}) : f(N0{});
                        case 0b001:
                        case 0b010:
                        case 0b011: return (v[1] >> 6) != 0b10 ? f(N4{}) : f(N0{});
                        case 0b100: return (v[1] >> 4) != 0b1000 ? f(N4{}) : f(N0{});
                    }
            }
        }
    }

    static void put_char(std::ostream& out, unsigned c, char const* newline = "\\n")
    {
        if (c >= 0x23 && c <= 127) {
            out << char(c);
        }
        else {
            char const * hex_table = "0123456789abcdef";
            switch (c) {
                case ' ':
                case '!': out << char(c); break;
                case '"': out << "\\\""; break;
                case '\b': out << "\\b"; break;
                case '\t': out << "\\t"; break;
                case '\r': out << "\\r"; break;
                case '\n': out << newline; break;
                default: out << "\\x" << hex_table[c >> 4] << hex_table[c & 0xf];
            }
        }
    }

    static std::ostream& put_dump_bytes(size_t pos, std::ostream& out, bytes_view x)
    {
        if (x.size() == 0){
            return out << "\"\"\n";
        }
        char const * hex_table = "0123456789abcdef";
        size_t q = 0;
        size_t split = 16;
        uint8_t tmpbuf[16];
        size_t i = 0;
        for (unsigned c : x) {
            if (q%split == 0){
                if (x.size()>split){
                    out << "\n\"";
                }
                else {
                    out << "\"";
                }
            }
            if (q++ == pos){ out << "\x1b[31m";}
            out << "\\x" << hex_table[c >> 4] << hex_table[c & 0xf];
            tmpbuf[i++] = c;
            if (q%split == 0){
                if (x.size()>split) {
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
            if (x.size()>split) {
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

    static void put_utf8_bytes(size_t pos, std::ostream& out, bytes_view v, char const* newline = "\\n")
    {
        auto print = [&](bytes_view x, bool is_markable){
            auto* p = x.as_u8p();
            auto* end = p + x.size();

            auto algo0 = [&](auto f0){
                return [&](std::size_t n){
                    if (n == 0) {
                        f0();
                        ++p;
                        out << char(*p);
                    }
                    else if (n == 1) {
                        put_char(out, *p, newline);
                        ++p;
                    }
                    else {
                        out.write(char_ptr_cast(p), n);
                        p += n;
                    }
                };
            };

            while (p + 4 < end) {
                utf8_char_process(p, end-p, algo0([]{}));
            }

            bool is_marked = false;

            while (p != end) {
                utf8_char_process(p, end-p, algo0([&]{
                    if (is_markable && utf8_byte_size_table[*p >> 3] != 0) {
                        out << "\x1b[31m";
                        is_marked = true;
                    }
                }));
            }

            if (is_markable && !is_marked) {
                out << "\x1b[31m";
            }
        };

        print(v.first(pos), pos != v.size());
        if (pos != v.size()) {
            out << "\x1b[31m";
            print(v.from_offset(pos), false);
            out << "\x1b[0m";
        }
    }

    static void put_utf8_bytes2(size_t pos, std::ostream& out, bytes_view v)
    {
        put_utf8_bytes(pos, out, v, "\n");
    }

    static void put_ascii_bytes(size_t pos, std::ostream& out, bytes_view v, char const* newline = "\\n")
    {
        auto print = [&](bytes_view x){
            for (uint8_t c : x) {
                if (c >= 0x23 && c <= 127) {
                    out << char(c);
                }
                else {
                    put_char(out, c, newline);
                }
            }
        };

        print(v.first(pos));
        if (pos != v.size()) {
            out << "\x1b[31m";
            print(v.from_offset(pos));
            out << "\x1b[0m";
        }
    }

    static void put_ascii_bytes2(size_t pos, std::ostream& out, bytes_view v)
    {
        put_ascii_bytes(pos, out, v, "\n");
    }

    static void put_hex_bytes(size_t pos, std::ostream& out, bytes_view v)
    {
        char const * hex_table = "0123456789abcdef";
        auto print = [&](bytes_view x){
            for (uint8_t c : x) {
                out << "\\x" << hex_table[c >> 4] << hex_table[c & 0xf];
            }
        };

        print(v.first(pos));
        if (pos != v.size()) {
            out << "\x1b[31m";
            print(v.from_offset(pos));
            out << "\x1b[0m";
        }
    }

    static void put_auto_bytes(size_t pos, std::ostream& out, bytes_view v)
    {
        auto n = std::min(int(v.size()), 36);
        auto* p = v.as_u8p();
        auto* end = p + n;
        int count_invalid = 0;
        while (p < end) {
            utf8_char_process(p, end-p, [&](std::size_t n){
                if (n == 0) {
                    ++count_invalid;
                    ++p;
                }
                else {
                    p += n;
                }
            });
        }

        if (count_invalid > n / 6) {
            put_ascii_bytes(pos, out, v);
        }
        else {
            put_utf8_bytes(pos, out, v);
        }
    }

    std::ostream & operator<<(std::ostream & out, Put2Mem const & x)
    {
        out << "\"";
        switch (x.pattern) {
            #define CASE(c, print) case c: \
                print(x.pos, out, x.lhs);  \
                out << "\" != \"";         \
                print(x.pos, out, x.rhs);  \
                break
            CASE('c', put_ascii_bytes);
            CASE('C', put_ascii_bytes2);
            CASE('s', put_utf8_bytes);
            CASE('S', put_utf8_bytes2);
            CASE('h', put_hex_bytes);
            CASE('d', put_dump_bytes);
            default:
            CASE('a', put_auto_bytes);
            #undef CASE
        }
        return out << "\"]";
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
