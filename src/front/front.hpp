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
    Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
               Dominique Lafages, Raphael Zhou, Meng Tan,
               Jennifer Inthavong
    Based on xrdp Copyright (C) Jay Sorg 2004-2010

    Front object (server), used to communicate with RDP client
*/

#pragma once

#include "capture/capture.hpp"
#include "capture/params_from_ini.hpp"
#include "configs/config.hpp"
#include "core/RDP/GraphicUpdatePDU.hpp"
#include "core/RDP/MonitorLayoutPDU.hpp"
#include "core/RDP/PersistentKeyListPDU.hpp"
#include "core/RDP/SaveSessionInfoPDU.hpp"
#include "core/RDP/SuppressOutputPDU.hpp"
#include "core/RDP/caches/bmpcache.hpp"
#include "core/RDP/caches/bmpcachepersister.hpp"
#include "core/RDP/caches/brushcache.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/caches/pointercache.hpp"
#include "core/RDP/capabilities/bitmapcachehostsupport.hpp"
#include "core/RDP/capabilities/bmpcache2.hpp"
#include "core/RDP/capabilities/cap_bmpcache.hpp"
#include "core/RDP/capabilities/cap_brushcache.hpp"
#include "core/RDP/capabilities/cap_font.hpp"
#include "core/RDP/capabilities/cap_glyphcache.hpp"
#include "core/RDP/capabilities/cap_share.hpp"
#include "core/RDP/capabilities/colcache.hpp"
#include "core/RDP/capabilities/compdesk.hpp"
#include "core/RDP/capabilities/input.hpp"
#include "core/RDP/capabilities/largepointer.hpp"
#include "core/RDP/capabilities/multifragmentupdate.hpp"
#include "core/RDP/capabilities/offscreencache.hpp"
#include "core/RDP/capabilities/pointer.hpp"
#include "core/RDP/capabilities/rail.hpp"
#include "core/RDP/capabilities/window.hpp"
#include "core/RDP/fastpath.hpp"
#include "core/RDP/gcc.hpp"
#include "core/RDP/lic.hpp"
#include "core/RDP/mcs.hpp"
#include "core/RDP/mppc.hpp"
#include "core/RDP/nego.hpp"
#include "core/RDP/remote_programs.hpp"
#include "core/RDP/sec.hpp"
#include "core/RDP/slowpath.hpp"
#include "core/RDP/tpdu_buffer.hpp"
#include "core/RDP/x224.hpp"
#include "core/app_path.hpp"
#include "core/callback.hpp"
#include "core/channel_list.hpp"
#include "core/channel_names.hpp"
#include "core/client_info.hpp"
#include "core/date_dir_from_filename.hpp"
#include "core/error.hpp"
#include "core/font.hpp"
#include "core/front_api.hpp"
#include "core/report_message_api.hpp"
#include "core/session_reactor.hpp"
#include "gdi/clip_from_cmd.hpp"
#include "keyboard/keymap2.hpp"
#include "transport/in_file_transport.hpp"
#include "utils/bitfu.hpp"
#include "utils/bitmap_private_data.hpp"
#include "utils/colors.hpp"
#include "utils/confdescriptor.hpp"
#include "utils/contiguous_sub_rect_f.hpp"
#include "utils/crypto/ssl_lib.hpp"
#include "utils/genfstat.hpp"
#include "utils/genrandom.hpp"
#include "utils/log.hpp"
#include "utils/pattutils.hpp"
#include "utils/rect.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/sugar/not_null_ptr.hpp"
#include "utils/strutils.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>


enum { MAX_DATA_BLOCK_SIZE = 1024 * 30 };

class Front : public FrontAPI
{
    using FrontAPI::draw;

    // for printf with %p
    using voidp = void const *;

    bool nomouse;
    int mouse_x = 0;
    int mouse_y = 0;

public:
    bool has_user_activity = true;
    std::unique_ptr<Capture> capture;

    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        basic_trace     = 1 << 0,
        basic_trace2    = 1 << 1,
        basic_trace3    = 1 << 2,
        basic_trace4    = 1 << 3,
        basic_trace5    = 1 << 5,
        graphic         = 1 << 6,
        channel         = 1 << 7,
        global_channel  = 1 << 13,
        sec_decrypted   = 1 << 14,
        keymap          = 1 << 15,

        // RDPSerializer
        bmp_cache        = 1 << 8,
        internal_buffer  = 1 << 9,

        // BmpCachePersister
        cache_from_disk  = 1 << 10,
        bmp_info         = 1 << 11,
    };

private:
    struct Graphics
    {
        BmpCache bmp_cache;
        std::unique_ptr<BmpCachePersister> bmp_cache_persister;
        BrushCache brush_cache;
        PointerCache pointer_cache;
        GlyphCache glyph_cache;

        struct PrivateGraphicsUpdatePDU final : GraphicsUpdatePDU {
            size_t max_data_block_size;

            PrivateGraphicsUpdatePDU(
                OrderCaps & client_order_caps
              , Transport & trans
              , uint16_t & userid
              , int & shareid
              , int & encryptionLevel
              , CryptContext & encrypt
              , const BitsPerPixel bpp
              , BmpCache & bmp_cache
              , GlyphCache & gly_cache
              , PointerCache & pointer_cache
              , const int bitmap_cache_version
              , const int use_bitmap_comp
              , const int op2
              , size_t max_data_block_size
              , bool experimental_enable_serializer_data_block_size_limit
              , bool fastpath_support
              , rdp_mppc_enc * mppc_enc
              , bool compression
              , bool send_new_pointer
              , GraphicsUpdatePDU::Verbose verbose
            )
            : GraphicsUpdatePDU(
                trans
              , userid
              , shareid
              , encryptionLevel
              , encrypt
              , bpp
              , bmp_cache
              , gly_cache
              , pointer_cache
              , bitmap_cache_version
              , use_bitmap_comp
              , op2
              , max_data_block_size
              , experimental_enable_serializer_data_block_size_limit
              , fastpath_support
              , mppc_enc
              , compression
              , send_new_pointer
              , verbose
            )
            , max_data_block_size(max_data_block_size)
            , client_order_caps(client_order_caps)
            {}

            using GraphicsUpdatePDU::draw;

            void draw(const RDP::FrameMarker & order) override {
                if (this->client_order_caps.orderSupportExFlags & ORDERFLAGS_EX_ALTSEC_FRAME_MARKER_SUPPORT) {
                    GraphicsUpdatePDU::draw(order);
                }
            }

            void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
                Bitmap new_bmp(this->capture_bpp, bmp);

                size_t const serializer_max_data_block_size = this->get_max_data_block_size();

                if (static_cast<size_t>(new_bmp.cx() * new_bmp.cy() * underlying_cast(new_bmp.bpp())) > serializer_max_data_block_size) { /*NOLINT*/
                    const uint16_t max_image_width
                      = std::min<uint16_t>(
                            ((serializer_max_data_block_size / nb_bytes_per_pixel(new_bmp.bpp())) & ~3),
                            new_bmp.cx()
                        );
                    const uint16_t max_image_height = serializer_max_data_block_size / (max_image_width * nb_bytes_per_pixel(new_bmp.bpp()));

                    contiguous_sub_rect_f(
                        CxCy{new_bmp.cx(), new_bmp.cy()},
                        SubCxCy{max_image_width, max_image_height},
                        [&](Rect subrect){
                            Bitmap sub_image(new_bmp, subrect);

                            StaticOutStream<65535> bmp_stream;
                            sub_image.compress(this->capture_bpp, bmp_stream);

                            RDPBitmapData sub_image_data = bitmap_data;

                            sub_image_data.dest_left += subrect.x;
                            sub_image_data.dest_top  += subrect.y;

                            sub_image_data.dest_right = std::min<uint16_t>(sub_image_data.dest_left + subrect.cx - 1, bitmap_data.dest_right);
                            sub_image_data.dest_bottom = sub_image_data.dest_top + subrect.cy - 1;

                            sub_image_data.width = subrect.cx;
                            sub_image_data.height = subrect.cy;

                            sub_image_data.bits_per_pixel = safe_int(sub_image.bpp());
                            sub_image_data.flags = BITMAP_COMPRESSION | NO_BITMAP_COMPRESSION_HDR; /*NOLINT*/
                            sub_image_data.bitmap_length = bmp_stream.get_offset();

                            GraphicsUpdatePDU::draw(sub_image_data, sub_image);
                        }
                    );
                }
                else {
                    StaticOutStream<65535> bmp_stream;
                    new_bmp.compress(this->capture_bpp, bmp_stream);

                    RDPBitmapData target_bitmap_data = bitmap_data;

                    target_bitmap_data.bits_per_pixel = safe_int(new_bmp.bpp());
                    target_bitmap_data.flags = BITMAP_COMPRESSION | NO_BITMAP_COMPRESSION_HDR; /*NOLINT*/
                    target_bitmap_data.bitmap_length = bmp_stream.get_offset();

                    GraphicsUpdatePDU::draw(target_bitmap_data, new_bmp);
                }
            }

            void set_palette(const BGRPalette& /*unused*/) override {
            }

            OrderCaps & client_order_caps;
        } graphics_update_pdu;

        Graphics(
            OrderCaps & client_order_caps
          , ClientInfo const & client_info
          , Transport & trans
          , uint16_t & userid
          , int & shareid
          , int & encryptionLevel
          , CryptContext & encrypt
          , const Inifile & ini
          , size_t max_data_block_size
          , bool fastpath_support
          , rdp_mppc_enc * mppc_enc
          , Verbose verbose
        )
        : bmp_cache(
            BmpCache::Front
          , client_info.screen_info.bpp
          , client_info.number_of_cache
          , ((client_info.cache_flags & ALLOW_CACHE_WAITING_LIST_FLAG) && ini.get<cfg::client::cache_waiting_list>()),
            BmpCache::CacheOption(
                client_info.cache1_entries
              , client_info.cache1_size
              , client_info.cache1_persistent),
            BmpCache::CacheOption(
                client_info.cache2_entries
              , client_info.cache2_size
              , client_info.cache2_persistent),
            BmpCache::CacheOption(
                client_info.cache3_entries
              , client_info.cache3_size
              , client_info.cache3_persistent),
            BmpCache::CacheOption(
                client_info.cache4_entries
              , client_info.cache4_size
              , client_info.cache4_persistent),
            BmpCache::CacheOption(
                client_info.cache5_entries
              , client_info.cache5_size
              , client_info.cache5_persistent),
            to_verbose_flags(ini.get<cfg::debug::cache>())
          )
        , bmp_cache_persister([&ini, verbose, this]() {
            std::unique_ptr<BmpCachePersister> bmp_cache_persister;

            if (ini.get<cfg::client::persistent_disk_bitmap_cache>() &&
                ini.get<cfg::client::persist_bitmap_cache_on_disk>() &&
                bmp_cache.has_cache_persistent()) {
                // Generates the name of file.
                char cache_filename[2048];
                ::snprintf(cache_filename, sizeof(cache_filename) - 1, "%s/client/PDBC-%s-%d",
                    app_path(AppPath::Persistent), ini.get<cfg::globals::host>().c_str(),
                    underlying_cast(this->bmp_cache.bpp));
                cache_filename[sizeof(cache_filename) - 1] = '\0';

                int fd = ::open(cache_filename, O_RDONLY);
                if (fd != -1) {
                    try {
                        InFileTransport ift(unique_fd{fd});

                        BmpCachePersister::Verbose cache_verbose
                            = ( bool(verbose & Verbose::cache_from_disk)
                                ? BmpCachePersister::Verbose::from_disk
                                : BmpCachePersister::Verbose::none
                            )|( bool(verbose & Verbose::bmp_info)
                                ? BmpCachePersister::Verbose::bmp_info
                                : BmpCachePersister::Verbose::none
                            );
                        bmp_cache_persister = std::make_unique<BmpCachePersister>(
                            this->bmp_cache, ift, cache_filename, cache_verbose);
                    }
                    catch (const Error & e) {
                        if (e.id != ERR_PDBC_LOAD) {
                            throw;
                        }
                    }
                }
            }

            return bmp_cache_persister;
        }())
        , pointer_cache(client_info.pointer_cache_entries)
        , glyph_cache(client_info.number_of_entries_in_glyph_cache)
        , graphics_update_pdu(
            client_order_caps
          , trans
          , userid
          , shareid
          , encryptionLevel
          , encrypt
          , client_info.screen_info.bpp
          , this->bmp_cache
          , this->glyph_cache
          , this->pointer_cache
          , client_info.bitmap_cache_version
          , ini.get<cfg::client::bitmap_compression>()
          , client_info.use_compact_packets
          , max_data_block_size
          , ini.get<cfg::globals::experimental_enable_serializer_data_block_size_limit>()
          , fastpath_support
          , mppc_enc
          , bool(ini.get<cfg::client::rdp_compression>()) ? client_info.rdp_compression : false
          , bool(ini.get<cfg::client::enable_new_pointer_update>()) ? client_info.supported_new_pointer_update : false
          , ( (ini.get<cfg::debug::primary_orders>()
                ? RDPSerializer::Verbose::primary_orders   : RDPSerializer::Verbose::none)
            | (ini.get<cfg::debug::secondary_orders>()
                ? RDPSerializer::Verbose::secondary_orders : RDPSerializer::Verbose::none)
            | (ini.get<cfg::debug::bitmap_update>()
                ? RDPSerializer::Verbose::bitmap_update    : RDPSerializer::Verbose::none)
            | (bool(verbose & Verbose::bmp_cache)
                ? RDPSerializer::Verbose::bmp_cache        : RDPSerializer::Verbose::none)
            | (bool(verbose & Verbose::internal_buffer)
                ? RDPSerializer::Verbose::internal_buffer  : RDPSerializer::Verbose::none)
            | (bool(verbose & Verbose::basic_trace4)
                ? RDPSerializer::Verbose::pointer          : RDPSerializer::Verbose::none)
            )
        )
        {}
    };

    struct GraphicsPointer
    {
        void initialize(
            OrderCaps & client_order_caps
          , ClientInfo const & client_info
          , Transport & trans
          , uint16_t & userid
          , int & shareid
          , int & encryptionLevel
          , CryptContext & encrypt
          , const Inifile & ini
          , size_t max_data_block_size
          , bool fastpath_support
          , rdp_mppc_enc * mppc_enc
          , Verbose verbose
        ) {
            if (this->is_initialized) {
                this->is_initialized = false;
                this->u.graphics.~Graphics();
            }

            new (&this->u.graphics) Graphics(
                client_order_caps, client_info, trans, userid, shareid, encryptionLevel, encrypt,
                ini, max_data_block_size, fastpath_support, mppc_enc, verbose
            );
            this->is_initialized = true;
        }

        void clear_bmp_cache_persister()
        {
            this->get_graphics().bmp_cache_persister.reset();
        }

        bool has_bmp_cache_persister() const
        {
            return this->is_initialized && this->get_graphics().bmp_cache_persister;
        }

        BmpCachePersister * bmp_cache_persister() const
        {
            return this->get_graphics().bmp_cache_persister.get();
        }

        BitsPerPixel bpp() const
        {
            return this->get_bmp_cache().bpp;
        }

        BmpCache const & get_bmp_cache() const
        {
            return this->get_graphics().bmp_cache;
        }

        int add_brush(uint8_t* brush_item_data, int& cache_idx)
        {
            return this->get_graphics().brush_cache.add_brush(brush_item_data, cache_idx);
        }

        brush_item const & brush_at(int cache_idx) const
        {
            return this->get_graphics().brush_cache.brush_items[cache_idx];
        }

        Graphics::PrivateGraphicsUpdatePDU & graphics_update_pdu()
        {
            return this->get_graphics().graphics_update_pdu;
        }

        ~GraphicsPointer()
        {
            if (this->is_initialized) {
                this->u.graphics.~Graphics();
            }
        }

    private:
        Graphics & get_graphics()
        {
            assert(this->is_initialized);
            return this->u.graphics;
        }

        Graphics const & get_graphics() const
        {
            assert(this->is_initialized);
            return this->u.graphics;
        }

        union U
        {
            Graphics graphics;
            char dummy;

            U() : dummy() {}
            ~U() {} /*NOLINT*/
        } u;
        bool is_initialized = false;
    } orders;

    /// \param fn  Fn(MCS::ChannelJoinRequest_Recv &)
    template<class Fn>
    void channel_join_request_transmission(InStream & x224_data, Fn fn) {
        assert(buf.current_pdu_get_type() == X224::DT_TPDU);
        X224::DT_TPDU_Recv x224(x224_data);
        MCS::ChannelJoinRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);

        fn(mcs);

        write_packets(
            this->trans,
            [&mcs](StreamSize<256>, OutStream & mcs_cjcf_data) {
                MCS::ChannelJoinConfirm_Send(
                    mcs_cjcf_data, MCS::RT_SUCCESSFUL,
                    mcs.initiator, mcs.channelId,
                    true, mcs.channelId,
                    MCS::PER_ENCODING
                );
            },
            X224::write_x224_dt_tpdu_fn{}
        );
    }

    not_null_ptr<gdi::GraphicApi> gd = &gdi::null_gd();
    not_null_ptr<gdi::GraphicApi> graphics_update = &gdi::null_gd();

    void set_gd(gdi::GraphicApi * new_gd) {
        this->gd = new_gd;
        this->graphics_update = new_gd;
    }

    void set_gd(gdi::GraphicApi & new_gd) {
        this->set_gd(&new_gd);
    }

public:
    Keymap2 keymap;

protected:
    CHANNELS::ChannelDefArray channel_list;

    // TODO private !!!
public:
    bool up_and_running = false;

private:
    int share_id = 65538;
    int encryptionLevel; /* 1, 2, 3 = low, medium, high */

public:
    ClientInfo client_info;

private:
    Transport & trans;

    uint16_t userid = 0;
    uint8_t pub_mod[512];
    uint8_t pri_exp[512];
    uint8_t server_random[32];
    CryptContext encrypt, decrypt;

    int order_level = 0;

private:
    Inifile & ini;
    CryptoContext & cctx;

    bool palette_memblt_sent[6];

    BGRPalette mod_palette_rgb {BGRPalette::classic_332()};

    BitsPerPixel mod_bpp {0};
    BitsPerPixel capture_bpp {0};

    enum {
        CONNECTION_INITIATION,
        BASIC_SETTINGS_EXCHANGE,
        CHANNEL_ATTACH_USER,
        CHANNEL_JOIN_REQUEST,
        CHANNEL_JOIN_CONFIRM_USER_ID,
        CHANNEL_JOIN_CONFIRM_CHECK_USER_ID,
        CHANNEL_JOIN_CONFIRM_LOOP,
        WAITING_FOR_LOGON_INFO,
        WAITING_FOR_ANSWER_TO_LICENCE,
        ACTIVATE_AND_PROCESS_DATA
    } state = CONNECTION_INITIATION;

    Random & gen;
    Fstat fstat;

    bool fastpath_support;                    // choice of programmer
    bool client_fastpath_input_event_support; // = choice of programmer
    bool server_fastpath_update_support;      // choice of programmer + capability of client
    bool tls_client_active;
    bool mem3blt_support;
    int clientRequestedProtocols;

    std::string server_capabilities_filename;

    std::unique_ptr<rdp_mppc_enc> mppc_enc;

    ReportMessageApi & report_message;
    bool       auth_info_sent;

    uint16_t rail_channel_id = 0;

    size_t max_data_block_size = MAX_DATA_BLOCK_SIZE;

    bool focus_on_password_textbox         = false;
    bool focus_on_unidentified_input_field = false;
    bool consent_ui_is_visible             = false;

    bool session_probe_started_ = false;

    bool is_client_disconnected = false;

    bool client_support_monitor_layout_pdu = false;

    bool is_first_memblt = true;

    SessionReactor& session_reactor;
    SessionReactor::TimerPtr handshake_timeout;
    SessionReactor::CallbackEventPtr incoming_event;
    SessionReactor::TimerPtr capture_timer;
     SessionReactor::TimerPtr flow_control_timer;

public:
    bool ignore_rdesktop_bogus_clip = false;

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
    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override {
        // TODO: check if this is still necessary with current rdesktop (likely not)
        /// NOTE force resize cliping with rdesktop...
        if (this->is_first_memblt && !this->ignore_rdesktop_bogus_clip){
            this->draw_impl(cmd, Rect(clip.x,clip.y,1,1), bmp);
            this->is_first_memblt = false;
        }
        this->draw_impl(cmd, clip, bmp);
    }
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override { this->draw_impl(cmd, clip, color_ctx, bmp); }
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override { this->draw_impl(cmd, clip, color_ctx, gly_cache); }

    void draw(RDPNineGrid const &  /*unused*/, Rect  /*unused*/, gdi::ColorCtx  /*unused*/, Bitmap const &  /*unused*/) override {}

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

    BGRPalette const & get_palette() const { return this->mod_palette_rgb; }

    const std::chrono::milliseconds rdp_keepalive_connection_interval;

