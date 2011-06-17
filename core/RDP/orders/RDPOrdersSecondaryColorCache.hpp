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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean

   New RDP Orders Coder / Decoder

*/

#if !defined(__RDPORDERSSECONDARYCOLORCACHE_HPP__)
#define __RDPORDERSSECONDARYCOLORCACHE_HPP__

    /*****************************************************************************/
    // [MS-RDPGDI] 2.2.2.2.1.2.4 Cache Color Table (CACHE_COLOR_TABLE_ORDER)

    // The Cache Color Table Secondary Drawing Order is used by the server
    // to instruct the client to store a color table in a particular Color Table
    // Cache entry. Color tables are used in the MemBlt (section 2.2.2.2.1.1.2.9)
    // and Mem3Blt (section 2.2.2.2.1.1.2.10) Primary Drawing Orders.

    // Support for color table caching is not negotiated in the Color Table Cache
    // Capability Set (section 2.2.1.1), but is instead implied by support for
    // the MemBlt (section 2.2.2.2.1.1.2.9) and Mem3Blt (section 2.2.2.2.1.1.2.10)
    // Primary Drawing Orders. If support for these orders is advertised in the
    // Order Capability Set (see [MS-RDPBCGR] section 2.2.7.1.3), the existence
    // of a color table cache with entries for six palettes is assumed when
    // palettized color is being used, and the Cache Color Table is used to
    // update these palettes.

    // header (6 bytes): A Secondary Order Header, as defined in section
    // 2.2.2.2.1.2.1.1. The embedded orderType field MUST be set to
    // TS_CACHE_COLOR_TABLE (0x01).

    // cacheIndex (1 byte): An 8-bit, unsigned integer. An entry in the Cache
    // Color Table where the color table MUST be stored. This value MUST be in
    // the range 0 to 5 (inclusive).

    // numberColors (2 bytes): A 16-bit, unsigned integer. The number of Color
    // Quad (section 2.2.2.2.1.2.4.1) structures in the colorTable field. This
    // field MUST be set to 256 entries.

    // colorTable (variable): A Color Table composed of an array of Color Quad
    // (section 2.2.2.2.1.2.4.1) structures. The number of entries in the array
    // is given by the numberColors field.

    // 2.2.2.2.1.2.4.1 Color Quad (TS_COLOR_QUAD)
    // The TS_COLOR_QUAD structure is used to express the red, green, and blue
    // components necessary to reproduce a color in the additive RGB space.

    // blue (1 byte): An 8-bit, unsigned integer. The blue RGB color component.

    // green (1 byte): An 8-bit, unsigned integer. The green RGB color component.

    // red (1 byte): An 8-bit, unsigned integer. The red RGB color component.

    // pad1Octet (1 byte): An 8-bit, unsigned integer. Padding. Values in this
    // field are arbitrary and MUST be ignored.

class RDPColCache {

    public:
    uint32_t palette[8][256]; // only 0..5 and 7 (global palette) are used
    uint8_t cacheIndex;

    RDPColCache(uint8_t cacheIndex)
    {
        memset(this->palette, 0, sizeof(palette));
    }

    #warning cacheIndex should be inside instance and array of color cache (patterns) outside color_cache
    void emit(Stream & stream)
    {
        using namespace RDP;

        uint8_t control = STANDARD | SECONDARY;
        stream.out_uint8(control);
        uint16_t len = 1027 - 7;    // length after type minus 7
        stream.out_uint16_le(len);
        stream.out_uint16_le(0);    // flags
        stream.out_uint8(TS_CACHE_COLOR_TABLE); // type

        stream.out_uint8(this->cacheIndex);
        stream.out_uint16_le(256); /* num colors */
        for (int i = 0; i < 256; i++) {
            stream.out_uint32_le(this->palette[this->cacheIndex][i]);
        }
    }

    void receive(Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header)
    {
        using namespace RDP;

        this->cacheIndex = stream.in_uint8();
        assert(this->cacheIndex >= 0 && this->cacheIndex < 6);

        uint16_t numberColors = stream.in_uint16_le();
        assert(numberColors == 256);

        for (size_t i = 0; i < 256; i++) {
            this->palette[this->cacheIndex][i] = stream.in_uint32_le();
        }
    }

    #define warning remove printf in operator== and show palette differences in test code
    bool operator==(const RDPColCache & other) const {
        for (uint8_t index = 0; index < 6 ; ++index){
            for (size_t i = 0; i < 256 ; ++i){
                if (this->palette[index][i] != other.palette[index][i]){
                    printf("palette differs at index %d: %x != %x\n",
                        (int)i, this->palette[index][i], other.palette[index][i]);
                    return false;
                }
            }
        }
        return true;
    }

    size_t str(char * buffer, size_t sz) const
    {
        size_t lg  = snprintf(
            buffer,
            sz,
            "RDPColCache("
            "[%d, %d, %d,...]"
            "[%d, %d, %d,...]"
            "[%d, %d, %d,...]"
            "[%d, %d, %d,...]"
            "[%d, %d, %d,...]"
            "[%d, %d, %d,...])\n",
            this->palette[0][0], this->palette[0][1], this->palette[0][2],
            this->palette[1][0], this->palette[1][1], this->palette[1][2],
            this->palette[2][0], this->palette[2][1], this->palette[2][2],
            this->palette[3][0], this->palette[3][1], this->palette[3][2],
            this->palette[4][0], this->palette[4][1], this->palette[4][2],
            this->palette[5][0], this->palette[5][1], this->palette[5][2]);
        if (lg >= sz){
            return sz;
        }
        return lg;
    }

};


#endif
