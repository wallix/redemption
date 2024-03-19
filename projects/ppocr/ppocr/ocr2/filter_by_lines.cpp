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

#include "ppocr/ocr2/filter_by_lines.hpp"
#include "ppocr/ocr2/words_infos.hpp"

#include "ppocr/filters/best_baseline.hpp"

#include <cassert>

namespace {
    using ppocr::ocr2::View;
    using ppocr::ocr2::view_ref_list;

    view_ref_list::iterator next_word(view_ref_list & vec) {
        auto first = vec.begin();
        auto last = vec.end();
        if (first != last) {
            auto word = first->get().word;
            while (++first != last && first->get().word == word) {
            }
        }
        return first;
    }

    template<class Predicate, class Counter>
    Counter filter_line(view_ref_list & vec, Predicate pred, Counter count) {
        auto first = next_word(vec);
        auto last = vec.end();

        if (first != last) {
            auto out = vec.begin();
            auto last = vec.end();
            if (!pred(*(first-1))) {
                do {
                    auto word = first->get().word;
                    first = std::find_if(first+1, last, [&](View const & view) { return view.word != word; });
                    ++count;
                } while (first != last && !pred(*first));
                out = first;
            }
            while (first != last) {
                auto word = first->get().word;
                auto rlast = std::find_if(first+1, last, [&](View const & view) { return view.word != word; });
                if (!pred(*first)) {
                    out = std::copy(std::make_move_iterator(first), std::make_move_iterator(rlast), out);
                    ++count;
                }
                first = rlast;
            }

            vec.erase(out, vec.end());
        }

        return count;
    }
}

void ppocr::ocr2::filter_by_lines(
    ambiguous_t & ambiguous,
    WordsInfos const & words_infos,
    std::vector<ppocr::Box> const & boxes
) {
    using iterator_base = std::vector<Box>::const_iterator;
    struct iterator_baseline : iterator_base
    {
        using value_type = unsigned;

        iterator_baseline(iterator_base base)
        : iterator_base(base)
        {}

        value_type operator*() const {
            return iterator_base::operator*().bottom();
        }
    };

    auto const baseline = filters::best_baseline(
        iterator_baseline(boxes.begin()),
        iterator_baseline(boxes.end())
    );

    std::map<unsigned, unsigned> meanline_map;

    //for (rdp_ppocr::view_ref_list & vec : ambiguous) {
    //    for (ppocr::ppocr::loader2::View const & view : vec) {
    //        std::cout << view.word << "(" << view.word.size() << ") ";
    //    }
    //    std::cout << "----\n";
    //}
    //std::cout << "#######\n";

    auto it = boxes.cbegin();
    assert(boxes.size() == ambiguous.size());
    for (ppocr::ocr2::view_ref_list & vec : ambiguous) {
        if (filter_line(vec, [&](View const & view) -> bool {
            if (auto p = words_infos.get(view.word)) {
                switch (p->lines.baseline) {
                    case WordLines::Upper:
                        return it->bottom() + 1 >= baseline;
                    case WordLines::Below:
                        return (it->bottom() < baseline ? baseline - it->bottom() : it->bottom() - baseline) > 1u;
                    case WordLines::Above:
                        return it->bottom() <= baseline + 1;
                    case WordLines::Upper | WordLines::Below:
                        return it->bottom() + 1 > baseline;
                    case WordLines::Below | WordLines::Above:
                        return it->bottom() < baseline + 1;
                }
            }
            return false;
        }, 0u) == 1u) {
            if (auto p = words_infos.get(vec[0].get().word)) {
                auto const & lines = p->lines;
                if (lines.baseline == WordLines::Below && lines.meanline == WordLines::Below) {
                    ++meanline_map[it->top()];
                }
            }
        }

        ++it;
    }

    if (!meanline_map.empty()) {
        using cP = decltype(meanline_map)::value_type const;
        auto meanline = std::max_element(meanline_map.begin(), meanline_map.end(), [](cP & a, cP & b) {
            return a.second < b.second;
        })->first;

        it = boxes.cbegin();
        for (view_ref_list & vec : ambiguous) {
            struct empty_counter { empty_counter & operator++() { return *this; } };
            filter_line(vec, [&](View const & view) -> bool {
                if (auto p = words_infos.get(view.word)) {
                    switch (p->lines.meanline) {
                        case WordLines::Upper:
                            return it->top() + 1 >= meanline;
                        case WordLines::Below:
                            return (it->top() < meanline ? meanline - it->top() : it->top() - meanline) > 1u;
                        case WordLines::Above:
                            return it->top() <= meanline + 1;
                        case WordLines::Upper | WordLines::Below:
                            return it->top() + 1 > meanline;
                        case WordLines::Below | WordLines::Above:
                            return it->top() < meanline + 1;
                    }
                }
                return false;
            }, empty_counter{});
            ++it;
        }
    }
}
