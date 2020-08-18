/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013-2017
   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen

   redver video verifier program
*/

#include "lib/do_recorder.hpp"

#include "system/scoped_crypto_init.hpp"
#include "program_options/program_options.hpp"

#include "capture/file_to_graphic.hpp"
#include "capture/params_from_ini.hpp"
#include "capture/wrm_meta_chunk.hpp"

#include "capture/capture_params.hpp"
#include "capture/drawable_params.hpp"
#include "capture/full_video_params.hpp"
#include "capture/kbd_log_params.hpp"
#include "capture/meta_params.hpp"
#include "capture/ocr_params.hpp"
#include "capture/pattern_params.hpp"
#include "capture/png_params.hpp"
#include "capture/sequenced_video_params.hpp"
#include "capture/video_params.hpp"
#include "capture/wrm_params.hpp"

#include "capture/capture.hpp"
#include "capture/cryptofile.hpp"
#include "capture/save_state_chunk.hpp"

#include "configs/config.hpp"

#include "core/RDP/RDPDrawable.hpp"

#include "transport/crypto_transport.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "transport/file_transport.hpp"
#include "transport/out_meta_sequence_transport.hpp"

#include "utils/compression_transport_builder.hpp"
#include "utils/fileutils.hpp"
#include "utils/genfstat.hpp"
#include "utils/genrandom.hpp"
#include "utils/log.hpp"
#include "utils/recording_progress.hpp"
#include "utils/redemption_info_version.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/iter.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "utils/sugar/unique_fd.hpp"


#include <string>
#include <vector>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <optional>

// opendir/closedir
#include <sys/types.h>
#include <dirent.h>

#include <csignal>


enum {
    USE_ORIGINAL_COMPRESSION_ALGORITHM = 0xFFFFFFFF
};

enum {
    USE_ORIGINAL_COLOR_DEPTH           = 0xFFFFFFFF
};


void clear_files_flv_meta_png(const char * path, const char * prefix)
{
    DIR * d{opendir(path)};
    if (!d){
        LOG(LOG_WARNING, "Failed to open directory %s [%d: %s]", path, errno, strerror(errno));
        return;
    }
    SCOPE_EXIT(closedir(d));

    char buffer[8192];
    size_t path_len = strlen(path);
    size_t prefix_len = strlen(prefix);
    size_t file_len = 1024;
    if (file_len + path_len + 1 > sizeof(buffer)) {
        LOG(LOG_WARNING, "Path len %zu > %zu", file_len + path_len + 1, sizeof(buffer));
        return;
    }
    strncpy(buffer, path, file_len + path_len + 1);
    if (buffer[path_len] != '/'){
        buffer[path_len] = '/'; path_len++; buffer[path_len] = 0;
    }

    while (struct dirent * result = readdir(d)) {
        if ((0 == strcmp(result->d_name, ".")) || (0 == strcmp(result->d_name, ".."))){
            continue;
        }

        if (0 != strncmp(result->d_name, prefix, prefix_len)){
            continue;
        }

        strncpy(buffer + path_len, result->d_name, file_len);
        size_t const name_len = strlen(result->d_name);
        const char * eob = buffer + path_len + name_len;
        const bool extension = ((name_len > 4) && (eob[-4] == '.')
            && ( ((eob[-3] == 'f') && (eob[-2] == 'l') && (eob[-1] == 'v'))
                || ((eob[-3] == 'p') && (eob[-2] == 'n') && (eob[-1] == 'g'))
                || ((eob[-3] == 'p') && (eob[-2] == 'g') && (eob[-1] == 's')) ))
            || ((name_len > 5) && (eob[-5] == '.')
                && (eob[-4] == 'm') && (eob[-3] == 'e') && (eob[-2] == 't') && (eob[-1] == 'a'))
            ;

        if (!extension){
            continue;
        }

        struct stat st;
        if (stat(buffer, &st) < 0){
            LOG(LOG_WARNING, "Failed to read file %s [%d: %s]", buffer, errno, strerror(errno));
            continue;
        }
        if (unlink(buffer) < 0){
            LOG(LOG_WARNING, "Failed to remove file %s [%d: %s]", buffer, errno, strerror(errno));
        }
    }
}


class ChunkToFile
{
    CompressionOutTransportBuilder compression_bullder;
    Transport & trans_target;
    Transport & trans;

    const uint8_t wrm_format_version;

    uint16_t info_version = 0;

public:
    ChunkToFile(
        Transport * trans, WrmMetaChunk info,
        WrmCompressionAlgorithm wrm_compression_algorithm)
    : compression_bullder(*trans, wrm_compression_algorithm)
    , trans_target(*trans)
    , trans(this->compression_bullder.get())
    , wrm_format_version(bool(this->compression_bullder.get_algorithm()) ? 4 : 3)
    {
        if (wrm_compression_algorithm != this->compression_bullder.get_algorithm()) {
            LOG( LOG_WARNING, "compression algorithm %u not fount. Compression disable."
               , static_cast<unsigned>(wrm_compression_algorithm));
        }

        info.version = this->wrm_format_version;
        info.compression_algorithm = wrm_compression_algorithm;
        info.send(this->trans_target);
    }

public:
    void chunk(WrmChunkType chunk_type, uint16_t chunk_count, InStream stream) /*override*/
    {
        switch (chunk_type)
        {
        case WrmChunkType::META_FILE:
            {
                WrmMetaChunk meta;
                meta.receive(stream);

                this->info_version = meta.version;
                meta.compression_algorithm = this->compression_bullder.get_algorithm();

                meta.send(this->trans_target);
            }
            break;

        case WrmChunkType::SAVE_STATE:
            {
                StateChunk sc;
                SaveStateChunk ssc;

                ssc.recv(stream, sc, this->info_version);

                StaticOutStream<65536> payload;

                ssc.send(payload, sc);

                send_wrm_chunk(this->trans, WrmChunkType::SAVE_STATE, payload.get_offset(), chunk_count);
                this->trans.send(payload.get_produced_bytes());
            }
            break;

        case WrmChunkType::RESET_CHUNK:
            {
                send_wrm_chunk(this->trans, WrmChunkType::RESET_CHUNK, 0, 1);
                this->trans.next();
            }
            break;

        case WrmChunkType::TIMESTAMP:
            {
                timeval record_now = stream.in_timeval_from_uint64le_usec();
                this->trans_target.timestamp(record_now);
            }
            REDEMPTION_CXX_FALLTHROUGH;
        default:
            {
                send_wrm_chunk(this->trans, chunk_type, stream.get_capacity(), chunk_count);
                this->trans.send(stream.get_data(), stream.get_capacity());
            }
            break;
        }
    }
};


class FileToChunk
{
    unsigned char stream_buf[65536];
    InStream stream;

    CompressionInTransportBuilder compression_builder;

    Transport * trans_source;
    Transport * trans;

    // variables used to read batch of orders "chunks"
    uint32_t chunk_size = 0;
    WrmChunkType chunk_type = WrmChunkType::RDP_UPDATE_ORDERS;
    uint16_t chunk_count = 0;

    ChunkToFile * consumer = nullptr;

public:
    timeval record_now;

    bool meta_ok = false;

    WrmMetaChunk info;

    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        end_of_transport = 1,
    };

    FileToChunk(Transport * trans, Verbose verbose)
        : stream(this->stream_buf)
        , compression_builder(*trans, WrmCompressionAlgorithm::no_compression)
        , trans_source(trans)
        , trans(trans)
        , verbose(verbose)
    {
        while (this->next_chunk()) {
            this->interpret_chunk();
            if (this->meta_ok) {
                break;
            }
        }
    }

    void set_consumer(ChunkToFile & chunk_device) {
        assert(!this->consumer);
        this->consumer = &chunk_device;
    }

    void play(bool const & requested_to_stop) {
        while (!requested_to_stop && this->next_chunk()) {
            this->interpret_chunk();
            if (this->consumer) {
                this->consumer->chunk(this->chunk_type, this->chunk_count, this->stream.clone());
            }
        }
    }

private:
    bool next_chunk() {
        try {
            {
                auto const buf_sz = WRM_HEADER_SIZE;
                unsigned char buf[buf_sz];
                this->trans->recv_boom(buf, buf_sz);
                InStream header(buf);
                this->chunk_type  = safe_cast<WrmChunkType>(header.in_uint16_le());
                this->chunk_size  = header.in_uint32_le();
                this->chunk_count = header.in_uint16_le();
            }

            if (this->chunk_size > 65536) {
                LOG(LOG_INFO,"chunk_size (%u) > 65536", this->chunk_size);
                return false;
            }
            this->stream = InStream({this->stream_buf, 0});   // empty stream
            auto const ssize = this->chunk_size - WRM_HEADER_SIZE;
            if (ssize > 0) {
                auto const size = size_t(ssize);
                this->trans->recv_boom(this->stream_buf, size);
                this->stream = InStream({this->stream_buf, size});
            }
        }
        catch (Error const & e) {
            if (e.id == ERR_TRANSPORT_OPEN_FAILED) {
                throw;
            }

            LOG_IF(bool(this->verbose), LOG_INFO, "receive error %u : end of transport", e.id);
            // receive error, end of transport
            return false;
        }

        return true;
    }

    void interpret_chunk() {
        switch (safe_cast<WrmChunkType>(this->chunk_type)) {
        case WrmChunkType::META_FILE:
            this->info.receive(this->stream);
            this->trans = &this->compression_builder.reset(
                *this->trans_source, this->info.compression_algorithm
            );

            this->stream.rewind();
            this->meta_ok = true;
            break;
        case WrmChunkType::RESET_CHUNK:
            this->info.compression_algorithm = WrmCompressionAlgorithm::no_compression;

            this->trans = this->trans_source;
            break;
        default :;
        }
    }   // void interpret_chunk()
};


