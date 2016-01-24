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

#include "transport/out_meta_sequence_transport.hpp"
#include "transport/out_filename_sequence_transport.hpp"

#include "nativecapture.hpp"
#include "staticcapture.hpp"
#include "new_kbdcapture.hpp"

#include "RDP/compress_and_draw_bitmap_update.hpp"

#include "wait_obj.hpp"

#include "gdi/graphic_api.hpp"
#include "gdi/railgraphic_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/input_kbd_api.hpp"
#include "gdi/capture_probe_api.hpp"

#include "utils/pattutils.hpp"
#include "dump_png24_from_rdp_drawable_adapter.hpp"
#include "gdi/utils/non_null.hpp"
#include "gdi/proxy.hpp"

#include "utils/graphic_capture_impl.hpp"

class Capture final : public RDPGraphicDevice, public RDPCaptureDevice
{
    // for snapshot
    using MouseInfo = MouseTrace;
    
    using Graphic = GraphicCaptureImpl;


    class Static
    {
        OutFilenameSequenceTransport trans;
        StaticCapture sc;

    public:
        Static(
            const timeval & now, bool clear_png, auth_api * authentifier, Graphic & graphic,
            const char * record_tmp_path, const char * basename, int groupid,
            const Inifile & ini)
        : trans(
            FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
            record_tmp_path, basename, ".png", groupid, authentifier)
        , sc(now, this->trans, this->trans.seqgen(), graphic.impl().width(), graphic.impl().height(),
             clear_png, ini, graphic.impl())
        {}

        void attach_apis(Capture & cap, const Inifile &) {
            cap.capture_list().push_back(this->sc);
            cap.graphic_snapshot_list().push_back(this->sc);
        }

        void zoom(unsigned percent) {
            this->sc.zoom(percent);
        }
    };


    class Native final : private gdi::InputKbdApi
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

        NativeCapture nc;

        std::size_t idx_kbd = -1u;

