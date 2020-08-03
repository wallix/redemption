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
   Copyright (C) Wallix 2020
   Author(s): Christophe Grosjean, Jonathan Poelen

   Unit test to conversion of RDP drawing orders to PNG images
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "gdi/graphic_api.hpp"
#include "test_only/core/font.hpp"

#include <iomanip>


RED_AUTO_TEST_CASE(TextMetrics)
{
    {
        gdi::TextMetrics text(global_font_lato_light_16(), "abc");
        RED_CHECK_EQUAL(21, text.height);
        RED_CHECK_EQUAL(27, text.width);
    }
    {
        gdi::TextMetrics text(global_font_lato_light_16(), "abcde");
        RED_CHECK_EQUAL(21, text.height);
        RED_CHECK_EQUAL(46, text.width);
    }
    {
        gdi::TextMetrics text(global_font_lato_light_16(), "Ay");
        RED_CHECK_EQUAL(21, text.height);
        RED_CHECK_EQUAL(20, text.width);
    }
}


struct LineForTest : gdi::MultiLineTextMetrics::Line
{};

static std::ostream& boost_test_print_type(std::ostream& ostr, LineForTest const& line)
{
    return ostr << "{.width=" << line.width << ", .text=" << std::quoted(line.text) << "}";
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
        const bool rx = lhs.width != rhs.width;
        if (REDEMPTION_UNLIKELY(r) || REDEMPTION_UNLIKELY(rx))
        {
            ar = false;

            if (rx) {
                ar.message() << "[" << lhs.width << " == " << rhs.width << (r ? " && " : "]");
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

#define TEST_LINES(font, s, max_width, ...) [&](                                            \
    gdi::MultiLineTextMetrics const& metrics                                                \
) {                                                                                         \
    gdi::MultiLineTextMetrics::Line expected_[] {__VA_ARGS__};                              \
    array_view lines_ = metrics.lines();                                                    \
    array_view<LineForTest> expected{                                                       \
        static_cast<LineForTest const*>(expected_), std::size(expected_)};                  \
    array_view lines = {                                                                    \
        static_cast<LineForTest const*>(lines_.data()), lines_.size()};                     \
    RED_CHECK_EQUAL_RANGES(lines, expected);                                                \
    if (lines.size() != expected.size()) {                                                  \
        std::size_t min = std::min(lines.size(), expected.size());                          \
        std::size_t max = std::max(lines.size(), expected.size());                          \
        RED_CHECK_EQUAL_RANGES(lines.first(min), expected.first(min));                      \
        for (std::size_t i = min; i < max; ++i) {                                           \
            auto line = i < lines.size() ? lines[i] : LineForTest{{"", -1}};                \
            auto expected_line = i < expected.size() ? expected[i] : LineForTest{{"", -1}}; \
            RED_TEST(line == expected_line);                                                \
        }                                                                                   \
    }                                                                                       \
    for (auto&& line : lines) {                                                             \
        RED_TEST_CONTEXT(std::quoted(line.text)) {                                          \
            RED_TEST(gdi::TextMetrics(font, line.text).width == line.width);                \
        }                                                                                   \
    }                                                                                       \
}(gdi::MultiLineTextMetrics(font, s, max_width))


RED_AUTO_TEST_CASE(MultiLineTextMetrics)
{
    auto& font14 = global_font_deja_vu_14();
    auto& font16 = global_font_lato_light_16();

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
        {"g jgsy", 44},
        {"kfhdis", 42},
        {"hnvlkj", 42},
        {"gks", 25},
        {"hxk.hf", 44},
    );

    TEST_LINES(font14, "annvhg jgsy kfhdis hnvlkj gks hxk.hf", 150,
        {"annvhg jgsy kfhdis", 135},
        {"hnvlkj gks hxk.hf", 121},
    );

    TEST_LINES(font14, "veryverylonglonglong string", 100,
        {"veryverylongl", 97},
        {"onglong", 57},
        {"string", 40},
    );

    TEST_LINES(font14, "  veryverylonglonglong string", 100,
        {"", 0},
        {"veryverylongl", 97},
        {"onglong", 57},
        {"string", 40},
    );

    TEST_LINES(font14, "  veryverylonglonglong string", 130,
        {"", 0},
        {"veryverylonglongl", 127},
        {"ong string", 72},
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
        {"bla bla", 45},
        {"", 0},
        {" - abc", 40},
        {" - def", 38},
    );

    TEST_LINES(font16,
        "abc efg fajshfkg kf gfjg hjgsj dj, fhsg h, sg, mshg szjh gkj,"
        " s hzgskhg shzktgs t lurzywiurtyzlis uhtzsli uyzi tyz liuhtzli"
        " tyzkr tyzdkl yzdk,  ylktyzdlk dlktuh lkzhluzo huwory gzl",
        300,
        {"abc efg fajshfkg kf gfjg hjgsj dj, fhsg h,", 299},
        {"sg, mshg szjh gkj, s hzgskhg shzktgs t", 283},
        {"lurzywiurtyzlis uhtzsli uyzi tyz liuhtzli", 276},
        {"tyzkr tyzdkl yzdk,  ylktyzdlk dlktuh", 258},
        {"lkzhluzo huwory gzl", 147},
    );

    TEST_LINES(font16,
        "Unauthorized access to this system is forbidden and will be prosecuted"
        " by law. By accessing this system, you agree that your actions may be"
        " monitored if unauthorized usage is suspected.",
        300,
        {"Unauthorized access to this system is", 286},
        {"forbidden and will be prosecuted by", 272},
        {"law. By accessing this system, you", 260},
        {"agree that your actions may be", 235},
        {"monitored if unauthorized usage is", 261},
        {"suspected.", 81},
    );

    TEST_LINES(font16,
        "Unauthorized access to this system is forbidden and will be prosecuted"
        " by law.\nBy accessing this system, you agree that your actions may be"
        " monitored if unauthorized usage is suspected.",
        300,
        {"Unauthorized access to this system is", 286},
        {"forbidden and will be prosecuted by", 272},
        {"law.", 30},
        {"By accessing this system, you agree", 272},
        {"that your actions may be monitored if", 285},
        {"unauthorized usage is suspected.", 250},
    );

    TEST_LINES(font16,
        "Unauthorized access to this system is forbidden and will be prosecuted"
        " by law.\n\nBy accessing this system, you agree that your actions may be"
        " monitored if unauthorized usage is suspected.",
        300,
        {"Unauthorized access to this system is", 286},
        {"forbidden and will be prosecuted by", 272},
        {"law.", 30},
        {"", 0},
        {"By accessing this system, you agree", 272},
        {"that your actions may be monitored if", 285},
        {"unauthorized usage is suspected.", 250},
    );
}
