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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Jonatan Poelen
*/

#pragma once

#include "utils/log.hpp"

#include "utils/difftimeval.hpp"
#include "utils/genrandom.hpp"
#include "utils/genfstat.hpp"
#include "utils/fileutils.hpp"
#include "utils/sugar/iter.hpp"
#include "utils/stream.hpp"

#include "capture/wrm_chunk_type.hpp"
#include "capture/cryptofile.hpp"
#include "transport/out_crypto_transport.hpp"

#include "gdi/capture_api.hpp"
#include "capture/wrm_params.hpp"

#include "gdi/kbd_input_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/capture_probe_api.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/caches/bmpcache.hpp"
#include "core/RDP/caches/pointercache.hpp"

#include "gdi/dump_png24.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "core/RDP/RDPSerializer.hpp"
#include "utils/compression_transport_builder.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "core/RDP/share.hpp"

#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>


struct WrmFGen
{
private:
    char         path[1024];
    char         filename[1012];
    char         extension[12];
    mutable char filename_gen[1024];

public:
    WrmFGen(const char * const prefix, const char * const filename, const char * const extension)
    {
        if (strlen(prefix) > sizeof(this->path) - 1
         || strlen(filename) > sizeof(this->filename) - 1
         || strlen(extension) > sizeof(this->extension) - 1) {
            throw Error(ERR_TRANSPORT);
        }

        strcpy(this->path, prefix);
        strcpy(this->filename, filename);
        strcpy(this->extension, extension);

        this->filename_gen[0] = 0;
    }

    const char * get(unsigned count) const
    {
        std::snprintf( this->filename_gen, sizeof(this->filename_gen), "%s%s-%06u%s", this->path
                , this->filename, count, this->extension);
        return this->filename_gen;
    }
};

