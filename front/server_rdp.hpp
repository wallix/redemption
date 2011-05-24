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

   header file. rdp layer at core module

*/

#if !defined(__RDP_HPP__)
#define __RDP_HPP__

#include "server_sec.hpp"
#include "client_info.hpp"
#include "config.hpp"
#include "error.hpp"
#include "callback.hpp"
#include "colors.hpp"
#include "altoco.hpp"

/* rdp */
struct server_rdp {
    int up_and_running;

    Callback & cb;
    int share_id;
    int mcs_channel;
    struct ClientInfo client_info;
    struct server_sec sec_layer;
    Stream front_stream;

    server_rdp(Callback & cb, Transport * trans, Inifile * ini)
        :
        up_and_running(0),
        cb(cb),
        share_id(65538),
        mcs_channel(0),
        client_info(ini),
        sec_layer(&client_info, trans),
        front_stream(16384)
    {
    }

    ~server_rdp()
    {
    }

    void server_send_to_channel(int channel_id, uint8_t *data, int data_len,
                               int total_data_len, int flags) throw (Error)
    {
        Stream stream(data_len + 1024); /* this should be big enough */
        this->sec_layer.server_sec_init(stream);
        this->sec_layer.mcs_layer.server_channel_init(&stream);
        stream.out_copy_bytes(data, data_len);
        stream.mark_end();

        this->sec_layer.mcs_layer.server_channel_send(
            stream, channel_id, total_data_len, flags);
        this->sec_layer.server_sec_send(stream, channel_id);
    }

    // Global palette cf [MS-RDPCGR] 2.2.9.1.1.3.1.1.1 Palette Update Data
    // -------------------------------------------------------------------

    // updateType (2 bytes): A 16-bit, unsigned integer. The graphics update type.
    // This field MUST be set to UPDATETYPE_PALETTE (0x0002).

    // pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding.
    // Values in this field are ignored.

    // numberColors (4 bytes): A 32-bit, unsigned integer.
    // The number of RGB triplets in the paletteData field.
    // This field MUST be set to NUM_8BPP_PAL_ENTRIES (256).

    void server_send_palette(const RGBPalette & palette) throw (Error)
    {
        #warning we should create some RDPData object created on init and sent before destruction
        Stream stream(8192);
        this->sec_layer.server_sec_init(stream);
        stream.rdp_hdr = stream.p;
        stream.p += 18;
        stream.out_uint16_le(RDP_UPDATE_PALETTE);

        stream.out_uint16_le(0);

        stream.out_uint16_le(256); /* # of colors */
        stream.out_uint16_le(0);

        for (int i = 0; i < 256; i++) {
            int color = palette[i];
            uint8_t r = color >> 16;
            uint8_t g = color >> 8;
            uint8_t b = color;
            stream.out_uint8(b);
            stream.out_uint8(g);
            stream.out_uint8(r);
        }
        stream.mark_end();

        stream.p = stream.rdp_hdr;
        int len = stream.end - stream.p;
        stream.out_uint16_le(len);
        stream.out_uint16_le(0x10 | RDP_PDU_DATA);
        stream.out_uint16_le(this->mcs_channel);
        stream.out_uint32_le(this->share_id);
        stream.out_uint8(0);
        stream.out_uint8(1);
        stream.out_uint16_le(len - 14);
        stream.out_uint8(RDP_DATA_PDU_UPDATE);
        stream.out_uint8(0);
        stream.out_uint16_le(0);

        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);


    }

//    2.2.9.1.1.4     Server Pointer Update PDU (TS_POINTER_PDU)
//    ----------------------------------------------------------
//    The Pointer Update PDU is sent from server to client and is used to convey
//    pointer information, including pointers' bitmap images, use of system or
//    hidden pointers, use of cached cursors and position updates.

//    tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

//    x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//      section 13.7.

//    mcsSDin (variable): Variable-length PER-encoded MCS Domain PDU which
//      encapsulates an MCS Send Data Indication structure, as specified in
//      [T125] (the ASN.1 structure definitions are given in [T125] section 7,
//      parts 7 and 10). The userData field of the MCS Send Data Indication
//      contains a Security Header and the Pointer Update PDU data.

//    securityHeader (variable): Optional security header. If the Encryption
//      Level (sections 5.3.2 and 2.2.1.4.3) selected by the server is greater
//      than ENCRYPTION_LEVEL_NONE (0) and the Encryption Method
//      (sections 5.3.2 and 2.2.1.4.3) selected by the server is greater than
//      ENCRYPTION_METHOD_NONE (0) then this field will contain one of the
//      following headers:

//      - Basic Security Header (section 2.2.8.1.1.2.1) if the Encryption Level
//        selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//        ENCRYPTION_LEVEL_LOW (1).

//      - Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption
//        Level selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//        ENCRYPTION_LEVEL_CLIENT_COMPATIBLE (2) or ENCRYPTION_LEVEL_HIGH (3).

//      - FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Level
//        selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//        ENCRYPTION_LEVEL_FIPS (4).

//      If the Encryption Level (sections 5.3.2 and 2.2.1.4.3) selected by the
//      server is ENCRYPTION_LEVEL_NONE (0) and the Encryption Method (sections
//      5.3.2 and 2.2.1.4.3) selected by the server is ENCRYPTION_METHOD_NONE
//      (0), then this header is not included in the PDU.

//    shareDataHeader (18 bytes): Share Data Header (section 2.2.8.1.1.1.2)
//      containing information about the packet. The type subfield of the
//      pduType field of the Share Control Header (section 2.2.8.1.1.1.1) MUST
//      be set to PDUTYPE_DATAPDU (7). The pduType2 field of the Share Data
//      Header MUST be set to PDUTYPE2_POINTER (27).

//    messageType (2 bytes): A 16-bit, unsigned integer. Type of pointer update.

