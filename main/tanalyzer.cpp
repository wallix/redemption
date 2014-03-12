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

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <string>

#define LOGPRINT
#include "log.hpp"

#include "channel_list.hpp"
#include "front_api.hpp"
#include "infiletransport.hpp"
#include "RDP/protocol.hpp"
#include "transparentplayer.hpp"
#include "version.hpp"

class Analyzer : public FrontAPI {
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
    RDPMultiOpaqueRect multiopaquerect;
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
        uint32_t multiopaquerect_count;
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
        , multiopaquerect_count(0)
        , polyline_count(0)
        {}
    } statistic;

public:
    // RDPGraphicDevice
    virtual void draw(const RDPOpaqueRect      & cmd, const Rect & clip) { REDASSERT(false); }
    virtual void draw(const RDPScrBlt          & cmd, const Rect & clip) { REDASSERT(false); }
    virtual void draw(const RDPDestBlt         & cmd, const Rect & clip) { REDASSERT(false); }
    virtual void draw(const RDPMultiDstBlt     & cmd, const Rect & clip) { REDASSERT(false); }
    virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) { REDASSERT(false); }
    virtual void draw(const RDPPatBlt          & cmd, const Rect & clip) { REDASSERT(false); }
    virtual void draw(const RDPMemBlt          & cmd, const Rect & clip, const Bitmap & bmp) { REDASSERT(false); }
    virtual void draw(const RDPMem3Blt         & cmd, const Rect & clip, const Bitmap & bmp) { REDASSERT(false); }
    virtual void draw(const RDPLineTo          & cmd, const Rect & clip) { REDASSERT(false); }
    virtual void draw(const RDPGlyphIndex      & cmd, const Rect & clip, const GlyphCache * gly_cache) { REDASSERT(false); }
    virtual void draw(const RDPPolygonSC       & cmd, const Rect & clip) { REDASSERT(false); }
    virtual void draw(const RDPPolygonCB       & cmd, const Rect & clip) { REDASSERT(false); }
    virtual void draw(const RDPPolyline        & cmd, const Rect & clip) { REDASSERT(false); }
    virtual void draw(const RDPEllipseSC       & cmd, const Rect & clip) { REDASSERT(false); }
    virtual void draw(const RDPEllipseCB       & cmd, const Rect & clip) { REDASSERT(false); }

    virtual void flush() { REDASSERT(false); }

    // DrawApi
    virtual void begin_update() { REDASSERT(false); }
    virtual void end_update() { REDASSERT(false); }

    virtual void text_metrics(const char * text, int & width, int & height) { REDASSERT(false); }

    virtual void server_draw_text( int16_t x, int16_t y, const char * text
                                 , uint32_t fgcolor, uint32_t bgcolor, const Rect & clip) { REDASSERT(false); }

    // FrontAPI
    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const {
        return this->channel_list;
    }
    virtual void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t * data
                                , size_t length, size_t chunk_size, int flags) {
        LOG(LOG_INFO, "send_to_channel: channel_name=\"%s\" data_length=%u chunk_size=%u flags=0x%X",
            channel.name, length, chunk_size, flags);
    }

    virtual void send_global_palette() throw(Error) { REDASSERT(false); }
    virtual void set_mod_palette(const BGRPalette & palette) { REDASSERT(false); }

    virtual int server_resize(int width, int height, int bpp) {
        LOG(LOG_INFO, "server_resize: width=%u height=%u bpp=%u", width, height, bpp);
        return 1;
    };

    virtual void send_data_indication_ex(uint16_t channelId, HStream & stream) {
        LOG(LOG_INFO, "send_data_indication_ex: channelId=%u stream_size=%u", channelId, stream.size());

        stream.p = stream.get_data();

        ShareControl_Recv sctrl(stream);

        switch (sctrl.pdu_type1) {
            case PDUTYPE_DATAPDU:
            {
                LOG(LOG_INFO, "send_data_indication_ex: Received PDUTYPE_DATAPDU(0x%X)", sctrl.pdu_type1);

                ShareData sdata(stream);
                sdata.recv_begin(&this->mppc_dec);
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
                        LOG(LOG_INFO, "send_data_indication_ex: Received PDUTYPE2_SAVE_SESSION_INFO(0x%X)", sdata.pdutype2);
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
                LOG(LOG_INFO, "send_data_indication_ex: ***** Received unexpected PDU, pdu_type1=0x%X *****", sctrl.pdu_type1);
            break;
        }
    }

    virtual void send_fastpath_data(Stream & data) {
        LOG(LOG_INFO, "send_fastpath_data: data_size=%u", data.size());

        while (data.in_remain()) {
            FastPath::Update_Recv fp_upd_r(data, &this->mppc_dec);
            switch (fp_upd_r.updateCode) {
                case FastPath::FASTPATH_UPDATETYPE_ORDERS:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_ORDERS(0x%X)", fp_upd_r.updateCode);
                    this->process_orders(fp_upd_r.payload, true);
                break;
                case FastPath::FASTPATH_UPDATETYPE_BITMAP:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_BITMAP(0x%X)", fp_upd_r.updateCode);
                    this->statistic.bitmapupdate_count++;
                break;
                case FastPath::FASTPATH_UPDATETYPE_PALETTE:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_PALETTE(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::FASTPATH_UPDATETYPE_SYNCHRONIZE:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_SYNCHRONIZE(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::FASTPATH_UPDATETYPE_PTR_NULL:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_PTR_NULL(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::FASTPATH_UPDATETYPE_PTR_DEFAULT:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_PTR_DEFAULT(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::FASTPATH_UPDATETYPE_PTR_POSITION:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_PTR_POSITION(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::FASTPATH_UPDATETYPE_COLOR:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_COLOR(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::FASTPATH_UPDATETYPE_POINTER:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_POINTER(0x%X)", fp_upd_r.updateCode);
                break;
                case FastPath::FASTPATH_UPDATETYPE_CACHED:
                    LOG(LOG_INFO, "send_fastpath_data: Received FASTPATH_UPDATETYPE_CACHED(0x%X)", fp_upd_r.updateCode);
                break;
                default:
                    LOG(LOG_INFO, "send_fastpath_data: ***** Received unexpected fast-past PDU, updateCode=0x%X *****", fp_upd_r.updateCode);
                break;
            }
        }
    }

    void process_orders(Stream & stream, bool fast_path) {
        RDP::OrdersUpdate_Recv odrs_upd_r(stream, fast_path);

        int processed = 0;
        while (processed < odrs_upd_r.number_orders) {
            RDP::DrawingOrder_RecvFactory drawodr_rf(stream);

            if (!drawodr_rf.control_flags & RDP::STANDARD) {
                LOG(LOG_ERR, "process_orders: Non standard order detected, protocol error");
                throw Error(ERR_RDP_PROTOCOL);
            }
            if (drawodr_rf.control_flags & RDP::SECONDARY) {
                RDPSecondaryOrderHeader sec_odr_h(stream);
                uint8_t * next_order = stream.p + sec_odr_h.order_data_length();
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
                stream.p = next_order;
            }
            else {
                RDPPrimaryOrderHeader pri_ord_h = this->common.receive(stream, drawodr_rf.control_flags);
                switch (this->common.order) {
/*
                    case RDP::DESTBLT:
                        LOG(LOG_INFO, "process_orders: Received TS_ENC_DSTBLT_ORDER(0x%X)", this->common.order);
                        this->destblt.receive(stream, pri_ord_h);
                        this->statistic.dstblt_count++;
                    break;
*/
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
                    case RDP::MULTIOPAQUERECT:
                        LOG(LOG_INFO, "process_orders: Received TS_ENC_MULTIOPAQUERECT_ORDER(0x%X)", this->common.order);
                        this->multiopaquerect.receive(stream, pri_ord_h);
                        this->statistic.multiopaquerect_count++;
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
            processed++;
        }
    }

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
    , multiopaquerect()
    , polyline() {
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
        LOG(LOG_INFO, "multiopaquerect count=%u", this->statistic.multiopaquerect_count);
        LOG(LOG_INFO, "polyline count=%u",        this->statistic.polyline_count);
        LOG(LOG_INFO, "****************************************");
    }
};  // class Analyzer

int main(int argc, char * argv[]) {
    openlog("tanalyzer", LOG_CONS | LOG_PERROR, LOG_USER);

    const char * copyright_notice =
        "\n"
        "ReDemPtion Transparent Analyzer " VERSION ".\n"
        "Copyright (C) Wallix 2010-2014.\n"
        "Christophe Grosjean, Raphael Zhou.\n"
        "\n"
        ;

    std::string input_filename;

    boost::program_options::options_description desc("Options");
    desc.add_options()
    ("help,h",    "produce help message")
    ("version,v", "show software version")

    ("input-file,i", boost::program_options::value(&input_filename), "input ini file name")
    ;

    boost::program_options::variables_map options;
    boost::program_options::store(
        boost::program_options::command_line_parser(argc, argv).options(desc).run(),
        options
    );
    boost::program_options::notify(options);

    if (options.count("help") > 0) {
        std::cout << copyright_notice;
        std::cout << "Usage: rdptanalyzer [options]\n\n";
        std::cout << desc << std::endl;
        exit(-1);
    }

    if (options.count("version") > 0) {
        std::cout << copyright_notice;
        exit(-1);
    }

    if (input_filename.empty()) {
        std::cout << "Use -i filename\n\n";
        exit(-1);
    }

    int fd = open(input_filename.c_str(), O_RDONLY);
    if (fd != -1) {
        {
            InFileTransport trans(fd);
            Analyzer        analyzer;

            TransparentPlayer player(&trans, &analyzer);

            while (player.interpret_chunk(/*real_time = */false));

            LOG(LOG_INFO, "");
            analyzer.show_statistic();
        }

        close(fd);
    }
    else {
        std::cout << "Failed to open input file: " << input_filename << "\n\n";
        exit(-1);
    }

    return 0;
}