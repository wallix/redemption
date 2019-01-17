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


#pragma once

#include "core/RDP/caches/bmpcache.hpp"
#include "transport/out_file_transport.hpp"
#include "utils/verbose_flags.hpp"
#include "utils/fileutils.hpp"
#include "utils/sugar/unique_fd.hpp"

#include <map>


namespace RDP {
struct BitmapCachePersistentListEntry;
} // namespace RDP

class BmpCachePersister
{
private:
    static const uint8_t CURRENT_VERSION = 1;

    using map_value = Bitmap;

    class map_key
    {
        uint8_t key[8];

    public:
        explicit map_key(const uint8_t (& sig)[8]) {
            memcpy(this->key, sig, sizeof(this->key));
        }

        bool operator<(const map_key & other) const /*noexcept*/ {
            auto p = std::mismatch(this->begin(), this->end(), other.begin());
            return p.first == this->end() ? false : *p.first < *p.second;
        }

        struct CString
        {
            explicit CString(const uint8_t (& sig)[8]) {
                std::snprintf(
                    this->s, sizeof(this->s), "%02X%02X%02X%02X%02X%02X%02X%02X",
                    unsigned(sig[0]), unsigned(sig[1]), unsigned(sig[2]), unsigned(sig[3]),
                    unsigned(sig[4]), unsigned(sig[5]), unsigned(sig[6]), unsigned(sig[7])
                );
            }

            const char * c_str() const
            { return this->s; }

        private:
            char s[17];
        };

        CString str() const {
            return CString(this->key);
        }

    private:
        uint8_t const * begin() const
        { return this->key; }

        uint8_t const * end() const
        { return this->key + sizeof(this->key); }
    };

    using container_type = std::map<map_key, map_value>;

    container_type bmp_map[BmpCache::MAXIMUM_NUMBER_OF_CACHES];

    BmpCache & bmp_cache;

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        from_disk = 1,
        bmp_info  = 0x100000
    };

    // Preloads bitmap from file to be used later with Client Persistent Key List PDUs.
    BmpCachePersister(BmpCache & bmp_cache, Transport & t, const char * filename, Verbose verbose)
    : bmp_cache(bmp_cache)
    , verbose(verbose)
    {
        uint8_t buf[16];
        InStream stream(buf);

        t.recv_boom(buf, 5);  /* magic(4) + version(1) */

        const uint8_t * magic   = stream.in_uint8p(4);  /* magic(4) */
              uint8_t   version = stream.in_uint8();

        //LOG( LOG_INFO, "BmpCachePersister: magic=\"%c%c%c%c\""
        //   , magic[0], magic[1], magic[2], magic[3]);
        //LOG( LOG_INFO, "BmpCachePersister: version=%u", version);

        if (0 != ::memcmp(magic, "PDBC", 4)) {
            LOG( LOG_ERR
               , "BmpCachePersister::BmpCachePersister: "
                 "File is not a persistent bitmap cache file. filename=\"%s\""
               , filename);
            throw Error(ERR_PDBC_LOAD);
        }

        if (version != CURRENT_VERSION) {
            LOG( LOG_ERR
               , "BmpCachePersister::BmpCachePersister: "
                 "Unsupported persistent bitmap cache file version(%u). filename=\"%s\""
               , version, filename);
            throw Error(ERR_PDBC_LOAD);
        }

        for (uint8_t cache_id = 0; cache_id < this->bmp_cache.number_of_cache; cache_id++) {
            this->preload_from_disk(t, cache_id);
        }
    }