//    +--------------------------------+---------------------------------------+
//    | 0x0001 TS_PTRMSGTYPE_SYSTEM    | Indicates a System Pointer Update     |
//    |                                | (section 2.2.9.1.1.4.3).              |
//    +--------------------------------+---------------------------------------+
//    | 0x0003 TS_PTRMSGTYPE_POSITION  | Indicates a Pointer Position Update   |
//    |                                | (section 2.2.9.1.1.4.2).              |
//    +--------------------------------+---------------------------------------+
//    | 0x0006 TS_PTRMSGTYPE_COLOR     | Indicates a Color Pointer Update      |
//    |                                | (section 2.2.9.1.1.4.4).              |
//    +--------------------------------+---------------------------------------+
//    | 0x0007 TS_PTRMSGTYPE_CACHED    | Indicates a Cached Pointer Update     |
//    |                                | (section 2.2.9.1.1.4.6).              |
//    +--------------------------------+---------------------------------------+
//    | 0x0008 TS_PTRMSGTYPE_POINTER   | Indicates a New Pointer Update        |
//    |                                | (section 2.2.9.1.1.4.5).              |
//    +--------------------------------+---------------------------------------+


//    2.2.9.1.1.4.2     Pointer Position Update (TS_POINTERPOSATTRIBUTE)
//    -------------------------------------------------------------------
//    The TS_POINTERPOSATTRIBUTE structure is used to indicate that the client
//    pointer should be moved to the specified position relative to the top-left
//    corner of the server's desktop (see [T128] section 8.14.4).

//    position (4 bytes): Point (section 2.2.9.1.1.4.1) structure containing
//     the new x-coordinates and y-coordinates of the pointer.
//            2.2.9.1.1.4.1  Point (TS_POINT16)
//            ---------------------------------
//            The TS_POINT16 structure specifies a point relative to the
//            top-left corner of the server's desktop.

//            xPos (2 bytes): A 16-bit, unsigned integer. The x-coordinate
//              relative to the top-left corner of the server's desktop.

//            yPos (2 bytes): A 16-bit, unsigned integer. The y-coordinate
//              relative to the top-left corner of the server's desktop.



//    2.2.9.1.1.4.3     System Pointer Update (TS_SYSTEMPOINTERATTRIBUTE)
//    -------------------------------------------------------------------
//    The TS_SYSTEMPOINTERATTRIBUTE structure is used to hide the pointer or to
//    set its shape to that of the operating system default (see [T128] section
//    8.14.1).

//    systemPointerType (4 bytes): A 32-bit, unsigned integer.
//    The type of system pointer.
//    +---------------------------|------------------------------+
//    | 0x00000000 SYSPTR_NULL    | The hidden pointer.          |
//    +---------------------------|------------------------------+
//    | 0x00007F00 SYSPTR_DEFAULT | The default system pointer.  |
//    +---------------------------|------------------------------+


//    2.2.9.1.1.4.4     Color Pointer Update (TS_COLORPOINTERATTRIBUTE)
//    -----------------------------------------------------------------
//    The TS_COLORPOINTERATTRIBUTE structure represents a regular T.128 24 bpp
//    color pointer, as specified in [T128] section 8.14.3. This pointer update
//    is used for both monochrome and color pointers in RDP.

    void server_rdp_send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y) throw (Error)
    {
        Stream stream(8192);
        this->sec_layer.server_sec_init(stream);
        stream.rdp_hdr = stream.p;
        stream.p += 18;

        stream.out_uint16_le(RDP_POINTER_COLOR);
        stream.out_uint16_le(0); /* pad */

//    cacheIndex (2 bytes): A 16-bit, unsigned integer. The zero-based cache
//      entry in the pointer cache in which to store the pointer image. The
//      number of cache entries is negotiated using the Pointer Capability Set
//      (section 2.2.7.1.5).

        stream.out_uint16_le(cache_idx);

//    hotSpot (4 bytes): Point (section 2.2.9.1.1.4.1) structure containing the
//      x-coordinates and y-coordinates of the pointer hotspot.
//            2.2.9.1.1.4.1  Point (TS_POINT16)
//            ---------------------------------
//            The TS_POINT16 structure specifies a point relative to the
//            top-left corner of the server's desktop.

//            xPos (2 bytes): A 16-bit, unsigned integer. The x-coordinate
//              relative to the top-left corner of the server's desktop.

        stream.out_uint16_le(x);

//            yPos (2 bytes): A 16-bit, unsigned integer. The y-coordinate
//              relative to the top-left corner of the server's desktop.

        stream.out_uint16_le(y);

//    width (2 bytes): A 16-bit, unsigned integer. The width of the pointer in
//      pixels (the maximum allowed pointer width is 32 pixels).

        stream.out_uint16_le(32);

//    height (2 bytes): A 16-bit, unsigned integer. The height of the pointer
//      in pixels (the maximum allowed pointer height is 32 pixels).

        stream.out_uint16_le(32);

//    lengthAndMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//      the andMaskData field.

        stream.out_uint16_le(128);

//    lengthXorMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//      the xorMaskData field.

        stream.out_uint16_le(3072);

//    xorMaskData (variable): Variable number of bytes: Contains the 24-bpp,
//      bottom-up XOR mask scan-line data. The XOR mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 3x3 pixel cursor
//      is being sent, then each scan-line will consume 10 bytes (3 pixels per
//      scan-line multiplied by 3 bpp, rounded up to the next even number of
//      bytes).


        #warning a memcopy (or equivalent build in stream) would be much more efficient
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                stream.out_uint8(*data++);
                stream.out_uint8(*data++);
                stream.out_uint8(*data++);
            }
        }

//    andMaskData (variable): Variable number of bytes: Contains the 1-bpp,
//      bottom-up AND mask scan-line data. The AND mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 7x7 pixel cursor
//      is being sent, then each scan-line will consume 2 bytes (7 pixels per
//      scan-line multiplied by 1 bpp, rounded up to the next even number of
//      bytes).


        stream.out_copy_bytes(mask, 128); /* mask */