public:
    Front( SessionReactor& session_reactor
         , Transport & trans
         , Random & gen
         , Inifile & ini
         , CryptoContext & cctx
         , ReportMessageApi & report_message
         , bool fp_support // If true, fast-path must be supported
         , bool mem3blt_support
         , time_t /*now*/
         , std::string server_capabilities_filename = {}
         )
    : nomouse(ini.get<cfg::globals::nomouse>())
    , verbose(static_cast<Verbose>(ini.get<cfg::debug::front>()))
    , keymap(bool(this->verbose & Verbose::keymap) ? 1 : 0)
    , encryptionLevel(underlying_cast(ini.get<cfg::globals::encryptionLevel>()) + 1)
    , trans(trans)
    , ini(ini)
    , cctx(cctx)
    , gen(gen)
    , fastpath_support(fp_support)
    , client_fastpath_input_event_support(fp_support)
    , server_fastpath_update_support(false)
    , tls_client_active(true)
    , mem3blt_support(mem3blt_support)
    , clientRequestedProtocols(X224::PROTOCOL_RDP)
    , server_capabilities_filename(std::move(server_capabilities_filename))
    , report_message(report_message)
    , auth_info_sent(false)
    , session_reactor(session_reactor)
    , rdp_keepalive_connection_interval(
            (ini.get<cfg::globals::rdp_keepalive_connection_interval>().count() &&
             (ini.get<cfg::globals::rdp_keepalive_connection_interval>() < std::chrono::milliseconds(1000))) ? std::chrono::milliseconds(1000) : ini.get<cfg::globals::rdp_keepalive_connection_interval>()
          )
    {
        if (this->ini.get<cfg::globals::handshake_timeout>().count()) {
            this->handshake_timeout = session_reactor.create_timer()
            .set_delay(this->ini.get<cfg::globals::handshake_timeout>())
            .on_action([](auto ctx){
                LOG(LOG_ERR, "Front::incoming: RDP handshake timeout reached!");
                throw Error(ERR_RDP_HANDSHAKE_TIMEOUT);
                return ctx.ready();
            });
        }

        // init TLS
        // --------------------------------------------------------


        // -------- Start of system wide SSL_Ctx option ------------------------------

        // ERR_load_crypto_strings() registers the error strings for all libcrypto
        // functions. SSL_load_error_strings() does the same, but also registers the
        // libssl error strings.

        // One of these functions should be called before generating textual error
        // messages. However, this is not required when memory usage is an issue.

        // ERR_free_strings() frees all previously loaded error strings.

        SSL_load_error_strings();

        // SSL_library_init() registers the available SSL/TLS ciphers and digests.
        // OpenSSL_add_ssl_algorithms() and SSLeay_add_ssl_algorithms() are synonyms
        // for SSL_library_init().

        // - SSL_library_init() must be called before any other action takes place.
        // - SSL_library_init() is not reentrant.
        // - SSL_library_init() always returns "1", so it is safe to discard the return
        // value.

        // Note: OpenSSL 0.9.8o and 1.0.0a and later added SHA2 algorithms to
        // SSL_library_init(). Applications which need to use SHA2 in earlier versions
        // of OpenSSL should call OpenSSL_add_all_algorithms() as well.

        SSL_library_init();

        // --------------------------------------------------------

        for (bool& x : this->palette_memblt_sent) {
            x = false;
        }

        // from server_sec
        // CGR: see if init has influence for the 3 following fields
        memset(this->server_random, 0, 32);

        // shared
        memset(this->decrypt.key, 0, 16);
        memset(this->encrypt.key, 0, 16);
        memset(this->decrypt.update_key, 0, 16);
        memset(this->encrypt.update_key, 0, 16);

        switch (this->encryptionLevel) {
        case 1:
        case 2:
            this->decrypt.encryptionMethod = 1; /* 40 bits */
            this->encrypt.encryptionMethod = 1; /* 40 bits */
        break;
        default:
        case 3:
            this->decrypt.encryptionMethod = 2; /* 128 bits */
            this->encrypt.encryptionMethod = 2; /* 128 bits */
        break;
        }

        if (this->rdp_keepalive_connection_interval.count()) {
            this->flow_control_timer = this->session_reactor.create_timer()
            .set_delay(std::chrono::milliseconds(0))
            .on_action([this](auto ctx){
                if (this->up_and_running) {
                    this->send_data_indication_ex_impl(
                        GCC::MCS_GLOBAL_CHANNEL,
                        [&](StreamSize<256>, OutStream & stream) {
                            ShareFlow_Send(stream, FLOW_TEST_PDU, 0, 0, this->userid + GCC::MCS_USERCHANNEL_BASE);
                            if (bool(this->verbose & Verbose::global_channel)) {
                                LOG(LOG_INFO, "Front::process_flow_control_event: Sec clear payload to send:");
                                hexdump_d(stream.get_data(), stream.get_offset());
                            }
                        }
                    );
                }

                return ctx.ready_to(this->rdp_keepalive_connection_interval);
            });
        }
    }

    ~Front() override {
        ERR_free_strings();

        if (this->orders.has_bmp_cache_persister()) {
            this->save_persistent_disk_bitmap_cache();
        }
    }

    ResizeResult server_resize(int width, int height, BitsPerPixel bpp) override
    {
        ResizeResult res = ResizeResult::no_need;

        this->mod_bpp = bpp;

        if (bpp == BitsPerPixel{8}) {
            this->palette_sent = false;
            for (bool & b : this->palette_memblt_sent) {
                b = false;
            }
        }

        if (this->client_info.screen_info.width != width
         || this->client_info.screen_info.height != height) {
            if (!this->client_info.remote_program) {
                /* older client can't resize */
                if (client_info.build <= 419) {
                    LOG(LOG_WARNING, "Front::server_resize: Resizing is not available on older RDP clients");
                    // resizing needed but not available
                    res = ResizeResult::fail;
                }
                else {
                    LOG(LOG_INFO, "Front::server_resize: Resizing client to : %d x %d x %d", width, height, this->client_info.screen_info.bpp);

                    this->client_info.screen_info.width = width;
                    this->client_info.screen_info.height = height;

                    this->ini.set_acl<cfg::context::opt_width>(this->client_info.screen_info.width);
                    this->ini.set_acl<cfg::context::opt_height>(this->client_info.screen_info.height);

                    // TODO Why are we not calling this->flush() instead ? Looks dubious.
                    // send buffered orders
                    this->orders.graphics_update_pdu().sync();

                    if (this->capture) {
                        if (this->ini.get<cfg::globals::experimental_support_resize_session_during_recording>()) {
                            this->capture->resize(width, height);
                        }
                        else {
                            this->must_be_stop_capture();
                            this->can_be_start_capture();
                        }
                    }

                    // clear all pending orders, caches data, and so on and
                    // start a send_deactive, send_deman_active process with
                    // the new resolution setting
                    /* shut down the rdp client */
                    this->up_and_running = false;
                    this->send_deactive();
                    /* this should do the actual resizing */
                    this->send_demand_active();
                    this->send_monitor_layout();

                    LOG(LOG_INFO, "Front::server_resize: ACTIVATED (resize)");
                    state = ACTIVATE_AND_PROCESS_DATA;
                    this->is_first_memblt = true;
                    res = ResizeResult::done;
                }
            }
            else {
                this->incoming_event = this->session_reactor
                .create_callback_event(std::ref(*this))
                .on_action(jln::one_shot([](Callback& cb, Front& front){
                    cb.refresh(Rect(0, 0, front.client_info.screen_info.width, front.client_info.screen_info.height));
                }));
                res = ResizeResult::remoteapp;
            }
        }

        // resizing not necessary
        return res;
    }

    void set_pointer(const Pointer & cursor) override {
        this->graphics_update->set_pointer(cursor);
    }

    void update_pointer_position(uint16_t xPos, uint16_t yPos) override
    {
        this->orders.graphics_update_pdu().update_pointer_position(xPos, yPos);
        //if (  this->capture
        //   && (this->capture_state == CAPTURE_STATE_STARTED)) {
        //    this->capture->update_pointer_position(xPos, yPos);
        //}
    }

    // ===========================================================================
    bool can_be_start_capture() override
    {
        // Recording is enabled.
        // TODO simplify use of movie flag. Should probably be tested outside before calling start_capture. Do we still really need that flag. Maybe sesman can just provide flags of recording types

        if (this->capture) {
            LOG(LOG_INFO, "Front::can_be_start_capture: session capture is already started");
            return false;
        }

        if (!ini.get<cfg::globals::is_rec>() &&
            bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::syslog) &&
            !::contains_kbd_or_ocr_pattern(ini.get<cfg::context::pattern_kill>().c_str()) &&
            !::contains_kbd_or_ocr_pattern(ini.get<cfg::context::pattern_notify>().c_str())
        ) {
            LOG(LOG_INFO, "Front::can_be_start_capture: Capture is not necessary");
            return false;
        }

        bool const capture_pattern_checker
          = (::contains_ocr_pattern(ini.get<cfg::context::pattern_kill>().c_str())
          || ::contains_ocr_pattern(ini.get<cfg::context::pattern_notify>().c_str()));

        if (!ini.get<cfg::globals::is_rec>()) {
            ini.set<cfg::video::capture_flags>(
                capture_pattern_checker ? CaptureFlags::ocr : CaptureFlags::none);
            ini.set<cfg::video::png_limit>(0);
        }

        LOG(LOG_INFO, "---<>  Front::can_be_start_capture  <>---");

        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::can_be_start_capture: movie_path    = %s\n", ini.get<cfg::globals::movie_path>());
            LOG(LOG_INFO, "Front::can_be_start_capture: auth_user     = %s\n", ini.get<cfg::globals::auth_user>());
            LOG(LOG_INFO, "Front::can_be_start_capture: host          = %s\n", ini.get<cfg::globals::host>());
            LOG(LOG_INFO, "Front::can_be_start_capture: target_device = %s\n", ini.get<cfg::globals::target_device>());
            LOG(LOG_INFO, "Front::can_be_start_capture: target_user   = %s\n", ini.get<cfg::globals::target_user>());
        }

        this->capture_bpp = ((ini.get<cfg::video::wrm_color_depth_selection_strategy>() == ColorDepthSelectionStrategy::depth16) ? BitsPerPixel{16} : BitsPerPixel{24});
        // TODO remove this after unifying capture interface
        VideoParams video_params = video_params_from_ini(
            this->client_info.screen_info.width, this->client_info.screen_info.height,
            std::chrono::seconds::zero(), ini);

        const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>().c_str();
        std::string record_path = ini.get<cfg::video::record_path>().to_string();
        const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
        std::string hash_path = ini.get<cfg::video::hash_path>().to_string();
        const char * movie_path = ini.get<cfg::globals::movie_path>().c_str();

        {
            DateDirFromFilename d(ini.get<cfg::session_log::log_path>());
            if (!d.has_date()) {
                LOG(LOG_WARNING, "Front::can_be_start_capture: failed to extract date");
            }

            hash_path.append(d.date_path().begin(), d.date_path().end());
            record_path.append(d.date_path().begin(), d.date_path().end());
        }

        for (auto* s : {&record_path, &hash_path}) {
            if (recursive_create_directory(s->c_str(), S_IRWXU | S_IRGRP | S_IXGRP, groupid) != 0) {
                LOG(LOG_ERR, "Front::can_be_start_capture: Failed to create directory: \"%s\"", *s);
            }
        }

        const CaptureFlags capture_flags = ini.get<cfg::video::capture_flags>();

        const bool capture_wrm = bool(capture_flags & CaptureFlags::wrm);

        const bool capture_ocr = bool(capture_flags & CaptureFlags::ocr) || capture_pattern_checker;
        const bool capture_video = bool(capture_flags & CaptureFlags::video);
        // TODO missing CaptureFlags::full_video
        const bool capture_video_full = false;
        // TODO missing CaptureFlags::meta
        const bool capture_meta = false /*bool(capture_flags & CaptureFlags::meta)*/;
        const bool capture_kbd = !bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::syslog)
          || ini.get<cfg::session_log::enable_session_log>()
          || ::contains_kbd_pattern(ini.get<cfg::context::pattern_kill>().c_str())
          || ::contains_kbd_pattern(ini.get<cfg::context::pattern_notify>().c_str())
        ;

        OcrParams const ocr_params = ocr_params_from_ini(ini);

        char path[1024];
        char basename[1024];
        char extension[128];

        // default value, actual one should come from movie_path
        auto const wrm_path_len = utils::strlcpy(path, app_path(AppPath::Wrm));
        if (wrm_path_len + 2 < std::size(path)) {
            path[wrm_path_len] = '/';
            path[wrm_path_len+1] = 0;
        }
        utils::strlcpy(basename, movie_path);
        extension[0] = 0; // extension is currently ignored

        if (!canonical_path(movie_path, path, sizeof(path), basename, sizeof(basename), extension, sizeof(extension))
        ) {
            LOG(LOG_ERR, "Front::can_be_start_capture: Buffer Overflowed: Path too long");
            throw Error(ERR_RECORDER_FAILED_TO_FOUND_PATH);
        }

        PngParams png_params = {
            0, 0,
            ini.get<cfg::video::png_interval>(),
            100u,
            ini.get<cfg::video::png_limit>(),
            true,
            this->client_info.remote_program,
            ini.get<cfg::video::rt_display>()
        };
        const bool capture_png = bool(capture_flags & CaptureFlags::png) && (png_params.png_limit > 0);

        DrawableParams const drawable_params{
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            nullptr
        };

        MetaParams const meta_params = meta_params_from_ini(ini);

        KbdLogParams const kbd_log_params = kbd_log_params_from_ini(ini);

        PatternParams const pattern_params = pattern_params_from_ini(ini);

        SequencedVideoParams const sequenced_video_params {};
        FullVideoParams const full_video_params {};

        WrmParams wrm_params = wrm_params_from_ini(
            this->capture_bpp,
            this->client_info.remote_program,
            this->cctx,
            this->gen,
            this->fstat,
            hash_path.c_str(),
            ini
        );

        CaptureParams capture_params{
            this->session_reactor.get_current_time(),
            basename,
            record_tmp_path,
            record_path.c_str(),
            groupid,
            &this->report_message,
            ini.get<cfg::video::smart_video_cropping>(),
            ini.get<cfg::debug::capture>()
        };

        this->capture = std::make_unique<Capture>(
            capture_params
          , drawable_params
          , capture_wrm, wrm_params
          , capture_png, png_params
          , capture_pattern_checker, pattern_params
          , capture_ocr, ocr_params
          , capture_video, sequenced_video_params
          , capture_video_full, full_video_params
          , capture_meta, meta_params
          , capture_kbd, kbd_log_params
          , video_params
          , nullptr
          , ((this->client_info.remote_program && (ini.get<cfg::video::smart_video_cropping>() != SmartVideoCropping::disable)) ?
              Rect(0, 0, 640, 480) : Rect())
        );

        if (this->nomouse) {
            this->capture->set_pointer_display();
        }
        if (this->capture->has_graphic_api()) {
            this->set_gd(this->capture.get());
            this->capture->add_graphic(this->orders.graphics_update_pdu());
        }

        this->capture_timer = this->session_reactor.create_timer()
        .set_delay(std::chrono::milliseconds(0))
        .on_action([this](auto ctx){
            auto const capture_ms = this->capture->periodic_snapshot(
                ctx.get_current_time(),
                this->mouse_x, this->mouse_y,
                false  // ignore frame in time interval
            ).ms();
            return (decltype(capture_ms)::max() == capture_ms)
                ? ctx.terminate()
                : ctx.ready_to(
                    std::chrono::duration_cast<std::chrono::milliseconds>(capture_ms));
        });

        if (this->client_info.remote_program && !this->rail_window_rect.isempty()) {
            this->capture->visibility_rects_event(this->rail_window_rect);
        }

        this->update_keyboard_input_mask_state();

        if (capture_wrm) {
            this->ini.set_acl<cfg::context::recording_started>(true);
        }

        if (capture_png && !this->ini.get<cfg::context::rt_ready>()) {
            this->ini.set_acl<cfg::context::rt_ready>(true);
        }

        return true;
    }

    bool must_be_stop_capture() override
    {
        if (this->capture) {
            LOG(LOG_INFO, "---<>  Front::must_be_stop_capture  <>---");
            this->capture.reset();
            this->capture_timer.reset();

            this->set_gd(this->orders.graphics_update_pdu());
            return true;
        }
        return false;
    }

    Capture::RTDisplayResult set_rt_display(bool enable_rt_display)
    {
        return this->capture
            ? this->capture->set_rt_display(enable_rt_display)
            : Capture::RTDisplayResult::Unchanged;
    }

    static int get_appropriate_compression_type(int client_supported_type, int front_supported_type)
    {
        if (((client_supported_type < PACKET_COMPR_TYPE_8K) || (client_supported_type > PACKET_COMPR_TYPE_RDP61)) ||
            ((front_supported_type  < PACKET_COMPR_TYPE_8K) || (front_supported_type  > PACKET_COMPR_TYPE_RDP61))) {
            return -1;
        }

        static int compress_type_selector[4][4] = {
            { PACKET_COMPR_TYPE_8K, PACKET_COMPR_TYPE_8K,  PACKET_COMPR_TYPE_8K,   PACKET_COMPR_TYPE_8K    },
            { PACKET_COMPR_TYPE_8K, PACKET_COMPR_TYPE_64K, PACKET_COMPR_TYPE_64K,  PACKET_COMPR_TYPE_64K   },
            { PACKET_COMPR_TYPE_8K, PACKET_COMPR_TYPE_64K, PACKET_COMPR_TYPE_RDP6, PACKET_COMPR_TYPE_RDP6  },
            { PACKET_COMPR_TYPE_8K, PACKET_COMPR_TYPE_64K, PACKET_COMPR_TYPE_RDP6, PACKET_COMPR_TYPE_RDP61 }
        };

        return compress_type_selector[client_supported_type][front_supported_type];
    }

    void save_persistent_disk_bitmap_cache() const
    {
        ::save_persistent_disk_bitmap_cache(
            this->orders.get_bmp_cache(),
            (app_path_s(AppPath::Persistent) + "/client").c_str(),
            this->ini.get<cfg::globals::host>().c_str(),
            this->orders.bpp(),
            report_error_from_reporter(this->report_message),
            convert_verbose_flags(this->verbose)
        );
    }

private:
    void reset()
    {
        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::reset: use_bitmap_comp=%d", this->ini.get<cfg::client::bitmap_compression>() ? 1 : 0);
            LOG(LOG_INFO, "Front::reset: use_compact_packets=%d", this->client_info.use_compact_packets);
            LOG(LOG_INFO, "Front::reset: bitmap_cache_version=%d", this->client_info.bitmap_cache_version);
        }

        this->max_data_block_size = MAX_DATA_BLOCK_SIZE;

        int const mppc_type = this->get_appropriate_compression_type(
            this->client_info.rdp_compression_type,
            static_cast<int>(this->ini.get<cfg::client::rdp_compression>()) - 1
        );
        this->mppc_enc = rdp_mppc_load_compressor(
            bool(this->verbose & Verbose::basic_trace), "Front::reset",
            mppc_type, this->ini.get<cfg::debug::compression>()
        );
        if (this->mppc_enc) {
            this->max_data_block_size = std::min<size_t>(MAX_DATA_BLOCK_SIZE,
                this->mppc_enc->get_max_data_block_size());
        }

        if (this->orders.has_bmp_cache_persister()) {
            this->save_persistent_disk_bitmap_cache();
        }

        this->orders.initialize(
            this->client_info.order_caps
          , this->client_info
          , this->trans
          , this->userid
          , this->share_id
          , this->encryptionLevel
          , this->encrypt
          , this->ini
          , this->max_data_block_size
          , this->server_fastpath_update_support
          , this->mppc_enc.get()
          , this->verbose
        );
        this->set_gd(this->orders.graphics_update_pdu());
    }

