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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean

   Configuration file descriptor objects
*/


#include "utils/confdescriptor.hpp"
#include "utils/cfgloader.hpp"
#include "utils/log.hpp"
#include "utils/parse.hpp"
#include "utils/sugar/array_view.hpp"

#include "core/RDP/capabilities/general.hpp"
#include "core/RDP/capabilities/cap_bitmap.hpp"
#include "core/RDP/capabilities/order.hpp"

namespace
{
// TODO We could probably use templated Entries instead of InputType_t and StorageType_t enums, this with also avoid combinatorial explosion. Well, not really, but the compiler will do the work.

enum InputType_t {
    INPUT_BOOLEAN,
    INPUT_UNSIGNED,
//     INPUT_LEVEL
};

enum StorageType_t {
    STORAGE_BYTE,
    STORAGE_UINT16,
    STORAGE_UINT32
};

struct Entry {
    char const    * key;
    InputType_t     input_type;
    StorageType_t   storage_type;
    void          * storage;

    Entry() = delete;

    Entry(const char * key, InputType_t input_type, StorageType_t storage_type, void * storage)
    : key(key)
    , input_type(input_type)
    , storage_type(storage_type)
    , storage(storage)
    {
    }

    bool match(const char * key) const
    {
        return 0 == strcasecmp(this->key, key);
    }

    bool set_value(const char * key, const char * value) const
    {
        if (!this->match(key)) {
            return false;
        }
        switch (this->input_type) {
            case INPUT_BOOLEAN: {
                // TODO: check behavior. Changed from previous version terminating zero is not checked
                // on value. It will return true for all string beginning by one of the keywords
                // as ulong_from_cstr also does not check for string end this seems to be the
                // logical thing to do. But we should check use cases nevertheless.
                unsigned long result = Parse(byte_ptr_cast(value)).bool_from_cstr();
                this->store(result);
            }
            break;
            case INPUT_UNSIGNED: {
                unsigned long result = Parse(byte_ptr_cast(value)).ulong_from_cstr();;
                this->store(result);
            }
            break;
            //case INPUT_LEVEL: {
            //    unsigned long level  = level_from_cstr(value);
            //    this->store(level);
            //}
            //break;
        }
        return true;
    }

    void store(unsigned long value) const
    {
        switch (this->storage_type) {
            case STORAGE_BYTE:
                static_cast<uint8_t *>(this->storage)[0]  = static_cast<uint8_t>(value);
            break;
            case STORAGE_UINT16:
                static_cast<uint16_t *>(this->storage)[0] = static_cast<uint16_t>(value);
            break;
            case STORAGE_UINT32:
                static_cast<uint32_t *>(this->storage)[0] = static_cast<uint32_t>(value);
            break;
        }
    }
};

struct Loader : public ConfigurationHolder
{
    char const * section_name;
    array_view<Entry const> entries;

    Loader(char const * section_name, array_view<Entry const> entries)
        : section_name(section_name)
        , entries(entries)
    {}

    void set_value(const char * section, const char * key, const char * value) override
    {
        // TODO parsing like this is very, very inefficient, change that later
        if (0 != strcasecmp(this->section_name, section)) {
            return;
        }

        for (Entry const & entry : this->entries) {
            if (entry.set_value(key, value)) {
                return;
            }
        }
    }
};

bool load(char const * filename, char const * section_name, std::initializer_list<Entry> entries)
{
    return configuration_load(Loader(section_name, {entries.begin(), entries.end()}), filename);
}

} // anonymous namespace

bool general_caps_load(GeneralCaps & caps, char const * filename)
{
    return load(filename, "General Capability Set", {
        Entry("osMajorType", INPUT_UNSIGNED, STORAGE_UINT16, &caps.os_major),
        Entry("osMinorType", INPUT_UNSIGNED, STORAGE_UINT16, &caps.os_minor),
        Entry("protocolVersion", INPUT_UNSIGNED, STORAGE_UINT16, &caps.protocolVersion),
        Entry("generalCompressionTypes", INPUT_UNSIGNED, STORAGE_UINT16, &caps.compressionType),
        Entry("extraFlags", INPUT_UNSIGNED, STORAGE_UINT16, &caps.extraflags),
        Entry("updateCapabilityFlag", INPUT_UNSIGNED, STORAGE_UINT16, &caps.updateCapability),
        Entry("remoteUnshareFlag", INPUT_UNSIGNED, STORAGE_UINT16, &caps.remoteUnshare),
        Entry("generalCompressionLevel", INPUT_UNSIGNED, STORAGE_UINT16, &caps.compressionLevel),
        Entry("refreshRectSupport", INPUT_UNSIGNED, STORAGE_BYTE, &caps.refreshRectSupport),
        Entry("suppressOutputSupport", INPUT_UNSIGNED, STORAGE_BYTE, &caps.suppressOutputSupport),
    });
}

