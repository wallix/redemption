/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/


#pragma once

namespace CHANNELS {
#define DEF_NAME(name) static constexpr const char * name = #name
    struct channel_names {
        DEF_NAME(cliprdr);
        DEF_NAME(rdpdr);
        DEF_NAME(rdpsnd);
        DEF_NAME(drdynvc);
        DEF_NAME(rail);
        DEF_NAME(sespro);
        DEF_NAME(audio_output);
    };
#undef DEF_NAME
}

using CHANNELS::channel_names;

//         DEF_NAME(dynamique = "drdynvc";
//         DEF_NAME(plug_and_play_devices = "pnpdr";
//         DEF_NAME(video_redirection = "tsmf";
//         DEF_NAME(printer_ticket = "tsvctkt";
//         DEF_NAME(printer_driver = "xpsrd";
//         DEF_NAME(drive_letters = "wmsdl";
//         DEF_NAME(audio_level = "wmsaud";
//         DEF_NAME(audio_input = "audio_input";
//         DEF_NAME(file_system = "rdpdr";
//         DEF_NAME(clipboard = "cliprdr";
//         DEF_NAME(sound = "rdpsnd";
//         DEF_NAME(echo = "echo";
//         DEF_NAME(usb = "urbdrc";

