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
    Translator translator;
    Theme      theme;


public:
    FrontDemoQtClient(RDPVerbose verbose)
      : FrontQtRDPGraphicAPI(verbose)
      , translator(Translation::language_t::FR)
    {}

    ~FrontDemoQtClient() {}

    virtual mod_api * init_mod() override {

        try {

            // VNC
            this->mod = new mod_vnc( *(this->socket)
                                   , this->user_name.c_str()
                                   , this->user_password.c_str()
                                   , *(this)
                                   , this->info.width
                                   , this->info.height
                                   , this->ini.get<cfg::font>()
                                   , this->translator
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
                                   , this->authentifier
                                   , 0xffffffff);

        } catch (const Error &) {
            return nullptr;
        }

        return this->mod;
    }

//     virtual void callback() override {
//         FrontQtRDPGraphicAPI::callback();
//     }
//
//     virtual bool connect() override {
//         return FrontQtRDPGraphicAPI::connect();
//     }

//     virtual void options() override {
//         return FrontQtRDPGraphicAPI::options();
//     }


};

     void show_video_real_time() {


        struct timeval now = tvtime();
        time_t movie_time_tmp = this->current_time_movie;
        this->current_time_movie = now.tv_sec - this->movie_time_start.tv_sec + this->begin;

        if (this->current_time_movie > movie_time_tmp) {
            time_t real_time_record = this->_front->replay_mod.get()->get_real_time_movie_begin() + this->current_time_movie;
            std::string data(ctime(&real_time_record));
            //std::string data_cut(data.c_str(), data.size()-1);
            QString movie_real_time = QString(" Video Time: ") + QString(data.c_str());
            this->movie_timer_label.setText(movie_real_time);

            this->barRepaint(this->current_time_movie, QColor(Qt::green));
        }

        //this->movie_timer_label.setText(QString("  0/")+QString(std::to_string(int(movie_time)).c_str()));
    }

///////////////////////////////
// APPLICATION
int main(int argc, char** argv){

    QApplication app(argc, argv);

    FrontDemoQtClient front_qt(RDPVerbose::none);

    app.exec();
}
