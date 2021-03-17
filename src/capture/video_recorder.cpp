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
   Copyright (C) Wallix 2010-2019
   Author(s): Christophe Grosjean, Jonathan Poelen
*/

// TODO: this should move to src/system as results are highly dependent on compilation system
// maybe some external utility could detect the target/variant of library and could avoid
// non determinist sizes in generated movies.

#include "capture/video_recorder.hpp"

#ifndef REDEMPTION_NO_FFMPEG

extern "C" {
    // On Debian lenny and on higher debian/ubuntu distribution, ffmpeg includes
    // aren't localized on the same path (usr/include/ffmpeg for lenny,
    // /usr/include/libXXX for ubuntu/debian testing/unstable)
    #ifndef UINT64_C
    #define UINT64_C uint64_t
    #endif

    #include <libavutil/avutil.h>
    #include <libavutil/dict.h>
    #include <libavutil/imgutils.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

#include "core/error.hpp"
#include "cxx/diagnostic.hpp"
#include "utils/image_view.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/log.hpp"
#include "acl/auth_api.hpp"
#include "transport/file_transport.hpp"

#include <algorithm>

namespace
{
    struct default_av_free
    {
        void operator()(void * ptr) noexcept
        {
            av_free(ptr);
        }
    };

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 65, 0)
    // not sure which version of libavcodec
    void avio_context_free(AVIOContext** ptr)
    {
        av_free(*ptr);
        *ptr = nullptr;
    }
#endif

    void video_transport_log_error(Error const & error)
    {
        if (error.id == ERR_TRANSPORT_WRITE_FAILED) {
            LOG(LOG_ERR, "VideoTransport::send: %s [%d]", strerror(error.errnum), error.errnum);
        }
    }

    void video_transport_acl_report(AclReportApi * acl_report, int errnum)
    {
        if (errnum == ENOSPC) {
            if (acl_report){
                acl_report->report("FILESYSTEM_FULL", "100|unknown");
            }
            else {
                LOG(LOG_ERR, "FILESYSTEM_FULL:100|unknown");
            }
        }
    }

    template<class Transport>
    struct IOVideoRecorderWithTransport
    {
        static int write(void * opaque, uint8_t * buf, int buf_size)
        {
            Transport * trans       = static_cast<Transport*>(opaque);
            int         return_code = buf_size;
            try {
                trans->send(buf, buf_size);
            }
            catch (Error const & e) {
                if (e.id == ERR_TRANSPORT_WRITE_NO_ROOM) {
                    LOG(LOG_ERR, "Video write_packet failure, no space left on device (id=%u)", e.id);
                }
                else {
                    LOG(LOG_ERR, "Video write_packet failure (id=%u, errnum=%d)", e.id, e.errnum);
                }
                return_code = -1;
            }
            return return_code;
        }

        static int64_t seek(void * opaque, int64_t offset, int whence)
        {
            // This function is like the fseek() C stdio function.
            // "whence" can be either one of the standard C values
            // (SEEK_SET, SEEK_CUR, SEEK_END) or one more value: AVSEEK_SIZE.

            // When "whence" has this value, your seek function must
            // not seek but return the size of your file handle in bytes.
            // This is also optional and if you don't implement it you must return <0.

            // Otherwise you must return the current position of your stream
            //  in bytes (that is, after the seeking is performed).
            // If the seek has failed you must return <0.
            if (whence == AVSEEK_SIZE) {
                LOG(LOG_ERR, "Video seek failure");
                return -1;
            }
            try {
                Transport * trans = static_cast<Transport*>(opaque);
                trans->seek(offset, whence);
                return offset;
            }
            catch (Error const & e){
                LOG(LOG_ERR, "Video seek failure (id=%u)", e.id);
                return -1;
            }
        }
    };
} // anonymous namespace

struct video_recorder::D
{
    std::string final_filename;
    std::string tmp_filename;
    OutFileTransport out_file;

    AVStream* video_st = nullptr;

    AVFrame* picture = nullptr;
    AVFrame* original_picture = nullptr;

    AVCodecContext* codec_ctx = nullptr;
    AVFormatContext* oc = nullptr;
    SwsContext* img_convert_ctx = nullptr;

    AVPacket pkt;
    int original_height;

    std::unique_ptr<uint8_t, default_av_free> picture_buf;
    std::unique_ptr<uint8_t, default_av_free> video_outbuf;

    /* custom IO */
    std::unique_ptr<uint8_t, default_av_free> custom_io_buffer;
    AVIOContext* custom_io_context = nullptr;

