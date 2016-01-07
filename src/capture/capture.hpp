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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#ifndef _REDEMPTION_CAPTURE_CAPTURE_HPP_
#define _REDEMPTION_CAPTURE_CAPTURE_HPP_

#include "out_meta_sequence_transport.hpp"
#include "out_meta_sequence_transport_with_sum.hpp"
#include "crypto_out_meta_sequence_transport.hpp"
#include "out_filename_sequence_transport.hpp"

#include "nativecapture.hpp"
#include "staticcapture.hpp"
#include "new_kbdcapture.hpp"

#include "RDP/compress_and_draw_bitmap_update.hpp"

#include "wait_obj.hpp"

#include "gdi/graphic_api.hpp"
#include "gdi/cache_api.hpp"
#include "gdi/frame_marker_api.hpp"
#include "gdi/server_set_pointer_api.hpp"
#include "gdi/set_mod_palette_api.hpp"
#include "gdi/flush_api.hpp"
#include "gdi/snapshot_api.hpp"
#include "gdi/input_kbd_api.hpp"
#include "gdi/session_update_api.hpp"
#include "gdi/update_config.hpp"
#include "gdi/external_event.hpp"
#include "utils/pattutils.hpp"

class Capture final : public RDPGraphicDevice, public RDPCaptureDevice {
public:
    const bool capture_wrm;
    const bool capture_png;
// for extension
// end extension

    StaticCapture * psc;
    NewKbdCapture * pkc;
// for extension
// end extension

    wait_obj capture_event;

private:
    const configs::TraceType trace_type;
    CryptoContext cctx;

public:
    OutFilenameSequenceTransport * png_trans;
private:
    Transport * wrm_trans;
// for extension
// end extension

    BmpCache      * pnc_bmp_cache;
    GlyphCache    * pnc_gly_cache;
    PointerCache  * pnc_ptr_cache;
    NativeCapture * pnc;

    RDPDrawable * drawable;
    RDPGraphicDevice * gd;

// for extension
// end extension

    timeval last_now;
    int     last_x;
    int     last_y;

    uint8_t order_bpp;
    uint8_t capture_bpp;

    const BGRPalette & mod_palette_rgb = BGRPalette::classic_332_rgb();

    struct NewGraphicDevice : gdi::GraphicApi {
        std::vector<RDPGraphicDevice *> gds;
    };

    struct NewGraphicDeviceProxy {
        template<class... Ts>
        void operator()(NewGraphicDevice & ngd, Ts && ... args) {
            for (RDPGraphicDevice * pgd : ngd) {
                gd->draw(args...);
            }
        }
    };

    template<class Encode, class Decode>
    struct NewGraphicDeviceEncoderProxy : Encode, Decode
    {
        NewGraphicDeviceEncoderProxy(Encode enc, Decode dec, Capture & cap)
        : Encode(enc)
        , Decode(dec)
        , cap(cap)
        {}

        Capture & cap;

        template<class... Ts>
        void operator()(NewGraphicDevice & ngd, Ts && ... args) {
            for (RDPGraphicDevice * pgd : ngd) {
                gd->draw(args...);
            }
        }

        void operator()(NewGraphicDevice & ngd, const RDPBitmapData & bitmap_data, const Bitmap & bmp) {
            if (this->cap.capture_wrm) {
                if (bmp.bpp() > this->cap.capture_bpp) {
                    // reducing the color depth of image.
                    Bitmap capture_bmp(this->cap.capture_bpp, bmp);

                    ::compress_and_draw_bitmap_update(bitmap_data, capture_bmp, this->cap.capture_bpp, *ngd.gd);
                }
                else if (!(bitmap_data.flags & BITMAP_COMPRESSION)) {
                    ::compress_and_draw_bitmap_update(bitmap_data, bmp, this->cap.capture_bpp, *ngd.gd);
                }
                else {
                    REDASSERT(bmp.has_data_compressed());
                    auto data_compressed = bmp.data_compressed();
                    for (RDPGraphicDevice * pgd : ngd) {
                        pgd->draw(bitmap_data, data_compressed.data(), data_compressed.size(), bmp);
                    }
                }
            }
            else if (bmp.has_data_compressed()) {
                auto data_compressed = bmp.data_compressed();
                for (RDPGraphicDevice * pgd : ngd) {
                    pgd->draw(bitmap_data, data_compressed.data(), data_compressed.size(), bmp);
                }
            }
            else {
                for (RDPGraphicDevice * pgd : ngd) {
                    pgd->draw(bitmap_data, bmp.data(), bmp.bmp_size(), bmp);
                }
            }
        }

