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
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CORE_RDP_CACHES_BMPCACHEPERSISTER_HPP_
#define _REDEMPTION_CORE_RDP_CACHES_BMPCACHEPERSISTER_HPP_

#include "bmpcache.hpp"
#include "transport.hpp"

class BmpCachePersister
{
private:
    static const uint8_t CURRENT_VERSION = 1;

    struct map_value {
        const Bitmap * bmp;

        map_value() : bmp(NULL) {
        }

        ~map_value() {
            if (this->bmp) {
                delete this->bmp;
            }
        }
    };

    typedef std::map<std::string, map_value> container_type;

    container_type bmp_map[BmpCache::MAXIMUM_NUMBER_OF_CACHES];

    BmpCache & bmp_cache;

    uint32_t verbose;

public:
    BmpCachePersister(BmpCache & bmp_cache, Transport & t, const char * filename, uint32_t verbose = 0)
    : bmp_cache(bmp_cache)
    , verbose(verbose) {
        BStream stream(16);

        t.recv(&stream.end, 5);  /* magic(4) + version(1) */

        const uint8_t * magic   = stream.in_uint8p(4);  /* magic(4) */
              uint8_t   version = stream.in_uint8();

        //LOG( LOG_INFO, "BmpCachePersister: magic=\"%c%c%c%c\""
        //   , magic[0], magic[1], magic[2], magic[3]);
        //LOG( LOG_INFO, "BmpCachePersister: version=%u", version);

        if (::memcmp(magic, "PDBC", 4)) {
            LOG( LOG_ERR
               , "BmpCachePersister::BmpCachePersister: File is not a persistent bitmap cache file. filename=\"%s\""
               , filename);
            throw Error(ERR_PDBC_LOAD);
        }

        if (version != CURRENT_VERSION) {
            LOG( LOG_ERR
               , "BmpCachePersister::BmpCachePersister: Unsupported persistent bitmap cache file version(%u). filename=\"%s\""
               , version, filename);
            throw Error(ERR_PDBC_LOAD);
        }

        for (uint8_t cache_id = 0; cache_id < this->bmp_cache.number_of_cache; cache_id++) {
            this->load_from_disk(t, filename, cache_id);
        }
    }

