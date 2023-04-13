/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "capture/wrm_capture.hpp"


struct HeadlessWrmCapture
{
    HeadlessWrmCapture(
        unique_fd&& fd,
        CRef<Drawable> drawable,
        MonotonicTimePoint now,
        RealTimePoint real_now,
        BitsPerPixel bits_per_pixel,
        bool remote_program,
        WrmCompressionAlgorithm wrm_compression_algorithm,
        RDPSerializerVerbose verbosity)
    : bmp_cache(
        BmpCache::Recorder, bits_per_pixel, 3, false,
        BmpCache::CacheOption(600, 768, false),
        BmpCache::CacheOption(300, 3072, false),
        BmpCache::CacheOption(262, 12288, false),
        BmpCache::CacheOption(),
        BmpCache::CacheOption(),
        BmpCache::Verbose::none)
    , out(std::move(fd))
    , graphic_to_file(
        now, real_now, this->out, bits_per_pixel, remote_program,
        Rect{0, 0, drawable.get().width(), drawable.get().height()},
        this->bmp_cache, this->gly_cache,
        this->pointer_cache.source_pointers_view(),
        drawable, wrm_compression_algorithm, verbosity)
    {
    }

    gdi::GraphicApi& gd()
    {
        return graphic_to_file;
    }

    void update_timestamp(MonotonicTimePoint now)
    {
        graphic_to_file.timestamp(now);
        graphic_to_file.send_timestamp_chunk();
    }

    void resized()
    {
        graphic_to_file.breakpoint();
    }

private:
    struct OutTransport final : SequencedTransport
    {
        OutTransport(unique_fd&& fd)
        : out(std::move(fd))
        {}

        bool next() override
        {
            return true;
        }

        void flush() override
        {
            out.flush();
        }

        void do_send(const uint8_t * buffer, size_t len) override
        {
            out.send(buffer, len);
        }

    private:
        OutFileTransport out;
    };

    PointerCache pointer_cache{gdi::CachePointerIndex::MAX_POINTER_COUNT};
    BmpCache bmp_cache;
    GlyphCache gly_cache;
    OutTransport out;
    GraphicToFile graphic_to_file;
};
