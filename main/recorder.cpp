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

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

//#define LOGPRINT
#include "version.hpp"

#include "apps/app_recorder.hpp"

int main(int argc, char** argv)
{
    struct CaptureMaker {
        Capture capture;

        CaptureMaker( const timeval & now, uint16_t width, uint16_t height, int order_bpp
                    , const char * path, const char * basename, Inifile & ini, bool /*clear*/)
        : capture( now, width, height, order_bpp
                 , ini.video.wrm_color_depth_selection_strategy
                 , path, path, ini.video.hash_path, basename
                 , false, false, NULL, ini, true)
        {}
    };
    app_recorder<CaptureMaker>(
        argc, argv
      , "ReDemPtion RECorder " VERSION ": An RDP movie converter.\n"
        "Copyright (C) Wallix 2010-2014.\n"
        "Christophe Grosjean, Jonathan Poelen and Raphael Zhou."
      , [](boost::program_options::options_description_easy_init const &){}
      , [](Inifile const &, boost::program_options::variables_map const &, std::string const & /*output_filename*/) { return 0; }
      , [](Inifile::Inifile_crypto const &) { return 0; }
      , [](Inifile const &) { return false; }/*has_extra_capture*/
    );
}
