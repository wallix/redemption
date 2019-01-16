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

#pragma once

#include "utils/sugar/bytes_view.hpp"
#include <iterator> // std:begin / std::end
#include <type_traits>


#if defined(IN_IDE_PARSER) && !defined(REDEMPTION_UNIT_TEST_CPP)

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
} // namespace redemption_unit_test__

# define FIXTURES_PATH "./tests/fixtures"
# define CFG_PATH "./sys/etc/rdpproxy"

# define RED_FAIL(mess) ::redemption_unit_test__::Stream{} << mess
# define RED_ERROR(mess) ::redemption_unit_test__::Stream{} << mess
# define RED_TEST_CHECKPOINT(name) do { } while(0)
# define RED_TEST_PASSPOINT() do { } while(0)
# define RED_TEST_DONT_PRINT_LOG_VALUE(type)
# define RED_TEST_PRINT_TYPE_FUNCTION_NAME red_test_print_type

# define RED_AUTO_TEST_CASE(test_name)       \
    struct test_name { void operator()(); }; \
    void test_name::operator()()

/// CHECK
//@{
# define RED_CHECK_EXCEPTION_ERROR_ID(stmt, id) do { stmt; (void)id; } while (0)
# define RED_CHECK_NO_THROW(stmt) do { stmt; } while (0)
# define RED_CHECK_THROW(stmt, exception) do { stmt; [](exception) {}; } while (0)
# define RED_CHECK_EXCEPTION(stmt, exception, predicate) do { \
    stmt; [](exception & e) { predicate(e); }; } while (0)
# define RED_CHECK_EQUAL(a, b) ::redemption_unit_test__::X(bool((a) == (b)))
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
# define RED_CHECK_MEM(mem, memref) void(mem), void(memref)
# define RED_CHECK_SMEM(mem, memref) void(mem), void(memref)

// require #include "utils/fileutils.hpp"
#define RED_CHECK_FILE_SIZE_AND_CLEAN(filename, size) \
    ::redemption_unit_test__::X(bool(filesize(filename) == (size)))

#define RED_CHECK_FILE_SIZE_AND_CLEAN2(filename, size1, size2) \
    ::redemption_unit_test__::X(bool(filesize(filename) == (void(size1), size2)))

#define RED_CHECK_FILE_SIZE_AND_CLEAN3(filename, size1, size2, size3) \
    ::redemption_unit_test__::X(bool(filesize(filename) == (void(size1), void(size2), size3)))

// require #include "test_only/get_file_contents.hpp"
#define RED_CHECK_FILE_CONTENTS(filename, contents) \
    ::redemption_unit_test__::X(bool(get_file_contents(filename) == (contents)))
//@}

/// REQUIRE
//@{
# define RED_REQUIRE_EXCEPTION_ERROR_ID(stmt, id) do { stmt; (void)id; } while (0)
# define RED_REQUIRE_NO_THROW(stmt) do { stmt; } while (0)
# define RED_REQUIRE_THROW(stmt, exception) do { stmt; [](exception) {}; } while (0)
# define RED_REQUIRE_EXCEPTION(stmt, exception, predicate) do { \
    stmt; [](exception & e) { predicate(e); }; } while (0)
# define RED_REQUIRE_EQUAL(a, b) ::redemption_unit_test__::X(bool((a) == (b)))
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
# define RED_REQUIRE_MEM(mem, memref) void(mem), void(memref)
# define RED_REQUIRE_SMEM(mem, memref) void(mem), void(memref)

// require #include "utils/fileutils.hpp"
# define RED_REQUIRE_FILE_SIZE_AND_CLEAN(filename, size) \
    ::redemption_unit_test__::X(bool(filesize(filename) == (size)))

# define RED_REQUIRE_FILE_SIZE_AND_CLEAN2(filename, size1, size2) \
    ::redemption_unit_test__::X(bool(filesize(filename) == (void(size1), size2)))

# define RED_REQUIRE_FILE_SIZE_AND_CLEAN3(filename, size1, size2, size3) \
    ::redemption_unit_test__::X(bool(filesize(filename) == (void(size1), void(size2), size3)))

// require #include "test_only/get_file_contents.hpp"
# define RED_REQUIRE_FILE_CONTENTS(filename, contents) \
    ::redemption_unit_test__::X(bool(get_file_contents(filename) == (contents)))
//@}

#else

# include "impl/redemption_unit_tests_impl.hpp"

# define RED_CHECK_MEM(mem, memref) RED_TEST_MEM(CHECK, mem, memref)
# define RED_CHECK_SMEM(mem, memref) RED_TEST_SMEM(CHECK, mem, memref)

/// CHECK
//@{
# define RED_CHECK_EXCEPTION_ERROR_ID(stmt, ErrId) \
    RED_TEST_EXCEPTION_ERROR_ID(CHECK, stmt, ErrId)

