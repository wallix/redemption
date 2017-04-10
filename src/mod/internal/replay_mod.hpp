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
              Meng Tan, Clément Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Use (implemented) basic RDP orders to draw some known test pattern
*/


#pragma once

#include "capture/capture.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "internal_mod.hpp"


class ReplayMod : public InternalMod
{
    std::string & auth_error_message;

    CryptoContext           cctx;
    struct TemporaryCtxPath
    {
        char extension[128];
        char prefix[4096];

        TemporaryCtxPath(const char * replay_path, const char * movie)
        {
            char path_movie[1024];
            std::snprintf(path_movie,  sizeof(path_movie)-1, "%s%s", replay_path, movie);
            path_movie[sizeof(path_movie)-1] = 0;
            LOG(LOG_INFO, "Playing %s", path_movie);

            char path[1024];
            char basename[1024];
            strcpy(path, RECORD_PATH); // default value, actual one should come from movie_path
            strcpy(basename, "replay"); // default value actual one should come from movie_path
            strcpy(this->extension, ".mwrm"); // extension is currently ignored

            const bool res = canonical_path(
                path_movie,
                path, sizeof(path),
                basename, sizeof(basename),
                this->extension, sizeof(this->extension)
            );

            if (!res) {
                LOG(LOG_ERR, "Buffer Overflowed: Path too long");
                throw Error(ERR_RECORDER_FAILED_TO_FOUND_PATH);
            }

            std::snprintf(this->prefix,  sizeof(this->prefix), "%s%s", path, basename);
        }
    } movie_path;
    InMetaSequenceTransport in_trans;
    FileToGraphic           reader;

    bool end_of_data;
    bool wait_for_escape;

public:
    using Verbose = FileToGraphic::Verbose;

private:


public:
    ReplayMod( FrontAPI & front
             , const char * replay_path
             , const char * movie
             , uint16_t width
             , uint16_t height
             , std::string & auth_error_message
             , Font const & font
             , bool wait_for_escape
             , Verbose debug_capture)
    : InternalMod(front, width, height, font, Theme{}, false)
    , auth_error_message(auth_error_message)
    , movie_path(replay_path, movie)
    , in_trans(&this->cctx, movie_path.prefix, movie_path.extension, 0)
    , reader(this->in_trans, /*begin_capture*/{0, 0}, /*end_capture*/{0, 0}, true, debug_capture)
    , end_of_data(false)
    , wait_for_escape(wait_for_escape)
    {
        switch (this->front.server_resize( this->reader.info_width
                                         , this->reader.info_height
                                         , this->reader.info_bpp)) {
        case FrontAPI::ResizeResult::no_need:
            // no resizing needed
            break;
        case FrontAPI::ResizeResult::instant_done:
        case FrontAPI::ResizeResult::done:
            // resizing done
            this->front_width  = this->reader.info_width;
            this->front_height = this->reader.info_height;

            this->screen.set_wh(this->reader.info_width, this->reader.info_height);

            break;
        case FrontAPI::ResizeResult::fail:
            // resizing failed
            // thow an Error ?
            LOG(LOG_WARNING, "Older RDP client can't resize to server asked resolution, disconnecting");
            throw Error(ERR_VNC_OLDER_RDP_CLIENT_CANT_RESIZE);
        }

        this->reader.add_consumer(&this->front, nullptr, nullptr, nullptr, nullptr);
    }

    ReplayMod( FrontAPI & front
             , const char * replay_path
             , const char * movie
             , uint16_t width
             , uint16_t height
             , std::string & auth_error_message
             , Font const & font
             , bool wait_for_escape
             , timeval & begin_read
             , timeval & end_read
             , Verbose debug_capture)
    : InternalMod(front, width, height, font, Theme{}, false)
    , auth_error_message(auth_error_message)
    , movie_path(replay_path, movie)
    , in_trans(&this->cctx, movie_path.prefix, movie_path.extension, 0)
    , reader(this->in_trans, begin_read, end_read, true, debug_capture)
    , end_of_data(false)
    , wait_for_escape(wait_for_escape)
    {
        switch (this->front.server_resize( this->reader.info_width
                                         , this->reader.info_height
                                         , this->reader.info_bpp)) {
        case FrontAPI::ResizeResult::no_need:
            // no resizing needed
            break;
        case FrontAPI::ResizeResult::instant_done:
        case FrontAPI::ResizeResult::done:
            // resizing done
            this->front_width  = this->reader.info_width;
            this->front_height = this->reader.info_height;

            this->screen.set_wh(this->reader.info_width, this->reader.info_height);

            break;
        case FrontAPI::ResizeResult::fail:
            // resizing failed
            // thow an Error ?
            LOG(LOG_WARNING, "Older RDP client can't resize to server asked resolution, disconnecting");
            throw Error(ERR_VNC_OLDER_RDP_CLIENT_CANT_RESIZE);
        }

        this->reader.add_consumer(&this->front, nullptr, nullptr, nullptr, nullptr);
    }

    void add_consumer(
        gdi::GraphicApi * graphic_ptr,
        gdi::CaptureApi * capture_ptr,
        gdi::KbdInputApi * kbd_input_ptr,
        gdi::CaptureProbeApi * capture_probe_ptr,
        gdi::ExternalCaptureApi * external_event_ptr
    ) {
        this->reader.add_consumer(
            graphic_ptr,
            capture_ptr,
            kbd_input_ptr,
            capture_probe_ptr,
            external_event_ptr
        );
    }

    void play() {
        this->reader.play(false);
    }

    FileToGraphic * get_reader() {
        return &(this->reader);
    }

    bool play_client() {
        return this->reader.play_client();
    }

    bool get_break_privplay_client() {
        return this->reader.break_privplay_client;
    }

    ~ReplayMod() override {
        this->screen.clear();
    }

    void rdp_input_invalidate(Rect /*rect*/) override {}

    void rdp_input_mouse(int /*device_flags*/, int /*x*/, int /*y*/, Keymap2 * /*keymap*/) override {}

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

    void set_pause(timeval & time) {
        this->reader.set_pause_client(time);
    }

    time_t get_real_time_movie_begin() {
        return this->in_trans.meta_line.start_time;
    }

    time_t get_movie_time_length() {
        time_t start = this->in_trans.meta_line.start_time;

        std::string movie_path_str(this->movie_path.prefix);
        movie_path_str += std::string(".mwrm");

        std::ifstream file(movie_path_str.c_str());
        std::string line;
        std::string last_line;
        if (file.good()) {
            while(!file.eof()) {
                last_line = line;
                getline(file,line);
            }
        }

        size_t pos = last_line.find(".wrm ");
        last_line = last_line.substr(pos+5, last_line.length());
        while (pos != last_line.length()) {
            pos = last_line.find(" ");
            last_line = last_line.substr(pos+1, last_line.length());
        }

        char end_chars[10] = {0};
        for (size_t i = 0; i < last_line.length();i++) {
            end_chars[i] = last_line.c_str()[i];
        }
        char * end_chars_end = end_chars;
        time_t end = strtoll(end_chars, &end_chars_end, 10);

        return end - start;
    }

    void refresh(Rect /*rect*/) override {}

    // event from back end (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    void draw_event(time_t now, gdi::GraphicApi & drawable) override {
        (void)now;
        (void)drawable;
        // TODO use system constants for sizes
        // TODO RZ: Support encrypted recorded file.
        if (!this->end_of_data) {
            try
            {
                int i;
                for (i = 0; (i < 500) && this->reader.next_order(); i++) {
                    this->reader.interpret_order();
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
            catch (Error const & e) {
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

    bool is_up_and_running() override { return true; }
};