// return 0 if success, otherwise a negatif number
inline void raw_write(int fd, const void * data, size_t len)
{
    size_t remaining_len = len;
    size_t total_sent = 0;
    while (remaining_len) {
        ssize_t ret = ::write(fd,
            static_cast<const char*>(data) + total_sent, remaining_len);
        if (ret <= 0){
            if (errno == EINTR){
                continue;
            }
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        remaining_len -= ret;
        total_sent += ret;
    }
}

struct MetaFilename
{
    char filename[2048];
    MetaFilename(const char * path, const char * basename)
    {
        int res = snprintf(this->filename, sizeof(this->filename)-1, "%s%s.mwrm", path, basename);
        if (res > int(sizeof(this->filename) - 6) || res < 0) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
    }
};


struct MetaSeqBuf {
    CryptoContext & cctx;
    Random & rnd;
    Fstat & fstat;

    char current_filename_[1024];
    WrmFGen filegen_;
    unsigned num_file_;
    int groupid_;

    MetaFilename mf_;
    MetaFilename hf_;
    time_t start_sec_;
    time_t stop_sec_;

    bool with_checksum;
    bool with_encryption;
    OutCryptoTransport meta_buf_encrypt_transport;
    OutCryptoTransport wrm_filter_encrypt_transport;

    void next_meta_file(uint8_t (&qhash)[MD_HASH::DIGEST_LENGTH], uint8_t (&fhash)[MD_HASH::DIGEST_LENGTH])
    {
        const char * filename = this->filegen_.get(this->num_file_);
        this->current_filename_[0] = 0;

        this->num_file_ ++;

        struct stat stat;
        if (fstat.stat(filename, stat)){
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }

        // 8Ko for a filename with expanded slash should be enough
        // or we will truncate filename at buffersize
        char tmp[8192];
        size_t j = 0;
        for (size_t i = 0; (filename[i]) && (j < sizeof(tmp)-2) ; i++){
            switch (filename[i]){
            case '\\':
            case ' ':
                tmp[j++] = '\\';
                REDEMPTION_CXX_FALLTHROUGH;
            default:
                tmp[j++] = filename[i];
            break;
            }
        }
        tmp[j] = 0;

        using ull = unsigned long long;
        using ll = long long;

        char mes[ sizeof(tmp) +
            (std::numeric_limits<ll>::digits10 + 1 + 1) * 8 +
            (std::numeric_limits<ull>::digits10 + 1 + 1) * 2 +
            (MD_HASH::DIGEST_LENGTH*2 + 1) * 2 + 1 +
            2
        ];
        ssize_t len = std::sprintf(
            mes,
            "%s %lld %llu %lld %lld %llu %lld %lld %lld",
            tmp,
            ll(stat.st_size),
            ull(stat.st_mode),
            ll(stat.st_uid),
            ll(stat.st_gid),
            ull(stat.st_dev),
            ll(stat.st_ino),
            ll(stat.st_mtim.tv_sec),
            ll(stat.st_ctim.tv_sec)
        );

        len += std::sprintf(
            mes + len,
            " %lld %lld",
            ll(this->start_sec_),
            ll(this->stop_sec_+1)
        );
        char * p = mes + len;

        if (this->with_checksum) {
            auto hexdump = [&p](uint8_t (&hash)[MD_HASH::DIGEST_LENGTH]) {
                *p++ = ' ';                // 1 octet
                for (unsigned c : iter(hash, MD_HASH::DIGEST_LENGTH)) {
                    sprintf(p, "%02x", c); // 64 octets (hash)
                    p += 2;
                }
            };
            hexdump(qhash);
            hexdump(fhash);
        }
        *p++ = '\n';

        this->meta_buf_encrypt_transport.send(mes, p-mes);
        this->start_sec_ = this->stop_sec_+1;
    }

public:
    explicit MetaSeqBuf(
        bool with_encryption,
        bool with_checksum,
        CryptoContext & cctx,
        Random & rnd,
        Fstat & fstat,
        time_t start_sec,
        const char * const hash_prefix,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid
    )
    : cctx(cctx)
    , rnd(rnd)
    , fstat(fstat)
    , current_filename_{}
    , filegen_(prefix, filename, extension)
    , num_file_(0)
    , groupid_(groupid)
    , mf_(prefix, filename)
    , hf_(hash_prefix, filename)
    , start_sec_(start_sec)
    , stop_sec_(start_sec)
    , with_checksum(with_checksum)
    , with_encryption(with_encryption)
    , meta_buf_encrypt_transport(with_encryption, with_checksum, cctx, rnd)
    , wrm_filter_encrypt_transport(with_encryption, with_checksum, cctx, rnd)
    {
//        LOG(LOG_INFO, "hash_prefix=%s prefix=%s", hash_prefix, prefix);

    }


    ssize_t open(uint16_t width, uint16_t height)
    {
        this->meta_buf_encrypt_transport.open(this->mf_.filename, S_IRUSR | S_IRGRP | S_IWUSR);
        char header1[3 + ((std::numeric_limits<unsigned>::digits10 + 1) * 2 + 2) + (10 + 1) + 2 + 1];
        const int len = sprintf(header1, "v2\n%u %u\n%s\n\n\n",
        unsigned(width),  unsigned(height), with_checksum?"checksum":"nochecksum");
        this->meta_buf_encrypt_transport.send(header1, len);
        return 0;
    }


    ~MetaSeqBuf()
    {
        this->close();
    }

    ssize_t write(const uint8_t * data, size_t len)
    {
        if (!this->wrm_filter_encrypt_transport.is_open()) {
            const char * filename = this->filegen_.get(this->num_file_);
            this->wrm_filter_encrypt_transport.open(filename, this->groupid_);
        }
        this->wrm_filter_encrypt_transport.send(data, len);
        return 0;
    }

    int next()
    {
        if (this->wrm_filter_encrypt_transport.is_open()) {
            uint8_t qhash[MD_HASH::DIGEST_LENGTH];
            uint8_t fhash[MD_HASH::DIGEST_LENGTH];
            this->wrm_filter_encrypt_transport.close(qhash, fhash);
            this->next_meta_file(qhash, fhash);
            return 0;
        }
        return -1;
    }

    int close()
    {
        if (this->wrm_filter_encrypt_transport.is_open()) {
            uint8_t qhash[MD_HASH::DIGEST_LENGTH];
            uint8_t fhash[MD_HASH::DIGEST_LENGTH];
            this->wrm_filter_encrypt_transport.close(qhash, fhash);
            this->next_meta_file(qhash, fhash);
        }

        uint8_t qhash[MD_HASH::DIGEST_LENGTH];
        uint8_t fhash[MD_HASH::DIGEST_LENGTH];

        this->meta_buf_encrypt_transport.close(qhash, fhash);

        char path[1024] = {};
        char basename[1024] = {};
        char extension[256] = {};
        char filename[2048] = {};

        canonical_path(
            this->mf_.filename,
            path, sizeof(path),
            basename, sizeof(basename),
            extension, sizeof(extension)
        );
        snprintf(filename, sizeof(filename), "%s%s", basename, extension);

//        LOG(LOG_ERR, "Writing hash file to %s", this->hf_.filename);

        OutCryptoTransport hash_buf_encrypt_transport(this->with_encryption, false, this->cctx, this->rnd);
        hash_buf_encrypt_transport.open(this->hf_.filename, S_IRUSR|S_IRGRP);
        char header[] = "v2\n\n\n";
        hash_buf_encrypt_transport.send(reinterpret_cast<uint8_t*>(header), sizeof(header)-1);

        struct stat stat;
        int err = fstat.stat(this->mf_.filename, stat);
        if (err) {
            LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n",
                this->hf_.filename, err);
            return 1;
        }

        // 8Ko for a filename with expanded slash should be enough
        // or we will truncate filename at buffersize
        char tmp[8192];
        size_t j = 0;
        for (size_t i = 0; (filename[i]) && (j < sizeof(tmp)-2) ; i++){
            switch (filename[i]){
            case '\\':
            case ' ':
                tmp[j++] = '\\';
                REDEMPTION_CXX_FALLTHROUGH;
            default:
                tmp[j++] = filename[i];
            break;
            }
        }
        tmp[j] = 0;

        using ull = unsigned long long;
        using ll = long long;
        char mes[ sizeof(tmp) +
            (std::numeric_limits<ll>::digits10 + 1 + 1) * 8 +
            (std::numeric_limits<ull>::digits10 + 1 + 1) * 2 +
            (MD_HASH::DIGEST_LENGTH*2 + 1) * 2 + 1 +
            2
        ];
        ssize_t len = std::sprintf(
            mes,
            "%s %lld %llu %lld %lld %llu %lld %lld %lld",
            tmp,
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
        if (this->with_checksum) {
            auto hexdump = [&p](uint8_t (&hash)[MD_HASH::DIGEST_LENGTH]) {
                *p++ = ' ';                // 1 octet
                for (unsigned c : iter(hash, MD_HASH::DIGEST_LENGTH)) {
                    sprintf(p, "%02x", c); // 64 octets (hash)
                    p += 2;
                }
            };
            hexdump(qhash);
            hexdump(fhash);
        }
        *p++ = '\n';

        hash_buf_encrypt_transport.send(reinterpret_cast<uint8_t*>(mes), p-mes);
        // qhash and fhash for hash_buf_encrypt_transport won't be used
        hash_buf_encrypt_transport.close(qhash, fhash);
        return 0;
    }

    void update_sec(time_t sec)
    { this->stop_sec_ = sec; }
};


struct wrmcapture_OutMetaSequenceTransport : public Transport
{
    private:
    MetaSeqBuf buf;


    public:
    wrmcapture_OutMetaSequenceTransport(
        bool with_encryption,
        bool with_checksum,
        CryptoContext & cctx,
        Random & rnd,
        Fstat & fstat,
        const char * path,
        const char * hash_path,
        const char * basename,
        timeval now,
        uint16_t width,
        uint16_t height,
        const int groupid,
        auth_api * authentifier)
    : buf(with_encryption, with_checksum, cctx, rnd, fstat, now.tv_sec, hash_path, path, basename, ".wrm", groupid) {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        const ssize_t res = this->buf.open(width, height);
        if (res < 0) {
            int err = errno;
            LOG(LOG_ERR, "Write to transport failed code=%d", err);

            if (err == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|%s", path);
                this->authentifier->report("FILESYSTEM_FULL", message);

                throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, err);
            }
            else {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, err);
            }
        }
    }

    ~wrmcapture_OutMetaSequenceTransport()
    {
    }

    void timestamp(timeval now) override {
        this->buf.update_sec(now.tv_sec);
    }

    bool next() override {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        const ssize_t res = this->buf.next();
        if (res) {
            this->status = false;
            if (res < 0){
                LOG(LOG_ERR, "Write to transport failed code=%d", errno);
                throw Error(ERR_TRANSPORT_WRITE_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
        ++this->seqno;
        return true;
    }

    bool disconnect() override {
        return !this->buf.close();
    }

private:
    void do_send(const uint8_t * data, size_t len) override {
        const ssize_t res = this->buf.write(data, len);
        if (res < 0) {
            this->status = false;
            if (errno == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|%s", buf.filegen_.get(buf.num_file_ - 1u));
                this->authentifier->report("FILESYSTEM_FULL", message);
                errno = ENOSPC;
                LOG(LOG_ERR, "OutMetaSequenceTransport::do_send() exception NO_ROOM");
                throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, ENOSPC);
            }
            else {
                LOG(LOG_ERR, "OutMetaSequenceTransport::do_send() exception WRITE_FAILED");
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        this->last_quantum_sent += res;
    }
};


// TODO temporary
struct DumpPng24FromRDPDrawableAdapter : gdi::DumpPng24Api  {
    RDPDrawable & drawable;

    explicit DumpPng24FromRDPDrawableAdapter(RDPDrawable & drawable) : drawable(drawable) {}

    void dump_png24(Transport& trans, bool bgr) const override {
      ::dump_png24(this->drawable.impl(), trans, bgr);
    }
};


inline void wrmcapture_send_wrm_chunk(Transport & t, WrmChunkType chunktype, uint16_t data_size, uint16_t count)
{
    StaticOutStream<8> header;
    header.out_uint16_le(safe_cast<uint16_t>(chunktype));
    header.out_uint32_le(8 + data_size);
    header.out_uint16_le(count);
    t.send(header.get_data(), header.get_offset());
}


inline void wrmcapture_send_meta_chunk(
    Transport & t
  , uint8_t wrm_format_version

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

  , uint8_t  index_algorithm
) {
    StaticOutStream<36> payload;
    payload.out_uint16_le(wrm_format_version);
    payload.out_uint16_le(info_width);
    payload.out_uint16_le(info_height);
    payload.out_uint16_le(info_bpp);

    payload.out_uint16_le(info_cache_0_entries);
    payload.out_uint16_le(info_cache_0_size);
    payload.out_uint16_le(info_cache_1_entries);
    payload.out_uint16_le(info_cache_1_size);
    payload.out_uint16_le(info_cache_2_entries);
    payload.out_uint16_le(info_cache_2_size);

    if (wrm_format_version > 3) {
        payload.out_uint8(info_number_of_cache);
        payload.out_uint8(info_use_waiting_list);

        payload.out_uint8(info_cache_0_persistent);
        payload.out_uint8(info_cache_1_persistent);
        payload.out_uint8(info_cache_2_persistent);

        payload.out_uint16_le(info_cache_3_entries);
        payload.out_uint16_le(info_cache_3_size);
        payload.out_uint8(info_cache_3_persistent);
        payload.out_uint16_le(info_cache_4_entries);
        payload.out_uint16_le(info_cache_4_size);
        payload.out_uint8(info_cache_4_persistent);

        payload.out_uint8(index_algorithm);
    }

    wrmcapture_send_wrm_chunk(t, WrmChunkType::META_FILE, payload.get_offset(), 1);
    t.send(payload.get_data(), payload.get_offset());
}


template <size_t SZ>
class OutChunkedBufferingTransport : public Transport
{
    Transport & trans;
    size_t max;
    uint8_t buf[SZ];
    OutStream stream;

    static_assert(SZ >= 8, "");

public:
    explicit OutChunkedBufferingTransport(Transport & trans)
        : trans(trans)
        , max(SZ-8)
        , stream(buf)
    {
    }

    void flush() override {
        if (this->stream.get_offset() > 0) {
            wrmcapture_send_wrm_chunk(this->trans, WrmChunkType::LAST_IMAGE_CHUNK, this->stream.get_offset(), 1);
            this->trans.send(this->stream.get_data(), this->stream.get_offset());
            this->stream = OutStream(buf);
        }
    }

private:
    void do_send(const uint8_t * const buffer, size_t len) override {
        size_t to_buffer_len = len;
        while (this->stream.get_offset() + to_buffer_len > this->max) {
            wrmcapture_send_wrm_chunk(this->trans, WrmChunkType::PARTIAL_IMAGE_CHUNK, this->max, 1);
            this->trans.send(this->stream.get_data(), this->stream.get_offset());
            size_t to_send = this->max - this->stream.get_offset();
            this->trans.send(buffer + len - to_buffer_len, to_send);
            to_buffer_len -= to_send;
            this->stream = OutStream(buf);
        }
        this->stream.out_copy_bytes(buffer + len - to_buffer_len, to_buffer_len);
    }
};


class wrmcapture_SaveStateChunk {
public:
    wrmcapture_SaveStateChunk() {}

    void recv(InStream & stream, StateChunk & sc, uint8_t info_version) {
        this->send_recv(stream, sc, info_version);
    }

    void send(OutStream & stream, StateChunk & sc) {
        this->send_recv(stream, sc, ~0);
    }

private:
    static void io_uint8(InStream & stream, uint8_t & value) { value = stream.in_uint8(); }
    static void io_uint8(OutStream & stream, uint8_t value) { stream.out_uint8(value); }

    static void io_sint8(InStream & stream, int8_t & value) { value = stream.in_sint8(); }
    static void io_sint8(OutStream & stream, int8_t value) { stream.out_sint8(value); }

    // TODO BUG this is an error
    static void io_uint8_unsafe(InStream & stream, uint16_t & value) { value = stream.in_uint8(); }
    static void io_uint8_unsafe(OutStream & stream, uint16_t value) { stream.out_uint8(value); }

    static void io_uint16_le(InStream & stream, uint16_t & value) { value = stream.in_uint16_le(); }
    static void io_uint16_le(OutStream & stream, uint16_t value) { stream.out_uint16_le(value); }

    static void io_sint16_le(InStream & stream, int16_t & value) { value = stream.in_sint16_le(); }
    static void io_sint16_le(OutStream & stream, int16_t value) { stream.out_sint16_le(value); }

    static void io_uint32_le(InStream & stream, uint32_t & value) { value = stream.in_uint32_le(); }
    static void io_uint32_le(OutStream & stream, uint32_t value) { stream.out_uint32_le(value); }

    static void io_uint32_le(InStream & stream, RDPColor & value) { value = RDPColor(stream.in_uint32_le()); }
    static void io_uint32_le(OutStream & stream, RDPColor value) { stream.out_uint32_le(value.as_bgr().to_u32()); }

    static void io_color(InStream & stream, uint32_t & color) {
        uint8_t const red   = stream.in_uint8();
        uint8_t const green = stream.in_uint8();
        uint8_t const blue  = stream.in_uint8();
        color = red | green << 8 | blue << 16;
    }
    static void io_color(OutStream & stream, uint32_t color) {
        stream.out_uint8(color);
        stream.out_uint8(color >> 8);
        stream.out_uint8(color >> 16);
    }

    static void io_color(InStream & stream, RDPColor & color) {
        uint8_t const red   = stream.in_uint8();
        uint8_t const green = stream.in_uint8();
        uint8_t const blue  = stream.in_uint8();
        color = RDPColor(blue << 16 | green << 8 | red);
    }
    static void io_color(OutStream & stream, RDPColor color) {
        stream.out_uint8(color.as_bgr().red());
        stream.out_uint8(color.as_bgr().green());
        stream.out_uint8(color.as_bgr().blue());
    }

    static void io_copy_bytes(InStream & stream, uint8_t * buf, unsigned n) { stream.in_copy_bytes(buf, n); }
    static void io_copy_bytes(OutStream & stream, uint8_t * buf, unsigned n) { stream.out_copy_bytes(buf, n); }

    template<class Stream>
    static void io_delta_encoded_rects(Stream & stream, array_view<RDP::DeltaEncodedRectangle> delta_rectangles) {
        // TODO: check room to write or enough data to read, another io unified function necessary io_avail()
        for (RDP::DeltaEncodedRectangle & delta_rectangle : delta_rectangles) {
            io_sint16_le(stream, delta_rectangle.leftDelta);
            io_sint16_le(stream, delta_rectangle.topDelta);
            io_sint16_le(stream, delta_rectangle.width);
            io_sint16_le(stream, delta_rectangle.height);
        }
    }

    template<class Stream>
    static void io_brush(Stream & stream, RDPBrush & brush) {
        io_sint8(stream, brush.org_x);
        io_sint8(stream, brush.org_y);
        io_uint8(stream, brush.style);
        io_uint8(stream, brush.hatch);
        io_copy_bytes(stream, brush.extra, 7);
    }

    template<class Stream>
    static void io_rect(Stream & stream, Rect & rect) {
        io_sint16_le(stream, rect.x);
        io_sint16_le(stream, rect.y);
        io_uint16_le(stream, rect.cx);
        io_uint16_le(stream, rect.cy);
    }

    template<class Stream>
    void send_recv(Stream & stream, StateChunk & sc, uint8_t info_version) {
        const bool mem3blt_support         = (info_version > 1);
        const bool polyline_support        = (info_version > 2);
        const bool multidstblt_support     = (info_version > 3);
        const bool multiopaquerect_support = (info_version > 3);
        const bool multipatblt_support     = (info_version > 3);
        const bool multiscrblt_support     = (info_version > 3);

        // RDPOrderCommon common;
        io_uint8(stream, sc.common.order);
        io_rect(stream, sc.common.clip);

        // RDPDestBlt destblt;
        io_rect(stream, sc.destblt.rect);
        io_uint8(stream, sc.destblt.rop);

        // RDPPatBlt patblt;
        io_rect(stream, sc.patblt.rect);
        io_uint8(stream, sc.patblt.rop);
        io_uint32_le(stream, sc.patblt.back_color);
        io_uint32_le(stream, sc.patblt.fore_color);
        io_brush(stream, sc.patblt.brush);

        // RDPScrBlt scrblt;
        io_rect(stream, sc.scrblt.rect);
        io_uint8(stream, sc.scrblt.rop);
        io_uint16_le(stream, sc.scrblt.srcx);
        io_uint16_le(stream, sc.scrblt.srcy);

        // RDPOpaqueRect opaquerect;
        io_rect(stream, sc.opaquerect.rect);
        io_color(stream, sc.opaquerect.color);

        // RDPMemBlt memblt;
        io_uint16_le(stream, sc.memblt.cache_id);
        io_rect(stream, sc.memblt.rect);
        io_uint8(stream, sc.memblt.rop);
        // TODO bad length
        io_uint8_unsafe(stream, sc.memblt.srcx);
        io_uint8_unsafe(stream, sc.memblt.srcy);
        io_uint16_le(stream, sc.memblt.cache_idx);

        // RDPMem3Blt memblt;
        if (mem3blt_support) {
            io_uint16_le(stream, sc.mem3blt.cache_id);
            io_rect(stream, sc.mem3blt.rect);
            io_uint8(stream, sc.mem3blt.rop);
            io_uint8_unsafe(stream, sc.mem3blt.srcx);
            io_uint8_unsafe(stream, sc.mem3blt.srcy);
            io_uint32_le(stream, sc.mem3blt.back_color);
            io_uint32_le(stream, sc.mem3blt.fore_color);
            io_brush(stream, sc.mem3blt.brush);
            io_uint16_le(stream, sc.mem3blt.cache_idx);
        }

        // RDPLineTo lineto;
        io_uint8(stream, sc.lineto.back_mode);
        io_sint16_le(stream, sc.lineto.startx);
        io_sint16_le(stream, sc.lineto.starty);
        io_sint16_le(stream, sc.lineto.endx);
        io_sint16_le(stream, sc.lineto.endy);
        io_uint32_le(stream, sc.lineto.back_color);
        io_uint8(stream, sc.lineto.rop2);
        io_uint8(stream, sc.lineto.pen.style);
        io_uint8(stream, sc.lineto.pen.width);
        io_uint32_le(stream, sc.lineto.pen.color);

        // RDPGlyphIndex glyphindex;
        io_uint8(stream, sc.glyphindex.cache_id);
        io_sint16_le(stream, sc.glyphindex.fl_accel);
        io_sint16_le(stream, sc.glyphindex.ui_charinc);
        io_sint16_le(stream, sc.glyphindex.f_op_redundant);
        io_uint32_le(stream, sc.glyphindex.back_color);
        io_uint32_le(stream, sc.glyphindex.fore_color);
        io_rect(stream, sc.glyphindex.bk);
        io_rect(stream, sc.glyphindex.op);
        io_brush(stream, sc.glyphindex.brush);
        io_sint16_le(stream, sc.glyphindex.glyph_x);
        io_sint16_le(stream, sc.glyphindex.glyph_y);
        io_uint8(stream, sc.glyphindex.data_len);
        io_copy_bytes(stream, sc.glyphindex.data, 256);

        // RDPPolyine polyline;
        if (polyline_support) {
            io_sint16_le(stream, sc.polyline.xStart);
            io_sint16_le(stream, sc.polyline.yStart);
            io_uint8(stream, sc.polyline.bRop2);
            io_uint16_le(stream, sc.polyline.BrushCacheEntry);
            io_uint32_le(stream, sc.polyline.PenColor);
            io_uint8(stream, sc.polyline.NumDeltaEntries);
            // TODO: check room to write or enough data to read, another io unified function necessary io_avail()
            for (uint8_t i = 0; i < sc.polyline.NumDeltaEntries; i++) {
                io_sint16_le(stream, sc.polyline.deltaEncodedPoints[i].xDelta);
                io_sint16_le(stream, sc.polyline.deltaEncodedPoints[i].yDelta);
            }
        }

        // RDPMultiDstBlt multidstblt;
        if (multidstblt_support) {
            io_sint16_le(stream, sc.multidstblt.nLeftRect);
            io_sint16_le(stream, sc.multidstblt.nTopRect);
            io_sint16_le(stream, sc.multidstblt.nWidth);
            io_sint16_le(stream, sc.multidstblt.nHeight);
            io_uint8(stream, sc.multidstblt.bRop);
            io_uint8(stream, sc.multidstblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multidstblt.deltaEncodedRectangles,
                sc.multidstblt.nDeltaEntries
            });
        }

        // RDPMultiOpaqueRect multiopaquerect;
        if (multiopaquerect_support) {
            io_sint16_le(stream, sc.multiopaquerect.nLeftRect);
            io_sint16_le(stream, sc.multiopaquerect.nTopRect);
            io_sint16_le(stream, sc.multiopaquerect.nWidth);
            io_sint16_le(stream, sc.multiopaquerect.nHeight);
            io_color(stream, sc.multiopaquerect._Color);
            io_uint8(stream, sc.multiopaquerect.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multiopaquerect.deltaEncodedRectangles,
                sc.multiopaquerect.nDeltaEntries
            });
        }

        // RDPMultiPatBlt multipatblt;
        if (multipatblt_support) {
            io_rect(stream, sc.multipatblt.rect);
            io_uint8(stream, sc.multipatblt.bRop);
            io_uint32_le(stream, sc.multipatblt.BackColor);
            io_uint32_le(stream, sc.multipatblt.ForeColor);
            io_brush(stream, sc.multipatblt.brush);
            io_uint8(stream, sc.multipatblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multipatblt.deltaEncodedRectangles,
                sc.multipatblt.nDeltaEntries
            });
        }

        // RDPMultiScrBlt multiscrblt;
        if (multiscrblt_support) {
            io_rect(stream, sc.multiscrblt.rect);
            io_uint8(stream, sc.multiscrblt.bRop);
            io_sint16_le(stream, sc.multiscrblt.nXSrc);
            io_sint16_le(stream, sc.multiscrblt.nYSrc);
            io_uint8(stream, sc.multiscrblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multiscrblt.deltaEncodedRectangles,
                sc.multiscrblt.nDeltaEntries
            });
        }
    }
};