public:
    void begin_update() override {
        if (bool(this->verbose & Verbose::graphic)) {
            LOG(LOG_INFO, "Front::begin_update: level=%d", this->order_level);
        }
        this->order_level++;
    }

    void end_update() override {
        if (bool(this->verbose & Verbose::graphic)) {
            LOG(LOG_INFO, "Front::end_update: level=%d", this->order_level);
        }
        this->order_level--;
        if (!this->up_and_running) {
            LOG(LOG_ERR, "Front::end_update: Front is not up and running.");
            throw Error(ERR_RDP_EXPECTING_CONFIRMACTIVEPDU);
        }
        if (this->order_level == 0) {
            this->sync();
        }
    }

    void disconnect()
    {
        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::disconnect");
        }

        if (!this->is_client_disconnected) {
            write_packets(
                this->trans,
                [](StreamSize<256>, OutStream & mcs_data) {
                    MCS::DisconnectProviderUltimatum_Send(mcs_data, 3, MCS::PER_ENCODING);
                },
                X224::write_x224_dt_tpdu_fn{}
            );
        }
    }

    const CHANNELS::ChannelDefArray & get_channel_list() const override {
        return this->channel_list;
    }

    void send_to_channel( const CHANNELS::ChannelDef & channel
                                , uint8_t const * chunk
                                , size_t length
                                , size_t chunk_size
                                , int flags) override {
        if (bool(this->verbose & Verbose::channel)) {
            LOG( LOG_INFO
               , "Front::send_to_channel: (channel='%s'(%d), data=%p, length=%zu, chunk_size=%zu, flags=%x)"
               , channel.name, channel.chanid, voidp(chunk), length, chunk_size, unsigned(flags));
        }

        if ((channel.flags & GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL) &&
            (channel.chanid != this->rail_channel_id)) {
            flags |= CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
        }

        CHANNELS::VirtualChannelPDU virtual_channel_pdu(bool(this->verbose & Verbose::channel));

        virtual_channel_pdu.send_to_client( this->trans, this->encrypt
                                          , this->encryptionLevel, userid, channel.chanid
                                          , length, flags, chunk, chunk_size);
    }

    TpduBuffer buf;
    size_t channel_list_index = 0;

    void incoming(Callback & cb, time_t /*now*/) /*NOLINT*/
    {
        if (bool(this->verbose & Verbose::basic_trace3)) {
            LOG(LOG_INFO, "Front::incoming");
        }

        buf.load_data(this->trans);
        while (buf.next_pdu())
        {
            InStream new_x224_stream(buf.current_pdu_buffer());
            cb.set_last_tram_len(new_x224_stream.in_remain());

            switch (this->state) {
            case CONNECTION_INITIATION:
            {
                // Connection Initiation
                // ---------------------

                // The client initiates the connection by sending the server an X.224 Connection
                //  Request PDU (class 0). The server responds with an X.224 Connection Confirm
                // PDU (class 0). From this point, all subsequent data sent between client and
                // server is wrapped in an X.224 Data Protocol Data Unit (PDU).

                // Client                                                     Server
                //    |------------X224 Connection Request PDU----------------> |
                //    | <----------X224 Connection Confirm PDU----------------- |

                if (bool(this->verbose & Verbose::basic_trace)) {
                    LOG(LOG_INFO, "Front::incoming: CONNECTION_INITIATION");
                    LOG(LOG_INFO, "Front::incoming: receiving x224 request PDU");
                }

                {
                    X224::CR_TPDU_Recv x224(new_x224_stream, this->ini.get<cfg::client::bogus_neg_request>());
                    if (x224._header_size != new_x224_stream.get_capacity()) {
                        LOG(LOG_WARNING, "Front::incoming: connection request : all data should have been consumed,"
                                     " %zu bytes remains", new_x224_stream.get_capacity() - x224._header_size);
                    }
                    this->clientRequestedProtocols = x224.rdp_neg_requestedProtocols;
                }

                if (!this->ini.get<cfg::client::tls_support>() && !this->ini.get<cfg::client::tls_fallback_legacy>()) {
                    LOG(LOG_WARNING, "Front::incoming: tls_support and tls_fallback_legacy should not be disabled at same time. tls_support is assumed to be enabled.");
                }

                if (// Proxy doesnt supports TLS or RDP client doesn't support TLS
                    (!this->ini.get<cfg::client::tls_support>() || 0 == (this->clientRequestedProtocols & X224::PROTOCOL_TLS))
                    // Fallback to legacy security protocol (RDP) is allowed.
                    && this->ini.get<cfg::client::tls_fallback_legacy>()
                ) {
                    LOG(LOG_INFO, "Front::incoming: Fallback to legacy security protocol");
                    this->tls_client_active = false;
                }
                else if ((0 == (this->clientRequestedProtocols & X224::PROTOCOL_TLS)) &&
                            !this->ini.get<cfg::client::tls_fallback_legacy>()) {
                    LOG(LOG_WARNING, "Front::incoming: TLS security protocol is not supported by client. Allow falling back to legacy security protocol is probably necessary");
                }

                if (bool(this->verbose & Verbose::basic_trace)) {
                    LOG(LOG_INFO, "Front::incoming: sending x224 connection confirm PDU");
                }

                {
                    uint8_t rdp_neg_type = 0;
                    uint8_t rdp_neg_flags = /*0*/RdpNego::EXTENDED_CLIENT_DATA_SUPPORTED;
                    uint32_t rdp_neg_code = 0;
                    if (this->tls_client_active) {
                        LOG(LOG_INFO, "-----------------> Front::incoming: TLS Support Enabled");
                        if (this->clientRequestedProtocols & X224::PROTOCOL_TLS) {
                            rdp_neg_type = X224::RDP_NEG_RSP;
                            rdp_neg_code = X224::PROTOCOL_TLS;
                            this->encryptionLevel = 0;
                        }
                        else {
                            rdp_neg_type = X224::RDP_NEG_FAILURE;
                            rdp_neg_code = X224::SSL_REQUIRED_BY_SERVER;
                        }
                    }
                    else {
                        LOG(LOG_INFO, "-----------------> Front::incoming: TLS Support not Enabled");
                    }

                    StaticOutStream<256> stream;
                    X224::CC_TPDU_Send x224(stream, rdp_neg_type, rdp_neg_flags, rdp_neg_code);
                    this->trans.send(stream.get_bytes());
                }

                if (this->tls_client_active) {
                    this->trans.enable_server_tls(
                        this->ini.get<cfg::globals::certificate_password>(),
                        this->ini.get<cfg::client::ssl_cipher_list>().c_str());
                }

                this->state = BASIC_SETTINGS_EXCHANGE;

                // 2.2.10.2 Early User Authorization Result PDU
                // ============================================

                // The Early User Authorization Result PDU is sent from server to client and is used
                // to convey authorization information to the client. This PDU is only sent by the server
                // if the client advertised support for it by specifying the PROTOCOL_HYBRID_EX (0x00000008)
                // flag in the requestedProtocols field of the RDP Negotiation Request (section 2.2.1.1.1)
                // structure and it MUST be sent immediately after the CredSSP handshake (section 5.4.5.2) has completed.

                // authorizationResult (4 bytes): A 32-bit unsigned integer. Specifies the authorization result.

                // +---------------------------------+--------------------------------------------------------+
                // | AUTHZ_SUCCESS 0x00000000        | The user has permission to access the server.          |
                // +---------------------------------+--------------------------------------------------------+
                // | AUTHZ_ACCESS_DENIED 0x0000052E | The user does not have permission to access the server.|
                // +---------------------------------+--------------------------------------------------------+
            }
            break;
            case BASIC_SETTINGS_EXCHANGE:
            {
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

                if (bool(this->verbose & Verbose::basic_trace)) {
                    LOG(LOG_INFO, "Front::incoming: Basic Settings Exchange");
                }

                X224::DT_TPDU_Recv x224(new_x224_stream);
                MCS::CONNECT_INITIAL_PDU_Recv mcs_ci(x224.payload, MCS::BER_ENCODING);

                // GCC User Data
                // -------------
                GCC::Create_Request_Recv gcc_cr(mcs_ci.payload);
                // TODO ensure gcc_data substream is fully consumed

                while (gcc_cr.payload.in_check_rem(4)) {
                    GCC::UserData::RecvFactory f(gcc_cr.payload);
                    switch (f.tag) {
                        case CS_CORE:
                        {
                            GCC::UserData::CSCore cs_core;
                            cs_core.recv(f.payload);
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                cs_core.log("Front::incoming: Received from Client");
                            }

                            this->client_info.screen_info.width     = cs_core.desktopWidth;
                            this->client_info.screen_info.height    = cs_core.desktopHeight;
                            this->client_info.keylayout = cs_core.keyboardLayout;
                            this->client_info.build     = cs_core.clientBuild;
                            for (size_t i = 0; i < 15 ; i++) {
                                this->client_info.hostname[i] = cs_core.clientName[i];
                            }
                            this->client_info.hostname[15] = 0;
                            //LOG(LOG_INFO, "hostname=\"%s\"", this->client_info.hostname);
                            this->client_info.screen_info.bpp = BitsPerPixel{8};
                            switch (cs_core.postBeta2ColorDepth) {
                                // TODO use a enum
                            case 0xca01:
                                /*
                                this->client_info.bpp =
                                    (cs_core.highColorDepth <= 24)?cs_core.highColorDepth:24;
                                */
                                this->client_info.screen_info.bpp = (
                                          (cs_core.earlyCapabilityFlags & GCC::UserData::RNS_UD_CS_WANT_32BPP_SESSION)
                                        ? BitsPerPixel{32}
                                        : BitsPerPixel{checked_int(cs_core.highColorDepth)}
                                    );
                            break;
                            case 0xca02:
                                this->client_info.screen_info.bpp = BitsPerPixel{15};
                            break;
                            case 0xca03:
                                this->client_info.screen_info.bpp = BitsPerPixel{16};
                            break;
                            case 0xca04:
                                this->client_info.screen_info.bpp = BitsPerPixel{24};
                            break;
                            default:
                            break;
                            }
                            if (bool(this->ini.get<cfg::client::max_color_depth>())) {
                                this->client_info.screen_info.bpp = std::min(
                                    this->client_info.screen_info.bpp,
                                    BitsPerPixel{checked_int(this->ini.get<cfg::client::max_color_depth>())});
                            }
                            this->client_support_monitor_layout_pdu =
                                (cs_core.earlyCapabilityFlags &
                                 GCC::UserData::RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU);
                        }
                        break;
                        case CS_SECURITY:
                        {
                            GCC::UserData::CSSecurity cs_sec;
                            cs_sec.recv(f.payload);
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                cs_sec.log("Front::incoming: Received from Client");
                            }
                        }
                        break;
                        case CS_NET:
                        {
                            GCC::UserData::CSNet cs_net;
                            cs_net.recv(f.payload);
                            for (uint32_t index = 0; index < cs_net.channelCount; index++) {
                                const auto & channel_def = cs_net.channelDefArray[index];
                                CHANNELS::ChannelDef channel_item;
                                channel_item.name = CHANNELS::ChannelNameId(channel_def.name);
                                channel_item.flags = channel_def.options;
                                channel_item.chanid = GCC::MCS_GLOBAL_CHANNEL + (index + 1);
                                this->channel_list.push_back(channel_item);

                                if (!this->rail_channel_id &&
                                    channel_item.name == channel_names::rail) {
                                    this->rail_channel_id = channel_item.chanid;
                                }
                            }
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                cs_net.log("Front::incoming: Received from Client");
                            }
                        }
                        break;
                        case CS_CLUSTER:
                        {
                            GCC::UserData::CSCluster cs_cluster;
                            cs_cluster.recv(f.payload);
                            this->client_info.console_session =
                                (0 != (cs_cluster.flags & GCC::UserData::CSCluster::REDIRECTED_SESSIONID_FIELD_VALID))
                                && (0 == cs_cluster.redirectedSessionID);
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                cs_cluster.log("Front::incoming: Receiving from Client");
                            }
                        }
                        break;
                        case CS_MONITOR:
                        {
                            GCC::UserData::CSMonitor & cs_monitor =
                                this->client_info.cs_monitor;
                            cs_monitor.recv(f.payload);
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                cs_monitor.log("Front::incoming: Receiving from Client");
                            }

                            Rect client_monitors_rect = this->client_info.cs_monitor.get_rect();
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                LOG(LOG_INFO, "Front::incoming: MonitorsRect=(%d, %d, %d, %d)",
                                    client_monitors_rect.x, client_monitors_rect.y,
                                    client_monitors_rect.cx, client_monitors_rect.cy);
                            }

                            if (this->ini.get<cfg::globals::allow_using_multiple_monitors>()) {
                                this->client_info.screen_info.width  = client_monitors_rect.cx + 1;
                                this->client_info.screen_info.height = client_monitors_rect.cy + 1;
                            }
                        }
                        break;
                        case CS_MCS_MSGCHANNEL:
                        {
                            GCC::UserData::CSMCSMsgChannel cs_mcs_msgchannel;
                            cs_mcs_msgchannel.recv(f.payload);
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                cs_mcs_msgchannel.log("Front::incoming: Receiving from Client");
                            }
                        }
                        break;
                        case CS_MULTITRANSPORT:
                        {
                            GCC::UserData::CSMultiTransport cs_multitransport;
                            cs_multitransport.recv(f.payload);
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                cs_multitransport.log("Front::incoming: Receiving from Client");
                            }
                        }
                        break;
                        default:
                            LOG(LOG_WARNING, "Front::incoming: Unexpected data block tag %x\n", f.tag);
                        break;
                    }
                }
                if (gcc_cr.payload.in_check_rem(1)) {
                    LOG(LOG_ERR, "Front::incoming: recv connect request parsing gcc data : short header");
                    throw Error(ERR_MCS_DATA_SHORT_HEADER);
                }

                write_packets(
                    this->trans,
                    [this](StreamSize<65536-1024>, OutStream & stream) {
                        {
                            GCC::UserData::SCCore sc_core;
                            sc_core.version = 0x00080004;
                            if (this->tls_client_active) {
                                sc_core.length = 12;
                                sc_core.clientRequestedProtocols = this->clientRequestedProtocols;
                            }
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                sc_core.log("Front::incoming: Sending to client");
                            }
                            sc_core.emit(stream);
                        }
                        // ------------------------------------------------------------------
                        {
                            GCC::UserData::SCNet sc_net;
                            sc_net.MCSChannelId = GCC::MCS_GLOBAL_CHANNEL;
                            sc_net.channelCount = this->channel_list.size();
                            for (size_t index = 0; index < this->channel_list.size(); ++index) {
                                sc_net.channelDefArray[index].id = this->channel_list[index].chanid;
                            }
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                sc_net.log("Front::incoming: Sending to client");
                            }
                            sc_net.emit(stream);
                        }
                        // ------------------------------------------------------------------
                        if (this->tls_client_active) {
                            GCC::UserData::SCSecurity sc_sec1;
                            sc_sec1.encryptionMethod = 0;
                            sc_sec1.encryptionLevel = 0;
                            sc_sec1.length = 12;
                            sc_sec1.serverRandomLen = 0;
                            sc_sec1.serverCertLen = 0;
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                sc_sec1.log("Front::incoming: Sending to client");
                            }
                            sc_sec1.emit(stream);
                        }
                        else {
                            GCC::UserData::SCSecurity sc_sec1;
                            /*
                            For now rsa_keys are not in a configuration file any more, but as we were not changing keys
                            the values have been embedded in code and the key generator file removed from source code.

                            It will be put back at some later time using a clean parser/writer module and sll calls
                            coherent with the remaining of ReDemPtion code. For reference to historical key generator
                            code look for utils/keygen.cpp in old repository code.

                            references for RSA Keys: http://www.securiteam.com/windowsntfocus/5EP010KG0G.html
                            */
                            uint8_t rsa_keys_pub_mod[64] = {
                                0x67, 0xab, 0x0e, 0x6a, 0x9f, 0xd6, 0x2b, 0xa3,
                                0x32, 0x2f, 0x41, 0xd1, 0xce, 0xee, 0x61, 0xc3,
                                0x76, 0x0b, 0x26, 0x11, 0x70, 0x48, 0x8a, 0x8d,
                                0x23, 0x81, 0x95, 0xa0, 0x39, 0xf7, 0x5b, 0xaa,
                                0x3e, 0xf1, 0xed, 0xb8, 0xc4, 0xee, 0xce, 0x5f,
                                0x6a, 0xf5, 0x43, 0xce, 0x5f, 0x60, 0xca, 0x6c,
                                0x06, 0x75, 0xae, 0xc0, 0xd6, 0xa4, 0x0c, 0x92,
                                0xa4, 0xc6, 0x75, 0xea, 0x64, 0xb2, 0x50, 0x5b
                            };
                            memcpy(this->pub_mod, rsa_keys_pub_mod, 64);

                            uint8_t rsa_keys_pri_exp[64] = {
                                0x41, 0x93, 0x05, 0xB1, 0xF4, 0x38, 0xFC, 0x47,
                                0x88, 0xC4, 0x7F, 0x83, 0x8C, 0xEC, 0x90, 0xDA,
                                0x0C, 0x8A, 0xB5, 0xAE, 0x61, 0x32, 0x72, 0xF5,
                                0x2B, 0xD1, 0x7B, 0x5F, 0x44, 0xC0, 0x7C, 0xBD,
                                0x8A, 0x35, 0xFA, 0xAE, 0x30, 0xF6, 0xC4, 0x6B,
                                0x55, 0xA7, 0x65, 0xEF, 0xF4, 0xB2, 0xAB, 0x18,
                                0x4E, 0xAA, 0xE6, 0xDC, 0x71, 0x17, 0x3B, 0x4C,
                                0xC2, 0x15, 0x4C, 0xF7, 0x81, 0xBB, 0xF0, 0x03
                            };
                            memcpy(sc_sec1.pri_exp, rsa_keys_pri_exp, 64);
                            memcpy(this->pri_exp, sc_sec1.pri_exp, 64);

                            uint8_t rsa_keys_pub_sig[64] = {
                                0x6a, 0x41, 0xb1, 0x43, 0xcf, 0x47, 0x6f, 0xf1,
                                0xe6, 0xcc, 0xa1, 0x72, 0x97, 0xd9, 0xe1, 0x85,
                                0x15, 0xb3, 0xc2, 0x39, 0xa0, 0xa6, 0x26, 0x1a,
                                0xb6, 0x49, 0x01, 0xfa, 0xa6, 0xda, 0x60, 0xd7,
                                0x45, 0xf7, 0x2c, 0xee, 0xe4, 0x8e, 0x64, 0x2e,
                                0x37, 0x49, 0xf0, 0x4c, 0x94, 0x6f, 0x08, 0xf5,
                                0x63, 0x4c, 0x56, 0x29, 0x55, 0x5a, 0x63, 0x41,
                                0x2c, 0x20, 0x65, 0x95, 0x99, 0xb1, 0x15, 0x7c
                            };

                            uint8_t rsa_keys_pub_exp[4] = { 0x01, 0x00, 0x01, 0x00 };

                            sc_sec1.encryptionMethod = this->encrypt.encryptionMethod;
                            sc_sec1.encryptionLevel = this->encryptionLevel;
                            sc_sec1.serverRandomLen = 32;
                            this->gen.random(this->server_random, 32);
                            memcpy(sc_sec1.serverRandom, this->server_random, 32);
                            sc_sec1.dwVersion = GCC::UserData::SCSecurity::CERT_CHAIN_VERSION_1;
                            sc_sec1.temporary = false;
                            memcpy(sc_sec1.proprietaryCertificate.RSAPK.pubExp, rsa_keys_pub_exp, SEC_EXPONENT_SIZE);
                            memcpy(sc_sec1.proprietaryCertificate.RSAPK.modulus, this->pub_mod, 64);
                            memcpy(sc_sec1.proprietaryCertificate.RSAPK.modulus + 64,
                                "\x00\x00\x00\x00\x00\x00\x00\x00", SEC_PADDING_SIZE);
                            memcpy(sc_sec1.proprietaryCertificate.wSignatureBlob, rsa_keys_pub_sig, 64);
                            memcpy(sc_sec1.proprietaryCertificate.wSignatureBlob + 64,
                                "\x00\x00\x00\x00\x00\x00\x00\x00", SEC_PADDING_SIZE);

                            if (bool(this->verbose & Verbose::basic_trace)) {
                                sc_sec1.log("Front::incoming: Sending to client");
                            }
                            sc_sec1.emit(stream);
                        }
                    },
                    [](StreamSize<256>, OutStream & gcc_header, std::size_t packed_size) {
                        GCC::Create_Response_Send(gcc_header, packed_size);
                    },
                    [](StreamSize<256>, OutStream & mcs_header, std::size_t packed_size) {
                        MCS::CONNECT_RESPONSE_Send mcs_cr(mcs_header, packed_size, MCS::BER_ENCODING);
                    },
                    X224::write_x224_dt_tpdu_fn{}
                );
            }
            this->state = CHANNEL_ATTACH_USER;
            break;
            case CHANNEL_ATTACH_USER:
            {
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

                if (bool(this->verbose & Verbose::channel)) {
                    LOG(LOG_INFO, "Front::incoming: Channel Connection");
                }

                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::incoming: Recv MCS::ErectDomainRequest");
                }
                {
                    assert(buf.current_pdu_get_type() == X224::DT_TPDU);
                    X224::DT_TPDU_Recv x224(new_x224_stream);
                    MCS::ErectDomainRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);
                }
            }
            this->state = CHANNEL_JOIN_REQUEST;
            break;
            case CHANNEL_JOIN_REQUEST:
            {
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::incoming: Recv MCS::AttachUserRequest");
                }
                {
                    X224::DT_TPDU_Recv x224(new_x224_stream);
                    MCS::AttachUserRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);
                }

                if (this->ini.get<cfg::client::bogus_user_id>()) {
                    // To avoid bug in freerdp 0.7.x and Remmina 0.8.x that causes client disconnection
                    //  when unexpected channel id is received.
                    this->userid = 32;
                }

                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::incoming: Send MCS::AttachUserConfirm userid=%u", this->userid);
                }

                write_packets(
                    this->trans,
                    [this](StreamSize<256>, OutStream & mcs_data) {
                        MCS::AttachUserConfirm_Send(mcs_data, MCS::RT_SUCCESSFUL, true, this->userid, MCS::PER_ENCODING);
                    },
                    X224::write_x224_dt_tpdu_fn{}
                );
                this->state = CHANNEL_JOIN_CONFIRM_USER_ID;
                break;
            }
            case CHANNEL_JOIN_CONFIRM_USER_ID:
            {
                this->channel_join_request_transmission(new_x224_stream, [this](MCS::ChannelJoinRequest_Recv &mcs) {
                    this->userid = mcs.initiator;
                });
                this->state = CHANNEL_JOIN_CONFIRM_CHECK_USER_ID;
                break;
            }
            case CHANNEL_JOIN_CONFIRM_CHECK_USER_ID:
            {
                this->channel_join_request_transmission(new_x224_stream, [this](MCS::ChannelJoinRequest_Recv & mcs) {
                    if (mcs.initiator != this->userid) {
                        LOG(LOG_ERR, "Front::incoming: MCS error bad userid, expecting %u got %u", this->userid, mcs.initiator);
                        throw Error(ERR_MCS_BAD_USERID);
                    }
                });
                this->state = CHANNEL_JOIN_CONFIRM_LOOP;
                this->channel_list_index = 0;
                break;
            }
            case CHANNEL_JOIN_CONFIRM_LOOP:
            {
                if (this->channel_list_index < this->channel_list.size()) {
                    this->channel_join_request_transmission(new_x224_stream, [this](MCS::ChannelJoinRequest_Recv & mcs) {
                        if (bool(this->verbose & Verbose::channel)) {
                            LOG(LOG_INFO, "Front::incoming: cjrq[%zu] = %" PRIu16 " -> cjcf", this->channel_list_index, mcs.channelId);
                        }

                        if (mcs.initiator != this->userid) {
                            LOG(LOG_ERR, "Front::incoming: MCS error bad userid, expecting %" PRIu16 " got %" PRIu16,
                                this->userid, mcs.initiator);
                            throw Error(ERR_MCS_BAD_USERID);
                        }

                        this->channel_list.set_chanid(this->channel_list_index, mcs.channelId);
                    });
                    ++this->channel_list_index;
                    break;
                }

                if (bool(this->verbose & Verbose::basic_trace)) {
                    LOG(LOG_INFO, "Front::incoming: RDP Security Commencement");
                }

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
                if (!this->tls_client_active) {
                    LOG(LOG_INFO, "Front::incoming: Legacy RDP mode: expecting exchange packet");
                    X224::DT_TPDU_Recv x224(new_x224_stream);

                    int mcs_type = MCS::peekPerEncodedMCSType(x224.payload);
                    if (mcs_type == MCS::MCSPDU_DisconnectProviderUltimatum) {
                        LOG(LOG_INFO, "Front::incoming: DisconnectProviderUltimatum received");
                        MCS::DisconnectProviderUltimatum_Recv mcs(x224.payload, MCS::PER_ENCODING);
                        const char * reason = MCS::get_reason(mcs.reason);
                        LOG(LOG_INFO, "Front::incoming: DisconnectProviderUltimatum: reason=%s [%u]", reason, mcs.reason);
                        this->is_client_disconnected = true;
                        throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
                    }

                    MCS::SendDataRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);
                    SEC::SecExchangePacket_Recv sec(mcs.payload);

                    uint8_t client_random[64] = {};
                    {
                        ssllib ssl;
                        ssl.ssl_xxxxxx(client_random, 64, sec.payload.get_data(), 64, this->pub_mod, 64, this->pri_exp);
                    }
                    // beware order of parameters for key generation (decrypt/encrypt)
                    // is inversed between server and client
                    SEC::KeyBlock key_block(client_random, this->server_random);
                    memcpy(this->encrypt.sign_key, key_block.blob0, 16);
                    if (this->encrypt.encryptionMethod == 1) {
                        ssllib ssl;
                        ssl.sec_make_40bit(this->encrypt.sign_key);
                    }
                    this->encrypt.generate_key(key_block.key1, this->encrypt.encryptionMethod);
                    this->decrypt.generate_key(key_block.key2, this->encrypt.encryptionMethod);
                    this->state = WAITING_FOR_LOGON_INFO;
                    break;
                }

                LOG(LOG_INFO, "Front::incoming: TLS mode: exchange packet disabled");
                this->state = WAITING_FOR_LOGON_INFO;
                REDEMPTION_CXX_FALLTHROUGH;
            }
            case WAITING_FOR_LOGON_INFO:
            // Secure Settings Exchange
            // ------------------------

            // Secure Settings Exchange: Secure client data (such as the username,
            // password and auto-reconnect cookie) is sent to the server using the Client
            // Info PDU.

            // Client                                                     Server
            //    |------ Client Info PDU      ---------------------------> |
            {
                LOG(LOG_INFO, "Front::incoming: Secure Settings Exchange");

                X224::DT_TPDU_Recv x224(new_x224_stream);

                int mcs_type = MCS::peekPerEncodedMCSType(x224.payload);
                if (mcs_type == MCS::MCSPDU_DisconnectProviderUltimatum) {
                    LOG(LOG_INFO, "Front::incoming: DisconnectProviderUltimatum received");
                    MCS::DisconnectProviderUltimatum_Recv mcs(x224.payload, MCS::PER_ENCODING);
                    const char * reason = MCS::get_reason(mcs.reason);
                    LOG(LOG_INFO, "Front::incoming: DisconnectProviderUltimatum: reason=%s [%u]", reason, mcs.reason);
                    this->is_client_disconnected = true;
                    throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
                }

                MCS::SendDataRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);
                SEC::SecSpecialPacket_Recv sec(mcs.payload, this->decrypt, this->encryptionLevel);
                if (bool(this->verbose & Verbose::sec_decrypted)) {
                    LOG(LOG_INFO, "Front::incoming: sec decrypted payload:");
                    hexdump_d(sec.payload.get_data(), sec.payload.get_capacity());
                }

                if (!(sec.flags & SEC::SEC_INFO_PKT)) {
                    throw Error(ERR_SEC_EXPECTED_LOGON_INFO);
                }

                /* this is the first test that the decrypt is working */
                this->client_info.process_logon_info(
                    sec.payload
                  , ini.get<cfg::client::ignore_logon_password>()
                  , ini.get<cfg::client::performance_flags_default>()
                  , ini.get<cfg::client::performance_flags_force_present>()
                  , ini.get<cfg::client::performance_flags_force_not_present>()
                  , ini.get<cfg::debug::password>()
                  , bool(this->verbose & Verbose::basic_trace)
                );

                if (sec.payload.in_remain()) {
                    LOG(LOG_ERR, "Front::incoming: process_logon all data should have been consumed %zu bytes trailing",
                        sec.payload.in_remain());
                }

                this->keymap.init_layout(this->client_info.keylayout);
                LOG(LOG_INFO, "Front::incoming: Keyboard Layout = 0x%x", unsigned(this->client_info.keylayout));
                this->ini.set_acl<cfg::client::keyboard_layout>(this->client_info.keylayout);
                if (this->client_info.is_mce) {
                    if (bool(this->verbose & Verbose::basic_trace2)) {
                        LOG(LOG_INFO, "Front::incoming: licencing client_info.is_mce");
                        LOG(LOG_INFO, "Front::incoming: licencing send_media_lic_response");
                    }

                    this->send_data_indication(
                        GCC::MCS_GLOBAL_CHANNEL,
                        [this](StreamSize<24>, OutStream & sec_header) {
                            /* mce */
                            /* some compilers need unsigned char to avoid warnings */
                            uint8_t lic3[] = {
                                0xff, 0x03, 0x10, 0x00,
                                0x07, 0x00, 0x00, 0x00,
                                0x02, 0x00, 0x00, 0x00,
                                0xf3, 0x99, 0x00, 0x00
                            };
                            static_assert(sizeof(lic3) == 16);

                            SEC::Sec_Send sec(
                                sec_header, lic3, sizeof(lic3),
                                SEC::SEC_LICENSE_PKT | 0x00100200, this->encrypt, 0
                            );
                            (void)sec;

                            sec_header.out_copy_bytes(lic3, sizeof(lic3));

                            if (bool(this->verbose & Verbose::global_channel)) {
                                LOG(LOG_INFO, "Front::incoming: Sec clear payload to send:");
                                hexdump_d(lic3, sizeof(lic3));
                            }
                        }
                    );

                    // proceed with capabilities exchange

                    // Capabilities Exchange
                    // ---------------------

                    // Capabilities Negotiation: The server sends the set of capabilities it
                    // supports to the client in a Demand Active PDU. The client responds with its
                    // capabilities by sending a Confirm Active PDU.

                    // Client                                                     Server
                    //    | <------- Demand Active PDU ---------------------------- |
                    //    |--------- Confirm Active PDU --------------------------> |

                    if (bool(this->verbose & Verbose::basic_trace)) {
                        LOG(LOG_INFO, "Front::incoming: send_demand_active");
                    }
                    this->send_demand_active();
                    this->send_monitor_layout();

                    LOG(LOG_INFO, "Front::incoming: ACTIVATED (mce)");
                    this->state = ACTIVATE_AND_PROCESS_DATA;
                }
                else {
                    if (bool(this->verbose & Verbose::channel)) {
                        LOG(LOG_INFO, "Front::incoming: licencing not client_info.is_mce");
                        LOG(LOG_INFO, "Front::incoming: licencing send_lic_initial");
                    }

                    this->send_data_indication(
                        GCC::MCS_GLOBAL_CHANNEL,
                        [this](StreamSize<314+8+4>, OutStream & sec_header) {
                            /* some compilers need unsigned char to avoid warnings */
                            static const uint8_t lic1[] = {
                                // SEC_RANDOM ?
                                0x7b, 0x3c, 0x31, 0xa6, 0xae, 0xe8, 0x74, 0xf6,
                                0xb4, 0xa5, 0x03, 0x90, 0xe7, 0xc2, 0xc7, 0x39,
                                0xba, 0x53, 0x1c, 0x30, 0x54, 0x6e, 0x90, 0x05,
                                0xd0, 0x05, 0xce, 0x44, 0x18, 0x91, 0x83, 0x81,
                                //
                                0x00, 0x00, 0x04, 0x00, 0x2c, 0x00, 0x00, 0x00,
                                0x4d, 0x00, 0x69, 0x00, 0x63, 0x00, 0x72, 0x00,
                                0x6f, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x66, 0x00,
                                0x74, 0x00, 0x20, 0x00, 0x43, 0x00, 0x6f, 0x00,
                                0x72, 0x00, 0x70, 0x00, 0x6f, 0x00, 0x72, 0x00,
                                0x61, 0x00, 0x74, 0x00, 0x69, 0x00, 0x6f, 0x00,
                                0x6e, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
                                0x32, 0x00, 0x33, 0x00, 0x36, 0x00, 0x00, 0x00,
                                0x0d, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00,
                                0x03, 0x00, 0xb8, 0x00, 0x01, 0x00, 0x00, 0x00,
                                0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                                0x06, 0x00, 0x5c, 0x00, 0x52, 0x53, 0x41, 0x31,
                                0x48, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
                                0x3f, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
                                0x01, 0xc7, 0xc9, 0xf7, 0x8e, 0x5a, 0x38, 0xe4,
                                0x29, 0xc3, 0x00, 0x95, 0x2d, 0xdd, 0x4c, 0x3e,
                                0x50, 0x45, 0x0b, 0x0d, 0x9e, 0x2a, 0x5d, 0x18,
                                0x63, 0x64, 0xc4, 0x2c, 0xf7, 0x8f, 0x29, 0xd5,
                                0x3f, 0xc5, 0x35, 0x22, 0x34, 0xff, 0xad, 0x3a,
                                0xe6, 0xe3, 0x95, 0x06, 0xae, 0x55, 0x82, 0xe3,
                                0xc8, 0xc7, 0xb4, 0xa8, 0x47, 0xc8, 0x50, 0x71,
                                0x74, 0x29, 0x53, 0x89, 0x6d, 0x9c, 0xed, 0x70,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x08, 0x00, 0x48, 0x00, 0xa8, 0xf4, 0x31, 0xb9,
                                0xab, 0x4b, 0xe6, 0xb4, 0xf4, 0x39, 0x89, 0xd6,
                                0xb1, 0xda, 0xf6, 0x1e, 0xec, 0xb1, 0xf0, 0x54,
                                0x3b, 0x5e, 0x3e, 0x6a, 0x71, 0xb4, 0xf7, 0x75,
                                0xc8, 0x16, 0x2f, 0x24, 0x00, 0xde, 0xe9, 0x82,
                                0x99, 0x5f, 0x33, 0x0b, 0xa9, 0xa6, 0x94, 0xaf,
                                0xcb, 0x11, 0xc3, 0xf2, 0xdb, 0x09, 0x42, 0x68,
                                0x29, 0x56, 0x58, 0x01, 0x56, 0xdb, 0x59, 0x03,
                                0x69, 0xdb, 0x7d, 0x37, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                                0x0e, 0x00, 0x0e, 0x00, 0x6d, 0x69, 0x63, 0x72,
                                0x6f, 0x73, 0x6f, 0x66, 0x74, 0x2e, 0x63, 0x6f,
                                0x6d, 0x00
                            };
                            static_assert(sizeof(lic1) == 314);

                            OutReservedStreamHelper hstream(sec_header.get_data(), 8, sec_header.get_capacity());
                            OutStream & stream = hstream.get_data_stream();

                            stream.out_uint8(LIC::LICENSE_REQUEST);
                            stream.out_uint8(2); // preamble flags : PREAMBLE_VERSION_2_0 (RDP 4.0)
                            stream.out_uint16_le(318); // wMsgSize = 318 including preamble

                            stream.out_copy_bytes(lic1, sizeof(lic1));

                            if (bool(this->verbose & Verbose::global_channel)) {
                                LOG(LOG_INFO, "Front::incoming: Sec clear payload to send:");
                                hexdump_av_d(stream.get_bytes());
                            }

                            StaticOutStream<8> tmp_sec_header;
                            SEC::Sec_Send sec(
                                tmp_sec_header, stream.get_data(), stream.get_offset(),
                                SEC::SEC_LICENSE_PKT, this->encrypt, 0
                            );
                            (void)sec;

                            auto packet = hstream.copy_to_head(tmp_sec_header);
                            sec_header = OutStream(packet.data(), packet.size(), packet.size());
                        }
                    );

                    if (bool(this->verbose & Verbose::basic_trace2)) {
                        LOG(LOG_INFO, "Front::incoming: Waiting for answer to lic_initial");
                    }
                    this->state = WAITING_FOR_ANSWER_TO_LICENCE;
                }
            }
            break;

            case WAITING_FOR_ANSWER_TO_LICENCE:
            {
                if (bool(this->verbose & Verbose::basic_trace2)) {
                    LOG(LOG_INFO, "Front::incoming: WAITING_FOR_ANSWER_TO_LICENCE");
                }
                X224::DT_TPDU_Recv x224(new_x224_stream);

                int mcs_type = MCS::peekPerEncodedMCSType(x224.payload);

                if (mcs_type == MCS::MCSPDU_DisconnectProviderUltimatum) {
                    MCS::DisconnectProviderUltimatum_Recv mcs(x224.payload, MCS::PER_ENCODING);
                    const char * reason = MCS::get_reason(mcs.reason);
                    LOG(LOG_INFO, "Front::incoming: DisconnectProviderUltimatum: reason=%s [%u]", reason, mcs.reason);
                    this->is_client_disconnected = true;
                    throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
                }

                MCS::SendDataRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);
                SEC::SecSpecialPacket_Recv sec(mcs.payload, this->decrypt, this->encryptionLevel);
                if (bool(this->verbose & Verbose::global_channel)) {
                    LOG(LOG_INFO, "Front::incoming: sec decrypted payload:");
                    hexdump_d(sec.payload.get_data(), sec.payload.get_capacity());
                }

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

                if (sec.flags & SEC::SEC_LICENSE_PKT) {
                    LIC::RecvFactory flic(sec.payload);
                    switch (flic.tag) {
                    case LIC::ERROR_ALERT:
                    {
                        if (bool(this->verbose & Verbose::basic_trace2)) {
                            LOG(LOG_INFO, "Front::incoming: LIC::ERROR_ALERT");
                        }
                        // TODO We should check what is actually returned by this message, as it may be an error
                        LIC::ErrorAlert_Recv lic(sec.payload);
                        LOG(LOG_ERR, "Front::incoming: License Alert: error=%u transition=%u",
                            lic.validClientMessage.dwErrorCode, lic.validClientMessage.dwStateTransition);

                    }
                    break;
                    case LIC::NEW_LICENSE_REQUEST:
                    {
                        if (bool(this->verbose & Verbose::basic_trace2)) {
                            LOG(LOG_INFO, "Front::incoming: LIC::NEW_LICENSE_REQUEST");
                        }
                        LIC::NewLicenseRequest_Recv lic(sec.payload);
                        // TODO Instead of returning a license we return a message saying that no license is OK
                        this->send_valid_client_license_data();
                    }
                    break;
                    case LIC::PLATFORM_CHALLENGE_RESPONSE:
                        // TODO As we never send a platform challenge, it is unlikely we ever receive a PLATFORM_CHALLENGE_RESPONSE
                        if (bool(this->verbose & Verbose::basic_trace2)) {
                            LOG(LOG_INFO, "Front::incoming: LIC::PLATFORM_CHALLENGE_RESPONSE");
                        }
                        break;
                    case LIC::LICENSE_INFO:
                        // TODO As we never send a server license request, it is unlikely we ever receive a LICENSE_INFO
                        if (bool(this->verbose & Verbose::basic_trace2)) {
                            LOG(LOG_INFO, "Front::incoming: LIC::LICENSE_INFO");
                        }
                        // TODO Instead of returning a license we return a message saying that no license is OK
                        this->send_valid_client_license_data();
                        break;
                    default:
                        if (bool(this->verbose & Verbose::basic_trace2)) {
                            LOG(LOG_INFO, "Front::incoming: LICENCE_TAG %u unknown or unsupported by server", flic.tag);
                        }
                        break;
                    }
                    // licence received, proceed with capabilities exchange

                    // Capabilities Exchange
                    // ---------------------

                    // Capabilities Negotiation: The server sends the set of capabilities it
                    // supports to the client in a Demand Active PDU. The client responds with its
                    // capabilities by sending a Confirm Active PDU.

                    // Client                                                     Server
                    //    | <------- Demand Active PDU ---------------------------- |
                    //    |--------- Confirm Active PDU --------------------------> |

                    if (bool(this->verbose & Verbose::basic_trace)) {
                        LOG(LOG_INFO, "Front::incoming: send_demand_active");
                    }
                    this->send_demand_active();
                    this->send_monitor_layout();

                    LOG(LOG_INFO, "Front::incoming: ACTIVATED (new license request)");
                    this->state = ACTIVATE_AND_PROCESS_DATA;
                }
                else {
                    if (bool(this->verbose & Verbose::basic_trace2)) {
                        LOG(LOG_INFO, "Front::incoming: non licence packet: still waiting for licence");
                    }
                    ShareControl_Recv sctrl(sec.payload);

                    switch (sctrl.pduType) {
                    case PDUTYPE_DEMANDACTIVEPDU: /* 1 */
                        if (bool(this->verbose & Verbose::basic_trace2)) {
                            LOG(LOG_INFO, "Front::incoming: Unexpected DEMANDACTIVE PDU while in licence negociation");
                        }
                        break;
                    case PDUTYPE_CONFIRMACTIVEPDU:
                        if (bool(this->verbose & Verbose::basic_trace2)) {
                            LOG(LOG_INFO, "Front::incoming: Unexpected CONFIRMACTIVE PDU");
                        }
                        {
                            unsigned expected = 6; /* shareId(4) + originatorId(2) */
                            if (!sctrl.payload.in_check_rem(expected)) {
                                LOG(LOG_ERR, "Front::incoming: Truncated CONFIRMACTIVE PDU, need=%u remains=%zu",
                                    expected, sctrl.payload.in_remain());
                                throw Error(ERR_MCS_PDU_TRUNCATED);
                            }
                            uint32_t share_id = sctrl.payload.in_uint32_le();
                            uint16_t originatorId = sctrl.payload.in_uint16_le();
                            this->process_confirm_active(sctrl.payload);
                            (void)share_id;
                            (void)originatorId;
                        }
                        if (!sctrl.payload.check_end()) {
                            LOG(LOG_ERR, "Front::incoming: Trailing data after CONFIRMACTIVE PDU remains=%zu",
                                sctrl.payload.in_remain());
                            throw Error(ERR_MCS_PDU_TRAILINGDATA);
                        }
                        break;
                    case PDUTYPE_DATAPDU: /* 7 */
                        if (bool(this->verbose & Verbose::basic_trace2)) {
                            LOG(LOG_INFO, "Front::incoming: Unexpected DATA PDU while in licence negociation");
                        }
                        // at this point licence negociation is still ongoing
                        // most data packets should not be received
                        // actually even input is dubious,
                        // but rdesktop actually sends input data
                        // also processing this is a problem because input data packets are broken
//                        this->process_data(sctrl.payload, cb);

                        // TODO check all payload data is consumed
                        break;
                    case PDUTYPE_DEACTIVATEALLPDU:
                        if (bool(this->verbose & Verbose::basic_trace2)) {
                            LOG(LOG_INFO, "Front::incoming: Unexpected DEACTIVATEALL PDU while in licence negociation");
                        }
                        // TODO check all payload data is consumed
                        break;
                    case PDUTYPE_SERVER_REDIR_PKT:
                        if (bool(this->verbose & Verbose::basic_trace2)) {
                            LOG(LOG_INFO, "Front::incoming: Unsupported SERVER_REDIR_PKT while in licence negociation");
                        }
                        // TODO check all payload data is consumed
                        break;
                    default:
                        LOG(LOG_WARNING, "Front::incoming: Unknown PDU type received while in licence negociation (%d)\n", sctrl.pduType);
                        break;
                    }
                    // TODO Check why this is necessary when using loop connection ?
                }
                sec.payload.in_skip_bytes(sec.payload.in_remain());
            }
            break;

            case ACTIVATE_AND_PROCESS_DATA:
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::incoming: ACTIVATE_AND_PROCESS_DATA");
            }
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
            // client and server after the connection has been finalized include "
            // connection management information and virtual channel messages (exchanged
            // between client-side plug-ins and server-side applications).
            {
                if (buf.current_pdu_is_fast_path()) {
                    FastPath::ClientInputEventPDU_Recv cfpie(new_x224_stream, this->decrypt);

                    int num_events = cfpie.numEvents;
                    for (uint8_t i = 0; i < num_events; i++) {
                        if (!cfpie.payload.in_check_rem(1)) {
                            LOG(LOG_ERR, "Front::incoming: Truncated Fast-Path input event PDU, need=1 remains=%zu",
                                cfpie.payload.in_remain());
                            throw Error(ERR_RDP_DATA_TRUNCATED);
                        }

                        uint8_t byte = cfpie.payload.in_uint8();
                        uint8_t eventCode  = (byte & 0xE0) >> 5;

                        switch (eventCode) {
                            case FastPath::FASTPATH_INPUT_EVENT_SCANCODE:
                            {
                                FastPath::KeyboardEvent_Recv ke(cfpie.payload, byte);

                                if (bool(this->verbose & Verbose::basic_trace3)) {
                                    LOG(LOG_INFO,
                                        "Front::incoming: Received Fast-Path PUD, scancode eventCode=0x%X SPKeyboardFlags=0x%X, keyCode=0x%X",
                                        ke.eventFlags, ke.spKeyboardFlags, ke.keyCode);
                                }

                                if ((1 == num_events) &&
                                    (0 == i) &&
                                    (cfpie.payload.in_remain() == 6) &&
                                    (0x1D == ke.keyCode) &&
                                    (this->ini.get<cfg::client::bogus_number_of_fastpath_input_event>() ==
                                     BogusNumberOfFastpathInputEvent::pause_key_only)) {
                                    LOG(LOG_INFO,
                                        "Front::incoming: BogusNumberOfFastpathInputEvent::pause_key_only");
                                    num_events = 4;
                                }

                                this->input_event_scancode(ke, cb, 0);
                            }
                            break;

                            case FastPath::FASTPATH_INPUT_EVENT_MOUSE:
                            {
                                FastPath::MouseEvent_Recv me(cfpie.payload, byte);

                                if (bool(this->verbose & Verbose::basic_trace3)) {
                                    LOG(LOG_INFO,
                                        "Front::incoming: Received Fast-Path PUD, mouse pointerFlags=0x%X, xPos=0x%X, yPos=0x%X",
                                        me.pointerFlags, me.xPos, me.yPos);
                                }

                                this->mouse_x = me.xPos;
                                this->mouse_y = me.yPos;
                                if (this->up_and_running) {
                                    cb.rdp_input_mouse(me.pointerFlags, me.xPos, me.yPos, &this->keymap);
                                    this->has_user_activity = true;
                                }

                                if ((me.pointerFlags & (SlowPath::PTRFLAGS_BUTTON1 |
                                                        SlowPath::PTRFLAGS_BUTTON2 |
                                                        SlowPath::PTRFLAGS_BUTTON3)) &&
                                    !(me.pointerFlags & SlowPath::PTRFLAGS_DOWN)) {
                                    if (this->capture) {
                                        this->capture->possible_active_window_change();
                                    }
                                }
                            }
                            break;

                            // XFreeRDP sends this message even if its support is not advertised in the Input Capability Set.
                            case FastPath::FASTPATH_INPUT_EVENT_MOUSEX:
                            {
                                FastPath::MouseExEvent_Recv me(cfpie.payload, byte);

                                LOG(LOG_WARNING,
                                    "Front::Received unexpected fast-path PDU, extended mouse pointerFlags=0x%X, xPos=0x%X, yPos=0x%X",
                                    me.pointerFlags, me.xPos, me.yPos);

                                if (this->up_and_running) {
                                    this->has_user_activity = true;
                                }

                                if ((me.pointerFlags & (SlowPath::PTRXFLAGS_BUTTON1 |
                                                        SlowPath::PTRXFLAGS_BUTTON2)) &&
                                    !(me.pointerFlags & SlowPath::PTRXFLAGS_DOWN)) {
                                    if (this->capture) {
                                        this->capture->possible_active_window_change();
                                    }
                                }
                            }
                            break;

                            case FastPath::FASTPATH_INPUT_EVENT_SYNC:
                            {
                                FastPath::SynchronizeEvent_Recv se(cfpie.payload, byte);

                                if (bool(this->verbose & Verbose::basic_trace3)) {
                                    LOG(LOG_INFO, "Front::incoming: Received Fast-Path PUD, sync eventFlags=0x%X",
                                        se.eventFlags);
                                }
                                LOG(LOG_INFO, "Front::incoming: (Fast-Path) Synchronize Event toggleFlags=0x%X",
                                    static_cast<unsigned int>(se.eventFlags));

                                this->keymap.synchronize(se.eventFlags);
                                if (this->up_and_running) {
                                    cb.rdp_input_synchronize(0, 0, se.eventFlags, 0);
                                    this->has_user_activity = true;
                                }
                            }
                            break;

                            case FastPath::FASTPATH_INPUT_EVENT_UNICODE:
                            {
                                FastPath::UnicodeKeyboardEvent_Recv uke(cfpie.payload, byte);

                                if (bool(this->verbose & Verbose::basic_trace3)) {
                                    LOG(LOG_INFO, "Front::incoming: Received Fast-Path PUD, unicode unicode=0x%04X",
                                        uke.unicodeCode);
                                }

                                if (this->up_and_running) {
                                    cb.rdp_input_unicode(uke.unicodeCode, uke.spKeyboardFlags);
                                    this->has_user_activity = true;
                                }
                            }
                            break;

                            default:
                                LOG(LOG_INFO,
                                    "Front::incoming: Received unexpected Fast-Path PUD, eventCode = %u",
                                    eventCode);
                                throw Error(ERR_RDP_FASTPATH);
                        }
                        if (bool(this->verbose & Verbose::basic_trace3)) {
                            LOG(LOG_INFO, "Front::incoming: Received Fast-Path PUD done");
                        }

                        if (cfpie.payload.in_remain() &&
                            (this->ini.get<cfg::client::bogus_number_of_fastpath_input_event>() ==
                             BogusNumberOfFastpathInputEvent::all_input_events) &&
                            !((i + 1) < num_events)) {
                            LOG(LOG_INFO,
                                "Front::incoming: BogusNumberOfFastpathInputEvent::all_input_events. in_remain=%zu num_events=%d",
                                cfpie.payload.in_remain(), num_events);
                            num_events++;
                        }
                    }

                    if (cfpie.payload.in_remain() != 0) {
                        LOG(LOG_WARNING, "Front::incoming: Received Fast-Path PUD, remains=%zu",
                            cfpie.payload.in_remain());
                    }
                    break;
                }

                // TODO We shall put a specific case when we get Disconnect Request
                if (buf.current_pdu_get_type() == X224::DR_TPDU) {
                    // TODO What is the clean way to actually disconnect ?
                    X224::DR_TPDU_Recv x224(new_x224_stream);
                    LOG(LOG_INFO, "Front::incoming: Received Disconnect Request from RDP client");
                    this->is_client_disconnected = true;
                    throw Error(ERR_X224_RECV_ID_IS_RD_TPDU);   // Disconnect Request - Transport Protocol Data Unit
                }
                if (buf.current_pdu_get_type() != X224::DT_TPDU) {
                    LOG(LOG_ERR, "Front::incoming: Unexpected non data PDU (got %d)", buf.current_pdu_get_type());
                    throw Error(ERR_X224_EXPECTED_DATA_PDU);
                }

                X224::DT_TPDU_Recv x224(new_x224_stream);

                int mcs_type = MCS::peekPerEncodedMCSType(x224.payload);
                if (mcs_type == MCS::MCSPDU_DisconnectProviderUltimatum) {
                    LOG(LOG_INFO, "Front::incoming: DisconnectProviderUltimatum received");
                    MCS::DisconnectProviderUltimatum_Recv mcs(x224.payload, MCS::PER_ENCODING);
                    const char * reason = MCS::get_reason(mcs.reason);
                    LOG(LOG_INFO, "Front::incoming: DisconnectProviderUltimatum: reason=%s [%u]", reason, mcs.reason);
                    this->is_client_disconnected = true;
                    throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
                }

                MCS::SendDataRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);
                SEC::Sec_Recv sec(mcs.payload, this->decrypt, this->encryptionLevel);
                if (bool(this->verbose & Verbose::sec_decrypted)) {
                    LOG(LOG_INFO, "Front::incoming: sec decrypted payload:");
                    hexdump_d(sec.payload.get_data(), sec.payload.get_capacity());
                }

                if (bool(this->verbose & Verbose::basic_trace4)) {
                    LOG(LOG_INFO, "Front::incoming: sec_flags=%x", sec.flags);
                }

                if (mcs.channelId != GCC::MCS_GLOBAL_CHANNEL) {
                    if (bool(this->verbose & Verbose::channel)) {
                        LOG(LOG_INFO, "Front::incoming: channel_data channelId=%u", mcs.channelId);
                    }

                    size_t num_channel_src = this->channel_list.size();
                    for (size_t index = 0; index < this->channel_list.size(); index++) {
                        if (this->channel_list[index].chanid == mcs.channelId) {
                            num_channel_src = index;
                            break;
                        }
                    }

                    if (num_channel_src >= this->channel_list.size()) {
                        LOG(LOG_ERR, "Front::incoming: Unknown Channel");
                        throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
                    }

                    const CHANNELS::ChannelDef & channel = this->channel_list[num_channel_src];
                    if (bool(this->verbose & Verbose::channel)) {
                        channel.log(mcs.channelId);
                    }

                    unsigned expected = 8; /* length(4) + flags(4) */
                    if (!sec.payload.in_check_rem(expected)) {
                        LOG(LOG_ERR, "Front::incoming: Data truncated, need=%u remains=%zu",
                            expected, sec.payload.in_remain());
                        throw Error(ERR_MCS);
                    }

                    uint32_t length = sec.payload.in_uint32_le();
                    uint32_t flags  = sec.payload.in_uint32_le();
                    size_t chunk_size = sec.payload.in_remain();

                    if (this->up_and_running) {
                        if (bool(this->verbose & Verbose::channel)) {
                            LOG(LOG_INFO, "Front::incoming: channel_name=\"%s\"", channel.name);
                        }

                        InStream chunk(sec.payload.get_current(), chunk_size);

                        cb.send_to_mod_channel(channel.name, chunk, length, flags);
                    }
                    else {
                        if (bool(this->verbose & Verbose::channel)) {
                            LOG(LOG_INFO, "Front::incoming: not up_and_running send_to_mod_channel dropped");
                        }
                    }
                    sec.payload.in_skip_bytes(chunk_size);
                }
                else {
                    while (sec.payload.get_current() < sec.payload.get_data_end()) {
                        ShareControl_Recv sctrl(sec.payload);

                        switch (sctrl.pduType) {
                        case PDUTYPE_DEMANDACTIVEPDU:
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                LOG(LOG_INFO, "Front::incoming: Received DEMANDACTIVEPDU (unsupported)");
                            }
                            break;
                        case PDUTYPE_CONFIRMACTIVEPDU:
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                LOG(LOG_INFO, "Front::incoming: Received CONFIRMACTIVEPDU");
                            }
                            {
                                unsigned expected = 6;   /* shareId(4) + originatorId(2) */
                                if (!sctrl.payload.in_check_rem(expected)) {
                                    LOG(LOG_ERR,
                                        "Front::incoming: Truncated Confirm Active PDU data, need=%u remains=%zu",
                                        expected, sctrl.payload.in_remain());
                                    throw Error(ERR_RDP_DATA_TRUNCATED);
                                }

                                uint32_t share_id = sctrl.payload.in_uint32_le();
                                uint16_t originatorId = sctrl.payload.in_uint16_le();
                                this->process_confirm_active(sctrl.payload);
                                (void)share_id;
                                (void)originatorId;
                            }
                            // reset caches, etc.
                            this->reset();
                            // resizing done
                            {
                                RDPColCache cmd(0, BGRPalette::classic_332());
                                this->orders.graphics_update_pdu().draw(cmd);
                            }
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                LOG(LOG_INFO, "Front::incoming: Received CONFIRMACTIVEPDU done");
                            }

                            break;
                        case PDUTYPE_DATAPDU: /* 7 */
                            if (bool(this->verbose & Verbose::basic_trace4)) {
                                LOG(LOG_INFO, "Front::incoming: Received DATAPDU");
                            }
                            // this is rdp_process_data that will set up_and_running to 1
                            // when fonts have been received
                            // we will not exit this loop until we are in this state.
                            //LOG(LOG_INFO, "sctrl.payload.len= %u sctrl.len = %u", sctrl.payload.size(), sctrl.len);
                            this->process_data(sctrl.payload, cb);
                            if (bool(this->verbose & Verbose::basic_trace4)) {
                                LOG(LOG_INFO, "Front::incoming: Received DATAPDU done");
                            }

                            if (!sctrl.payload.check_end())
                            {
                                LOG(LOG_ERR,
                                    "Front::incoming: Trailing data after DATAPDU: remains=%zu",
                                    sctrl.payload.in_remain());
                                throw Error(ERR_MCS_PDU_TRAILINGDATA);
                            }
                            break;
                        case PDUTYPE_DEACTIVATEALLPDU:
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                LOG(LOG_INFO, "Front::incoming: Received DEACTIVATEALLPDU (unsupported)");
                            }
                            break;
                        case PDUTYPE_SERVER_REDIR_PKT:
                            if (bool(this->verbose & Verbose::basic_trace)) {
                                LOG(LOG_INFO, "Front::incoming: Received SERVER_REDIR_PKT (unsupported)");
                            }
                            break;
                        default:
                            LOG(LOG_WARNING, "Front::incoming: Received unknown PDU type in session_data (%d)\n", sctrl.pduType);
                            break;
                        }

                        // TODO check all sctrl.payload data is consumed
                    }
                }

                if (!sec.payload.check_end())
                {
                    LOG(LOG_ERR,
                        "Front::incoming: Trailing data after SEC: remains=%zu",
                        sec.payload.in_remain());
                    throw Error(ERR_SEC_TRAILINGDATA);
                }
            }
            break;
            }
        }
    }

    void send_valid_client_license_data() {
        this->send_data_indication(
            GCC::MCS_GLOBAL_CHANNEL,
            [this](StreamSize<24>, OutStream & sec_header) {
                // Valid Client License Data (LICENSE_VALID_CLIENT_DATA)

                /* some compilers need unsigned char to avoid warnings */
                uint8_t lic2[16] = {
                    0xff,                   // bMsgType : ERROR_ALERT
                    0x02,                   // NOT EXTENDED_ERROR_MSG_SUPPORTED, PREAMBLE_VERSION_2_0
                    0x10, 0x00,             // wMsgSize: 16 bytes including preamble
                    0x07, 0x00, 0x00, 0x00, // dwErrorCode : STATUS_VALID_CLIENT
                    0x02, 0x00, 0x00, 0x00, // dwStateTransition ST_NO_TRANSITION
                    0x28, 0x14,             // wBlobType : ignored because wBlobLen is 0
                    0x00, 0x00              // wBlobLen  : 0
                };
                SEC::Sec_Send sec(
                    sec_header, lic2, sizeof(lic2),
                    SEC::SEC_LICENSE_PKT | 0x00100000, this->encrypt, 0
                );
                (void)sec;

                if (bool(this->verbose & Verbose::global_channel)) {
                    LOG(LOG_INFO, "Front::send_valid_client_license_data: Sec clear payload to send:");
                    hexdump_d(lic2, sizeof(lic2));
                }

                sec_header.out_copy_bytes(lic2, sizeof(lic2));
            }
        );
    }

    template<class DataWriter>
    void send_data_indication(uint16_t channelId, DataWriter data_writer)
    {
        write_packets(
            this->trans,
            data_writer,
            [channelId, this](StreamSize<256>, OutStream & mcs_header, std::size_t packet_sz) {
                MCS::SendDataIndication_Send mcs(
                    mcs_header,
                    this->userid, channelId,
                    1, 3, packet_sz,
                    MCS::PER_ENCODING
                );
                (void)mcs;
            },
            X224::write_x224_dt_tpdu_fn{}
        );
    }

