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

TODO("We could probably use templated Entries instead of InputType_t and StorageType_t enums, this with also avoid combinatorial explosion."
     "Well, not really, but the compiler will do the work.")

enum InputType_t
{
    INPUT_BOOLEAN,
    INPUT_UNSIGNED,
    INPUT_LEVEL,
};

enum StorageType_t
{
    STORAGE_BYTE
    STORAGE_UINT16
};

class Entry 
{
    char key[64];
    ParserType_t parser_type;
    InputType_t input_type;
    void * storage;
    
    Entry(const char * key, ParserType_t parser_type, StorageType_t input_type, void * storage) 
    : parser_type(parser_type)
    , input_type(input_type)
    , storage(storage)
    {
        strncpy(this->key, key, sizeof(this->key));
    }

    bool match(const char * key) {
        return 0 == strncasecmp(this->key, key, sizeof(this->key)); 
    }

    bool set_value(const char * key, const char * value) {
        if (!this->match(key)) {
            return false;
        }
        switch (this->parser_type) 
        {
            case INPUT_BOOLEAN:{
                unsigned long result = bool_from_cstr(value);
                this->store(result);
            }
            break;
            case INPUT_UNSIGNED:{
                unsigned long result = ulong_from_cstr(value);
                this->store(result);
            }
            break;
            case INPUT_LEVEL:{
                unsigned long level = level_from_cstr(value);
                this->store(result);
            }
            break;
        }
        return true;
    }
    
    void store(unsigned long value) {
        switch (value){
            case STORAGE_BYTE:
                static_cast<uint8_t *>(this->storage)[0] = static_cast<uint8_t>(value);
            break;
            case STORAGE_UINT16:
                static_cast<uint16_t *>(this->storage)[0] = static_cast<uint16_t>(value);
            break;
        }
    }
};

TODO("We also have to define BitmapCaps and OrderCaps loaders, then we will be able to use this in capability_sets.hpp") 

class GeneralCapsLoader 
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

    virtual bool match(const char * name) 
    {
        return 0 == strncasecmp(this->name, name, sizeof(this->name)); 
    }
    
    void set_value(const char * section, const char * key, const char * value) 
    {
        TODO("parsing like this is very, very inefficient, change that later")

        if (!this->match(section)) {
            return false;
        }
        for (size_t i = 0 ; i < NUMBER_OF_CAPS ; i++){
            if (this->entries[i].set_value(key, value)){
                return true;
            }
        }
        return false;
    }

};
#endif

#endif