/**
 * To keep things easy all chunks have 8 bytes headers
 * starting with chunk_type, chunk_size and order_count
 *  (whatever it means, depending on chunks)
 */
class GraphicToFile
: public RDPSerializer
, public gdi::KbdInputApi
, public gdi::CaptureProbeApi
{
    enum {
        GTF_SIZE_KEYBUF_REC = 1024
    };

    CompressionOutTransportBuilder compression_bullder;
    Transport & trans_target;
    Transport & trans;
    StaticOutStream<65536> buffer_stream_orders;
    StaticOutStream<65536> buffer_stream_bitmaps;

    const std::chrono::microseconds delta_time = std::chrono::seconds(1);
    timeval timer;
    timeval last_sent_timer;
    const uint16_t width;
    const uint16_t height;
    uint16_t mouse_x;
    uint16_t mouse_y;
    const bool send_input;
    gdi::DumpPng24Api & dump_png24_api;


    uint8_t keyboard_buffer_32_buf[GTF_SIZE_KEYBUF_REC * sizeof(uint32_t)];
    // Extractor
    OutStream keyboard_buffer_32;

    const uint8_t wrm_format_version;

public:
    enum class SendInput { NO, YES };

    GraphicToFile(const timeval & now
                , Transport & trans
                , const uint16_t width
                , const uint16_t height
                , const uint8_t  capture_bpp
                , BmpCache & bmp_cache
                , GlyphCache & gly_cache
                , PointerCache & ptr_cache
                , gdi::DumpPng24Api & dump_png24
                , WrmCompressionAlgorithm wrm_compression_algorithm
                , SendInput send_input = SendInput::NO
                , Verbose verbose = Verbose::none)
    : RDPSerializer( this->buffer_stream_orders, this->buffer_stream_bitmaps, capture_bpp
                   , bmp_cache, gly_cache, ptr_cache, 0, true, true, 32 * 1024, verbose)
    , compression_bullder(trans, wrm_compression_algorithm)
    , trans_target(trans)
    , trans(this->compression_bullder.get())
    , timer(now)
    , last_sent_timer{0, 0}
    , width(width)
    , height(height)
    , mouse_x(0)
    , mouse_y(0)
    , send_input(send_input == SendInput::YES)
    , dump_png24_api(dump_png24)
    , keyboard_buffer_32(keyboard_buffer_32_buf)
    , wrm_format_version(bool(this->compression_bullder.get_algorithm()) ? 4 : 3)
    {
        if (wrm_compression_algorithm != this->compression_bullder.get_algorithm()) {
            LOG( LOG_WARNING, "compression algorithm %u not fount. Compression disable."
               , static_cast<unsigned>(wrm_compression_algorithm));
        }

        this->order_count = 0;

        this->send_meta_chunk();
        this->send_image_chunk();
    }

    void dump_png24(Transport & trans, bool bgr) const {
        this->dump_png24_api.dump_png24(trans, bgr);
    }

    /// \brief Update timestamp but send nothing, the timestamp will be sent later with the next effective event
    void timestamp(const timeval& now)
    {
        if (this->timer < now) {
            this->flush_orders();
            this->flush_bitmaps();
            this->timer = now;
            this->trans.timestamp(now);
        }
    }

    void mouse(uint16_t mouse_x, uint16_t mouse_y)
    {
        this->mouse_x = mouse_x;
        this->mouse_y = mouse_y;
    }

    bool kbd_input(const timeval & now, uint32_t uchar) override {
        (void)now;
        if (keyboard_buffer_32.has_room(sizeof(uint32_t))) {
            keyboard_buffer_32.out_uint32_le(uchar);
        }
        return true;
    }

    void enable_kbd_input_mask(bool) override {
    }

    void send_meta_chunk()
    {
        const BmpCache::cache_ & c0 = this->bmp_cache.get_cache(0);
        const BmpCache::cache_ & c1 = this->bmp_cache.get_cache(1);
        const BmpCache::cache_ & c2 = this->bmp_cache.get_cache(2);
        const BmpCache::cache_ & c3 = this->bmp_cache.get_cache(3);
        const BmpCache::cache_ & c4 = this->bmp_cache.get_cache(4);

        wrmcapture_send_meta_chunk(
            this->trans_target
          , this->wrm_format_version

          , this->width
          , this->height
          , this->capture_bpp

          , c0.entries()
          , c0.bmp_size()
          , c1.entries()
          , c1.bmp_size()
          , c2.entries()
          , c2.bmp_size()

          , this->bmp_cache.number_of_cache
          , this->bmp_cache.use_waiting_list

          , c0.persistent()
          , c1.persistent()
          , c2.persistent()

          , c3.entries()
          , c3.bmp_size()
          , c3.persistent()
          , c4.entries()
          , c4.bmp_size()
          , c4.persistent()

          , static_cast<unsigned>(this->compression_bullder.get_algorithm())
        );
    }

    // this one is used to store some embedded image inside WRM
    void send_image_chunk(void)
    {
        OutChunkedBufferingTransport<65536> png_trans(this->trans);
        this->dump_png24_api.dump_png24(png_trans, false);
    }

    void send_reset_chunk()
    {
        wrmcapture_send_wrm_chunk(this->trans, WrmChunkType::RESET_CHUNK, 0, 1);
    }

    void send_timestamp_chunk(bool ignore_time_interval = false)
    {
        StaticOutStream<12 + GTF_SIZE_KEYBUF_REC * sizeof(uint32_t) + 1> payload;
        payload.out_timeval_to_uint64le_usec(this->timer);
        if (this->send_input) {
            payload.out_uint16_le(this->mouse_x);
            payload.out_uint16_le(this->mouse_y);

            payload.out_uint8(ignore_time_interval ? 1 : 0);

            payload.out_copy_bytes(keyboard_buffer_32.get_data(), keyboard_buffer_32.get_offset());
            keyboard_buffer_32 = OutStream(keyboard_buffer_32_buf);
        }

        wrmcapture_send_wrm_chunk(this->trans, WrmChunkType::TIMESTAMP, payload.get_offset(), 1);
        this->trans.send(payload.get_data(), payload.get_offset());

        this->last_sent_timer = this->timer;
    }

    void send_save_state_chunk()
    {
        StaticOutStream<4096> payload;
        wrmcapture_SaveStateChunk ssc;
        ssc.send(payload, this->ssc);

        //------------------------------ missing variable length ---------------

        wrmcapture_send_wrm_chunk(this->trans, WrmChunkType::SAVE_STATE, payload.get_offset(), 1);
        this->trans.send(payload.get_data(), payload.get_offset());
    }

    void save_bmp_caches()
    {
        for (uint8_t cache_id = 0
        ; cache_id < this->bmp_cache.number_of_cache
        ; ++cache_id) {
            const size_t entries = this->bmp_cache.get_cache(cache_id).entries();
            for (size_t i = 0; i < entries; i++) {
                this->bmp_cache.set_cached(cache_id, i, false);
            }
        }
    }

    void save_glyph_caches()
    {
        for (uint8_t cacheId = 0; cacheId < NUMBER_OF_GLYPH_CACHES; ++cacheId) {
            for (uint8_t cacheIndex = 0; cacheIndex < NUMBER_OF_GLYPH_CACHE_ENTRIES; ++cacheIndex) {
                this->glyph_cache.set_cached(cacheId, cacheIndex, false);
            }
        }
    }

    void save_ptr_cache() {
        for (int index = 0; index < MAX_POINTER_COUNT; ++index) {
            this->pointer_cache.set_cached(index, false);
        }
    }

    void send_caches_chunk()
    {
        this->save_bmp_caches();
        this->save_glyph_caches();
        this->save_ptr_cache();
        if (this->order_count > 0) {
            this->send_orders_chunk();
        }
    }

    void breakpoint()
    {
        this->flush_orders();
        this->flush_bitmaps();
        this->send_timestamp_chunk();
        if (bool(this->compression_bullder.get_algorithm())) {
            this->send_reset_chunk();
        }
        this->trans.next();
        this->send_meta_chunk();
        this->send_timestamp_chunk();
        this->send_save_state_chunk();

        OutChunkedBufferingTransport<65536> png_trans(this->trans);

        this->dump_png24_api.dump_png24(png_trans, true);

        this->send_caches_chunk();
    }

private:
    std::chrono::microseconds elapsed_time() const
    {
        using us = std::chrono::microseconds;
        return us(ustime(this->timer)) - us(ustime(this->last_sent_timer));
    }

protected:
    void flush_orders() override {
        if (this->order_count > 0) {
            if (this->elapsed_time() >= delta_time) {
                this->send_timestamp_chunk();
            }
            this->send_orders_chunk();
        }
    }

public:
    void send_orders_chunk()
    {
        wrmcapture_send_wrm_chunk(this->trans, WrmChunkType::RDP_UPDATE_ORDERS, this->stream_orders.get_offset(), this->order_count);
        this->trans.send(this->stream_orders.get_data(), this->stream_orders.get_offset());
        this->order_count = 0;
        this->stream_orders.rewind();
    }

protected:
    void flush_bitmaps() override {
        if (this->bitmap_count > 0) {
            if (this->elapsed_time() >= delta_time) {
                this->send_timestamp_chunk();
            }
            this->send_bitmaps_chunk();
        }
    }

public:
    void sync() override {
        this->flush_bitmaps();
        this->flush_orders();
    }

    void send_bitmaps_chunk()
    {
        wrmcapture_send_wrm_chunk(this->trans, WrmChunkType::RDP_UPDATE_BITMAP, this->stream_bitmaps.get_offset(), this->bitmap_count);
        this->trans.send(this->stream_bitmaps.get_data(), this->stream_bitmaps.get_offset());
        this->bitmap_count = 0;
        this->stream_bitmaps.rewind();
    }

protected:
    void send_pointer(int cache_idx, const Pointer & cursor) override {
        if ((cursor.width != 32) || (cursor.height != 32)) {
            this->send_pointer2(cache_idx, cursor);
            return;
        }

        size_t size =   2           // mouse x
                      + 2           // mouse y
                      + 1           // cache index
                      + 1           // hotspot x
                      + 1           // hotspot y
                      + 32 * 32 * 3 // data
                      + 128         // mask
                      ;
        wrmcapture_send_wrm_chunk(this->trans, WrmChunkType::POINTER, size, 0);

        StaticOutStream<16> payload;
        payload.out_uint16_le(this->mouse_x);
        payload.out_uint16_le(this->mouse_y);
        payload.out_uint8(cache_idx);
        payload.out_uint8(cursor.x);
        payload.out_uint8(cursor.y);
        this->trans.send(payload.get_data(), payload.get_offset());

        this->trans.send(cursor.data, cursor.data_size());
        this->trans.send(cursor.mask, cursor.mask_size());
    }

    void send_pointer2(int cache_idx, const Pointer & cursor) {
        size_t size =   2                   // mouse x
                      + 2                   // mouse y
                      + 1                   // cache index

                      + 1                   // mouse width
                      + 1                   // mouse height
                      + 1                   // mouse bpp

                      + 1                   // hotspot x
                      + 1                   // hotspot y

                      + 2                   // data_size
                      + 2                   // mask_size

                      + cursor.data_size()  // data
                      + cursor.mask_size()  // mask
                      ;
        wrmcapture_send_wrm_chunk(this->trans, WrmChunkType::POINTER2, size, 0);

        StaticOutStream<32> payload;
        payload.out_uint16_le(this->mouse_x);
        payload.out_uint16_le(this->mouse_y);
        payload.out_uint8(cache_idx);

        payload.out_uint8(cursor.width);
        payload.out_uint8(cursor.height);
        payload.out_uint8(24);

        payload.out_uint8(cursor.x);
        payload.out_uint8(cursor.y);

        payload.out_uint16_le(cursor.data_size());
        payload.out_uint16_le(cursor.mask_size());

        this->trans.send(payload.get_data(), payload.get_offset());

        this->trans.send(cursor.data, cursor.data_size());
        this->trans.send(cursor.mask, cursor.mask_size());
    }

    void set_pointer(int cache_idx) override {
        size_t size =   2                   // mouse x
                      + 2                   // mouse y
                      + 1                   // cache index
                      ;
        wrmcapture_send_wrm_chunk(this->trans, WrmChunkType::POINTER, size, 0);

        StaticOutStream<16> payload;
        payload.out_uint16_le(this->mouse_x);
        payload.out_uint16_le(this->mouse_y);
        payload.out_uint8(cache_idx);
        this->trans.send(payload.get_data(), payload.get_offset());
    }

public:
    void session_update(timeval const & now, array_view_const_char message) override {
        uint16_t message_length = message.size() + 1;       // Null-terminator is included.

        StaticOutStream<16> payload;
        payload.out_timeval_to_uint64le_usec(now);
        payload.out_uint16_le(message_length);

        wrmcapture_send_wrm_chunk(this->trans, WrmChunkType::SESSION_UPDATE, payload.get_offset() + message_length, 1);
        this->trans.send(payload.get_data(), payload.get_offset());
        this->trans.send(message.data(), message.size());
        this->trans.send("\0", 1);

        this->last_sent_timer = this->timer;
    }

    void possible_active_window_change() override {}

    using RDPSerializer::set_pointer;
};  // struct GraphicToFile



