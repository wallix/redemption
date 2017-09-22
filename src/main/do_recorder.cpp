/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013-2017
   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen

   redver video verifier program
*/

#include "main/do_recorder.hpp"
#include "main/version.hpp"

#include "system/scoped_crypto_init.hpp"
#include "program_options/program_options.hpp"

#include "capture/flv_params.hpp"
#include "capture/flv_params_from_ini.hpp"
#include "capture/ocr_params.hpp"
#include "capture/png_params.hpp"
#include "capture/wrm_params.hpp"

#include "capture/capture.hpp"
#include "capture/cryptofile.hpp"
#include "capture/save_state_chunk.hpp"

#include "configs/config.hpp"

#include "core/RDP/RDPSerializer.hpp" // RDPSerializer::Verbose
#include "core/RDP/RDPDrawable.hpp"

#include "transport/crypto_transport.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/out_meta_sequence_transport.hpp"

#include "utils/apps/recording_progress.hpp"
#include "utils/chex_to_int.hpp"
#include "utils/fileutils.hpp"
#include "utils/genrandom.hpp"
#include "utils/genfstat.hpp"
#include "utils/log.hpp"
#include "utils/sugar/iter.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/word_identification.hpp"


#include <string>
#include <vector>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>

// opendir/closedir
#include <sys/types.h>
#include <dirent.h>

#include <signal.h>


enum {
    USE_ORIGINAL_COMPRESSION_ALGORITHM = 0xFFFFFFFF
};

enum {
    USE_ORIGINAL_COLOR_DEPTH           = 0xFFFFFFFF
};



enum { QUICK_CHECK_LENGTH = 4096 };


// Compute HmacSha256
// up to check_size or end of file whicherver happens first
// if check_size == 0, checks to eof
// return 0 on success and puts signature in provided buffer
// return -1 if some system error occurs, errno contains actual error
static inline int file_start_hmac_sha256(const char * filename,
                     uint8_t const (&crypto_key)[HMAC_KEY_LENGTH],
                     size_t          check_size,
                     uint8_t (& hash)[SslSha256::DIGEST_LENGTH])
{
    unique_fd file(filename, O_RDONLY);
    int fd = file.fd();
    if (fd < 0) {
        return fd;
    }

    SslHMAC_Sha256 hmac(crypto_key, HMAC_KEY_LENGTH);

    ssize_t ret = 0;
    uint8_t buf[4096] = {};
    size_t  number_of_bytes_read = 0;

    while ((ret = ::read(fd, buf, sizeof(buf)))) {
        if (ret < 0){
            // interruption signal, not really an error
            if (errno == EINTR){
                continue;
            }
            return -1;
        }
        if (check_size && number_of_bytes_read + ret >= check_size){
            hmac.update(buf, check_size - number_of_bytes_read);
            break;
        }
        hmac.update(buf, ret);
        number_of_bytes_read += ret;
    }
    hmac.final(hash);
    return 0;
}

static inline int encryption_type(const std::string & full_filename, CryptoContext & cctx)
{
    uint8_t tmp_buf[4] ={};
    int fd = open(full_filename.c_str(), O_RDONLY);
    if (fd == -1){
        std::cerr << "Input file missing.\n";
        return -1;
    }

    {
        unique_fd file(fd);

        const size_t len = sizeof(tmp_buf);
        size_t remaining_len = len;
        while (remaining_len) {
            ssize_t ret = ::read(fd, &tmp_buf[len - remaining_len], remaining_len);
            if (ret < 0){
                if (ret == 0){
                    std::cerr << "Input file truncated\n";
                    return -1;
                }
                if (errno == EINTR){
                    continue;
                }
                // Error should still be there next time we try to read
                std::cerr << "Input file error\n";
                return -1;
            }
            // We must exit loop or we will enter infinite loop
            remaining_len -= ret;
        }
    }

    const uint32_t magic = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);
    if (magic == WABCRYPTOFILE_MAGIC) {
        Fstat fstat;
        InCryptoTransport in_test(cctx, InCryptoTransport::EncryptionMode::Encrypted, fstat);
        in_test.open(full_filename.c_str());
        try {
            char mem[4096];
            auto len = in_test.partial_read(mem, sizeof(mem));
            (void)len;
        } catch (Error const&) {
            cctx.old_encryption_scheme = 1;
            return 1;
        }
        return 2;
    }
    return 0;
}

