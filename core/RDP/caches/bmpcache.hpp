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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier
*/

#ifndef _REDEMPTION_CORE_RDP_CACHES_BMPCACHE_HPP_
#define _REDEMPTION_CORE_RDP_CACHES_BMPCACHE_HPP_

#include "bitmap.hpp"
#include "RDP/PersistentKeyListPDU.hpp"

enum {
      BITMAP_FOUND_IN_CACHE
    , BITMAP_ADDED_TO_CACHE
};

struct BmpCache {
    static const uint8_t  MAXIMUM_NUMBER_OF_CACHES        = 5;
    static const uint16_t MAXIMUM_NUMBER_OF_CACHE_ENTRIES = 8192;

    static const uint8_t IN_WAIT_LIST = 0x80;

    const uint8_t bpp;

    uint8_t number_of_cache;

    uint16_t cache_0_entries;
    uint16_t cache_0_size;
    bool     cache_0_persistent;
    uint16_t cache_1_entries;
    uint16_t cache_1_size;
    bool     cache_1_persistent;
    uint16_t cache_2_entries;
    uint16_t cache_2_size;
    bool     cache_2_persistent;
    uint16_t cache_3_entries;
    uint16_t cache_3_size;
    bool     cache_3_persistent;
    uint16_t cache_4_entries;
    uint16_t cache_4_size;
    bool     cache_4_persistent;

    private:
    const Bitmap * cache [MAXIMUM_NUMBER_OF_CACHES + 1 /* wait_list */][MAXIMUM_NUMBER_OF_CACHE_ENTRIES];
    uint32_t       stamps[MAXIMUM_NUMBER_OF_CACHES + 1 /* wait_list */][MAXIMUM_NUMBER_OF_CACHE_ENTRIES];
    uint8_t        sha1  [MAXIMUM_NUMBER_OF_CACHES + 1 /* wait_list */][MAXIMUM_NUMBER_OF_CACHE_ENTRIES][20];

    public:
    uint32_t stamp;
    uint32_t verbose;

    public:
        BmpCache(const uint8_t bpp,
                 uint8_t number_of_cache,
                 uint16_t cache_0_entries,     uint16_t cache_0_size,     bool cache_0_persistent,
                 uint16_t cache_1_entries,     uint16_t cache_1_size,     bool cache_1_persistent,
                 uint16_t cache_2_entries,     uint16_t cache_2_size,     bool cache_2_persistent,
                 uint16_t cache_3_entries = 0, uint16_t cache_3_size = 0, bool cache_3_persistent = false,
                 uint16_t cache_4_entries = 0, uint16_t cache_4_size = 0, bool cache_4_persistent = false,
                 uint32_t verbose = 0)
            : bpp(bpp)
            , number_of_cache(number_of_cache)
            , cache_0_entries   (cache_0_entries)
            , cache_0_size      (cache_0_size)
            , cache_0_persistent(cache_0_persistent)
            , cache_1_entries   (cache_1_entries)
            , cache_1_size      (cache_1_size)
            , cache_1_persistent(cache_1_persistent)
            , cache_2_entries   (cache_2_entries)
            , cache_2_size      (cache_2_size)
            , cache_2_persistent(cache_2_persistent)
            , cache_3_entries   (cache_3_entries)
            , cache_3_size      (cache_3_size)
            , cache_3_persistent(cache_3_persistent)
            , cache_4_entries   (cache_4_entries)
            , cache_4_size      (cache_4_size)
            , cache_4_persistent(cache_4_persistent)
            , verbose(verbose)
        {
            if (this->number_of_cache > MAXIMUM_NUMBER_OF_CACHES) {
                LOG(LOG_ERR, "BmpCache: number_of_cache(%u) > %u", this->number_of_cache,
                    MAXIMUM_NUMBER_OF_CACHES);
                throw Error(ERR_RDP_PROTOCOL);
            }
            this->reset_values();
        }

        ~BmpCache() {
            this->destroy_cache();
        }

    private:
        void destroy_cache() {
            for (uint8_t cid = 0; cid < MAXIMUM_NUMBER_OF_CACHES + 1 /* wait_list */; cid++) {
                for (uint16_t cidx = 0; cidx < MAXIMUM_NUMBER_OF_CACHE_ENTRIES; cidx++) {
                    delete this->cache[cid][cidx];
                }
            }
        }

        void reset_values() {
            this->stamp = 0;
            for (uint8_t cid = 0; cid < MAXIMUM_NUMBER_OF_CACHES + 1 /* wait_list */; cid++) {
                for (uint16_t cidx = 0; cidx < MAXIMUM_NUMBER_OF_CACHE_ENTRIES; cidx++) {
                    this->cache[cid][cidx]  = NULL;
                    this->stamps[cid][cidx] = 0;
                    bzero(this->sha1[cid][cidx], sizeof(this->sha1[cid][cidx]));
                }
            }
        }

