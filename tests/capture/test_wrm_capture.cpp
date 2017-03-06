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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to capture interface to video recording to flv or mp4frecv
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWrmCapture
#include "system/redemption_unit_tests.hpp"


//#define LOGNULL
#define LOGPRINT

#include "utils/log.hpp"

#include <snappy-c.h>
#include <memory>

#include "utils/png.hpp"
#include "utils/drawable.hpp"

#include "transport/transport.hpp"
#include "transport/test_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/in_file_transport.hpp"

#include "check_sig.hpp"
#include "get_file_contents.hpp"
#include "utils/bitmap_shrink.hpp"
#include "capture/capture.hpp"

#include "capture/wrm_capture.hpp"
#include "transport/in_meta_sequence_transport.hpp"


struct wrmcapture_OutFilenameSequenceTransport : public Transport
{
    char current_filename_[1024];
    WrmFGen filegen_;
    iofdbuf buf_;
    unsigned num_file_;
    int groupid_;

    wrmcapture_OutFilenameSequenceTransport(
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        auth_api * authentifier)
    : filegen_(prefix, filename, extension)
    , buf_()
    , num_file_(0)
    , groupid_(groupid)
    {
        this->current_filename_[0] = 0;
        if (authentifier) {
            this->set_authentifier(authentifier);
        }
    }

    bool next() override {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        ssize_t res = 1;
        if (this->buf_.is_open()) {
            this->buf_.close();
            // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
            res = this->rename_filename() ? 0 : 1;
        }

        if (res) {
            this->status = false;
            if (res < 0){
                LOG(LOG_ERR, "Write to transport failed (M1): code=%d", errno);
                throw Error(ERR_TRANSPORT_WRITE_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
        ++this->seqno;
        return true;
    }

    bool disconnect() override {
        return !this->close();
    }

    void request_full_cleaning() override {
        unsigned i = this->num_file_ + 1;
        while (i > 0 && !::unlink(this->filegen_.get(--i))) {}
        if (this->buf_.is_open()) {
            this->buf_.close();
        }
    }

    ~wrmcapture_OutFilenameSequenceTransport() {
        this->close();
    }

private:

    void do_send(const uint8_t * data, size_t len) override {
        const ssize_t res = this->write(data, len);
        if (res < 0) {
            this->status = false;
            if (errno == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|unknown");
                this->authentifier->report("FILESYSTEM_FULL", message);
                errno = ENOSPC;
                throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, ENOSPC);
            }
            else {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        this->last_quantum_sent += res;
    }

    public:

    int close()
    {
        ssize_t res = 1;
        if (this->buf_.is_open()) {
            this->buf_.close();
            // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
            res = this->rename_filename() ? 0 : 1;
        }
        return res;
    }

    ssize_t write(const void * data, size_t len)
    {
        if (!this->buf_.is_open()) {
            const int res = this->open_filename(this->filegen_.get(this->num_file_));
            if (res < 0) {
                return res;
            }
        }
        return this->buf_.write(data, len);
    }


    iofdbuf & buf() noexcept
    { return this->buf_; }

    const char * current_path() const
    {
        if (!this->current_filename_[0] && !this->num_file_) {
            return nullptr;
        }
        return this->filegen_.get(this->num_file_ - 1);
    }

protected:
    ssize_t open_filename(const char * filename)
    {
        snprintf(this->current_filename_, sizeof(this->current_filename_),
                    "%sred-XXXXXX.tmp", filename);
        const int fd = ::mkostemps(this->current_filename_, 4, O_WRONLY | O_CREAT);
        if (fd < 0) {
            return fd;
        }
        if (chmod(this->current_filename_, this->groupid_ ? (S_IRUSR | S_IRGRP) : S_IRUSR) == -1) {
            LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
               , this->current_filename_
               , this->groupid_ ? "u+r, g+r" : "u+r"
               , strerror(errno), errno);
        }
        this->filegen_.set_last_filename(this->num_file_, this->current_filename_);
        return this->buf_.open(fd);
    }

    const char * rename_filename()
    {
        const char * filename = this->get_filename_generate();
        const int res = ::rename(this->current_filename_, filename);
        if (res < 0) {
            LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
               , this->current_filename_, filename, errno, strerror(errno));
            return nullptr;
        }

        this->current_filename_[0] = 0;
        ++this->num_file_;
        this->filegen_.set_last_filename(-1u, "");

        return filename;
    }

    const char * get_filename_generate()
    {
        this->filegen_.set_last_filename(-1u, "");
        const char * filename = this->filegen_.get(this->num_file_);
        this->filegen_.set_last_filename(this->num_file_, this->current_filename_);
        return filename;
    }
};


template<class Writer>
void wrmcapture_write_meta_headers(Writer & writer, const char * path,
                        uint16_t width, uint16_t height,
                        auth_api * authentifier,
                        bool has_checksum
                       )
{
    char header1[3 + ((std::numeric_limits<unsigned>::digits10 + 1) * 2 + 2) + (10 + 1) + 2 + 1];
    const int len = sprintf(
        header1,
        "v2\n"
        "%u %u\n"
        "%s\n"
        "\n\n",
        unsigned(width),
        unsigned(height),
        has_checksum  ? "checksum" : "nochecksum"
    );
    const ssize_t res = writer.write(header1, len);
    if (res < 0) {
        int err = errno;
        LOG(LOG_ERR, "Write to transport failed (M2): code=%d", err);

        if (err == ENOSPC) {
            char message[1024];
            snprintf(message, sizeof(message), "100|%s", path);
            authentifier->report("FILESYSTEM_FULL", message);

            throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, err);
        }
        else {
            throw Error(ERR_TRANSPORT_WRITE_FAILED, err);
        }
    }
}

BOOST_AUTO_TEST_CASE(TestSimpleBreakpoint)
{
    Rect scr(0, 0, 800, 600);
    const int groupid = 0;
    wrmcapture_OutFilenameSequenceTransport trans("./", "test", ".wrm", groupid, nullptr);

    struct timeval now;
    now.tv_sec = 1000;
    now.tv_usec = 0;

    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(600, 768, false),
                       BmpCache::CacheOption(300, 3072, false),
                       BmpCache::CacheOption(262, 12288, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(800, 600);
    DumpPng24FromRDPDrawableAdapter dump_png{drawable};
    GraphicToFile graphic_to_file(now, trans, 800, 600, 24,
        bmp_cache, gly_cache, ptr_cache, dump_png, WrmCompressionAlgorithm::no_compression
    );
    WrmCaptureImpl::NativeCaptureLocal consumer(graphic_to_file, now, std::chrono::seconds{1}, std::chrono::seconds{5});
    auto const color_cxt = gdi::ColorCtx::depth24();

    drawable.show_mouse_cursor(false);

    bool ignore_frame_in_timeval = false;

    drawable.draw(RDPOpaqueRect(scr, RED), scr, color_cxt);
    graphic_to_file.draw(RDPOpaqueRect(scr, RED), scr, color_cxt);
    consumer.periodic_snapshot(now, 10, 10, ignore_frame_in_timeval);
    now.tv_sec += 6;
    consumer.periodic_snapshot(now, 10, 10, ignore_frame_in_timeval);
    trans.disconnect();

    const char * filename0 = "./test-000000.wrm";
    BOOST_CHECK_EQUAL(1560, ::filesize(filename0));
   ::unlink(filename0);
    const char * filename1 = "./test-000001.wrm";
    BOOST_CHECK_EQUAL(3365, ::filesize(filename1));
   ::unlink(filename1);
}


BOOST_AUTO_TEST_CASE(TestWrmCapture)
{
    {
        // Timestamps are applied only when flushing
        timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);

        LCGRandom rnd(0);
        CryptoContext cctx;

        GraphicToFile::Verbose wrm_verbose = to_verbose_flags(0)
//         |GraphicToFile::Verbose::primary_orders)
//         |GraphicToFile::Verbose::secondary_orders)
//         |GraphicToFile::Verbose::bitmap_update)
        ;

        WrmCompressionAlgorithm wrm_compression_algorithm = WrmCompressionAlgorithm::no_compression;
        std::chrono::duration<unsigned int, std::ratio<1l, 100l> > wrm_frame_interval = std::chrono::seconds{1};
        std::chrono::seconds wrm_break_interval = std::chrono::seconds{3};
        TraceType wrm_trace_type = TraceType::localfile;

        const char * record_path = "./";
        const int groupid = 0; // www-data
        const char * hash_path = "/tmp";

        char path[1024];
        char basename[1024];
        char extension[128];
        strcpy(path, WRM_PATH "/");     // default value, actual one should come from movie_path
        strcpy(basename, "capture");
        strcpy(extension, "");          // extension is currently ignored

        WrmParams wrm_params(
            24,
            wrm_trace_type,
            cctx,
            rnd,
            record_path,
            hash_path,
            basename,
            groupid,
            wrm_frame_interval,
            wrm_break_interval,
            wrm_compression_algorithm,
            int(wrm_verbose)
        );

        RDPDrawable gd_drawable(scr.cx, scr.cy);

        WrmCaptureImpl wrm(now, wrm_params, nullptr /* authentifier */, gd_drawable);

        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        gd_drawable.draw(RDPOpaqueRect(scr, GREEN), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(scr, GREEN), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), BLUE), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), BLUE), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), WHITE), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), WHITE), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), RED), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), RED), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), BLACK), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), BLACK), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), PINK), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), PINK), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), WABGREEN), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), WABGREEN), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
        // The destruction of capture object will finalize the metafile content
    }

    {
        // TODO: we may have several mwrm sizes as it contains varying length numbers
        // the right solution would be to inject predictable fstat in test environment

        struct CheckFiles {
            const char * filename;
            size_t size;
            size_t alt_size;
        } fileinfo[] = {
            {"./capture-000000.wrm", 1646, 0},
            {"./capture-000001.wrm", 3508, 0},
            {"./capture-000002.wrm", 3463, 0},
            {"./capture-000003.wrm", static_cast<size_t>(-1), static_cast<size_t>(-1)},
            {"./capture.mwrm", 288, 285},
        };
        for (auto x: fileinfo) {
            size_t fsize = filesize(x.filename);
            if (x.alt_size != fsize) {
                BOOST_CHECK_EQUAL(x.size, fsize);
            }
            ::unlink(x.filename);
        }
    }
}

