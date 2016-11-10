/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redver video verifier program
*/

#include <string.h>
#include "do_recorder.hpp"
#include "main/version.hpp"
#include "capture/capture.hpp"

#include "utils/apps/app_verifier.hpp"
#include "utils/apps/app_recorder.hpp"
#include "utils/apps/app_decrypter.hpp"
#include "configs/config.hpp"
#include "program_options/program_options.hpp"

namespace po = program_options;

extern "C" {
    __attribute__((__visibility__("default")))
    int recmemcpy(char * dest, char * source, int len)
    {
        ::memcpy(dest, source, static_cast<size_t>(len));
        return 0;
    }

    __attribute__((__visibility__("default")))
    int do_main(int role, int argc, char** argv,
            get_hmac_key_prototype * hmac_fn,
            get_trace_key_prototype * trace_fn)
    {
        Inifile ini;
        ini.set<cfg::debug::config>(false);
        auto config_filename = CFG_PATH "/" RDPPROXY_INI;

        UdevRandom rnd;
        CryptoContext cctx(rnd, ini);
        cctx.set_get_hmac_key_cb(hmac_fn);
        cctx.set_get_trace_key_cb(trace_fn);

        int res = -1;

        switch (role){
        case 0: // RECorder
            try {
                uint32_t    flv_break_interval = 10*60;
                std::string flv_quality;
                bool        full_video = false;
                unsigned    ocr_version = -1u;

                res = app_recorder<Capture>(
                    argc, argv
                    , "ReDemPtion RECorder " VERSION ": An RDP movie converter.\n"
                    "Copyright (C) Wallix 2010-2016.\n"
                    "Christophe Grosjean, Jonathan Poelen and Raphael Zhou.\n"
                    "Compatible with any WRM file format up to 4."
                    , [&](po::options_description & add_option){
                        add_option.add({'t', "ocr", "enable ocr title bar detection"});
                        add_option.add({'c', "chunk", "chunk splitting on title bar change detection"});

                        add_option.add({'a', "flvbreakinterval", &flv_break_interval, "number of seconds between splitting flv files in seconds(default, one flv every 10 minutes)"});

                        add_option.add({'q', "flv-quality", &flv_quality, "flv quality (high, medium, low)"});

                        add_option.add({'f', "flv", "enable flv capture"});

                        add_option.add({'u', "full", "create full video in addition to OCR chunked video"});
                        add_option.add({"ocr-version", &ocr_version, "version 1 or 2"});

                        //add_option.add({'M', "extract-meta-data", "extract session metadata"});
                    }
                    , [&](Inifile & ini, po::variables_map const & options, std::string const & output_filename) -> int {
                        ini.set<cfg::globals::capture_chunk>(options.count("chunk") > 0);

                        ini.set<cfg::video::flv_break_interval>(std::chrono::seconds{flv_break_interval});

                        if (ini.get<cfg::globals::capture_chunk>()) {
                            ini.get_ref<cfg::video::capture_flags>()
                                |= CaptureFlags::flv
                                |  CaptureFlags::ocr
                                |  CaptureFlags::png;

                            ini.set<cfg::video::disable_keyboard_log>(
                                ini.get<cfg::video::disable_keyboard_log>() & ~KeyboardLogFlags::meta);

                            ini.set<cfg::video::flv_break_interval>(std::chrono::minutes{10});
                            ini.set<cfg::video::png_interval>(std::chrono::minutes{1});
                            ini.set<cfg::video::png_limit>(0xFFFF);
                            ini.set<cfg::ocr::interval>(std::chrono::seconds{1});
                        }
                        else {
                            auto set_flag = [&](char const * opt, CaptureFlags f) {
                                if (options.count(opt) > 0) {
                                    ini.get_ref<cfg::video::capture_flags>() |= f;
                                }
                                else {
                                    ini.get_ref<cfg::video::capture_flags>() &= ~f;
                                }
                            };
                            set_flag("flv", CaptureFlags::flv);
                            set_flag("ocr", CaptureFlags::ocr);
                        }

                        if (options.count("ocr-version")) {
                            ini.set<cfg::ocr::version>(ocr_version == 2 ? OcrVersion::v2 : OcrVersion::v1);
                        }

                        //if (options.count("extract-meta-data") &&
                        //    (options.count("png") || options.count("flv") || options.count("wrm") || options.count("chunk"))) {
                        //    std::cerr << "Option --extract-meta-data is not compatible with options --png, --flv, --wrm or --chunk" << std::endl;
                        //    return -1;
                        //}

                        // TODO("extract-meta-data should be independant capture type")
                        ////if (options.count("extract-meta-data") && !options.count("ocr")) {
                        //    std::cerr << "Option --extract-meta-data should be used with option --ocr" << std::endl;
                        //    return -1;
                        //}

                        if (output_filename.length() && !bool(ini.get<cfg::video::capture_flags>())) {
                            std::cerr << "Missing target format : need --png, --ocr, --flv, --wrm or --chunk" << std::endl;
                            return -1;
                        }

                        if (options.count("flv-quality") > 0) {
                                if (0 == strcmp(flv_quality.c_str(), "high")) {
                                ini.set<cfg::globals::video_quality>(Level::high);
                            }
                            else if (0 == strcmp(flv_quality.c_str(), "low")) {
                                ini.set<cfg::globals::video_quality>(Level::low);
                            }
                            else  if (0 == strcmp(flv_quality.c_str(), "medium")) {
                                ini.set<cfg::globals::video_quality>(Level::medium);
                            }
                            else {
                                std::cerr << "Unknown video quality" << std::endl;
                                return -1;
                            }
                        }

                        full_video = (options.count("full") > 0);

                        //extract_meta_data = (options.count("extract-meta-data") > 0);

                        return 0;
                    }
                    // TODO: now that we have cctx the lambda is useless
                    , config_filename
                    , ini
                    , cctx
                    , rnd
                    , [](Inifile const & ini) {
                        return bool(ini.get<cfg::video::capture_flags>()
                            & (CaptureFlags::flv | CaptureFlags::ocr)
                        );
                    } /*has_extra_capture*/
                    , full_video
                );
            } catch (const Error & e) {
                printf("decrypt failed: with id=%d\n", e.id);
            }
        break;
        case 1: // VERIFIER
            ini.set<cfg::debug::config>(false);
            try {
                res = app_verifier(ini,
                    argc, argv
                    , "ReDemPtion VERifier " VERSION ".\n"
                    "Copyright (C) Wallix 2010-2016.\n"
                    "Christophe Grosjean, Raphael Zhou."
                    , cctx);
                puts(res == 0 ? "verify ok\n" : "verify failed\n");
            } catch (const Error & e) {
                printf("verify failed: with id=%d\n", e.id);
            }
        break;
        default:
            try {
                res = app_decrypter(
                    argc, argv
                  , "ReDemPtion DECrypter " VERSION ".\n"
                    "Copyright (C) Wallix 2010-2015.\n"
                    "Christophe Grosjean, Raphael Zhou."
                  , cctx);
                puts(res == 0 ? "decrypt ok\n" : "decrypt failed\n");
            } catch (const Error & e) {
                printf("decrypt failed: with id=%d\n", e.id);
            }
        break;
        }
        return res;
    }
}
