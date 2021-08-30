/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou, Meng Tan,
              Jennifer Inthavong
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   stream object, used for input / output communication between
   entities
*/

#pragma once

#include "utils/sugar/bytes_view.hpp"
#include "utils/only_type.hpp"

#include <cstdint>
#include <string>

enum {
      maximum_length_of_utf8_character_in_bytes = 4
};


// UTF8Len assumes input is valid utf8, zero terminated, that has been checked before
std::size_t UTF8Len(byte_ptr source) noexcept;
std::size_t UTF16ByteLen(bytes_view source) noexcept;

void UTF16Lower(uint8_t * source, std::size_t max_len) noexcept;
void UTF16Upper(uint8_t * source, std::size_t max_len) noexcept;


// UTF8GetLen find the number of bytes of the len first characters of input.
// It assumes input is valid utf8, zero terminated (that has been checked before).
std::size_t UTF8GetPos(uint8_t const * source, std::size_t len) noexcept;

// UTF8Len assumes input is valid utf8, zero terminated, that has been checked before
std::size_t UTF8StringAdjustedNbBytes(const uint8_t * source, std::size_t max_len) noexcept;

// UTF8RemoveOne assumes input is valid utf8, zero terminated, that has been checked before
void UTF8RemoveOne(writable_bytes_view source) noexcept;

// UTF8InsertUtf16 assumes input is valid utf8, zero terminated, that has been checked before
// UTF8InsertUtf16 won't insert anything and return false if modified string buffer does not have enough space to insert
bool UTF8InsertUtf16(writable_bytes_view source, std::size_t bytes_used, uint16_t unicode_char) noexcept;

// UTF8toUTF16 never writes the trailing zero
std::size_t UTF8toUTF16(bytes_view source, uint8_t * target, size_t t_len) noexcept;
std::size_t UTF8toUTF16(bytes_view source, writable_bytes_view target) noexcept;
//std::size_t UTF8toUTF16(const uint8_t * source, std::size_t s_len, uint8_t * target, std::size_t t_len);
std::vector<uint8_t> UTF8toUTF16(bytes_view source) noexcept;

// UTF8toUTF16 never writes the trailing zero (with Lf to CrLf conversion).
std::size_t UTF8toUTF16_CrLf(bytes_view source, uint8_t * target, std::size_t t_len) noexcept;

template<class ResizableArray>
void UTF8toResizableUTF16(bytes_view utf16_source, ResizableArray& utf8_target)
{
    utf8_target.resize(utf16_source.size() * 4);
    auto len = UTF8toUTF16(utf16_source, make_writable_array_view(utf8_target));
    utf8_target.resize(len);
}

template<class ResizableArray>
ResizableArray UTF8toResizableUTF16(bytes_view utf16_source)
{
    ResizableArray utf8_target;
    UTF8toResizableUTF16(utf16_source, utf8_target);
    return utf8_target;
}

template<class ResizableArray>
void UTF8toResizableUTF16_zstring(bytes_view utf16_source, ResizableArray& utf8_target)
{
    utf8_target.resize(utf16_source.size() * 4);
    auto len = UTF8toUTF16(utf16_source, make_writable_array_view(utf8_target));
    utf8_target[len    ] = '\0';
    utf8_target[len + 1] = '\0';
    utf8_target.resize(len + 2);
}

template<class ResizableArray>
ResizableArray UTF8toResizableUTF16_zstring(bytes_view utf16_source)
{
    ResizableArray utf8_target;
    UTF8toResizableUTF16_zstring(utf16_source, utf8_target);
    return utf8_target;
}


class UTF8toUnicodeIterator
{
public:
    explicit UTF8toUnicodeIterator(byte_ptr str) noexcept;

    UTF8toUnicodeIterator & operator++() noexcept;

    uint32_t operator*() const noexcept
    { return this->ucode; }

    [[nodiscard]] uint32_t code() const noexcept
    { return this->ucode; }

    [[nodiscard]] uint8_t const * pos() const noexcept
    { return this->source; }

private:
    const uint8_t * source;
    uint32_t ucode = 0;
};

// Return number of UTF8 bytes used to encode UTF16 input
// do not write trailing 0
std::size_t UTF16toUTF8(const uint8_t * utf16_source, std::size_t utf16_len, uint8_t * utf8_target, std::size_t target_len) noexcept;
// Return number of UTF8 bytes used to encode UTF16 input
// do not write trailing 0
writable_bytes_view UTF16toUTF8_buf(bytes_view utf16_source, writable_bytes_view utf8_target) noexcept;
writable_bytes_view UTF16toUTF8_buf(only_type<uint16_t> utf16_source, writable_bytes_view utf8_target) noexcept;
std::string UTF16toUTF8(bytes_view utf16_source) noexcept;

template<class ResizableArray>
void UTF16toResizableUTF8(bytes_view utf16_source, ResizableArray& utf8_target)
{
    utf8_target.resize(utf16_source.size() * 2);
    auto len = UTF16toUTF8_buf(utf16_source, make_writable_array_view(utf8_target)).size();
    utf8_target.resize(len);
}

template<class ResizableArray>
ResizableArray UTF16toResizableUTF8(bytes_view utf16_source)
{
    ResizableArray utf8_target;
    UTF16toResizableUTF8(utf16_source, utf8_target);
    return utf8_target;
}

template<class ResizableArray>
void UTF16toResizableUTF8_zstring(bytes_view utf16_source, ResizableArray& utf8_target)
{
    utf8_target.resize(utf16_source.size() * 2 + 1);
    auto len = UTF16toUTF8_buf(utf16_source, make_writable_array_view(utf8_target)).size();
    utf8_target[len] = '\0';
    utf8_target.resize(len + 1);
}

template<class ResizableArray>
ResizableArray UTF16toResizableUTF8_zstring(bytes_view utf16_source)
{
    ResizableArray utf8_target;
    UTF16toResizableUTF8_zstring(utf16_source, utf8_target);
    return utf8_target;
}

// Return number of UTF8 bytes used to encode UTF16 input
// do not write trailing 0
std::size_t UTF16toUTF8(const uint16_t * utf16_source, std::size_t utf16_len, uint8_t * utf8_target, std::size_t target_len) noexcept;

// Return number of UTF8 bytes used to encode UTF32 input
// do not write trailing 0
std::size_t UTF32toUTF8(const uint8_t * utf32_source, std::size_t utf32_len, uint8_t * utf8_target, std::size_t target_len) noexcept;

// Return number of UTF8 bytes used to encode UTF32 input
// do not write trailing 0
std::size_t UTF32toUTF8(uint32_t utf32_char, uint8_t * utf8_target, std::size_t target_len) noexcept;

size_t UTF8CharNbBytes(const uint8_t * source) noexcept;

bool is_utf8_string(uint8_t const * s, int length = -1) noexcept; /*NOLINT*/

bool is_ASCII_string(bytes_view source) noexcept;
bool is_ASCII_string(byte_ptr source) noexcept;

std::size_t UTF8StrLenInChar(byte_ptr source) noexcept;

std::size_t UTF16toLatin1(const uint8_t * utf16_source_, std::size_t utf16_len, uint8_t * latin1_target, std::size_t latin1_len) noexcept;

std::size_t Latin1toUTF16(bytes_view latin1_source,
        uint8_t * utf16_target, std::size_t utf16_len) noexcept;

std::size_t Latin1toUTF8(
    const uint8_t * latin1_source, std::size_t latin1_len,
    uint8_t * utf8_target, std::size_t utf8_len) noexcept;
