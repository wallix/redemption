/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "configs/attributes/spec.hpp"
#include "configs/generators/utils/spec_writer.hpp"
#include "configs/generators/utils/write_template.hpp"
#include "configs/enumeration.hpp"

#include "configs/generators/python_spec.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <chrono>
#include <locale>
#include <vector>
#include <unordered_map>

#include <cerrno>
#include <cstring>


namespace cfg_generators {

namespace ini_writer {

using namespace cfg_attributes;

template<class Inherit>
struct IniWriterBase : python_spec_writer::PythonSpecWriterBase<Inherit>
{
    using base_type = IniWriterBase;

    using base_type_ = typename python_spec_writer::PythonSpecWriterBase<Inherit>::base_type;

    using base_type_::base_type_;
    using base_type_::write_type_info;

    template<class Pack>
    void do_member(
        std::string const & section_name,
        std::string const & member_name,
        Pack const & infos
    ) {
        // comments variable
        base_type_::do_member(section_name, '#' + member_name, infos);
    }

    template<class... Ts>
    void write_comment_line(Ts const & ... args)
    {
        this->out() << "\"# ";
        (void)std::initializer_list<int>{(void(this->out() << args), 0)...};
        this->out() << "\\n\"\n";
    }

    template<class T>
    void write_type_info(type_<bool>, T)
    { this->write_comment_line("0 or 1"); }

    template<class Int, class T>
    std::enable_if_t<
        std::is_base_of<types::integer_base, Int>::value
        or
        std::is_integral<Int>::value
    >
    write_type_info(type_<Int>, T)
    {
        if (std::is_unsigned<Int>::value || std::is_base_of<types::unsigned_base, Int>::value) {
            this->write_comment_line("min = 0");
        }
    }

    template<class Int, long min, long max, class T>
    void write_type_info(type_<types::range<Int, min, max>>, T)
    { this->write_comment_line("min = ", min, ", max = ", max); }


    template<class T, class Ratio, class U>
    void write_type_info(type_<std::chrono::duration<T, Ratio>>, U)
    { this->write_comment_line("min = 0"); }

    template<unsigned N, class T>
    void write_type_info(type_<types::fixed_binary<N>>, T const &)
    { this->write_comment_line("(hexadecimal string) size = ", N*2); }

    template<unsigned N, class T>
    void write_type_info(type_<types::fixed_string<N>>, T const &)
    {this->write_comment_line("size_max = ", N); }

    template<class T>
    void write_type_info(type_<types::dirpath>, T const & x)
    { this->write_type_info(type_<typename types::dirpath::fixed_type>{}, x); }

    template<class T, class E>
    enable_if_enum_t<T>
    write_type_info(type_<T>, E const & x)
    {
        static_assert(std::is_same<T, E>::value, "");
        apply_enumeration_for<T>(this->enums, [&x, this](auto const & e) {
            this->write_enum_info(e, static_cast<std::underlying_type_t<E>>(x));
        });
    }

    template<class T>
    void write_enum_info(type_enumeration const & e, T /*default_value*/)
    {
        if (e.flag == type_enumeration::flags) {
            this->write_comment_line("(flags) min = 0, max = ", e.max());
        }
        else if (e.is_icase_parser) {
            this->out() << "\"# ";
            for (type_enumeration::Value const & v : e.values) {
                this->out() << "'" << (v.alias ? v.alias : v.name) << "', ";
            }
            this->out() << "\\n\"\n";
        }
        else {
            this->write_comment_line("min = 0, max = ", e.count());
        }
    }

    template<class T>
    void write_enum_info(type_enumeration_set const & e, T /*default_value*/)
    {
        this->out() << "\"# ";
        for (type_enumeration_set::Value const & v : e.values) {
            this->out() << v.val << ", ";
        }
        this->out() << "\\n\"\n";
    }


    static std::string stringize_bool(bool x)
    {
        return bool(x) ? "1" : "0";
    }

    static std::string stringize_bool(cpp::macro x)
    {
        return base_type_::stringize_integral(x);
    }


    template<class T>
    void write_type(type_<bool>, T x)
    { this->out() << stringize_bool(x); }

    template<class T>
    void write_type(type_<std::string>, T const & s)
    { this->out() << this->quoted2(s); }

    template<class Int, class T>
    std::enable_if_t<
        std::is_base_of<types::integer_base, Int>::value
        or
        std::is_integral<Int>::value
    >
    write_type(type_<Int>, T i)
    { this->out() << this->stringize_integral(i); }

    template<class Int, long min, long max, class T>
    void write_type(type_<types::range<Int, min, max>>, T i)
    { this->out() << this->stringize_integral(i); }


    template<class T, class Ratio, class U>
    void write_type(type_<std::chrono::duration<T, Ratio>>, U i)
    { this->out() << this->stringize_integral(i); }

    template<unsigned N, class T>
    void write_type(type_<types::fixed_binary<N>>, T const & x)
    { this->out() << io_hexkey{this->get_string(x), N}; }

    template<unsigned N, class T>
    void write_type(type_<types::fixed_string<N>>, T const & x)
    { this->out() << this->quoted2(x); }

    template<class T>
    void write_type(type_<types::dirpath>, T const & x)
    { this->write_type(type_<typename types::dirpath::fixed_type>{}, x); }

    template<class T>
    void write_type(type_<types::ip_string>, T const & x)
    { this->out() << this->quoted2(x); }

    template<class T, class L>
    void write_type(type_<types::list<T>>, L const & s)
    {
        if (!is_empty(s)) {
            this->out() << this->quoted2(s);
        }
    }

    template<class T, class E>
    enable_if_enum_t<T>
    write_type(type_<T>, E const & x)
    {
        static_assert(std::is_same<T, E>::value, "");
        apply_enumeration_for<T>(this->enums, [&x, this](auto const & e) {
            this->write_enum_value(e, static_cast<std::underlying_type_t<E>>(x));
        });
    }

    template<class T>
    void write_enum_value(type_enumeration const & e, T default_value)
    {
        if (e.flag == type_enumeration::flags) {
            this->out() << default_value;
        }
        else if (e.is_icase_parser) {
            this->out() << e.values[default_value].name;
        }
        else {
            this->out() << default_value;
        }
    }

    template<class T>
    void write_enum_value(type_enumeration_set const &, T default_value)
    { this->out() << default_value; }
};

}


template<class SpecWriter>
int app_write_ini(int ac, char const * const * av)
{
    if (ac < 2) {
        std::cerr << av[0] << " out-spec.h\n";
        return 1;
    }

    SpecWriter writer(av[1]);
    writer.evaluate();

    if (!writer.out_file_) {
        std::cerr << av[0] << ": " << av[1] << ": " << strerror(errno) << "\n";
        return 1;
    }
    return 0;
}

}
