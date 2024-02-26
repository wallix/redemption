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

#ifndef PPOCR_SRC_UTILS_UTF_HPP
#define PPOCR_SRC_UTILS_UTF_HPP

namespace ppocr { namespace utf {
// TODO redemption classe
    class UTF8toUnicodeIterator
    {
        const uint8_t * source;
        uint32_t ucode = 0;


    public:
        UTF8toUnicodeIterator(const uint8_t * str)
        : source(str)
        { ++*this; }

        UTF8toUnicodeIterator(const char * str)
        : UTF8toUnicodeIterator(reinterpret_cast<const uint8_t*>(str))
        {}

        UTF8toUnicodeIterator & operator++()
        {
            this->ucode = *source;
            ++source;
            switch (this->ucode >> 4 ){
                case 0:
                case 1: case 2: case 3:
                case 4: case 5: case 6: case 7:
                break;
                /* handle U+0080..U+07FF inline : 2 bytes sequences */
                case 0xC: case 0xD:
                    this->ucode = ((this->ucode & 0x1F) << 6)|(source[0] & 0x3F);
                    source += 1;
                break;
                /* handle U+8FFF..U+FFFF inline : 3 bytes sequences */
                case 0xE:
                    this->ucode = ((this->ucode & 0x0F) << 12)|((source[0] & 0x3F) << 6)|(source[1] & 0x3F);
                    source += 2;
                break;
                case 0xF:
                    this->ucode = ((this->ucode & 0x07) << 18)|((source[0] & 0x3F) << 12)|((source[1] & 0x3F) << 6)|(source[2] & 0x3F);
                    source += 3;
                break;
                // these should never happen on valid UTF8
                case 8: case 9: case 0x0A: case 0x0B:
                    ucode = 0;
                break;
            }
            return *this;
        }

        uint32_t operator*() const
        { return this->ucode; }

        uint32_t code() const
        { return this->ucode; }

        uint8_t const * pos() const
        { return this->source; }
    };

    class UTF8Iterator
    {
        const uint8_t * source;
        uint32_t ucode = 0;

    public:
        UTF8Iterator(const uint8_t * str)
        : source(str)
        { ++*this; }

        UTF8Iterator(const char * str)
        : UTF8Iterator(reinterpret_cast<const uint8_t*>(str))
        {}

        UTF8Iterator & operator++()
        {
            this->ucode = *source;
            ++source;
            switch (this->ucode >> 4 ){
                case 0:
                case 1: case 2: case 3:
                case 4: case 5: case 6: case 7:
                break;
                /* handle U+0080..U+07FF inline : 2 bytes sequences */
                case 0xC: case 0xD:
                    this->ucode = (this->ucode << 8) | source[0];
                    source += 1;
                break;
                /* handle U+8FFF..U+FFFF inline : 3 bytes sequences */
                case 0xE:
                    this->ucode = (this->ucode << 16) | (source[0] << 8) | source[1];
                    source += 2;
                break;
                case 0xF:
                    this->ucode = (this->ucode << 24) | (source[0] << 16) | (source[1] << 8) | source[2];
                    source += 3;
                break;
                // these should never happen on valid UTF8
                default: //case 8: case 9: case 0x0A: case 0x0B:
                    ucode = 0;
                break;
            }
            return *this;
        }

        uint32_t operator*() const
        { return this->ucode; }

        uint32_t code() const
        { return this->ucode; }

        uint8_t const * pos() const
        { return this->source; }
    };
} }

#endif
