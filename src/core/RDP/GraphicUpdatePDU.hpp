/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include <cinttypes>

#include "utils/log.hpp"
#include "utils/sugar/underlying_cast.hpp"
#include "core/RDP/RDPSerializer.hpp"
#include "core/RDP/gcc.hpp"
#include "core/RDP/sec.hpp"
#include "core/RDP/mcs.hpp"
#include "core/RDP/x224.hpp"

static inline void send_data_indication_ex( Transport & trans
                                          , int encryptionLevel, CryptContext & encrypt
                                          , uint16_t initiator
                                          , StaticOutReservedStreamHelper<1024, 65536-1024> & stream)
{
    StaticOutStream<256> security_header;
    SEC::Sec_Send sec(security_header, stream.get_packet().data(), stream.get_packet().size(), 0, encrypt, encryptionLevel);
    stream.copy_to_head(security_header);

    StaticOutStream<256> mcs_header;
    MCS::SendDataIndication_Send mcs( static_cast<OutPerStream &>(static_cast<OutStream &>(mcs_header))
                                    , initiator
                                    , GCC::MCS_GLOBAL_CHANNEL
                                    , 1 // dataPriority
                                    , 3 // segmentation
                                    , stream.get_packet().size()
                                    , MCS::PER_ENCODING);

    StaticOutStream<256> x224_header;
    X224::DT_TPDU_Send(x224_header, stream.get_packet().size() + mcs_header.get_offset());

    auto packet = stream.copy_to_head(x224_header, mcs_header);
    trans.send(packet.data(), packet.size());
}

template<class... DataWriter>
void send_data_indication_ex( Transport & trans
                            , uint16_t channelId
                            , int encryptionLevel, CryptContext & encrypt
                            , uint16_t initiator, DataWriter... data_writer)
{
    write_packets(
        trans,
        data_writer...,
        [&](StreamSize<256>, OutStream & security_header, uint8_t * data, std::size_t data_sz) {
            SEC::Sec_Send sec(security_header, data, data_sz, 0, encrypt, encryptionLevel);
        },
        [&](StreamSize<256>, OutStream & mcs_header, std::size_t packet_size) {
            MCS::SendDataIndication_Send mcs( static_cast<OutPerStream &>(mcs_header)
                                            , initiator
                                            , channelId
                                            , 1 // dataPriority
                                            , 3 // segmentation
                                            , packet_size
                                            , MCS::PER_ENCODING);
        },
        [](StreamSize<256>, OutStream & x224_header, std::size_t packet_size) {
            X224::DT_TPDU_Send(x224_header, packet_size);
        }
    );
}

inline
void send_share_data_ex( Transport & trans, uint8_t pduType2, bool compression_support
                       , rdp_mppc_enc * mppc_enc, uint32_t shareId, int encryptionLevel
                       , CryptContext & encrypt, uint16_t initiator
                       , StaticOutReservedStreamHelper<1024, 65536-1024> & data
                       // TODO enum Verbose
                       , uint32_t log_condition, uint32_t verbose) {
    assert(!compression_support || mppc_enc);

    StaticOutReservedStreamHelper<1024, 65536-1024> data_compressed;
    std::reference_wrapper<StaticOutReservedStreamHelper<1024, 65536-1024> > data_(data);

    uint8_t compressionFlags = 0;

    if (compression_support) {
        uint16_t compressed_data_size = 0;

        mppc_enc->compress( data.get_packet().data()
                          , data.get_packet().size()
                          , compressionFlags, compressed_data_size
                          , rdp_mppc_enc::MAX_COMPRESSED_DATA_SIZE_UNUSED
                          );

        if (compressionFlags & PACKET_COMPRESSED) {
            mppc_enc->get_compressed_data(data_compressed.get_data_stream());
            data_ = std::ref(data_compressed);
        }
    }


    StaticOutStream<256> share_data_header;
    ShareData share_data(share_data_header);
    share_data.emit_begin( pduType2, shareId, RDP::STREAM_MED
                         , data.get_packet().size() + 18 /* TS_SHAREDATAHEADER(18) */
                         , compressionFlags
                         , (compressionFlags ? data_.get().get_packet().size() + 18 /* TS_SHAREDATAHEADER(18) */ : 0)
                         );
    share_data.emit_end();
    data_.get().copy_to_head(share_data_header);

    StaticOutStream<256> share_ctrl_header;
    ShareControl_Send( share_ctrl_header, PDUTYPE_DATAPDU, initiator + GCC::MCS_USERCHANNEL_BASE
                     , data_.get().get_packet().size());
    data_.get().copy_to_head(share_ctrl_header);

    if (verbose & log_condition) {
        LOG(LOG_INFO, "Sec clear payload to send:");
        hexdump_d(data_.get().get_packet().data(), data_.get().get_packet().size());
    }

    ::send_data_indication_ex(trans, encryptionLevel, encrypt, initiator, data_.get());
}

