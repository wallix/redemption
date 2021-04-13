/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013-2017
   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen

   redver video verifier program
*/

#include "lib/do_recorder.hpp"

#include "system/scoped_crypto_init.hpp"

#include "capture/file_to_graphic.hpp"
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
#include "capture/regions_capture.hpp"

#include "capture/capture.hpp"
#include "capture/cryptofile.hpp"
#include "capture/save_state_chunk.hpp"

#include "configs/config.hpp"

#include "core/RDP/RDPDrawable.hpp"

#include "transport/crypto_transport.hpp"
#include "transport/mwrm_reader.hpp"
#include "transport/file_transport.hpp"
#include "transport/out_meta_sequence_transport.hpp"
#include "transport/in_multi_crypto_transport.hpp"
#include "transport/mwrm_file_data.hpp"

#include "utils/compression_transport_builder.hpp"
#include "utils/fileutils.hpp"
#include "utils/genrandom.hpp"
#include "utils/log.hpp"
#include "utils/cli.hpp"
#include "utils/cli_chrono.hpp"
#include "utils/recording_progress.hpp"
#include "utils/redemption_info_version.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/strutils.hpp"
#include "utils/hexadecimal_string_to_buffer.hpp"
#include "utils/ref.hpp"


#include <string>
#include <vector>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <optional>
#include <charconv>

// opendir/closedir
#include <sys/types.h>
#include <dirent.h>

#include <csignal>

using namespace std::chrono_literals;
using namespace std::string_view_literals;

namespace
{

enum {
    USE_ORIGINAL_COMPRESSION_ALGORITHM = 0xFFFFFFFF
};

enum {
    USE_ORIGINAL_COLOR_DEPTH           = 0xFFFFFFFF
};

template<class T>
struct OriginalOr
{
    OriginalOr() = default;

    OriginalOr(T const& x)
    : is_set(true)
    , value(x)
    {}

    bool is_original() const noexcept
    {
        return !is_set;
    }

    void set(T const& x)
    {
        this->value = x;
        this->is_set = true;
    }

    void reset() noexcept
    {
        this->is_set = false;
    }

    T const& get_or(T const& default_value) const noexcept
    {
        return is_set ? value : default_value;
    }

private:
    bool is_set = false;
    T value;
};


class ChunkToFile
{
    CompressionOutTransportBuilder compression_bullder;
    OutMetaSequenceTransport & trans_target;
    Transport & trans;

    const uint8_t wrm_format_version;

    uint16_t info_version = 0;

    RealTimePoint real_time {};

public:
    ChunkToFile(
        OutMetaSequenceTransport& trans, WrmMetaChunk info,
        WrmCompressionAlgorithm wrm_compression_algorithm)
    : compression_bullder(trans, wrm_compression_algorithm)
    , trans_target(trans)
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
        auto resend = [&]{
            send_wrm_chunk(this->trans, chunk_type, stream.get_capacity(), chunk_count);
            this->trans.send(stream.get_data(), stream.get_capacity());
        };

        REDEMPTION_DIAGNOSTIC_PUSH()
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
        switch (chunk_type)
        {
        case WrmChunkType::META_FILE: {
            WrmMetaChunk meta;
            meta.receive(stream);

            this->info_version = meta.version;
            meta.compression_algorithm = this->compression_bullder.get_algorithm();

            meta.send(this->trans_target);

            break;
        }

        case WrmChunkType::SAVE_STATE: {
            StateChunk sc;
            SaveStateChunk ssc;

            ssc.recv(stream, sc, this->info_version);

            StaticOutStream<65536> payload;

            ssc.send(payload, sc);

            send_wrm_chunk(this->trans, WrmChunkType::SAVE_STATE, payload.get_offset(), chunk_count);
            this->trans.send(payload.get_produced_bytes());

            break;
        }

        case WrmChunkType::RESET_CHUNK: {
            send_wrm_chunk(this->trans, WrmChunkType::RESET_CHUNK, 0, 1);
            this->trans.next();

            break;
        }

        case WrmChunkType::TIMES: {
            if (this->real_time == RealTimePoint{}) {
                stream.in_skip_bytes(8);
                this->real_time = RealTimePoint(std::chrono::microseconds(stream.in_uint64_le()));
                this->trans_target.timestamp(this->real_time);
            }
            resend();

            break;
        }

        case WrmChunkType::TIMESTAMP_OR_RECORD_DELAY: {
            auto us = std::chrono::microseconds(stream.in_uint64_le());
            this->trans_target.timestamp(this->real_time + us);
            resend();

            break;
        }

        default:
            resend();
        }
        REDEMPTION_DIAGNOSTIC_POP()
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

    bool meta_ok = false;

    WrmMetaChunk info;

public:
    enum class Verbose : uint32_t
    {
        none,
        end_of_transport = 1,
    };

    Verbose verbose;

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

    WrmMetaChunk const& get_wrm_info() const noexcept { return this->info; }

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

            LOG_IF(this->verbose == Verbose::end_of_transport, LOG_INFO,
                "receive error %u : end of transport", e.id);
            return false;
        }

        return true;
    }

    void interpret_chunk() {
        switch (underlying_cast(this->chunk_type)) {
        case underlying_cast(WrmChunkType::META_FILE):
            this->info.receive(this->stream);
            this->trans = &this->compression_builder.reset(
                *this->trans_source, this->info.compression_algorithm
            );

            this->stream.rewind();
            this->meta_ok = true;
            break;
        case underlying_cast(WrmChunkType::RESET_CHUNK):
            this->info.compression_algorithm = WrmCompressionAlgorithm::no_compression;

            this->trans = this->trans_source;
            break;
        }
    }   // void interpret_chunk()
};