        BGRColor color(BGRColor c) {
            return Encode::operator()(Decode::operator()(c));
        }

        void operator()(NewGraphicDevice & ngd, const RDPMultiOpaqueRect & cmd, const Rect & clip) {
            RDPMultiOpaqueRect capture_cmd = cmd;
            capture_cmd._Color = this->color(cmd._Color);
            for (RDPGraphicDevice * pgd : ngd) {
                pgd->draw(capture_cmd, clip);
            }
        }

        void operator()(NewGraphicDevice & ngd, const RDP::RDPMultiPatBlt & cmd, const Rect & clip) {
            RDP::RDPMultiPatBlt capture_cmd = cmd;
            capture_cmd.BackColor = this->color(cmd.BackColor);
            capture_cmd.ForeColor = this->color(cmd.ForeColor);
            for (RDPGraphicDevice * pgd : ngd) {
                pgd->draw(capture_cmd, clip);
            }
        }

        void operator()(NewGraphicDevice & ngd, const RDPPatBlt & cmd, const Rect &clip) {
            RDPPatBlt capture_cmd = cmd;
            capture_cmd.back_color = this->color(cmd.back_color);
            capture_cmd.fore_color = this->color(cmd.fore_color);
            for (RDPGraphicDevice * pgd : ngd) {
                pgd->draw(capture_cmd, clip);
            }
        }

        void operator()(NewGraphicDevice & ngd, const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) {
            RDPMem3Blt capture_cmd = cmd;
            capture_cmd.back_color = this->color(cmd.back_color);
            capture_cmd.fore_color = this->color(cmd.fore_color);
            for (RDPGraphicDevice * pgd : ngd) {
                pgd->draw(capture_cmd, clip, bmp);
            }
        }

        void operator()(NewGraphicDevice & ngd, const RDPOpaqueRect & cmd, const Rect & clip) {
            RDPOpaqueRect capture_cmd = cmd;
            capture_cmd.color = this->color(cmd.color);
            for (RDPGraphicDevice * pgd : ngd) {
                pgd->draw(capture_cmd, clip);
            }
        }

        void operator()(NewGraphicDevice & ngd, const RDPLineTo & cmd, const Rect & clip) {
            RDPLineTo capture_cmd = cmd;
            capture_cmd.back_color = this->color(cmd.back_color);
            capture_cmd.pen.color = this->color(cmd.pen.color);
            for (RDPGraphicDevice * pgd : ngd) {
                pgd->draw(capture_cmd, clip);
            }
        }

        void operator()(NewGraphicDevice & ngd, const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache & gly_cache) {
            RDPGlyphIndex capture_cmd = cmd;
            capture_cmd.back_color = this->color(cmd.back_color);
            capture_cmd.fore_color = this->color(cmd.fore_color);
            for (RDPGraphicDevice * pgd : ngd) {
                pgd->draw(capture_cmd, clip, &gly_cache);
            }
        }

        void operator()(NewGraphicDevice & ngd, const RDPPolygonSC & cmd, const Rect & clip) {
            RDPPolygonSC capture_cmd = cmd;
            capture_cmd.BrushColor = this->color(cmd.BrushColor);
            for (RDPGraphicDevice * pgd : ngd) {
                pgd->draw(capture_cmd, clip);
            }
        }

        void operator()(NewGraphicDevice & ngd, const RDPPolygonCB & cmd, const Rect & clip) {
            RDPPolygonCB capture_cmd = cmd;
            capture_cmd.foreColor = this->color(cmd.foreColor);
            capture_cmd.backColor = this->color(cmd.backColor);
            for (RDPGraphicDevice * pgd : ngd) {
                pgd->draw(capture_cmd, clip);
            }
        }

        void operator()(NewGraphicDevice & ngd, const RDPPolyline & cmd, const Rect & clip) {
            RDPPolyline capture_cmd = cmd;
            capture_cmd.PenColor = this->color(cmd.PenColor);
            for (RDPGraphicDevice * pgd : ngd) {
                pgd->draw(capture_cmd, clip);
            }
        }

