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
#include "qt_input_output_api/qt_IO_graphic_mouse_keyboard.hpp"
#include "qt_input_output_api/qt_input_socket.hpp"


#pragma GCC diagnostic pop




int main(int argc, char** argv) {


    QApplication app(argc, argv);

    QtIOGraphicMouseKeyboard * graphic_control_qt = new QtIOGraphicMouseKeyboard();
    QWidget * qwidget_parent = graphic_control_qt->get_static_qwidget();

    ClientIOClipboardAPI  * clipboard_api = new QtInputOutputClipboard(qwidget_parent);
    ClientOutputSoundAPI  * sound_api     = new QtOutputSound(qwidget_parent);
    ClientInputSocketAPI  * socket_api    = new QtInputSocket(qwidget_parent);


    RDPVerbose verbose = to_verbose_flags(0);

    ClientRedemption client_qt( argv, argc, verbose
                              , graphic_control_qt
                              , clipboard_api
                              , sound_api
                              , socket_api
                              , graphic_control_qt);

    app.exec();

    delete(graphic_control_qt);

}


