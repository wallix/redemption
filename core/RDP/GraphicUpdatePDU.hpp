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

struct GraphicsUpdatePDU : public RDPGraphicDevice
{
    Stream stream;
    // Internal state of orders
    RDPOrderCommon common;
    RDPDestBlt destblt;
    RDPPatBlt patblt;
    RDPScrBlt scrblt;
    RDPOpaqueRect opaquerect;
    RDPMemBlt memblt;
    RDPLineTo lineto;
    RDPGlyphIndex glyphindex;
    // state variables for gathering batch of orders
    size_t order_count;
    uint32_t offset_order_count;
    X224Out * tpdu;
    McsOut * mcs_sdin;
    SecOut * sec_out;
    ShareControlOut * out_control;
    ShareDataOut * out_data;
    Transport * trans;
    uint16_t & userid;
    int & shareid;
    int & crypt_level;
    CryptContext & encrypt;
    const Inifile * ini;

    GraphicsUpdatePDU(Transport * trans,
                      uint16_t & userid,
                      int & shareid,
                      int & crypt_level,
                      CryptContext & encrypt,
                      const Inifile * ini)
        :    stream(4096),
        // Internal state of orders
        common(RDP::PATBLT, Rect(0, 0, 1, 1)),
        destblt(Rect(), 0),
        patblt(Rect(), 0, 0, 0, RDPBrush()),
        scrblt(Rect(), 0, 0, 0),
        opaquerect(Rect(), 0),
        memblt(0, Rect(), 0, 0, 0, 0),
        lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
        glyphindex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)""),
        // state variables for a batch of orders
        order_count(0),
        offset_order_count(0),
        tpdu(NULL),
        mcs_sdin(NULL),
        sec_out(NULL),
        out_control(NULL),
        out_data(NULL),
        trans(trans),
        userid(userid),
        shareid(shareid),
        crypt_level(crypt_level),
        encrypt(encrypt),
        ini(ini)
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
        this->stream.init(4096);
        this->tpdu = new X224Out(X224Packet::DT_TPDU, this->stream);
        this->mcs_sdin = new McsOut(this->stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        this->sec_out = new SecOut(this->stream, this->crypt_level, SEC_ENCRYPT, this->encrypt);
        this->out_control = new ShareControlOut(this->stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        this->out_data = new ShareDataOut(this->stream, PDUTYPE2_UPDATE, this->shareid);

        this->stream.out_uint16_le(RDP_UPDATE_ORDERS);
        this->stream.out_clear_bytes(2); /* pad */
        this->offset_order_count = this->stream.get_offset(0);
        this->stream.out_clear_bytes(2); /* number of orders, set later */
        this->stream.out_clear_bytes(2); /* pad */
    }

    virtual void flush()
    {
        if (this->order_count > 0){
            if (this->ini->globals.debug.primary_orders){
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

    /*****************************************************************************/
    // check if the next order will fit in available packet size
    // if not send previous orders we got and init a new packet
    virtual void reserve_order(size_t asked_size)
    {
        if (this->ini->globals.debug.primary_orders){
            LOG(LOG_INFO, "GraphicsUpdatePDU::reserve_order[%u](%u) remains=%u", this->order_count, asked_size, std::min(this->stream.capacity, (size_t)4096) - this->stream.get_offset(0));
        }
        size_t max_packet_size = std::min(this->stream.capacity, (size_t)4096);
        size_t used_size = this->stream.get_offset(0);
        const size_t max_order_batch = 4096;
        if ((this->order_count >= max_order_batch)
        || (used_size + asked_size + 100) > max_packet_size) {
            this->flush();
        }
        this->order_count++;
    }

    virtual void send(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->reserve_order(23);
        RDPOrderCommon newcommon(RDP::RECT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->opaquerect);
        this->common = newcommon;
        this->opaquerect = cmd;
        if (this->ini->globals.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void send(const RDPScrBlt & cmd, const Rect &clip)
    {
        this->reserve_order(25);
        RDPOrderCommon newcommon(RDP::SCREENBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->scrblt);
        this->common = newcommon;
        this->scrblt = cmd;
        if (this->ini->globals.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void send(const RDPDestBlt & cmd, const Rect &clip)
    {
        this->reserve_order(21);
        RDPOrderCommon newcommon(RDP::DESTBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->destblt);
        this->common = newcommon;
        this->destblt = cmd;
        if (this->ini->globals.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void send(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->reserve_order(29);
        using namespace RDP;
        RDPOrderCommon newcommon(RDP::PATBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->patblt);
        this->common = newcommon;
        this->patblt = cmd;
        if (this->ini->globals.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }


    virtual void send(const RDPMemBlt & cmd, const Rect & clip)
    {
        this->reserve_order(30);
        RDPOrderCommon newcommon(RDP::MEMBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->memblt);
        this->common = newcommon;
        this->memblt = cmd;
        if (this->ini->globals.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void send(const RDPLineTo& cmd, const Rect & clip)
    {
        this->reserve_order(32);
        RDPOrderCommon newcommon(RDP::LINE, clip);
        cmd.emit(this->stream, newcommon, this->common, this->lineto);
        this->common = newcommon;
        this->lineto = cmd;
        if (this->ini->globals.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void send(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        this->reserve_order(297);
        RDPOrderCommon newcommon(RDP::GLYPHINDEX, clip);
        cmd.emit(this->stream, newcommon, this->common, this->glyphindex);
        this->common = newcommon;
        this->glyphindex = cmd;
        if (this->ini->globals.debug.primary_orders){
            LOG(LOG_INFO, "glyphIndex");
//            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void send(const RDPBrushCache & cmd)
    {
        this->reserve_order(cmd.size + 12);
        cmd.emit(this->stream);
        if (this->ini->globals.debug.secondary_orders){
            cmd.log(LOG_INFO);
        }
    }

    virtual void send(const RDPColCache & cmd)
    {
        this->reserve_order(2000);
        cmd.emit(this->stream);
        if (this->ini->globals.debug.secondary_orders){
            cmd.log(LOG_INFO);
        }
    }

    virtual void send(const RDPBmpCache & cmd)
    {
        this->reserve_order(cmd.bmp->bmp_size(cmd.bpp) + 16);
        cmd.emit(this->stream);
        if (this->ini->globals.debug.secondary_orders){
            cmd.log(LOG_INFO);
        }
    }

    virtual void send(const RDPGlyphCache & cmd)
    {
        #warning compute actual size, instead of a majoration as below
        this->reserve_order(1000);
        cmd.emit(this->stream);
        if (this->ini->globals.debug.secondary_orders){
            cmd.log(LOG_INFO);
        }
    }

};

#endif