BOOST_AUTO_TEST_CASE(TestWrmCaptureLocalHashed)
{
    {
        // Timestamps are applied only when flushing
        timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);

        LCGRandom rnd(0);
        CryptoContext cctx;
        cctx.set_master_key(cstr_array_view(
            "\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd"
            "\xa6\x50\x38\xfc\xd8\x86\x51\x4f"
            "\x59\x7e\x8e\x90\x81\xf6\xf4\x48"
            "\x9c\x77\x41\x51\x0f\x53\x0e\xe8"
        ));
        cctx.set_hmac_key(cstr_array_view(
             "\x86\x41\x05\x58\xc4\x95\xcc\x4e"
             "\x49\x21\x57\x87\x47\x74\x08\x8a"
             "\x33\xb0\x2a\xb8\x65\xcc\x38\x41"
             "\x20\xfe\xc2\xc9\xb8\x72\xc8\x2c"
        ));


        WrmParams wrm_params(
            24,
            TraceType::localfile_hashed,
            cctx,
            rnd,
            "./",
            "/tmp",
            "capture",
            1000, // ini.get<cfg::video::capture_groupid>()
            std::chrono::seconds{1},
            std::chrono::seconds{3},
            WrmCompressionAlgorithm::no_compression,
            0xFFFF
        );

        RDPDrawable gd_drawable(scr.cx, scr.cy);

        WrmCaptureImpl wrm(now, wrm_params, nullptr /* authentifier */, gd_drawable);

        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        gd_drawable.draw(RDPOpaqueRect(scr, GREEN), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(scr, GREEN), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), BLUE), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), BLUE), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), WHITE), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), WHITE), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), RED), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), RED), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), BLACK), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), BLACK), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), PINK), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), PINK), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), WABGREEN), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), WABGREEN), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
        // The destruction of capture object will finalize the metafile content
    }

    // TODO: we may have several mwrm sizes as it contains varying length numbers
    // the right solution would be to inject predictable fstat in test environment
    struct CheckFiles {
        const char * filename;
        size_t size;
        size_t altsize;
    } fileinfo[] = {
        {"./capture-000000.wrm", 1646, 0},
        {"./capture-000001.wrm", 3508, 0},
        {"./capture-000002.wrm", 3463, 0},
        {"./capture-000003.wrm", static_cast<size_t>(-1), static_cast<size_t>(-1)},
        {"./capture.mwrm", 676, 673},
    };
    for (auto x: fileinfo) {
        size_t fsize = filesize(x.filename);
        if (x.size != fsize){
            BOOST_CHECK_EQUAL(x.altsize, fsize);
        }
        ::unlink(x.filename);
    }
}

