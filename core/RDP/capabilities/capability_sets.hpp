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
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CORE_RDP_CAPABILITIES_CAPABILITY_SETS_HPP_
#define _REDEMPTION_CORE_RDP_CAPABILITIES_CAPABILITY_SETS_HPP_

#include "cfgloader.hpp"
#include "log.hpp"

#include "cap_bitmap.hpp"
#include "general.hpp"
#include "order.hpp"

struct CapabilitySets : public ConfigurationHolder {
    GeneralCaps general_caps;
    BitmapCaps  bitmap_caps;
    OrderCaps   order_caps;

    CapabilitySets() {}

    virtual void set_value(const char * section, const char * key, const char * value) {
        if (!strcmp(section, "General Capability Set")) {
                 if (!strcmp(key, "osMajorType")) {
                this->general_caps.os_major              = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "osMinorType")) {
                this->general_caps.os_minor              = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "protocolVersion")) {
                this->general_caps.protocolVersion       = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "generalCompressionTypes")) {
                this->general_caps.compressionType       = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "extraFlags")) {
                this->general_caps.extraflags            = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "updateCapabilityFlag")) {
                this->general_caps.updateCapability      = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "remoteUnshareFlag")) {
                this->general_caps.remoteUnshare         = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "generalCompressionLevel")) {
                this->general_caps.compressionLevel      = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "refreshRectSupport")) {
                this->general_caps.refreshRectSupport    = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "suppressOutputSupport")) {
                this->general_caps.suppressOutputSupport = ulong_from_cstr(value);
            }
            else {
                LOG( LOG_INFO
                   , "CapabilitySets::set_value() Unknown key: section=\"%s\", key=\"%s\", value=\"%s\""
                   , section, key, value);
            }
        }   // if (!strcmp(section, "General Capability Set"))
        else if (!strcmp(section, "Bitmap Capability Set")) {
                 if (!strcmp(key, "preferredBitsPerPixel")) {
                this->bitmap_caps.preferredBitsPerPixel    = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "receive1BitPerPixel")) {
                this->bitmap_caps.receive1BitPerPixel      = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "receive4BitsPerPixel")) {
                this->bitmap_caps.receive4BitsPerPixel     = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "receive8BitsPerPixel")) {
                this->bitmap_caps.receive8BitsPerPixel     = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "desktopWidth")) {
                this->bitmap_caps.desktopWidth             = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "desktopHeight")) {
                this->bitmap_caps.desktopHeight            = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "desktopResizeFlag")) {
                this->bitmap_caps.desktopResizeFlag        = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "bitmapCompressionFlag")) {
                this->bitmap_caps.bitmapCompressionFlag    = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "highColorFlags")) {
                this->bitmap_caps.highColorFlags           = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "drawingFlags")) {
                this->bitmap_caps.drawingFlags             = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "multipleRectangleSupport")) {
                this->bitmap_caps.multipleRectangleSupport = ulong_from_cstr(value);
            }
            else {
                LOG( LOG_INFO
                   , "CapabilitySets::set_value() Unknown key: section=\"%s\", key=\"%s\", value=\"%s\""
                   , section, key, value);
            }
        }   // else if (!strcmp(section, "Bitmap Capability Set"))
        else if (!strcmp(section, "Order Capability Set")) {
                 if (!strcmp(key, "desktopSaveXGranularity")) {
                this->order_caps.desktopSaveXGranularity                       = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "desktopSaveYGranularity")) {
                this->order_caps.desktopSaveYGranularity                       = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "maximumOrderLevel")) {
                this->order_caps.maximumOrderLevel                             = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "numberFonts")) {
                this->order_caps.numberFonts                                   = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "orderFlags")) {
                this->order_caps.orderFlags                                    = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_DSTBLT_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_DSTBLT_INDEX]             = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_PATBLT_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_PATBLT_INDEX]             = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_SCRBLT_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_SCRBLT_INDEX]             = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_MEMBLT_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_MEMBLT_INDEX]             = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_MEM3BLT_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_MEM3BLT_INDEX]            = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_DRAWNINEGRID_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_DRAWNINEGRID_INDEX]       = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_LINETO_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_LINETO_INDEX]             = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_MULTI_DRAWNINEGRID_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_SAVEBITMAP_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_SAVEBITMAP_INDEX]         = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_MULTIDSTBLT_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_MULTIDSTBLT_INDEX]        = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_MULTIPATBLT_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_MULTIPATBLT_INDEX]        = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_MULTISCRBLT_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_MULTISCRBLT_INDEX]        = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_MULTIOPAQUERECT_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX]    = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_FAST_INDEX_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_FAST_INDEX_INDEX]         = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_POLYGON_SC_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_POLYGON_SC_INDEX]         = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_POLYGON_CB_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_POLYGON_CB_INDEX]         = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_POLYLINE_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_POLYLINE_INDEX]           = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_FAST_GLYPH_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_FAST_GLYPH_INDEX]         = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_ELLIPSE_SC_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_ELLIPSE_SC_INDEX]         = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_ELLIPSE_CB_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_ELLIPSE_CB_INDEX]         = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "TS_NEG_INDEX_INDEX")) {
                this->order_caps.orderSupport[TS_NEG_INDEX_INDEX]              = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "orderSupportExFlags")) {
                this->order_caps.orderSupportExFlags                           = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "desktopSaveSize")) {
                this->order_caps.desktopSaveSize                               = ulong_from_cstr(value);
            }
            else if (!strcmp(key, "textANSICodePage")) {
                this->order_caps.textANSICodePage                              = ulong_from_cstr(value);
            }
            else {
                LOG( LOG_INFO
                   , "CapabilitySets::set_value() Unknown key: section=\"%s\", key=\"%s\", value=\"%s\""
                   , section, key, value);
            }
        }   // else if (!strcmp(section, "Order Capability Set"))
    }   // void set_value(const char * section, const char * key, const char * value)
};

#endif  // _REDEMPTION_CORE_RDP_CAPABILITIES_CAPABILITY_SETS_HPP_

