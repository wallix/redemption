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
*   Copyright (C) Wallix 2012-2022
*   Author(s): Jonathan Poelen
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/log_buffered.hpp"

#include "capture/utils/pattern_searcher.hpp"


namespace
{

struct ComparablePatternFound : PatternSearcher::PatternFound
{
    bool operator == (ComparablePatternFound const& other) const
    {
        return id == other.id
            && is_pattern_kill == other.is_pattern_kill
            && match_len == other.match_len
            && std::equal(pattern.begin(), pattern.end(),
                          other.pattern.begin(), other.pattern.end());
    }
};

struct PatternResults
{
    array_view<PatternSearcher::PatternFound> results;

    PatternResults() = default;

    PatternResults(array_view<PatternSearcher::PatternFound> results)
    : results(results)
    {}

    template<std::size_t N>
    PatternResults(PatternSearcher::PatternFound const (&a)[N]) noexcept
    : results(a)
    {}

#if defined(IN_IDE_PARSER) || REDEMPTION_UNIT_TEST_FAST_CHECK
    array_view<ComparablePatternFound> to_comparable() const
    {
        return array_view{
            static_cast<ComparablePatternFound const*>(results.begin()),
            static_cast<ComparablePatternFound const*>(results.end())
        };
    }

    bool operator == (PatternResults const& other) const
    {
        auto a = to_comparable();
        auto b = other.to_comparable();
        return std::equal(a.begin(), a.end(), b.begin(), b.end());
    }
#endif
};

using PatternResultArray = PatternSearcher::PatternFound[];
#define PattResultA(...) PatternResults(PatternResultArray{__VA_ARGS__})

} // anonymous namespace

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
# include "utils/sugar/int_to_chars.hpp"
# include "test_only/test_framework/compare_collection.hpp"
# include <string_view>

namespace
{

static ut::assertion_result test_comp_patt_results(PatternResults a, PatternResults b)
{
    auto putseq = [&](std::ostream& out, array_view<ComparablePatternFound> results){
        if (results.empty()) {
            out << "{}";
            return;
        }
        for (auto result : results) {
            out << "{"
                ".pattern=" << result.pattern.as<std::string_view>() << ", "
                ".id=" << result.id << ", "
                ".is_pattern_kill="<< result.is_pattern_kill << ", "
                ".match_len=" << result.match_len <<
                "}, "
            ;
        }
    };

    auto to_comparable = [](PatternResults results){
        return array_view{
            static_cast<ComparablePatternFound const*>(results.results.begin()), /*NOLINT*/
            static_cast<ComparablePatternFound const*>(results.results.end())    /*NOLINT*/
        };
    };

    auto results1 = to_comparable(a);
    auto results2 = to_comparable(b);

    return ut::ops::compare_collection_EQ(results1, results2, [&](
        std::ostream& out, size_t /*pos*/, char const* op
    ){
        ut::put_data_with_diff(out, results1, op, results2, putseq);
    });
}

} // anonymous namespace

RED_TEST_DISPATCH_COMPARISON_EQ((), (::PatternResults), (::PatternResults), ::test_comp_patt_results)
#endif


RED_AUTO_TEST_CASE(TestPatternSearcherKbd1)
{
    CapturePattern cap_patterns1[]{
        {CapturePattern::Filters{.is_ocr=true, .is_kbd=true}, CapturePattern::PatternType::reg, "a"_av},
    };
    PatternSearcher searcher(cap_patterns1, {}, CapturePattern::CaptureType::kbd);
    RED_CHECK(PatternResults(searcher.scan("xy"_av)) == PatternResults());
    RED_CHECK(PatternResults(searcher.scan("zabcd"_av)) == PattResultA(
        {.pattern="a"_av, .id=0, .is_pattern_kill=1, .match_len=4},
    ));
}

RED_AUTO_TEST_CASE(TestPatternSearcherKbd2)
{
    CapturePattern cap_patterns1[]{
        {CapturePattern::Filters{.is_ocr=false, .is_kbd=true},  CapturePattern::PatternType::reg, "a"_av},
        {CapturePattern::Filters{.is_ocr=false, .is_kbd=true},  CapturePattern::PatternType::reg, "b"_av},
        {CapturePattern::Filters{.is_ocr=false, .is_kbd=false}, CapturePattern::PatternType::reg, "c"_av},
        {CapturePattern::Filters{.is_ocr=true,  .is_kbd=false}, CapturePattern::PatternType::reg, "d"_av},
    };
    CapturePattern cap_patterns2[]{
        {CapturePattern::Filters{.is_ocr=true, .is_kbd=true}, CapturePattern::PatternType::reg, "c"_av},
        {CapturePattern::Filters{.is_ocr=true, .is_kbd=true}, CapturePattern::PatternType::reg, "x"_av},
        {CapturePattern::Filters{.is_ocr=true, .is_kbd=true}, CapturePattern::PatternType::reg, "de"_av},
    };
    PatternSearcher searcher(cap_patterns1, cap_patterns2, CapturePattern::CaptureType::kbd);
    RED_CHECK(PatternResults(searcher.scan("xy"_av)) == PattResultA(
        {.pattern="x"_av, .id=3, .is_pattern_kill=0, .match_len=2},
    ));
    RED_CHECK(PatternResults(searcher.scan("zabcd"_av)) == PattResultA(
        {.pattern="a"_av, .id=0, .is_pattern_kill=1, .match_len=4},
        {.pattern="b"_av, .id=1, .is_pattern_kill=1, .match_len=3},
        {.pattern="c"_av, .id=2, .is_pattern_kill=0, .match_len=2},
    ));
    RED_CHECK(PatternResults(searcher.scan("ef"_av)) == PattResultA(
        {.pattern="de"_av, .id=4, .is_pattern_kill=0, .match_len=3},
    ));
    RED_CHECK(PatternResults(searcher.scan("de"_av)) == PattResultA(
        {.pattern="de"_av, .id=4, .is_pattern_kill=0, .match_len=2},
    ));
}

