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
   Copyright (C) Wallix 2010-2016
   Author(s): Clément Moroldo
*/


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"

#include "utils/log.hpp"


#include "client_redemption.hpp"


#include "qt_input_output_api/qt_output_sound.hpp"
#include "qt_input_output_api/qt_input_output_clipboard.hpp"
#include "qt_input_output_api/qt_input_socket.hpp"
#include "qt_input_output_api/qt_IO_graphic_mouse_keyboard.hpp"

#pragma GCC diagnostic pop





///////////////////////////////
// APPLICATION
int main(int argc, char** argv){

    //" -name QA\\administrateur -pwd '' -ip 10.10.46.88 -p 3389";

    // sudo python ./sesman/sesmanlink/WABRDPAuthentifier

    // sudo nano /etc/rdpproxy/rdpproxy.ini

    // /etc/rdpproxy/cert

    // bjam san -j4 rdpproxy

    // sudo bin/gcc-4.9.2/san/rdpproxy -nf

    // sudo bin/gcc-4.9.2/release/link-static/rdpproxy -nf

    //bjam -s qt=4 debug client_rdp_Qt4 && bin/gcc-4.9.2/debug/threading-multi/client_rdp_Qt4

    // sed '/usr\/include\/qt4\|threading-multi\/src\/Qt4\/\|in expansion of macro .*Q_OBJECT\|Wzero/,/\^/d' &&

    // ../../tools/c++-analyzer/bt  bin/gcc-4.9.2/debug/threading-multi/client_rdp_Qt4

    // ../packager/packager.py --version 1.0.0 --no-entry-changelog --build-package

    // sudo dpkg -i /home/qa/Desktop/redemption_0.9.740bjessie_amd64.deb



    QApplication app(argc, argv);

    QtIOGraphicMouseKeyboard * graphic_control_api = new QtIOGraphicMouseKeyboard();
    QtInputOutputClipboard * clipboard_api = new QtInputOutputClipboard(graphic_control_api->get_static_qwidget());
    QtOutputSound          * sound_api = new QtOutputSound(graphic_control_api->get_static_qwidget());
    QtInputSocket          * socket_api = new QtInputSocket(graphic_control_api->get_static_qwidget());


    // RDPVerbose::rdpdr_dump | RDPVerbose::cliprdr;
    //RDPVerbose::graphics | RDPVerbose::cliprdr | RDPVerbose::rdpdr;
    RDPVerbose verbose = to_verbose_flags(0);

    ClientRedemption front_qt(argv, argc, verbose, graphic_control_api, clipboard_api, sound_api, socket_api, graphic_control_api);


    app.exec();

    // scp -P 22 -r cmoroldo@10.10.43.46:/home/cmoroldo/Bureau/redemption_test_charge/movie.wrm /home/qa/Desktop/movie_sample_data/


    //  xfreerdp /u:x /p: /port:3389 /v:10.10.43.46 /multimon /monitors:2
}
