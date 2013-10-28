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

#ifndef _REDEMPTION_UTILS_CONFDESCRIPTOR_HPP__
#define _REDEMPTION_UTILS_CONFDESCRIPTOR_HPP_

#include "log.hpp"
#include "parser.hpp"
#include "cfgloader.hpp"

#include "RDP/capabilities/general.hpp"

TODO("We could probably use templated Entries instead of InputType_t and StorageType_t enums, this with also avoid combinatorial explosion."
     "Well, not really, but the compiler will do the work.")

enum InputType_t {
    INPUT_BOOLEAN,
    INPUT_UNSIGNED,
    INPUT_LEVEL,
};

enum StorageType_t {
    STORAGE_BYTE,
    STORAGE_UINT16
};

struct Entry {
    char            key[64];
    InputType_t     input_type;
    StorageType_t   storage_type;
    void          * storage;

    Entry()
    : input_type(INPUT_BOOLEAN)
    , storage_type(STORAGE_BYTE)
    , storage(NULL) {
        ::memset(this->key, 0, sizeof(this->key));
    }

    Entry(const char * key, InputType_t input_type, StorageType_t storage_type, void * storage)
    : input_type(input_type)
    , storage_type(storage_type)
    , storage(storage) {
        size_t length = sizeof(this->key) - 1;
        strncpy(this->key, key, length);
        this->key[length] = 0;
    }

    bool match(const char * key) {
        return 0 == strncasecmp(this->key, key, sizeof(this->key));
    }

    bool set_value(const char * key, const char * value) {
        if (!this->match(key)) {
            return false;
        }
        switch (this->input_type) {
            case INPUT_BOOLEAN: {
                unsigned long result = bool_from_cstr(value);
                this->store(result);
            }
            break;
            case INPUT_UNSIGNED: {
                unsigned long result = ulong_from_cstr(value);
                this->store(result);
            }
            break;
            case INPUT_LEVEL: {
                unsigned long level  = level_from_cstr(value);
                this->store(level);
            }
            break;
        }
        return true;
    }

    void store(unsigned long value) {
        switch (this->storage_type) {
            case STORAGE_BYTE:
                static_cast<uint8_t *>(this->storage)[0]  = static_cast<uint8_t>(value);
            break;
            case STORAGE_UINT16:
                static_cast<uint16_t *>(this->storage)[0] = static_cast<uint16_t>(value);
            break;
        }
    }
};

TODO("We also have to define BitmapCaps and OrderCaps loaders, then we will be able to use this in capability_sets.hpp")
struct GeneralCapsLoader : public ConfigurationHolder
{
    char name[64];

    enum {
        AUTHID_GENERALCAPS_OS_MAJOR,
        AUTHID_GENERALCAPS_OS_MINOR,
        AUTHID_GENERALCAPS_PROTOCOLVERSION,
        AUTHID_GENERALCAPS_COMPRESSIONTYPE,
        AUTHID_GENERALCAPS_EXTRAFLAGS,
        AUTHID_GENERALCAPS_UPDATECAPABILITY,
        AUTHID_GENERALCAPS_REMOTEUNSHARE,
        AUTHID_GENERALCAPS_COMPRESSIONLEVEL,
        AUTHID_GENERALCAPS_REFRESHRECTSUPPORT,
        AUTHID_GENERALCAPS_SUPPRESSOUTPUTSUPPORT,
        NUMBER_OF_CAPS
    };

    Entry entries[NUMBER_OF_CAPS];

    GeneralCapsLoader(GeneralCaps & caps)
    {
        strncpy(this->name, "General Capability Set", sizeof(this->name));

        this->entries[AUTHID_GENERALCAPS_OS_MAJOR] =
            Entry("osMajorType", INPUT_UNSIGNED, STORAGE_UINT16, &caps.os_major);

        this->entries[AUTHID_GENERALCAPS_OS_MINOR] =
            Entry("osMinorType", INPUT_UNSIGNED, STORAGE_UINT16, &caps.os_minor);

        this->entries[AUTHID_GENERALCAPS_PROTOCOLVERSION] =
            Entry("protocolVersion", INPUT_UNSIGNED, STORAGE_UINT16, &caps.protocolVersion);

        this->entries[AUTHID_GENERALCAPS_COMPRESSIONTYPE] =
            Entry("generalCompressionTypes", INPUT_UNSIGNED, STORAGE_UINT16, &caps.compressionType);

        this->entries[AUTHID_GENERALCAPS_EXTRAFLAGS] =
            Entry("extraFlags", INPUT_UNSIGNED, STORAGE_UINT16, &caps.extraflags);

        this->entries[AUTHID_GENERALCAPS_UPDATECAPABILITY] =
            Entry("updateCapabilityFlag", INPUT_UNSIGNED, STORAGE_UINT16, &caps.updateCapability);

        this->entries[AUTHID_GENERALCAPS_REMOTEUNSHARE] =
            Entry("remoteUnshareFlag", INPUT_UNSIGNED, STORAGE_UINT16, &caps.remoteUnshare);

        this->entries[AUTHID_GENERALCAPS_COMPRESSIONLEVEL] =
            Entry("generalCompressionLevel", INPUT_UNSIGNED, STORAGE_UINT16, &caps.compressionLevel);

        this->entries[AUTHID_GENERALCAPS_REFRESHRECTSUPPORT] =
            Entry("refreshRectSupport", INPUT_UNSIGNED, STORAGE_BYTE, &caps.refreshRectSupport);

        this->entries[AUTHID_GENERALCAPS_SUPPRESSOUTPUTSUPPORT] =
            Entry("suppressOutputSupport", INPUT_UNSIGNED, STORAGE_BYTE, &caps.suppressOutputSupport);
    }