    public:
        Native(
            const timeval & now, uint8_t capture_bpp, configs::TraceType trace_type,
            CryptoContext & cctx,
            const char * record_path, const char * hash_path, const char * basename,
            int groupid, auth_api * authentifier,
            RDPDrawable & drawable, const Inifile & ini, Capture & cap
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
        , nc(this->graphic_to_file, this->dump_png24_api, now, ini)
        {}

        void attach_apis(Capture & cap, const Inifile & ini) {
            cap.graphic_list().push_back({this->graphic_to_file, this->graphic_to_file.capture_bpp});
            cap.rail_graphic_list().push_back(this->graphic_to_file);
            cap.capture_list().push_back(this->nc);
            cap.capture_probe_list().push_back(this->graphic_to_file);

            if (!bool(ini.get<cfg::video::disable_keyboard_log>() & configs::KeyboardLogFlags::wrm)) {
                auto & list = cap.input_kbd_list();
                this->idx_kbd = list.size();
                list.push_back(this->graphic_to_file);
            }
        }

        void enable_keyboard_input_mask(Capture & cap, bool enable) {
            if (this->idx_kbd == -1u) {
                return ;
            }
            auto & kbd = cap.input_kbd_list()[this->idx_kbd];
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

        void next_file() {
            this->trans_variant.trans->next();
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


    class Kbd
    {
        NewKbdCapture kc;

    public:
        Kbd(const timeval & now, auth_api * authentifier, const Inifile & ini)
        : kc(now, authentifier, ini.get<cfg::context::pattern_kill>().c_str(),
            ini.get<cfg::context::pattern_notify>().c_str(),
            !bool(ini.get<cfg::video::disable_keyboard_log>() & configs::KeyboardLogFlags::syslog),
            /*is_kc_driven_by_ocr=*/false,
            ini.get<cfg::debug::capture>())
        {}

        void attach_apis(Capture & cap, const Inifile & ini) {
            cap.capture_probe_list().push_back(this->kc);
            cap.input_kbd_list().push_back(this->kc);
        }

        void enable_keyboard_input_mask(bool enable) {
            this->kc.enable_keyboard_input_mask(enable);
        }
    };

    struct CaptureProxy 
    {
        CaptureProxy(Capture & cap) : cap(cap) {}
        
        template<class Tag, class... Ts>
        void operator()(Tag tag, gdi::CaptureApi &, Ts && ... args) {
            gdi::CaptureProxy prox;
            for (gdi::CaptureApi & cap : this->caps) {
                prox(tag, cap, std::forward<Ts>(args)...);
            }
        }

        std::chrono::microseconds operator()(
            gdi::CaptureProxy::snapshot_tag, gdi::CaptureApi &,
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        ) {
            this->cap.capture_event.reset();

            if (this->cap.gd) {
                this->cap.gd->rdp_drawable().set_mouse_cursor_pos(cursor_x, cursor_y);
            }

            this->cap.mouse_info = {now, cursor_x, cursor_y};

            std::chrono::microseconds time = std::chrono::microseconds::max();
            if (!this->caps.empty()) {
                for (gdi::CaptureApi & cap : this->caps) {
                    time = std::min(time, cap.snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval));
                }
                this->cap.capture_event.update(time.count());
            }
            return time;
        }
        
        std::vector<std::reference_wrapper<gdi::CaptureApi>> caps;

    private:
        Capture & cap;
    };

    using NewCapture = gdi::CaptureAdapter<CaptureProxy>;


    struct NewInputKbd : gdi::InputKbdApi 
    {
        bool input_kbd(const timeval & now, array_view<uint8_t const> const & input_data_32) override {
            bool ret = true;
            for (gdi::InputKbdApi & kpd : this->kbds) {
                ret &= kpd.input_kbd(now, input_data_32);
            }
            return ret;
        }

        std::vector<std::reference_wrapper<gdi::InputKbdApi>> kbds;
    };


    struct NewCaptureProbe : gdi::CaptureProbeApi 
    {
        void possible_active_window_change() override {
            for (gdi::CaptureProbeApi & cap_prob : this->cds) {
                cap_prob.possible_active_window_change();
            }
        }

        void session_update(const timeval& now, array_const_u8 const & message) override {
            for (gdi::CaptureProbeApi & cap_prob : this->cds) {
                cap_prob.session_update(now, message);
            }
        }

        std::vector<std::reference_wrapper<gdi::CaptureProbeApi>> cds;
    };



public:
    const bool capture_wrm;
    const bool capture_png;
// for extension
// end extension

    wait_obj capture_event;

private:
    CryptoContext & cctx;

private:
// for extension
// end extension

    MouseInfo mouse_info;

    uint8_t order_bpp;
    uint8_t capture_bpp;

    std::unique_ptr<Graphic> gd;
    std::unique_ptr<Native> pnc;
    std::unique_ptr<Static> psc;
    std::unique_ptr<Kbd> pkc;

    NewCapture capture_api;
    NewInputKbd input_kbd_api;
    NewCaptureProbe capture_probe_api;
    Graphic::GraphicApi * graphic_api = nullptr;
    Graphic::RAILGraphicApi * rail_graphic_api = nullptr;

    const Inifile & ini;

    
    std::vector<Graphic::GdRef> & graphic_list() {
        assert(this->graphic_api);
        return this->graphic_api->get_proxy().gds;
    }

    std::vector<std::reference_wrapper<gdi::RAILGraphicApi>> & rail_graphic_list() {
        assert(this->rail_graphic_api);
        return this->rail_graphic_api->get_proxy().apis;
    }

    std::vector<std::reference_wrapper<gdi::CaptureApi>> & graphic_snapshot_list() {
        assert(this->graphic_api);
        return this->graphic_api->get_proxy().snapshoters;
    }

    std::vector<std::reference_wrapper<gdi::CaptureApi>> & capture_list() {
        return this->capture_api.get_proxy().caps;
    }

    std::vector<std::reference_wrapper<gdi::InputKbdApi>> & input_kbd_list() {
        return this->input_kbd_api.kbds;
    }

    std::vector<std::reference_wrapper<gdi::CaptureProbeApi>> & capture_probe_list() {
        return this->capture_probe_api.cds;
    }

public:
    Capture(
        const timeval & now,
        int width, int height, int order_bpp, int capture_bpp,
        bool clear_png, bool no_timestamp, auth_api * authentifier,
        const Inifile & ini, Random & rnd, CryptoContext & cctx,
        bool full_video, bool extract_meta_data)
    : capture_wrm(bool(ini.get<cfg::video::capture_flags>() & configs::CaptureFlags::wrm))
    , capture_png(ini.get<cfg::video::png_limit>() > 0)
    , capture_event{}
    , cctx(cctx)
    , mouse_info{now, width / 2, height / 2}
    , order_bpp(order_bpp)
    , capture_bpp(capture_bpp)
    // TODO
    , capture_api(*this)
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
            this->gd.reset(new Graphic(width, height, order_bpp, this->mouse_info));
            this->graphic_api = &this->gd->get_graphic_api();
            this->rail_graphic_api = &this->gd->get_rail_graphic_api();

            if (this->capture_png) {
                if (recursive_create_directory(record_tmp_path, S_IRWXU|S_IRWXG, groupid) != 0) {
                    LOG(LOG_ERR, "Failed to create directory: \"%s\"", record_tmp_path);
                }

                this->psc.reset(new Static(
                    now, clear_png, authentifier, *this->gd,
                    record_tmp_path, basename, groupid, ini
                ));
            }

            if (this->capture_wrm) {
                if (recursive_create_directory(record_path, S_IRWXU | S_IRGRP | S_IXGRP, groupid) != 0) {
                    LOG(LOG_ERR, "Failed to create directory: \"%s\"", record_path);
                }

                if (recursive_create_directory(hash_path, S_IRWXU | S_IRGRP | S_IXGRP, groupid) != 0) {
                    LOG(LOG_ERR, "Failed to create directory: \"%s\"", hash_path);
                }

                this->pnc.reset(new Native(
                    now, capture_bpp, ini.get<cfg::globals::trace_type>(),
                    this->cctx, record_path, hash_path, basename,
                    groupid, authentifier, this->gd->rdp_drawable(), ini, *this
                ));
            }
        }

        if (!bool(ini.get<cfg::video::disable_keyboard_log>() & configs::KeyboardLogFlags::syslog) ||
            ini.get<cfg::session_log::enable_session_log>() ||
            ::contains_kbd_pattern(ini.get<cfg::context::pattern_kill>().c_str()) ||
            ::contains_kbd_pattern(ini.get<cfg::context::pattern_notify>().c_str())) {
            this->pkc.reset(new Kbd(now, authentifier, ini));
        }

        //TODO if (this->gd ) { this->gd->attach_apis (*this, ini); }
        if (this->pnc) { this->pnc->attach_apis(*this, ini); }
        if (this->psc) { this->psc->attach_apis(*this, ini); }
        if (this->pkc) { this->pkc->attach_apis(*this, ini); }

        if (this->gd ) { this->gd->start(order_bpp); }
    }