    public:
        void reset() {
            this->destroy_cache();
            this->reset_values();
        }

        void put(uint8_t id, uint16_t idx, const Bitmap * const bmp) {
            REDASSERT((id & IN_WAIT_LIST) == 0);
            delete this->cache[id][idx];
            this->cache[id][idx]  = bmp;
            this->stamps[id][idx] = ++this->stamp;
            bmp->compute_sha1(this->sha1[id][idx]);
        }

        void restamp(uint8_t id, uint16_t idx) {
            REDASSERT((id & IN_WAIT_LIST) == 0);
            this->stamps[id][idx] = ++this->stamp;
        }

        const Bitmap * get(uint8_t id, uint16_t idx) {
            if (id & IN_WAIT_LIST)
                return this->cache[MAXIMUM_NUMBER_OF_CACHES][idx];

            return this->cache[id][idx];
        }

        unsigned get_stamp(uint8_t id, uint16_t idx) {
            REDASSERT((id & IN_WAIT_LIST) == 0);
            return this->stamps[id][idx];
        }

        bool is_cache_persistent(uint8_t id) {
            switch (id) {
                case 0:                        return this->cache_0_persistent;
                case 1:                        return this->cache_1_persistent;
                case 2:                        return this->cache_2_persistent;
                case 3:                        return this->cache_3_persistent;
                case 4:                        return this->cache_4_persistent;
                // Wait list.
                case MAXIMUM_NUMBER_OF_CACHES: return true;
            }

            LOG(LOG_ERR, "BmpCache: index_of_cache(%u) > %u", id, MAXIMUM_NUMBER_OF_CACHES);
            throw Error(ERR_RDP_PROTOCOL);
            return false;
        }