public:
    template<class... Writer>
    void send_data_indication_ex_impl(uint16_t channelId, Writer... writer) {
        ::send_data_indication_ex(
            this->trans, channelId, this->encryptionLevel, this->encrypt, this->userid,
            writer...
        );
    }

private:
    void session_probe_started(bool started) override {
        this->session_probe_started_ = started;

        this->update_keyboard_input_mask_state();

        if (!started) {
            this->session_update("Front::session_probe_started: Probe.Status=Unknown");
        }
    }

    void set_keylayout(int LCID) override {
        this->keymap.init_layout(LCID);
    }

    int get_keylayout() const override {
        return this->keymap.layout_id();
    }

    void set_focus_on_password_textbox(bool set) override {
        this->focus_on_password_textbox = set;

        this->update_keyboard_input_mask_state();
    }

    void set_focus_on_unidentified_input_field(bool set) override {
        this->focus_on_unidentified_input_field = set;

        this->update_keyboard_input_mask_state();
    }

    void set_consent_ui_visible(bool set) override {
        this->consent_ui_is_visible = set;

        this->update_keyboard_input_mask_state();
    }

    void session_update(array_view_const_char message) override {
        if (this->capture) {
            this->capture->session_update(this->session_reactor.get_current_time(), message);
        }
    }

    /*****************************************************************************/
    void send_data_update_sync()
    {
        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_data_update_sync");
        }

        StaticOutReservedStreamHelper<1024, 65536-1024> stream;

        ::send_server_update( this->trans
                            , this->server_fastpath_update_support
                            , (bool(this->ini.get<cfg::client::rdp_compression>()) ? this->client_info.rdp_compression : 0)
                            , this->mppc_enc.get()
                            , this->share_id
                            , this->encryptionLevel
                            , this->encrypt
                            , this->userid
                            , SERVER_UPDATE_GRAPHICS_SYNCHRONIZE
                            , 0
                            , stream
                            , underlying_cast(this->verbose)
                            );
    }

    /*****************************************************************************/
    void send_demand_active()
    {
        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_demand_active");
        }

        this->send_data_indication_ex_impl(
            GCC::MCS_GLOBAL_CHANNEL,
            [this](StreamSize<65536>, OutStream & stream) {
                size_t caps_count = 0;

                // Payload
                stream.out_uint32_le(this->share_id);
                stream.out_uint16_le(4); /* 4 chars for RDP\0 */

                /* 2 bytes size after num caps, set later */
                uint32_t caps_size_offset = stream.get_offset();
                stream.out_clear_bytes(2);

                stream.out_copy_bytes("RDP", 4);

                /* 4 byte num caps, set later */
                uint32_t caps_count_offset = stream.get_offset();
                stream.out_clear_bytes(4);

                GeneralCaps general_caps;

                if (this->fastpath_support) {
                    general_caps.extraflags |= FASTPATH_OUTPUT_SUPPORTED;
                }
                if (!this->server_capabilities_filename.empty()) {
                    general_caps_load(general_caps, this->server_capabilities_filename);
                }
                if (bool(this->verbose)) {
                    general_caps.log("Front::send_demand_active: Sending to client");
                }
                general_caps.emit(stream);
                caps_count++;

                BitmapCaps bitmap_caps;
                bitmap_caps.preferredBitsPerPixel = safe_int(this->client_info.screen_info.bpp);
                bitmap_caps.desktopWidth = this->client_info.screen_info.width;
                bitmap_caps.desktopHeight = this->client_info.screen_info.height;
                bitmap_caps.drawingFlags = DRAW_ALLOW_SKIP_ALPHA;
                if (!this->server_capabilities_filename.empty()) {
                    bitmap_caps_load(bitmap_caps, this->server_capabilities_filename);
                }
                if (bool(this->verbose)) {
                    bitmap_caps.log("Front::send_demand_active: Sending to client");
                }
                bitmap_caps.emit(stream);
                caps_count++;

                FontCaps font_caps;
                if (bool(this->verbose)) {
                    font_caps.log("Front::send_demand_active: Sending to client");
                }
                font_caps.emit(stream);
                caps_count++;

                OrderCaps order_caps;
                order_caps.pad4octetsA = 0x40420f00;
                order_caps.numberFonts = 0x2f;
                order_caps.orderFlags = 0x22;
                order_caps.orderSupport[TS_NEG_DSTBLT_INDEX]             = 1;
                order_caps.orderSupport[TS_NEG_PATBLT_INDEX]             = 1;
                order_caps.orderSupport[TS_NEG_SCRBLT_INDEX]             = 1;
                order_caps.orderSupport[TS_NEG_MEMBLT_INDEX]             = 1;
                order_caps.orderSupport[TS_NEG_MEM3BLT_INDEX]            = (this->mem3blt_support ? 1 : 0);
                order_caps.orderSupport[TS_NEG_LINETO_INDEX]             = 1;
                order_caps.orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] = 1;
                order_caps.orderSupport[TS_NEG_POLYLINE_INDEX]           = 1;
                order_caps.orderSupport[TS_NEG_ELLIPSE_SC_INDEX]         = 1;
                order_caps.orderSupport[TS_NEG_INDEX_INDEX]              = 1;
                order_caps.orderSupport[UnusedIndex3] = 1;
                order_caps.textFlags = 0x06a1;
                order_caps.pad4octetsB = 0x0f4240;
                order_caps.desktopSaveSize = 0x0f4240;
                order_caps.pad2octetsC = 1;
                if (!this->server_capabilities_filename.empty()) {
                    order_caps_load(order_caps, this->server_capabilities_filename);
                }
                if (bool(this->verbose)) {
                    order_caps.log("Front::send_demand_active: Sending to client");
                }
                order_caps.emit(stream);
                caps_count++;

                if (this->ini.get<cfg::client::persistent_disk_bitmap_cache>()) {
                    BitmapCacheHostSupportCaps bitmap_cache_host_support_caps;
                    if (bool(this->verbose)) {
                        bitmap_cache_host_support_caps.log("Front::send_demand_active: Sending to client");
                    }
                    bitmap_cache_host_support_caps.emit(stream);
                    caps_count++;
                }

                ColorCacheCaps colorcache_caps;
                if (bool(this->verbose)) {
                    colorcache_caps.log("Front::send_demand_active: Sending to client");
                }
                colorcache_caps.emit(stream);
                caps_count++;

                PointerCaps pointer_caps;
                pointer_caps.colorPointerCacheSize = 0x19;
                pointer_caps.pointerCacheSize = 0x19;
                if (bool(this->verbose)) {
                    pointer_caps.log("Front::send_demand_active: Sending to client");
                }
                pointer_caps.emit(stream);
                caps_count++;

                ShareCaps share_caps;
                share_caps.nodeId = this->userid + GCC::MCS_USERCHANNEL_BASE;
                share_caps.pad2octets = 0xb5e2; /* 0x73e1 */
                if (bool(this->verbose)) {
                    share_caps.log("Front::send_demand_active: Sending to client");
                }
                share_caps.emit(stream);
                caps_count++;

                InputCaps input_caps;

                // Slow/Fast-path
                input_caps.inputFlags          =
                    INPUT_FLAG_SCANCODES
                    | (this->ini.get<cfg::globals::unicode_keyboard_event_support>() ? INPUT_FLAG_UNICODE : 0)
                    | (  this->client_fastpath_input_event_support
                    ? (INPUT_FLAG_FASTPATH_INPUT | INPUT_FLAG_FASTPATH_INPUT2) : 0);
                input_caps.keyboardLayout      = 0;
                input_caps.keyboardType        = 0;
                input_caps.keyboardSubType     = 0;
                input_caps.keyboardFunctionKey = 0;
                if (bool(this->verbose)) {
                    input_caps.log("Front::send_demand_active: Sending to client");
                }
                input_caps.emit(stream);
                caps_count++;

                if (this->client_info.remote_program) {
                    RailCaps rail_caps;
                    rail_caps.RailSupportLevel = TS_RAIL_LEVEL_SUPPORTED | TS_RAIL_LEVEL_DOCKED_LANGBAR_SUPPORTED | TS_RAIL_LEVEL_HANDSHAKE_EX_SUPPORTED;
                    if (bool(this->verbose)) {
                        rail_caps.log("Front::send_demand_active: Sending to client");
                    }
                    rail_caps.emit(stream);
                    caps_count++;

                    WindowListCaps window_list_caps;
                    window_list_caps.WndSupportLevel = TS_WINDOW_LEVEL_SUPPORTED_EX;
                    window_list_caps.NumIconCaches = 3;
                    window_list_caps.NumIconCacheEntries = 12;
                    if (bool(this->verbose)) {
                        window_list_caps.log("Front::send_demand_active: Sending to client");
                    }
                    window_list_caps.emit(stream);
                    caps_count++;
                }

                if (this->ini.get<cfg::globals::large_pointer_support>()) {
                    LargePointerCaps large_pointer_caps;

                    large_pointer_caps.largePointerSupportFlags = LARGE_POINTER_FLAG_96x96;
                    if (bool(this->verbose)) {
                        large_pointer_caps.log("Front::send_demand_active: Sending to client");
                    }
                    large_pointer_caps.emit(stream);
                    caps_count++;

                    MultiFragmentUpdateCaps multifrag_caps;
                    multifrag_caps.MaxRequestSize = 38055;
                    if (bool(this->verbose)) {
                        multifrag_caps.log("Front::send_demand_active: Sending to client");
                    }
                    multifrag_caps.emit(stream);
                    caps_count++;
                }

                size_t caps_size = stream.get_offset() - caps_count_offset;
                stream.set_out_uint16_le(caps_size, caps_size_offset);
                stream.set_out_uint32_le(caps_count, caps_count_offset);

                stream.out_clear_bytes(4); /* sessionId(4). This field is ignored by the client. */
            },
            [this](StreamSize<256>, OutStream & sctrl_header, std::size_t packet_size) {
                ShareControl_Send(sctrl_header, PDUTYPE_DEMANDACTIVEPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, packet_size);

            },
            [this](StreamSize<0>, OutStream &, uint8_t  const * packet_data, std::size_t packet_size) {
                if (bool(this->verbose & Verbose::global_channel)) {
                    LOG(LOG_INFO, "Front::send_demand_active: Sec clear payload to send:");
                    hexdump_d(packet_data, packet_size);
                }
            }
        );

        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_demand_active: done");
        }
    }   // send_demand_active

    void process_confirm_active(InStream & stream)
    {
        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::process_confirm_active");
        }
        // TODO We should separate the parts relevant to caps processing and the part relevant to actual confirm active
        // TODO Server Caps management should go to RDP layer and be unified between client (mod/rdp.hpp and server code front.hpp)

        unsigned expected = 4; /* lengthSourceDescriptor(2) + lengthCombinedCapabilities(2) */
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR, "Front::process_confirm_active: Truncated CONFIRMACTIVE PDU, need=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        uint16_t lengthSourceDescriptor = stream.in_uint16_le(); /* sizeof RDP_SOURCE */
        uint16_t lengthCombinedCapabilities = stream.in_uint16_le();

        if (!stream.in_check_rem(lengthSourceDescriptor)) {
            LOG(LOG_ERR, "Front::process_confirm_active: Truncated CONFIRMACTIVE PDU lengthSourceDescriptor, need=%u remains=%zu",
                lengthSourceDescriptor, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        stream.in_skip_bytes(lengthSourceDescriptor);

        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::process_confirm_active: lengthSourceDescriptor = %u", lengthSourceDescriptor);
            LOG(LOG_INFO, "Front::process_confirm_active: lengthCombinedCapabilities = %u", lengthCombinedCapabilities);
        }

        uint8_t const * start = stream.get_current();
        uint8_t const * theoricCapabilitiesEnd = start + lengthCombinedCapabilities;
        uint8_t const * actualCapabilitiesEnd = stream.get_data_end();

        expected = 4; /* numberCapabilities(2) + pad(2) */
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR, "Front::process_confirm_active: Truncated CONFIRMACTIVE PDU numberCapabilities, need=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        int numberCapabilities = stream.in_uint16_le();
        stream.in_skip_bytes(2); /* pad */

        for (int n = 0; n < numberCapabilities; n++) {
            if (bool(this->verbose & Verbose::basic_trace5)) {
                LOG(LOG_INFO, "Front::capability %d / %d", n, numberCapabilities );
            }
            if (stream.get_current() + 4 > theoricCapabilitiesEnd) {
                LOG(LOG_ERR, "Front::process_confirm_active: Incomplete capabilities received (bad length):"
                    " expected length=%" PRIu16 " need=%" PRIdPTR " available=%zu",
                    lengthCombinedCapabilities,
                    stream.get_current()-start,
                    stream.in_remain());
            }
            if (stream.get_current() + 4 > actualCapabilitiesEnd) {
                LOG(LOG_ERR, "Front::process_confirm_active: Incomplete capabilities received (need more data):"
                    " expected length=%" PRIu16 " need=%" PRIdPTR " available=%zu",
                    lengthCombinedCapabilities,
                    stream.get_current()-start,
                    stream.in_remain());
                return;
            }

            uint16_t capset_type = stream.in_uint16_le();
            uint16_t capset_length = stream.in_uint16_le();
            uint8_t const * next = (stream.get_current() + capset_length) - 4;

            switch (capset_type) {
            case CAPSTYPE_GENERAL: {
                    this->client_info.general_caps.recv(stream, capset_length);
                    if (bool(this->verbose)) {
                        this->client_info.general_caps.log("Front::process_confirm_active: Receiving from client");
                    }
                    this->client_info.use_compact_packets =
                        bool(this->client_info.general_caps.extraflags & NO_BITMAP_COMPRESSION_HDR);

                    this->server_fastpath_update_support =
                        (   this->fastpath_support
                         && ((this->client_info.general_caps.extraflags & FASTPATH_OUTPUT_SUPPORTED) != 0)
                        );
                }
                break;
            case CAPSTYPE_BITMAP: {
                    this->client_info.bitmap_caps.recv(stream, capset_length);
                    if (bool(this->verbose)) {
                        this->client_info.bitmap_caps.log("Front::process_confirm_active: Receiving from client");
                    }
/*
                    this->client_info.bpp    =
                          (this->client_info.bitmap_caps.preferredBitsPerPixel >= 24)
                        ? 24 : this->client_info.bitmap_caps.preferredBitsPerPixel;
*/
                    // Fixed bug in rdesktop
                    // Desktop size in Client Core Data != Desktop size in Bitmap Capability Set
                    if (!this->client_info.screen_info.width || !this->client_info.screen_info.height)
                    {
                        this->client_info.screen_info.width  = this->client_info.bitmap_caps.desktopWidth;
                        this->client_info.screen_info.height = this->client_info.bitmap_caps.desktopHeight;
                    }
                }
                break;
            case CAPSTYPE_ORDER: { /* 3 */
                    this->client_info.order_caps.recv(stream, capset_length);
                    if (bool(this->verbose)) {
                        this->client_info.order_caps.log("Front::process_confirm_active: Receiving from client");
                    }
                }
                break;
            case CAPSTYPE_BITMAPCACHE: {
                    this->client_info.bmp_cache_caps.recv(stream, capset_length);
                    if (bool(this->verbose)) {
                        this->client_info.bmp_cache_caps.log("Front::process_confirm_active: Receiving from client");
                    }
                    this->client_info.number_of_cache      = 3;
                    this->client_info.cache1_entries       = this->client_info.bmp_cache_caps.cache0Entries;
                    this->client_info.cache1_persistent    = false;
                    this->client_info.cache1_size          = this->client_info.bmp_cache_caps.cache0MaximumCellSize;
                    this->client_info.cache2_entries       = this->client_info.bmp_cache_caps.cache1Entries;
                    this->client_info.cache2_persistent    = false;
                    this->client_info.cache2_size          = this->client_info.bmp_cache_caps.cache1MaximumCellSize;
                    this->client_info.cache3_entries       = this->client_info.bmp_cache_caps.cache2Entries;
                    this->client_info.cache3_persistent    = false;
                    this->client_info.cache3_size          = this->client_info.bmp_cache_caps.cache2MaximumCellSize;
                    this->client_info.cache4_entries       = 0;
                    this->client_info.cache5_entries       = 0;
                    this->client_info.cache_flags          = 0;
                    this->client_info.bitmap_cache_version = 0;
                }
                break;
            case CAPSTYPE_CONTROL: /* 5 */
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSTYPE_CONTROL");
                }
                break;
            case CAPSTYPE_ACTIVATION: /* 7 */
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSTYPE_ACTIVATION");
                }
                break;
            case CAPSTYPE_POINTER: {  /* 8 */
                    PointerCaps pointer_caps;
                    pointer_caps.recv(stream, capset_length);
                    if (bool(this->verbose)) {
                        pointer_caps.log("Front::process_confirm_active: Receiving from client");
                    }

                    this->client_info.pointer_cache_entries =
                        std::min<int>(pointer_caps.colorPointerCacheSize, MAX_POINTER_COUNT);
                    this->client_info.supported_new_pointer_update =
                        (pointer_caps.pointerCacheSize != 0);
                }
                break;
            case CAPSTYPE_SHARE: /* 9 */
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSTYPE_SHARE");
                }
                break;
            case CAPSTYPE_COLORCACHE: /* 10 */
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSTYPE_COLORCACHE");
                }
                break;
            case CAPSTYPE_SOUND:
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSTYPE_SOUND");
                }
                break;
            case CAPSTYPE_INPUT: /* 13 */
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSTYPE_INPUT");
                }
                break;
            case CAPSTYPE_FONT: /* 14 */
                break;
            case CAPSTYPE_BRUSH: { /* 15 */
                    if (bool(this->verbose)) {
                        LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSTYPE_BRUSH");
                    }
                    BrushCacheCaps brushcache_caps;
                    brushcache_caps.recv(stream, capset_length);
                    if (bool(this->verbose)) {
                        brushcache_caps.log("Front::process_confirm_active: Receiving from client");
                    }
                    this->client_info.brush_cache_code = brushcache_caps.brushSupportLevel;
                }
                break;
            case CAPSTYPE_GLYPHCACHE: { /* 16 */
                    if (bool(this->verbose)) {
                        LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSTYPE_GLYPHCACHE");
                    }
                    this->client_info.glyph_cache_caps.recv(stream, capset_length);
                    if (bool(this->verbose)) {
                        this->client_info.glyph_cache_caps.log("Front::process_confirm_active: Receiving from client");
                    }
                    if (ini.get<cfg::client::bogus_ios_glyph_support_level>() &&
                        (this->client_info.general_caps.os_major == OSMAJORTYPE_IOS)) {
                        LOG(LOG_INFO, "Front::process_confirm_active: Support of bogus iOS glyph support level enabled.");

                        this->client_info.glyph_cache_caps.GlyphSupportLevel = GlyphCacheCaps::GLYPH_SUPPORT_NONE;
                    }
                    if (ini.get<cfg::client::transform_glyph_to_bitmap>()) {
                        LOG(LOG_INFO, "Front::process_confirm_active: Transform Glyph to Bitmap.");

                        this->client_info.glyph_cache_caps.GlyphSupportLevel = GlyphCacheCaps::GLYPH_SUPPORT_NONE;
                    }
                    for (uint8_t i = 0; i < NUMBER_OF_GLYPH_CACHES; ++i) {
                        this->client_info.number_of_entries_in_glyph_cache[i] =
                            this->client_info.glyph_cache_caps.GlyphCache[i].CacheEntries;
                    }
                }
                break;
            case CAPSTYPE_OFFSCREENCACHE: /* 17 */
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSTYPE_OFFSCREENCACHE");
                }
                this->client_info.off_screen_cache_caps.recv(stream, capset_length);
                if (bool(this->verbose)) {
                    this->client_info.off_screen_cache_caps.log("Front::process_confirm_active: Receiving from client");
                }
                break;
            case CAPSTYPE_BITMAPCACHE_HOSTSUPPORT: /* 18 */
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSTYPE_BITMAPCACHE_HOSTSUPPORT");
                }
                break;
            case CAPSTYPE_BITMAPCACHE_REV2: {
                    this->client_info.bmp_cache_2_caps.recv(stream, capset_length);
                    if (bool(this->verbose)) {
                        this->client_info.bmp_cache_2_caps.log("Front::process_confirm_active: Receiving from client");
                    }

                    // TODO We only use the first 3 caches (those existing in Rev1), we should have 2 more caches for rev2
                    this->client_info.number_of_cache = this->client_info.bmp_cache_2_caps.numCellCaches;
                    int Bpp = nb_bytes_per_pixel(this->client_info.screen_info.bpp);
                    if (this->client_info.bmp_cache_2_caps.numCellCaches > 0) {
                        this->client_info.cache1_entries    = (this->client_info.bmp_cache_2_caps.bitmapCache0CellInfo & 0x7fffffff);
                        this->client_info.cache1_persistent = (this->client_info.bmp_cache_2_caps.bitmapCache0CellInfo & 0x80000000);
                        this->client_info.cache1_size       = 256 * Bpp;
                    }
                    else {
                        this->client_info.cache1_entries = 0;
                    }
                    if (this->client_info.bmp_cache_2_caps.numCellCaches > 1) {
                        this->client_info.cache2_entries    = (this->client_info.bmp_cache_2_caps.bitmapCache1CellInfo & 0x7fffffff);
                        this->client_info.cache2_persistent = (this->client_info.bmp_cache_2_caps.bitmapCache1CellInfo & 0x80000000);
                        this->client_info.cache2_size       = 1024 * Bpp;
                    }
                    else {
                        this->client_info.cache2_entries = 0;
                    }
                    if (this->client_info.bmp_cache_2_caps.numCellCaches > 2) {
                        this->client_info.cache3_entries    = (this->client_info.bmp_cache_2_caps.bitmapCache2CellInfo & 0x7fffffff);
                        this->client_info.cache3_persistent = (this->client_info.bmp_cache_2_caps.bitmapCache2CellInfo & 0x80000000);
                        this->client_info.cache3_size       = 4096 * Bpp;
                    }
                    else {
                        this->client_info.cache3_entries = 0;
                    }
                    if (this->client_info.bmp_cache_2_caps.numCellCaches > 3) {
                        this->client_info.cache4_entries    = (this->client_info.bmp_cache_2_caps.bitmapCache3CellInfo & 0x7fffffff);
                        this->client_info.cache4_persistent = (this->client_info.bmp_cache_2_caps.bitmapCache3CellInfo & 0x80000000);
                        this->client_info.cache4_size       = 6144 * Bpp;
                    }
                    else {
                        this->client_info.cache4_entries = 0;
                    }
                    if (this->client_info.bmp_cache_2_caps.numCellCaches > 4) {
                        this->client_info.cache5_entries    = (this->client_info.bmp_cache_2_caps.bitmapCache4CellInfo & 0x7fffffff);
                        this->client_info.cache5_persistent = (this->client_info.bmp_cache_2_caps.bitmapCache4CellInfo & 0x80000000);
                        this->client_info.cache5_size       = 8192 * Bpp;
                    }
                    else {
                        this->client_info.cache5_entries = 0;
                    }
                    this->client_info.cache_flags          = this->client_info.bmp_cache_2_caps.cacheFlags;
                    this->client_info.bitmap_cache_version = 2;
                }
                break;
            case CAPSTYPE_VIRTUALCHANNEL: /* 20 */
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSTYPE_VIRTUALCHANNEL");
                }
                break;
            case CAPSTYPE_DRAWNINEGRIDCACHE: /* 21 */
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSTYPE_DRAWNINEGRIDCACHE");
                }
                break;
            case CAPSTYPE_DRAWGDIPLUS: /* 22 */
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSTYPE_DRAWGDIPLUS");
                }
                break;
            case CAPSTYPE_RAIL: /* 23 */
                this->client_info.rail_caps.recv(stream, capset_length);
                if (bool(this->verbose)) {
                    this->client_info.rail_caps.log("Front::process_confirm_active: Receiving from client");
                }
                break;
            case CAPSTYPE_WINDOW: /* 24 */
                this->client_info.window_list_caps.recv(stream, capset_length);
                if (bool(this->verbose)) {
                    this->client_info.window_list_caps.log("Front::process_confirm_active: Receiving from client");
                }
                break;
            case CAPSETTYPE_COMPDESK: { /* 25 */
                    CompDeskCaps cap;
                    cap.recv(stream, capset_length);
                    if (bool(this->verbose)) {
                        cap.log("Front::process_confirm_active: Receiving from client");
                    }
                }
                break;
            case CAPSETTYPE_MULTIFRAGMENTUPDATE: /* 26 */
                this->client_info.multi_fragment_update_caps.recv(stream, capset_length);
                if (bool(this->verbose)) {
                    this->client_info.multi_fragment_update_caps.log("Front::process_confirm_active: Receiving from client");
                }
                break;
            case CAPSETTYPE_LARGE_POINTER: /* 27 */
                this->client_info.large_pointer_caps.recv(stream, capset_length);
                if (bool(this->verbose)) {
                    this->client_info.large_pointer_caps.log("Front::process_confirm_active: Receiving from client");
                }
                break;
            case CAPSETTYPE_SURFACE_COMMANDS: /* 28 */
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSETTYPE_SURFACE_COMMANDS");
                }
                break;
            case CAPSETTYPE_BITMAP_CODECS: /* 29 */
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSETTYPE_BITMAP_CODECS");
                }
                break;
            case CAPSETTYPE_FRAME_ACKNOWLEDGE: /* 30 */
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client CAPSETTYPE_FRAME_ACKNOWLEDGE");
                }
                break;
            default:
                if (bool(this->verbose)) {
                    LOG(LOG_INFO, "Front::process_confirm_active: Receiving from client unknown caps %u", capset_type);
                }
                break;
            }
            if (stream.get_current() > next) {
                LOG(LOG_ERR, "Front::process_confirm_active: Read out of bound detected");
                throw Error(ERR_MCS);
            }
            stream.in_skip_bytes(next - stream.get_current());
        }
        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::process_confirm_active: done p=%p end=%p",
                voidp(stream.get_current()), voidp(stream.get_data_end()));
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
// header is not include " in the PDU.

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

    void send_synchronize()
    {
        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_synchronize");
        }

        StaticOutReservedStreamHelper<1024, 65536-1024> stream;
        // Payload
        stream.get_data_stream().out_uint16_le(1);    // messageType = SYNCMSGTYPE_SYNC(1)
        stream.get_data_stream().out_uint16_le(1002); // targetUser (MCS channel ID of the target user.)

        const uint32_t log_condition = (128 | 1);
        ::send_share_data_ex( this->trans
                            , PDUTYPE2_SYNCHRONIZE
                            , false
                            , this->mppc_enc.get()
                            , this->share_id
                            , this->encryptionLevel
                            , this->encrypt
                            , this->userid
                            , stream
                            , log_condition
                            , underlying_cast(this->verbose)
                            );

        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_synchronize: done");
        }
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

    void send_control(int action)
    {
        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_control: action=%d", action);
        }

        StaticOutReservedStreamHelper<1024, 65536-1024> stream;

        // Payload
        stream.get_data_stream().out_uint16_le(action);
        stream.get_data_stream().out_uint16_le(0); // userid
        stream.get_data_stream().out_uint32_le(1002); // control id

        const uint32_t log_condition = (128 | 1);
        ::send_share_data_ex( this->trans
                            , PDUTYPE2_CONTROL
                            , false
                            , this->mppc_enc.get()
                            , this->share_id
                            , this->encryptionLevel
                            , this->encrypt
                            , this->userid
                            , stream
                            , log_condition
                            , underlying_cast(this->verbose)
                            );

        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_control: done. action=%d", action);
        }
    }

    /*****************************************************************************/
    void send_fontmap()
    {
        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_fontmap");
        }

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

        StaticOutReservedStreamHelper<1024, 65536-1024> stream;

        // Payload
        stream.get_data_stream().out_copy_bytes(g_fontmap, 172);

        const uint32_t log_condition = (128 | 1);
        ::send_share_data_ex( this->trans
                            , PDUTYPE2_FONTMAP
                            , false
                            , this->mppc_enc.get()
                            , this->share_id
                            , this->encryptionLevel
                            , this->encrypt
                            , this->userid
                            , stream
                            , log_condition
                            , underlying_cast(this->verbose)
                            );

        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_fontmap: done");
        }
    }

    void send_savesessioninfo() override {
        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_savesessioninfo");
        }

        StaticOutReservedStreamHelper<1024, 65536-1024> stream;

        // Payload
        stream.get_data_stream().out_uint32_le(RDP::INFOTYPE_LOGON);

        RDP::LogonInfoVersion1_Send sender(stream.get_data_stream(),
                                      byte_ptr_cast(""),
                                      byte_ptr_cast(ini.get<cfg::globals::auth_user>().c_str()),
                                      getpid());

        const uint32_t log_condition = (128 | 1);
        ::send_share_data_ex( this->trans
                            , PDUTYPE2_SAVE_SESSION_INFO
                            , false
                            , this->mppc_enc.get()
                            , this->share_id
                            , this->encryptionLevel
                            , this->encrypt
                            , this->userid
                            , stream
                            , log_condition
                            , underlying_cast(this->verbose)
                            );

        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_savesessioninfo: done");
        }
    }   // void send_savesessioninfo()

    void send_monitor_layout() {
        if (!this->ini.get<cfg::globals::allow_using_multiple_monitors>() &&
            this->client_info.cs_monitor.monitorCount &&
            this->client_info.remote_program) {
            LOG(LOG_WARNING, "Front::send_monitor_layout: RemoteApp in multimon mode, but the use of multiple monitors is not allowed. You may experience display issues!");
        }

        if (!this->ini.get<cfg::globals::allow_using_multiple_monitors>() ||
            !this->client_info.cs_monitor.monitorCount ||
            !this->client_support_monitor_layout_pdu) {
            return;
        }

        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_monitor_layout");
        }

        MonitorLayoutPDU monitor_layout_pdu;

        monitor_layout_pdu.set(this->client_info.cs_monitor);
        monitor_layout_pdu.log("Front::send_monitor_layout: Send to client");

        StaticOutReservedStreamHelper<1024, 65536-1024> stream;

        // Payload
        monitor_layout_pdu.emit(stream.get_data_stream());

        const uint32_t log_condition = (128 | 1);
        ::send_share_data_ex( this->trans
                            , PDUTYPE2_MONITOR_LAYOUT_PDU
                            , false
                            , this->mppc_enc.get()
                            , this->share_id
                            , this->encryptionLevel
                            , this->encrypt
                            , this->userid
                            , stream
                            , log_condition
                            , underlying_cast(this->verbose)
                            );

        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_monitor_layout: done");
        }
    }

    /* PDUTYPE_DATAPDU */
    void process_data(InStream & stream, Callback & cb)
    {
        if (bool(this->verbose & Verbose::basic_trace4)) {
            LOG(LOG_INFO, "Front::process_data");
        }
        unsigned expected;
        ShareData_Recv sdata_in(stream, nullptr);
        if (bool(this->verbose & Verbose::basic_trace4)) {
            LOG(LOG_INFO, "Front::process_data: sdata_in.pdutype2=%" PRIu8
                          " sdata_in.len=%" PRIu16
                          " sdata_in.compressedLen=%" PRIu16
                          " remains=%zu"
                          " payload_len=%zu",
                sdata_in.pdutype2,
                sdata_in.len,
                sdata_in.compressedLen,
                stream.in_remain(),
                sdata_in.payload.get_capacity()
            );
        }

        switch (sdata_in.pdutype2) {
        case PDUTYPE2_UPDATE:  // Update PDU (section 2.2.9.1.1.3)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_UPDATE");
            }
            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;
        case PDUTYPE2_CONTROL: // 20(0x14) Control PDU (section 2.2.1.15.1)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_CONTROL");
            }
            {
                expected = 8;   /* action(2) + grantId(2) + controlId(4) */
                if (!sdata_in.payload.in_check_rem(expected)) {
                    LOG(LOG_ERR, "Front::process_data: Truncated Control PDU data, need=%u remains=%zu",
                        expected, sdata_in.payload.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }

                int action = sdata_in.payload.in_uint16_le();
                sdata_in.payload.in_skip_bytes(2); /* user id */
                sdata_in.payload.in_skip_bytes(4); /* control id */
                switch (action) {
                    case RDP_CTL_REQUEST_CONTROL:
                        this->send_control(RDP_CTL_GRANT_CONTROL);
                    break;
                    case RDP_CTL_COOPERATE:
                        this->send_control(RDP_CTL_COOPERATE);
                    break;
                    default:
                        LOG(LOG_WARNING, "Front::process_data: DATA_PDU_CONTROL unknown action (%d)\n", action);
                }
            }
            break;
        case PDUTYPE2_POINTER: // Pointer Update PDU (section 2.2.9.1.1.4)
            if (bool(this->verbose & Verbose::basic_trace3)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_POINTER");
            }
            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;
        case PDUTYPE2_INPUT:   // 28(0x1c) Input PDU (section 2.2.8.1.1.3)
            {
                SlowPath::ClientInputEventPDU_Recv cie(sdata_in.payload);

                if (bool(this->verbose & Verbose::basic_trace3)) {
                    LOG(LOG_INFO, "Front::process_data: PDUTYPE2_INPUT num_events=%u", cie.numEvents);
                }

                for (int index = 0; index < cie.numEvents; index++) {
                    SlowPath::InputEvent_Recv ie(cie.payload);

                    // TODO we should always call send_input with original data  if the other side is rdp it will merely transmit it to the other end without change. If the other side is some internal module it will be it's own responsibility to decode it
                    // TODO with the scheme above  any kind of keymap management is only necessary for internal modules or if we convert mapping. But only the back-end module really knows what the target mapping should be.
                    switch (ie.messageType) {
                        case SlowPath::INPUT_EVENT_SYNC:
                        {
                            SlowPath::SynchronizeEvent_Recv se(ie.payload);

                            if (bool(this->verbose & Verbose::basic_trace3)) {
                                LOG(LOG_INFO, "Front::process_data: Slow-Path INPUT_EVENT_SYNC eventTime=%u toggleFlags=0x%04X",
                                    ie.eventTime, se.toggleFlags);
                            }
                            LOG(LOG_INFO, "Front::process_data: (Slow-Path) Synchronize Event toggleFlags=0x%X",
                                se.toggleFlags);

                            // happens when client gets focus and sends key modifier info
                            this->keymap.synchronize(se.toggleFlags & 0xFFFF);
                            if (this->up_and_running) {
                                cb.rdp_input_synchronize(ie.eventTime, 0, se.toggleFlags & 0xFFFF, (se.toggleFlags & 0xFFFF0000) >> 16);
                                this->has_user_activity = true;
                            }
                        }
                        break;

                        case SlowPath::INPUT_EVENT_MOUSE:
                        {
                            SlowPath::MouseEvent_Recv me(ie.payload);

                            if (bool(this->verbose & Verbose::basic_trace3)) {
                                LOG(LOG_INFO, "Front::process_data: Slow-Path INPUT_EVENT_MOUSE eventTime=%u pointerFlags=0x%04X, xPos=%u, yPos=%u)",
                                    ie.eventTime, me.pointerFlags, me.xPos, me.yPos);
                            }
                            this->mouse_x = me.xPos;
                            this->mouse_y = me.yPos;
                            if (this->up_and_running) {
                                cb.rdp_input_mouse(me.pointerFlags, me.xPos, me.yPos, &this->keymap);
                                this->has_user_activity = true;
                            }

                            if ((me.pointerFlags & (SlowPath::PTRFLAGS_BUTTON1 |
                                                    SlowPath::PTRFLAGS_BUTTON2 |
                                                    SlowPath::PTRFLAGS_BUTTON3)) &&
                                !(me.pointerFlags & SlowPath::PTRFLAGS_DOWN)) {
                                if (this->capture) {
                                    this->capture->possible_active_window_change();
                                }
                            }
                        }
                        break;

                        // XFreeRDP sends this message even if its support is not advertised in the Input Capability Set.
                        case SlowPath::INPUT_EVENT_MOUSEX:
                        {
                            SlowPath::ExtendedMouseEvent_Recv me(ie.payload);

                            LOG(LOG_WARNING, "Front::process_data: Unexpected Slow-Path INPUT_EVENT_MOUSEX eventTime=%u pointerFlags=0x%04X, xPos=%u, yPos=%u)",
                                ie.eventTime, me.pointerFlags, me.xPos, me.yPos);

                            if (this->up_and_running) {
                                this->has_user_activity = true;
                            }

                            if ((me.pointerFlags & (SlowPath::PTRXFLAGS_BUTTON1 |
                                                    SlowPath::PTRXFLAGS_BUTTON2)) &&
                                !(me.pointerFlags & SlowPath::PTRXFLAGS_DOWN)) {
                                if (this->capture) {
                                    this->capture->possible_active_window_change();
                                }
                            }
                        }
                        break;

                        case SlowPath::INPUT_EVENT_SCANCODE:
                        {
                            SlowPath::KeyboardEvent_Recv ke(ie.payload);

                            if (bool(this->verbose & Verbose::basic_trace3)) {
                                LOG(LOG_INFO, "Front::process_data: Slow-Path INPUT_EVENT_SCANCODE eventTime=%u keyboardFlags=0x%04X keyCode=0x%04X",
                                    ie.eventTime, ke.keyboardFlags, ke.keyCode);
                            }

                            this->input_event_scancode(ke, cb, ie.eventTime);
                        }
                        break;

                        case SlowPath::INPUT_EVENT_UNICODE:
                        {
                            SlowPath::UnicodeKeyboardEvent_Recv uke(ie.payload);

                            if (bool(this->verbose & Verbose::basic_trace3)) {
                                LOG(LOG_INFO, "Front::process_data: Slow-Path INPUT_EVENT_UNICODE eventTime=%u unicodeCode=0x%04X",
                                    ie.eventTime, uke.unicodeCode);
                            }
                            // happens when client gets focus and sends key modifier info
                            if (this->up_and_running) {
                                cb.rdp_input_unicode(uke.unicodeCode, uke.keyboardFlags);
                                this->has_user_activity = true;
                            }
                        }
                        break;

                        default:
                            LOG(LOG_WARNING, "Front::process_data: Unsupported PDUTYPE2_INPUT message type %u", ie.messageType);
                        break;
                    }
                }
                if (bool(this->verbose & Verbose::basic_trace3)) {
                    LOG(LOG_INFO, "Front::process_data: PDUTYPE2_INPUT done");
                }
            }
        break;
        case PDUTYPE2_SYNCHRONIZE:  // Synchronize PDU (section 2.2.1.14.1)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_SYNCHRONIZE");
            }
            {
                expected = 4;   /* messageType(2) + targetUser(4) */
                if (!sdata_in.payload.in_check_rem(expected)) {
                    LOG(LOG_ERR, "Front::process_data: Truncated Synchronize PDU data, need=%u remains=%zu",
                        expected, sdata_in.payload.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }

                uint16_t messageType = sdata_in.payload.in_uint16_le();
                uint16_t controlId = sdata_in.payload.in_uint16_le();
                if (bool(this->verbose & Verbose::basic_trace4)) {
                    LOG(LOG_INFO, "Front::process_data: PDUTYPE2_SYNCHRONIZE"
                                  " messageType=%u controlId=%u",
                                  static_cast<unsigned>(messageType),
                                  static_cast<unsigned>(controlId));
                }
                this->send_synchronize();
            }
        break;
        case PDUTYPE2_REFRESH_RECT: // Refresh Rect PDU (section 2.2.11.2.1)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_REFRESH_RECT");
            }
            // numberOfAreas (1 byte): An 8-bit, unsigned integer. The number of Inclusive Rectangle
            // (section 2.2.11.1) structures in the areasToRefresh field.

            // pad3Octects (3 bytes): A 3-element array of 8-bit, unsigned integer values. Padding.
            // Values in this field MUST be ignored.

            // areasToRefresh (variable): An array of TS_RECTANGLE16 structures (variable number of
            // bytes). Array of screen area Inclusive Rectangles to redraw. The number of rectangles
            // is given by the numberOfAreas field.

            // 2.2.11.1 Inclusive Rectangle (TS_RECTANGLE16)
            // =============================================
            // The TS_RECTANGLE16 structure describes a rectangle expressed in inclusive coordinates
            // (the right and bottom coordinates are include " in the rectangle bounds).
            // left (2 bytes): A 16-bit, unsigned integer. The leftmost bound of the rectangle.
            // top (2 bytes): A 16-bit, unsigned integer. The upper bound of the rectangle.
            // right (2 bytes): A 16-bit, unsigned integer. The rightmost bound of the rectangle.
            // bottom (2 bytes): A 16-bit, unsigned integer. The lower bound of the rectangle.

            {
                expected = 4;   /* numberOfAreas(1) + pad3Octects(3) */
                if (!sdata_in.payload.in_check_rem(expected)) {
                    LOG(LOG_ERR, "Front::process_data: Truncated Refresh rect PDU data, need=%u remains=%zu",
                        expected, sdata_in.payload.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }

                size_t numberOfAreas = sdata_in.payload.in_uint8();
                sdata_in.payload.in_skip_bytes(3);

                expected = numberOfAreas * 8;   /* numberOfAreas * (left(2) + top(2) + right(2) + bottom(2)) */
                if (!sdata_in.payload.in_check_rem(expected)) {
                    LOG(LOG_ERR, "Front::process_data: Truncated Refresh rect PDU data, need=%u remains=%zu",
                        expected, sdata_in.payload.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }

                auto rects_raw = std::make_unique<Rect[]>(numberOfAreas);
                array_view<Rect> rects(rects_raw.get(), numberOfAreas);
                for (Rect & rect : rects) {
                    int left = sdata_in.payload.in_uint16_le();
                    int top = sdata_in.payload.in_uint16_le();
                    int right = sdata_in.payload.in_uint16_le();
                    int bottom = sdata_in.payload.in_uint16_le();
                    rect = Rect(left, top, (right - left) + 1, (bottom - top) + 1);
                    if (bool(this->verbose & Verbose::basic_trace4)) {

                    LOG(LOG_INFO, "Front::process_data: PDUTYPE2_REFRESH_RECT"
                            " left=%d top=%d right=%d bottom=%d cx=%u cy=%u",
                            left, top, right, bottom, rect.cx, rect.cy);
                    }
                    // // TODO we should consider adding to API some function to refresh several rects at once
                    // if (this->up_and_running) {
                    //     cb.rdp_input_invalidate(rect);
                    // }
                }
                cb.rdp_input_invalidate2(rects);
            }
        break;
        case PDUTYPE2_PLAY_SOUND:   // Play Sound PDU (section 2.2.9.1.1.5.1):w
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_PLAY_SOUND");
            }
            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;
        case PDUTYPE2_SUPPRESS_OUTPUT:  // Suppress Output PDU (section 2.2.11.3.1)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_SUPPRESS_OUTPUT");
            }
            // PDUTYPE2_SUPPRESS_OUTPUT comes when minimizing a full screen
            // mstsc.exe 2600. I think this is saying the client no longer wants
            // screen updates and it will issue a PDUTYPE2_REFRESH_RECT above
            // to catch up so minimized apps don't take bandwidth
            {
                RDP::SuppressOutputPDUData sopdud;

                sopdud.receive(sdata_in.payload);
                //sopdud.log(LOG_INFO);

                if (this->ini.get<cfg::client::enable_suppress_output>()) {
                    if (RDP::ALLOW_DISPLAY_UPDATES == sopdud.get_allowDisplayUpdates()) {
                        cb.rdp_allow_display_updates(sopdud.get_left(), sopdud.get_top(),
                            sopdud.get_right(), sopdud.get_bottom());
                    }
                    else {
                        cb.rdp_suppress_display_updates();
                    }
                }
            }
            break;

        break;
        case PDUTYPE2_SHUTDOWN_REQUEST: // Shutdown Request PDU (section 2.2.2.2.1)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_SHUTDOWN_REQUEST");
            }
            {
                // when this message comes, send a PDUTYPE2_SHUTDOWN_DENIED back
                // so the client is sure the connection is alive and it can ask
                // if user really wants to disconnect

                StaticOutReservedStreamHelper<1024, 65536-1024> stream;

                const uint32_t log_condition = (128 | 8);
                ::send_share_data_ex( this->trans
                                    , PDUTYPE2_SHUTDOWN_DENIED
                                    , (bool(this->ini.get<cfg::client::rdp_compression>()) ? this->client_info.rdp_compression : 0)
                                    , this->mppc_enc.get()
                                    , this->share_id
                                    , this->encryptionLevel
                                    , this->encrypt
                                    , this->userid
                                    , stream
                                    , log_condition
                                    , underlying_cast(this->verbose)
                                    );
            }
        break;
        case PDUTYPE2_SHUTDOWN_DENIED:  // Shutdown Request Denied PDU (section 2.2.2.3.1)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_SHUTDOWN_DENIED");
            }
            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;
        case PDUTYPE2_SAVE_SESSION_INFO: // Save Session Info PDU (section 2.2.10.1.1)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_SAVE_SESSION_INFO");
            }
            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;
        case PDUTYPE2_FONTLIST: // 39(0x27) Font List PDU (section 2.2.1.18.1)
        {
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_FONTLIST");
            }
        // 2.2.1.18.1 Font List PDU Data (TS_FONT_LIST_PDU)
        // ================================================
        // The TS_FONT_LIST_PDU structure contains the contents of the Font
        // List PDU, which is a Share Data Header (section 2.2.8.1.1.1.2) and
        // four fields.

        // shareDataHeader (18 bytes): Share Data Header (section 2.2.8.1.1.1.2)
        // containing information about the packet. The type subfield of the
        // pduType field of the Share Control Header (section 2.2.8.1.1.1.1)
        // MUST be set to PDUTYPE_DATAPDU (7). The pduType2 field of the Share
        // Data Header MUST be set to PDUTYPE2_FONTLIST (39).

        // numberFonts (2 bytes): A 16-bit, unsigned integer. The number of
        // fonts. This field SHOULD be set to 0.

        // totalNumFonts (2 bytes): A 16-bit, unsigned integer. The total number
        // of fonts. This field SHOULD be set to 0.

        // listFlags (2 bytes): A 16-bit, unsigned integer. The sequence flags.
        // This field SHOULD be set to 0x0003, which is the logical OR'ed value
        // of FONTLIST_FIRST (0x0001) and FONTLIST_LAST (0x0002).

        // entrySize (2 bytes): A 16-bit, unsigned integer. The entry size. This
        // field SHOULD be set to 0x0032 (50 bytes).

            expected = 8;   /* numberFonts(2) + totalNumFonts(2) + listFlags(2) + entrySize(2) */
            if (!sdata_in.payload.in_check_rem(expected)) {
                LOG(LOG_ERR, "Front::process_data: Truncated Font list PDU data, need=%u remains=%zu",
                    expected, sdata_in.payload.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            sdata_in.payload.in_uint16_le(); /* numberFont -> 0*/
            sdata_in.payload.in_uint16_le(); /* totalNumFonts -> 0 */
            int seq = sdata_in.payload.in_uint16_le();
            sdata_in.payload.in_uint16_le(); /* entrySize -> 50 */

            /* 419 client sends Seq 1, then 2 */
            /* 2600 clients sends only Seq 3 */
            /* after second font message, we are up and running */
            if (seq == 2 || seq == 3)
            {
                this->send_fontmap();
                this->send_data_update_sync();

                if (this->client_info.screen_info.bpp == BitsPerPixel{8}) {
                    RDPColCache cmd(0, BGRPalette::classic_332());
                    this->orders.graphics_update_pdu().draw(cmd);
                }

                if (bool(this->verbose & (Verbose::basic_trace4 | Verbose::basic_trace))) {
                    LOG(LOG_INFO, "Front::process_data: --------------> UP AND RUNNING <--------------");
                }

                if (this->capture && this->capture->has_graphic_api()) {
                    this->set_gd(this->capture.get());
                }
                else {
                    this->set_gd(this->orders.graphics_update_pdu());
                }

                this->up_and_running = true;
                this->handshake_timeout.reset();
                cb.rdp_input_up_and_running();
                // TODO we should use accessors to set that, also not sure it's the right place to set it
                this->ini.set_acl<cfg::context::opt_width>(this->client_info.screen_info.width);
                this->ini.set_acl<cfg::context::opt_height>(this->client_info.screen_info.height);
                this->ini.set_acl<cfg::context::opt_bpp>(safe_int(this->client_info.screen_info.bpp));

                if (!this->auth_info_sent) {
                    char         username_a_domain[516];
                    const char * username;
                    if (this->client_info.domain[0] &&
                        !strchr(this->client_info.username, '@') &&
                        !strchr(this->client_info.username, '\\')) {
                        snprintf(username_a_domain, sizeof(username_a_domain), "%s@%s", this->client_info.username, this->client_info.domain);
                        username = username_a_domain;
                    }
                    else {
                        username = this->client_info.username;
                    }

                    LOG(LOG_INFO, "Front::process_data: asking for selector");
                    this->ini.set_acl<cfg::globals::auth_user>(username);
                    this->ini.ask<cfg::context::selector>();
                    this->ini.ask<cfg::globals::target_user>();
                    this->ini.ask<cfg::globals::target_device>();
                    this->ini.ask<cfg::context::target_protocol>();
                    if (this->client_info.password[0]) {
                        this->ini.set_acl<cfg::context::password>(this->client_info.password);
                    }

                    this->auth_info_sent = true;
                }

                if (BitsPerPixel{8} != this->mod_bpp) {
                    this->send_palette();
                }

                // if (this->client_info.remote_program && this->rail_channel_id) {
                //     CHANNELS::ChannelDef const* rail_channel = this->channel_list.get_by_id(this->rail_channel_id);

                //     StaticOutStream<64> rail_handshake_pdu_stream;

                //     RAILPDUHeader_Send raid_pdu_header_send(rail_handshake_pdu_stream);

                //     raid_pdu_header_send.emit_begin(TS_RAIL_ORDER_HANDSHAKE);

                //     HandshakePDU_Send(rail_handshake_pdu_stream, 0x1771);

                //     raid_pdu_header_send.emit_end();

                //     this->send_to_channel(
                //         *rail_channel,
                //         rail_handshake_pdu_stream.get_data(),
                //         rail_handshake_pdu_stream.get_offset(),
                //         rail_handshake_pdu_stream.get_offset(),
                //         CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST);
                // }
            }
        }
        break;
        case PDUTYPE2_FONTMAP:  // Font Map PDU (section 2.2.1.22.1)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_FONTMAP");
            }
            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;
        case PDUTYPE2_SET_KEYBOARD_INDICATORS: // Set Keyboard Indicators PDU (section 2.2.8.2.1.1)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_SET_KEYBOARD_INDICATORS");
            }
            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;
        case PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST: // Persistent Key List PDU (section 2.2.1.17.1)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST");
            }

            if (this->ini.get<cfg::client::persistent_disk_bitmap_cache>() &&
                this->orders.bmp_cache_persister()) {
                RDP::PersistentKeyListPDUData pklpdud;

                pklpdud.receive(sdata_in.payload);
                if (bool(this->verbose & Verbose::basic_trace4)) {
                    pklpdud.log(LOG_INFO, "Front::process_data: Receiving from client");
                }

                // TODO mutable and static is a bad idea
                static uint16_t cache_entry_index[BmpCache::MAXIMUM_NUMBER_OF_CACHES] = { 0, 0, 0, 0, 0 };

                RDP::BitmapCachePersistentListEntry * entries = pklpdud.entries;

                for (unsigned i = 0; i < BmpCache::MAXIMUM_NUMBER_OF_CACHES; ++i) {
                    if (pklpdud.numEntriesCache[i]) {
                        this->orders.bmp_cache_persister()->process_key_list(
                            i, entries, pklpdud.numEntriesCache[i] , cache_entry_index[i]
                        );
                        entries              += pklpdud.numEntriesCache[i];
                        cache_entry_index[i] += pklpdud.numEntriesCache[i];
                    }
                }

                if (pklpdud.bBitMask & RDP::PERSIST_LAST_PDU) {
                    this->orders.clear_bmp_cache_persister();
                }
            }

            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;
        case PDUTYPE2_BITMAPCACHE_ERROR_PDU: // Bitmap Cache Error PDU (see [MS-RDPEGDI] section 2.2.2.3.1)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_BITMAPCACHE_ERROR_PDU");
            }
            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;
        case PDUTYPE2_SET_KEYBOARD_IME_STATUS: // Set Keyboard IME Status PDU (section 2.2.8.2.2.1)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_SET_KEYBOARD_IME_STATUS");
            }
            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;
        case PDUTYPE2_OFFSCRCACHE_ERROR_PDU: // Offscreen Bitmap Cache Error PDU (see [MS-RDPEGDI] section 2.2.2.3.2)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_OFFSCRCACHE_ERROR_PDU");
            }
            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;
        case PDUTYPE2_SET_ERROR_INFO_PDU: // Set Error Info PDU (section 2.2.5.1.1)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_SET_ERROR_INFO_PDU");
            }
            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;
        case PDUTYPE2_DRAWNINEGRID_ERROR_PDU: // DrawNineGrid Cache Error PDU (see [MS-RDPEGDI] section 2.2.2.3.3)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_DRAWNINEGRID_ERROR_PDU");
            }
            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;
        case PDUTYPE2_DRAWGDIPLUS_ERROR_PDU: // GDI+ Error PDU (see [MS-RDPEGDI] section 2.2.2.3.4)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_DRAWGDIPLUS_ERROR_PDU");
            }
            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;
        case PDUTYPE2_ARC_STATUS_PDU: // Auto-Reconnect Status PDU (section 2.2.4.1.1)
            if (bool(this->verbose & Verbose::basic_trace4)) {
                LOG(LOG_INFO, "Front::process_data: PDUTYPE2_ARC_STATUS_PDU");
            }
            // TODO this quickfix prevents a tech crash, but consuming the data should be a better behaviour
            sdata_in.payload.in_skip_bytes(sdata_in.payload.in_remain());
        break;

        default:
            LOG(LOG_WARNING, "Front::process_data: Unsupported PDUTYPE in process_data %d\n", sdata_in.pdutype2);
            break;
        }

        stream.in_skip_bytes(sdata_in.payload.get_current() - stream.get_current());

        if (bool(this->verbose & Verbose::basic_trace4)) {
            LOG(LOG_INFO, "Front::process_data: done");
        }
    }

    void send_deactive()
    {
        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_deactive");
        }

        this->send_data_indication_ex_impl(
            GCC::MCS_GLOBAL_CHANNEL,
            [&](StreamSize<256>, OutStream & stream) {
                ShareControl_Send(stream, PDUTYPE_DEACTIVATEALLPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, 0);
                if (bool(this->verbose & Verbose::global_channel)) {
                    LOG(LOG_INFO, "Front::send_deactive: Sec clear payload to send:");
                    hexdump_av_d(stream.get_bytes());
                }
            }
        );

        if (bool(this->verbose & Verbose::basic_trace)) {
            LOG(LOG_INFO, "Front::send_deactive: done");
        }
    }

    void set_keyboard_indicators(uint16_t LedFlags) override
    {
        this->keymap.toggle_caps_lock(LedFlags & SlowPath::TS_SYNC_CAPS_LOCK);
        this->keymap.toggle_scroll_lock(LedFlags & SlowPath::TS_SYNC_SCROLL_LOCK);
        this->keymap.toggle_num_lock(LedFlags & SlowPath::TS_SYNC_NUM_LOCK);
    }