        void operator()(NewGraphicDevice & ngd, const RDPEllipseSC & cmd, const Rect & clip) {
            RDPEllipseSC capture_cmd = cmd;
            capture_cmd.color = this->color(cmd.color);
            for (RDPGraphicDevice * pgd : ngd) {
                pgd->draw(capture_cmd, clip);
            }
        }

        void operator()(NewGraphicDevice & ngd, const RDPEllipseCB & cmd, const Rect & clip) {
            RDPEllipseCB capture_cmd = cmd;
            capture_cmd.back_color = this->color(cmd.back_color);
            capture_cmd.fore_color = this->color(cmd.fore_color);
            for (RDPGraphicDevice * pgd : ngd) {
                pgd->draw(capture_cmd, clip);
            }
        }
    };

    template<class Enc, class Dec>
    NewGraphicDevice * gd_enc_dec(Enc const & enc, Dec const & dec) {
        using EncoderProxy = NewGraphicDeviceEncoderProxy<Enc, Dec>;
        return new gdi::GraphicDelegate<EncoderProxy, NewGraphicDevice>(enc, dec, *this);
    }

    template<class Enc>
    NewGraphicDevice * gd_enc_dec(Enc const &, Enc const &) {
        return new gdi::GraphicDelegate<NewGraphicDeviceProxy, NewGraphicDevice>();
    }

    NewGraphicDevice * compile_color_enc_dec() {
        switch (this->capture_bpp) {
            case 8 : return this->compile_color_enc_dec(encode_color8{});
            case 15: return this->compile_color_enc_dec(encode_color15{});
            case 16: return this->compile_color_enc_dec(encode_color16{});
            case 24:
            case 32: return this->compile_color_enc_dec(encode_color24{});
            default: return nullptr;
        }
    }

    template<class Encoder>
    NewGraphicDevice * compile_color_enc_dec(Encoder encoder) {
        switch (this->order_bpp) {
            case 8 : return this->gd_enc_dec(encoder, to_color8_palette<decode_color8_opaquerect>{this->mod_palette_rgb});
            case 15: return this->gd_enc_dec(encoder, decode_color15_opaquerect{});
            case 16: return this->gd_enc_dec(encoder, decode_color16_opaquerect{});
            case 24:
            case 32: return this->gd_enc_dec(encoder, decode_color24_opaquerect{});
            default: return nullptr;
        }
    }

    NewGraphicDevice * gd_api = nullptr;

    struct NewCache : gdi::CacheApi {
        NewCache(RDPGraphicDevice * gd) : gd(gd) {}

        void cache(RDPBrushCache const & cmd) override { this->gd->draw(cmd); }
        void cache(RDPColCache   const & cmd) override { this->gd->draw(cmd); }
        void cache(RDPMemBlt     const &    ) override { /* NOTE unimplmented*/ }
        void cache(RDPMem3Blt    const &    ) override { /* NOTE unimplmented*/ }

        RDPGraphicDevice * gd;
    };

    NewCache * cache_api = nullptr;

    struct NewFrameMarker : gdi::FrameMarkerApi {
        NewFrameMarker(Capture & cap) : cap(cap) {}

        void marker(RDP::FrameMarker const & order) override {
            if (this->cap.gd) {
                this->cap.gd->draw(order);
            }

            if (order.action == RDP::FrameMarker::FrameEnd) {
                if (this->cap.capture_png) {
                    bool requested_to_stop = false;
                    this->cap.psc->snapshot(this->cap.last_now, this->cap.last_x, this->cap.last_y, false, requested_to_stop);
                }
            }
        }

        Capture & cap;
    };

    NewFrameMarker * frame_marker_api = nullptr;

    struct NewSetPointer : gdi::ServerSetPointerApi {
        void server_set_pointer(const Pointer& cursor) override {
            for (RDPGraphicDevice * pgd : gds) {
                pgd->server_set_pointer(cursor);
            }
        }

        std::vector<RDPGraphicDevice *> gds;
    };

    NewSetPointer * set_pointer_api = nullptr;

    struct NewSetModePalette : gdi::SetModPaletteApi {
        NewSetModePalette(RDPGraphicDevice * gd) : gd(gd) {}

        void set_mod_palette(const BGRPalette& palette) override {
            for (RDPGraphicDevice * pgd : gds) {
                pgd->set_mod_palette(palette);
            }
        }

