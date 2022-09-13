/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2022
Author(s): Proxies Team
*/

#include "utils/rect.hpp"

#include <array>
#include <string> // char_traits


namespace detail
{

constexpr std::size_t txt2d_to_rects_impl(
    Rect* rects,
    bool* const visited_array,
    char const* const* const txt2d,
    int const w,
    int const h)
{
    std::size_t len = 0;

    auto search_rect = [=, &len](int x, int y) -> int {
        bool* visited = visited_array + y * w + x;

        if (*visited) {
            return 1;
        }

        char const* line = txt2d[y] + x * 2;

        if (*line != '#') {
            return 1;
        }

        Rect& rect = rects[len];
        ++len;

        int end_x = x;
        rect.x = static_cast<int16_t>(x);
        rect.y = static_cast<int16_t>(y);
        do {
            *visited++ = true;
            line += 2;
            ++end_x;
        } while (end_x < w && !*visited && *line == '#');

        int rect_w = end_x - x;
        rect.cx = static_cast<uint16_t>(rect_w);
        rect.cy = 1;

        ++y;
        visited -= rect_w;

        // search y rect
        for (; y < h; ++y) {
            line = txt2d[y] + x * 2;
            visited += w;
            for (int x2 = 0; x2 < rect_w; ++x2) {
                // is an incomplet line
                if (line[x2*2] != '#' || visited[x2]) {
                    return rect_w;
                }
            }

            // xxxx
            // xxxx
            // xxxxxx <-- stop here (ignore this line)
            //    ^_____/
            if (end_x != w && !visited[1] && line[rect_w*2] == '#') {
                return rect_w;
            }

            //   xxxx
            //   xxxx
            // xxxxxx <-- stop here (ignore this line)
            //   ^______/
            if (x != 0 && !visited[-1] && line[-2] == '#') {
                return rect_w;
            }

            // set visited pixel
            for (int x2 = 0; x2 < rect_w; ++x2) {
                visited[x2] = true;
            }
            ++rect.cy;
        }

        return rect_w;
    };

    for (int y = 0; y < h; ++y) {
        int x = 0;
        while (x < w) {
            x += search_rect(x, y);
        }
    }

    return len;
}

}

#define TXT2D_TO_RECTS(...) txt2d_to_rects([]{ return std::array{__VA_ARGS__}; })

/// Transform 2d text image to rects.
/// A pixel is represented by 2 \c ##, other characters are ignored.
/// \code
/// 1 pixel:
///     "##"
/// a rectangle of 2 pixels:
///     "####",
///     "####"
/// \endcode
/// \pre line_creators() should return a `std::array<char const*, N>`
/// \pre each line of the text image should be the same size
/// \return std::array<Rect, M>
template<class LineCreators>
constexpr auto txt2d_to_rects(LineCreators line_creators)
{
    constexpr auto txt2d = line_creators();
    constexpr std::size_t h = std::tuple_size_v<decltype(txt2d)>;
    constexpr std::size_t w_ = std::char_traits<char>::length(txt2d[0]);
    constexpr std::size_t w = w_ / 2;

    static_assert((w_ & 1) == 0, "bad size, should be multiple of 2");

    struct Impl
    {
        Rect rects[(h * w + 1) / 2] {};
        bool visited[h * w] {};
        std::size_t len;

        constexpr Impl(std::array<char const*, h> const& txt2d, std::size_t w, std::size_t h)
        : len(detail::txt2d_to_rects_impl(
            rects, visited, txt2d.data(), static_cast<int>(w), static_cast<int>(h)))
        {}
    };

    constexpr Impl impl {txt2d, w, h};
    std::array<Rect, impl.len> rects {};
    for (std::size_t i = 0; i < impl.len; ++i) {
        rects[i] = impl.rects[i];
    }
    return rects;
}