protected:
    template<class Cmd, class... Args>
    void draw_impl(Cmd const & cmd, Rect clip, Args && ... args) {
        if (!clip.intersect(clip_from_cmd(cmd)).isempty()) {
            this->graphics_update->draw(cmd, clip, args...);
        }
    }

    void draw_impl(RDPScrBlt const & cmd, Rect clip) {
        Rect drect = clip.intersect(this->client_info.screen_info.width, this->client_info.screen_info.height).intersect(clip_from_cmd(cmd));
        if (!drect.isempty()) {
            const signed int deltax = static_cast<int16_t>(cmd.srcx) - cmd.rect.x;
            const signed int deltay = static_cast<int16_t>(cmd.srcy) - cmd.rect.y;



            int srcx = drect.x + deltax;
            int srcy = drect.y + deltay;

            if (srcx < 0) {
                drect.x  -= srcx;
                drect.cx += srcx;

                srcx = 0;
            }
            if (srcy < 0) {
                drect.y  -= srcy;
                drect.cy += srcy;

                srcy = 0;
            }

            Rect srect = Rect(srcx, srcy, drect.cx, drect.cy).intersect(
                this->client_info.screen_info.width, this->client_info.screen_info.height);

            drect.cx = srect.cx;
            drect.cy = srect.cy;



            this->graphics_update->draw(RDPScrBlt(drect, cmd.rop, srcx, srcy), clip);
        }
    }

    void draw_impl(RDPMemBlt const& cmd, Rect clip, Bitmap const & bitmap) {
        if (this->client_info.order_caps.orderSupport[TS_NEG_PATBLT_INDEX]) {
            this->priv_draw_memblt(cmd, clip, bitmap);
        }
        else {
            Rect dest_rect = clip.intersect(cmd.rect);
            auto drew_bitmap = [this](Bitmap const &bitmap, Rect const & rect) {
                RDPBitmapData bitmap_data;

                bitmap_data.dest_left = rect.x;
                bitmap_data.dest_top = rect.y;
                bitmap_data.dest_right = rect.x + rect.cx - 1;
                bitmap_data.dest_bottom = rect.y + rect.cy - 1;

                bitmap_data.width = bitmap.cx();
                bitmap_data.height = bitmap.cy();
                bitmap_data.bits_per_pixel = safe_int(bitmap.bpp());
                bitmap_data.flags = 0;

                bitmap_data.bitmap_length = bitmap.bmp_size();

                this->draw_impl(bitmap_data, bitmap);
            };
            if (!dest_rect.isempty()) {
                if (dest_rect == cmd.rect) {
                    Bitmap new_bitmap(bitmap, Rect(cmd.srcx, cmd.srcy, cmd.rect.cx, cmd.rect.cy));
                    drew_bitmap(new_bitmap, cmd.rect);
                }
                else {
                    Bitmap new_bitmap(bitmap, Rect(cmd.srcx + dest_rect.x - cmd.rect.x,
                        cmd.srcy + dest_rect.y - cmd.rect.y, dest_rect.cx, dest_rect.cy));
                    drew_bitmap(new_bitmap, dest_rect);
                }
            }
        }
    }

    void draw_impl(RDPMem3Blt const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bitmap) {
        this->priv_draw_memblt(cmd, clip, bitmap, color_ctx);
    }

    void draw_impl(RDPPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) {
        if (this->client_info.order_caps.orderSupport[TS_NEG_PATBLT_INDEX]) {
            this->priv_draw_and_update_cache_brush(cmd, clip, color_ctx);
        }
        else {
            LOG(LOG_WARNING, "Front::draw_impl(RDPPatBlt): This Primary Drawing Order is not supported by client!");
        }
    }

    void draw_impl(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) {
        this->priv_draw_and_update_cache_brush(cmd, clip, color_ctx);
    }

    Rect rail_window_rect;

    void draw_impl(RDP::RAIL::NewOrExistingWindow const & cmd) {
        this->graphics_update->draw(cmd);

        if (!this->capture &&
            (cmd.header.FieldsPresentFlags() & (RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET)) &&
            (cmd.NumVisibilityRects() == 1)) {
            this->rail_window_rect = static_cast<Rect>(cmd.VisibilityRects(0)).offset(
                    cmd.VisibleOffsetX(), cmd.VisibleOffsetY()
                );
        }
        else {
            this->rail_window_rect.empty();
        }
    }

    void draw_impl(RDP::RAIL::DeletedWindow const & cmd) {
        this->graphics_update->draw(cmd);

        this->rail_window_rect.empty();
    }

