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
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou, Jonathan Poelen
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Use (implemented) basic RDP orders to draw some known test pattern
*/

#ifndef REDEMPTION_MOD_INTERNAL_REPLAY_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_REPLAY_MOD_HPP

#include "FileToGraphic.hpp"
#include "RDP/RDPGraphicDevice.hpp"
#include "inbymetasequencetransport.hpp"
#include "internal_mod.hpp"

class ReplayMod : public InternalMod {
    char movie[1024];

    redemption::string & auth_error_message;

    InByMetaSequenceTransport * in_trans;
    FileToGraphic             * reader;

public:
    ReplayMod( FrontAPI & front
             , char * replay_path
             , char * movie
             , uint16_t width
             , uint16_t height
             , redemption::string & auth_error_message)
    : InternalMod(front, width, height)
    , auth_error_message(auth_error_message)
    {
        strcpy(this->movie, replay_path);
        strcat(this->movie, movie);
        LOG(LOG_INFO, "Playing %s", this->movie);

        char path[1024];
        char basename[1024];
        char extension[128];
        strcpy(path, RECORD_PATH); // default value, actual one should come from movie_path
        strcpy(basename, "replay"); // default value actual one should come from movie_path
        strcpy(extension, ".mwrm"); // extension is currently ignored
        char prefix[4096];

        canonical_path(this->movie, path, sizeof(path), basename, sizeof(basename), extension, sizeof(extension));
        sprintf(prefix, "%s%s", path, basename);

        this->in_trans = new InByMetaSequenceTransport(prefix, extension);
        timeval begin_capture; begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
        timeval end_capture; end_capture.tv_sec = 0; end_capture.tv_usec = 0;
        this->reader = new FileToGraphic(this->in_trans, begin_capture, end_capture, true, 0);

        switch (this->front.server_resize( this->reader->info_width
                                         , this->reader->info_height
                                         , this->reader->info_bpp)) {
        case 0:
            // no resizing needed
            break;
        case 1:
            // resizing done
            this->front_width  = this->reader->info_width;
            this->front_height = this->reader->info_height;

            this->screen.rect.cx = this->reader->info_width;
            this->screen.rect.cy = this->reader->info_height;

            break;
        case -1:
            // resizing failed
            // thow an Error ?
            LOG(LOG_WARNING, "Older RDP client can't resize to server asked resolution, disconnecting");
            throw Error(ERR_VNC_OLDER_RDP_CLIENT_CANT_RESIZE);
            break;
        }

        this->reader->add_consumer(&this->front);
        this->front.send_global_palette();
    }

    virtual ~ReplayMod()
    {
        if (reader)
            delete reader;

        if (in_trans)
            delete in_trans;
    }

    virtual void rdp_input_invalidate(const Rect & /*rect*/)
    {
    }

    virtual void rdp_input_mouse(int /*device_flags*/, int /*x*/, int /*y*/, Keymap2 * /*keymap*/)
    {
    }

    virtual void rdp_input_scancode(long /*param1*/, long /*param2*/,
                                    long /*param3*/, long /*param4*/, Keymap2 * /*keymap*/)
    {
    }

    virtual void rdp_input_synchronize(uint32_t /*time*/, uint16_t /*device_flags*/,
                                       int16_t param1, int16_t /*param2*/)
    {
    }

    // event from back end (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    virtual void draw_event()
    {
        TODO("use system constants for sizes");
        TODO("RZ: Support encrypted recorded file.")
        try
        {
            int i;
            for (i = 0; (i < 500) && this->reader->next_order(); i++) {
                this->reader->interpret_order();
            }
            if (i == 500) {
                this->event.set(1);
            }
            else {
                this->front.flush();
                this->event.signal = BACK_EVENT_STOP;
                this->event.set(1);
            }
        }
        catch (Error & e) {
            if (e.id == ERR_TRANSPORT_OPEN_FAILED) {
                this->auth_error_message.copy_c_str("The recorded file is inaccessible or corrupted!");

                this->event.signal = BACK_EVENT_NEXT;
                this->event.set(1);
            }
            else {
                throw;
            }
        }
    }
};

#endif
