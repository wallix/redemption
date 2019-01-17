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
#include "utils/parse.hpp"
#include "utils/sugar/array_view.hpp"

#include "core/RDP/capabilities/general.hpp"
#include "core/RDP/capabilities/cap_bitmap.hpp"
#include "core/RDP/capabilities/order.hpp"

namespace
{

enum class StorageType : unsigned char
{
    Byte,
    Uint16,
    Uint32,
};

template<class>
struct to_storage_type;

#define TO_STORAGE_TYPE(type, value)                                    \
    template<>                                                          \
    struct to_storage_type<type>                                        \
    {                                                                   \
        static constexpr StorageType storage_type = StorageType::value; \
    }

TO_STORAGE_TYPE(uint8_t, Byte);
TO_STORAGE_TYPE(uint16_t, Uint16);
TO_STORAGE_TYPE(uint32_t, Uint32);

#undef TO_STORAGE_TYPE


struct Entry
{
    Entry() = delete;

    template<class T>
    Entry(const char * key, T* storage) noexcept
    : key(key)
    , storage(storage)
    , storage_type(to_storage_type<T>::storage_type)
    {}

    bool set_value(const char * key, const char * str_value) const noexcept
    {
        if (!this->match(key)) {
            return false;
        }

        auto const value = Parse(byte_ptr_cast(str_value)).ulong_from_cstr();

        switch (this->storage_type) {
            case StorageType::Byte:
                *static_cast<uint8_t*>(this->storage) = value;
            break;
            case StorageType::Uint16:
                *static_cast<uint16_t*>(this->storage) = value;
            break;
            case StorageType::Uint32:
                *static_cast<uint32_t*>(this->storage) = value;
            break;
        }

        return true;
    }

private:
    char const    * key;
    void          * storage;
    StorageType   storage_type;

    bool match(const char * key) const
    {
        return 0 == strcasecmp(this->key, key);
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
        Entry("osMajorType", &caps.os_major),
        Entry("osMinorType", &caps.os_minor),
        Entry("protocolVersion", &caps.protocolVersion),
        Entry("generalCompressionTypes", &caps.compressionType),
        Entry("extraFlags", &caps.extraflags),
        Entry("updateCapabilityFlag", &caps.updateCapability),
        Entry("remoteUnshareFlag", &caps.remoteUnshare),
        Entry("generalCompressionLevel", &caps.compressionLevel),
        Entry("refreshRectSupport", &caps.refreshRectSupport),
        Entry("suppressOutputSupport", &caps.suppressOutputSupport),
    });
}

bool bitmap_caps_load(BitmapCaps & caps, char const * filename)
{
    return load(filename, "Bitmap Capability Set", {
        Entry("preferredBitsPerPixel", &caps.preferredBitsPerPixel),
        Entry("receive1BitPerPixel", &caps.receive1BitPerPixel),
        Entry("receive4BitsPerPixel", &caps.receive4BitsPerPixel),
        Entry("receive8BitsPerPixel", &caps.receive8BitsPerPixel),
        Entry("desktopWidth", &caps.desktopWidth),
        Entry("desktopHeight", &caps.desktopHeight),
        Entry("desktopResizeFlag", &caps.desktopResizeFlag),
        Entry("bitmapCompressionFlag", &caps.bitmapCompressionFlag),
        Entry("highColorFlags", &caps.highColorFlags),
        Entry("drawingFlags", &caps.drawingFlags),
        Entry("multipleRectangleSupport", &caps.multipleRectangleSupport),
    });
}

bool order_caps_load(OrderCaps & caps, char const * filename)
{
    return load(filename, "Order Capability Set", {
        Entry("desktopSaveXGranularity", &caps.desktopSaveXGranularity),
        Entry("desktopSaveYGranularity", &caps.desktopSaveYGranularity),
        Entry("maximumOrderLevel", &caps.maximumOrderLevel),
        Entry("numberFonts", &caps.numberFonts),
        Entry("orderFlags", &caps.orderFlags),
        Entry("TS_NEG_DSTBLT_INDEX", &caps.orderSupport[TS_NEG_DSTBLT_INDEX]),
        Entry("TS_NEG_PATBLT_INDEX", &caps.orderSupport[TS_NEG_PATBLT_INDEX]),
        Entry("TS_NEG_SCRBLT_INDEX", &caps.orderSupport[TS_NEG_SCRBLT_INDEX]),
        Entry("TS_NEG_MEMBLT_INDEX", &caps.orderSupport[TS_NEG_MEMBLT_INDEX]),
        Entry("TS_NEG_MEM3BLT_INDEX", &caps.orderSupport[TS_NEG_MEM3BLT_INDEX]),
        Entry("TS_NEG_DRAWNINEGRID_INDEX", &caps.orderSupport[TS_NEG_DRAWNINEGRID_INDEX]),
        Entry("TS_NEG_LINETO_INDEX", &caps.orderSupport[TS_NEG_LINETO_INDEX]),
        Entry("TS_NEG_MULTI_DRAWNINEGRID_INDEX", &caps.orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX]),
        Entry("TS_NEG_SAVEBITMAP_INDEX", &caps.orderSupport[TS_NEG_SAVEBITMAP_INDEX]),
        Entry("TS_NEG_MULTIDSTBLT_INDEX", &caps.orderSupport[TS_NEG_MULTIDSTBLT_INDEX]),
        Entry("TS_NEG_MULTIPATBLT_INDEX", &caps.orderSupport[TS_NEG_MULTIPATBLT_INDEX]),
        Entry("TS_NEG_MULTISCRBLT_INDEX", &caps.orderSupport[TS_NEG_MULTISCRBLT_INDEX]),
        Entry("TS_NEG_MULTIOPAQUERECT_INDEX", &caps.orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX]),
        Entry("TS_NEG_FAST_INDEX_INDEX", &caps.orderSupport[TS_NEG_FAST_INDEX_INDEX]),
        Entry("TS_NEG_POLYGON_SC_INDEX", &caps.orderSupport[TS_NEG_POLYGON_SC_INDEX]),
        Entry("TS_NEG_POLYGON_CB_INDEX", &caps.orderSupport[TS_NEG_POLYGON_CB_INDEX]),
        Entry("TS_NEG_POLYLINE_INDEX", &caps.orderSupport[TS_NEG_POLYLINE_INDEX]),
        Entry("TS_NEG_FAST_GLYPH_INDEX", &caps.orderSupport[TS_NEG_FAST_GLYPH_INDEX]),
        Entry("TS_NEG_ELLIPSE_SC_INDEX", &caps.orderSupport[TS_NEG_ELLIPSE_SC_INDEX]),
        Entry("TS_NEG_ELLIPSE_CB_INDEX", &caps.orderSupport[TS_NEG_ELLIPSE_CB_INDEX]),
        Entry("TS_NEG_INDEX_INDEX", &caps.orderSupport[TS_NEG_INDEX_INDEX]),
        Entry("orderSupportExFlags", &caps.orderSupportExFlags),
        Entry("desktopSaveSize", &caps.desktopSaveSize),
        Entry("textANSICodePage", &caps.textANSICodePage),
    });
}