enum ServerUpdateType {
    SERVER_UPDATE_GRAPHICS_ORDERS,
    SERVER_UPDATE_GRAPHICS_BITMAP,
    SERVER_UPDATE_GRAPHICS_PALETTE,
    SERVER_UPDATE_GRAPHICS_SYNCHRONIZE,
    SERVER_UPDATE_POINTER_COLOR,
    SERVER_UPDATE_POINTER_CACHED,
    SERVER_UPDATE_POINTER_POSITION,
    SERVER_UPDATE_POINTER_NEW
};

inline
void send_server_update( Transport & trans, bool fastpath_support, bool compression_support
                       , rdp_mppc_enc * mppc_enc, uint32_t shareId, int encryptionLevel
                       , CryptContext & encrypt, uint16_t initiator, ServerUpdateType type
                       , uint16_t data_extra
                       , StaticOutReservedStreamHelper<1024, 65536-1024> & data_common
                       // TODO enum Verbose
                       , uint32_t verbose) {
    if (verbose & 4) {
        LOG( LOG_INFO
           , "send_server_update: fastpath_support=%s compression_support=%s shareId=%u "
             "encryptionLevel=%d initiator=%u type=%u data_extra=%u"
           , (fastpath_support ? "yes" : "no"), (compression_support ? "yes" : "no"), shareId
           , encryptionLevel, initiator, type, data_extra
           );
    }

    assert(!compression_support || mppc_enc);

    if (fastpath_support) {
        uint8_t compressionFlags = 0;
        FastPath::UpdateType updateCode       = FastPath::UpdateType::ORDERS;

        switch (type) {
            case SERVER_UPDATE_GRAPHICS_ORDERS:
                {
                    updateCode = FastPath::UpdateType::ORDERS;
                    StaticOutStream<2> data;
                    data.out_uint16_le(data_extra);
                    data_common.copy_to_head(data);
                }
                break;

            case SERVER_UPDATE_GRAPHICS_BITMAP:
                updateCode = FastPath::UpdateType::BITMAP;
                break;

            case SERVER_UPDATE_GRAPHICS_PALETTE:
                updateCode = FastPath::UpdateType::PALETTE;
                break;

            case SERVER_UPDATE_GRAPHICS_SYNCHRONIZE:
                updateCode = FastPath::UpdateType::SYNCHRONIZE;
                break;

            case SERVER_UPDATE_POINTER_COLOR:
                updateCode = FastPath::UpdateType::COLOR;
                break;

            case SERVER_UPDATE_POINTER_CACHED:
                updateCode = FastPath::UpdateType::CACHED;
                break;

            case SERVER_UPDATE_POINTER_POSITION:
                updateCode = FastPath::UpdateType::PTR_POSITION;
                break;

            case SERVER_UPDATE_POINTER_NEW:
                updateCode = FastPath::UpdateType::POINTER;
                break;

            default:
                assert(false);
                break;
        }

        uint8_t compression = 0;

        StaticOutReservedStreamHelper<1024, 65536-1024> data_common_compressed;
        std::reference_wrapper<StaticOutReservedStreamHelper<1024, 65536-1024> > data_common_(data_common);

        if (compression_support) {
            uint16_t compressed_data_size;

            mppc_enc->compress( data_common.get_packet().data()
                              , data_common.get_packet().size()
                              , compressionFlags, compressed_data_size
                              , rdp_mppc_enc::MAX_COMPRESSED_DATA_SIZE_UNUSED
                              );

            if (compressionFlags & PACKET_COMPRESSED) {
                compression = FastPath::FASTPATH_OUTPUT_COMPRESSION_USED;

                mppc_enc->get_compressed_data(data_common_compressed.get_data_stream());
                data_common_ = std::ref(data_common_compressed);
            }
        }

        StaticOutStream<8> update_header;
        // Fast-Path Update (TS_FP_UPDATE)
        FastPath::Update_Send Upd( update_header
                                 , data_common_.get().get_packet().size()
                                 , static_cast<uint8_t>(updateCode)
                                 , FastPath::FASTPATH_FRAGMENT_SINGLE
                                 , compression
                                 , compressionFlags
                                 );
        data_common_.get().copy_to_head(update_header);

        StaticOutStream<256> server_update_header;
         // Server Fast-Path Update PDU (TS_FP_UPDATE_PDU)
        FastPath::ServerUpdatePDU_Send SvrUpdPDU( server_update_header
                                                , data_common_.get().get_packet().data()
                                                , data_common_.get().get_packet().size()
                                                , ((encryptionLevel > 1) ? FastPath::FASTPATH_OUTPUT_ENCRYPTED : 0)
                                                , encrypt
                                                );
        data_common_.get().copy_to_head(server_update_header);

        auto packet = data_common_.get().get_packet();

        trans.send(packet.data(), packet.size());
    }
    else {
        uint8_t pduType2 = 0;

        switch (type) {
            case SERVER_UPDATE_GRAPHICS_ORDERS:
                {
                    pduType2 = PDUTYPE2_UPDATE;

                    StaticOutStream<64> data;

                    data.out_uint16_le(RDP_UPDATE_ORDERS);
                    data.out_clear_bytes(2);
                    data.out_uint16_le(data_extra);
                    data.out_clear_bytes(2);

                    data_common.copy_to_head(data);
                }
                break;

            case SERVER_UPDATE_GRAPHICS_BITMAP:
            case SERVER_UPDATE_GRAPHICS_PALETTE:
                pduType2 = PDUTYPE2_UPDATE;
                break;

            case SERVER_UPDATE_GRAPHICS_SYNCHRONIZE:
                {
                    pduType2 = PDUTYPE2_UPDATE;

                    StaticOutStream<64> data;

                    data.out_uint16_le(RDP_UPDATE_SYNCHRONIZE);
                    data.out_clear_bytes(2);

                    data_common.copy_to_head(data);
                }
                break;

            case SERVER_UPDATE_POINTER_COLOR:
            case SERVER_UPDATE_POINTER_CACHED:
            case SERVER_UPDATE_POINTER_POSITION:
            case SERVER_UPDATE_POINTER_NEW:
                {
                    pduType2 = PDUTYPE2_POINTER;

                    static constexpr uint16_t update_type_table[] {
                        RDP_POINTER_COLOR,
                        RDP_POINTER_CACHED,
                        RDP_POINTER_MOVE,
                        RDP_POINTER_NEW
                    };
                    static_assert(SERVER_UPDATE_POINTER_COLOR + 1 == SERVER_UPDATE_POINTER_CACHED, "");
                    static_assert(SERVER_UPDATE_POINTER_CACHED + 1 == SERVER_UPDATE_POINTER_POSITION, "");
                    static_assert(SERVER_UPDATE_POINTER_POSITION + 1 == SERVER_UPDATE_POINTER_NEW, "");
                    uint16_t const updateType = update_type_table[type - SERVER_UPDATE_POINTER_COLOR];

if (updateType == RDP_POINTER_NEW) {
    LOG(LOG_INFO, "Send Slow-Path New Pointer Update");
}

                    StaticOutStream<64> data;

                    data.out_uint16_le(updateType);
                    data.out_clear_bytes(2);

                    data_common.copy_to_head(data);
                }
                break;

            default:
                assert(false);
                break;
        }

        const uint32_t log_condition = (128 | 4);
        send_share_data_ex( trans, pduType2, compression_support, mppc_enc, shareId
                          , encryptionLevel, encrypt, initiator, data_common
                          , log_condition, verbose);
    }

    if (verbose & 4) {
        LOG(LOG_INFO, "send_server_update done");
    }
}   // send_server_update