public:
    class GlyphTo24Bitmap
    {
        // TODO BGRArray<n>
        uint8_t raw_data[RDPSerializer::MAX_ORDERS_SIZE];

    public:
        uint8_t const * data() const noexcept { return this->raw_data; }

        GlyphTo24Bitmap(
            FontChar const & fc,
            const BGRColor color_fore,
            const BGRColor color_back) noexcept
        {
            assert(fc.width*fc.height*3 < int(sizeof(this->raw_data)));

            const uint8_t * fc_data = fc.data.get();

            for (int y = 0 ; y < fc.height; y++) {
                uint8_t fc_bit_mask = 128;
                for (int x = 0 ; x < fc.width; x++) {
                    if (!fc_bit_mask) {
                        fc_data++;
                        fc_bit_mask = 128;
                    }

                    const uint16_t xpix = x * 3;
                    const uint16_t ypix = y * fc.width * 3;

                    const BGRColor color = (fc_bit_mask & *fc_data) ? color_back : color_fore;
                    this->raw_data[xpix + ypix    ] = color.blue();
                    this->raw_data[xpix + ypix + 1] = color.green();
                    this->raw_data[xpix + ypix + 2] = color.red();

                    fc_bit_mask >>= 1;
                }
                fc_data++;
            }
        }
    };


