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

   RDPGraphicDevice is an abstract class that describe a device able to
   proceed RDP Drawing Orders. How the drawing will be actually done
   depends on the implementation.
   - It may be sent on the wire,
   - Used to draw on some internal bitmap,
   - etc.

*/

#if !defined(__GRAPHIC_UPDATE_PDU_HPP__)
#define __GRAPHIC_UPDATE_PDU_HPP__


#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"
#include "RDP/rdp.hpp"
#include "RDP/sec.hpp"
#include "RDP/lic.hpp"
#include "RDP/orders/RDPGraphicDevice.hpp"
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

struct GraphicsUpdatePDU : public RDPSerializer
{
    X224Out * tpdu;
    McsOut * mcs_sdin;
    SecOut * sec_out;
    ShareControlOut * out_control;
    ShareDataOut * out_data;
    uint16_t & userid;
    int & shareid;
    int & crypt_level;
    CryptContext & encrypt;

    GraphicsUpdatePDU(Transport * trans,
                      uint16_t & userid,
                      int & shareid,
                      int & crypt_level,
                      CryptContext & encrypt,
                      const Inifile * ini,
                      const uint8_t bpp,
                      uint32_t small_entries, uint32_t small_size,
                      uint32_t medium_entries, uint32_t medium_size,
                      uint32_t big_entries, uint32_t big_size,
                      const int bitmap_cache_version,
                      const int use_bitmap_comp,
                      const int op2)
        : RDPSerializer(trans, ini, 
            bpp, 
            small_entries, small_size,
            medium_entries, medium_size,
            big_entries, big_size,
            bitmap_cache_version, use_bitmap_comp, op2),
        tpdu(NULL),
        mcs_sdin(NULL),
        sec_out(NULL),
        out_control(NULL),
        out_data(NULL),
        userid(userid),
        shareid(shareid),
        crypt_level(crypt_level),
        encrypt(encrypt)
    {
        this->init();
    }

    ~GraphicsUpdatePDU(){
        if (this->tpdu){ delete this->tpdu; }
        if (this->mcs_sdin){ delete this->mcs_sdin; }
        if (this->sec_out){ delete this->sec_out; }
        if (this->out_control){ delete this->out_control; }
        if (this->out_data){ delete this->out_data; }
    }

    void init(){
        if (this->tpdu){ delete this->tpdu; }
        if (this->mcs_sdin){ delete this->mcs_sdin; }
        if (this->sec_out){ delete this->sec_out; }
        if (this->out_control){ delete this->out_control; }
        if (this->out_data){ delete this->out_data; }

        if (this->ini->globals.debug.primary_orders){
            LOG(LOG_INFO, "GraphicsUpdatePDU::init::Initializing orders batch mcs_userid=%u shareid=%u", this->userid, this->shareid);
        }
        this->stream.init(32768);
        this->tpdu = new X224Out(X224Packet::DT_TPDU, this->stream);
        this->mcs_sdin = new McsOut(this->stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        uint32_t sec_flags = this->crypt_level?SEC_ENCRYPT:0;
        this->sec_out = new SecOut(this->stream, sec_flags, this->encrypt);
        this->out_control = new ShareControlOut(this->stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        this->out_data = new ShareDataOut(this->stream, PDUTYPE2_UPDATE, this->shareid, RDP::STREAM_MED);

        this->stream.out_uint16_le(RDP_UPDATE_ORDERS);
        this->stream.out_clear_bytes(2); /* pad */
        this->offset_order_count = this->stream.get_offset(0);
        this->stream.out_clear_bytes(2); /* number of orders, set later */
        this->stream.out_clear_bytes(2); /* pad */
    }

    virtual void flush()
    {
        if (this->order_count > 0){
            if (this->ini->globals.debug.primary_orders > 63){
                LOG(LOG_INFO, "GraphicsUpdatePDU::flush: order_count=%d", this->order_count);
            }
            this->stream.set_out_uint16_le(this->order_count, this->offset_order_count);
            this->order_count = 0;

            this->out_data->end();
            this->out_control->end();
            this->sec_out->end();
            this->mcs_sdin->end();
            this->tpdu->end();
            this->tpdu->send(this->trans);
            this->init();
        }
    }

};

#endif
