/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou

    rdp transparent analyzer module main header file
*/
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <string>

#define LOGPRINT
#include "utils/log.hpp"

#include "core/channel_list.hpp"
#include "core/front_api.hpp"
#include "transport/in_file_transport.hpp"
#include "core/RDP/autoreconnect.hpp"
#include "core/RDP/mppc_unified_dec.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"
#include "core/RDP/protocol.hpp"
#include "core/RDP/SaveSessionInfoPDU.hpp"
#include "capture/transparentplayer.hpp"
#include "main/version.hpp"
#include "utils/sugar/local_fd.hpp"
#include "program_options/program_options.hpp"

struct GraphicNull
{
    GraphicNull() = default;

    template<class... Ts>
    void draw(Ts const & ...) {}

    void set_pointer(Pointer    const &) {}
    void set_palette(BGRPalette const &) {}
    void sync() {}
    void set_row(std::size_t, const uint8_t *) {}
    void begin_update() {}
    void end_update() {}
};


class Analyzer : public FrontAPI
{
    GraphicNull get_graphic_proxy() { return {}; }

private:
    CHANNELS::ChannelDefArray channel_list;

    rdp_mppc_unified_dec mppc_dec;

    RDPOrderCommon     common;
    RDPDestBlt         destblt;
    RDPPatBlt          patblt;
    RDPScrBlt          scrblt;
    RDPLineTo          lineto;
    RDPOpaqueRect      opaquerect;
    RDPMemBlt          memblt;
    RDPMem3Blt         mem3blt;
    RDPMultiDstBlt     multidstblt;
    RDPMultiOpaqueRect multiopaquerect;
    RDP::RDPMultiPatBlt multipatblt;
    RDP::RDPMultiScrBlt multiscrblt;
    RDPPolyline        polyline;

    struct Statistic {
        uint32_t cache_bitmap_compressed_count;
        uint32_t cache_bitmap_uncompressed_count;
        uint32_t cache_bitmap_compressed_rev2_count;
        uint32_t cache_bitmap_uncompressed_rev2_count;
        uint32_t cache_bitmap_compressed_rev3_count;

        uint32_t bitmapupdate_count;

        uint32_t dstblt_count;
        uint32_t patblt_count;
        uint32_t scrblt_count;
        uint32_t lineto_count;
        uint32_t opaquerect_count;
        uint32_t memblt_count;
        uint32_t mem3blt_count;
        uint32_t multidstblt_count;
        uint32_t multiopaquerect_count;
        uint32_t multipatblt_count;
        uint32_t multiscrblt_count;
        uint32_t polyline_count;

        Statistic()
        : cache_bitmap_compressed_count(0)
        , cache_bitmap_uncompressed_count(0)
        , cache_bitmap_compressed_rev2_count(0)
        , cache_bitmap_uncompressed_rev2_count(0)
        , cache_bitmap_compressed_rev3_count(0)
        , bitmapupdate_count(0)
        , dstblt_count(0)
        , patblt_count(0)
        , scrblt_count(0)
        , lineto_count(0)
        , opaquerect_count(0)
        , memblt_count(0)
        , mem3blt_count(0)
        , multidstblt_count(0)
        , multiopaquerect_count(0)
        , multipatblt_count(0)
        , multiscrblt_count(0)
        , polyline_count(0)
        {}
    } statistic;

public:
    // DrawApi
    bool can_be_start_capture(auth_api*) override { REDASSERT(false); return false; }
    bool must_be_stop_capture() override { REDASSERT(false); return false; }

