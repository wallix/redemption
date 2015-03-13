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
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CORE_RDP_ORDERS_ALTERNATESECONDARYWINDOWING_HPP_
#define _REDEMPTION_CORE_RDP_ORDERS_ALTERNATESECONDARYWINDOWING_HPP_

#include "cast.hpp"
#include "log.hpp"
#include "noncopyable.hpp"
#include "stream.hpp"

namespace RDP {

namespace RAIL {

// [MS-RDPERP] - 2.2.1.2.2 Rectangle (TS_RECTANGLE_16)
// ===================================================

// The TS_RECTANGLE_16 structure describes a rectangle by using its top-left
//  and bottom-right coordinates. The units depend on the context in which
//  this structure is used.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |              Left             |              Top              |
// +-------------------------------+-------------------------------+
// |             Right             |             Bottom            |
// +-------------------------------+-------------------------------+

// Left (2 bytes): An unsigned 16-bit integer. The x-coordinate of the
//  rectangle's top-left corner.

// Top (2 bytes): An unsigned 16-bit integer. The y-coordinate of the
//  rectangle's top-left corner.

// Right (2 bytes): An unsigned 16-bit integer. The x-coordinate of the
//  rectangle's bottom-right corner.

// Bottom (2 bytes): An unsigned 16-bit integer. The y-coordinate of the
//  rectangle's bottom-right corner.

class Rectangle {
    uint16_t Left   = 0;
    uint16_t Top    = 0;
    uint16_t Right  = 0;
    uint16_t Bottom = 0;

public:
    inline void emit(Stream & stream) const {
        stream.out_uint16_le(this->Left);
        stream.out_uint16_le(this->Top);
        stream.out_uint16_le(this->Right);
        stream.out_uint16_le(this->Bottom);
    }

    void receive(Stream & stream) {
        {
            const unsigned expected =
                8;  // Left(2) + Top(2) + Right(2) + Bottom(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Rectangle: expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->Left   = stream.in_uint16_le();
        this->Top    = stream.in_uint16_le();
        this->Right  = stream.in_uint16_le();
        this->Bottom = stream.in_uint16_le();
    }

    inline static size_t size() {
        return 8;   /* Left(2) + Top(2) + Right(2) + Bottom(2) */
    }

    inline size_t str(char * buffer, size_t size) const {
        const size_t length =
            ::snprintf(buffer, size,
                       "(Left=%u Top=%u Right=%u Bottom=%u)",
                       this->Left, this->Top, this->Right, this->Bottom);
        return ((length < size) ? length : size - 1);
    }
};

// [MS-RDPEGDI] - 2.2.1.2.3 Icon Info (TS_ICON_INFO)
// =================================================

// The TS_ICON_INFO packet describes an icon.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           CacheEntry          |    CacheId    |      Bpp      |
// +-------------------------------+---------------+---------------+
// |             Width             |             Height            |
// +-------------------------------+-------------------------------+
// |    CbColorTable (optional)    |           CbBitsMask          |
// +-------------------------------+-------------------------------+
// |          CbBitsColor          |      BitsMask (variable)      |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                     ColorTable (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                      BitsColor (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// CacheEntry (2 bytes): An unsigned 16-bit integer. The index within an icon
//  cache at which this icon MUST be stored at the client. The index is
//  unique within a given CacheId (see following description). The maximum
//  value of CacheEntry is negotiated between server and client through the
//  NumIconCacheEntries field of the Window List Capability Set during the
//  connection establishment phase.

// CacheId (1 byte): An unsigned 8-bit integer. The index of the icon cache
//  at which this icon MUST be stored at the client. If the value is 0xFFFF,
//  the icon SHOULD NOT be cached. The CacheId is unique within a remote
//  session.

//  The maximum value of CacheId is negotiated between server and client
//  through the NumIconCaches field of the Window List Capability Set while
//  establishing the connection.

// Bpp (1 byte): An unsigned 8-bit integer. The color depth of the icon.
//  Valid values are as follows:

//  1
//  4
//  8
//  16
//  24
//  32

// Width (2 bytes): An unsigned 16-bit integer. The width, in pixels, of the
//  icon.

// Height (2 bytes): An unsigned 16-bit integer. The height, in pixels, of
//  the icon.

// CbColorTable (2 bytes): An unsigned 16-bit integer. The size, in bytes, of
//  the color table data. This field is ONLY present if the bits per pixel
//  (Bpp) value is 1, 4, or 8.

// CbBitsMask (2 bytes): An unsigned 16-bit integer. The size, in bytes, of
//  the icon's one-bit color-depth mask image.

// CbBitsColor (2 bytes): An unsigned 16-bit integer. The size, in bytes, of
//  the icon's color image.

// BitsMask (variable): The image data for the 1-bpp bitmap. The length, in
//  bytes, of this field is equal to the value of CbBitsMask. This field is
//  optional.

// ColorTable (variable): The image data for the color bitmap. The length, in
//  bytes, of this field is equal to the value of CbColorTable. This field is
//  only present if the Bpp value is 1, 4, or 8.

// BitsColor (variable): The image data for the icon's color image. The
//  length, in bytes, of this field is equal to the value of CbBitsColor.
//  This field is optional.

class IconInfo {
    uint16_t CacheEntry = 0;
    uint8_t  CacheId    = 0;
    uint8_t  Bpp        = 0;
    uint16_t Width      = 0;
    uint16_t Height     = 0;

    StaticStream bits_mask;
    StaticStream color_table;
    StaticStream bits_color;

public:
    void emit(Stream & stream) const {
        stream.out_uint16_le(this->CacheEntry);
        stream.out_uint16_le(this->CacheId);

        stream.out_uint8(this->Bpp);

        stream.out_uint16_le(this->Width);
        stream.out_uint16_le(this->Height);

        if ((this->Bpp == 1) || (this->Bpp == 4) || (this->Bpp == 8)) {
            stream.out_uint16_le(this->color_table.get_capacity());
        }

        stream.out_uint16_le(this->bits_mask.get_capacity());
        stream.out_uint16_le(this->bits_color.get_capacity());

        stream.out_copy_bytes(this->bits_mask.get_data(), this->bits_mask.get_capacity());
        stream.out_copy_bytes(this->color_table.get_data(), this->color_table.get_capacity());
        stream.out_copy_bytes(this->bits_color.get_data(), this->bits_color.get_capacity());
    }

