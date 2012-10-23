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
   Author(s): Christophe Grosjean, Jonathan Poelen

   RDPGraphicDevice is an abstract class that describe a device able to
   proceed RDP Drawing Orders. How the drawing will be actually done
   depends on the implementation.
   - It may be sent on the wire,
   - Used to draw on some internal bitmap,
   - etc.

*/

#if !defined(__GRAPHIC_TO_FILE_HPP__)
#define __GRAPHIC_TO_FILE_HPP__

#include <sys/time.h>
#include <ctime>

#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"
#include "RDP/rdp.hpp"
#include "RDP/sec.hpp"
#include "RDP/lic.hpp"
#include "RDP/RDPSerializer.hpp"
#include "difftimeval.hpp"

// MS-RDPECGI 2.2.2.2 Fast-Path Orders Update (TS_FP_UPDATE_ORDERS)
// ================================================================
// The TS_FP_UPDATE_ORDERS structure contains primary, secondary, and alternate
// secondary drawing orders aligned on byte boundaries. This structure conforms
// to the layout of a Fast-Path Update (see [MS-RDPBCGR] section 2.2.9.1.2.1)
// and is encapsulated within a Fast-Path Update PDU (see [MS-RDPBCGR] section
// 2.2.9.1.2.1.1).

// updateHeader (1 byte): An 8-bit, unsigned integer. The format of this field
//   is the same as the updateHeader byte field described in the Fast-Path
//   Update structure (see [MS-RDPBCGR] section 2.2.9.1.2.1). The updateCode
//   bitfield (4 bits in size) MUST be set to FASTPATH_UPDATETYPE_ORDERS (0x0).

// compressionFlags (1 byte): An 8-bit, unsigned integer. The format of this
//   optional field (as well as the possible values) is the same as the
//   compressionFlags field described in the Fast-Path Update structure
//   specified in [MS-RDPBCGR] section 2.2.9.1.2.1.

// size (2 bytes): A 16-bit, unsigned integer. The format of this field (as well
//   as the possible values) is the same as the size field described in the
//   Fast-Path Update structure specified in [MS-RDPBCGR] section 2.2.9.1.2.1.

// numberOrders (2 bytes): A 16-bit, unsigned integer. The number of Drawing
//   Order (section 2.2.2.1.1) structures contained in the orderData field.

// orderData (variable): A variable-sized array of Drawing Order (section
//   2.2.2.1.1) structures packed on byte boundaries. Each structure contains a
//   primary, secondary, or alternate secondary drawing order. The controlFlags
//   field of the Drawing Order identifies the type of drawing order.


// MS-RDPECGI 2.2.2.1 Orders Update (TS_UPDATE_ORDERS_PDU_DATA)
// ============================================================
// The TS_UPDATE_ORDERS_PDU_DATA structure contains primary, secondary, and
// alternate secondary drawing orders aligned on byte boundaries. This structure
// conforms to the layout of a Slow Path Graphics Update (see [MS-RDPBCGR]
// section 2.2.9.1.1.3.1) and is encapsulated within a Graphics Update PDU (see
// [MS-RDPBCGR] section 2.2.9.1.1.3.1.1).

// shareDataHeader (18 bytes): Share Data Header (see [MS-RDPBCGR], section
//   2.2.8.1.1.1.2) containing information about the packet. The type subfield
//   of the pduType field of the Share Control Header (section 2.2.8.1.1.1.1)
//   MUST be set to PDUTYPE_DATAPDU (7). The pduType2 field of the Share Data
//   Header MUST be set to PDUTYPE2_UPDATE (2).

// updateType (2 bytes): A 16-bit, unsigned integer. The field contains the
//   graphics update type. This field MUST be set to UPDATETYPE_ORDERS (0x0000).

// pad2OctetsA (2 bytes): A 16-bit, unsigned integer used as a padding field.
//   Values in this field are arbitrary and MUST be ignored.

// numberOrders (2 bytes): A 16-bit, unsigned integer. The number of Drawing
//   Order (section 2.2.2.1.1) structures contained in the orderData field.

// pad2OctetsB (2 bytes): A 16-bit, unsigned integer used as a padding field.
//   Values in this field are arbitrary and MUST be ignored.

