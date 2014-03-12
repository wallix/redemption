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
               Dominique Lafages, Raphael Zhou, Meng Tan
    Based on xrdp Copyright (C) Jay Sorg 2004-2010

    Front object (server), used to communicate with RDP client
*/

#ifndef _REDEMPTION_FRONT_FRONT_HPP_
#define _REDEMPTION_FRONT_FRONT_HPP_

#include "log.hpp"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "stream.hpp"
#include "transport.hpp"
#include "RDP/x224.hpp"
#include "RDP/nego.hpp"
#include "RDP/mcs.hpp"
#include "RDP/lic.hpp"
#include "RDP/logon.hpp"
#include "channel_list.hpp"
#include "RDP/gcc.hpp"
#include "RDP/sec.hpp"
#include "colors.hpp"
#include "RDP/capabilities.hpp"
#include "RDP/fastpath.hpp"
#include "RDP/slowpath.hpp"

#include "ssl_calls.hpp"
#include "bitfu.hpp"
#include "rect.hpp"
#include "region.hpp"
#include "capture.hpp"
#include "font.hpp"
#include "bitmap.hpp"
#include "RDP/caches/bmpcache.hpp"
#include "RDP/caches/fontcache.hpp"
#include "RDP/caches/pointercache.hpp"
#include "RDP/caches/brushcache.hpp"
#include "client_info.hpp"
#include "config.hpp"
#include "error.hpp"
#include "callback.hpp"
#include "colors.hpp"
#include "bitfu.hpp"
#include "confdescriptor.hpp"

#include "RDP/GraphicUpdatePDU.hpp"
#include "RDP/capabilities.hpp"
#include "RDP/SaveSessionInfoPDU.hpp"
#include "RDP/PersistentKeyListPDU.hpp"

#include "front_api.hpp"
#include "genrandom.hpp"

#include "auth_api.hpp"

enum {
    FRONT_DISCONNECTED,
    FRONT_CONNECTING,
    FRONT_RUNNING,
};

class Front : public FrontAPI {
    using FrontAPI::draw;
public:
    enum CaptureState {
          CAPTURE_STATE_UNKNOWN
        , CAPTURE_STATE_STARTED
        , CAPTURE_STATE_PAUSED
        , CAPTURE_STATE_STOPED
    } capture_state;
    Capture * capture;

    BmpCache * bmp_cache;
    GraphicsUpdatePDU * orders;
    Keymap2 keymap;
    CHANNELS::ChannelDefArray channel_list;
    int up_and_running;
    int share_id;
    struct ClientInfo client_info;
    uint32_t packet_number;
    Transport * trans;
    uint16_t userid;
    uint8_t pub_mod[512];
    uint8_t pri_exp[512];
    uint8_t server_random[32];
    CryptContext encrypt, decrypt;

    int order_level;
    Inifile * ini;
    uint32_t verbose;

    struct Font font;
    BrushCache brush_cache;
    PointerCache pointer_cache;
    GlyphCache glyph_cache;

    bool palette_sent;
    bool palette_memblt_sent[6];
    BGRPalette palette332;
    BGRPalette mod_palette;
    uint8_t mod_bpp;
    BGRPalette memblt_mod_palette;
    bool mod_palette_setted;

    enum {
        CONNECTION_INITIATION,
        WAITING_FOR_LOGON_INFO,
        WAITING_FOR_ANSWER_TO_LICENCE,
        ACTIVATE_AND_PROCESS_DATA,
    } state;

    Random * gen;

    bool fastpath_support;                    // choice of programmer
    bool client_fastpath_input_event_support; // = choice of programmer
    bool server_fastpath_update_support;      // choice of programmer + capability of client
    bool tls_client_active;
    bool mem3blt_support;
    int rdp_compression;
    int clientRequestedProtocols;

    uint32_t bitmap_update_count;

    GeneralCaps        client_general_caps;
    BitmapCaps         client_bitmap_caps;
    OrderCaps          client_order_caps;
    BmpCacheCaps       client_bmpcache_caps;
    OffScreenCacheCaps client_offscreencache_caps;
    BmpCache2Caps      client_bmpcache2_caps;
    bool               use_bitmapcache_rev2;

    redemption::string server_capabilities_filename;

    rdp_mppc_enc              * mppc_enc;
    rdp_mppc_enc_match_finder * mppc_enc_match_finder;

    auth_api * authentifier;

