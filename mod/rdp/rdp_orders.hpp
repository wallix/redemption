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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   rdp module process orders
*/

#ifndef _REDEMPTION_MOD_RDP_RDP_ORDERS_HPP_
#define _REDEMPTION_MOD_RDP_RDP_ORDERS_HPP__

#include <string.h>

#include "infiletransport.hpp"
#include "log.hpp"
#include "mod_api.hpp"
#include "outfiletransport.hpp"
#include "stream.hpp"

#include "RDP/protocol.hpp"

#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"

#include "RDP/caches/bmpcache.hpp"
#include "RDP/caches/bmpcachepersister.hpp"

/* orders */
struct rdp_orders {
    // State
    RDPOrderCommon     common;
    RDPMemBlt          memblt;
    RDPMem3Blt         mem3blt;
    RDPOpaqueRect      opaquerect;
    RDPScrBlt          scrblt;
    RDPDestBlt         destblt;
    RDPMultiDstBlt     multidstblt;
    RDPMultiOpaqueRect multiopaquerect;
    RDPPatBlt          patblt;
    RDPLineTo          lineto;
    RDPGlyphIndex      glyph_index;
    RDPPolyline        polyline;
    RDPEllipseSC       ellipseSC;

    BGRPalette cache_colormap[6];
    BGRPalette global_palette;

    BmpCache * bmp_cache;

    GlyphCache gly_cache;

    uint32_t verbose;

    size_t recv_bmp_cache_count;
    size_t recv_order_count;

    redemption::string target_device;
    bool               enable_persistent_disk_bitmap_cache;

