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
    BmpCachePersister( BmpCache & bmp_cache, const char * filename, uint32_t verbose = 0)
    : bmp_cache(bmp_cache)
    , verbose(verbose) {
        int fd = ::open(filename, O_RDONLY);
        if (fd == -1) {
            return;
        }

        uint8_t magic[4];
        if (::read(fd, magic, sizeof(magic)) != sizeof(magic)) {
            LOG( LOG_ERR, "BmpCachePersister: failed to read from file. filename=\"%s\""
               , filename);
            throw Error(ERR_PDBC_LOAD);
        }

        uint8_t version;
        if (::read(fd, &version, sizeof(version)) != sizeof(version)) {
            LOG( LOG_ERR, "BmpCachePersister: failed to read from file. filename=\"%s\""
               , filename);
            throw Error(ERR_PDBC_LOAD);
        }

        //LOG( LOG_INFO, "BmpCachePersister: magic=\"%c%c%c%c\""
        //   , magic[0], magic[1], magic[2], magic[3]);
        //LOG( LOG_INFO, "BmpCachePersister: version=%u", version);

        if (::memcmp(magic, "PDBC", sizeof(magic))) {
            LOG( LOG_ERR, "BmpCachePersister: \"%s\" is not a persistent bitmap cache file."
               , filename);
            throw Error(ERR_PDBC_LOAD);
        }

        if (version != CURRENT_VERSION) {
            LOG( LOG_ERR, "BmpCachePersister: Unsupported persistent bitmap cache file version(%u)."
               , version);
            throw Error(ERR_PDBC_LOAD);
        }

        try
        {
            for (uint8_t cache_id = 0; cache_id < this->bmp_cache.number_of_cache; cache_id++) {
                this->load_from_disk(fd, filename, cache_id);
            }
        }
        catch (...) {
            ::close(fd);
            throw;
        }

        ::close(fd);
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
    void load_from_disk( int fd, const char * filename, uint8_t cache_id) {
        uint16_t bitmap_count;
        if (::read(fd, &bitmap_count, sizeof(bitmap_count)) != sizeof(bitmap_count)) {
            LOG(LOG_ERR, "BmpCachePersister: failed to read from file. filename=\"%s\"", filename);
            throw Error(ERR_PDBC_LOAD);
        }
        if (verbose & 1) {
            LOG(LOG_INFO, "BmpCachePersister: bitmap_count=%u", bitmap_count);
        }

        for (uint16_t i = 0; i < bitmap_count; i++) {
            uint8_t    sig[8];
            uint8_t    original_bpp;
            uint16_t   cx, cy;
            BGRPalette original_palette;
            uint16_t   bmp_size;
            uint8_t    data_bitmap[65536];

            if (   ( ::read(fd, sig,               sizeof(sig             )) != sizeof(sig             ))
                || ( ::read(fd, &original_bpp,     sizeof(original_bpp    )) != sizeof(original_bpp    ))
                || ( ::read(fd, &cx,               sizeof(cx              )) != sizeof(cx              ))
                || ( ::read(fd, &cy,               sizeof(cy              )) != sizeof(cy              ))
                || ((original_bpp == 8) &&
                    (::read(fd, &original_palette, sizeof(original_palette)) != sizeof(original_palette))
                   )
                || ( ::read(fd, &bmp_size,         sizeof(bmp_size        )) != sizeof(bmp_size        ))
                || ( ::read(fd, data_bitmap,              bmp_size         ) !=        bmp_size         )
               ) {
                LOG(LOG_ERR, "BmpCachePersister: failed to read from file. filename=\"%s\"", filename);
                throw Error(ERR_PDBC_LOAD);
            }

            if (this->bmp_cache.cache_persistent[cache_id]) {
                char key[20];

                snprintf( key, sizeof(key), "%02X%02X%02X%02X%02X%02X%02X%02X"
                        , sig[0], sig[1], sig[2], sig[3], sig[4], sig[5], sig[6], sig[7]);

                if (verbose & 1) {
                    LOG( LOG_INFO, "BmpCachePersister: sig=\"%s\" original_bpp=%u cx=%u cy=%u bmp_size=%u"
                       , key, original_bpp, cx, cy, bmp_size);
                }

                REDASSERT(this->bmp_map[cache_id][key].bmp == NULL);

                this->bmp_map[cache_id][key].bmp = new Bitmap( this->bmp_cache.bpp, original_bpp
                                                             , &original_palette, cx, cy, data_bitmap
                                                             , bmp_size);
            }
        }
    }

public:
    static void load_all_from_disk(BmpCache & bmp_cache, const char * filename) {
        int fd = ::open(filename, O_RDONLY);
        if (fd == -1) {
            return;
        }

        uint8_t magic[4];
        if (::read(fd, magic, sizeof(magic)) != sizeof(magic)) {
            LOG( LOG_ERR, "BmpCachePersister: failed to read from file. filename=\"%s\""
               , filename);
            throw Error(ERR_PDBC_LOAD);
        }

        uint8_t version;
        if (::read(fd, &version, sizeof(version)) != sizeof(version)) {
            LOG( LOG_ERR, "BmpCachePersister: failed to read from file. filename=\"%s\""
               , filename);
            throw Error(ERR_PDBC_LOAD);
        }

        //LOG( LOG_INFO, "BmpCachePersister: magic=\"%c%c%c%c\""
        //   , magic[0], magic[1], magic[2], magic[3]);
        //LOG( LOG_INFO, "BmpCachePersister: version=%u", version);

        if (::memcmp(magic, "PDBC", sizeof(magic))) {
            LOG( LOG_ERR, "BmpCachePersister: \"%s\" is not a persistent bitmap cache file."
               , filename);
            throw Error(ERR_PDBC_LOAD);
        }

        if (version != CURRENT_VERSION) {
            LOG( LOG_ERR, "BmpCachePersister: Unsupported persistent bitmap cache file version(%u)."
               , version);
            throw Error(ERR_PDBC_LOAD);
        }

        try
        {
            for (uint8_t cache_id = 0; cache_id < bmp_cache.number_of_cache; cache_id++) {
                load_cache_from_disk(fd, filename, bmp_cache, cache_id);
            }
        }
        catch (...) {
            ::close(fd);
            throw;
        }

        ::close(fd);
    }

    static void load_cache_from_disk( int fd, const char * filename, BmpCache & bmp_cache
                                    , uint8_t cache_id) {
        uint16_t bitmap_count;
        if (::read(fd, &bitmap_count, sizeof(bitmap_count)) != sizeof(bitmap_count)) {
            LOG(LOG_ERR, "BmpCachePersister: failed to read from file. filename=\"%s\"", filename);
            throw Error(ERR_PDBC_LOAD);
        }
        LOG(LOG_INFO, "BmpCachePersister: bitmap_count=%u", bitmap_count);

        for (uint16_t i = 0; i < bitmap_count; i++) {
            union {
                uint8_t  sig_8[8];
                uint32_t sig_32[2];
            } sig;
            uint8_t    original_bpp;
            uint16_t   cx, cy;
            BGRPalette original_palette;
            uint16_t   bmp_size;
            uint8_t    data_bitmap[65536];

            if (   ( ::read(fd, sig.sig_8,         sizeof(sig.sig_8       )) != sizeof(sig.sig_8       ))
                || ( ::read(fd, &original_bpp,     sizeof(original_bpp    )) != sizeof(original_bpp    ))
                || ( ::read(fd, &cx,               sizeof(cx              )) != sizeof(cx              ))
                || ( ::read(fd, &cy,               sizeof(cy              )) != sizeof(cy              ))
                || ((original_bpp == 8) &&
                    (::read(fd, &original_palette, sizeof(original_palette)) != sizeof(original_palette))
                   )
                || ( ::read(fd, &bmp_size,         sizeof(bmp_size        )) != sizeof(bmp_size        ))
                || ( ::read(fd, data_bitmap,              bmp_size         ) !=        bmp_size         )
               ) {
                LOG(LOG_ERR, "BmpCachePersister: failed to read from file. filename=\"%s\"", filename);
                throw Error(ERR_PDBC_LOAD);
            }

            if ((bmp_cache.cache_persistent[cache_id]) &&
                (i < bmp_cache.cache_entries[cache_id])) {
                Bitmap * bmp = new Bitmap( bmp_cache.bpp, original_bpp
                                         , &original_palette, cx, cy, data_bitmap, bmp_size);

                bmp_cache.put(cache_id, i, bmp, sig.sig_32[0], sig.sig_32[1]);
            }
        }
    }

    static void save_all_to_disk(const BmpCache & bmp_cache, const char * filename_final, uint32_t verbose = 0) {
        if (verbose & 1) {
            bmp_cache.log();
        }

        char persistent_path[1024];
        char persistent_basename[1024];
        char persistent_extension[256];

        ::strcpy(persistent_path, PERSISTENT_PATH "/");
        ::strcpy(persistent_basename, "redemption");
        ::strcpy(persistent_extension, "");

        if (!::canonical_path( filename_final
                             , persistent_path, sizeof(persistent_path)
                             , persistent_basename, sizeof(persistent_basename)
                             , persistent_extension, sizeof(persistent_extension))) {
            LOG(LOG_ERR, "BmpCachePersister: buffer overflowed, path too long. filename=\"%s\"",
                filename_final);
            throw Error(ERR_BITMAP_CACHE_PERSISTENT, 0);
        }

        // Ensures that the directory exists.
        if (::recursive_create_directory(persistent_path, S_IRWXU | S_IRWXG, 0) != 0) {
            LOG(LOG_ERR, "BmpCachePersister: failed to create directory \"%s\".", persistent_path);
            throw Error(ERR_BITMAP_CACHE_PERSISTENT, 0);
        }

        char filename_temporary[2048];

        // Generates the name of temporary file.
        ::snprintf(filename_temporary, sizeof(filename_temporary) - 1, "%s/%s-XXXXXX.tmp",
            persistent_path, persistent_basename);
        filename_temporary[sizeof(filename_temporary) - 1] = '\0';

        int fd = ::mkostemps(filename_temporary, 4, O_CREAT | O_WRONLY);
        if (fd == -1) {
            LOG( LOG_ERR
               , "BmpCachePersister: failed to open (temporary) file for writing. filename=\"%s\""
               , filename_temporary);
            throw Error(ERR_PDBC_SAVE);
        }

        if (::write(fd, "PDBC", 4) != 4) {
            LOG( LOG_ERR, "BmpCachePersister: failed to write to file. filename=\"%s\""
               , filename_temporary);
            ::close(fd);
            ::unlink(filename_temporary);
            throw Error(ERR_PDBC_SAVE);
        }

        uint8_t version = CURRENT_VERSION;
        if (::write(fd, &version, sizeof(version)) != sizeof(CURRENT_VERSION)) {
            LOG( LOG_ERR, "BmpCachePersister: failed to write to file. filename=\"%s\""
               , filename_temporary);
            ::close(fd);
            ::unlink(filename_temporary);
            throw Error(ERR_PDBC_SAVE);
        }

        try
        {
            for (uint8_t cache_id = 0; cache_id < bmp_cache.number_of_cache; cache_id++) {
                save_to_disk(bmp_cache, cache_id, fd, verbose);
            }
        }
        catch (...)
        {
            ::close(fd);
            ::unlink(filename_temporary);
            throw;
        }

        ::close(fd);

        if (::rename(filename_temporary, filename_final) == -1) {
            LOG( LOG_ERR
               , "BmpCachePersister: failed to rename the (temporary) file. "
                 "old_filename=\"%s\" new_filename=\"%s\""
               , filename_temporary, filename_final);
            throw Error(ERR_PDBC_SAVE);
            ::unlink(filename_temporary);
        }
    }

private:
    static void save_to_disk(const BmpCache & bmp_cache, uint8_t cache_id, int fd, uint32_t verbose) {
        uint16_t bitmap_count = 0;
        if (bmp_cache.cache_persistent[cache_id]) {
            for (uint16_t cache_index = 0; cache_index < bmp_cache.cache_entries[cache_id]; cache_index++) {
                if (bmp_cache.cache[cache_id][cache_index]) {
                    bitmap_count++;
                }
            }
        }
        if (::write(fd, &bitmap_count, sizeof(bitmap_count)) != sizeof(bitmap_count)) {
            LOG(LOG_ERR, "BmpCachePersister: failed to write to file.");
            throw Error(ERR_PDBC_SAVE);
        }
        if (!bitmap_count) {
            return;
        }

        for (uint16_t cache_index = 0; cache_index < bmp_cache.cache_entries[cache_id]; cache_index++) {
            if (bmp_cache.cache[cache_id][cache_index]) {
                const Bitmap   * bmp      = bmp_cache.cache[cache_id][cache_index];
                const uint8_t  * sig      = bmp_cache.sig[cache_id][cache_index].sig_8;
                const uint16_t   bmp_size = bmp->bmp_size;
                const void     * bmp_data = bmp->data_bitmap.get();

                char key[20];

                snprintf( key, sizeof(key), "%02X%02X%02X%02X%02X%02X%02X%02X"
                        , sig[0], sig[1], sig[2], sig[3], sig[4], sig[5], sig[6], sig[7]);

                if (verbose & 1) {
                    LOG( LOG_INFO, "BmpCachePersister: sig=\"%s\" original_bpp=%u cx=%u cy=%u bmp_size=%u"
                       , key, bmp->original_bpp, bmp->cx, bmp->cy, bmp_size);
                }

                if (   ( ::write(fd, sig,                    8                            ) != 8                            )
                    || ( ::write(fd, &bmp->original_bpp,     sizeof(bmp->original_bpp    )) != sizeof(bmp->original_bpp    ))
                    || ( ::write(fd, &bmp->cx,               sizeof(bmp->cx              )) != sizeof(bmp->cx              ))
                    || ( ::write(fd, &bmp->cy,               sizeof(bmp->cy              )) != sizeof(bmp->cy              ))
                    || ((bmp->original_bpp == 8) &&
                        (::write(fd, &bmp->original_palette, sizeof(bmp->original_palette)) != sizeof(bmp->original_palette))
                       )
                    || ( ::write(fd, &bmp_size,              sizeof(bmp_size             )) != sizeof(bmp_size             ))
                    || ( ::write(fd, bmp_data,               bmp_size                     ) != bmp_size                     )
                   ) {
                    LOG(LOG_ERR, "BmpCachePersister: failed to write to file.");
                    throw Error(ERR_PDBC_SAVE);
                }
            }
        }
    }
};

#endif  // #ifndef _REDEMPTION_CORE_RDP_CACHES_BMPCACHEPERSISTER_HPP_
