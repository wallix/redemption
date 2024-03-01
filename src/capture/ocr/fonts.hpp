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

#include "mln/image/image2d.hh"
#include "locale/locale_id.hpp"
#include "utils/sugar/array_view.hpp"
#include <ppocr/box_char/box.hpp>

#include <type_traits>
#include <string_view>


namespace ocr {

inline constexpr char unknown[2] = "?";

namespace fonts {
    struct Pixel {
        const ::mln::image2d<bool>& ima_;
        const ::ppocr::Box & bbox_;
        const unsigned col_;

        inline bool operator()(unsigned pos) const noexcept
        {
            return this->ima_[{this->bbox_.x() + pos % this->col_,
                               this->bbox_.y() + pos / this->col_}];
        }
    };

    struct Font {
        struct Replacement {
            std::string_view pattern;
            std::string_view replace;
        };

        std::string_view name;
        std::string_view (*const classify)(unsigned ncol, unsigned nrow, unsigned npix, Pixel accessor);
        array_view<Replacement> replacements;

        unsigned min_height_char;
        unsigned max_height_char;
        unsigned whitespace_width;

        //bool latin_to_cyrillic;
    };

#   include "ocr1/common_classifier.hxx"
#   include "ocr1/latin_classifier.hxx"
#   include "ocr1/cyrillic_classifier.hxx"

    inline constexpr Font latin_fonts[] = {
#       include "ocr1/common_classifier.names.hxx"
#       include "ocr1/latin_classifier.names.hxx"
    };

    inline constexpr Font cyrillic_fonts[] = {
#       include "ocr1/common_classifier.names.hxx"
#       include "ocr1/cyrillic_classifier.names.hxx"
    };

    inline constexpr Font const * fonts[] = {latin_fonts, cyrillic_fonts};

    using LocaleId = locale::LocaleId;

    inline constexpr unsigned nfonts[] = {
        sizeof(latin_fonts)/sizeof(latin_fonts[0])
      , sizeof(cyrillic_fonts)/sizeof(cyrillic_fonts[0])
    };

    namespace internal {
        template<std::size_t N>
        constexpr inline unsigned min_height(Font const (& fonts)[N]) noexcept
        {
            unsigned ret = fonts[0].min_height_char;
            for (unsigned i = 1; i < N; ++i) {
                if (ret > fonts[i].min_height_char) {
                    ret = fonts[i].min_height_char;
                }
            }
            return ret;
        }

        template<std::size_t N>
        constexpr inline unsigned max_height(Font const (& fonts)[N]) noexcept
        {
            unsigned ret = 0;
            for (unsigned i = 0; i < N; ++i) {
                if (ret < fonts[i].max_height_char) {
                    ret = fonts[i].max_height_char;
                }
            }
            return ret;
        }
    } // namespace internal

    inline constexpr unsigned min_height_font[] = {
        internal::min_height(latin_fonts)
      , internal::min_height(cyrillic_fonts)
    };
    inline constexpr unsigned max_height_font[] = {
        internal::max_height(latin_fonts)
      , internal::max_height(cyrillic_fonts)
    };

    inline unsigned font_id_by_name(LocaleId locale_id, std::string_view name) noexcept
    {
        unsigned id = static_cast<unsigned>(locale_id);
        for (unsigned i = 0; i < nfonts[id]; ++i) {
            if (name == fonts[id][i].name) {
                return i;
            }
        }
        return -1u;
    }
} // namespace fonts
} // namespace ocr
