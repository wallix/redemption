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

    struct MinMaxBox
    {
        unsigned min_top;
        unsigned max_top = 0;
        unsigned min_bottom;
        unsigned max_bottom = 0;

        using iterator = std::vector<ppocr::Box>::const_iterator;

        MinMaxBox(iterator first, iterator last)
        : min_top(first->top())
        , min_bottom(first->bottom())
        {
            while (++first != last) {
                min_top = std::min(min_top, first->top());
                max_top = std::max(max_top, first->top());
                min_bottom = std::min(min_bottom, first->bottom());
                max_bottom = std::max(max_bottom, first->bottom());
            }
        }
    };
}

void ppocr::ocr2::filter_by_lines(
    ambiguous_t & ambiguous,
    WordsInfos const & words_infos,
    std::vector<ppocr::Box> const & boxes
) {
    assert(boxes.size() == ambiguous.size());

    if (boxes.empty()) {
        return;
    }

    MinMaxBox const min_max_box{boxes.begin(), boxes.end()};
    unsigned bottom_ys_len = min_max_box.max_bottom - min_max_box.min_bottom + 1u;
    unsigned top_ys_len = min_max_box.max_top - min_max_box.min_top + 1u;

    std::vector<unsigned> uint_buffer(std::max(bottom_ys_len, top_ys_len), 0u);

    auto const baseline = [&]{
        auto& ys = uint_buffer;
        for (auto const& box : boxes) {
            ++ys[box.bottom() - min_max_box.min_bottom];
        }
        auto max_value_idx = std::max_element(ys.begin(), ys.begin() + bottom_ys_len) - ys.begin();
        return static_cast<unsigned>(max_value_idx) + min_max_box.min_bottom;
    }();

    std::fill(uint_buffer.begin(), uint_buffer.begin() + std::min(bottom_ys_len, top_ys_len), 0u);
    auto& meanline_map = uint_buffer;
    bool has_meanline = false;

    //for (rdp_ppocr::view_ref_list & vec : ambiguous) {
    //    for (ppocr::ppocr::loader2::View const & view : vec) {
    //        std::cout << view.word << "(" << view.word.size() << ") ";
    //    }
    //    std::cout << "----\n";
    //}
    //std::cout << "#######\n";

    auto line_pred = [](uint16_t line_flags, unsigned ypos, unsigned line_pos){
        switch (line_flags) {
            case WordLines::Upper:
                return ypos + 1 >= line_pos;
            case WordLines::Below:
                return (ypos < line_pos ? line_pos - ypos : ypos - line_pos) > 1u;
            case WordLines::Above:
                return ypos <= line_pos + 1;
            case WordLines::Upper | WordLines::Below:
                return ypos + 1 > line_pos;
            case WordLines::Below | WordLines::Above:
                return ypos < line_pos + 1;
        }
        return false;
    };

    auto it = boxes.cbegin();
    for (ppocr::ocr2::view_ref_list & vec : ambiguous) {
        if (filter_line(vec, [&](View const & view) -> bool {
            auto p = words_infos.get(view.word);
            return p && line_pred(p->lines.baseline, it->bottom(), baseline);
        }, 0u) == 1u) {
            if (auto p = words_infos.get(vec[0].get().word)) {
                auto const & lines = p->lines;
                if (lines.baseline == WordLines::Below && lines.meanline == WordLines::Below) {
                    ++meanline_map[it->top() - min_max_box.min_top];
                    has_meanline = true;
                }
            }
        }

        ++it;
    }

    if (has_meanline) {
        auto max_value_idx = std::max_element(meanline_map.begin(), meanline_map.begin() + top_ys_len) - meanline_map.begin();
        auto meanline = static_cast<unsigned>(max_value_idx) + min_max_box.min_top;

        it = boxes.cbegin();
        for (view_ref_list & vec : ambiguous) {
            struct empty_counter { empty_counter & operator++() { return *this; } };
            filter_line(vec, [&](View const & view) -> bool {
                auto p = words_infos.get(view.word);
                return p && line_pred(p->lines.meanline, it->top(), meanline);
            }, empty_counter{});
            ++it;
        }
    }
}
