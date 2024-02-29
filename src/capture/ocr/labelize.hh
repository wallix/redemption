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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#pragma once

#include <mln/image/image2d.hh>

#include <vector>
#include <algorithm>

namespace ocr {
namespace internal {

    struct column_info_t {
        unsigned nb_pixel;
        unsigned row_first;
        unsigned row_last;
    };

    inline
    bool adjacent_right(::mln::image2d<bool> const & input, unsigned row, unsigned nrows, unsigned col)
    {
        /**      .... __ adjacent pixel (min-1 ; max+1)
         vline __..x.   |
              |  .xx.   |
              |  .xx.   |
              |__.xx.   |
                 ..x. __|
                 ....
         */

        auto inc_row = [&](unsigned& row, unsigned nrows, unsigned col) {
            while (row < nrows && !input[{row, col}]) {
                ++row;
            }
        };

        inc_row(row, nrows, col);
        if (row >= nrows) {
            return false;
        }

        unsigned nrows2 = nrows < input.nrows() ? nrows + 1 : input.nrows();
        unsigned row2 = row ? row - 1 : 0u;

        inc_row(row2, nrows2, col + 1);
        if (row2 >= nrows2) {
            return false;
        }

        for (;;) {
            if (row <= row2) {
                if (row2 - row <= 1) {
                    return true;
                }
                row = std::max(row + 1, row2 - 1);
                inc_row(row, nrows, col);
                if (row >= nrows) {
                    return false;
                }
            }
            else {
                if (row - row2 <= 1) {
                    return true;
                }
                row2 = std::max(row2 + 1, row - 1);
                inc_row(row2, nrows2, col + 1);
                if (row2 >= nrows2) {
                    return false;
                }
            }
        }
    }

    inline
    column_info_t col_info(::mln::image2d<bool> const & input, unsigned nrows, unsigned col)
    {
        column_info_t ret = {0,nrows,0};
        unsigned brow = 0;
        for (; brow < nrows; ++brow) {
            if (input[{brow, col}]) {
                unsigned erow = nrows;
                while (--erow >= brow) {
                    if (input[{erow, col}]) {
                        break;
                    }
                }
                ret.row_first = brow;
                ret.row_last = erow;
                ++ret.nb_pixel;
                while (++brow <= erow) {
                    if (input[{brow, col}]) {
                        ++ret.nb_pixel;
                    }
                }
                break;
            }
        }
        return ret;
    }
} // namespace internal

struct label_attr_t
{
    ::mln::box2d bbox;
    unsigned area;
};

inline
void labelize(std::vector<label_attr_t> & attributes, const ::mln::image2d<bool>& input)
{
    const unsigned nrows = input.nrows();
    const unsigned ncols = input.ncols();
    label_attr_t tmp;

    for (unsigned col = 0; col < ncols; ++col) {
        internal::column_info_t info = internal::col_info(input, nrows, col);
        if (info.nb_pixel) {
            unsigned ccol = col;
            tmp.area = info.nb_pixel;
            unsigned min_row = info.row_first;
            unsigned max_row = info.row_last+1;
            while (ccol < ncols-1) {
                if (!internal::adjacent_right(input, min_row, max_row, ccol)) {
                    break;
                }
                ++ccol;
                internal::column_info_t info2 = internal::col_info(input, nrows, ccol);
                if (!info2.nb_pixel) {
                    break;
                }
                tmp.area += info2.nb_pixel;
                info.row_first = std::min(info.row_first, info2.row_first);
                info.row_last = std::max(info.row_last, info2.row_last);
                min_row = info2.row_first;
                max_row = info2.row_last+1;
            }

            tmp.bbox.pmin.col = col;
            tmp.bbox.pmax.col = ccol;
            tmp.bbox.pmin.row = info.row_first;
            tmp.bbox.pmax.row = info.row_last;

            attributes.push_back(tmp);

            col = ccol;
        }
    }
}

} //namespace ocr
