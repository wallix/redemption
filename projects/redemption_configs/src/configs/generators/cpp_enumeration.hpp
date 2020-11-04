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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "configs/generators/utils/write_template.hpp"
#include "configs/generators/utils/multi_filename_writer.hpp"

#include "configs/enumeration.hpp"

#include <iostream>
#include <utility>


namespace cfg_generators {

namespace cpp_enumeration_writer
{

    template<class E>
    auto e_map(E & e) {
        return map(
            bind('e', e.name),
            bind('u', e.max()),
            bind('d', cpp_comment(e.desc, 0)),
            bind('i', cpp_comment(e.info, 0))
        );
    }

    struct upper_streamer
    {
        char const* cstr;
    };

    inline std::ostream& operator<<(std::ostream& out, upper_streamer const& d)
    {
        for (char const * s = d.cstr; *s; ++s) {
            char c = *s;
            out << char(('a' <= c && c <= 'z') ? c - 'a' + 'A' : c);
        }
        return out;
    }

    template<class T>
    char const* get_value_name(T const& x)
    {
        return x.alias ? x.alias : x.name;
    }

    inline void write_utility_ini(std::ostream & out, type_enumerations const & enums, bool is_hpp)
    {
        auto write = [&](auto & e, char const * fmt) {
            write_template(out, e_map(e), fmt);
        };

        auto loop = [&](auto & e, char const * fmt) {
            for (auto & v : e.values) {
                write_template(
                    out, map(
                        bind('s', v.name),
                        bind('a', get_value_name(v)),
                        bind('A', upper_streamer{get_value_name(v)}),
                        bind('d', cpp_comment(v.desc, 4))
                    ).merge(e_map(e)),
                    fmt
                );
            }
        };

        auto parse_fmt = [&](auto & e, auto lazy_integral_parse_fmt){
            if (is_hpp) {
                if (e.is_string_parser) {
                    write(e,
                        "template<>\n"
                        "struct str_buffer_size<%e>\n"
                        "{\n"
                        "    static const std::size_t value = 0;\n"
                        "};\n\n"
                    );
                }

                return;
            }

            if (e.is_string_parser) {
                out <<
                    "namespace\n"
                    "{\n"
                    "    inline constexpr zstring_view enum_zstr_" << e.name << "[] {\n";
                loop(e, "        \"%s\"_zv,\n");
                out <<
                    "    };\n"
                    "\n"
                    "    inline constexpr std::pair<chars_view, " << e.name << "> enum_str_value_" << e.name << "[] {\n";
                loop(e, "        {\"%A\"_av, %e::%s},\n");
                out <<
                    "    };\n"
                    "}\n\n";
            }

            write(e,
                "zstring_view assign_zbuf_from_cfg(\n"
                "    writable_chars_view zbuf,\n"
                "    cfg_s_type<%e> /*type*/,\n"
                "    %e x\n"
                ")"
                "{\n"
            );

            if (e.is_string_parser) {
                out <<
                    "    (void)zbuf;\n"
                    "    assert(is_valid_enum_value(x));\n"
                    "    return enum_zstr_" << e.name << "[static_cast<unsigned long>(x)];\n";
            }
            else {
                out <<
                    "    static_assert(sizeof(" << e.name << ") <= sizeof(unsigned long));\n"
                    "    int sz = snprintf(zbuf.data(), zbuf.size(), \"%lu\", static_cast<unsigned long>(x));\n"
                    "    return zstring_view(zstring_view::is_zero_terminated{}, zbuf.data(), sz);\n";
            }
            out << "}\n\n";

            write(e,
                "parse_error parse_from_cfg(%e & x, ::configs::spec_type<%e> /*type*/, bytes_view value)\n"
                "{\n"
            );

            if (e.is_string_parser) {
                out <<
                    "    return parse_str_value_pairs<enum_str_value_" << e.name << ">(\n"
                    "        x, value, \"bad value, expected: ";
                auto beg = e.values.begin();
                auto end = e.values.end();
                out << get_value_name(*beg);
                while (++beg != end) {
                    out << ", " << get_value_name(*beg);
                }
                out << "\");\n";
            }
            else {
                lazy_integral_parse_fmt();
            }

            out << "}\n\n";
        };

        out <<
            "//\n"
            "// DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN\n"
            "//\n\n"
        ;

        if (is_hpp) {
            out <<
                "#pragma once\n"
                "\n"
                "#include \"configs/zbuffer.hpp\"\n"
                "#include \"configs/autogen/enums.hpp\"\n"
                "\n"
                "\n"
                "namespace configs\n"
                "{\n"
                "\n"
            ;
        }
        else {
            out <<
                "namespace\n"
                "{\n"
                "\n"
            ;
        }

        auto write_header = [&](auto& e){
            out <<
                "    using ul = unsigned long;\n"
                "    using enum_int = std::underlying_type_t<" << e.name << ">;\n"
                "    static_assert(min_integral<ul>::value <= min_integral<enum_int>::value);\n"
                "    static_assert(max_integral<ul>::value >= max_integral<enum_int>::value);\n"
                "\n"
                "    ul xi = 0;\n"
            ;
        };
        auto write_footer = [&](auto& e) {
            out <<
                "\n"
                "    x = static_cast<" << e.name << ">(xi);\n"
                "    return no_parse_error;\n"
            ;
        };

        for (auto & e : enums.enumerations_) {
            parse_fmt(e, [&]{
                write_header(e);
                out <<
                    "    if (parse_error err = parse_integral(\n"
                    "        xi, value,\n"
                    "        zero_integral<ul>(),\n"
                    "        std::integral_constant<ul, " << e.max() << ">()\n"
                    "    )) {\n"
                    "        return err;\n"
                    "    }\n"
                ;
                write_footer(e);
            });
        }
        for (auto & e : enums.enumerations_set_) {
            parse_fmt(e, [&]{
                write_header(e);
                out <<
                    "    if (parse_error err = parse_integral(\n"
                    "        xi, value,\n"
                    "        min_integral<ul>(),\n"
                    "        max_integral<ul>()\n"
                    "    )) {\n"
                    "        return err;\n"
                    "    }\n"
                    "\n"
                    "    switch (xi) {\n";
                loop(e,
                    "        case ul(%e::%s): break;\n"
                );
                out <<
                    "        default: return parse_error{\"unknown value\"};\n"
                    "    }\n"
                ;
                write_footer(e);
            });
        }

        if (is_hpp) {
            out << "} // namespace config\n";
        }
        else {
            out << "} // anonymous namespace\n";
        }
    }

