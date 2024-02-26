/*
* Copyright (C) 2016 Wallix
* 
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
* 
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
* 
* You should have received a copy of the GNU Lesser General Public License along
* with this library; if not, write to the Free Software Foundation, Inc., 59
* Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "ppocr/read_definitions_file_and_normalize.hpp"

#include <stdexcept>
#include <algorithm>
#include <ostream>
#include <fstream>

#include <cstring>
#include <cerrno>

namespace ppocr {

std::vector<Definition> read_definitions_file_and_normalize(
    const char* filename, DataLoader& loader, std::ostream * out
) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error(strerror(errno));
    }

    std::vector<Definition> definitions = read_definitions(file, loader);

    if (!file.eof()) {
        throw std::runtime_error("read error");
    }

    if (out) {
        *out << "definitions.size = " << definitions.size() << "\n\n";
    }

    std::sort(
        definitions.begin(), definitions.end(),
        [](Definition const & lhs, Definition const & rhs) {
            return lhs.datas < rhs.datas;
        }
    );
    definitions.erase(
        std::unique(
            definitions.begin(), definitions.end(),
            [](Definition const & lhs, Definition const & rhs) {
                return lhs.datas == rhs.datas;
            }
        ),
        definitions.end()
    );

    if (out) {
        *out << "unique definitions.size = " << definitions.size() << "\n\n";
    }

    return definitions;
}

}
