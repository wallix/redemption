#pragma once

#include "extract_data.hh"
#include "rgb8.hpp"

#include "mln/image/image2d.hh"

#include <vector>
#include <cstdlib>
#include <stdint.h>

#include <iostream>

namespace ocr
{
    struct titlebar_color {
        rgb8 bg;
        rgb8 fg;
        unsigned adjust_height;

        template<class Color>
        bool threshold_bars(const Color & c) const
        { return this->bg == c; }

        template<class Color>
        bool threshold_chars(const Color & c) const
        { return this->fg == c; }

        template<class Color>
        bool is_color_bar(const Color & c) const
        { return this->bg == c || this->fg == c; }
    };

    static const titlebar_color titlebar_colors[] = {
        {rgb8(8,   36,  107), rgb8(255,255,255), 0},
        {rgb8(8,   32,  107), rgb8(255,255,255), 0}, // 2008r2 (russe IT-Bastion)
        {rgb8(10,  36,  106), rgb8(255,255,255), 0}, // 2008r2
        {rgb8(0,   0,   128), rgb8(255,255,255), 0},
        {rgb8(0,   85,  231), rgb8(255,255,255), 0},
        {rgb8(99,  203, 239), rgb8(0,0,0)      , 5}, // windows 2012
        {rgb8(99,  203, 231), rgb8(0,0,0)      , 5}, // windows 2012 VNC
        {rgb8(0,   0,   132), rgb8(255,255,255), 0}, // windows 2000
        {rgb8(156, 182, 214), rgb8(0,0,0)      , 2}, // russian (areo theme ?)
    };

    struct titlebar_color_id {
        enum type {
            WINDOWS_2012 = 5,
            WINDOWS_2012_VNC = 6,
            WINDOWS_RUSSIAN = 8
        };
    };

    static_assert(sizeof(titlebar_colors)/sizeof(titlebar_colors[0]) == 9, "Please, check titlebar_color_id");

    struct titlebar_color_windows2012_standard {
        template<class Color>
        bool threshold_bars(const Color & c) const
        {
            return
                rgb8(99, 203, 239) == c
             || rgb8(90, 186, 214) == c
             || rgb8(82, 166, 198) == c
             || rgb8(74, 146, 173) == c
            ;
        }

        template<class Color>
        bool threshold_chars(const Color & c) const
        {
            return
                rgb8(57, 113, 132) == c
             || rgb8(33,  69,  82) == c
             || rgb8( 0,   0,   0) == c
            ;
        }

        template<class Color>
        bool is_color_bar(const Color & c) const
        { return this->threshold_bars(c) || this->threshold_chars(c); }
    };

    struct titlebar_color_windows2012_vnc_standard {
        template<class Color>
        bool threshold_bars(const Color & c) const
        {
            return
                rgb8(99, 203, 231) == c
             || rgb8(99, 195, 222) == c
             || rgb8(90, 186, 214) == c
             || rgb8(90, 174, 198) == c
             || rgb8(82, 166, 189) == c
             || rgb8(74, 154, 173) == c
             || rgb8(74, 150, 173) == c
             || rgb8(74, 142, 165) == c
            ;
        }

        template<class Color>
        bool threshold_chars(const Color & c) const
        {
            return
                rgb8(66, 138, 156) == c
             || rgb8(57, 113, 132) == c
             || rgb8(49, 105, 123) == c
             || rgb8(49,  97, 115) == c
             || rgb8(49,  77,  90) == c
             || rgb8(41,  77,  90) == c
             || rgb8(24,  52,  57) == c
             || rgb8(16,  32,  33) == c
             || rgb8( 8,  12,  16) == c
             || rgb8( 8,  20,  24) == c
             || rgb8( 8,  24,  24) == c
             || rgb8( 0,   0,   0) == c
            ;
        }

        template<class Color>
        bool is_color_bar(const Color & c) const
        { return this->threshold_bars(c) || this->threshold_chars(c); }
    };

    template<class Color>
    unsigned titlebar_color_id_by_bg(const Color & color)
    {
        for (unsigned ret = 0; ret < sizeof(titlebar_colors)/sizeof(titlebar_colors[0]); ++ret) {
            if (titlebar_colors[ret].threshold_bars(color)) {
                return ret;
            }
        }
        return -1u;
    }

