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

#include "core/session_reactor.hpp"
#include "client_redemption/client_redemption.hpp"
#include "utils/set_exception_handler_pretty_message.hpp"

#include "qt_input_output_api/qt_output_sound.hpp"
#include "qt_input_output_api/qt_input_output_clipboard.hpp"
#include "qt_input_output_api/qt_IO_graphic_mouse_keyboard.hpp"
#include "qt_input_output_api/qt_input_socket.hpp"
#include "qt_input_output_api/IO_disk.hpp"



int main(int argc, char** argv)
{
    set_exception_handler_pretty_message();

    SessionReactor reactor;

    QApplication app(argc, argv);

    QtIOGraphicMouseKeyboard graphic_control_qt_obj;
    QWidget * qwidget_parent = graphic_control_qt_obj.get_static_qwidget();
    QtInputOutputClipboard clipboard_api_obj(qwidget_parent);
    QtOutputSound sound_api_obj(qwidget_parent);
    IODisk ioDisk_api_obj;
    QtInputSocket socket_api_obj(reactor, qwidget_parent);

    ClientOutputGraphicAPI      * graphic_qt = &graphic_control_qt_obj;
    ClientInputMouseKeyboardAPI * control_qt = &graphic_control_qt_obj;
    ClientIOClipboardAPI * clipboard_api = &clipboard_api_obj;
    ClientOutputSoundAPI * sound_api     = &sound_api_obj;
    ClientInputSocketAPI * socket_api    = &socket_api_obj;
    ClientIODiskAPI      * ioDisk_api    = &ioDisk_api_obj;

    //RDPVerbose::rdpdr_dump;           //to_verbose_flags(0x0);
    //;           //RDPVerbose::graphics;
    //        to_verbose_flags(0x0);
    //RDPVerbose::graphics;
    RDPVerbose verbose = to_verbose_flags(0x0);             //(RDPVerbose::rdpdr_dump | RDPVerbose::rdpdr);

    ClientRedemption client_qt( reactor, argv, argc, verbose
                              , graphic_qt
                              , clipboard_api
                              , sound_api
                              , socket_api
                              , control_qt
                              , ioDisk_api);

    app.exec();
}
