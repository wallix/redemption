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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Jonathan Poelen,

   Standalone drawing test program for drawable class

*/
#include "log.hpp"
#include <iostream>
#include <boost/program_options.hpp>

#include "version.hpp"

namespace po = boost::program_options;

const char * copyright_notice =
"\n"
"ReDemPtion Drawing Test " VERSION ": standalone tests for drawing module of ReDemPtion.\n"
"Copyright (C) Wallix 2012.\n"
"Christophe Grosjean, Jonathan Poelen\n"
"\n"
;

int main(int argc, char** argv)
{
    setlocale(LC_CTYPE, "C");

    po::options_description desc("Options");
    desc.add_options()
    // -help, --help, -h
    ("help,h", "produce help message")
    // -version, --version, -v
    ("version,v", "show software version")
    ;

    using namespace std;

    po::variables_map options;
    po::store(po::parse_command_line(argc, argv, desc), options);
    po::notify(options);

    if (options.count("help")) {
        cout << copyright_notice;
        cout << "Usage: drawtest [options]\n\n";
        cout << desc << endl;
        _exit(0);
    }
    if (options.count("version")) {
        cout << copyright_notice;
        cout << "Version " VERSION "\n" << endl;
        _exit(0);
    }

    openlog("drawtest", LOG_CONS | LOG_PERROR, LOG_USER);

    LOG(LOG_INFO, "ReDemPtion Drawing Test " VERSION " starting");

    return 0;
}
