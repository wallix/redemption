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
#include "gdi/capture_api.hpp"
#include "gdi/synchronise_api.hpp"
#include "gdi/input_kbd_api.hpp"
#include "gdi/capture_probe_api.hpp"
#include "gdi/synchronise_api.hpp"


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
    CryptoContext & cctx;

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
        NewGraphicDevice(Capture & cap) : cap(cap) {}
        std::vector<RDPGraphicDevice *> gds;
        std::vector<gdi::CaptureApi*> snapshoters;
        Capture & cap;
    };

    struct NewGraphicDeviceProxy {
        template<class... Ts>
        void operator()(NewGraphicDevice & ngd, Ts const & ... args) {
            this->dispatch(ngd, args...);
        }

        void operator()(NewGraphicDevice & ngd, RDP::FrameMarker const & cmd) {
            this->dispatch(ngd, cmd);

            if (cmd.action == RDP::FrameMarker::FrameEnd) {
                for (gdi::CaptureApi * cap : ngd.snapshoters) {
                    cap->snapshot(ngd.cap.last_now, ngd.cap.last_x, ngd.cap.last_y, false);
                }
            }
        }

        template<class... Ts>
        void dispatch(NewGraphicDevice & ngd, Ts const & ... args) {
            for (RDPGraphicDevice * pgd : ngd.gds) {
                this->adapt(pgd, args...);
            }
        }

        template<class... Ts>
        void adapt(RDPGraphicDevice * pgd, Ts const & ... args) {
            pgd->draw(args...);
        }

        void adapt(RDPGraphicDevice * pgd, const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache & gly_cache) {
            pgd->draw(cmd, clip, &gly_cache);
        }

        void adapt(RDPGraphicDevice * pgd, BGRPalette const & palette) {
            pgd->set_mod_palette(palette);
        }

        void adapt(RDPGraphicDevice * pgd, Pointer const & pointer) {
            pgd->server_set_pointer(pointer);
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

        // TODO
        Capture & cap;

        template<class... Ts>
        void operator()(NewGraphicDevice & ngd, Ts && ... args) {
            this->encode(1, ngd, std::forward<Ts>(args)...);
        }

        // TODO
        void operator()(NewGraphicDevice & ngd, const RDPBitmapData & bitmap_data, const Bitmap & bmp) {
            if (this->cap.capture_wrm) {
                if (bmp.bpp() > this->cap.capture_bpp) {
                    // reducing the color depth of image.
                    Bitmap capture_bmp(this->cap.capture_bpp, bmp);

                    for (RDPGraphicDevice * pgd : ngd.gds) {
                        ::compress_and_draw_bitmap_update(bitmap_data, capture_bmp, this->cap.capture_bpp, *pgd);
                    }
                }
                else if (!(bitmap_data.flags & BITMAP_COMPRESSION)) {
                    for (RDPGraphicDevice * pgd : ngd.gds) {
                        ::compress_and_draw_bitmap_update(bitmap_data, bmp, this->cap.capture_bpp, *pgd);
                    }
                }
                else {
                    REDASSERT(bmp.has_data_compressed());
                    for (RDPGraphicDevice * pgd : ngd.gds) {
                        ::compress_and_draw_bitmap_update(bitmap_data, bmp, this->cap.capture_bpp, *pgd);
                    }
                }
            }
            else if (bmp.has_data_compressed()) {
                auto data_compressed = bmp.data_compressed();
                for (RDPGraphicDevice * pgd : ngd.gds) {
                    pgd->draw(bitmap_data, data_compressed.data(), data_compressed.size(), bmp);
                }
            }
            else {
                for (RDPGraphicDevice * pgd : ngd.gds) {
                    pgd->draw(bitmap_data, bmp.data(), bmp.bmp_size(), bmp);
                }
            }
        }

        BGRColor encode_color(BGRColor c) {
            return Encode::operator()(Decode::operator()(c));
        }

        void encode_cmd_color(RDPMultiOpaqueRect & cmd) {
            cmd._Color = this->encode_color(cmd._Color);
        }

        void encode_cmd_color(RDP::RDPMultiPatBlt & cmd) {
            cmd.BackColor = this->encode_color(cmd.BackColor);
            cmd.ForeColor = this->encode_color(cmd.ForeColor);
        }

        void encode_cmd_color(RDPPatBlt & cmd) {
            cmd.back_color = this->encode_color(cmd.back_color);
            cmd.fore_color = this->encode_color(cmd.fore_color);
        }

        void encode_cmd_color(RDPMem3Blt & cmd) {
            cmd.back_color = this->encode_color(cmd.back_color);
            cmd.fore_color = this->encode_color(cmd.fore_color);
        }

        void encode_cmd_color(RDPOpaqueRect & cmd) {
            cmd.color = this->encode_color(cmd.color);
        }

        void encode_cmd_color(RDPLineTo & cmd) {
            cmd.back_color = this->encode_color(cmd.back_color);
            cmd.pen.color = this->encode_color(cmd.pen.color);
        }

        void encode_cmd_color(RDPGlyphIndex & cmd) {
            cmd.back_color = this->encode_color(cmd.back_color);
            cmd.fore_color = this->encode_color(cmd.fore_color);
        }

        void encode_cmd_color(RDPPolygonSC & cmd) {
            cmd.BrushColor = this->encode_color(cmd.BrushColor);
        }

        void encode_cmd_color(RDPPolygonCB & cmd) {
            cmd.foreColor = this->encode_color(cmd.foreColor);
            cmd.backColor = this->encode_color(cmd.backColor);
        }

        void encode_cmd_color(RDPPolyline & cmd) {
            cmd.PenColor = this->encode_color(cmd.PenColor);
        }

        void encode_cmd_color(RDPEllipseSC & cmd) {
            cmd.color = this->encode_color(cmd.color);
        }

        void encode_cmd_color(RDPEllipseCB & cmd) {
            cmd.back_color = this->encode_color(cmd.back_color);
            cmd.fore_color = this->encode_color(cmd.fore_color);
        }

        template<class Cmd, class... Ts>
        auto encode(int, NewGraphicDevice & ngd, Cmd const & cmd, Ts && ... args)
        -> decltype(this->encode_cmd_color(std::declval<Cmd&>())) {
            auto capture_cmd = cmd;
            this->encode_cmd_color(capture_cmd);
            NewGraphicDeviceProxy()(ngd, capture_cmd, std::forward<Ts>(args)...);
        }

        template<class Cmd, class... Ts>
        void encode(unsigned, NewGraphicDevice & ngd, Cmd const & cmd, Ts && ... args) {
            NewGraphicDeviceProxy()(ngd, cmd, std::forward<Ts>(args)...);
        }
    };

    template<class Enc, class Dec>
    NewGraphicDevice * gd_enc_dec(Enc const & enc, Dec const & dec) {
        using EncoderProxy = NewGraphicDeviceEncoderProxy<Enc, Dec>;
        return new gdi::GraphicAdaptor<EncoderProxy, NewGraphicDevice>(
            EncoderProxy{enc, dec, *this}, *this
        );
    }

    template<class Enc>
    NewGraphicDevice * gd_enc_dec(Enc const &, Enc const &) {
        return new gdi::GraphicAdaptor<NewGraphicDeviceProxy, NewGraphicDevice>(
            NewGraphicDeviceProxy{}, *this
        );
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

    struct NewCaptureBase : gdi::CaptureApi {
        std::vector<gdi::CaptureApi *> caps;
        StaticCapture * psc;
        NativeCapture * pnc;
        Capture & cap;

        NewCaptureBase(Capture & cap) : cap(cap) {}
    };

    struct NewCaptureProxy {
        template<class Tag, class... Ts>
        void operator()(Tag tag, NewCaptureBase & api, Ts && ... args) {
            gdi::CaptureProxy prox;
            for (gdi::CaptureApi * pcap : api.caps) {
                prox(tag, *pcap, std::forward<Ts>(args)...);
            }
        }

        std::chrono::microseconds operator()(
            gdi::CaptureProxy::snapshot_tag, NewCaptureBase & api,
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        ) {
            api.cap.capture_event.reset();

            if (api.cap.drawable) {
                api.cap.drawable->set_mouse_cursor_pos(cursor_x, cursor_y);
            }

            api.cap.last_now = now;
            api.cap.last_x   = cursor_x;
            api.cap.last_y   = cursor_y;

            std::chrono::microseconds time = std::chrono::microseconds::max();
            if (!api.caps.empty()) {
                for (gdi::CaptureApi * pcap : api.caps) {
                    time = std::min(time, pcap->snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval));
                }
                api.cap.capture_event.update(time.count());
            }
            return time;
        }

        void operator()(gdi::CaptureProxy::update_config_tag, NewCaptureBase & api, Inifile const & ini) {
            //TODO enable_real_time
            if (api.psc) {
                api.psc->update_config(ini);
            }
            if (api.pnc) {
                api.pnc->update_config(ini);
            }
        }
    };

    using NewCapture = gdi::CaptureAdaptor<NewCaptureProxy, NewCaptureBase>;

    NewCapture capture_api;

    struct NewCacheDevice : gdi::CacheApi {
        std::vector<RDPGraphicDevice *> gds;
    };

    struct NewCacheDeviceProxy {
        template<class... Ts>
        void operator()(NewCacheDevice & ngd, Ts const & ... args) {
            for (RDPGraphicDevice * gd : ngd.gds) {
                gd->draw(args...);
            }
        }
    };

    using NewCache = gdi::CacheAdaptor<NewCacheDeviceProxy, NewCacheDevice>;

    NewCache * cache_api = nullptr;

    struct NewSynchronise : gdi::SynchroniseApi {
        void sync() override {
            for (gdi::SynchroniseApi * pgd : gds) {
                pgd->sync();
            }
        }

        std::vector<gdi::SynchroniseApi *> gds;
    };

    NewSynchronise synchronise_api;

    struct NewInputKbd : gdi::InputKbdApi {
        bool input_kbd(const timeval & now, array_view<uint8_t const> const & input_data_32) override {
            bool ret = true;
            for (gdi::InputKbdApi * pkpd : this->kbds) {
                ret &= pkpd->input_kbd(now, input_data_32);
            }
            return ret;
        }

        std::vector<gdi::InputKbdApi *> kbds;
    };

    NewInputKbd input_kbd_api;

    struct NewCaptureProbe : gdi::CaptureProbeApi {
        void possible_active_window_change() override {
            for (RDPCaptureDevice * pcd : this->cds) {
                pcd->possible_active_window_change();
            }
        }

        void session_update(const timeval& now, const array_view< const char >& message) override {
            for (RDPCaptureDevice * pcd : this->cds) {
                pcd->session_update(now, message.data()/* TODO , message.size()*/);
            }
        }

        std::vector<RDPCaptureDevice *> cds;
    };

    NewCaptureProbe capture_probe_api;


    const Inifile & ini;
    // TODO: why so many uninitialized constants ?
public:
    Capture(
        const timeval & now,
        int width, int height, int order_bpp, int capture_bpp,
        bool clear_png, bool no_timestamp, auth_api * authentifier,
        const Inifile & ini, Random & rnd, CryptoContext & cctx,
        bool full_video, bool extract_meta_data)
    : capture_wrm(bool(ini.get<cfg::video::capture_flags>() & configs::CaptureFlags::wrm))
    , capture_png(ini.get<cfg::video::png_limit>() > 0)
    , psc(nullptr)
    , pkc(nullptr)
    , capture_event{}
    , trace_type(ini.get<cfg::globals::trace_type>())
    , cctx(cctx)
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
    // TODO
    , capture_api(NewCaptureProxy{}, *this)
    , ini(ini)
    {
        TODO("Remove that after change of capture interface")
        (void)full_video;
        TODO("Remove that after change of capture interface")
        (void)extract_meta_data;
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
                    &this->cctx,
                    record_path, hash_path, basename, now,
                    width, height, groupid, authentifier);
                this->wrm_trans = trans;
            }
            this->pnc = new NativeCapture( now, *this->wrm_trans, width, height, capture_bpp
                                         , *this->pnc_bmp_cache, *this->pnc_gly_cache, *this->pnc_ptr_cache
                                         , *this->drawable, ini
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

        capture_api.pnc = this->pnc;
        capture_api.psc = this->psc;

        if (this->gd_api) {
            this->cache_api = new NewCache;
            if (this->pnc) {
                this->gd_api->gds.push_back(&this->pnc->recorder);
                this->synchronise_api.gds.push_back(this->pnc);
                this->capture_api.caps.push_back(this->pnc);
                this->capture_probe_api.cds.push_back(this->pnc);
                this->cache_api->gds.push_back(&this->pnc->recorder);
            }
            if (this->psc) {
                this->capture_api.caps.push_back(this->psc);
                this->gd_api->snapshoters.push_back(this->psc);
            }
            if (this->drawable) {
                this->gd_api->gds.push_back(this->drawable);
            }
        }
        if (this->pkc) {
            this->capture_probe_api.cds.push_back(this->pkc);
            this->input_kbd_api.kbds.push_back(this->pkc);
        }

        if (this->pnc && !bool(ini.get<cfg::video::disable_keyboard_log>() & configs::KeyboardLogFlags::wrm)) {
            this->input_kbd_api.kbds.push_back(this->pnc);
        }
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
    }

    void request_full_cleaning()
    {
        this->wrm_trans->request_full_cleaning();
    }

    void pause() {
        if (this->capture_png) {
            timeval now = tvtime();
            this->capture_api.pause_capture(now);
        }
    }

    void resume() {
        if (this->capture_wrm){
            this->wrm_trans->next();
            timeval now = tvtime();
            this->pnc->recorder.timestamp(now);
            this->pnc->recorder.send_timestamp_chunk(true);

            this->capture_api.resume_capture(now);
        }
    }

    void update_config(const Inifile & ini) {
        this->capture_api.update_config(ini);
    }

    void set_row(size_t rownum, const uint8_t * data) override {
        if (this->drawable){
            this->drawable->set_row(rownum, data);
        }
    }

    void snapshot(const timeval & now, int x, int y, bool ignore_frame_in_timeval,
    // TODO
                          bool const & requested_to_stop) override {
        this->capture_api.snapshot(now, x, y, ignore_frame_in_timeval);
    }

    void flush() override {
        if (this->capture_wrm) {
            this->synchronise_api.sync();
        }
    }

    bool input(const timeval & now, uint8_t const * input_data_32, std::size_t data_sz) override {
        return this->input_kbd_api.input_kbd(now, {input_data_32, data_sz});
    }

    // TODO is not virtual
    void enable_keyboard_input_mask(bool enable) {
        if (this->capture_wrm) {
            this->pnc->enable_keyboard_input_mask(enable);
        }

        if (this->pkc) {
            this->pkc->enable_keyboard_input_mask(enable);
        }
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPDestBlt & cmd, const Rect &clip) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPMultiDstBlt & cmd, const Rect & clip) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPLineTo & cmd, const Rect & clip) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPBrushCache & cmd) override {
        if (this->cache_api) {
            this->cache_api->cache(cmd);
        }
    }

    void draw(const RDPColCache & cmd) override {
        if (this->cache_api) {
            this->cache_api->cache(cmd);
        }
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip, *gly_cache);
        }
    }

    void draw(const RDPBitmapData & bitmap_data, const uint8_t * data , size_t size, const Bitmap & bmp) override {
        if (this->gd_api) {
            this->gd_api->draw(bitmap_data, bmp);
        }
    }

    void draw(const RDP::FrameMarker & cmd) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd);
        }
    }

    void draw(const RDPPolygonSC & cmd, const Rect & clip) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPPolygonCB & cmd, const Rect & clip) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPEllipseSC & cmd, const Rect & clip) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDPEllipseCB & cmd, const Rect & clip) override {
        if (this->gd_api) {
            this->gd_api->draw(cmd, clip);
        }
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        if (this->gd_api) {
            this->gd->draw(order);
        }
    }

    void draw(const RDP::RAIL::WindowIcon & order) override {
        if (this->gd_api) {
            this->gd->draw(order);
        }
    }

    void draw(const RDP::RAIL::CachedIcon & order) override {
        if (this->gd_api) {
            this->gd->draw(order);
        }
    }

    void draw(const RDP::RAIL::DeletedWindow & order) override {
        if (this->gd_api) {
            this->gd->draw(order);
        }
    }

    void server_set_pointer(const Pointer & cursor) override {
        if (this->gd_api) {
            this->gd_api->draw(cursor);
        }
    }

    void set_mod_palette(const BGRPalette & palette) override {
        if (this->drawable) {
            this->gd_api->draw(palette);
        }
    }

    void set_pointer_display() override {
        if (this->drawable) {
            this->drawable->show_mouse_cursor(false);
        }
    }

    // toggles externally genareted breakpoint.
    void external_breakpoint() override {
        this->capture_api.external_breakpoint();
    }

    void external_time(const timeval & now) override {
        this->capture_api.external_time(now);
    }

    void session_update(const timeval & now, const char * message) override {
        this->capture_probe_api.session_update(now, {message, strlen(message)});
    }

    void possible_active_window_change() override {
        this->capture_probe_api.possible_active_window_change();
    }
};

#endif