inline char * wrmcapture_swrite_hash(char * p, wrmcapture_hash_type const & hash)
{
    auto write = [&p](unsigned char const * hash) {
        *p++ = ' ';                // 1 octet
        for (unsigned c : iter(hash, MD_HASH_LENGTH)) {
            sprintf(p, "%02x", c); // 64 octets (hash)
            p += 2;
        }
    };
    write(hash);
    write(hash + MD_HASH_LENGTH);
    return p;
}


//BOOST_AUTO_TEST_CASE(TestOSumBuf)
//{
//    CryptoContext cctx;
//    cctx.set_master_key(cstr_array_view(
//        "\x00\x01\x02\x03\x04\x05\x06\x07"
//        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
//        "\x10\x11\x12\x13\x14\x15\x16\x17"
//        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
//    ));
//    cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));
//    wrmcapture_ochecksum_buf_null_buf buf(cctx.get_hmac_key());
//    buf.open();
//    BOOST_CHECK_EQUAL(buf.write("ab", 2), 2);
//    BOOST_CHECK_EQUAL(buf.write("cde", 3), 3);

//    wrmcapture_hash_type hash;
//    buf.close(hash);

//    char hash_str[wrmcapture_hash_string_len + 1];
//    *wrmcapture_swrite_hash(hash_str, hash) = 0;
//    BOOST_CHECK_EQUAL(
//        hash_str,
//        " 03cb482c5a6af0d37b74d0a8b1facf6a02b619068e92495f469e0098b662fe3f"
//        " 03cb482c5a6af0d37b74d0a8b1facf6a02b619068e92495f469e0098b662fe3f"
//    );
//}

template<class Writer>
int wrmcapture_write_filename(Writer & writer, const char * filename)
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