    Front ( Transport * trans
          , const char * default_font_name // SHARE_PATH "/" DEFAULT_FONT_NAME
          , Random * gen
          , Inifile * ini
          , bool fp_support // If true, fast-path must be supported
          , bool mem3blt_support
          , int rdp_compression
          , const char * server_capabilities_filename = ""
          )
        : FrontAPI(ini->globals.notimestamp, ini->globals.nomouse)
        , capture_state(CAPTURE_STATE_UNKNOWN)
        , capture(NULL)
        , bmp_cache(NULL)
        , orders(NULL)
        , up_and_running(0)
        , share_id(65538)
        , client_info(ini->globals.encryptionLevel, ini->client.bitmap_compression, ini->globals.bitmap_cache)
        , packet_number(1)
        , trans(trans)
        , userid(0)
        , order_level(0)
        , ini(ini)
        , verbose(this->ini->debug.front)
        , font(default_font_name)
        , brush_cache()
        , pointer_cache()
        , glyph_cache()
        , state(CONNECTION_INITIATION)
        , gen(gen)
        , fastpath_support(fp_support)
        , client_fastpath_input_event_support(fp_support)
        , server_fastpath_update_support(false)
        , tls_client_active(true)
        , mem3blt_support(mem3blt_support)
        , rdp_compression(rdp_compression)
        , clientRequestedProtocols(X224::PROTOCOL_RDP)
        , bitmap_update_count(0)
        , use_bitmapcache_rev2(false)
        , server_capabilities_filename(server_capabilities_filename)
        , mppc_enc(NULL)
        , mppc_enc_match_finder(NULL)
        , authentifier(NULL)
    {
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

        init_palette332(this->palette332);
        this->mod_palette_setted = false;
        this->palette_sent = false;
        for (size_t i = 0; i < 6 ; i++){
            this->palette_memblt_sent[i] = false;
        }

        // from server_sec
        // CGR: see if init has influence for the 3 following fields
        memset(this->server_random, 0, 32);

        // shared
        memset(this->decrypt.key, 0, 16);
        memset(this->encrypt.key, 0, 16);
        memset(this->decrypt.update_key, 0, 16);
        memset(this->encrypt.update_key, 0, 16);

        switch (this->client_info.encryptionLevel) {
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
    }

    ~Front(){
        if (this->mppc_enc) {
            delete this->mppc_enc;
        }
        if (this->mppc_enc_match_finder) {
            delete this->mppc_enc_match_finder;
        }

        if (this->bmp_cache) {
            this->save_persistent_disk_bitmap_cache();
            delete this->bmp_cache;
        }

        if (this->orders) {
            delete this->orders;
        }

        if (this->capture){
            delete this->capture;
        }
    }

    int server_resize(int width, int height, int bpp)
    {
        uint32_t res = 0;
        this->mod_bpp = bpp;
        if (bpp == 8){
            this->mod_palette_setted = false;
            this->palette_sent = false;
            for (size_t i = 0; i < 6 ; i++){
                this->palette_memblt_sent[i] = false;
            }
        }

        if (this->client_info.width != width
        || this->client_info.height != height) {
            /* older client can't resize */
            if (client_info.build <= 419) {
                LOG(LOG_WARNING, "Resizing is not available on older RDP clients");
                // resizing needed but not available
                res = -1;
            }
            else {
                LOG(LOG_INFO, "Resizing client to : %d x %d x %d", width, height, this->client_info.bpp);

                if (this->capture)
                {
                    CaptureState original_capture_state = this->capture_state;

                    auth_api * authentifier = this->authentifier;
                    this->stop_capture();
                    this->start_capture(width, height, *this->ini, authentifier);

                    this->capture_state = original_capture_state;
                }

                this->client_info.width = width;
                this->client_info.height = height;

                TODO("Why are we not calling this->flush() instead ? Looks dubious.")
                // send buffered orders
                this->orders->flush();

                // clear all pending orders, caches data, and so on and
                // start a send_deactive, send_deman_active process with
                // the new resolution setting
                /* shut down the rdp client */
                this->up_and_running = 0;
                this->send_deactive();
                /* this should do the actual resizing */
                this->send_demand_active();

                LOG(LOG_INFO, "Front::server_resize::ACTIVATED (resize)");
                state = ACTIVATE_AND_PROCESS_DATA;
                res = 1;
            }
        }

        // resizing not necessary
        return res;
    }

    void server_set_pointer(const Pointer & cursor)
    {
        int cache_idx = 0;
        switch (this->pointer_cache.add_pointer(cursor, cache_idx)) {
        case POINTER_TO_SEND:
            this->send_pointer(cache_idx, cursor);
        break;
        default:
        case POINTER_ALLREADY_SENT:
            this->set_pointer(cache_idx);
        break;
        }
    }

    virtual void text_metrics(const char * text, int & width, int & height){
        height = 0;
        width = 0;
        uint32_t uni[256];
        size_t len_uni = UTF8toUnicode(reinterpret_cast<const uint8_t *>(text), uni, sizeof(uni)/sizeof(uni[0]));
        if (len_uni){
            for (size_t index = 0; index < len_uni; index++) {
                uint32_t charnum = uni[index]; //
                FontChar *font_item = this->font.glyph_defined(charnum)?this->font.font_items[charnum]:NULL;
                if (!font_item) {
                    LOG(LOG_WARNING, "Front::text_metrics() - character not defined >0x%02x<", charnum);
                    font_item = this->font.font_items['?'];
                }
                width += font_item->incby;
                height = std::max(height, font_item->height);
            }
        }
    }

    TODO(" implementation of the server_draw_text function below is a small subset of possibilities text can be packed (detecting duplicated strings). See MS-RDPEGDI 2.2.2.2.1.1.2.13 GlyphIndex (GLYPHINDEX_ORDER)");
    virtual void server_draw_text(int16_t x, int16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
    {
        this->send_global_palette();

        // add text to glyph cache
        int len = strlen(text);
        TODO("we should put some loop here for text to be splitted between chunks of UTF8 characters and loop on them")
        if (len > 120) {
            len = 120;
        }

        if (len > 0){
            uint32_t uni[128];
            size_t part_len = UTF8toUnicode(reinterpret_cast<const uint8_t *>(text), uni, sizeof(uni)/sizeof(uni[0]));
            int total_width = 0;
            int total_height = 0;
            uint8_t data[256];
            int f = 7;
            int distance_from_previous_fragment = 0;
            for (size_t index = 0; index < part_len; index++) {
                int c = 0;
                uint32_t charnum = uni[index];
                FontChar *font_item = this->font.glyph_defined(charnum)?this->font.font_items[charnum]:NULL;
                if (!font_item) {
                    LOG(LOG_WARNING, "Front::text_metrics() - character not defined >0x%02x<", charnum);
                    font_item = this->font.font_items['?'];
                }
                TODO(" avoid passing parameters by reference to get results")
                switch (this->glyph_cache.add_glyph(font_item, f, c))
                {
                    case GlyphCache::GLYPH_ADDED_TO_CACHE:
                    {
                        RDPGlyphCache cmd(f, 1, c,
                            font_item->offset,
                            font_item->baseline,
                            font_item->width,
                            font_item->height,
                            font_item->data);

                        this->orders->draw(cmd);

                        if (  this->capture
                           && (this->capture_state == CAPTURE_STATE_STARTED)) {
                            this->capture->draw(cmd);
                        }
                    }
                    break;
                    default:
                    break;
                }
                data[index * 2] = c;
                data[index * 2 + 1] = distance_from_previous_fragment;
                distance_from_previous_fragment = font_item->incby;
                total_width += font_item->incby;
                total_height = std::max(total_height, font_item->height);
            }

            const Rect bk(x, y, total_width + 1, total_height + 1);

             RDPGlyphIndex glyphindex(
                f, // cache_id
                0x03, // fl_accel
                0x0, // ui_charinc
                1, // f_op_redundant,
                bgcolor, // bgcolor
                fgcolor, // fgcolor
                bk, // bk
                bk, // op
                // brush
                RDPBrush(0, 0, 3, 0xaa,
                    (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55"),
                x,  // glyph_x
                y + total_height, // glyph_y
                part_len * 2, // data_len in bytes
                data // data
            );

            this->draw(glyphindex, clip, NULL);

            x += total_width;
        }
    }

    // ===========================================================================
    void start_capture(int width, int height, Inifile & ini, auth_api * authentifier)
    {
        if (this->capture) {
            LOG(LOG_INFO, "Front::start_capture: session capture is already started");

            return;
        }

        if (ini.globals.movie.get()) {
            LOG(LOG_INFO, "---<>  Front::start_capture  <>---");
            struct timeval now = tvtime();

            if (this->verbose & 1) {
                LOG(LOG_INFO, "movie_path    = %s\n", ini.globals.movie_path.get_cstr());
                LOG(LOG_INFO, "codec_id      = %s\n", ini.globals.codec_id.get_cstr());
                LOG(LOG_INFO, "video_quality = %s\n", ini.globals.video_quality.get_cstr());
                LOG(LOG_INFO, "auth_user     = %s\n", ini.globals.auth_user.get_cstr());
                LOG(LOG_INFO, "host          = %s\n", ini.globals.host.get_cstr());
                LOG(LOG_INFO, "target_device = %s\n", ini.globals.target_device.get().c_str());
                LOG(LOG_INFO, "target_user   = %s\n", ini.globals.target_user.get_cstr());
            }

            char path[1024];
            char basename[1024];
            char extension[128];
            strcpy(path, WRM_PATH "/");     // default value, actual one should come from movie_path
            strcpy(basename, "redemption"); // default value actual one should come from movie_path
            strcpy(extension, "");          // extension is currently ignored
            bool res = true;
            res = canonical_path(ini.globals.movie_path.get_cstr(), path,
                                 sizeof(path), basename, sizeof(basename), extension,
                                 sizeof(extension));
            if (!res) {
                LOG(LOG_ERR, "Buffer Overflowed: Path too long");
                throw Error(ERR_RECORDER_FAILED_TO_FOUND_PATH);
            }
            this->capture = new Capture( now, width, height
                                       , ini.video.record_path
                                       , ini.video.record_tmp_path
                                       , ini.video.hash_path, basename
                                       , true
                                       , false
                                       , authentifier
                                       , ini
                                       );
            if (this->nomouse) {
                this->capture->set_pointer_display();
            }
            this->capture->capture_event.set();
            this->capture_state = CAPTURE_STATE_STARTED;

            this->authentifier = authentifier;
        }
    }

    void pause_capture() {
        LOG(LOG_INFO, "---<>  Front::pause_capture  <>---");
        if (this->capture_state != CAPTURE_STATE_STARTED) {
            return;
        }

        this->capture->pause();
        this->capture->capture_event.reset();
        this->capture_state = CAPTURE_STATE_PAUSED;
    }

    void resume_capture() {
        LOG(LOG_INFO, "---<>  Front::resume_capture <>---");
        if (this->capture_state != CAPTURE_STATE_PAUSED) {
            return;
        }

        this->capture->resume();
        this->capture->capture_event.set();
        this->capture_state = CAPTURE_STATE_STARTED;
    }

    void update_config(Inifile & ini){
        if (  this->capture
           && (this->capture_state == CAPTURE_STATE_STARTED)){
            this->capture->update_config(ini);
        }
    }

    void periodic_snapshot()
    {
        if (  this->capture
           && (this->capture_state == CAPTURE_STATE_STARTED)){
            struct timeval now = tvtime();
            this->capture->snapshot( now, this->mouse_x, this->mouse_y
                                   , false  // ignore frame in time interval
                                   );
        }
    }

    void stop_capture()
    {
        if (this->capture){
            LOG(LOG_INFO, "---<>   Front::stop_capture  <>---");
            this->authentifier = NULL;
            delete this->capture;
            this->capture = 0;

            this->capture_state = CAPTURE_STATE_STOPED;
        }
    }
    // ===========================================================================

    static int get_appropriate_compression_type(int client_supported_type, int front_supported_type)
    {
        if (((client_supported_type < PACKET_COMPR_TYPE_8K) || (client_supported_type > PACKET_COMPR_TYPE_RDP61)) ||
            ((front_supported_type  < PACKET_COMPR_TYPE_8K) || (front_supported_type  > PACKET_COMPR_TYPE_RDP61)))
            return -1;

        static int compress_type_selector[4][4] = {
            { PACKET_COMPR_TYPE_8K, PACKET_COMPR_TYPE_8K,  PACKET_COMPR_TYPE_8K,   PACKET_COMPR_TYPE_8K    },
            { PACKET_COMPR_TYPE_8K, PACKET_COMPR_TYPE_64K, PACKET_COMPR_TYPE_64K,  PACKET_COMPR_TYPE_64K   },
            { PACKET_COMPR_TYPE_8K, PACKET_COMPR_TYPE_64K, PACKET_COMPR_TYPE_RDP6, PACKET_COMPR_TYPE_RDP6  },
            { PACKET_COMPR_TYPE_8K, PACKET_COMPR_TYPE_64K, PACKET_COMPR_TYPE_RDP6, PACKET_COMPR_TYPE_RDP61 }
        };

        return compress_type_selector[client_supported_type][front_supported_type];
    }

    void save_persistent_disk_bitmap_cache() const {
        const char * client_persistent_path = PERSISTENT_PATH "/client";
        this->bmp_cache->save_all_to_disk(client_persistent_path);
    }

    virtual void reset(){
        if (this->verbose & 1){
            LOG(LOG_INFO, "Front::reset::use_bitmap_comp=%u", this->client_info.use_bitmap_comp);
            LOG(LOG_INFO, "Front::reset::use_compact_packets=%u", this->client_info.use_compact_packets);
            LOG(LOG_INFO, "Front::reset::bitmap_cache_version=%u", this->client_info.bitmap_cache_version);
        }

        if (this->mppc_enc) {
            delete this->mppc_enc;
            this->mppc_enc = NULL;
        }
        if (this->mppc_enc_match_finder) {
            delete this->mppc_enc_match_finder;
            this->mppc_enc_match_finder = NULL;
        }

        switch (Front::get_appropriate_compression_type(this->client_info.rdp_compression_type, this->rdp_compression - 1))
        {
        case PACKET_COMPR_TYPE_RDP61:
            if (this->verbose & 1) {
                LOG(LOG_INFO, "Front: Use RDP 6.1 Bulk compression");
            }
            this->mppc_enc_match_finder = new rdp_mppc_61_enc_hash_based_match_finder();
            //this->mppc_enc_match_finder = new rdp_mppc_61_enc_sequential_search_match_finder();
            this->mppc_enc = new rdp_mppc_61_enc(this->mppc_enc_match_finder);
            break;
        case PACKET_COMPR_TYPE_RDP6:
            if (this->verbose & 1) {
                LOG(LOG_INFO, "Front: Use RDP 6.0 Bulk compression");
            }
            this->mppc_enc = new rdp_mppc_60_enc();
            break;
        case PACKET_COMPR_TYPE_64K:
            if (this->verbose & 1) {
                LOG(LOG_INFO, "Front: Use RDP 5.0 Bulk compression");
            }
            this->mppc_enc = new rdp_mppc_50_enc();
            break;
        case PACKET_COMPR_TYPE_8K:
            if (this->verbose & 1) {
                LOG(LOG_INFO, "Front: Use RDP 4.0 Bulk compression");
            }
            this->mppc_enc = new rdp_mppc_40_enc();
            break;
        }

        // reset outgoing orders and reset caches
        if (this->bmp_cache) {
            this->save_persistent_disk_bitmap_cache();
            delete this->bmp_cache;
        }
        this->bmp_cache = new BmpCache(
                        this->client_info.bpp,
                        this->client_info.number_of_cache,
                        ((this->client_info.cache_flags & ALLOW_CACHE_WAITING_LIST_FLAG) &&
                             this->ini->client.cache_waiting_list),
                        this->client_info.cache1_entries,
                        this->client_info.cache1_size,
                        this->client_info.cache1_persistent,
                        this->client_info.cache2_entries,
                        this->client_info.cache2_size,
                        this->client_info.cache2_persistent,
                        this->client_info.cache3_entries,
                        this->client_info.cache3_size,
                        this->client_info.cache3_persistent,
                        this->client_info.cache4_entries,
                        this->client_info.cache4_size,
                        this->client_info.cache4_persistent,
                        this->client_info.cache5_entries,
                        this->client_info.cache5_size,
                        this->client_info.cache5_persistent/*, 8192*/);

        delete this->orders;
        this->orders = new GraphicsUpdatePDU(
              trans
            , this->userid
            , this->share_id
            , this->client_info.encryptionLevel
            , this->encrypt
            , *this->ini
            , this->client_info.bpp
            , *this->bmp_cache
            , this->client_info.bitmap_cache_version
            , this->client_info.use_bitmap_comp
            , this->client_info.use_compact_packets
            , this->server_fastpath_update_support
            , this->mppc_enc
            , this->rdp_compression ? this->client_info.rdp_compression : 0
            , this->rdp_compression ? this->client_info.rdp_compression_type : 0
            );

        this->pointer_cache.reset(this->client_info);
        this->brush_cache.reset(this->client_info);
        this->glyph_cache.reset(this->client_info);
    }

    void init_pointers()
    {
        Pointer pointer0(Pointer::POINTER_CURSOR0);
        this->pointer_cache.add_pointer_static(pointer0, 0);
        this->send_pointer(0, pointer0);

        Pointer pointer1(Pointer::POINTER_CURSOR1);
        this->pointer_cache.add_pointer_static(pointer1, 1);
        this->send_pointer(1, pointer1);
    }

    virtual void begin_update()
    {
        if (this->verbose & 64){
            LOG(LOG_INFO, "Front::begin_update()");
        }
        this->order_level++;
    }

    virtual void end_update()
    {
        if (this->verbose & 64) {
            LOG(LOG_INFO, "Front::end_update()");
        }
        this->order_level--;
        if (!this->up_and_running) {
            LOG(LOG_ERR, "Front is not up and running.");
            throw Error(ERR_RDP_EXPECTING_CONFIRMACTIVEPDU);
        }
        if (this->order_level == 0) {
            this->flush();
        }
    }

    void disconnect() throw (Error)
    {
        if (this->verbose & 1){
            LOG(LOG_INFO, "Front::disconnect()");
        }

        BStream x224_header(256);
        HStream mcs_data(256, 512);
        MCS::DisconnectProviderUltimatum_Send(mcs_data, 3, MCS::PER_ENCODING);
        X224::DT_TPDU_Send(x224_header,  mcs_data.size());

        this->trans->send(x224_header, mcs_data);
    }

    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const
    {
        return this->channel_list;
    }

    virtual void send_to_channel( const CHANNELS::ChannelDef & channel
                                , uint8_t * chunk
                                , size_t length
                                , size_t chunk_size
                                , int flags) {
        if (this->verbose & 16) {
            LOG( LOG_INFO
               , "Front::send_to_channel(channel, data=%p, length=%u, chunk_size=%u, flags=%x)"
               , chunk, length, chunk_size, flags);
        }

        if (channel.flags & GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
        }

        CHANNELS::VirtualChannelPDU virtual_channel_pdu(this->verbose);

        virtual_channel_pdu.send_to_client( *this->trans, this->encrypt
                                          , this->client_info.encryptionLevel, userid, channel.chanid
                                          , length, flags, chunk, chunk_size);
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

    void GeneratePaletteUpdateData(Stream & stream) {
        const BGRPalette & palette = (this->mod_bpp == 8)?this->memblt_mod_palette:this->palette332;

        // Payload
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
    }

/*
    void SendLogonInfo(const uint8_t * user_name)
    {
        BStream stream(65536);
        ShareData sdata_out(stream);
        sdata_out.emit_begin(PDUTYPE2_SAVE_SESSION_INFO, this->share_id,
            RDP::STREAM_MED);

        RDP::SaveSessionInfoPDUData_Send ssipdu(stream, RDP::INFOTYPE_LOGON);
        RDP::LogonInfoVersion1_Send      liv1(stream,
                                              reinterpret_cast<const uint8_t *>(""),
                                              user_name, getpid());

        stream.mark_end();

        // Packet trailer
        sdata_out.emit_end();

        BStream sctrl_header(256);
        ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU,
            this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

        HStream target_stream(1024, 65536);
        target_stream.out_copy_bytes(sctrl_header);
        target_stream.out_copy_bytes(stream);
        target_stream.mark_end();

        if ((this->verbose & (128|8)) == (128|8)){
            LOG(LOG_INFO, "Sec clear payload to send:");
            hexdump_d(target_stream.get_data(), target_stream.size());
        }

        this->send_data_indication_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);
    }
*/

    void send_global_palette() throw (Error)
    {
        if (!this->palette_sent && (this->client_info.bpp == 8)){
            if (this->verbose & 4){
                LOG(LOG_INFO, "Front::send_global_palette()");
            }

            if (this->server_fastpath_update_support == false) {
                BStream stream(65536);

                ShareData sdata(stream);
                sdata.emit_begin(PDUTYPE2_UPDATE, this->share_id, RDP::STREAM_MED);

                GeneratePaletteUpdateData(stream);

                // Packet trailer
                sdata.emit_end();

                BStream sctrl_header(256);
                ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

                HStream target_stream(1024, 65536);
                target_stream.out_copy_bytes(sctrl_header);
                target_stream.out_copy_bytes(stream);
                target_stream.mark_end();

                if (this->verbose & 128) {
                    LOG(LOG_INFO, "Sec clear payload to send:");
                    hexdump_d(target_stream.get_data(), target_stream.size());
                }

                this->send_data_indication_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);
            }
            else {
                HStream stream(1024, 65536);

                if (this->verbose & 4){
                    LOG(LOG_INFO, "Front::send_global_palette: fast-path");
                }

                size_t header_size = FastPath::Update_Send::GetSize(0);

                stream.out_clear_bytes(header_size); // Fast-Path Update (TS_FP_UPDATE structure) size

                GeneratePaletteUpdateData(stream);

                SubStream Upd_s(stream, 0, header_size);

                FastPath::Update_Send Upd( Upd_s
                                         , stream.size() - header_size
                                         , FastPath::FASTPATH_UPDATETYPE_PALETTE
                                         , FastPath::FASTPATH_FRAGMENT_SINGLE
                                         , /*FastPath:: FASTPATH_OUTPUT_COMPRESSION_USED*/0
                                         , 0
                                         );

                BStream SvrUpdPDU_s(256);

                FastPath::ServerUpdatePDU_Send SvrUpdPDU(
                      SvrUpdPDU_s
                    , stream
                    , ((this->client_info.encryptionLevel > 1) ? FastPath::FASTPATH_OUTPUT_ENCRYPTED : 0)
                    , this->encrypt
                    );
                // Server Fast-Path Update PDU (TS_FP_UPDATE_PDU)
                // Fast-Path Update (TS_FP_UPDATE)
                this->trans->send(SvrUpdPDU_s, stream);
            }

            this->palette_sent = true;
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

    void GenerateColorPointerUpdateData(Stream & stream, int cache_idx, const Pointer & cursor)
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
        stream.mark_end();
    }

    virtual void send_pointer(int cache_idx, const Pointer & cursor) throw(Error) {
        if (this->verbose & 4) {
            LOG(LOG_INFO, "Front::send_pointer(cache_idx=%u x=%u y=%u)",
                cache_idx, cursor.x, cursor.y);
        }

        if (this->server_fastpath_update_support == false) {
            BStream stream(65536);

            ShareData sdata(stream);
            sdata.emit_begin(PDUTYPE2_POINTER, this->share_id, RDP::STREAM_MED);

            // Payload
            stream.out_uint16_le(RDP_POINTER_COLOR);
            stream.out_uint16_le(0); /* pad */

            GenerateColorPointerUpdateData(stream, cache_idx, cursor);

            // Packet trailer
            sdata.emit_end();

            BStream sctrl_header(256);
            ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU,
                this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

            HStream target_stream(1024, 65536);
            target_stream.out_copy_bytes(sctrl_header);
            target_stream.out_copy_bytes(stream);
            target_stream.mark_end();

            if (this->verbose & 4) {
                LOG(LOG_INFO, "Sec clear payload to send:");
                hexdump_d(target_stream.get_data(), target_stream.size());
            }

            this->send_data_indication_ex(GCC::MCS_GLOBAL_CHANNEL,
                target_stream);
        }
        else {
            HStream stream(1024, 65536);

            if (this->verbose & 4) {
                LOG(LOG_INFO, "Front::send_pointer: fast-path");
            }

            size_t header_size = FastPath::Update_Send::GetSize(0);

            stream.out_clear_bytes(header_size);    // Fast-Path Update (TS_FP_UPDATE structure) size

            GenerateColorPointerUpdateData(stream, cache_idx, cursor);

            SubStream Upd_s(stream, 0, header_size);

            FastPath::Update_Send Upd(Upd_s,
                                      stream.size() - header_size,
                                      FastPath::FASTPATH_UPDATETYPE_COLOR,
                                      FastPath::FASTPATH_FRAGMENT_SINGLE,
                                      /*FastPath:: FASTPATH_OUTPUT_COMPRESSION_USED*/0,
                                      0);

            BStream fastpath_header(256);

            FastPath::ServerUpdatePDU_Send SvrUpdPDU(
                fastpath_header,
                stream,
                ((this->client_info.encryptionLevel > 1) ?
                 FastPath::FASTPATH_OUTPUT_ENCRYPTED : 0),
                this->encrypt);
            this->trans->send(fastpath_header, stream);
        }

        if (this->capture &&
            (this->capture_state == CAPTURE_STATE_STARTED)) {
            this->capture->send_pointer(cache_idx, cursor);
        }

        if (this->verbose & 4) {
            LOG(LOG_INFO, "Front::send_pointer done");
        }
    }   // void send_pointer(int cache_idx, uint8_t* data, uint8_t* mask,
        //     int hotspot_x, hotspot_int y)

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

    virtual void set_pointer(int cache_idx) {
        if (this->verbose & 4) {
            LOG(LOG_INFO, "Front::set_pointer(cache_idx=%u)", cache_idx);
        }

        if (this->server_fastpath_update_support == false) {
            BStream stream(65536);

            ShareData sdata(stream);
            sdata.emit_begin(PDUTYPE2_POINTER, this->share_id,
                RDP::STREAM_MED);

            // Payload
            stream.out_uint16_le(RDP_POINTER_CACHED);
            stream.out_uint16_le(0); /* pad */
            stream.out_uint16_le(cache_idx);
            stream.mark_end();

            // Packet trailer
            sdata.emit_end();

            BStream sctrl_header(256);
            ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU,
                this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

            HStream target_stream(1024, 65536);
            target_stream.out_copy_bytes(sctrl_header);
            target_stream.out_copy_bytes(stream);
            target_stream.mark_end();

            if (this->verbose & (128 | 4)) {
                LOG(LOG_INFO, "Sec clear payload to send:");
                hexdump_d(target_stream.get_data(), target_stream.size());
            }

            this->send_data_indication_ex(GCC::MCS_GLOBAL_CHANNEL,
                target_stream);
        }
        else {
            HStream stream(1024, 65536);

            if (this->verbose & 4) {
                LOG(LOG_INFO, "Front::set_pointer: fast-path");
            }

            size_t header_size = FastPath::Update_Send::GetSize(0);

            stream.out_clear_bytes(header_size);    // Fast-Path Update (TS_FP_UPDATE structure) size

            // Payload
            stream.out_uint16_le(cache_idx);
            stream.mark_end();

            SubStream Upd_s(stream, 0, header_size);

            FastPath::Update_Send Upd(Upd_s,
                                      stream.size() - header_size,
                                      FastPath::FASTPATH_UPDATETYPE_CACHED,
                                      FastPath::FASTPATH_FRAGMENT_SINGLE,
                                      /*FastPath:: FASTPATH_OUTPUT_COMPRESSION_USED*/0,
                                      0);

            BStream fastpath_header(256);

             // Server Fast-Path Update PDU (TS_FP_UPDATE_PDU)
            FastPath::ServerUpdatePDU_Send SvrUpdPDU(
                fastpath_header,
                stream,
                ((this->client_info.encryptionLevel > 1) ?
                 FastPath::FASTPATH_OUTPUT_ENCRYPTED : 0),
                this->encrypt);
            this->trans->send(fastpath_header, stream);
        }

        if (this->capture &&
            (this->capture_state == CAPTURE_STATE_STARTED)) {
            this->capture->set_pointer(cache_idx);
        }

        if (this->verbose & 4) {
            LOG(LOG_INFO, "Front::set_pointer done");
        }
    }   // void set_pointer(int cache_idx)

    virtual void set_pointer_display() {
        if (this->capture) {
            this->capture->set_pointer_display();
        }
    }

    void incoming(Callback & cb) throw(Error)
    {
        unsigned expected;

        if (this->verbose & 4){
            LOG(LOG_INFO, "Front::incoming()");
        }

        switch (this->state){
        case CONNECTION_INITIATION:
        {
            // Connection Initiation
            // ---------------------
            LOG(LOG_INFO, "Front::incoming:CONNECTION_INITIATION");

            // The client initiates the connection by sending the server an X.224 Connection
            //  Request PDU (class 0). The server responds with an X.224 Connection Confirm
            // PDU (class 0). From this point, all subsequent data sent between client and
            // server is wrapped in an X.224 Data Protocol Data Unit (PDU).

            // Client                                                     Server
            //    |------------X224 Connection Request PDU----------------> |
            //    | <----------X224 Connection Confirm PDU----------------- |

            if (this->verbose & 1){
                LOG(LOG_INFO, "Front::incoming::receiving x224 request PDU");
            }

            {
                BStream stream(65536);
                X224::RecvFactory fac_x224(*this->trans, stream);
                X224::CR_TPDU_Recv x224(*this->trans, stream, this->ini->client.bogus_neg_request);
                if (x224._header_size != (size_t)(stream.size())){
                    LOG(LOG_ERR, "Front::incoming::connection request : all data should have been consumed,"
                                 " %d bytes remains", stream.size() - x224._header_size);
                }
                this->clientRequestedProtocols = x224.rdp_neg_requestedProtocols;

                if (// Proxy doesnt supports TLS or RDP client doesn't support TLS
                    (!this->ini->client.tls_support || 0 == (this->clientRequestedProtocols & X224::PROTOCOL_TLS))
                    // Fallback to legacy security protocol (RDP) is allowed.
                    && this->ini->client.tls_fallback_legacy) {
                    LOG(LOG_INFO, "Fallback to legacy security protocol");
                    this->tls_client_active = false;
                }
            }

            if (this->verbose & 1){
                LOG(LOG_INFO, "Front::incoming::sending x224 connection confirm PDU");
            }
            {
                BStream stream(256);
                uint8_t rdp_neg_type = 0;
                uint8_t rdp_neg_flags = 0;
                uint32_t rdp_neg_code = 0;
                if (this->tls_client_active){
                    LOG(LOG_INFO, "-----------------> Front::TLS Support Enabled");
                    if (this->clientRequestedProtocols & X224::PROTOCOL_TLS) {
                        rdp_neg_type = X224::RDP_NEG_RSP;
                        rdp_neg_code = X224::PROTOCOL_TLS;
                        this->client_info.encryptionLevel = 0;
                    }
                    else {
                        rdp_neg_type = X224::RDP_NEG_FAILURE;
                        rdp_neg_code = X224::SSL_REQUIRED_BY_SERVER;
                    }
                }
                else {
                    LOG(LOG_INFO, "-----------------> Front::TLS Support not Enabled");
                }

                X224::CC_TPDU_Send x224(stream, rdp_neg_type, rdp_neg_flags, rdp_neg_code);
                this->trans->send(stream);

                if (this->tls_client_active){
                    this->trans->enable_server_tls(this->ini->globals.certificate_password);

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
            // | AUTHZ _ACCESS_DENIED 0x0000052E | The user does not have permission to access the server.|
            // +---------------------------------+--------------------------------------------------------+

                }
            }
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

            if (this->verbose & 1){
                LOG(LOG_INFO, "Front::incoming::Basic Settings Exchange");
            }

            BStream x224_data(65536);
            X224::RecvFactory f(*this->trans, x224_data);
            X224::DT_TPDU_Recv x224(*this->trans, x224_data);
            MCS::CONNECT_INITIAL_PDU_Recv mcs_ci(x224.payload, MCS::BER_ENCODING);

            // GCC User Data
            // -------------
            SubStream & gcc_data = mcs_ci.payload;
            GCC::Create_Request_Recv gcc_cr(gcc_data);


            while (gcc_cr.payload.in_check_rem(4)) {
                GCC::UserData::RecvFactory f(gcc_cr.payload);
                switch (f.tag){
                    case CS_CORE:
                    {
                        GCC::UserData::CSCore cs_core;
                        cs_core.recv(f.payload);
                        if (this->verbose & 1) {
                            cs_core.log("Received from Client");
                        }

                        this->client_info.width     = cs_core.desktopWidth;
                        this->client_info.height    = cs_core.desktopHeight;
                        this->client_info.keylayout = cs_core.keyboardLayout;
                        this->client_info.build     = cs_core.clientBuild;
                        for (size_t i = 0; i < 16 ; i++){
                            this->client_info.hostname[i] = cs_core.clientName[i];
                        }
                        this->client_info.bpp = 8;
                        switch (cs_core.postBeta2ColorDepth){
                        case 0xca01:
                            /*
                            this->client_info.bpp =
                                (cs_core.highColorDepth <= 24)?cs_core.highColorDepth:24;
                            */
                            this->client_info.bpp = (
                                      (cs_core.earlyCapabilityFlags & GCC::UserData::RNS_UD_CS_WANT_32BPP_SESSION)
                                    ? 32
                                    : cs_core.highColorDepth
                                );
                        break;
                        case 0xca02:
                            this->client_info.bpp = 15;
                        break;
                        case 0xca03:
                            this->client_info.bpp = 16;
                        break;
                        case 0xca04:
                            this->client_info.bpp = 24;
                        break;
                        default:
                        break;
                        }
                        if (this->ini->client.max_color_depth) {
                            this->client_info.bpp = std::min<int>(
                                this->client_info.bpp, this->ini->client.max_color_depth);
                        }
                    }
                    break;
                    case CS_SECURITY:
                    {
                        GCC::UserData::CSSecurity cs_sec;
                        cs_sec.recv(f.payload);
                        if (this->verbose & 1) {
                            cs_sec.log("Received from Client");
                        }
                    }
                    break;
                    case CS_NET:
                    {
                        GCC::UserData::CSNet cs_net;
                        cs_net.recv(f.payload);
                        for (uint32_t index = 0; index < cs_net.channelCount; index++) {
                            CHANNELS::ChannelDef channel_item;
                            memcpy(channel_item.name, cs_net.channelDefArray[index].name, 8);
                            channel_item.flags = cs_net.channelDefArray[index].options;
                            channel_item.chanid = GCC::MCS_GLOBAL_CHANNEL + (index + 1);
                            this->channel_list.push_back(channel_item);
                        }
                        if (this->verbose & 1) {
                            cs_net.log("Received from Client");
                        }
                    }
                    break;
                    case CS_CLUSTER:
                    {
                        GCC::UserData::CSCluster cs_cluster;
                        cs_cluster.recv(f.payload);
                        this->client_info.console_session =
                            (0 != (cs_cluster.flags & GCC::UserData::CSCluster::REDIRECTED_SESSIONID_FIELD_VALID));
                        if (this->verbose & 1) {
                            cs_cluster.log("Receiving from Client");
                        }
                    }
                    break;
                    case CS_MONITOR:
                    {
                        GCC::UserData::CSMonitor cs_monitor;
                        cs_monitor.recv(f.payload);
                        if (this->verbose & 1) {
                            cs_monitor.log("Receiving from Client");
                        }
                    }
                    break;
                    default:
                        LOG(LOG_WARNING, "Unexpected data block tag %x\n", f.tag);
                    break;
                }
            }
            if (gcc_cr.payload.in_check_rem(1)) {
                LOG(LOG_ERR, "recv connect request parsing gcc data : short header");
                throw Error(ERR_MCS_DATA_SHORT_HEADER);
            }

            // ------------------------------------------------------------------
            HStream stream(1024, 65536);
            // ------------------------------------------------------------------
//            GCC::UserData::ServerToClient_Send(stream, this->clientRequestedProtocols, this->channel_list.size());

            GCC::UserData::SCCore sc_core;
            sc_core.version = 0x00080004;
            if (this->tls_client_active){
                sc_core.length = 12;
                sc_core.clientRequestedProtocols = this->clientRequestedProtocols;
            }
            if (this->verbose & 1) {
                sc_core.log("Sending to client");
            }
            sc_core.emit(stream);
            // ------------------------------------------------------------------
            GCC::UserData::SCNet sc_net;
            const uint8_t num_channels = this->channel_list.size();
            sc_net.MCSChannelId = GCC::MCS_GLOBAL_CHANNEL;
            sc_net.channelCount = num_channels;
            for (int index = 0; index < num_channels; index++) {
                sc_net.channelDefArray[index].id = GCC::MCS_GLOBAL_CHANNEL + index + 1;
            }
            if (this->verbose & 1) {
                sc_net.log("Sending to client");
            }
            sc_net.emit(stream);
            // ------------------------------------------------------------------
            if (this->tls_client_active){
                GCC::UserData::SCSecurity sc_sec1;
                sc_sec1.encryptionMethod = 0;
                sc_sec1.encryptionLevel = 0;
                sc_sec1.length = 12;
                sc_sec1.serverRandomLen = 0;
                sc_sec1.serverCertLen = 0;
                if (this->verbose & 1) {
                    sc_sec1.log("Sending to client");
                }
                sc_sec1.emit(stream);
            }
            else {
                GCC::UserData::SCSecurity sc_sec1;
                /*
                   For now rsa_keys are not in a configuration file any more, but as we were not changing keys
                   the values have been embedded in code and the key generator file removed from source code.

                   It will be put back at some later time using a clean parser/writer module and sll calls
                   coherent with the remaining of ReDemPtion code. For reference to historical key generator code
                   look for utils/keygen.cpp in old repository code.

                   references for RSA Keys: http://www.securiteam.com/windowsntfocus/5EP010KG0G.html
                */
                uint8_t rsa_keys_pub_mod[64] = {
                    0x67, 0xab, 0x0e, 0x6a, 0x9f, 0xd6, 0x2b, 0xa3, 0x32, 0x2f, 0x41, 0xd1, 0xce, 0xee, 0x61, 0xc3,
                    0x76, 0x0b, 0x26, 0x11, 0x70, 0x48, 0x8a, 0x8d, 0x23, 0x81, 0x95, 0xa0, 0x39, 0xf7, 0x5b, 0xaa,
                    0x3e, 0xf1, 0xed, 0xb8, 0xc4, 0xee, 0xce, 0x5f, 0x6a, 0xf5, 0x43, 0xce, 0x5f, 0x60, 0xca, 0x6c,
                    0x06, 0x75, 0xae, 0xc0, 0xd6, 0xa4, 0x0c, 0x92, 0xa4, 0xc6, 0x75, 0xea, 0x64, 0xb2, 0x50, 0x5b
                };
                memcpy(this->pub_mod, rsa_keys_pub_mod, 64);

                uint8_t rsa_keys_pri_exp[64] = {
                    0x41, 0x93, 0x05, 0xB1, 0xF4, 0x38, 0xFC, 0x47, 0x88, 0xC4, 0x7F, 0x83, 0x8C, 0xEC, 0x90, 0xDA,
                    0x0C, 0x8A, 0xB5, 0xAE, 0x61, 0x32, 0x72, 0xF5, 0x2B, 0xD1, 0x7B, 0x5F, 0x44, 0xC0, 0x7C, 0xBD,
                    0x8A, 0x35, 0xFA, 0xAE, 0x30, 0xF6, 0xC4, 0x6B, 0x55, 0xA7, 0x65, 0xEF, 0xF4, 0xB2, 0xAB, 0x18,
                    0x4E, 0xAA, 0xE6, 0xDC, 0x71, 0x17, 0x3B, 0x4C, 0xC2, 0x15, 0x4C, 0xF7, 0x81, 0xBB, 0xF0, 0x03
                };
                memcpy(sc_sec1.pri_exp, rsa_keys_pri_exp, 64);
                memcpy(this->pri_exp, sc_sec1.pri_exp, 64);

                uint8_t rsa_keys_pub_sig[64] = {
                    0x6a, 0x41, 0xb1, 0x43, 0xcf, 0x47, 0x6f, 0xf1, 0xe6, 0xcc, 0xa1, 0x72, 0x97, 0xd9, 0xe1, 0x85,
                    0x15, 0xb3, 0xc2, 0x39, 0xa0, 0xa6, 0x26, 0x1a, 0xb6, 0x49, 0x01, 0xfa, 0xa6, 0xda, 0x60, 0xd7,
                    0x45, 0xf7, 0x2c, 0xee, 0xe4, 0x8e, 0x64, 0x2e, 0x37, 0x49, 0xf0, 0x4c, 0x94, 0x6f, 0x08, 0xf5,
                    0x63, 0x4c, 0x56, 0x29, 0x55, 0x5a, 0x63, 0x41, 0x2c, 0x20, 0x65, 0x95, 0x99, 0xb1, 0x15, 0x7c
                };

                uint8_t rsa_keys_pub_exp[4] = { 0x01, 0x00, 0x01, 0x00 };

                sc_sec1.encryptionMethod = this->encrypt.encryptionMethod;
                sc_sec1.encryptionLevel = this->client_info.encryptionLevel;
                sc_sec1.serverRandomLen = 32;
                this->gen->random(this->server_random, 32);
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

                if (this->verbose & 1) {
                    sc_sec1.log("Sending to client");
                }
                sc_sec1.emit(stream);
            }
            stream.mark_end();

            // ------------------------------------------------------------------
            BStream gcc_header(256);
            BStream mcs_header(256);
            BStream x224_header(256);

            GCC::Create_Response_Send(gcc_header, stream.size());
            MCS::CONNECT_RESPONSE_Send mcs_cr(mcs_header, gcc_header.size() + stream.size(), MCS::BER_ENCODING);
            X224::DT_TPDU_Send(x224_header, mcs_header.size() + gcc_header.size() + stream.size());
            this->trans->send(x224_header, mcs_header, gcc_header, stream);

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

            if (this->verbose & 16){
                LOG(LOG_INFO, "Front::incoming::Channel Connection");
            }

            if (this->verbose){
                LOG(LOG_INFO, "Front::incoming::Recv MCS::ErectDomainRequest");
            }
            {
                BStream x224_data(256);
                X224::RecvFactory f(*this->trans, x224_data);
                X224::DT_TPDU_Recv x224(*trans, x224_data);
                MCS::ErectDomainRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);
            }
            if (this->verbose){
                LOG(LOG_INFO, "Front::incoming::Recv MCS::AttachUserRequest");
            }
            {
                BStream x224_data(256);
                X224::RecvFactory f(*this->trans, x224_data);
                X224::DT_TPDU_Recv x224(*trans, x224_data);
                MCS::AttachUserRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);
            }
            if (this->verbose){
                LOG(LOG_INFO, "Front::incoming::Send MCS::AttachUserConfirm", this->userid);
            }
            {
                BStream x224_header(256);
                HStream mcs_data(256, 512);
                MCS::AttachUserConfirm_Send(mcs_data, MCS::RT_SUCCESSFUL, true, this->userid, MCS::PER_ENCODING);
                X224::DT_TPDU_Send(x224_header, mcs_data.size());
                this->trans->send(x224_header, mcs_data);
            }

            {
                // read tpktHeader (4 bytes = 3 0 len)
                // TPDU class 0    (3 bytes = LI F0 PDU_DT)
                BStream x224_data(256);
                X224::RecvFactory f(*this->trans, x224_data);
                X224::DT_TPDU_Recv x224(*this->trans, x224_data);
                MCS::ChannelJoinRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);
                this->userid = mcs.initiator;

                BStream x224_header(256);
                HStream mcs_cjcf_data(256, 512);

                MCS::ChannelJoinConfirm_Send(mcs_cjcf_data, MCS::RT_SUCCESSFUL,
                                             mcs.initiator,
                                             mcs.channelId,
                                             true, mcs.channelId,
                                             MCS::PER_ENCODING);
                X224::DT_TPDU_Send(x224_header, mcs_cjcf_data.size());
                this->trans->send(x224_header, mcs_cjcf_data);
            }

            {
                BStream x224_data(256);
                X224::RecvFactory f(*this->trans, x224_data);
                X224::DT_TPDU_Recv x224(*this->trans, x224_data);
                MCS::ChannelJoinRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);
                if (mcs.initiator != this->userid){
                    LOG(LOG_ERR, "MCS error bad userid, expecting %u got %u", this->userid, mcs.initiator);
                    throw Error(ERR_MCS_BAD_USERID);
                }

                BStream x224_header(256);
                HStream mcs_cjcf_data(256, 512);

                MCS::ChannelJoinConfirm_Send(mcs_cjcf_data, MCS::RT_SUCCESSFUL,
                                             mcs.initiator,
                                             mcs.channelId,
                                             true, mcs.channelId,
                                             MCS::PER_ENCODING);
                X224::DT_TPDU_Send(x224_header, mcs_cjcf_data.size());
                this->trans->send(x224_header, mcs_cjcf_data);
            }

            for (size_t i = 0 ; i < this->channel_list.size() ; i++){
                BStream x224_data(256);
                X224::RecvFactory f(*this->trans, x224_data);
                X224::DT_TPDU_Recv x224(*this->trans, x224_data);
                MCS::ChannelJoinRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);

                if (this->verbose & 16){
                    LOG(LOG_INFO, "cjrq[%u] = %u -> cjcf", i, mcs.channelId);
                }

                if (mcs.initiator != this->userid){
                    LOG(LOG_ERR, "MCS error bad userid, expecting %u got %u", this->userid, mcs.initiator);
                    throw Error(ERR_MCS_BAD_USERID);
                }

                BStream x224_header(256);
                HStream mcs_cjcf_data(256, 512);

                MCS::ChannelJoinConfirm_Send(mcs_cjcf_data, MCS::RT_SUCCESSFUL,
                                             mcs.initiator,
                                             mcs.channelId,
                                             true, mcs.channelId,
                                             MCS::PER_ENCODING);
                X224::DT_TPDU_Send(x224_header, mcs_cjcf_data.size());
                this->trans->send(x224_header, mcs_cjcf_data);

                this->channel_list.set_chanid(i, mcs.channelId);
            }

            if (this->verbose & 1){
                LOG(LOG_INFO, "Front::incoming::RDP Security Commencement");
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
            // header is include " with the data if encryption is in force (the Client Info
            // and licensing PDUs are an exception in that they always have a security
            // header). The Security Header follows the X.224 and MCS Headers and indicates
            // whether the attached data is encrypted.

            // Even if encryption is in force server-to-client traffic may not always be
            // encrypted, while client-to-server traffic will always be encrypted by
            // Microsoft RDP implementations (encryption of licensing PDUs is optional,
            // however).

            // Client                                                     Server
            //    |------Security Exchange PDU ---------------------------> |
            if (this->tls_client_active){
                LOG(LOG_INFO, "TLS mode: exchange packet disabled");
            }
            else
            {
                LOG(LOG_INFO, "Legacy RDP mode: expecting exchange packet");
                BStream pdu(65536);
                X224::RecvFactory f(*this->trans, pdu);
                X224::DT_TPDU_Recv x224(*this->trans, pdu);

                MCS::RecvFactory mcs_fac(x224.payload, MCS::PER_ENCODING);
                if (mcs_fac.type == MCS::MCSPDU_DisconnectProviderUltimatum){
                    LOG(LOG_INFO, "Front::incoming::DisconnectProviderUltimatum received");
                    x224.payload.rewind();
                    MCS::DisconnectProviderUltimatum_Recv mcs(x224.payload, MCS::PER_ENCODING);
                    const char * reason = MCS::get_reason(mcs.reason);
                    LOG(LOG_INFO, "Front DisconnectProviderUltimatum: reason=%s [%d]", reason, mcs.reason);
                    throw Error(ERR_MCS);
                }

                MCS::SendDataRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);
                SEC::SecExchangePacket_Recv sec(mcs.payload, mcs.payload_size);

                TODO("see possible factorisation with ssl_calls.hpp/ssllib::rsa_encrypt")
                uint8_t client_random[64];
                memset(client_random, 0, 64);
                {
                    uint8_t l_out[64]; memset(l_out, 0, 64);
                    uint8_t l_in[64];  rmemcpy(l_in, sec.payload.get_data(), 64);
                    uint8_t l_mod[64]; rmemcpy(l_mod, this->pub_mod, 64);
                    uint8_t l_exp[64]; rmemcpy(l_exp, this->pri_exp, 64);

                    BN_CTX* ctx = BN_CTX_new();
                    BIGNUM lmod; BN_init(&lmod); BN_bin2bn(l_mod, 64, &lmod);
                    BIGNUM lexp; BN_init(&lexp); BN_bin2bn(l_exp, 64, &lexp);
                    BIGNUM lin; BN_init(&lin);  BN_bin2bn(l_in, 64, &lin);
                    BIGNUM lout; BN_init(&lout); BN_mod_exp(&lout, &lin, &lexp, &lmod, ctx);

                    int rv = BN_bn2bin(&lout, l_out);
                    if (rv <= 64) {
                        reverseit(l_out, rv);
                        memcpy(client_random, l_out, 64);
                    }
                    BN_free(&lin);
                    BN_free(&lout);
                    BN_free(&lexp);
                    BN_free(&lmod);
                    BN_CTX_free(ctx);
                }

                // beware order of parameters for key generation (decrypt/encrypt) is inversed between server and client
                SEC::KeyBlock key_block(client_random, this->server_random);
                memcpy(this->encrypt.sign_key, key_block.blob0, 16);
                ssllib ssl;
                if (this->encrypt.encryptionMethod == 1){
                    ssl.sec_make_40bit(this->encrypt.sign_key);
                }

                this->encrypt.generate_key(key_block.key1, this->encrypt.encryptionMethod);
                this->decrypt.generate_key(key_block.key2, this->encrypt.encryptionMethod);
            }
            this->state = WAITING_FOR_LOGON_INFO;
        }
        break;

        case WAITING_FOR_LOGON_INFO:
        // Secure Settings Exchange
        // ------------------------

        // Secure Settings Exchange: Secure client data (such as the username,
        // password and auto-reconnect cookie) is sent to the server using the Client
        // Info PDU.

        // Client                                                     Server
        //    |------ Client Info PDU      ---------------------------> |
        {
            LOG(LOG_INFO, "Front::incoming::Secure Settings Exchange");

            BStream stream(65536);
            X224::RecvFactory fx224(*this->trans, stream);
            X224::DT_TPDU_Recv x224(*this->trans, stream);

            MCS::RecvFactory mcs_fac(x224.payload, MCS::PER_ENCODING);
            if (mcs_fac.type == MCS::MCSPDU_DisconnectProviderUltimatum){
                LOG(LOG_INFO, "Front::incoming::DisconnectProviderUltimatum received");
                x224.payload.rewind();
                MCS::DisconnectProviderUltimatum_Recv mcs(x224.payload, MCS::PER_ENCODING);
                const char * reason = MCS::get_reason(mcs.reason);
                LOG(LOG_INFO, "Front DisconnectProviderUltimatum: reason=%s [%d]", reason, mcs.reason);
                throw Error(ERR_MCS);
            }

            MCS::SendDataRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);

            SEC::SecSpecialPacket_Recv sec(mcs.payload, this->decrypt, this->client_info.encryptionLevel);
            if (this->verbose & 128){
                LOG(LOG_INFO, "sec decrypted payload:");
                hexdump_d(sec.payload.get_data(), sec.payload.size());
            }

            if (!sec.flags & SEC::SEC_INFO_PKT) {
                throw Error(ERR_SEC_EXPECTED_LOGON_INFO);
            }

            /* this is the first test that the decrypt is working */
            this->client_info.process_logon_info( sec.payload
                                                , ini->client.ignore_logon_password
                                                , ini->client.performance_flags_default
                                                , ini->client.performance_flags_force_present
                                                , ini->client.performance_flags_force_not_present
                                                , (this->verbose & 128)
                                                );

            if (sec.payload.in_remain()){
                LOG(LOG_ERR, "Front::incoming::process_logon all data should have been consumed %u bytes trailing",
                    (unsigned)sec.payload.in_remain());
            }

            this->keymap.init_layout(this->client_info.keylayout);
            this->ini->client.keyboard_layout.set(this->client_info.keylayout);
            if (this->client_info.is_mce) {
                if (this->verbose & 2){
                    LOG(LOG_INFO, "Front::incoming::licencing client_info.is_mce");
                    LOG(LOG_INFO, "Front::incoming::licencing send_media_lic_response");
                }

                {
                    HStream stream(1024, 65535);

                    /* mce */
                    /* some compilers need unsigned char to avoid warnings */
                    static uint8_t lic3[16] = { 0xff, 0x03, 0x10, 0x00,
                                             0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                                             0xf3, 0x99, 0x00, 0x00
                                             };

                    stream.out_copy_bytes((char*)lic3, 16);
                    stream.mark_end();

                    BStream sec_header(256);

                    if ((this->verbose & (128|2)) == (128|2)){
                        LOG(LOG_INFO, "Sec clear payload to send:");
                        hexdump_d(stream.get_data(), stream.size());
                    }

                    SEC::Sec_Send sec(sec_header, stream, SEC::SEC_LICENSE_PKT | 0x00100200, this->encrypt, 0);
                    stream.copy_to_head(sec_header.get_data(), sec_header.size());

                    this->send_data_indication(GCC::MCS_GLOBAL_CHANNEL, stream);
                }
                // proceed with capabilities exchange

                // Capabilities Exchange
                // ---------------------

                // Capabilities Negotiation: The server sends the set of capabilities it
                // supports to the client in a Demand Active PDU. The client responds with its
                // capabilities by sending a Confirm Active PDU.

                // Client                                                     Server
                //    | <------- Demand Active PDU ---------------------------- |
                //    |--------- Confirm Active PDU --------------------------> |

                if (this->verbose & 1){
                    LOG(LOG_INFO, "Front::incoming::send_demand_active");
                }
                this->send_demand_active();

                LOG(LOG_INFO, "Front::incoming::ACTIVATED (mce)");
                this->state = ACTIVATE_AND_PROCESS_DATA;
            }
            else {
                if (this->verbose & 16){
                    LOG(LOG_INFO, "Front::incoming::licencing not client_info.is_mce");
                    LOG(LOG_INFO, "Front::incoming::licencing send_lic_initial");
                }

                HStream stream(1024, 65535);

                stream.out_uint8(LIC::LICENSE_REQUEST);
                stream.out_uint8(2); // preamble flags : PREAMBLE_VERSION_2_0 (RDP 4.0)
                stream.out_uint16_le(318); // wMsgSize = 318 including preamble

                /* some compilers need unsigned char to avoid warnings */
                static uint8_t lic1[314] = {
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

                stream.out_copy_bytes((char*)lic1, 314);
                stream.mark_end();

                BStream sec_header(256);

                if ((this->verbose & (128|2)) == (128|2)){
                    LOG(LOG_INFO, "Sec clear payload to send:");
                    hexdump_d(stream.get_data(), stream.size());
                }

                SEC::Sec_Send sec(sec_header, stream, SEC::SEC_LICENSE_PKT,
                    this->encrypt, 0);
                stream.copy_to_head(sec_header.get_data(), sec_header.size());

                this->send_data_indication(GCC::MCS_GLOBAL_CHANNEL, stream);

                if (this->verbose & 2){
                    LOG(LOG_INFO, "Front::incoming::waiting for answer to lic_initial");
                }
                this->state = WAITING_FOR_ANSWER_TO_LICENCE;
            }
        }
        break;

        case WAITING_FOR_ANSWER_TO_LICENCE:
        {
            if (this->verbose & 2){
                LOG(LOG_INFO, "Front::incoming::WAITING_FOR_ANSWER_TO_LICENCE");
            }
            BStream stream(65536);
            X224::RecvFactory fx224(*this->trans, stream);
            X224::DT_TPDU_Recv x224(*this->trans, stream);

            MCS::RecvFactory mcs_fac(x224.payload, MCS::PER_ENCODING);
            if (mcs_fac.type == MCS::MCSPDU_DisconnectProviderUltimatum){
                LOG(LOG_INFO, "Front::incoming::DisconnectProviderUltimatum received");
                x224.payload.rewind();
                MCS::DisconnectProviderUltimatum_Recv mcs(x224.payload, MCS::PER_ENCODING);
                const char * reason = MCS::get_reason(mcs.reason);
                LOG(LOG_INFO, "Front DisconnectProviderUltimatum: reason=%s [%d]", reason, mcs.reason);
                throw Error(ERR_MCS);
            }

            MCS::SendDataRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);

            SEC::SecSpecialPacket_Recv sec(mcs.payload, this->decrypt, this->client_info.encryptionLevel);
            if ((this->verbose & (128|2)) == (128|2)){
                LOG(LOG_INFO, "sec decrypted payload:");
                hexdump_d(sec.payload.get_data(), sec.payload.size());
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
                    if (this->verbose & 2){
                        LOG(LOG_INFO, "Front::ERROR_ALERT");
                    }
                    TODO("We should check what is actually returned by this message, as it may be an error")
                    LIC::ErrorAlert_Recv lic(sec.payload);
                    LOG(LOG_ERR, "Front::License Alert: error=%u transition=%u",
                        lic.validClientMessage.dwErrorCode, lic.validClientMessage.dwStateTransition);

                }
                break;
                case LIC::NEW_LICENSE_REQUEST:
                {
                    if (this->verbose & 2){
                        LOG(LOG_INFO, "Front::NEW_LICENSE_REQUEST");
                    }
                    LIC::NewLicenseRequest_Recv lic(sec.payload);
                    TODO("Instead of returning a license we return a message saying that no license is OK")
                    HStream stream(1024, 65535);
                    // Valid Client License Data (LICENSE_VALID_CLIENT_DATA)

                    /* some compilers need unsigned char to avoid warnings */
                    static uint8_t lic2[16] = {
                        0xff,       // bMsgType : ERROR_ALERT
                        0x02,       // NOT EXTENDED_ERROR_MSG_SUPPORTED, PREAMBLE_VERSION_2_0
                        0x10, 0x00, // wMsgSize: 16 bytes including preamble
                        0x07, 0x00, 0x00, 0x00, // dwErrorCode : STATUS_VALID_CLIENT
                        0x02, 0x00, 0x00, 0x00, // dwStateTransition ST_NO_TRANSITION
                        0x28, 0x14, // wBlobType : ignored because wBlobLen is 0
                        0x00, 0x00  // wBlobLen  : 0
                    };
                    stream.out_copy_bytes((char*)lic2, 16);
                    stream.mark_end();

                    BStream sec_header(256);

                    if ((this->verbose & (128|2)) == (128|2)){
                        LOG(LOG_INFO, "Sec clear payload to send:");
                        hexdump_d(stream.get_data(), stream.size());
                    }

                    SEC::Sec_Send sec(sec_header, stream, SEC::SEC_LICENSE_PKT | 0x00100000, this->encrypt, 0);
                    stream.copy_to_head(sec_header.get_data(), sec_header.size());

                    this->send_data_indication(GCC::MCS_GLOBAL_CHANNEL, stream);
                }
                break;
                case LIC::PLATFORM_CHALLENGE_RESPONSE:
                    TODO("As we never send a platform challenge, it is unlikely we ever receive a PLATFORM_CHALLENGE_RESPONSE")
                    if (this->verbose & 2){
                        LOG(LOG_INFO, "Front::PLATFORM_CHALLENGE_RESPONSE");
                    }
                    break;
                case LIC::LICENSE_INFO:
                    TODO("As we never send a server license request, it is unlikely we ever receive a LICENSE_INFO")
                    if (this->verbose & 2){
                        LOG(LOG_INFO, "Front::LICENSE_INFO");
                    }
                    break;
                default:
                    if (this->verbose & 2){
                        LOG(LOG_INFO, "Front::LICENCE_TAG %u unknown or unsupported by server", flic.tag);
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

                if (this->verbose & 1){
                    LOG(LOG_INFO, "Front::incoming::send_demand_active");
                }
                this->send_demand_active();

                LOG(LOG_INFO, "Front::incoming::ACTIVATED (new license request)");
                this->state = ACTIVATE_AND_PROCESS_DATA;
            }
            else {
                if (this->verbose & 2){
                    LOG(LOG_INFO, "non licence packet: still waiting for licence");
                }
                ShareControl_Recv sctrl(sec.payload);

                switch (sctrl.pdu_type1) {
                case PDUTYPE_DEMANDACTIVEPDU: /* 1 */
                    if (this->verbose & 2){
                        LOG(LOG_INFO, "unexpected DEMANDACTIVE PDU while in licence negociation");
                    }
                    break;
                case PDUTYPE_CONFIRMACTIVEPDU:
                    if (this->verbose & 2){
                        LOG(LOG_INFO, "Unexpected CONFIRMACTIVE PDU");
                    }
                    {
                        expected = 6; /* shareId(4) + originatorId(2) */
                        if (!sctrl.payload.in_check_rem(expected)){
                            LOG(LOG_ERR, "Truncated CONFIRMACTIVE PDU, need=%u remains=%u",
                                expected, sctrl.payload.in_remain());
                            throw Error(ERR_MCS_PDU_TRUNCATED);
                        }
                        uint32_t share_id = sctrl.payload.in_uint32_le();
                        uint16_t originatorId = sctrl.payload.in_uint16_le();
                        this->process_confirm_active(sctrl.payload);
(void)share_id;
(void)originatorId;
                    }
                    if (!sctrl.payload.check_end()){
                        LOG(LOG_ERR, "Trailing data after CONFIRMACTIVE PDU remains=%u",
                            sctrl.payload.in_remain());
                        throw Error(ERR_MCS_PDU_TRAILINGDATA);
                    }
                    break;
                case PDUTYPE_DATAPDU: /* 7 */
                    if (this->verbose & 2){
                        LOG(LOG_INFO, "unexpected DATA PDU while in licence negociation");
                    }
                    // at this point licence negociation is still ongoing
                    // most data packets should not be received
                    // actually even input is dubious,
                    // but rdesktop actually sends input data
                    // also processing this is a problem because input data packets are broken
//                    this->process_data(sctrl.payload, cb);

                    TODO("check all payload data is consumed")
                    break;
                case PDUTYPE_DEACTIVATEALLPDU:
                    if (this->verbose & 2){
                        LOG(LOG_INFO, "unexpected DEACTIVATEALL PDU while in licence negociation");
                    }
                    TODO("check all payload data is consumed")
                    break;
                case PDUTYPE_SERVER_REDIR_PKT:
                    if (this->verbose & 2){
                        LOG(LOG_INFO, "unsupported SERVER_REDIR_PKT while in licence negociation");
                    }
                    TODO("check all payload data is consumed")
                    break;
                default:
                    LOG(LOG_WARNING, "unknown PDU type received while in licence negociation (%d)\n", sctrl.pdu_type1);
                    break;
                }
                TODO("Check why this is necessary when using loop connection ?")
            }
            sec.payload.p = sec.payload.end;
        }
        break;

        case ACTIVATE_AND_PROCESS_DATA:
        if (this->verbose & 8){
            LOG(LOG_INFO, "Front::incoming::ACTIVATE_AND_PROCESS_DATA");
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
            BStream stream(65536);

            // Detect fast-path PDU
            this->trans->recv(&stream.end, 1);
            uint8_t byte = stream.in_uint8();


            if ((byte & FastPath::FASTPATH_INPUT_ACTION_X224) == 0){
                FastPath::ClientInputEventPDU_Recv cfpie(*this->trans, stream, this->decrypt);

                uint8_t byte;
                uint8_t eventCode;

                for (uint8_t i = 0; i < cfpie.numEvents; i++){
                    if (!cfpie.payload.in_check_rem(1)){
                        LOG(LOG_ERR, "Truncated Fast-Path input event PDU, need=1 remains=%u",
                            cfpie.payload.in_remain());
                        throw Error(ERR_RDP_DATA_TRUNCATED);
                    }

                    byte = cfpie.payload.in_uint8();

                    eventCode  = (byte & 0xE0) >> 5;

                    switch (eventCode){
                        case FastPath::FASTPATH_INPUT_EVENT_SCANCODE:
                        {
                            FastPath::KeyboardEvent_Recv ke(cfpie.payload, byte);

                            if (this->verbose & 4){
                                LOG(LOG_INFO,
                                    "Front::Received fast-path PUD, scancode keyboardFlags=0x%X, keyCode=0x%X",
                                    ke.spKeyboardFlags, ke.keyCode);
                            }

                            BStream decoded_data(256);
                            bool    tsk_switch_shortcuts;

                            this->keymap.event(ke.spKeyboardFlags, ke.keyCode, decoded_data, tsk_switch_shortcuts);
                            decoded_data.mark_end();

                            if (  this->capture
                               && (this->capture_state == CAPTURE_STATE_STARTED)
                               && decoded_data.size()) {
                                struct timeval now = tvtime();

                                this->capture->input(now, decoded_data);
                            }

                            if (this->up_and_running) {
                                if (tsk_switch_shortcuts && this->ini->client.disable_tsk_switch_shortcuts.get()) {
                                    LOG(LOG_INFO, "Ctrl+Alt+Del and Ctrl+Shift+Esc keyboard sequences ignored.");
                                }
                                else {
                                    cb.rdp_input_scancode(ke.keyCode, 0, ke.spKeyboardFlags, 0, &this->keymap);
                                }
                            }
                        }
                        break;

                        case FastPath::FASTPATH_INPUT_EVENT_MOUSE:
                        {
                            FastPath::MouseEvent_Recv me(cfpie.payload, byte);

                            if (this->verbose & 4){
                                LOG(LOG_INFO,
                                    "Front::Received fast-path PUD, mouse pointerFlags=0x%X, xPos=0x%X, yPos=0x%X",
                                    me.pointerFlags, me.xPos, me.yPos);
                            }

                            this->mouse_x = me.xPos;
                            this->mouse_y = me.yPos;
                            if (this->up_and_running) {
                                cb.rdp_input_mouse(me.pointerFlags, me.xPos, me.yPos, &this->keymap);
                            }
                        }
                        break;

/*
                        case FastPath::FASTPATH_INPUT_EVENT_MOUSEX:
                        break;
*/

                        case FastPath::FASTPATH_INPUT_EVENT_SYNC:
                        {
                            FastPath::SynchronizeEvent_Recv se(cfpie.payload, byte);

                            if (this->verbose & 4){
                                LOG(LOG_INFO, "Front::Received fast-path PUD, sync eventFlags=0x%X",
                                    se.eventFlags);
                            }

                            this->keymap.synchronize(se.eventFlags & 0xFFFF);
                            if (this->up_and_running) {
                                cb.rdp_input_synchronize(0, 0, se.eventFlags & 0xFFFF, 0);
                            }
                        }
                        break;

/*
                        case FastPath::FASTPATH_INPUT_EVENT_UNICODE:
                        break;
*/

                        default:
                            LOG(LOG_INFO,
                                "Front::Received unexpected fast-path PUD, eventCode = %u",
                                eventCode);
                            throw Error(ERR_RDP_FASTPATH);
                        break;
                    }
                    if (this->verbose & 4){
                        LOG(LOG_INFO, "Front::Received fast-path PUD done");
                    }
                }

                if (cfpie.payload.in_remain() != 0) {
                    LOG(LOG_WARNING, "Front::Received fast-path PUD, remains=%u",
                        cfpie.payload.in_remain());
                }
                break;
            }
            else {
                X224::RecvFactory fx224(*this->trans, stream);
                TODO("We shall put a specific case when we get Disconnect Request")
                if (fx224.type == X224::DR_TPDU){
                    TODO("What is the clean way to actually disconnect ?")
                    X224::DR_TPDU_Recv x224(*this->trans, stream);
                    LOG(LOG_INFO, "Front::Received Disconnect Request from RDP client");
                    throw Error(ERR_X224_EXPECTED_DATA_PDU);
                }
                else if (fx224.type != X224::DT_TPDU){
                    LOG(LOG_ERR, "Front::Unexpected non data PDU (got %u)", fx224.type);
                    throw Error(ERR_X224_EXPECTED_DATA_PDU);
                }

                X224::DT_TPDU_Recv x224(*this->trans, stream);

                MCS::RecvFactory mcs_fac(x224.payload, MCS::PER_ENCODING);
                if (mcs_fac.type == MCS::MCSPDU_DisconnectProviderUltimatum){
                    LOG(LOG_INFO, "Front::incoming::DisconnectProviderUltimatum received");
                    x224.payload.rewind();
                    MCS::DisconnectProviderUltimatum_Recv mcs(x224.payload, MCS::PER_ENCODING);
                    const char * reason = MCS::get_reason(mcs.reason);
                    LOG(LOG_INFO, "Front DisconnectProviderUltimatum: reason=%s [%d]", reason, mcs.reason);
                    throw Error(ERR_MCS);
                }

                MCS::SendDataRequest_Recv mcs(x224.payload, MCS::PER_ENCODING);

                SEC::Sec_Recv sec(mcs.payload, this->decrypt, this->client_info.encryptionLevel);
                if (this->verbose & 128){
                    LOG(LOG_INFO, "sec decrypted payload:");
                    hexdump_d(sec.payload.get_data(), sec.payload.size());
                }

                if (this->verbose & 8){
                    LOG(LOG_INFO, "Front::incoming::sec_flags=%x", sec.flags);
                }

                if (mcs.channelId != GCC::MCS_GLOBAL_CHANNEL) {
                    size_t num_channel_src = this->channel_list.size();
                    for (size_t index = 0; index < this->channel_list.size(); index++){
                        if (this->channel_list[index].chanid == mcs.channelId){
                            num_channel_src = index;
                            break;
                        }
                    }

                    if (this->verbose & 16){
                        LOG(LOG_INFO, "Front::incoming::channel_data channelId=%u", mcs.channelId);
                    }

                    if (num_channel_src >= this->channel_list.size()) {
                        LOG(LOG_ERR, "Front::incoming::Unknown Channel");
                        throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
                    }

                    const CHANNELS::ChannelDef & channel = this->channel_list[num_channel_src];
                    if (this->verbose & 16){
                        channel.log(mcs.channelId);
                    }

                    expected = 8; /* length(4) + flags(4) */
                    if (!sec.payload.in_check_rem(expected)){
                        LOG(LOG_ERR, "Front::incoming::data truncated, need=%u remains=%u",
                            expected, sec.payload.in_remain());
                        throw Error(ERR_MCS);
                    }

                    int length = sec.payload.in_uint32_le();
                    int flags  = sec.payload.in_uint32_le();

                    size_t chunk_size = sec.payload.in_remain();

                    if (this->up_and_running){
                        if (!this->ini->client.device_redirection.get()
                           && !strncmp(this->channel_list[num_channel_src].name, "rdpdr", 8)
                           ) {
                            LOG(LOG_INFO, "Front::incoming::rdpdr channel disabled");
                        }
                        else {
                            if (this->verbose & 16){
                                LOG(LOG_INFO, "Front::send_to_mod_channel");
                            }
                            SubStream chunk(sec.payload, sec.payload.get_offset(), chunk_size);

                            cb.send_to_mod_channel(channel.name, chunk, length, flags);
                        }
                    }
                    else {
                        if (this->verbose & 16){
                            LOG(LOG_INFO, "Front::not up_and_running send_to_mod_channel dropped");
                        }
                    }
                    sec.payload.p += chunk_size;
                }
                else {
                    while (sec.payload.p < sec.payload.end) {
                        ShareControl_Recv sctrl(sec.payload);

                        switch (sctrl.pdu_type1) {
                        case PDUTYPE_DEMANDACTIVEPDU:
                            if (this->verbose & 1){
                                LOG(LOG_INFO, "Front received DEMANDACTIVEPDU (unsupported)");
                            }
                            break;
                        case PDUTYPE_CONFIRMACTIVEPDU:
                            if (this->verbose & 1){
                                LOG(LOG_INFO, "Front received CONFIRMACTIVEPDU");
                            }
                            {
                                expected = 6;   /* shareId(4) + originatorId(2) */
                                if (!sctrl.payload.in_check_rem(expected)){
                                    LOG(LOG_ERR,
                                        "Truncated Confirm active PDU data, need=%u remains=%u",
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
                            BGRPalette palette;
                            init_palette332(palette);
                            {
                                RDPColCache cmd(0, palette);
                                this->orders->draw(cmd);
                            }
                            this->init_pointers();
                            if (this->verbose & 1){
                                LOG(LOG_INFO, "Front received CONFIRMACTIVEPDU done");
                            }

                            break;
                        case PDUTYPE_DATAPDU: /* 7 */
                            if (this->verbose & 8){
                                LOG(LOG_INFO, "Front received DATAPDU");
                            }
                            // this is rdp_process_data that will set up_and_running to 1
                            // when fonts have been received
                            // we will not exit this loop until we are in this state.
                            //LOG(LOG_INFO, "sctrl.payload.len= %u sctrl.len = %u", sctrl.payload.size(), sctrl.len);
                            this->process_data(sctrl.payload, cb);
                            if (this->verbose & 8){
                                LOG(LOG_INFO, "Front received DATAPDU done");
                            }

                            if (!sctrl.payload.check_end())
                            {
                                LOG(LOG_ERR,
                                    "Trailing data after DATAPDU: remains=%u",
                                    sctrl.payload.in_remain());
                                throw Error(ERR_MCS_PDU_TRAILINGDATA);
                            }
                            break;
                        case PDUTYPE_DEACTIVATEALLPDU:
                            if (this->verbose & 1){
                                LOG(LOG_INFO, "Front received DEACTIVATEALLPDU (unsupported)");
                            }
                            break;
                        case PDUTYPE_SERVER_REDIR_PKT:
                            if (this->verbose & 1){
                                LOG(LOG_INFO, "Front received SERVER_REDIR_PKT (unsupported)");
                            }
                            break;
                        default:
                            LOG(LOG_WARNING, "Front received unknown PDU type in session_data (%d)\n", sctrl.pdu_type1);
                            break;
                        }

                        TODO("check all sctrl.payload data is consumed")
                        sec.payload.p = sctrl.payload.p;
                    }
                }

                if (!sec.payload.check_end())
                {
                    LOG(LOG_ERR,
                        "Trailing data after SEC: remains=%u",
                        sec.payload.in_remain());
                    throw Error(ERR_SEC_TRAILINGDATA);
                }
            }
        }
        break;
        }
    }

    void send_data_indication(uint16_t channelId, HStream & stream)
    {
        BStream x224_header(256);
        OutPerBStream mcs_header(256);

        MCS::SendDataIndication_Send mcs(mcs_header, this->userid, channelId,
                                         1, 3, stream.size(),
                                         MCS::PER_ENCODING);

        X224::DT_TPDU_Send(x224_header, stream.size() + mcs_header.size());
        this->trans->send(x224_header, mcs_header, stream);
    }

    void send_data_indication_ex(uint16_t channelId, HStream & stream)
    {
        BStream x224_header(256);
        OutPerBStream mcs_header(256);
        BStream sec_header(256);

        SEC::Sec_Send sec(sec_header, stream, 0, this->encrypt, this->client_info.encryptionLevel);
        stream.copy_to_head(sec_header.get_data(), sec_header.size());

        MCS::SendDataIndication_Send mcs(mcs_header, this->userid, channelId,
                                         1, 3, stream.size(),
                                         MCS::PER_ENCODING);

        X224::DT_TPDU_Send(x224_header, stream.size() + mcs_header.size());

        this->trans->send(x224_header, mcs_header, stream);
    }

/*
    void send_server_update(HStream & data) {
        BStream fastpath_header(256);

        FastPath::ServerUpdatePDU_Send SvrUpdPDU(
              fastpath_header
            , data
            , ((this->client_info.encryptionLevel > 1) ? FastPath::FASTPATH_OUTPUT_ENCRYPTED : 0)
            , this->encrypt
            );
        this->trans->send(fastpath_header, data);
    }
*/
    virtual void send_fastpath_data(Stream & data) {
        HStream stream(1024, 1024 + 65536);

        stream.out_copy_bytes(data.get_data(), data.size());
        stream.mark_end();

        if (this->verbose & 4) {
            LOG(LOG_INFO, "Front::send_data: fast-path");
        }

        BStream fastpath_header(256);

        FastPath::ServerUpdatePDU_Send SvrUpdPDU(
            fastpath_header,
            stream,
            ((this->client_info.encryptionLevel > 1) ?
             FastPath::FASTPATH_OUTPUT_ENCRYPTED : 0),
            this->encrypt);
        this->trans->send(fastpath_header, stream);
    }

    virtual bool retrieve_client_capability_set(Capability & caps) {
        switch (caps.capabilityType) {
            case CAPSTYPE_GENERAL:
                ::memcpy(&caps, &this->client_general_caps, sizeof(this->client_general_caps));
            break;

            case CAPSTYPE_BITMAP:
                ::memcpy(&caps, &this->client_bitmap_caps, sizeof(this->client_bitmap_caps));
            break;

            case CAPSTYPE_ORDER:
                ::memcpy(&caps, &this->client_order_caps, sizeof(this->client_order_caps));
            break;

            case CAPSTYPE_BITMAPCACHE:
                if (use_bitmapcache_rev2) {
                    return false;
                }
                ::memcpy(&caps, &this->client_bmpcache_caps, sizeof(this->client_bmpcache_caps));
            break;

            case CAPSTYPE_OFFSCREENCACHE:
                ::memcpy(&caps, &this->client_offscreencache_caps, sizeof(this->client_offscreencache_caps));
            break;

            case CAPSTYPE_BITMAPCACHE_REV2:
                if (!use_bitmapcache_rev2) {
                    return false;
                }
                ::memcpy(&caps, &this->client_bmpcache2_caps, sizeof(this->client_bmpcache2_caps));
            break;
        }
        return true;
    }

    /*****************************************************************************/
    void send_data_update_sync() throw (Error)
    {
        if (this->verbose & 1){
            LOG(LOG_INFO, "Front::send_data_update_sync");
        }

        if (this->server_fastpath_update_support == false) {
            BStream stream(65536);

            ShareData sdata(stream);
            sdata.emit_begin(PDUTYPE2_UPDATE, this->share_id, RDP::STREAM_MED);

            // Payload
            stream.out_uint16_le(RDP_UPDATE_SYNCHRONIZE);
            stream.out_clear_bytes(2);
            stream.mark_end();

            // Packet trailer
            sdata.emit_end();

            BStream sctrl_header(256);
            ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU,
                this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

            HStream target_stream(1024, 65536);
            target_stream.out_copy_bytes(sctrl_header);
            target_stream.out_copy_bytes(stream);
            target_stream.mark_end();

            if ((this->verbose & (128|1)) == (128|1)){
                LOG(LOG_INFO, "Sec clear payload to send:");
                hexdump_d(target_stream.get_data(), target_stream.size());
            }

            this->send_data_indication_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);
        }
        else {
            if (this->verbose & 4){
                LOG(LOG_INFO, "Front::send_data_update_sync: fast-path");
            }
            HStream stream(256, 65536);

            size_t header_size = FastPath::Update_Send::GetSize(0);

            stream.out_clear_bytes(header_size); // Fast-Path Update (TS_FP_UPDATE structure) size
            stream.mark_end();

            SubStream Upd_s(stream, 0, header_size);

            FastPath::Update_Send Upd( Upd_s
                                     , stream.size() - header_size
                                     , FastPath::FASTPATH_UPDATETYPE_SYNCHRONIZE
                                     , FastPath::FASTPATH_FRAGMENT_SINGLE
                                     , /*FastPath:: FASTPATH_OUTPUT_COMPRESSION_USED*/0
                                     , 0
                                     );

            BStream fastpath_header(256);

            FastPath::ServerUpdatePDU_Send SvrUpdPDU(
                  fastpath_header
                , stream
                , ((this->client_info.encryptionLevel > 1) ? FastPath::FASTPATH_OUTPUT_ENCRYPTED : 0)
                , this->encrypt
                );
            this->trans->send(fastpath_header, stream);
        }
    }

    /*****************************************************************************/
    void send_demand_active() throw (Error)
    {
        if (this->verbose & 1){
            LOG(LOG_INFO, "Front::send_demand_active");
        }

        BStream stream(65536);

        // Payload
        size_t caps_count = 0;
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
        if (this->server_fastpath_update_support) {
            general_caps.extraflags |= FASTPATH_OUTPUT_SUPPORTED;
        }
        if (!this->server_capabilities_filename.is_empty()) {
            GeneralCapsLoader generalcaps_loader(general_caps);

            ConfigurationLoader cfg_loader(generalcaps_loader, this->server_capabilities_filename.c_str());
        }
        if (this->verbose) {
            general_caps.log("Sending to client");
        }
        general_caps.emit(stream);
        caps_count++;

        BitmapCaps bitmap_caps;
        bitmap_caps.preferredBitsPerPixel = this->client_info.bpp;
        bitmap_caps.desktopWidth = this->client_info.width;
        bitmap_caps.desktopHeight = this->client_info.height;
        bitmap_caps.drawingFlags = DRAW_ALLOW_SKIP_ALPHA;
        if (!this->server_capabilities_filename.is_empty()) {
            BitmapCapsLoader bitmapcaps_loader(bitmap_caps);

            ConfigurationLoader cfg_loader(bitmapcaps_loader, this->server_capabilities_filename.c_str());
        }
        if (this->verbose) {
            bitmap_caps.log("Sending to client");
        }
        bitmap_caps.emit(stream);
        caps_count++;

        FontCaps font_caps;
        if (this->verbose) {
            font_caps.log("Sending to client");
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
        if (!this->server_capabilities_filename.is_empty()) {
            OrderCapsLoader ordercaps_loader(order_caps);

            ConfigurationLoader cfg_loader(ordercaps_loader, this->server_capabilities_filename.c_str());
        }
        if (this->verbose) {
            order_caps.log("Sending to client");
        }
        order_caps.emit(stream);
        caps_count++;

        if (this->ini->client.persistent_disk_bitmap_cache) {
            BitmapCacheHostSupportCaps bitmap_cache_host_support_caps;
            if (this->verbose) {
                bitmap_cache_host_support_caps.log("Sending to client");
            }
            bitmap_cache_host_support_caps.emit(stream);
            caps_count++;
        }

        ColorCacheCaps colorcache_caps;
        if (this->verbose) {
            colorcache_caps.log("Sending to client");
        }
        colorcache_caps.emit(stream);
        caps_count++;

        PointerCaps pointer_caps;
        pointer_caps.colorPointerCacheSize = 0x19;
        pointer_caps.pointerCacheSize = 0x19;
        if (this->verbose) {
            pointer_caps.log("Sending to client");
        }
        pointer_caps.emit(stream);
        caps_count++;

        ShareCaps share_caps;
        share_caps.nodeId = this->userid + GCC::MCS_USERCHANNEL_BASE;
        share_caps.pad2octets = 0xb5e2; /* 0x73e1 */
        if (this->verbose) {
            share_caps.log("Sending to client");
        }
        share_caps.emit(stream);
        caps_count++;

        InputCaps input_caps;
// Slow/Fast-path
        if (this->client_fastpath_input_event_support == false) {
            input_caps.inputFlags = INPUT_FLAG_SCANCODES;
        }
        else {
            input_caps.inputFlags = INPUT_FLAG_SCANCODES | INPUT_FLAG_FASTPATH_INPUT | INPUT_FLAG_FASTPATH_INPUT2;
        }
        input_caps.keyboardLayout = 0;
        input_caps.keyboardType = 0;
        input_caps.keyboardSubType = 0;
        input_caps.keyboardFunctionKey = 0;
        if (this->verbose) {
            input_caps.log("Sending to client");
        }
        input_caps.emit(stream);
        caps_count++;

        size_t caps_size = stream.get_offset() - caps_count_offset;
        stream.set_out_uint16_le(caps_size, caps_size_offset);
        stream.set_out_uint32_le(caps_count, caps_count_offset);

        stream.out_clear_bytes(4); /* sessionId(4). This field is ignored by the client. */
        stream.mark_end();

        BStream sctrl_header(256);
        ShareControl_Send(sctrl_header, PDUTYPE_DEMANDACTIVEPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

        HStream target_stream(1024, 65536);
        target_stream.out_copy_bytes(sctrl_header);
        target_stream.out_copy_bytes(stream);
        target_stream.mark_end();

        if ((this->verbose & (128|1)) == (128|1)){
            LOG(LOG_INFO, "Sec clear payload to send:");
            hexdump_d(target_stream.get_data(), target_stream.size());
        }

        this->send_data_indication_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);
    }

    /* store the number of client cursor cache in client_info */
    void capset_pointercache(Stream & stream, int len)
    {
        if (this->verbose & 32){
            LOG(LOG_INFO, "capset_pointercache");
        }
    }

    void process_confirm_active(Stream & stream)
    {
        if (this->verbose & 1){
            LOG(LOG_INFO, "process_confirm_active");
        }
        TODO("We should separate the parts relevant to caps processing and the part relevant to actual confirm active")
        TODO("Server Caps management should go to RDP layer and be unified between client (mod/rdp.hpp and server code front.hpp)")

        unsigned expected = 4; /* lengthSourceDescriptor(2) + lengthCombinedCapabilities(2) */
        if (!stream.in_check_rem(expected)){
            LOG(LOG_ERR, "Truncated CONFIRMACTIVE PDU, need=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        uint16_t lengthSourceDescriptor = stream.in_uint16_le(); /* sizeof RDP_SOURCE */
        uint16_t lengthCombinedCapabilities = stream.in_uint16_le();

        if (!stream.in_check_rem(lengthSourceDescriptor)){
            LOG(LOG_ERR, "Truncated CONFIRMACTIVE PDU lengthSourceDescriptor, need=%u remains=%u",
                lengthSourceDescriptor, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        stream.in_skip_bytes(lengthSourceDescriptor);

        if (this->verbose & 1){
            LOG(LOG_INFO, "lengthSourceDescriptor = %u", lengthSourceDescriptor);
            LOG(LOG_INFO, "lengthCombinedCapabilities = %u", lengthCombinedCapabilities);
        }

        uint8_t * start = stream.p;
        uint8_t* theoricCapabilitiesEnd = start + lengthCombinedCapabilities;
        uint8_t* actualCapabilitiesEnd = stream.end;

        expected = 4; /* numberCapabilities(2) + pad(2) */
        if (!stream.in_check_rem(expected)){
            LOG(LOG_ERR, "Truncated CONFIRMACTIVE PDU numberCapabilities, need=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        int numberCapabilities = stream.in_uint16_le();
        stream.in_skip_bytes(2); /* pad */

        for (int n = 0; n < numberCapabilities; n++) {
            if (this->verbose & 32){
                LOG(LOG_INFO, "Front::capability %u / %u", n, numberCapabilities );
            }
            if (stream.p + 4 > theoricCapabilitiesEnd) {
                LOG(LOG_ERR, "Incomplete capabilities received (bad length): expected length=%d need=%d available=%d",
                    lengthCombinedCapabilities,
                    stream.p-start,
                    stream.in_remain());
            }
            if (stream.p + 4 > actualCapabilitiesEnd) {
                LOG(LOG_ERR, "Incomplete capabilities received (need more data): expected length=%d need=%d available=%d",
                    lengthCombinedCapabilities,
                    stream.p-start,
                    stream.in_remain());
                return;
            }

            uint16_t capset_type = stream.in_uint16_le();
            uint16_t capset_length = stream.in_uint16_le();
            uint8_t * next = (stream.p + capset_length) - 4;

            switch (capset_type) {
            case CAPSTYPE_GENERAL: {
                    this->client_general_caps.recv(stream, capset_length);
                    if (this->verbose) {
                        this->client_general_caps.log("Receiving from client");
                    }
                    this->client_info.use_compact_packets =
                        (this->client_general_caps.extraflags & NO_BITMAP_COMPRESSION_HDR) ?
                        1 : 0;

                    this->server_fastpath_update_support =
                        (   this->fastpath_support
                         && ((this->client_general_caps.extraflags & FASTPATH_OUTPUT_SUPPORTED) != 0)
                        );
                }
                break;
            case CAPSTYPE_BITMAP: {
                    this->client_bitmap_caps.recv(stream, capset_length);
                    if (this->verbose) {
                        this->client_bitmap_caps.log("Receiving from client");
                    }
/*
                    this->client_info.bpp    =
                          (this->client_bitmap_caps.preferredBitsPerPixel >= 24)
                        ? 24 : this->client_bitmap_caps.preferredBitsPerPixel;
*/
                    // Fixed bug in rdesktop
                    // Desktop size in Client Core Data != Desktop size in Bitmap Capability Set
                    if (!this->client_info.width || !this->client_info.height)
                    {
                        this->client_info.width  = this->client_bitmap_caps.desktopWidth;
                        this->client_info.height = this->client_bitmap_caps.desktopHeight;
                    }
                }
                break;
            case CAPSTYPE_ORDER: { /* 3 */
                    this->client_order_caps.recv(stream, capset_length);
                    if (this->verbose) {
                        this->client_order_caps.log("Receiving from client");
                    }
                }
                break;
            case CAPSTYPE_BITMAPCACHE: {
                    this->client_bmpcache_caps.recv(stream, capset_length);
                    if (this->verbose) {
                        this->client_bmpcache_caps.log("Receiving from client");
                    }
                    this->client_info.number_of_cache      = 3;
                    this->client_info.cache1_entries       = this->client_bmpcache_caps.cache0Entries;
                    this->client_info.cache1_persistent    = false;
                    this->client_info.cache1_size          = this->client_bmpcache_caps.cache0MaximumCellSize;
                    this->client_info.cache2_entries       = this->client_bmpcache_caps.cache1Entries;
                    this->client_info.cache2_persistent    = false;
                    this->client_info.cache2_size          = this->client_bmpcache_caps.cache1MaximumCellSize;
                    this->client_info.cache3_entries       = this->client_bmpcache_caps.cache2Entries;
                    this->client_info.cache3_persistent    = false;
                    this->client_info.cache3_size          = this->client_bmpcache_caps.cache2MaximumCellSize;
                    this->client_info.cache4_entries       = 0;
                    this->client_info.cache5_entries       = 0;
                    this->client_info.cache_flags          = 0;
                    this->client_info.bitmap_cache_version = 0;
                }
                break;
            case CAPSTYPE_CONTROL: /* 5 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSTYPE_CONTROL");
                }
                break;
            case CAPSTYPE_ACTIVATION: /* 7 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSTYPE_ACTIVATION");
                }
                break;
            case CAPSTYPE_POINTER: {  /* 8 */
                    if (this->verbose) {
                        LOG(LOG_INFO, "Receiving from client CAPSTYPE_POINTER");
                    }

                    expected = 4; /* color pointer(2) + pointer_cache_entries(2) */
                    if (!stream.in_check_rem(expected)){
                        LOG(LOG_ERR, "Truncated CAPSTYPE_POINTER, need=%u remains=%u",
                            expected, stream.in_remain());
                        throw Error(ERR_MCS_PDU_TRUNCATED);
                    }

                    stream.in_skip_bytes(2); /* color pointer */
                    int i = stream.in_uint16_le();
                    this->client_info.pointer_cache_entries = std::min(i, 32);
                }
                break;
            case CAPSTYPE_SHARE: /* 9 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSTYPE_SHARE");
                }
                break;
            case CAPSTYPE_COLORCACHE: /* 10 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSTYPE_COLORCACHE");
                }
                break;
            case CAPSTYPE_SOUND:
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSTYPE_SOUND");
                }
                break;
            case CAPSTYPE_INPUT: /* 13 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSTYPE_INPUT");
                }
                break;
            case CAPSTYPE_FONT: /* 14 */
                break;
            case CAPSTYPE_BRUSH: { /* 15 */
                    if (this->verbose) {
                        LOG(LOG_INFO, "Receiving from client CAPSTYPE_BRUSH");
                    }
                    BrushCacheCaps brushcache_caps;
                    brushcache_caps.recv(stream, capset_length);
                    if (this->verbose) {
                        brushcache_caps.log("Receiving from client");
                    }
                    this->client_info.brush_cache_code = brushcache_caps.brushSupportLevel;
                }
                break;
            case CAPSTYPE_GLYPHCACHE: /* 16 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSTYPE_GLYPHCACHE");
                }
                break;
            case CAPSTYPE_OFFSCREENCACHE: /* 17 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSTYPE_OFFSCREENCACHE");
                }
                this->client_offscreencache_caps.recv(stream, capset_length);
                if (this->verbose) {
                    this->client_offscreencache_caps.log("Receiving from client");
                }
                break;
            case CAPSTYPE_BITMAPCACHE_HOSTSUPPORT: /* 18 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSTYPE_BITMAPCACHE_HOSTSUPPORT");
                }
                break;
            case CAPSTYPE_BITMAPCACHE_REV2: {
                    this->use_bitmapcache_rev2 = true;

                    this->client_bmpcache2_caps.recv(stream, capset_length);
                    this->client_bmpcache2_caps.log("Receiving from client");

                    TODO("We only use the first 3 caches (those existing in Rev1), we should have 2 more caches for rev2")
                    this->client_info.number_of_cache = this->client_bmpcache2_caps.numCellCaches;
                    int Bpp = nbbytes(this->client_info.bpp);
                    if (this->client_bmpcache2_caps.numCellCaches > 0) {
                        this->client_info.cache1_entries    = (this->client_bmpcache2_caps.bitmapCache0CellInfo & 0x7fffffff);
                        this->client_info.cache1_persistent = (this->client_bmpcache2_caps.bitmapCache0CellInfo & 0x80000000);
                        this->client_info.cache1_size       = 256 * Bpp;
                    }
                    else {
                        this->client_info.cache1_entries = 0;
                    }
                    if (this->client_bmpcache2_caps.numCellCaches > 1) {
                        this->client_info.cache2_entries    = (this->client_bmpcache2_caps.bitmapCache1CellInfo & 0x7fffffff);
                        this->client_info.cache2_persistent = (this->client_bmpcache2_caps.bitmapCache1CellInfo & 0x80000000);
                        this->client_info.cache2_size       = 1024 * Bpp;
                    }
                    else {
                        this->client_info.cache2_entries = 0;
                    }
                    if (this->client_bmpcache2_caps.numCellCaches > 2) {
                        this->client_info.cache3_entries    = (this->client_bmpcache2_caps.bitmapCache2CellInfo & 0x7fffffff);
                        this->client_info.cache3_persistent = (this->client_bmpcache2_caps.bitmapCache2CellInfo & 0x80000000);
                        this->client_info.cache3_size       = 4096 * Bpp;
                    }
                    else {
                        this->client_info.cache3_entries = 0;
                    }
                    if (this->client_bmpcache2_caps.numCellCaches > 3) {
                        this->client_info.cache4_entries    = (this->client_bmpcache2_caps.bitmapCache3CellInfo & 0x7fffffff);
                        this->client_info.cache4_persistent = (this->client_bmpcache2_caps.bitmapCache3CellInfo & 0x80000000);
                        this->client_info.cache4_size       = 6144 * Bpp;
                    }
                    else {
                        this->client_info.cache4_entries = 0;
                    }
                    if (this->client_bmpcache2_caps.numCellCaches > 4) {
                        this->client_info.cache5_entries    = (this->client_bmpcache2_caps.bitmapCache4CellInfo & 0x7fffffff);
                        this->client_info.cache5_persistent = (this->client_bmpcache2_caps.bitmapCache4CellInfo & 0x80000000);
                        this->client_info.cache5_size       = 8192 * Bpp;
                    }
                    else {
                        this->client_info.cache5_entries = 0;
                    }
                    this->client_info.cache_flags          = this->client_bmpcache2_caps.cacheFlags;
                    this->client_info.bitmap_cache_version = 2;
                }
                break;
            case CAPSTYPE_VIRTUALCHANNEL: /* 20 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSTYPE_VIRTUALCHANNEL");
                }
                break;
            case CAPSTYPE_DRAWNINEGRIDCACHE: /* 21 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSTYPE_DRAWNINEGRIDCACHE");
                }
                break;
            case CAPSTYPE_DRAWGDIPLUS: /* 22 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSTYPE_DRAWGDIPLUS");
                }
                break;
            case CAPSTYPE_RAIL: /* 23 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSTYPE_RAIL");
                }
                break;
            case CAPSTYPE_WINDOW: /* 24 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSTYPE_WINDOW");
                }
                break;
            case CAPSETTYPE_COMPDESK: { /* 25 */
                    CompDeskCaps cap;
                    cap.recv(stream, capset_length);
                    if (this->verbose) {
                        cap.log("Receiving from client");
                    }
                }
                break;
            case CAPSETTYPE_MULTIFRAGMENTUPDATE: { /* 26 */
                    MultiFragmentUpdateCaps cap;
                    cap.recv(stream, capset_length);
                    if (this->verbose) {
                        cap.log("Receiving from client");
                    }
                }
                break;
            case CAPSETTYPE_LARGE_POINTER: /* 27 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSETTYPE_LARGE_POINTER");
                }
                break;
            case CAPSETTYPE_SURFACE_COMMANDS: /* 28 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSETTYPE_SURFACE_COMMANDS");
                }
                break;
            case CAPSETTYPE_BITMAP_CODECS: /* 29 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSETTYPE_BITMAP_CODECS");
                }
                break;
            case CAPSETTYPE_FRAME_ACKNOWLEDGE: /* 30 */
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client CAPSETTYPE_FRAME_ACKNOWLEDGE");
                }
                break;
            default:
                if (this->verbose) {
                    LOG(LOG_INFO, "Receiving from client unknown caps %u", capset_type);
                }
                break;
            }
            if (stream.p > next){
                LOG(LOG_ERR, "read out of bound detected");
                throw Error(ERR_MCS);
            }
            stream.p = next;
        }
        // After Capabilities read optional SessionId
        if (stream.in_remain() >= 4){
            // From the documentation SessionId is ignored by client.
            stream.in_skip_bytes(4); /* Session Id */
        }
        if (this->verbose & 1){
            LOG(LOG_INFO, "process_confirm_active done p=%p end=%p", stream.p, stream.end);
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

    TODO(" duplicated code in mod/rdp");
    void send_synchronize()
    {
        if (this->verbose & 1){
            LOG(LOG_INFO, "send_synchronize");
        }

        BStream stream(65536);
        ShareData sdata(stream);
        sdata.emit_begin(PDUTYPE2_SYNCHRONIZE, this->share_id, RDP::STREAM_MED);

        // Payload
        stream.out_uint16_le(1);    // messageType
        stream.out_uint16_le(1002); // control id
        stream.mark_end();

        // Packet trailer
        sdata.emit_end();

        BStream sctrl_header(256);
        ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

        HStream target_stream(1024, 65536);
        target_stream.out_copy_bytes(sctrl_header);
        target_stream.out_copy_bytes(stream);
        target_stream.mark_end();

        if ((this->verbose & (128|1)) == (128|1)){
            LOG(LOG_INFO, "Sec clear payload to send:");
            hexdump_d(target_stream.get_data(), target_stream.size());
        }

        this->send_data_indication_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);

        if (this->verbose & 1){
            LOG(LOG_INFO, "send_synchronize done");
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
        if (this->verbose & 1){
            LOG(LOG_INFO, "send_control action=%u", action);
        }

        BStream stream(65536);
        ShareData sdata(stream);
        sdata.emit_begin(PDUTYPE2_CONTROL, this->share_id, RDP::STREAM_MED);

        // Payload
        stream.out_uint16_le(action);
        stream.out_uint16_le(0); /* userid */
        stream.out_uint32_le(1002); /* control id */
        stream.mark_end();

        // Packet trailer
        sdata.emit_end();

        BStream sctrl_header(256);
        ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

        HStream target_stream(1024, 65536);
        target_stream.out_copy_bytes(sctrl_header);
        target_stream.out_copy_bytes(stream);
        target_stream.mark_end();

        if ((this->verbose & (128|1)) == (128|1)){
            LOG(LOG_INFO, "Sec clear payload to send:");
            hexdump_d(target_stream.get_data(), target_stream.size());
        }

        this->send_data_indication_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);

        if (this->verbose & 1){
            LOG(LOG_INFO, "send_control action=%u", action);
        }
    }

    /*****************************************************************************/
    void send_fontmap() throw (Error)
    {
        if (this->verbose & 1){
            LOG(LOG_INFO, "send_fontmap");
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

        BStream stream(65536);
        ShareData sdata(stream);
        sdata.emit_begin(PDUTYPE2_FONTMAP, this->share_id, RDP::STREAM_MED);

        // Payload
        stream.out_copy_bytes((char*)g_fontmap, 172);
        stream.mark_end();

        // Packet trailer
        sdata.emit_end();

        BStream sctrl_header(256);
        ShareControl_Send sctrl(sctrl_header, PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

        HStream target_stream(1024, 65535);
        target_stream.out_copy_bytes(sctrl_header);
        target_stream.out_copy_bytes(stream);
        target_stream.mark_end();

        if ((this->verbose & (128|1)) == (128|1)){
            LOG(LOG_INFO, "Sec clear payload to send:");
            hexdump_d(target_stream.get_data(), target_stream.size());
        }

        this->send_data_indication_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);

        if (this->verbose & 1){
            LOG(LOG_INFO, "send_fontmap");
        }
    }

    /* PDUTYPE_DATAPDU */
    void process_data(Stream & stream, Callback & cb) throw (Error)
    {
        unsigned expected;
        if (this->verbose & 8){
            LOG(LOG_INFO, "Front::process_data(...)");
        }
        ShareData sdata_in(stream);
        sdata_in.recv_begin();
        if (this->verbose & 8){
            LOG(LOG_INFO, "sdata_in.pdutype2=%u"
                          " sdata_in.len=%u"
                          " sdata_in.compressedLen=%u"
                          " remains=%u"
                          " payload_len=%u",
                (unsigned)sdata_in.pdutype2,
                (unsigned)sdata_in.len,
                (unsigned)sdata_in.compressedLen,
                (unsigned)(stream.in_remain()),
                (unsigned)(sdata_in.payload.size())
            );
        }

        switch (sdata_in.pdutype2) {
        case PDUTYPE2_UPDATE:  // Update PDU (section 2.2.9.1.1.3)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_UPDATE");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;
        case PDUTYPE2_CONTROL: // 20(0x14) Control PDU (section 2.2.1.15.1)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_CONTROL");
            }
            {
                expected = 8;   /* action(2) + grantId(2) + controlId(4) */
                if (!sdata_in.payload.in_check_rem(expected)){
                    LOG(LOG_ERR, "Truncated Control PDU data, need=%u remains=%u",
                        expected, sdata_in.payload.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }

                int action = sdata_in.payload.in_uint16_le();
                sdata_in.payload.in_skip_bytes(2); /* user id */
                sdata_in.payload.in_skip_bytes(4); /* control id */
                switch (action){
                    case RDP_CTL_REQUEST_CONTROL:
                        this->send_control(RDP_CTL_GRANT_CONTROL);
                    break;
                    case RDP_CTL_COOPERATE:
                        this->send_control(RDP_CTL_COOPERATE);
                    break;
                    default:
                        LOG(LOG_WARNING, "process DATA_PDU_CONTROL unknown action (%d)\n", action);
                }
            }
            break;
        case PDUTYPE2_POINTER: // Pointer Update PDU (section 2.2.9.1.1.4)
            if (this->verbose & 4){
                LOG(LOG_INFO, "PDUTYPE2_POINTER");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;
        case PDUTYPE2_INPUT:   // 28(0x1c) Input PDU (section 2.2.8.1.1.3)
            {
                SlowPath::ClientInputEventPDU_Recv cie(sdata_in.payload);

                if (this->verbose & 4){
                    LOG(LOG_INFO, "PDUTYPE2_INPUT num_events=%u", cie.numEvents);
                }

                for (int index = 0; index < cie.numEvents; index++) {
                    SlowPath::InputEvent_Recv ie(cie.payload);

                    TODO(" we should always call send_input with original data  if the other side is rdp it will merely transmit it to the other end without change. If the other side is some internal module it will be it's own responsibility to decode it")
                    TODO(" with the scheme above  any kind of keymap management is only necessary for internal modules or if we convert mapping. But only the back-end module really knows what the target mapping should be.")
                    switch (ie.messageType) {
                        case SlowPath::INPUT_EVENT_SYNC:
                        {
                            SlowPath::SynchronizeEvent_Recv se(ie.payload);

                            if (this->verbose & 4){
                                LOG(LOG_INFO, "SlowPath INPUT_EVENT_SYNC eventTime=%u toggleFlags=0x%04X",
                                    ie.eventTime, se.toggleFlags);
                            }
                            // happens when client gets focus and sends key modifier info
                            this->keymap.synchronize(se.toggleFlags & 0xFFFF);
                            if (this->up_and_running){
                                cb.rdp_input_synchronize(ie.eventTime, 0, se.toggleFlags & 0xFFFF, (se.toggleFlags & 0xFFFF0000) >> 16);
                            }
                        }
                        break;

                        case SlowPath::INPUT_EVENT_MOUSE:
                        {
                            SlowPath::MouseEvent_Recv me(ie.payload);

                            if (this->verbose & 4){
                                LOG(LOG_INFO, "Slow-path INPUT_EVENT_MOUSE eventTime=%u pointerFlags=0x%04X, xPos=%u, yPos=%u)",
                                    ie.eventTime, me.pointerFlags, me.xPos, me.yPos);
                            }
                            this->mouse_x = me.xPos;
                            this->mouse_y = me.yPos;
                            if (this->up_and_running){
                                cb.rdp_input_mouse(me.pointerFlags, me.xPos, me.yPos, &this->keymap);
                            }
                        }
                        break;

                        case SlowPath::INPUT_EVENT_SCANCODE:
                        {
                            SlowPath::KeyboardEvent_Recv ke(ie.payload);

                            if (this->verbose & 4){
                                LOG(LOG_INFO, "Slow-path INPUT_EVENT_SYNC eventTime=%u keyboardFlags=0x%04X keyCode=0x%04X",
                                    ie.eventTime, ke.keyboardFlags, ke.keyCode);
                            }

                            BStream decoded_data(256);
                            bool    tsk_switch_shortcuts;

                            this->keymap.event(ke.keyboardFlags, ke.keyCode, decoded_data, tsk_switch_shortcuts);
                            decoded_data.mark_end();

                            if (  this->capture
                               && (this->capture_state == CAPTURE_STATE_STARTED)
                               && decoded_data.size()) {
                                struct timeval now = tvtime();

                                this->capture->input(now, decoded_data);
                            }

                            if (this->up_and_running) {
                                if (tsk_switch_shortcuts && this->ini->client.disable_tsk_switch_shortcuts.get()) {
                                    LOG(LOG_INFO, "Ctrl+Alt+Del and Ctrl+Shift+Esc keyboard sequences ignored.");
                                }
                                else {
                                    cb.rdp_input_scancode(ke.keyCode, 0, ke.keyboardFlags, ie.eventTime, &this->keymap);
                                }
                            }
                        }
                        break;

                        default:
                            LOG(LOG_WARNING, "unsupported PDUTYPE2_INPUT message type %u", ie.messageType);
                        break;
                    }
                }
                if (this->verbose & 4){
                    LOG(LOG_INFO, "PDUTYPE2_INPUT done");
                }
            }
        break;
        case PDUTYPE2_SYNCHRONIZE:  // Synchronize PDU (section 2.2.1.14.1)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_SYNCHRONIZE");
            }
            {
                expected = 4;   /* messageType(2) + targetUser(4) */
                if (!sdata_in.payload.in_check_rem(expected)){
                    LOG(LOG_ERR, "Truncated Synchronize PDU data, need=%u remains=%u",
                        expected, sdata_in.payload.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }

                uint16_t messageType = sdata_in.payload.in_uint16_le();
                uint16_t controlId = sdata_in.payload.in_uint16_le();
                if (this->verbose & 8){
                    LOG(LOG_INFO, "PDUTYPE2_SYNCHRONIZE"
                                  " messageType=%u controlId=%u",
                                  (unsigned)messageType,
                                  static_cast<unsigned>(controlId));
                }
                this->send_synchronize();
            }
        break;
        case PDUTYPE2_REFRESH_RECT: // Refresh Rect PDU (section 2.2.11.2.1)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_REFRESH_RECT");
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
                if (!sdata_in.payload.in_check_rem(expected)){
                    LOG(LOG_ERR, "Truncated Refresh rect PDU data, need=%u remains=%u",
                        expected, sdata_in.payload.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }

                size_t numberOfAreas = sdata_in.payload.in_uint8();
                sdata_in.payload.in_skip_bytes(3);

                expected = numberOfAreas * 8;   /* numberOfAreas * (left(2) + top(2) + right(2) + bottom(2)) */
                if (!sdata_in.payload.in_check_rem(expected)){
                    LOG(LOG_ERR, "Truncated Refresh rect PDU data, need=%u remains=%u",
                        expected, sdata_in.payload.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }

                for (size_t i = 0; i < numberOfAreas ; i++){

                    int left = sdata_in.payload.in_uint16_le();
                    int top = sdata_in.payload.in_uint16_le();
                    int right = sdata_in.payload.in_uint16_le();
                    int bottom = sdata_in.payload.in_uint16_le();
                    Rect rect(left, top, (right - left) + 1, (bottom - top) + 1);
                    if (this->verbose & (64|4)){
                        LOG(LOG_INFO, "PDUTYPE2_REFRESH_RECT"
                            " left=%u top=%u right=%u bottom=%u cx=%u cy=%u",
                            left, top, right, bottom, rect.x, rect.cy);
                    }
                    TODO("we should consider adding to API some function to refresh several rects at once")
                    if (this->up_and_running){
                        cb.rdp_input_invalidate(rect);
                    }
                }
            }
        break;
        case PDUTYPE2_PLAY_SOUND:   // Play Sound PDU (section 2.2.9.1.1.5.1):w
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_PLAY_SOUND");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;
        case PDUTYPE2_SUPPRESS_OUTPUT:  // Suppress Output PDU (section 2.2.11.3.1)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_SUPPRESS_OUTPUT");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;

            // PDUTYPE2_SUPPRESS_OUTPUT comes when minimizing a full screen
            // mstsc.exe 2600. I think this is saying the client no longer wants
            // screen updates and it will issue a PDUTYPE2_REFRESH_RECT above
            // to catch up so minimized apps don't take bandwidth
            break;

        break;
        case PDUTYPE2_SHUTDOWN_REQUEST: // Shutdown Request PDU (section 2.2.2.2.1)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_SHUTDOWN_REQUEST");
            }
            {
                // when this message comes, send a PDUTYPE2_SHUTDOWN_DENIED back
                // so the client is sure the connection is alive and it can ask
                // if user really wants to disconnect */

                BStream stream(65536);
                ShareData sdata_out(stream);
                sdata_out.emit_begin(PDUTYPE2_SHUTDOWN_DENIED, this->share_id, RDP::STREAM_MED);
                stream.mark_end();

                // Packet trailer
                sdata_out.emit_end();

                BStream sctrl_header(256);
                ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

                HStream target_stream(1024, 65536);
                target_stream.out_copy_bytes(sctrl_header);
                target_stream.out_copy_bytes(stream);
                target_stream.mark_end();

                if ((this->verbose & (128|8)) == (128|8)){
                    LOG(LOG_INFO, "Sec clear payload to send:");
                    hexdump_d(target_stream.get_data(), target_stream.size());
                }

                this->send_data_indication_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);
            }
        break;
        case PDUTYPE2_SHUTDOWN_DENIED:  // Shutdown Request Denied PDU (section 2.2.2.3.1)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_SHUTDOWN_DENIED");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;
        case PDUTYPE2_SAVE_SESSION_INFO: // Save Session Info PDU (section 2.2.10.1.1)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_SAVE_SESSION_INFO");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;
        case PDUTYPE2_FONTLIST: // 39(0x27) Font List PDU (section 2.2.1.18.1)
        {
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_FONTLIST");
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
            if (!sdata_in.payload.in_check_rem(expected)){
                LOG(LOG_ERR, "Truncated Font list PDU data, need=%u remains=%u",
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

                if (this->client_info.bpp == 8){
                    BGRPalette palette;
                    init_palette332(palette);
                    RDPColCache cmd(0, palette);
                    this->orders->draw(cmd);
                }
                this->init_pointers();

                if (this->verbose & (8|1)){
                    LOG(LOG_INFO, "--------------> UP AND RUNNING <----------------");
                }
                cb.rdp_input_up_and_running();
                this->up_and_running = 1;
                cb.on_front_up_and_running();
                TODO("we should use accessors to set that, also not sure it's the right place to set it")
                this->ini->context.opt_width.set(this->client_info.width);
                this->ini->context.opt_height.set(this->client_info.height);
                this->ini->context.opt_bpp.set(this->client_info.bpp);
                char username_a_domain[512];
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
                this->ini->parse_username(username);
                if (this->client_info.password[0]) {
                    this->ini->context_set_value(AUTHID_PASSWORD, this->client_info.password);
                }
            }
        }
        break;
        case PDUTYPE2_FONTMAP:  // Font Map PDU (section 2.2.1.22.1)
            if (this->verbose & (8|1)){
                LOG(LOG_INFO, "PDUTYPE2_FONTMAP");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;
        case PDUTYPE2_SET_KEYBOARD_INDICATORS: // Set Keyboard Indicators PDU (section 2.2.8.2.1.1)
            if (this->verbose & (4|8)){
                LOG(LOG_INFO, "PDUTYPE2_SET_KEYBOARD_INDICATORS");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;
        case PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST: // Persistent Key List PDU (section 2.2.1.17.1)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST");
            }

            if (!sdata_in.payload.in_check_rem(sdata_in.len)){
                LOG(LOG_ERR, "Truncated Persistent key list PDU data, need=%u remains=%u",
                    sdata_in.len, sdata_in.payload.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            {
                RDP::PersistentKeyListPDUData pklpdud;

                pklpdud.receive(sdata_in.payload);
                pklpdud.log(LOG_INFO, "Receiving from client Persistent Key List PDU Data");

                static uint16_t cache_0_entry_index = 0;
                static uint16_t cache_1_entry_index = 0;
                static uint16_t cache_2_entry_index = 0;
                static uint16_t cache_3_entry_index = 0;
                static uint16_t cache_4_entry_index = 0;

                const char                                * client_persistent_path = PERSISTENT_PATH "/client";
                      RDP::BitmapCachePersistentListEntry * entries                = pklpdud.entries;

                if (pklpdud.numEntriesCache0) {
                    this->bmp_cache->load_from_disk(client_persistent_path, 0, entries, pklpdud.numEntriesCache0,
                        cache_0_entry_index);
                    entries             += pklpdud.numEntriesCache0;
                    cache_0_entry_index += pklpdud.numEntriesCache0;
                }
                if (pklpdud.numEntriesCache1) {
                    this->bmp_cache->load_from_disk(client_persistent_path, 1, entries, pklpdud.numEntriesCache1,
                        cache_1_entry_index);
                    entries             += pklpdud.numEntriesCache1;
                    cache_1_entry_index += pklpdud.numEntriesCache1;
                }
                if (pklpdud.numEntriesCache2) {
                    this->bmp_cache->load_from_disk(client_persistent_path, 2, entries, pklpdud.numEntriesCache2,
                        cache_2_entry_index);
                    entries             += pklpdud.numEntriesCache2;
                    cache_2_entry_index += pklpdud.numEntriesCache2;
                }
                if (pklpdud.numEntriesCache3) {
                    this->bmp_cache->load_from_disk(client_persistent_path, 3, entries, pklpdud.numEntriesCache3,
                        cache_3_entry_index);
                    entries             += pklpdud.numEntriesCache3;
                    cache_3_entry_index += pklpdud.numEntriesCache3;
                }
                if (pklpdud.numEntriesCache4) {
                    this->bmp_cache->load_from_disk(client_persistent_path, 4, entries, pklpdud.numEntriesCache4,
                        cache_4_entry_index);
                    entries             += pklpdud.numEntriesCache4;
                    cache_4_entry_index += pklpdud.numEntriesCache4;
                }
            }

            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;
        case PDUTYPE2_BITMAPCACHE_ERROR_PDU: // Bitmap Cache Error PDU (see [MS-RDPEGDI] section 2.2.2.3.1)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_BITMAPCACHE_ERROR_PDU");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;
        case PDUTYPE2_SET_KEYBOARD_IME_STATUS: // Set Keyboard IME Status PDU (section 2.2.8.2.2.1)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_SET_KEYBOARD_IME_STATUS");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;
        case PDUTYPE2_OFFSCRCACHE_ERROR_PDU: // Offscreen Bitmap Cache Error PDU (see [MS-RDPEGDI] section 2.2.2.3.2)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_OFFSCRCACHE_ERROR_PDU");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;
        case PDUTYPE2_SET_ERROR_INFO_PDU: // Set Error Info PDU (section 2.2.5.1.1)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_SET_ERROR_INFO_PDU");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;
        case PDUTYPE2_DRAWNINEGRID_ERROR_PDU: // DrawNineGrid Cache Error PDU (see [MS-RDPEGDI] section 2.2.2.3.3)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_DRAWNINEGRID_ERROR_PDU");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;
        case PDUTYPE2_DRAWGDIPLUS_ERROR_PDU: // GDI+ Error PDU (see [MS-RDPEGDI] section 2.2.2.3.4)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_DRAWGDIPLUS_ERROR_PDU");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;
        case PDUTYPE2_ARC_STATUS_PDU: // Auto-Reconnect Status PDU (section 2.2.4.1.1)
            if (this->verbose & 8){
                LOG(LOG_INFO, "PDUTYPE2_ARC_STATUS_PDU");
            }
            TODO("this quickfix prevents a tech crash, but consuming the data should be a better behaviour")
            sdata_in.payload.p = sdata_in.payload.end;
        break;

        default:
            LOG(LOG_WARNING, "unsupported PDUTYPE in process_data %d\n", sdata_in.pdutype2);
            break;
        }

        sdata_in.recv_end();
        stream.p = sdata_in.payload.p;

        if (this->verbose & (4|8)){
            LOG(LOG_INFO, "process_data done");
        }
    }

    void send_deactive() throw (Error)
    {
        if (this->verbose & 1){
            LOG(LOG_INFO, "send_deactive");
        }

        HStream stream(1024, 1024 + 256);
        ShareControl_Send(stream, PDUTYPE_DEACTIVATEALLPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, 0);

        if ((this->verbose & (128|1)) == (128|1)){
            LOG(LOG_INFO, "Sec clear payload to send:");
            hexdump_d(stream.get_data(), stream.size());
        }

        this->send_data_indication_ex(GCC::MCS_GLOBAL_CHANNEL, stream);

        if (this->verbose & 1){
            LOG(LOG_INFO, "send_deactive done");
        }
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){

            this->send_global_palette();

            RDPOpaqueRect new_cmd = cmd;
            if (this->client_info.bpp != this->mod_bpp){
                const BGRColor color24 = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
                new_cmd.color = color_encode(color24, this->client_info.bpp);
            }
            this->orders->draw(new_cmd, clip);

            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)){
                RDPOpaqueRect new_cmd24 = cmd;
                new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){
            this->orders->draw(cmd, clip);

            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)){
                this->capture->draw(cmd, clip);
            }
        }
    }

    void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){
            this->orders->draw(cmd, clip);
            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)){
                this->capture->draw(cmd, clip);
            }
        }
    }

    void draw(const RDPMultiDstBlt & cmd, const Rect & clip) {
        if (!clip.isempty() &&
            !clip.intersect(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight)).isempty()) {
            this->orders->draw(cmd, clip);
            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)) {
                this->capture->draw(cmd, clip);
            }
        }
    }