    static unique_fd open_file(char* tmp_filename, const int groupid, AclReportApi * acl_report)
    {
        int fd = ::mkostemps(tmp_filename, 4, O_WRONLY | O_CREAT);
        if (fd == -1) {
            int const errnum = errno;
            LOG( LOG_ERR, "can't open temporary file %s : %s [%d]"
               , tmp_filename, strerror(errnum), errnum);
            video_transport_acl_report(acl_report, errnum);
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errnum);
        }

        if (fchmod(fd, groupid ? (S_IRUSR|S_IRGRP) : S_IRUSR) == -1) {
            int const errnum = errno;
            LOG( LOG_ERR, "can't set file %s mod to %s : %s [%d]"
               , tmp_filename, groupid ? "u+r, g+r" : "u+r", strerror(errnum), errnum);
            ::close(fd);
            unlink(tmp_filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errnum);
        }

        return unique_fd{fd};
    }

    D(std::string_view filename, const int groupid, AclReportApi * acl_report)
    : final_filename(filename)
    , tmp_filename(str_concat(filename, "red-XXXXXX.tmp"_av))
    , out_file(open_file(tmp_filename.data(), groupid, acl_report), [acl_report](const Error & error){
        video_transport_log_error(error);
        video_transport_acl_report(acl_report, error.errnum);
    })
    , picture(av_frame_alloc())
    , original_picture(av_frame_alloc())
    {
      ::unlink(this->final_filename.c_str());
    }

    ~D()
    {
        this->out_file.close();
        if (::rename(this->tmp_filename.c_str(), this->final_filename.c_str()) < 0) {
            int const errnum = errno;
            LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed errno=%d : %s"
               , this->tmp_filename, this->final_filename, errnum, strerror(errnum));
        }

        avio_context_free(&this->custom_io_context);
        sws_freeContext(this->img_convert_ctx);
        avformat_free_context(this->oc);
        avcodec_free_context(&this->codec_ctx);
        av_frame_free(&this->picture);
        av_frame_free(&this->original_picture);
    }
};

template<class... ExtraMsg>
static void throw_if(bool test, char const* msg, ExtraMsg const& ... extra_msg)
{
    if (test) {
        if constexpr (sizeof...(ExtraMsg) == 0) {
            LOG(LOG_ERR, "video recorder: %s", msg);
        }
        else {
            LOG(LOG_ERR, "video recorder: %s%s", msg, extra_msg...);
        }
        throw Error(ERR_VIDEO_RECORDER);
    }
}

static void log_av_errnum(int errnum, char const* msg)
{
    char errbuf[AV_ERROR_MAX_STRING_SIZE]{};
    LOG(LOG_ERR, "video recorder: %s: %s",
        msg, av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, errnum));
}

static void check_errnum(int errnum, char const* msg)
{
    if (errnum < 0) {
        log_av_errnum(errnum, msg);
        throw Error(ERR_VIDEO_RECORDER);
    }
}

// https://libav.org/documentation/doxygen/master/output_8c-example.html
// https://libav.org/documentation/doxygen/master/encode_video_8c-example.html