// MS-RDPEGDI 2.2.2.2 Fast-Path Orders Update (TS_FP_UPDATE_ORDERS)
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


// MS-RDPEGDI 2.2.2.1 Orders Update (TS_UPDATE_ORDERS_PDU_DATA)
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

namespace detail
{
    // fix: field 'buffer_stream_orders' is uninitialized
    struct GraphicsUpdatePDUBuffer
    {
        StaticOutReservedStreamHelper<1024, 65536-1024> buffer_stream_orders;
        StaticOutReservedStreamHelper<1024, 65536-1024> buffer_stream_bitmaps;
    };
}

class GraphicsUpdatePDU : private detail::GraphicsUpdatePDUBuffer, public RDPSerializer
{
    uint16_t     & userid;
    int          & shareid;
    int          & encryptionLevel;
    CryptContext & encrypt;

    uint32_t offset_bitmap_count;

    bool fastpath_support;

    rdp_mppc_enc * mppc_enc;
    bool           compression;

    bool send_new_pointer;

    Transport & trans;

public:
    GraphicsUpdatePDU( Transport & trans
                     , uint16_t & userid
                     , int & shareid
                     , int & encryptionLevel
                     , CryptContext & encrypt
                     , const uint8_t bpp
                     , BmpCache & bmp_cache
                     , GlyphCache & gly_cache
                     , PointerCache & pointer_cache
                     , const int bitmap_cache_version
                     , const int use_bitmap_comp
                     , const int op2
                     , size_t max_bitmap_size
                     , bool fastpath_support
                     , rdp_mppc_enc * mppc_enc
                     , bool compression
                     , bool send_new_pointer
                     , Verbose verbose
                     )
        : RDPSerializer( this->buffer_stream_orders.get_data_stream()
                       , this->buffer_stream_bitmaps.get_data_stream()
                       , bpp, bmp_cache, gly_cache, pointer_cache
                       , bitmap_cache_version, use_bitmap_comp, op2, max_bitmap_size, verbose)
        , userid(userid)
        , shareid(shareid)
        , encryptionLevel(encryptionLevel)
        , encrypt(encrypt)
        , offset_bitmap_count(0)
        , fastpath_support(fastpath_support)
        , mppc_enc(mppc_enc)
        , compression(compression)
        , send_new_pointer(send_new_pointer)
        , trans(trans) {
        this->init_orders();
        this->init_bitmaps();
    }

