/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "gdi/multiline_text_metrics.hpp"
#include "test_only/core/font.hpp"

#include <iomanip>

struct LineForTest : gdi::MultiLineTextMetrics::Line
{};

static std::ostream& boost_test_print_type(std::ostream& ostr, LineForTest const& line)
{
    return ostr << "{.cx=" << line.cx << ", .text=" << std::quoted(line.text) << "}";
}

namespace boost {
namespace test_tools {
namespace assertion {
namespace op {

template<>
struct EQ<::LineForTest, ::LineForTest>
{
    using result_type = assertion_result;
    using L = ::LineForTest;
    using R = L;
    using OP = EQ<L, R>;
    using elem_op = op::EQ<L, R>;

    static assertion_result
    eval(L const& lhs, R const& rhs)
    {
        assertion_result ar(true);

        bytes_view a{lhs.text, strlen(lhs.text)};
        bytes_view b{rhs.text, strlen(rhs.text)};

        size_t pos = std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first - a.begin();
        const bool r = pos != a.size() || a.size() != b.size();
        const bool rx = lhs.cx != rhs.cx;
        if (REDEMPTION_UNLIKELY(r) || REDEMPTION_UNLIKELY(rx))
        {
            ar = false;

            if (rx) {
                ar.message() << "[" << lhs.cx << " == " << rhs.cx << (r ? " && " : "]");
            }

            if (r) {
                ar.message()
                << (rx ? "" : "[")
                << redemption_unit_test__::Put2Mem{
                    pos, a, b, " == ", 0, ::ut::PatternView::ascii
                }
                << "] (text mismatch at position " << pos << ")";
            }
        }

        return ar;
    }

    template<class PrevExprType>
    static void
    report(std::ostream&, PrevExprType const&, R const&)
    {}

    static char const* revert()
    { return " != "; }
};

}
}
}
}

#define TEST_LINES(font, s, max_width, ...) [](                            \
    gdi::MultiLineTextMetrics const& metrics                               \
) {                                                                        \
    gdi::MultiLineTextMetrics::Line expected_[] {__VA_ARGS__};             \
    array_view lines_ = metrics.lines();                                   \
    array_view<LineForTest> expected{                                      \
        static_cast<LineForTest const*>(expected_), std::size(expected_)}; \
    array_view lines = {                                                   \
        static_cast<LineForTest const*>(lines_.data()), lines_.size()};    \
    RED_CHECK_EQUAL_RANGES(lines, expected);                               \
    if (lines.size() != expected.size()) {                                 \
        std::size_t min = std::min(lines.size(), expected.size());         \
        RED_CHECK_EQUAL_RANGES(lines.first(min), expected.first(min));     \
    }                                                                      \
}(gdi::MultiLineTextMetrics(font, s, max_width))


RED_AUTO_TEST_CASE(MultiLineTextMetricsCounter)
{
    auto& font14 = global_font_deja_vu_14();

    TEST_LINES(font14, "abc", 100,
        {"abc", 25},
    );

    TEST_LINES(font14, "abc", 17,
        {"ab", 17},
        {"c", 8},
    );

    TEST_LINES(font14, "abc", 1,
        {"a", 8},
        {"b", 9},
        {"c", 8},
    );

    TEST_LINES(font14, "a\nb\nc", 100,
        {"a", 8},
        {"b", 9},
        {"c", 8},
    );

    TEST_LINES(font14, "a\nb\nc", 17,
        {"a", 8},
        {"b", 9},
        {"c", 8},
    );

    TEST_LINES(font14, "a\nb\nc", 1,
        {"a", 8},
        {"b", 9},
        {"c", 8},
    );

    TEST_LINES(font14, "ab cd", 17,
        {"ab", 17},
        {"cd", 17},
    );

    TEST_LINES(font14, "ab cd", 1,
        {"a", 8},
        {"b", 9},
        {"", 0},
        {"c", 8},
        {"d", 9},
    );

    TEST_LINES(font14, "annvhg jgsy kfhdis hnvlkj gks hxk.hf", 50,
        {"annvh", 44},
        {"g jgsy", 39},
        {"kfhdis", 42},
        {"hnvlkj", 42},
        {"gks", 25},
        {"hxk.hf", 44},
    );

    TEST_LINES(font14, "annvhg jgsy kfhdis hnvlkj gks hxk.hf", 150,
        {"annvhg jgsy kfhdis", 125},
        {"hnvlkj gks hxk.hf", 111},
    );

    TEST_LINES(font14, "veryverylonglonglong string", 100,
        {"veryverylongl", 97},
        {"onglong string", 97},
    );

    TEST_LINES(font14, "  veryverylonglonglong string", 100,
        {"", 0},
        {"veryverylongl", 97},
        {"onglong string", 97},
    );

    TEST_LINES(font14, "  veryverylonglonglong\n string", 100,
        {"", 0},
        {"veryverylongl", 97},
        {"onglong", 57},
        {" string", 45},
    );

    TEST_LINES(font14, "  veryverylonglonglong \nstring", 100,
        {"", 0},
        {"veryverylongl", 97},
        {"onglong", 57},
        {"string", 40},
    );

    TEST_LINES(font14, "bla bla\n\n - abc\n - def", 100,
        {"bla bla", 40},
        {"", 0},
        {" - abc", 35},
        {" - def", 33},
    );
}