//    colorPointerData (1 byte): Single byte representing unused padding.
//      The contents of this byte should be ignored.

        stream.mark_end();

        stream.p = stream.rdp_hdr;
        int len = stream.end - stream.p;
        stream.out_uint16_le(len);
        stream.out_uint16_le(0x10 | RDP_PDU_DATA);
        stream.out_uint16_le(this->mcs_channel);
        stream.out_uint32_le(this->share_id);
        stream.out_uint8(0);
        stream.out_uint8(1);
        stream.out_uint16_le(len - 14);
        stream.out_uint8(RDP_DATA_PDU_POINTER);
        stream.out_uint8(0);
        stream.out_uint16_le(0);

        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);

    }


//    2.2.9.1.1.4.5    New Pointer Update (TS_POINTERATTRIBUTE)
//    ---------------------------------------------------------
//    The TS_POINTERATTRIBUTE structure is used to send pointer data at an
//    arbitrary color depth. Support for the New Pointer Update is advertised
//    in the Pointer Capability Set (section 2.2.7.1.5).

//    xorBpp (2 bytes): A 16-bit, unsigned integer. The color depth in
//      bits-per-pixel of the XOR mask contained in the colorPtrAttr field.

//    colorPtrAttr (variable): Encapsulated Color Pointer Update (section
//      2.2.9.1.1.4.4) structure which contains information about the pointer.
//      The Color Pointer Update fields are all used, as specified in section
//      2.2.9.1.1.4.4; however, the XOR mask data alignment packing is slightly
//      different. For monochrome (1 bpp) pointers the XOR data is always padded
//      to a 4-byte boundary per scan line, while color pointer XOR data is
//      still packed on a 2-byte boundary. Color XOR data is presented in the
///     color depth described in the xorBpp field (for 8 bpp, each byte contains
//      one palette index; for 4 bpp, there are two palette indices per byte).

//    2.2.9.1.1.4.6    Cached Pointer Update (TS_CACHEDPOINTERATTRIBUTE)
//    ------------------------------------------------------------------
//    The TS_CACHEDPOINTERATTRIBUTE structure is used to instruct the client to
//    change the current pointer shape to one already present in the pointer
//    cache.