template<class Writer>
int wrmcapture_write_meta_file(
    Writer & writer, const char * filename,
    time_t start_sec, time_t stop_sec,
    wrmcapture_hash_type const * hash
) {
    struct stat stat;
    int err = ::stat(filename, &stat);
    if (err){
        return err;
    }

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

    if (err) {
        return err;
    }

    using ull = unsigned long long;
    using ll = long long;
    char mes[
        (std::numeric_limits<ll>::digits10 + 1 + 1) * 8 +
        (std::numeric_limits<ull>::digits10 + 1 + 1) * 2 +
        wrmcapture_hash_string_len + 1 +
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
    len += std::sprintf(
        mes + len,
        " %lld %lld",
        ll(start_sec),
        ll(stop_sec)
    );

    char * p = mes + len;
    if (hash) {
        auto write = [&p](unsigned char const * hash) {
            *p++ = ' ';                // 1 octet
            for (unsigned c : iter(hash, MD_HASH_LENGTH)) {
                sprintf(p, "%02x", c); // 64 octets (hash)
                p += 2;
            }
        };
        write(&(*hash)[0]);
        write(&(*hash)[MD_HASH_LENGTH]);
    }
    *p++ = '\n';

    ssize_t res = writer.write(mes, p-mes);

    if (res < p-mes) {
        return res < 0 ? res : 1;
    }
    return 0;
}

#include <string>

BOOST_AUTO_TEST_CASE(TestWriteFilename)
{
    struct {
        std::string s;

        int write(char const * data, std::size_t len) {
            s.append(data, len);
            return len;
        }
    } writer;

#define TEST_WRITE_FILENAME(origin_filename, wrote_filename) \
    wrmcapture_write_filename(writer, origin_filename);                 \
    BOOST_CHECK_EQUAL(writer.s, wrote_filename);             \
    writer.s.clear()

    TEST_WRITE_FILENAME("abcde.txt", "abcde.txt");

    TEST_WRITE_FILENAME(R"(\abcde.txt)", R"(\\abcde.txt)");
    TEST_WRITE_FILENAME(R"(abc\de.txt)", R"(abc\\de.txt)");
    TEST_WRITE_FILENAME(R"(abcde.txt\)", R"(abcde.txt\\)");
    TEST_WRITE_FILENAME(R"(abc\\de.txt)", R"(abc\\\\de.txt)");
    TEST_WRITE_FILENAME(R"(\\\\)", R"(\\\\\\\\)");

    TEST_WRITE_FILENAME(R"( abcde.txt)", R"(\ abcde.txt)");
    TEST_WRITE_FILENAME(R"(abc de.txt)", R"(abc\ de.txt)");
    TEST_WRITE_FILENAME(R"(abcde.txt )", R"(abcde.txt\ )");
    TEST_WRITE_FILENAME(R"(abc  de.txt)", R"(abc\ \ de.txt)");
    TEST_WRITE_FILENAME(R"(    )", R"(\ \ \ \ )");
}

BOOST_AUTO_TEST_CASE(TestWriteHash)
{
    wrmcapture_hash_type hash;
    std::iota(std::begin(hash), std::end(hash), 0);

    char hash_str[wrmcapture_hash_string_len + 1];
    *wrmcapture_swrite_hash(hash_str, hash) = 0;
    BOOST_CHECK_EQUAL(
        hash_str,
        " 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"
        " 202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f"
    );
}

BOOST_AUTO_TEST_CASE(TestOutmetaTransport)
{
    unsigned sec_start = 1352304810;
    {
        timeval now;
        now.tv_sec = sec_start;
        now.tv_usec = 0;
        const int groupid = 0;
        wrmcapture_OutMetaSequenceTransport wrm_trans("./", "./hash-", "xxx", now, 800, 600, groupid);
        wrm_trans.send("AAAAX", 5);
        wrm_trans.send("BBBBX", 5);
        wrm_trans.next();
        wrm_trans.send("CCCCX", 5);
    } // brackets necessary to force closing sequence

    struct {
        size_t len = 0;
        ssize_t write(char const *, size_t len) {
            this->len += len;
            return len;
        }
    } meta_len_writer;

    const char * meta_path = "./xxx.mwrm";
    const char * filename = meta_path + 2;
    const char * meta_hash_path = "./hash-xxx.mwrm";
    meta_len_writer.len = 5; // header
    struct stat stat;
    BOOST_CHECK(!::stat(meta_path, &stat));

    int err = wrmcapture_write_filename(meta_len_writer, filename);
    if (!err) {
        using ull = unsigned long long;
        using ll = long long;
        char mes[
            (std::numeric_limits<ll>::digits10 + 1 + 1) * 8 +
            (std::numeric_limits<ull>::digits10 + 1 + 1) * 2 +
            wrmcapture_hash_string_len + 1 +
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

        ssize_t res = meta_len_writer.write(mes, p-mes);

        if (res < p-mes) {
            err = res < 0 ? res : 1;
        }
        else {
            err = 0;
        }
    }

    BOOST_CHECK_EQUAL(err, 0);
    BOOST_CHECK_EQUAL(meta_len_writer.len, filesize(meta_hash_path));
    BOOST_CHECK_EQUAL(0, ::unlink(meta_hash_path));


    meta_len_writer.len = 0;

    wrmcapture_write_meta_headers(meta_len_writer, nullptr, 800, 600, nullptr, false);

    const char * file1 = "./xxx-000000.wrm";
    BOOST_CHECK(!wrmcapture_write_meta_file(meta_len_writer, file1, sec_start, sec_start+1, nullptr));
    BOOST_CHECK_EQUAL(10, filesize(file1));
    BOOST_CHECK_EQUAL(0, ::unlink(file1));

    const char * file2 = "./xxx-000001.wrm";
    BOOST_CHECK(!wrmcapture_write_meta_file(meta_len_writer, file2, sec_start, sec_start+1, nullptr));
    BOOST_CHECK_EQUAL(5, filesize(file2));
    BOOST_CHECK_EQUAL(0, ::unlink(file2));

    BOOST_CHECK_EQUAL(meta_len_writer.len, filesize(meta_path));
    BOOST_CHECK_EQUAL(0, ::unlink(meta_path));
}


BOOST_AUTO_TEST_CASE(TestOutmetaTransportWithSum)
{
    unsigned sec_start = 1352304810;
    {
        CryptoContext cctx;
        cctx.set_master_key(cstr_array_view(
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        ));
        cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));
        timeval now;
        now.tv_sec = sec_start;
        now.tv_usec = 0;
        const int groupid = 0;
        wrmcapture_OutMetaSequenceTransportWithSum wrm_trans(cctx, "./", "./", "xxx", now, 800, 600, groupid, nullptr);
        wrm_trans.send("AAAAX", 5);
        wrm_trans.send("BBBBX", 5);
        wrm_trans.next();
        wrm_trans.send("CCCCX", 5);
    } // brackets necessary to force closing sequence

    struct {
        size_t len = 0;
        ssize_t write(char const *, size_t len) {
            this->len += len;
            return len;
        }
    } meta_len_writer;
    wrmcapture_write_meta_headers(meta_len_writer, nullptr, 800, 600, nullptr, true);

    const unsigned hash_size = (1 + MD_HASH_LENGTH*2) * 2;

//    char file1[1024];
//    snprintf(file1, 1024, "./xxx-%06u-%06u.wrm", getpid(), 0);
    const char * file1 = "./xxx-000000.wrm";
    wrmcapture_write_meta_file(meta_len_writer, file1, sec_start, sec_start+1, nullptr);
    meta_len_writer.len += hash_size;
    BOOST_CHECK_EQUAL(10, filesize(file1));
    BOOST_CHECK_EQUAL(0, ::unlink(file1));

//    char file2[1024];
//    snprintf(file2, 1024, "./xxx-%06u-%06u.wrm", getpid(), 1);
    const char * file2 = "./xxx-000001.wrm";
    wrmcapture_write_meta_file(meta_len_writer, file2, sec_start, sec_start+1, nullptr);
    meta_len_writer.len += hash_size;
    BOOST_CHECK_EQUAL(5, filesize(file2));
    BOOST_CHECK_EQUAL(0, ::unlink(file2));

//    char meta_path[1024];
//    snprintf(meta_path, 1024, "./xxx-%06u.mwrm", getpid());
    const char * meta_path = "./xxx.mwrm";
    BOOST_CHECK_EQUAL(meta_len_writer.len, filesize(meta_path));
    BOOST_CHECK_EQUAL(0, ::unlink(meta_path));
}


