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
#include "configs/generators/cpp_config.hpp"
#include "configs/generators/python_spec.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

#include <cerrno>
#include <cstring>


namespace cfg_generators
{

namespace sesman_dialog_writer
{

using namespace cfg_attributes;

template<unsigned n>
void write_type(std::ostream& out, type_<types::fixed_string<n>>, int)
{
    out << "std::string(maxlen=" << n << ")";
}

template<class T>
void write_type(std::ostream& out, type_<T> type, char)
{
    cpp_config_writer::write_type(out, type);
}


class SesmanDialogWriterBase
{
    std::ofstream out;
    int errnum = 0;

public:
    SesmanDialogWriterBase(std::string const& filename)
    : out(filename)
    {
        if (!out) {
            errnum = errno;
            std::cerr << "SesmanDialogWriterBase: " << filename << ": " << strerror(errnum) << "\n";
        }

        out << do_not_edit << "\n"
            "       cpp name       |       sesman / passthrough name\n\n"
        ;
    }

    void do_init()
    {}

    int do_finish()
    {
        if (!errnum) {
            out.flush();
            if (!out) {
                if (errno) {
                    errnum = errno;
                }
            }
        }
        return errnum;
    }

    void do_start_section(Names const& /*names*/)
    {}

    void do_stop_section(Names const& /*names*/)
    {
        this->out << "\n";
    }

    template<class Pack>
    void evaluate_member(Names const& section_names, Pack const & infos, type_enumerations& enums)
    {
        if constexpr (is_convertible_v<Pack, sesman_io_t>) {
            using sesman_io = sesman::internal::io;
            auto const properties = sesman_io_t(infos).value;
            auto cpp_type = get_t_elem<cfg_attributes::type_>(infos);
            auto sesman_type = get_type<spec::type_>(infos);

            bool is_enum_parser = false;
            auto semantic_type = python_spec_writer::get_semantic_type(sesman_type, cpp_type, &is_enum_parser);

            char const* dialog = " ⇔ ";

            if ((properties & sesman_io::rw) == sesman_io::sesman_to_proxy) {
                dialog = " ⇐ ";
            }
            else if ((properties & sesman_io::rw) == sesman_io::proxy_to_sesman) {
                dialog = " ⇒ ";
            }

            this->out
                << "cfg::" << section_names.sesman_name() << "::"
                << static_cast<Names const&>(infos).cpp
                << dialog
                << sesman_network_name(infos, section_names)
                << "   ["
            ;
            write_type(this->out, cpp_type, 1);

            if constexpr (!std::is_same_v<decltype(cpp_type), decltype(sesman_type)>) {
                this->out << dialog;
                write_type(this->out, sesman_type, 1);
            }

            this->out << "]\n";

            std::stringstream comments;

            python_spec_writer::write_description(comments, enums, semantic_type, get_desc(infos));
            python_spec_writer::write_enumeration_value_description(comments, enums, semantic_type, infos, is_enum_parser);

            this->out << io_prefix_lines{comments.str().c_str(), "    ", "", 0};
        }
    }
};

}

}