inline
static int do_recompress(
    CryptoContext & cctx,
    Random & rnd,
    Transport & in_wrm_trans,
    const std::chrono::seconds begin_record,
    bool & program_requested_to_shutdown,
    OriginalOr<WrmCompressionAlgorithm> wrm_compression_algorithm,
    std::string const & output_filename,
    int group_id,
    FilePermissions file_permissions,
    char const* hash_path,
    uint32_t verbose
) {
    FileToChunk player(&in_wrm_trans, safe_cast<FileToChunk::Verbose>(verbose));
    auto outfile = ParsePath(output_filename);

    if (verbose) {
        std::cout << "Output file path: " << outfile.directory << outfile.basename << outfile.extension << '\n' << std::endl;
    }

    if (recursive_create_directory(outfile.directory.c_str(), S_IRWXU | S_IRGRP | S_IXGRP, group_id) != 0) {
        std::cerr << "Failed to create directory: \"" << outfile.directory << "\"" << std::endl;
    }

    const auto wrm_compression = wrm_compression_algorithm.get_or(
        player.get_wrm_info().compression_algorithm);

    int return_code = 0;
    try {
        CryptoContext cctx_no_crypto;

        OutMetaSequenceTransport trans(
            cctx,
            rnd,
            outfile.directory.c_str(),
            hash_path,
            outfile.basename.c_str(),
            RealTimePoint(begin_record),
            player.get_wrm_info().width,
            player.get_wrm_info().height,
            group_id,
            nullptr,
            file_permissions
        );

        ChunkToFile recorder(trans, player.get_wrm_info(), wrm_compression);

        player.set_consumer(recorder);

        player.play(program_requested_to_shutdown);
    }
    catch (...) {
        return_code = -1;
    }

    return return_code;
}   // do_recompress


using EncryptionMode = InCryptoTransport::EncryptionMode;

static inline bool check_file(
    const std::string & filename, const MetaLine & metadata,
    bool quick, MetaHeader const& meta_header, uint8_t const (&hmac_key)[HMAC_KEY_LENGTH])
{
    if (meta_header.version != WrmVersion::v1) {
        struct stat64 sb {};
        if (lstat64(filename.c_str(), &sb) < 0){
            std::cerr << "File \"" << filename << "\" is invalid! (can't stat file)\n" << std::endl;
            return false;
        }

        if (sb.st_size != metadata.size){
            std::cerr << "File \"" << filename << "\" is invalid! (size mismatch "
                << sb.st_size << " != " << metadata.size << ")\n" << std::endl;
            return false;
        }
    }

    if (meta_header.has_checksum){
        InCryptoTransport::HASH hash;
        bool read_is_ok = quick
            ? InCryptoTransport::read_qhash(filename.c_str(), hmac_key, hash)
            : InCryptoTransport::read_fhash(filename.c_str(), hmac_key, hash);
        if (!read_is_ok) {
            std::cerr << "Error reading file \"" << filename << "\"\n" << std::endl;
            return false;
        }

        if (0 != memcmp(hash.hash, quick ? metadata.hash1 : metadata.hash2, MD_HASH::DIGEST_LENGTH)){
            std::cerr << "Error checking file \"" << filename << "\" (invalid checksum)\n" << std::endl;
            return false;
        }
    }

    return true;
}

