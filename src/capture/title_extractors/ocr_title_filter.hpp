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

#pragma once

#include <vector>
#include <algorithm>

#include <cstring>

#include "ocr_title.hpp"
#include "utils/sugar/array_view.hpp"


class OcrTitleFilter
{
    char last_title[256] {};

    std::size_t last_title_len = 0;
    bool title_changed = false;

public:
    OcrTitleFilter() = default;

    array_view_const_char get_title() const {
        return {this->last_title, this->last_title_len};
    }

    bool has_title_changed() const {
        return this->title_changed;
    }

    /// \return  index of best title or 0 if titles is empty
    std::size_t extract_best_title(std::vector<OcrTitle> const & titles)
    {
        constexpr std::size_t last_title_sz_limit = sizeof(this->last_title) - 1u;
        this->title_changed = false;
        std::size_t maxlen = 0;
        std::size_t maxi = 0;
        if (titles.size() == 1u) {
            maxlen = std::min(titles[0].text.size(), last_title_sz_limit);
        }
        else if (!titles.empty()) {
            auto is_title_bar_fn = [](OcrTitle const & a) { return a.is_title_bar; };
            auto p1 = std::find_if(titles.begin(), titles.end(), is_title_bar_fn);
            // only one title is a title bar
            if (titles.end() != p1
             && titles.end() == std::find_if(p1 + 1, titles.end(), is_title_bar_fn)) {
                maxi = p1 - titles.begin();
                maxlen = std::min(p1->text.size(), last_title_sz_limit);
            }
            else {
                auto const unrecognized_rate_limit = std::min_element(
                  titles.begin(), titles.end()
                , [](OcrTitle const & a, OcrTitle const & b) {
                    return a.unrecognized_rate < b.unrecognized_rate;
                })->unrecognized_rate + 5u;

                std::size_t index = 0;
                // TODO I should detect the best message...
                for (OcrTitle const & title : titles) {
                    if (title.unrecognized_rate <= unrecognized_rate_limit) {
                        std::size_t const len_title = std::min(title.text.size(), last_title_sz_limit);
                        if (len_title > maxlen){
                            maxlen = len_title;
                            maxi = index;
                        }
                    }
                    ++index;
                }
            }
        }

        if (maxi < titles.size()){
            char const * title_text = titles[maxi].text.data();
            if ((maxlen > this->last_title_len)
            /// if @a last_title begins with @a title_text, @a last_title is keep (suppose a window partially shadow)
            || (0 != strncmp(title_text, this->last_title, std::min(maxlen, this->last_title_len)))){
                memcpy(this->last_title, title_text, maxlen);
                this->last_title[maxlen] = 0;

                this->last_title_len = maxlen;
                this->title_changed = true;
            }
        }

        return maxi;
    }
};