    virtual bool match(const char * name) {
        return 0 == strncasecmp(this->name, name, sizeof(this->name));
    }

    virtual void set_value(const char * section, const char * key, const char * value) {
        TODO("parsing like this is very, very inefficient, change that later")

        if (!this->match(section)) {
            return;
        }
        for (size_t i = 0; i < NUMBER_OF_CAPS; i++) {
            if (this->entries[i].set_value(key, value)) {
                return;
            }
        }
    }
};

/*
        AUTHID_BITMAPCAPS_PREFERREDBITSPERPIXEL,
        AUTHID_BITMAPCAPS_RECEIVE1BITPERPIXEL,
        AUTHID_BITMAPCAPS_RECEIVE4BITPERPIXEL,
        AUTHID_BITMAPCAPS_RECEIVE8BITPERPIXEL,
        AUTHID_BITMAPCAPS_DESKTOPWIDTH,
        AUTHID_BITMAPCAPS_DESKTOPHEIGHT,
        AUTHID_BITMAPCAPS_DESKTOPRESIZEFLAG,
        AUTHID_BITMAPCAPS_BITMAPCOMPRESSIONFLAG,
        AUTHID_BITMAPCAPS_HIGHCOLORFLAGS,
        AUTHID_BITMAPCAPS_DRAWINGFLAGS,
        AUTHID_BITMAPCAPS_MULTIPLERECTANGLESUPPORT,

        AUTHID_ORDERCAPS_DESKTOPSAVEXGRANULARITY,
        AUTHID_ORDERCAPS_DESKTOPSAVEYGRANULARITY,
        AUTHID_ORDERCAPS_MAXIMUMORDERLEVEL,
        AUTHID_ORDERCAPS_NUMBERFONTS,
        AUTHID_ORDERCAPS_ORDERFLAGS,
        AUTHID_ORDERCAPS_ORDERSUPPORT_DSTBLT,
        AUTHID_ORDERCAPS_ORDERSUPPORT_PATBLT,
        AUTHID_ORDERCAPS_ORDERSUPPORT_SCRBLT,
        AUTHID_ORDERCAPS_ORDERSUPPORT_MEMBLT,
        AUTHID_ORDERCAPS_ORDERSUPPORT_MEM3BLT,
        AUTHID_ORDERCAPS_ORDERSUPPORT_DRAWNINEGRID,
        AUTHID_ORDERCAPS_ORDERSUPPORT_LINETO,
        AUTHID_ORDERCAPS_ORDERSUPPORT_MULTIDRAWNINEGRID,
        AUTHID_ORDERCAPS_ORDERSUPPORT_SAVEBITMAP,
        AUTHID_ORDERCAPS_ORDERSUPPORT_MULTIDSTBLT,
        AUTHID_ORDERCAPS_ORDERSUPPORT_MULTIPATBLT,
        AUTHID_ORDERCAPS_ORDERSUPPORT_MULTISCRBLT,
        AUTHID_ORDERCAPS_ORDERSUPPORT_MULTIOPAQUERECT,
        AUTHID_ORDERCAPS_ORDERSUPPORT_FASTINDEX,
        AUTHID_ORDERCAPS_ORDERSUPPORT_POLYGONSC,
        AUTHID_ORDERCAPS_ORDERSUPPORT_POLYGONCB,
        AUTHID_ORDERCAPS_ORDERSUPPORT_POLYLINE,
        AUTHID_ORDERCAPS_ORDERSUPPORT_FASTGLYPH,
        AUTHID_ORDERCAPS_ORDERSUPPORT_ELLIPSESC,
        AUTHID_ORDERCAPS_ORDERSUPPORT_ELLIPSECB,
        AUTHID_ORDERCAPS_ORDERSUPPORT_GLYPHINDEX,
        AUTHID_ORDERCAPS_ORDERSUPPORT_TEXTFLAGS,
        AUTHID_ORDERCAPS_ORDERSUPPORT_ORDERSUPPORTEXFLAGS,
        AUTHID_ORDERCAPS_ORDERSUPPORT_DESKTOPSAVESIZE,
        AUTHID_ORDERCAPS_ORDERSUPPORT_TEXTANSICODEPAGE,
*/

#endif