class WrmCaptureImpl :
    public gdi::KbdInputApi,
    public gdi::CaptureApi,
    public gdi::GraphicApi,
    public gdi::CaptureProbeApi,
    public gdi::ExternalCaptureApi // from gdi/capture_api.hpp
{
public:
    BmpCache     bmp_cache;
    GlyphCache   gly_cache;
    PointerCache ptr_cache;

    DumpPng24FromRDPDrawableAdapter dump_png24_api;

    wrmcapture_OutMetaSequenceTransport out;

private:
    struct Serializer final : GraphicToFile {
        Serializer(const timeval & now
                , Transport & trans
                , const uint16_t width
                , const uint16_t height
                , const uint8_t  capture_bpp
                , BmpCache & bmp_cache
                , GlyphCache & gly_cache
                , PointerCache & ptr_cache
                , gdi::DumpPng24Api & dump_png24
                , WrmCompressionAlgorithm wrm_compression_algorithm
                , SendInput send_input = SendInput::NO
                , GraphicToFile::Verbose verbose = GraphicToFile::Verbose::none)
            : GraphicToFile(now, trans, width, height,
                            capture_bpp,
                            bmp_cache, gly_cache, ptr_cache,
                            dump_png24, wrm_compression_algorithm,
                            send_input, verbose)
        {}

        using GraphicToFile::draw;
        using GraphicToFile::capture_bpp;

        void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
            auto compress_and_draw_bitmap_update = [&bitmap_data, this](const Bitmap & bmp) {
                StaticOutStream<65535> bmp_stream;
                bmp.compress(this->capture_bpp, bmp_stream);

                RDPBitmapData target_bitmap_data = bitmap_data;

                target_bitmap_data.bits_per_pixel = bmp.bpp();
                target_bitmap_data.flags          = BITMAP_COMPRESSION | NO_BITMAP_COMPRESSION_HDR;
                target_bitmap_data.bitmap_length  = bmp_stream.get_offset();

                GraphicToFile::draw(target_bitmap_data, bmp);
            };

            if (bmp.bpp() > this->capture_bpp) {
                // reducing the color depth of image.
                Bitmap capture_bmp(this->capture_bpp, bmp);
                compress_and_draw_bitmap_update(capture_bmp);
            }
            else if (!bmp.has_data_compressed()) {
                compress_and_draw_bitmap_update(bmp);
            }
            else {
                GraphicToFile::draw(bitmap_data, bmp);
            }
        }

        void enable_kbd_input_mask(bool) override {}

        bool kbd_input(const timeval & now, uint32_t uchar) override {
            return this->GraphicToFile::kbd_input(now, uchar);
        }

        ~Serializer() {
        }

    } graphic_to_file;