    static const unsigned uninitialized_titlebar_color_id = -1u;

    class ExtractTitles
    {
        std::vector<uint8_t> deja_vu;
        std::vector<mln::box2d> rect_deja_vu;
        unsigned col_deja_vu;
        unsigned bbox_min_height;
        unsigned bbox_max_height;

        struct {
            unsigned col_button;
            unsigned row_first_text;
            unsigned row_last_text;
            unsigned col_first_text;
            unsigned col_last_text;
            unsigned bbox_min_height;
            unsigned bbox_max_height;

            ::mln::box2d box() const
            {
                return ::mln::box2d(
                    ::mln::point2d(this->row_first_text, this->col_first_text),
                    ::mln::point2d(this->row_last_text, this->col_last_text)
                );
            }
        } context;

    public:
        ExtractTitles(unsigned box_min_height = ocr::bbox_min_height,
                      unsigned box_max_height = ocr::bbox_max_height)
        : col_deja_vu(0)
        , bbox_min_height(box_min_height)
        , bbox_max_height(box_max_height)
        {}

        void set_box_height(unsigned box_min_height, unsigned box_max_height)
        {
            this->bbox_min_height = box_min_height;
            this->bbox_max_height = box_max_height;
        }

        template <class ImageView, class Callback>
        void extract_titles(ImageView const & input, Callback f, unsigned title_id = uninitialized_titlebar_color_id)
        {
            if (input.width() < ocr::bbox_min_width){
                return ;
            }

            typedef std::vector<mln::box2d>::iterator box_iterator;
            for (box_iterator first = this->rect_deja_vu.begin(), last = this->rect_deja_vu.end(); first != last; ++first) {
                for (unsigned y = first->min_row(), y_max = first->max_row(); y != y_max; ++y) {
                    std::fill(this->deja_vu.begin() + this->col_deja_vu * y + first->min_col(),
                              this->deja_vu.begin() + this->col_deja_vu * y + first->max_col(),
                              false);
                }
            }

            const unsigned nx = input.width();
            const unsigned ny = input.height();

            this->deja_vu.resize(nx * ny, 0);
            this->rect_deja_vu.clear();
            this->col_deja_vu = nx;

            this->context.bbox_min_height = this->bbox_min_height;
            this->context.bbox_max_height = this->bbox_max_height;

            for (unsigned y = 0; y < ny; ++y) {
                for (unsigned x = 0; x < nx; x += ocr::bbox_min_width) {
                    if (!this->deja_vu[y * nx + x]) {
                        unsigned tid = title_id;
                        if (tid == -1u
                          ? (tid = titlebar_color_id_by_bg(input(y, x))) == -1u
                          : !titlebar_colors[tid].threshold_bars(input(y, x))
                        ) {
                            continue ;
                        }

                        titlebar_color const & tcolor = titlebar_colors[tid];

                        unsigned xp = x;
                        unsigned min = x < ocr::bbox_min_width ? 0 : x - ocr::bbox_min_width;
                        while (xp > min && !this->deja_vu[y * nx + xp-1] && tcolor.threshold_bars(input(y, xp-1))) {
                            --xp;
                        }

                        if (!this->deja_vu[y * nx + xp] && tcolor.threshold_bars(input(y, xp))) {
                            this->context.bbox_max_height += tcolor.adjust_height;

                            // title bar of width 4096 height 25...
                            const unsigned x_max = std::min(xp + 4096, nx);
                            const unsigned y_max = std::min(y + this->context.bbox_max_height + ocr::bbox_treshold + 1, ny);
                            if (x_max - xp >= ocr::bbox_min_width && y_max - y >= ocr::bbox_treshold) {
                                if (tid == titlebar_color_id::WINDOWS_2012) {
                                    this->propagate(input, titlebar_color_windows2012_standard(), xp, x, y, x_max, y_max);
                                }
                                else if (tid == titlebar_color_id::WINDOWS_2012_VNC) {
                                    this->propagate(input, titlebar_color_windows2012_vnc_standard(), xp, x, y, x_max, y_max);
                                }
                                else {
                                    this->propagate(input, tcolor, xp, x, y, x_max, y_max);
                                }
                                if (this->context.col_last_text != 0) {
                                    f(input, tid, this->context.box(), this->context.col_button);
                                    x = this->context.col_button;
                                }
//                                 mln::point2d point2d = this->propagate_old(input, tcolor, xp, y, x_max, y_max);
//                                 if (unsigned(point2d.col()) >= x) {
//                                     if (1 + point2d.col() - xp > ocr::bbox_min_width
//                                      && std::abs(int(point2d.row() - y) - int(this->context.bbox_max_height))
//                                       <= ocr::bbox_treshold) {
//                                         f(input, tid, mln::box2d(mln::point2d(y,xp), point2d), point2d.col());
//                                     }
//                                     x = point2d.col();
//                                 }
                            }
                        }

                        this->context.bbox_max_height -= tcolor.adjust_height;
                    }
                }
            }
        }