        TODO("palette to use for conversion when we are in 8 bits mode should be passed from memblt.cache_id, not stored in bitmap");
        uint32_t cache_bitmap(const Bitmap & oldbmp) {
            const Bitmap * bmp = new Bitmap(this->bpp, oldbmp);

            uint8_t bmp_sha1[20];
            bmp->compute_sha1(bmp_sha1);

            uint16_t oldest_cidx = 0;

            uint16_t entries    = 0;
            uint8_t  id_real    = 0;
            uint8_t  id         = 0;
            uint32_t bmp_size   = bmp->bmp_size;
            bool     persistent = false;

                   if (this->cache_0_entries && (bmp_size <= this->cache_0_size)) {
                entries    = this->cache_0_entries;
                id_real    = 0;
                persistent = this->cache_0_persistent;
            } else if (this->cache_1_entries && (bmp_size <= this->cache_1_size)) {
                entries    = this->cache_1_entries;
                id_real    = 1;
                persistent = this->cache_1_persistent;
            } else if (this->cache_2_entries && (bmp_size <= this->cache_2_size)) {
                entries    = this->cache_2_entries;
                id_real    = 2;
                persistent = this->cache_2_persistent;
            } else if (this->cache_3_entries && (bmp_size <= this->cache_3_size)) {
                entries    = this->cache_3_entries;
                id_real    = 3;
                persistent = this->cache_3_persistent;
            } else if (this->cache_4_entries && (bmp_size <= this->cache_4_size)) {
                entries    = this->cache_4_entries;
                id_real    = 4;
                persistent = this->cache_4_persistent;
            }
            else {
                LOG(LOG_ERR,
                    "BmpCache: bitmap size(%u) too big: cache_0=%u cache_1=%u cache_2=%u cache_3=%u cache_4=%u",
                    bmp_size,
                    (this->cache_0_entries ? this->cache_0_size : 0),
                    (this->cache_1_entries ? this->cache_1_size : 0),
                    (this->cache_2_entries ? this->cache_2_size : 0),
                    (this->cache_3_entries ? this->cache_3_size : 0),
                    (this->cache_4_entries ? this->cache_4_size : 0));
                REDASSERT(0);
                throw Error(ERR_BITMAP_CACHE_TOO_BIG);
            }
            id = id_real;

            unsigned oldstamp = this->stamps[id_real][0];

            for (uint16_t cidx = 0 ; cidx < entries; cidx++) {
                if (0 == memcmp(bmp_sha1, this->sha1[id_real][cidx], sizeof(bmp_sha1))) {
                    //LOG(LOG_INFO, "Bitmap already in cache: this->cache[%u][%u]=%p", id_real, cidx,
                    //    this->cache[id_real][cidx]);
                    //hexdump_d(bmp_sha1, sizeof(bmp_sha1));
                    if (this->cache[id_real][cidx]->cx == bmp->cx) {
                        if (this->cache[id_real][cidx]->cy == bmp->cy) {
                            delete bmp;
                            return (BITMAP_FOUND_IN_CACHE << 24) | (id_real << 16) | cidx;
                        }
                    }
                }
                if (this->stamps[id_real][cidx] < oldstamp) {
                    oldest_cidx = cidx;
                    oldstamp    = this->stamps[id_real][cidx];
                }
            }

            if (persistent) {
                // The bitmap cache is persistent.
                bool bitmap_encountered = false;

                uint16_t wait_list_oldest_cidx = 0;
                unsigned wait_list_oldstamp    = this->stamps[MAXIMUM_NUMBER_OF_CACHES][0];

                for (uint16_t cidx = 0 ; cidx < MAXIMUM_NUMBER_OF_CACHE_ENTRIES; cidx++) {
                    if (0 == memcmp(bmp_sha1, this->sha1[MAXIMUM_NUMBER_OF_CACHES][cidx],
                                    sizeof(bmp_sha1))) {
                        //LOG(LOG_INFO, "Bitmap already in wait list: wait_list[%u]=%p", cidx,
                        //    this->cache[MAXIMUM_NUMBER_OF_CACHES][cidx]);
                        //hexdump_d(bmp_sha1, sizeof(bmp_sha1));
                        if (this->cache[MAXIMUM_NUMBER_OF_CACHES][cidx]->cx == bmp->cx) {
                            if (this->cache[MAXIMUM_NUMBER_OF_CACHES][cidx]->cy == bmp->cy) {
                                bitmap_encountered = true;

                                this->cache [MAXIMUM_NUMBER_OF_CACHES][cidx] = NULL;
                                this->stamps[MAXIMUM_NUMBER_OF_CACHES][cidx] = 0;
                                bzero(this->sha1[MAXIMUM_NUMBER_OF_CACHES][cidx],
                                      sizeof(this->sha1[MAXIMUM_NUMBER_OF_CACHES][cidx]));

                                if (this->verbose & 512) {
                                    LOG(LOG_INFO, "BmpCache: Put bitmap into persistent cache.");
                                }

                                break;
                            }
                        }
                    }
                    if (this->stamps[MAXIMUM_NUMBER_OF_CACHES][cidx] < wait_list_oldstamp) {
                        wait_list_oldest_cidx = cidx;
                        wait_list_oldstamp    = this->stamps[MAXIMUM_NUMBER_OF_CACHES][cidx];
                    }
                }

                if (!bitmap_encountered) {
                    id_real     =  MAXIMUM_NUMBER_OF_CACHES;
                    id          |= IN_WAIT_LIST;
                    oldest_cidx =  wait_list_oldest_cidx;

                    if (this->verbose & 512) {
                        LOG(LOG_INFO, "BmpCache: Put bitmap into wait list.");
                    }
                }
            }

            // find oldest stamp (or 0) and replace bitmap
            delete this->cache[id_real][oldest_cidx];
            this->cache [id_real][oldest_cidx] = bmp;
            this->stamps[id_real][oldest_cidx] = ++this->stamp;
            ::memcpy(this->sha1[id_real][oldest_cidx], bmp_sha1, 20);
            return (BITMAP_ADDED_TO_CACHE << 24) | (id << 16) | oldest_cidx;
        }