# define RED_CHECK_EQUAL_RANGES(a, b) \
    RED_TEST_EQUAL_RANGES(CHECK, a, b)

// require #include "utils/fileutils.hpp"
# define RED_CHECK_FILE_SIZE_AND_CLEAN(filename, size) \
    RED_TEST_FILE_SIZE_AND_CLEAN(CHECK, filename, size)

# define RED_CHECK_FILE_SIZE_AND_CLEAN2(filename, size1, size2) \
    RED_TEST_FILE_SIZE_AND_CLEAN2(CHECK, filename, size1, size2)

# define RED_CHECK_FILE_SIZE_AND_CLEAN3(filename, size1, size2, size3) \
    RED_TEST_FILE_SIZE_AND_CLEAN3(CHECK, filename, size1, size2, size3)

// require #include "test_only/get_fikle_contents.hpp"
# define RED_CHECK_FILE_CONTENTS(filename, contents) \
    RED_TEST_FILE_CONTENTS(CHECK, filename, contents)
//@}

/// REQUIRE
//@{
# define RED_REQUIRE_EXCEPTION_ERROR_ID(stmt, ErrId) \
    RED_TEST_EXCEPTION_ERROR_ID(REQUIRE, stmt, ErrId)

# define RED_REQUIRE_EQUAL_RANGES(a, b) \
    RED_TEST_EQUAL_RANGES(REQUIRE, a, b)

// require #include "utils/fileutils.hpp"
# define RED_REQUIRE_FILE_SIZE_AND_CLEAN(filename, size) \
    RED_TEST_FILE_SIZE_AND_CLEAN(REQUIRE, filename, size)

# define RED_REQUIRE_FILE_SIZE_AND_CLEAN2(filename, size1, size2) \
    RED_TEST_FILE_SIZE_AND_CLEAN2(REQUIRE, filename, size1, size2)

# define RED_REQUIRE_FILE_SIZE_AND_CLEAN3(filename, size1, size2, size3) \
    RED_TEST_FILE_SIZE_AND_CLEAN3(REQUIRE, filename, size1, size2, size3)

// require #include "test_only/get_fikle_contents.hpp"
# define RED_REQUIRE_FILE_CONTENTS(filename, contents) \
    RED_TEST_FILE_CONTENTS(REQUIRE, filename, contents)
//@}


# define RED_TEST_STRING_CHECK "check"
# define RED_TEST_STRING_REQUIRE "require"

# define RED_TEST_EXCEPTION_ERROR_ID(lvl, stmt, ErrId) \
    RED_##lvl##_EXCEPTION(                             \
        stmt, Error,                                   \
        [&](Error const & e__) {                       \
            RED_CHECK_EQUAL(e__.id, ErrId);            \
            return (e__.id == ErrId);                  \
        }                                              \
    )