    // FrontAPI
    const CHANNELS::ChannelDefArray & get_channel_list() const override {
        return this->channel_list;
    }
    void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * /*data*/
                                , size_t length, size_t chunk_size, int flags) override {
        LOG(LOG_INFO, "send_to_channel: channel_name=\"%s\"(%d) data_length=%zu chunk_size=%zu flags=0x%X",
            channel.name, channel.chanid, length, chunk_size, flags);
    }

    ResizeResult server_resize(int width, int height, int bpp) override {
        LOG(LOG_INFO, "server_resize: width=%u height=%u bpp=%u", width, height, bpp);
        return ResizeResult::done;
    }

    void send_data_indication_ex(uint16_t channelId, uint8_t const * data, std::size_t data_size) override {
        LOG(LOG_INFO, "send_data_indication_ex: channelId=%u stream_size=%zu", channelId, data_size);

        InStream stream(data, data_size);
        ShareControl_Recv sctrl(stream);

        switch (sctrl.pduType) {
            case PDUTYPE_DATAPDU:
            {
                LOG(LOG_INFO, "send_data_indication_ex: Received PDUTYPE_DATAPDU(0x%X)", sctrl.pduType);

                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                switch (sdata.pdutype2) {
                    case PDUTYPE2_UPDATE:
                    {
                        LOG(LOG_INFO, "send_data_indication_ex: Received PDUTYPE2_UPDATE(0x%X)", sdata.pdutype2);
                        SlowPath::GraphicsUpdate_Recv gp_udp_r(sdata.payload);
                        switch (gp_udp_r.update_type) {
                            case RDP_UPDATE_ORDERS:
                                LOG(LOG_INFO, "send_data_indication_ex: Received RDP_UPDATE_ORDERS(0x%X)", gp_udp_r.update_type);
                                this->process_orders(sdata.payload, false);
                            break;
                            case RDP_UPDATE_BITMAP:
                                LOG(LOG_INFO, "send_data_indication_ex: Received UPDATETYPE_BITMAP(0x%X)", gp_udp_r.update_type);
                                this->statistic.bitmapupdate_count++;
                            break;
                            case RDP_UPDATE_PALETTE:
                                LOG(LOG_INFO, "send_data_indication_ex: Received UPDATETYPE_PALETTE(0x%X)", gp_udp_r.update_type);
                            break;
                            case RDP_UPDATE_SYNCHRONIZE:
                                LOG(LOG_INFO, "send_data_indication_ex: Received UPDATETYPE_SYNCHRONIZE(0x%X)", gp_udp_r.update_type);
                            break;
                            default:
                                LOG(LOG_INFO, "send_data_indication_ex: Received unexpected Server Graphics Update Type (0x%X)", gp_udp_r.update_type);
                            break;
                        }
                    }
                    break;
                    case PDUTYPE2_SAVE_SESSION_INFO:
                    {
                        LOG(LOG_INFO, "send_data_indication_ex: Received PDUTYPE2_SAVE_SESSION_INFO(0x%X)", sdata.pdutype2);
                        RDP::SaveSessionInfoPDUData_Recv ssipdudata(sdata.payload);

                        switch (ssipdudata.infoType) {
                        case RDP::INFOTYPE_LOGON:
                        {
                            LOG(LOG_INFO, "send_data_indication_ex: Received INFOTYPE_LOGON(0x%X)", ssipdudata.infoType);
                            RDP::LogonInfoVersion1_Recv liv1(ssipdudata.payload);
                        }
                        break;
                        case RDP::INFOTYPE_LOGON_LONG:
                        {
                            LOG(LOG_INFO, "send_data_indication_ex: Received INFOTYPE_LOGON_LONG(0x%X)", ssipdudata.infoType);
                            RDP::LogonInfoVersion2_Recv liv2(ssipdudata.payload);
                        }
                        break;
                        case RDP::INFOTYPE_LOGON_PLAINNOTIFY:
                        {
                            LOG(LOG_INFO, "send_data_indication_ex: Received INFOTYPE_LOGON_PLAINNOTIFY(0x%X)", ssipdudata.infoType);
                            RDP::PlainNotify_Recv pn(ssipdudata.payload);
                        }
                        break;
                        case RDP::INFOTYPE_LOGON_EXTENDED_INFO:
                        {
                            LOG(LOG_INFO, "send_data_indication_ex: Received INFOTYPE_LOGON_EXTENDED_INFO(0x%X)", ssipdudata.infoType);
                            RDP::LogonInfoExtended_Recv lie(ssipdudata.payload);

                            RDP::LogonInfoField_Recv lif(lie.payload);

                            if (lie.FieldsPresent & RDP::LOGON_EX_AUTORECONNECTCOOKIE) {
                                LOG(LOG_INFO, "send_data_indication_ex : Auto-reconnect cookie");

                                RDP::ServerAutoReconnectPacket_Recv sarp(lif.payload);
                            }
                            if (lie.FieldsPresent & RDP::LOGON_EX_LOGONERRORS) {
                                LOG(LOG_INFO, "send_data_indication_ex : Logon Errors Info");

                                RDP::LogonErrorsInfo_Recv lei(lif.payload);
                            }
                        }
                        break;
                        }
                    }
                    break;
                    case PDUTYPE2_SET_ERROR_INFO_PDU:
                        LOG(LOG_INFO, "send_data_indication_ex: Received PDUTYPE2_SET_ERROR_INFO_PDU(0x%X)", sdata.pdutype2);
                    break;
                    default:
                        LOG(LOG_INFO, "send_data_indication_ex: ***** Received unexpected data PDU, pdu_type2=0x%X *****", sdata.pdutype2);
                    break;
                }
            }
            break;

            default:
                LOG(LOG_INFO, "send_data_indication_ex: ***** Received unexpected PDU, pdu_type1=0x%X *****", sctrl.pduType);
            break;
        }
    }

    void send_fastpath_data(InStream & data) override {
        LOG(LOG_INFO, "send_fastpath_data: data_size=%zu", data.get_capacity());

        while (data.in_remain()) {
            FastPath::Update_Recv fp_upd_r(data, &this->mppc_dec);
            switch (static_cast<FastPath::UpdateType>(fp_upd_r.updateCode)) {
                case FastPath::UpdateType::ORDERS:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_ORDERS(0x%X)", fp_upd_r.updateCode);
                    this->process_orders(fp_upd_r.payload, true);
                break;
                case FastPath::UpdateType::BITMAP:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_BITMAP(0x%X)", fp_upd_r.updateCode);
                    this->statistic.bitmapupdate_count++;
                break;
                case FastPath::UpdateType::PALETTE:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_PALETTE(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::UpdateType::SYNCHRONIZE:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_SYNCHRONIZE(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::UpdateType::SURFCMDS:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_SURFCMDS(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::UpdateType::PTR_NULL:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_PTR_NULL(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::UpdateType::PTR_DEFAULT:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_PTR_DEFAULT(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::UpdateType::PTR_POSITION:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_PTR_POSITION(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::UpdateType::COLOR:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_COLOR(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::UpdateType::POINTER:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_POINTER(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::UpdateType::CACHED:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_CACHED(0x%X)", fp_upd_r.updateCode);
                break;
                default:
                    LOG(LOG_INFO, "send_fastpath_data: ***** Received unexpected fast-past PDU, updateCode=0x%X *****", fp_upd_r.updateCode);
                break;
            }
        }
    }

    void process_orders(InStream & stream, bool fast_path) {
        RDP::OrdersUpdate_Recv odrs_upd_r(stream, fast_path);

        int processed = 0;
        while (processed < odrs_upd_r.number_orders) {
            RDP::DrawingOrder_RecvFactory drawodr_rf(stream);

            if ((drawodr_rf.control_flags & (RDP::STANDARD | RDP::SECONDARY)) == (RDP::STANDARD | RDP::SECONDARY)) {
                RDPSecondaryOrderHeader sec_odr_h(stream);
                uint8_t const * next_order = stream.get_current() + sec_odr_h.order_data_length();
                switch (sec_odr_h.type) {
                    case RDP::TS_CACHE_BITMAP_COMPRESSED:
                        LOG(LOG_INFO, "process_orders: Received FASTPATH_UPDATETYPE_BITMAP(0x%X)", sec_odr_h.type);
                        this->statistic.cache_bitmap_compressed_count++;
                    break;
                    case RDP::TS_CACHE_BITMAP_UNCOMPRESSED:
                        LOG(LOG_INFO, "process_orders: Received TS_CACHE_BITMAP_UNCOMPRESSED(0x%X)", sec_odr_h.type);
                        this->statistic.cache_bitmap_uncompressed_count++;
                    break;
                    case RDP::TS_CACHE_COLOR_TABLE:
                        LOG(LOG_INFO, "process_orders: Received TS_CACHE_COLOR_TABLE(0x%X)", sec_odr_h.type);
                    break;
                    case RDP::TS_CACHE_GLYPH:
                        LOG(LOG_INFO, "process_orders: Received TS_CACHE_GLYPH(0x%X)", sec_odr_h.type);
                    break;
                    case RDP::TS_CACHE_BITMAP_COMPRESSED_REV2:
                        LOG(LOG_INFO, "process_orders: Received TS_CACHE_BITMAP_COMPRESSED_REV2(0x%X)", sec_odr_h.type);
                        this->statistic.cache_bitmap_compressed_rev2_count++;
                    break;
                    case RDP::TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
                        LOG(LOG_INFO, "process_orders: Received TS_CACHE_BITMAP_UNCOMPRESSED_REV2(0x%X)", sec_odr_h.type);
                        this->statistic.cache_bitmap_uncompressed_rev2_count++;
                    break;
                    case RDP::TS_CACHE_BITMAP_COMPRESSED_REV3:
                        LOG(LOG_INFO, "process_orders: Received TS_CACHE_BITMAP_COMPRESSED_REV3(0x%X)", sec_odr_h.type);
                        this->statistic.cache_bitmap_compressed_rev3_count++;
                    break;
                    default:
                        LOG(LOG_INFO, "process_orders: ***** Received unexpected Secondary Drawing Order, type=0x%X *****", sec_odr_h.type);
                    break;
                }
                stream.in_skip_bytes(next_order - stream.get_current());
            }
            else if ((drawodr_rf.control_flags & (RDP::STANDARD | RDP::SECONDARY)) == RDP::STANDARD) {
                RDPPrimaryOrderHeader pri_ord_h = this->common.receive(stream, drawodr_rf.control_flags);
                switch (this->common.order) {
                    case RDP::DESTBLT:
                        LOG(LOG_INFO, "process_orders: Received TS_ENC_DSTBLT_ORDER(0x%X)", this->common.order);
                        this->destblt.receive(stream, pri_ord_h);
                        this->statistic.dstblt_count++;
                    break;
                    case RDP::PATBLT:
                        LOG(LOG_INFO, "process_orders: Received TS_ENC_PATBLT_ORDER(0x%X)", this->common.order);
                        this->patblt.receive(stream, pri_ord_h);
                        this->statistic.patblt_count++;
                    break;
                    case RDP::SCREENBLT:
                        LOG(LOG_INFO, "process_orders: Received TS_ENC_SCRBLT_ORDER(0x%X)", this->common.order);
                        this->scrblt.receive(stream, pri_ord_h);
                        this->statistic.scrblt_count++;
                    break;
                    case RDP::MEMBLT:
                        LOG(LOG_INFO, "process_orders: Received TS_ENC_MEMBLT_ORDER(0x%X)", this->common.order);
                        this->memblt.receive(stream, pri_ord_h);
                        this->statistic.memblt_count++;
                    break;
                    case RDP::MEM3BLT:
                        LOG(LOG_INFO, "process_orders: Received TS_ENC_MEM3BLT_ORDER(0x%X)", this->common.order);
                        this->mem3blt.receive(stream, pri_ord_h);
                        this->statistic.mem3blt_count++;
                    break;
                    case RDP::LINE:
                        LOG(LOG_INFO, "process_orders: Received TS_ENC_LINETO_ORDER(0x%X)", this->common.order);
                        this->lineto.receive(stream, pri_ord_h);
                        this->statistic.lineto_count++;
                    break;
                    case RDP::RECT:
                        LOG(LOG_INFO, "process_orders: Received TS_ENC_OPAQUERECT_ORDER(0x%X)", this->common.order);
                        this->opaquerect.receive(stream, pri_ord_h);
                        this->statistic.opaquerect_count++;
                    break;
                    case RDP::MULTIDSTBLT:
                        LOG(LOG_INFO, "process_orders: Received TS_ENC_MULTIDSTBLT_ORDER(0x%X)", this->common.order);
                        this->multidstblt.receive(stream, pri_ord_h);
                        this->statistic.multidstblt_count++;
                    break;
                    case RDP::MULTIOPAQUERECT:
                        LOG(LOG_INFO, "process_orders: Received TS_ENC_MULTIOPAQUERECT_ORDER(0x%X)", this->common.order);
                        this->multiopaquerect.receive(stream, pri_ord_h);
                        this->statistic.multiopaquerect_count++;
                    break;
                    case RDP::MULTIPATBLT:
                        LOG(LOG_INFO, "process_orders: Received TS_ENC_MULTIPATBLT_ORDER(0x%X)", this->common.order);
                        this->multipatblt.receive(stream, pri_ord_h);
                        this->statistic.multipatblt_count++;
                    break;
                    case RDP::MULTISCRBLT:
                        LOG(LOG_INFO, "process_orders: Received TS_ENC_MULTISCRBLT_ORDER(0x%X)", this->common.order);
                        this->multiscrblt.receive(stream, pri_ord_h);
                        this->statistic.multiscrblt_count++;
                    break;
                    case RDP::POLYLINE:
                        LOG(LOG_INFO, "process_orders: Received TS_ENC_POLYLINE_ORDER(0x%X)", this->common.order);
                        this->polyline.receive(stream, pri_ord_h);
                        this->statistic.polyline_count++;
                    break;
                    default:
                        LOG(LOG_INFO, "process_orders: ***** Received unexpected Primary Drawing Order, type=0x%X *****", this->common.order);
                    break;
                }
            }
            else if ((drawodr_rf.control_flags & (RDP::STANDARD | RDP::SECONDARY)) == RDP::SECONDARY) {
                RDP::AltsecDrawingOrderHeader header(drawodr_rf.control_flags);
                switch (header.orderType) {
                    case RDP::AltsecDrawingOrderHeader::FrameMarker:
                    {
                        LOG(LOG_INFO, "process_orders: Received TS_ALTSEC_FRAME_MARKER(0x%X)", header.orderType);
                        RDP::FrameMarker order;

                        order.receive(stream, header);
                    }
                    break;
                    default:
                        LOG(LOG_ERR, "unsupported Alternate Secondary Drawing Order (%d)", header.orderType);
                        /* error, unknown order */
                    break;
                }
            }
            else {
                LOG(LOG_ERR, "process_orders: Non standard order detected, protocol error");
                throw Error(ERR_RDP_PROTOCOL);
            }
            processed++;
        }
    }

    void update_pointer_position(uint16_t, uint16_t) override {}

public:
    void draw(RDP::FrameMarker    const & cmd) override { this->draw_impl(cmd); }
    void draw(RDPDestBlt          const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPScrBlt           const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override { this->draw_impl(cmd, bmp); }
    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override { this->draw_impl(cmd, clip, bmp);}
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override { this->draw_impl(cmd, clip, color_ctx, bmp); }
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override { this->draw_impl(cmd, clip, color_ctx, gly_cache); }

    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::WindowIcon                     & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::CachedIcon                     & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::DeletedWindow                  & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override { this->draw_impl(cmd); }

    void draw(RDPColCache   const & cmd) override { this->draw_impl(cmd); }
    void draw(RDPBrushCache const & cmd) override { this->draw_impl(cmd); }

    void set_pointer(Pointer    const & pointer) override {
        this->get_graphic_proxy().set_pointer(pointer);
    }

    void set_palette(BGRPalette const & palette) override {
        this->get_graphic_proxy().set_palette(palette);
    }

    void sync() override {
        this->get_graphic_proxy().sync();
    }

    void set_row(std::size_t rownum, const uint8_t * data) override {
        this->get_graphic_proxy().set_row(rownum, data);
    }

    void begin_update() override {
        this->get_graphic_proxy().begin_update();
    }

    void end_update() override {
        this->get_graphic_proxy().end_update();
    }

protected:
    template<class... Ts>
    void draw_impl(Ts const & ... args) {
        this->get_graphic_proxy().draw(args...);
    }

public:
    Analyzer()
    : FrontAPI(false, false)
    , common(RDP::PATBLT, Rect(0, 0, 1, 1))
    , destblt(Rect(), 0)
    , patblt(Rect(), 0, 0, 0, RDPBrush())
    , scrblt(Rect(), 0, 0, 0)
    , lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0))
    , opaquerect(Rect(), 0)
    , memblt(0, Rect(), 0, 0, 0, 0)
    , mem3blt(0, Rect(), 0, 0, 0, 0, 0, RDPBrush(), 0)
    , multidstblt()
    , multiopaquerect()
    , multipatblt()
    , multiscrblt()
    , polyline()
    {
        InitializeVirtualChannelList();
    }

    void InitializeVirtualChannelList() {
        CHANNELS::ChannelDef channel_item;

        strcpy(channel_item.name, "rdpdr");
        channel_item.flags  = 0x80800000;
        channel_item.chanid = 1004;
        this->channel_list.push_back(channel_item);

        strcpy(channel_item.name, "rdpsnd");
        channel_item.flags  = 0xc0000000;
        channel_item.chanid = 1005;
        this->channel_list.push_back(channel_item);

        strcpy(channel_item.name, "drdynvc");
        channel_item.flags  = 0xc0800000;
        channel_item.chanid = 1006;
        this->channel_list.push_back(channel_item);

        strcpy(channel_item.name, "cliprdr");
        channel_item.flags  = 0xc0a00000;
        channel_item.chanid = 1007;
        this->channel_list.push_back(channel_item);
    }

    void show_statistic() {
        LOG(LOG_INFO, "****************************************");
        LOG(LOG_INFO, "cache_bitmap_compressed count=%u",        this->statistic.cache_bitmap_compressed_count);
        LOG(LOG_INFO, "cache_bitmap_uncompressed count=%u",      this->statistic.cache_bitmap_uncompressed_count);
        LOG(LOG_INFO, "cache_bitmap_compressed_rev2 count=%u",   this->statistic.cache_bitmap_compressed_rev2_count);
        LOG(LOG_INFO, "cache_bitmap_uncompressed_rev2 count=%u", this->statistic.cache_bitmap_uncompressed_rev2_count);

        LOG(LOG_INFO, "bitmapupdate count=%u", this->statistic.bitmapupdate_count);

        LOG(LOG_INFO, "dstblt count=%u",          this->statistic.dstblt_count);
        LOG(LOG_INFO, "patblt count=%u",          this->statistic.patblt_count);
        LOG(LOG_INFO, "scrblt count=%u",          this->statistic.scrblt_count);
        LOG(LOG_INFO, "lineto count=%u",          this->statistic.lineto_count);
        LOG(LOG_INFO, "opaquerect count=%u",      this->statistic.opaquerect_count);
        LOG(LOG_INFO, "memblt count=%u",          this->statistic.memblt_count);
        LOG(LOG_INFO, "mem3blt count=%u",         this->statistic.mem3blt_count);
        LOG(LOG_INFO, "multidstblt count=%u",     this->statistic.multidstblt_count);
        LOG(LOG_INFO, "multiopaquerect count=%u", this->statistic.multiopaquerect_count);
        LOG(LOG_INFO, "multipatblt count=%u",     this->statistic.multipatblt_count);
        LOG(LOG_INFO, "multiscrblt count=%u",     this->statistic.multiscrblt_count);
        LOG(LOG_INFO, "polyline count=%u",        this->statistic.polyline_count);
        LOG(LOG_INFO, "****************************************");
    }
};  // class Analyzer

int main(int argc, char * argv[]) {
    openlog("tanalyzer", LOG_CONS | LOG_PERROR, LOG_USER);

    const char * copyright_notice =
        "\n"
        "ReDemPtion Transparent Analyzer " VERSION ".\n"
        "Copyright (C) Wallix 2010-2015.\n"
        "Christophe Grosjean, Raphael Zhou.\n"
        "\n"
        ;

    std::string input_filename;

    program_options::options_description desc({
        {'h', "help",    "produce help message"},
        {'v', "version", "show software version"},

        {'i', "input-file", &input_filename, "input ini file name"},
    });

    auto options = program_options::parse_command_line(argc, argv, desc);

    if (options.count("help") > 0) {
        std::cout << copyright_notice;
        std::cout << "Usage: rdptanalyzer [options]\n\n";
        std::cout << desc << std::endl;
        return 1;
    }

    if (options.count("version") > 0) {
        std::cout << copyright_notice;
        return 1;
    }

    if (input_filename.empty()) {
        std::cout << "Use -i filename\n\n";
        return 1;
    }

    local_fd file(input_filename.c_str(), O_RDONLY);
    if (file.is_open()) {
        InFileTransport trans(file.fd());
        Analyzer        analyzer;

        TransparentPlayer player(&trans, &analyzer);

        while (player.interpret_chunk(/*real_time = */false));

        std::cout << "\n";
        analyzer.show_statistic();
    }
    else {
        std::cout << "Failed to open input file: " << input_filename << "\n\n";
        return 1;
    }

    return 0;
}
