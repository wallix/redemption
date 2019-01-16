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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean
*/

#include "redemption_unit_tests_impl.hpp"
#include "register_exception.hpp"

#include <vector>
#include <iostream>
#include <stdexcept>


namespace redemption_unit_test__
{
namespace emscripten
{
    std::ostream& operator<<(std::ostream& out, PrintableChar const& x)
    {
        auto const c = x.c;

        if (31 < c && c < 127) {
            out << c;
        }
        else {
            char const* h = "0123456789abcdef";
            char s[] = "'\\x  '";
            s[3] = h[c>>4];
            s[4] = h[c&0xf];
            out << s;
        }
        return out;
    }

    PrintableChar ostream_wrap(char x) noexcept
    {
        return PrintableChar{static_cast<unsigned char>(x)};
    }

    PrintableChar ostream_wrap(unsigned char x) noexcept
    {
        return PrintableChar{x};
    }

    PrintableChar ostream_wrap(signed char x) noexcept
    {
        return PrintableChar{static_cast<unsigned char>(x)};
    }

    namespace
    {
        struct item
        {
            const char* file;
            const char* name;
            void (*fn)();
            int line;
        };

        struct UnitTest
        {
            std::vector<item> tests;
            std::vector<exception_translator_fn> exception_translators;
            char const* point_file = nullptr;
            char const* point_name = nullptr;
            int point_line = 0;
            int failure = 0;
            const char * current_name;
        };

        // abort static initialization order fiasco
        UnitTest& TESTS() noexcept
        {
            static UnitTest r;
            return r;
        }
    }

    void passpoint(char const* filename, int line) noexcept
    {
        TESTS().point_file = filename;
        TESTS().point_line = line;
        TESTS().point_name = nullptr;
    }

    void checkpoint(char const* filename, int line, char const* name) noexcept
    {
        TESTS().point_file = filename;
        TESTS().point_line = line;
        TESTS().point_name = name;
    }

    void failure() noexcept
    {
        ++TESTS().failure;
    }

    char const* current_name() noexcept
    {
        return TESTS().current_name;
    }

    std::ostream& get_output() noexcept
    {
        return std::cerr;
    }

    void add_test_case(char const* filename, int line, char const* name, void(*fn)())
    {
        TESTS().tests.push_back({filename, name, fn, line});
    }

    void add_register_exception_translator(exception_translator_fn f)
    {
        TESTS().exception_translators.push_back(std::move(f));
    }

    static void print_current_exception()
    {
        #define CATCH(Exception) catch (Exception const& e) { \
            get_output() << #Exception ": " << e.what();      \
        }

        try {
            throw;
        }
        // https://en.cppreference.com/w/cpp/header/stdexcept
        CATCH(std::underflow_error)
        CATCH(std::overflow_error)
        CATCH(std::range_error)
        CATCH(std::runtime_error)
        CATCH(std::out_of_range)
        CATCH(std::length_error)
        CATCH(std::invalid_argument)
        CATCH(std::domain_error)
        CATCH(std::logic_error)
        catch (...) {
            try {
                for (auto const& fn : TESTS().exception_translators) {
                    fn();
                }
            }
            catch (...) {
                print_current_exception();
                return ;
            }
            get_output() << "unknown type";
        }

        #undef CATCH
    }

    int execute_tests(char const* module_name)
    {
        get_output() << "Running " << TESTS().tests.size() << " test cases...\n";

        auto first = TESTS().tests.begin();
        auto last = TESTS().tests.end();

        while (first != last) {
            try {
                do {
                    TESTS().current_name = first->name;
                    TESTS().point_file = first->file;
                    TESTS().point_line = first->line;
                    first->fn();
                    ++first;
                } while (first != last);
            }
            catch (...)
            {
                get_output() << "fatal error: in \"" << first->name << "\": ";
                print_current_exception();
                get_output() << "\n" << TESTS().point_file << "("
                             << TESTS().point_line << "): last checkpoint\n";
                ++TESTS().failure;
                ++first;
            }
        }

        if (TESTS().failure) {
            get_output()
              << "\n***" << TESTS().failure
              << " failure is detected in test module \"" << module_name << "\""
            ;
            return 1;
        }

        get_output() << "\n*** No errors detected\n";
        return 0;
    }
} // namespace emscripten
} // namespace redemption_unit_test__