//    cacheIndex (2 bytes): A 16-bit, unsigned integer. A zero-based cache entry
//      containing the cache index of the cached pointer to which the client's
//      pointer should be changed. The pointer data should have already been
//      cached using either the Color Pointer Update (section 2.2.9.1.1.4.4) or
//      New Pointer Update (section 2.2.9.1.1.4.5).

    void server_rdp_set_pointer(int cache_idx) throw (Error)
    {
        Stream stream(8192);
        this->sec_layer.server_sec_init(stream);
        stream.rdp_hdr = stream.p;
        stream.p += 18;

        stream.out_uint16_le(RDP_POINTER_CACHED);
        stream.out_uint16_le(0); /* pad */
        stream.out_uint16_le(cache_idx);
        stream.mark_end();

        stream.p = stream.rdp_hdr;
        int len = stream.end - stream.p;
        stream.out_uint16_le(len);
        stream.out_uint16_le(0x10 | RDP_PDU_DATA);
        stream.out_uint16_le(this->mcs_channel);
        stream.out_uint32_le(this->share_id);
        stream.out_uint8(0);
        stream.out_uint8(1);
        stream.out_uint16_le(len - 14);
        stream.out_uint8(RDP_DATA_PDU_POINTER);
        stream.out_uint8(0);
        stream.out_uint16_le(0);

        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);

    }


    #warning activate_and_process_data is horrible because it does two largely unrelated tasks. One is to wait for the client to be up and running (initialization phase) the other is management of normal rdp packets once initialisation is finished. We should be able to separate both tasks, but it's not easy as code is quite intricated between layers.
    void activate_and_process_data()
    {
        int cont = 1;
        while (cont || !this->up_and_running) {
            int code = this->server_rdp_recv(this->front_stream);
            switch (code) {
            case -1:
                this->server_rdp_send_demand_active();
                #warning do we need to call this for every mcs packet? maybe every 5 or so
                /* Inform the callback that an mcs packet has been sent.  This is needed so
                the module can send any high priority mcs packets like audio. */
                this->cb.callback(0x5556, 0, 0, 0, 0);
                break;
            case 0:
                break;
            case RDP_PDU_CONFIRM_ACTIVE: /* 3 */
                this->server_rdp_process_confirm_active();
                break;
            case RDP_PDU_DATA: /* 7 */
                // this is rdp_process_data that will set up_and_running to 1
                // when fonts have been received
                // we will not exit this loop until we are in this state.
                this->server_rdp_process_data();
                break;
            default:
                LOG(LOG_WARNING, "unknown in session_data (%d)\n", code);
                break;
            }
            cont = this->front_stream.next_packet
                && (this->front_stream.next_packet < this->front_stream.end);
        }
    }

    int server_rdp_recv(Stream & stream) throw (Error)
    {
        if (this->front_stream.next_packet == 0 || this->front_stream.next_packet >= stream.end) {
            int chan = 0;
            int error = this->sec_layer.server_sec_recv(this->front_stream, &chan);
            if (error == -1) { /* special code for send demand active */
                this->front_stream.next_packet = 0;
                return -1;
            }
            if (error != 0) {
                throw Error(ERR_SERVER_RDP_RECV_ERR);
            }
            if ((chan != MCS_GLOBAL_CHANNEL) && (chan > 0)) {
                if (chan > MCS_GLOBAL_CHANNEL) {
                    /*****************************************************************************/
                    /* This is called from the secure layer to process an incoming non global
                       channel packet.
                       'chan' passed in here is the mcs channel id so it is
                       MCS_GLOBAL_CHANNEL plus something. */

                    /* this assumes that the channels are in order of chanid(mcs channel id)
                       but they should be, see server_sec_process_mcs_data_channels
                       the first channel should be MCS_GLOBAL_CHANNEL + 1, second
                       one should be MCS_GLOBAL_CHANNEL + 2, and so on */
                    int channel_id = (chan - MCS_GLOBAL_CHANNEL) - 1;

                    struct mcs_channel_item* channel = this->sec_layer.mcs_layer.channel_list[channel_id];

                    if (channel == 0) {
                        throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
                    }
                    int length = this->front_stream.in_uint32_le();
                    int flags = this->front_stream.in_uint32_le();

                    int size = (int)(this->front_stream.end - this->front_stream.p);
                    #warning check the long parameter is OK for p here. At start it is a pointer, converting to long is dangerous. See why this should be necessary in callback.
                    int rv = this->cb.callback(0x5555,
                                           ((flags & 0xffff) << 16) | (channel_id & 0xffff),
                                           size, (long)(this->front_stream.p), length);
                    if (rv != 0){
                        throw Error(ERR_CHANNEL_SESSION_CALLBACK_FAILED);
                    }
                }
                this->front_stream.next_packet = 0;
                return 0;
            }
            this->front_stream.next_packet = this->front_stream.p;
        }
        else {
            this->front_stream.p = this->front_stream.next_packet;
        }
        int len = this->front_stream.in_uint16_le();
        #warning looks like length can be 8 bits, check in protocol documentation, it may be the problem with properJavaRDP.
        if (len == 0x8000) {
            this->front_stream.next_packet += 8;
            return 0;
        }
        int pdu_code = this->front_stream.in_uint16_le();
        this->front_stream.skip_uint8(2); /* mcs user id */
        this->front_stream.next_packet += len;
        return pdu_code & 0xf;
    }

    /*****************************************************************************/
    void server_rdp_send(Stream & stream, int pdu_type) throw (Error)
    {
        stream.p = stream.rdp_hdr;
        int len = stream.end - stream.p;
        stream.out_uint16_le(len);
        stream.out_uint16_le(0x10 | pdu_type);
        stream.out_uint16_le(this->mcs_channel);
        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);
    }

    /*****************************************************************************/
    void server_rdp_send_data_update_sync() throw (Error)
    {
        Stream stream(8192);
        this->sec_layer.server_sec_init(stream);
        stream.rdp_hdr = stream.p;
        stream.p += 18;
        stream.out_uint16_le(RDP_UPDATE_SYNCHRONIZE);
        stream.out_clear_bytes(2);
        stream.mark_end();

        stream.p = stream.rdp_hdr;
        int len = stream.end - stream.p;
        stream.out_uint16_le(len);
        stream.out_uint16_le(0x10 | RDP_PDU_DATA);
        stream.out_uint16_le(this->mcs_channel);
        stream.out_uint32_le(this->share_id);
        stream.out_uint8(0);
        stream.out_uint8(1);
        stream.out_uint16_le(len - 14);
        stream.out_uint8(RDP_DATA_PDU_UPDATE);
        stream.out_uint8(0);
        stream.out_uint16_le(0);

        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);
    }

    void server_rdp_incoming(Rsakeys * rsa_keys) throw (Error)
    {
        this->sec_layer.server_sec_incoming(rsa_keys);
        this->mcs_channel = this->sec_layer.mcs_layer.userid + MCS_USERCHANNEL_BASE;
    }

    /*****************************************************************************/
    void server_rdp_send_demand_active() throw (Error)
    {
        int caps_count;
        int caps_size;
        uint8_t* caps_count_ptr;
        uint8_t* caps_size_ptr;
        uint8_t* caps_ptr;

        Stream stream(8192);

        this->sec_layer.server_sec_init(stream);
        stream.rdp_hdr = stream.p;
        stream.p += 6;

        caps_count = 0;
        stream.out_uint32_le(this->share_id);
        stream.out_uint16_le(4); /* 4 chars for RDP\0 */
        /* 2 bytes size after num caps, set later */
        caps_size_ptr = stream.p;
        stream.out_clear_bytes(2);
        stream.out_copy_bytes("RDP", 4);
        /* 4 byte num caps, set later */
        caps_count_ptr = stream.p;
        stream.out_clear_bytes(4);
        caps_ptr = stream.p;

        /* Output share capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_SHARE);
        stream.out_uint16_le(RDP_CAPLEN_SHARE);
        stream.out_uint16_le(this->mcs_channel);
        stream.out_uint16_be(0xb5e2); /* 0x73e1 */

        /* Output general capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_GENERAL); /* 1 */
        stream.out_uint16_le(RDP_CAPLEN_GENERAL); /* 24(0x18) */
        stream.out_uint16_le(1); /* OS major type */
        stream.out_uint16_le(3); /* OS minor type */
        stream.out_uint16_le(0x200); /* Protocol version */
        stream.out_uint16_le(0); /* pad */
        stream.out_uint16_le(0); /* Compression types */
        stream.out_uint16_le(0); /* pad use 0x40d for rdp packets, 0 for not */
        stream.out_uint16_le(0); /* Update capability */
        stream.out_uint16_le(0); /* Remote unshare capability */
        stream.out_uint16_le(0); /* Compression level */
        stream.out_uint16_le(0); /* Pad */

        /* Output bitmap capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_BITMAP); /* 2 */
        stream.out_uint16_le(RDP_CAPLEN_BITMAP); /* 28(0x1c) */
        stream.out_uint16_le(this->client_info.bpp); /* Preferred BPP */
        stream.out_uint16_le(1); /* Receive 1 BPP */
        stream.out_uint16_le(1); /* Receive 4 BPP */
        stream.out_uint16_le(1); /* Receive 8 BPP */
        stream.out_uint16_le(this->client_info.width); /* width */
        stream.out_uint16_le(this->client_info.height); /* height */
        stream.out_uint16_le(0); /* Pad */
        stream.out_uint16_le(1); /* Allow resize */
        stream.out_uint16_le(1); /* bitmap compression */
        stream.out_uint16_le(0); /* unknown */
        stream.out_uint16_le(0); /* unknown */
        stream.out_uint16_le(0); /* pad */

        /* Output font capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_FONT); /* 14 */
        stream.out_uint16_le(RDP_CAPLEN_FONT); /* 4 */

        /* Output order capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_ORDER); /* 3 */
        stream.out_uint16_le(RDP_CAPLEN_ORDER); /* 88(0x58) */
        stream.out_clear_bytes(16);
        stream.out_uint32_be(0x40420f00);
        stream.out_uint16_le(1); /* Cache X granularity */
        stream.out_uint16_le(20); /* Cache Y granularity */
        stream.out_uint16_le(0); /* Pad */
        stream.out_uint16_le(1); /* Max order level */
        stream.out_uint16_le(0x2f); /* Number of fonts */
        stream.out_uint16_le(0x22); /* Capability flags */
        /* caps */
        stream.out_uint8(1); /* dest blt */
        stream.out_uint8(1); /* pat blt */
        stream.out_uint8(1); /* screen blt */
        stream.out_uint8(1); /* mem blt */
        stream.out_uint8(0); /* tri blt */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* nine grid */
        stream.out_uint8(1); /* line to */
        stream.out_uint8(0); /* multi nine grid */
        stream.out_uint8(1); /* rect */
        stream.out_uint8(0); /* desk save */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* multi dest blt */
        stream.out_uint8(0); /* multi pat blt */
        stream.out_uint8(0); /* multi screen blt */
        stream.out_uint8(0); /* multi rect */
        stream.out_uint8(0); /* fast index */
        stream.out_uint8(0); /* polygon */
        stream.out_uint8(0); /* polygon */
        stream.out_uint8(0); /* polyline */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* fast glyph */
        stream.out_uint8(0); /* ellipse */
        stream.out_uint8(0); /* ellipse */
        stream.out_uint8(0); /* ? */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint16_le(0x6a1);
        stream.out_clear_bytes(2); /* ? */
        stream.out_uint32_le(0x0f4240); /* desk save */
        stream.out_uint32_le(0x0f4240); /* desk save */
        stream.out_uint32_le(1); /* ? */
        stream.out_uint32_le(0); /* ? */

        /* Output color cache capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_COLCACHE);
        stream.out_uint16_le(RDP_CAPLEN_COLCACHE);
        stream.out_uint16_le(6); /* cache size */
        stream.out_uint16_le(0); /* pad */

        /* Output pointer capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_POINTER);
        stream.out_uint16_le(RDP_CAPLEN_POINTER);
        stream.out_uint16_le(1); /* Colour pointer */
        stream.out_uint16_le(0x19); /* Cache size */
        stream.out_uint16_le(0x19); /* Cache size */

        /* Output input capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_INPUT); /* 13(0xd) */
        stream.out_uint16_le(RDP_CAPLEN_INPUT); /* 88(0x58) */
        stream.out_uint8(1);
        stream.out_clear_bytes(83);

        stream.out_clear_bytes(4); /* pad */

        stream.mark_end();

        caps_size = (int)(stream.end - caps_ptr);
        caps_size_ptr[0] = caps_size;
        caps_size_ptr[1] = caps_size >> 8;

        caps_count_ptr[0] = caps_count;
        caps_count_ptr[1] = caps_count >> 8;
        caps_count_ptr[2] = caps_count >> 16;
        caps_count_ptr[3] = caps_count >> 24;

        this->server_rdp_send(stream, RDP_PDU_DEMAND_ACTIVE);
    }

    /*****************************************************************************/
    void capset_general(Stream & stream, int len)
    {
        LOG(LOG_INFO, "capset_general");
        stream.skip_uint8(10);
        /* use_compact_packets is pretty much 'use rdp5' */
        this->client_info.use_compact_packets = stream.in_uint16_le();
        if (this->client_info.use_compact_packets){
            LOG(LOG_INFO, "Use compact packets");
        }
        /* op2 is a boolean to use compact bitmap headers in bitmap cache */
        /* set it to same as 'use rdp5' boolean */
        this->client_info.op2 = this->client_info.use_compact_packets;
        if (this->client_info.op2){
            LOG(LOG_INFO, "Use compact headers for cache");
        }
    }

    /*****************************************************************************/
    void capset_order(Stream & stream, int len)
    {

        stream.skip_uint8(20); /* Terminal desc, pad */
        stream.skip_uint8(2); /* Cache X granularity */
        stream.skip_uint8(2); /* Cache Y granularity */
        stream.skip_uint8(2); /* Pad */
        stream.skip_uint8(2); /* Max order level */
        stream.skip_uint8(2); /* Number of fonts */
        stream.skip_uint8(2); /* Capability flags */
        char order_caps[32];
        memcpy(order_caps, stream.in_uint8p(32), 32); /* Orders supported */
        LOG(LOG_INFO, "dest blt-0 %d\n", order_caps[0]);
        LOG(LOG_INFO, "pat blt-1 %d\n", order_caps[1]);
        LOG(LOG_INFO, "screen blt-2 %d\n", order_caps[2]);
        LOG(LOG_INFO, "memblt-3-13 %d %d\n", order_caps[3], order_caps[13]);
        LOG(LOG_INFO, "triblt-4-14 %d %d\n", order_caps[4], order_caps[14]);
        LOG(LOG_INFO, "line-8 %d\n", order_caps[8]);
        LOG(LOG_INFO, "line-9 %d\n", order_caps[9]);
        LOG(LOG_INFO, "rect-10 %d\n", order_caps[10]);
        LOG(LOG_INFO, "desksave-11 %d\n", order_caps[11]);
        LOG(LOG_INFO, "polygon-20 %d\n", order_caps[20]);
        LOG(LOG_INFO, "polygon2-21 %d\n", order_caps[21]);
        LOG(LOG_INFO, "polyline-22 %d\n", order_caps[22]);
        LOG(LOG_INFO, "ellipse-25 %d\n", order_caps[25]);
        LOG(LOG_INFO, "ellipse2-26 %d\n", order_caps[26]);
        LOG(LOG_INFO, "text2-27 %d\n", order_caps[27]);
        LOG(LOG_INFO, "order_caps dump\n");
        stream.skip_uint8(2); /* Text capability flags */
        stream.skip_uint8(6); /* Pad */
        /* desktop cache size, usually 0x38400 */
        this->client_info.desktop_cache = stream.in_uint32_le();;
        LOG(LOG_INFO, "desktop cache size %d\n", this->client_info.desktop_cache);
        stream.skip_uint8(4); /* Unknown */
        stream.skip_uint8(4); /* Unknown */
    }

    /* store the bitmap cache size in client_info */
    void capset_bmpcache(Stream & stream, int len)
    {
        LOG(LOG_INFO, "capset_bmpcache");
        stream.skip_uint8(24);
        this->client_info.cache1_entries = stream.in_uint16_le();
        this->client_info.cache1_size = stream.in_uint16_le();
        this->client_info.cache2_entries = stream.in_uint16_le();
        this->client_info.cache2_size = stream.in_uint16_le();
        this->client_info.cache3_entries = stream.in_uint16_le();
        this->client_info.cache3_size = stream.in_uint16_le();
        LOG(LOG_INFO, "cache1_entries=%d cache1_size=%d "
                      "cache2_entries=%d cache2_size=%d "
                      "cache3_entries=%d cache3_size=%d\n",
            this->client_info.cache1_entries, this->client_info.cache1_size,
            this->client_info.cache2_entries, this->client_info.cache2_size,
            this->client_info.cache3_entries, this->client_info.cache3_size);
    }

    /* store the bitmap cache size in client_info */
    void capset_bmpcache2(Stream & stream, int len)
    {
        LOG(LOG_INFO, "capset_bmpcache2");
        this->client_info.bitmap_cache_version = 2;
        int Bpp = nbbytes(this->client_info.bpp);
        this->client_info.bitmap_cache_persist_enable = stream.in_uint16_le();
        stream.skip_uint8(2); /* number of caches in set, 3 */
        this->client_info.cache1_entries = stream.in_uint32_le();
        this->client_info.cache1_size = 256 * Bpp;
        this->client_info.cache2_entries = stream.in_uint32_le();
        this->client_info.cache2_size = 1024 * Bpp;
        this->client_info.cache3_entries = (stream.in_uint32_le() & 0x7fffffff);
        this->client_info.cache3_size = 4096 * Bpp;
    }

    /* store the number of client cursor cache in client_info */
    void capset_pointercache(Stream & stream, int len)
    {
        stream.skip_uint8(2); /* color pointer */
        int i = stream.in_uint16_le();
        i = std::min(i, 32);
        this->client_info.pointer_cache_entries = i;
    }

    /* store the number of client brush cache in client_info */
    void capset_brushcache(Stream & stream, int len)
    {
        int code = stream.in_uint32_le();
        this->client_info.brush_cache_code = code;
    }

    void server_rdp_process_confirm_active()
    {
        this->front_stream.skip_uint8(4); /* rdp_shareid */
        this->front_stream.skip_uint8(2); /* userid */
        int source_len = this->front_stream.in_uint16_le(); /* sizeof RDP_SOURCE */
        // int cap_len = this->front_stream.in_uint16_le();
        this->front_stream.skip_uint8(2); // skip cap_len
        this->front_stream.skip_uint8(source_len);
        int num_caps = this->front_stream.in_uint16_le();
        this->front_stream.skip_uint8(2); /* pad */

        for (int index = 0; index < num_caps; index++) {
            uint8_t *p = this->front_stream.p;
            int type = this->front_stream.in_uint16_le();
            int len = this->front_stream.in_uint16_le();

            switch (type) {
            case RDP_CAPSET_GENERAL: /* 1 */
                this->capset_general(this->front_stream, len);
                break;
            case RDP_CAPSET_BITMAP: /* 2 */
                break;
            case RDP_CAPSET_ORDER: /* 3 */
                this->capset_order(this->front_stream, len);
                break;
            case RDP_CAPSET_BMPCACHE: /* 4 */
                this->capset_bmpcache(this->front_stream, len);
                break;
            case RDP_CAPSET_CONTROL: /* 5 */
                break;
            case RDP_CAPSET_ACTIVATE: /* 7 */
                break;
            case RDP_CAPSET_POINTER: /* 8 */
                this->capset_pointercache(this->front_stream, len);
                break;
            case RDP_CAPSET_SHARE: /* 9 */
                break;
            case RDP_CAPSET_COLCACHE: /* 10 */
                break;
            case 12: /* 12 */
                break;
            case 13: /* 13 */
                break;
            case 14: /* 14 */
                break;
            case RDP_CAPSET_BRUSHCACHE: /* 15 */
                this->capset_brushcache(this->front_stream, len);
                break;
            case 16: /* 16 */
                break;
            case 17: /* 17 */
                break;
            case RDP_CAPSET_BMPCACHE2: /* 19 */
                this->capset_bmpcache2(this->front_stream, len);
                break;
            case 20: /* 20 */
                break;
            case 21: /* 21 */
                break;
            case 22: /* 22 */
                break;
            case 26: /* 26 */
                break;
            default:
                break;
            }
            this->front_stream.p = p + len;
        }
    }

    /*****************************************************************************/
    void server_rdp_send_control(int action) throw (Error)
    {
        #warning we should create some RDPData object created on init and sent before destruction
        Stream stream(8192);
        this->sec_layer.server_sec_init(stream);
        stream.rdp_hdr = stream.p;
        stream.p += 18;
        stream.out_uint16_le(action);
        stream.out_uint16_le(0); /* userid */
        stream.out_uint32_le(1002); /* control id */
        stream.mark_end();

        stream.p = stream.rdp_hdr;
        int len = stream.end - stream.p;
        stream.out_uint16_le(len);
        stream.out_uint16_le(0x10 | RDP_PDU_DATA);
        stream.out_uint16_le(this->mcs_channel);
        stream.out_uint32_le(this->share_id);
        stream.out_uint8(0);
        stream.out_uint8(1);
        stream.out_uint16_le(len - 14);
        stream.out_uint8(RDP_DATA_PDU_CONTROL);
        stream.out_uint8(0);
        stream.out_uint16_le(0);

        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);


    }



    /*****************************************************************************/
    void server_rdp_send_unknown1() throw (Error)
    {
    static uint8_t g_unknown1[172] = { 0xff, 0x02, 0xb6, 0x00, 0x28, 0x00, 0x00, 0x00,
                                0x27, 0x00, 0x27, 0x00, 0x03, 0x00, 0x04, 0x00,
                                0x00, 0x00, 0x26, 0x00, 0x01, 0x00, 0x1e, 0x00,
                                0x02, 0x00, 0x1f, 0x00, 0x03, 0x00, 0x1d, 0x00,
                                0x04, 0x00, 0x27, 0x00, 0x05, 0x00, 0x0b, 0x00,
                                0x06, 0x00, 0x28, 0x00, 0x08, 0x00, 0x21, 0x00,
                                0x09, 0x00, 0x20, 0x00, 0x0a, 0x00, 0x22, 0x00,
                                0x0b, 0x00, 0x25, 0x00, 0x0c, 0x00, 0x24, 0x00,
                                0x0d, 0x00, 0x23, 0x00, 0x0e, 0x00, 0x19, 0x00,
                                0x0f, 0x00, 0x16, 0x00, 0x10, 0x00, 0x15, 0x00,
                                0x11, 0x00, 0x1c, 0x00, 0x12, 0x00, 0x1b, 0x00,
                                0x13, 0x00, 0x1a, 0x00, 0x14, 0x00, 0x17, 0x00,
                                0x15, 0x00, 0x18, 0x00, 0x16, 0x00, 0x0e, 0x00,
                                0x18, 0x00, 0x0c, 0x00, 0x19, 0x00, 0x0d, 0x00,
                                0x1a, 0x00, 0x12, 0x00, 0x1b, 0x00, 0x14, 0x00,
                                0x1f, 0x00, 0x13, 0x00, 0x20, 0x00, 0x00, 0x00,
                                0x21, 0x00, 0x0a, 0x00, 0x22, 0x00, 0x06, 0x00,
                                0x23, 0x00, 0x07, 0x00, 0x24, 0x00, 0x08, 0x00,
                                0x25, 0x00, 0x09, 0x00, 0x26, 0x00, 0x04, 0x00,
                                0x27, 0x00, 0x03, 0x00, 0x28, 0x00, 0x02, 0x00,
                                0x29, 0x00, 0x01, 0x00, 0x2a, 0x00, 0x05, 0x00,
                                0x2b, 0x00, 0x2a, 0x00
                              };

        #warning we should create some RDPStream object created on init and sent before destruction
        Stream stream(8192);
        this->sec_layer.server_sec_init(stream);
        stream.rdp_hdr = stream.p;
        stream.p += 18;
        stream.out_copy_bytes((char*)g_unknown1, 172);
        stream.mark_end();

        stream.p = stream.rdp_hdr;
        int len = stream.end - stream.p;
        stream.out_uint16_le(len);
        stream.out_uint16_le(0x10 | RDP_PDU_DATA);
        stream.out_uint16_le(this->mcs_channel);
        stream.out_uint32_le(this->share_id);
        stream.out_uint8(0);
        stream.out_uint8(1);
        stream.out_uint16_le(len - 14);
        stream.out_uint8(0x28);
        stream.out_uint8(0);
        stream.out_uint16_le(0);

        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);

    }

    /* RDP_PDU_DATA */
    void server_rdp_process_data() throw (Error)
    {
        this->front_stream.skip_uint8(6);
        this->front_stream.in_uint16_le(); // len
        int data_type = this->front_stream.in_uint8();
        this->front_stream.in_uint8(); // ctype
        this->front_stream.in_uint16_le(); // clen
        switch (data_type) {
        case RDP_DATA_PDU_POINTER: /* 27(0x1b) */
            break;
        case RDP_DATA_PDU_INPUT: /* 28(0x1c) */
            {
                int num_events = this->front_stream.in_uint16_le();
                this->front_stream.skip_uint8(2); /* pad */
                for (int index = 0; index < num_events; index++) {
                    int time = this->front_stream.in_uint32_le();
                    uint16_t msg_type = this->front_stream.in_uint16_le();
                    uint16_t device_flags = this->front_stream.in_uint16_le();
                    int16_t param1 = this->front_stream.in_sint16_le();
                    int16_t param2 = this->front_stream.in_sint16_le();
                    /* msg_type can be
                       RDP_INPUT_SYNCHRONIZE = 0
                       RDP_INPUT_SCANCODE = 4
                       RDP_INPUT_MOUSE = 0x8001 */
                    if (msg_type == 4){
//                        LOG(LOG_INFO, "receive input: time=%u device_flags = %u param1=%u param2=%u\n", time, device_flags, param1, param2);
                    }
                    this->cb.callback(msg_type, param1, param2, device_flags, time);
                }
            }
            break;
        case RDP_DATA_PDU_CONTROL: /* 20(0x14) */
            {
                int action = this->front_stream.in_uint16_le();
                this->front_stream.skip_uint8(2); /* user id */
                this->front_stream.skip_uint8(4); /* control id */
                if (action == RDP_CTL_REQUEST_CONTROL) {
                    #warning we should create some RDPData object created on init and sent before destruction
                    Stream stream(8192);
                    this->sec_layer.server_sec_init(stream);
                    stream.rdp_hdr = stream.p;
                    stream.p += 18;
                    stream.out_uint16_le(1);
                    stream.out_uint16_le(1002);
                    stream.mark_end();

                    stream.p = stream.rdp_hdr;
                    int len = stream.end - stream.p;
                    stream.out_uint16_le(len);
                    stream.out_uint16_le(0x10 | RDP_PDU_DATA);
                    stream.out_uint16_le(this->mcs_channel);
                    stream.out_uint32_le(this->share_id);
                    stream.out_uint8(0);
                    stream.out_uint8(1);
                    stream.out_uint16_le(len - 14);
                    stream.out_uint8(RDP_DATA_PDU_SYNCHRONISE);
                    stream.out_uint8(0);
                    stream.out_uint16_le(0);

                    this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);

                    #warning do we need to call this for every mcs packet? maybe every 5 or so
                    /* Inform the callback that an mcs packet has been sent.  This is needed so
                    the module can send any high priority mcs packets like audio. */
                    this->cb.callback(0x5556, 0, 0, 0, 0);
                    this->server_rdp_send_control(RDP_CTL_COOPERATE);
                    #warning do we need to call this for every mcs packet? maybe every 5 or so
                    /* Inform the callback that an mcs packet has been sent.  This is needed so
                   the module can send any high priority mcs packets like audio. */
                    this->cb.callback(0x5556, 0, 0, 0, 0);
                    this->server_rdp_send_control(RDP_CTL_GRANT_CONTROL);
                    #warning do we need to call this for every mcs packet? maybe every 5 or so
                    /* Inform the callback that an mcs packet has been sent.  This is needed so
                   the module can send any high priority mcs packets like audio. */
                    this->cb.callback(0x5556, 0, 0, 0, 0);

                }
                else {
                    #warning we sometimes get action 4. Add support for it
                    if (action != 4){
                        LOG(LOG_WARNING, "process DATA_PDU_CONTROL unknown action (%d)\n", action);
                    }
                }
            }
            break;
        case RDP_DATA_PDU_SYNCHRONISE: /* 31(0x1f) */
            break;
        case 33: /* 33(0x21) ?? Invalidate an area I think */
            {
                /* int op = */ this->front_stream.in_uint32_le();
                int left = this->front_stream.in_uint16_le();
                int top = this->front_stream.in_uint16_le();
                int right = this->front_stream.in_uint16_le();
                int bottom = this->front_stream.in_uint16_le();
                int cx = (right - left) + 1;
                int cy = (bottom - top) + 1;
                this->cb.callback(0x4444, left, top, cx, cy);
            }
            break;
        case 35: /* 35(0x23) */
            /* 35 ?? this comes when minimuzing a full screen mstsc.exe 2600 */
            /* I think this is saying the client no longer wants screen */
            /* updates and it will issue a 33 above to catch up */
            /* so minimized apps don't take bandwidth */
            break;
        case 36: /* 36(0x24) ?? disconnect query? */
            {
                /* when this message comes, send a 37 back so the client */
                /* is sure the connection is alive and it can ask if user */
                /* really wants to disconnect */
                Stream stream(8192);
                this->sec_layer.server_sec_init(stream);
                stream.rdp_hdr = stream.p;
                stream.p += 18;
                stream.mark_end();

                stream.p = stream.rdp_hdr;
                int len = stream.end - stream.p;
                stream.out_uint16_le(len);
                stream.out_uint16_le(0x10 | RDP_PDU_DATA);
                stream.out_uint16_le(this->mcs_channel);
                stream.out_uint32_le(this->share_id);
                stream.out_uint8(0);
                stream.out_uint8(1);
                stream.out_uint16_le(len - 14);
                stream.out_uint8(37);
                stream.out_uint8(0);
                stream.out_uint16_le(0);

                this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);

                #warning do we need to call this for every mcs packet? maybe every 5 or so
                /* Inform the callback that an mcs packet has been sent.  This is needed so
               the module can send any high priority mcs packets like audio. */
                this->cb.callback(0x5556, 0, 0, 0, 0);
            }
            break;
        case RDP_DATA_PDU_FONT2: /* 39(0x27) */
            this->front_stream.skip_uint8(2); /* num of fonts */
            this->front_stream.skip_uint8(2); /* unknown */
            {
                int seq = this->front_stream.in_uint16_le();
                /* 419 client sends Seq 1, then 2 */
                /* 2600 clients sends only Seq 3 */
                /* after second font message, we are up and running */
                if (seq == 2 || seq == 3)
                {
                    this->server_rdp_send_unknown1();
                    #warning do we need to call this for every mcs packet? maybe every 5 or so
                    /* Inform the callback that an mcs packet has been sent.  This is needed so
                   the module can send any high priority mcs packets like audio. */
                    this->cb.callback(0x5556, 0, 0, 0, 0);
                    this->up_and_running = 1;
                    this->server_rdp_send_data_update_sync();
                    #warning do we need to call this for every mcs packet? maybe every 5 or so
                    /* Inform the callback that an mcs packet has been sent.  This is needed so
                    the module can send any high priority mcs packets like audio. */
                    this->cb.callback(0x5556, 0, 0, 0, 0);
                }
            }
            break;
        default:
            LOG(LOG_WARNING, "unknown in server_rdp_process_data %d\n", data_type);
            break;
        }
    }

    void server_rdp_disconnect() throw (Error)
    {
        this->sec_layer.server_sec_disconnect();
    }

    void server_rdp_send_deactive() throw (Error)
    {
        #warning we should create some RDPStream object created on init and sent before destruction
        Stream stream(8192);
        this->sec_layer.server_sec_init(stream);
        stream.rdp_hdr = stream.p;
        stream.p += 6;
        stream.mark_end();
        this->server_rdp_send(stream, RDP_PDU_DEACTIVATE);
    }
};

#endif