    void process_key_list( uint8_t cache_id, RDP::BitmapCachePersistentListEntry * entries
                         , uint8_t number_of_entries, uint16_t first_entry_index) {
              uint16_t   max_number_of_entries = this->bmp_cache.cache_entries[cache_id];
              uint16_t   cache_index           = first_entry_index;
        const union Sig {
            uint8_t  sig_8[8];
            uint32_t sig_32[2];
        }              * sig                   = reinterpret_cast<const union Sig *>(entries);
        for (uint8_t entry_index = 0;
             (entry_index < number_of_entries) && (cache_index < max_number_of_entries);
             entry_index++, cache_index++, sig++) {
            REDASSERT(!this->bmp_cache.cache[cache_id][cache_index]);

            char key[20];

            snprintf( key, sizeof(key), "%02X%02X%02X%02X%02X%02X%02X%02X"
                    , sig->sig_8[0], sig->sig_8[1], sig->sig_8[2], sig->sig_8[3]
                    , sig->sig_8[4], sig->sig_8[5], sig->sig_8[6], sig->sig_8[7]);

            container_type::iterator it = this->bmp_map[cache_id].find(key);
            if (it != this->bmp_map[cache_id].end()) {
                if (this->verbose & 1) {
                    LOG(LOG_INFO, "BmpCachePersister: bitmap found. key=\"%s\"", key);
                }

                if (this->bmp_cache.cache_entries[cache_id] > cache_index) {
                    this->bmp_cache.put(cache_id, cache_index, it->second.bmp, sig->sig_32[0], sig->sig_32[1]);
                }

                it->second.bmp = NULL;
                this->bmp_map[cache_id].erase(it);
            }
            else if (this->verbose & 0x100000) {
                LOG(LOG_WARNING, "BmpCachePersister: bitmap not found!!! key=\"%s\"", key);
            }
        }
    }

private:
    void load_from_disk(Transport & t, const char * filename, uint8_t cache_id) {
        BStream stream(65536);
        t.recv(&stream.end, 2);

        uint16_t bitmap_count = stream.in_uint16_le();
        LOG(LOG_INFO, "BmpCachePersister::load_from_disk: bitmap_count=%u", bitmap_count);

        for (uint16_t i = 0; i < bitmap_count; i++) {
            t.recv(&stream.end, 13); // sig(8) + original_bpp(1) + cx(2) + cy(2);

            uint8_t sig[8];

            stream.in_copy_bytes(sig, 8); // sig(8);

            uint8_t  original_bpp = stream.in_uint8();
            REDASSERT((original_bpp == 8) || (original_bpp == 15) || (original_bpp == 16) ||
                (original_bpp == 24) || (original_bpp == 32));
            uint16_t cx           = stream.in_uint16_le();
            uint16_t cy           = stream.in_uint16_le();

            BGRPalette original_palette;
            if (original_bpp == 8) {
                t.recv(&stream.end, sizeof(original_palette));

                stream.in_copy_bytes(reinterpret_cast<uint8_t *>(original_palette), sizeof(original_palette));
            }

            uint16_t bmp_size;
            t.recv(&stream.end, sizeof(bmp_size));
            bmp_size = stream.in_uint16_le();

            stream.reset();

            t.recv(&stream.end, bmp_size);

            if (bmp_cache.cache_persistent[cache_id]) {
                char key[20];

                snprintf( key, sizeof(key), "%02X%02X%02X%02X%02X%02X%02X%02X"
                        , sig[0], sig[1], sig[2], sig[3], sig[4], sig[5], sig[6], sig[7]);

                if (verbose & 1) {
                    LOG( LOG_INFO, "BmpCachePersister::load_from_disk: sig=\"%s\" original_bpp=%u cx=%u cy=%u bmp_size=%u"
                       , key, original_bpp, cx, cy, bmp_size);
                }

                REDASSERT(this->bmp_map[cache_id][key].bmp == NULL);

                this->bmp_map[cache_id][key].bmp = new Bitmap( this->bmp_cache.bpp, original_bpp
                                                             , &original_palette, cx, cy, stream.get_data()
                                                             , bmp_size);
            }

            stream.reset();
        }
    }

public:
    static void load_all_from_disk( BmpCache & bmp_cache, Transport & t, const char * filename
                                  , uint32_t verbose = 0) {
        BStream stream(16);

        t.recv(&stream.end, 5);  /* magic(4) + version(1) */

        const uint8_t * magic   = stream.in_uint8p(4);  /* magic(4) */
              uint8_t   version = stream.in_uint8();

        //LOG( LOG_INFO, "BmpCachePersister: magic=\"%c%c%c%c\""
        //   , magic[0], magic[1], magic[2], magic[3]);
        //LOG( LOG_INFO, "BmpCachePersister: version=%u", version);

        if (::memcmp(magic, "PDBC", 4)) {
            LOG( LOG_ERR
               , "BmpCachePersister::load_all_from_disk: File is not a persistent bitmap cache file. filename=\"%s\""
               , filename);
            throw Error(ERR_PDBC_LOAD);
        }

        if (version != CURRENT_VERSION) {
            LOG( LOG_ERR
               , "BmpCachePersister::load_all_from_disk: Unsupported persistent bitmap cache file version(%u). filename=\"%s\""
               , version, filename);
            throw Error(ERR_PDBC_LOAD);
        }

        for (uint8_t cache_id = 0; cache_id < bmp_cache.number_of_cache; cache_id++) {
            load_cache_from_disk(bmp_cache, t, filename, cache_id, verbose);
        }
    }

