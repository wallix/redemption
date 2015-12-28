/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2010-2013
 * Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 * Raphaël Zhou
 *
 * recorder main program
 *
 */

//#define LOGPRINT
#include "version.hpp"

#include "capture.hpp"

#include "utils/apps/app_recorder.hpp"
#include "program_options/program_options.hpp"

namespace po = program_options;

int main(int argc, char** argv)
{
    std::string config_filename = CFG_PATH "/" RDPPROXY_INI;
    Inifile ini;
    { ConfigurationLoader cfg_loader_full(ini, config_filename.c_str()); }

    UdevRandom rnd;
    CryptoContext cctx(rnd);

    TODO("We don't know yet if we need the keys, we should replace that init with some internal code inside CryptoContext")
    memcpy(cctx.crypto_key, ini.get<cfg::crypto::key0>(), sizeof(cctx.crypto_key));
    memcpy(cctx.hmac_key,   ini.get<cfg::crypto::key1>(), sizeof(cctx.hmac_key  ));


    struct CaptureMaker {
        Capture capture;

        CaptureMaker( const timeval & now, uint16_t width, uint16_t height, int order_bpp, int capture_bpp
                    , const char * path, const char * basename, const char * /*extension*/
                    , Inifile & ini, Random & rnd, bool /*clear*/, uint32_t /*verbose*/)
        : capture( now, width, height, order_bpp
                 , capture_bpp
                 , path, path, ini.get<cfg::video::hash_path>(), basename
                 , false, false, nullptr, ini, rnd, true)
        {}
    };
    app_recorder<CaptureMaker>(
        argc, argv
      , "ReDemPtion RECorder " VERSION ": An RDP movie converter.\n"
        "Copyright (C) Wallix 2010-2015.\n"
        "Christophe Grosjean, Jonathan Poelen and Raphael Zhou."
      , [](po::options_description const &){}
      , [](Inifile const & ini, po::variables_map const &, std::string const & output_filename) -> int {
            if (   output_filename.length()
                && !(
                    bool(ini.get<cfg::video::capture_flags>()
                        & (configs::CaptureFlags::png | configs::CaptureFlags::wrm)
                    ) | ini.get<cfg::globals::capture_chunk>()
                )
            ) {
                std::cerr << "Missing target format : need --png or --wrm\n" << std::endl;
                return -1;
            }
            return 0;
      }
      // TODO: now that we have cctx the lambda is useless
      , config_filename, ini, cctx, rnd
      , [](Inifile const &) { return false; }/*has_extra_capture*/
    );
}
