/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redver video verifier program
*/

#include "main/do_recorder.hpp"

#include <type_traits>
#include <string>
#include <vector>
#include <utility>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "system/ssl_calls.hpp"

#include "openssl_crypto.hpp"

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/exchange.hpp"
#include "utils/sugar/iter.hpp"
#include "utils/chex_to_int.hpp"
#include "utils/fdbuf.hpp"
#include "utils/fileutils.hpp"
#include "utils/urandom_read.hpp"
#include "utils/log.hpp"
#include "utils/word_identification.hpp"
#include "configs/config.hpp"
#include "program_options/program_options.hpp"

#include "main/version.hpp"

#include "transport/in_filename_transport.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/out_meta_sequence_transport.hpp"
#include "transport/transport.hpp"

#include "acl/auth_api.hpp"
#include "capture/capture.hpp"
#include "capture/cryptofile.hpp"
#include "capture/FileToChunk.hpp"
#include "capture/GraphicToFile.hpp"
#include "utils/apps/recording_progress.hpp"





struct HashHeader {
    unsigned version;
};


inline void load_hash(
    MetaLine2 & hash_line,
    const std::string & full_hash_path, const std::string & input_filename,
    unsigned int infile_version, bool infile_is_checksumed,
    CryptoContext & cctx, bool infile_is_encrypted, int verbose
) {
    ifile_read_encrypted in_hash_fb(cctx, infile_is_encrypted);

    if (in_hash_fb.open(full_hash_path.c_str()) < 0) {
        throw Error(ERR_TRANSPORT_OPEN_FAILED);
    }

    char buffer[8192]{};
    ssize_t len;
    {
        ssize_t remaining = sizeof(buffer);
        char * p = buffer;
        while ((len = in_hash_fb.read(p, remaining))) {
            if (len < 0){
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
            p += len;
            remaining -= len;
        }
        len = p - buffer;
    }

    char * eof = &buffer[len];
    char * cur = &buffer[0];

    if (infile_version == 1) {
        if (verbose) {
            LOG(LOG_INFO, "Hash data v1");
        }
        // Filename HASH_64_BYTES
        //         ^
        //         |
        //     separator

        int len = input_filename.length()+1;
        if (eof-cur < len){
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }

        if (0 != memcmp(cur, input_filename.c_str(), input_filename.length()))
        {
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        cur += input_filename.length();
        if (cur[0] != ' '){
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        cur++;
        in_copy_bytes(hash_line.hash1, MD_HASH_LENGTH, cur, eof, ERR_TRANSPORT_READ_FAILED);
        in_copy_bytes(hash_line.hash2, MD_HASH_LENGTH, cur, eof, ERR_TRANSPORT_READ_FAILED);
    }
    else {
        if (verbose) {
            LOG(LOG_INFO, "Hash data v2 or higher");
        }

        // v2
        if (cur == eof || cur[0] != 'v'){
            Error(ERR_TRANSPORT_READ_FAILED, errno);
        }

        // skip 3 lines
        for (auto i = 0 ; i < 3 ; i++)
        {
            char * pos = std::find(cur, eof, '\n');
            if (pos == eof) {
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
            cur = pos + 1;
        }

        // Line format "fffff
        // st_size st_mode st_uid st_gid st_dev st_ino st_mtime
        // st_ctime hhhhh HHHHH"
        //         ^  ^  ^  ^
        //         |  |  |  |
        //         |hash1|  |
        //         |     |  |
        //       space   |hash2
        //                  |
        //                space
        //
        // filename(1 or >) + space(1)
        // + stat_info(ll|ull * 8) + space(1)
        // + hash1(64) + space(1) + hash2(64) >= 135

        // filename(1 or >) followed by space
        {
            char * pos = std::find(cur, eof, ' ');
            if (pos == eof){
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
            if (size_t(pos-cur) != input_filename.length()
            || (0 != strncmp(cur, input_filename.c_str(), pos-cur)))
            {
                std::cerr << "File name mismatch: \""
                            << input_filename
                            << "\"" << std::endl
                            << std::endl;
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
            memcpy(hash_line.filename, cur, pos - cur);
            hash_line.filename[pos-cur]=0;
            cur = pos + 1;
        }
        hash_line.size = get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
        hash_line.mode = get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
        hash_line.uid = get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
        hash_line.gid = get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
        hash_line.dev = get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
        hash_line.ino = get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
        hash_line.mtime = get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
        hash_line.ctime = get_ll(cur, eof, infile_is_checksumed ? ' ' : '\n', ERR_TRANSPORT_READ_FAILED);

        if (infile_is_checksumed){
            // HASH1 + space
            in_hex256(hash_line.hash1, MD_HASH_LENGTH, cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
            // HASH1 + CR
            in_hex256(hash_line.hash2, MD_HASH_LENGTH, cur, eof, '\n', ERR_TRANSPORT_READ_FAILED);
        }
    }
}

static inline bool meta_line_stat_equal_stat(MetaLine2 const & metadata, struct stat64 const & sb)
{
    return metadata.dev == sb.st_dev
        && metadata.ino == sb.st_ino
        && metadata.mode == sb.st_mode
        && metadata.uid == sb.st_uid
        && metadata.gid == sb.st_gid
        && metadata.mtime == sb.st_mtime
        && metadata.ctime == sb.st_ctime
        && metadata.size == sb.st_size;
}

struct out_is_mismatch
{
    bool & is_mismatch;
};

static inline int check_file(const std::string & filename, const MetaLine2 & metadata,
                      bool quick, bool has_checksum, bool ignore_stat_info,
                      uint8_t const * hmac_key, size_t hmac_key_len, bool update_stat_info, out_is_mismatch has_mismatch_stat)
{
    has_mismatch_stat.is_mismatch = false;
    struct stat64 sb;
    memset(&sb, 0, sizeof(sb));
    if (lstat64(filename.c_str(), &sb) < 0){
        std::cerr << "File \"" << filename << "\" is invalid! (can't stat file)\n" << std::endl;
        return false;
    }

    if (has_checksum){
        if (sb.st_size != metadata.size){
            std::cerr << "File \"" << filename << "\" is invalid! (size mismatch)\n" << std::endl;
            return false;
        }

        uint8_t hash[SHA256_DIGEST_LENGTH]{};
        if (file_start_hmac_sha256(filename.c_str(),
                             hmac_key, hmac_key_len,
                             quick?QUICK_CHECK_LENGTH:0, hash) < 0) {
            std::cerr << "Error reading file \"" << filename << "\"\n" << std::endl;
            return false;
        }
        if (0 != memcmp(hash, quick?metadata.hash1:metadata.hash2, SHA256_DIGEST_LENGTH)){
            std::cerr << "Error checking file \"" << filename << "\" (invalid checksum)\n" << std::endl;
            return false;
        }
    }
    else if ((!ignore_stat_info || update_stat_info) && !meta_line_stat_equal_stat(metadata, sb)) {
        if (update_stat_info) {
            has_mismatch_stat.is_mismatch = true;
        }
        if (!ignore_stat_info) {
            std::cerr << "File \"" << filename << "\" is invalid! (metafile changed)\n" << std::endl;
        }
        return false;
    }
    return true;
}

static inline int check_encrypted_or_checksumed(
    std::string const & input_filename,
    std::string const & mwrm_path,
    std::string const & hash_path,
    bool quick_check,
    bool ignore_stat_info,
    bool update_stat_info,
    uint32_t verbose,
    CryptoContext & cctx
) {

    std::string const full_mwrm_filename = mwrm_path + input_filename;

    // Let(s ifile_read autodetect encryption at opening for first file
    int encryption = 2;

//    cctx.old_encryption_scheme = true;
    ifile_read_encrypted ibuf(cctx, encryption);
    
    if (ibuf.open(full_mwrm_filename.c_str()) < 0){
        throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
    }

    // now force encryption for sub files
    bool infile_is_encrypted = ibuf.encrypted;

    MwrmReader reader(ibuf);
    reader.read_meta_headers();

    // if we have version 1 header, ignore stat info
    ignore_stat_info |= (reader.header.version == 1);
    // if we have version >1 header and not checksum, update stat info
    update_stat_info &= (reader.header.version > 1) & !reader.header.has_checksum & !ibuf.encrypted;
    ignore_stat_info |= update_stat_info;

    /*****************
    * Load file hash *
    *****************/
    LOG(LOG_INFO, "Load file hash");
    MetaLine2 hash_line = {{}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}};

    std::string const full_hash_path = hash_path + input_filename;

    // if reading hash fails
    try {
        load_hash(hash_line, full_hash_path, input_filename, reader.header.version, reader.header.has_checksum, cctx, infile_is_encrypted, verbose);
    }
    catch (...) {
        std::cerr << "Cannot read hash file: \"" << full_hash_path << "\"\n" << std::endl;
        // this is an error because checksum comes from hash file
        // and extended stat info also comes from hash file
        // if we can't read hash files we are in troubles
        if (reader.header.has_checksum || !ignore_stat_info){
            return 1;
        }
    }

    bool has_mismatch_stat_hash = false;

    /******************
    * Check mwrm file *
    ******************/
    LOG(LOG_INFO, "Check mwrm file");
    if (!check_file(
        full_mwrm_filename, hash_line, quick_check, reader.header.has_checksum,
        ignore_stat_info, cctx.get_hmac_key(), 32,
        update_stat_info, out_is_mismatch{has_mismatch_stat_hash}
    )){
        if (!has_mismatch_stat_hash) {
            return 1;
        }
    }

    struct MetaLine2CtxForRewriteStat
    {
        std::string wrm_filename;
        std::string filename;
        time_t start_time;
        time_t stop_time;
    };
    std::vector<MetaLine2CtxForRewriteStat> meta_line_ctx_list;
    bool wrm_stat_is_ok = true;

    MetaLine2 meta_line_wrm;

    while (reader.read_meta_file(meta_line_wrm)) {
        size_t tmp_wrm_filename_len = 0;
        const char * tmp_wrm_filename = basename_len(meta_line_wrm.filename, tmp_wrm_filename_len);
        std::string const meta_line_wrm_filename = std::string(tmp_wrm_filename, tmp_wrm_filename_len);
        std::string const full_part_filename = mwrm_path + meta_line_wrm_filename;

        bool has_mismatch_stat_mwrm = false;
        if (!check_file(
            full_part_filename, meta_line_wrm, quick_check, reader.header.has_checksum,
            ignore_stat_info, cctx.get_hmac_key(), 32,
            update_stat_info, out_is_mismatch{has_mismatch_stat_mwrm})
        ){
            if (has_mismatch_stat_mwrm) {
                wrm_stat_is_ok = false;
            }
            else {
                return 1;
            }
        }

        if (update_stat_info) {
            meta_line_ctx_list.push_back({
                full_part_filename,
                meta_line_wrm.filename,
                meta_line_wrm.start_time,
                meta_line_wrm.stop_time
            });
        }
    }

    ibuf.close();


    /*******************
    * Rewite stat info *
    ********************/
    LOG(LOG_INFO, "Rewrite state info");
    struct local_auto_remove
    {
        char const * filename;
        ~local_auto_remove() {
            if (this->filename) {
                remove(this->filename);
            }
        }
    };

    if (!wrm_stat_is_ok) {
        if (verbose) {
            LOG(LOG_INFO, "%s", "Update mwrm file");
        }

        has_mismatch_stat_hash = true;

        auto full_mwrm_filename_tmp = full_mwrm_filename + ".tmp";

        // out_meta_sequence_filename_buf_impl ctor
        transbuf::ofile_buf_out mwrm_file_cp;
        if (mwrm_file_cp.open(full_mwrm_filename_tmp.c_str(), S_IRUSR | S_IRGRP | S_IWUSR) < 0) {
            LOG(LOG_ERR, "Failed to open meta file %s", full_mwrm_filename_tmp.c_str());
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
        }
        local_auto_remove auto_remove{full_mwrm_filename_tmp.c_str()};
        if (chmod(full_mwrm_filename_tmp.c_str(), S_IRUSR | S_IRGRP) == -1) {
            LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                , full_mwrm_filename_tmp.c_str()
                , "u+r, g+r"
                , strerror(errno), errno);
            return 1;
        }

        // copy mwrm headers
        {
            ifile_read mwrm_file;
            mwrm_file.open(full_mwrm_filename.c_str());
            LineReader line_reader(mwrm_file);

            // v2, w h, nochecksum, blank, blank
            for (int i = 0; i < 5; ++i) {
                if (!line_reader.next_line()) {
                    throw Error(ERR_TRANSPORT_READ_FAILED, 0);
                }
                auto av = line_reader.get_buf();
                if (mwrm_file_cp.write(av.data(), av.size()) != ssize_t(av.size())) {
                    throw Error(ERR_TRANSPORT_WRITE_FAILED, 0);
                }
            }
        }

        for (MetaLine2CtxForRewriteStat & ctx : meta_line_ctx_list) {
            struct stat sb;
            if (lstat(ctx.wrm_filename.c_str(), &sb) < 0
             || detail::write_meta_file_impl<true>(mwrm_file_cp, ctx.filename.c_str(), sb, ctx.start_time, ctx.stop_time)
            ) {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, 0);
            }
        }

        if (rename(full_mwrm_filename_tmp.c_str(), full_mwrm_filename.c_str())) {
            std::cerr << strerror(errno) << std::endl;
            return 1;
        }

        if (verbose) {
            LOG(LOG_INFO, "%s", "Update mwrm file, done");
        }
        auto_remove.filename = nullptr;
    }

    if (has_mismatch_stat_hash) {
        if (verbose) {
            LOG(LOG_INFO, "%s", "Update hash file");
        }

        auto const full_hash_path_tmp = (full_hash_path + ".tmp");
        transbuf::ofile_buf_out hash_file_cp;

        local_auto_remove auto_remove{full_hash_path_tmp.c_str()};

        if (detail::write_meta_hash(
            full_hash_path_tmp.c_str(), full_mwrm_filename.c_str(),
            hash_file_cp, nullptr
        )) {
            return 1;
        }

        if (rename(full_hash_path_tmp.c_str(), full_hash_path.c_str())) {
            std::cerr << strerror(errno) << std::endl;
            return 1;
        }

        auto_remove.filename = nullptr;
        if (verbose) {
            LOG(LOG_INFO, "%s", "Update hash file, done");
        }
    }

    std::cout << "No error detected during the data verification.\n" << std::endl;
    return 0;
}

enum {
    USE_ORIGINAL_COMPRESSION_ALGORITHM = 0xFFFFFFFF
};

enum {
    USE_ORIGINAL_COLOR_DEPTH           = 0xFFFFFFFF
};

inline unsigned get_file_count(
    InMetaSequenceTransport & in_wrm_trans,
    uint32_t & begin_cap, uint32_t & end_cap,
    timeval & begin_record, timeval & end_record
) {
    in_wrm_trans.next();
    begin_record.tv_sec = in_wrm_trans.begin_chunk_time();
    // TODO a negative time should be a time relative to end of movie
    // less than 1 year means we are given a time relatve to beginning of movie
    if (begin_cap && (begin_cap < 31536000)) {  // less than 1 year, it is relative not absolute timestamp
        // begin_capture.tv_usec is 0
        begin_cap += in_wrm_trans.begin_chunk_time();
    }
    if (end_cap && (end_cap < 31536000)) { // less than 1 year, it is relative not absolute timestamp
        // begin_capture.tv_usec is 0
        end_cap += in_wrm_trans.begin_chunk_time();
    }
    while (begin_cap >= in_wrm_trans.end_chunk_time()) {
        in_wrm_trans.next();
    }
    unsigned const result = in_wrm_trans.get_seqno();
    try {
        do {
            end_record.tv_sec = in_wrm_trans.end_chunk_time();
            in_wrm_trans.next();
        }
        while (true);
    }
    catch (const Error & e) {
        if (e.id != static_cast<unsigned>(ERR_TRANSPORT_NO_MORE_DATA)) {
            throw;
        }
    };
    return result;
}

inline void remove_file(
    InMetaSequenceTransport & in_wrm_trans, const char * hash_path, const char * infile_path
  , const char * infile_basename, const char * infile_extension, bool is_encrypted
) {
    std::vector<std::string> files;

    char infile_fullpath[2048];
    if (is_encrypted) {
        std::snprintf(infile_fullpath, sizeof(infile_fullpath), "%s%s%s", hash_path, infile_basename, infile_extension);
        files.push_back(infile_fullpath);
    }
    std::snprintf(infile_fullpath, sizeof(infile_fullpath), "%s%s%s", infile_path, infile_basename, infile_extension);
    files.push_back(infile_fullpath);

    try {
        do {
            in_wrm_trans.next();
            files.push_back(in_wrm_trans.path());
        }
        while (true);
    }
    catch (const Error & e) {
        if (e.id != ERR_TRANSPORT_NO_MORE_DATA) {
            throw;
        }
    };

    std::cout << std::endl;
    for (auto & s : iter(files.rbegin(), files.rend())) {
        unlink(s.c_str());
        std::cout << "Removed : " << s << std::endl;
    }
}

inline
static void raise_error(std::string const & output_filename, int code, const char * message) {
    if (!output_filename.length()) {
        return;
    }

    char outfile_pid[32];
    std::snprintf(outfile_pid, sizeof(outfile_pid), "%06u", unsigned(getpid()));

    char outfile_path     [1024] = {};
    char outfile_basename [1024] = {};
    char outfile_extension[1024] = {};

    canonical_path( output_filename.c_str()
                  , outfile_path
                  , sizeof(outfile_path)
                  , outfile_basename
                  , sizeof(outfile_basename)
                  , outfile_extension
                  , sizeof(outfile_extension)
                  );

    char progress_filename[4096];
    std::snprintf( progress_filename, sizeof(progress_filename), "%s%s-%s.pgs"
            , outfile_path, outfile_basename, outfile_pid);

    UpdateProgressData update_progress_data(progress_filename, 0, 0, 0, 0);

    update_progress_data.raise_error(code, message);
}

inline
static void show_metadata(FileToGraphic const & player) {
    std::cout
    << "\nWRM file version      : " << player.info_version
    << "\nWidth                 : " << player.info_width
    << "\nHeight                : " << player.info_height
    << "\nBpp                   : " << player.info_bpp
    << "\nCache 0 entries       : " << player.info_cache_0_entries
    << "\nCache 0 size          : " << player.info_cache_0_size
    << "\nCache 1 entries       : " << player.info_cache_1_entries
    << "\nCache 1 size          : " << player.info_cache_1_size
    << "\nCache 2 entries       : " << player.info_cache_2_entries
    << "\nCache 2 size          : " << player.info_cache_2_size
    << '\n';

    if (player.info_version > 3) {
        //cout << "Cache 3 entries       : " << player.info_cache_3_entries                         << endl;
        //cout << "Cache 3 size          : " << player.info_cache_3_size                            << endl;
        //cout << "Cache 4 entries       : " << player.info_cache_4_entries                         << endl;
        //cout << "Cache 4 size          : " << player.info_cache_4_size                            << endl;
        std::cout << "Compression algorithm : " << static_cast<int>(player.info_compression_algorithm) << '\n';
    }
    std::cout.flush();
}

// TODO Signals related code should not be here, all globals if any should be in main
static bool program_requested_to_shutdown = false;

inline void shutdown(int sig)
{
    LOG(LOG_INFO, "shutting down : signal %d pid=%d\n", sig, getpid());

    program_requested_to_shutdown = true;
}

inline void init_signals(void)
{
    struct sigaction sa;

    sa.sa_flags = 0;

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTERM);

    sa.sa_handler = shutdown;
    sigaction(SIGTERM, &sa, nullptr);
}


inline
static void show_statistics(FileToGraphic::Statistics const & statistics) {
    std::cout
    << "\nDstBlt                : " << statistics.DstBlt
    << "\nMultiDstBlt           : " << statistics.MultiDstBlt
    << "\nPatBlt                : " << statistics.PatBlt
    << "\nMultiPatBlt           : " << statistics.MultiPatBlt
    << "\nOpaqueRect            : " << statistics.OpaqueRect
    << "\nMultiOpaqueRect       : " << statistics.MultiOpaqueRect
    << "\nScrBlt                : " << statistics.ScrBlt
    << "\nMultiScrBlt           : " << statistics.MultiScrBlt
    << "\nMemBlt                : " << statistics.MemBlt
    << "\nMem3Blt               : " << statistics.Mem3Blt
    << "\nLineTo                : " << statistics.LineTo
    << "\nGlyphIndex            : " << statistics.GlyphIndex
    << "\nPolyline              : " << statistics.Polyline

    << "\nCacheBitmap           : " << statistics.CacheBitmap
    << "\nCacheColorTable       : " << statistics.CacheColorTable
    << "\nCacheGlyph            : " << statistics.CacheGlyph

    << "\nFrameMarker           : " << statistics.FrameMarker

    << "\nBitmapUpdate          : " << statistics.BitmapUpdate

    << "\nCachePointer          : " << statistics.CachePointer
    << "\nPointerIndex          : " << statistics.PointerIndex

    << "\ngraphics_update_chunk : " << statistics.graphics_update_chunk
    << "\nbitmap_update_chunk   : " << statistics.bitmap_update_chunk
    << "\ntimestamp_chunk       : " << statistics.timestamp_chunk
    << std::endl;
}

static int do_record( Transport & in_wrm_trans, const timeval begin_record, const timeval end_record
                    , const timeval begin_capture, const timeval end_capture, std::string const & output_filename
                    , int capture_bpp, int wrm_compression_algorithm_

                    , bool enable_rt
                    , bool no_timestamp
                    , auth_api * authentifier
                    , Inifile & ini, CryptoContext & cctx, Random & gen
                    , unsigned file_count, uint32_t order_count, uint32_t clear, unsigned zoom
                    , unsigned png_width, unsigned png_height
                    , bool show_file_metadata, bool show_statistics, uint32_t verbose
                    , bool full_video) {
    for (unsigned i = 1; i < file_count ; i++) {
        in_wrm_trans.next();
    }

    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(verbose));

    if (show_file_metadata) {
        show_metadata(player);
        std::cout << "Duration (in seconds) : " << (end_record.tv_sec - begin_record.tv_sec + 1) << std::endl;
        if (!show_statistics && !output_filename.length()) {
            return 0;
        }
    }

    player.max_order_count = order_count;

    int return_code = 0;

    if (output_filename.length()) {
//        char outfile_pid[32];
//        std::snprintf(outfile_pid, sizeof(outfile_pid), "%06u", getpid());

        char outfile_path     [1024] = {};
        char outfile_basename [1024] = {};
        char outfile_extension[1024] = {};

        canonical_path( output_filename.c_str()
                      , outfile_path
                      , sizeof(outfile_path)
                      , outfile_basename
                      , sizeof(outfile_basename)
                      , outfile_extension
                      , sizeof(outfile_extension)
                      );

        if (verbose) {
//            std::cout << "Output file path: " << outfile_path << outfile_basename << '-' << outfile_pid << outfile_extension <<
            std::cout << "Output file path: " << outfile_path << outfile_basename << outfile_extension <<
                '\n' << std::endl;
        }

        if (clear == 1) {
            clear_files_flv_meta_png(outfile_path, outfile_basename);
        }

//        if (ini.get<cfg::video::wrm_compression_algorithm>() == USE_ORIGINAL_COMPRESSION_ALGORITHM) {
//            ini.set<cfg::video::wrm_compression_algorithm>(player.info_compression_algorithm);
//        }
        ini.set<cfg::video::wrm_compression_algorithm>(
            (wrm_compression_algorithm_ == static_cast<int>(USE_ORIGINAL_COMPRESSION_ALGORITHM))
            ? player.info_compression_algorithm
            : static_cast<WrmCompressionAlgorithm>(wrm_compression_algorithm_)
        );

//        if (ini.get<cfg::video::wrm_color_depth_selection_strategy>() == USE_ORIGINAL_COLOR_DEPTH) {
//            ini.set<cfg::video::wrm_color_depth_selection_strategy>(player.info_bpp);
//        }
        if (capture_bpp == static_cast<int>(USE_ORIGINAL_COLOR_DEPTH)) {
            capture_bpp = player.info_bpp;
        }
//        ini.set<cfg::video::wrm_color_depth_selection_strategy>(capture_bpp);

        {
            ini.set<cfg::video::hash_path>(outfile_path);
            ini.set<cfg::video::record_tmp_path>(outfile_path);
            ini.set<cfg::video::record_path>(outfile_path);

            ini.set<cfg::globals::movie_path>(&output_filename[0]);
            Capture capture(
                    ((player.record_now.tv_sec > begin_capture.tv_sec) ? player.record_now : begin_capture)
                    , player.screen_rect.cx
                    , player.screen_rect.cy
                    , player.info_bpp
                    , capture_bpp
                    , enable_rt
                    , no_timestamp
                    , authentifier
                    , ini
                    , cctx
                    , gen
                    , full_video);

            if (capture.capture_png) {
                if (png_width && png_height) {
                    auto get_percent = [](unsigned target_dim, unsigned source_dim) -> unsigned {
                        return ((target_dim * 100 / source_dim) + ((target_dim * 100 % source_dim) ? 1 : 0));
                    };
                    zoom = std::max<unsigned>(
                            get_percent(png_width, player.screen_rect.cx),
                            get_percent(png_height, player.screen_rect.cy)
                        );
                    //std::cout << "zoom: " << zoom << '%' << std::endl;
                }

                capture.zoom(zoom);
            }
            player.add_consumer(&capture, &capture, &capture, &capture, &capture);

            char progress_filename[4096];
            std::snprintf( progress_filename, sizeof(progress_filename), "%s%s.pgs"
                    , outfile_path, outfile_basename);

            UpdateProgressData update_progress_data(
                progress_filename, begin_record.tv_sec, end_record.tv_sec, begin_capture.tv_sec, end_capture.tv_sec
            );

            if (update_progress_data.is_valid()) {
                try {
                    player.play(std::ref(update_progress_data), program_requested_to_shutdown);

                    if (program_requested_to_shutdown) {
                        update_progress_data.raise_error(65537, "Program requested to shutdown");
                    }
                }
                catch (Error const & e) {
                    const bool msg_with_error_id = false;
                    update_progress_data.raise_error(e.id, e.errmsg(msg_with_error_id));

                    return_code = -1;
                }
                catch (...) {
                    update_progress_data.raise_error(65536, "Unknown error");

                    return_code = -1;
                }
            }
            else {
                return_code = -1;
            }
        }

        if (!return_code && program_requested_to_shutdown) {
            clear_files_flv_meta_png(outfile_path, outfile_basename);
        }
    }
    else {
        try {
            player.play(program_requested_to_shutdown);
        }
        catch (Error const &) {
            return_code = -1;
        }
    }

    if (show_statistics && return_code == 0) {
      ::show_statistics(player.statistics);
    }

    return return_code;
}   // do_record

inline
static int do_recompress(
    CryptoContext & cctx, Random & rnd, Transport & in_wrm_trans, const timeval begin_record,
    int wrm_compression_algorithm_, std::string const & output_filename, Inifile & ini, uint32_t verbose
) {
    FileToChunk player(&in_wrm_trans, to_verbose_flags(verbose));

/*
    char outfile_path     [1024] = PNG_PATH "/"   ; // default value, actual one should come from output_filename
    char outfile_basename [1024] = "redrec_output"; // default value, actual one should come from output_filename
    char outfile_extension[1024] = ""             ; // extension is ignored for targets anyway

    canonical_path( output_filename.c_str()
                  , outfile_path
                  , sizeof(outfile_path)
                  , outfile_basename
                  , sizeof(outfile_basename)
                  , outfile_extension
                  , sizeof(outfile_extension)
                  );
*/
    std::string outfile_path;
    std::string outfile_basename;
    std::string outfile_extension;
    ParsePath(output_filename.c_str(), outfile_path, outfile_basename, outfile_extension);

    if (verbose) {
        std::cout << "Output file path: " << outfile_path << outfile_basename << outfile_extension << '\n' << std::endl;
    }

    if (recursive_create_directory(outfile_path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP, ini.get<cfg::video::capture_groupid>()) != 0) {
        std::cerr << "Failed to create directory: \"" << outfile_path << "\"" << std::endl;
    }

//    if (ini.get<cfg::video::wrm_compression_algorithm>() == USE_ORIGINAL_COMPRESSION_ALGORITHM) {
//        ini.set<cfg::video::wrm_compression_algorithm>(player.info_compression_algorithm);
//    }
    ini.set<cfg::video::wrm_compression_algorithm>(
        (wrm_compression_algorithm_ == static_cast<int>(USE_ORIGINAL_COMPRESSION_ALGORITHM))
        ? player.info_compression_algorithm
        : static_cast<WrmCompressionAlgorithm>(wrm_compression_algorithm_)
    );

    int return_code = 0;
    try {
        auto run = [&](Transport && trans) {
            {
                ChunkToFile recorder( &trans
                                    , player.info_width
                                    , player.info_height
                                    , player.info_bpp
                                    , player.info_cache_0_entries
                                    , player.info_cache_0_size
                                    , player.info_cache_1_entries
                                    , player.info_cache_1_size
                                    , player.info_cache_2_entries
                                    , player.info_cache_2_size

                                    , player.info_number_of_cache
                                    , player.info_use_waiting_list

                                    , player.info_cache_0_persistent
                                    , player.info_cache_1_persistent
                                    , player.info_cache_2_persistent

                                    , player.info_cache_3_entries
                                    , player.info_cache_3_size
                                    , player.info_cache_3_persistent
                                    , player.info_cache_4_entries
                                    , player.info_cache_4_size
                                    , player.info_cache_4_persistent

                                    , ini.get<cfg::video::wrm_compression_algorithm>());

                player.add_consumer(&recorder);

                player.play(program_requested_to_shutdown);
            }

            if (program_requested_to_shutdown) {
                trans.request_full_cleaning();
            }
        };

        if (ini.get<cfg::globals::trace_type>() == TraceType::cryptofile) {
            run(CryptoOutMetaSequenceTransport(
                cctx,
                rnd,
                outfile_path.c_str(),
                ini.get<cfg::video::hash_path>().c_str(),
                outfile_basename.c_str(),
                begin_record,
                player.info_width,
                player.info_height,
                ini.get<cfg::video::capture_groupid>()
            ));
        }
        else {
            run(OutMetaSequenceTransport(
                outfile_path.c_str(),
                ini.get<cfg::video::hash_path>().c_str(),
                outfile_basename.c_str(),
                begin_record,
                player.info_width,
                player.info_height,
                ini.get<cfg::video::capture_groupid>()
            ));
        }
    }
    catch (...) {
        return_code = -1;
    }

    return return_code;
}   // do_recompress




inline int is_encrypted_file(const char * input_filename, bool & infile_is_encrypted)
{
    infile_is_encrypted = false;
    const int fd_test = open(input_filename, O_RDONLY);
    if (fd_test != -1) {
        uint8_t data[4] = {};
        ssize_t res_test = read(fd_test, data, 4);
        const uint32_t magic = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
        if ((res_test == 4) && (magic == WABCRYPTOFILE_MAGIC)) {
            infile_is_encrypted = true;
            std::cout << "Input file is encrypted.\n";
        }
        close(fd_test);
        return 0;
    }

    return -1;
}


inline int app_recorder(
    int argc, char const * const * argv, const char * copyright_notice,
    std::string config_filename, Inifile & ini, CryptoContext & cctx, Random & rnd
) {
    openlog("redrec", LOG_CONS | LOG_PERROR, LOG_USER);

    init_signals();

    unsigned png_width  = 0;
    unsigned png_height = 0;

    std::string input_filename;
    std::string output_filename;

    bool const enable_rt = false;
    bool const no_timestamp = false;

    uint32_t    verbose            = 0;
    uint32_t    clear              = 1; // default on
    uint32_t    begin_cap          = 0;
    uint32_t    end_cap            = 0;
    uint32_t    png_limit          = 10;
    uint32_t    png_interval       = 60;
    uint32_t    wrm_frame_interval = 100;
    uint32_t    wrm_break_interval = 86400;
    uint32_t    order_count        = 0;
    unsigned    zoom               = 100;
    bool        show_file_metadata = false;
    bool        show_statistics    = false;
    bool        auto_output_file   = false;
    bool        remove_input_file  = false;
    uint32_t    flv_break_interval = 10*60;
    std::string flv_quality;
    unsigned    ocr_version = -1u;

    std::string wrm_compression_algorithm;  // output compression algorithm.
    std::string wrm_color_depth;
    std::string wrm_encryption;
    std::string png_geometry;
    std::string hash_path;
    std::string video_codec;

    program_options::options_description desc({
        {'h', "help", "produce help message"},
        {'v', "version", "show software version"},
        {'o', "output-file", &output_filename, "output base filename"},
        {'i', "input-file", &input_filename, "input base filename"},

        {'b', "begin", &begin_cap, "begin capture time (in seconds), default=none"},
        {'e', "end", &end_cap, "end capture time (in seconds), default=none"},
        {"count", &order_count, "Number of orders to execute before stopping, default=0 execute all orders"},

        {'l', "png_limit", &png_limit, "maximum number of png files to create (remove older), default=10, 0 will disable png capture"},
        {'n', "png_interval", &png_interval, "time interval between png captures, default=60 seconds"},

        {'r', "frameinterval", &wrm_frame_interval, "time between consecutive capture frames (in 100/th of seconds), default=100 one frame per second"},

        {'k', "breakinterval", &wrm_break_interval, "number of seconds between splitting wrm files in seconds(default, one wrm every day)"},

        {'p', "png", "enable png capture"},
        {'w', "wrm", "enable wrm capture"},
        {'t', "ocr", "enable ocr title bar detection"},
        {'f', "flv", "enable flv capture"},
        {'u', "full", "create full video"},
        {'c', "chunk", "chunk splitting on title bar change detection"},

        {"clear", &clear, "clear old capture files with same prefix (default on)"},
        {"verbose", &verbose, "more logs"},
        {"zoom", &zoom, "scaling factor for png capture (default 100%)"},
        {'g', "png-geometry", &png_geometry, "png capture geometry (Ex. 160x120)"},
        {'m', "meta", "show file metadata"},
        {'s', "statistics", "show statistics"},

        {'z', "compression", &wrm_compression_algorithm, "wrm compression algorithm (default=original, none, gzip, snappy)"},
        {'d', "color-depth", &wrm_color_depth,           "wrm color depth (default=original, 16, 24)"},
        {'y', "encryption",  &wrm_encryption,            "wrm encryption (default=original, enable, disable)"},

        {"auto-output-file",  "append suffix to input base filename to generate output base filename automatically"},
        {"remove-input-file", "remove input file"},

        {"config-file", &config_filename, "used an another ini file"},

        {"hash-path", &hash_path, "output hash dirname (if empty, use hash_path of ini)"},

        {'a', "flvbreakinterval", &flv_break_interval, "number of seconds between splitting flv files (by default, one flv every 10 minutes)"},

        {'q', "flv-quality", &flv_quality, "flv quality (high, medium, low)"},

        {"ocr-version", &ocr_version, "version 1 or 2"},

        {"video-codec", &video_codec, "ffmpeg video codec id (flv, mp4, etc)"},
    });

    auto options = program_options::parse_command_line(argc, argv, desc);

    if (options.count("help") > 0) {
        std::cout << copyright_notice;
        std::cout << "\n\nUsage: redrec [options]\n\n";
        std::cout << desc << "\n\n";
        return 0;
    }

    if (options.count("version") > 0) {
        std::cout << copyright_notice << std::endl << std::endl;
        return 0;
    }

    if (input_filename.empty()) {
        std::cerr << "Missing input filename : use -i filename\n\n";
        return 1;
    }

    show_file_metadata = (options.count("meta"             ) > 0);
    show_statistics    = (options.count("statistics"       ) > 0);
    auto_output_file   = (options.count("auto-output-file" ) > 0);
    remove_input_file  = (options.count("remove-input-file") > 0);

    if (!show_file_metadata && !show_statistics && !auto_output_file && output_filename.empty()) {
        std::cerr << "Missing output filename : use -o filename\n\n";
        return 1;
    }

    if (!output_filename.empty() && auto_output_file) {
        std::cerr << "Conflicting options : --output-file and --auto-output-file\n\n";
        return 1;
    }

    if ((options.count("zoom") > 0) & (options.count("png-geometry") > 0)) {
        std::cerr << "Conflicting options : --zoom and --png-geometry\n\n";
        return 1;
    }

    if (options.count("png-geometry") > 0) {
        const char * png_geometry_c = png_geometry.c_str();
        const char * separator      = strchr(png_geometry_c, 'x');
        int          png_w          = atoi(png_geometry_c);
        int          png_h          = 0;
        if (separator) {
            png_h = atoi(separator + 1);
        }
        if (!png_w || !png_h) {
            std::cerr << "Invalide png geometry\n\n";
            return 1;
        }
        png_width  = png_w;
        png_height = png_h;
        std::cout << "png-geometry: " << png_width << "x" << png_height << std::endl;
    }

    { ConfigurationLoader cfg_loader_full(ini.configuration_holder(), config_filename.c_str()); }

    if (!hash_path.empty()) {
        hash_path += '/';
        ini.set<cfg::video::hash_path>(hash_path);
    }

    int wrm_compression_algorithm_;

    if (options.count("compression") > 0) {
             if (wrm_compression_algorithm == "none") {
//            ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);
            wrm_compression_algorithm_ = static_cast<int>(WrmCompressionAlgorithm::no_compression);
        }
        else if (wrm_compression_algorithm == "gzip") {
//            ini.set<cfg::video::wrm_compression_algorithm>(1);
            wrm_compression_algorithm_ = static_cast<int>(WrmCompressionAlgorithm::gzip);
        }
        else if (wrm_compression_algorithm == "snappy") {
//            ini.set<cfg::video::wrm_compression_algorithm>(2);
            wrm_compression_algorithm_ = static_cast<int>(WrmCompressionAlgorithm::snappy);
        }
        else if (wrm_compression_algorithm == "original") {
//            ini.set<cfg::video::wrm_compression_algorithm>(USE_ORIGINAL_COMPRESSION_ALGORITHM);
            wrm_compression_algorithm_ = static_cast<int>(USE_ORIGINAL_COMPRESSION_ALGORITHM);
        }
        else {
            std::cerr << "Unknown wrm compression algorithm\n\n";
            return 1;
        }
    }
    else {
//        ini.set<cfg::video::wrm_compression_algorithm>(USE_ORIGINAL_COMPRESSION_ALGORITHM);
        wrm_compression_algorithm_ = static_cast<int>(USE_ORIGINAL_COMPRESSION_ALGORITHM);
    }

    int capture_bpp = 16;

    if (options.count("color-depth") > 0) {
             if (wrm_color_depth == "16") {
//            ini.set<cfg::video::wrm_color_depth_selection_strategy>(16);
            capture_bpp = 16;
        }
        else if (wrm_color_depth == "24") {
//            ini.set<cfg::video::wrm_color_depth_selection_strategy>(24);
            capture_bpp = 24;
        }
        else if (wrm_color_depth == "original") {
//            ini.set<cfg::video::wrm_color_depth_selection_strategy>(USE_ORIGINAL_COLOR_DEPTH);
            capture_bpp = static_cast<int>(USE_ORIGINAL_COLOR_DEPTH);
        }
        else {
            std::cerr << "Unknown wrm color depth\n\n";
            return 1;
        }
    }
    else {
//        ini.set<cfg::video::wrm_color_depth_selection_strategy>(USE_ORIGINAL_COLOR_DEPTH);
        capture_bpp = static_cast<int>(USE_ORIGINAL_COLOR_DEPTH);
    }

    ini.set<cfg::video::png_limit>(png_limit);
    ini.set<cfg::video::png_interval>(std::chrono::seconds{png_interval});
    ini.set<cfg::video::frame_interval>(std::chrono::duration<unsigned int, std::centi>{wrm_frame_interval});
    ini.set<cfg::video::break_interval>(std::chrono::seconds{wrm_break_interval});
    ini.get_ref<cfg::video::capture_flags>() &= ~(CaptureFlags::wrm | CaptureFlags::png);
    if (options.count("wrm") > 0) {
        ini.get_ref<cfg::video::capture_flags>() |= CaptureFlags::wrm;
    }
    if (options.count("png") > 0) {
        ini.get_ref<cfg::video::capture_flags>() |= CaptureFlags::png;
    }

    ini.set<cfg::globals::capture_chunk>(options.count("chunk") > 0);

    ini.set<cfg::video::flv_break_interval>(std::chrono::seconds{flv_break_interval});

    if (ini.get<cfg::globals::capture_chunk>()) {
        ini.get_ref<cfg::video::capture_flags>()
            |= CaptureFlags::flv
            |  CaptureFlags::ocr
            |  CaptureFlags::png;

        ini.get_ref<cfg::video::disable_keyboard_log>() &= ~KeyboardLogFlags::meta;

        ini.set<cfg::video::flv_break_interval>(std::chrono::minutes{10});
        ini.set<cfg::video::png_interval>(std::chrono::minutes{1});
        ini.set<cfg::video::png_limit>(0xFFFF);
        ini.set<cfg::ocr::interval>(std::chrono::seconds{1});
    }
    else {
        auto set_flag = [&](char const * opt, CaptureFlags f) {
            if (options.count(opt) > 0) {
                ini.get_ref<cfg::video::capture_flags>() |= f;
            }
            else {
                ini.get_ref<cfg::video::capture_flags>() &= ~f;
            }
        };
        set_flag("flv", CaptureFlags::flv);
        set_flag("ocr", CaptureFlags::ocr);
    }

    if (options.count("ocr-version")) {
        ini.set<cfg::ocr::version>(ocr_version == 2 ? OcrVersion::v2 : OcrVersion::v1);
    }

    //if (options.count("extract-meta-data") &&
    //    (options.count("png") || options.count("flv") || options.count("wrm") || options.count("chunk"))) {
    //    std::cerr << "Option --extract-meta-data is not compatible with options --png, --flv, --wrm or --chunk" << std::endl;
    //    return 1;
    //}

    // TODO("extract-meta-data should be independant capture type")
    ////if (options.count("extract-meta-data") && !options.count("ocr")) {
    //    std::cerr << "Option --extract-meta-data should be used with option --ocr" << std::endl;
    //    return 1;
    //}

    bool const full_video = (options.count("full") > 0);

    if (output_filename.length() && (!full_video && !bool(ini.get<cfg::video::capture_flags>()))) {
        std::cerr << "Missing target format : need --png, --ocr, --flv, --full, --wrm or --chunk" << std::endl;
        return 1;
    }

    if (options.count("flv-quality") > 0) {
            if (0 == strcmp(flv_quality.c_str(), "high")) {
            ini.set<cfg::globals::video_quality>(Level::high);
        }
        else if (0 == strcmp(flv_quality.c_str(), "low")) {
            ini.set<cfg::globals::video_quality>(Level::low);
        }
        else  if (0 == strcmp(flv_quality.c_str(), "medium")) {
            ini.set<cfg::globals::video_quality>(Level::medium);
        }
        else {
            std::cerr << "Unknown video quality" << std::endl;
            return 1;
        }
    }

    if (options.count("video-codec") > 0) {
        ini.set<cfg::globals::codec_id>(video_codec);
    }

    ini.set<cfg::video::rt_display>(bool(ini.get<cfg::video::capture_flags>() & CaptureFlags::png));

    {
        std::string directory          ;
        std::string filename           ;
        std::string extension = ".mwrm";

        ParsePath(input_filename.c_str(), directory, filename, extension);
        if (!directory.size()) {
            if (file_exist(input_filename.c_str())) {
                directory = "./";
            }
            else {
                directory = ini.get<cfg::video::record_path>().c_str();
            }
        }
        MakePath(input_filename, directory.c_str(), filename.c_str(), extension.c_str());
    }
    std::cout << "Input file is \"" << input_filename << "\".\n";

    bool infile_is_encrypted;
    if (is_encrypted_file(input_filename.c_str(), infile_is_encrypted) == -1) {
        std::cerr << "Input file is absent.\n";
        return 1;
    }

    if (options.count("encryption") > 0) {
             if (0 == strcmp(wrm_encryption.c_str(), "enable")) {
            ini.set<cfg::globals::trace_type>(TraceType::cryptofile);
        }
        else if (0 == strcmp(wrm_encryption.c_str(), "disable")) {
            ini.set<cfg::globals::trace_type>(TraceType::localfile);
        }
        else if (0 == strcmp(wrm_encryption.c_str(), "original")) {
            ini.set<cfg::globals::trace_type>(infile_is_encrypted ? TraceType::cryptofile : TraceType::localfile);
        }
        else {
            std::cerr << "Unknown wrm encryption parameter\n\n";
            return 1;
        }
    }
    else {
        ini.set<cfg::globals::trace_type>(infile_is_encrypted ? TraceType::cryptofile : TraceType::localfile);
    }

    if (infile_is_encrypted || (ini.get<cfg::globals::trace_type>() == TraceType::cryptofile)) {
        OpenSSL_add_all_digests();
    }

    bool const force_record
      = bool(ini.get<cfg::video::capture_flags>() & (CaptureFlags::flv | CaptureFlags::ocr))
      || full_video;

    std::string infile_path;
    std::string infile_basename;
    std::string infile_extension;
    ParsePath(input_filename.c_str(), infile_path, infile_basename, infile_extension);

    char infile_prefix[4096];
    std::snprintf(infile_prefix, sizeof(infile_prefix), "%s%s", infile_path.c_str(), infile_basename.c_str());

    if (auto_output_file) {
        output_filename =  infile_path;
        output_filename += infile_basename;
        output_filename += "-redrec";
        output_filename += infile_extension;

        std::cout << "\nOutput file is \"" << output_filename << "\" (autogenerated)." << std::endl;
    }
    else if (output_filename.size()) {
        std::string directory = PNG_PATH "/"; // default value, actual one should come from output_filename
        std::string filename                ;
        std::string extension = ".mwrm"     ;

        ParsePath(output_filename.c_str(), directory, filename, extension);
        MakePath(output_filename, directory.c_str(), filename.c_str(), extension.c_str());
        std::cout << "Output file is \"" << output_filename << "\".\n";
    }

    // TODO before continuing to work with input file, check if it's mwrm or wrm and use right object in both cases

    // TODO also check if it contains any wrm at all and at wich one we should start depending on input time
    // TODO if start and stop time are outside wrm, users should also be warned

    timeval  begin_record = { 0, 0 };
    timeval  end_record   = { 0, 0 };
    unsigned file_count   = 0;
    try {
        InMetaSequenceTransport in_wrm_trans_tmp(
            &cctx,
            infile_prefix,
            infile_extension.c_str(),
            infile_is_encrypted?1:0);
        file_count = get_file_count(in_wrm_trans_tmp, begin_cap, end_cap, begin_record, end_record);
    }
    catch (const Error & e) {
        if (e.id == static_cast<unsigned>(ERR_TRANSPORT_NO_MORE_DATA)) {
            std::cerr << "Asked time not found in mwrm file\n";
        }
        else {
            std::cerr << "Error: " << e.errmsg() << std::endl;
        }
        const bool msg_with_error_id = false;
        raise_error(output_filename, e.id, e.errmsg(msg_with_error_id));
        return -1;
    };

    {
        InMetaSequenceTransport trans(
            &cctx, infile_prefix,
            infile_extension.c_str(),
            infile_is_encrypted?1:0
        );

        timeval begin_capture = {0, 0};
        timeval end_capture = {0, 0};

        int result = -1;
        try {
            bool test = (
                force_record
                || bool(ini.get<cfg::video::capture_flags>() & CaptureFlags::png)
    //             || ini.get<cfg::video::wrm_color_depth_selection_strategy>() != USE_ORIGINAL_COLOR_DEPTH
                || capture_bpp != static_cast<int>(USE_ORIGINAL_COLOR_DEPTH)
                || show_file_metadata
                || show_statistics
                || file_count > 1
                || order_count
                || begin_cap != begin_record.tv_sec
                || end_cap != begin_cap);

            if (test){
                std::cout << "[A]" << std::endl;
                result = do_record(trans
                            , begin_record, end_record
                            , begin_capture, end_capture
                            , output_filename, capture_bpp
                            , wrm_compression_algorithm_
                            , enable_rt
                            , no_timestamp
                            , nullptr
                            , ini, cctx, rnd
                            , file_count, order_count, clear, zoom
                            , png_width, png_height
                            , show_file_metadata, show_statistics, verbose
                            , full_video
                            );
            }
            else {
                std::cout << "[B]" << std::endl;
                result = do_recompress(
                    cctx,
                    rnd,
                    trans,
                    begin_record,
                    wrm_compression_algorithm_,
                    output_filename,
                    ini,
                    verbose);
            }
        }
        catch (const Error & e) {
            const bool msg_with_error_id = false;
            raise_error(output_filename, e.id, e.errmsg(msg_with_error_id));
        }

        if (!result && remove_input_file) {
            InMetaSequenceTransport in_wrm_trans_tmp(
                &cctx,
                infile_prefix,
                infile_extension.c_str(),
                infile_is_encrypted?1:0);

            remove_file( in_wrm_trans_tmp, ini.get<cfg::video::hash_path>().c_str(), infile_path.c_str()
                        , infile_basename.c_str(), infile_extension.c_str()
                        , infile_is_encrypted);
        }

        std::cout << std::endl;

        return result;
    }
}

extern "C" {
    __attribute__((__visibility__("default")))
    int recmemcpy(char * dest, char * source, int len)
    {
        ::memcpy(dest, source, static_cast<size_t>(len));
        return 0;
    }

    __attribute__((__visibility__("default")))
    int do_main(int argc, char const ** argv,
            get_hmac_key_prototype * hmac_fn,
            get_trace_key_prototype * trace_fn)
    {

        int arg_used = 0;


          int command = 0;
//        int command = ends_with(argv[arg_used], {"recorder.py", "verifier.py", "decoder.py"});
//        if (command){
//            command = command - 1;
//        }
//        // default command is redrec;

        if (argc > arg_used){
            command = in(argv[arg_used+1], {"redrec", "redver", "reddec"});
            if (command){
                command = command - 1;
                arg_used++;
            }
            // default command is previous one;
        }

        Inifile ini;
        ini.set<cfg::debug::config>(false);
        auto config_filename = CFG_PATH "/" RDPPROXY_INI;

        UdevRandom rnd;
        CryptoContext cctx;
        cctx.set_get_hmac_key_cb(hmac_fn);
        cctx.set_get_trace_key_cb(trace_fn);

        uint8_t tmp[32] = {};
        for (auto a : {0, 1}) {
            if (argc < arg_used + 1){
                break;
            }
            auto k = argv[arg_used+1];
            if (strlen(k) != 64){
                break;
            }
            int c1 = -1;
            int c2 = -1;
            for (unsigned i = 0; i < 32; ++i) {
                auto char_to_hex = [](char c){
                    auto in = [&c](char left, char right) { return left <= c && c <= right; };
                    return
                        in('0', '9') ? c-'0'
                        : in('a', 'f') ? 10 + c-'a'
                        : in('A', 'F') ? 10 + c-'A'
                        : -1;
                };
                c1 = char_to_hex(k[i*2]);
                c2 = char_to_hex(k[i*2+1]);
                if (c1 == -1 or c2 == -1){
                    break;
                }
                tmp[i] = c1 << 4 | c2;
            }
            // if any character not an hexa digit, ignore option
            if (c1 == -1 or c2 == -1){
                break;
            }
            if (a == 0){
                cctx.set_hmac_key(tmp);
            }
            else {
                cctx.set_master_key(tmp);
            }
            arg_used++;
        }

        argv += arg_used;
        argc -= arg_used;
        int res = -1;

        const char * copyright_notice = "ReDemPtion " VERSION ".\n"
            "Copyright (C) Wallix 2010-2016.\n"
            "Christophe Grosjean, Jonathan Poelen, Raphael Zhou.";

        switch (command){
        case 0: // RECorder
            try {
                res = app_recorder(
                    argc, argv
                  , copyright_notice
                  , config_filename
                  , ini
                  , cctx
                  , rnd);
            } catch (const Error & e) {
                std::printf("decrypt failed: with id=%d\n", e.id);
            }
        break;
        case 1: // VERifier
            ini.set<cfg::debug::config>(false);
            try {
                openlog("verifier", LOG_CONS | LOG_PERROR, LOG_USER);

                std::string hash_path      = ini.get<cfg::video::hash_path>().c_str()  ;
                std::string mwrm_path      = ini.get<cfg::video::record_path>().c_str();
                std::string input_filename;
                bool        quick_check    = false;
                bool      ignore_stat_info = false;
                bool      update_stat_info = false;
                uint32_t    verbose        = 0;

                program_options::options_description desc({
                    {'h', "help",    "produce help message"},
                    {'v', "version", "show software version"},
                    {'q', "quick",   "quick check only"},
                    {'s', "hash-path",  &hash_path,         "hash file path"       },
                    {'m', "mwrm-path",  &mwrm_path,         "mwrm file path"       },
                    {'i', "input-file", &input_filename,    "input mwrm file name" },
                    {'S', "ignore-stat-info", "ignore stat info data mismatch" },
                    {'U', "update-stat-info", "update stat info data mismatch "
                                              "(only if not checksum and no encrypted)" },
                    {"verbose",         &verbose,           "more logs"            },
                })
                ;

                auto options = program_options::parse_command_line(argc, argv, desc);

                if (options.count("help") > 0) {
                    std::cout << copyright_notice;
                    std::cout << "Usage: redver [options]\n\n";
                    std::cout << desc << std::endl;
                    return 0;
                }

                if (options.count("version") > 0) {
                    std::cout << copyright_notice;
                    return 0;
                }

                if (options.count("quick") > 0) {
                    quick_check = true;
                }

                if (options.count("ignore-stat-info") > 0) {
                    ignore_stat_info = true;
                }

                if (options.count("update-stat-info") > 0) {
                    update_stat_info = true;
                }

                if (hash_path.c_str()[0] == 0) {
                    std::cerr << "Missing hash-path : use -h path\n\n";
                    return 1;
                }

                if (mwrm_path.c_str()[0] == 0) {
                    std::cerr << "Missing mwrm-path : use -m path\n\n";
                    return 1;
                }

                if (input_filename.c_str()[0] == 0) {
                    std::cerr << "Missing input mwrm file name : use -i filename\n\n";
                    return 1;
                }

                {
                    char temp_path[1024]     = {};
                    char temp_basename[1024] = {};
                    char temp_extension[256] = {};

                    canonical_path(input_filename.c_str(), temp_path, sizeof(temp_path), temp_basename, sizeof(temp_basename), temp_extension, sizeof(temp_extension));

                    if (strlen(temp_path) > 0) {
                        mwrm_path       = temp_path;
                        input_filename  = temp_basename;
                        input_filename += temp_extension;
                    }
                    if (mwrm_path.back() != '/'){
                        mwrm_path.push_back('/');
                    }
                    if (hash_path.back() != '/'){
                        hash_path.push_back('/');
                    }

                }
                std::cout << "Input file is \"" << mwrm_path << input_filename << "\".\n";

                if (verbose) {
                    LOG(LOG_INFO, "hash_path=\"%s\"", hash_path.c_str());
                    LOG(LOG_INFO, "mwrm_path=\"%s\"", mwrm_path.c_str());
                    LOG(LOG_INFO, "file_name=\"%s\"", input_filename.c_str());
                }

                OpenSSL_add_all_digests();

                res = check_encrypted_or_checksumed(
                    input_filename, mwrm_path, hash_path,
                    quick_check, ignore_stat_info, update_stat_info, verbose, cctx
                );
                std::puts(res == 0 ? "verify ok\n" : "verify failed\n");
            } catch (const Error & e) {
                std::printf("verify failed: with id=%d\n", e.id);
            }
        break;
        default: // DECrypter
            try {
                openlog("decrypter", LOG_CONS | LOG_PERROR, LOG_USER);

                std::string input_filename;
                std::string output_filename;

                uint32_t verbose = 0;

                program_options::options_description desc({
                    {'h', "help",    "produce help message"},
                    {'v', "version", "show software version"},

                    {'o', "output-file", &output_filename, "output base filename"},
                    {'i', "input-file",  &input_filename,  "input base filename"},
                    {"verbose",          &verbose,         "more logs"}
                });

                auto options = program_options::parse_command_line(argc, argv, desc);

                if (options.count("help") > 0) {
                    std::cout << copyright_notice << "\n\n";
                    std::cout << "Usage: redrec [options]\n\n";
                    std::cout << desc << std::endl;
                    return 0;
                }

                if (options.count("version") > 0) {
                    std::cout << copyright_notice << std::endl << std::endl;
                    return 0;
                }

                if (input_filename.empty()) {
                    std::cerr << "Missing input filename : use -i filename\n\n";
                    return -1;
                }

                if (output_filename.empty()) {
                    std::cerr << "Missing output filename : use -o filename\n\n";
                    return -1;
                }

                int fd  = open(input_filename.c_str(), O_RDONLY);
                if (fd == -1) {
                    std::cerr << "can't open file " << input_filename << "\n\n";
                    std::puts("decrypt failed\n");
                    return -1;
                }

                OpenSSL_add_all_digests();

                size_t base_len = 0;
                const uint8_t * base = reinterpret_cast<const uint8_t *>(
                                basename_len(input_filename.c_str(), base_len));

                InFilenameTransport in_t(cctx, fd, base, base_len);
                if (!in_t.is_encrypted()){
                    std::cout << "Input file is unencrypted.\n\n";
                    return 0;
                }

                const int fd1 = open(output_filename.c_str(), O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);
                if (fd1 != -1) {
                    OutFileTransport out_t(fd1);

                    char mem[4096];
                    char *buf;

                    try {
                        while (1) {
                            buf = mem;
                            in_t.recv(&buf, sizeof(mem));
                            out_t.send(mem, buf-mem);
                        }
                    }
                    catch (Error const & e) {
                        if (e.id != ERR_TRANSPORT_NO_MORE_DATA) {
                            std::cerr << "Exception code: " << e.id << std::endl << std::endl;
                        }
                        else {
                            out_t.send(mem, buf - mem);
                        }
                    }
                    close(fd);
                }
                else {
                    std::cerr << strerror(errno) << std::endl << std::endl;
                }
                std::puts("decrypt ok\n");
                return 0;
            } catch (const Error & e) {
                std::printf("decrypt failed: with id=%d\n", e.id);
            }
        break;
        }
        return res;
    }
}
