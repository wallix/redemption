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
   Author(s): Clément Moroldo

*/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"

#include "rdp_client_graphic_api/front_qt_rdp_graphic_api.hpp"

// VNC
#include "mod/vnc/vnc.hpp"

#pragma GCC diagnostic pop



class FrontDemoQtClient : public FrontQtRDPGraphicAPI
{

    // VNC
    Inifile    ini;
    Theme      theme;
    bool is_apple;
    WindowListCaps windowListCaps;
    ClientExecute exe;


public:
    FrontDemoQtClient(RDPVerbose verbose)
      : FrontQtRDPGraphicAPI(verbose)
      , exe(*(this),  this->windowListCaps,  false)
      , is_apple(true)
    {}

    ~FrontDemoQtClient() {}

    virtual mod_api * init_mod() override {

        try {
            // VNC MOD
            this->mod = new mod_vnc( *(this->socket)
                                   , this->user_name.c_str()
                                   , this->user_password.c_str()
                                   , *(this)
                                   , this->info.width
                                   , this->info.height
                                   , this->ini.get<cfg::font>()
                                   , nullptr
                                   , nullptr
                                   , this->theme
                                   , this->info.keylayout
                                   , 0
                                   , true
                                   , true
                                   , "0,1,-239"
                                   , false
                                   , true
                                   , mod_vnc::ClipboardEncodingType::UTF8
                                   , VncBogusClipboardInfiniteLoop::delayed
                                   , this->reportMessage
                                   , this->is_apple
                                   , &(this->exe)
                                   , to_verbose_flags(0xfffffffd));

        } catch (const Error &) {
            return nullptr;
        }

        return this->mod;
    }

    virtual void callback() override {
        FrontQtRDPGraphicAPI::callback();
    }

    virtual void connect() override {
        FrontQtRDPGraphicAPI::connect();
        // TODO check verr num state
    }

    virtual void options() override {
        FrontQtRDPGraphicAPI::options();
    }

};



///////////////////////////////
// APPLICATION
int main(int argc, char** argv){

    QApplication app(argc, argv);

    FrontDemoQtClient front_qt(RDPVerbose::none);

    app.exec();
}