        void load_from_disk(const char * persistent_path, uint8_t cache_id,
            RDP::BitmapCachePersistentListEntry * entries, uint8_t number_of_entries)
        {
            char filename[2048];

            // ensures the directory exists
            if (recursive_create_directory(persistent_path, S_IRWXU | S_IRWXG, 0) != 0) {
                LOG(LOG_ERR, "BmpCache: Failed to create client persistent directory - \"%s\"",
                    persistent_path);
                throw Error(ERR_BITMAP_CACHE_PERSISTENT, 0);
            }

            uint16_t cache_index = 0;
            while (this->cache[cache_id][cache_index])
                cache_index++;

            const uint8_t * sig = reinterpret_cast<const uint8_t *>(entries);
            for (uint8_t entry_index = 0;
                 (entry_index < number_of_entries) && (cache_index < MAXIMUM_NUMBER_OF_CACHE_ENTRIES);
                 entry_index++, sig += sizeof(RDP::BitmapCachePersistentListEntry), cache_index++) {
                // generates the name of file
                snprintf(filename, sizeof(filename) - 1, "%s/%02X%02X%02X%02X%02X%02X%02X%02X",
                    persistent_path, sig[0], sig[1], sig[2], sig[3], sig[4], sig[5], sig[6], sig[7]);
                filename[sizeof(filename) - 1] = '\0';
                if (this->verbose & 512) {
                    LOG(LOG_INFO, "BmpCache: Load from disk cache, filename=\"%s\"", filename);
                }

                FILE * fd = ::fopen(filename, "r");
                if (!fd) {
                    if (this->verbose & 512) {
                        LOG(LOG_INFO, "BmpCache: Persistent disk bitmap file \"%s\" does not exist",
                            filename);
                    }
                    continue;
                }
                uint8_t    original_bpp;
                uint16_t   cx, cy;
                BGRPalette original_palette;
                uint32_t   bmp_size;
                uint8_t    data_bitmap[65536];
                if ((::fread(&original_bpp,     1, sizeof(original_bpp),     fd) != sizeof(original_bpp    )) ||
                    (::fread(&cx,               1, sizeof(cx),               fd) != sizeof(cx              )) ||
                    (::fread(&cy,               1, sizeof(cy),               fd) != sizeof(cy              )) ||
                    (::fread(&original_palette, 1, sizeof(original_palette), fd) != sizeof(original_palette)) ||
                    (::fread(&bmp_size,         1, sizeof(bmp_size),         fd) != sizeof(bmp_size        )) ||
                    (::fread(data_bitmap,       1, bmp_size,                 fd) != bmp_size               )) {
                    LOG(LOG_INFO, "BmpCache: Failed to load persistent disk bitmap file - \"%s\"", filename);
                    continue;
                }
                ::fclose(fd);

                Bitmap * bmp = new Bitmap(this->bpp, original_bpp, &original_palette, cx, cy, data_bitmap, bmp_size);

                this->put(cache_id, cache_index, bmp);
                if (this->verbose & 512) {
                    LOG(LOG_INFO,
                        "BmpCache: Load from disk cache, cache_id=%u cache_index=%u cx=%u cy=%u size=%u",
                        cache_id, cache_index, bmp->cx, bmp->cy, bmp->bmp_size);
                }
            }
        }

        void save_to_disk(const char * persistent_path, uint8_t cache_id) const {
            char filename[2048];

            // ensures the directory exists
            if (recursive_create_directory(persistent_path, S_IRWXU | S_IRWXG, 0) != 0) {
                LOG(LOG_ERR, "Failed to create client persistent directory: %s", persistent_path);
                throw Error(ERR_BITMAP_CACHE_PERSISTENT, 0);
            }

            for (uint16_t cache_index = 0; cache_index < MAXIMUM_NUMBER_OF_CACHE_ENTRIES; cache_index++) {
                if (this->cache[cache_id][cache_index]) {
                    const Bitmap  * bmp = this->cache[cache_id][cache_index];
                    const uint8_t * sig = this->sha1[cache_id][cache_index];

                    // generates the name of file
                    snprintf(filename, sizeof(filename) - 1, "%s/%02X%02X%02X%02X%02X%02X%02X%02X",
                        persistent_path, sig[0], sig[1], sig[2], sig[3], sig[4], sig[5], sig[6], sig[7]);
                    filename[sizeof(filename) - 1] = '\0';
                    if (this->verbose & 512) {
                        LOG(LOG_INFO,
                            "BmpCache: Write to disk cache, cache_id=%u cache_index=%u cx=%u cy=%u size=%u filename=\"%s\"",
                            cache_id, cache_index, bmp->cx, bmp->cy, bmp->bmp_size, filename);
                    }

                    FILE * fd = ::fopen(filename, "wb");
                    if (!fd) {
                        LOG(LOG_ERR, "Failed to open persistent disk bitmap file for writing: %s", filename);
                        throw Error(ERR_BITMAP_CACHE_PERSISTENT);
                    }
                    ::fwrite(&bmp->original_bpp, sizeof(bmp->original_bpp), 1, fd);
                    ::fwrite(&bmp->cx, sizeof(bmp->cx), 1, fd);
                    ::fwrite(&bmp->cy, sizeof(bmp->cy), 1, fd);
                    ::fwrite(&bmp->original_palette, sizeof(bmp->original_palette), 1, fd);
                    uint32_t bmp_size = bmp->bmp_size;
                    ::fwrite(&bmp_size, sizeof(bmp_size), 1, fd);
                    void * bmp_data = bmp->data_bitmap.get();
                    ::fwrite(bmp_data, bmp->bmp_size, 1, fd);
                    ::fclose(fd);
                }
            }
        }
};
#endif