BOOST_AUTO_TEST_CASE(TestRequestFullCleaning)
{
    unlink("./xxx-000000.wrm");
    unlink("./xxx-000001.wrm");
    unlink("./xxx.mwrm");
    unlink("./hash-xxx.mwrm");

    {

        timeval now;
        now.tv_sec = 1352304810;
        now.tv_usec = 0;
        const int groupid = 0;
        wrmcapture_OutMetaSequenceTransport wrm_trans("./", "./hash-", "xxx", now, 800, 600, groupid, nullptr);
        wrm_trans.send("AAAAX", 5);
        wrm_trans.send("BBBBX", 5);
        wrm_trans.next();
        wrm_trans.send("CCCCX", 5);

// TODO: we can't really check files are here and of expected size
// because they will only be flushed when wrm_trans object destructor is called.
// we should call a flush or explicit close for that purpose
// but it's no part yet of our Transport API.
//        BOOST_CHECK_EQUAL(10, filesize("./xxx-000000.wrm"));
//        BOOST_CHECK_EQUAL(5, filesize("./xxx-000001.wrm"));
//        BOOST_CHECK_EQUAL(69, filesize("./hash-xxx.mwrm"));
//        BOOST_CHECK_EQUAL(209, filesize("./xxx.mwrm"));

        wrm_trans.request_full_cleaning();
    }

    // TODO: request full_cleaning does not remove hash signature
    // not sure what to do with that and even if request full cleaning
    // is a good idea. Wouldn't it remove partial traces whenever
    // a problem occurs (like full disk ?)
    ::unlink("./hash-xxx.mwrm");

    BOOST_CHECK_EQUAL(-1 , filesize("./xxx-000000.wrm"));
    BOOST_CHECK_EQUAL(-1 , filesize("./xxx-000001.wrm"));
    BOOST_CHECK_EQUAL(-1 , filesize("./hash-xxx.mwrm"));
    BOOST_CHECK_EQUAL(-1 , filesize("./xxx.mwrm"));
}

//void simple_movie(timeval now, unsigned duration, RDPDrawable & drawable, gdi::CaptureApi & capture, bool ignore_frame_in_timeval, bool mouse);

//void simple_movie(timeval now, unsigned duration, RDPDrawable & drawable, gdi::CaptureApi & capture, bool ignore_frame_in_timeval, bool mouse)
//{
//    Rect screen(0, 0, drawable.width(), drawable.height());
//    auto const color_cxt = gdi::ColorCtx::depth24();
//    drawable.draw(RDPOpaqueRect(screen, BLUE), screen, color_cxt);

//    uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
//    Rect r(10, 10, 50, 50);
//    int vx = 5;
//    int vy = 4;
//    for (size_t x = 0; x < duration; x++) {
//        drawable.draw(RDPOpaqueRect(r, BLUE), screen, color_cxt);
//        r.y += vy;
//        r.x += vx;
//        drawable.draw(RDPOpaqueRect(r, WABGREEN), screen, color_cxt);
//        usec += 40000LL;
//        now.tv_sec  = usec / 1000000LL;
//        now.tv_usec = (usec % 1000000LL);
//        //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
//        int cursor_x = mouse?r.x + 10:0;
//        int cursor_y = mouse?r.y + 10:0;
//        drawable.set_mouse_cursor_pos(cursor_x, cursor_y);
//        capture.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
//        capture.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
//        if ((r.x + r.cx >= drawable.width())  || (r.x < 0)) { vx = -vx; }
//        if ((r.y + r.cy >= drawable.height()) || (r.y < 0)) { vy = -vy; }
//    }
//}

//BOOST_AUTO_TEST_CASE(TestSequencedVideoCapture)
//{
//    {
//        struct notified_on_video_change : public NotifyNextVideo
//        {
//            void notify_next_video(const timeval& now, reason reason)
//            {
//                LOG(LOG_INFO, "next video: now=%u:%u reason=%u",
//                    static_cast<unsigned>(now.tv_sec),
//                    static_cast<unsigned>(now.tv_usec),
//                    static_cast<unsigned>(reason));
//            }
//        } next_video_notifier;

//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
//        SequencedVideoCaptureImpl video_capture(now,
//            "./", "opaquerect_videocapture",
//            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
//            std::chrono::microseconds{2 * 1000000l}, next_video_notifier);
//        simple_movie(now, 250, drawable, video_capture, false, true);
//    }

//    struct CheckFiles {
//        const char * filename;
//        size_t size;
//    } fileinfo[] = {
//        {"./opaquerect_videocapture-000000.png", 3099},
//        {"./opaquerect_videocapture-000000.flv", 40677},
//        {"./opaquerect_videocapture-000001.png", 3104},
//        {"./opaquerect_videocapture-000001.flv", 40011},
//        {"./opaquerect_videocapture-000002.png", 3107},
//        {"./opaquerect_videocapture-000002.flv", 41172},
//        {"./opaquerect_videocapture-000003.png", 3099},
//        {"./opaquerect_videocapture-000003.flv", 40610},
//        {"./opaquerect_videocapture-000004.png", 3098},
//        {"./opaquerect_videocapture-000004.flv", 40173},
//        {"./opaquerect_videocapture-000005.png", 3098},
//        {"./opaquerect_videocapture-000005.flv", 13539},
//    };
//    for (auto x: fileinfo) {
//        size_t fsize = filesize(x.filename);
//        BOOST_CHECK_EQUAL(x.size, fsize);
//        ::unlink(x.filename);
//    }
//}

//BOOST_AUTO_TEST_CASE(TestSequencedVideoCaptureMP4)
//{
//    {
//        struct notified_on_video_change : public NotifyNextVideo
//        {
//            void notify_next_video(const timeval& now, reason reason)
//            {
//                LOG(LOG_INFO, "next video: now=%u:%u reason=%u",
//                    static_cast<unsigned>(now.tv_sec),
//                    static_cast<unsigned>(now.tv_usec),
//                    static_cast<unsigned>(reason));
//            }
//        } next_video_notifier;

//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", 0};
//        SequencedVideoCaptureImpl video_capture(now,
//            "./", "opaquerect_videocapture",
//            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
//            std::chrono::microseconds{2 * 1000000l}, next_video_notifier);
//        simple_movie(now, 250, drawable, video_capture, false, true);
//    }

