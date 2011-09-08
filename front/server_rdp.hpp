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

#if !defined(FRONT_SERVER_RDP_HPP__)
#define FRONT_SERVER_RDP_HPP__

#include "RDP/sec.hpp"
#include "client_info.hpp"
#include "config.hpp"
#include "error.hpp"
#include "callback.hpp"
#include "colors.hpp"
#include "altoco.hpp"
#include "RDP/x224.hpp"

/* rdp */
struct server_rdp {
    int up_and_running;

    int share_id;
    struct ClientInfo client_info;
    struct Sec sec_layer;
    uint32_t packet_number;
    Transport * trans;
    int userid;

    server_rdp(Transport * trans, Inifile * ini)
        :
        up_and_running(0),
        share_id(65538),
        client_info(ini),
        sec_layer(this->client_info.crypt_level),
        packet_number(1),
        trans(trans),
        userid(0)
    {
    }

    ~server_rdp()
    {
    }


    enum {
        CHANNEL_CHUNK_LENGTH = 8192,
        CHANNEL_FLAG_FIRST = 0x01,
        CHANNEL_FLAG_LAST = 0x02,
        CHANNEL_FLAG_SHOW_PROTOCOL = 0x10,
    };


    #warning we should provide directly target channel informations, no need to seek it in channel_list here
    void server_send_to_channel(ChannelList & channel_list, int channel_id, uint8_t *data, int data_len, int total_data_len, int flags) throw (Error)
    {
        Stream stream(data_len + 1024); /* this should be big enough */
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, channel_id);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->sec_layer.encrypt);

        stream.channel_hdr = stream.p;
        stream.p += 8;

        stream.out_copy_bytes(data, data_len);
        stream.mark_end();

        size_t index = channel_id - MCS_GLOBAL_CHANNEL - 1;
        size_t count = channel_list.size();
        if (index >= count) {
            throw Error(ERR_MCS_CHANNEL_NOT_FOUND);
        }
        const McsChannelItem & channel = channel_list[index];

        stream.p = stream.channel_hdr;
        stream.out_uint32_le(total_data_len);
        if (channel.flags & CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= CHANNEL_FLAG_SHOW_PROTOCOL;
        }
        stream.out_uint32_le(flags);
        assert(channel.chanid == channel_id);

        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);
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

    void send_global_palette(const BGRPalette & palette) throw (Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->sec_layer.encrypt);

        ShareControlAndDataOut rdp_out(stream, PDUTYPE_DATAPDU, PDUTYPE2_UPDATE, this->userid + MCS_USERCHANNEL_BASE, this->share_id);

        stream.out_uint16_le(RDP_UPDATE_PALETTE);
        stream.out_uint16_le(0);
        stream.out_uint32_le(256); /* # of colors */

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
        rdp_out.end();

        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);

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
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->sec_layer.encrypt);
        ShareControlAndDataOut rdp_out(stream, PDUTYPE_DATAPDU, PDUTYPE2_POINTER, this->userid + MCS_USERCHANNEL_BASE, this->share_id);

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
        rdp_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);
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
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->sec_layer.encrypt);
        ShareControlAndDataOut rdp_out(stream, PDUTYPE_DATAPDU, PDUTYPE2_POINTER, this->userid + MCS_USERCHANNEL_BASE, this->share_id);

        stream.out_uint16_le(RDP_POINTER_CACHED);
        stream.out_uint16_le(0); /* pad */
        stream.out_uint16_le(cache_idx);

        stream.mark_end();
        rdp_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);

    }




    // Secure Settings Exchange
    // ------------------------

    // Secure Settings Exchange: Secure client data (such as the username,
    // password and auto-reconnect cookie) is sent to the server using the Client
    // Info PDU.

    // Client                                                     Server
    //    |------ Client Info PDU      ---------------------------> |

    // Licensing
    // ---------

    // Licensing: The goal of the licensing exchange is to transfer a
    // license from the server to the client.

    // The client should store this license and on subsequent
    // connections send the license to the server for validation.
    // However, in some situations the client may not be issued a
    // license to store. In effect, the packets exchanged during this
    // phase of the protocol depend on the licensing mechanisms
    // employed by the server. Within the context of this document
    // we will assume that the client will not be issued a license to
    // store. For details regarding more advanced licensing scenarios
    // that take place during the Licensing Phase, see [MS-RDPELE].

    // Client                                                     Server
    //    | <------ Licence Error PDU Valid Client ---------------- |

    // Capabilities Exchange
    // ---------------------

    // Capabilities Negotiation: The server sends the set of capabilities it
    // supports to the client in a Demand Active PDU. The client responds with its
    // capabilities by sending a Confirm Active PDU.

    // Client                                                     Server
    //    | <------- Demand Active PDU ---------------------------- |
    //    |--------- Confirm Active PDU --------------------------> |

    // Connection Finalization
    // -----------------------

    // Connection Finalization: The client and server send PDUs to finalize the
    // connection details. The client-to-server and server-to-client PDUs exchanged
    // during this phase may be sent concurrently as long as the sequencing in
    // either direction is maintained (there are no cross-dependencies between any
    // of the client-to-server and server-to-client PDUs). After the client receives
    // the Font Map PDU it can start sending mouse and keyboard input to the server,
    // and upon receipt of the Font List PDU the server can start sending graphics
    // output to the client.

    // Client                                                     Server
    //    |----------Synchronize PDU------------------------------> |
    //    |----------Control PDU Cooperate------------------------> |
    //    |----------Control PDU Request Control------------------> |
    //    |----------Persistent Key List PDU(s)-------------------> |
    //    |----------Font List PDU--------------------------------> |

    //    | <--------Synchronize PDU------------------------------- |
    //    | <--------Control PDU Cooperate------------------------- |
    //    | <--------Control PDU Granted Control------------------- |
    //    | <--------Font Map PDU---------------------------------- |

    // All PDU's in the client-to-server direction must be sent in the specified
    // order and all PDU's in the server to client direction must be sent in the
    // specified order. However, there is no requirement that client to server PDU's
    // be sent before server-to-client PDU's. PDU's may be sent concurrently as long
    // as the sequencing in either direction is maintained.


    // Besides input and graphics data, other data that can be exchanged between
    // client and server after the connection has been finalized includes
    // connection management information and virtual channel messages (exchanged
    // between client-side plug-ins and server-side applications).


    void server_rdp_incoming(ChannelList & channel_list) throw (Error)
    {
        LOG(LOG_INFO, "Connection Initiation");
        // Connection Initiation
        // ---------------------

        // The client initiates the connection by sending the server an X.224 Connection
        //  Request PDU (class 0). The server responds with an X.224 Connection Confirm
        // PDU (class 0). From this point, all subsequent data sent between client and
        // server is wrapped in an X.224 Data Protocol Data Unit (PDU).

        // Client                                                     Server
        //    |------------X224 Connection Request PDU----------------> |
        //    | <----------X224 Connection Confirm PDU----------------- |

        recv_x224_connection_request_pdu(this->trans);
        send_x224_connection_confirm_pdu(this->trans);

        // Basic Settings Exchange
        // -----------------------

        // Basic Settings Exchange: Basic settings are exchanged between the client and
        // server by using the MCS Connect Initial and MCS Connect Response PDUs. The
        // Connect Initial PDU contains a GCC Conference Create Request, while the
        // Connect Response PDU contains a GCC Conference Create Response.

        // These two Generic Conference Control (GCC) packets contain concatenated
        // blocks of settings data (such as core data, security data and network data)
        // which are read by client and server

        // Client                                                     Server
        //    |--------------MCS Connect Initial PDU with-------------> |
        //                   GCC Conference Create Request
        //    | <------------MCS Connect Response PDU with------------- |
        //                   GCC conference Create Response

        LOG(LOG_INFO, "Basic Settings Exchange");
        LOG(LOG_INFO, "front:basic_settings:channel_list : %u", channel_list.size());

        recv_mcs_connect_initial_pdu_with_gcc_conference_create_request(
                this->trans,
                &this->client_info,
                channel_list);

        this->sec_layer.send_mcs_connect_response_pdu_with_gcc_conference_create_response(this->trans, &this->client_info, channel_list);

        // Channel Connection
        // ------------------

        // Channel Connection: The client sends an MCS Erect Domain Request PDU,
        // followed by an MCS Attach User Request PDU to attach the primary user
        // identity to the MCS domain.

        // The server responds with an MCS Attach User Response PDU containing the user
        // channel ID.

        // The client then proceeds to join the :
        // - user channel,
        // - the input/output (I/O) channel
        // - and all of the static virtual channels

        // (the I/O and static virtual channel IDs are obtained from the data embedded
        //  in the GCC packets) by using multiple MCS Channel Join Request PDUs.

        // The server confirms each channel with an MCS Channel Join Confirm PDU.
        // (The client only sends a Channel Join Request after it has received the
        // Channel Join Confirm for the previously sent request.)

        // From this point, all subsequent data sent from the client to the server is
        // wrapped in an MCS Send Data Request PDU, while data sent from the server to
        //  the client is wrapped in an MCS Send Data Indication PDU. This is in
        // addition to the data being wrapped by an X.224 Data PDU.

        // Client                                                     Server
        //    |-------MCS Erect Domain Request PDU--------------------> |
        //    |-------MCS Attach User Request PDU---------------------> |

        //    | <-----MCS Attach User Confirm PDU---------------------- |

        //    |-------MCS Channel Join Request PDU--------------------> |
        //    | <-----MCS Channel Join Confirm PDU--------------------- |

        LOG(LOG_INFO, "Channel Connection");
        LOG(LOG_INFO, "AURQ");
        {
            #warning change userid to uint16_t
            uint16_t tmp_userid;
            recv_mcs_erect_domain_and_attach_user_request_pdu(this->trans, tmp_userid);
            this->userid = tmp_userid;
        }

        send_mcs_attach_user_confirm_pdu(this->trans, this->userid);

        {
            uint16_t tmp_userid;
            uint16_t tmp_chanid;
            recv_mcs_channel_join_request_pdu(this->trans, tmp_userid, tmp_chanid);
        }

        send_mcs_channel_join_confirm_pdu(this->trans, this->userid, this->userid + MCS_USERCHANNEL_BASE);

        {
            uint16_t tmp_userid;
            uint16_t tmp_chanid;
            recv_mcs_channel_join_request_pdu(this->trans, tmp_userid, tmp_chanid);
        }

        send_mcs_channel_join_confirm_pdu(this->trans, this->userid, MCS_GLOBAL_CHANNEL);

        LOG(LOG_INFO, "channel_list = %u", channel_list.size());
        for (size_t i = 0 ; i < channel_list.size() ; i++){
                uint16_t tmp_userid;
                uint16_t tmp_chanid;
                recv_mcs_channel_join_request_pdu(this->trans, tmp_userid, tmp_chanid);
                send_mcs_channel_join_confirm_pdu(this->trans, tmp_userid, tmp_chanid);
        }
        LOG(LOG_INFO, "RDP Security Commencement");

        // RDP Security Commencement
        // -------------------------

        // RDP Security Commencement: If standard RDP security methods are being
        // employed and encryption is in force (this is determined by examining the data
        // embedded in the GCC Conference Create Response packet) then the client sends
        // a Security Exchange PDU containing an encrypted 32-byte random number to the
        // server. This random number is encrypted with the public key of the server
        // (the server's public key, as well as a 32-byte server-generated random
        // number, are both obtained from the data embedded in the GCC Conference Create
        //  Response packet).

        // The client and server then utilize the two 32-byte random numbers to generate
        // session keys which are used to encrypt and validate the integrity of
        // subsequent RDP traffic.

        // From this point, all subsequent RDP traffic can be encrypted and a security
        // header is included with the data if encryption is in force (the Client Info
        // and licensing PDUs are an exception in that they always have a security
        // header). The Security Header follows the X.224 and MCS Headers and indicates
        // whether the attached data is encrypted.

        // Even if encryption is in force server-to-client traffic may not always be
        // encrypted, while client-to-server traffic will always be encrypted by
        // Microsoft RDP implementations (encryption of licensing PDUs is optional,
        // however).

        // Client                                                     Server
        //    |------Security Exchange PDU ---------------------------> |


    }


    void activate_and_process_data(Callback & cb, ChannelList & channel_list)
    {
        #warning this code needs (yet and again) much clarification

        Stream input_stream(65535);

        do {
            input_stream.init(65535);
            X224In tpdu(this->trans, input_stream);
            McsIn mcs_in(input_stream);

            // Disconnect Provider Ultimatum datagram
            if ((mcs_in.opcode >> 2) == MCS_DPUM) {
                throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
            }

            if ((mcs_in.opcode >> 2) != MCS_SDRQ) {
                throw Error(ERR_MCS_APPID_NOT_MCS_SDRQ);
            }

            SecIn sec(input_stream, this->sec_layer.decrypt);

            #warning this should move to incoming (connection initiation phase)
            if (sec.flags & SEC_CLIENT_RANDOM) { /* 0x01 */
                this->recv_client_random(input_stream);
                if (!this->up_and_running){
//                    input_stream.next_packet = input_stream.end;
                    continue;
                }
            }
            else if (sec.flags & SEC_LOGON_INFO) { /* 0x40 */
                this->sec_layer.server_sec_process_logon_info(input_stream, &this->client_info);
                if (this->client_info.is_mce) {
//                    LOG(LOG_INFO, "server_sec_send media_lic_response");
                    this->sec_layer.server_sec_send_media_lic_response(this->trans, this->userid);
                    this->server_rdp_send_demand_active();
                    if (!this->up_and_running){
//                        input_stream.next_packet = input_stream.end;
                        continue;
                    }
                }
                else {
//                    LOG(LOG_INFO, "server_sec_send lic_initial");
                    this->sec_layer.server_sec_send_lic_initial(this->trans, this->userid);
                    if (!this->up_and_running){
//                        input_stream.next_packet = input_stream.end;
                        continue;
                    }
                }
            }
            else if (sec.flags & SEC_LICENCE_NEG) { /* 0x80 */
//                LOG(LOG_INFO, "server_sec_send lic_response");
                this->sec_layer.server_sec_send_lic_response(this->trans, this->userid);
                this->server_rdp_send_demand_active();
                if (!this->up_and_running){
//                    input_stream.next_packet = input_stream.end;
                    continue;
                }
            }
            else if (mcs_in.chan_id > MCS_GLOBAL_CHANNEL) {
            #warning is it possible to get channel data when we are not up and running ?
                /*****************************************************************************/
                /* This is called from the secure layer to process an incoming non global
                   channel packet.
                   'chan' passed in here is the mcs channel id so it is
                   MCS_GLOBAL_CHANNEL plus something. */

                /* this assumes that the channels are in order of chanid(mcs channel id)
                   but they should be, see server_sec_process_mcs_data_channels
                   the first channel should be MCS_GLOBAL_CHANNEL + 1, second
                   one should be MCS_GLOBAL_CHANNEL + 2, and so on */
                size_t channel_id = (mcs_in.chan_id - MCS_GLOBAL_CHANNEL) - 1;

                LOG(LOG_INFO, "received data in channel %u(=%u) [%s]",
                    channel_list[channel_id].chanid, mcs_in.chan_id,
                    channel_list[channel_id].name);


                if (channel_id >= channel_list.size()) {
                    throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
                }

                int length = input_stream.in_uint32_le();
                int flags = input_stream.in_uint32_le();

                int size = (int)(input_stream.end - input_stream.p);

                LOG(LOG_INFO, "up_and_running=%u", this->up_and_running);
                LOG(LOG_INFO, "got channel data from client chan_id=%u [%s] length=%u size=%u, sending to server length=%u size=%u", mcs_in.chan_id, channel_list[channel_id].name, length, size);

                #warning check the long parameter is OK for p here. At start it is a pointer, converting to long is dangerous. See why this should be necessary in callback.
                cb.callback(WM_CHANNELDATA,
                          ((flags & 0xffff) << 16) | (channel_id & 0xffff),
                          size, (long)(input_stream.p), length);
                // We consume all the data of the packet
                input_stream.p = input_stream.end;
            }
            LOG(LOG_INFO, "PDUTYPE DATA");

            input_stream.next_packet = input_stream.p;

            if (input_stream.next_packet < input_stream.end){
                int length = input_stream.in_uint16_le();
                if (length == 0x8000) {
                    input_stream.next_packet += 8;
                }
                else {
                    int pdu_code = input_stream.in_uint16_le();
                    input_stream.skip_uint8(2); /* mcs user id */
                    input_stream.next_packet += length;
                    switch (pdu_code & 0xf) {

                    case 0:
//                        LOG(LOG_INFO, "PDUTYPE_ZERO");
                        break;
                    case PDUTYPE_DEMANDACTIVEPDU: /* 1 */
//                        LOG(LOG_INFO, "PDUTYPE_DEMANDACTIVEPDU");
                        break;
                    case PDUTYPE_CONFIRMACTIVEPDU:
//                        LOG(LOG_INFO, "PDUTYPE_CONFIRMACTIVEPDU");
                        this->process_confirm_active(input_stream);
                        break;
                    case PDUTYPE_DATAPDU: /* 7 */
//                        LOG(LOG_INFO, "PDUTYPE_DATAPDU");
                        // this is rdp_process_data that will set up_and_running to 1
                        // when fonts have been received
                        // we will not exit this loop until we are in this state.
                        #warning see what happen if we never receive up_and_running due to some error in client code ?
                        this->process_data(input_stream, cb);
//                        LOG(LOG_INFO, "PROCESS_DATA_DONE");
                        break;
                    case PDUTYPE_DEACTIVATEALLPDU:
                        LOG(LOG_WARNING, "unsupported PDU DEACTIVATEALLPDU in session_data (%d)\n", pdu_code & 0xf);
                        break;
                    case PDUTYPE_SERVER_REDIR_PKT:
                        LOG(LOG_WARNING, "unsupported PDU SERVER_REDIR_PKT in session_data (%d)\n", pdu_code & 0xf);
                        break;
                    default:
                        LOG(LOG_WARNING, "unknown PDU type in session_data (%d)\n", pdu_code & 0xf);
                        break;
                    }
                }
            }
            LOG(LOG_INFO, "READY TO LOOP IN activate and process data");
        } while ((input_stream.next_packet < input_stream.end) || !this->up_and_running);

        #warning the postcondition could be changed to signify we want to get hand back immediately, because we still have data to process.
        LOG(LOG_INFO, "out of activate and process data");
    }


    void recv_client_random(Stream & stream)
    {
        stream.in_uint32_le(); // len

        memcpy(this->sec_layer.client_crypt_random, stream.in_uint8p(64), 64);

        ssl_mod_exp(this->sec_layer.client_random, 64,
                this->sec_layer.client_crypt_random, 64,
                this->sec_layer.pub_mod, 64,
                this->sec_layer.pri_exp, 64);
        {
            ssllib ssl;

            uint8_t pre_master_secret[48];
            uint8_t master_secret[48];
            uint8_t key_block[48];

            /* Construct pre-master secret (session key) */
            memcpy(key_block, this->sec_layer.client_random, 24);
            memcpy(key_block + 24, this->sec_layer.server_random, 24);

            /* Generate master secret and then key material */
            sec_hash_48(master_secret, key_block, this->sec_layer.client_random, this->sec_layer.server_random, 65);
            sec_hash_48(pre_master_secret, master_secret, this->sec_layer.client_random, this->sec_layer.server_random, 88);

            /* First 16 bytes of key material is MAC secret */
            memcpy(this->sec_layer.encrypt.sign_key, pre_master_secret, 16);

            /* Generate export keys from next two blocks of 16 bytes */
            sec_hash_16(this->sec_layer.encrypt.key, &pre_master_secret[16], this->sec_layer.client_random, this->sec_layer.server_random);
            sec_hash_16(this->sec_layer.decrypt.key, &pre_master_secret[32], this->sec_layer.client_random, this->sec_layer.server_random);

            if (this->sec_layer.rc4_key_size == 1) {
                sec_make_40bit(this->sec_layer.encrypt.sign_key);
                sec_make_40bit(this->sec_layer.encrypt.key);
                sec_make_40bit(this->sec_layer.decrypt.key);
                this->sec_layer.decrypt.rc4_key_len = 8;
                this->sec_layer.encrypt.rc4_key_len = 8;
            } else {
                this->sec_layer.decrypt.rc4_key_len = 16;
                this->sec_layer.encrypt.rc4_key_len = 16;
            }
            memcpy(this->sec_layer.decrypt.update_key, this->sec_layer.decrypt.key, 16);
            memcpy(this->sec_layer.encrypt.update_key, this->sec_layer.encrypt.key, 16);
            ssl.rc4_set_key(this->sec_layer.decrypt.rc4_info, this->sec_layer.decrypt.key, this->sec_layer.decrypt.rc4_key_len);
            ssl.rc4_set_key(this->sec_layer.encrypt.rc4_info, this->sec_layer.encrypt.key, this->sec_layer.encrypt.rc4_key_len);
        }
    }

    /*****************************************************************************/
    void server_rdp_send_data_update_sync() throw (Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->sec_layer.encrypt);

        ShareControlAndDataOut rdp_out(stream, PDUTYPE_DATAPDU, PDUTYPE2_UPDATE, this->userid + MCS_USERCHANNEL_BASE, this->share_id);

        stream.out_uint16_le(RDP_UPDATE_SYNCHRONIZE);
        stream.out_clear_bytes(2);

        stream.mark_end();
        rdp_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);

    }



    /*****************************************************************************/
    void server_rdp_send_demand_active() throw (Error)
    {

//        LOG(LOG_INFO, "server_rdp_send_demand_active()");
        int caps_count;
        int caps_size;
        uint8_t* caps_count_ptr;
        uint8_t* caps_size_ptr;
        uint8_t* caps_ptr;

        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->sec_layer.encrypt);
        ShareControlOut rdp_out(stream, PDUTYPE_DEMANDACTIVEPDU, this->userid + MCS_USERCHANNEL_BASE);

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
        stream.out_uint16_le(this->userid + MCS_USERCHANNEL_BASE);
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

        rdp_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);


    }

    /*****************************************************************************/
    void capset_general(Stream & stream, int len)
    {
//        LOG(LOG_INFO, "capset_general");
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
    }


    void process_confirm_active(Stream & stream)
    {
        stream.skip_uint8(4); /* rdp_shareid */
        stream.skip_uint8(2); /* userid */
        int source_len = stream.in_uint16_le(); /* sizeof RDP_SOURCE */
        // int cap_len = stream.in_uint16_le();
        stream.skip_uint8(2); // skip cap_len
        stream.skip_uint8(source_len);
        int num_caps = stream.in_uint16_le();
        stream.skip_uint8(2); /* pad */

        for (int index = 0; index < num_caps; index++) {
            uint8_t *p = stream.p;
            int type = stream.in_uint16_le();
            int len = stream.in_uint16_le();

            switch (type) {
            case RDP_CAPSET_GENERAL: /* 1 */
                this->capset_general(stream, len);
                break;
            case RDP_CAPSET_BITMAP: /* 2 */
                break;
            case RDP_CAPSET_ORDER: /* 3 */
                this->capset_order(stream, len);
                break;
            case RDP_CAPSET_BMPCACHE: /* 4 */
                this->capset_bmpcache(stream, len);
                break;
            case RDP_CAPSET_CONTROL: /* 5 */
                break;
            case RDP_CAPSET_ACTIVATE: /* 7 */
                break;
            case RDP_CAPSET_POINTER: /* 8 */
                {
                    stream.skip_uint8(2); /* color pointer */
                    int i = stream.in_uint16_le();
                    this->client_info.pointer_cache_entries = std::min(i, 32);
                }
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
                this->client_info.brush_cache_code = stream.in_uint32_le();
                break;
            case 16: /* 16 */
                break;
            case 17: /* 17 */
                break;
            case RDP_CAPSET_BMPCACHE2: /* 19 */
                this->capset_bmpcache2(stream, len);
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
            stream.p = p + len;
        }
    }

// 2.2.1.19 Server Synchronize PDU
// ===============================

// The Server Synchronize PDU is an RDP Connection Sequence PDU sent from server
// to client during the Connection Finalization phase (see section 1.3.1.1). It
// is sent after receiving the Confirm Active PDU (section 2.2.1.13.2).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224] section 13.7.

// mcsSDin (variable): Variable-length PER-encoded MCS Domain PDU which
//   encapsulates an MCS Send Data Indication structure, as specified in [T125]
//   (the ASN.1 structure definitions are given in section 7, parts 7 and 10 of
//   [T125]). The userData field of the MCS Send Data Indication contains a
//   Security Header and the Synchronize PDU Data (section 2.2.1.14.1).

// securityHeader (variable): Optional security header. If the Encryption Level
//   (sections 5.3.2 and 2.2.1.4.3) selected by the server is greater than
//   ENCRYPTION_LEVEL_NONE (0) and the Encryption Method (sections 5.3.2 and
//   2.2.1.4.3) selected by the server is greater than ENCRYPTION_METHOD_NONE
//   (0) then this field will contain one of the following headers:

//   - Basic Security Header (section 2.2.8.1.1.2.1) if the Encryption Level
//     selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//     ENCRYPTION_LEVEL_LOW (1).

//  - Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption Level
//    selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//    ENCRYPTION_LEVEL_CLIENT_COMPATIBLE (2), or ENCRYPTION_LEVEL_HIGH (3).

//  - FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Level
//    selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//    ENCRYPTION_LEVEL_FIPS (4).

// If the Encryption Level (sections 5.3.2 and 2.2.1.4.3) selected by the server
// is ENCRYPTION_LEVEL_NONE (0) and the Encryption Method (sections 5.3.2 and
// 2.2.1.4.3) selected by the server is ENCRYPTION_METHOD_NONE (0), then this
// header is not included in the PDU.

// synchronizePduData (22 bytes): The contents of the Synchronize PDU as
// described in section 2.2.1.14.1.

// 2.2.1.14.1 Synchronize PDU Data (TS_SYNCHRONIZE_PDU)
// ====================================================
// The TS_SYNCHRONIZE_PDU structure is a standard T.128 Synchronize PDU (see
// [T128] section 8.6.1).

// shareDataHeader (18 bytes): Share Control Header (section 2.2.8.1.1.1.1)
//   containing information about the packet. The type subfield of the pduType
//   field of the Share Control Header MUST be set to PDUTYPE_DATAPDU (7). The
//   pduType2 field of the Share Data Header MUST be set to PDUTYPE2_SYNCHRONIZE
//   (31).

// messageType (2 bytes): A 16-bit, unsigned integer. The message type. This
//   field MUST be set to SYNCMSGTYPE_SYNC (1).

// targetUser (2 bytes): A 16-bit, unsigned integer. The MCS channel ID of the
//   target user.

    void server_rdp_send_synchronize()
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->sec_layer.encrypt);
        ShareControlAndDataOut rdp_out(stream, PDUTYPE_DATAPDU, PDUTYPE2_SYNCHRONIZE, this->userid + MCS_USERCHANNEL_BASE, this->share_id);

        stream.out_uint16_le(1); /* messageType */
        stream.out_uint16_le(1002); /* control id */

        rdp_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);

    }

