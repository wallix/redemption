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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou, Clément Moroldo
*/


#pragma once

#include "utils/sugar/bytes_view.hpp"
#include "utils/sugar/ranges.hpp"
#include "utils/literals/utf16.hpp"
#include "utils/utf.hpp"

#include <cinttypes>
#include <vector>
#include <iterator>


namespace Cliprdr
{
    enum class IsLongFormat : bool;
    enum class IsAscii : bool;

    struct UnicodeName
    {
        explicit constexpr UnicodeName(bytes_view name) noexcept
        : bytes(name)
        {}

        bytes_view bytes;
    };

    struct AsciiName
    {
        explicit constexpr AsciiName(bytes_view name) noexcept
        : bytes(name)
        {}

        bytes_view bytes;
    };

    inline namespace formats
    {
        struct Names
        {
            array_view_const_char ascii_name;
            array_view_const_char unicode_name;

            operator AsciiName () const noexcept { return AsciiName(this->ascii_name); }
            operator UnicodeName () const noexcept { return UnicodeName(this->unicode_name); }

            [[nodiscard]] bool same_as(UnicodeName const& unicode_name) const noexcept
            {
                return ranges_equal(this->unicode_name, unicode_name.bytes);
            }

            [[nodiscard]] bool same_as(AsciiName const& ascii_name) const noexcept
            {
                return ranges_equal(this->ascii_name, ascii_name.bytes);
            }
        };

#define REDEMPTION_CLIPRDR_DEF_FORMAT_NAME(var_name, format_name)         \
    REDEMPTION_DIAGNOSTIC_PUSH /* for emscripten */                       \
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-variable-declarations") \
    inline constexpr ::Cliprdr::formats::Names var_name {                 \
        format_name ""_av, format_name ""_utf16_le};                      \
    REDEMPTION_DIAGNOSTIC_POP

        REDEMPTION_CLIPRDR_DEF_FORMAT_NAME(file_group_descriptor_w, "FileGroupDescriptorW")
        REDEMPTION_CLIPRDR_DEF_FORMAT_NAME(preferred_drop_effect, "Preferred DropEffect")
        REDEMPTION_CLIPRDR_DEF_FORMAT_NAME(file_contents, "FileContents")
    } // namespace formats

    struct FormatName
    {
        using FormatId = uint32_t;

        FormatName(FormatId format_id, UnicodeName unicode_name) noexcept
        : format_id_(format_id)
        , len_(UTF16toUTF8_buf(unicode_name.bytes, make_array_view(this->utf8_buffer_)).size())
        {}

        FormatName(FormatId format_id, AsciiName ascii_name) noexcept
        : format_id_(format_id)
        , len_(std::min(ascii_name.bytes.size(), std::size(this->utf8_buffer_)))
        {
            memcpy(this->utf8_buffer_, ascii_name.bytes.data(), this->len_);
        }

        FormatName(FormatName const& other) noexcept
        : format_id_(other.format_id_)
        , len_(other.len_)
        {
            memcpy(this->utf8_buffer_, other.utf8_buffer_, other.len_);
        }

        FormatName& operator=(FormatName const& other) noexcept
        {
            this->format_id_ = other.format_id_;
            this->len_ = other.len_;
            memcpy(this->utf8_buffer_, other.utf8_buffer_, other.len_);
            return *this;
        }

        [[nodiscard]] bytes_view utf8_name() const noexcept
        {
            return {this->utf8_buffer_, this->len_};
        }

        [[nodiscard]] FormatId format_id() const noexcept
        {
            return this->format_id_;
        }

    private:
        static constexpr size_t utf8_buffer_buf_len = 123;

        FormatId format_id_;
        // TODO static_vector<raw_buf_len>
        uint8_t len_;
        uint8_t utf8_buffer_[utf8_buffer_buf_len];
    };

    struct FormatNameInventory : std::vector<FormatName>
    {
        using FormatId = FormatName::FormatId;

        FormatName const& push(FormatId format_id, UnicodeName unicode_name)
        {
            return this->emplace_back(format_id, unicode_name);
        }

        FormatName const& push(FormatId format_id, AsciiName ascii_name)
        {
            return this->emplace_back(format_id, ascii_name);
        }

        [[nodiscard]] FormatName const* find(FormatId format_id) const noexcept
        {
            for (auto const& format : *this) {
                if (format.format_id() == format_id) {
                    return &format;
                }
            }
            return nullptr;
        }
    };
} // namespace Cliprdr
