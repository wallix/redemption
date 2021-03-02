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
    inline std::string_view underlying_type_str(unsigned long long max)
    {
        using namespace std::string_view_literals;

        return
            0xFFull >= max ? "uint8_t"sv
          : 0xFFFFull >= max ? "uint16_t"sv
          : 0xFFFFFFFFull >= max ? "uint32_t"sv
          : "uint64_t"sv
          ;
    }

    inline void write_utility_ini_cpp(std::ostream & out, type_enumerations const & enums)
    {
        out << cpp_comment(do_not_edit, 0)
            << "\n"
            "namespace\n"
            "{\n"
            "\n"
        ;

        for (type_enumeration const& e : enums.enumerations_) {
            const auto max = e.max();
            const auto type = underlying_type_str(max);

            switch (e.cat) {
            case type_enumeration::Category::flags:
                out <<
                    "zstring_view assign_zbuf_from_cfg(\n"
                    "    writable_chars_view zbuf,\n"
                    "    cfg_s_type<" << e.name << "> /*type*/,\n"
                    "    " << e.name << " x\n"
                    ")"
                    "{\n"
                    "    auto r = std::to_chars(zbuf.begin(), zbuf.end(), " << type << "(x));\n"
                    "    *r.ptr = '\\0';\n"
                    "    return zstring_view(zstring_view::is_zero_terminated{}, {zbuf.data(), r.ptr});\n"
                    "}\n\n"
                ;
                break;

            case type_enumeration::Category::set:
            case type_enumeration::Category::autoincrement:
                auto append_assign_impl = [&](type_enumeration const& e, char const* prefix){
                    if (e.cat == type_enumeration::Category::autoincrement) {
                        out <<
                            "    assert(is_valid_enum_value<" << e.name << ">::is_valid(" << type << "(x)));\n"
                            "    return " << prefix << e.name << "[" << type << "(x)];\n"
                        ;
                    }
                    else {
                        int i = 0;
                        out <<
                            "    switch (x) {\n"
                        ;
                        for (auto & v : e.values) {
                            out << "        case " << e.name << "::" << v.name
                                << ": return " << prefix << e.name << "[" << i << "];\n";
                            ++i;
                        }
                        out <<
                            "    }\n"
                            "    assert(!\"unknown value\");\n"
                            "    return \"\"_zv;\n"
                        ;
                    }
                };

                out <<
                    "inline constexpr zstring_view enum_zstr_" << e.name << "[] {\n"
                ;
                for (auto & v : e.values) {
                    out << "    \"" << v.get_name() << "\"_zv,\n";
                }
                out <<
                    "};\n"
                    "\n"
                    "inline constexpr zstring_view enum_zint_" << e.name << "[] {\n"
                ;
                for (auto & v : e.values) {
                    out << "    \"" << v.val << "\"_zv,\n";
                }
                out <<
                    "};\n"
                    "\n"
                    "zstring_view assign_zbuf_from_cfg(\n"
                    "    writable_chars_view zbuf,\n"
                    "    cfg_s_type<" << e.name << "> /*type*/,\n"
                    "    " << e.name << " x\n"
                    ")"
                    "{\n"
                    "    (void)zbuf;\n"
                ;
                append_assign_impl(e, "enum_zint_");
                out <<
                    "}\n"
                    "\n"
                    "zstring_view assign_zbuf_from_cfg(\n"
                    "    writable_chars_view zbuf,\n"
                    "    cfg_s_type<std::string> /*type*/,\n"
                    "    " << e.name << " x\n"
                    ")"
                    "{\n"
                    "    (void)zbuf;\n"
                ;
                append_assign_impl(e, "enum_zstr_");
                out <<
                    "}\n\n"
                ;
            }

            out <<
                "parse_error parse_from_cfg(" << e.name << " & x, ::configs::spec_type<" << e.name << "> /*type*/, bytes_view value)\n"
                "{\n"
                "    using ul = " << type << ";\n"
                "\n"
                "    ul xi = 0;\n"
            ;

            switch (e.cat) {
            case type_enumeration::Category::flags:
            case type_enumeration::Category::autoincrement:
                out <<
                    "    if (parse_error err = parse_integral(\n"
                    "        xi, value,\n"
                    "        zero_integral<ul>(),\n"
                    "        std::integral_constant<ul, " << max << ">()\n"
                    "    )) {\n"
                    "        return err;\n"
                    "    }\n"
                ;
                break;

            case type_enumeration::Category::set:
                out <<
                    "    if (parse_error err = parse_integral(\n"
                    "        xi, value,\n"
                    "        min_integral<ul>(),\n"
                    "        max_integral<ul>()\n"
                    "    )) {\n"
                    "        return err;\n"
                    "    }\n"
                    "\n"
                    "    switch (xi) {\n"
                ;
                for (auto & v : e.values) {
                    out << "        case " << type << "(" << e.name << "::" << v.name << "): break;\n";
                }
                out <<
                    "        default: return parse_error{\"unknown value\"};\n"
                    "    }\n"
                ;
                break;
            }

            out <<
                "\n"
                "    x = static_cast<" << e.name << ">(xi);\n"
                "    return no_parse_error;\n"
                "}\n"
                "\n"
            ;

            switch (e.cat) {
            case type_enumeration::Category::flags:
                break;

            case type_enumeration::Category::autoincrement:
            case type_enumeration::Category::set:
                out <<
                    "\n"
                    "inline constexpr std::pair<chars_view, " << e.name << "> enum_str_value_" << e.name << "[] {\n"
                ;
                for (auto & v : e.values) {
                    out << "    {\"" << io_upper{v.get_name()}
                        << "\"_av, " << e.name << "::" << v.name << "},\n";
                }
                out <<
                    "};\n"
                    "\n"
                    "parse_error parse_from_cfg(" << e.name << " & x, ::configs::spec_type<std::string> /*type*/, bytes_view value)\n"
                    "{\n"
                    "    return parse_str_value_pairs<enum_str_value_" << e.name << ">(\n"
                    "        x, value, \"bad value, expected: "
                ;

                auto beg = e.values.begin();
                auto end = e.values.end();
                out << beg->get_name();
                while (++beg != end) {
                    out << ", " << beg->get_name();
                }
                out <<
                    "\");\n"
                    "}\n\n"
                ;
                break;
            }
        }

        out << "} // anonymous namespace\n";
    }

    inline void write_type(std::ostream & out, type_enumerations const & enums)
    {
        out << cpp_comment(do_not_edit, 0)
            << "\n"
            "#pragma once\n"
            "\n"
            "#include <cstdint>\n"
            "\n"
            "\n"
            "template<class E> struct is_valid_enum_value {};\n"
            "\n"
        ;

        for (auto const& e : enums.enumerations_) {
            const auto max = e.max();
            const auto type = underlying_type_str(max);

            out << cpp_comment(e.desc, 0)
                << cpp_comment(e.info, 0)
                << "enum class " << e.name << " : " << type << "\n"
                "{\n";

            for (auto & v : e.values) {
                out << cpp_comment(v.desc, 4)
                    << "    " << v.name << " = " << v.val << ",\n"
                ;
            }

            out << "};\n\n";

            switch (e.cat) {
            case type_enumeration::Category::flags:
                out <<
                    "template<> struct is_valid_enum_value<" << e.name << ">\n"
                    "{\n"
                    "    constexpr static bool is_valid(uint64_t n) { return n <= " << max << "; }\n"
                    "};\n\n"
                    "inline " << e.name << " operator | (" << e.name << " x, " << e.name << " y)\n"
                    "{ return " << e.name << "(" << type << "(x) | " << type << "(y)); }\n"
                    "inline " << e.name << " operator & (" << e.name << " x, " << e.name << " y)\n"
                    "{ return " << e.name << "(" << type << "(x) & " << type << "(y)); }\n"
                    "inline " << e.name << " operator ~ (" << e.name << " x)\n"
                    "{ return " << e.name << "(~" << type << "(x) & " << type << "(" << max << ")); }\n"
                    "// inline " << e.name << " operator + (" << e.name << " x, " << e.name << " y) { return x | y; }\n"
                    "// inline " << e.name << " operator - (" << e.name << " x, " << e.name << " y) { return x & ~y; }\n"
                    "inline " << e.name << " & operator |= (" << e.name << " & x, " << e.name << " y) { return x = x | y; }\n"
                    "inline " << e.name << " & operator &= (" << e.name << " & x, " << e.name << " y) { return x = x & y; }\n"
                    "// inline " << e.name << " & operator += (" << e.name << " & x, " << e.name << " y) { return x = x + y; }\n"
                    "// inline " << e.name << " & operator -= (" << e.name << " & x, " << e.name << " y) { return x = x - y; }\n"
                    "\n"
                ;
                break;

            case type_enumeration::Category::autoincrement:
                out <<
                    "template<> struct is_valid_enum_value<" << e.name << ">\n"
                    "{\n"
                    "    constexpr static bool is_valid(uint64_t n) { return n <= " << max << "; }\n"
                    "};\n\n"
                ;
                break;

            case type_enumeration::Category::set:
                out <<
                    "template<> struct is_valid_enum_value<" << e.name << ">\n"
                    "{\n"
                    "    constexpr static bool is_valid(uint64_t n)\n"
                    "    {\n"
                    "        switch (n) {\n"
                ;
                for (auto & v : e.values) {
                    out << "        case uint64_t(" << v.val << "):\n";
                }
                out <<
                    "            return true;\n"
                    "        }\n"
                    "        return false;\n"
                    "    }\n\n"
                    "};\n\n"
                ;
                break;
            }
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