video_recorder::video_recorder(
    char const* filename, const int groupid, AclReportApi * acl_report,
    ImageView const& image_view, int frame_rate,
    const char * codec_name, char const* codec_options, int log_level
)
: d(std::make_unique<D>(filename, groupid, acl_report))
{
    d->original_height = image_view.height();
    const int image_view_width = image_view.width();
    const int image_view_height = image_view.height();

    #if LIBAVCODEC_VERSION_MAJOR <= 57
    /* initialize libavcodec, and register all codecs and formats */
    av_register_all();
    #endif

    av_log_set_level(log_level);


    this->d->oc = avformat_alloc_context();
    throw_if(!this->d->oc, "Failed allocating output media context");

    /* auto detect the output format from the name. default is mpeg. */
    AVOutputFormat *fmt = av_guess_format(codec_name, nullptr, nullptr);
    throw_if(!fmt || fmt->video_codec == AV_CODEC_ID_NONE, "Could not find codec ", codec_name);

    const auto codec_id = fmt->video_codec;
    const AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;

    this->d->oc->oformat = fmt;

    // add the video streams using the default format codecs and initialize the codecs
    this->d->video_st = avformat_new_stream(this->d->oc, nullptr);
    throw_if(!this->d->video_st, "Could not find suitable output format");

    this->d->video_st->time_base = AVRational{1, frame_rate};

    AVCodec * codec = avcodec_find_encoder(codec_id);
    throw_if(!codec, "Codec not found");

    this->d->codec_ctx = avcodec_alloc_context3(codec);

    this->d->codec_ctx->codec_id = codec_id;
    this->d->codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;

    // this->d->codec_ctx->bit_rate = bitrate;
    // this->d->codec_ctx->bit_rate_tolerance = bitrate;
    // resolution must be a multiple of 2
    this->d->codec_ctx->width = image_view_width & ~1;
    this->d->codec_ctx->height = image_view_height & ~1;

    // time base: this is the fundamental unit of time (in seconds)
    // in terms of which frame timestamps are represented.
    // for fixed-fps content, timebase should be 1/framerate
    // and timestamp increments should be identically 1
    this->d->codec_ctx->time_base = AVRational{1, frame_rate};

    // impact: keyframe, filesize and time of generating
    // high value = ++time, --size
    // keyframe managed by this->d->pkt.flags |= AV_PKT_FLAG_KEY and av_interleaved_write_frame
    this->d->codec_ctx->gop_size = std::max(2, frame_rate);

    this->d->codec_ctx->pix_fmt = pix_fmt;

    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch")
    switch (codec_id){
        case AV_CODEC_ID_H264:
            //codec_ctx->coder_type = FF_CODER_TYPE_AC;
            //codec_ctx->flags2 = CODEC_FLAG2_WPRED | CODEC_FLAG2_MIXED_REFS |
            //                                CODEC_FLAG2_8X8DCT | CODEC_FLAG2_FASTPSKIP;

            //codec_ctx->partitions = X264_PART_I8X8 | X264_PART_P8X8 | X264_PART_I4X4;
            this->d->codec_ctx->me_range = 16;
            //codec_ctx->refs = 1;
            //codec_ctx->flags = CODEC_FLAG_4MV | CODEC_FLAG_LOOP_FILTER;
            // this->d->codec_ctx->flags |= AVFMT_NOTIMESTAMPS;
            this->d->codec_ctx->qcompress = 0.0;
            this->d->codec_ctx->max_qdiff = 4;
        break;
        case AV_CODEC_ID_MPEG1VIDEO:
            // Needed to avoid using macroblocks in which some coeffs overflow.
            // This does not happen with normal video, it just happens here as
            // the motion of the chroma plane does not match the luma plane.
            this->d->codec_ctx->mb_decision = 2;
        break;
    }
    REDEMPTION_DIAGNOSTIC_POP()

    // some formats want stream headers to be separate
    if(fmt->flags & AVFMT_GLOBALHEADER){
        this->d->codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    // dump_format can be handy for debugging
    // it dump information about file to stderr
    // dump_format(this->d->oc, 0, file, 1);

    // ************** open_video ****************
    // now we can open the audio and video codecs
    // and allocate the necessary encode buffers
    // find the video encoder

    {
        AVDictionary *av_dict = nullptr;
        SCOPE_EXIT(av_dict_free(&av_dict));
        int errnum = av_dict_parse_string(&av_dict, codec_options, "=", " ", 0);
        if (errnum < 0) {
            log_av_errnum(errnum, "av_dict_parse_string error");
        }

        check_errnum(avcodec_open2(this->d->codec_ctx, codec, &av_dict),
            "Failed to open codec, possible bad codec option");
    }

    check_errnum(avcodec_parameters_from_context(this->d->video_st->codecpar, this->d->codec_ctx),
        "Failed to copy codec parameters");

    int const video_outbuf_size = image_view_width * image_view_height * 3 * 5;

    this->d->video_outbuf.reset(static_cast<uint8_t*>(av_malloc(video_outbuf_size)));
    throw_if(!this->d->video_outbuf, "Failed to allocate video output buffer");
    av_init_packet(&this->d->pkt);
    this->d->pkt.data = this->d->video_outbuf.get();
    this->d->pkt.size = video_outbuf_size;

    // init picture frame
    {
        int const size = av_image_get_buffer_size(pix_fmt, image_view_width, image_view_height, 1);
        if (size) {
            this->d->picture_buf.reset(static_cast<uint8_t*>(av_malloc(size)));
            std::fill_n(this->d->picture_buf.get(), size, 0);
        }
        throw_if(!this->d->picture_buf, "Failed to allocate picture buf");
        av_image_fill_arrays(
            this->d->picture->data, this->d->picture->linesize,
            this->d->picture_buf.get(), pix_fmt, image_view_width, image_view_height, 1
        );

        this->d->picture->width = image_view_width;
        this->d->picture->height = image_view_height;
        this->d->picture->format = codec_id;
        this->d->original_picture->format = codec_id;
    }

    const std::size_t io_buffer_size = 32768;

    this->d->custom_io_buffer.reset(static_cast<unsigned char *>(av_malloc(io_buffer_size)));
    throw_if(!this->d->custom_io_buffer, "Failed to allocate io");

    this->d->custom_io_context = avio_alloc_context(
        this->d->custom_io_buffer.get(), // buffer
        io_buffer_size,               // buffer size
        1,                            // writable
        &this->d->out_file,           // user-specific data
        nullptr,                      // function for refilling the buffer, may be nullptr.
        IOVideoRecorderWithTransport<OutFileTransport>::write,
        IOVideoRecorderWithTransport<OutFileTransport>::seek
    );
    if (!this->d->custom_io_context) {
        throw Error(ERR_RECORDER_ALLOCATION_FAILED);
    }

    this->d->oc->pb = this->d->custom_io_context;

    check_errnum(avformat_write_header(this->d->oc, nullptr), "video recorder: Failed to write header");

    av_image_fill_arrays(
        this->d->original_picture->data, this->d->original_picture->linesize,
        image_view.data(), AV_PIX_FMT_BGR24, image_view.width(), image_view.height(), 1
    );

    this->d->img_convert_ctx = sws_getContext(
        image_view.width(), image_view.height(), AV_PIX_FMT_BGR24,
        image_view_width, image_view_height, pix_fmt,
        SWS_BICUBIC, nullptr, nullptr, nullptr
    );

    throw_if(!this->d->img_convert_ctx, "Cannot initialize the conversion context");
}

static std::pair<int, char const*> encode_frame(
    AVFrame* picture, AVFormatContext* oc, AVCodecContext* codec_ctx,
    AVStream* video_st, AVPacket* pkt)
{
    int errnum = avcodec_send_frame(codec_ctx, picture);

    if (errnum < 0) {
        return {errnum, "Failed encoding a video frame"};
    }

    while (errnum >= 0) {
        av_init_packet(pkt);
        errnum = avcodec_receive_packet(codec_ctx, pkt);

        if (errnum < 0) {
            if (errnum == AVERROR(EAGAIN) || errnum == AVERROR_EOF) {
                errnum = 0;
            }
            break;
        }

        av_packet_rescale_ts(pkt, codec_ctx->time_base, video_st->time_base);
        pkt->stream_index = video_st->index;
        /* Write the compressed frame to the media file. */
        errnum = av_interleaved_write_frame(oc, pkt);
        // av_packet_unref(&this->d->pkt);
    }

    return {errnum, "Failed while writing video frame"};
}

video_recorder::~video_recorder() /*NOLINT*/
{
    // flush the encoder
    auto [errnum, errmsg] = encode_frame(
        nullptr, this->d->oc, this->d->codec_ctx,
        this->d->video_st, &this->d->pkt);

    if (errnum < 0) {
        log_av_errnum(errnum, errmsg);
    }

    /* write the trailer, if any.  the trailer must be written
     * before you close the CodecContexts open when you wrote the
     * header; otherwise write_trailer may try to use memory that
     * was freed on av_codec_close() */
    av_write_trailer(this->d->oc);
}

void video_recorder::preparing_video_frame()
{
    /* stat */// LOG(LOG_INFO, "preparing_video_frame");
    sws_scale(
        this->d->img_convert_ctx,
        this->d->original_picture->data, this->d->original_picture->linesize,
        0, this->d->original_height, this->d->picture->data, this->d->picture->linesize);
}

void video_recorder::encoding_video_frame(int64_t frame_index)
{
    /* stat */// LOG(LOG_INFO, "encoding_video_frame %ld", frame_index);

    this->d->picture->pts = frame_index;
    auto [errnum, errmsg] = encode_frame(
        this->d->picture, this->d->oc, this->d->codec_ctx,
        this->d->video_st, &this->d->pkt);

    check_errnum(errnum, errmsg);
}

#else

struct video_recorder::D {};

video_recorder::video_recorder(
    write_packet_fn_t write_packet_fn, seek_fn_t /*seek_fn*/, void * io_params,
    ImageView const & /*image_view*/, int /*frame_rate*/,
    const char * /*codec_name*/, char const* /*codec_options*/, int /*log_level*/
) {
    uint8_t buf[1]{};
    // force file create
    write_packet_fn(io_params, buf, 0);
}

video_recorder::~video_recorder() = default;

void video_recorder::preparing_video_frame() { }

void video_recorder::encoding_video_frame(int64_t /*frame_index*/) { }

#endif
