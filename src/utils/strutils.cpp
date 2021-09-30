/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#include "utils/strutils.hpp"

#include <cstring>

namespace
{

// For replace_substr_between_tags() function
bool check_tags_syntax_validity(std::string_view str,
                                std::string_view opening_tag,
                                std::string_view closure_tag)
{
    assert(!opening_tag.empty() && !closure_tag.empty());

    bool unique_tag = (opening_tag == closure_tag);
    bool reversed_tag = false;
    size_t nb_tags_reached = 0;
    size_t current_pos = 0;

    while (1)
    {
        size_t opening_tag_pos = str.find(opening_tag, current_pos);

        if (opening_tag_pos == std::string_view::npos)
        {
            if (unique_tag)
            {
                break;
            }
        }
        else
        {
            ++nb_tags_reached;
        }

        size_t closure_tag_pos = 0;

        if (unique_tag)
        {
            closure_tag_pos = str.find(closure_tag,
                                       opening_tag_pos + opening_tag.size());
            if (closure_tag_pos == std::string_view::npos)
            {
                break;
            }

            ++nb_tags_reached;
        }
        else
        {
            closure_tag_pos = str.find(closure_tag, current_pos);
            if (closure_tag_pos == std::string_view::npos)
            {
                break;
            }

            ++nb_tags_reached;

            if (opening_tag_pos == std::string_view::npos)
            {
                break;
            }
            else if (closure_tag_pos < opening_tag_pos)
            {
                reversed_tag = true;
                break;
            }
        }

        current_pos = closure_tag_pos + closure_tag.size();
    }

    return !reversed_tag && nb_tags_reached % 2 == 0;
}

} // namespace


namespace utils
{

bool starts_with(chars_view str, chars_view prefix) noexcept
{
    return str.size() >= prefix.size()
        && strncmp(str.data(), prefix.data(), prefix.size()) == 0;
}

bool ends_with(chars_view str, chars_view suffix) noexcept
{
    return str.size() >= suffix.size()
        && strncmp(str.data() + str.size() - suffix.size(), suffix.data(), suffix.size()) == 0;
}

bool ends_case_with(chars_view str, chars_view suffix) noexcept
{
    return str.size() >= suffix.size()
        && strncasecmp(str.data() + str.size() - suffix.size(), suffix.data(), suffix.size()) == 0;
}


std::size_t strlcpy(char* dest, chars_view src, std::size_t n) noexcept
{
    auto const nsrc = src.size();
    if (n) {
        auto const ncp = std::min(n-1u, nsrc);
        std::memcpy(dest, src.data(), ncp);
        dest[ncp] = 0;
    }
    return nsrc;
}

std::size_t strlcpy(char* dest, char const* src, std::size_t n) noexcept
{
    return strlcpy(dest, chars_view{src, strlen(src)}, n);
}


void replace_substr_on_tag(std::string& str,
                           std::string_view tag,
                           std::string_view replacement)
{
    assert(!str.empty() && !tag.empty());

    size_t pos = 0;

    while ((pos = str.find(tag, pos)) != std::string::npos)
    {
        str.replace(pos, tag.size(), replacement.data(), replacement.size());

        pos += replacement.size();
    }
}

void replace_substr_on_tag(std::string& str,
                           std::string_view tag,
                           std::string_view replacement,
                           std::string_view decorator)
{
    assert(!str.empty() && !tag.empty());

    size_t pos = 0;

    while ((pos = str.find(tag, pos)) != std::string::npos)
    {
        str.insert(pos,
                   decorator.data(),
                   decorator.size());
        str.replace(pos + decorator.size(),
                    tag.size(),
                    replacement.data(),
                    replacement.size());
        str.insert(pos + decorator.size() + replacement.size(),
                   decorator.data(),
                   decorator.size());

        pos += replacement.size() + decorator.size() * 2;
    }
}

void replace_substr_between_tags(std::string& str,
                                 std::string_view replacement,
                                 std::string_view opening_tag,
                                 std::string_view closure_tag)
{
    assert(!str.empty());

    if (!check_tags_syntax_validity(str, opening_tag, closure_tag))
    {
        return;
    }

    size_t current_pos = 0;

    while (1)
    {
        size_t opening_tag_pos = str.find(opening_tag, current_pos);

        if (opening_tag_pos == std::string_view::npos)
        {
            break;
        }

        str.replace(opening_tag_pos, opening_tag.size(), "");

        size_t closure_tag_pos = str.find(closure_tag, opening_tag_pos);

        assert(closure_tag_pos != std::string_view::npos);

        str.replace(opening_tag_pos,
                    closure_tag_pos - opening_tag_pos,
                    replacement.data(),
                    replacement.size());

        str.replace(opening_tag_pos + replacement.size(),
                    closure_tag.size(),
                    "");

        current_pos = opening_tag_pos + replacement.size();
    }
}

void replace_substr_between_tags(std::string& str,
                                 std::string_view replacement,
                                 std::string_view tag)
{
    replace_substr_between_tags(str, replacement, tag, tag);
}

} // namespace utils