    private:
        static bool is_win2012(const titlebar_color_windows2012_standard &)
        { return true; }

        static bool is_win2012(const titlebar_color_windows2012_vnc_standard &)
        { return true; }

        static bool is_win2012(const titlebar_color & tcolor)
        {
            return &tcolor == &titlebar_colors[titlebar_color_id::WINDOWS_2012]
                || &tcolor == &titlebar_colors[titlebar_color_id::WINDOWS_2012_VNC];
        }

        template <class ImageView, class TitlebarColor>
        void propagate(
            ImageView const & input, TitlebarColor const & tcolor,
            unsigned bx, unsigned bxx, unsigned y, unsigned x_max, unsigned y_max)
        {
            // ignore icons
            const unsigned px_ignore = is_win2012(tcolor) ? 108 : this->context.bbox_max_height*2;
            /*const*/ unsigned x = std::min(bx + px_ignore, x_max - 1);
            /*const*/ unsigned xx = std::min(bxx + px_ignore, x_max - 1);

            unsigned iw = xx + 1;

            while (iw < x_max && !this->deja_vu[y * input.width() + iw] && tcolor.threshold_bars(input(y, iw))) {
                ++iw;
            }
            x_max = iw;
            --iw;
            this->context.col_last_text = 0;

            if (iw - x < ocr::bbox_min_width - this->context.bbox_max_height*2) {
                this->rect_deja_vu.push_back(mln::box2d(mln::point2d(y,iw), mln::point2d(y+1,iw+1)));
                this->deja_vu[y * input.width() + iw] = true;
                return ;
            }

            unsigned ih = y + 1;

            struct auto_set_deja_vu {
                ExtractTitles & extract_titles;
                ImageView const & input;
                unsigned x, y, w, & ih;

                auto_set_deja_vu(
                    ExtractTitles & extract_titles_, ImageView const & input_,
                    unsigned x_, unsigned y_, unsigned w_, unsigned & ih_)
                : extract_titles(extract_titles_)
                , input(input_)
                , x(x_), y(y_), w(w_), ih(ih_)
                {}

                ~auto_set_deja_vu()
                {
                    const unsigned width = this->input.width();
                    const unsigned h = std::min(this->ih, this->input.height()) - this->y;
                    typedef std::vector<uint8_t>::iterator iterator;
                    iterator first = this->extract_titles.deja_vu.begin() + this->y * width + this->x;
                    iterator last = first + width * h;
                    for (; first != last; first += width) {
                        std::fill(first, first + this->w, true);
                    }
                    this->extract_titles.rect_deja_vu.push_back(
                        mln::box2d(mln::point2d(this->y,this->x), mln::point2d(this->y+h, this->x+this->w)));
                }
            } set_deja_vu(*this, input, bx, y, iw - x, ih);

            while (ih < y_max && !this->deja_vu[ih * input.width() + xx] && tcolor.threshold_bars(input(ih, xx))) {
                unsigned w = x;
                while (w < x_max && !this->deja_vu[ih * input.width() + w] && tcolor.threshold_bars(input(ih, w))) {
                    ++w;
                }
                if (w < x_max) {
                    if (this->deja_vu[ih * input.width() + w]) {
                        return ;
                    }
                    if (!tcolor.threshold_bars(input(ih, w))) {
                        break;
                    }
                }

                ++ih;

                if (ih - y > this->context.bbox_max_height + ocr::bbox_padding) {
                    return ;
                }
            }

            if (ih < y_max && !this->deja_vu[ih * input.width() + xx] && !tcolor.is_color_bar(input(ih, xx))) {
                return;
            }

            const unsigned savih = ih++;
            bool is_empty_line = false;

            unsigned x_no_title = x_max;
            unsigned last_char = x;
            unsigned prev_last_char = x;

            while (ih < y_max && !this->deja_vu[ih * input.width() + xx] && tcolor.is_color_bar(input(ih, xx))) {
                unsigned w = x;
                bool contains_char = false;
                while (w < iw && !this->deja_vu[ih * input.width() + w]) {
                    if (tcolor.threshold_bars(input(ih, w))) {
                        //nada
                    }
                    else if (tcolor.threshold_chars(input(ih, w))) {
                        contains_char = true;
                        if (w < x_no_title - 2 && w > last_char) {
                            prev_last_char = last_char;
                            last_char = w;
                        }
                    }
                    else {
                        x_no_title = std::min(x_no_title, w);
                        last_char = prev_last_char;
                        break;
                    }
                    ++w;
                }
                if (w < iw) {
                    if (this->deja_vu[ih * input.width() + w]) {
                        return ;
                    }
                }

                if (contains_char == false) {
                    if (is_empty_line) {
                        break;
                    }
                }
                is_empty_line = (contains_char == false);

                ++ih;

                if (ih - y > this->context.bbox_max_height + ocr::bbox_padding) {
                    return ;
                }
            }

            if (x == x_no_title) {
                return ;
            }

            if (is_empty_line) {
                --ih;
            }

            const unsigned hbarre = ih - savih;

            if (hbarre < this->context.bbox_min_height || hbarre > this->context.bbox_max_height + ocr::bbox_treshold) {
                return ;
            }

            this->context.col_button = x_no_title - (
                tcolor.threshold_bars(input(savih + hbarre/2, x_no_title-1)
            ) ? 1u : 2u);
            this->context.row_last_text = savih + hbarre;
            this->context.col_last_text = std::min(last_char + 2, x_max);
            set_deja_vu.w = this->context.col_button - bx;

            unsigned newx = x;
            unsigned xtext = x;
            for (; newx > bx; --newx) {
                unsigned yy = 0;
                for (; yy < hbarre; ++yy) {
                    typename ImageView::value_type const & c = input(savih+yy, newx);
                    if (tcolor.threshold_bars(c)) {
                        //nada
                    }
                    else if (tcolor.threshold_chars(c)) {
                        xtext = newx;
                    }
                    else {
                        break;
                    }
                }
                if (yy != hbarre) {
                    if (newx != x) {
                        newx = xtext;
                    }
                    break;
                }
            }
            this->context.col_first_text = newx;
            this->context.row_first_text = y;

            //while (this->context.row_first_text < this->context.row_last_text
            //    && this->is_line_bars(
            //        input, tcolor,
            //        this->context.col_first_text, this->context.col_last_text, this->context.row_first_text))
            //{
            //    ++this->context.row_first_text;
            //}
            //
            //while (this->context.row_last_text - 1 > this->context.row_first_text
            //    && this->is_line_bars(
            //        input, tcolor,
            //        this->context.col_first_text, this->context.col_last_text, this->context.row_last_text-1))
            //{
            //   --this->context.row_last_text;
            //}
            //
            //while (this->context.col_first_text < this->context.col_last_text) {
            //    unsigned y = this->context.row_first_text;
            //    for (; y < this->context.row_last_text; ++y) {
            //        if (tcolor.threshold_chars(input(y, this->context.col_first_text))) {
            //            break;
            //        }
            //    }
            //    if (y != this->context.row_last_text) {
            //        break;
            //    }
            //    ++this->context.col_first_text;
            //}
        }