void clear_files_flv_meta_png(const char * path, const char * prefix)
{
    struct D {
        DIR * d;

        ~D() { closedir(d); }
        operator DIR * () const { return d; }
    } d{opendir(path)};

    if (d){
//        char static_buffer[8192];
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

        // TODO size_t len = offsetof(struct dirent, d_name) + NAME_MAX + 1 ?
        struct dirent * result;
        while ((result = readdir(d))) {
            if ((0 == strcmp(result->d_name, ".")) || (0 == strcmp(result->d_name, ".."))){
                continue;
            }

            if (strncmp(result->d_name, prefix, prefix_len)){
                continue;
            }

            strncpy(buffer + path_len, result->d_name, file_len);
            const char * eob = buffer + path_len + strlen(result->d_name);
            const bool extension = ((strlen(result->d_name) > 4) && (eob[-4] == '.')
                    && (((eob[-3] == 'f') && (eob[-2] == 'l') && (eob[-1] == 'v'))
                      ||((eob[-3] == 'p') && (eob[-2] == 'n') && (eob[-1] == 'g'))
                      ||((eob[-3] == 'p') && (eob[-2] == 'g') && (eob[-1] == 's'))))
                || (((strlen(result->d_name) > 5) && (eob[-5] == '.')
                    && (eob[-4] == 'm') && (eob[-3] == 'e') && (eob[-2] == 't') && (eob[-1] == 'a')))
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
    else {
        LOG(LOG_WARNING, "Failed to open directory %s [%d: %s]", path, errno, strerror(errno));
    }
}

using std::begin;
using std::end;



// struct RDPChunkedDevice {
//     virtual ~RDPChunkedDevice () {}
//
//     virtual void chunk(uint16_t chunk_type, uint16_t chunk_count, InStream data) = 0;
// };


class ChunkToFile //: public RDPChunkedDevice
{
    CompressionOutTransportBuilder compression_bullder;
    Transport & trans_target;
    Transport & trans;

    const uint8_t wrm_format_version;

    uint16_t info_version = 0;

public:
    ChunkToFile(Transport * trans

               , uint16_t info_width
               , uint16_t info_height
               , uint16_t info_bpp
               , uint16_t info_cache_0_entries
               , uint16_t info_cache_0_size
               , uint16_t info_cache_1_entries
               , uint16_t info_cache_1_size
               , uint16_t info_cache_2_entries
               , uint16_t info_cache_2_size

               , uint16_t info_number_of_cache
               , bool     info_use_waiting_list

               , bool     info_cache_0_persistent
               , bool     info_cache_1_persistent
               , bool     info_cache_2_persistent

               , uint16_t info_cache_3_entries
               , uint16_t info_cache_3_size
               , bool     info_cache_3_persistent
               , uint16_t info_cache_4_entries
               , uint16_t info_cache_4_size
               , bool     info_cache_4_persistent

               , WrmCompressionAlgorithm wrm_compression_algorithm)
    : compression_bullder(*trans, wrm_compression_algorithm)
    , trans_target(*trans)
    , trans(this->compression_bullder.get())
    , wrm_format_version(bool(this->compression_bullder.get_algorithm()) ? 4 : 3)
    {
        if (wrm_compression_algorithm != this->compression_bullder.get_algorithm()) {
            LOG( LOG_WARNING, "compression algorithm %u not fount. Compression disable."
               , static_cast<unsigned>(wrm_compression_algorithm));
        }

        send_meta_chunk(
            this->trans_target
          , this->wrm_format_version

          , info_width
          , info_height
          , info_bpp
          , info_cache_0_entries
          , info_cache_0_size
          , info_cache_1_entries
          , info_cache_1_size
          , info_cache_2_entries
          , info_cache_2_size

          , info_number_of_cache
          , info_use_waiting_list

          , info_cache_0_persistent
          , info_cache_1_persistent
          , info_cache_2_persistent

          , info_cache_3_entries
          , info_cache_3_size
          , info_cache_3_persistent
          , info_cache_4_entries
          , info_cache_4_size
          , info_cache_4_persistent

          , static_cast<unsigned>(this->compression_bullder.get_algorithm())
        );
    }

public:
    void chunk(uint16_t chunk_type, uint16_t chunk_count, InStream stream) /*override*/
    {
        auto wrm_chunk_type = safe_cast<WrmChunkType>(chunk_type);
        switch (wrm_chunk_type)
        {
        case WrmChunkType::META_FILE:
            {
                this->info_version                  = stream.in_uint16_le();
                uint16_t info_width                 = stream.in_uint16_le();
                uint16_t info_height                = stream.in_uint16_le();
                uint16_t info_bpp                   = stream.in_uint16_le();
                uint16_t info_cache_0_entries       = stream.in_uint16_le();
                uint16_t info_cache_0_size          = stream.in_uint16_le();
                uint16_t info_cache_1_entries       = stream.in_uint16_le();
                uint16_t info_cache_1_size          = stream.in_uint16_le();
                uint16_t info_cache_2_entries       = stream.in_uint16_le();
                uint16_t info_cache_2_size          = stream.in_uint16_le();

                uint16_t info_number_of_cache       = 3;
                bool     info_use_waiting_list      = false;

                bool     info_cache_0_persistent    = false;
                bool     info_cache_1_persistent    = false;
                bool     info_cache_2_persistent    = false;

                uint16_t info_cache_3_entries       = 0;
                uint16_t info_cache_3_size          = 0;
                bool     info_cache_3_persistent    = false;
                uint16_t info_cache_4_entries       = 0;
                uint16_t info_cache_4_size          = 0;
                bool     info_cache_4_persistent    = false;

                if (this->info_version > 3) {
                    info_number_of_cache            = stream.in_uint8();
                    info_use_waiting_list           = (stream.in_uint8() ? true : false);

                    info_cache_0_persistent         = (stream.in_uint8() ? true : false);
                    info_cache_1_persistent         = (stream.in_uint8() ? true : false);
                    info_cache_2_persistent         = (stream.in_uint8() ? true : false);

                    info_cache_3_entries            = stream.in_uint16_le();
                    info_cache_3_size               = stream.in_uint16_le();
                    info_cache_3_persistent         = (stream.in_uint8() ? true : false);

                    info_cache_4_entries            = stream.in_uint16_le();
                    info_cache_4_size               = stream.in_uint16_le();
                    info_cache_4_persistent         = (stream.in_uint8() ? true : false);

                    //uint8_t info_compression_algorithm = stream.in_uint8();
                    //REDASSERT(info_compression_algorithm < 3);
                }


                send_meta_chunk(
                    this->trans_target
                  , this->wrm_format_version

                  , info_width
                  , info_height
                  , info_bpp
                  , info_cache_0_entries
                  , info_cache_0_size
                  , info_cache_1_entries
                  , info_cache_1_size
                  , info_cache_2_entries
                  , info_cache_2_size

                  , info_number_of_cache
                  , info_use_waiting_list

                  , info_cache_0_persistent
                  , info_cache_1_persistent
                  , info_cache_2_persistent

                  , info_cache_3_entries
                  , info_cache_3_size
                  , info_cache_3_persistent
                  , info_cache_4_entries
                  , info_cache_4_size
                  , info_cache_4_persistent

                  , static_cast<unsigned>(this->compression_bullder.get_algorithm())
                );
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
                this->trans.send(payload.get_data(), payload.get_offset());
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
                timeval record_now;
                stream.in_timeval_from_uint64le_usec(record_now);
                this->trans_target.timestamp(record_now);
            }
            REDEMPTION_CXX_FALLTHROUGH;
        default:
            {
                send_wrm_chunk(this->trans, wrm_chunk_type, stream.get_capacity(), chunk_count);
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
    uint32_t chunk_size;
    uint16_t chunk_type;
    uint16_t chunk_count;

    ChunkToFile * consumer;

public:
    timeval record_now;

    bool meta_ok;

    uint16_t info_version;
    uint16_t info_width;
    uint16_t info_height;
    uint16_t info_bpp;
    uint16_t info_number_of_cache;
    bool     info_use_waiting_list;
    uint16_t info_cache_0_entries;
    uint16_t info_cache_0_size;
    bool     info_cache_0_persistent;
    uint16_t info_cache_1_entries;
    uint16_t info_cache_1_size;
    bool     info_cache_1_persistent;
    uint16_t info_cache_2_entries;
    uint16_t info_cache_2_size;
    bool     info_cache_2_persistent;
    uint16_t info_cache_3_entries;
    uint16_t info_cache_3_size;
    bool     info_cache_3_persistent;
    uint16_t info_cache_4_entries;
    uint16_t info_cache_4_size;
    bool     info_cache_4_persistent;
    WrmCompressionAlgorithm info_compression_algorithm;

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
        // variables used to read batch of orders "chunks"
        , chunk_size(0)
        , chunk_type(0)
        , chunk_count(0)
        , consumer(nullptr)
        , meta_ok(false)
        , info_version(0)
        , info_width(0)
        , info_height(0)
        , info_bpp(0)
        , info_number_of_cache(0)
        , info_use_waiting_list(true)
        , info_cache_0_entries(0)
        , info_cache_0_size(0)
        , info_cache_0_persistent(false)
        , info_cache_1_entries(0)
        , info_cache_1_size(0)
        , info_cache_1_persistent(false)
        , info_cache_2_entries(0)
        , info_cache_2_size(0)
        , info_cache_2_persistent(false)
        , info_cache_3_entries(0)
        , info_cache_3_size(0)
        , info_cache_3_persistent(false)
        , info_cache_4_entries(0)
        , info_cache_4_size(0)
        , info_cache_4_persistent(false)
        , info_compression_algorithm(WrmCompressionAlgorithm::no_compression)
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
                auto const buf_sz = FileToGraphic::HEADER_SIZE;
                unsigned char buf[buf_sz];
                this->trans->recv_boom(buf, buf_sz);
                InStream header(buf);
                this->chunk_type  = header.in_uint16_le();
                this->chunk_size  = header.in_uint32_le();
                this->chunk_count = header.in_uint16_le();
            }

            if (this->chunk_size > 65536) {
                LOG(LOG_INFO,"chunk_size (%u) > 65536", this->chunk_size);
                return false;
            }
            this->stream = InStream(this->stream_buf, 0);   // empty stream
            auto const ssize = this->chunk_size - FileToGraphic::HEADER_SIZE;
            if (ssize > 0) {
                auto const size = size_t(ssize);
                this->trans->recv_boom(this->stream_buf, size);
                this->stream = InStream(this->stream_buf, size);
            }
        }
        catch (Error const & e) {
            if (e.id == ERR_TRANSPORT_OPEN_FAILED) {
                throw;
            }

            if (bool(this->verbose)) {
                LOG(LOG_INFO, "receive error %u : end of transport", e.id);
            }
            // receive error, end of transport
            return false;
        }

        return true;
    }

    void interpret_chunk() {
        switch (safe_cast<WrmChunkType>(this->chunk_type)) {
        case WrmChunkType::META_FILE:
            this->info_version                   = this->stream.in_uint16_le();
            this->info_width                     = this->stream.in_uint16_le();
            this->info_height                    = this->stream.in_uint16_le();
            this->info_bpp                       = this->stream.in_uint16_le();
            this->info_cache_0_entries           = this->stream.in_uint16_le();
            this->info_cache_0_size              = this->stream.in_uint16_le();
            this->info_cache_1_entries           = this->stream.in_uint16_le();
            this->info_cache_1_size              = this->stream.in_uint16_le();
            this->info_cache_2_entries           = this->stream.in_uint16_le();
            this->info_cache_2_size              = this->stream.in_uint16_le();

            if (this->info_version <= 3) {
                this->info_number_of_cache       = 3;
                this->info_use_waiting_list      = false;

                this->info_cache_0_persistent    = false;
                this->info_cache_1_persistent    = false;
                this->info_cache_2_persistent    = false;
            }
            else {
                this->info_number_of_cache       = this->stream.in_uint8();
                this->info_use_waiting_list      = (this->stream.in_uint8() ? true : false);

                this->info_cache_0_persistent    = (this->stream.in_uint8() ? true : false);
                this->info_cache_1_persistent    = (this->stream.in_uint8() ? true : false);
                this->info_cache_2_persistent    = (this->stream.in_uint8() ? true : false);

                this->info_cache_3_entries       = this->stream.in_uint16_le();
                this->info_cache_3_size          = this->stream.in_uint16_le();
                this->info_cache_3_persistent    = (this->stream.in_uint8() ? true : false);

                this->info_cache_4_entries       = this->stream.in_uint16_le();
                this->info_cache_4_size          = this->stream.in_uint16_le();
                this->info_cache_4_persistent    = (this->stream.in_uint8() ? true : false);

                this->info_compression_algorithm = static_cast<WrmCompressionAlgorithm>(this->stream.in_uint8());
                REDASSERT(is_valid_enum_value(this->info_compression_algorithm));
                if (!is_valid_enum_value(this->info_compression_algorithm)) {
                    this->info_compression_algorithm = WrmCompressionAlgorithm::no_compression;
                }

                // re-init
                this->trans = &this->compression_builder.reset(
                    *this->trans_source, this->info_compression_algorithm
                );
            }

            this->stream.rewind();

            if (!this->meta_ok) {
                this->meta_ok = true;
            }
            break;
        case WrmChunkType::RESET_CHUNK:
            this->info_compression_algorithm = WrmCompressionAlgorithm::no_compression;

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
        CryptoContext cctx_no_crypto;

        OutMetaSequenceTransport trans(
            ini.get<cfg::globals::trace_type>() == TraceType::cryptofile ? cctx : cctx_no_crypto,
            rnd,
            fstat,
            outfile_path.c_str(),
            ini.get<cfg::video::hash_path>().c_str(),
            outfile_basename.c_str(),
            begin_record,
            player.info_width,
            player.info_height,
            ini.get<cfg::video::capture_groupid>(),
            nullptr
        );
        {
            ChunkToFile recorder(
                &trans
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
              , ini.get<cfg::video::wrm_compression_algorithm>()
            );

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


struct HashHeader {
    unsigned version;
};

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

    if (verbose) {
        LOG(LOG_INFO, "%s", infile_version == WrmVersion::v1 ? "Hash data v1" : "Hash data v2 or higher");
    }
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

        uint8_t hash[MD_HASH::DIGEST_LENGTH]{};
        if (file_start_hmac_sha256(filename.c_str(), hmac_key, quick?QUICK_CHECK_LENGTH:0, hash) < 0) {
            std::cerr << "Error reading file \"" << filename << "\"\n" << std::endl;
            return false;
        }

        if (0 != memcmp(hash, quick?metadata.hash1:metadata.hash2, MD_HASH::DIGEST_LENGTH)){
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

class dorecorder_ofile_buf_out
{
    int fd;
public:
    dorecorder_ofile_buf_out() : fd(-1) {}
    ~dorecorder_ofile_buf_out()
    {
        this->close();
    }

    int open(const char * filename, mode_t mode)
    {
        this->close();
        this->fd = ::open(filename, O_WRONLY | O_CREAT, mode);
        return this->fd;
    }

    int close()
    {
        if (this->is_open()) {
            const int ret = ::close(this->fd);
            this->fd = -1;
            return ret;
        }
        return 0;
    }

    ssize_t write(const void * data, size_t len)
    {
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            ssize_t ret = ::write(this->fd,
                static_cast<const char*>(data) + total_sent, remaining_len);
            if (ret <= 0){
                if (errno == EINTR){
                    continue;
                }
                return -1;
            }
            remaining_len -= ret;
            total_sent += ret;
        }
        return total_sent;
    }

    bool is_open() const noexcept
    { return -1 != this->fd; }

    int flush() const
    { return 0; }
};

static inline int dorecorder_write_filename(dorecorder_ofile_buf_out & writer, const char * filename)
{
    auto pfile = filename;
    auto epfile = filename;
    for (; *epfile; ++epfile) {
        if (*epfile == '\\') {
            ssize_t len = epfile - pfile + 1;
            auto res = writer.write(pfile, len);
            if (res < len) {
                return res < 0 ? res : 1;
            }
            pfile = epfile;
        }
        if (*epfile == ' ') {
            ssize_t len = epfile - pfile;
            auto res = writer.write(pfile, len);
            if (res < len) {
                return res < 0 ? res : 1;
            }
            res = writer.write("\\", 1u);
            if (res < 1) {
                return res < 0 ? res : 1;
            }
            pfile = epfile;
        }
    }

    if (pfile != epfile) {
        ssize_t len = epfile - pfile;
        auto res = writer.write(pfile, len);
        if (res < len) {
            return res < 0 ? res : 1;
        }
    }

    return 0;
}

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
    if (verbose) {
        LOG(LOG_INFO, "Load file hash. Is encrypted %d. Is checksumed: %d",
            infile_is_encrypted, reader.get_header().has_checksum);
    }
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

//        LOG(LOG_INFO, "checking part %s", full_part_filename);


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
        dorecorder_ofile_buf_out mwrm_file_cp;
        if (mwrm_file_cp.open(full_mwrm_filename_tmp.c_str(), S_IRUSR | S_IRGRP | S_IWUSR) < 0) {
            LOG(LOG_ERR, "Failed to open meta file %s", full_mwrm_filename_tmp);
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
        }
        local_auto_remove auto_remove{full_mwrm_filename_tmp.c_str()};
        if (chmod(full_mwrm_filename_tmp.c_str(), S_IRUSR | S_IRGRP) == -1) {
            LOG( LOG_ERR, "can't set file %s mod to %s : %s [%d]"
                , full_mwrm_filename_tmp
                , "u+r, g+r"
                , strerror(errno), errno);
            return 1;
        }

        // copy mwrm headers
        {
            InCryptoTransport mwrm_file(cctx, EncryptionMode::NotEncrypted, fstat);
            mwrm_file.open(full_mwrm_filename.c_str());
            LineReader line_reader(mwrm_file);

            // v2, w h, nochecksum, blank, blank
            for (int i = 0; i < 5; ++i) {
                if (Transport::Read::Eof == line_reader.next_line()) {
                    throw Error(ERR_TRANSPORT_READ_FAILED, 0);
                }
                auto av = line_reader.get_buf();
                if (mwrm_file_cp.write(av.data(), av.size()) != ssize_t(av.size())) {
                    throw Error(ERR_TRANSPORT_WRITE_FAILED, 0);
                }
            }
        }


        // TODO: this is much too complicated, use factorized code from wrm_capture to compute hash file
        for (MetaLine2CtxForRewriteStat & ctx : meta_line_ctx_list) {
            struct stat sb;
            if (lstat(ctx.wrm_filename.c_str(), &sb) < 0) {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, 0);
            }

            const char * filename = ctx.filename.c_str();
            auto start_sec = ctx.start_time;
            auto stop_sec = ctx.stop_time;
            int err = dorecorder_write_filename(mwrm_file_cp, filename);
            if (err){
                throw Error(ERR_TRANSPORT_WRITE_FAILED, 0);
            }

            using ull = unsigned long long;
            using ll = long long;
            char mes[
                (std::numeric_limits<ll>::digits10 + 1 + 1) * 8 +
                (std::numeric_limits<ull>::digits10 + 1 + 1) * 2 +
                MD_HASH::DIGEST_LENGTH*4 + 1 +
                2
            ];
            ssize_t len = std::sprintf(
                mes,
                " %lld %llu %lld %lld %llu %lld %lld %lld",
                ll(sb.st_size),
                ull(sb.st_mode),
                ll(sb.st_uid),
                ll(sb.st_gid),
                ull(sb.st_dev),
                ll(sb.st_ino),
                ll(sb.st_mtim.tv_sec),
                ll(sb.st_ctim.tv_sec)
            );
            len += std::sprintf(
                mes + len,
                " %lld %lld",
                ll(start_sec),
                ll(stop_sec)
            );

            char * p = mes + len;
            *p++ = '\n';

            ssize_t res = mwrm_file_cp.write(mes, p-mes);

            if (res < p-mes) {
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
        dorecorder_ofile_buf_out hash_file_cp;

        local_auto_remove auto_remove{full_hash_path_tmp.c_str()};

        char const * hash_filename = full_hash_path_tmp.c_str();
        char const * meta_filename = full_mwrm_filename.c_str();

        char path[1024] = {};
        char basename[1024] = {};
        char extension[256] = {};
        char filename[2048] = {};

        canonical_path(
            meta_filename,
            path, sizeof(path),
            basename, sizeof(basename),
            extension, sizeof(extension)
        );

        snprintf(filename, sizeof(filename), "%s%s", basename, extension);

        if (hash_file_cp.open(hash_filename, S_IRUSR|S_IRGRP) >= 0) {
            char header[] = "v2\n\n\n";
            hash_file_cp.write(header, sizeof(header)-1);

            struct stat stat;
            int err = ::stat(meta_filename, &stat);
            if (!err) {
               err = dorecorder_write_filename(hash_file_cp, filename);
                if (err) {
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                    return 1;
                }

                using ull = unsigned long long;
                using ll = long long;
                char mes[
                    (std::numeric_limits<ll>::digits10 + 1 + 1) * 8 +
                    (std::numeric_limits<ull>::digits10 + 1 + 1) * 2 +
                    MD_HASH::DIGEST_LENGTH*4 + 1 +
                    2
                ];
                ssize_t len = std::sprintf(
                    mes,
                    " %lld %llu %lld %lld %llu %lld %lld %lld",
                    ll(stat.st_size),
                    ull(stat.st_mode),
                    ll(stat.st_uid),
                    ll(stat.st_gid),
                    ull(stat.st_dev),
                    ll(stat.st_ino),
                    ll(stat.st_mtim.tv_sec),
                    ll(stat.st_ctim.tv_sec)
                );

                char * p = mes + len;
                *p++ = '\n';

                ssize_t res = hash_file_cp.write(mes, p-mes);

                if (res < p-mes) {
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", filename, err);
                    return 1;
                }
            }
            if (!err) {
                err = hash_file_cp.close(/*hash*/);
            }
            if (err) {
                LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                return 1;
            }
        }
        else {
            int e = errno;
            LOG(LOG_ERR, "Open to transport failed: code=%d", e);
            errno = e;
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
        next_wrm();
    }
    unsigned const result = in_wrm_trans.get_seqno();
    try {
        do {
            end_record.tv_sec = in_wrm_trans.end_chunk_time();
            next_wrm();
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
  , const char * input_filename, const char * infile_extension, bool is_encrypted
) {
    std::vector<std::string> files;

    char infile_fullpath[2048];
    if (is_encrypted) {
        std::snprintf(infile_fullpath, sizeof(infile_fullpath), "%s%s%s", hash_path, input_filename, infile_extension);
        files.push_back(infile_fullpath);
    }
    std::snprintf(infile_fullpath, sizeof(infile_fullpath), "%s%s%s", infile_path, input_filename, infile_extension);
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
static void raise_error(UpdateProgressData::Format pgs_format, std::string const & output_filename, int code, const char * message) {
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

    UpdateProgressData update_progress_data(pgs_format, progress_filename, 0, 0, 0, 0);

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

inline int replay(std::string & infile_path, std::string & input_basename, std::string & infile_extension,
                  std::string & hash_path,
                  CaptureFlags & capture_flags,
                  UpdateProgressData::Format pgs_format,
                  bool chunk,
                  unsigned ocr_version,
                  std::string & output_filename,
                  int64_t begin_cap,
                  int64_t end_cap,
                  PngParams & png_params,
                  FlvParams & flv_params,
                  int wrm_color_depth,
                  uint32_t wrm_frame_interval,
                  uint32_t wrm_break_interval,
                  bool const no_timestamp,
                  bool infile_is_encrypted,
                  uint32_t order_count,
                  bool show_file_metadata,
                  bool show_statistics,
                  uint32_t clear,
                  bool full_video,
                  bool remove_input_file,
                  int wrm_compression_algorithm,
                  uint32_t flv_break_interval,
                  TraceType encryption_type,
                  Inifile & ini, CryptoContext & cctx,
                  Random & rnd, Fstat & fstat,
                  uint32_t verbose)
{
    char infile_prefix[4096];
    std::snprintf(infile_prefix, sizeof(infile_prefix), "%s%s", infile_path.c_str(), input_basename.c_str());
    ini.set<cfg::video::hash_path>(hash_path);

    ini.set<cfg::video::frame_interval>(std::chrono::duration<unsigned int, std::centi>{wrm_frame_interval});
    ini.set<cfg::video::break_interval>(std::chrono::seconds{wrm_break_interval});
    ini.set<cfg::video::flv_break_interval>(std::chrono::seconds{flv_break_interval});
    ini.set<cfg::globals::trace_type>(encryption_type);
    ini.set<cfg::video::rt_display>(bool(capture_flags & CaptureFlags::png));

    ini.set<cfg::globals::capture_chunk>(chunk);
    ini.set<cfg::ocr::version>(ocr_version == 2 ? OcrVersion::v2 : OcrVersion::v1);

    if (chunk){
        ini.get_ref<cfg::video::disable_keyboard_log>() &= ~KeyboardLogFlags::meta;
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

            time_t start_time = 0;

            if (Transport::Read::Ok == mwrm_reader.read_meta_line(meta_line)) {
                start_time = meta_line.start_time;
                while (Transport::Read::Ok == mwrm_reader.read_meta_line(meta_line)) {
                }

                auto duration = meta_line.stop_time - start_time;

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
        if (e.id == static_cast<unsigned>(ERR_TRANSPORT_NO_MORE_DATA)) {
            std::cerr << "Asked time not found in mwrm file\n";
        }
        else {
            std::cerr << "Error: " << e.errmsg() << std::endl;
        }
        const bool msg_with_error_id = false;
        raise_error(pgs_format, output_filename, e.id, e.errmsg(msg_with_error_id));
        return -1;
    };

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
        bool test = (
                bool(capture_flags & CaptureFlags::flv)
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
            for (unsigned i = 1; i < file_count ; i++) {
                in_wrm_trans.next();
            }

            LOG(LOG_INFO, "player begin_capture = %ld", begin_capture.tv_sec);
            FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(verbose));

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
                        std::cout << "Output file path: "
                                    << outfile_path << outfile_basename << outfile_extension
                                    << '\n' << std::endl;
                    }

                    if (clear == 1) {
                        clear_files_flv_meta_png(outfile_path, outfile_basename);
                    }

                    ini.set<cfg::video::wrm_compression_algorithm>(
                        (wrm_compression_algorithm == static_cast<int>(USE_ORIGINAL_COMPRESSION_ALGORITHM))
                        ? player.info_compression_algorithm
                        : static_cast<WrmCompressionAlgorithm>(wrm_compression_algorithm)
                    );

                    if (wrm_color_depth == static_cast<int>(USE_ORIGINAL_COLOR_DEPTH)) {
                        wrm_color_depth = player.info_bpp;
                    }

                    {
                        ini.set<cfg::video::hash_path>(outfile_path);
                        ini.set<cfg::video::record_tmp_path>(outfile_path);
                        ini.set<cfg::video::record_path>(outfile_path);

                        ini.set<cfg::globals::movie_path>(&output_filename[0]);

                        char progress_filename[4096];
                        std::snprintf( progress_filename, sizeof(progress_filename), "%s%s.pgs"
                                , outfile_path, outfile_basename);
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
                                    get_percent(png_params.png_width, player.screen_rect.cx),
                                    get_percent(png_params.png_height, player.screen_rect.cy)
                                );
                            //std::cout << "zoom: " << zoom << '%' << std::endl;
                        }

                        ini.set<cfg::globals::video_quality>(flv_params.video_quality);
                        ini.set<cfg::globals::codec_id>(flv_params.codec);
                        flv_params = flv_params_from_ini(
                            player.screen_rect.cx, player.screen_rect.cy, ini);
                        flv_params.no_timestamp = no_timestamp;

                        RDPSerializer::Verbose wrm_verbose
                            = to_verbose_flags(ini.get<cfg::debug::capture>())
                            | (ini.get<cfg::debug::primary_orders>()
                                ? RDPSerializer::Verbose::primary_orders
                                : RDPSerializer::Verbose::none)
                            | (ini.get<cfg::debug::secondary_orders>()
                                ? RDPSerializer::Verbose::secondary_orders
                                : RDPSerializer::Verbose::none)
                            | (ini.get<cfg::debug::bitmap_update>()
                                ? RDPSerializer::Verbose::bitmap_update
                                : RDPSerializer::Verbose::none)
                        ;

                        WrmCompressionAlgorithm wrm_compression_algorithm = ini.get<cfg::video::wrm_compression_algorithm>();
                        std::chrono::duration<unsigned int, std::ratio<1l, 100l> > wrm_frame_interval = ini.get<cfg::video::frame_interval>();
                        std::chrono::seconds wrm_break_interval = ini.get<cfg::video::break_interval>();

                        const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>().c_str();
                        const char * record_path = record_tmp_path;

                        bool capture_wrm = bool(capture_flags & CaptureFlags::wrm);
                        bool capture_png = bool(capture_flags & CaptureFlags::png) && (png_params.png_limit > 0);
                        bool capture_pattern_checker = false;

                        bool capture_ocr = bool(capture_flags & CaptureFlags::ocr)
                                            || capture_pattern_checker;
                        bool capture_flv = bool(capture_flags & CaptureFlags::flv);
                        bool capture_flv_full = full_video;
                        bool capture_meta = capture_ocr;
                        bool capture_kbd = false;

                        OcrParams ocr_params = {
                            ini.get<cfg::ocr::version>(),
                            ocr::locale::LocaleId(
                                static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>())),
                            ini.get<cfg::ocr::on_title_bar_only>(),
                            ini.get<cfg::ocr::max_unrecog_char_rate>(),
                            ini.get<cfg::ocr::interval>(),
                            ini.get<cfg::debug::ocr>()
                        };

                        if (ini.get<cfg::debug::capture>()) {
                            LOG(LOG_INFO, "Enable capture:  %s%s  kbd=%d %s%s%s  ocr=%d %s",
                                capture_wrm ?"wrm ":"",
                                capture_png ?"png ":"",
                                capture_kbd ? 1 : 0,
                                capture_flv ?"flv ":"",
                                capture_flv_full ?"flv_full ":"",
                                capture_pattern_checker ?"pattern ":"",
                                capture_ocr ? (ocr_params.ocr_version == OcrVersion::v2 ? 2 : 1) : 0,
                                capture_meta?"meta ":""
                            );
                        }

                        const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
                        const char * hash_path = ini.get<cfg::video::hash_path>().c_str();
                        const char * movie_path = ini.get<cfg::globals::movie_path>().c_str();

                        char path[1024];
                        char basename[1024];
                        char extension[128];
                        strcpy(path, app_path(AppPath::Wrm));     // default value, actual one should come from movie_path
                        strcat(path, "/");
                        strcpy(basename, movie_path);
                        strcpy(extension, "");          // extension is currently ignored

                        if (!canonical_path(movie_path, path, sizeof(path), basename, sizeof(basename), extension, sizeof(extension))
                        ) {
                            LOG(LOG_ERR, "Buffer Overflowed: Path too long");
                            throw Error(ERR_RECORDER_FAILED_TO_FOUND_PATH);
                        }

                        LOG(LOG_INFO, "canonical_path : %s%s%s\n", path, basename, extension);

                        // PngParams
                        png_params.report_message = nullptr;
                        png_params.record_tmp_path = record_tmp_path;
                        png_params.basename = basename;
                        png_params.groupid = groupid;
                        png_params.remote_program_session = false;
                        png_params.rt_display = ini.get<cfg::video::rt_display>();

                        MetaParams meta_params{
                            MetaParams::EnableSessionLog::No,
                            MetaParams::HideNonPrintable::No
                        };
                        KbdLogParams kbdlog_params;
                        PatternParams pattern_params{
                            ini.get<cfg::context::pattern_notify>().c_str(),
                            ini.get<cfg::context::pattern_kill>().c_str(),
                            ini.get<cfg::debug::capture>()
                        };
                        SequencedVideoParams sequenced_video_params;
                        FullVideoParams full_video_params;

                        cctx.set_trace_type(ini.get<cfg::globals::trace_type>());

                        WrmParams wrm_params(
                            wrm_color_depth,
                            cctx,
                            rnd,
                            fstat,
                            record_path,
                            hash_path,
                            basename,
                            groupid,
                            wrm_frame_interval,
                            wrm_break_interval,
                            wrm_compression_algorithm,
                            bool(ini.get<cfg::video::disable_keyboard_log>()
                                & KeyboardLogFlags::wrm),
                            uint32_t(wrm_verbose) // TODO
                        );

                        bool syslog_keyboard_log = bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::syslog);
                        bool session_log_enabled = false;
                        bool keyboard_fully_masked = ini.get<cfg::session_log::keyboard_input_masking_level>()
                            != ::KeyboardInputMaskingLevel::fully_masked;
                        bool meta_keyboard_log = bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::meta);

                        RDPDrawable rdp_drawable{
                            player.screen_rect.cx, player.screen_rect.cy};

                        // std::optional<Capture> storage;
                        class CaptureStorage
                        {
                            union U {
                                char dummy;
                                Capture capture;

                                U() : dummy(){}
                                ~U() {}
                            } u;
                            bool is_loaded = false;

                        public:
                            void * get_storage()
                            {
                                this->is_loaded = true;
                                return &this->u.capture;
                            }

                            ~CaptureStorage()
                            {
                                if (this->is_loaded) {
                                    this->u.capture.~Capture();
                                }
                            }
                        } storage;

                        auto set_capture_consumer = [&](timeval const & now) {
                            auto * capture = new(storage.get_storage()) Capture(
                                  capture_wrm, wrm_params
                                , capture_png, png_params
                                , capture_pattern_checker, pattern_params
                                , capture_ocr, ocr_params
                                , capture_flv, sequenced_video_params
                                , capture_flv_full, full_video_params
                                , capture_meta, meta_params
                                , capture_kbd, kbdlog_params
                                , basename
                                , now
                                , player.screen_rect.cx
                                , player.screen_rect.cy
                                , record_tmp_path
                                , record_path
                                , groupid
                                , flv_params
                                , nullptr
                                , &update_progress_data
                                , syslog_keyboard_log
                                , session_log_enabled
                                , keyboard_fully_masked
                                , meta_keyboard_log
                                , Rect()
                                , &rdp_drawable
                                );

                            player.clear_consumer();
                            player.add_consumer(capture, capture, capture, capture, capture);
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

                            CaptureMaker(decltype(lazy_capture) & load_capture)
                            : load_capture(load_capture)
                            {}

                            decltype(lazy_capture) & load_capture;
                        };
                        CaptureMaker capture_maker(lazy_capture);

                        if (begin_capture.tv_sec) {
                            player.add_consumer(
                                &rdp_drawable, nullptr, nullptr, nullptr, &capture_maker);
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
    std::string config_filename;
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
    PngParams png_params = {0, 0, std::chrono::seconds{60}, 100, 0, false , nullptr, nullptr, nullptr, 0, false, false};
    FlvParams flv_params;

    // flv output options
    bool full_video; // create full video
    uint32_t    flv_break_interval = 10*60;
    std::string flv_quality;

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
    bool infile_is_encrypted = 0;
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

    program_options::options_description desc({
        {'h', "help", "produce help message"},
        {'v', "version", "show software version"},
        {'o', "output-file", &recorder.output_filename, "output base filename"},
        {'i', "input-file", &recorder.input_filename, "input base filename"},

        {'H', "hash-path",  &recorder.hash_path, "output hash dirname (if empty, use hash_path of ini)"},
        {'M', "mwrm-path",  &recorder.mwrm_path,         "mwrm file path"       },

        // verifier options
        {'Q', "quick",   "quick check only"},
        {'S', "ignore-stat-info", "ignore stat info data mismatch" },
        {'U', "update-stat-info", "update stat info data mismatch "
                                  "(only if not checksum and no encrypted)" },

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

        {"clear", &recorder.clear, "clear old capture files with same prefix (default on)"},
        {"verbose", &verbose, "more logs"},
        {"zoom", &recorder.png_params.zoom, "scaling factor for png capture (default 100%)"},
        {'g', "png-geometry", &png_geometry, "png capture geometry (Ex. 160x120)"},
        {'m', "meta", "show file metadata"},
        {'s', "statistics", "show statistics"},

        {'z', "compression", &wrm_compression_algorithm, "wrm compression algorithm (default=original, none, gzip, snappy)"},
        {'d', "color-depth", &color_depth,           "wrm color depth (default=original, 16, 24)"},
        {'y', "encryption",  &recorder.wrm_encryption,            "wrm encryption (default=original, enable, disable)"},

        {"remove-input-file", "remove input file"},

        {"config-file", &recorder.config_filename, "used another ini file"},

        {'a', "flvbreakinterval", &recorder.flv_break_interval, "number of seconds between splitting flv files (by default, one flv every 10 minutes)"},

        {'q', "flv-quality", &recorder.flv_quality, "flv quality (high, medium, low)"},

        {"ocr-version", &recorder.ocr_version, "version 1 or 2"},

        {"video-codec", &recorder.flv_params.codec, "ffmpeg video codec id (flv, mp4, etc)"},

        {"json-pgs", "use json format to .pgs file"},
    });

    auto options = program_options::parse_command_line(argc, argv, desc);

    const char * copyright_notice = "ReDemPtion " VERSION ".\n"
        "Copyright (C) Wallix 2010-2016.\n"
        "Christophe Grosjean, Jonathan Poelen, Raphael Zhou.";

    if (options.count("help") > 0) {
        std::cout << copyright_notice;
        std::cout << "\n\nUsage: redrec [options]\n\n";
        // TODO error code description
        std::cout << desc << "\n\n";
        return ClRes::Exit;
    }

    if (options.count("version") > 0) {
        std::cout << copyright_notice << std::endl << std::endl;
        return ClRes::Exit;
    }

    if (options.count("config-file") > 0) {
        configuration_load(ini.configuration_holder(), recorder.config_filename);
    }
    else {
        recorder.config_filename = app_path(AppPath::CfgIni);
    }

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

    recorder.flv_params.video_quality = Level::high;
    recorder.chunk = options.count("chunk") > 0;
    recorder.capture_flags
      = (                   (options.count("wrm") > 0)
        ? CaptureFlags::wrm : CaptureFlags::none)
      | ((recorder.chunk || (options.count("png") > 0))
        ? CaptureFlags::png : CaptureFlags::none)
      | ((recorder.chunk || (options.count("video") > 0))
        ? CaptureFlags::flv : CaptureFlags::none)
      | ((recorder.chunk || (options.count("ocr") > 0))
        ? CaptureFlags::ocr : CaptureFlags::none);

    if (options.count("flv-quality") > 0) {
            if (0 == strcmp(recorder.flv_quality.c_str(), "high")) {
            recorder.flv_params.video_quality = Level::high;
        }
        else if (0 == strcmp(recorder.flv_quality.c_str(), "low")) {
            recorder.flv_params.video_quality = Level::low;
        }
        else  if (0 == strcmp(recorder.flv_quality.c_str(), "medium")) {
            recorder.flv_params.video_quality = Level::medium;
        }
        else {
            std::cerr << "Unknown video quality" << std::endl;
            return ClRes::Err;
        }
    }

    recorder.remove_input_file  = (options.count("remove-input-file") > 0);

    if (options.count("color-depth") > 0){
        recorder.wrm_color_depth = (color_depth == "16") ? 16
                                 : (color_depth == "24") ? 24
                                 : (color_depth == "original") ? static_cast<int>(USE_ORIGINAL_COLOR_DEPTH)
                                 : 0;
        if (!recorder.wrm_color_depth){
            std::cerr << "Unknown wrm color depth\n\n";
            return ClRes::Err;
        }
    }

    if (options.count("png_interval") > 0){
        recorder.png_params.png_interval = std::chrono::seconds{png_interval};
    }

    if ((options.count("zoom") > 0)
    && (options.count("png-geometry") > 0)) {
        std::cerr << "Conflicting options : --zoom and --png-geometry\n\n";
        return ClRes::Err;
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
            return ClRes::Err;
        }
        recorder.png_params.png_width  = png_w;
        recorder.png_params.png_height = png_h;
        std::cout << "png-geometry: " << recorder.png_params.png_width << "x" << recorder.png_params.png_height << std::endl;
    }

    //recorder.flv_params.video_codec = "flv";

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
            std::cerr << "Unknown wrm compression algorithm\n\n";
            return ClRes::Err;
        }
    }

    if (options.count("hash-path") > 0){
        if (recorder.hash_path.c_str()[0] == 0) {
            std::cerr << "Missing hash-path : use -h path\n\n";
            return ClRes::Err;
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
            std::cerr << "Missing mwrm-path : use -m path\n\n";
            return ClRes::Err;
        }
    }
    else {
        recorder.mwrm_path = ini.get<cfg::video::record_path>().c_str();
    }

    if (recorder.input_filename.c_str()[0] == 0) {
        std::cerr << "Missing input mwrm file name : use -i filename\n\n";
        return ClRes::Err;
    }

    // Input path rule is as follow:
    // -----------------------------
    //  default serach directory for mwrm is given in config file
    // if --mwrm-path is provided on command line it will be preferably used as default
    // if -i has a path component it will be used instead of mwrm-path
    // if relative all command lines path are relative to current working directory

    {
        char temp_path[1024]     = {};
        char temp_basename[1024] = {};
        char temp_extension[256] = {};

        canonical_path(recorder.input_filename.c_str(), temp_path, sizeof(temp_path), temp_basename, sizeof(temp_basename), temp_extension, sizeof(temp_extension));

        if (strlen(temp_path) > 0) {
            recorder.mwrm_path       = temp_path;
        }

        recorder.input_basename = "";
        recorder.input_filename = "";
        recorder.infile_extension = ".mwrm";
        if (strlen(temp_basename) > 0) {
            recorder.input_basename  = temp_basename;
            recorder.input_filename  = temp_basename;
            recorder.infile_extension = (strlen(temp_extension) > 0)?temp_extension:".mwrm";
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
        std::cerr << "Input file is missing.\n";
        return ClRes::Err;
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
            std::cerr << "Unknown wrm encryption parameter\n\n";
            return ClRes::Err;
        }
    }

    recorder.full_video = (options.count("full") > 0);
    recorder.show_file_metadata = (options.count("meta"             ) > 0);
    recorder.show_statistics    = (options.count("statistics"       ) > 0);

    if (recorder.output_filename.size()) {
        std::string directory = app_path(AppPath::Wrm); directory += "/";
        std::string filename                ;
        std::string extension = ".mwrm"     ;

        ParsePath(recorder.output_filename.c_str(), directory, filename, extension);
        MakePath(recorder.output_filename, directory.c_str(), filename.c_str(), extension.c_str());
        std::cout << "Output file is \"" << recorder.output_filename << "\".\n";
    }

    return ClRes::Ok;
}

extern "C" {
    REDEMPTION_LIB_EXPORT
    int do_main(int argc, char const ** argv,
            get_hmac_key_prototype * hmac_fn,
            get_trace_key_prototype * trace_fn)
    {
        ScopedCryptoInit scoped_crypto;

        int arg_used = 0;
        int command = 0;

        if (argc > arg_used + 1){
            command = in(argv[arg_used+1], {"redrec", "redver", "reddec"});
            if (command){
                command = command - 1;
                arg_used++;
            }
            // default command is previous one;
        }

        Inifile ini;
        ini.set<cfg::debug::config>(false);

        UdevRandom rnd;
        Fstat fstat;
        CryptoContext cctx;
        cctx.set_get_hmac_key_cb(hmac_fn);
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
            && rp.output_filename.empty()) {
                std::cerr << "Missing output filename : use -o filename\n\n";
                return -1;
            }

            if (rp.chunk) {
                rp.flv_break_interval = 60*10; // 10 minutes
                rp.png_params.png_interval = std::chrono::seconds{1};
            }

            if (rp.output_filename.length()
            && !rp.full_video
            && !bool(rp.capture_flags)) {
                std::cerr << "Missing target format : need --png, --ocr, --flv, --full, --wrm or --chunk" << std::endl;
                return 1;
            }

            // TODO before continuing to work with input file, check if it's mwrm or wrm and use right object in both cases

            // TODO also check if it contains any wrm at all and at wich one we should start depending on input time
            // TODO if start and stop time are outside wrm, userreplay(s should also be warned

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
                          rp.flv_params,
                          rp.wrm_color_depth,
                          rp.wrm_frame_interval,
                          rp.wrm_break_interval,
                          false, // no_timestamp,
                          rp.infile_is_encrypted,
                          rp.order_count,
                          rp.show_file_metadata,
                          rp.show_statistics,
                          rp.clear,
                          rp.full_video,
                          rp.remove_input_file,
                          rp.wrm_compression_algorithm_,
                          rp.flv_break_interval,
                          rp.encryption_type,
                          ini, cctx, rnd, fstat,
                          verbose);

            } catch (const Error & e) {
                std::cout << "decrypt failed: with id=" << e.id << std::endl;
            }
        break;
        case 1: // VERifier
            ini.set<cfg::debug::config>(false);
            try {
                encryption_type(rp.full_path, cctx);

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
                // TODO file is unused
                unique_fd file(rp.full_path, O_RDONLY);

                if (!file.is_open()) {
                    std::cerr << "can't open file " << rp.full_path << "\n\n";
                    std::cout << "decrypt failed" << std::endl;
                    return -1;
                }

                if (0 == encryption_type(rp.full_path, cctx)){
                    std::cout << "Input file is not encrypted." << std::endl;
                    return 0;
                }

                Fstat fstat;
                InCryptoTransport in_t(cctx, EncryptionMode::Encrypted, fstat);

                size_t res = -1ull;
                unique_fd fd1(rp.output_filename, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);

                if (fd1.is_open()) {
                    OutFileTransport out_t(std::move(fd1));

                    try {
                        char mem[4096];

                        in_t.open(rp.full_path.c_str());
                        while (1) {
                            res = in_t.partial_read(mem, sizeof(mem));
                            if (res == 0){
                                break;
                            }
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
                else {
                    std::cout << "decrypt failed" << std::endl;
                    return -1;
                }
            } catch (const Error & e) {
                std::cout << "decrypt failed: with id=" << e.id << std::endl;
            }
        break;
        }

        return res;
    }
}