        std::vector<RDPGraphicDevice *> gds;
    };

    NewSetModePalette * set_palette_api = nullptr;

    struct NewFlush : gdi::FlushApi {
        NewFlush(RDPGraphicDevice * gd) : gd(gd) {}

        void flush() override {
            for (RDPGraphicDevice * pgd : gds) {
                pgd->flush();
            }
        }

        std::vector<RDPGraphicDevice *> gds;
    };

    NewFlush * flush_api = nullptr;

    struct NewSnapshot : gdi::SnapshotApi {
        NewSnapshot(Capture & cap) : cap(cap) {}

        void snapshot(const timeval & now, int x, int y, bool ignore_frame_in_timeval, wait_obj & capture_event) override {
            capture_event.reset();

            if (cap.capture_drawable) {
                cap.drawable->set_mouse_cursor_pos(x, y);
            }

            cap.last_now = now;
            cap.last_x   = x;
            cap.last_y   = y;

            bool const requested_to_stop = false;

            if (cap.capture_png) {
                cap.psc->snapshot(now, x, y, ignore_frame_in_timeval, requested_to_stop);
                capture_event.update(cap.psc->time_to_wait);
            }
            if (cap.capture_wrm) {
                cap.pnc->snapshot(now, x, y, ignore_frame_in_timeval, requested_to_stop);
                capture_event.update(cap.pnc->time_to_wait);
            }

            if (cap.pkc) {
                cap.pkc->snapshot(now, x, y, ignore_frame_in_timeval, requested_to_stop);
                capture_event.update(cap.pkc->time_to_wait);
            }
        }

        Capture & cap;
    };

    NewSnapshot * snapshot_api = nullptr;

    struct NewInputKbd : gdi::InputKbdApi {
        NewInputKbd(NativeCapture * nc, NewKbdCapture * kc) : nc(nc), kc(kc) {}

        bool input_kbd(const timeval & now, uint8_t const * input_data_32, std::size_t data_sz) override {
            if (this->nc) {
                this->nc->input(now, input_data_32, data_sz);
            }

            if (this->kc) {
                this->kc->input(now, input_data_32, data_sz);
            }

            return true;
        }

        NativeCapture * nc;
        NewKbdCapture * kc;
    };

    NewInputKbd * input_kbd_api = nullptr;

    struct NewSessionUpdate : gdi::SessionUpdateApi {
        void session_update(const timeval& now, const array_view< const char >& message) override {
            bool contian_window_title = false;
            for (RDPCaptureDevice * pcd : this->cds) {
                pcd->session_update(now, message.data(), message.size(), contian_window_title);
            }
        }

        std::vector<RDPCaptureDevice *> cds;
    };

    NewSessionUpdate * session_update_api = nullptr;

    struct NewUpdateConfig : gdi::UpdateConfigApi {
        void update_config(const Inifile & ini) override {
            // TODO
        }

        std::vector<RDPGraphicDevice *> gds;
    };

    NewUpdateConfig * update_config_api = nullptr;

    struct NewExternalEvent : gdi::ExternalEventApi {
        void external_breakpoint() override {
            for (RDPCaptureDevice * pcd : this->cds) {
                pcd->external_breakpoint();
            }
        }

        void external_time(const timeval& now) override {
            for (RDPCaptureDevice * pcd : this->cds) {
                pcd->external_time(now);
            }
        }

        std::vector<RDPCaptureDevice *> cds;
    };

    NewExternalEvent * external_event_api = nullptr;