private:
    static ScreenInfo extract_screen_info(InStream& stream)
    {
        BitsPerPixel original_bpp {stream.in_uint8()};
        assert(original_bpp == BitsPerPixel{8}
            || original_bpp == BitsPerPixel{15}
            || original_bpp == BitsPerPixel{16}
            || original_bpp == BitsPerPixel{24}
            || original_bpp == BitsPerPixel{32});

        uint16_t cx = stream.in_uint16_le();
        uint16_t cy = stream.in_uint16_le();

        return ScreenInfo{original_bpp, cx, cy};
    }

    void preload_from_disk(Transport & t, uint8_t cache_id) {
        uint8_t buf[65536];
        InStream stream(buf);
        auto end = buf;
        t.recv_boom(end, 2);
        end += 2;

        uint16_t bitmap_count = stream.in_uint16_le();
        if (bool(this->verbose & Verbose::from_disk)) {
            LOG(LOG_INFO, "BmpCachePersister::preload_from_disk: bitmap_count=%u", bitmap_count);
        }

        BGRPalette original_palette = BGRPalette::classic_332();

        for (uint16_t i = 0; i < bitmap_count; i++) {
            t.recv_boom(end, 13); // sig(8) + original_bpp(1) + cx(2) + cy(2);
            end += 13;

            uint8_t sig[8];

            stream.in_copy_bytes(sig, 8); // sig(8);

            auto original_info = extract_screen_info(stream);

            if (original_info.bpp == BitsPerPixel{8}) {
                // TODO implementation and endianness dependent
                t.recv_boom(end, sizeof(original_palette));
                end += sizeof(original_palette);

                stream.in_copy_bytes(const_cast<char*>(original_palette.data()), sizeof(original_palette)); /*NOLINT*/
            }

            uint16_t bmp_size;
            t.recv_boom(end, sizeof(bmp_size));
            bmp_size = stream.in_uint16_le();

            end = buf;
            stream = InStream(buf);

            t.recv_boom(end, bmp_size);

            if (bmp_cache.get_cache(cache_id).persistent()) {
                map_key key(sig);

                if (bool(this->verbose & Verbose::bmp_info)) {
                    LOG( LOG_INFO,
                        "BmpCachePersister::preload_from_disk: sig=\"%s\" original_bpp=%u cx=%u cy=%u bmp_size=%u",
                        key.str(), original_info.bpp,
                        original_info.width, original_info.height, bmp_size);
                }

                assert(this->bmp_map[cache_id][key].is_valid() == false);

                Bitmap bmp( this->bmp_cache.bpp, original_info.bpp
                          , &original_palette
                          , safe_int(original_info.width), safe_int(original_info.height)
                          , stream.get_data(), bmp_size);

                uint8_t sha1[SslSha1::DIGEST_LENGTH];
                bmp.compute_sha1(sha1);
                if (0 != memcmp(sig, sha1, sizeof(sig))) {
                    LOG( LOG_ERR
                       , "BmpCachePersister::preload_from_disk: Preload failed. Cause: bitmap or key corruption.");
                    assert(false);
                }
                else {
                    this->bmp_map[cache_id][key] = bmp;
                }
            }

            end = buf;
            stream = InStream(buf);
        }
    }

public:
    // Places bitmaps of Persistent Key List into the cache.
    void process_key_list( uint8_t cache_id, RDP::BitmapCachePersistentListEntry * entries
                         , uint8_t number_of_entries, uint16_t first_entry_index) {
        uint16_t   max_number_of_entries = this->bmp_cache.get_cache(cache_id).size();
        uint16_t   cache_index           = first_entry_index;
        const union Sig {
            uint8_t  sig_8[8];
            uint32_t sig_32[2];
        }              * sig             = reinterpret_cast<const Sig *>(entries); /*NOLINT*/
        for (uint8_t entry_index = 0;
             (entry_index < number_of_entries) && (cache_index < max_number_of_entries);
             entry_index++, cache_index++, sig++) {
            assert(!this->bmp_cache.get_cache(cache_id)[cache_index]);

            map_key key(sig->sig_8);

            container_type::iterator it = this->bmp_map[cache_id].find(key);
            if (it != this->bmp_map[cache_id].end()) {
                if (bool(this->verbose & Verbose::bmp_info)) {
                    LOG(LOG_INFO, "BmpCachePersister: bitmap found. key=\"%s\"", key.str());
                }

                if (this->bmp_cache.get_cache(cache_id).size() > cache_index) {
                    this->bmp_cache.put(cache_id, cache_index, it->second, sig->sig_32[0], sig->sig_32[1]);
                }

                this->bmp_map[cache_id].erase(it);
            }
            else if (bool(this->verbose & Verbose::bmp_info)) {
                LOG(LOG_WARNING, "BmpCachePersister: bitmap not found!!! key=\"%s\"", key.str());
            }
        }
    }

    // Loads bitmap from file to be placed immediately into the cache.
    static void load_all_from_disk( BmpCache & bmp_cache, Transport & t, const char * filename
                                  , Verbose verbose) {
        uint8_t buf[16];
        InStream stream(buf);

        t.recv_boom(buf, 5);  /* magic(4) + version(1) */

        const uint8_t * magic   = stream.in_uint8p(4);  /* magic(4) */
              uint8_t   version = stream.in_uint8();

        //LOG( LOG_INFO, "BmpCachePersister: magic=\"%c%c%c%c\""
        //   , magic[0], magic[1], magic[2], magic[3]);
        //LOG( LOG_INFO, "BmpCachePersister: version=%u", version);

        if (0 != ::memcmp(magic, "PDBC", 4)) {
            LOG( LOG_ERR
               , "BmpCachePersister::load_all_from_disk: "
                 "File is not a persistent bitmap cache file. filename=\"%s\""
               , filename);
            throw Error(ERR_PDBC_LOAD);
        }

        if (version != CURRENT_VERSION) {
            LOG( LOG_ERR
               , "BmpCachePersister::load_all_from_disk: "
                 "Unsupported persistent bitmap cache file version(%u). filename=\"%s\""
               , version, filename);
            throw Error(ERR_PDBC_LOAD);
        }

        for (uint8_t cache_id = 0; cache_id < bmp_cache.number_of_cache; cache_id++) {
            load_from_disk(bmp_cache, t, cache_id, verbose);
        }
    }