protected:
    void draw_impl(RDPGlyphIndex const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache)
    {
        if (this->client_info.glyph_cache_caps.GlyphSupportLevel == GlyphCacheCaps::GLYPH_SUPPORT_NONE) {
            const bool has_delta_bytes = (!cmd.ui_charinc && !(cmd.fl_accel & 0x20));
            const BGRColor color_fore = color_decode(cmd.fore_color, color_ctx);
            const BGRColor color_back = color_decode(cmd.back_color, color_ctx);

            uint16_t draw_pos_ref = 0;
            InStream variable_bytes(cmd.data, cmd.data_len);

            this->draw(RDPOpaqueRect(cmd.bk, cmd.fore_color), clip, color_ctx);

            while (variable_bytes.in_remain()) {
                uint8_t data = variable_bytes.in_uint8();
                if (data <= 0xFD) {
                    FontChar const & fc = gly_cache.glyphs[cmd.cache_id][data].font_item;
                    assert(fc);

                    if (has_delta_bytes) {
                        data = variable_bytes.in_uint8();
                        if (data == 0x80) {
                            draw_pos_ref += variable_bytes.in_uint16_le();
                        }
                        else {
                            draw_pos_ref += data;
                        }
                    }

                    if (fc) {
                        const int16_t x = cmd.bk.x + draw_pos_ref;
                        const int16_t y = cmd.bk.y;

                        contiguous_sub_rect_f(CxCy{fc.width, fc.height}, SubCxCy{64, 64}, [&](Rect rect){
                            GlyphTo24Bitmap glyphBitmap(fc, color_fore, color_back);

                            RDPBitmapData rdpbd;
                            rdpbd.dest_left      = rect.x + x;
                            rdpbd.dest_top       = rect.y + y + fc.offsety;
                            rdpbd.dest_right     = rect.cx + rect.x + x - 1;
                            rdpbd.dest_bottom    = rect.cy + rect.y + y + fc.offsety - 1;
                            rdpbd.bits_per_pixel = 24;
                            rdpbd.flags          = NO_BITMAP_COMPRESSION_HDR | BITMAP_COMPRESSION; /*NOLINT*/
                            rdpbd.bitmap_length  = rect.cx * rect.cy * 3;

                            const Rect tile(0, 0, rect.cx, rect.cy);
                            Bitmap bmp(glyphBitmap.data(), fc.width, fc.height, BitsPerPixel{24}, tile);

                            StaticOutStream<65535> bmp_stream;
                            bmp.compress(this->client_info.screen_info.bpp, bmp_stream);

                            rdpbd.width          = bmp.cx();
                            rdpbd.height         = bmp.cy();

                            this->draw_impl(rdpbd, bmp);
                        });
                    }

                    if (cmd.ui_charinc) {
                        draw_pos_ref += cmd.ui_charinc;
                    }
                }
                else if (data == 0xFE) {
                     LOG(LOG_WARNING, "Front::draw_impl(RDPGlyphIndex): Glyph fragment not implemented yet");
                }
                else if (data == 0xFF)  {
                    LOG(LOG_WARNING, "Front::draw_impl(RDPGlyphIndex): Glyph fragment not implemented yet");
                }
            }

        } else {
            this->priv_draw_and_update_cache_brush(cmd, clip, color_ctx, gly_cache);
        }
    }

    void draw_impl(RDPBitmapData const & bitmap_data, Bitmap const & bmp) {
        //LOG(LOG_INFO, "Front::draw(BitmapUpdate)");

        if (!this->ini.get<cfg::globals::enable_bitmap_update>() &&
            this->client_info.order_caps.orderSupport[TS_NEG_MEMBLT_INDEX]) {
            Rect boundary(bitmap_data.dest_left,
                          bitmap_data.dest_top,
                          bitmap_data.dest_right - bitmap_data.dest_left + 1,
                          bitmap_data.dest_bottom - bitmap_data.dest_top + 1
                         );

            this->draw(RDPMemBlt(0, boundary, 0xCC, 0, 0, 0), boundary, bmp);
        }
        else {
            this->graphics_update->draw(bitmap_data, bmp);
        }
    }

    void draw_impl(RDPLineTo const & cmd, Rect clip, gdi::ColorCtx color_ctx) {
        if (this->client_info.order_caps.orderSupport[TS_NEG_LINETO_INDEX]) {
            this->graphics_update->draw(cmd, clip, color_ctx);
        }
        else {
            if ((cmd.startx == cmd.endx) || (cmd.starty == cmd.endy)) {
                int16_t const min_x = std::min<int16_t>(cmd.startx, cmd.endx);
                int16_t const max_x = std::max<int16_t>(cmd.startx, cmd.endx);
                int16_t const min_y = std::min<int16_t>(cmd.starty, cmd.endy);
                int16_t const max_y = std::max<int16_t>(cmd.starty, cmd.endy);
                RDPOpaqueRect other_cmd(Rect(min_x, min_y, max_x - min_x + 1, max_y - min_y + 1), cmd.pen.color);
                this->draw_impl(other_cmd, clip, color_ctx);
            }
            else {
                LOG(LOG_WARNING, "Front::draw_impl(RDPLineTo): This Primary Drawing Order is not supported by client!");
            }
        }
    }

    void draw_impl(RDPOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx) {
        Rect dest_rect = cmd.rect.intersect(clip);
        if (dest_rect.isempty()) {
            return;
        }

        if (this->client_info.order_caps.orderSupport[TS_NEG_PATBLT_INDEX]) {
            this->graphics_update->draw(cmd, clip, color_ctx);
        }
        else {
            Rect image_rect = dest_rect;
            image_rect.cx = align4(dest_rect.cx);

            BitsPerPixel image_bpp = (bool(this->capture_bpp) ? this->capture_bpp : this->client_info.screen_info.bpp);

            size_t const serializer_max_data_block_size
              = this->orders.graphics_update_pdu().get_max_data_block_size();

            const uint16_t max_image_width
              = std::min<uint16_t>(
                    ((serializer_max_data_block_size / nb_bytes_per_pixel(image_bpp)) & ~3),
                    image_rect.cx
                );
            const uint16_t max_image_height = serializer_max_data_block_size / (max_image_width * nb_bytes_per_pixel(image_bpp));

            BGRColor order_color = color_decode(cmd.color, color_ctx);
            RDPColor image_color = color_encode(order_color, image_bpp);
            uint32_t pixel_color = ((nb_bytes_per_pixel(image_bpp) <= 2) ? image_color.as_bgr().to_u32() : BGRColor(image_color.as_rgb()).to_u32());

            std::vector<Bitmap> image_collection;

            auto get_image = [&image_collection](uint16_t width, uint16_t height, BitsPerPixel bpp, uint32_t color) -> Bitmap const & {
                    std::vector<Bitmap>::iterator iter = std::find_if(image_collection.begin(), image_collection.end(),
                        [width, height](Bitmap const & bitmap) {
                                return ((bitmap.cx() == width) && (bitmap.cy() == height));
                            });
                    if (image_collection.end() != iter) {
                        return *iter;
                    }

                    unsigned int const Bpp = nb_bytes_per_pixel(bpp);

                    image_collection.emplace_back();

                    Bitmap & bitmap = image_collection.back();

                    Bitmap::PrivateData::Data & data = Bitmap::PrivateData::initialize(bitmap, bpp, width, height);

                    uint8_t * begin_ptr = data.get();
                    uint8_t * write_ptr = begin_ptr;
                    for (uint16_t i = 0; i < width; ++i, write_ptr += Bpp) {
                        memcpy(write_ptr, &color, Bpp);
                    }

                    unsigned int const line_size = data.line_size();

                    write_ptr = data.get() + line_size;
                    for (uint16_t i = 1; i < height; ++i, write_ptr += line_size) {
                        memcpy(write_ptr, begin_ptr, line_size);
                    }

                    StaticOutStream<65535> bmp_stream;
                    bitmap.compress(bpp, bmp_stream);

                    return bitmap;
                };

            for (uint32_t y = 0; y < image_rect.cy; y += max_image_height) {
                for (uint32_t x = 0; x < image_rect.cx; x += max_image_width) {
                    const uint16_t sub_image_width = std::min<uint16_t>(image_rect.cx - x, max_image_width);
                    const uint16_t sub_image_height = std::min<uint16_t>(image_rect.cy - y, max_image_height);

                    Bitmap const & sub_image = get_image(sub_image_width, sub_image_height, image_bpp, pixel_color);

                    RDPBitmapData sub_image_data;

                    sub_image_data.dest_left = dest_rect.x + x;
                    sub_image_data.dest_top  = dest_rect.y + y;

                    sub_image_data.dest_right = std::min<uint16_t>(sub_image_data.dest_left + sub_image_width - 1, dest_rect.x + dest_rect.cx - 1);
                    sub_image_data.dest_bottom = sub_image_data.dest_top + sub_image_height - 1;

                    sub_image_data.width = sub_image_width;
                    sub_image_data.height = sub_image_height;

                    sub_image_data.bits_per_pixel = safe_int(sub_image.bpp());
                    sub_image_data.flags = BITMAP_COMPRESSION | NO_BITMAP_COMPRESSION_HDR; /*NOLINT*/
                    sub_image_data.bitmap_length = sub_image.data_compressed().size();

                    this->draw_impl(sub_image_data, sub_image);
                }
            }
        }
    }

    template<class Cmd>
    void draw_impl(Cmd const & cmd) {
        this->graphics_update->draw(cmd);
    }

    void draw_impl(RDP::FrameMarker const & order) {
        this->gd->draw(order);
    }

    void draw_impl(RDPColCache const & cmd) {
        this->orders.graphics_update_pdu().draw(cmd);
    }

    void draw_impl(RDPBrushCache const & /*unused*/) {
        // TODO unimplemented
    }