    const Inifile & ini;
    // TODO: why so many uninitialized constants ?
public:
    Capture( const timeval & now, int width, int height, int order_bpp, int capture_bpp
           , bool clear_png, bool no_timestamp, auth_api * authentifier, const Inifile & ini
           , Random & rnd, bool externally_generated_breakpoint = false)
    : capture_wrm(bool(ini.get<cfg::video::capture_flags>() & configs::CaptureFlags::wrm))
    , capture_png(ini.get<cfg::video::png_limit>() > 0)
    , psc(nullptr)
    , pkc(nullptr)
    , capture_event{}
    , trace_type(ini.get<cfg::globals::trace_type>())
    , cctx(rnd)
    , png_trans(nullptr)
    , wrm_trans(nullptr)
    , pnc_bmp_cache(nullptr)
    , pnc_gly_cache(nullptr)
    , pnc_ptr_cache(nullptr)
    , pnc(nullptr)
    , drawable(nullptr)
    , gd(nullptr)
    , last_now(now)
    , last_x(width / 2)
    , last_y(height / 2)
    , order_bpp(order_bpp)
    , capture_bpp(capture_bpp)
    , ini(ini)
    {
        const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
        const bool capture_drawable = this->capture_wrm || this->capture_png;
        const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>();
        const char * record_path = ini.get<cfg::video::record_path>();
        const char * hash_path = ini.get<cfg::video::hash_path>();

        char path[1024];
        char basename[1024];
        char extension[128];
        strcpy(path, WRM_PATH "/");     // default value, actual one should come from movie_path
        strcpy(basename, "redemption"); // default value actual one should come from movie_path
        strcpy(extension, "");          // extension is currently ignored
        const bool res = canonical_path(ini.get<cfg::globals::movie_path>().c_str(),
                                        path, sizeof(path),
                                        basename, sizeof(basename),
                                        extension, sizeof(extension));
        if (!res) {
            LOG(LOG_ERR, "Buffer Overflowed: Path too long");
            throw Error(ERR_RECORDER_FAILED_TO_FOUND_PATH);
        }


        if (capture_drawable) {
            this->drawable = new RDPDrawable(width, height, capture_bpp);

            this->gd_api = this->compile_color_enc_dec();
            REDASSERT(this->gd_api);
        }

        if (this->capture_png) {
            if (recursive_create_directory(record_tmp_path, S_IRWXU|S_IRWXG,
                        groupid) != 0) {
                LOG(LOG_ERR, "Failed to create directory: \"%s\"", record_tmp_path);
            }
            this->png_trans = new OutFilenameSequenceTransport(
                                FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
                                record_tmp_path, basename, ".png", groupid, authentifier);
            this->psc = new StaticCapture(now, *this->png_trans,
                                          this->png_trans->seqgen(),
                                          width, height, clear_png, ini,
                                          this->drawable->impl());
        }

        if (this->capture_wrm) {
            if (recursive_create_directory(record_path, S_IRWXU | S_IRGRP | S_IXGRP, groupid) != 0) {
                LOG(LOG_ERR, "Failed to create directory: \"%s\"", record_path);
            }

            if (recursive_create_directory(hash_path, S_IRWXU | S_IRGRP | S_IXGRP, groupid) != 0) {
                LOG(LOG_ERR, "Failed to create directory: \"%s\"", hash_path);
            }

            this->cctx.set_crypto_key(ini.get<cfg::crypto::key0>());
            this->cctx.set_hmac_key(ini.get<cfg::crypto::key1>());

            TODO("there should only be one outmeta, not two."
                 " Capture code should not really care if file is encrypted or not."
                 "Here is not the right level to manage anything related to encryption.")
            TODO("Also we may wonder why we are encrypting wrm and not png"
                 "(This is related to the path split between png and wrm)."
                 "We should stop and consider what we should actually do")

            this->pnc_bmp_cache = new BmpCache( BmpCache::Recorder, capture_bpp, 3, false
                                              , BmpCache::CacheOption(600, 768, false)
                                              , BmpCache::CacheOption(300, 3072, false)
                                              , BmpCache::CacheOption(262, 12288, false)
                                              );
            this->pnc_gly_cache = new GlyphCache();
            const int pointerCacheSize = 0x19;
            this->pnc_ptr_cache = new PointerCache(pointerCacheSize);

            if (this->trace_type == configs::TraceType::cryptofile) {
                auto * trans = new CryptoOutMetaSequenceTransport(
                    &this->cctx, record_path, hash_path, basename, now
                  , width, height, groupid, authentifier);
                this->wrm_trans = trans;
            }
            else if (this->trace_type == configs::TraceType::localfile_hashed) {
                auto * trans = new OutMetaSequenceTransportWithSum(
                    &this->cctx, record_path, hash_path, basename,
                    now, width, height, groupid, authentifier);
                this->wrm_trans = trans;
            }
            else {
                auto * trans = new OutMetaSequenceTransport(
                    record_path, hash_path, basename, now,
                    width, height, groupid, authentifier);
                this->wrm_trans = trans;
            }
            this->pnc = new NativeCapture( now, *this->wrm_trans, width, height, capture_bpp
                                         , *this->pnc_bmp_cache, *this->pnc_gly_cache, *this->pnc_ptr_cache
                                         , *this->drawable, ini, externally_generated_breakpoint
                                         , NativeCapture::SendInput::YES);
        }

        if (!bool(ini.get<cfg::video::disable_keyboard_log>() & configs::KeyboardLogFlags::syslog) ||
            ini.get<cfg::session_log::enable_session_log>() ||
            ::contains_kbd_pattern(ini.get<cfg::context::pattern_kill>().c_str()) ||
            ::contains_kbd_pattern(ini.get<cfg::context::pattern_notify>().c_str())) {
            const bool is_kc_driven_by_ocr = false;
            this->pkc = new NewKbdCapture(now, authentifier, ini.get<cfg::context::pattern_kill>().c_str(),
                    ini.get<cfg::context::pattern_notify>().c_str(),
                    !bool(ini.get<cfg::video::disable_keyboard_log>() & configs::KeyboardLogFlags::syslog),
                    is_kc_driven_by_ocr,
                    ini.get<cfg::debug::capture>()
                );
        }

        if (this->capture_wrm) {
            this->gd = this->pnc;
        }
        else if (this->drawable) {
            this->gd = this->drawable;
        }

        if (this->gd_api) {
            this->gd_api->gds.push_back(this->gd);
            this->cache_api = new NewCache(this->gd);
            this->set_pointer_api = new NewSetPointer;
            this->set_pointer_api->gds.push_back(this->gd);
            this->set_palette_api = new NewSetModePalette(this->drawable);
            this->flush_api = new NewFlush;
            this->snapshot_api = new NewSnapshot(*this);
            this->session_update_api = new NewSessionUpdate;
            this->external_event_api = new NewExternalEvent;
            if (this->pnc) {
                this->gd_api->gds.push_back(this->pnc);
                this->set_pointer_api->gds.push_back(this->pnc);
                this->flush_api->gds.push_back(this->pnc);
                this->session_update_api->cds.push_back(this->pnc);
                this->external_event_api->cds.push_back(this->pnc);
            }
        }
        this->frame_marker_api = new NewFrameMarker(*this);
        this->input_kbd_api = new NewInputKbd(
            this->pnc && !bool(ini.get<cfg::video::disable_keyboard_log>() & configs::KeyboardLogFlags::wrm)
                ? this->pnc
                : nullptr,
            this->pkc
        );
    }