bool bitmap_caps_load(BitmapCaps & caps, char const * filename)
{
    return load(filename, "Bitmap Capability Set", {
        Entry("preferredBitsPerPixel", INPUT_UNSIGNED, STORAGE_UINT16, &caps.preferredBitsPerPixel),
        Entry("receive1BitPerPixel", INPUT_UNSIGNED, STORAGE_UINT16, &caps.receive1BitPerPixel),
        Entry("receive4BitsPerPixel", INPUT_UNSIGNED, STORAGE_UINT16, &caps.receive4BitsPerPixel),
        Entry("receive8BitsPerPixel", INPUT_UNSIGNED, STORAGE_UINT16, &caps.receive8BitsPerPixel),
        Entry("desktopWidth", INPUT_UNSIGNED, STORAGE_UINT16, &caps.desktopWidth),
        Entry("desktopHeight", INPUT_UNSIGNED, STORAGE_UINT16, &caps.desktopHeight),
        Entry("desktopResizeFlag", INPUT_UNSIGNED, STORAGE_UINT16, &caps.desktopResizeFlag),
        Entry("bitmapCompressionFlag", INPUT_UNSIGNED, STORAGE_UINT16, &caps.bitmapCompressionFlag),
        Entry("highColorFlags", INPUT_UNSIGNED, STORAGE_BYTE, &caps.highColorFlags),
        Entry("drawingFlags", INPUT_UNSIGNED, STORAGE_BYTE, &caps.drawingFlags),
        Entry("multipleRectangleSupport", INPUT_UNSIGNED, STORAGE_UINT16, &caps.multipleRectangleSupport),
    });
}

bool order_caps_load(OrderCaps & caps, char const * filename)
{
    return load(filename, "Order Capability Set", {
        Entry("desktopSaveXGranularity", INPUT_UNSIGNED, STORAGE_UINT16, &caps.desktopSaveXGranularity),
        Entry("desktopSaveYGranularity", INPUT_UNSIGNED, STORAGE_UINT16, &caps.desktopSaveYGranularity),
        Entry("maximumOrderLevel", INPUT_UNSIGNED, STORAGE_UINT16, &caps.maximumOrderLevel),
        Entry("numberFonts", INPUT_UNSIGNED, STORAGE_UINT16, &caps.numberFonts),
        Entry("orderFlags", INPUT_UNSIGNED, STORAGE_UINT16, &caps.orderFlags),
        Entry("TS_NEG_DSTBLT_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_DSTBLT_INDEX]),
        Entry("TS_NEG_PATBLT_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_PATBLT_INDEX]),
        Entry("TS_NEG_SCRBLT_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_SCRBLT_INDEX]),
        Entry("TS_NEG_MEMBLT_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_MEMBLT_INDEX]),
        Entry("TS_NEG_MEM3BLT_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_MEM3BLT_INDEX]),
        Entry("TS_NEG_DRAWNINEGRID_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_DRAWNINEGRID_INDEX]),
        Entry("TS_NEG_LINETO_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_LINETO_INDEX]),
        Entry("TS_NEG_MULTI_DRAWNINEGRID_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX]),
        Entry("TS_NEG_SAVEBITMAP_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_SAVEBITMAP_INDEX]),
        Entry("TS_NEG_MULTIDSTBLT_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_MULTIDSTBLT_INDEX]),
        Entry("TS_NEG_MULTIPATBLT_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_MULTIPATBLT_INDEX]),
        Entry("TS_NEG_MULTISCRBLT_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_MULTISCRBLT_INDEX]),
        Entry("TS_NEG_MULTIOPAQUERECT_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX]),
        Entry("TS_NEG_FAST_INDEX_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_FAST_INDEX_INDEX]),
        Entry("TS_NEG_POLYGON_SC_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_POLYGON_SC_INDEX]),
        Entry("TS_NEG_POLYGON_CB_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_POLYGON_CB_INDEX]),
        Entry("TS_NEG_POLYLINE_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_POLYLINE_INDEX]),
        Entry("TS_NEG_FAST_GLYPH_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_FAST_GLYPH_INDEX]),
        Entry("TS_NEG_ELLIPSE_SC_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_ELLIPSE_SC_INDEX]),
        Entry("TS_NEG_ELLIPSE_CB_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_ELLIPSE_CB_INDEX]),
        Entry("TS_NEG_INDEX_INDEX", INPUT_UNSIGNED, STORAGE_BYTE, &caps.orderSupport[TS_NEG_INDEX_INDEX]),
        Entry("orderSupportExFlags", INPUT_UNSIGNED, STORAGE_UINT16, &caps.orderSupportExFlags),
        Entry("desktopSaveSize", INPUT_UNSIGNED, STORAGE_UINT32, &caps.desktopSaveSize),
        Entry("textANSICodePage", INPUT_UNSIGNED, STORAGE_UINT16, &caps.textANSICodePage),
    });
}