//    struct CheckFiles {
//        const char * filename;
//        size_t size;
//        size_t alternativesize;
//    } fileinfo[] = {
//        {"./opaquerect_videocapture-000000.png", 3099, 3099},
//        {"./opaquerect_videocapture-000000.mp4", 12999, 12985},
//        {"./opaquerect_videocapture-000001.png", 3104, 3104},
//        {"./opaquerect_videocapture-000001.mp4", 11726, 11712},
//        {"./opaquerect_videocapture-000002.png", 3107, 3107},
//        {"./opaquerect_videocapture-000002.mp4", 10798, 10784},
//        {"./opaquerect_videocapture-000003.png", 3099, 3099},
//        {"./opaquerect_videocapture-000003.mp4", 11329, 11315},
//        {"./opaquerect_videocapture-000004.png", 3098, 3098},
//        {"./opaquerect_videocapture-000004.mp4", 12331, 12317},
//        {"./opaquerect_videocapture-000005.png", 3098, 3098},
//        {"./opaquerect_videocapture-000005.mp4", 262, 0},
//    };
//    for (auto x: fileinfo) {
//        size_t fsize = filesize(x.filename);
//        if (fsize != x.size && fsize != x.alternativesize){
//            BOOST_CHECK_EQUAL(x.size, fsize);
//        }
//        ::unlink(x.filename);
//    }
//}

//BOOST_AUTO_TEST_CASE(TestVideoCaptureOneChunkFLV)
//{
//    struct notified_on_video_change : public NotifyNextVideo
//    {
//        void notify_next_video(const timeval& now, reason reason)
//        {
//            LOG(LOG_INFO, "next video: now=%u:%u reason=%u",
//                static_cast<unsigned>(now.tv_sec),
//                static_cast<unsigned>(now.tv_usec),
//                static_cast<unsigned>(reason));
//        }
//    } next_video_notifier;

//    {
//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
//        SequencedVideoCaptureImpl video_capture(now,
//            "./", "opaquerect_videocapture_one_chunk_xxx",
//            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
//            std::chrono::microseconds{1000 * 1000000l}, next_video_notifier);
//        simple_movie(now, 1000, drawable, video_capture, false, true);
//    }

//    struct CheckFiles {
//        const char * filename;
//        size_t size;
//    } fileinfo[] = {
//        {"./opaquerect_videocapture_one_chunk_xxx-000000.png", 3099},
//        {"./opaquerect_videocapture_one_chunk_xxx-000000.flv", 645722},
//        {"./opaquerect_videocapture_one_chunk_xxx-000001.png", static_cast<long unsigned>(-1)},
//        {"./opaquerect_videocapture_one_chunk_xxx-000001.flv", static_cast<long unsigned>(-1)},
//    };
//    for (auto x: fileinfo) {
//        size_t fsize = filesize(x.filename);
//        BOOST_CHECK_EQUAL(x.size, fsize);
//        ::unlink(x.filename);
//    }
//}

//BOOST_AUTO_TEST_CASE(TestFullVideoCaptureFlv)
//{
//    {
//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
//        FullVideoCaptureImpl video_capture(now,
//            "./", "opaquerect_fullvideocapture_timestamp1",
//            0 /* groupid */, false /* no_timestamp */, drawable, flv_params);
//        simple_movie(now, 250, drawable, video_capture, false, true);
//    }
//    const char * filename = "./opaquerect_fullvideocapture_timestamp1.flv";
//    BOOST_CHECK_EQUAL(164693, filesize(filename));
//    ::unlink(filename);

//}

//BOOST_AUTO_TEST_CASE(TestFullVideoCaptureFlv2)
//{
//    {
//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
//        FullVideoCaptureImpl video_capture(now,
//            "./", "opaquerect_fullvideocapture_timestamp_mouse0",
//            0 /* groupid */, false /* no_timestamp */, drawable, flv_params);
//        simple_movie(now, 250, drawable, video_capture, false, false);
//    }
//    const char * filename = "./opaquerect_fullvideocapture_timestamp_mouse0.flv";
//    BOOST_CHECK_EQUAL(158699, filesize(filename));
//    ::unlink(filename);

//}

//BOOST_AUTO_TEST_CASE(TestFullVideoCaptureX264)
//{
//    {
//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", 0};
//        FullVideoCaptureImpl video_capture(now,
//            "./", "opaquerect_fullvideocapture_timestamp2",
//            0 /* groupid */, false /* no_timestamp */, drawable, flv_params);
//        simple_movie(now, 250, drawable, video_capture, false, true);
//    }
//    const char * filename = "./opaquerect_fullvideocapture_timestamp2.mp4";
//    size_t fsize = filesize(filename);
//     // size actually depends on the codec version and at least two slightltly different ones exists for h264
//    if (fsize != 54190 && fsize != 54176){
//        BOOST_CHECK_EQUAL(54190, filesize(filename));
//    }
//    ::unlink(filename);

//}

//BOOST_AUTO_TEST_CASE(SequencedVideoCaptureFLV)
//{
//    struct notified_on_video_change : public NotifyNextVideo
//    {
//        void notify_next_video(const timeval& now, reason reason)
//        {
//            LOG(LOG_INFO, "next video: now=%u:%u reason=%u",
//                static_cast<unsigned>(now.tv_sec),
//                static_cast<unsigned>(now.tv_usec),
//                static_cast<unsigned>(reason));
//        }
//    } next_video_notifier;

