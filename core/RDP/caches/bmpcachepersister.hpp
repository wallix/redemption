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
    static const uint8_t  CURRENT_VERSION      = 1;
    static const uint32_t INVALID_CACHE_OFFSET = 0xFFFFFFFF;

    struct PersistentDiskBitmapCacheFileHeader {
        char     magic[4];
        uint8_t  version;
        uint32_t cache_offset[BmpCache::MAXIMUM_NUMBER_OF_CACHES];

        PersistentDiskBitmapCacheFileHeader()
            : version(CURRENT_VERSION)
        {
            ::memcpy(this->magic, "PDBC", sizeof(this->magic));

            for (unsigned i = 0; i < BmpCache::MAXIMUM_NUMBER_OF_CACHES; i++)
                this->cache_offset[i] = INVALID_CACHE_OFFSET;
        }
    };

    struct PersistentDiskBitmapCacheHeader {
        uint32_t bitmap_count;

        PersistentDiskBitmapCacheHeader() : bitmap_count(0) {}
    };

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

public:
    BmpCachePersister( BmpCache & bmp_cache, const char * filename)
    : bmp_cache(bmp_cache) {
        int fd = ::open(filename, O_RDONLY);
        if (fd == -1) {
            return;
        }

        PersistentDiskBitmapCacheFileHeader cache_file_header;

        if (::read(fd, &cache_file_header, sizeof(cache_file_header)) != sizeof(cache_file_header)) {
            LOG( LOG_ERR, "BmpCachePersister: failed to read from file. filename=\"%s\""
               , filename);
            throw Error(ERR_PDBC_LOAD);
        }
        //LOG( LOG_INFO, "BmpCachePersister: magic=\"%c%c%c%c\""
        //   , cache_file_header.magic[0], cache_file_header.magic[1], cache_file_header.magic[2]
        //   , cache_file_header.magic[3]);
        //LOG( LOG_INFO, "BmpCachePersister: version=%u", cache_file_header.version);
        //LOG( LOG_INFO, "BmpCachePersister: cache_0_offset=%u", cache_file_header.cache_offset[0]);
        //LOG( LOG_INFO, "BmpCachePersister: cache_1_offset=%u", cache_file_header.cache_offset[1]);
        //LOG( LOG_INFO, "BmpCachePersister: cache_2_offset=%u", cache_file_header.cache_offset[2]);
        //LOG( LOG_INFO, "BmpCachePersister: cache_3_offset=%u", cache_file_header.cache_offset[3]);
        //LOG( LOG_INFO, "BmpCachePersister: cache_4_offset=%u", cache_file_header.cache_offset[4]);

        if (::memcmp(cache_file_header.magic, "PDBC", sizeof(cache_file_header.magic))) {
            LOG( LOG_ERR, "BmpCachePersister: \"%s\" is not a persistent bitmap cache file."
               , filename);
            throw Error(ERR_PDBC_LOAD);
        }

        if (cache_file_header.version != CURRENT_VERSION) {
            LOG( LOG_ERR, "BmpCachePersister: Unsupported persistent bitmap cache file version(%u)."
               , cache_file_header.version);
            throw Error(ERR_PDBC_LOAD);
        }

        try
        {
            if ((this->bmp_cache.number_of_cache > 0) && (this->bmp_cache.cache_0_persistent)) {
                this->load_from_disk(fd, cache_file_header, 0);
            }
            if ((this->bmp_cache.number_of_cache > 1) && (this->bmp_cache.cache_1_persistent)) {
                this->load_from_disk(fd, cache_file_header, 1);
            }
            if ((this->bmp_cache.number_of_cache > 2) && (this->bmp_cache.cache_2_persistent)) {
                this->load_from_disk(fd, cache_file_header, 2);
            }
            if ((this->bmp_cache.number_of_cache > 3) && (this->bmp_cache.cache_3_persistent)) {
                this->load_from_disk(fd, cache_file_header, 3);
            }
            if ((this->bmp_cache.number_of_cache > 4) && (this->bmp_cache.cache_4_persistent)) {
                this->load_from_disk(fd, cache_file_header, 4);
            }
        }
        catch (...) {
            ::close(fd);
            throw;
        }

        ::close(fd);
    }

    void process_key_list( uint8_t cache_id, RDP::BitmapCachePersistentListEntry * entries
                         , uint8_t number_of_entries, uint16_t max_number_of_entries
                         , uint16_t first_entry_index) {
             uint16_t   cache_index = first_entry_index;
        const uint8_t * sig         = reinterpret_cast<const uint8_t *>(entries);
        for (uint8_t entry_index = 0;
             (entry_index < number_of_entries) && (cache_index < max_number_of_entries);
             entry_index++, cache_index++, sig += sizeof(RDP::BitmapCachePersistentListEntry)) {
            REDASSERT(!this->bmp_cache.cache[cache_id][cache_index]);

            char key[20];

            snprintf( key, sizeof(key), "%02X%02X%02X%02X%02X%02X%02X%02X"
                    , sig[0], sig[1], sig[2], sig[3], sig[4], sig[5], sig[6], sig[7]);

            container_type::iterator it = this->bmp_map[cache_id].find(key);
            if (it != this->bmp_map[cache_id].end()) {
                //LOG(LOG_INFO, "BmpCachePersister: bitmap found. key=\"%s\"", key);

                this->bmp_cache.put(cache_id, cache_index, it->second.bmp);
                it->second.bmp = NULL;

                this->bmp_map[cache_id].erase(it);
            }
            //else {
            //    LOG(LOG_WARNING, "BmpCachePersister: bitmap not found!!! key=\"%s\"", key);
            //}
        }
    }