RED_AUTO_TEST_CASE(TestPatternSearcherOcr1)
{
    CapturePattern cap_patterns1[]{
        {CapturePattern::Filters{.is_ocr=true, .is_kbd=true}, CapturePattern::PatternType::reg, "a"_av},
    };
    CapturePattern cap_patterns2[]{
        {CapturePattern::Filters{.is_ocr=true, .is_kbd=true}, CapturePattern::PatternType::exact_reg, ".?.?c"_av},
    };
    PatternSearcher searcher(cap_patterns1, cap_patterns2, CapturePattern::CaptureType::ocr);
    RED_CHECK(PatternResults(searcher.scan("zabcd"_av)) == PattResultA(
        {.pattern="a"_av, .id=0, .is_pattern_kill=1, .match_len=0},
    ));
    RED_CHECK(PatternResults(searcher.scan("zbcd"_av)) == PatternResults());
    RED_CHECK(PatternResults(searcher.scan("zbc"_av)) == PattResultA(
        {.pattern=".?.?c"_av, .id=1, .is_pattern_kill=0, .match_len=0},
    ));
}

RED_AUTO_TEST_CASE(TestPatternSearcherOcr2)
{
    CapturePattern cap_patterns1[]{
        {CapturePattern::Filters{.is_ocr=true, .is_kbd=true}, CapturePattern::PatternType::reg, "^é+$"_av},
    };
    PatternSearcher searcher(cap_patterns1, {}, CapturePattern::CaptureType::ocr);
    RED_CHECK(PatternResults(searcher.scan("ééé"_av)) == PattResultA(
        {.pattern="^é+$"_av, .id=0, .is_pattern_kill=1, .match_len=0},
    ));
}

RED_AUTO_TEST_CASE(TestPatternSearcherWithInvalidRegex1)
{
    CapturePattern cap_patterns1[]{
        {CapturePattern::Filters{.is_ocr=true, .is_kbd=true}, CapturePattern::PatternType::reg, "??"_av},
        {CapturePattern::Filters{.is_ocr=true, .is_kbd=true}, CapturePattern::PatternType::reg, "a"_av},
        {CapturePattern::Filters{.is_ocr=true, .is_kbd=true}, CapturePattern::PatternType::reg, "**"_av},
    };
    ut::log_buffered log;
    PatternSearcher searcher(cap_patterns1, {}, CapturePattern::CaptureType::ocr);
    RED_CHECK(searcher.has_pattern());
    RED_CHECK(log.buf() ==
        "ERR -- PatternsSearcher::PatternsSearcher(): '?""?' "
            "failed compilation with error: Invalid repeat at index 0.\n"
        "ERR -- PatternsSearcher::PatternsSearcher(): '*""*' "
            "failed compilation with error: Invalid repeat at index 0.\n");
    RED_CHECK(PatternResults(searcher.scan("zabcd"_av)) == PattResultA(
        {.pattern="a"_av, .id=0, .is_pattern_kill=1, .match_len=0},
    ));
}

RED_AUTO_TEST_CASE(TestPatternSearcherWithInvalidRegex2)
{
    CapturePattern cap_patterns1[]{
        {CapturePattern::Filters{.is_ocr=true, .is_kbd=true}, CapturePattern::PatternType::reg, "??"_av},
        {CapturePattern::Filters{.is_ocr=true, .is_kbd=true}, CapturePattern::PatternType::reg, "**"_av},
    };
    ut::log_buffered log;
    PatternSearcher searcher(cap_patterns1, {}, CapturePattern::CaptureType::kbd);
    RED_CHECK(!searcher.has_pattern());
    RED_CHECK(log.buf() ==
        "ERR -- PatternsSearcher::PatternsSearcher(): '?""?' "
            "failed compilation with error: Invalid repeat at index 0.\n"
        "ERR -- PatternsSearcher::PatternsSearcher(): '*""*' "
            "failed compilation with error: Invalid repeat at index 0.\n"
        "ERR -- PatternsSearcher::PatternsSearcher(): Invalid parameter: elements is zero\n");
    RED_CHECK(PatternResults(searcher.scan("zabcd"_av)) == PatternResults());
}
