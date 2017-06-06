#pragma once

#define BOOST_TEST_MODULE RED_TEST_MODULE

#include <boost/test/auto_unit_test.hpp>
#include <algorithm>

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
# define RED_CHECK_EXCEPTION_ERROR_ID(stmt, id) do { stmt; id; } while (0)
# define RED_CHECK_NO_THROW(stmt) do { stmt; } while (0)
# define RED_CHECK_THROW(stmt, exception) do { stmt; [](exception) {}; } while (0)
# define RED_CHECK_EXCEPTION(stmt, exception, predicate) do {\
    stmt; [](exception & e) { predicate(e); }; } while (0)
# define RED_CHECK_EQUAL(a, b) (a) == (b)
# define RED_CHECK_EQ(a, b) (a) == (b)
# define RED_CHECK_NE(a, b) (a) != (b)
# define RED_CHECK_LT(a, b) (a) < (b)
# define RED_CHECK_LE(a, b) (a) <= (b)
# define RED_CHECK_GT(a, b) (a) > (b)
# define RED_CHECK_GE(a, b) (a) >= (b)
# define RED_CHECK(a) (a)
# define RED_CHECK_MESSAGE(a, iostream_expr) (a), "" << iostream_expr
# define RED_CHECK_EQUAL_COLLECTIONS(a, b) (a) == (b)
# define RED_CHECK_PREDICATE(pred, arg_list) pred arg_list

# define RED_REQUIRE_NO_THROW(stmt) do { stmt; } while (0)
# define RED_REQUIRE_THROW(stmt, exception) do { stmt; [](exception) {}; } while (0)
# define RED_REQUIRE_EXCEPTION(stmt, exception, predicate) do {\
    stmt; [](exception & e) { predicate(e); }; } while (0)
# define RED_REQUIRE_EQUAL(a, b) (a) == (b)
# define RED_REQUIRE_EQ(a, b) (a) == (b)
# define RED_REQUIRE_NE(a, b) (a) != (b)
# define RED_REQUIRE_LT(a, b) (a) < (b)
# define RED_REQUIRE_LE(a, b) (a) <= (b)
# define RED_REQUIRE_GT(a, b) (a) > (b)
# define RED_REQUIRE_GE(a, b) (a) >= (b)
# define RED_REQUIRE(a) (a)
# define RED_REQUIRE_MESSAGE(a, iostream_expr) (a), "" << iostream_expr
# define RED_REQUIRE_EQUAL_COLLECTIONS(a, b) (a) == (b)
# define RED_REQUIRE_PREDICATE(pred, arg_list) pred arg_list

# define RED_FAIL(mess) mess
# define RED_ERROR(mess) mess

#else

# define RED_CHECK_NO_THROW BOOST_CHECK_NO_THROW
# define RED_CHECK_THROW BOOST_CHECK_THROW
# define RED_CHECK_EXCEPTION BOOST_CHECK_EXCEPTION
# define RED_CHECK_EQUAL BOOST_CHECK_EQUAL
# define RED_CHECK_EQ BOOST_CHECK_EQUAL
# define RED_CHECK_NE BOOST_CHECK_NE
# define RED_CHECK_LT BOOST_CHECK_LT
# define RED_CHECK_LE BOOST_CHECK_LE
# define RED_CHECK_GT BOOST_CHECK_GT
# define RED_CHECK_GE BOOST_CHECK_GE
# define RED_CHECK BOOST_CHECK
# define RED_CHECK_MESSAGE BOOST_CHECK_MESSAGE
# define RED_CHECK_EQUAL_COLLECTIONS BOOST_CHECK_EQUAL_COLLECTIONS
# define RED_CHECK_PREDICATE BOOST_CHECK_PREDICATE


# define RED_REQUIRE_NO_THROW BOOST_REQUIRE_NO_THROW
# define RED_REQUIRE_THROW BOOST_REQUIRE_THROW
# define RED_REQUIRE_EXCEPTION BOOST_REQUIRE_EXCEPTION
# define RED_REQUIRE_EQUAL BOOST_REQUIRE_EQUAL
# define RED_REQUIRE_EQ BOOST_REQUIRE_EQUAL
# define RED_REQUIRE_NE BOOST_REQUIRE_NE
# define RED_REQUIRE_LT BOOST_REQUIRE_LT
# define RED_REQUIRE_LE BOOST_REQUIRE_LE
# define RED_REQUIRE_GT BOOST_REQUIRE_GT
# define RED_REQUIRE_GE BOOST_REQUIRE_GE
# define RED_REQUIRE BOOST_REQUIRE
# define RED_REQUIRE_MESSAGE BOOST_REQUIRE_MESSAGE
# define RED_REQUIRE_EQUAL_COLLECTIONS BOOST_REQUIRE_EQUAL_COLLECTIONS
# define RED_REQUIRE_PREDICATE BOOST_REQUIRE_PREDICATE