public:

    // EXTERNAL CAPTURE API
    void external_breakpoint() override {
        this->nc.external_breakpoint();
    }

    void external_time(timeval const & now) override {
        this->nc.external_time(now);
    }

    // CAPTURE PROBE API
    void session_update(timeval const & now, array_view_const_char message) override {
        this->graphic_to_file.session_update(now, message);
    }
    void possible_active_window_change() override { this->graphic_to_file.possible_active_window_change(); }

    // GRAPHIC API
    void draw(RDP::FrameMarker    const & cmd) override { this->graphic_to_file.draw(cmd);}
    void draw(RDPDestBlt          const & cmd, Rect clip) override {this->graphic_to_file.draw(cmd, clip);}
    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override {this->graphic_to_file.draw(cmd, clip);}
    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPScrBlt           const & cmd, Rect clip) override {this->graphic_to_file.draw(cmd, clip);}
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override {this->graphic_to_file.draw(cmd, clip);}
    void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override {
        this->graphic_to_file.draw(cmd, bmp);
    }
    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override {
        this->graphic_to_file.draw(cmd, clip, bmp);
    }
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx, bmp);
    }
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx, gly_cache);
    }
    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::WindowIcon                     & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::CachedIcon                     & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::DeletedWindow                  & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(RDPColCache   const & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(RDPBrushCache const & cmd) override {
        this->graphic_to_file.draw(cmd);
    }

    class NativeCaptureLocal : public gdi::CaptureApi, public gdi::ExternalCaptureApi
    {
        timeval start_native_capture;
        uint64_t inter_frame_interval_native_capture;

        timeval start_break_capture;
        uint64_t inter_frame_interval_start_break_capture;

        GraphicToFile & recorder;
        uint64_t time_to_wait;

    public:
        NativeCaptureLocal(
            GraphicToFile & recorder,
            const timeval & now,
            std::chrono::duration<unsigned int, std::ratio<1, 100>> frame_interval,
            std::chrono::seconds break_interval
        )
        : start_native_capture(now)
        , inter_frame_interval_native_capture(
            std::chrono::duration_cast<std::chrono::microseconds>(frame_interval).count())
        , start_break_capture(now)
        , inter_frame_interval_start_break_capture(
            std::chrono::duration_cast<std::chrono::microseconds>(break_interval).count())
        , recorder(recorder)
        , time_to_wait(0)
        {}

        ~NativeCaptureLocal() override {
            this->recorder.sync();
        }

        // toggles externally genareted breakpoint.
        void external_breakpoint() override {
            this->recorder.breakpoint();
        }

        void external_time(const timeval & now) override {
            this->recorder.sync();
            this->recorder.timestamp(now);
        }

    private:
        std::chrono::microseconds do_snapshot(
            const timeval & now, int x, int y, bool ignore_frame_in_timeval
        ) override {
            (void)ignore_frame_in_timeval;
            if (difftimeval(now, this->start_native_capture)
                    >= this->inter_frame_interval_native_capture) {
                this->recorder.timestamp(now);
                this->time_to_wait = this->inter_frame_interval_native_capture;
                this->recorder.mouse(static_cast<uint16_t>(x), static_cast<uint16_t>(y));
                this->start_native_capture = now;
                if ((difftimeval(now, this->start_break_capture) >=
                     this->inter_frame_interval_start_break_capture)) {
                    this->recorder.breakpoint();
                    this->start_break_capture = now;
                }
            }
            else {
                this->time_to_wait = this->inter_frame_interval_native_capture - difftimeval(now, this->start_native_capture);
            }
            return std::chrono::microseconds{this->time_to_wait};
        }

    } nc;

    bool kbd_input_mask_enabled;

public:
    WrmCaptureImpl(const timeval & now, const WrmParams wrm_params, auth_api * authentifier, RDPDrawable & drawable)
    : bmp_cache(
        BmpCache::Recorder, wrm_params.capture_bpp, 3, false,
        BmpCache::CacheOption(600, 768, false),
        BmpCache::CacheOption(300, 3072, false),
        BmpCache::CacheOption(262, 12288, false))
    , ptr_cache(/*pointerCacheSize=*/0x19)
    , dump_png24_api{drawable}
    , out((wrm_params.trace_type == TraceType::cryptofile),
          (wrm_params.trace_type == TraceType::localfile_hashed),
           wrm_params.cctx,
           wrm_params.rnd,
           wrm_params.fstat,
           wrm_params.record_path,
           wrm_params.hash_path,
           wrm_params.basename,
           now, drawable.width(), drawable.height(), wrm_params.groupid, authentifier)
    , graphic_to_file(now, this->out, drawable.width(), drawable.height(), wrm_params.capture_bpp,
        this->bmp_cache, this->gly_cache, this->ptr_cache, this->dump_png24_api,
        wrm_params.wrm_compression_algorithm, GraphicToFile::SendInput::YES,
        GraphicToFile::Verbose(wrm_params.wrm_verbose)
    )
    , nc(this->graphic_to_file, now, wrm_params.frame_interval, wrm_params.break_interval)
    , kbd_input_mask_enabled{false}
    {}

    ~WrmCaptureImpl()
    {
    }
    // shadow text
    bool kbd_input(const timeval& now, uint32_t uchar) override {
        return this->graphic_to_file.kbd_input(now, this->kbd_input_mask_enabled?'*':uchar);
    }

    void enable_kbd_input_mask(bool enable) override {
        this->kbd_input_mask_enabled = enable;
    }


    void enable_keyboard_log()
    {
        this->kbd_input_mask_enabled = false;
    }

    void send_timestamp_chunk(timeval const & now, bool ignore_time_interval) {
        this->graphic_to_file.timestamp(now);
        this->graphic_to_file.send_timestamp_chunk(ignore_time_interval);
    }

    std::chrono::microseconds do_snapshot(
        const timeval & now, int x, int y, bool ignore_frame_in_timeval
    ) override {
        return this->nc.periodic_snapshot(now, x, y, ignore_frame_in_timeval);
    }

};

