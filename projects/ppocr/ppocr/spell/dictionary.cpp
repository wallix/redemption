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

#include "ppocr//spell/dictionary.hpp"
#include "ppocr/utils/utf.hpp"

#include <fstream>
#include <algorithm>


namespace ppocr { namespace spell {

Dictionary::Dictionary(container::trie<uint32_t> const & trie)
: trie_(trie)
{}

Dictionary::Dictionary(container::flat_trie<uint32_t> trie)
: trie_(std::move(trie))
{}

Dictionary::Dictionary(std::vector<std::string> const & words)
{
    std::vector<std::vector<uint32_t>> uwords;
    for (auto & old_word : words) {
        std::vector<uint32_t> new_word;
        utf::UTF8Iterator it(old_word.data());
        for (uint32_t c; (c = *it); ++it) {
            new_word.push_back(c);
        }
        uwords.push_back(std::move(new_word));
    }

    std::sort(uwords.begin(), uwords.end());
    uwords.erase(std::unique(uwords.begin(), uwords.end()), uwords.end());
    this->trie_ = container::trie<uint32_t>(uwords.begin(), uwords.end());
}

namespace {
    struct IODictionary : Dictionary::Manipulator
    { using Dictionary::Manipulator::trie; };
}

std::ostream& operator<<(std::ostream& os, Dictionary const & dict)
{
    for (auto node : IODictionary().trie(dict).all()) {
        os
          << node.get() << " "
          << node.relative_pos() << " "
          << node.size() << " "
          << node.is_terminal()
          << "\n";
    }
    return os;
}

std::istream& operator>>(std::istream& is, Dictionary & dict)
{
    struct Saver {
        std::ios & io_;
        std::ios::fmtflags flags_;

        Saver(std::ios & io)
        : io_(io), flags_(io.flags())
        { io.unsetf(std::ios::skipws); }

        ~Saver()
        { this->io_.flags(flags_); }
    } saver{is};

    using trie_type = container::flat_trie<uint32_t>;
    std::vector<trie_type::node_type> nodes;
    trie_type::value_type c;
    unsigned pos;
    unsigned sz;
    bool terminal;
    char e1, e2, e3, e4;
    while (is >> c >> e1 >> pos >> e2 >> sz >> e3 >> terminal >> e4) {
        if (e1 != ' ' || e2 != ' ' || e3 != ' ' || e4 != '\n') {
            throw std::runtime_error("bad format");
        }
        nodes.emplace_back(c, pos, sz, terminal);
    }
    dict = Dictionary(std::move(nodes));
    return is;
}

} }