        //template <class ImageView>
        //static bool is_line_bars(
        //    ImageView const & input, titlebar_color const & tcolor,
        //    unsigned x, unsigned xmax, unsigned y)
        //{
        //    for (; x < xmax; ++x) {
        //        if (tcolor.threshold_chars(input(y, x))) {
        //            return false;
        //        }
        //    }
        //    return true;
        //}

//         template <class ImageView>
//         mln::point2d propagate_old(
//             ImageView const & input, titlebar_color const & tcolor,
//             unsigned x, unsigned y, unsigned x_max, unsigned y_max)
//         {
//             unsigned ih = y + 1;
//             unsigned iw = x + 1;
//
//             while (ih < y_max && !this->deja_vu[ih * input.width() + iw] && tcolor.is_color_bar(input(ih, iw))) {
//                 this->deja_vu[ih * input.width() + iw] = true;
//                 ++ih;
//             }
//
//             const unsigned hbarre = ih - (y + 1);
//             unsigned wbarre = 0;
//
//             if (hbarre >= this->context.bbox_min_height && hbarre <= this->context.bbox_max_height + ocr::bbox_treshold) {
//                 ih = y;
//
//                 while (iw < x_max && !this->deja_vu[ih * input.width() + iw] && tcolor.is_color_bar(input(ih, iw))) {
//                     while (ih < y_max && !this->deja_vu[ih * input.width() + iw]
//                       && tcolor.is_color_bar(input(ih, iw))) {
//                         ++ih;
//                     }
//                     ih = y;
//                     ++iw;
//                     ++wbarre;
//                 }
//
//                 std::vector<uint8_t>::iterator it = this->deja_vu.begin() + y * input.width() + x;
//                 for (unsigned i = 0; i < hbarre; ++i) {
//                     std::fill(it, it + wbarre, true);
//                     it += input.width();
//                 }
//
//                 typedef std::vector<uint8_t>::iterator iterator;
//                 iterator first = this->deja_vu.begin() + y * input.width() + x;
//                 iterator last = first + input.width() * hbarre;
//                 for (; first != last; first += input.width()) {
//                     std::fill(first, first + wbarre, true);
//                 }
//                 this->rect_deja_vu.push_back(mln::box2d(mln::point2d(y, x), mln::point2d(y + hbarre, x + wbarre)));
//
//                 y += hbarre;
//                 x += wbarre;
//             }
//
//             return mln::point2d(y, x);
//         }
    };