    rdp_orders(const char * target_device, bool enable_persistent_disk_bitmap_cache, uint32_t verbose)
            : common(RDP::PATBLT, Rect(0, 0, 1, 1))
            , memblt(0, Rect(), 0, 0, 0, 0)
            , mem3blt(0, Rect(), 0, 0, 0, 0, 0, RDPBrush(), 0)
            , opaquerect(Rect(), 0)
            , scrblt(Rect(), 0, 0, 0)
            , destblt(Rect(), 0)
            , patblt(Rect(), 0, 0, 0, RDPBrush())
            , lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0))
            , glyph_index( 0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0
                         , (uint8_t *)"")
            , bmp_cache(NULL)
            , verbose(verbose)
            , recv_bmp_cache_count(0)
            , recv_order_count(0)
            , target_device(target_device)
            , enable_persistent_disk_bitmap_cache(enable_persistent_disk_bitmap_cache) {
        memset(this->cache_colormap, 0, sizeof(this->cache_colormap));
        memset(this->global_palette, 0, sizeof(this->global_palette));
    }

    void reset()
    {
        this->common      = RDPOrderCommon(RDP::PATBLT, Rect(0, 0, 1, 1));
        this->memblt      = RDPMemBlt(0, Rect(), 0, 0, 0, 0);
        this->mem3blt     = RDPMem3Blt(0, Rect(), 0, 0, 0, 0, 0, RDPBrush(), 0);
        this->opaquerect  = RDPOpaqueRect(Rect(), 0);
        this->scrblt      = RDPScrBlt(Rect(), 0, 0, 0);
        this->destblt     = RDPDestBlt(Rect(), 0);
        this->patblt      = RDPPatBlt(Rect(), 0, 0, 0, RDPBrush());
        this->lineto      = RDPLineTo(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0));
        this->glyph_index = RDPGlyphIndex( 0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1)
                                         , RDPBrush(), 0, 0, 0, (uint8_t *)"");

        if (this->bmp_cache) {
            this->bmp_cache->reset();
        }

        memset(this->cache_colormap, 0, sizeof(this->cache_colormap));
        memset(this->global_palette, 0, sizeof(this->global_palette));
    }

    ~rdp_orders() {
        if (this->bmp_cache) {
            this->save_persistent_disk_bitmap_cache();
            delete this->bmp_cache;
        }
    }

    void save_persistent_disk_bitmap_cache() const {
        if (!this->enable_persistent_disk_bitmap_cache)
            return;

        const char * persistent_path = PERSISTENT_PATH "/mod_rdp";

        // Ensures that the directory exists.
        if (::recursive_create_directory( persistent_path, S_IRWXU | S_IRWXG, 0) != 0) {
            LOG( LOG_ERR
               , "rdp_orders::save_persistent_disk_bitmap_cache: failed to create directory \"%s\"."
               , persistent_path);
            throw Error(ERR_BITMAP_CACHE_PERSISTENT, 0);
        }

        // Generates the name of file.
        char filename[2048];
        ::snprintf(filename, sizeof(filename) - 1, "%s/PDBC-%s-%d",
            persistent_path, this->target_device.c_str(), this->bmp_cache->bpp);
        filename[sizeof(filename) - 1] = '\0';

        char filename_temporary[2048];
        ::snprintf(filename_temporary, sizeof(filename_temporary) - 1, "%s/PDBC-%s-%d-XXXXXX.tmp",
            persistent_path, this->target_device.c_str(), this->bmp_cache->bpp);
        filename_temporary[sizeof(filename_temporary) - 1] = '\0';

        int fd = ::mkostemps(filename_temporary, 4, O_CREAT | O_WRONLY);
        if (fd == -1) {
            LOG( LOG_ERR
               , "rdp_orders::save_persistent_disk_bitmap_cache: "
                 "failed to open (temporary) file for writing. filename=\"%s\""
               , filename_temporary);
            throw Error(ERR_PDBC_SAVE);
        }

        try
        {
            OutFileTransport oft(fd);

            BmpCachePersister::save_all_to_disk(*this->bmp_cache, oft, this->verbose);
        }
        catch (...)
        {
            ::close(fd);
            ::unlink(filename_temporary);
            throw;
        }

        ::close(fd);

        if (::rename(filename_temporary, filename) == -1) {
            LOG( LOG_ERR
               , "rdp_orders::save_persistent_disk_bitmap_cache: failed to rename the (temporary) file. "
                 "old_filename=\"%s\" new_filename=\"%s\""
               , filename_temporary, filename);
            throw Error(ERR_PDBC_SAVE);
            ::unlink(filename_temporary);
        }
    }

    void create_cache_bitmap(const uint8_t bpp,
        uint16_t small_entries, uint16_t small_size, bool small_persistent,
        uint16_t medium_entries, uint16_t medium_size, bool medium_persistent,
        uint16_t big_entries, uint16_t big_size, bool big_persistent)
    {
        if (this->bmp_cache) {
            this->save_persistent_disk_bitmap_cache();
            delete this->bmp_cache;
            this->bmp_cache = NULL;
        }

        this->bmp_cache = new BmpCache(bpp, 3, false, small_entries, small_size, small_persistent,
            medium_entries, medium_size, medium_persistent, big_entries, big_size, big_persistent,
            0, 0, false, 0, 0, false, this->verbose);

        if (this->enable_persistent_disk_bitmap_cache) {
            // Generates the name of file.
            char filename[2048];
            ::snprintf(filename, sizeof(filename) - 1, "%s/PDBC-%s-%d",
                PERSISTENT_PATH "/mod_rdp", this->target_device.c_str(), this->bmp_cache->bpp);
            filename[sizeof(filename) - 1] = '\0';

            int fd = ::open(filename, O_RDONLY);
            if (fd == -1) {
                return;
            }

            InFileTransport ift(fd);

            try {
                if (this->verbose & 1) {
                    LOG(LOG_INFO, "rdp_orders::create_cache_bitmap: filename=\"%s\"", filename);
                }
                BmpCachePersister::load_all_from_disk(*this->bmp_cache, ift, filename, this->verbose);
            }
            catch (...) {
                ::close(fd);
                throw;
            }

            ::close(fd);
        }
    }

