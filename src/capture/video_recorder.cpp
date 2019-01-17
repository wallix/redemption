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

#include <cstdint>
#include <cstdlib>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

#define STREAM_PIX_FMT AV_PIX_FMT_YUV420P /* default pix_fmt */
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

video_recorder::video_recorder(
    write_packet_fn_t write_packet_fn, seek_fn_t seek_fn, void * io_params,
    ConstImageDataView const & image_view, int bitrate,
    int frame_rate, int qscale, const char * codec_id,
    int log_level
)
: original_height(image_view.height())
{
    const int image_view_width = image_view.width();
    const int image_view_height = image_view.height();

    /* initialize libavcodec, and register all codecs and formats */
    av_register_all();

    this->oc.reset(avformat_alloc_context());
    if (!this->oc) {
        LOG(LOG_ERR, "recorder failed allocating output media context");
        throw Error(ERR_RECORDER_FAILED_ALLOCATING_OUTPUT_MEDIA_CONTEXT);
    }

    /* auto detect the output format from the name. default is mpeg. */
    AVOutputFormat *fmt = av_guess_format(codec_id, nullptr, nullptr);
    if (!fmt) {
        LOG(LOG_WARNING, "Could not deduce output format from codec: falling back to MPEG.");
        fmt = av_guess_format("mpeg", nullptr, nullptr);
    }
    if (!fmt) {
        LOG(LOG_ERR, "Could not find suitable output format");
        throw Error(ERR_RECORDER_NO_OUTPUT_CODEC);
    }

    if (fmt->video_codec == AV_CODEC_ID_NONE) {
        LOG(LOG_ERR, "video recorder error : no codec defined");
        throw Error(ERR_RECORDER_CODEC_NOT_FOUND);
    }

    this->oc->oformat = fmt;
    //strncpy(this->oc->filename, file, sizeof(this->oc->filename));

    // add the video streams using the default format codecs and initialize the codecs
    this->video_st = avformat_new_stream(this->oc.get(), nullptr);

    if (!this->video_st) {
        LOG(LOG_ERR, "Could not find suitable output format");
        throw Error(ERR_RECORDER_FAILED_TO_ALLOC_STREAM);
    }

    this->video_st->r_frame_rate.num = 1;
    this->video_st->r_frame_rate.den = frame_rate;

    this->video_st->time_base.num = 1;
    this->video_st->time_base.den = frame_rate;

    this->video_st->codec->codec_id = fmt->video_codec;
    this->video_st->codec->codec_type = AVMEDIA_TYPE_VIDEO;

    this->video_st->codec->bit_rate = bitrate;
    this->video_st->codec->bit_rate_tolerance = bitrate;
    // resolution must be a multiple of 2
    this->video_st->codec->width = image_view_width & ~1;
    this->video_st->codec->height = image_view_height & ~1;

    // time base: this is the fundamental unit of time (in seconds)
    // in terms of which frame timestamps are represented.
    // for fixed-fps content, timebase should be 1/framerate
    // and timestamp increments should be identically 1
    this->video_st->codec->time_base.num = 1;
    this->video_st->codec->time_base.den = frame_rate;

    // impact: keyframe, filesize and time of generating
    // high value = ++time, --size
    // keyframe managed by this->pkt.flags |= AV_PKT_FLAG_KEY and av_interleaved_write_frame
    this->video_st->codec->gop_size = std::max(2, frame_rate);

    this->video_st->codec->pix_fmt = STREAM_PIX_FMT;
    this->video_st->codec->flags |= CODEC_FLAG_QSCALE; // TODO
    this->video_st->codec->global_quality = FF_QP2LAMBDA * qscale; // TODO

    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
    switch (this->video_st->codec->codec_id){
        case AV_CODEC_ID_H264:
            //this->video_st->codec->coder_type = FF_CODER_TYPE_AC;
            //this->video_st->codec->flags2 = CODEC_FLAG2_WPRED | CODEC_FLAG2_MIXED_REFS |
            //                                CODEC_FLAG2_8X8DCT | CODEC_FLAG2_FASTPSKIP;

            //this->video_st->codec->partitions = X264_PART_I8X8 | X264_PART_P8X8 | X264_PART_I4X4;
            this->video_st->codec->me_range = 16;
            //this->video_st->codec->refs = 1;
            //this->video_st->codec->flags = CODEC_FLAG_4MV | CODEC_FLAG_LOOP_FILTER;
            this->video_st->codec->flags |= AVFMT_NOTIMESTAMPS;
            this->video_st->codec->qcompress = 0.0;
            this->video_st->codec->max_qdiff = 4;
            //this->video_st->codec->gop_size = frame_rate;
        break;
        case AV_CODEC_ID_MPEG2VIDEO:
            //this->video_st->codec->gop_size = frame_rate;
        break;
        case AV_CODEC_ID_MPEG1VIDEO:
            // Needed to avoid using macroblocks in which some coeffs overflow.
            // This does not happen with normal video, it just happens here as
            // the motion of the chroma plane does not match the luma plane.
            this->video_st->codec->mb_decision = 2;
        break;
        case AV_CODEC_ID_MPEG4:
        break;
        case AV_CODEC_ID_FLV1:
        break;
        default:
        break;
    }
    REDEMPTION_DIAGNOSTIC_POP

    // some formats want stream headers to be separate
    if(this->oc->oformat->flags & AVFMT_GLOBALHEADER){
        this->video_st->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    AVCodec * codec = avcodec_find_encoder(this->video_st->codec->codec_id);
    if (codec == nullptr) {
        LOG(LOG_ERR, "video recorder error : codec not found");
        throw Error(ERR_RECORDER_CODEC_NOT_FOUND);
    }

    // dump_format can be handy for debugging
    // it dump information about file to stderr
    // dump_format(this->oc, 0, file, 1);

    // ************** open_video ****************
    // now we can open the audio and video codecs
    // and allocate the necessary encode buffers
    // find the video encoder

    int const video_outbuf_size = image_view_width * image_view_height * 3 * 5;

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

        if (this->video_st->codec->codec_id == AV_CODEC_ID_H264) {
            // low quality  (baseline, main, hight, ...)
            av_dict.add("profile", "baseline");
            av_dict.add("preset", "ultrafast");
            //av_dict.add("vsync", "2");
        }

        // open the codec
        if (avcodec_open2(this->video_st->codec, codec, &av_dict.d) < 0) {
            LOG(LOG_ERR, "video recorder error : failed to open codec");
            throw Error(ERR_RECORDER_FAILED_TO_OPEN_CODEC);
        }
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

    if (!(this->oc->oformat->flags & AVFMT_RAWPICTURE)) {
        /* allocate output buffer */
        /* XXX: API change will be done */
        /* buffers passed into lav* can be allocated any way you prefer,
            as long as they're aligned enough for the architecture, and
            they're freed appropriately (such as using av_free for buffers
            allocated with av_malloc) */
        this->video_outbuf.reset(static_cast<uint8_t*>(av_malloc(video_outbuf_size)));
        if (!this->video_outbuf){
            LOG(LOG_ERR, "video recorder error : failed to allocate video output buffer");
            throw Error(ERR_RECORDER_FAILED_TO_ALLOCATE_PICTURE);
        }
    }

    // init picture frame
    {
        AVPixelFormat const pix_fmt = this->video_st->codec->pix_fmt;
        int const size = av_image_get_buffer_size(pix_fmt, image_view_width, image_view_height, 1);
        if (size) {
            this->picture_buf.reset(static_cast<uint8_t*>(av_malloc(size)));
            std::fill_n(this->picture_buf.get(), size, 0);
        }
        if (!this->picture_buf) {
            LOG(LOG_ERR, "video recorder error : failed to allocate picture buf");
            throw Error(ERR_RECORDER_FAILED_TO_ALLOCATE_PICTURE_BUF);
        }
        av_image_fill_arrays(
            this->picture->data, this->picture->linesize,
            this->picture_buf.get(), pix_fmt, image_view_width, image_view_height, 1
        );

        this->picture->width = image_view_width;
        this->picture->height = image_view_height;
        this->picture->format = this->video_st->codec->codec_id;
        this->original_picture->format = this->video_st->codec->codec_id;
    }

    this->custom_io_buffer.reset(static_cast<unsigned char *>(av_malloc(32768)));
    if (!this->custom_io_buffer) {
        throw Error(ERR_RECORDER_ALLOCATION_FAILED);
    }

    this->custom_io_context.reset(avio_alloc_context(
        this->custom_io_buffer.get(), // buffer
        32768,                        // buffer size
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
        LOG(LOG_ERR, "video recorder error : failed to write header");
    }

    av_image_fill_arrays(
        this->original_picture->data, this->original_picture->linesize,
        image_view.data(), AV_PIX_FMT_BGR24, image_view.width(), image_view.height(), 1
    );

    this->img_convert_ctx.reset(sws_getContext(
        image_view.width(), image_view.height(), AV_PIX_FMT_BGR24,
        image_view_width, image_view_height, STREAM_PIX_FMT,
        SWS_BICUBIC, nullptr, nullptr, nullptr
    ));

    if (!this->img_convert_ctx) {
        LOG(LOG_ERR, "Cannot initialize the conversion context");
        throw Error(ERR_RECORDER_FAILED_TO_INITIALIZE_CONVERSION_CONTEXT);
    }

    av_codec_close_if_fails.ptr = nullptr;

    av_init_packet(&this->pkt);
    this->pkt.data = this->video_outbuf.get();
    this->pkt.size = video_outbuf_size;

    av_log_set_level(log_level);
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

    if (err) {
        const size_t errbuf_size = 1024;
        char errbuf[errbuf_size]{};
        LOG(LOG_ERR, "video recorder: failed to write encoded frame: %s",
            av_make_error_string(errbuf, errbuf_size, err));
        throw Error(ERR_RECORDER_FAILED_TO_WRITE_ENCODED_FRAME);
    }
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