private:
    template<class Cmd, class... Args>
    void priv_draw_and_update_cache_brush(Cmd const & cmd, Rect clip, Args const & ... args) {
        if (!clip.intersect(clip_from_cmd(cmd)).isempty()) {
            if (this->updatable_cache_brush(cmd.brush)) {
                Cmd new_cmd = cmd;
                // this change the brush and send it to to remote cache
                this->update_cache_brush(new_cmd.brush);
                this->graphics_update->draw(new_cmd, clip, args...);
            }
            else {
              this->graphics_update->draw(cmd, clip, args...);
            }
        }
    }

    void draw_tile(Rect dst_tile, Rect src_tile, const RDPMemBlt & cmd, const Bitmap & bitmap, Rect clip)
    {
        if (bool(this->verbose & Verbose::graphic)) {
            LOG(LOG_INFO, "Front::draw_tile(MemBlt)((%d, %d, %u, %u) (%d, %d, %u, %u))",
                 dst_tile.x, dst_tile.y, dst_tile.cx, dst_tile.cy,
                 src_tile.x, src_tile.y, src_tile.cx, src_tile.cy);
        }

        const Bitmap tiled_bmp(bitmap, src_tile);
        const RDPMemBlt cmd2(0, dst_tile, cmd.rop, 0, 0, 0);

        this->graphics_update->draw(cmd2, clip, tiled_bmp);
    }

    void draw_tile(Rect dst_tile, Rect src_tile, const RDPMem3Blt & cmd, const Bitmap & bitmap, Rect clip, gdi::ColorCtx color_ctx)
    {
        if (bool(this->verbose & Verbose::graphic)) {
            LOG(LOG_INFO, "Front::draw_tile(Mem3Blt)((%d, %d, %u, %u) (%d, %d, %u, %u)",
                 dst_tile.x, dst_tile.y, dst_tile.cx, dst_tile.cy,
                 src_tile.x, src_tile.y, src_tile.cx, src_tile.cy);
        }

        const Bitmap tiled_bmp(bitmap, src_tile);
        RDPMem3Blt cmd2(0, dst_tile, cmd.rop, 0, 0, cmd.back_color, cmd.fore_color, cmd.brush, 0);

        if (this->client_info.screen_info.bpp != this->mod_bpp) {
            const BGRColor back_color24 = color_decode(cmd.back_color, this->mod_bpp, this->mod_palette_rgb);
            const BGRColor fore_color24 = color_decode(cmd.fore_color, this->mod_bpp, this->mod_palette_rgb);

            cmd2.back_color = color_encode(back_color24, this->client_info.screen_info.bpp);
            cmd2.fore_color = color_encode(fore_color24, this->client_info.screen_info.bpp);
        }

        // this may change the brush add send it to to remote cache
        //this->cache_brush(cmd2.brush);

        this->graphics_update->draw(cmd2, clip, color_ctx, tiled_bmp);
    }

    template<class MemBlt, class... ColorCtx>
    void priv_draw_memblt(const MemBlt & cmd, Rect clip, const Bitmap & bitmap, ColorCtx... color_ctx)
    {
        if (bitmap.cx() < cmd.srcx || bitmap.cy() < cmd.srcy) {
            return;
        }

        const uint8_t palette_id = 0;
        if (this->client_info.screen_info.bpp == BitsPerPixel{8}) {
            if (!this->palette_memblt_sent[palette_id]) {
                RDPColCache cmd(palette_id, bitmap.palette());
                this->orders.graphics_update_pdu().draw(cmd);
                this->palette_memblt_sent[palette_id] = true;
            }
        }

        const uint16_t dst_x = cmd.rect.x;
        const uint16_t dst_y = cmd.rect.y;
        // clip dst as it can be larger than source bitmap
        const uint16_t dst_cx = std::min<uint16_t>(bitmap.cx() - cmd.srcx, cmd.rect.cx);
        const uint16_t dst_cy = std::min<uint16_t>(bitmap.cy() - cmd.srcy, cmd.rect.cy);

        // check if target bitmap can be fully stored inside one front cache entry
        // if so no need to tile it.
        uint32_t front_bitmap_size = nb_bytes_per_pixel(this->client_info.screen_info.bpp) * align4(dst_cx) * dst_cy;
        // even if cache seems to be large enough, cache entries cant be used
        // for values whose width is larger or equal to 256 after alignment
        // hence, we check for this case. There does not seem to exist any
        // similar restriction on cy actual reason of this is unclear
        // (I don't even know if it's related to redemption code or client code).
//        LOG(LOG_INFO, "cache1=%u cache2=%u cache3=%u bmp_size==%u",
//            this->client_info.cache1_size,
//            this->client_info.cache2_size,
//            this->client_info.cache3_size,
//            front_bitmap_size);
        size_t const serializer_max_data_block_size = [this]{
            Graphics::PrivateGraphicsUpdatePDU& graphics_update_pdu_ = this->orders.graphics_update_pdu();
            return graphics_update_pdu_.get_max_data_block_size();
        }();
        if (front_bitmap_size <= serializer_max_data_block_size
            && align4(dst_cx) < 128 && dst_cy < 128) {
            // clip dst as it can be larger than source bitmap
            const Rect dst_tile(dst_x, dst_y, dst_cx, dst_cy);
            const Rect src_tile(cmd.srcx, cmd.srcy, dst_cx, dst_cy);
            this->draw_tile(dst_tile, src_tile, cmd, bitmap, clip, color_ctx...);
        }
        else {
            // if not we have to split it
            const uint16_t TILE_CX = ((nb_bytes_per_pixel(this->client_info.screen_info.bpp) * 64 * 64 < serializer_max_data_block_size) ? 64 : 32);
            const uint16_t TILE_CY = TILE_CX;

            contiguous_sub_rect_f(CxCy{dst_cx, dst_cy}, SubCxCy{TILE_CX, TILE_CY}, [&](Rect r){
                const Rect dst_tile = r.offset(dst_x, dst_y);
                const Rect src_tile = r.offset(cmd.srcx, cmd.srcy);
                this->draw_tile(dst_tile, src_tile, cmd, bitmap, clip, color_ctx...);
            });
        }
    }

    bool updatable_cache_brush(RDPBrush const & brush) const {
        return brush.style == 3 && this->client_info.brush_cache_code == 1;
    }

    void cache_brush(RDPBrush & brush)
    {
        if (this->updatable_cache_brush(brush)) {
            this->update_cache_brush(brush);
        }
    }

    void update_cache_brush(RDPBrush & brush)
    {
        uint8_t pattern[8];
        pattern[0] = brush.hatch;
        memcpy(pattern+1, brush.extra, 7);
        int cache_idx = 0;
        if (BRUSH_TO_SEND == this->orders.add_brush(pattern, cache_idx)) {
            RDPBrushCache cmd(cache_idx, 1, 8, 8, 0x81,
                sizeof(this->orders.brush_at(cache_idx).pattern),
                this->orders.brush_at(cache_idx).pattern);
            this->orders.graphics_update_pdu().draw(cmd);
        }
        brush.hatch = cache_idx;
        brush.style = 0x81;
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

    void GeneratePaletteUpdateData(OutStream & stream) {
        // Payload
        stream.out_uint16_le(RDP_UPDATE_PALETTE);
        stream.out_uint16_le(0);

        stream.out_uint32_le(256); /* # of colors */
        for (BGRColor color : this->mod_palette_rgb) {
            // Palette entries is in BGR triplet format.
            stream.out_uint8(color.blue());
            stream.out_uint8(color.green());
            stream.out_uint8(color.red());
        }
    }

    bool palette_sent = false;

    void send_palette() {
        if (BitsPerPixel{8} != this->client_info.screen_info.bpp || this->palette_sent) {
            return ;
        }

        if (bool(this->verbose & Verbose::basic_trace3)) {
            LOG(LOG_INFO, "Front::send_palette");
        }

        StaticOutReservedStreamHelper<1024, 65536-1024> stream;
        GeneratePaletteUpdateData(stream.get_data_stream());

        ::send_server_update(
            this->trans
          , this->server_fastpath_update_support
          , (bool(this->ini.get<cfg::client::rdp_compression>()) ? this->client_info.rdp_compression : 0)
          , this->mppc_enc.get()
          , this->share_id
          , this->encryptionLevel
          , this->encrypt
          , this->userid
          , SERVER_UPDATE_GRAPHICS_PALETTE
          , 0
          , stream
          , underlying_cast(this->verbose)
        );

        this->sync();

        this->palette_sent = true;
    }

public:
    void sync() override {
        if (bool(this->verbose & Verbose::graphic)) {
            LOG(LOG_INFO, "Front::sync");
        }
        this->gd->sync();
    }

    void set_palette(const BGRPalette & palette) override {
        this->mod_palette_rgb = palette;

        this->gd->set_palette(palette);

        this->palette_sent = false;
        this->send_palette();
    }

private:
    template<class KeyboardEvent_Recv>
    void input_event_scancode(KeyboardEvent_Recv & ke, Callback & cb, long event_time) {
        bool tsk_switch_shortcuts;

        struct KeyboardFlags {
            static uint16_t get(SlowPath::KeyboardEvent_Recv const & ke) {
                return ke.keyboardFlags;
            }
            static uint16_t get(FastPath::KeyboardEvent_Recv const & ke) {
                return ke.spKeyboardFlags;
            }
        };

        Keymap2::DecodedKeys decoded_keys = this->keymap.event(
            KeyboardFlags::get(ke), ke.keyCode, tsk_switch_shortcuts);
        //LOG(LOG_INFO, "Decoded keyboard input data:");
        //hexdump_d(decoded_data.get_data(), decoded_data.size());

        bool const send_to_mod = [&]{
            if (!this->capture || 0 == decoded_keys.count) {
                return true;
            }
            auto const timeval = this->session_reactor.get_current_time();
            return (1 == decoded_keys.count
                    && this->capture->kbd_input(timeval, decoded_keys.uchars[0]))
                || (2 == decoded_keys.count
                    && this->capture->kbd_input(timeval, decoded_keys.uchars[0])
                    && this->capture->kbd_input(timeval, decoded_keys.uchars[1]));
            ;
        }();

        if (this->up_and_running) {
            if (tsk_switch_shortcuts && this->ini.get<cfg::client::disable_tsk_switch_shortcuts>()) {
                LOG(LOG_INFO, "Front::input_event_scancode: Ctrl+Alt+Del and Ctrl+Shift+Esc keyboard sequences ignored.");
            }
            else {
                if (send_to_mod) {
                    cb.rdp_input_scancode(ke.keyCode, 0, KeyboardFlags::get(ke), event_time, &this->keymap);
                }
                this->has_user_activity = true;
            }
        }

        if (this->keymap.is_application_switching_shortcut_pressed) {
            if (this->capture) {
                this->capture->possible_active_window_change();
            }
        }
    }

    void update_keyboard_input_mask_state() {
        const ::KeyboardInputMaskingLevel keyboard_input_masking_level =
            this->ini.get<cfg::session_log::keyboard_input_masking_level>();

        if (keyboard_input_masking_level == ::KeyboardInputMaskingLevel::unmasked) return;

        const bool mask_unidentified_data =
            ((keyboard_input_masking_level ==
                  ::KeyboardInputMaskingLevel::password_and_unidentified) ?
             (!this->session_probe_started_) : false);

        if (this->capture) {
            this->capture->enable_kbd_input_mask(
                    this->focus_on_password_textbox ||
                    ((keyboard_input_masking_level == ::KeyboardInputMaskingLevel::password_and_unidentified) &&
                     this->focus_on_unidentified_input_field) ||
                    this->consent_ui_is_visible || mask_unidentified_data
                );
        }
    }
};
