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

// 2.2.1.3.4.1 Channel Definition Structure (CHANNEL_DEF)
// ------------------------------------------------------
// The CHANNEL_DEF packet contains information for a particular static
// virtual channel.

// name (8 bytes): An 8-byte array containing a null-terminated collection
//                 of seven ANSI characters that uniquely identify the
//                 channel.

// options (4 bytes): A 32-bit, unsigned integer. Channel option flags.
//
//           Flag                             Meaning
// -------------------------------------------------------------------------
// CHANNEL_OPTION_INITIALIZED   Absence of this flag indicates that this
//        0x80000000            channel is a placeholder and that the
//                              server MUST NOT set it up.
// -----------------------------------------------------------------------
// CHANNEL_OPTION_ENCRYPT_RDP   This flag is unused and its value MUST be
//        0x40000000            ignored by the server.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_ENCRYPT_SC    This flag is unused and its value MUST be
//        0x20000000            ignored by the server.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_ENCRYPT_CS    This flag is unused and its value MUST be
//        0x10000000            ignored by the server.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_PRI_HIGH      Channel data MUST be sent with high MCS
//        0x08000000            priority.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_PRI_MED       Channel data MUST be sent with medium
//        0x04000000            MCS priority.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_PRI_LOW       Channel data MUST be sent with low MCS
//        0x02000000            priority.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_COMPRESS_RDP  Virtual channel data MUST be compressed
//        0x00800000            if RDP data is being compressed.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_COMPRESS      Virtual channel data MUST be compressed,
//        0x00400000            regardless of RDP compression settings.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_SHOW_PROTOCOL The value of this flag MUST be ignored by
//        0x00200000            the server. The visibility of the Channel
//                              PDU Header (section 2.2.6.1.1) is
//                              determined by the CHANNEL_FLAG_SHOW_PROTOCOL
//                              (0x00000010) flag as defined in the flags
//                              field (section 2.2.6.1.1).
// -------------------------------------------------------------------------
// REMOTE_CONTROL_PERSISTENT    Channel MUST be persistent across remote
//        0x00100000            control transactions.



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

        CHANNEL_OPTION_INITIALIZED   = 0x80000000,
        CHANNEL_OPTION_ENCRYPT_RDP   = 0x40000000,
        CHANNEL_OPTION_ENCRYPT_SC    = 0x20000000,
        CHANNEL_OPTION_ENCRYPT_CS    = 0x10000000,
        CHANNEL_OPTION_PRI_HIGH      = 0x08000000,
        CHANNEL_OPTION_PRI_MED       = 0x04000000,
        CHANNEL_OPTION_PRI_LOW       = 0x02000000,
        CHANNEL_OPTION_COMPRESS_RDP  = 0x00800000,
        CHANNEL_OPTION_COMPRESS      = 0x00400000,
        CHANNEL_OPTION_SHOW_PROTOCOL = 0x00200000,
        REMOTE_CONTROL_PERSISTENT    = 0x00100000,
    };

    uint32_t flags;
    int chanid;

    ChannelDef(){
        this->name[0] = 0;
        this->flags = 0;
        this->chanid = 0;
    }

    void log(){
        LOG(LOG_INFO, "Channel %s [%u]", this->name, this->chanid);
        if (this->flags & CHANNEL_OPTION_INITIALIZED){
            LOG(LOG_INFO, "channel::options::INITIALIZED");
        }
        if (this->flags & CHANNEL_OPTION_PRI_HIGH){
            LOG(LOG_INFO, "channel::options::PRI_HIGH");
        }
        if (this->flags & CHANNEL_OPTION_PRI_MED){
            LOG(LOG_INFO, "channel::options::PRI_MED");
        }
        if (this->flags & CHANNEL_OPTION_PRI_LOW){
            LOG(LOG_INFO, "channel::options::PRI_LOW");
        }
        if (this->flags & CHANNEL_OPTION_COMPRESS_RDP){
            LOG(LOG_INFO, "channel::options::COMPRESS_RDP");
        }
        if (this->flags & CHANNEL_OPTION_COMPRESS){
            LOG(LOG_INFO, "channel::options::COMPRESS");
        }
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
};


#endif
