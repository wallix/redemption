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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Channels descriptors

*/

#if !defined(CORE_CHANNEL_LIST_HPP__)
#define CORE_CHANNEL_LIST_HPP__

struct ChannelDef {
    char name[16];

    enum {
        PACKET_COMPR_TYPE_8K    = 0x0, // RDP 4.0 bulk compression (see section 3.1.8.4.1).
        PACKET_COMPR_TYPE_64K   = 0x1, // RDP 5.0 bulk compression (see section 3.1.8.4.2).
        PACKET_COMPR_TYPE_RDP6  = 0x2, // 6.0 bulk compression (see [MS-RDPEGDI] section 3.1.8.1).
        PACKET_COMPR_TYPE_RDP61 = 0x3, // 6.1 bulk compression (see [MS-RDPEGDI] section 3.1.8.2).
    };

    enum {
        CHANNEL_CHUNK_LENGTH = 8192,
    };

    enum {
        CHANNEL_FLAG_FIRST           = 0x00000001,
        CHANNEL_FLAG_LAST            = 0x00000002,
        CHANNEL_FLAG_SHOW_PROTOCOL   = 0x00000010,
        CHANNEL_FLAG_SUSPEND         = 0x00000020,
        CHANNEL_FLAG_RESUME          = 0x00000040,
        CHANNEL_PACKET_COMPRESSED    = 0x00200000,
        CHANNEL_PACKET_AT_FRONT      = 0x00400000,
        CHANNEL_PACKET_FLUSHED       = 0x00800000,
        CompressionTypeMask          = 0x000F0000,
    };

    enum { CB_MONITOR_READY        = 0x0001
         , CB_FORMAT_LIST          = 0x0002
         , CB_FORMAT_LIST_RESPONSE = 0x0003
         , CB_FORMAT_DATA_REQUEST  = 0x0004
         , CB_FORMAT_DATA_RESPONSE = 0x0005
         , CB_TEMP_DIRECTORY       = 0x0006
         , CB_CLIP_CAPS            = 0x0007
         , CB_FILECONTENTS_REQUEST = 0x0008
    };

    uint32_t flags;
    int chanid;

    ChannelDef(){
        this->name[0] = 0;
        this->flags = 0;
        this->chanid = 0;
    }

    void log(unsigned index){
        LOG(LOG_INFO, "ChannelDef[%u]::(name = %s, flags = %8x, chanid = %u)", 
            index, this->name, (unsigned)this->flags, (unsigned)this->chanid);
    }
};

class ChannelDefArray
{
    // The number of requested static virtual channels (the maximum allowed is 31).
    size_t channelCount;
    ChannelDef items[32];

public:
    ChannelDefArray() : channelCount(0) {}

    const ChannelDef & operator[](size_t index) const {
        return this->items[index];
    }

    void set_chanid(size_t index, int chanid){
        this->items[index].chanid = chanid;
    }

    size_t size() const {
        return this->channelCount;
    }

    void push_back(const ChannelDef & item){
        this->items[this->channelCount] = item;
        this->channelCount++;
    }

    const ChannelDef * get(const char * const name) const
    {
        const ChannelDef * channel = NULL;
        for (size_t index = 0; index < this->size(); index++){
            const ChannelDef & item = this->items[index];
            if (strcmp(name, item.name) == 0){
                channel = &item;
                break;
            }
        }
        return channel;
    }

    void log(char * name){
        LOG(LOG_INFO, "%s channels %u channels defined", name, this->channelCount);
        for (unsigned index = 0 ; index < this->channelCount ; index++){
            this->items[index].log(index);
        }
    }
};


#endif
