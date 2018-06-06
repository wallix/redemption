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

#pragma once

#include <cstdint>

#ifndef REDEMPTION_NO_FFMPEG

#include <memory>

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

extern "C" {
    // On Debian lenny and on higher debian/ubuntu distribution, ffmpeg includes
    // aren't localized on the same path (usr/include/ffmpeg for lenny,
    // /usr/include/libXXX for ubuntu/debian testing/unstable)
    #ifndef UINT64_C
    #define UINT64_C uint64_t
    #endif

    #include <libavcodec/avcodec.h> // AVPacket
}

#ifdef exit
# undef exit
#endif

class AVFormatContext;
class AVIOContext;
class AVStream;
class SwsContext;

#endif

class ConstImageDataView;

class video_recorder
{

#ifndef REDEMPTION_NO_FFMPEG

    struct default_av_free { void operator()(void * ptr); };
    struct default_av_free_format_context { void operator()(AVFormatContext * ctx); };
    struct default_sws_free_context { void operator()(SwsContext * sws_ctx); };

    class AVFramePtr
    {
        AVFrame * frame;

    public:
        AVFramePtr();
        ~AVFramePtr();

        AVFrame * operator->() { return this->frame; }
        AVFrame * get() { return this->frame; }
    };


    const int original_height;
    uint64_t old_frame_index = 0;

    /* video output */

    std::unique_ptr<uint8_t, default_av_free> picture_buf;
    AVStream * video_st = nullptr;

    AVFramePtr picture;
    AVFramePtr original_picture;
    std::unique_ptr<uint8_t, default_av_free> video_outbuf;

    std::unique_ptr<AVFormatContext, default_av_free_format_context> oc;
    std::unique_ptr<SwsContext, default_sws_free_context> img_convert_ctx;

    /* custom IO */
    std::unique_ptr<unsigned char, default_av_free> custom_io_buffer;
    std::unique_ptr<AVIOContext, default_av_free> custom_io_context;

    AVPacket pkt;

#endif

    //static const unsigned frame_key_limit = 100;
    //unsigned frame_key = frame_key_limit;

public:
    using write_packet_fn_t = int(*)(void *io_params, uint8_t *buf, int buf_size);
    using seek_fn_t = int64_t(*)(void *io_params, int64_t offset, int whence);

    video_recorder(
        write_packet_fn_t write_packet_fn, seek_fn_t seek_fn, void * io_params,
        ConstImageDataView const & image_view, int bitrate,
        int frame_rate, int qscale, const char * codec_id,
        int log_level
    );

    ~video_recorder();

    void preparing_video_frame();

    void encoding_video_frame(uint64_t frame_index = 1);
};
