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

#ifndef PPOCR_SRC_SPELL_DICTIONARY_HPP
#define PPOCR_SRC_SPELL_DICTIONARY_HPP

#include "ppocr/container/trie.hpp"

#include <iosfwd>

namespace ppocr { namespace spell {

struct Dictionary
{
    class Manipulator {
    protected:
        using trie_type = container::flat_trie<uint32_t>;

        trie_type const & trie(Dictionary const & dict) const
        { return dict.trie_; }
    };

    Dictionary(std::vector<std::string> const & words);
    Dictionary(container::flat_trie<uint32_t> trie);
    Dictionary(container::trie<uint32_t> const & trie);
    Dictionary() = default;

    bool empty() const { return this->trie_.all().empty(); }

private:
    container::flat_trie<uint32_t> trie_;
};

std::istream & operator >> (std::istream & is, Dictionary & dict);
std::ostream & operator << (std::ostream & os, Dictionary const & dict);

} }

#endif
