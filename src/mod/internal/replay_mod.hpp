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
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou, Jonathan Poelen,
              Meng Tan
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Use (implemented) basic RDP orders to draw some known test pattern
*/

#ifndef REDEMPTION_MOD_INTERNAL_REPLAY_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_REPLAY_MOD_HPP

#include "FileToGraphic.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "internal_mod.hpp"

class ReplayMod : public InternalMod {
    char movie[1024];

    std::string & auth_error_message;

    InMetaSequenceTransport * in_trans;
    FileToGraphic           * reader;

    bool end_of_data;
    bool wait_for_escape;

public:
    ReplayMod( FrontAPI & front
             , const char * replay_path
             , const char * movie
             , uint16_t width
             , uint16_t height
             , std::string & auth_error_message
             , Font const & font
             , bool wait_for_escape
             , uint32_t debug_capture)
    : InternalMod(front, width, height, font)
    , auth_error_message(auth_error_message)
    , end_of_data(false)
    , wait_for_escape(wait_for_escape)
    {
        strncpy(this->movie, replay_path, sizeof(this->movie)-1);
        strncat(this->movie, movie, sizeof(this->movie)-1);
        LOG(LOG_INFO, "Playing %s", this->movie);

        char path[1024];
        char basename[1024];
        char extension[128];
        strcpy(path, RECORD_PATH); // default value, actual one should come from movie_path
        strcpy(basename, "replay"); // default value actual one should come from movie_path
        strcpy(extension, ".mwrm"); // extension is currently ignored
        char prefix[4096];
        const bool res = canonical_path( this->movie
                                       , path, sizeof(path)
                                       , basename, sizeof(basename)
                                       , extension, sizeof(extension)
                                       );
        if (!res) {
            LOG(LOG_ERR, "Buffer Overflowed: Path too long");
            throw Error(ERR_RECORDER_FAILED_TO_FOUND_PATH);
        }
        snprintf(prefix,  sizeof(prefix), "%s%s", path, basename);

        TODO("We don't know yet how to manage encryption for replay. For now its just not supported, rnd and ini will never be used");
        LCGRandom rnd(0);
        Inifile ini;
        TODO("This use of cctx for replay will lead to memory leak because the object is never deallocated, fix that");
        CryptoContext * cctx = new CryptoContext(rnd, ini); // cctx will not be really used
        this->in_trans = new InMetaSequenceTransport(cctx, prefix, extension, 0, 0);

        timeval const begin_capture{0, 0};
        timeval const end_capture{0, 0};
        this->reader = new FileToGraphic( this->in_trans, begin_capture, end_capture, true, debug_capture);

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
        }

        this->reader->add_consumer(&this->front, nullptr, nullptr, nullptr, nullptr);
    }

    ~ReplayMod() override {
        delete reader;
        delete in_trans;
        this->screen.clear();
    }

    void rdp_input_invalidate(const Rect & /*rect*/) override {
    }

    void rdp_input_mouse(int /*device_flags*/, int /*x*/, int /*y*/, Keymap2 * /*keymap*/) override {
    }

    void rdp_input_scancode(long /*param1*/, long /*param2*/,
                                    long /*param3*/, long /*param4*/, Keymap2 * keymap) override {
        if (keymap->nb_kevent_available() > 0
         && keymap->get_kevent() == Keymap2::KEVENT_ESC) {
            this->event.signal = BACK_EVENT_STOP;
            this->event.set();
        }
    }

    void rdp_input_synchronize(uint32_t /*time*/, uint16_t /*device_flags*/,
                                       int16_t /*param1*/, int16_t /*param2*/) override {
    }

    // event from back end (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    void draw_event(time_t now, const GraphicApi & drawable) override {
        TODO("use system constants for sizes");
        TODO("RZ: Support encrypted recorded file.");
        if (!this->end_of_data) {
            try
            {
                int i;
                for (i = 0; (i < 500) && this->reader->next_order(); i++) {
                    this->reader->interpret_order();
                    //sleep(1);
                }
                if (i == 500) {
                    this->event.set(1);
                }
                else {
                    this->front.sync();

                    if (!this->wait_for_escape) {
                        this->event.signal = BACK_EVENT_STOP;
                        this->event.set(1);
                    }
                    this->end_of_data = true;
                }
            }
            catch (Error & e) {
                if (e.id == ERR_TRANSPORT_OPEN_FAILED) {
                    this->auth_error_message = "The recorded file is inaccessible or corrupted!";

                    this->event.signal = BACK_EVENT_NEXT;
                    this->event.set(1);
                }
                else {
                    throw;
                }
            }
        }
    }
};

#endif
