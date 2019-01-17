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
   Copyright (C) Wallix 2010
   Author(s): Clément Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to writing RDP orders to file and rereading them
*/

#pragma once

#include <unistd.h>
#include <iostream>
#include <string>

#include <QtCore/QStringList>


#ifndef CLIENT_REDMPTION_MAIN_PATH
//# error "undefined MAIN_PATH macro"
# define CLIENT_REDMPTION_MAIN_PATH ""
#endif



namespace FormSpec {

    const QString title("ReDemPtion RDP Client");

    enum : int {
        w  = 400,
        h = 300
    };

    namespace Label {
        const QString ip("IP server :");
        const QString user("User name : ");
        const QString password("Password :  ");
        const QString port("Port :      ");
        const QString empty_space(""            );
        const QString password_combobox("Save password:");
    }

    namespace Button {

        namespace connect {
            const QString label("Connection");
            const QString tooltip("Connection");

            enum : int {
                w = 110,
                h = 24,
                x = 280,
                y = 256
            };
        }

        namespace options {
            const QString label("Options");
            const QString tooltip("Options");

            enum : int {
                w = 110,
                h = 24,
                x = 10,
                y = 256
            };
        }

        namespace replay {
            QString label("Replay");
            QString tooltip("Replay");

            enum : int {
                w = 110,
                h = 24,
                x = 10,
                y = 226
            };
        }
    }

    namespace tag {
        const std::string save_pwd("save_pwd");
        const std::string last_target("last_target");
        const std::string title("title");
        const std::string IP("IP");
        const std::string name("name");
        const std::string pwd("pwd");
        const std::string options_profil("options_profil");
    }

    namespace dialog {
        const char * title = "Open a Movie";
        const char * regex = "Movie Files(*.mwrm)";
    }
}






namespace ScreenSpec {

    const QString title("ReDemPtion RDP Client");

    namespace Buttons {

        const QString play("Play");
        const QString stop("Stop");
        const QString close("Close");
        const QString pause("Pause");

        const QString ctrl_alt_del("CTRL + ALT + DELETE");
        const QString refresh("Refresh");
        const QString disconnection("Disconnection");

        enum :  int {
            h = 20
        };

    }

    const QString load_progresse_bar_title("Loading Movie");
}




namespace Global {

    constexpr long long WINDOWS_TICK = 10000000;
    constexpr long long SEC_TO_UNIX_EPOCH = 11644473600LL;

    enum : int {
        CHANID_CLIPDRD = 1601,
        CHANID_RDPDR   = 1602,
        CHANID_WABDIAG = 1603,
        CHANID_RDPSND  = 1604,
        CHANID_RAIL    = 1605
    };
}

namespace OptionsSpec {

}


namespace Error {

}