    ~Capture() override {
        delete this->pkc;

        delete this->psc;
        delete this->png_trans;

        if (this->pnc) {
            timeval now = tvtime();
            this->pnc->recorder.timestamp(now);
            this->pnc->recorder.send_timestamp_chunk(false);
            delete this->pnc;
        }
        delete this->pnc_bmp_cache;
        delete this->pnc_gly_cache;
        delete this->pnc_ptr_cache;
        delete this->drawable;
        delete this->wrm_trans;

        delete this->gd_api;
        delete this->cache_api;
        delete this->frame_marker_api;
        delete this->set_pointer_api;
        delete this->set_palette_api;
        delete this->flush_api;
        delete this->snapshot_api;
        delete this->input_kbd_api;
    }

    void request_full_cleaning()
    {
        this->wrm_trans->request_full_cleaning();
    }

    void pause() {
        if (this->capture_png) {
            timeval now = tvtime();
            this->psc->pause_snapshot(now);
        }
    }

    void resume() {
        if (this->capture_wrm){
            this->wrm_trans->next();
            timeval now = tvtime();
            this->pnc->recorder.timestamp(now);
            this->pnc->recorder.send_timestamp_chunk(true);
        }
    }

    void update_config(const Inifile & ini) {
        if (this->capture_png) {
            this->psc->update_config(ini);
        }
        if (this->capture_wrm) {
            this->pnc->update_config(ini);
        }
    }

    void set_row(size_t rownum, const uint8_t * data) override {
        if (this->drawable){
            this->drawable->set_row(rownum, data);
        }
    }

