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

#ifndef PPOCR_SRC_SPELL_WORD_DISAMBIGUOUSER_HPP
#define PPOCR_SRC_SPELL_WORD_DISAMBIGUOUSER_HPP

#include "ppocr/spell/dictionary.hpp"
#include "ppocr/utils/utf.hpp"

#include <algorithm>


namespace ppocr { namespace spell {

struct WordDisambiguouser : Dictionary::Manipulator
{
    template<class FwRngStrIt>
    bool operator()(Dictionary const & dict, FwRngStrIt first, FwRngStrIt last, std::string & output)
    {
        if (first == last || this->trie(dict).all().empty()) {
            return false;
        }
        auto sz = output.size();
        if (disambiguous_impl(this->trie(dict).childrens(), first, last, output)) {
            std::swap_ranges(output.begin() + sz, output.begin() + sz + (output.size() - sz) / 2, output.rbegin());
            return true;
        }
        return false;
    }

private:
    template<class FwRngStrIt>
    bool disambiguous_impl(trie_type::range rng, FwRngStrIt first, FwRngStrIt last, std::string & output) {
        for (auto & s : *first) {
            if (disambiguous_utf_char(rng, first, last, utf::UTF8Iterator(s.data()), output)) {
                return true;
            }
        }
        return false;
    }

    template<class FwRngStrIt>
    bool disambiguous_utf_char(trie_type::range rng, FwRngStrIt first, FwRngStrIt last, utf::UTF8Iterator it, std::string & output) {
        auto const c = *it;
        auto pos = rng.lower_bound(c);
        if (pos != rng.end() && pos->get() == c) {
            auto next_first = first;
            ++next_first;
            if (next_first == last) {
                if (pos->is_terminal()) {
                    set_c(output, c);
                    return true;
                }
            }
            else if (!pos->empty()) {
                if (*++it) {
                    if (disambiguous_utf_char(pos->childrens(), first, last, it, output)) {
                        set_c(output, c);
                        return true;
                    }
                }
                else if (disambiguous_impl(pos->childrens(), next_first, last, output)) {
                    set_c(output, c);
                    return true;
                }
            }
        }
        return false;
    }

    void set_c(std::string & s, uint32_t code) {
        char c[] = {
            char((code & 0x000000FF)),
            char((code & 0x0000FF00) >> 8),
            char((code & 0x00FF0000) >> 16),
            char((code & 0xFF000000) >> 24),
            '\0'
        };
        s += c;
    }
};

} }

#endif
