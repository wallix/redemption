/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_CAPTURE_UTILS_WRM_CAPTURE_IMPL_HPP
#define REDEMPTION_CAPTURE_UTILS_WRM_CAPTURE_IMPL_HPP

#include "RDP/caches/bmpcache.hpp"
#include "RDP/caches/glyphcache.hpp"
#include "RDP/caches/pointercache.hpp"
#include "dump_png24_from_rdp_drawable_adapter.hpp"
#include "transport/out_meta_sequence_transport.hpp"
#include "nativecapture.hpp"
#include "apis_register.hpp"


class WrmCaptureImpl final : private gdi::InputKbdApi, public gdi::CaptureProxy<WrmCaptureImpl>
{
    BmpCache     bmp_cache;
    GlyphCache   gly_cache;
    PointerCache ptr_cache;

    DumpPng24FromRDPDrawableAdapter dump_png24_api;

    struct Transport {
        union Variant
        {
            OutMetaSequenceTransportWithSum out_with_sum;
            CryptoOutMetaSequenceTransport out_crypto;
            OutMetaSequenceTransport out;

            struct {} dummy;
            Variant() : dummy() {}
            ~Variant() {}
        } variant;
        ::Transport * trans;

        template<class... Ts>
        Transport(configs::TraceType trace_type, Ts && ... args)
        {
            TODO("there should only be one outmeta, not two."
                " Capture code should not really care if file is encrypted or not."
                "Here is not the right level to manage anything related to encryption.")
            TODO("Also we may wonder why we are encrypting wrm and not png"
                "(This is related to the path split between png and wrm)."
                "We should stop and consider what we should actually do")
            switch (trace_type) {
                case configs::TraceType::cryptofile:
                    this->trans = new (&this->variant.out_crypto)
                    CryptoOutMetaSequenceTransport(std::forward<Ts>(args)...);
                    break;
                case configs::TraceType::localfile_hashed:
                    this->trans = new (&this->variant.out_with_sum)
                    OutMetaSequenceTransportWithSum(std::forward<Ts>(args)...);
                    break;
                default :
                    this->trans = new (&this->variant.out)
                    OutMetaSequenceTransport(std::forward<Ts>(args)...);
                    break;
            }
        }

        ~Transport() {
            this->trans->~Transport();
        }
    } trans_variant;

    struct Serializer final : GraphicToFile {
        using GraphicToFile::GraphicToFile;

        using GraphicToFile::GraphicToFile::draw;
        using GraphicToFile::GraphicToFile::capture_bpp;

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
    } graphic_to_file;

    struct WrmCapture final : NativeCapture {
        using NativeCapture::NativeCapture;
    } nc;

    std::size_t idx_kbd = -1u;

public:
    WrmCaptureImpl(
        const timeval & now, uint8_t capture_bpp, configs::TraceType trace_type,
        CryptoContext & cctx,
        const char * record_path, const char * hash_path, const char * basename,
        int groupid, auth_api * authentifier,
        RDPDrawable & drawable, const Inifile & ini
    )
    : bmp_cache(
        BmpCache::Recorder, capture_bpp, 3, false,
        BmpCache::CacheOption(600, 768, false),
        BmpCache::CacheOption(300, 3072, false),
        BmpCache::CacheOption(262, 12288, false))
    , ptr_cache(/*pointerCacheSize=*/0x19)
    , dump_png24_api{drawable}
    , trans_variant(
        trace_type, &cctx, record_path, hash_path, basename, now,
        drawable.width(), drawable.height(), groupid, authentifier)
    , graphic_to_file(
        now, *this->trans_variant.trans, drawable.width(), drawable.height(), capture_bpp,
        this->bmp_cache, this->gly_cache, this->ptr_cache,
        this->dump_png24_api, ini, GraphicToFile::SendInput::YES, ini.get<cfg::debug::capture>())
    , nc(this->graphic_to_file, now, ini)
    {}

    void attach_apis(ApisRegister & apis_register, const Inifile & ini) {
        apis_register.graphic_list->push_back(this->graphic_to_file);
        apis_register.capture_list.push_back(*this);
        apis_register.capture_probe_list.push_back(this->graphic_to_file);

        if (!bool(ini.get<cfg::video::disable_keyboard_log>() & configs::KeyboardLogFlags::wrm)) {
            auto & list = apis_register.input_kbd_list;
            this->idx_kbd = list.size();
            list.push_back(this->graphic_to_file);
        }
    }

    void enable_keyboard_input_mask(ApisRegister & apis_register, bool enable) {
        if (this->idx_kbd == -1u) {
            return ;
        }
        auto & kbd = apis_register.input_kbd_list[this->idx_kbd];
        assert(&kbd.get() == this || &kbd.get() == &this->graphic_to_file);
        kbd = enable
            ? static_cast<gdi::InputKbdApi&>(*this)
            : static_cast<gdi::InputKbdApi&>(this->graphic_to_file);
    }

    void send_timestamp_chunk(timeval const & now, bool ignore_time_interval) {
        this->graphic_to_file.timestamp(now);
        this->graphic_to_file.send_timestamp_chunk(ignore_time_interval);
    }

    void request_full_cleaning() {
        this->trans_variant.trans->request_full_cleaning();
    }

private:
    friend gdi::CaptureCoreAccess;
    WrmCapture & get_capture_proxy_impl() {
        return this->nc;
    }

    void resume_capture(const timeval& now) override {
        this->trans_variant.trans->next();
        this->send_timestamp_chunk(now, true);
    }

private:
    bool input_kbd(const timeval& now, const array_const_u8& input_data_32) override {
        static const char shadow_buf[] =
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
            "*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0*\0\0\0"
        ;
        static_assert((sizeof(shadow_buf)-1) % 4 == 0, "");
        return this->graphic_to_file.input_kbd(now, {
            reinterpret_cast<unsigned char const *>(shadow_buf),
            std::min(input_data_32.size(), sizeof(shadow_buf)-1)
        });
    }
};

#endif
