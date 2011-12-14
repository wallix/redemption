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

   Object used to save rsa_keys.ini information

*/



#include "rsa_keys.hpp"
#include "log.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <string.h>


using namespace std;
namespace po = boost::program_options;

#include <stdio.h>


Rsakeys::Rsakeys(const char * filename) {
    ifstream  Rsakeys_stream(filename);
    this->init(Rsakeys_stream);
}


Rsakeys::Rsakeys(istream & Rsakeys_stream){
    this->init(Rsakeys_stream);
}


void Rsakeys::init(istream & Rsakeys_stream){
    po::options_description Rsakeys_desc("RSA keys file.ini configuration file:");
    Rsakeys_desc.add_options()
    ("keys.pub_exp", po::value<string>()->default_value(""), "")
    ("keys.pub_mod", po::value<string>()->default_value(""), "")
    ("keys.pub_sig", po::value<string>()->default_value(""), "")
    ("keys.pri_exp", po::value<string>()->default_value(""), "")
    ;
    try{
        po::variables_map vm;
        po::store(po::parse_config_file(Rsakeys_stream, Rsakeys_desc), vm);
        po::notify(vm);

        string pub_exp = vm["keys.pub_exp"].as<string>();
        string pub_mod = vm["keys.pub_mod"].as<string>();
        string pub_sig = vm["keys.pub_sig"].as<string>();
        string pri_mod = vm["keys.pri_exp"].as<string>();

        char exp[24];
        char mod[512];
        char sig[512];
        char pri[512];


        strncpy(exp, pub_exp.data(), 24);
        strncpy(mod, pub_mod.data(), 512);
        strncpy(sig, pub_sig.data(), 512);
        strncpy(pri, pri_mod.data(), 512);

        this->hex_str_to_bin(exp, this->pub_exp, 4);
        this->hex_str_to_bin(mod, this->pub_mod, 64);
        this->hex_str_to_bin(sig, this->pub_sig, 64);
        this->hex_str_to_bin(pri, this->pri_exp, 64);

    } catch (exception& e){
TODO(" TODO some cleaner syntax error management. I could define a testconf target in command line and show errors. Catching all errors at once would also be handy.")
        clog << "Exception raised " << e.what();
        _exit(1);
    };
}