    ~GraphicsUpdatePDU() = default;

    void init_orders() {
        if (bool(this->verbose & Verbose::internal_buffer)) {
            LOG( LOG_INFO
               , "GraphicsUpdatePDU::init::Initializing orders batch mcs_userid=%u shareid=%d"
               , this->userid
               , this->shareid);
        }
    }

    void init_bitmaps() {
        if (bool(this->verbose & Verbose::internal_buffer)) {
            LOG( LOG_INFO
               , "GraphicsUpdatePDU::init::Initializing bitmaps batch mcs_userid=%u shareid=%d"
               , this->userid
               , this->shareid);
        }

        this->stream_bitmaps.out_uint16_le(RDP_UPDATE_BITMAP);  // updateType (2 bytes)
        this->offset_bitmap_count = this->stream_bitmaps.get_offset();
        this->stream_bitmaps.out_clear_bytes(2);  // number of bitmap, set later
    }

public:
    void sync() override {
        this->flush_bitmaps();
        this->flush_orders();
    }

protected:
    void flush_orders() override {
        if (this->order_count > 0){
            if (bool(this->verbose & Verbose::internal_buffer)) {
                LOG( LOG_INFO, "GraphicsUpdatePDU::flush_orders: order_count=%zu"
                   , this->order_count);
            }

            ::send_server_update( this->trans, this->fastpath_support, this->compression
                                , this->mppc_enc, this->shareid, this->encryptionLevel
                                , this->encrypt, this->userid, SERVER_UPDATE_GRAPHICS_ORDERS
                                , this->order_count, this->buffer_stream_orders, underlying_cast(this->verbose));

            this->order_count = 0;
            this->buffer_stream_orders.rewind();
            this->init_orders();
        }
    }