private:
    static void load_from_disk( BmpCache & bmp_cache, Transport & t
                              , uint8_t cache_id, Verbose verbose) {
        uint8_t buf[65536];
        InStream stream(buf);
        auto end = buf;
        t.recv_boom(end, 2);
        end += 2;

        uint16_t bitmap_count = stream.in_uint16_le();
        if (bool(verbose & Verbose::from_disk)) {
            LOG(LOG_INFO, "BmpCachePersister::load_from_disk: bitmap_count=%u", bitmap_count);
        }

        BGRPalette original_palette = BGRPalette::classic_332();

        for (uint16_t i = 0; i < bitmap_count; i++) {
            t.recv_boom(end, 13); // sig(8) + original_bpp(1) + cx(2) + cy(2);
            end +=  13;

            union {
                uint8_t  sig_8[8];
                uint32_t sig_32[2];
            } sig;

            stream.in_copy_bytes(sig.sig_8, 8); // sig(8);

            auto original_info = extract_screen_info(stream);

            if (original_info.bpp == BitsPerPixel{8}) {
                // TODO implementation and endianness dependent
                t.recv_boom(end, sizeof(original_palette));
                end += sizeof(original_palette);

                stream.in_copy_bytes(const_cast<char*>(original_palette.data()), sizeof(original_palette)); /*NOLINT*/
            }

            uint16_t bmp_size;
            t.recv_boom(end, sizeof(bmp_size));
            bmp_size = stream.in_uint16_le();

            end = buf;
            stream = InStream(buf);

            t.recv_boom(end, bmp_size);
            end += bmp_size;

            if (bmp_cache.get_cache(cache_id).persistent() && (i < bmp_cache.get_cache(cache_id).size())) {
                if (bool(verbose & Verbose::bmp_info)) {
                    map_key key(sig.sig_8);
                    LOG( LOG_INFO
                        , "BmpCachePersister::load_from_disk: sig=\"%s\" original_bpp=%u cx=%u cy=%u bmp_size=%u"
                        , key.str(), original_info.bpp
                        , original_info.width, original_info.height, bmp_size);
                }


                Bitmap bmp( bmp_cache.bpp, original_info.bpp
                          , &original_palette
                          , safe_int(original_info.width), safe_int(original_info.height)
                          , stream.get_data(), stream.get_data() - end);

                bmp_cache.put(cache_id, i, bmp, sig.sig_32[0], sig.sig_32[1]);
            }

            end = buf;
            stream = InStream(buf);
        }
    }

public:
    // Saves content of cache to file.
    static void save_all_to_disk(const BmpCache & bmp_cache, Transport & t, Verbose verbose) {
        if (bool(verbose & Verbose::from_disk)) {
            bmp_cache.log();
        }

        StaticOutStream<128> stream;

        stream.out_copy_bytes("PDBC", 4);  // Magic(4)
        stream.out_uint8(CURRENT_VERSION);

        t.send(stream.get_bytes());

        for (uint8_t cache_id = 0; cache_id < bmp_cache.number_of_cache; cache_id++) {
            save_to_disk(bmp_cache, cache_id, t, verbose);
        }
    }

