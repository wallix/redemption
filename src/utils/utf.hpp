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

#include <cstdint>
#include <vector>

enum {
      maximum_length_of_utf8_character_in_bytes = 4
};


// UTF8Len assumes input is valid utf8, zero terminated, that has been checked before
std::size_t UTF8Len(const_byte_ptr source) noexcept;
std::size_t UTF16ByteLen(const_bytes_view source) noexcept;

// TODO deprecated
std::size_t UTF16StrLen(const uint8_t * utf16_s) noexcept;

void UTF16Upper(uint8_t * source, std::size_t max_len) noexcept;


// UTF8GetLen find the number of bytes of the len first characters of input.
// It assumes input is valid utf8, zero terminated (that has been checked before).
std::size_t UTF8GetPos(uint8_t const * source, std::size_t len) noexcept;


// UTF8InsertAtPos assumes input is valid utf8, zero terminated, that has been checked before
// UTF8InsertAtPos won't insert anything and return false if modified string buffer does not have enough space to insert
bool UTF8InsertAtPos(uint8_t * source, std::size_t len, const uint8_t * to_insert, std::size_t max_source) noexcept;

// UTF8Len assumes input is valid utf8, zero terminated, that has been checked before
std::size_t UTF8StringAdjustedNbBytes(const uint8_t * source, std::size_t max_len) noexcept;

// UTF8RemoveOneAtPos assumes input is valid utf8, zero terminated, that has been checked before
void UTF8RemoveOneAtPos(uint8_t * source, std::size_t len) noexcept;

// UTF8InsertAtPos assumes input is valid utf8, zero terminated, that has been checked before
// UTF8InsertAtPos won't insert anything and return false if modified string buffer does not have enough space to insert
bool UTF8InsertOneAtPos(uint8_t * source, std::size_t len, const uint32_t to_insert_char, std::size_t max_source) noexcept;

// UTF8toUTF16 never writes the trailing zero
std::size_t UTF8toUTF16(const_bytes_view source, uint8_t * target, size_t t_len) noexcept;
std::size_t UTF8toUTF16(const_bytes_view source, bytes_view target) noexcept;
//std::size_t UTF8toUTF16(const uint8_t * source, std::size_t s_len, uint8_t * target, std::size_t t_len);
std::vector<uint8_t> UTF8toUTF16(const_bytes_view source) noexcept;

// UTF8toUTF16 never writes the trailing zero (with Lf to CrLf conversion).
std::size_t UTF8toUTF16_CrLf(const_bytes_view source, uint8_t * target, std::size_t t_len) noexcept;


class UTF8toUnicodeIterator
{
public:
    explicit UTF8toUnicodeIterator(const_byte_ptr str) noexcept;

    UTF8toUnicodeIterator & operator++() noexcept;

    uint32_t operator*() const noexcept
    { return this->ucode; }

    uint32_t code() const noexcept
    { return this->ucode; }

    uint8_t const * pos() const noexcept
    { return this->source; }

private:
    const uint8_t * source;
    uint32_t ucode = 0;
};


// convert UTF16 input to UTF8 returns a new vector containing encoded data
inline std::vector<uint8_t> encode_UTF16_to_UTF8(const_bytes_view utf16_source) noexcept
{
    // TODO: Fix that, it does not handle invalid UTF16 encodings.
    // TODO: Also, we should do something for multiwords UTF16 codes
    // TODO: also it stops if input string containes a 0x0000. Is it what we want ?
    // shouldn't we rather stop when input is finished
    std::vector<uint8_t> target;
    const auto len = utf16_source.size() - (utf16_source.size() & 1u);
    for (size_t i = 0 ; i < len; i += 2){
        uint8_t lo = utf16_source[i];
        uint8_t hi  = utf16_source[i+1];
        if (lo == 0 && hi == 0){
            break;
        }
        if (hi & 0xF8){
            // 3 bytes
            target.push_back(0xE0 | ((hi >> 4) & 0x0F));
            target.push_back(0x80 | ((hi & 0x0F) << 2) | (lo >> 6));
            target.push_back(0x80 | (lo & 0x3F));
        }
        else if (hi || (lo & 0x80)) {
            // 2 bytes
            target.push_back(0xC0 | ((hi << 2) & 0x1C) | ((lo >> 6) & 3));
            target.push_back(0x80 | (lo & 0x3F));
        }
        else {
            target.push_back(lo);
        }
    }
    return target;
}


// Return number of UTF8 bytes used to encode UTF16 input
// do not write trailing 0
std::size_t UTF16toUTF8(const uint8_t * utf16_source, std::size_t utf16_len, uint8_t * utf8_target, std::size_t target_len) noexcept;
// Return number of UTF8 bytes used to encode UTF16 input
// do not write trailing 0
bytes_view UTF16toUTF8_buf(const_bytes_view utf16_source, bytes_view utf8_target) noexcept;

// Return number of UTF8 bytes used to encode UTF16 input
// do not write trailing 0
std::size_t UTF16toUTF8(const uint16_t * utf16_source, std::size_t utf16_len, uint8_t * utf8_target, std::size_t target_len) noexcept;

// Return number of UTF8 bytes used to encode UTF32 input
// do not write trailing 0
std::size_t UTF32toUTF8(const uint8_t * utf32_source, std::size_t utf32_len, uint8_t * utf8_target, std::size_t target_len) noexcept;

// Return number of UTF8 bytes used to encode UTF32 input
// do not write trailing 0
std::size_t UTF32toUTF8(uint32_t utf32_char, uint8_t * utf8_target, std::size_t target_len) noexcept;

// Copy as many characters from source to dest fitting in dest buffer.
// Returns the number of UTF8 characters copied.
// The destination string will always be 0 terminated (dest_size 0 is forbidden)
// The buffer after final 0 is not padded.

std::size_t UTF8ToUTF8LCopy(uint8_t * dest, std::size_t dest_size, const uint8_t * source) noexcept;

size_t UTF8CharNbBytes(const uint8_t * source) noexcept;

bool is_utf8_string(uint8_t const * s, int length = -1) noexcept;

bool is_ASCII_string(const_bytes_view source) noexcept;
bool is_ASCII_string(const_byte_ptr source) noexcept;

std::size_t UTF8StrLenInChar(const_byte_ptr source) noexcept;

std::size_t UTF16toLatin1(const uint8_t * utf16_source_, std::size_t utf16_len, uint8_t * latin1_target, std::size_t latin1_len) noexcept;

std::size_t Latin1toUTF16(const_bytes_view latin1_source,
        uint8_t * utf16_target, std::size_t utf16_len) noexcept;

std::size_t Latin1toUTF8(
    const uint8_t * latin1_source, std::size_t latin1_len,
    uint8_t * utf8_target, std::size_t utf8_len) noexcept;
