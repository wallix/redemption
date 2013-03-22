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

#ifndef _REDEMPTION_CORE_CHANNEL_LIST_HPP_
#define _REDEMPTION_CORE_CHANNEL_LIST_HPP_

#define CLIPBOARD_VIRTUAL_CHANNEL_NAME "cliprdr"

/* Clipboard constants, "borrowed" from GCC system headers in the w32 cross compiler */
enum {
    CF_TEXT                        = 1,
    CF_BITMAP                      = 2,
    CF_METAFILEPICT                = 3,
    CF_SYLK                        = 4,
    CF_DIF                         = 5,
    CF_TIFF                        = 6,
    CF_OEMTEXT                     = 7,
    CF_DIB                         = 8,
    CF_PALETTE                     = 9,
    CF_PENDATA                     = 10,
    CF_RIFF                        = 11,
    CF_WAVE                        = 12,
    CF_UNICODETEXT                 = 13,
    CF_ENHMETAFILE                 = 14,
    CF_HDROP                       = 15,
    CF_LOCALE                      = 16,
    CF_MAX                         = 17,
    CF_OWNERDISPLAY                = 128,
    CF_DSPTEXT                     = 129,
    CF_DSPBITMAP                   = 130,
    CF_DSPMETAFILEPICT             = 131,
    CF_DSPENHMETAFILE              = 142,
    CF_PRIVATEFIRST                = 512,
    CF_PRIVATELAST                 = 767,
    CF_GDIOBJFIRST                 = 768,
    CF_GDIOBJLAST                  = 1023,
};

//    1.3.3 Static Virtual Channels
//    =============================

//    Static virtual channels allow lossless communication between client and server components over the
//    main RDP data connection. Virtual channel data is application-specific and opaque to RDP. A
//    maximum of 30 static virtual channels can be created at connection time.
//    The list of desired virtual channels is requested and confirmed during the Basic Settings Exchange
//    phase of the connection sequence (as specified in section 1.3.1.1) and the endpoints are joined
//    during the Channel Connection phase (as specified in section 1.3.1.1). Once joined, the client and
//    server endpoints should be prevented from exchanging data until the connection sequence has
//    completed.
//    Static virtual channel data must be broken up into chunks of up to 1600 bytes in size before being
//    transmitted (this size does not include "DP headers). Each virtual channel acts as an independent
//    data stream. The client and server examine the data received on each virtual channel and route the
//    data stream to the appropriate endpoint for further processing. A particular client or server
//    implementation can decide whether to pass on individual chunks of data as they are received, or to
//    assemble the separate chunks of data into a complete block before passing it on to the endpoint.

//    2.2.1 Clipboard PDU Header (CLIPRDR_HEADER)
//    ===========================================

//    The CLIPRDR_HEADER structure is present in all clipboard PDUs. It is used to identify the PDU type,
//    specify the length of the PDU, and convey message flags.

// 0x0001 CB_MONITOR_READY Monitor Ready PDU
// 0x0002 CB_FORMAT_LIST Format List PDU
// 0x0003 CB_FORMAT_LIST_RESPONSE Format List Response PDU
// 0x0004 CB_FORMAT_DATA_REQUEST Format Data Request PDU
// 0x0005 CB_FORMAT_DATA_RESPONSE Format Data Response PDU
// 0x0006 CB_TEMP_DIRECTORY Temporary Directory PDU
// 0x0007 CB_CLIP_CAPS Clipboard Capabilities PDU
// 0x0008 CB_FILECONTENTS_REQUEST File Contents Request PDU
// 0x0009 CB_FILECONTENTS_RESPONSE File Contents Response PDU
// 0x000A CB_LOCK_CLIPDATA Lock Clipboard Data PDU
// 0x000B CB_UNLOCK_CLIPDATA Unlock Clipboard Data PDU

// dataLen (4 bytes): An unsigned, 32-bit integer that specifies the size, in bytes, of the data
// which follows the Clipboard PDU Header.


// 0x0001 CB_RESPONSE_OK   Used by the Format List Response PDU, Format Data Response PDU, and File Contents Response PDU
//                        to indicate that the associated request Format List PDU, Format Data Request PDU, and File
//                        Contents Request PDU were processed successfully.
//                        
// 0x0002 CB_RESPONSE_FAIL Used by the Format List Response PDU, Format Data Response PDU, and File Contents Response PDU
//                        to indicate that the associated Format List PDU, Format Data Request PDU, and File Contents 
//                        Request PDU were not processed successfully.

// 0x0004 CB_ASCII_NAMES   Used by the Short Format Name variant of the Format List Response PDU to indicate the format
//                        names are in ASCII 8.




struct ChannelDef {
    char name[16];

    enum {
        PACKET_COMPR_TYPE_8K    = 0x0, // RDP 4.0 bulk compression (see section 3.1.8.4.1).
        PACKET_COMPR_TYPE_64K   = 0x1, // RDP 5.0 bulk compression (see section 3.1.8.4.2).
        PACKET_COMPR_TYPE_RDP6  = 0x2, // 6.0 bulk compression (see [MS-RDPEGDI] section 3.1.8.1).
        PACKET_COMPR_TYPE_RDP61 = 0x3, // 6.1 bulk compression (see [MS-RDPEGDI] section 3.1.8.2).
    };

    enum {
        CHANNEL_CHUNK_LENGTH = 1600,
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


    enum { CB_MONITOR_READY         = 0x0001
         , CB_FORMAT_LIST           = 0x0002
         , CB_FORMAT_LIST_RESPONSE  = 0x0003
         , CB_FORMAT_DATA_REQUEST   = 0x0004
         , CB_FORMAT_DATA_RESPONSE  = 0x0005
         , CB_TEMP_DIRECTORY        = 0x0006
         , CB_CLIP_CAPS             = 0x0007
         , CB_FILECONTENTS_REQUEST  = 0x0008
         , CB_FILECONTENTS_RESPONSE = 0x0009 
         , CB_LOCK_CLIPDATA         = 0x000A
         , CB_UNLOCK_CLIPDATA       = 0x000B
    };


    enum { CB_RESPONSE_OK   = 0x0001
         , CB_RESPONSE_FAIL = 0x0002
    };

    uint32_t flags;
    int chanid;

    ChannelDef(){
        this->name[0] = 0;
        this->flags = 0;
        this->chanid = 0;
    }

    void log(unsigned index) const {
        LOG(LOG_INFO, "ChannelDef[%u]::(name = %s, flags = %8x, chanid = %u)", 
            index, this->name, (unsigned)this->flags, (unsigned)this->chanid);
    }
};

class ChannelDefArray
{
public:
    // The number of requested static virtual channels (the maximum allowed is 31).
    size_t channelCount;
    ChannelDef items[32];

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

    const ChannelDef * get(int chanid) const
    {
        const ChannelDef * channel = NULL;
        for (size_t index = 0; index < this->size(); index++){
            const ChannelDef & item = this->items[index];
            if (item.chanid == chanid){
                channel = &item;
                break;
            }
        }
        return channel;
    }

    int get_index(const char * const name) const
    {
        int res = -1;
        for (size_t index = 0; index < this->size(); index++){
            if (strcmp(name, this->items[index].name) == 0){
                res = index;
                break;
            }
        }
        return res;
    }

    void log(char * name) const {
        LOG(LOG_INFO, "%s channels %u channels defined", name, this->channelCount);
        for (unsigned index = 0 ; index < this->channelCount ; index++){
            this->items[index].log(index);
        }
    }
};


#endif
