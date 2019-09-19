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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

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

#ifndef CODEC_FLAG_QSCALE
#define CODEC_FLAG_QSCALE AV_CODEC_FLAG_QSCALE
#endif

#ifndef CODEC_FLAG_GLOBAL_HEADER
#define CODEC_FLAG_GLOBAL_HEADER AV_CODEC_FLAG_GLOBAL_HEADER
#endif

#ifndef AVFMT_RAWPICTURE
#define AVFMT_RAWPICTURE AVFMT_NOFILE
#endif

#ifdef exit
# undef exit
#endif

#ifndef AV_PKT_FLAG_KEY
#define AV_PKT_FLAG_KEY PKT_FLAG_KEY
#define AVMEDIA_TYPE_VIDEO CODEC_TYPE_VIDEO
#define av_guess_format guess_format
#endif

#include "core/error.hpp"
#include "cxx/diagnostic.hpp"
#include "utils/image_data_view.hpp"
#include "utils/log.hpp"

#include <algorithm>


void video_recorder::default_av_free::operator()(void * ptr)
{
    av_free(ptr);
}

void video_recorder::default_av_free_format_context::operator()(AVFormatContext * ctx)
{
    avformat_free_context(ctx);
}

void video_recorder::default_sws_free_context::operator()(SwsContext * sws_ctx)
{
    sws_freeContext(sws_ctx);
}

video_recorder::AVFramePtr::AVFramePtr() /*NOLINT*/
    : frame(av_frame_alloc())
{}

video_recorder::AVFramePtr::~AVFramePtr() /*NOLINT*/
{
    av_frame_free(&this->frame);
}

static void throw_if(bool test, char const* msg)
{
    if (test) {
        LOG(LOG_ERR, "video recorder: %s", msg);
        throw Error(ERR_VIDEO_RECORDER);
    }
}

static void check_errnum(int errnum, char const* msg)
{
    if (errnum < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE]{};
        LOG(LOG_ERR, "video recorder: %s: %s",
            msg, av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, errnum));
        throw Error(ERR_VIDEO_RECORDER);
    }
}

// https://libav.org/documentation/doxygen/master/output_8c-example.html
// https://libav.org/documentation/doxygen/master/encode_video_8c-example.html