    static void load_cache_from_disk( BmpCache & bmp_cache, Transport & t, const char * filename
                                    , uint8_t cache_id, uint32_t verbose) {
        BStream stream(65536);
        t.recv(&stream.end, 2);

        uint16_t bitmap_count = stream.in_uint16_le();
        LOG(LOG_INFO, "BmpCachePersister::load_cache_from_disk: bitmap_count=%u", bitmap_count);

        for (uint16_t i = 0; i < bitmap_count; i++) {
            t.recv(&stream.end, 13); // sig(8) + original_bpp(1) + cx(2) + cy(2);

            union {
                uint8_t  sig_8[8];
                uint32_t sig_32[2];
            } sig;

            stream.in_copy_bytes(sig.sig_8, 8); // sig(8);

            uint8_t  original_bpp = stream.in_uint8();
            REDASSERT((original_bpp == 8) || (original_bpp == 15) || (original_bpp == 16) ||
                (original_bpp == 24) || (original_bpp == 32));
            uint16_t cx           = stream.in_uint16_le();
            uint16_t cy           = stream.in_uint16_le();

            BGRPalette original_palette;
            if (original_bpp == 8) {
                t.recv(&stream.end, sizeof(original_palette));

                stream.in_copy_bytes(reinterpret_cast<uint8_t *>(original_palette), sizeof(original_palette));
            }

            uint16_t bmp_size;
            t.recv(&stream.end, sizeof(bmp_size));
            bmp_size = stream.in_uint16_le();

            stream.reset();

            t.recv(&stream.end, bmp_size);

            if ((bmp_cache.cache_persistent[cache_id]) &&
                (i < bmp_cache.cache_entries[cache_id])) {
                if (verbose & 1) {
                    char key[20];

                    snprintf( key, sizeof(key), "%02X%02X%02X%02X%02X%02X%02X%02X"
                            , sig.sig_8[0], sig.sig_8[1], sig.sig_8[2], sig.sig_8[3]
                            , sig.sig_8[4], sig.sig_8[5], sig.sig_8[6], sig.sig_8[7]);

                    LOG( LOG_INFO, "BmpCachePersister::load_cache_from_disk: sig=\"%s\" original_bpp=%u cx=%u cy=%u bmp_size=%u"
                       , key, original_bpp, cx, cy, bmp_size);
                }

                Bitmap * bmp = new Bitmap( bmp_cache.bpp, original_bpp
                                         , &original_palette, cx, cy, stream.get_data(), stream.size());

                bmp_cache.put(cache_id, i, bmp, sig.sig_32[0], sig.sig_32[1]);
            }

            stream.reset();
        }
    }

    static void save_all_to_disk(const BmpCache & bmp_cache, Transport & t, uint32_t verbose = 0) {
        if (verbose & 1) {
            bmp_cache.log();
        }

        BStream stream(128);

        stream.out_copy_bytes("PDBC", 4);  // Magic(4)
        stream.out_uint8(CURRENT_VERSION);
        stream.mark_end();

        t.send(stream);

        for (uint8_t cache_id = 0; cache_id < bmp_cache.number_of_cache; cache_id++) {
            save_to_disk(bmp_cache, cache_id, t, verbose);
        }
    }

private:
    static void save_to_disk(const BmpCache & bmp_cache, uint8_t cache_id, Transport & t, uint32_t verbose) {
        uint16_t bitmap_count = 0;
        if (bmp_cache.cache_persistent[cache_id]) {
            for (uint16_t cache_index = 0; cache_index < bmp_cache.cache_entries[cache_id]; cache_index++) {
                if (bmp_cache.cache[cache_id][cache_index]) {
                    bitmap_count++;
                }
            }
        }

        BStream stream(65535);
        stream.out_uint16_le(bitmap_count);
        stream.mark_end();
        t.send(stream);
        if (!bitmap_count) {
            return;
        }

        for (uint16_t cache_index = 0; cache_index < bmp_cache.cache_entries[cache_id]; cache_index++) {
            if (bmp_cache.cache[cache_id][cache_index]) {
                stream.reset();

                const Bitmap   * bmp      = bmp_cache.cache[cache_id][cache_index];
                const uint8_t  * sig      = bmp_cache.sig[cache_id][cache_index].sig_8;
                const uint16_t   bmp_size = bmp->bmp_size;
                const uint8_t  * bmp_data = bmp->data_bitmap.get();

                char key[20];

                snprintf( key, sizeof(key), "%02X%02X%02X%02X%02X%02X%02X%02X"
                        , sig[0], sig[1], sig[2], sig[3], sig[4], sig[5], sig[6], sig[7]);

                if (verbose & 1) {
                    LOG( LOG_INFO, "BmpCachePersister::save_to_disk: sig=\"%s\" original_bpp=%u cx=%u cy=%u bmp_size=%u"
                       , key, bmp->original_bpp, bmp->cx, bmp->cy, bmp_size);
                }

                stream.out_copy_bytes(sig, 8);
                stream.out_uint8(bmp->original_bpp);
                stream.out_uint16_le(bmp->cx);
                stream.out_uint16_le(bmp->cy);
                if (bmp->original_bpp == 8) {
                    stream.out_copy_bytes(reinterpret_cast<const uint8_t *>(bmp->original_palette), sizeof(bmp->original_palette));
                }
                stream.out_uint16_le(bmp_size);
                stream.mark_end();
                t.send(stream);

                t.send(bmp_data, bmp_size);
            }
        }
    }
};

#endif  // #ifndef _REDEMPTION_CORE_RDP_CACHES_BMPCACHEPERSISTER_HPP_
