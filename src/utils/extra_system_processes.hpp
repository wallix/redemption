/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#pragma once

#include "utils/sugar/make_unique.hpp"

#include <string>
#include <vector>

#include <cstring>


class ExtraSystemProcesses
{
    std::vector<std::string> processes;

public:
    ExtraSystemProcesses(const char * comme_separated_processes) {
        if (comme_separated_processes) {
            const char * process = comme_separated_processes;

            while (*process) {
                if ((*process == ',') || (*process == '\t') || (*process == ' ')) {
                    process++;
                    continue;
                }

                char const * process_begin = process;

                const char * process_separator = strchr(process, ',');

                std::string name_string(process_begin, (process_separator ? process_separator - process_begin : ::strlen(process_begin)));

                this->processes.push_back(std::move(name_string));

                if (!process_separator) {
                    break;
                }

                process = process_separator + 1;
            }
        }
    }

    bool get(
        size_t index,
        std::string & out__name
    ) {
        if (this->processes.size() <= index) {
            out__name.clear();

            return false;
        }

        out__name = this->processes[index];

        return true;
    }
};