// 2.2.1.15.1 Control PDU Data (TS_CONTROL_PDU)
// ============================================

// The TS_CONTROL_PDU structure is a standard T.128 Synchronize PDU (see [T128]
// section 8.12).

// shareDataHeader (18 bytes): Share Data Header (section 2.2.8.1.1.1.2)
//   containing information about the packet. The type subfield of the pduType
//   field of the Share Control Header (section 2.2.8.1.1.1.1) MUST be set to
//   PDUTYPE_DATAPDU (7). The pduType2 field of the Share Data Header MUST be set
//   to PDUTYPE2_CONTROL (20).

// action (2 bytes): A 16-bit, unsigned integer. The action code.
// 0x0001 CTRLACTION_REQUEST_CONTROL Request control
// 0x0002 CTRLACTION_GRANTED_CONTROL Granted control
// 0x0003 CTRLACTION_DETACH Detach
// 0x0004 CTRLACTION_COOPERATE Cooperate

// grantId (2 bytes): A 16-bit, unsigned integer. The grant identifier.

// controlId (4 bytes): A 32-bit, unsigned integer. The control identifier.

    void server_rdp_send_control(int action)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->sec_layer.encrypt);
        ShareControlAndDataOut rdp_out(stream, PDUTYPE_DATAPDU, PDUTYPE2_CONTROL, this->userid + MCS_USERCHANNEL_BASE, this->share_id);

        stream.out_uint16_le(action);
        stream.out_uint16_le(0); /* userid */
        stream.out_uint32_le(1002); /* control id */

        rdp_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);

    }



    /*****************************************************************************/
    void server_rdp_send_fontmap() throw (Error)
    {
    static uint8_t g_fontmap[172] = { 0xff, 0x02, 0xb6, 0x00, 0x28, 0x00, 0x00, 0x00,
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
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->sec_layer.encrypt);
        ShareControlAndDataOut rdp_out(stream, PDUTYPE_DATAPDU, PDUTYPE2_FONTMAP, this->userid + MCS_USERCHANNEL_BASE, this->share_id);
        stream.out_copy_bytes((char*)g_fontmap, 172);
        stream.mark_end();

        rdp_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);

    }

    /* PDUTYPE_DATAPDU */
    void process_data(Stream & stream, Callback & cb) throw (Error)
    {
        stream.skip_uint8(6);
        stream.in_uint16_le(); // len
        int data_type = stream.in_uint8();
        stream.in_uint8(); // ctype
        stream.in_uint16_le(); // clen
        switch (data_type) {
        case PDUTYPE2_POINTER: /* 27(0x1b) */
//            LOG(LOG_INFO, "PDUTYPE2_POINTER");
            break;
        case PDUTYPE2_INPUT: /* 28(0x1c) */
//            LOG(LOG_INFO, "PDUTYPE2_INPUT");
            {
                int num_events = stream.in_uint16_le();
                stream.skip_uint8(2); /* pad */
                for (int index = 0; index < num_events; index++) {
                    int time = stream.in_uint32_le();
                    uint16_t msg_type = stream.in_uint16_le();
                    uint16_t device_flags = stream.in_uint16_le();
                    int16_t param1 = stream.in_sint16_le();
                    int16_t param2 = stream.in_sint16_le();
                    /* msg_type can be
                       RDP_INPUT_SYNCHRONIZE = 0
                       RDP_INPUT_SCANCODE = 4
                       RDP_INPUT_MOUSE = 0x8001 */
                    if (msg_type == 4){
//                        LOG(LOG_INFO, "receive input: time=%u device_flags = %u param1=%u param2=%u\n", time, device_flags, param1, param2);
                    }
                    cb.callback(msg_type, param1, param2, device_flags, time);
                }
            }
            break;
        case PDUTYPE2_CONTROL: /* 20(0x14) */
//            LOG(LOG_INFO, "PDUTYPE2_CONTROL");
            {
                int action = stream.in_uint16_le();
                stream.skip_uint8(2); /* user id */
                stream.skip_uint8(4); /* control id */
                switch (action){
                    case RDP_CTL_REQUEST_CONTROL:
                        this->server_rdp_send_control(RDP_CTL_GRANT_CONTROL);
                    break;
                    case RDP_CTL_COOPERATE:
                        this->server_rdp_send_control(RDP_CTL_COOPERATE);
                    break;
                    default:
                        LOG(LOG_WARNING, "process DATA_PDU_CONTROL unknown action (%d)\n", action);
                }
            }
            break;
        case PDUTYPE2_SYNCHRONIZE:
//            LOG(LOG_INFO, "PDUTYPE2_SYNCHRONIZE");
            this->server_rdp_send_synchronize();
            break;
        case PDUTYPE2_REFRESH_RECT:
//            LOG(LOG_INFO, "PDUTYPE2_REFRESH_RECT");
            {
                /* int op = */ stream.in_uint32_le();
                int left = stream.in_uint16_le();
                int top = stream.in_uint16_le();
                int right = stream.in_uint16_le();
                int bottom = stream.in_uint16_le();
                int cx = (right - left) + 1;
                int cy = (bottom - top) + 1;
                cb.callback(0x4444, left, top, cx, cy);
            }
            break;
        case PDUTYPE2_SUPPRESS_OUTPUT:
//            LOG(LOG_INFO, "PDUTYPE2_SUPPRESS_OUTPUT");
            // PDUTYPE2_SUPPRESS_OUTPUT comes when minimizing a full screen
            // mstsc.exe 2600. I think this is saying the client no longer wants
            // screen updates and it will issue a PDUTYPE2_REFRESH_RECT above
            // to catch up so minimized apps don't take bandwidth
            break;
        case PDUTYPE2_SHUTDOWN_REQUEST:
//            LOG(LOG_INFO, "PDUTYPE2_SHUTDOWN_REQUEST");
            {
                // when this message comes, send a PDUTYPE2_SHUTDOWN_DENIED back
                // so the client is sure the connection is alive and it can ask
                // if user really wants to disconnect */
                Stream stream(8192);
                X224Out tpdu(X224Packet::DT_TPDU, stream);
                McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
                SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->sec_layer.encrypt);
                ShareControlAndDataOut rdp_out(stream, PDUTYPE_DATAPDU, PDUTYPE2_SHUTDOWN_DENIED, this->userid + MCS_USERCHANNEL_BASE, this->share_id);
                stream.mark_end();
                rdp_out.end();
                sec_out.end();
                sdin_out.end();
                tpdu.end();
                tpdu.send(this->trans);

            }
            break;
        case PDUTYPE2_FONTLIST: /* 39(0x27) */
//            LOG(LOG_INFO, "PDUTYPE2_FONTLIST");
            stream.skip_uint8(2); /* num of fonts */
            stream.skip_uint8(2); /* unknown */
            {
                int seq = stream.in_uint16_le();
                /* 419 client sends Seq 1, then 2 */
                /* 2600 clients sends only Seq 3 */
                /* after second font message, we are up and running */
                if (seq == 2 || seq == 3)
                {
                    this->server_rdp_send_fontmap();
                    this->up_and_running = 1;
                    this->server_rdp_send_data_update_sync();
                }
            }
            break;
        default:
            LOG(LOG_WARNING, "unsupported PDUTYPE in process_data %d\n", data_type);
            break;
        }
    }

    void server_rdp_disconnect() throw (Error)
    {
        this->sec_layer.server_sec_disconnect(this->trans);
    }

    void server_rdp_send_deactive() throw (Error)
    {
        #warning we should create some RDPStream object created on init and sent before destruction
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->sec_layer.encrypt);

        ShareControlOut(stream, PDUTYPE_DEACTIVATEALLPDU, this->userid + MCS_USERCHANNEL_BASE).end();
        stream.mark_end();

        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);
    }
};

#endif