    inline void write_utility_ini_hpp(std::ostream & out, type_enumerations const & enums)
    {
        write_utility_ini(out, enums, true);
    }

    inline void write_utility_ini_cpp(std::ostream & out, type_enumerations const & enums)
    {
        write_utility_ini(out, enums, false);
    }

    inline void write_type(std::ostream & out, type_enumerations const & enums)
    {
        auto write = [&](auto & e, char const * fmt){
            write_template(out, e_map(e), fmt);
        };

        auto enum_def = [&](auto & e, auto next_value) {
            write(e,
                "%d"
                "%i"
                "enum class %e : "
            );
            auto min = e.min();
            auto max = e.max();
            out << ((0 <= min && min <= 255 && 0 <= max && max <= 255) ? "unsigned char"
                  : (0 <= min && min <= ((1 << 16)-1) && 0 <= max && max <= ((1 << 16)-1)) ? "unsigned short"
                  : "int")
                << "\n{\n";
            for (auto & v : e.values) {
                write_template(
                    out, map(
                        bind('s', v.name),
                        bind('d', cpp_comment(v.desc, 4)),
                        bind('x', next_value(v))
                    ),
                    "%d"
                    "    %s = %x,\n"
                );
            }
            out << "};\n\n";
        };

        out <<
            "//\n"
            "// DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN\n"
            "//\n\n"
            "#pragma once\n"
            "\n"
            "\n"
        ;
        for (auto const& e : enums.enumerations_) {
            if (e.flag == type_enumeration::flags) {
                enum_def(e, [d = 0] (auto &) mutable {
                    return type_enumeration::mask_of(d++);
                });
                write(e,
                    "inline bool is_valid_enum_value(%e e)\n"
                    "{\n"
                    "    return static_cast<unsigned long>(e) <= %u;\n"
                    "}\n\n"
                    "inline %e operator | (%e x, %e y)\n"
                    "{ return static_cast<%e>(static_cast<unsigned long>(x) | static_cast<unsigned long>(y)); }\n"
                    "inline %e operator & (%e x, %e y)\n"
                    "{ return static_cast<%e>(static_cast<unsigned long>(x) & static_cast<unsigned long>(y)); }\n"
                    "inline %e operator ~ (%e x)\n"
                    "{ return static_cast<%e>(~static_cast<unsigned long>(x) & static_cast<unsigned long>(%u)); }\n"
                    "inline %e operator + (%e & x, %e y) { return x | y; }\n"
                    "inline %e operator - (%e & x, %e y) { return x & ~y; }\n"
                    "inline %e & operator |= (%e & x, %e y) { return x = x | y; }\n"
                    "inline %e & operator &= (%e & x, %e y) { return x = x & y; }\n"
                    "inline %e & operator += (%e & x, %e y) { return x = x + y; }\n"
                    "inline %e & operator -= (%e & x, %e y) { return x = x - y; }\n\n"
                );
            }
            else {
                enum_def(e, [i = 0] (auto &) mutable { return i++; });
                write(e,
                    "inline bool is_valid_enum_value(%e e)\n"
                    "{ return static_cast<unsigned long>(e) <= %u; }\n\n"
                );
            }
        }

        for (auto const& e : enums.enumerations_set_) {
            enum_def(e, [] (auto & v) mutable { return v.val; });
            write(e,
                "inline bool is_valid_enum_value(%e e)\n"
                "{\n"
                "    auto const i = static_cast<unsigned long>(e);\n"
                "    return false /* NOLINT(readability-simplify-boolean-expr) */\n"
            );
            for (auto & v : e.values) {
                out << "     || i == " << v.val << "\n";
            }
            write(e,
                "    ;\n"
                "}\n\n"
            );
        }
    }
}

template<class Writer>
int app_write_cpp_enumeration(Writer && writer, char const * progname)
{
    type_enumerations e;
    writer(e);
    MultiFilenameWriter<type_enumerations> sw(e);
    sw.then("autogen/include/configs/autogen/enums.hpp",
            &cpp_enumeration_writer::write_type)
      .then("autogen/include/configs/autogen/enums_func_ini.hpp",
            &cpp_enumeration_writer::write_utility_ini_hpp)
      .then("autogen/include/configs/autogen/enums_func_ini.tcc",
            &cpp_enumeration_writer::write_utility_ini_cpp)
    ;
    if (sw.err) {
        std::cerr << progname << ": " << sw.filename << ": " << (errno ? strerror(errno) : "unknown error") << "\n";
        return sw.errnum;
    }
    return 0;
}

}