inline
static int do_recompress(
    CryptoContext & cctx, Random & rnd, Fstat & fstat, Transport & in_wrm_trans, const timeval begin_record,
    bool & program_requested_to_shutdown,
    int wrm_compression_algorithm_, std::string const & output_filename, Inifile & ini, uint32_t verbose
) {
    FileToChunk player(&in_wrm_trans, to_verbose_flags(verbose));
    auto outfile = ParsePath(output_filename);

    if (verbose) {
        std::cout << "Output file path: " << outfile.directory << outfile.basename << outfile.extension << '\n' << std::endl;
    }

    if (recursive_create_directory(outfile.directory.c_str(), S_IRWXU | S_IRGRP | S_IXGRP, ini.get<cfg::video::capture_groupid>()) != 0) {
        std::cerr << "Failed to create directory: \"" << outfile.directory << "\"" << std::endl;
    }

//    if (ini.get<cfg::video::wrm_compression_algorithm>() == USE_ORIGINAL_COMPRESSION_ALGORITHM) {
//        ini.set<cfg::video::wrm_compression_algorithm>(player.info_compression_algorithm);
//    }
    ini.set<cfg::video::wrm_compression_algorithm>(
        (wrm_compression_algorithm_ == static_cast<int>(USE_ORIGINAL_COMPRESSION_ALGORITHM))
        ? player.info.compression_algorithm
        : static_cast<WrmCompressionAlgorithm>(wrm_compression_algorithm_)
    );

    int return_code = 0;
    try {
        CryptoContext cctx_no_crypto;

        OutMetaSequenceTransport trans(
            ini.get<cfg::globals::trace_type>() == TraceType::cryptofile ? cctx : cctx_no_crypto,
            rnd,
            fstat,
            outfile.directory.c_str(),
            ini.get<cfg::video::hash_path>().c_str(),
            outfile.basename.c_str(),
            begin_record,
            player.info.width,
            player.info.height,
            ini.get<cfg::video::capture_groupid>(),
            nullptr,
            -1
        );
        {
            ChunkToFile recorder(
                &trans, player.info, ini.get<cfg::video::wrm_compression_algorithm>());

            player.set_consumer(recorder);

            player.play(program_requested_to_shutdown);
        }

        //if (program_requested_to_shutdown) {
        //    trans.request_full_cleaning();
        //}
    }
    catch (...) {
        return_code = -1;
    }

    return return_code;
}   // do_recompress


using EncryptionMode = InCryptoTransport::EncryptionMode;

inline void load_hash(
    MetaLine & hash_line,
    const std::string & full_hash_path, const std::string & input_filename,
    WrmVersion infile_version, bool infile_is_checksumed,
    CryptoContext & cctx, Fstat & fstat, bool infile_is_encrypted, int verbose
) {
    InCryptoTransport in_hash_fb(cctx, infile_is_encrypted ? EncryptionMode::Encrypted : EncryptionMode::NotEncrypted, fstat);

    try {
        in_hash_fb.open(full_hash_path.c_str());
    }
    catch (Error const &) {
        LOG(LOG_ERR, "Open load_hash failed");
        throw Error(ERR_TRANSPORT_OPEN_FAILED);
    }

    LOG_IF(verbose, LOG_INFO, "%s", infile_version == WrmVersion::v1 ? "Hash data v1" : "Hash data v2 or higher");
    MwrmReader reader(in_hash_fb);
    reader.set_header({infile_version, infile_is_checksumed});
    reader.read_meta_hash_line(hash_line);
    if (input_filename != hash_line.filename) {
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }
}

static inline bool meta_line_stat_equal_stat(MetaLine const & metadata, struct stat64 const & sb)
{
    return
//           metadata.dev == sb.st_dev
//        && metadata.ino == sb.st_ino
//        &&
           metadata.mode == sb.st_mode
        && metadata.uid == sb.st_uid
        && metadata.gid == sb.st_gid
        && metadata.mtime == sb.st_mtime
//        && metadata.ctime == sb.st_ctime
        && metadata.size == sb.st_size;
}

struct out_is_mismatch
{
    bool & is_mismatch;
};

