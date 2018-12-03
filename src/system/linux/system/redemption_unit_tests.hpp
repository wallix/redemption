#pragma once

#define BOOST_TEST_MODULE RED_TEST_MODULE

#include <boost/test/auto_unit_test.hpp>

#include "utils/sugar/bytes_view.hpp"

#include <iterator>
#include <type_traits>


// // fixed link error (API changed)
// #ifdef __clang__
// namespace boost { namespace unit_test { namespace ut_detail {
//     std::string normalize_test_case_name(const_string name) {
//         return ( name[0] == '&' ? std::string(name.begin()+1, name.size()-1) : std::string(name.begin(), name.size() ));
//     }
// } } }
// #endif

#ifdef IN_IDE_PARSER

namespace redemption_unit_test__
{
    void X(bool);

    struct Stream
    {
        template<class T>
        Stream operator<<(T const&)
        {
            return *this;
        }
    };
}

# define FIXTURES_PATH "./tests/fixtures"
# define CFG_PATH "./sys/etc/rdpproxy"
# define RED_CHECK_EXCEPTION_ERROR_ID(stmt, id) do { stmt; (void)id; } while (0)
# define RED_CHECK_NO_THROW(stmt) do { stmt; } while (0)
# define RED_CHECK_THROW(stmt, exception) do { stmt; [](exception) {}; } while (0)
# define RED_CHECK_EXCEPTION(stmt, exception, predicate) do { \
    stmt; [](exception & e) { predicate(e); }; } while (0)
# define RED_CHECK_EQUAL(a, b) (a) == (b)
# define RED_CHECK_EQ(a, b) ::redemption_unit_test__::X(bool((a) == (b)))
# define RED_CHECK_NE(a, b) ::redemption_unit_test__::X(bool((a) != (b)))
# define RED_CHECK_LT(a, b) ::redemption_unit_test__::X(bool((a) < (b)))
# define RED_CHECK_LE(a, b) ::redemption_unit_test__::X(bool((a) <= (b)))
# define RED_CHECK_GT(a, b) ::redemption_unit_test__::X(bool((a) > (b)))
# define RED_CHECK_GE(a, b) ::redemption_unit_test__::X(bool((a) >= (b)))
# define RED_CHECK(a) ::redemption_unit_test__::X(bool(a))
# define RED_CHECK_MESSAGE(a, iostream_expr) ::redemption_unit_test__::X(bool(a)), \
    ::redemption_unit_test__::Stream{} << iostream_expr
# define RED_CHECK_EQUAL_COLLECTIONS(first1, last1, first2, last2) \
    ::redemption_unit_test__::X(first1 == last1 && first2 == last2)
# define RED_CHECK_EQUAL_RANGES(a, b) ::redemption_unit_test__::X((void(a), void(b), true))
# define RED_CHECK_PREDICATE(pred, arg_list) pred arg_list

# define RED_REQUIRE_NO_THROW(stmt) do { stmt; } while (0)
# define RED_REQUIRE_THROW(stmt, exception) do { stmt; [](exception) {}; } while (0)
# define RED_REQUIRE_EXCEPTION(stmt, exception, predicate) do {\
    stmt; [](exception & e) { predicate(e); }; } while (0)
# define RED_REQUIRE_EQUAL(a, b) ::redemption_unit_test__::X(bool((a) == (b)))
# define RED_REQUIRE_EQ(a, b) ::redemption_unit_test__::X(bool((a) == (b)))
# define RED_REQUIRE_NE(a, b) ::redemption_unit_test__::X(bool((a) != (b)))
# define RED_REQUIRE_LT(a, b) ::redemption_unit_test__::X(bool((a) < (b)))
# define RED_REQUIRE_LE(a, b) ::redemption_unit_test__::X(bool((a) <= (b)))
# define RED_REQUIRE_GT(a, b) ::redemption_unit_test__::X(bool((a) > (b)))
# define RED_REQUIRE_GE(a, b) ::redemption_unit_test__::X(bool((a) >= (b)))
# define RED_REQUIRE(a) ::redemption_unit_test__::X(bool(a))
# define RED_REQUIRE_MESSAGE(a, iostream_expr) ::redemption_unit_test__::X(bool(a)), \
    ::redemption_unit_test__::Stream{} << iostream_expr
# define RED_REQUIRE_EQUAL_COLLECTIONS(first1, last1, first2, last2) \
    ::redemption_unit_test__::X(first1 == last1 && first2 == last2)
# define RED_REQUIRE_EQUAL_RANGES(a, b) ::redemption_unit_test__::X((void(a), void(b), true))
# define RED_REQUIRE_PREDICATE(pred, arg_list) pred arg_list

# define RED_FAIL(mess) ::redemption_unit_test__::Stream{} << mess
# define RED_ERROR(mess) ::redemption_unit_test__::Stream{} << mess

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
        [&](Error const & e__) {                   \
            RED_CHECK_EQUAL(e__.id, ErrId);        \
            return (e__.id == ErrId);              \
        }                                          \
    )

