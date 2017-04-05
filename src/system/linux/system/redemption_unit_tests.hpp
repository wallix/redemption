#pragma once

#include <boost/test/auto_unit_test.hpp>

// // fixed link error (API changed)
// #ifdef __clang__
// namespace boost { namespace unit_test { namespace ut_detail {
//     std::string normalize_test_case_name(const_string name) {
//         return ( name[0] == '&' ? std::string(name.begin()+1, name.size()-1) : std::string(name.begin(), name.size() ));
//     }
// } } }
// #endif

#ifdef IN_IDE_PARSER
# define FIXTURES_PATH "./tests/fixtures"
# define CFG_PATH "./sys/etc/rdpproxy"
# define CHECK_EXCEPTION_ERROR_ID(stmt, id) do { stmt; id; } while (0)
# define BOOST_CHECK_NO_THROW(stmt) do { stmt; } while (0)
# define BOOST_CHECK_THROW(stmt, exception) do { stmt; [](exception) {}; } while (0)
# define BOOST_CHECK_EXCEPTION(stmt, exception, predicate) do {\
    stmt; [](exception & e) { predicate(e); }; } while (0)
# define BOOST_CHECK_EQUAL(a, b) (a) == (b)
# define BOOST_CHECK_EQ(a, b) (a) == (b)
# define BOOST_CHECK_NE(a, b) (a) != (b)
# define BOOST_CHECK_LT(a, b) (a) < (b)
# define BOOST_CHECK_LE(a, b) (a) <= (b)
# define BOOST_CHECK_GT(a, b) (a) > (b)
# define BOOST_CHECK_GE(a, b) (a) >= (b)
# define BOOST_CHECK(a) (a)
# define BOOST_CHECK_MESSAGE(a, iostream_expr) (a), ""
# define BOOST_CHECK_EQUAL_RANGES(a, b) (a) != (b)

# define BOOST_REQUIRE_NO_THROW(stmt) do { stmt; } while (0)
# define BOOST_REQUIRE_THROW(stmt, exception) do { stmt; [](exception) {}; } while (0)
# define BOOST_REQUIRE_EXCEPTION(stmt, exception, predicate) do {\
    stmt; [](exception & e) { predicate(e); }; } while (0)
# define BOOST_REQUIRE_EQUAL(a, b) (a) == (b)
# define BOOST_REQUIRE_EQ(a, b) (a) == (b)
# define BOOST_REQUIRE_NE(a, b) (a) != (b)
# define BOOST_REQUIRE_LT(a, b) (a) < (b)
# define BOOST_REQUIRE_LE(a, b) (a) <= (b)
# define BOOST_REQUIRE_GT(a, b) (a) > (b)
# define BOOST_REQUIRE_GE(a, b) (a) >= (b)
# define BOOST_REQUIRE(a) (a)
# define BOOST_REQUIRE_MESSAGE(a, iostream_expr) (a), ""
# define BOOST_REQUIRE_EQUAL_RANGES(a, b) (a) != (b)
#else
# define BOOST_CHECK_EQ BOOST_CHECK_EQUAL
# define BOOST_REQUIRE_EQ BOOST_REQUIRE_EQUAL
# define CHECK_EXCEPTION_ERROR_ID(stmt, ErrId)  \
    BOOST_CHECK_EXCEPTION(                      \
        stmt, Error,                            \
        [&](Error const & e) {                  \
            if (e.id == ErrId) {                \
                BOOST_CHECK_EQUAL(e.id, ErrId); \
                return true;                    \
            }                                   \
            return false;                       \
        }                                       \
    )
# define BOOST_CHECK_EQUAL_RANGES(a_, b_)                 \
    do {                                                  \
        auto const & A__CHECK_RANGES = a_;                \
        auto const & B__CHECK_RANGES = b_;                \
        using std::begin;                                 \
        using std::end;                                   \
        BOOST_CHECK_EQUAL_COLLECTIONS(                    \
            begin(A__CHECK_RANGES), end(A__CHECK_RANGES), \
            begin(B__CHECK_RANGES), end(B__CHECK_RANGES)  \
        );                                                \
    } while (0)
# define BOOST_REQUIRE_EQUAL_RANGES(a_, b_)               \
    do {                                                  \
        auto const & A__CHECK_RANGES = a_;                \
        auto const & B__CHECK_RANGES = b_;                \
        using std::begin;                                 \
        using std::end;                                   \
        BOOST_REQUIRE_EQUAL_COLLECTIONS(                  \
            begin(A__CHECK_RANGES), end(A__CHECK_RANGES), \
            begin(B__CHECK_RANGES), end(B__CHECK_RANGES)  \
        );                                                \
    } while (0)
#endif

#include "cxx/cxx.hpp"

namespace redemption_unit_test__
{
    REDEMPTION_LIB_EXPORT
    bool check_mem(const void * p, std::size_t len, const void * mem, char * message);
}

#define CHECK_MEM(p, len, mem)                                          \
    do {                                                                \
        char message[len * 5 + 256];                                    \
        if (!redemption_unit_test__::check_mem(p, len, mem, message)) { \
            BOOST_CHECK_MESSAGE(false, message);                        \
        }                                                               \
    } while (0)


// force line to last checkpoint
#ifndef IN_IDE_PARSER
# undef BOOST_AUTO_TEST_CASE
# define BOOST_AUTO_TEST_CASE(test_name)                                                              \
    BOOST_FIXTURE_TEST_CASE(test_name##_start__, BOOST_AUTO_TEST_CASE_FIXTURE) { BOOST_CHECK(true); } \
    BOOST_FIXTURE_TEST_CASE(test_name, BOOST_AUTO_TEST_CASE_FIXTURE)
#endif