public:
    void process_bmpcache( uint8_t bpp, Stream & stream, const uint8_t control
                         , const RDPSecondaryOrderHeader & header)
    {
        if (this->verbose & 64) {
            LOG(LOG_INFO, "rdp_orders_process_bmpcache bpp=%u", bpp);
        }
        RDPBmpCache bmp(this->verbose);
        bmp.receive(bpp, stream, control, header, this->global_palette);

        this->recv_bmp_cache_count++;

        REDASSERT(bmp.bmp);
        this->bmp_cache->put(bmp.id, bmp.idx, bmp.bmp, bmp.key1, bmp.key2);
        if (this->verbose & 64) {
            LOG( LOG_ERR
               , "rdp_orders_process_bmpcache bitmap id=%u idx=%u cx=%u cy=%u bmp_size=%u original_bpp=%u bpp=%u"
               , bmp.id, bmp.idx, bmp.bmp->cx, bmp.bmp->cy, bmp.bmp->bmp_size, bmp.bmp->original_bpp, bpp);
        }
    }

    void server_add_char( int font, int character
                        , int offset, int baseline
                        , int width, int height, const uint8_t * data
                        , mod_api * mod) {
        struct FontChar fi(offset, baseline, width, height, 0);
        memcpy(fi.data, data, fi.datasize());

        RDPGlyphCache cmd(font, 1, character, fi.offset, fi.baseline, fi.width, fi.height, fi.data);
        this->gly_cache.set_glyph(cmd);
        mod->draw(cmd);
    }

    void process_fontcache(Stream & stream, int flags, mod_api * mod) {
        if (this->verbose & 64) {
            LOG(LOG_INFO, "rdp_orders_process_fontcache");
        }
        int font    = stream.in_uint8();
        int nglyphs = stream.in_uint8();
        for (int i = 0; i < nglyphs; i++) {
            int character = stream.in_uint16_le();
            int offset    = stream.in_uint16_le();
            int baseline  = stream.in_uint16_le();
            int width     = stream.in_uint16_le();
            int height    = stream.in_uint16_le();

            int             datasize = (height * nbbytes(width) + 3) & ~3;
            const uint8_t * data     = stream.in_uint8p(datasize);

            this->server_add_char(font, character, offset, baseline, width, height, data, mod);
        }
        if (this->verbose & 64) {
            LOG(LOG_INFO, "rdp_orders_process_fontcache done");
        }
    }

    void process_colormap( Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header
                         , mod_api * mod) {
        if (this->verbose & 64) {
            LOG(LOG_INFO, "process_colormap");
        }
        RDPColCache colormap;
        colormap.receive(stream, control, header);
        memcpy(this->cache_colormap[colormap.cacheIndex], &colormap.palette, sizeof(BGRPalette));
        RDPColCache cmd(colormap.cacheIndex, colormap.palette);
        mod->draw(cmd);

        if (this->verbose & 64) {
            LOG(LOG_INFO, "process_colormap done");
        }
    }

    /*****************************************************************************/
    int process_orders(uint8_t bpp, Stream & stream, bool fast_path, mod_api * mod) {
        if (this->verbose & 64) {
            LOG(LOG_INFO, "process_orders bpp=%u", bpp);
        }

        using namespace RDP;

        OrdersUpdate_Recv orders_update(stream, fast_path);

        this->recv_order_count += orders_update.number_orders;

        int processed = 0;
        while (processed < orders_update.number_orders) {
            DrawingOrder_RecvFactory drawing_order(stream);

            if (!drawing_order.control_flags & STANDARD) {
                /* error, this should always be set */
                LOG(LOG_ERR, "Non standard order detected : protocol error");
                REDASSERT(false);
                break;
            }
            if (drawing_order.control_flags & SECONDARY) {
                RDPSecondaryOrderHeader header(stream);
//                LOG(LOG_INFO, "secondary order=%d", header.type);
                uint8_t * next_order = stream.p + header.order_data_length();
                switch (header.type) {
                case TS_CACHE_BITMAP_COMPRESSED:
                case TS_CACHE_BITMAP_UNCOMPRESSED:
                case TS_CACHE_BITMAP_COMPRESSED_REV2:
                case TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
                    this->process_bmpcache(bpp, stream, drawing_order.control_flags, header);
                    break;
                case TS_CACHE_COLOR_TABLE:
                    this->process_colormap(stream, drawing_order.control_flags, header, mod);
                    break;
                case TS_CACHE_GLYPH:
                    this->process_fontcache(stream, header.flags, mod);
                    break;
                case TS_CACHE_BITMAP_COMPRESSED_REV3:
                    LOG( LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV3 (%d)"
                       , header.type);
                    break;
                default:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER (%d)", header.type);
                    /* error, unknown order */
                    break;
                }
                stream.p = next_order;
            }
            else {
                RDPPrimaryOrderHeader header = this->common.receive(stream, drawing_order.control_flags);
                const Rect & cmd_clip = ( (drawing_order.control_flags & BOUNDS)
                                        ? this->common.clip
                                        : Rect(0, 0, mod->front_width, mod->front_height)
                                        );
                // LOG(LOG_INFO, "/* order=%d ordername=%s */", this->common.order, ordernames[this->common.order]);
                switch (this->common.order) {
                case GLYPHINDEX:
                    this->glyph_index.receive(stream, header);
                    mod->draw(this->glyph_index, cmd_clip, &this->gly_cache);
                    break;
                case DESTBLT:
                    this->destblt.receive(stream, header);
                    mod->draw(this->destblt, cmd_clip);
                    break;
                case MULTIDSTBLT:
                    this->multidstblt.receive(stream, header);
                    mod->draw(this->multidstblt, cmd_clip);
                    //this->multidstblt.log(LOG_INFO, cmd_clip);
                    break;
                case MULTIOPAQUERECT:
                    this->multiopaquerect.receive(stream, header);
                    mod->draw(this->multiopaquerect, cmd_clip);
                    //this->multiopaquerect.log(LOG_INFO, cmd_clip);
                    break;
                case PATBLT:
                    this->patblt.receive(stream, header);
                    mod->draw(this->patblt, cmd_clip);
                    break;
                case SCREENBLT:
                    this->scrblt.receive(stream, header);
                    mod->draw(this->scrblt, cmd_clip);
                    break;
                case LINE:
                    this->lineto.receive(stream, header);
                    mod->draw(this->lineto, cmd_clip);
                    break;
                case RECT:
                    this->opaquerect.receive(stream, header);
                    mod->draw(this->opaquerect, cmd_clip);
                    break;
                case MEMBLT:
                    this->memblt.receive(stream, header);
                    {
                        if ((this->memblt.cache_id >> 8) >= 6) {
                            LOG( LOG_INFO, "colormap out of range in memblt:%x"
                               , (this->memblt.cache_id >> 8));
                            this->memblt.log(LOG_INFO, cmd_clip);
                            assert(false);
                        }
                        const Bitmap * bitmap =
                            this->bmp_cache->get(this->memblt.cache_id & 0x3, this->memblt.cache_idx);
                        TODO("CGR: check if bitmap has the right palette...");
                        TODO("CGR: 8 bits palettes should probabily be transmitted to front, not stored in bitmaps");
                        if (bitmap) {
                            mod->draw(this->memblt, cmd_clip, *bitmap);
                        }
                    }
                    break;
                case MEM3BLT:
                    this->mem3blt.receive(stream, header);
                    {
                        if ((this->mem3blt.cache_id >> 8) >= 6){
                            LOG( LOG_INFO, "colormap out of range in mem3blt: %x"
                               , (this->mem3blt.cache_id >> 8));
                            this->mem3blt.log(LOG_INFO, cmd_clip);
                            assert(false);
                        }
                        const Bitmap * bitmap =
                            this->bmp_cache->get(this->mem3blt.cache_id & 0x3, this->mem3blt.cache_idx);
                        TODO("CGR: check if bitmap has the right palette...");
                        TODO("CGR: 8 bits palettes should probabily be transmitted to front, not stored in bitmaps");
                        if (bitmap) {
                            mod->draw(this->mem3blt, cmd_clip, *bitmap);
                        }
                    }
                    break;
                case POLYLINE:
                    this->polyline.receive(stream, header);
                    mod->draw(this->polyline, cmd_clip);
                    break;
                case ELLIPSESC:
                    this->ellipseSC.receive(stream, header);
                    mod->draw(this->ellipseSC, cmd_clip);
                    break;
                default:
                    /* error unknown order */
                    LOG(LOG_ERR, "unsupported PRIMARY ORDER (%d)", this->common.order);
                    break;
                }
            }
            processed++;
        }
        if (this->verbose & 64){
            LOG(LOG_INFO, "process_orders done");
        }
        return 0;
    }   // int process_orders(uint8_t bpp, Stream & stream, bool fast_path, mod_api * mod)
};

#endif
