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


#ifndef _NO_QT

#ifndef _NO_SOUND
#include "qt_input_output_api/qt_output_sound.hpp"
#endif

#ifndef _NO_CLIPBOARD
#include "qt_input_output_api/qt_input_output_clipboard.hpp"
#endif

#ifndef _NO_GRAPHICS
#include "qt_input_output_api/qt_IO_graphic_mouse_keyboard.hpp"
#endif

#include "qt_input_output_api/qt_input_socket.hpp"

#endif

#pragma GCC diagnostic pop


// #define _NO_QT
// #define _NO_GRAPHICS
// #define _NO_CLIPBOARD
// #define _NO_SOUND


int main(int argc, char** argv) {


#ifndef _NO_QT
    QApplication app(argc, argv);
#endif

    ClientOutputGraphicAPI      * graphic_api = nullptr;
    ClientIOClipboardAPI        * clipboard_api =nullptr;
    ClientOutputSoundAPI        * sound_api = nullptr;
    ClientInputSocketAPI        * socket_api = nullptr;
    ClientInputMouseKeyboardAPI * control_api = nullptr;

#ifndef _NO_QT

    QWidget * qwidget_parent = nullptr;

#ifndef _NO_GRAPHICS
    QtIOGraphicMouseKeyboard * graphic_control_qt = new QtIOGraphicMouseKeyboard();
    graphic_api = graphic_control_qt;
    control_api = graphic_control_qt;
    qwidget_parent = graphic_control_qt->get_static_qwidget();
#endif


#ifndef _NO_CLIPBOARD
    QtInputOutputClipboard * clipboard_qt = new QtInputOutputClipboard(qwidget_parent);
    clipboard_api = clipboard_qt;
#endif


#ifndef _NO_SOUND
    QtOutputSound          * sound_qt= new QtOutputSound(qwidget_parent);
    sound_api = sound_qt;
#endif

    QtInputSocket          * socket_qt= new QtInputSocket(qwidget_parent);
    socket_api = socket_qt;

#endif


    RDPVerbose verbose = to_verbose_flags(0);

    ClientRedemption front_qt(argv, argc, verbose
                            , graphic_api
                            , clipboard_api
                            , sound_api
                            , socket_api
                            , control_api);

#ifndef _NO_QT
    app.exec();
#endif

#ifndef _NO_GRAPHICS
    delete(graphic_api);
#else
    delete(clipboard_api);
    delete(sound_api);
    delete(socket_api);
#endif

}