    template<class ImageView>
    bool is_title_bar(
        ImageView const & input, unsigned tid,
        unsigned min_row, unsigned max_row, unsigned button_col, unsigned max_h)
    {
        const titlebar_color & tcolor = titlebar_colors[tid];

        if (button_col + 2 >= input.width()) {
            return false;
        }

        if (tid == titlebar_color_id::WINDOWS_2012 || tid == titlebar_color_id::WINDOWS_2012_VNC) {
            rgb8 const cbutton(49, tid == titlebar_color_id::WINDOWS_2012 ? 48 : 52, 49);
            /*
             * legend:
             *  b -> bar
             *  x -> rgb(49,48,49)
             *  ... -> etc
             *
             * bxb
             * bxb
             * ...
             * bxb
             * bxb
             * bxx
             */
            max_row = std::min<unsigned>(max_row + 5, input.height());
            unsigned y = min_row;
            for (; y < max_row; ++y) {
                if (!(tcolor.threshold_bars(input(y, button_col))
                 && cbutton == input(y, button_col+1)
                 && tcolor.threshold_bars(input(y, button_col+2)))) {
                    break;
                }
            }
            if (y == max_row || y == min_row) {
                /*
                 * legend:
                 *  b -> bar
                 *  x -> rgb(90,56,49)
                 *  c -> rgb(198,89,82)
                 *  ... -> etc
                 *
                 * bxc
                 * bxc
                 * ...
                 * bxc
                 * bxc
                 * bxx
                 */
                y = min_row;
                for (; y < max_row; ++y) {
                    if (!(tcolor.threshold_bars(input(y, button_col))
                     && rgb8(90, 56, 49) == input(y, button_col+1)
                     && rgb8(198, 89, 82) == input(y, button_col+2))) {
                        break;
                    }
                }
                if (y == max_row || y == min_row) {
                    return false;
                }
                if (tcolor.threshold_bars(input(y, button_col))
                 && rgb8(90, 56, 49) == input(y, button_col+1)
                 && rgb8(90, 56, 49) == input(y, button_col+2)) {
                    return true;
                }
                return false;
            }
            if (tcolor.threshold_bars(input(y, button_col))
             && cbutton == input(y, button_col+1)
             && cbutton == input(y, button_col+2)) {
                return true;
            }
            return false;
        }

        /*
         * legend:
         *  b -> bar
         *  w -> white
         *  g -> grey
         *  l -> lite grey
         *  L -> other grey
         *  G -> other grey
         *  ... -> etc
         *
         * bb
         * bwgg
         * bwgl
         * bwgl
         * ...
         * bwgl
         * bwgl
         * bLL
         * GG
         */
        if (tid == titlebar_color_id::WINDOWS_RUSSIAN) {
            if (button_col + 4 >= input.width()) {
                return false;
            }
            constexpr rgb8 w(255, 255, 255);
            constexpr rgb8 g(231, 227, 231);
            constexpr rgb8 l(247, 243, 247);
            constexpr rgb8 L(165, 162, 165);
            constexpr rgb8 G(107, 105, 107);

            if (tcolor.threshold_bars(input(min_row, button_col))
             && tcolor.threshold_bars(input(min_row, button_col+1))) {
                ++min_row;
            }
            if (tcolor.threshold_bars(input(min_row, button_col))
             && w == input(min_row, button_col+1)
             && g == input(min_row, button_col+2)
             && g == input(min_row, button_col+3)) {
                ++min_row;
            }

            //max_row = std::min<unsigned>(max_row + 5, input.height());
            for (; min_row < max_row; ++min_row) {
                if (!(tcolor.threshold_bars(input(min_row, button_col))
                 && w == input(min_row, button_col+1)
                 && g == input(min_row, button_col+2)
                 && l == input(min_row, button_col+3)
                )) {
                   break;
                }
            }

            if (min_row == max_row) {
                return true;
            }

            if (++min_row < max_row) {
                if (!(tcolor.threshold_bars(input(min_row, button_col))
                 && L == input(min_row, button_col+1)
                 && L == input(min_row, button_col+2)
                )) {
                    return false;
                }
                if (++min_row < max_row) {
                    if (!(G == input(min_row, button_col)
                       && G == input(min_row, button_col+1)
                    )) {
                        return false;
                    }
                    if (++min_row < max_row) {
                        return tcolor.threshold_bars(input(min_row, button_col))
                            && tcolor.threshold_bars(input(min_row, button_col+1));
                    }
                }
            }

            return true;
        }

        /*
         * legend:
         *  b -> bar
         *  w -> text
         *  x -> not bar and not text
         *  ... -> etc
         *
         * bb
         * bww
         * bw
         * bw
         * ...
         * bw
         * bw
         *   x
         * bb
         */
        if (tcolor.threshold_bars(input(min_row, button_col))) {
            ++min_row;
        }
        unsigned last_row = std::max<int>(int(min_row)-int(max_h/2u), 0);
        for (; min_row >= last_row; --min_row) {
            if (!(tcolor.threshold_bars(input(min_row, button_col))
             && tcolor.threshold_chars(input(min_row, button_col+1)))) {
                break;
            }
        }
        if (min_row != -1u) {
            if (!(
                tcolor.threshold_bars(input(min_row, button_col))
             && tcolor.threshold_bars(input(min_row, button_col+1))
             //&& tcolor.threshold_bars(input(min_row, button_col+2))
             && tcolor.threshold_chars(input(min_row+1, button_col+2))
            )) {
                return false;
            }
        }

        last_row = std::min<int>(int(max_row)+int(max_h/2u), input.height()) + 1;
        for (; max_row < last_row; ++max_row) {
            if (!(tcolor.threshold_bars(input(max_row, button_col))
             && tcolor.threshold_chars(input(max_row, button_col+1)))) {
                break;
            }
        }
        if (max_row < last_row) {
            if (tcolor.is_color_bar(input(max_row-1, button_col+2))) {
                return false;
            }
            if (max_row + 1 < last_row && !(
                tcolor.threshold_bars(input(max_row+1, button_col))
             && tcolor.threshold_bars(input(max_row+1, button_col+1))
             //&& threshold_bars(input(max_row+1, button_col+2))
            )) {
                return false;
            }
        }

        if (min_row != -1u || max_row < last_row) {
            return true;
        }

        return false;
    }
}