    void flush_bitmaps() override {
        if (this->bitmap_count > 0) {
            if (bool(this->verbose & Verbose::internal_buffer)) {
                LOG( LOG_INFO
                   , "GraphicsUpdatePDU::flush_bitmaps: bitmap_count=%zu offset=%" PRIu32
                   , this->bitmap_count, this->offset_bitmap_count);
            }
            this->stream_bitmaps.set_out_uint16_le(this->bitmap_count, this->offset_bitmap_count);

            ::send_server_update( this->trans, this->fastpath_support, this->compression
                                , this->mppc_enc, this->shareid, this->encryptionLevel, this->encrypt
                                , this->userid, SERVER_UPDATE_GRAPHICS_BITMAP, 0
                                , this->buffer_stream_bitmaps, underlying_cast(this->verbose));

            this->bitmap_count = 0;
            this->buffer_stream_bitmaps.rewind();
            this->init_bitmaps();
        }
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
//      (0), then this header is not include " in the PDU.

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

    void GenerateColorPointerUpdateData(OutStream & stream, int cache_idx, const Pointer & cursor)
    {
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

        stream.out_uint16_le(cursor.x);

//            yPos (2 bytes): A 16-bit, unsigned integer. The y-coordinate
//              relative to the top-left corner of the server's desktop.

        stream.out_uint16_le(cursor.y);

//    width (2 bytes): A 16-bit, unsigned integer. The width of the pointer in
//      pixels (the maximum allowed pointer width is 32 pixels).

        stream.out_uint16_le(cursor.width);

//    height (2 bytes): A 16-bit, unsigned integer. The height of the pointer
//      in pixels (the maximum allowed pointer height is 32 pixels).

        stream.out_uint16_le(cursor.height);

//    lengthAndMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//      the andMaskData field.

        stream.out_uint16_le(cursor.mask_size());

//    lengthXorMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//      the xorMaskData field.

        stream.out_uint16_le(cursor.data_size());

//    xorMaskData (variable): Variable number of bytes: Contains the 24-bpp,
//      bottom-up XOR mask scan-line data. The XOR mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 3x3 pixel cursor
//      is being sent, then each scan-line will consume 10 bytes (3 pixels per
//      scan-line multiplied by 3 bpp, rounded up to the next even number of
//      bytes).
        stream.out_copy_bytes(cursor.data, cursor.data_size());

//    andMaskData (variable): Variable number of bytes: Contains the 1-bpp,
//      bottom-up AND mask scan-line data. The AND mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 7x7 pixel cursor
//      is being sent, then each scan-line will consume 2 bytes (7 pixels per
//      scan-line multiplied by 1 bpp, rounded up to the next even number of
//      bytes).
        stream.out_copy_bytes(cursor.mask, cursor.mask_size()); /* mask */

//    colorPointerData (1 byte): Single byte representing unused padding.
//      The contents of this byte should be ignored.
    }

    void GenerateNewPointerUpdateData(OutStream & stream, int cache_idx, const Pointer & cursor)
    {
//    xorBpp (2 bytes): A 16-bit, unsigned integer. The color depth in bits-per-pixel of the XOR mask
//      contained in the colorPtrAttr field.
        stream.out_uint16_le(cursor.only_black_white ? 1 : 32);

        stream.out_uint16_le(cache_idx);

//    hotSpot (4 bytes): Point (section 2.2.9.1.1.4.1) structure containing the
//      x-coordinates and y-coordinates of the pointer hotspot.
//            2.2.9.1.1.4.1  Point (TS_POINT16)
//            ---------------------------------
//            The TS_POINT16 structure specifies a point relative to the
//            top-left corner of the server's desktop.

//            xPos (2 bytes): A 16-bit, unsigned integer. The x-coordinate
//              relative to the top-left corner of the server's desktop.

        stream.out_uint16_le(cursor.x);

//            yPos (2 bytes): A 16-bit, unsigned integer. The y-coordinate
//              relative to the top-left corner of the server's desktop.

        stream.out_uint16_le(cursor.y);

//    width (2 bytes): A 16-bit, unsigned integer. The width of the pointer in
//      pixels (the maximum allowed pointer width is 32 pixels).

        stream.out_uint16_le(cursor.width);

//    height (2 bytes): A 16-bit, unsigned integer. The height of the pointer
//      in pixels (the maximum allowed pointer height is 32 pixels).

        stream.out_uint16_le(cursor.height);


        const unsigned int remainder = (cursor.width % 8);
        const unsigned int and_line_length_in_byte = cursor.width / 8 + (remainder ? 1 : 0);
        const unsigned int and_padded_line_length_in_byte =
            ((and_line_length_in_byte % 2) ?
             and_line_length_in_byte + 1 :
             and_line_length_in_byte);

        const unsigned int xor_padded_line_length_in_byte = (cursor.only_black_white ? and_padded_line_length_in_byte : cursor.width * 4);


//    lengthAndMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//      the andMaskData field.

        stream.out_uint16_le(and_padded_line_length_in_byte * cursor.height);

//    lengthXorMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//      the xorMaskData field.

        stream.out_uint16_le(xor_padded_line_length_in_byte * cursor.height);

//    xorMaskData (variable): Variable number of bytes: Contains the 24-bpp,
//      bottom-up XOR mask scan-line data. The XOR mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 3x3 pixel cursor
//      is being sent, then each scan-line will consume 10 bytes (3 pixels per
//      scan-line multiplied by 3 bpp, rounded up to the next even number of
//      bytes).

        const unsigned int source_xor_line_length_in_byte = cursor.width * 3;
        const unsigned int source_xor_padded_line_length_in_byte =
            ((source_xor_line_length_in_byte % 2) ?
             source_xor_line_length_in_byte + 1 :
             source_xor_line_length_in_byte);

        if (cursor.only_black_white) {
            uint8_t xorMaskData[Pointer::MAX_WIDTH * Pointer::MAX_HEIGHT * 1 / 8] = { 0 };

            for (unsigned int h = 0; h < cursor.height; ++h) {
                const uint8_t * psource = cursor.data
                    + (cursor.height - h - 1) * source_xor_padded_line_length_in_byte;
                uint8_t * pdest   = xorMaskData + h * xor_padded_line_length_in_byte;
                uint8_t xor_bit_mask_generation = 7;
                uint8_t xor_byte = 0;

                for (unsigned int w = 0; w < cursor.width; ++w) {
                    if ((*psource) || (*(psource + 1)) || (*(psource + 2))) {
                        xor_byte |= (1 << xor_bit_mask_generation);
                    }

                    if (!xor_bit_mask_generation) {
                        xor_bit_mask_generation = 8;
                        *pdest = xor_byte;
                        xor_byte = 0;
                        pdest++;
                    }
                    xor_bit_mask_generation--;
                    psource += 3;
                }
                if (xor_bit_mask_generation != 7) {
                    *pdest = xor_byte;
                }
            }

            stream.out_copy_bytes(xorMaskData, xor_padded_line_length_in_byte * cursor.height);
        }
        else {
            uint8_t xorMaskData[Pointer::MAX_WIDTH * Pointer::MAX_HEIGHT * 4] = { 0 };

            for (unsigned int h = 0; h < cursor.height; ++h) {
                const uint8_t* psource = cursor.data + (cursor.height - h - 1) * source_xor_padded_line_length_in_byte;
                      uint8_t* pdest   = xorMaskData + (cursor.height - h - 1) * xor_padded_line_length_in_byte;

                const uint8_t* andMask = cursor.mask + (cursor.height - h - 1) * and_padded_line_length_in_byte;
                unsigned char and_bit_extraction_mask = 7;


                for (unsigned int w = 0; w < cursor.width; ++w) {
                    * pdest      = * psource;
                    *(pdest + 1) = *(psource + 1);
                    *(pdest + 2) = *(psource + 2);
                    if ((*andMask) & (1 << and_bit_extraction_mask)) {
                        *(pdest + 3) = 0x00;
                    }
                    else {
                        *(pdest + 3) = 0xFF;
                    }

                    pdest   += 4;
                    psource += 3;

                    if (and_bit_extraction_mask) {
                        and_bit_extraction_mask--;
                    }
                    else {
                        and_bit_extraction_mask = 7;
                        andMask++;
                    }
                }
            }

            stream.out_copy_bytes(xorMaskData, xor_padded_line_length_in_byte * cursor.height);
        }

//    andMaskData (variable): Variable number of bytes: Contains the 1-bpp,
//      bottom-up AND mask scan-line data. The AND mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 7x7 pixel cursor
//      is being sent, then each scan-line will consume 2 bytes (7 pixels per
//      scan-line multiplied by 1 bpp, rounded up to the next even number of
//      bytes).

        if (cursor.only_black_white) {
            uint8_t andMaskData[Pointer::MAX_WIDTH * Pointer::MAX_HEIGHT / 8] = { 0 };

            for (unsigned int h = 0; h < cursor.height; ++h) {
                const uint8_t* psource = cursor.mask + (cursor.height - h - 1) * and_padded_line_length_in_byte;
                      uint8_t* pdest   = andMaskData + h * and_padded_line_length_in_byte;

                memcpy(pdest, psource, and_padded_line_length_in_byte);
            }

            stream.out_copy_bytes(andMaskData, and_padded_line_length_in_byte * cursor.height); /* mask */
        }
        else {
            stream.out_copy_bytes(cursor.mask, cursor.mask_size()); /* mask */
        }

//    colorPointerData (1 byte): Single byte representing unused padding.
//      The contents of this byte should be ignored.
    }

    void send_pointer(int cache_idx, const Pointer & cursor) override {
        if (bool(this->verbose & Verbose::pointer)) {
            LOG(LOG_INFO, "GraphicsUpdatePDU::send_pointer(cache_idx=%d x=%d y=%d)",
                cache_idx, cursor.x, cursor.y);
        }

        StaticOutReservedStreamHelper<1024, 65536-1024> stream;

        if (this->send_new_pointer) {
            GenerateNewPointerUpdateData(stream.get_data_stream(), cache_idx, cursor);
        }
        else {
            GenerateColorPointerUpdateData(stream.get_data_stream(), cache_idx, cursor);
        }

        ::send_server_update( this->trans, this->fastpath_support, this->compression
                            , this->mppc_enc, this->shareid, this->encryptionLevel
                            , this->encrypt, this->userid
                            , (this->send_new_pointer ? SERVER_UPDATE_POINTER_NEW : SERVER_UPDATE_POINTER_COLOR)
                            , 0, stream, underlying_cast(this->verbose));

        if (bool(this->verbose & Verbose::pointer)) {
            LOG(LOG_INFO, "GraphicsUpdatePDU::send_pointer done");
        }
    }   // void send_pointer(int cache_idx, const Pointer & cursor)

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

    void set_pointer(int cache_idx) override {
        if (bool(this->verbose & Verbose::pointer)) {
            LOG(LOG_INFO, "GraphicsUpdatePDU::set_pointer(cache_idx=%d)", cache_idx);
        }

        StaticOutReservedStreamHelper<1024, 65536-1024> stream;
        stream.get_data_stream().out_uint16_le(cache_idx);

        ::send_server_update( this->trans, this->fastpath_support, this->compression
                            , this->mppc_enc, this->shareid, this->encryptionLevel
                            , this->encrypt, this->userid, SERVER_UPDATE_POINTER_CACHED
                            , 0, stream, underlying_cast(this->verbose));

        if (bool(this->verbose & Verbose::pointer)) {
            LOG(LOG_INFO, "GraphicsUpdatePDU::set_pointer done");
        }
    }   // void set_pointer(int cache_idx)

public:
    using RDPSerializer::set_pointer;

    void update_pointer_position(uint16_t xPos, uint16_t yPos) {
        if (bool(this->verbose & Verbose::pointer)) {
            LOG(LOG_INFO, "GraphicsUpdatePDU::update_pointer_position(xPos=%u, yPos=%u)", xPos, yPos);
        }

        StaticOutReservedStreamHelper<1024, 65536-1024> stream;
        stream.get_data_stream().out_uint16_le(xPos);
        stream.get_data_stream().out_uint16_le(yPos);

        ::send_server_update( this->trans, this->fastpath_support, this->compression
                            , this->mppc_enc, this->shareid, this->encryptionLevel
                            , this->encrypt, this->userid, SERVER_UPDATE_POINTER_POSITION
                            , 0, stream, underlying_cast(this->verbose));

        if (bool(this->verbose & Verbose::pointer)) {
            LOG(LOG_INFO, "GraphicsUpdatePDU::update_pointer_position done");
        }
    }
};