# define RED_TEST_EQUAL_RANGES(lvl, a, b)     \
    [](auto const & a__, auto const & b__) {  \
        using std::begin;                     \
        using std::end;                       \
        RED_##lvl##_EQUAL_COLLECTIONS(        \
            (void(#a), begin(a__)), end(a__), \
            (void(#b), begin(b__)), end(b__)  \
        );                                    \
    }(a, b)

# define RED_TEST_MEM(lvl, mem, memref)                                     \
    [](auto const& x_mem__, auto const& x_memref__){                        \
        size_t res__ = 0;                                                   \
        ::redemption_unit_test__::xarray_color mem__{res__, x_mem__};       \
        ::redemption_unit_test__::xarray_color memref__{res__, x_memref__}; \
        RED_##lvl##_EQUAL(                                                  \
            (void("mem__ = " #mem), mem__.size()),                          \
            (void("memref__ = " #memref), memref__.size()));                \
        RED_##lvl##_EQUAL(                                                  \
            (void("mem__ = " #mem), mem__),                                 \
            (void("memref__ = " #memref), memref__));                       \
    }(mem, memref)

# define RED_TEST_SMEM(lvl, mem, memref)                        \
    [](auto const& x_mem__, auto const& x_memref__){            \
        ::redemption_unit_test__::xsarray mem__{x_mem__};       \
        ::redemption_unit_test__::xsarray memref__{x_memref__}; \
        RED_##lvl##_EQUAL(                                      \
            (void("mem__ = " #mem), mem__.size()),              \
            (void("memref__ = " #memref), memref__.size()));    \
        RED_##lvl##_EQUAL(                                      \
            (void("mem__ = " #mem), mem__),                     \
            (void("memref__ = " #memref), memref__));           \
    }(mem, memref)

# define RED_TEST_FILE_SIZE_AND_CLEAN(lvl, filename, size)          \
    [](auto&& filename__, int const size__) {                       \
        int const fsize__ = filesize(filename__);                   \
        RED_##lvl##_MESSAGE(                                        \
            fsize__ == size__,                                      \
            RED_TEST_STRING_##lvl << fsize__ << " == " << size__ << \
            " has failed [filesize(" #filename ") != " #size "]");  \
        ::unlink(filename__);                                       \
    }(filename, size)


# define RED_TEST_FILE_SIZE_AND_CLEAN2(lvl, filename, size1, size2)   \
    [](auto&& filename__, int const size1__, int const size2__) {     \
        int const fsize__ = filesize(filename__);                     \
        RED_##lvl##_MESSAGE(                                          \
            fsize__ == size1__ || fsize__ == size2__,                 \
            RED_TEST_STRING_##lvl << fsize__ << " == (" << size1__ << \
            " or " << size2__ << ") has failed [filesize(" #filename  \
            ") != (" #size1 " or " #size2 ")]");                      \
        ::unlink(filename__);                                         \
    }(filename, size1, size2)


# define RED_TEST_FILE_SIZE_AND_CLEAN3(lvl, filename, size1, size2, size3)                 \
    [](auto&& filename__, int const size1__, int const size2__, int const size3__) {       \
        int const fsize__ = filesize(filename__);                                          \
        RED_##lvl##_MESSAGE(                                                               \
            fsize__ == size1__ || fsize__ == size2__ || fsize__ == size3__,                \
            RED_TEST_STRING_##lvl << fsize__ << " == (" << size1__ << " or " << size2__ << \
            " or " << size3__ << ") has failed [filesize(" #filename                       \
            ") != (" #size1 " or " #size2 " or " #size3 ")]");                             \
        ::unlink(filename__);                                                              \
    }(filename, size1, size2, size3)


// require #include "test_only/get_file_contents.hpp"
# define RED_TEST_FILE_CONTENTS(lvl, filename, contents)      \
    [](auto&& filename__, auto&& contents__) {                \
        auto fcontents__ = get_file_contents(filename__);     \
        RED_##lvl##_MESSAGE(                                  \
            contents__ == fcontents__,                        \
            RED_TEST_STRING_##lvl << fcontents__ << " == " << \
            contents__ << " has failed [get_file_contents("   \
            #filename ") != " #contents "]");                 \
    }(filename, contents)


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
} // namespace redemption_unit_test__

#endif

#define RED_TEST_DELEGATE_PRINT(type, exp)                   \
  inline ::std::ostream& RED_TEST_PRINT_TYPE_FUNCTION_NAME ( \
    ::std::ostream& ostr, [[maybe_unused]] type const& x     \
  ) { return ostr << exp; }

#define RED_TEST_DELEGATE_PRINT_NS(ns, type, exp) \
  namespace ns { RED_TEST_DELEGATE_PRINT(type, exp) }


/// CHECK
//@{
#define RED_CHECK_EQ RED_CHECK_EQUAL
#define RED_CHECK_EQ_RANGES RED_CHECK_EQUAL_RANGES

#define RED_CHECK_MEM_C(mem, memref) \
    RED_CHECK_MEM(mem, cstr_array_view("" memref))
#define RED_CHECK_MEM_AC(mem, memref) \
    RED_CHECK_MEM(make_array_view(mem), cstr_array_view("" memref))
#define RED_CHECK_MEM_AA(mem, memref) \
    RED_CHECK_MEM(make_array_view(mem), make_array_view(memref))

#define RED_CHECK_SMEM_C(mem, memref) \
    RED_CHECK_SMEM(mem, cstr_array_view("" memref))
#define RED_CHECK_SMEM_AC(mem, memref) \
    RED_CHECK_SMEM(make_array_view(mem), cstr_array_view("" memref))
#define RED_CHECK_SMEM_AA(mem, memref) \
    RED_CHECK_SMEM(make_array_view(mem), make_array_view(memref))
//@}

/// REQUIRE
//@{
#define RED_REQUIRE_EQ RED_REQUIRE_EQUAL
#define RED_REQUIRE_EQ_RANGES RED_REQUIRE_EQUAL_RANGES

#define RED_REQUIRE_MEM_C(mem, memref) \
    RED_REQUIRE_MEM(mem, cstr_array_view("" memref))
#define RED_REQUIRE_MEM_AC(mem, memref) \
    RED_REQUIRE_MEM(make_array_view(mem), cstr_array_view("" memref))
#define RED_REQUIRE_MEM_AA(mem, memref) \
    RED_REQUIRE_MEM(make_array_view(mem), make_array_view(memref))

#define RED_REQUIRE_SMEM_C(mem, memref) \
    RED_REQUIRE_SMEM(mem, cstr_array_view("" memref))
#define RED_REQUIRE_SMEM_AC(mem, memref) \
    RED_REQUIRE_SMEM(make_array_view(mem), cstr_array_view("" memref))
#define RED_REQUIRE_SMEM_AA(mem, memref) \
    RED_REQUIRE_SMEM(make_array_view(mem), make_array_view(memref))
//@}