// orderData (variable): A variable-sized array of Drawing Order (section
//   2.2.2.1.1) structures packed on byte boundaries. Each structure contains a
//   primary, secondary, or alternate secondary drawing order. The controlFlags
//   field of the Drawing Order identifies the type of drawing order.

struct GraphicToFile : public RDPSerializer
REDOC("To keep things easy all chunks have 8 bytes headers"
      " starting with chunk_type, chunk_size"
      " and order_count (whatever it means, depending on chunks")
{
    timeval last_sent_timer;
    timeval timer;
    const uint16_t width;
    const uint16_t height;
    const uint8_t  bpp;


    GraphicToFile(Transport * trans
                , Stream * pstream
                , const Inifile * ini
                , const uint16_t width
                , const uint16_t height
                , const uint8_t  bpp
                , uint32_t small_entries
                , uint32_t small_size
                , uint32_t medium_entries
                , uint32_t medium_size
                , uint32_t big_entries
                , uint32_t big_size
                , const timeval& now)
    : RDPSerializer(trans, pstream, ini, bpp,
                    small_entries, small_size,
                    medium_entries, medium_size,
                    big_entries, big_size,
                    0, 1, 1)
    , timer(now)
    , width(width)
    , height(height)
    , bpp(bpp)
    {
        last_sent_timer.tv_sec = 0;
        last_sent_timer.tv_usec = 0;
        this->order_count = 0;
        
        this->send_meta_chunk();
    }

    ~GraphicToFile(){
    }

    virtual void timestamp(const timeval& now)
    REDOC("Update timestamp but send nothing, the timestamp will be sent later with the next effective event")
    {
        uint64_t old_timer = this->timer.tv_sec * 1000000ULL + this->timer.tv_usec;
        uint64_t current_timer = now.tv_sec * 1000000ULL + now.tv_usec;
        if (old_timer < current_timer){
            this->timer = now;
        }
    }

    class WRMChunk_Send
    {
        public:
        WRMChunk_Send(Stream & stream, uint16_t chunktype, uint16_t data_size, uint16_t count)
        {
            stream.out_uint16_le(chunktype);
            stream.out_uint16_le(8 + data_size);
            stream.out_uint16_le(count);
            stream.out_uint16_le(0);
            stream.mark_end();
        } 
    };

    void send_meta_chunk(void)
    {
        BStream stream(8);
        stream.out_uint16_le(this->width);
        stream.out_uint16_le(this->height);
        stream.out_uint16_le(this->bpp);
        stream.out_uint16_le(0);
        stream.mark_end();

        BStream header(8);
        WRMChunk_Send chunk(header, WRMChunk::META_FILE, 8, 1);
        this->trans->send(header.data, header.size());
        this->trans->send(stream.data, stream.size());
    }


    void send_timestamp_chunk(void)
    {
        uint64_t old_timer = this->last_sent_timer.tv_sec * 1000000ULL + this->last_sent_timer.tv_usec;
        uint64_t current_timer = this->timer.tv_sec * 1000000ULL + this->timer.tv_usec;
        if (old_timer < current_timer){
            BStream stream(8);
            stream.out_uint64_le(current_timer);
            stream.mark_end();

            BStream header(8);
            WRMChunk_Send chunk(header, WRMChunk::TIMESTAMP, 8, 1);
            this->trans->send(header.data, header.size());
            this->trans->send(stream.data, stream.size());
        }
    }

    virtual void flush()
    {
        if (this->order_count > 0){
            this->send_timestamp_chunk();
            if (this->ini && this->ini->globals.debug.primary_orders){
                LOG(LOG_INFO, "GraphicToFile::flush: order_count=%d", this->order_count);
            }
            this->send_orders();
        }
    }

    void send_orders()
    {
        this->pstream->mark_end();
        BStream header(8);
        WRMChunk_Send chunk(header, RDP_UPDATE_ORDERS, this->pstream->size(), this->order_count);
        this->trans->send(header.data, header.size());
        this->trans->send(this->pstream->data, this->pstream->size());
        this->order_count = 0;
        this->pstream->reset();
    }

};

#endif
