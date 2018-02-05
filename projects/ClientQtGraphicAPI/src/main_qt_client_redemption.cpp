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





int main(int argc, char** argv) {

    QApplication app(argc, argv);

    QtIOGraphicMouseKeyboard * graphic_control_api = new QtIOGraphicMouseKeyboard();
    QtInputOutputClipboard * clipboard_api = new QtInputOutputClipboard(graphic_control_api->get_static_qwidget());
    QtOutputSound          * sound_api = new QtOutputSound(graphic_control_api->get_static_qwidget());
    QtInputSocket          * socket_api = new QtInputSocket(graphic_control_api->get_static_qwidget());


    // RDPVerbose::rdpdr_dump | RDPVerbose::cliprdr;
    //RDPVerbose::graphics | RDPVerbose::cliprdr | RDPVerbose::rdpdr;
    RDPVerbose verbose = to_verbose_flags(0);

    ClientRedemption front_qt(argv, argc, verbose
                            , graphic_control_api
                            , clipboard_api
                            , nullptr                       // no sound implementation
                            , socket_api
                            , graphic_control_api);

    app.exec();

    delete(graphic_control_api);
}