static inline int check_file(const std::string & filename, const MetaLine & metadata,
                      bool quick, bool has_checksum, bool ignore_stat_info,
                      uint8_t const (&hmac_key)[HMAC_KEY_LENGTH], bool update_stat_info, out_is_mismatch has_mismatch_stat)
{
    has_mismatch_stat.is_mismatch = false;
    struct stat64 sb {};
    if (lstat64(filename.c_str(), &sb) < 0){
        std::cerr << "File \"" << filename << "\" is invalid! (can't stat file)\n" << std::endl;
        return false;
    }

    if (has_checksum){
        if (sb.st_size != metadata.size){
            std::cerr << "File \"" << filename << "\" is invalid! (size mismatch)\n" << std::endl;
            return false;
        }

        InCryptoTransport::HASH hash;
        bool read_is_ok = quick
            ? InCryptoTransport::read_qhash(filename.c_str(), hmac_key, hash)
            : InCryptoTransport::read_fhash(filename.c_str(), hmac_key, hash);
        if (!read_is_ok) {
            std::cerr << "Error reading file \"" << filename << "\"\n" << std::endl;
            return false;
        }

        if (0 != memcmp(hash.hash, quick?metadata.hash1:metadata.hash2, MD_HASH::DIGEST_LENGTH)){
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

class OutFilenameCp
{
    OutFileTransport file;
    char const * filename;

public:
    OutFilenameCp(const char * filename, mode_t mode)
      : file(unique_fd{::open(filename, O_WRONLY | O_CREAT, mode)})
      , filename(filename)
    {}

    bool rename_to(const char * new_filename) noexcept
    {
        if (!rename(this->filename, new_filename)) {
            this->filename = nullptr;
            return true;
        }
        return false;
    }

    ~OutFilenameCp()
    {
        if (this->filename) {
            remove(this->filename);
        }
    }

    void write(chars_view data)
    {
        this->file.send(data);
    }

    [[nodiscard]] bool is_open() const noexcept
    {
        return -1 != this->file.get_fd();
    }
};

static inline int check_encrypted_or_checksumed(
    std::string const & input_filename,
    std::string const & mwrm_path,
    std::string const & hash_path,
    bool quick_check,
    bool ignore_stat_info,
    bool update_stat_info,
    CryptoContext & cctx,
    Fstat & fstat,
    uint32_t verbose
) {
    std::string const full_mwrm_filename = mwrm_path + input_filename;

    InCryptoTransport ibuf(cctx, EncryptionMode::Auto, fstat);

    try {
        ibuf.open(full_mwrm_filename.c_str());
    }
    catch (Error const&) {
        LOG(LOG_INFO, "ibuf.open error");
        throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
    }

    // now force encryption for sub files
    bool const infile_is_encrypted = ibuf.is_encrypted();

    MwrmReader reader(ibuf);
    reader.read_meta_headers();

    // if we have version 1 header, ignore stat info
    ignore_stat_info |= (reader.get_header().version == WrmVersion::v1);
    // if we have version >1 header and not checksum, update stat info
    update_stat_info &= (int(reader.get_header().version) > int(WrmVersion::v1)) & !reader.get_header().has_checksum & !ibuf.is_encrypted();
    ignore_stat_info |= update_stat_info;

    /*****************
    * Load file hash *
    *****************/
    LOG_IF(verbose, LOG_INFO, "Load file hash. Is encrypted %d. Is checksumed: %d",
        infile_is_encrypted, reader.get_header().has_checksum);
    MetaLine hash_line {};

    std::string const full_hash_path = hash_path + input_filename;

    // if reading hash fails
    try {
        load_hash(hash_line, full_hash_path, input_filename, reader.get_header().version, reader.get_header().has_checksum, cctx, fstat, infile_is_encrypted, verbose);
    }
    catch (...) {
        std::cerr << "Cannot read hash file: \"" << full_hash_path << "\"\n" << std::endl;
        // this is an error because checksum comes from hash file
        // and extended stat info also comes from hash file
        // if we can't read hash files we are in troubles
        if (reader.get_header().has_checksum || !ignore_stat_info){
            return 1;
        }
    }

    bool has_mismatch_stat_hash = false;

    /******************
    * Check mwrm file *
    ******************/
    if (!check_file(
        full_mwrm_filename, hash_line, quick_check, reader.get_header().has_checksum,
        ignore_stat_info, cctx.get_hmac_key(),
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


    MetaLine meta_line_wrm;

    while (Transport::Read::Ok == reader.read_meta_line(meta_line_wrm)) {
        size_t tmp_wrm_filename_len = 0;
        const char * tmp_wrm_filename = basename_len(meta_line_wrm.filename, tmp_wrm_filename_len);
        std::string const meta_line_wrm_filename = std::string(tmp_wrm_filename, tmp_wrm_filename_len);
        std::string const full_part_filename = mwrm_path + meta_line_wrm_filename;
        // LOG(LOG_INFO, "checking part %s", full_part_filename);

        bool has_mismatch_stat_mwrm = false;
        if (!check_file(
            full_part_filename, meta_line_wrm, quick_check, reader.get_header().has_checksum,
            ignore_stat_info, cctx.get_hmac_key(),
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
    if (!wrm_stat_is_ok) {
        LOG_IF(verbose, LOG_INFO, "%s", "Update mwrm file");

        has_mismatch_stat_hash = true;

        auto const str_full_mwrm_filename_tmp = full_mwrm_filename + ".tmp";
        auto * const full_mwrm_filename_tmp = str_full_mwrm_filename_tmp.c_str();

        OutFilenameCp mwrm_file_cp(
            full_mwrm_filename_tmp, S_IRUSR | S_IRGRP | S_IWUSR);
        if (not mwrm_file_cp.is_open()) {
            LOG(LOG_ERR, "Failed to open meta file %s", full_mwrm_filename_tmp);
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
        }

        if (chmod(full_mwrm_filename_tmp, S_IRUSR | S_IRGRP) == -1) {
            LOG( LOG_ERR, "can't set file %s mod to %s : %s [%d]"
                , full_mwrm_filename_tmp
                , "u+r, g+r"
                , strerror(errno), errno);
            return 1;
        }

        // copy mwrm headers
        {
            MwrmWriterBuf mwrm_file_buf;
            mwrm_file_buf.write_header(reader.get_header());
            mwrm_file_cp.write(mwrm_file_buf.buffer());
        }

        for (MetaLine2CtxForRewriteStat & ctx : meta_line_ctx_list) {
            struct stat sb;
            if (lstat(ctx.wrm_filename.c_str(), &sb) < 0) {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, 0);
            }

            MwrmWriterBuf mwrm_file_buf;
            MwrmWriterBuf::HashArray dummy_hash;
            mwrm_file_buf.write_line(
                ctx.filename.c_str(), sb,
                ctx.start_time, ctx.stop_time,
                false, dummy_hash, dummy_hash);
            mwrm_file_cp.write(mwrm_file_buf.buffer());
        }

        if (not mwrm_file_cp.rename_to(full_mwrm_filename.c_str())) {
            std::cerr << strerror(errno) << std::endl;
            return 1;
        }

        LOG_IF(verbose, LOG_INFO, "%s", "Update mwrm file, done");
    }

    if (has_mismatch_stat_hash) {
        LOG_IF(verbose, LOG_INFO, "%s", "Update hash file");

        auto const full_hash_path_tmp = full_hash_path + ".tmp";
        auto * const hash_filename = full_hash_path_tmp.c_str();
        auto * const meta_filename = full_mwrm_filename.c_str();

        char filename[2048] = {};

        auto meta_output = ParsePath(meta_filename);

        snprintf(filename, sizeof(filename), "%s%s", meta_output.basename.c_str(), meta_output.extension.c_str());

        OutFilenameCp hash_file_cp(hash_filename, S_IRUSR | S_IRGRP);
        if (hash_file_cp.is_open()) {
            struct stat stat;
            int err = ::stat(meta_filename, &stat);
            if (!err) {
                MwrmWriterBuf mwrm_file_buf;
                MwrmWriterBuf::HashArray dummy_hash;
                mwrm_file_buf.write_hash_file(
                    filename, stat,
                    false, dummy_hash, dummy_hash);

                try {
                    hash_file_cp.write(mwrm_file_buf.buffer());
                }
                catch (Error const&) {
                    err = -1;
                }
            }
            if (err) {
                LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]", hash_filename, err);
                return 1;
            }
        }
        else {
            int e = errno;
            LOG(LOG_ERR, "Open to transport failed: code=%d", e);
            errno = e;
            return 1;
        }

        if (not hash_file_cp.rename_to(full_hash_path.c_str())) {
            std::cerr << strerror(errno) << std::endl;
            return 1;
        }

        LOG_IF(verbose, LOG_INFO, "%s", "Update hash file, done");
    }

    std::cout << "No error detected during the data verification.\n" << std::endl;
    return 0;
}


inline unsigned get_file_count(
    InMetaSequenceTransport & in_wrm_trans,
    int64_t & begin_cap, int64_t & end_cap,
    timeval & begin_record, timeval & end_record,
    Fstat & fstat, uint64_t & total_wrm_file_len, unsigned & count_wrm_file
) {
    auto next_wrm = [&]{
        struct stat stat {};
        in_wrm_trans.next();
        fstat.stat(in_wrm_trans.path(), stat);
        total_wrm_file_len += stat.st_size;
        ++count_wrm_file;
    };

    next_wrm();
    begin_record.tv_sec = in_wrm_trans.begin_chunk_time();
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
        next_wrm();
    }
    unsigned const result = in_wrm_trans.get_seqno(); /*NOLINT(clang-analyzer-deadcode.DeadStores)*/
    try {
        do {
            end_record.tv_sec = in_wrm_trans.end_chunk_time();
            next_wrm();
        }
        while (true);
    }
    catch (const Error & e) {
        if (e.id != ERR_TRANSPORT_NO_MORE_DATA) {
            throw;
        }
    }
    return result;
}

inline void remove_file(
    InMetaSequenceTransport & in_wrm_trans, const char * hash_path, const char * infile_path
  , const char * input_filename, const char * infile_extension, bool is_encrypted
) {
    std::vector<std::string> files;

    if (is_encrypted) {
        files.emplace_back(str_concat(hash_path, input_filename, infile_extension));
    }
    files.emplace_back(str_concat(infile_path, input_filename, infile_extension));

    try {
        do {
            in_wrm_trans.next();
            files.emplace_back(in_wrm_trans.path());
        }
        while (true);
    }
    catch (const Error & e) {
        if (e.id != ERR_TRANSPORT_NO_MORE_DATA) {
            throw;
        }
    }

    std::cout << std::endl;
    for (auto & s : iter(files.rbegin(), files.rend())) {
        unlink(s.c_str());
        std::cout << "Removed : " << s << std::endl;
    }
}

inline
static void raise_error(
    UpdateProgressData::Format pgs_format,
    std::string const& output_filename,
    int code, const char * message)
{
    if (output_filename.empty()) {
        return;
    }

    auto outfile = ParsePath(output_filename);

    char progress_filename[4096];
    std::snprintf( progress_filename, sizeof(progress_filename), "%s%s.pgs"
            , outfile.directory.c_str(), outfile.basename.c_str());

    (void)unlink(progress_filename);
    UpdateProgressData update_progress_data(pgs_format, progress_filename, 0, 0, 0, 0);

    update_progress_data.raise_error(code, message);
}

static void show_metadata(FileToGraphic const & player) {
    std::cout
    << "\nWRM file version      : " << static_cast<int>(player.info.version)
    << "\nWidth                 : " << player.info.width
    << "\nHeight                : " << player.info.height
    << "\nBpp                   : " << static_cast<int>(player.info.bpp)
    << "\nCache 0 entries       : " << player.info.cache_0_entries
    << "\nCache 0 size          : " << player.info.cache_0_size
    << "\nCache 1 entries       : " << player.info.cache_1_entries
    << "\nCache 1 size          : " << player.info.cache_1_size
    << "\nCache 2 entries       : " << player.info.cache_2_entries
    << "\nCache 2 size          : " << player.info.cache_2_size
    << '\n';

    if (player.info.version > 3) {
        //cout << "Cache 3 entries       : " << player.info_cache_3_entries                         << endl;
        //cout << "Cache 3 size          : " << player.info_cache_3_size                            << endl;
        //cout << "Cache 4 entries       : " << player.info_cache_4_entries                         << endl;
        //cout << "Cache 4 size          : " << player.info_cache_4_size                            << endl;
        std::cout << "Compression algorithm : " << static_cast<int>(player.info.compression_algorithm) << '\n';
    }
    if (!player.max_image_frame_rect.isempty()) {
        std::cout << "Max image frame rect  : (" <<
            player.max_image_frame_rect.x << ", "  <<
            player.max_image_frame_rect.y << ", "  <<
            player.max_image_frame_rect.cx << ", "  <<
            player.max_image_frame_rect.cy << ")"  << '\n';
    }
    if (!player.min_image_frame_dim.isempty()) {
        std::cout << "Min image frame dim   : (" <<
            player.min_image_frame_dim.w << ", "  <<
            player.min_image_frame_dim.h << ")"  << '\n';
    }
    std::cout << "RemoteApp session     : " << (player.info.remote_app ? "Yes" : "No") << '\n';
    std::cout.flush();
}

inline
static void show_metadata2(FileToGraphic const & player) {
    if (!player.max_image_frame_rect.isempty()) {
        std::cout << "Max image frame rect  : (" <<
            player.max_image_frame_rect.x << ", "  <<
            player.max_image_frame_rect.y << ", "  <<
            player.max_image_frame_rect.cx << ", "  <<
            player.max_image_frame_rect.cy << ")"  << '\n';
    }
    if (!player.min_image_frame_dim.isempty()) {
        std::cout << "Min image frame dim   : (" <<
            player.min_image_frame_dim.w << ", "  <<
            player.min_image_frame_dim.h << ")"  << '\n';
    }
    std::cout.flush();
}

// TODO Signals related code should not be here, all globals if any should be in main
static bool program_requested_to_shutdown = false;

inline void shutdown(int sig)
{
    LOG(LOG_INFO, "shutting down : signal %d pid=%d", sig, getpid());

    program_requested_to_shutdown = true;
}

inline void init_signals()
{
    struct sigaction sa;

    sa.sa_flags = 0;

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTERM);

    sa.sa_handler = shutdown;
    sigaction(SIGTERM, &sa, nullptr);
}


inline int count_digit(uint64_t n)
{
    return
        n < 100000
            ? n < 10000
                ? n < 1000
                    ? n < 100
                        ? n < 10
                            ? 1
                            : 2
                        : 3
                    : 4
                :  5
            : n < 1000000 ? 6
            : n < 10000000 ? 7
            : n < 100000000 ? 8
            : n < 1000000000 ? 9
            : n < 10000000000 ? 10
            : n < 100000000000 ? 11
            : n < 1000000000000 ? 12
            : n < 10000000000000 ? 13
            : n < 100000000000000 ? 14
            : n < 1000000000000000 ? 15
            : n < 10000000000000000 ? 16
            : n < 100000000000000000 ? 17
            : n < 1000000000000000000 ? 18
            : 19
        ;
}

template<class Mem>
int fiels_size(FileToGraphic::Statistics const & statistics, Mem mem)
{
    return std::max({
        count_digit(statistics.DstBlt.*mem),
        count_digit(statistics.MultiDstBlt.*mem),
        count_digit(statistics.PatBlt.*mem),
        count_digit(statistics.MultiPatBlt.*mem),
        count_digit(statistics.OpaqueRect.*mem),
        count_digit(statistics.MultiOpaqueRect.*mem),
        count_digit(statistics.ScrBlt.*mem),
        count_digit(statistics.MultiScrBlt.*mem),
        count_digit(statistics.MemBlt.*mem),
        count_digit(statistics.Mem3Blt.*mem),
        count_digit(statistics.LineTo.*mem),
        count_digit(statistics.GlyphIndex.*mem),
        count_digit(statistics.Polyline.*mem),

        count_digit(statistics.CacheBitmap.*mem),
        count_digit(statistics.CacheColorTable.*mem),
        count_digit(statistics.CacheGlyph.*mem),

        count_digit(statistics.FrameMarker.*mem),

        count_digit(statistics.BitmapUpdate.*mem),

        count_digit(statistics.CachePointer.*mem),
        count_digit(statistics.PointerIndex.*mem),

        count_digit(statistics.timestamp_chunk.*mem),
    });
}

struct OutStatisticField
{
    int count_len;
    int total_len;
    FileToGraphic::Statistics::Order const & stat;

    friend std::ostream & operator <<(std::ostream & out, OutStatisticField const & f)
    {
        return out << std::setw(f.count_len) << f.stat.count
            << "  ( " << std::setw(f.total_len) << f.stat.total_len << " bytes)";
    }
};

inline
static void show_statistics(
    FileToGraphic::Statistics const & statistics,
    uint64_t total_wrm_file_len, unsigned count_wrm_file)
{
    using Stat = FileToGraphic::Statistics::Order;
    int const count_field_len = fiels_size(statistics, &Stat::count);
    int const total_field_len = fiels_size(statistics, &Stat::total_len);

    auto f = [=](Stat const & stat){
      return OutStatisticField{count_field_len, total_field_len, stat};
    };

    std::cout
    << "\nCount wrm file        : " << count_wrm_file
    << "\nTotal wrm files size  : " << total_wrm_file_len << " bytes"
    << "\nTotal orders size     : " << statistics.total_read_len << " bytes. Ratio : x" << (statistics.total_read_len / double(total_wrm_file_len))
    << "\nInternal orders size  : " << statistics.internal_order_read_len << " bytes"
    << "\n"
    << "\nDstBlt                : " << f(statistics.DstBlt)
    << "\nMultiDstBlt           : " << f(statistics.MultiDstBlt)
    << "\nPatBlt                : " << f(statistics.PatBlt)
    << "\nMultiPatBlt           : " << f(statistics.MultiPatBlt)
    << "\nOpaqueRect            : " << f(statistics.OpaqueRect)
    << "\nMultiOpaqueRect       : " << f(statistics.MultiOpaqueRect)
    << "\nScrBlt                : " << f(statistics.ScrBlt)
    << "\nMultiScrBlt           : " << f(statistics.MultiScrBlt)
    << "\nMemBlt                : " << f(statistics.MemBlt)
    << "\nMem3Blt               : " << f(statistics.Mem3Blt)
    << "\nLineTo                : " << f(statistics.LineTo)
    << "\nGlyphIndex            : " << f(statistics.GlyphIndex)
    << "\nPolyline              : " << f(statistics.Polyline)
    << "\n"
    << "\nCacheBitmap           : " << f(statistics.CacheBitmap)
    << "\nCacheColorTable       : " << f(statistics.CacheColorTable)
    << "\nCacheGlyph            : " << f(statistics.CacheGlyph)
    << "\n"
    << "\nFrameMarker           : " << f(statistics.FrameMarker)
    << "\n"
    << "\nBitmapUpdate          : " << f(statistics.BitmapUpdate)
    << "\n"
    << "\nCachePointer          : " << f(statistics.CachePointer)
    << "\nPointerIndex          : " << f(statistics.PointerIndex)
    << "\n"
    << "\nNewOrExistingWindow   : " << f(statistics.NewOrExistingWindow)
    << "\nDeletedWindow         : " << f(statistics.DeletedWindow)
    << "\n"
    << "\ngraphics_update_chunk : " << std::setw(count_field_len) << statistics.graphics_update_chunk
    << "\nbitmap_update_chunk   : " << std::setw(count_field_len) << statistics.bitmap_update_chunk
    << "\ntimestamp_chunk       : " << f(statistics.timestamp_chunk)
    << std::endl;
}


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

inline void get_join_visibility_rect(
    Rect & out_max_image_frame_rect,
    Rect & out_min_image_frame_rect,
    Dimension & out_max_screen_dim,
    std::string const& infile_path,
    std::string const& input_basename,
    std::string const& infile_extension,
    std::string const& hash_path,
    bool infile_is_encrypted,
    Inifile & ini, CryptoContext & cctx,
    Fstat & fstat, uint32_t verbose)
{
    char infile_prefix[4096];
    std::snprintf(infile_prefix, sizeof(infile_prefix), "%s%s", infile_path.c_str(), input_basename.c_str());
    ini.set<cfg::video::hash_path>(hash_path);

    auto const encryption_mode = infile_is_encrypted
      ? InMetaSequenceTransport::EncryptionMode::Encrypted
      : InMetaSequenceTransport::EncryptionMode::NotEncrypted;

    unsigned file_count = 0;
    {
        InCryptoTransport buf_meta(cctx, encryption_mode, fstat);
        MwrmReader mwrm_reader(buf_meta);
        MetaLine meta_line;

        buf_meta.open((infile_prefix + infile_extension).c_str());
        mwrm_reader.read_meta_headers();

        meta_line.start_time = 0;
        meta_line.stop_time = 0;

        if (Transport::Read::Ok == mwrm_reader.read_meta_line(meta_line)) {
            while (Transport::Read::Ok == mwrm_reader.read_meta_line(meta_line)) {
                file_count++;
            }
        }
    }

    InMetaSequenceTransport in_wrm_trans(
        cctx, infile_prefix,
        infile_extension.c_str(),
        encryption_mode,
        fstat
    );

    timeval begin_capture = {0, 0};
    timeval end_capture = {0, 0};

    for (unsigned i = 1; i < file_count ; i++) {
        in_wrm_trans.next();
    }

    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, ini.get<cfg::video::play_video_with_corrupted_bitmap>(), to_verbose_flags(verbose));

    player.play(program_requested_to_shutdown);

    if (player.info.remote_app) {
        out_max_image_frame_rect = player.max_image_frame_rect.intersect(Rect(0, 0, player.info.width, player.info.height));
        if (out_max_image_frame_rect.cx & 1)
        {
            if (out_max_image_frame_rect.x + out_max_image_frame_rect.cx < player.info.width) {
                out_max_image_frame_rect.cx += 1;
            }
            else if (out_max_image_frame_rect.x > 0) {
                out_max_image_frame_rect.x  -=1;
                out_max_image_frame_rect.cx += 1;
            }
        }

        out_min_image_frame_rect = Rect(0, 0,
            std::min(player.min_image_frame_dim.w, player.info.width),
            std::min(player.min_image_frame_dim.h, player.info.height));
        if (!out_min_image_frame_rect.isempty()) {
            if (out_min_image_frame_rect.cx & 1) {
                out_min_image_frame_rect.cx++;
            }

            out_min_image_frame_rect.x = (player.info.width  - out_min_image_frame_rect.cx) / 2;
            out_min_image_frame_rect.y = (player.info.height - out_min_image_frame_rect.cy) / 2;
        }
    }

    out_max_screen_dim = player.max_screen_dim;
}

inline int replay(std::string & infile_path, std::string & input_basename, std::string & infile_extension,
                  std::string const& hash_path,
                  CaptureFlags const& capture_flags,
                  UpdateProgressData::Format pgs_format,
                  bool chunk,
                  unsigned ocr_version,
                  std::string const& output_filename,
                  int64_t begin_cap,
                  int64_t end_cap,
                  PngParams & png_params,
                  VideoParams & video_params,
                  FullVideoParams const& full_video_params,
                  int wrm_color_depth,
                  uint32_t wrm_frame_interval,
                  uint32_t wrm_break_interval,
                  bool infile_is_encrypted,
                  uint32_t order_count,
                  bool show_file_metadata,
                  bool show_statistics,
                  uint32_t clear,
                  bool full_video,
                  bool remove_input_file,
                  int wrm_compression_algorithm,
                  uint32_t video_break_interval,
                  TraceType encryption_type,
                  Inifile & ini, CryptoContext & cctx,
                  Rect const & crop_rect,
                  Dimension const & max_screen_dim,
                  Random & rnd, Fstat & fstat,
                  uint32_t verbose)
{
    char infile_prefix[4096];
    std::snprintf(infile_prefix, sizeof(infile_prefix), "%s%s", infile_path.c_str(), input_basename.c_str());
    ini.set<cfg::video::hash_path>(hash_path);

    ini.set<cfg::video::frame_interval>(std::chrono::duration<unsigned int, std::centi>{wrm_frame_interval});
    ini.set<cfg::video::break_interval>(std::chrono::seconds{wrm_break_interval});
    ini.set<cfg::globals::trace_type>(encryption_type);

    ini.set<cfg::globals::capture_chunk>(chunk);
    ini.set<cfg::ocr::version>(ocr_version == 2 ? OcrVersion::v2 : OcrVersion::v1);

    if (chunk){
        ini.set<cfg::ocr::interval>(std::chrono::seconds{1});
    }

    auto const encryption_mode = infile_is_encrypted
      ? InMetaSequenceTransport::EncryptionMode::Encrypted
      : InMetaSequenceTransport::EncryptionMode::NotEncrypted;

    timeval  begin_record = { 0, 0 };
    timeval  end_record   = { 0, 0 };
    unsigned file_count   = 0;
    uint64_t total_wrm_file_len = 0;
    unsigned count_wrm_file = 0;
    try {
        // begin or end relative to end of trace
        if (begin_cap < 0 || end_cap < 0) {
            InCryptoTransport buf_meta(cctx, encryption_mode, fstat);
            MwrmReader mwrm_reader(buf_meta);
            MetaLine meta_line;

            buf_meta.open((infile_prefix + infile_extension).c_str());
            mwrm_reader.read_meta_headers();

            meta_line.start_time = 0;
            meta_line.stop_time = 0;

            if (Transport::Read::Ok == mwrm_reader.read_meta_line(meta_line)) {
                time_t const start_time = meta_line.start_time;
                while (Transport::Read::Ok == mwrm_reader.read_meta_line(meta_line)) {
                }

                auto const duration = meta_line.stop_time - start_time;

                if (begin_cap < 0) {
                    begin_cap = std::max<decltype(begin_cap)>(begin_cap + duration, 0);
                }

                if (end_cap < 0) {
                    end_cap = std::max<decltype(end_cap)>(end_cap + duration, 0);
                }
            }
        }
        InMetaSequenceTransport in_wrm_trans_tmp(
            cctx,
            infile_prefix,
            infile_extension.c_str(),
            encryption_mode,
            fstat);
        file_count = get_file_count(
            in_wrm_trans_tmp,
            begin_cap, end_cap,
            begin_record, end_record,
            fstat, total_wrm_file_len, count_wrm_file);
    }
    catch (const Error & e) {
        if (e.id == ERR_TRANSPORT_NO_MORE_DATA) {
            std::cerr << "Asked time not found in mwrm file\n";
        }
        else {
            std::cerr << "Error: " << e.errmsg() << std::endl;
        }
        const bool msg_with_error_id = false;
        raise_error(pgs_format, output_filename, e.id, e.errmsg(msg_with_error_id));
        return -1;
    }

    InMetaSequenceTransport in_wrm_trans(
        cctx, infile_prefix,
        infile_extension.c_str(),
        encryption_mode,
        fstat
    );

    timeval begin_capture = {begin_cap, 0};
    timeval end_capture = {end_cap, 0};

    int result = -1;
    try {
        bool const test = (
               bool(capture_flags & CaptureFlags::video)
            || bool(capture_flags & CaptureFlags::ocr)
            || bool(capture_flags & CaptureFlags::png)
            || full_video
            || wrm_color_depth != static_cast<int>(USE_ORIGINAL_COLOR_DEPTH)
            || show_file_metadata
            || show_statistics
            || file_count > 1
            || order_count
            || begin_cap != begin_record.tv_sec
            || end_cap != begin_cap);

        if (test){
            for (unsigned i = 1; i < file_count; i++) {
                in_wrm_trans.next();
            }

            LOG(LOG_INFO, "player begin_capture = %ld", begin_capture.tv_sec);
            FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, ini.get<cfg::video::play_video_with_corrupted_bitmap>(), to_verbose_flags(verbose));

            if (show_file_metadata) {
                show_metadata(player);
                std::cout << "Duration (in seconds) : " << (end_record.tv_sec - begin_record.tv_sec + 1) << std::endl;
            }

            if (show_file_metadata && !show_statistics && !output_filename.length()) {
                result = 0;
            }
            else {
                player.max_order_count = order_count;

                int return_code = 0;

                if (not output_filename.empty()) {
            //        char outfile_pid[32];
            //        std::snprintf(outfile_pid, sizeof(outfile_pid), "%06u", getpid());

                    auto outfile = ParsePath(output_filename);

                    if (verbose) {
                        std::cout << "Output file path: "
                                    << outfile.directory << outfile.basename << outfile.extension
                                    << '\n' << std::endl;
                    }

                    if (clear) {
                        clear_files_flv_meta_png(outfile.directory.c_str(), outfile.basename.c_str());
                    }

                    ini.set<cfg::video::wrm_compression_algorithm>(
                        (wrm_compression_algorithm == static_cast<int>(USE_ORIGINAL_COMPRESSION_ALGORITHM))
                        ? player.info.compression_algorithm
                        : static_cast<WrmCompressionAlgorithm>(wrm_compression_algorithm)
                    );

                    if (wrm_color_depth == static_cast<int>(USE_ORIGINAL_COLOR_DEPTH)) {
                        wrm_color_depth = safe_int(player.info.bpp);
                    }

                    {
                        ini.set<cfg::video::record_tmp_path>(outfile.directory);
                        ini.set<cfg::video::record_path>(outfile.directory);

                        char progress_filename[4096];
                        std::snprintf( progress_filename, sizeof(progress_filename), "%s%s.pgs"
                                , outfile.directory.c_str(), outfile.basename.c_str());
                        UpdateProgressData update_progress_data(
                            pgs_format, progress_filename,
                            begin_record.tv_sec, end_record.tv_sec,
                            begin_capture.tv_sec, end_capture.tv_sec
                        );

                        if (png_params.png_width && png_params.png_height) {
                            auto get_percent = [](unsigned target_dim, unsigned source_dim) -> unsigned {
                                return ((target_dim * 100 / source_dim) + ((target_dim * 100 % source_dim) ? 1 : 0));
                            };
                            png_params.zoom = std::max<unsigned>(
//                                    get_percent(png_params.png_width, player.screen_rect.cx),
                                    get_percent(png_params.png_width, max_screen_dim.w),
//                                    get_percent(png_params.png_height, player.screen_rect.cy)
                                    get_percent(png_params.png_height, max_screen_dim.h)
                                );
                            //std::cout << "zoom: " << zoom << '%' << std::endl;
                        }

                        ini.set<cfg::video::bogus_vlc_frame_rate>(video_params.bogus_vlc_frame_rate);
                        ini.set<cfg::video::ffmpeg_options>(video_params.codec_options);
                        ini.set<cfg::video::codec_id>(video_params.codec);
                        video_params = video_params_from_ini(
                            std::chrono::seconds{video_break_interval}, ini);

                        const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>().c_str();
                        const char * record_path = record_tmp_path;

                        bool capture_wrm = bool(capture_flags & CaptureFlags::wrm);
                        bool capture_png = bool(capture_flags & CaptureFlags::png);
                        bool capture_pattern_checker = false;

                        bool capture_ocr = bool(capture_flags & CaptureFlags::ocr)
                                            || capture_pattern_checker;
                        bool capture_video = bool(capture_flags & CaptureFlags::video);
                        bool capture_video_full = full_video;
                        bool capture_meta = capture_ocr;
                        bool capture_kbd = false;

                        OcrParams const ocr_params = ocr_params_from_ini(ini);

                        if (ini.get<cfg::debug::capture>()) {
                            LOG(LOG_INFO, "Enable capture:  %s%s  kbd=%d %s%s%s  ocr=%d %s",
                                capture_wrm ?"wrm ":"",
                                capture_png ?"png ":"",
                                capture_kbd ? 1 : 0,
                                capture_video ?"video ":"",
                                capture_video_full ?"video_full ":"",
                                capture_pattern_checker ?"pattern ":"",
                                capture_ocr ? (ocr_params.ocr_version == OcrVersion::v2 ? 2 : 1) : 0,
                                capture_meta?"meta ":""
                            );
                        }

                        const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
                        const char * hash_path = ini.get<cfg::video::hash_path>().c_str();

                        auto spath = ParsePath(output_filename);

                        // PngParams
                        png_params.remote_program_session = false;
                        png_params.rt_display = ini.get<cfg::video::rt_display>();

                        RDPDrawable rdp_drawable{
                            max_screen_dim.w, max_screen_dim.h};

                        DrawableParams const drawable_params{
                            max_screen_dim.w,
                            max_screen_dim.h,
                            &rdp_drawable
                        };

                        MetaParams const meta_params{
                            MetaParams::EnableSessionLog::No,
                            MetaParams::HideNonPrintable::No,

                            MetaParams::LogClipboardActivities((ini.get<cfg::video::disable_clipboard_log>() & ClipboardLogFlags::meta) != ClipboardLogFlags::meta),
                            MetaParams::LogFileSystemActivities((ini.get<cfg::video::disable_file_system_log>() & FileSystemLogFlags::meta) != FileSystemLogFlags::meta),

                            MetaParams::LogOnlyRelevantClipboardActivities(ini.get<cfg::mod_rdp::log_only_relevant_clipboard_activities>())
                        };

                        KbdLogParams kbd_log_params = kbd_log_params_from_ini(ini);
                        kbd_log_params.session_log_enabled = false;

                        PatternParams const pattern_params = pattern_params_from_ini(ini);

                        SequencedVideoParams const sequenced_video_params {};

                        cctx.set_trace_type(ini.get<cfg::globals::trace_type>());

                        WrmParams const wrm_params = wrm_params_from_ini(
                            checked_int(wrm_color_depth),
                            player.info.remote_app,
                            cctx,
                            rnd,
                            fstat,
                            hash_path,
                            ini
                        );

                        auto set_capture_consumer = [
                            &, capture = std::optional<Capture>()
                        ](timeval const & now) mutable {
                            CaptureParams capture_params{
                                now,
                                spath.basename.c_str(),
                                record_tmp_path,
                                record_path,
                                groupid,
                                nullptr,
                                ini.get<cfg::video::smart_video_cropping>(),
                                0
                            };
                            auto* ptr = &capture.emplace(
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
                                , &update_progress_data
                                , crop_rect
                            );

                            player.clear_consumer();
                            player.add_consumer(ptr, ptr, ptr, ptr, ptr, ptr, ptr);
                        };

                        auto lazy_capture = [&](timeval const & now) {
                            if (begin_capture.tv_sec > now.tv_sec) {
                                return;
                            }
                            set_capture_consumer(begin_capture);
                        };

                        struct CaptureMaker : gdi::ExternalCaptureApi
                        {
                            void external_breakpoint() override {}

                            void external_time(const timeval & now) override
                            {
                                this->load_capture(now);
                            }

                            explicit CaptureMaker(decltype(lazy_capture) & load_capture)
                            : load_capture(load_capture)
                            {}

                            decltype(lazy_capture) & load_capture;
                        };
                        CaptureMaker capture_maker(lazy_capture);

                        if (begin_capture.tv_sec) {
                            player.add_consumer(
                                &rdp_drawable, nullptr, nullptr, nullptr, &capture_maker, nullptr, nullptr);
                        }
                        else {
                            set_capture_consumer(player.record_now);
                        }

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
                        clear_files_flv_meta_png(outfile.directory.c_str(), outfile.basename.c_str());
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

                if (show_file_metadata) {
                    show_metadata2(player);
                }

                if (show_statistics && return_code == 0) {
                    ::show_statistics(player.statistics, total_wrm_file_len, count_wrm_file);
                }

                result = return_code;
            }
        }
        else {
            result = do_recompress(
                cctx,
                rnd,
                fstat,
                in_wrm_trans,
                begin_record,
                program_requested_to_shutdown,
                wrm_compression_algorithm,
                output_filename,
                ini,
                verbose);
        }
    }
    catch (const Error & e) {
        const bool msg_with_error_id = false;
        raise_error(pgs_format, output_filename, e.id, e.errmsg(msg_with_error_id));
    }

    if (!result && remove_input_file) {
        InMetaSequenceTransport in_wrm_trans_tmp(
            cctx,
            infile_prefix,
            infile_extension.c_str(),
            encryption_mode,
            fstat);

        remove_file( in_wrm_trans_tmp, ini.get<cfg::video::hash_path>().c_str(), infile_path.c_str()
                    , input_basename.c_str(), infile_extension.c_str()
                    , infile_is_encrypted);
    }

    std::cout << std::endl;

    return result;
}

struct RecorderParams {
    std::string input_filename;
    std::string input_basename;
    std::string infile_extension;
    std::string config_filename = app_path(AppPath::CfgIni).to_string();
    std::string mwrm_path;
    std::string hash_path;
    std::string full_path;

    bool show_file_metadata = false;
    bool show_statistics    = false;

    // input restrictions
    // ==================
    // "begin capture time (in seconds), either absolute or relative to video start
    // (negative number means relative to video end), default=from start"
    int64_t begin_cap = 0;
    // "end capture time (in seconds), either absolute or relative to video start,
    // (nagative number means relative to video end), default=none"
    int64_t end_cap = 0;
    // "Number of orders to execute before stopping, default=0 execute all orders"
    uint32_t order_count = 0;

    // common output options
    std::string output_filename;

    // png output options
    PngParams png_params = {0, 0, std::chrono::seconds{60}, 100, 0, false , false, false};
    VideoParams video_params;
    FullVideoParams full_video_params;

    // video output options
    bool full_video; // create full video
    uint32_t    video_break_interval = 10*60;

    // wrm output options
    int wrm_compression_algorithm_ = static_cast<int>(USE_ORIGINAL_COMPRESSION_ALGORITHM);
    int wrm_color_depth = static_cast<int>(USE_ORIGINAL_COLOR_DEPTH);
    std::string wrm_encryption;
    uint32_t    wrm_frame_interval = 100;
    uint32_t    wrm_break_interval = 86400;
    TraceType encryption_type = TraceType::localfile;

    // ocr output options
    unsigned    ocr_version = -1u;

    // miscellaneous options
    CaptureFlags capture_flags = CaptureFlags::none; // output control
    bool auto_output_file   = false;
    bool remove_input_file  = false;
    uint32_t    clear       = 1; // default on
    bool infile_is_encrypted = false;
    bool chunk = false;

    // verifier options
    bool        quick_check    = false;
    bool      ignore_stat_info = false;
    bool      update_stat_info = false;

    bool json_pgs = false;
};

enum class ClRes
{
    Ok,
    Err,
    Exit,
};

inline
ClRes parse_command_line_options(int argc, char const ** argv, RecorderParams & recorder, Inifile & ini, uint32_t & verbose)
{
    std::string png_geometry;
    std::string wrm_compression_algorithm;  // output compression algorithm.
    std::string color_depth;
    uint32_t png_interval = 0;
    std::string ignored_value;

    program_options::options_description desc({
        {'h', "help", "produce help message"},
        {'v', "version", "show software version"},
        {'o', "output-file", &recorder.output_filename, "output base filename"},
        {'i', "input-file", &recorder.input_filename, "mwrm input filename"},

        {'H', "hash-path",  &recorder.hash_path, "output hash dirname (if empty, use hash_path of ini)"},
        {'M', "mwrm-path",  &recorder.mwrm_path, "mwrm file path"},

        // verifier options
        {'Q', "quick",   "quick check only"},
        {'S', "ignore-stat-info", "ignore stat info data mismatch"},
        {'U', "update-stat-info", "update stat info data mismatch "
                                  "(only if not checksum and no encrypted)"},

        {'b', "begin", &recorder.begin_cap, "begin capture time (in seconds), default=none"},
        {'e', "end", &recorder.end_cap, "end capture time (in seconds), default=none"},
        {"count", &recorder.order_count, "Number of orders to execute before stopping, default=0 execute all orders"},

        {'n', "png_interval", &png_interval, "time interval between png captures, default=60 seconds"},

        {'r', "frameinterval", &recorder.wrm_frame_interval, "time between consecutive capture frames (in 100/th of seconds), default=100 one frame per second"},

        {'k', "breakinterval", &recorder.wrm_break_interval, "number of seconds between splitting wrm files in seconds(default, one wrm every day)"},

        {'p', "png", "enable png capture"},
        {'w', "wrm", "enable wrm capture"},
        {'t', "ocr", "enable ocr title bar detection"},
        {'f', "video", "enable video capture"},
        {'u', "full", "create full video"},
        {'c', "chunk", "chunk splitting on title bar change detection"},

        {"clear", &recorder.clear, "clear old capture files with same prefix (default on, 0 to disable)"},
        {"verbose", &verbose, "more logs"},
        {"zoom", &recorder.png_params.zoom, "scaling factor for png capture (default 100%)"},
        {'g', "png-geometry", &png_geometry, "png capture geometry (Ex. 160x120)"},
        {'m', "meta", "show file metadata"},
        {'s', "statistics", "show statistics"},

        {'z', "compression", &wrm_compression_algorithm, "wrm compression algorithm (default=original, none, gzip, snappy)"},
        {'d', "color-depth", &color_depth, "wrm color depth (default=original, 16, 24)"},
        {'y', "encryption",  &recorder.wrm_encryption, "wrm encryption (default=original, enable, disable)"},

        {"remove-input-file", "remove input file"},

        {"config-file", &recorder.config_filename, "use another ini file"},

        {'a', "video-break-interval", &recorder.video_break_interval, "number of seconds between splitting video files (by default, one video every 10 minutes)"},

        {'q', "video-quality", &ignored_value, "video quality (ignored, please use --video-codec-options)"},
        {'D', "video-codec-options", &recorder.video_params.codec_options, "FFmpeg codec option, format: key1=value1 key2=value2"},

        {"ocr-version", &recorder.ocr_version, "version 1 or 2"},

        {"video-codec", &recorder.video_params.codec, "ffmpeg video codec name (flv, mp4, etc)"},
        {"bogus-vlc", "Needed to play a video with ffplay or VLC."},
        {"disable-bogus-vlc", ""},

        {"json-pgs", "use json format to .pgs file"},
    });

    auto cl_error = [&recorder](char const* mes, int const errnum = 1) /*NOLINT*/ {
        std::cerr << mes << "\n";
        if (recorder.json_pgs) {
            raise_error(UpdateProgressData::JSON_FORMAT, recorder.output_filename, errnum, mes);
        }
        return ClRes::Err;
    };

    auto options = program_options::parse_command_line(argc, argv, desc);

    const char * copyright_notice = "\n"
        "Copyright (C) Wallix 2010-2018.\n"
        "Christophe Grosjean, Jonathan Poelen, Raphael Zhou.";

    if (options.count("help") > 0) {
        std::cout << redemption_info_version() << copyright_notice;
        std::cout << "\n\nUsage: redrec [options]\n\n";
        // TODO error code description
        std::cout << desc << "\n" << std::endl;
        return ClRes::Exit;
    }

    if (options.count("version") > 0) {
        std::cout << redemption_info_version() << copyright_notice << "\n" << std::endl;
        return ClRes::Exit;
    }

    configuration_load(ini.configuration_holder(), recorder.config_filename.c_str());

    if (options.count("quick") > 0) {
        recorder.quick_check = true;
    }

    if (options.count("ignore-stat-info") > 0) {
        recorder.ignore_stat_info = true;
    }

    if (options.count("update-stat-info") > 0) {
        recorder.update_stat_info = true;
    }

    if (options.count("json-pgs") > 0) {
        recorder.json_pgs = true;
    }

    recorder.full_video_params.bogus_vlc_frame_rate = ini.get<cfg::video::bogus_vlc_frame_rate>();
    recorder.video_params.bogus_vlc_frame_rate = false;
    if (options.count("bogus-vlc")) {
        recorder.full_video_params.bogus_vlc_frame_rate = true;
        recorder.video_params.bogus_vlc_frame_rate = true;
    }
    if (options.count("disable-bogus-vlc")) {
        recorder.full_video_params.bogus_vlc_frame_rate = false;
        recorder.video_params.bogus_vlc_frame_rate = false;
    }
    recorder.chunk = options.count("chunk") > 0;
    recorder.capture_flags
      = (                   options.count("wrm")    ? CaptureFlags::wrm   : CaptureFlags::none)
      | ((!recorder.chunk && options.count("png"))  ? CaptureFlags::png   : CaptureFlags::none)
      | ((recorder.chunk || options.count("video")) ? CaptureFlags::video : CaptureFlags::none)
      | ((recorder.chunk || options.count("ocr"))   ? CaptureFlags::ocr   : CaptureFlags::none);

    if (options.count("video-codec-options") == 0) {
        recorder.video_params.codec_options = ini.get<cfg::video::ffmpeg_options>();
    }

    if (options.count("video-codec") == 0) {
        recorder.video_params.codec = ini.get<cfg::video::codec_id>();
    }

    recorder.remove_input_file  = (options.count("remove-input-file") > 0);

    if (options.count("color-depth") > 0){
        recorder.wrm_color_depth = (color_depth == "16") ? 16
                                 : (color_depth == "24") ? 24
                                 : (color_depth == "original") ? static_cast<int>(USE_ORIGINAL_COLOR_DEPTH)
                                 : 0;
        if (!recorder.wrm_color_depth){
            return cl_error("Unknown wrm color depth");
        }
    }

    if (options.count("png_interval") > 0){
        recorder.png_params.png_interval = std::chrono::seconds{png_interval};
    }

    if ((options.count("zoom") > 0)
    && (options.count("png-geometry") > 0)) {
        return cl_error("Conflicting options: --zoom and --png-geometry");
    }

    if (options.count("png-geometry") > 0) {
        char * end;
        auto png_w = strtoul(png_geometry.c_str(), &end, 10);
        auto png_h = (*end == 'x') ? strtoul(end+1, &end, 10) : 0L;

        unsigned long const max_size = 16 * 1024;

        if (!png_w || !png_h || *end || png_w > max_size || png_h > max_size) {
            return cl_error("Invalide png geometry");
        }
        recorder.png_params.png_width  = unsigned(png_w);
        recorder.png_params.png_height = unsigned(png_h);
        std::cout << "png-geometry: " << recorder.png_params.png_width << "x" << recorder.png_params.png_height << std::endl;
    }

    //recorder.video_params.video_codec = "flv";

    if (options.count("compression") > 0) {
        if (wrm_compression_algorithm == "none") {
            recorder.wrm_compression_algorithm_ = static_cast<int>(WrmCompressionAlgorithm::no_compression);
        }
        else if (wrm_compression_algorithm == "gzip") {
            recorder.wrm_compression_algorithm_ = static_cast<int>(WrmCompressionAlgorithm::gzip);
        }
        else if (wrm_compression_algorithm == "snappy") {
            recorder.wrm_compression_algorithm_ = static_cast<int>(WrmCompressionAlgorithm::snappy);
        }
        else if (wrm_compression_algorithm == "original") {
            recorder.wrm_compression_algorithm_ = static_cast<int>(USE_ORIGINAL_COMPRESSION_ALGORITHM);
        }
        else {
            return cl_error("Unknown wrm compression algorithm");
        }
    }

    if (options.count("hash-path") > 0){
        if (recorder.hash_path.c_str()[0] == 0) {
            return cl_error("Missing hash-path: use -h path");
        }
    }
    else {
        recorder.hash_path      = ini.get<cfg::video::hash_path>().c_str();
    }
    // TODO: check we do not already have a trailing slash
    if (!recorder.hash_path.empty()) {
        recorder.hash_path += '/';
    }

    if (options.count("mwrm-path") > 0){
        if (recorder.mwrm_path.c_str()[0] == 0) {
            return cl_error("Missing mwrm-path: use -m path");
        }
    }
    else {
        recorder.mwrm_path = ini.get<cfg::video::record_path>().c_str();
    }

    if (recorder.input_filename.c_str()[0] == 0) {
        return cl_error("Missing input mwrm file name: use -i filename");
    }

    // Input path rule is as follow:
    // -----------------------------
    //  default search directory for mwrm is given in config file
    // if --mwrm-path is provided on command line it will be preferably used as default
    // if -i has a path component it will be used instead of mwrm-path
    // if relative all command lines path are relative to current working directory

    {
        auto input = ParsePath(recorder.input_filename);

        if (not input.directory.empty()) {
            recorder.mwrm_path = input.directory;
        }

        recorder.input_basename = "";
        recorder.input_filename = "";
        recorder.infile_extension = ".mwrm";
        if (not input.basename.empty()) {
            recorder.input_basename = input.basename;
            recorder.input_filename = input.basename;
            recorder.infile_extension = (strlen(input.extension.c_str()) > 0)?input.extension.c_str():".mwrm";
            recorder.input_filename += recorder.infile_extension;
        }

        if (recorder.mwrm_path.back() != '/'){
            recorder.mwrm_path.push_back('/');
        }
        if (recorder.hash_path.back() != '/'){
            recorder.hash_path.push_back('/');
        }
    }
    recorder.full_path = recorder.mwrm_path + recorder.input_filename;

    if (verbose) {
        LOG(LOG_INFO, "Input file full_path=\"%s\"", recorder.full_path);
        LOG(LOG_INFO, "Input file base name=\"%s\"", recorder.input_filename);
        LOG(LOG_INFO, "hash_path=\"%s\"", recorder.hash_path);
        LOG(LOG_INFO, "mwrm_path=\"%s\"", recorder.mwrm_path);
    }

    if (is_encrypted_file(recorder.full_path.c_str(), recorder.infile_is_encrypted) == -1) {
        int const errnum = errno;
        auto const mes = str_concat("Input file error: ", strerror(errnum));
        return cl_error(mes.c_str(), -errnum);
    }

    if (options.count("encryption") > 0) {
        if (0 == strcmp(recorder.wrm_encryption.c_str(), "enable")) {
            recorder.encryption_type = TraceType::cryptofile;
        }
        else if (0 == strcmp(recorder.wrm_encryption.c_str(), "disable")) {
            recorder.encryption_type = TraceType::localfile;
        }
        else if (0 == strcmp(recorder.wrm_encryption.c_str(), "original")) {
            recorder.encryption_type = recorder.infile_is_encrypted ? TraceType::cryptofile : TraceType::localfile;
        }
        else {
            return cl_error("Unknown wrm encryption parameter");
        }
    }

    recorder.full_video = (options.count("full") > 0);
    recorder.show_file_metadata = (options.count("meta"             ) > 0);
    recorder.show_statistics    = (options.count("statistics"       ) > 0);

    if (!recorder.output_filename.empty()) {
        auto output = ParsePath(recorder.output_filename);
        if (output.directory.empty()){
            std::string directory = str_concat(app_path(AppPath::Wrm), '/');
            recorder.output_filename.insert(0, directory);
        }
        if (output.extension.empty()){
            std::string extension = ".mwrm"     ;
            recorder.output_filename += extension;
        }
        std::cout << "Output file is \"" << recorder.output_filename << "\".\n";
    }
    return ClRes::Ok;
}


extern "C" {
    REDEMPTION_LIB_EXPORT
    int do_main(int argc, char const ** argv,
            uint8_t * hmac_key,
            get_trace_key_prototype * trace_fn)
    {
        ScopedCryptoInit scoped_crypto;

        int arg_used = 0;
        int command = 0;

        if (argc > arg_used + 1){
            char const* scmd = argv[arg_used+1];
            command = !strcmp("redrec", scmd) ? 1
                    : !strcmp("redver", scmd) ? 2
                    : !strcmp("reddec", scmd) ? 3
                    : 0;
            if (command) {
                --command;
                ++arg_used;
            }
            // default command is previous one;
        }

        Inifile ini;
        ini.set<cfg::debug::config>(false);

        UdevRandom rnd;
        Fstat fstat;
        CryptoContext cctx;
        if (hmac_key) {
            cctx.set_hmac_key(CryptoContext::key_data::from_ptr(hmac_key));
        }
        cctx.set_get_trace_key_cb(trace_fn);

        uint8_t tmp[32] = {};
        for (auto a : {0, 1}) {
            if (argc <= arg_used + 1){
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

        uint32_t    verbose     = 0;
        RecorderParams rp;
        // TODO: annoying, if we read default hash_path and mwrm_path from ini
        // we should do that after config_filename was eventually changed...

        switch (parse_command_line_options(argc, argv, rp, ini, verbose)) {
            case ClRes::Exit: return 0;
            case ClRes::Err: return -1;
            case ClRes::Ok: ;
        }

        {
            size_t base_len = 0;
            char const * base = basename_len(rp.input_filename.c_str(), base_len);
            cctx.set_master_derivator({base, base_len});
        }

        switch (command){
        case 0: // RECorder
        try {
            init_signals();

            if (rp.input_filename.empty()) {
                std::cerr << "Missing input filename : use -i filename\n\n";
                return -1;
            }

            if (!rp.show_file_metadata
             && !rp.show_statistics
             && !rp.auto_output_file
             && rp.output_filename.empty()
            ) {
                std::cerr << "Missing output filename : use -o filename\n\n";
                return -1;
            }

            if (rp.chunk) {
                rp.video_break_interval = 60*10; // 10 minutes
                rp.png_params.png_interval = std::chrono::seconds{1};
            }

            if (!rp.output_filename.empty()
             && !rp.full_video
             && !bool(rp.capture_flags)
             && !rp.show_file_metadata
             && !rp.show_statistics
            ) {
                std::cerr << "Missing target format : need --png, --ocr, --video, --full, --wrm, --meta, --statistics or --chunk" << std::endl;
                return 1;
            }

            // TODO before continuing to work with input file, check if it's mwrm or wrm and use right object in both cases

            // TODO also check if it contains any wrm at all and at wich one we should start depending on input time
            // TODO if start and stop time are outside wrm, userreplay(s should also be warned

            Rect      crop_rect;
            Dimension max_screen_dim;
            {
                Rect      max_joint_visibility_rect;
                Rect      min_joint_visibility_rect;
                try {
                    get_join_visibility_rect(
                        max_joint_visibility_rect,
                        min_joint_visibility_rect,
                        max_screen_dim,
                        rp.mwrm_path, rp.input_basename, rp.infile_extension,
                        rp.hash_path,
                        rp.infile_is_encrypted,
                        ini, cctx, fstat,
                        verbose
                    );

                    if (ini.get<cfg::video::smart_video_cropping>() == SmartVideoCropping::v2) {
                        crop_rect = min_joint_visibility_rect;
                    }
                    else if (ini.get<cfg::video::smart_video_cropping>() != SmartVideoCropping::disable) {
                        crop_rect = max_joint_visibility_rect;
                    }
                }
                catch (Error const&) {
                    // ignore exceptions, logged within replay()
                }
            }

            res = replay(rp.mwrm_path, rp.input_basename, rp.infile_extension,
                         rp.hash_path,
                         rp.capture_flags,
                         rp.json_pgs ? UpdateProgressData::JSON_FORMAT : UpdateProgressData::OLD_FORMAT,
                         rp.chunk,
                         rp.ocr_version,
                         rp.output_filename,
                         rp.begin_cap,
                         rp.end_cap,
                         rp.png_params,
                         rp.video_params,
                         rp.full_video_params,
                         rp.wrm_color_depth,
                         rp.wrm_frame_interval,
                         rp.wrm_break_interval,
                         rp.infile_is_encrypted,
                         rp.order_count,
                         rp.show_file_metadata,
                         rp.show_statistics,
                         rp.clear,
                         rp.full_video,
                         rp.remove_input_file,
                         rp.wrm_compression_algorithm_,
                         rp.video_break_interval,
                         rp.encryption_type,
                         ini, cctx, crop_rect, max_screen_dim,
                         rnd, fstat, verbose);

            } catch (const Error & e) {
                std::cout << "decrypt failed: with id=" << e.id << std::endl;
            }
        break;
        case 1: // VERifier
            ini.set<cfg::debug::config>(false);
            try {
                Error out_error{NO_ERROR};
                switch (get_encryption_scheme_type(cctx, rp.full_path.c_str(), bytes_view{}, &out_error))
                {
                    case EncryptionSchemeTypeResult::Error:
                        throw out_error; /* NOLINT(misc-throw-by-value-catch-by-reference) */
                    case EncryptionSchemeTypeResult::OldScheme:
                        cctx.old_encryption_scheme = true;
                        break;
                    default:
                        break;
                }

                res = check_encrypted_or_checksumed(
                    rp.input_filename, rp.mwrm_path, rp.hash_path,
                    rp.quick_check, rp.ignore_stat_info, rp.update_stat_info,
                    cctx, fstat, verbose
                );
                std::cout << "verify " << (res == 0 ? "ok" : "failed") << std::endl;
            } catch (const Error & e) {
                std::cout << "verify failed: with id=" << e.id << std::endl;
            }
        break;
        default: // DECrypter
            try {
                Fstat fstat;
                InCryptoTransport in_t(cctx, EncryptionMode::Auto, fstat);
                Error out_error{NO_ERROR};
                switch (open_if_possible_and_get_encryption_scheme_type(
                    in_t, rp.full_path.c_str(), bytes_view{}, &out_error))
                {
                    case EncryptionSchemeTypeResult::Error:
                        std::cerr
                          << "can't open file " << rp.full_path.c_str()
                          << ": " << out_error.errmsg() << std::endl;
                        std::cout << "decrypt failed" << std::endl;
                        return -1;
                    case EncryptionSchemeTypeResult::NoEncrypted:
                        std::cout << "Input file is not encrypted." << std::endl;
                        return 0;
                    case EncryptionSchemeTypeResult::OldScheme:
                        cctx.old_encryption_scheme = true;
                        in_t.open(rp.full_path.c_str());
                        break;
                    default:
                        break;
                }

                size_t res = -1ull;
                unique_fd fd1(rp.output_filename, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);

                if (fd1.is_open()) {
                    OutFileTransport out_t(std::move(fd1));

                    try {
                        if (!in_t.is_open()) {
                            in_t.open(rp.full_path.c_str());
                        }

                        char mem[4096];
                        while ((res = in_t.partial_read(mem, sizeof(mem)))) {
                            out_t.send(mem, res);
                        }
                    }
                    catch (Error const & e) {
                        LOG(LOG_INFO, "Exited on exception: %s", e.errmsg());
                        res = -1ull;
                    }
                }
                else {
                    std::cerr << strerror(errno) << std::endl << std::endl;
                }

                if (res == 0){
                    std::cout << "decrypt ok" << std::endl;
                    return 0;
                }

                std::cout << "decrypt failed" << std::endl;
                return -1;
            } catch (const Error & e) {
                std::cout << "decrypt failed: with id=" << e.id << std::endl;
            }
        break;
        }

        return res;
    }
}