//    {
//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
//        SequencedVideoCaptureImpl video_capture(now,
//            "./", "opaquerect_seqvideocapture",
//            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
//            std::chrono::microseconds{1000000}, next_video_notifier);
//        simple_movie(now, 250, drawable, video_capture, false, true);
//    }
//
//    struct CheckFiles {
//        const char * filename;
//        size_t size;
//    } fileinfo[] = {
//        {"./opaquerect_seqvideocapture-000000.png", 3099},
//        {"./opaquerect_seqvideocapture-000000.flv", 29439},
//        {"./opaquerect_seqvideocapture-000001.png", 3099},
//        {"./opaquerect_seqvideocapture-000001.flv", 30726},
//        {"./opaquerect_seqvideocapture-000002.png", 3104},
//        {"./opaquerect_seqvideocapture-000002.flv", 29119},
//        {"./opaquerect_seqvideocapture-000003.png", 3101},
//        {"./opaquerect_seqvideocapture-000003.flv", 29108},
//        {"./opaquerect_seqvideocapture-000004.png", 3107},
//        {"./opaquerect_seqvideocapture-000004.flv", 29088},
//        {"./opaquerect_seqvideocapture-000005.png", 3101},
//        {"./opaquerect_seqvideocapture-000005.flv", 30560},
//        {"./opaquerect_seqvideocapture-000006.png", 3099},
//        {"./opaquerect_seqvideocapture-000006.flv", 29076},
//        {"./opaquerect_seqvideocapture-000007.png", 3101},
//        {"./opaquerect_seqvideocapture-000007.flv", 30125},
//        {"./opaquerect_seqvideocapture-000008.png", 3098},
//        {"./opaquerect_seqvideocapture-000008.flv", 28966},
//        {"./opaquerect_seqvideocapture-000009.png", 3098},
//        {"./opaquerect_seqvideocapture-000009.flv", 29309},
//        {"./opaquerect_seqvideocapture-000010.png", 3098},
//        {"./opaquerect_seqvideocapture-000010.flv", 13539}
//    };
//    for (auto x: fileinfo) {
//        size_t fsize = filesize(x.filename);
//        BOOST_CHECK_EQUAL(x.size, fsize);
//        ::unlink(x.filename);
//    }
//}


//BOOST_AUTO_TEST_CASE(SequencedVideoCaptureX264)
//{
//    struct notified_on_video_change : public NotifyNextVideo
//    {
//        void notify_next_video(const timeval& now, reason reason)
//        {
//            LOG(LOG_INFO, "next video: now=%u:%u reason=%u",
//                static_cast<unsigned>(now.tv_sec),
//                static_cast<unsigned>(now.tv_usec),
//                static_cast<unsigned>(reason));
//        }
//    } next_video_notifier;

//    {
//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", 0};
//        SequencedVideoCaptureImpl video_capture(now,
//            "./", "opaquerect_seqvideocapture_timestamp2",
//            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
//            std::chrono::microseconds{1000000}, next_video_notifier);
//        simple_movie(now, 250, drawable, video_capture, false, true);
//    }
//    struct CheckFiles {
//        const char * filename;
//        size_t size;
//        // size actually depends on the codec version and at least two slightltly different ones exists for h264
//        size_t alternativesize;
//    } fileinfo[] = {
//        {"./opaquerect_seqvideocapture_timestamp2-000000.png", 3099, 3099},
//        {"./opaquerect_seqvideocapture_timestamp2-000000.mp4", 7323, 7309},
//        {"./opaquerect_seqvideocapture_timestamp2-000001.png", 3099, 3099},
//        {"./opaquerect_seqvideocapture_timestamp2-000001.mp4", 6889, 6875},
//        {"./opaquerect_seqvideocapture_timestamp2-000002.png", 3104, 3104},
//        {"./opaquerect_seqvideocapture_timestamp2-000002.mp4", 6629, 6615},
//        {"./opaquerect_seqvideocapture_timestamp2-000003.png", 3101, 3101},
//        {"./opaquerect_seqvideocapture_timestamp2-000003.mp4", 6385, 6371},
//        {"./opaquerect_seqvideocapture_timestamp2-000004.png", 3107, 3107},
//        {"./opaquerect_seqvideocapture_timestamp2-000004.mp4", 6013, 5999},
//        {"./opaquerect_seqvideocapture_timestamp2-000005.png", 3101, 3101},
//        {"./opaquerect_seqvideocapture_timestamp2-000005.mp4", 6036, 6022},
//        {"./opaquerect_seqvideocapture_timestamp2-000006.png", 3099, 3099},
//        {"./opaquerect_seqvideocapture_timestamp2-000006.mp4", 6133, 6119},
//        {"./opaquerect_seqvideocapture_timestamp2-000007.png", 3101, 3101},
//        {"./opaquerect_seqvideocapture_timestamp2-000007.mp4", 6410, 6396},
//        {"./opaquerect_seqvideocapture_timestamp2-000008.png", 3098, 3098},
//        {"./opaquerect_seqvideocapture_timestamp2-000008.mp4", 6631, 6617},
//        {"./opaquerect_seqvideocapture_timestamp2-000009.png", 3098, 3098},
//        {"./opaquerect_seqvideocapture_timestamp2-000009.mp4", 6876, 6862},
//        {"./opaquerect_seqvideocapture_timestamp2-000010.png", 3098, 3098},
//        {"./opaquerect_seqvideocapture_timestamp2-000010.mp4", 262, 262}
//    };
//    for (auto x: fileinfo) {
//        size_t fsize = filesize(x.filename);
//        if ((x.size != fsize) && (x.alternativesize != fsize)){
//            BOOST_CHECK_EQUAL(x.size, filesize(x.filename));
//        }
//        ::unlink(x.filename);
//    }
//}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM1)
{
    // This is what we are actually testing, chaining of several files content
    InMetaSequenceTransport wrm_trans(static_cast<CryptoContext*>(nullptr),
        FIXTURES_PATH "/sample", ".mwrm", 0);
    unsigned char buffer[10000];
    unsigned char * pbuffer = buffer;
    size_t total = 0;
    auto test = [&]{
        for (size_t i = 0; i < 221 ; i++){
            pbuffer = buffer;
            wrm_trans.recv_new(pbuffer, sizeof(buffer));
            total += sizeof(buffer);
        }
    };
    CHECK_EXCEPTION_ERROR_ID(test(), ERR_TRANSPORT_NO_MORE_DATA);
    total += pbuffer - buffer;
    // total size if sum of sample sizes
    BOOST_CHECK_EQUAL(2200000, total);                             // 1471394 + 444578 + 290245
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM1_v2)
{
    // This is what we are actually testing, chaining of several files content
    InMetaSequenceTransport wrm_trans(static_cast<CryptoContext*>(nullptr), FIXTURES_PATH "/sample_v2", ".mwrm", 0);
    unsigned char buffer[10000];
    unsigned char * pbuffer = buffer;
    size_t total = 0;
    auto test = [&]{
        for (size_t i = 0; i < 221 ; i++){
            pbuffer = buffer;
            wrm_trans.recv_new(pbuffer, sizeof(buffer));
            total += sizeof(buffer);
        }
    };
    CHECK_EXCEPTION_ERROR_ID(test(), ERR_TRANSPORT_NO_MORE_DATA);
    total += pbuffer - buffer;
    // total size if sum of sample sizes
    BOOST_CHECK_EQUAL(2200000, total);
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM2)
{
//        "800 600\n",
//        "0\n",
//        "\n",
//        FIXTURES_PATH "/sample0.wrm 1352304810 1352304870\n",
//        FIXTURES_PATH "/sample1.wrm 1352304870 1352304930\n",
//        FIXTURES_PATH "/sample2.wrm 1352304930 1352304990\n",

    // This is what we are actually testing, chaining of several files content
    {
        InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), FIXTURES_PATH "/sample", ".mwrm", 0);
        BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample0.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample1.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample2.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());

        CHECK_EXCEPTION_ERROR_ID(mwrm_trans.next(), ERR_TRANSPORT_NO_MORE_DATA);
    }

    // check we can do it two times
    InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), FIXTURES_PATH "/sample", ".mwrm", 0);

    BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample0.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample1.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample2.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM2_RIO)
{
//        "800 600\n",
//        "0\n",
//        "\n",
//        FIXTURES_PATH "/sample0.wrm 1352304810 1352304870\n",
//        FIXTURES_PATH "/sample1.wrm 1352304870 1352304930\n",
//        FIXTURES_PATH "/sample2.wrm 1352304930 1352304990\n",

    // This is what we are actually testing, chaining of several files content
    InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), FIXTURES_PATH "/sample", ".mwrm", 0);
    BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample0.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample1.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample2.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());

    CHECK_EXCEPTION_ERROR_ID(mwrm_trans.next(), ERR_TRANSPORT_NO_MORE_DATA);
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM3)
{
//        "800 600\n",
//        "0\n",
//        "\n",
//        "/var/rdpproxy/recorded/sample0.wrm 1352304810 1352304870\n",
//        "/var/rdpproxy/recorded/sample1.wrm 1352304870 1352304930\n",
//        "/var/rdpproxy/recorded/sample2.wrm 1352304930 1352304990\n",

    // This is what we are actually testing, chaining of several files content

    {
        InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), FIXTURES_PATH "/moved_sample", ".mwrm", 0);
        BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample0.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample1.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample2.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());

        CHECK_EXCEPTION_ERROR_ID(mwrm_trans.next(), ERR_TRANSPORT_NO_MORE_DATA);
    }

    // check we can do it two times
    InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), FIXTURES_PATH "/moved_sample", ".mwrm", 0);

    BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample0.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample1.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL(FIXTURES_PATH "/sample2.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());
}