static inline int check_encrypted_or_checksumed(
    std::string const & input_filename,
    std::string const & mwrm_path,
    std::string const & hash_path,
    bool quick_check,
    CryptoContext & cctx,
    uint32_t verbose
) {
    std::string const full_mwrm_filename = mwrm_path + input_filename;

    InCryptoTransport ibuf(cctx, EncryptionMode::Auto);

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

    /*****************
    * Load file hash *
    *****************/
    LOG_IF(verbose, LOG_INFO, "Load file hash. Is encrypted %d. Is checksumed: %d",
        infile_is_encrypted, reader.get_header().has_checksum);
    MetaLine hash_line {};

    std::string const full_hash_path = hash_path + input_filename;

    // if reading hash fails
    bool hash_file_is_open = false;
    try {
        auto const encryption_mode = infile_is_encrypted
            ? EncryptionMode::Encrypted
            : EncryptionMode::NotEncrypted;
        InCryptoTransport hash_file(cctx, encryption_mode);

        hash_file.open(full_hash_path.c_str());
        hash_file_is_open = true;

        auto const infile_version = reader.get_header().version;
        LOG_IF(verbose, LOG_INFO, "%s", infile_version == WrmVersion::v1
            ? "Hash data v1"
            : "Hash data v2 or higher");

        MwrmReader hash_reader(hash_file);
        hash_reader.set_header({infile_version, reader.get_header().has_checksum});
        hash_reader.read_meta_hash_line(hash_line);
        if (input_filename != hash_line.filename) {
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
    }
    catch (...) {
        if (!hash_file_is_open) {
            LOG(LOG_ERR, "Open load_hash failed");
        }
        std::cerr << "Cannot read hash file: \"" << full_hash_path << "\"\n" << std::endl;
        return 1;
    }

    /******************
    * Check mwrm file *
    ******************/
    if (!check_file(
        full_mwrm_filename, hash_line, quick_check, reader.get_header(),
        cctx.get_hmac_key()
    )){
        return 1;
    }

    MetaLine meta_line_wrm;

    while (Transport::Read::Ok == reader.read_meta_line(meta_line_wrm)) {
        size_t tmp_wrm_filename_len = 0;
        char const* tmp_wrm_filename = basename_len(meta_line_wrm.filename, tmp_wrm_filename_len);
        auto const meta_line_wrm_filename = std::string_view(tmp_wrm_filename, tmp_wrm_filename_len);
        auto const full_part_filename = str_concat(mwrm_path, meta_line_wrm_filename);
        // LOG(LOG_INFO, "checking part %s", full_part_filename);

        if (!check_file(
            full_part_filename, meta_line_wrm, quick_check, reader.get_header(),
            cctx.get_hmac_key())
        ){
            return 1;
        }
    }

    std::cout << "No error detected during the data verification.\n" << std::endl;

    return 0;
}

static void raise_error(
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
    UpdateProgressData update_progress_data(
        progress_filename, MonotonicTimePoint(), MonotonicTimePoint());

    update_progress_data.raise_error(code, message);
}

static int raise_error_and_log(
    std::string const& output_filename,
    int code, zstring_view message)
{
    std::cerr << message << std::endl;
    raise_error(output_filename, -1, message.c_str());
    return code;
}

static inline int update_filename_and_check_size(
    std::string const& output_filename,
    std::vector<MetaLine> const& wrms,
    std::vector<std::string>& filenames,
    std::string_view other_wrm_directory,
    bool ignore_file_size)
{
    struct stat st;
    std::string prefix = (other_wrm_directory.empty() || other_wrm_directory.back() != '/')
        ? std::string(other_wrm_directory)
        : str_concat(other_wrm_directory, '/');

    filenames.reserve(wrms.size());
    for (auto const& wrm : wrms) {
        if (::stat(wrm.filename, &st)) {
            bool has_error = true;
            if (errno == ENOENT && !other_wrm_directory.empty()) {
                size_t len = 0;
                char const* basename = basename_len(wrm.filename, len);
                filenames.push_back(str_concat(prefix, std::string_view{basename, len}));
                has_error = ::stat(filenames.back().c_str(), &st);
            }

            if (has_error) {
                LOG(LOG_INFO, "Wrm file not found: %s", filenames.back());
                return raise_error_and_log(output_filename, -1, "wrm file not fount"_zv);
            }
        }
        else {
            filenames.push_back(wrm.filename);
        }

        if (!ignore_file_size && wrm.size != st.st_size) {
            using ull = unsigned long long;
            LOG(LOG_INFO, "Wrm file size mismatch (%llu != %llu): %s",
                ull(wrm.size), ull(st.st_size), filenames.back());
            return raise_error_and_log(output_filename, -1, "wrm file size mismatch"_zv);
        }
    }

    return 0;
}


static void show_metadata(FileToGraphic const & player) {
    auto& info = player.get_wrm_info();
    std::cout
        << "\nWRM file version      : " << static_cast<int>(info.version)
        << "\nWidth                 : " << info.width
        << "\nHeight                : " << info.height
        << "\nBpp                   : " << static_cast<int>(info.bpp)
        << "\nCache 0 entries       : " << info.cache_0_entries
        << "\nCache 0 size          : " << info.cache_0_size
        << "\nCache 1 entries       : " << info.cache_1_entries
        << "\nCache 1 size          : " << info.cache_1_size
        << "\nCache 2 entries       : " << info.cache_2_entries
        << "\nCache 2 size          : " << info.cache_2_size
        << '\n';

    if (info.version > 3) {
        //cout << "Cache 3 entries       : " << player.info_cache_3_entries                         << endl;
        //cout << "Cache 3 size          : " << player.info_cache_3_size                            << endl;
        //cout << "Cache 4 entries       : " << player.info_cache_4_entries                         << endl;
        //cout << "Cache 4 size          : " << player.info_cache_4_size                            << endl;
        std::cout << "Compression algorithm : " << static_cast<int>(info.compression_algorithm) << '\n';
    }

    if (!player.max_image_frame_rect.isempty()) {
        std::cout << "Max image frame rect  : (" <<
            player.max_image_frame_rect.x << ", " <<
            player.max_image_frame_rect.y << ", " <<
            player.max_image_frame_rect.cx << ", " <<
            player.max_image_frame_rect.cy << ")" << '\n';
    }

    if (!player.min_image_frame_dim.isempty()) {
        std::cout << "Min image frame dim   : (" <<
            player.min_image_frame_dim.w << ", " <<
            player.min_image_frame_dim.h << ")" << '\n';
    }

    std::cout << "RemoteApp session     : " << (info.remote_app ? "Yes" : "No") << '\n';
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


enum class IsEncryptedResult
{
    Error,
    IsEncrypted,
    IsUnencrypted,
};

inline IsEncryptedResult is_encrypted_file(const char * input_filename)
{
    auto ufd = unique_fd{input_filename, O_RDONLY};
    if (REDEMPTION_UNLIKELY(!ufd)) {
        return IsEncryptedResult::Error;
    }

    uint8_t data[4] = {};
    ssize_t const res_test = read(ufd.fd(), data, 4);
    if (REDEMPTION_UNLIKELY(res_test == -1)) {
        return IsEncryptedResult::Error;
    }

    uint32_t const magic =  uint32_t(data[0])
                         | (uint32_t(data[1]) << 8)
                         | (uint32_t(data[2]) << 16)
                         | (uint32_t(data[3]) << 24);
    return (res_test == 4 && magic == WABCRYPTOFILE_MAGIC)
        ? IsEncryptedResult::IsEncrypted
        : IsEncryptedResult::IsUnencrypted;
}

struct CaptureTimes
{
    using Seconds = std::chrono::seconds;

    // TODO currently a timestamp, should be a duration
    Seconds begin_cap;
    Seconds end_cap;

    // start and stop as timestamp
    void adjust_time(Seconds start_time, Seconds stop_time)
    {
        // begin or end relative to end of trace
        {
            Seconds const duration ( stop_time - start_time);

            if (begin_cap.count() < 0) {
                begin_cap = std::max(begin_cap + duration, Seconds(0));
            }

            if (end_cap.count() < 0) {
                end_cap = std::max(end_cap + duration, Seconds(0));
            }
        }

        // begin or end relative to start of trace
        {
            // less than 1 year, it is relative not absolute timestamp
            auto relative_time_barrier = 31536000s;

            if (begin_cap.count() && begin_cap < relative_time_barrier) {
                begin_cap += Seconds(start_time);
            }

            if (end_cap.count() && end_cap < relative_time_barrier) {
                end_cap += Seconds(start_time);
            }
        }
    }
};

struct RecorderParams
{
    std::string input_filename;
    std::string input_basename;
    std::string infile_extension;
    std::string config_filename = app_path(AppPath::CfgIni).to_string();
    std::string mwrm_path;
    std::string hash_path;
    std::string full_path;

    bool show_file_metadata = false;
    bool show_statistics    = false;

    bool play_video_with_corrupted_bitmap = false;

    uint32_t verbosity = 0;

    int group_id;
    FilePermissions file_permissions = FilePermissions::user_permissions(
        FilePermissions::BitPermissions::read);

    // input restrictions
    // ==================
    // "begin capture time (in seconds), either absolute or relative to video start
    // (negative number means relative to video end)"
    std::chrono::seconds begin_cap {};
    // "end capture time (in seconds), either absolute or relative to video start,
    // (nagative number means relative to video end)"
    std::chrono::seconds end_cap {};

    // common output options
    std::string output_filename;

    // png output options
    Dimension png_geometry {};
    std::chrono::seconds png_interval = 60s;

    // video output options
    VideoParams video_params {};
    FullVideoParams full_video_params {};
    SequencedVideoParams sequenced_video_params {};
    SmartVideoCropping smart_video_cropping {};
    bool full_video = false; // create full video

    // ocr output options
    OcrParams ocr_params { OcrVersion::v2, ocr::locale::LocaleId::latin, {}, {}, {}, {}};

    // wrm output options
    OriginalOr<WrmCompressionAlgorithm> wrm_compression_algorithm;
    OriginalOr<BitsPerPixel> wrm_color_depth;
    std::chrono::seconds wrm_break_interval {24h};
    TraceType encryption_type;
    RDPSerializerVerbose wrm_verbosity;

    // meta output options
    MetaParams::LogClipboardActivities log_cliboard_activities;
    MetaParams::LogFileSystemActivities log_file_system_activities;
    MetaParams::LogOnlyRelevantClipboardActivities log_only_relevant_clipboard_activities;

    // kbd output options
    bool wrm_keyboard_log;
    bool meta_keyboard_log;

    // miscellaneous options
    CaptureFlags capture_flags = CaptureFlags::none; // output control
    bool clear = true;
    bool infile_is_encrypted = false;

    // verifier options
    bool quick_check      = false;
    bool ignore_file_size = false;
};

static inline int replay(
    InMultiCryptoTransport&& in_wrm_trans,
    std::vector<MetaLine> const& wrm_lines,
    RecorderParams const& rp,
    const CaptureTimes capture_times,
    CryptoContext & cctx,
    Rect const & crop_rect,
    Dimension const & max_screen_dim,
    Random & rnd)
{
    using Seconds = std::chrono::seconds;

    int file_count = 0;

    // number of file up to begin_cap
    if (capture_times.begin_cap.count())
    {
        auto first = wrm_lines.begin();
        auto last = wrm_lines.end();

        while (first < last && capture_times.begin_cap >= Seconds(first->stop_time)) {
            ++first;
        }

        if (first == last) {
            return raise_error_and_log(rp.output_filename, -1, "Asked time not found in mwrm file"_zv);
        }

        file_count = checked_int{first - wrm_lines.begin() + 1};
    }

    std::chrono::seconds begin_record = Seconds(wrm_lines.front().start_time);
    std::chrono::seconds end_record   = Seconds(wrm_lines.back().stop_time);
    unsigned count_wrm_file = wrm_lines.size();
    // TODO wrm_lines.size() ?
    uint64_t total_wrm_file_len = 0;

    int result = -1;
    try {
        bool const test = (
               bool(rp.capture_flags & CaptureFlags::video)
            || bool(rp.capture_flags & CaptureFlags::ocr)
            || bool(rp.capture_flags & CaptureFlags::png)
            || rp.full_video
            || !rp.wrm_color_depth.is_original()
            || rp.show_file_metadata
            || rp.show_statistics
            || file_count > 1
            || capture_times.begin_cap != begin_record
            || capture_times.end_cap != capture_times.begin_cap);

        if (test){
            for (int i = 1; i < file_count; i++) {
                in_wrm_trans.next();
            }

            LOG(LOG_INFO, "player begin_capture = %ld", capture_times.begin_cap.count());
            FileToGraphic player(
                in_wrm_trans,
                MonotonicTimePoint{capture_times.begin_cap},
                MonotonicTimePoint{capture_times.end_cap},
                rp.play_video_with_corrupted_bitmap,
                safe_cast<FileToGraphic::Verbose>(rp.verbosity));

            if (rp.show_file_metadata) {
                show_metadata(player);
                std::cout << "Duration (in seconds) : " << ((end_record - begin_record).count() + 1) << std::endl;
            }

            if (rp.show_file_metadata && !rp.show_statistics && !rp.output_filename.length()) {
                result = 0;
            }
            else {
                int return_code = 0;

                if (not rp.output_filename.empty()) {
                    auto outfile = ParsePath(rp.output_filename);

                    if (rp.verbosity) {
                        std::cout << "Output file path: "
                                  << outfile.directory << outfile.basename << outfile.extension
                                  << '\n' << std::endl;
                    }

                    if (rp.clear) {
                        clear_files_flv_meta_png(outfile.directory.c_str(), outfile.basename.c_str());
                    }

                    {
                        char progress_filename[4096];
                        std::snprintf( progress_filename, sizeof(progress_filename), "%s%s.pgs"
                                     , outfile.directory.c_str(), outfile.basename.c_str());
                        UpdateProgressData update_progress_data(
                            progress_filename,
                            MonotonicTimePoint(capture_times.begin_cap != 0s
                                                ? capture_times.begin_cap
                                                : begin_record),
                            MonotonicTimePoint(capture_times.end_cap != 0s
                                                ? capture_times.end_cap
                                                : end_record));

                        const char * record_tmp_path = outfile.directory.c_str();
                        const char * record_path = record_tmp_path;

                        const bool capture_wrm = bool(rp.capture_flags & CaptureFlags::wrm);
                        const bool capture_png = bool(rp.capture_flags & CaptureFlags::png);
                        const bool capture_pattern_checker = false;

                        const bool capture_ocr = bool(rp.capture_flags & CaptureFlags::ocr)
                                              || capture_pattern_checker;
                        const bool capture_video = bool(rp.capture_flags & CaptureFlags::video);
                        const bool capture_video_full = rp.full_video;
                        const bool capture_meta = capture_ocr;
                        const bool capture_kbd = false;

                        const auto spath = ParsePath(rp.output_filename);

                        PngParams png_params{
                            rp.png_geometry.w,
                            rp.png_geometry.h,
                            rp.png_interval,
                            /*limit = */0,
                            /*real_time_image_capture = */false,
                            /*remote_program_session = */false,
                            /*rt_display = */false,
                            /*real_basename = */spath.basename.c_str(),
                        };

                        RDPDrawable rdp_drawable{max_screen_dim.w, max_screen_dim.h};

                        cctx.set_trace_type(rp.encryption_type);

                        std::optional<Capture> retarded_capture {};
                        auto set_capture_consumer = [&](
                            MonotonicTimePoint now, RealTimePoint real_time
                        ) mutable {
                            DrawableParams const drawable_params
                                = DrawableParams::shared_drawable(rdp_drawable,
                                                                  player.pointers_view());

                            MetaParams const meta_params{
                                MetaParams::EnableSessionLog::No,
                                MetaParams::HideNonPrintable::No,
                                rp.log_cliboard_activities,
                                rp.log_file_system_activities,
                                rp.log_only_relevant_clipboard_activities,
                            };

                            KbdLogParams const kbd_log_params{
                                rp.wrm_keyboard_log,
                                false,
                                false,
                                rp.meta_keyboard_log,
                            };

                            WrmParams const wrm_params = WrmParams{
                                rp.wrm_color_depth.get_or(player.get_wrm_info().bpp),
                                player.get_wrm_info().remote_app,
                                cctx,
                                rnd,
                                rp.hash_path.c_str(),
                                rp.wrm_break_interval,
                                rp.wrm_compression_algorithm.get_or(
                                    player.get_wrm_info().compression_algorithm),
                                rp.wrm_verbosity,
                                rp.file_permissions
                            };

                            CaptureParams capture_params{
                                now,
                                real_time,
                                spath.basename.c_str(),
                                record_tmp_path,
                                record_path,
                                rp.group_id,
                                nullptr,
                                rp.smart_video_cropping,
                                0
                            };

                            auto* ptr = &retarded_capture.emplace(
                                  capture_params
                                , drawable_params
                                , capture_wrm, wrm_params
                                , capture_png, png_params
                                , capture_pattern_checker, PatternParams{}
                                , capture_ocr, rp.ocr_params
                                , capture_video, rp.sequenced_video_params
                                , capture_video_full, rp.full_video_params
                                , capture_meta, meta_params
                                , capture_kbd, kbd_log_params
                                , rp.video_params
                                , &update_progress_data
                                , crop_rect
                                // TODO rail_window_rect
                                , Rect()
                            );

                            player.clear_consumer();
                            player.add_consumer(ptr, ptr, ptr, ptr, ptr, ptr, ptr);
                        };

                        struct CaptureMaker final : gdi::ExternalCaptureApi
                        {
                            void external_breakpoint() override {}

                            // old format starts with timestamp (real tune = monotonic_time)
                            void external_monotonic_time_point(MonotonicTimePoint now) override
                            {
                                if (this->old_format_begin_capture > now) {
                                    return;
                                }

                                this->load_capture(now, RealTimePoint{now.time_since_epoch()});
                            }

                            // new format starts with time_points
                            void external_times(MonotonicTimePoint::duration monotonic_delay, RealTimePoint real_time) override
                            {
                                if (this->new_format_begin_capture > monotonic_delay) {
                                    return;
                                }

                                auto elapsed = monotonic_delay - this->new_format_begin_capture;
                                this->load_capture(
                                    MonotonicTimePoint(monotonic_delay - elapsed),
                                    real_time - elapsed
                                );

                                if (elapsed > 250ms) {
                                    this->retarded_capture->external_times(
                                        monotonic_delay, real_time);
                                }
                            }

                            explicit CaptureMaker(
                                decltype(set_capture_consumer) & load_capture,
                                std::optional<Capture> & retarded_capture,
                                MonotonicTimePoint old_format_begin_capture,
                                MonotonicTimePoint::duration new_format_begin_capture)
                            : old_format_begin_capture(old_format_begin_capture)
                            , new_format_begin_capture(new_format_begin_capture)
                            , load_capture(load_capture)
                            , retarded_capture(retarded_capture)
                            {}

                        private:
                            MonotonicTimePoint old_format_begin_capture;
                            MonotonicTimePoint::duration new_format_begin_capture;
                            decltype(set_capture_consumer) & load_capture;
                            std::optional<Capture> & retarded_capture;
                        };

                        CaptureMaker capture_maker(
                            set_capture_consumer,
                            retarded_capture,
                            MonotonicTimePoint(capture_times.begin_cap),
                            MonotonicTimePoint::duration(
                                capture_times.begin_cap - Seconds(wrm_lines.front().start_time)
                            )
                        );

                        if (capture_times.begin_cap.count()) {
                            player.add_consumer(
                                &rdp_drawable, nullptr, nullptr, nullptr,
                                &capture_maker, nullptr, nullptr);
                        }
                        else {
                            set_capture_consumer(
                                player.get_monotonic_time(), player.get_real_time());
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

                if (rp.show_file_metadata) {
                    show_metadata2(player);
                }

                if (rp.show_statistics && return_code == 0) {
                    ::show_statistics(player.statistics, total_wrm_file_len, count_wrm_file);
                }

                result = return_code;
            }
        }
        else {
            result = do_recompress(
                cctx,
                rnd,
                in_wrm_trans,
                begin_record,
                program_requested_to_shutdown,
                rp.wrm_compression_algorithm,
                rp.output_filename,
                rp.group_id,
                rp.file_permissions,
                rp.hash_path.c_str(),
                rp.verbosity);
        }
    }
    catch (const Error & e) {
        const bool msg_with_error_id = false;
        raise_error(rp.output_filename, e.id, e.errmsg(msg_with_error_id));
    }

    std::cout << std::endl;

    return result;
}

enum class ClRes
{
    Ok,
    Err,
    Exit,
};

inline
ClRes parse_command_line_options(int argc, char const ** argv, RecorderParams & recorder)
{
    enum class WrmEncryption {
        unspecified,
        enable,
        disable,
        original,
    };

    OriginalOr<TraceType> wrm_encryption(TraceType::localfile);
    std::string wrm_compression_algorithm;  // output compression algorithm.
    std::string color_depth;
    int bogus_vlc = 2; /* 0 = explicitly disabled, 1 = explicitly enabled */
    std::string_view msg_error;
    std::string_view codec_options;
    std::chrono::seconds video_break_interval {};
    bool chunk = false;

    auto const options = cli::options(
        cli::option('h', "help").help("produce help message")
            .parser(cli::help()),

        cli::option('v', "version").help("show software version")
            .parser(cli::quit([]{
                std::cout
                    << redemption_info_version() << "\n"
                    << redemption_info_copyright() << std::endl
                ;
            })),

        cli::option('o', "output-file").help("output base filename")
            .parser(cli::arg_location(recorder.output_filename)).argname("<path>"),

        cli::option('i', "input-file").help("mwrm input filename")
            .parser(cli::arg_location(recorder.input_filename)).argname("<path>"),

        cli::option('H', "hash-path")
            .help("output hash dirname (if empty, use hash_path of ini)")
            .parser(cli::arg_location(recorder.hash_path)).argname("<directory-path>"),

        cli::option('M', "mwrm-path").help("mwrm file path")
            .parser(cli::arg_location(recorder.mwrm_path)).argname("<directory-path>"),

        cli::option('Q', "quick").help("quick check only")
            .parser(cli::on_off_location(recorder.quick_check)),

        cli::option('S', "ignore-file-size").help("ignore file size mismatch")
            .parser(cli::on_off_location(recorder.ignore_file_size)),

        cli::option('b', "begin").help("begin capture time (in seconds)")
            .parser(cli::arg_location(recorder.begin_cap)).argname("<seconds>"),

        cli::option('e', "end").help("end capture time (in seconds)")
            .parser(cli::arg_location(recorder.end_cap)).argname("<seconds>"),

        cli::option('n', "png-interval")
            .help("time interval between png captures, default=60 seconds")
            .parser(cli::arg_location(recorder.png_interval)),

        cli::option("frame-rate").help("frame per second, default=5 frames ")
            .parser(cli::arg_location(recorder.video_params.frame_rate)),

        cli::option('k', "breakinterval")
            .help("number of seconds between splitting wrm files in seconds (default, one wrm every day)")
            .parser(cli::arg_location(recorder.wrm_break_interval)),

        cli::option('p', "png").help("enable png capture")
            .parser(cli::on_off_bit_location<CaptureFlags::png>(recorder.capture_flags)),

        cli::option('w', "wrm").help("enable wrm capture")
            .parser(cli::on_off_bit_location<CaptureFlags::wrm>(recorder.capture_flags)),

        cli::option('t', "ocr").help("enable ocr title bar detection")
            .parser(cli::on_off_bit_location<CaptureFlags::ocr>(recorder.capture_flags)),

        cli::option('f', "video").help("enable video capture")
            .parser(cli::on_off_bit_location<CaptureFlags::video>(recorder.capture_flags)),

        cli::option('u', "full").help("create full video")
            .parser(cli::on_off_location(recorder.full_video)),

        cli::option('c', "chunk").help("chunk splitting on title bar change detection")
            .parser(cli::on_off_location(chunk)),

        cli::option("clear")
            .help("clear old capture files with same prefix (default on, 0 to disable)")
            .parser(cli::on_off_location(recorder.clear)),

        cli::option("verbose").help("more logs")
            .parser(cli::arg_location(recorder.verbosity)).argname("<verbosity>"),

        cli::option('g', "png-geometry").help("png capture geometry (ex. -g 160x120)")
            .parser(cli::arg([&](std::string_view geometry){
                auto& dim = recorder.png_geometry;
                auto rw = std::from_chars(geometry.begin(), geometry.end(), dim.w);
                if (rw.ec == std::errc() && *rw.ptr == 'x') {
                    auto rh = std::from_chars(rw.ptr+1, geometry.end(), dim.h);
                    if (rh.ec == std::errc() && !*rh.ptr) {
                        unsigned long const max_size = 16 * 1024;
                        if (dim.w <= max_size && dim.h <= max_size) {
                            return cli::Res::Ok;
                        }
                    }
                }

                msg_error = "Invalide png geometry";
                return cli::Res::BadFormat;
            })).argname("<geometry>"),

        cli::option('m', "meta").help("show file metadata")
            .parser(cli::on_off_location(recorder.show_file_metadata)),

        cli::option('s', "statistics").help("show statistics")
            .parser(cli::on_off_location(recorder.show_statistics)),

        cli::option('z', "compression")
            .help("wrm compression algorithm (default=original, none, gzip, snappy)")
            .parser(cli::arg([&](std::string_view level){
                if (level == "none") {
                    recorder.wrm_compression_algorithm.set(
                        WrmCompressionAlgorithm::no_compression);
                }
                else if (level == "gzip") {
                    recorder.wrm_compression_algorithm.set(
                        WrmCompressionAlgorithm::gzip);
                }
                else if (level == "snappy") {
                    recorder.wrm_compression_algorithm.set(
                        WrmCompressionAlgorithm::snappy);
                }
                else if (level == "original") {
                    recorder.wrm_compression_algorithm.reset();
                }
                else {
                    msg_error = "Unknown wrm compression algorithm";
                    return cli::Res::BadFormat;
                }

                return cli::Res::Ok;
            })).argname("<algorithm>"),

        cli::option('d', "color-depth")
            .help("wrm color depth (default=original, 16, 24)")
            .parser(cli::arg([&](std::string_view depth){
                if (depth == "16") {
                    recorder.wrm_color_depth.set(BitsPerPixel::BitsPP16);
                }
                else if (depth == "24") {
                    recorder.wrm_color_depth.set(BitsPerPixel::BitsPP24);
                }
                else if (depth == "original") {
                    recorder.wrm_color_depth.reset();
                }
                else {
                    msg_error = "Unknown wrm color depth";
                    return cli::Res::BadFormat;
                }

                return cli::Res::Ok;
            })).argname("<depth>"),

        cli::option('y', "encryption")
            .help("wrm encryption (default=original, enable, disable)")
            .parser(cli::arg([&](std::string_view level){
                if (level == "enable") {
                    wrm_encryption.set(TraceType::cryptofile);
                }
                else if (level == "disable") {
                    wrm_encryption.set(TraceType::localfile);
                }
                else if (level == "original") {
                    wrm_encryption.reset();
                }
                else {
                    msg_error = "Unknown wrm encryption parameter";
                    return cli::Res::BadFormat;
                }

                return cli::Res::Ok;
            })),

        cli::option("config-file").help("use another ini file")
            .parser(cli::arg_location(recorder.config_filename)).argname("<path>"),

        cli::option('a', "video-break-interval")
            .help("number of seconds between splitting video files (by default, one video every 10 minutes)")
            .parser(cli::arg_location(video_break_interval)),

        cli::option('D', "video-codec-options")
            .help("FFmpeg codec option, format: key1=value1 key2=value2")
            .parser(cli::arg_location(codec_options)).argname("<ffmpeg-option>"),

        cli::option("video-codec").help("ffmpeg video codec name (flv, mp4, etc)")
            .parser(cli::arg_location(recorder.video_params.codec)).argname("<codec>"),

        cli::option("bogus-vlc").help("Needed to play a video with old ffplay or VLC v1")
            .parser(cli::on_off_location(bogus_vlc)),

        cli::option("disable-bogus-vlc")
            .parser(cli::on_off([&](bool x){ bogus_vlc = !x; }))
    );

    auto cl_error = [&recorder](char const* mes, int const errnum = 1) /*NOLINT*/ {
        std::cerr << mes << "\n";
        raise_error(recorder.output_filename, errnum, mes);
        return ClRes::Err;
    };

    auto const cli_result = cli::parse(options, argc, argv);
    switch (cli_result.res) {
        case cli::Res::Ok:
            break;
        case cli::Res::Exit:
            return ClRes::Exit;
        case cli::Res::Help:
            std::cout << "Usage: redrec [options]\n\n";
            cli::print_help(options, std::cout);
            return ClRes::Exit;
        case cli::Res::BadFormat:
        case cli::Res::BadOption:
        case cli::Res::NotOption:
        case cli::Res::StopParsing:
            std::cerr << "Bad " << (cli_result.res == cli::Res::BadFormat ? "format" : "option") << " at parameter " << cli_result.opti;
            if (cli_result.opti < cli_result.argc) {
                std::cerr << " (" << cli_result.argv[cli_result.opti] << ")";
            }
            if (msg_error.data()) {
                std::cerr << "\n" << msg_error;
                cl_error(msg_error.data());
            }
            std::cerr << "\n";
            return ClRes::Err;
    }


    Inifile ini;
    ini.set<cfg::debug::config>(false);
    configuration_load(ini.configuration_holder(), recorder.config_filename.c_str());


    // VideoParams
    //@{
    if (!recorder.video_params.frame_rate) {
        recorder.video_params.frame_rate = ini.get<cfg::video::framerate>();
    }

    if (recorder.video_params.codec.empty()) {
        recorder.video_params.codec = ini.get<cfg::video::codec_id>();
    }

    recorder.video_params.codec_options = codec_options.data()
        ? codec_options
        : ini.get<cfg::video::ffmpeg_options>();

    recorder.video_params.no_timestamp = ini.get<cfg::video::notimestamp>();
    recorder.video_params.verbosity = ini.get<cfg::debug::ffmpeg>();
    //@}


    // SequencedVideoParams and FullVideoParams
    //@{
    recorder.sequenced_video_params.break_interval = video_break_interval.count()
        ? video_break_interval
        : ini.get<cfg::video::break_interval>();

    recorder.full_video_params.bogus_vlc_frame_rate = ini.get<cfg::video::bogus_vlc_frame_rate>();
    recorder.sequenced_video_params.bogus_vlc_frame_rate = false;
    if (1 == bogus_vlc) {
        recorder.full_video_params.bogus_vlc_frame_rate = true;
        recorder.sequenced_video_params.bogus_vlc_frame_rate = true;
    }
    if (0 == bogus_vlc) {
        recorder.full_video_params.bogus_vlc_frame_rate = false;
        recorder.sequenced_video_params.bogus_vlc_frame_rate = false;
    }
    //@}


    // MetaParams
    //@{
    recorder.log_cliboard_activities = MetaParams::LogClipboardActivities(
        (ini.get<cfg::video::disable_clipboard_log>() & ClipboardLogFlags::meta)
        != ClipboardLogFlags::meta);
    recorder.log_file_system_activities = MetaParams::LogFileSystemActivities(
        (ini.get<cfg::video::disable_file_system_log>() & FileSystemLogFlags::meta)
        != FileSystemLogFlags::meta);
    recorder.log_only_relevant_clipboard_activities = MetaParams::LogOnlyRelevantClipboardActivities(
        ini.get<cfg::mod_rdp::log_only_relevant_clipboard_activities>());
    //@}


    // OcrParams
    //@{
    recorder.ocr_params = OcrParams{
        ini.get<cfg::ocr::version>(),
        ocr::locale::LocaleId(
            static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>())),
        ini.get<cfg::ocr::on_title_bar_only>(),
        ini.get<cfg::ocr::max_unrecog_char_rate>(),
        chunk ? 1s : ini.get<cfg::ocr::interval>(),
        ini.get<cfg::debug::ocr>()
    };
    //@}


    // KbdLogParams
    //@{
    auto const disable_keyboard_log = ini.get<cfg::video::disable_keyboard_log>();
    auto const keyboard_input_fully_masked
      = ini.get<cfg::session_log::keyboard_input_masking_level>()
        == ::KeyboardInputMaskingLevel::fully_masked;

    recorder.wrm_keyboard_log = !keyboard_input_fully_masked
                             && !bool(disable_keyboard_log & KeyboardLogFlags::wrm);
    recorder.meta_keyboard_log = !keyboard_input_fully_masked
                              && !bool(disable_keyboard_log & KeyboardLogFlags::meta);
    //@}


    if (chunk) {
        recorder.capture_flags |= CaptureFlags::video | CaptureFlags::ocr;
        recorder.capture_flags &= ~CaptureFlags::png;
    }

    if (recorder.png_geometry.w) {
        std::cout << "png-geometry: " << recorder.png_geometry.w << "x" << recorder.png_geometry.h << std::endl;
    }

    if (recorder.hash_path.empty()) {
        recorder.hash_path = ini.get<cfg::video::hash_path>().as_string();
    }
    else if (recorder.hash_path.back() != '/') {
        recorder.hash_path += '/';
    }

    if (recorder.mwrm_path.empty()) {
        recorder.mwrm_path = ini.get<cfg::video::record_path>().as_string();
    }
    else if (recorder.mwrm_path.back() != '/') {
        recorder.mwrm_path += '/';
    }

    if (recorder.input_filename.empty()) {
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

        if (input.basename.empty()) {
            recorder.input_basename = "";
            recorder.input_filename = "";
            recorder.infile_extension = ".mwrm";
        }
        else {
            recorder.input_basename = input.basename;
            recorder.infile_extension = (input.extension.size() > 0)
                ? input.extension.c_str()
                : ".mwrm";
            str_assign(recorder.input_filename, input.basename, recorder.infile_extension);
        }

        if (recorder.mwrm_path.back() != '/'){
            recorder.mwrm_path.push_back('/');
        }
        if (recorder.hash_path.back() != '/'){
            recorder.hash_path.push_back('/');
        }
    }
    recorder.full_path = recorder.mwrm_path + recorder.input_filename;

    if (recorder.verbosity) {
        LOG(LOG_INFO, "Input file full_path=\"%s\"", recorder.full_path);
        LOG(LOG_INFO, "Input file base name=\"%s\"", recorder.input_filename);
        LOG(LOG_INFO, "hash_path=\"%s\"", recorder.hash_path);
        LOG(LOG_INFO, "mwrm_path=\"%s\"", recorder.mwrm_path);
    }

    switch (is_encrypted_file(recorder.full_path.c_str())) {
        case IsEncryptedResult::Error: {
            int const errnum = errno;
            auto const mes = str_concat("Input file error: ", strerror(errnum));
            return cl_error(mes.c_str(), -errnum);
        }

        case IsEncryptedResult::IsEncrypted:
            std::cout << "Input file is encrypted.\n";
            recorder.infile_is_encrypted = true;
            break;

        case IsEncryptedResult::IsUnencrypted:
            recorder.infile_is_encrypted = false;
            break;
    }

    recorder.encryption_type = wrm_encryption.get_or(
        recorder.infile_is_encrypted
            ? TraceType::cryptofile
            : TraceType::localfile
    );

    if (!recorder.output_filename.empty()) {
        auto output = ParsePath(recorder.output_filename);
        if (output.directory.empty()){
            recorder.output_filename = str_concat(
                app_path(AppPath::Wrm), '/', recorder.output_filename,
                output.extension.empty() ? ".mwrm"_av : ""_av);
        }
        else if (output.extension.empty()){
            recorder.output_filename += ".mwrm";
        }
        std::cout << "Output file is \"" << recorder.output_filename << "\".\n";
    }

    recorder.play_video_with_corrupted_bitmap
        = ini.get<cfg::video::play_video_with_corrupted_bitmap>();
    recorder.smart_video_cropping = ini.get<cfg::video::smart_video_cropping>();
    recorder.group_id = checked_int{ini.get<cfg::video::capture_groupid>()};
    recorder.file_permissions = ini.get<cfg::video::file_permissions>();
    recorder.wrm_verbosity = safe_cast<RDPSerializerVerbose>(ini.get<cfg::debug::capture>());

    return ClRes::Ok;
}

} // anonymous namespace

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
            std::string_view scmd = argv[arg_used+1];
            command = "redrec"sv == scmd ? 1
                    : "redver"sv == scmd ? 2
                    : "reddec"sv == scmd ? 3
                    : 0;
            if (command) {
                --command;
                ++arg_used;
            }
            // default command is previous one;
        }

        UdevRandom rnd;
        CryptoContext cctx;
        if (hmac_key) {
            cctx.set_hmac_key(CryptoContext::key_data::from_ptr(hmac_key));
        }
        cctx.set_get_trace_key_cb(trace_fn);

        for (auto a : {0, 1}) {
            if (argc <= arg_used + 1) {
                break;
            }

            std::string_view k = argv[arg_used+1];
            if (k.size() != 64) {
                break;
            }

            uint8_t tmp[32];
            // if any character not an hexa digit, ignore option
            if (!hexadecimal_string_to_buffer(k, make_writable_array_view(tmp))) {
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

        RecorderParams rp {};

        switch (parse_command_line_options(argc, argv, rp)) {
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
             && rp.output_filename.empty()
            ) {
                std::cerr << "Missing output filename : use -o filename\n\n";
                return -1;
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

            auto const encryption_mode = rp.infile_is_encrypted ? EncryptionMode::Encrypted
                                                                : EncryptionMode::NotEncrypted;
            auto const mwrm_prefix = str_concat(rp.mwrm_path, rp.input_basename);
            auto const mwrm_filename = str_concat(mwrm_prefix, rp.infile_extension);

            auto mwrm_infos = load_mwrm_file_data(mwrm_filename.c_str(), cctx, encryption_mode);
            auto& wrms = mwrm_infos.wrms;

            if (wrms.empty()) {
                return raise_error_and_log(
                    rp.output_filename, -1, "wrm file not found in mwrm file"_zv
                );
            }

            CaptureTimes capture_times {rp.begin_cap, rp.end_cap};

            // consumes the first wrm
            if (capture_times.begin_cap.count()) {
                auto it = std::find_if(wrms.begin(), wrms.end(), [&](MetaLine const& meta_line){
                    using Seconds = std::chrono::seconds;
                    return Seconds(meta_line.stop_time) > capture_times.begin_cap;
                });
                if (it != wrms.end()) {
                    wrms.erase(wrms.begin(), it);
                }
                else {
                    return raise_error_and_log(
                        rp.output_filename, -1, "no recording on the requested time slot"_zv
                    );
                }
            }

            std::vector<std::string> wrm_filenames;

            if (int r = update_filename_and_check_size(rp.output_filename,
                                                       wrms,
                                                       wrm_filenames,
                                                       rp.mwrm_path,
                                                       rp.ignore_file_size)
            ) {
                return r;
            }

            Rect      crop_rect;
            Dimension max_screen_dim;
            std::unique_ptr<unsigned long long[]> updatable_frame_marker_end_bitset;
            try {
                InMultiCryptoTransport trans(wrm_filenames, cctx, mwrm_infos.encryption_mode);
                auto r = RegionsCapture::compute_regions(
                    trans,
                    rp.smart_video_cropping,
                    rp.full_video || bool(rp.capture_flags & CaptureFlags::video)
                        ? std::chrono::microseconds(1000000L / rp.video_params.frame_rate)
                        : MonotonicTimePoint::duration(),
                    MonotonicTimePoint(capture_times.begin_cap),
                    MonotonicTimePoint(capture_times.end_cap),
                    rp.play_video_with_corrupted_bitmap,
                    ExplicitCRef(program_requested_to_shutdown),
                    safe_cast<FileToGraphic::Verbose>(rp.verbosity));
                crop_rect = r.crop_rect;
                max_screen_dim = r.max_screen_dim;
                updatable_frame_marker_end_bitset = std::move(r.updatable_frame_marker_end.p);
                rp.video_params.updatable_frame_marker_end_bitset_view = {
                    updatable_frame_marker_end_bitset.get(),
                    r.updatable_frame_marker_end.len
                };
            }
            catch (Error const& e) {
                const bool msg_with_error_id = false;
                raise_error(rp.output_filename, e.id, e.errmsg(msg_with_error_id));
            }

            capture_times.adjust_time(
                std::chrono::seconds(wrms.back().start_time),
                std::chrono::seconds(wrms.front().stop_time));

            InMultiCryptoTransport in_wrm_trans(
                std::move(wrm_filenames),
                cctx,
                mwrm_infos.encryption_mode);

            res = replay(std::move(in_wrm_trans),
                         wrms,
                         rp,
                         capture_times,
                         cctx,
                         crop_rect,
                         max_screen_dim,
                         rnd);

            } catch (const Error & e) {
                std::cout << "decrypt failed: with id=" << e.id << std::endl;
            }
        break;

        case 1: { // VERifier
            Error out_error{NO_ERROR};
            switch (get_encryption_scheme_type(cctx, rp.full_path.c_str(), bytes_view{}, &out_error))
            {
                case EncryptionSchemeTypeResult::OldScheme:
                    cctx.old_encryption_scheme = true;
                    [[fallthrough]];
                case EncryptionSchemeTypeResult::NewScheme:
                case EncryptionSchemeTypeResult::NoEncrypted:
                    res = check_encrypted_or_checksumed(
                        rp.input_filename, rp.mwrm_path, rp.hash_path,
                        rp.quick_check, cctx, rp.verbosity
                    );
                    break;
                case EncryptionSchemeTypeResult::Error:
                    break;
            }

            std::cout << "verify " << (res == 0 ? "ok" : "failed") << std::endl;
        }
        break;

        default: // DECrypter
            try {
                InCryptoTransport in_t(cctx, EncryptionMode::Auto);
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
                    case EncryptionSchemeTypeResult::NewScheme:
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

                        char mem[128*1024];
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
                int errnum = errno;
                LOG(LOG_WARNING, "Failed to read file %s [%d: %s]",
                    buffer, errnum, strerror(errnum));
                continue;
            }
            if (unlink(buffer) < 0){
                int errnum = errno;
                LOG(LOG_WARNING, "Failed to remove file %s [%d: %s]",
                    buffer, errnum, strerror(errnum));
            }
        }
    }
}