private:
    static void save_to_disk(const BmpCache & bmp_cache, uint8_t cache_id, Transport & t, Verbose verbose) {
        uint16_t bitmap_count = 0;
        BmpCache::cache_ const & cache = bmp_cache.get_cache(cache_id);

        if (cache.persistent()) {
            for (uint16_t cache_index = 0; cache_index < cache.size(); cache_index++) {
                if (cache[cache_index]) {
                    bitmap_count++;
                }
            }
        }

        StaticOutStream<65535> stream;
        stream.out_uint16_le(bitmap_count);
        t.send(stream.get_bytes());
        if (!bitmap_count) {
            return;
        }

        for (uint16_t cache_index = 0; cache_index < cache.size(); cache_index++) {
            if (cache[cache_index]) {
                stream.rewind();

                const Bitmap &   bmp      = cache[cache_index].bmp;
                const uint8_t (& sig)[8]  = cache[cache_index].sig.sig_8;
                const uint16_t   bmp_size = bmp.bmp_size();
                const uint8_t  * bmp_data = bmp.data();

                // if (bmp_cache.owner == BmpCache::Front) {
                //     uint8_t sha1[SslSha1::DIGEST_LENGTH];
                //     bmp.compute_sha1(sha1);

                //     char sig_sig[SslSha1::DIGEST_LENGTH];

                //     snprintf( sig_sig, sizeof(sig_sig), "%02X%02X%02X%02X%02X%02X%02X%02X"
                //             , sig[0], sig[1], sig[2], sig[3], sig[4], sig[5], sig[6], sig[7]);

                //     char sig_sha1[SslSha1::DIGEST_LENGTH];

                //     snprintf( sig_sha1, sizeof(sig_sig), "%02X%02X%02X%02X%02X%02X%02X%02X"
                //             , sha1[0], sha1[1], sha1[2], sha1[3], sha1[4], sha1[5], sha1[6], sha1[7]);

                //     LOG( LOG_INFO
                //        , "BmpCachePersister::save_to_disk: sig=\"%s\" sha1=\"%s\" original_bpp=%u cx=%u cy=%u bmp_size=%u"
                //        , sig_sig, sig_sha1, bmp.bpp(), bmp.cx(), bmp.cy(), bmp_size);

                //     assert(!memcmp(bmp_cache.sig[cache_id][cache_index].sig_8, sha1, sizeof(bmp_cache.sig[cache_id][cache_index].sig_8)));
                // }

                map_key key(sig);

                if (bool(verbose & Verbose::bmp_info)) {
                    LOG( LOG_INFO
                       , "BmpCachePersister::save_to_disk: sig=\"%s\" original_bpp=%u cx=%u cy=%u bmp_size=%u"
                       , key.str(), bmp.bpp(), bmp.cx(), bmp.cy(), bmp_size);
                }

                stream.out_copy_bytes(sig, 8);
                stream.out_uint8(safe_int(bmp.bpp()));
                stream.out_uint16_le(bmp.cx());
                stream.out_uint16_le(bmp.cy());
                if (bmp.bpp() == BitsPerPixel{8}) {
                    // TODO implementation and endianness dependent
                    stream.out_copy_bytes(bmp.palette().data(), sizeof(bmp.palette()));
                }
                stream.out_uint16_le(bmp_size);
                t.send(stream.get_bytes());

                t.send(bmp_data, bmp_size);
            }
        }
    }
};

inline void save_persistent_disk_bitmap_cache(
    BmpCache const & bmp_cache,
    const char * persistent_path,
    const char * target_host,
    BitsPerPixel bpp,
    ReportError report_error,
    BmpCachePersister::Verbose verbose
)
{
    // Ensures that the directory exists.
    if (::recursive_create_directory(persistent_path, S_IRWXU | S_IRWXG, -1) != 0) {
        LOG( LOG_ERR
            , "save_persistent_disk_bitmap_cache: failed to create directory \"%s\"."
            , persistent_path);
        throw Error(ERR_BITMAP_CACHE_PERSISTENT, 0);
    }

    char filename_temporary[2048];
    ::snprintf(filename_temporary, sizeof(filename_temporary) - 1, "%s/PDBC-%s-%d-XXXXXX.tmp",
        persistent_path, target_host, underlying_cast(bpp));
    filename_temporary[sizeof(filename_temporary) - 1] = '\0';

    int fd = ::mkostemps(filename_temporary, 4, O_CREAT | O_WRONLY);
    if (fd == -1) {
        LOG( LOG_ERR
            , "save_persistent_disk_bitmap_cache: "
                "failed to open (temporary) file for writing. filename=\"%s\""
            , filename_temporary);
        throw Error(ERR_PDBC_SAVE);
    }

    try
    {
        {
            OutFileTransport oft(unique_fd{fd}, std::move(report_error));
            BmpCachePersister::save_all_to_disk(bmp_cache, oft, verbose);
        }

        // Generates the name of file.
        char filename[2048];
        ::snprintf(filename, sizeof(filename) - 1, "%s/PDBC-%s-%d", persistent_path, target_host, underlying_cast(bpp));
        filename[sizeof(filename) - 1] = '\0';

        if (::rename(filename_temporary, filename) == -1) {
            LOG( LOG_WARNING
                , "save_persistent_disk_bitmap_cache: failed to rename the (temporary) file. "
                    "old_filename=\"%s\" new_filename=\"%s\""
                , filename_temporary, filename);
            ::unlink(filename_temporary);
        }
    }
    catch (...) {
        LOG( LOG_WARNING
            , "save_persistent_disk_bitmap_cache: failed to write (temporary) file. "
                "filename=\"%s\""
            , filename_temporary);
        ::unlink(filename_temporary);
    }
}
