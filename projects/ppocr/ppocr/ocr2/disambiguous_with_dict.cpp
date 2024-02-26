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

#include "ppocr/ocr2/disambiguous_with_dict.hpp"
#include "ppocr/ocr2/glyphs.hpp"

#include "ppocr/spell/word_disambiguouser.hpp"
#include "ppocr/spell/dictionary.hpp"

#include <cassert>
#include <iostream>


namespace {
    using namespace ppocr::ocr2;

    inline void append(std::string & result, Glyphs const & glyphs, view_ref const & v) {
        auto & s = glyphs.get_word(v);
        result.append(s.begin(), s.size());
    }

    inline void append(std::string & result, Glyphs const & glyphs, view_ref_list const & l) {
        if (!l.empty()) {
            append(result, glyphs, l.front());
        }
    }
}

unsigned ppocr::ocr2::disambiguous_with_dict(
    ambiguous_t & ambiguous,
    Glyphs const & glyphs,
    spell::Dictionary const & dict,
    std::vector<unsigned>::const_iterator it_space,
    std::string & result
) {
    unsigned unrecognized_count = 0;

    result.clear();
    auto search_fn = [&](ppocr::ocr2::view_ref_list const & vlist) -> bool {
        if (vlist.empty()) {
            return true;
        }
        auto & s = glyphs.get_word(vlist.front());
        return (s.size() == 1
            // TODO punct
            && (s.front() == '!'

            || s.front() == '#'
            || s.front() == '$'
            || s.front() == '%'
            || s.front() == '&'
            || s.front() == '\''
            || s.front() == '('
            || s.front() == ')'
            || s.front() == '*'
            || s.front() == '+'
            || s.front() == ','
            || s.front() == '-'
            || s.front() == '.'

            || s.front() == ':'
            || s.front() == ';'
            || s.front() == '<'
            || s.front() == '='
            || s.front() == '>'
            || s.front() == '?'

            || s.front() == '['
            || s.front() == ']'

            || s.front() == '{'
            || s.front() == '}'
            )
        );
    };
    ppocr::spell::WordDisambiguouser word_disambiguouser;

    using ambiguous_iterator_base = decltype(ambiguous.begin());
    struct ambiguous_view_ref_list_iterator : ambiguous_iterator_base
    {
        using Glyphs = ppocr::ocr2::Glyphs;

        Glyphs const & glyphs;

        ambiguous_view_ref_list_iterator(ambiguous_iterator_base it, Glyphs const & glyphs)
        : ambiguous_iterator_base(it)
        , glyphs(glyphs)
        {}

        struct value_type {
            std::reference_wrapper<ppocr::ocr2::view_ref_list> ref_list;
            Glyphs const & glyphs;

            struct range_string_iterator
            {
                using iterator = view_ref_list::const_iterator;
                iterator cur;
                iterator end;
                Glyphs const & glyphs;

                range_string_iterator(iterator first, iterator last, Glyphs const & glyphs)
                : cur(first)
                , end(last)
                , glyphs(glyphs)
                {}

                using value_type = Glyphs::string;

                value_type const & operator*() const {
                    return this->glyphs.get_word(*this->cur);
                }

                range_string_iterator & operator++() {
                    unsigned const i = this->cur->get().word;
                    while (++this->cur != this->end) {
                        if (i != this->cur->get().word) {
                            break;
                        }
                    }
                    return *this;
                }

                bool operator==(range_string_iterator const & other) const {
                    return this->cur == other.cur;
                }

                bool operator !=(range_string_iterator const & other) const {
                    return !(*this == other);
                }
            };

            range_string_iterator begin() const {
                return {ref_list.get().begin(), ref_list.get().end(), this->glyphs};
            }
            range_string_iterator end() const {
                return {ref_list.get().end(), ref_list.get().end(), this->glyphs};
            }
        };

        value_type operator*() const {
            return {ambiguous_iterator_base::operator*(), this->glyphs};
        }

        ambiguous_view_ref_list_iterator & operator++() {
            auto & base = static_cast<ambiguous_iterator_base &>(*this);
            ++base;
            return *this;
        }

        ambiguous_view_ref_list_iterator operator+(std::ptrdiff_t i) const = delete;
        ambiguous_view_ref_list_iterator & operator++(int) = delete;
    };

    auto first = ambiguous.begin();
    auto last = ambiguous.end();
    auto middle = std::find_if_not(first, first + *it_space, search_fn);
    for (; first != middle; ++first) {
        append(result, glyphs, *first);
    }
    if (first == ambiguous.begin() + *it_space) {
        ++it_space;
    }
    while (first != last) {
        auto e = ambiguous.begin() + *it_space;
        assert(e <= ambiguous.end());
        assert(first < e);
        auto middle = std::find_if(first, e, search_fn);
        assert(middle <= e);
        using It = ambiguous_view_ref_list_iterator;
        if (!word_disambiguouser(dict, It(first, glyphs), It(middle, glyphs), result)) {
            for (; first != middle; ++first) {
                assert(first <= ambiguous.end());
                append(result, glyphs, *first);
            }
        }

        for (; middle != e; ++middle) {
            if (middle->get().empty()) {
                result += '?';
                ++unrecognized_count;
            }
            else if (search_fn(*middle)) {
                append(result, glyphs, middle->get().front());
            }
            else {
                break;
            }
        }
        if (middle == e && e != last) {
            result += ' ';
            ++it_space;
        }

        first = middle;
    }

    return unrecognized_count;
}