    ~Capture() override {
        this->pkc.reset();

        this->psc.reset();

        if (this->pnc) {
            timeval now = tvtime();
            this->pnc->send_timestamp_chunk(now, false);
            this->pnc.reset();
        }

        this->gd.reset();
    }

    void request_full_cleaning()
    {
        if (this->pnc) {
            this->pnc->request_full_cleaning();
        }
    }

    void pause() {
        if (this->capture_png) {
            timeval now = tvtime();
            this->capture_api.pause_capture(now);
        }
    }

    void resume() {
        if (this->capture_wrm){
            this->pnc->next_file();
            timeval now = tvtime();
            this->pnc->send_timestamp_chunk(now, true);

            this->capture_api.resume_capture(now);
        }
    }

    void update_config(const Inifile & ini) {
        this->capture_api.update_config(ini);
    }

    void set_row(size_t rownum, const uint8_t * data) override {
        if (this->gd){
            this->gd->rdp_drawable().set_row(rownum, data);
        }
    }

    void snapshot(const timeval & now, int x, int y, bool ignore_frame_in_timeval,
    // TODO
                          bool const & requested_to_stop) override {
        this->capture_api.snapshot(now, x, y, ignore_frame_in_timeval);
    }

    void flush() override {
        if (this->graphic_api) {
            this->graphic_api->sync();
        }
    }

    bool input(const timeval & now, uint8_t const * input_data_32, std::size_t data_sz) override {
        return this->input_kbd_api.input_kbd(now, {input_data_32, data_sz});
    }

    // TODO is not virtual
    void enable_keyboard_input_mask(bool enable) {
        if (this->pnc) {
            this->pnc->enable_keyboard_input_mask(*this, enable);
        }

        if (this->pkc) {
            this->pkc->enable_keyboard_input_mask(enable);
        }
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPDestBlt & cmd, const Rect &clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPMultiDstBlt & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPLineTo & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPBrushCache & cmd) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd);
        }
    }

    void draw(const RDPColCache & cmd) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd);
        }
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip, *gly_cache);
        }
    }

    void draw(const RDPBitmapData & bitmap_data, const uint8_t * data , size_t size, const Bitmap & bmp) override {
        if (this->graphic_api) {
            this->graphic_api->draw(bitmap_data, bmp);
        }
    }

    void draw(const RDP::FrameMarker & cmd) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd);
        }
    }

    void draw(const RDPPolygonSC & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPPolygonCB & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPEllipseSC & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPEllipseCB & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        if (this->rail_graphic_api) {
            this->rail_graphic_api->draw(order);
        }
    }

    void draw(const RDP::RAIL::WindowIcon & order) override {
        if (this->rail_graphic_api) {
            this->rail_graphic_api->draw(order);
        }
    }

    void draw(const RDP::RAIL::CachedIcon & order) override {
        if (this->rail_graphic_api) {
            this->rail_graphic_api->draw(order);
        }
    }

    void draw(const RDP::RAIL::DeletedWindow & order) override {
        if (this->rail_graphic_api) {
            this->rail_graphic_api->draw(order);
        }
    }

    void server_set_pointer(const Pointer & cursor) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cursor);
        }
    }

    void set_mod_palette(const BGRPalette & palette) override {
        if (this->graphic_api) {
            this->graphic_api->draw(palette);
        }
    }

    void set_pointer_display() override {
        if (this->gd) {
            this->gd->rdp_drawable().show_mouse_cursor(false);
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
        this->capture_probe_api.session_update(now, {
            reinterpret_cast<unsigned char const *>(message), strlen(message)
        });
    }

    void possible_active_window_change() override {
        this->capture_probe_api.possible_active_window_change();
    }

    // TODO move to ctor
    void zoom(unsigned percent) {
        assert(this->pnc);
        this->psc->zoom(percent);
    }
};

#endif