# define RED_FAIL BOOST_FAIL
# define RED_ERROR BOOST_ERROR


# define RED_CHECK_EXCEPTION_ERROR_ID(stmt, ErrId) \
    RED_CHECK_EXCEPTION(                           \
        stmt, Error,                               \
        [&](Error const & e) {                     \
            if (e.id == ErrId) {                   \
                return true;                       \
            }                                      \
            RED_CHECK_EQUAL(e.id, ErrId);          \
            return false;                          \
        }                                          \
    )

#endif

# define RED_CHECK_EQUAL_RANGES(a_, b_)                   \
    do {                                                  \
        auto const & A__CHECK_RANGES = a_;                \
        auto const & B__CHECK_RANGES = b_;                \
        using std::begin;                                 \
        using std::end;                                   \
        RED_CHECK_EQUAL_COLLECTIONS(                      \
            begin(A__CHECK_RANGES), end(A__CHECK_RANGES), \
            begin(B__CHECK_RANGES), end(B__CHECK_RANGES)  \
        );                                                \
    } while (0)

# define RED_REQUIRE_EQUAL_RANGES(a_, b_)                 \
    do {                                                  \
        auto const & A__CHECK_RANGES = a_;                \
        auto const & B__CHECK_RANGES = b_;                \
        using std::begin;                                 \
        using std::end;                                   \
        RED_REQUIRE_EQUAL_COLLECTIONS(                    \
            begin(A__CHECK_RANGES), end(A__CHECK_RANGES), \
            begin(B__CHECK_RANGES), end(B__CHECK_RANGES)  \
        );                                                \
    } while (0)

namespace std
{
    // this is a hack...
    template<class Ch, class Tr>
    std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & out, decltype(nullptr) const &)
    {
        return out << "nullptr";
    }
}

#include "cxx/cxx.hpp"
#include "utils/sugar/byte.hpp"

namespace redemption_unit_test__
{
    struct xarray
    {
        const_byte_array sig;

        std::size_t size() const
        {
            return sig.size();
        }

        bool operator == (xarray const & other) const
        {
            return other.sig.size() == other.sig.size()
                && std::equal(sig.begin(), sig.end(), other.sig.begin());
        }
    };

    inline std::ostream & operator<<(std::ostream & out, xarray const & x)
    {
        out << "\"";
        char const * hex_table = "0123456789abcdef";
        for (unsigned c : x.sig) {
            out << "\\x" << hex_table[c >> 4] << hex_table[c & 0xf];
        }
        return out << "\"";
    }
}

#define RED_CHECK_MEM(mem, memref)                        \
    do {                                                  \
        redemption_unit_test__::xarray mem__ {mem};       \
        redemption_unit_test__::xarray memref__ {memref}; \
        RED_CHECK_EQUAL(mem__.size(), memref__.size());   \
        RED_CHECK_EQUAL(mem__, memref__);                 \
    } while (0)

#ifdef IN_IDE_PARSER
# define RED_CHECK_MEM_C(mem, memref) void(mem), void("" memref)
# define RED_CHECK_MEM_AC(mem, memref) void(mem), void("" memref)
# define RED_CHECK_MEM_AA(mem, memref) void(mem), void(memref)
#else
# define RED_CHECK_MEM_C(mem, memref) RED_CHECK_MEM(mem, cstr_array_view("" memref))
# define RED_CHECK_MEM_AC(mem, memref) RED_CHECK_MEM(make_array_view(mem), cstr_array_view("" memref))
# define RED_CHECK_MEM_AA(mem, memref) RED_CHECK_MEM(make_array_view(mem), make_array_view(memref))
# endif

// force line to last checkpoint
#ifndef IN_IDE_PARSER
# undef BOOST_AUTO_TEST_CASE
# define BOOST_AUTO_TEST_CASE(test_name)                                                              \
    BOOST_FIXTURE_TEST_CASE(test_name##_start__, BOOST_AUTO_TEST_CASE_FIXTURE) { BOOST_CHECK(true); } \
    BOOST_FIXTURE_TEST_CASE(test_name, BOOST_AUTO_TEST_CASE_FIXTURE)
#endif

#define RED_AUTO_TEST_CASE BOOST_AUTO_TEST_CASE