# define RED_TEST_EQUAL_RANGES_IMPL(impl, a, b) \
    [](auto const & a__, auto const & b__) {    \
        using std::begin;                       \
        using std::end;                         \
        impl(                                   \
            (void(#a), begin(a__)), end(a__),   \
            (void(#b), begin(b__)), end(b__)    \
        );                                      \
    }(a, b)

# define RED_CHECK_EQUAL_RANGES(a, b) \
    RED_TEST_EQUAL_RANGES_IMPL(RED_CHECK_EQUAL_COLLECTIONS, a, b)

# define RED_REQUIRE_EQUAL_RANGES(a, b) \
    RED_TEST_EQUAL_RANGES_IMPL(RED_REQUIRE_REQUIRE_COLLECTIONS, a, b)

#endif

#define RED_CHECK_EQ_RANGES RED_CHECK_EQUAL_RANGES
#define RED_REQUIRE_EQ_RANGES RED_REQUIRE_EQUAL_RANGES

namespace std // NOLINT(cert-dcl58-cpp)
{
    // this is a hack...
    template<class Ch, class Tr>
    std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & out, decltype(nullptr) const & /*nullptr*/)
    {
        return out << "nullptr";
    }
} // namespace std


template<class Ch, class Tr, class E>
typename std::enable_if<
    std::is_enum<E>::value,
    std::basic_ostream<Ch, Tr>&
>::type
operator<<(std::basic_ostream<Ch, Tr> & out, E const & e)
{
    return out << +underlying_cast(e); // '+' for transform u8/s8 to int
}


namespace redemption_unit_test__
{
    struct xarray
    {
        const_bytes_view sig;

        std::size_t size() const
        {
            return sig.size();
        }

        bool operator == (xarray const & other) const;
    };

    struct xarray_color
    {
        size_t & res;
        const_bytes_view sig;

        std::size_t size() const
        {
            return sig.size();
        }

        bool operator == (xarray_color const & other) const;
    };

    std::ostream & operator<<(std::ostream & out, xarray_color const & x);
    std::ostream & operator<<(std::ostream & out, xarray const & x);
}  // namespace redemption_unit_test__

#define RED_TEST_OP_VAR_MESSAGE(mode, op, v1, v2, src1, src2) \
    RED_CHECK_MESSAGE(                                        \
        (v1) op (v2),                                         \
        #mode " " #src1 " " #op " " #src2 " failed ["         \
        << v1 << " " #op " " << v2 << "]"                     \
    );

#define RED_TEST_MEM_IMPL(mode, mem, memref)                              \
    [](auto const& x_mem__, auto const& x_memref__){                      \
        size_t res__ = 0;                                                 \
        redemption_unit_test__::xarray_color mem__{res__, x_mem__};       \
        redemption_unit_test__::xarray_color memref__{res__, x_memref__}; \
        RED_TEST_OP_VAR_MESSAGE(                                          \
            mode, ==, mem__.size(), memref__.size(),                      \
            mem.size(), memref.size());                                   \
        RED_TEST_OP_VAR_MESSAGE(mode, ==, mem__, memref__, mem, memref);  \
    }(mem, memref)                                                        \

#define RED_CHECK_MEM(mem, memref) \
    RED_TEST_MEM_IMPL(check, mem, memref)

#define RED_REQUIRE_MEM(mem, memref) \
    RED_TEST_MEM_IMPL(require, mem, memref)

#ifdef IN_IDE_PARSER
# define RED_CHECK_MEM_C(mem, memref) void(mem), void("" memref)
# define RED_CHECK_MEM_AC(mem, memref) void(mem), void("" memref)
# define RED_CHECK_MEM_AA(mem, memref) void(mem), void(memref)
# define RED_REQUIRE_MEM_C(mem, memref) void(mem), void("" memref)
# define RED_REQUIRE_MEM_AC(mem, memref) void(mem), void("" memref)
# define RED_REQUIRE_MEM_AA(mem, memref) void(mem), void(memref)
#else
# define RED_CHECK_MEM_C(mem, memref) RED_CHECK_MEM(mem, cstr_array_view("" memref))
# define RED_CHECK_MEM_AC(mem, memref) RED_CHECK_MEM(make_array_view(mem), cstr_array_view("" memref))
# define RED_CHECK_MEM_AA(mem, memref) RED_CHECK_MEM(make_array_view(mem), make_array_view(memref))
# define RED_REQUIRE_MEM_C(mem, memref) RED_REQUIRE_MEM(mem, cstr_array_view("" memref))
# define RED_REQUIRE_MEM_AC(mem, memref) RED_REQUIRE_MEM(make_array_view(mem), cstr_array_view("" memref))
# define RED_REQUIRE_MEM_AA(mem, memref) RED_REQUIRE_MEM(make_array_view(mem), make_array_view(memref))
# endif

namespace redemption_unit_test__
{
    struct xsarray
    {
        const_bytes_view sig;

        std::size_t size() const
        {
            return sig.size();
        }

        bool operator == (xsarray const & other) const;
    };

    std::ostream & operator<<(std::ostream & out, xsarray const & x);
}  // namespace redemption_unit_test__

#define RED_TEST_SMEM_IMPL(mode, mem, memref)                 \
    [](auto const& x_mem__, auto const& x_memref__){          \
        redemption_unit_test__::xsarray mem__{x_mem__};       \
        redemption_unit_test__::xsarray memref__{x_memref__}; \
        RED_TEST_OP_VAR_MESSAGE(                              \
            mode, ==, mem__.size(), memref__.size(),          \
            mem.size(), memref.size());                       \
        RED_TEST_OP_VAR_MESSAGE(                              \
            mode, ==, mem__, memref__,                        \
            mem, memref);                                     \
    }(mem, memref)

#define RED_CHECK_SMEM(mem, memref) \
    RED_TEST_SMEM_IMPL(check, mem, memref)

#ifdef IN_IDE_PARSER
# define RED_CHECK_SMEM_C(mem, memref) void(mem), void("" memref)
# define RED_CHECK_SMEM_AC(mem, memref) void(mem), void("" memref)
# define RED_CHECK_SMEM_AA(mem, memref) void(mem), void(memref)
#else
# define RED_CHECK_SMEM_C(mem, memref) RED_CHECK_SMEM(mem, cstr_array_view("" memref))
# define RED_CHECK_SMEM_AC(mem, memref) RED_CHECK_SMEM(make_array_view(mem), cstr_array_view("" memref))
# define RED_CHECK_SMEM_AA(mem, memref) RED_CHECK_SMEM(make_array_view(mem), make_array_view(memref))
# endif


// require #include "utils/fileutils.hpp"
#define RED_CHECK_FILE_SIZE_AND_CLEAN(filename, size)                     \
    [](auto&& filename__, ::std::size_t size__) {                         \
        RED_CHECK_EQUAL(filesize((void(#filename), filename__)), size__); \
        ::unlink(filename__);                                             \
    }(filename, size)

#define RED_CHECK_FILE_SIZE_AND_CLEAN2(filename, size1, size2)                    \
    [](auto&& filename__, ::std::size_t size1__, ::std::size_t size2__) {         \
        size_t fsize__ = filesize(filename__);                                    \
        if (fsize__ != size2__){                                                  \
            RED_CHECK_EQUAL((void("filesize(" #filename ")"), fsize__), size1__); \
        }                                                                         \
        ::unlink(filename__);                                                     \
    }(filename, size1, size2)

#define RED_CHECK_FILE_SIZE_AND_CLEAN3(filename, size1, size2, size3)                            \
    [](auto&& filename__, ::std::size_t size1__, ::std::size_t size2__, ::std::size_t size3__) { \
        size_t fsize__ = filesize(filename__);                                                   \
        if (fsize__ != size2__ && fsize__ != size3__) {                                          \
            RED_CHECK_EQUAL((void("filesize(" #filename ")"), fsize__), size1__);                \
        }                                                                                        \
        ::unlink(filename__);                                                                    \
    }(filename, size1, size2, size3)


#define RED_CHECK_FILE_EXISTS(filename)                               \
    [](auto&& filename__) {                                           \
        RED_CHECK_MESSAGE(                                            \
            file_exist(filename__),                                   \
            "check file_exist(\"" << filename__ << "\") has failed"); \
    }(filename)

#define RED_CHECK_FILE_NOT_EXISTS(filename)                            \
    [](auto&& filename__) {                                            \
        RED_CHECK_MESSAGE(                                             \
            !file_exist(filename__),                                   \
            "check !file_exist(\"" << filename__ << "\") has failed"); \
    }(filename)

// require #include "test_only/get_file_contents.hpp"
#define RED_CHECK_FILE_CONTENTS(filename, contents)                                 \
    [](auto&& filename__, auto&& contents__) {                                      \
        RED_CHECK_EQ(get_file_contents((void(#filename), filename__)), contents__); \
        ::unlink(filename__);                                                       \
    }(filename, contents)


#if !defined(REDEMPTION_UNIT_TEST_CPP) && defined(RED_TEST_MODULE)
# ifdef IN_IDE_PARSER
#  undef BOOST_AUTO_TEST_CASE
#  define BOOST_AUTO_TEST_CASE(test_name)     \
    struct test_name { static void test(); }; \
    void test_name::test()
# endif
# define RED_AUTO_TEST_CASE BOOST_AUTO_TEST_CASE
#else
# undef BOOST_AUTO_TEST_CASE
#endif
