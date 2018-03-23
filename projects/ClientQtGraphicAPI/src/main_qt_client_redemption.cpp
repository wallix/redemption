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

    QtIOGraphicMouseKeyboard graphic_control_qt_obj;

    //new QtIOGraphicMouseKeyboard();
    ClientOutputGraphicAPI      * graphic_qt = &graphic_control_qt_obj;
    ClientInputMouseKeyboardAPI * control_qt = &graphic_control_qt_obj;
    QWidget * qwidget_parent = graphic_control_qt_obj.get_static_qwidget();

    QtInputOutputClipboard clipboard_api_obj(qwidget_parent);
    ClientIOClipboardAPI  * clipboard_api = &clipboard_api_obj;
    QtOutputSound sound_api_obj(qwidget_parent);
    ClientOutputSoundAPI  * sound_api     = &sound_api_obj;
    QtInputSocket socket_api_obj(qwidget_parent);
    ClientInputSocketAPI  * socket_api    = &socket_api_obj;


    RDPVerbose verbose = to_verbose_flags(0x0);

    ClientRedemption client_qt( argv, argc, verbose
                              , graphic_qt
                              , clipboard_api
                              , sound_api
                              , socket_api
                              , control_qt);
;
    app.exec();

//     delete(graphic_control_qt);

}