private:
    void load_from_disk( int fd, const PersistentDiskBitmapCacheFileHeader & cache_file_header
                       , uint8_t cache_id) {
        if (cache_file_header.cache_offset[cache_id] == INVALID_CACHE_OFFSET)
            return;

        if (::lseek(fd, cache_file_header.cache_offset[cache_id], SEEK_SET) == -1) {
            LOG( LOG_ERR
               , "BmpCachePersister: failed to reposition file read offset.");
            throw Error(ERR_PDBC_LOAD);
        }

        PersistentDiskBitmapCacheHeader cache_header;

        if (::read(fd, &cache_header, sizeof(cache_header)) != sizeof(cache_header)) {
            LOG(LOG_ERR, "BmpCachePersister: failed to read from file.");
            throw Error(ERR_PDBC_LOAD);
        }
        LOG( LOG_INFO, "BmpCachePersister: bitmap_count=%u", cache_header.bitmap_count);

        for (uint16_t i = 0; i < cache_header.bitmap_count; i++) {
            uint8_t    sig[8];
            uint8_t    original_bpp;
            uint16_t   cx, cy;
            BGRPalette original_palette;
            uint16_t   bmp_size;
            uint8_t    data_bitmap[65536];

            if (   (::read(fd, sig,               sizeof(sig             )) != sizeof(sig             ))
                || (::read(fd, &original_bpp,     sizeof(original_bpp    )) != sizeof(original_bpp    ))
                || (::read(fd, &cx,               sizeof(cx              )) != sizeof(cx              ))
                || (::read(fd, &cy,               sizeof(cy              )) != sizeof(cy              ))
                || (::read(fd, &original_palette, sizeof(original_palette)) != sizeof(original_palette))
                || (::read(fd, &bmp_size,         sizeof(bmp_size        )) != sizeof(bmp_size        ))
                || (::read(fd, data_bitmap,              bmp_size         ) !=        bmp_size         )
               ) {
                LOG(LOG_ERR, "BmpCachePersister: failed to read from file.");
                throw Error(ERR_PDBC_LOAD);
            }

            char key[20];

            snprintf( key, sizeof(key), "%02X%02X%02X%02X%02X%02X%02X%02X"
                    , sig[0], sig[1], sig[2], sig[3], sig[4], sig[5], sig[6], sig[7]);

            //LOG( LOG_INFO, "BmpCachePersister: sig=\"%s\" original_bpp=%u cx=%u cy=%u bmp_size=%u"
            //   , key, original_bpp, cx, cy, bmp_size);

            this->bmp_map[cache_id][key].bmp = new Bitmap( this->bmp_cache.bpp, original_bpp
                                                         , &original_palette, cx, cy, data_bitmap
                                                         , bmp_size);
        }
    }