BOOST_AUTO_TEST_CASE(TestCryptoInmetaSequenceTransport)
{
    OpenSSL_add_all_digests();

    // cleanup of possible previous test files
    {
        const char * file[] = {"/tmp/TESTOFS.mwrm", "TESTOFS.mwrm", "TESTOFS-000000.wrm", "TESTOFS-000001.wrm"};
        for (size_t i = 0; i < sizeof(file)/sizeof(char*); ++i){
            ::unlink(file[i]);
        }
    }

    BOOST_CHECK(true);

    CryptoContext cctx;
    cctx.set_master_key(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));

    BOOST_CHECK(true);

    {
        LCGRandom rnd(0);
        timeval tv;
        tv.tv_usec = 0;
        tv.tv_sec = 1352304810;
        const int groupid = 0;
        wrmcapture_CryptoOutMetaSequenceTransport crypto_trans(cctx, rnd, "", "/tmp/", "TESTOFS", tv, 800, 600, groupid, nullptr);
        crypto_trans.send("AAAAX", 5);
        tv.tv_sec += 100;
        crypto_trans.timestamp(tv);
        crypto_trans.next();
        crypto_trans.send("BBBBXCCCCX", 10);
        tv.tv_sec += 100;
        crypto_trans.timestamp(tv);
        BOOST_CHECK(true);
    }

    {
        InMetaSequenceTransport crypto_trans(&cctx, "TESTOFS", ".mwrm", 1);

        char buffer[1024] = {};
        char * bob = buffer;

        BOOST_CHECK(true);

        BOOST_CHECK_NO_THROW(crypto_trans.recv_new(bob, 15));

        BOOST_CHECK(true);

        if (0 != memcmp(buffer, "AAAAXBBBBXCCCCX", 15)){
            BOOST_CHECK_EQUAL(0, buffer[15]); // this one should not have changed
            buffer[15] = 0;
            BOOST_CHECK(true);
            LOG(LOG_ERR, "expected \"AAAAXBBBBXCCCCX\" got \"%s\"", buffer);
            BOOST_CHECK(false);
        }

        BOOST_CHECK(true);
    }

    const char * file[] = {
        "/tmp/TESTOFS.mwrm", // hash
        "TESTOFS.mwrm",
        "TESTOFS-000000.wrm",
        "TESTOFS-000001.wrm"
    };
    for (size_t i = 0; i < sizeof(file)/sizeof(char*); ++i){
        if (::unlink(file[i])){
            BOOST_CHECK(false);
            LOG(LOG_ERR, "failed to unlink %s", file[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(CryptoTestInMetaSequenceTransport2)
{
    CryptoContext cctx;
    cctx.set_master_key(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));

    CHECK_EXCEPTION_ERROR_ID(InMetaSequenceTransport(&cctx, "TESTOFSXXX", ".mwrm", 1), ERR_TRANSPORT_OPEN_FAILED);
}
