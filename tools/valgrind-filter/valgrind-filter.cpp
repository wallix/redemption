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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan,
 *              Loïc Michaux
 */

#include <iostream>
#include <vector>
#include <string>

struct cstring {
    char * s;
    std::size_t len;
};

int main(int ac, char ** av)
{
    if (ac == 1) {
        std::cerr << "usage: valgrind ... |& " << av[0] << " filter_names...";
        return 1;
    }

    std::vector<cstring> names;
    names.resize(ac-1);
    {
        auto it = names.begin();
        for (int i = 1; i < ac; ++i) {
            it->s = av[i];
            it->len = std::char_traits<char>::length(av[i]);
            ++it;
        }
    }

    std::string pidinfo;
    std::string line;
    std::string block;
    while (std::cin >> pidinfo && std::getline(std::cin, line)) {
        if (line.empty() || (line.size() == 1 && line[0] == ' ')) {
            [&](){
                for (cstring & cs: names) {
                    if (block.find(cs.s, 0, cs.len) != std::string::npos) {
                        return;
                    }
                }
                std::cout << block << pidinfo << line << std::endl;
            }();
            block.clear();
        }
        else {
            block += pidinfo;
            block += line;
            block += '\n';
        }
    }
    std::cout << pidinfo << block;

    return 0;
}