public:
    static void save_all_to_disk( const BmpCache & bmp_cache, const char * filename_final) {
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

        PersistentDiskBitmapCacheFileHeader cache_file_header;

        if (::lseek(fd, sizeof(cache_file_header), SEEK_SET) == -1) {
            LOG( LOG_ERR
               , "BmpCachePersister: failed to reposition file write offset. filename=\"%s\""
               , filename_temporary);
            ::close(fd);
            ::unlink(filename_temporary);
            throw Error(ERR_PDBC_SAVE);
        }

        try
        {
            if ((bmp_cache.number_of_cache > 0) && bmp_cache.cache_0_persistent) {
                cache_file_header.cache_offset[0] = ::lseek(fd, 0, SEEK_CUR);
                save_to_disk(bmp_cache, bmp_cache.cache_0_entries, fd, 0);
            }
            if ((bmp_cache.number_of_cache > 1) && bmp_cache.cache_1_persistent) {
                cache_file_header.cache_offset[1] = ::lseek(fd, 0, SEEK_CUR);
                save_to_disk(bmp_cache, bmp_cache.cache_1_entries, fd, 1);
            }
            if ((bmp_cache.number_of_cache > 2) && bmp_cache.cache_2_persistent) {
                cache_file_header.cache_offset[2] = ::lseek(fd, 0, SEEK_CUR);
                save_to_disk(bmp_cache, bmp_cache.cache_2_entries, fd, 2);
            }
            if ((bmp_cache.number_of_cache > 3) && bmp_cache.cache_3_persistent) {
                cache_file_header.cache_offset[3] = ::lseek(fd, 0, SEEK_CUR);
                save_to_disk(bmp_cache, bmp_cache.cache_3_entries, fd, 3);
            }
            if ((bmp_cache.number_of_cache > 4) && bmp_cache.cache_4_persistent) {
                cache_file_header.cache_offset[4] = ::lseek(fd, 0, SEEK_CUR);
                save_to_disk(bmp_cache, bmp_cache.cache_4_entries, fd, 4);
            }
        }
        catch (...)
        {
            ::close(fd);
            ::unlink(filename_temporary);
            throw;
        }

        if (::lseek(fd, 0, SEEK_SET) == -1) {
            LOG( LOG_ERR
               , "BmpCachePersister: failed to reposition file write offset. filename=\"%s\""
               , filename_temporary);
            ::close(fd);
            ::unlink(filename_temporary);
            throw Error(ERR_PDBC_SAVE);
        }

        if (::write(fd, &cache_file_header, sizeof(cache_file_header)) == -1) {
            LOG( LOG_ERR, "BmpCachePersister: failed to write to file. filename=\"%s\""
               , filename_temporary);
            ::close(fd);
            ::unlink(filename_temporary);
            throw Error(ERR_PDBC_SAVE);
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
    static void save_to_disk(const BmpCache & bmp_cache, uint16_t cache_entries, int fd, uint8_t cache_id) {

        PersistentDiskBitmapCacheHeader cache_header;

        off_t cache_start_offset = ::lseek(fd, 0, SEEK_CUR);
        if (cache_start_offset == -1) {
            LOG( LOG_ERR
               , "BmpCachePersister: failed to retrieve file write offset.");
            throw Error(ERR_PDBC_SAVE);
        }

        if (::lseek(fd, sizeof(cache_header), SEEK_CUR) == -1) {
            LOG( LOG_ERR
               , "BmpCachePersister: failed to reposition file write offset.");
            throw Error(ERR_PDBC_SAVE);
        }

        for (uint16_t cache_index = 0; cache_index < cache_entries; cache_index++) {
            if (bmp_cache.cache[cache_id][cache_index]) {
                const Bitmap   * bmp      = bmp_cache.cache[cache_id][cache_index];
                const uint8_t  * sig      = bmp_cache.sha1[cache_id][cache_index];
                const uint16_t   bmp_size = bmp->bmp_size;
                const void     * bmp_data = bmp->data_bitmap.get();

                if (   (::write(fd, sig,                    8                            ) == -1)
                    || (::write(fd, &bmp->original_bpp,     sizeof(bmp->original_bpp    )) == -1)
                    || (::write(fd, &bmp->cx,               sizeof(bmp->cx              )) == -1)
                    || (::write(fd, &bmp->cy,               sizeof(bmp->cy              )) == -1)
                    || (::write(fd, &bmp->original_palette, sizeof(bmp->original_palette)) == -1)
                    || (::write(fd, &bmp_size,              sizeof(bmp_size             )) == -1)
                    || (::write(fd, bmp_data,               bmp_size                     ) == -1)
                   ) {
                    LOG(LOG_ERR, "BmpCachePersister: failed to write to file.");
                    throw Error(ERR_PDBC_SAVE);
                }
                cache_header.bitmap_count++;
            }
        }
        off_t cache_end_offset = ::lseek(fd, 0, SEEK_CUR);
        if (cache_end_offset == -1) {
            LOG( LOG_ERR
               , "BmpCachePersister: failed to retrieve file write offset.");
            throw Error(ERR_PDBC_SAVE);
        }

        if (::lseek(fd, cache_start_offset, SEEK_SET) == -1) {
            LOG( LOG_ERR
               , "BmpCachePersister: failed to reposition file write offset.");
            throw Error(ERR_PDBC_SAVE);
        }
        if (::write(fd, &cache_header, sizeof(cache_header)) == -1) {
            LOG(LOG_ERR, "BmpCachePersister: failed to write to file.");
            throw Error(ERR_PDBC_SAVE);
        }

        if (::lseek(fd, cache_end_offset, SEEK_SET) == -1) {
            LOG( LOG_ERR
               , "BmpCachePersister: failed to reposition file write offset.");
            throw Error(ERR_PDBC_SAVE);
        }
    }
};

#endif  // #ifndef _REDEMPTION_CORE_RDP_CACHES_BMPCACHEPERSISTER_HPP_
