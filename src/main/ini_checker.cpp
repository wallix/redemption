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


#include "configs/config.hpp"
#include "utils/cfgloader.hpp"

#include <iostream>
#include <fstream>

#include <cerrno>
#include <cstring>

int main(int ac, char ** av)
{
    if (ac > 2 || (ac == 2 && (!strcmp(av[1], "-h") || !strcmp(av[1], "--help") || !strcmp(av[1], "-?")))) {
        std::cerr << "Usage: " << av[0] << " [inifile = " << app_path(AppPath::CfgIni) << "]" << std::endl;
        return 1;
    }

    char const* filename = ac == 2 ? av[1] : app_path(AppPath::CfgIni);
    std::cout << "filename: " << filename << "\n";

    if (std::ifstream inifile{filename}) {
        bool const is_ok = configuration_load(Inifile().configuration_holder(), inifile);
        if (!inifile.eof()) {
            std::cerr << av[1] << ": " << strerror(errno) << std::endl;
            return 3;
        }
        if (!is_ok) {
            return 3;
        }
    }
    else {
        std::cerr << av[1] << ": " << strerror(errno) << std::endl;
        return 2;
    }
}
