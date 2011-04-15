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

   header file. Check object, used to check file's existance

*/

#if !defined(__CHECK_FILES_HPP__)
#define __CHECK_FILES_HPP__

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "file_loc.hpp"

struct Check_files{

    bool ad24b;
    bool ad256;
    bool cursor0;
    bool cursor1;
    bool sans;
    bool logo;
    bool wallix_logo;
    bool keys;
    bool config_file;

    Check_files(){
        this->ad24b = false;
        this->cursor0 = false;
        this->cursor1 = false;
        this->sans = false;
        this->logo = false;
        this->keys = false;
        this->config_file = false;
    }

    bool check_share(){
        using namespace std;
        string login_1(LOGIN_LOGO24);
        string cursor_arrow(CURSOR0);
        string cursor_pointer(CURSOR1);
        string font(FONT1);
        string logo(REDEMPTION_LOGO24);

        DIR * dirp = opendir(SHARE_PATH);
        if (!dirp){
            cerr << "Error opening " SHARE_PATH " directory : "
                      << strerror(errno)
                      << endl;
            return 0;
        }
        struct dirent * d = readdir(dirp);
        while (d){
            string line(d->d_name);
            if (login_1.compare(line) == 0){
                this->ad24b = true;
            }
            if(cursor_arrow.compare(line) == 0){
                this->cursor0 = true;
            }
            if(cursor_pointer.compare(line) == 0){
                this->cursor1 = true;
            }
            if(font.compare(line) == 0){
                this->sans = true;
            }
            if(logo.compare(line) == 0){
                this->logo = true;
            }
            d = readdir(dirp);
        }
        closedir(dirp);

        return this->ad24b
            && this->cursor0
            && this->cursor1
            && this->sans
            && this->logo;
    }

    bool check_etc(){
        using namespace std;
        string keys(RSAKEYS_INI);
        string config_file(RDPPROXY_INI);

        DIR * dirp = opendir(CFG_PATH);
        if (!dirp){
            cerr << "Error opening " CFG_PATH " directory : "
                      << strerror(errno)
                      << endl;
            return 0;
        }
        struct dirent * d = readdir(dirp);
        while (d){
            string line(d->d_name);
            if (keys.compare(line) == 0){
                this->keys = true;
            }
            if(config_file.compare(line) == 0){
                this->config_file = true;
            }
            d = readdir(dirp);
        }
        closedir(dirp);

        return this->keys
            && this->config_file;
    }
};

#endif