video_recorder::video_recorder(
    write_packet_fn_t write_packet_fn, seek_fn_t seek_fn, void * io_params,
    ConstImageDataView const & image_view, int bitrate,
    int frame_rate, int qscale, const char * codec_name,
    int log_level
)
: original_height(image_view.height())
{
    const int image_view_width = image_view.width();
    const int image_view_height = image_view.height();

    /* initialize libavcodec, and register all codecs and formats */
    av_register_all();

    av_log_set_level(log_level);


    this->oc.reset(avformat_alloc_context());
    throw_if(!this->oc, "Failed allocating output media context");

    /* auto detect the output format from the name. default is mpeg. */
    AVOutputFormat *fmt = av_guess_format(codec_name, nullptr, nullptr);
    if (!fmt) {
        LOG(LOG_WARNING, "Could not deduce output format from codec: falling back to MPEG.");
        fmt = av_guess_format("mpeg", nullptr, nullptr);
    }
    throw_if(!fmt || fmt->video_codec == AV_CODEC_ID_NONE, "Could not find codec");

    const auto codec_id = fmt->video_codec;
    const AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;

    this->oc->oformat = fmt;
    //strncpy(this->oc->filename, file, sizeof(this->oc->filename));

    // add the video streams using the default format codecs and initialize the codecs
    this->video_st = avformat_new_stream(this->oc.get(), nullptr);

    throw_if(!this->video_st, "Could not find suitable output format");

    this->video_st->r_frame_rate.num = 1;
    this->video_st->r_frame_rate.den = frame_rate;

    this->video_st->time_base.num = 1;
    this->video_st->time_base.den = frame_rate;

    auto* codec_ctx = this->video_st->codec;

    codec_ctx->codec_id = codec_id;
    codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;

    codec_ctx->bit_rate = bitrate;
    codec_ctx->bit_rate_tolerance = bitrate;
    // resolution must be a multiple of 2
    codec_ctx->width = image_view_width & ~1;
    codec_ctx->height = image_view_height & ~1;

    // time base: this is the fundamental unit of time (in seconds)
    // in terms of which frame timestamps are represented.
    // for fixed-fps content, timebase should be 1/framerate
    // and timestamp increments should be identically 1
    codec_ctx->time_base.num = 1;
    codec_ctx->time_base.den = frame_rate;

    // impact: keyframe, filesize and time of generating
    // high value = ++time, --size
    // keyframe managed by this->pkt.flags |= AV_PKT_FLAG_KEY and av_interleaved_write_frame
    codec_ctx->gop_size = std::max(2, frame_rate);

    codec_ctx->pix_fmt = pix_fmt;
    codec_ctx->flags |= CODEC_FLAG_QSCALE; // TODO
    codec_ctx->global_quality = FF_QP2LAMBDA * qscale; // TODO

    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wswitch")
    switch (codec_id){
        case AV_CODEC_ID_H264:
            //codec_ctx->coder_type = FF_CODER_TYPE_AC;
            //codec_ctx->flags2 = CODEC_FLAG2_WPRED | CODEC_FLAG2_MIXED_REFS |
            //                                CODEC_FLAG2_8X8DCT | CODEC_FLAG2_FASTPSKIP;

            //codec_ctx->partitions = X264_PART_I8X8 | X264_PART_P8X8 | X264_PART_I4X4;
            codec_ctx->me_range = 16;
            //codec_ctx->refs = 1;
            //codec_ctx->flags = CODEC_FLAG_4MV | CODEC_FLAG_LOOP_FILTER;
            codec_ctx->flags |= AVFMT_NOTIMESTAMPS;
            codec_ctx->qcompress = 0.0;
            codec_ctx->max_qdiff = 4;
            //codec_ctx->gop_size = frame_rate;
        break;
        case AV_CODEC_ID_MPEG2VIDEO:
            //codec_ctx->gop_size = frame_rate;
        break;
        case AV_CODEC_ID_MPEG1VIDEO:
            // Needed to avoid using macroblocks in which some coeffs overflow.
            // This does not happen with normal video, it just happens here as
            // the motion of the chroma plane does not match the luma plane.
            codec_ctx->mb_decision = 2;
        break;
    }
    REDEMPTION_DIAGNOSTIC_POP

    // some formats want stream headers to be separate
    if(fmt->flags & AVFMT_GLOBALHEADER){
        codec_ctx->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    AVCodec * codec = avcodec_find_encoder(codec_id);
    throw_if(!codec, "Codec not found");

    // dump_format can be handy for debugging
    // it dump information about file to stderr
    // dump_format(this->oc, 0, file, 1);

    // ************** open_video ****************
    // now we can open the audio and video codecs
    // and allocate the necessary encode buffers
    // find the video encoder

    {
        struct AVDict {
            void add(char const * k, char const * v) {
                if (av_dict_set(&this->d, k, v, 0) < 0) {
                    LOG(LOG_ERR, "av_dict_set error on '%s' with '%s'", k, v);
                }
            }
            ~AVDict() { av_dict_free(&this->d); } /*NOLINT*/
            AVDictionary *d = nullptr;
        } av_dict;

        if (codec_id == AV_CODEC_ID_H264) {
            // low quality  (baseline, main, hight, ...)
            av_dict.add("profile", "baseline");
            av_dict.add("preset", "ultrafast");
            //av_dict.add("vsync", "2");
        }

        // open the codec
        check_errnum(avcodec_open2(this->video_st->codec, codec, &av_dict.d), "Failed to open codec");
    }

    struct AvCodecPtr
    {
        video_recorder * ptr;
        ~AvCodecPtr() {
            if (ptr) {
                avcodec_close(ptr->video_st->codec);
            }
        }
    };
    AvCodecPtr av_codec_close_if_fails{this};

    int const video_outbuf_size = image_view_width * image_view_height * 3 * 5;

    this->video_outbuf.reset(static_cast<uint8_t*>(av_malloc(video_outbuf_size)));
    throw_if(!this->video_outbuf, "Failed to allocate video output buffer");
    av_init_packet(&this->pkt);
    this->pkt.data = this->video_outbuf.get();
    this->pkt.size = video_outbuf_size;

    // init picture frame
    {
        int const size = av_image_get_buffer_size(pix_fmt, image_view_width, image_view_height, 1);
        if (size) {
            this->picture_buf.reset(static_cast<uint8_t*>(av_malloc(size)));
            std::fill_n(this->picture_buf.get(), size, 0);
        }
        throw_if(!this->picture_buf, "Failed to allocate picture buf");
        av_image_fill_arrays(
            this->picture->data, this->picture->linesize,
            this->picture_buf.get(), pix_fmt, image_view_width, image_view_height, 1
        );

        this->picture->width = image_view_width;
        this->picture->height = image_view_height;
        this->picture->format = codec_id;
        this->original_picture->format = codec_id;
    }

    const std::size_t io_buffer_size = 32768;

    this->custom_io_buffer.reset(static_cast<unsigned char *>(av_malloc(io_buffer_size)));
    throw_if(!this->custom_io_buffer, "Failed to allocate io");

    this->custom_io_context.reset(avio_alloc_context(
        this->custom_io_buffer.get(), // buffer
        io_buffer_size,               // buffer size
        1,                            // writable
        io_params,                    // user-specific data
        nullptr,                      // function for refilling the buffer, may be nullptr.
        write_packet_fn,              // function for writing the buffer contents, may be nullptr.
        seek_fn                       // function for seeking to specified byte position, may be nullptr.
    ));
    if (!this->custom_io_context) {
        throw Error(ERR_RECORDER_ALLOCATION_FAILED);
    }

    this->oc->pb = this->custom_io_context.get();

    int res = avformat_write_header(this->oc.get(), nullptr);
    if (res < 0){
        LOG(LOG_ERR, "video recorder: failed to write header");
    }

    av_image_fill_arrays(
        this->original_picture->data, this->original_picture->linesize,
        image_view.data(), AV_PIX_FMT_BGR24, image_view.width(), image_view.height(), 1
    );

    this->img_convert_ctx.reset(sws_getContext(
        image_view.width(), image_view.height(), AV_PIX_FMT_BGR24,
        image_view_width, image_view_height, pix_fmt,
        SWS_BICUBIC, nullptr, nullptr, nullptr
    ));

    throw_if(!this->img_convert_ctx, "Cannot initialize the conversion context");

    av_codec_close_if_fails.ptr = nullptr;
}

video_recorder::~video_recorder() /*NOLINT*/
{
    // write last frame : we must ensure writing at least one frame to avoid empty movies
    // encoding_video_frame();

    // write the last second for mp4 (if preset != ultrafast ...)
    //if (bool(this->video_st->codec->flags & AVFMT_NOTIMESTAMPS)) {
    //    auto const frame_rate = 1000u / this->duration_frame.count();
    //    int const loop = frame_rate - this->frame_key % frame_rate;
    //    for (int i = 0; i < loop; ++i) {
    //        encoding_video_frame();
    //    }
    // --- or --- ?
    // int got_packet = 1;
    // for(; got_packet;)
    //    avcodec_encode_video2(this->video_st->codec, &this->pkt, nullptr, &got_packet);
    //}

    /* write the trailer, if any.  the trailer must be written
        * before you close the CodecContexts open when you wrote the
        * header; otherwise write_trailer may try to use memory that
        * was freed on av_codec_close() */
    av_write_trailer(this->oc.get());

    // close each codec */
    avcodec_close(this->video_st->codec);
}

void video_recorder::preparing_video_frame()
{
    /* stat */// LOG(LOG_INFO, "%s", "preparing_video_frame");
    sws_scale(
        this->img_convert_ctx.get(),
        this->original_picture->data, this->original_picture->linesize,
        0, this->original_height, this->picture->data, this->picture->linesize);
}

void video_recorder::encoding_video_frame(uint64_t frame_index)
{
    /* stat */// LOG(LOG_INFO, "encoding_video_frame");

    /**
    * Encode a frame of video.
    *
    * Takes input raw video data from frame and writes the next output packet, if
    * available, to avpkt. The output packet does not necessarily contain data for
    * the most recent frame, as encoders can delay and reorder input frames
    * internally as needed.
    *
    * @param avctx     codec context
    * @param avpkt     output AVPacket.
    *                  The user can supply an output buffer by setting
    *                  avpkt->data and avpkt->size prior to calling the
    *                  function, but if the size of the user-provided data is not
    *                  large enough, encoding will fail. All other AVPacket fields
    *                  will be reset by the encoder using av_init_packet(). If
    *                  avpkt->data is NULL, the encoder will allocate it.
    *                  The encoder will set avpkt->size to the size of the
    *                  output packet. The returned data (if any) belongs to the
    *                  caller, he is responsible for freeing it.
    * @param[in] frame AVFrame containing the raw video data to be encoded.
    *                  May be NULL when flushing an encoder that has the
    *                  CODEC_CAP_DELAY capability set.
    * @param[out] got_packet_ptr This field is set to 1 by libavcodec if the
    *                            output packet is non-empty, and to 0 if it is
    *                            empty. If the function returns an error, the
    *                            packet can be assumed to be invalid, and the
    *                            value of got_packet_ptr is undefined and should
    *                            not be used.
    * @return          0 on success, negative error code on failure
    */
    // int avcodec_encode_video2(AVCodecContext *avctx, AVPacket *avpkt, const AVFrame *frame, int *got_packet_ptr);

    auto const frame_interval = this->video_st->r_frame_rate.den;
    auto const pts = this->video_st->time_base.den * frame_index / frame_interval;
    auto const old_pts = this->video_st->time_base.den * this->old_frame_index / frame_interval;
    auto const dur = pts - old_pts;
    //auto const dur = this->video_st->time_base.den * (frame_index - this->old_frame_index) / frame_interval;

    this->pkt.pts = pts;
    this->picture->pts = pts;
    this->pkt.duration = dur;

    int got_packet = 0;
    int err = avcodec_encode_video2(
        this->video_st->codec,
        &this->pkt,
        this->picture.get(),
        &got_packet);

    if (err == 0 && got_packet) {
        //if (this->frame_key == frame_key_limit) {
        //    this->pkt.flags |= AV_PKT_FLAG_KEY;
        //    this->frame_key = 0;
        //}
        //++this->frame_key;
        this->pkt.stream_index = this->video_st->index;
        this->pkt.duration = dur;
        this->pkt.pts = pts;
        this->pkt.dts = old_pts;

        this->old_frame_index = frame_index;

        err = av_interleaved_write_frame(this->oc.get(), &this->pkt);
    }

    check_errnum(err, "Failed to write encoded frame");
}

#else

video_recorder::video_recorder(
    write_packet_fn_t write_packet_fn, seek_fn_t /*seek_fn*/, void * io_params,
    ConstImageDataView const & /*image_view*/, int /*bitrate*/,
    int /*frame_rate*/, int /*qscale*/, const char * /*codec_id*/,
    int /*log_level*/
) {
    uint8_t buf[1]{};
    // force file create
    write_packet_fn(io_params, buf, 0);
}

video_recorder::~video_recorder() = default;

void video_recorder::preparing_video_frame() {};

void video_recorder::encoding_video_frame(uint64_t /*frame_index*/) {};

#endif