    void snapshot(const timeval & now, int x, int y, bool ignore_frame_in_timeval,
                          bool const & requested_to_stop) override {
        this->snapshot_api->snapshot(now, x, y, ignore_frame_in_timeval, this->capture_event);
        this->capture_event.reset();

//         if (this->drawable) {
//             this->drawable->set_mouse_cursor_pos(x, y);
//         }
//
//         this->last_now = now;
//         this->last_x   = x;
//         this->last_y   = y;
//
//         if (this->capture_png) {
//             this->psc->snapshot(now, x, y, ignore_frame_in_timeval, requested_to_stop);
//             this->capture_event.update(this->psc->time_to_wait);
//         }
//         if (this->capture_wrm) {
//             this->pnc->snapshot(now, x, y, ignore_frame_in_timeval, requested_to_stop);
//             this->capture_event.update(this->pnc->time_to_wait);
//         }
//
//         if (this->pkc) {
//             this->pkc->snapshot(now, x, y, ignore_frame_in_timeval, requested_to_stop);
//             this->capture_event.update(this->pkc->time_to_wait);
//         }
    }

    void flush() override {
        if (this->capture_wrm) {
            this->flush_api->flush();
        }
    }

    bool input(const timeval & now, uint8_t const * input_data_32, std::size_t data_sz) override {
        return this->input_kbd_api->input_kbd(now, input_data_32, data_sz);
//         if (this->capture_wrm) {
//             this->pnc->input(now, input_data_32, data_sz);
//         }
//
//         bool send_to_mod = true;
//
//         if (this->pkc) {
//             send_to_mod = this->pkc->input(now, input_data_32, data_sz);
//         }
//
//         return send_to_mod;
    }

    void enable_keyboard_input_mask(bool enable) {
        if (this->capture_wrm) {
            this->pnc->enable_keyboard_input_mask(enable);
        }

        if (this->pkc) {
            this->pkc->enable_keyboard_input_mask(enable);
        }
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPDestBlt & cmd, const Rect &clip) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPMultiDstBlt & cmd, const Rect & clip) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPLineTo & cmd, const Rect & clip) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPBrushCache & cmd) override {
        if (this->gd) {
            this->cache_api->cache(cmd);
        }
    }

    void draw(const RDPColCache & cmd) override {
        if (this->gd) {
            this->cache_api->cache(cmd);
        }
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip, *gly_cache);
        }
    }

    void draw(const RDPBitmapData & bitmap_data, const uint8_t * data , size_t size, const Bitmap & bmp) override {
        if (this->gd) {
            this->gd_api->draw(bitmap_data, bmp);
        }
    }

    void draw(const RDP::FrameMarker & order) override {
        this->frame_marker_api->marker(order);
    }

    void draw(const RDPPolygonSC & cmd, const Rect & clip) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPPolygonCB & cmd, const Rect & clip) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPEllipseSC & cmd, const Rect & clip) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPEllipseCB & cmd, const Rect & clip) override {
        if (this->gd) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        if (this->gd) {
            this->gd->draw(order);
        }
    }

    void draw(const RDP::RAIL::WindowIcon & order) override {
        if (this->gd) {
            this->gd->draw(order);
        }
    }

    void draw(const RDP::RAIL::CachedIcon & order) override {
        if (this->gd) {
            this->gd->draw(order);
        }
    }

    void draw(const RDP::RAIL::DeletedWindow & order) override {
        if (this->gd) {
            this->gd->draw(order);
        }
    }

    void server_set_pointer(const Pointer & cursor) override {
        if (this->gd) {
            this->set_pointer_api->server_set_pointer(cursor);
        }
    }

    void set_mod_palette(const BGRPalette & palette) override {
        if (this->capture_drawable) {
            this->set_palette_api->set_mod_palette(palette);
        }
//         if (this->drawable) {
//             this->drawable->set_mod_palette(palette);
//         }
    }

    void set_pointer_display() override {
        if (this->drawable) {
            this->drawable->show_mouse_cursor(false);
        }
    }

    // toggles externally genareted breakpoint.
    void external_breakpoint() override {
        if (this->capture_wrm) {
            this->pnc->external_breakpoint();
        }
    }

    void external_time(const timeval & now) override {
        if (this->capture_wrm) {
            this->pnc->external_time(now);
        }
    }

    void session_update(const timeval & now, const char * message) override {
        if (this->capture_wrm) {
            this->pnc->session_update(now, message);
        }

        if (this->pkc) {
            this->pkc->session_update(now, message);
        }
    }

    void possible_active_window_change() override {
        if (this->pkc) {
            this->pkc->possible_active_window_change();
        }
    }
};

#endif