    void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) {
        if (!clip.isempty() &&
            !clip.intersect(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight)).isempty()) {
            this->send_global_palette();

            RDPMultiOpaqueRect new_cmd = cmd;
            if (this->client_info.bpp != this->mod_bpp) {
                const BGRColor color24 = color_decode_opaquerect(cmd._Color, this->mod_bpp, this->mod_palette);
                new_cmd._Color = color_encode(color24, this->client_info.bpp);
            }
            this->orders->draw(new_cmd, clip);

            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)) {
                RDPMultiOpaqueRect new_cmd24 = cmd;
                new_cmd24._Color = color_decode_opaquerect(cmd._Color, this->mod_bpp, this->mod_palette);
                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){
            this->send_global_palette();

            const BGRColor back_color24 = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
            const BGRColor fore_color24 = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);

            RDPPatBlt new_cmd = cmd;
            if (this->client_info.bpp != this->mod_bpp){
                new_cmd.back_color= color_encode(back_color24, this->client_info.bpp);
                new_cmd.fore_color= color_encode(fore_color24, this->client_info.bpp);
                // this may change the brush add send it to to remote cache
            }
            this->cache_brush(new_cmd.brush);
            this->orders->draw(new_cmd, clip);

            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)){
                RDPPatBlt new_cmd24 = cmd;
                new_cmd24.back_color = back_color24;
                new_cmd24.fore_color = fore_color24;
                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw_vnc(const Rect & rect, const uint8_t bpp, const BGRPalette & palette332, const uint8_t * raw, uint32_t need_size)
    {
        const uint16_t TILE_CX = 32;
        const uint16_t TILE_CY = 32;

        for (int y = 0; y < rect.cy ; y += TILE_CY) {
            int cy = std::min(TILE_CY, (uint16_t)(rect.cy - y));

            for (int x = 0; x < rect.cx ; x += TILE_CX) {
                int cx = std::min(TILE_CX, (uint16_t)(rect.cx - x));

                const Rect dst_tile(rect.x + x, rect.y + y, cx, cy);
                const Rect src_tile(x, y, cx, cy);

                const Bitmap tiled_bmp(raw, rect.cx, rect.cy, bpp, src_tile);
                const RDPMemBlt cmd2(0, dst_tile, 0xCC, 0, 0, 0);
                this->orders->draw(cmd2, dst_tile, tiled_bmp);
                if (  this->capture
                   && (this->capture_state == CAPTURE_STATE_STARTED)){
                    this->capture->draw(cmd2, dst_tile, tiled_bmp);
                }
            }
        }

    }

    void draw_tile(const Rect & dst_tile, const Rect & src_tile, const RDPMemBlt & cmd, const Bitmap & bitmap, const Rect & clip)
    {
        if (this->verbose & 64){
            LOG(LOG_INFO, "front::draw:draw_tile((%u, %u, %u, %u) (%u, %u, %u, %u)",
                 dst_tile.x, dst_tile.y, dst_tile.cx, dst_tile.cy,
                 src_tile.x, src_tile.y, src_tile.cx, src_tile.cy);
        }

        // No need to resize bitmap

//        if (dst_tile.x + dst_tile.cx == this->client_info.width
//        && dst_tile.y + dst_tile.cy == this->client_info.height){
//            return;
//        }


        if (src_tile == Rect(0, 0, bitmap.cx, bitmap.cy)){
            const RDPMemBlt cmd2(0, dst_tile, cmd.rop, 0, 0, 0);
            this->orders->draw(cmd2, clip, bitmap);
            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)){
                this->capture->draw(cmd2, clip, bitmap);
            }
        }
        else {
            const Bitmap tiled_bmp(bitmap, src_tile);
            const RDPMemBlt cmd2(0, dst_tile, cmd.rop, 0, 0, 0);
            this->orders->draw(cmd2, clip, tiled_bmp);
            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)){
                this->capture->draw(cmd2, clip, tiled_bmp);
            }
        }
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bitmap)
    {
        if (bitmap.cx < cmd.srcx || bitmap.cy < cmd.srcy){
            return;
        }

        this->send_global_palette();

        const uint8_t palette_id = 0;
        if (this->client_info.bpp == 8){
            if (!this->palette_memblt_sent[palette_id]) {
                RDPColCache cmd(palette_id, bitmap.original_palette);
                this->orders->draw(cmd);
                this->palette_memblt_sent[palette_id] = true;
            }
        }

        // if not we have to split it
        const uint16_t TILE_CX = 64;
        const uint16_t TILE_CY = 64;

        const uint16_t dst_x = cmd.rect.x;
        const uint16_t dst_y = cmd.rect.y;
        // clip dst as it can be larger than source bitmap
        const uint16_t dst_cx = std::min<uint16_t>(bitmap.cx - cmd.srcx, cmd.rect.cx);
        const uint16_t dst_cy = std::min<uint16_t>(bitmap.cy - cmd.srcy, cmd.rect.cy);

        // check if target bitmap can be fully stored inside one front cache entry
        // if so no need to tile it.
        uint32_t front_bitmap_size = ::nbbytes(this->client_info.bpp) * align4(dst_cx) * dst_cy;
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
        if (front_bitmap_size <= this->client_info.cache3_size
            && align4(dst_cx) < 128 && dst_cy < 128){
            // clip dst as it can be larger than source bitmap
            const Rect dst_tile(dst_x, dst_y, dst_cx, dst_cy);
            const Rect src_tile(cmd.srcx, cmd.srcy, dst_cx, dst_cy);
            this->draw_tile(dst_tile, src_tile, cmd, bitmap, clip);
        }
        else {
            for (int y = 0; y < dst_cy ; y += TILE_CY) {
                int cy = std::min(TILE_CY, (uint16_t)(dst_cy - y));

                for (int x = 0; x < dst_cx ; x += TILE_CX) {
                    int cx = std::min(TILE_CX, (uint16_t)(dst_cx - x));

                    const Rect dst_tile(dst_x + x, dst_y + y, cx, cy);
                    const Rect src_tile(cmd.srcx + x, cmd.srcy + y, cx, cy);
                    this->draw_tile(dst_tile, src_tile, cmd, bitmap, clip);
                }
            }
        }
    }

    void draw_tile3(const Rect & dst_tile, const Rect & src_tile, const RDPMem3Blt & cmd, const Bitmap & bitmap, const Rect & clip)
    {
        if (this->verbose & 64){
            LOG(LOG_INFO, "front::draw:draw_tile3((%u, %u, %u, %u) (%u, %u, %u, %u)",
                 dst_tile.x, dst_tile.y, dst_tile.cx, dst_tile.cy,
                 src_tile.x, src_tile.y, src_tile.cx, src_tile.cy);
        }

        // No need to resize bitmap

//        if (dst_tile.x + dst_tile.cx == this->client_info.width
//        && dst_tile.y + dst_tile.cy == this->client_info.height){
//            return;
//        }

        const BGRColor back_color24 = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
        const BGRColor fore_color24 = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);

        if (src_tile == Rect(0, 0, bitmap.cx, bitmap.cy)){
            RDPMem3Blt cmd2(0, dst_tile, cmd.rop, 0, 0, cmd.back_color, cmd.fore_color, cmd.brush, 0);

            if (this->client_info.bpp != this->mod_bpp){
                cmd2.back_color= color_encode(back_color24, this->client_info.bpp);
                cmd2.fore_color= color_encode(fore_color24, this->client_info.bpp);
                // this may change the brush add send it to to remote cache
            }

            this->orders->draw(cmd2, clip, bitmap);
            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)){
                cmd2.back_color= back_color24;
                cmd2.fore_color= fore_color24;

                this->capture->draw(cmd2, clip, bitmap);
            }
        }
        else {
            const Bitmap tiled_bmp(bitmap, src_tile);
            RDPMem3Blt cmd2(0, dst_tile, cmd.rop, 0, 0, cmd.back_color, cmd.fore_color, cmd.brush, 0);

            if (this->client_info.bpp != this->mod_bpp){
                cmd2.back_color= color_encode(back_color24, this->client_info.bpp);
                cmd2.fore_color= color_encode(fore_color24, this->client_info.bpp);
                // this may change the brush add send it to to remote cache
            }

            this->orders->draw(cmd2, clip, tiled_bmp);
            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)){
                cmd2.back_color= back_color24;
                cmd2.fore_color= fore_color24;

                this->capture->draw(cmd2, clip, tiled_bmp);
            }
        }
    }

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bitmap) {
        // LOG(LOG_INFO, "Mem3Blt::rop = %X", cmd.rop);
        if (bitmap.cx < cmd.srcx || bitmap.cy < cmd.srcy){
            return;
        }

        this->send_global_palette();

        const uint8_t palette_id = 0;
        if (this->client_info.bpp == 8){
            if (!this->palette_memblt_sent[palette_id]) {
                RDPColCache cmd(palette_id, bitmap.original_palette);
                this->orders->draw(cmd);
                this->palette_memblt_sent[palette_id] = true;
            }
        }

        // if not we have to split it
        const uint16_t TILE_CX = 64;
        const uint16_t TILE_CY = 64;

        const uint16_t dst_x = cmd.rect.x;
        const uint16_t dst_y = cmd.rect.y;
        // clip dst as it can be larger than source bitmap
        const uint16_t dst_cx = std::min<uint16_t>(bitmap.cx - cmd.srcx, cmd.rect.cx);
        const uint16_t dst_cy = std::min<uint16_t>(bitmap.cy - cmd.srcy, cmd.rect.cy);

        // check if target bitmap can be fully stored inside one front cache entry
        // if so no need to tile it.
        uint32_t front_bitmap_size = ::nbbytes(this->client_info.bpp) * align4(dst_cx) * dst_cy;
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
        if (front_bitmap_size <= this->client_info.cache3_size
            && align4(dst_cx) < 128 && dst_cy < 128){
            // clip dst as it can be larger than source bitmap
            const Rect dst_tile(dst_x, dst_y, dst_cx, dst_cy);
            const Rect src_tile(cmd.srcx, cmd.srcy, dst_cx, dst_cy);
            this->draw_tile3(dst_tile, src_tile, cmd, bitmap, clip);
        }
        else {
            for (int y = 0; y < dst_cy ; y += TILE_CY) {
                int cy = std::min(TILE_CY, (uint16_t)(dst_cy - y));

                for (int x = 0; x < dst_cx ; x += TILE_CX) {
                    int cx = std::min(TILE_CX, (uint16_t)(dst_cx - x));

                    const Rect dst_tile(dst_x + x, dst_y + y, cx, cy);
                    const Rect src_tile(cmd.srcx + x, cmd.srcy + y, cx, cy);
                    this->draw_tile3(dst_tile, src_tile, cmd, bitmap, clip);
                }
            }
        }
    }

    void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        const uint16_t minx = std::min(cmd.startx, cmd.endx);
        const uint16_t miny = std::min(cmd.starty, cmd.endy);
        const Rect rect(minx, miny,
                        std::max(cmd.startx, cmd.endx)-minx+1,
                        std::max(cmd.starty, cmd.endy)-miny+1);

        if (!clip.isempty() && !clip.intersect(rect).isempty()){

            RDPLineTo new_cmd = cmd;
            if (this->client_info.bpp != this->mod_bpp){
                const BGRColor back_color24 = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
                new_cmd.back_color = color_encode(back_color24, this->client_info.bpp);
                const BGRColor pen_color24 = color_decode_opaquerect(cmd.pen.color, this->mod_bpp, this->mod_palette);
                new_cmd.pen.color = color_encode(pen_color24, this->client_info.bpp);
            }

            this->orders->draw(new_cmd, clip);

            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)){
                RDPLineTo new_cmd24 = cmd;
                new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
                new_cmd24.pen.color = color_decode_opaquerect(cmd.pen.color, this->mod_bpp, this->mod_palette);
                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache)
    {
        if (!clip.isempty() && !clip.intersect(cmd.bk).isempty()){
            this->send_global_palette();

            RDPGlyphIndex new_cmd = cmd;
            if (this->client_info.bpp != this->mod_bpp){
                const BGRColor back_color24 = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
                const BGRColor fore_color24 = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
                new_cmd.back_color = color_encode(back_color24, this->client_info.bpp);
                new_cmd.fore_color = color_encode(fore_color24, this->client_info.bpp);
            }

            if (gly_cache)
            {
                bool has_delta_byte = (!new_cmd.ui_charinc && !(new_cmd.fl_accel & 0x20/*SO_CHAR_INC_EQUAL_BM_BASE*/));
                for (uint8_t i = 0; i < new_cmd.data_len;)
                {
                    if (new_cmd.data[i] <= 0xFD)
                    {
//                      LOG(LOG_INFO, "Index in the fragment cache=%u", new_cmd.data[i]);
                        FontChar * fc = gly_cache->char_items[new_cmd.cache_id][new_cmd.data[i]].font_item;
                        REDASSERT(fc);
                        int g_idx = this->glyph_cache.find_glyph(fc, new_cmd.cache_id);
                        REDASSERT(g_idx >= 0);
                        new_cmd.data[i] = static_cast<uint8_t>(g_idx);
                        if (has_delta_byte)
                        {
                            if (new_cmd.data[++i] == 0x80)
                            {
                                i += 3;
                            }
                            else
                            {
                                i++;
                            }
                        }
                    }
                    else if (new_cmd.data[i] == 0xFE)
                    {
                        LOG(LOG_INFO, "Front::draw(RDPGlyphIndex, ...): Unsupported data");
                        throw Error(ERR_RDP_UNSUPPORTED);
                    }
                    else if (new_cmd.data[i] == 0xFF)
                    {
                        i += 3;
                        REDASSERT(i == new_cmd.data_len);
                    }
                }
            }

            // this may change the brush and send it to to remote cache
            this->cache_brush(new_cmd.brush);

            this->orders->draw(new_cmd, clip, gly_cache);

            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)){
                RDPGlyphIndex new_cmd24 = /*cmd*/new_cmd;
                new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
                new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
                this->capture->draw(new_cmd24, clip, gly_cache);
            }
        }
    }

    void draw(const RDPGlyphCache & cmd)
    {
        FontChar font_item(cmd.glyphData_x, cmd.glyphData_y,
            cmd.glyphData_cx, cmd.glyphData_cy, -1);
        memcpy(font_item.data, cmd.glyphData_aj, font_item.datasize());

        int cacheidx = 0;

        if (this->glyph_cache.add_glyph(&font_item, cmd.cacheId, cacheidx) ==
            GlyphCache::GLYPH_ADDED_TO_CACHE)
        {
            RDPGlyphCache cmd2(cmd.cacheId, 1, cacheidx,
                            cmd.glyphData_x,
                            cmd.glyphData_y,
                            cmd.glyphData_cx,
                            cmd.glyphData_cy,
                            cmd.glyphData_aj);

            this->orders->draw(cmd2);

            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)) {
                this->capture->draw(cmd2);
            }
        }
    }

    void draw(const RDPPolygonSC & cmd, const Rect & clip) {
        int16_t minx, miny, maxx, maxy, previousx, previousy;

        minx = maxx = previousx = cmd.xStart;
        miny = maxy = previousy = cmd.yStart;

        for (uint8_t i = 0; i < cmd.NumDeltaEntries; i++) {
            previousx += cmd.deltaPoints[i].xDelta;
            previousy += cmd.deltaPoints[i].yDelta;

            minx = std::min(minx, previousx);
            miny = std::min(miny, previousy);

            maxx = std::max(maxx, previousx);
            maxy = std::max(maxy, previousy);
        }
        const Rect rect(minx, miny, maxx-minx+1, maxy-miny+1);

        if (!clip.isempty() && !clip.intersect(rect).isempty()) {
            RDPPolygonSC new_cmd = cmd;
            if (this->client_info.bpp != this->mod_bpp) {
                const BGRColor pen_color24 = color_decode_opaquerect(cmd.BrushColor, this->mod_bpp, this->mod_palette);
                new_cmd.BrushColor = color_encode(pen_color24, this->client_info.bpp);
            }

            this->orders->draw(new_cmd, clip);

            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)) {
                RDPPolygonSC new_cmd24 = cmd;
                new_cmd24.BrushColor = color_decode_opaquerect(cmd.BrushColor, this->mod_bpp, this->mod_palette);
                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPPolygonCB & cmd, const Rect & clip) {
        int16_t minx, miny, maxx, maxy, previousx, previousy;

        minx = maxx = previousx = cmd.xStart;
        miny = maxy = previousy = cmd.yStart;

        for (uint8_t i = 0; i < cmd.NumDeltaEntries; i++) {
            previousx += cmd.deltaPoints[i].xDelta;
            previousy += cmd.deltaPoints[i].yDelta;

            minx = std::min(minx, previousx);
            miny = std::min(miny, previousy);

            maxx = std::max(maxx, previousx);
            maxy = std::max(maxy, previousy);
        }
        const Rect rect(minx, miny, maxx-minx+1, maxy-miny+1);

        if (!clip.isempty() && !clip.intersect(rect).isempty()) {
            RDPPolygonCB new_cmd = cmd;
            if (this->client_info.bpp != this->mod_bpp) {
                const BGRColor fore_pen_color24 = color_decode_opaquerect(cmd.foreColor, this->mod_bpp, this->mod_palette);
                new_cmd.foreColor = color_encode(fore_pen_color24, this->client_info.bpp);
                const BGRColor back_pen_color24 = color_decode_opaquerect(cmd.backColor, this->mod_bpp, this->mod_palette);
                new_cmd.backColor = color_encode(back_pen_color24, this->client_info.bpp);
            }

            this->orders->draw(new_cmd, clip);

            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)) {
                RDPPolygonCB new_cmd24 = cmd;
                new_cmd24.foreColor = color_decode_opaquerect(cmd.foreColor, this->mod_bpp, this->mod_palette);
                new_cmd24.backColor = color_decode_opaquerect(cmd.backColor, this->mod_bpp, this->mod_palette);
                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) {
        int16_t minx, miny, maxx, maxy, previousx, previousy;

        minx = maxx = previousx = cmd.xStart;
        miny = maxy = previousy = cmd.yStart;

        for (uint8_t i = 0; i < cmd.NumDeltaEntries; i++) {
            previousx += cmd.deltaEncodedPoints[i].xDelta;
            previousy += cmd.deltaEncodedPoints[i].yDelta;

            minx = std::min(minx, previousx);
            miny = std::min(miny, previousy);

            maxx = std::max(maxx, previousx);
            maxy = std::max(maxy, previousy);
        }
        const Rect rect(minx, miny, maxx-minx+1, maxy-miny+1);

        if (!clip.isempty() && !clip.intersect(rect).isempty()) {
            RDPPolyline new_cmd = cmd;
            if (this->client_info.bpp != this->mod_bpp) {
                const BGRColor pen_color24 = color_decode_opaquerect(cmd.PenColor, this->mod_bpp, this->mod_palette);
                new_cmd.PenColor = color_encode(pen_color24, this->client_info.bpp);
            }

            this->orders->draw(new_cmd, clip);

            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)) {
                RDPPolyline new_cmd24 = cmd;
                new_cmd24.PenColor = color_decode_opaquerect(cmd.PenColor, this->mod_bpp, this->mod_palette);
                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPEllipseSC & cmd, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.el.get_rect()).isempty()){

            this->send_global_palette();

            RDPEllipseSC new_cmd = cmd;
            if (this->client_info.bpp != this->mod_bpp){
                const BGRColor color24 = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
                new_cmd.color = color_encode(color24, this->client_info.bpp);
            }
            this->orders->draw(new_cmd, clip);

            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)){
                RDPEllipseSC new_cmd24 = cmd;
                new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPEllipseCB & cmd, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.el.get_rect()).isempty()){

            this->send_global_palette();

            RDPEllipseCB new_cmd = cmd;
            if (this->client_info.bpp != this->mod_bpp){
                const BGRColor back_color24 = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
                new_cmd.back_color = color_encode(back_color24, this->client_info.bpp);

                const BGRColor fore_color24 = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
                new_cmd.fore_color = color_encode(fore_color24, this->client_info.bpp);

            }
            this->orders->draw(new_cmd, clip);

            if (  this->capture
               && (this->capture_state == CAPTURE_STATE_STARTED)){
                RDPEllipseCB new_cmd24 = cmd;
                new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
                new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
                this->capture->draw(new_cmd24, clip);
            }
        }
    }


    virtual void flush() {
        this->orders->flush();
        if (  this->capture
           && (this->capture_state == CAPTURE_STATE_STARTED)) {
            this->capture->flush();
        }
    }

    void cache_brush(RDPBrush & brush)
    {
        if ((brush.style == 3) && (this->client_info.brush_cache_code == 1)) {
            uint8_t pattern[8];
            pattern[0] = brush.hatch;
            memcpy(pattern+1, brush.extra, 7);
            int cache_idx = 0;
            if (BRUSH_TO_SEND == this->brush_cache.add_brush(pattern, cache_idx)){
                RDPBrushCache cmd(cache_idx, 1, 8, 8, 0x81,
                    sizeof(this->brush_cache.brush_items[cache_idx].pattern),
                    this->brush_cache.brush_items[cache_idx].pattern);
                this->orders->draw(cmd);
            }
            brush.hatch = cache_idx;
            brush.style = 0x81;
        }
    }

    virtual void draw(const RDPColCache & cmd)
    {
        this->orders->draw(cmd);
    }

    void set_mod_palette(const BGRPalette & palette)
    {
        this->mod_palette_setted = true;
        for (unsigned i = 0; i < 256 ; i++){
            this->mod_palette[i] = palette[i];
            this->memblt_mod_palette[i] = RGBtoBGR(palette[i]);
        }
        this->palette_sent = false;
    }

    virtual void intersect_order_caps(int idx, uint8_t * proxy_order_caps) {
        proxy_order_caps[idx] &= this->client_order_caps.orderSupport[idx];
    }

    virtual void draw(const RDPBitmapData & bitmap_data, const uint8_t * data
                     , size_t size, const Bitmap & bmp) {
//        LOG(LOG_INFO, "Front::draw(BitmapUpdate)");
        this->orders->draw(bitmap_data, data, size, bmp);
        if (  this->capture
           && (this->capture_state == CAPTURE_STATE_STARTED)) {
            this->capture->draw(bitmap_data, data, size, bmp);
        }
    }
};

#endif