    void receive(Stream & stream) {
        {
            const unsigned expected =
                9;  // CacheEntry(2) + CacheId(2) + Bpp(1) + Width(2) + Height(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated IconInfo (0): expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->CacheEntry = stream.in_uint16_le();
        this->CacheId    = stream.in_uint16_le();

        this->Bpp = stream.in_uint8();

        this->Width  = stream.in_uint16_le();
        this->Height = stream.in_uint16_le();

        if ((this->Bpp == 1) || (this->Bpp == 4) || (this->Bpp == 8)) {
            const unsigned expected = 2;  // CbColorTable(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated IconInfo (1): expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        const uint16_t CbColorTable =
            (((this->Bpp == 1) || (this->Bpp == 4) || (this->Bpp == 8)) ?
             stream.in_uint16_le() : 0);

        {
            const unsigned expected = 4;  // CbBitsMask(2) + CbBitsColor(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated IconInfo (2): expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        const uint16_t CbBitsMask   = stream.in_uint16_le();
        const uint16_t CbBitsColor  = stream.in_uint16_le();

        {
            const unsigned expected = CbColorTable +    // BitsMask(variable)
                                      CbBitsMask +      // ColorTable(variable)
                                      CbBitsColor;      // BitsColor(variable)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated IconInfo (3): expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->bits_mask.resize(stream.p, CbBitsMask);
        stream.in_skip_bytes(CbBitsMask);

        this->color_table.resize(stream.p, CbColorTable);
        stream.in_skip_bytes(CbColorTable);

        this->bits_color.resize(stream.p, CbBitsColor);
        stream.in_skip_bytes(CbBitsColor);
    }

    inline size_t size() const {
        return 9 +  // CacheEntry(2) + CacheId(2) + Bpp(1) + Width(2) + Height(2)
            (((this->Bpp == 1) || (this->Bpp == 4) || (this->Bpp == 8)) ? 2 /* CbColorTable(2) */ : 0) +
            4 + // CbBitsMask(2) + CbBitsColor(2)
            this->bits_mask.get_capacity() +
            (((this->Bpp == 1) || (this->Bpp == 4) || (this->Bpp == 8)) ? this->color_table.get_capacity() : 0) +
            this->bits_color.get_capacity();
    }

    inline size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        const size_t result = ::snprintf(
            buffer + length, size - length,
            "(CacheEntry=%u CacheId=%u Bpp=%u Width=%u Height=%u",
            this->CacheEntry, this->CacheId, this->Bpp,
            this->Width, this->Height);
        length += (
                   (result < (size - length)) ?
                   result :
                   ((size - length) - 1)
                  );

        auto str_optional = [&length, buffer, size] (Stream const & optional, const char * label) {
                if (optional.get_capacity()) {
                    const size_t result = ::snprintf(
                        buffer + length, size - length,
                        " %s=%llu", label, optional.get_capacity());
                    length += (
                               (result < (size - length)) ?
                               result :
                               ((size - length) - 1)
                              );
                }
            };

        str_optional(this->color_table, "CbColorTable");
        str_optional(this->bits_mask,   "CbBitsMask");
        str_optional(this->bits_color,  "CbBitsColor");

        return length;
    }
};  // IconInfo

// [MS-RDPERP] - 2.2.1.2.4 Cached Icon Info (TS_CACHED_ICON_INFO)
// ==============================================================

// The TS_CACHED_ICON_INFO packet describes a cached icon.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           CacheEntry          |    CacheId    |
// +-------------------------------+---------------+

// CacheEntry (2 bytes): An unsigned 16-bit integer. The index within an icon
//  cache at the client that refers to the cached icon. This value MUST have
//  been previously specified by the server in the Icon Info structure
//  (section 2.2.1.2.3) of a Window Information Order (section 2.2.1.3.1) or
//  Icon structure of a New or Existing Notification Icon (section
//  2.2.1.3.2.2.1).

// CacheId (1 byte): An unsigned 8-bit integer. The index of the icon cache
//  containing the cached icon. This value MUST have been previously
//  specified by the server in the Icon Info structure of a Window
//  Information Order or Icon structure of a New or Existing Notification
//  Icon.

class CachedIconInfo {
    uint16_t CacheEntry = 0;
    uint8_t  CacheId    = 0;

public:
    inline void emit(Stream & stream) const {
        stream.out_uint16_le(this->CacheEntry);
        stream.out_uint16_le(this->CacheId);
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected = 4;  // CacheEntry(2) + CacheId(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated CachedIconInfo: expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->CacheEntry = stream.in_uint16_le();
        this->CacheId    = stream.in_uint16_le();
    }

    inline static size_t size() {
        return 4;   // CacheEntry(2) + CacheId(2)
    }

    inline size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        const size_t result = ::snprintf(
            buffer + length, size - length,
            "(CacheEntry=%u CacheId=%u)",
            this->CacheEntry, this->CacheId);
        length += (
                   (result < (size - length)) ?
                   result :
                   ((size - length) - 1)
                  );

        return length;
    }
};

// [MS-RDPERP] - 2.2.1.3.1.1 Common Header (TS_WINDOW_ORDER_HEADER)
// ================================================================

// The TS_WINDOW_ORDER_HEADER packet contains information common to every
//  Windowing Alternate Secondary Drawing Order describing a window.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     Header    |           OrderSize           | FieldsPresent |
// |               |                               |     Flags     |
// +---------------+-------------------------------+---------------+
// |                      ...                      |    WindowId   |
// +---------------+-------------------------------+---------------+
// |                      ...                      |
// +-----------------------------------------------+

// Header (1 byte): An unsigned 8-bit integer. An Alternate Secondary Order
//  Header, as specified in [MS-RDPEGDI] section 2.2.2.2.1.3.1.1. The
//  embedded orderType field MUST be set to 0x0B (TS_ALTSEC_WINDOW).

// OrderSize (2 bytes): An unsigned 16-bit integer. The size of the entire
//  packet, in bytes.

// FieldsPresentFlags (4 bytes): An unsigned 32-bit integer. The flags
//  indicating which fields are present in the packet. See Orders.

// WindowId (4 bytes): An unsigned 32-bit integer. The ID of the window being
//  described in the drawing order. It is generated by the server and is
//  unique for every window in the session.

class WindowInformationCommonHeader {
    mutable uint16_t OrderSize           = 0;
            uint32_t FieldsPresentFlags_ = 0;
            uint32_t WindowId            = 0;

    mutable uint32_t   offset_of_OrderSize = 0;
    mutable Stream   * output_stream       = nullptr;

protected:
    //inline void AddFieldsPresentFlags(uint32_t FieldsPresentFlagsToAdd) {
    //    this->FieldsPresentFlags_ |= FieldsPresentFlagsToAdd;
    //}

    //inline void RemoveFieldsPresentFlags(uint32_t FieldsPresentFlagsToRemove) {
    //    this->FieldsPresentFlags_ &= ~FieldsPresentFlagsToRemove;
    //}

    inline void emit_begin(Stream & stream) const {
        REDASSERT(this->output_stream == nullptr);

        this->output_stream = &stream;

        this->offset_of_OrderSize = stream.get_offset();
        stream.out_skip_bytes(2); // OrderSize(2)

        stream.out_uint32_le(this->FieldsPresentFlags_);
        stream.out_uint32_le(this->WindowId);

        stream.mark_end();
    }

    inline void emit_end() const {
        REDASSERT(this->output_stream != nullptr);

        this->output_stream->set_out_uint16_le(
            this->output_stream->get_offset() - this->offset_of_OrderSize,
            this->offset_of_OrderSize);
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected =
                10;  // OrderSize(2) + FieldsPresentFlags(4) + WindowId(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Window Information Common Header: "
                        "expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->OrderSize           = stream.in_uint16_le();
        this->FieldsPresentFlags_ = stream.in_uint32_le();
        this->WindowId            = stream.in_uint32_le();
    }

    inline static size_t size() {
        return 6;   // OrderSize(2) + FieldsPresentFlags(2) + WindowId(2)
    }

    inline size_t str(char * buffer, size_t size) const {
        const size_t length =
            ::snprintf(buffer, size,
                       "(OrderSize=%u FieldsPresentFlags=0x%08X WindowId=%u)",
                       this->OrderSize, this->FieldsPresentFlags_,
                       this->WindowId);
        return ((length < size) ? length : size - 1);
    }

public:
    inline uint32_t FieldsPresentFlags() const { return this->FieldsPresentFlags_; }
};  // WindowInformationCommonHeader

// [MS-RDPERP] - 2.2.1.3.1.2.1 New or Existing Window
// ==================================================

// A Window Information Order is generated by the server whenever a new
//  window is created on the server or when a property on a new or existing
//  window is updated.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-----------------------------------------------+---------------+
// |                      ...                      | OwnerWindowId |
// |                                               |   (optional)  |
// +-----------------------------------------------+---------------+
// |                      ...                      |     Style     |
// |                                               |   (optional)  |
// +-----------------------------------------------+---------------+
// |                      ...                      | ExtendedStyle |
// |                                               |   (optional)  |
// +-----------------------------------------------+---------------+
// |                      ...                      |   ShowState   |
// |                                               |   (optional)  |
// +-----------------------------------------------+---------------+
// |                      TitleInfo (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                    ClientOffsetX (optional)                   |
// +---------------------------------------------------------------+
// |                    ClientOffsetY (optional)                   |
// +---------------------------------------------------------------+
// |                   ClientAreaWidth (optional)                  |
// +---------------------------------------------------------------+
// |                  ClientAreaHeight (optional)                  |
// +---------------+-----------------------------------------------+
// |   RPContent   |          RootParentHandle (optional)          |
// |   (optional)  |                                               |
// +---------------+-----------------------------------------------+
// |      ...      |            WindowOffsetX (optional)           |
// +---------------+-----------------------------------------------+
// |      ...      |            WindowOffsetY (optional)           |
// +---------------+-----------------------------------------------+
// |      ...      |         WindowClientDeltaX (optional)         |
// +---------------+-----------------------------------------------+
// |      ...      |         WindowClientDeltaY (optional)         |
// +---------------+-----------------------------------------------+
// |      ...      |             WindowWidth (optional)            |
// +---------------+-----------------------------------------------+
// |      ...      |             WindowHeight (optional)           |
// +---------------+-------------------------------+---------------+
// |      ...      |   NumWindowRects (optional)   |  WindowRects  |
// |               |                               |   (variable)  |
// +---------------+-------------------------------+---------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                   VisibleOffsetX (optional)                   |
// +---------------------------------------------------------------+
// |                   VisibleOffsetY (optional)                   |
// +-------------------------------+-------------------------------+
// | NumVisibilityRects (optional) |   VisibilityRects (variable)  |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Hdr (11 bytes): Eleven bytes. Common Window AltSec Order header,
//  TS_WINDOW_ORDER_HEADER. The FieldsPresentFlags field of the header MUST
//  conform to the values defined as follows.

//  +-------------------------------------+------------------------------------+
//  | Value                               | Meaning                            |
//  +-------------------------------------+------------------------------------+
//  | 0x01000000                          | Indicates a Windowing Alternate    |
//  | WINDOW_ORDER_TYPE_WINDOW            | Secondary Drawing Order            |
//  |                                     | describing a window. This flag     |
//  |                                     | MUST be set.                       |
//  +-------------------------------------+------------------------------------+
//  | 0x10000000                          | Indicates that the Windowing       |
//  | WINDOW_ORDER_STATE_NEW              | Alternate Secondary Drawing Order  |
//  |                                     | contains information for a new     |
//  |                                     | window. If this flag is not set,   |
//  |                                     | the order contains information for |
//  |                                     | an existing window.                |
//  +-------------------------------------+------------------------------------+
//  | 0x00000002                          | Indicates that the OwnerWindowId   |
//  | WINDOW_ORDER_FIELD_OWNER            | field is present.                  |
//  +-------------------------------------+------------------------------------+
//  | 0x00000008                          | Indicates that the Style and       |
//  | WINDOW_ORDER_FIELD_STYLE            | ExtendedStyle fields are present.  |
//  +-------------------------------------+------------------------------------+
//  | 0x00000010                          | Indicates that the ShowState field |
//  | WINDOW_ORDER_FIELD_SHOW             | is present.                        |
//  +-------------------------------------+------------------------------------+
//  | 0x00000004                          | Indicates that the TitleInfo field |
//  | WINDOW_ORDER_FIELD_TITLE            | is present.                        |
//  +-------------------------------------+------------------------------------+
//  | 0x00004000                          | Indicates that the ClientOffsetX   |
//  | WINDOW_ORDER_FIELD_CLIENTAREAOFFSET | and ClientOffsetY fields are       |
//  |                                     | present.                           |
//  +-------------------------------------+------------------------------------+
//  | 0x00010000                          | Indicates that the ClientAreaWidth |
//  | WINDOW_ORDER_FIELD_CLIENTAREASIZE   | and ClientAreaHeight fields are    |
//  |                                     | present.<3>                        |
//  +-------------------------------------+------------------------------------+
//  | 0x00020000                          | Indicates that the RPContent field |
//  | WINDOW_ORDER_FIELD_RPCONTENT        | is present. <4>                    |
//  +-------------------------------------+------------------------------------+
//  | 0x00040000                          | Indicates that the                 |
//  | WINDOW_ORDER_FIELD_ROOTPARENT       | RootParentHandle field is present. |
//  |                                     | <5>                                |
//  +-------------------------------------+------------------------------------+
//  | 0x00000800                          | Indicates that the WindowOffsetX   |
//  | WINDOW_ORDER_FIELD_WNDOFFSET        | and WindowOffsetY fields are       |
//  |                                     | present.                           |
//  +-------------------------------------+------------------------------------+
//  | 0x00008000                          | Indicates that the                 |
//  | WINDOW_ORDER_FIELD_WNDCLIENTDELTA   | WindowClientDeltaX and             |
//  |                                     | WindowClientDeltaY fields are      |
//  |                                     | present.                           |
//  +-------------------------------------+------------------------------------+
//  | 0x00000400                          | Indicates that the WindowWidth and |
//  | WINDOW_ORDER_FIELD_WNDSIZE          | WindowHeight fields are present.   |
//  +-------------------------------------+------------------------------------+
//  | 0x00000100                          | Indicates that the NumWindowRects  |
//  | WINDOW_ORDER_FIELD_WNDRECTS         | and WindowRects fields are         |
//  |                                     | present.                           |
//  +-------------------------------------+------------------------------------+
//  | 0x00001000                          | Indicates that the VisibleOffsetX  |
//  | WINDOW_ORDER_FIELD_VISOFFSET        | and VisibleOffsetY fields are      |
//  |                                     | present.                           |
//  +-------------------------------------+------------------------------------+
//  | 0x00000200                          | Indicates that the                 |
//  | WINDOW_ORDER_FIELD_VISIBILITY       | NumVisibilityRects and             |
//  |                                     | VisibilityRects fields are         |
//  |                                     | present.                           |
//  +-------------------------------------+------------------------------------+

enum {
      WINDOW_ORDER_TYPE_WINDOW            = 0x01000000
    , WINDOW_ORDER_STATE_NEW              = 0x10000000
    , WINDOW_ORDER_FIELD_OWNER            = 0x00000002
    , WINDOW_ORDER_FIELD_STYLE            = 0x00000008
    , WINDOW_ORDER_FIELD_SHOW             = 0x00000010
    , WINDOW_ORDER_FIELD_TITLE            = 0x00000004
    , WINDOW_ORDER_FIELD_CLIENTAREAOFFSET = 0x00004000
    , WINDOW_ORDER_FIELD_CLIENTAREASIZE   = 0x00010000
    , WINDOW_ORDER_FIELD_RPCONTENT        = 0x00020000
    , WINDOW_ORDER_FIELD_ROOTPARENT       = 0x00040000
    , WINDOW_ORDER_FIELD_WNDOFFSET        = 0x00000800
    , WINDOW_ORDER_FIELD_WNDCLIENTDELTA   = 0x00008000
    , WINDOW_ORDER_FIELD_WNDSIZE          = 0x00000400
    , WINDOW_ORDER_FIELD_WNDRECTS         = 0x00000100
    , WINDOW_ORDER_FIELD_VISOFFSET        = 0x00001000
    , WINDOW_ORDER_FIELD_VISIBILITY       = 0x00000200
};

// OwnerWindowId (4 bytes): An unsigned 32-bit integer. The ID of the window
//  on the server that is the owner of the window specified in WindowId field
//  of Hdr. For more information on owned windows, see [MSDN-WINFEATURE].
//  This field is present if and only if the WINDOW_ORDER_FIELD_OWNER flag is
//  set in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

// Style (4 bytes): An unsigned 32-bit integer. Describes the window's
//  current style. Window styles determine the appearance and behavior of a
//  window. For more information, see [MSDN-WINSTYLE]. This field is present
//  if and only if the WINDOW_ORDER_FIELD_STYLE flag is set in the
//  FieldsPresentFlags field of the TS_WINDOW_ORDER_HEADER.

// ExtendedStyle (4 bytes): An unsigned 32-bit integer. Extended window style
//  information. For more information about extended window styles, see
//  [MSDN-CREATEWINEX].

//  This field is present if and only if the WINDOW_ORDER_FIELD_STYLE flag is
//  set in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

// ShowState (1 byte): An unsigned 8-bit integer. Describes the show state of
//  the window.

//  This field is present if and only if the WINDOW_ORDER_FIELD_SHOW flag is
//  set in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

//  The field MUST be one of the following values.

//  +-------+---------------------------------------------------+
//  | Value | Meaning                                           |
//  +-------+---------------------------------------------------+
//  | 0x00  | Do not show the window.                           |
//  +-------+---------------------------------------------------+
//  | 0x02  | Show the window minimized.                        |
//  +-------+---------------------------------------------------+
//  | 0x03  | Show the window maximized.                        |
//  +-------+---------------------------------------------------+
//  | 0x05  | Show the window in its current size and position. |
//  +-------+---------------------------------------------------+

// TitleInfo (variable): UNICODE_STRING. Variable length. Contains the
//  window's title string. The maximum value for the CbString field of
//  UNICODE_STRING is 520 bytes. This structure is present only if the
//  WINDOW_ORDER_FIELD_TITLE flag is set in the FieldsPresentFlags field of
//  TS_WINDOW_ORDER_HEADER.

// ClientOffsetX (4 bytes): A 32-bit signed integer. The X (horizontal)
//  offset from the top-left corner of the screen to the top-left corner of
//  the window's client area, expressed in screen coordinates.

//  This field is present only if the WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
//  flag is set in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

// ClientOffsetY (4 bytes): A 32-bit signed integer. The Y (vertical) offset
//  from the top-left corner of the screen to the top-left corner of the
//  window's client area, expressed in screen coordinates.

//  This field is present only if the WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
//  flag is set in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

// ClientAreaWidth (4 bytes): An unsigned 32-bit integer specifying the width
//  of the client area rectangle of the target window.

//  This field only appears if the WndSupportLevel field of the Window List
//  Capability Set message is set to TS_WINDOW_LEVEL_SUPPORTED_EX (as
//  specified in section 2.2.1.1.2) and the WINDOW_ORDER_FIELD_CLIENTAREASIZE
//  flag is set in the FieldsPresentFlags field of the TS_WINDOW_ORDER_HEADER
//  packet (section 2.2.1.3.1.1).

// ClientAreaHeight (4 bytes): An unsigned 32-bit integer specifying the
//  height of the client area rectangle of the target window.

//  This field only appears if the WndSupportLevel field of the Window List
//  Capability Set message is set to TS_WINDOW_LEVEL_SUPPORTED_EX (as
//  specified in section 2.2.1.1.2) and the Hdr field has the
//  WINDOW_ORDER_FIELD_CLIENTAREASIZE flag is set in the FieldsPresentFlags
//  field of the TS_WINDOW_ORDER_HEADER packet (section 2.2.1.3.1.1).

// RPContent (1 byte): An unsigned BYTE that MUST be set to one of the
//  following possible values.

//  +-------+--------------------------------------------------------------+
//  | Value | Meaning                                                      |
//  +-------+--------------------------------------------------------------+
//  | 0x00  | The window is not used by a render plug-in to do client-side |
//  |       | rendering.                                                   |
//  +-------+--------------------------------------------------------------+
//  | 0x01  | The window is used by a render plug-in to do client-side     |
//  |       | rendering.                                                   |
//  +-------+--------------------------------------------------------------+

//  This field only appears if the WndSupportLevel field of the Window List
//  Capability Set message is set to TS_WINDOW_LEVEL_SUPPORTED_EX (as
//  specified in section 2.2.1.1.2) and the Hdr field has the
//  WINDOW_ORDER_FIELD_RPCONTENT flag is set in the FieldsPresentFlags field
//  of the TS_WINDOW_ORDER_HEADER packet (section 2.2.1.3.1.1).

// RootParentHandle (4 bytes): An unsigned 32-bit integer specifying the
//  server-side target window's top-level parent window handle. A Top-Level
//  parent window is the window immediately below "desktop" in the window
//  hierarchy. If the target window is a top-level window, the window handle
//  of the target window is sent.

//  This field only appears if the WndSupportLevel field of the Window List
//  Capability Set message is set to TS_WINDOW_LEVEL_SUPPORTED_EX (as
//  specified in section 2.2.1.1.2) and the Hdr field has the
//  WINDOW_ORDER_FIELD_ROOTPARENT flag is set in the FieldsPresentFlags
//  field of the TS_WINDOW_ORDER_HEADER packet (section 2.2.1.3.1.1).

// WindowOffsetX (4 bytes): A 32-bit signed integer. The X (horizontal)
//  offset from the top-left corner of the window to the top-left corner of
//  the window's client area, expressed in screen coordinates.

//  This field is present only if the WINDOW_ORDER_FIELD_WNDOFFSET flag is
//  set in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

// WindowOffsetY (4 bytes): A 32-bit signed integer. The Y (vertical) offset
//  from the top-left corner of the window to the top-left corner of the
//  window's client area, expressed in screen coordinates.

//  This field is present only if the WINDOW_ORDER_FIELD_WNDOFFSET flag is
//  set in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

// WindowClientDeltaX (4 bytes): A 32-bit signed integer. The X (horizontal)
//  delta between the top-left corner of the window and the window's client
//  area.

//  This field is present only if the WINDOW_ORDER_FIELD_CLIENTDELTA flag is
//  set in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

// WindowClientDeltaY (4 bytes): A 32-bit signed integer. The Y (vertical)
//  delta between the top-left corner of the window and the window's client
//  area.

//  This field is present only if the WINDOW_ORDER_FIELD_CLIENTDELTA flag is
//  set in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

// WindowWidth (4 bytes): An unsigned 32-bit integer. The window width, in
//  screen coordinates.

//  This field is present only if the WINDOW_ORDER_FIELD_WNDSIZE flag is set
//  in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

// WindowHeight (4 bytes): An unsigned 32-bit integer. The window height, in
//  screen coordinates.

//  This field is present only if the WINDOW_ORDER_FIELD_WNDSIZE flag is set
//  in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

// NumWindowRects (2 bytes): An unsigned 16-bit integer. A count of
//  rectangles describing the window geometry.

//  This field is present only if the WINDOW_ORDER_FIELD_WNDRECTS flag is set
//  in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

// WindowRects (variable): An array of TS_RECTANGLE_16 structures,
//  NumWindowRects wide, describing the window geometry. All coordinates are
//  window coordinates.

//  This field is present only if the NumWindowRects field is greater than 0
//  and the WINDOW_ORDER_FIELD_WNDRECTS flag is set in the FieldsPresentFlags
//  field of TS_WINDOW_ORDER_HEADER.

// VisibleOffsetX (4 bytes): A 32-bit signed integer. The X (horizontal)
//  offset from the top-left corner of the screen to the top-left corner of
//  the window visible region's bounding rectangle, expressed in screen
//  coordinates.

//  This field is present only if the WINDOW_ORDER_FIELD_VISOFFSET flag is
//  set in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

// VisibleOffsetY (4 bytes): A 32-bit signed integer. The Y (vertical) offset
//  from the top-left corner of the screen to the top-left corner of the
//  window visible region's bounding rectangle, expressed in screen
//  coordinates.

//  This field is present only if the WINDOW_ORDER_FIELD_VISOFFSET flag is
//  set in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

// NumVisibilityRects (2 bytes): An unsigned 16-bit integer. A count of
//  rectangles describing the window visible region.

//  This field is present only if the WINDOW_ORDER_FIELD_VISIBILITY flag is
//  set in the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

// VisibilityRects (variable): An array of TS_RECTANGLE_16 structures,
//  NumVisibilityRects wide, describing the window visible region. All
//  coordinates are window coordinates.

//  This field is present only if the value of the NumVisibilityRects field
//  is greater than 0 and the WINDOW_ORDER_FIELD_VISIBILITY flag is set in
//  the FieldsPresentFlags field of TS_WINDOW_ORDER_HEADER.

class NewOrExistingWindow : public WindowInformationCommonHeader {
    uint32_t OwnerWindowId = 0;
    uint32_t Style         = 0;
    uint32_t ExtendedStyle = 0;
    uint8_t  ShowState     = 0;

    std::string title_info;

    int32_t ClientOffsetX = 0;
    int32_t ClientOffsetY = 0;

    uint32_t ClientAreaWidth  = 0;
    uint32_t ClientAreaHeight = 0;

    uint8_t RPContent = 0;

    uint32_t RootParentHandle = 0;

    int32_t WindowOffsetX = 0;
    int32_t WindowOffsetY = 0;

    int32_t WindowClientDeltaX = 0;
    int32_t WindowClientDeltaY = 0;

    uint32_t WindowWidth  = 0;
    uint32_t WindowHeight = 0;

    uint16_t NumWindowRects = 0;

    std::vector<Rectangle> window_rects;

    int32_t VisibleOffsetX = 0;
    int32_t VisibleOffsetY = 0;

    uint16_t NumVisibilityRects = 0;

    std::vector<Rectangle> visibility_rects;

public:
    void emit(Stream & stream) const {
        WindowInformationCommonHeader::emit_begin(stream);

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_OWNER) {
            stream.out_uint32_le(this->OwnerWindowId);
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_STYLE) {
            stream.out_uint32_le(this->Style);
            stream.out_uint32_le(this->ExtendedStyle);
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_SHOW) {
            stream.out_uint8(this->ShowState);
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_TITLE) {
            const size_t maximum_length_of_TitleInfo_in_bytes = this->title_info.length() * 2;

            uint8_t * const unicode_data = static_cast<uint8_t *>(::alloca(
                        maximum_length_of_TitleInfo_in_bytes));
            const size_t size_of_unicode_data = ::UTF8toUTF16(
                reinterpret_cast<const uint8_t *>(this->title_info.c_str()), unicode_data,
                maximum_length_of_TitleInfo_in_bytes);

            stream.out_uint16_le(size_of_unicode_data);

            stream.out_copy_bytes(unicode_data, size_of_unicode_data);
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_CLIENTAREAOFFSET) {
            stream.out_uint32_le(this->ClientOffsetX);
            stream.out_uint32_le(this->ClientOffsetY);
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_CLIENTAREASIZE) {
            stream.out_uint32_le(this->ClientAreaWidth);
            stream.out_uint32_le(this->ClientAreaHeight);
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_RPCONTENT) {
            stream.out_uint8(this->RPContent);
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_ROOTPARENT) {
            stream.out_uint32_le(this->RootParentHandle);
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDOFFSET) {
            stream.out_sint32_le(this->WindowOffsetX);
            stream.out_sint32_le(this->WindowOffsetY);
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDCLIENTDELTA) {
            stream.out_sint32_le(this->WindowClientDeltaX);
            stream.out_sint32_le(this->WindowClientDeltaY);
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDSIZE) {
            stream.out_uint32_le(this->WindowWidth);
            stream.out_uint32_le(this->WindowHeight);
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDRECTS) {
            stream.out_uint16_le(this->NumWindowRects);

            for (Rectangle rectangle : this->window_rects) {
                rectangle.emit(stream);
            }
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_VISOFFSET) {
            stream.out_sint32_le(this->VisibleOffsetX);
            stream.out_sint32_le(this->VisibleOffsetY);
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_VISIBILITY) {
            stream.out_uint16_le(this->NumVisibilityRects);

            for (Rectangle rectangle : this->visibility_rects) {
                rectangle.emit(stream);
            }
        }

        stream.mark_end();

        WindowInformationCommonHeader::emit_end();
    }   // emit

    void receive(Stream & stream) {
        WindowInformationCommonHeader::receive(stream);

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_OWNER) {
            {
                const unsigned expected = 4;  // OwnerWindowId(4)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (0): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            this->OwnerWindowId = stream.in_uint32_le();
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_STYLE) {
            {
                const unsigned expected = 8;  // Style(4) + ExtendedStyle(4)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (1): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            this->Style         = stream.in_uint32_le();
            this->ExtendedStyle = stream.in_uint32_le();
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_SHOW) {
            {
                const unsigned expected = 1;  // ShowState(1)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (2): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            this->ShowState = stream.in_uint8();
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_TITLE) {
            {
                const unsigned expected = 2;  // CbString(2)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (3): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            const uint16_t CbString = stream.in_uint16_le();

            if (!stream.in_check_rem(CbString)) {
                LOG(LOG_ERR,
                    "Truncated NewOrExistingWindow (4): expected=%u remains=%u",
                    CbString, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }

            uint8_t * const unicode_data = static_cast<uint8_t *>(::alloca(CbString));

            {
                const unsigned expected = CbString;  // String(variable)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (5): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            stream.in_copy_bytes(unicode_data, CbString);

            const size_t maximum_length_of_utf8_character_in_bytes = 4;

            const size_t size_of_utf8_string =
                        CbString / 2 * maximum_length_of_utf8_character_in_bytes + 1;
            uint8_t * const utf8_string = static_cast<uint8_t *>(
                ::alloca(size_of_utf8_string));
            const size_t length_of_utf8_string = ::UTF16toUTF8(
                unicode_data, CbString / 2, utf8_string, size_of_utf8_string);
            this->title_info.assign(::char_ptr_cast(utf8_string),
                length_of_utf8_string);
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_CLIENTAREAOFFSET) {
            {
                const unsigned expected = 8;  // ClientOffsetX(4) + ClientOffsetY(4)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (6): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            this->ClientOffsetX = stream.in_uint32_le();
            this->ClientOffsetY = stream.in_uint32_le();
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_CLIENTAREASIZE) {
            {
                const unsigned expected = 8;  // ClientAreaWidth(4) + ClientAreaHeight(4)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (7): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            this->ClientAreaWidth  = stream.in_uint32_le();
            this->ClientAreaHeight = stream.in_uint32_le();
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_RPCONTENT) {
            {
                const unsigned expected = 1;  // RPContent(1)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (8): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            this->RPContent = stream.in_uint8();
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_ROOTPARENT) {
            {
                const unsigned expected = 4;  // RootParentHandle(4)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (9): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            this->RootParentHandle = stream.in_uint32_le();
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDOFFSET) {
            {
                const unsigned expected = 8;  // WindowOffsetX(4) + WindowOffsetY(4)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (10): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            this->WindowOffsetX = stream.in_sint32_le();
            this->WindowOffsetY = stream.in_sint32_le();
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDCLIENTDELTA) {
            {
                const unsigned expected = 8;  // WindowClientDeltaX(4) + WindowClientDeltaY(4)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (11): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            this->WindowClientDeltaX = stream.in_sint32_le();
            this->WindowClientDeltaY = stream.in_sint32_le();
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDSIZE) {
            {
                const unsigned expected = 8;  // WindowWidth(4) + WindowHeight(4)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (12): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            this->WindowWidth  = stream.in_uint32_le();
            this->WindowHeight = stream.in_uint32_le();
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDRECTS) {
            {
                const unsigned expected = 2;  // NumWindowRects(2)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (13): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            this->NumWindowRects = stream.in_uint16_le();

            for (uint16_t i = 0; i < this->NumWindowRects; ++i) {
                Rectangle rectangle;
                rectangle.receive(stream);

                this->window_rects.push_back(rectangle);
            }
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_VISOFFSET) {
            {
                const unsigned expected = 8;  // VisibleOffsetX(4) + VisibleOffsetY(4)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (15): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            this->VisibleOffsetX = stream.in_sint32_le();
            this->VisibleOffsetY = stream.in_sint32_le();
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_VISIBILITY) {
            {
                const unsigned expected = 2;  // NumVisibilityRects(2)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated NewOrExistingWindow (13): expected=%u remains=%u",
                        expected, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
            }

            this->NumVisibilityRects = stream.in_uint16_le();

            for (uint16_t i = 0; i < this->NumVisibilityRects; ++i) {
                Rectangle rectangle;
                rectangle.receive(stream);

                this->visibility_rects.push_back(rectangle);
            }
        }
    }   // receive

    size_t size() const {
        size_t count = 0;

        count += WindowInformationCommonHeader::size();

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_OWNER) {
            count += 4; // OwnerWindowId(4)
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_STYLE) {
            count += 8; // Style(4) + ExtendedStyle(4)
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_SHOW) {
            count += 1; // ShowState(1)
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_TITLE) {
            const size_t maximum_length_of_TitleInfo_in_bytes = this->title_info.length() * 2;

            count += maximum_length_of_TitleInfo_in_bytes; // ShowState(1)
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_CLIENTAREAOFFSET) {
            count += 8; // ClientOffsetX(4) + ClientOffsetY(4)
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_CLIENTAREASIZE) {
            count += 8; // ClientAreaWidth(4) + ClientAreaHeight(4)
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_RPCONTENT) {
            count += 1; // RPContent(1)
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_ROOTPARENT) {
            count += 4; // RootParentHandle(4)
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDOFFSET) {
            count += 8; // WindowOffsetX(4) + WindowOffsetY(4)
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDCLIENTDELTA) {
            count += 8; // WindowClientDeltaX(4) + WindowClientDeltaY(4)
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDSIZE) {
            count += 8; // WindowWidth(4) + WindowHeight(4)
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDRECTS) {
            count += 2; // NumWindowRects(2)
            count += this->NumWindowRects * Rectangle::size();
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_VISOFFSET) {
            count += 8; // VisibleOffsetX(4) + VisibleOffsetY(4)
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_VISIBILITY) {
            count += 2; // NumVisibilityRects(2)
            count += this->NumVisibilityRects * Rectangle::size();
        }

        return count;
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "NewOrExistingWindow ");
        length += ((result < size - length) ? result : (size - length - 1));

        length += WindowInformationCommonHeader::str(buffer + length, size - length);

        result = ::snprintf(buffer + length, size - length, ":");
        length += ((result < size - length) ? result : (size - length - 1));

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_OWNER) {
            result = ::snprintf(buffer + length, size - length, " OwnerWindowId=%u",
                this->OwnerWindowId);
            length += ((result < size - length) ? result : (size - length - 1));
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_STYLE) {
            result = ::snprintf(buffer + length, size - length, " Style=%u ExtendedStyle=%u",
                this->Style, this->ExtendedStyle);
            length += ((result < size - length) ? result : (size - length - 1));
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_SHOW) {
            result = ::snprintf(buffer + length, size - length, " ShowState=%u",
                this->ShowState);
            length += ((result < size - length) ? result : (size - length - 1));
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_TITLE) {
            result = ::snprintf(buffer + length, size - length, " TitleInfo=\"%s\"",
                this->title_info.c_str());
            length += ((result < size - length) ? result : (size - length - 1));
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_CLIENTAREAOFFSET) {
            result = ::snprintf(buffer + length, size - length, " ClientOffsetX=%u ClientOffsetY=%u",
                this->ClientOffsetX, this->ClientOffsetY);
            length += ((result < size - length) ? result : (size - length - 1));
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_CLIENTAREASIZE) {
            result = ::snprintf(buffer + length, size - length, " ClientAreaWidth=%u ClientAreaHeight=%u",
                this->ClientAreaWidth, this->ClientAreaHeight);
            length += ((result < size - length) ? result : (size - length - 1));
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_RPCONTENT) {
            result = ::snprintf(buffer + length, size - length, " RPContent=%u",
                this->RPContent);
            length += ((result < size - length) ? result : (size - length - 1));
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_ROOTPARENT) {
            result = ::snprintf(buffer + length, size - length, " RootParentHandle=%u",
                this->RootParentHandle);
            length += ((result < size - length) ? result : (size - length - 1));
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDOFFSET) {
            result = ::snprintf(buffer + length, size - length, " WindowOffsetX=%d WindowOffsetY=%d",
                this->WindowOffsetX, this->WindowOffsetY);
            length += ((result < size - length) ? result : (size - length - 1));
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDCLIENTDELTA) {
            result = ::snprintf(buffer + length, size - length, " WindowClientDeltaX=%d WindowClientDeltaY=%d",
                this->WindowClientDeltaX, this->WindowClientDeltaY);
            length += ((result < size - length) ? result : (size - length - 1));
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDSIZE) {
            result = ::snprintf(buffer + length, size - length, " WindowWidth=%u WindowHeight=%u",
                this->WindowWidth, this->WindowHeight);
            length += ((result < size - length) ? result : (size - length - 1));
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_WNDRECTS) {
            result = ::snprintf(buffer + length, size - length, " WindowRects=(");
            length += ((result < size - length) ? result : (size - length - 1));

            for (Rectangle rectangle : this->window_rects) {
                length += rectangle.str(buffer + length, size - length);
            }

            result = ::snprintf(buffer + length, size - length, ")");
            length += ((result < size - length) ? result : (size - length - 1));
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_VISOFFSET) {
            result = ::snprintf(buffer + length, size - length, " VisibleOffsetX=%d VisibleOffsetY=%d",
                this->VisibleOffsetX, this->VisibleOffsetY);
            length += ((result < size - length) ? result : (size - length - 1));
        }

        if (this->FieldsPresentFlags() & WINDOW_ORDER_FIELD_VISIBILITY) {
            result = ::snprintf(buffer + length, size - length, " VisibilityRects=(");
            length += ((result < size - length) ? result : (size - length - 1));

            for (Rectangle rectangle : this->visibility_rects) {
                length += rectangle.str(buffer + length, size - length);
            }

            result = ::snprintf(buffer + length, size - length, ")");
            length += ((result < size - length) ? result : (size - length - 1));
        }

        return length;
    }   // str

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }
};  // NewOrExistingWindow

// [MS-RDPERP] - 2.2.1.3.1.2.2 Window Icon
// =======================================

// The Window Icon packet is a Window Information Order generated by the
//  server when a new or existing window sets or updates its associated icon.

// Icons are created by combining two bitmaps of the same size. The mask
//  bitmap is always 1 bpp, although the color depth of the color bitmap can
//  vary. The color bitmap may have an associated color table.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-----------------------------------------------+---------------+
// |                      ...                      |    IconInfo   |
// |                                               |   (optional)  |
// +-----------------------------------------------+---------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Hdr (11 bytes): Eleven bytes. A TS_WINDOW_ORDER_HEADER structure. The
//  FieldsPresentFlags field of the header MUST be constructed using the
//  following values.

//  +-----------------------------+--------------------------------------------+
//  | Value                       | Meaning                                    |
//  +-----------------------------+--------------------------------------------+
//  | 0x01000000                  | Indicates a Windowing Alternate Secondary  |
//  | WINDOW_ORDER_TYPE_WINDOW    | Drawing Order that describes a window.     |
//  |                             | This flag MUST be set.                     |
//  +-----------------------------+--------------------------------------------+
//  | 0x10000000                  | Indicates that the Windowing Alternate     |
//  | WINDOW_ORDER_STATE_NEW      | Secondary Drawing Order contains           |
//  |                             | information for a new window. If this flag |
//  |                             | is not set, the order contains information |
//  |                             | for an existing window.                    |
//  +-----------------------------+--------------------------------------------+
//  | 0x40000000                  | Indicates that the order contains icon     |
//  | WINDOW_ORDER_ICON           | information for the window. This flag MUST |
//  |                             | be set.                                    |
//  +-----------------------------+--------------------------------------------+
//  | 0x00002000                  | Indicates that the large version of the    |
//  | WINDOW_ORDER_FIELD_ICON_BIG | icon is being sent. If this flag is not    |
//  |                             | present, the icon is a small icon. <6>     |
//  +-----------------------------+--------------------------------------------+

enum {
      WINDOW_ORDER_ICON           = 0x40000000
    , WINDOW_ORDER_FIELD_ICON_BIG = 0x00002000
};

// IconInfo (variable): Variable length. TS_ICON_INFO structure. Describes
//  the window's icon.

class WindowIcon : public WindowInformationCommonHeader {
    IconInfo icon_info;

public:
    inline void emit(Stream & stream) const {
        WindowInformationCommonHeader::emit_begin(stream);

        this->icon_info.emit(stream);

        stream.mark_end();

        WindowInformationCommonHeader::emit_end();
    }   // emit

    inline void receive(Stream & stream) {
        WindowInformationCommonHeader::receive(stream);

        this->icon_info.receive(stream);
    }   // receive

    inline size_t size() const {
        return WindowInformationCommonHeader::size() +
            this->icon_info.size();
    }

private:
    inline size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "WindowIcon ");
        length += ((result < size - length) ? result : (size - length - 1));

        length += WindowInformationCommonHeader::str(buffer + length, size - length);

        result = ::snprintf(buffer + length, size - length, ":");
        length += ((result < size - length) ? result : (size - length - 1));

        length += this->icon_info.str(buffer + length, size - length);

        return length;
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        size_t length = this->str(buffer, sizeof(buffer));
        buffer[length] = '\0';
        LOG(level, buffer);
    }
};  // WindowIcon

// [MS-RDPERP] - 2.2.1.3.1.2.3 Cached Icon
// =======================================

// The Cached Icon Window Information Order is generated by the server when a
//  new or existing window sets or updates the icon in its title bar or in
//  the Alt-Tab dialog box. If the icon information was transmitted by the
//  server in a previous Window Information Order or Notification Icon
//  Information Order in the same session, and the icon was cacheable (that
//  is, the server specified a cacheEntry and cacheId for the icon), the
//  server reports the icon cache entries to avoid sending duplicate
//  information.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-----------------------------------------------+---------------+
// |                      ...                      |   CachedIcon  |
// +-------------------------------+---------------+---------------+
// |              ...              |
// +-------------------------------+

// Hdr (11 bytes): Eleven bytes. A TS_WINDOW_ORDER_HEADER structure. The
//  FieldsPresentFlags field of the header MUST conform to the values defined
//  as follows.

//  +-----------------------------+--------------------------------------------+
//  | Value                       | Description                                |
//  +-----------------------------+--------------------------------------------+
//  | WINDOW_ORDER_TYPE_WINDOW    | Indicates a Windowing Alternate Secondary  |
//  | 0x01000000                  | Drawing Order that describes a window.     |
//  |                             | This flag MUST be set.                     |
//  +-----------------------------+--------------------------------------------+
//  | WINDOW_ORDER_STATE_NEW      | Indicates that the Windowing Alternate     |
//  | 0x10000000                  | Secondary Drawing Order contains           |
//  |                             | information for a new window. If this flag |
//  |                             | is not set, the order contains information |
//  |                             | for an existing window.                    |
//  +-----------------------------+--------------------------------------------+
//  | WINDOW_ORDER_CACHEDICON     | Indicates that the order contains cached   |
//  | 0x80000000                  | icon information for the window. This flag |
//  |                             | MUST be set.                               |
//  +-----------------------------+--------------------------------------------+
//  | WINDOW_ORDER_FIELD_ICON_BIG | Indicates that the large version of the    |
//  | 0x00002000                  | icon is being referred to. If this flag is |
//  |                             | not present, the icon is a small icon. <7> |
//  +-----------------------------+--------------------------------------------+

enum {
      WINDOW_ORDER_CACHEDICON = 0x80000000
};

// CachedIcon (3 bytes): Three bytes. TS_CACHED ICON_INFO structure.
//  Describes a cached icon on the client.

class CachedIcon : public WindowInformationCommonHeader {
    CachedIconInfo cached_icon_info;

public:
    inline void emit(Stream & stream) const {
        WindowInformationCommonHeader::emit_begin(stream);

        this->cached_icon_info.emit(stream);

        stream.mark_end();

        WindowInformationCommonHeader::emit_end();
    }   // emit

    inline void receive(Stream & stream) {
        WindowInformationCommonHeader::receive(stream);

        this->cached_icon_info.receive(stream);
    }   // receive

    inline static size_t size() {
        return WindowInformationCommonHeader::size() +
            CachedIconInfo::size();
    }

private:
    inline size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "CachedIcon ");
        length += ((result < size - length) ? result : (size - length - 1));

        length += WindowInformationCommonHeader::str(buffer + length, size - length);

        result = ::snprintf(buffer + length, size - length, ":");
        length += ((result < size - length) ? result : (size - length - 1));

        length += this->cached_icon_info.str(buffer + length, size - length);

        return length;
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }
};  // CachedIcon

// [MS-RDPERP] - 2.2.1.3.1.2.4 Deleted Window
// ==========================================

// The Deleted Window Information Order is generated by the server whenever
//  an existing window is destroyed on the server.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-----------------------------------------------+---------------+
// |                      ...                      |
// +-----------------------------------------------+

// Hdr (11 bytes): Eleven bytes. A TS_WINDOW_ORDER_HEADER structure. The
//  FieldsPresentFlags field of the header MUST be constructed using the
//  following values.

//  +----------------------------+-------------------------------------------+
//  | Value                      | Meaning                                   |
//  +----------------------------+-------------------------------------------+
//  | 0x01000000                 | Indicates a Windowing Alternate Secondary |
//  | WINDOW_ORDER_TYPE_WINDOW   | Drawing Order describing a window. This   |
//  |                            | flag MUST be set.                         |
//  +----------------------------+-------------------------------------------+
//  | 0x20000000                 | Indicates that the window is deleted. If  |
//  | WINDOW_ORDER_STATE_DELETED | this flag is set, the order MUST NOT      |
//  |                            | contain any other information.            |
//  +----------------------------+-------------------------------------------+

enum {
      WINDOW_ORDER_STATE_DELETED = 0x20000000
};

class DeletedWindow : public WindowInformationCommonHeader {
public:
    inline void emit(Stream & stream) const {
        WindowInformationCommonHeader::emit_begin(stream);

        stream.mark_end();

        WindowInformationCommonHeader::emit_end();
    }   // emit

    inline void receive(Stream & stream) {
        WindowInformationCommonHeader::receive(stream);
    }   // receive

    inline static size_t size() {
        return WindowInformationCommonHeader::size();
    }

private:
    inline size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "DeletedWindow ");
        length += ((result < size - length) ? result : (size - length - 1));

        length += WindowInformationCommonHeader::str(buffer + length, size - length);

        return length;
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }
};  // CachedIcon

// [MS-RDPERP] - 2.2.1.3.3.1 Common Header (TS_DESKTOP_ORDER_HEADER)
// =================================================================

// The TS_DESKTOP_ORDER_HEADER packet contains information common to every
//  order specifying the desktop.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     Header    |           OrderSize           | FieldsPresent |
// |               |                               |     Flags     |
// +---------------+-------------------------------+---------------+
// |                      ...                      |
// +-----------------------------------------------+

// Header (1 byte): An unsigned 8-bit integer. An Alternate Secondary Order
//  Header, as specified in [MS-RDPEGDI] section 2.2.2.2.1.3.1.1. The
//  embedded orderType field MUST be set to 0x0B (TS_ALTSEC_WINDOW).

// OrderSize (2 bytes): An unsigned 16-bit integer. The size of the entire
//  packet in bytes.

// FieldsPresentFlags (4 bytes): An unsigned 32-bit integer. The flags
//  indicating which fields are present in the packet. See Actively Monitored
//  Desktop for values and use.

}   // namespace RAIL
}   // namespace RDP

#endif  // #ifndef _REDEMPTION_CORE_RDP_ORDERS_ALTERNATESECONDARYWINDOWING_HPP_
