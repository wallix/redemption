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

#include "capture/file_to_graphic.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "mod/internal/internal_mod.hpp"


class ReplayMod : public InternalMod
{
    std::string & auth_error_message;

    CryptoContext           cctx;
    struct TemporaryCtxPath
    {
        char extension[128];
        char prefix[4096];

        //TODO: should be generalized to some wide use FilePath object
        // with basename, path, ext, etc. methods and use it for passing
        // around all of redemption pathes.
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
    std::unique_ptr<InMetaSequenceTransport> in_trans;
    std::unique_ptr<FileToGraphic> reader;

    bool end_of_data;
    bool wait_for_escape;

    time_t balise_time_frame;
    bool sync_setted;
    bool loop_on_movie;

public:
    using Verbose = FileToGraphic::Verbose;


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
    : ReplayMod(
        front, replay_path, movie, width, height, auth_error_message,
        font, wait_for_escape, timeval{0, 0}, timeval{0, 0}, 0, debug_capture)
    {
    }

    ReplayMod( FrontAPI & front
             , const char * replay_path
             , const char * movie
             , uint16_t width
             , uint16_t height
             , std::string & auth_error_message
             , Font const & font
             , bool wait_for_escape
             , timeval const & begin_read
             , timeval const & end_read
             , time_t balise_time_frame
             , Verbose debug_capture)
    : InternalMod(front, width, height, font, Theme{}, false)
    , auth_error_message(auth_error_message)
    , movie_path(replay_path, movie)
    //, in_trans(this->cctx, movie_path.prefix, movie_path.extension, InCryptoTransport::EncryptionMode::NotEncrypted)
    //, reader(this->in_trans, begin_read, end_read, true, debug_capture)
    , end_of_data(false)
    , wait_for_escape(wait_for_escape)
    , balise_time_frame(balise_time_frame)
    , loop_on_movie(false)
    {
        this->in_trans = std::make_unique<InMetaSequenceTransport>(this->cctx, movie_path.prefix, movie_path.extension, InCryptoTransport::EncryptionMode::NotEncrypted);
        this->reader =  std::make_unique<FileToGraphic>(*(this->in_trans.get()), begin_read, end_read, true, debug_capture);

        switch (this->front.server_resize( this->reader.get()->info_width
                                         , this->reader.get()->info_height
                                         , this->reader.get()->info_bpp)) {
        case FrontAPI::ResizeResult::no_need:
            // no resizing needed
            break;
        case FrontAPI::ResizeResult::instant_done:
        case FrontAPI::ResizeResult::done:
            // resizing done;
            this->front_width  = this->reader.get()->info_width;
            this->front_height = this->reader.get()->info_height;

            this->screen.set_wh(this->reader.get()->info_width, this->reader.get()->info_height);

            break;
        case FrontAPI::ResizeResult::fail:
            // resizing failed
            // thow an Error ?
            LOG(LOG_WARNING, "Older RDP client can't resize to server asked resolution, disconnecting");
            throw Error(ERR_VNC_OLDER_RDP_CLIENT_CANT_RESIZE);
        }

        this->reader.get()->add_consumer(&this->front, nullptr, nullptr, nullptr, nullptr);
        time_t begin_file_read = begin_read.tv_sec+this->in_trans.get()->get_meta_line().start_time - this->balise_time_frame;
        this->in_trans.get()->set_begin_time(begin_file_read);
        this->front.can_be_start_capture();
    }

    void add_consumer(
        gdi::GraphicApi * graphic_ptr,
        gdi::CaptureApi * capture_ptr,
        gdi::KbdInputApi * kbd_input_ptr,
        gdi::CaptureProbeApi * capture_probe_ptr,
        gdi::ExternalCaptureApi * external_event_ptr
    ) {
        this->reader.get()->add_consumer(
            graphic_ptr,
            capture_ptr,
            kbd_input_ptr,
            capture_probe_ptr,
            external_event_ptr
        );
    }

    void play() {
        this->reader.get()->play(false);
    }

    FileToGraphic * get_reader() {
        return this->reader.get();
    }

    bool play_client() {
        return this->reader.get()->play_client();
    }

    void set_sync() {
        this->reader.get()->set_sync();
    }

    WrmVersion get_wrm_version() {
        return this->in_trans.get()->get_wrm_version();
    }

    bool get_break_privplay_client() {
        return this->reader.get()->break_privplay_client;
    }

    ~ReplayMod() override {
        this->screen.clear();
    }

    void instant_play_client(std::chrono::microseconds endin_frame) {
            this->reader.get()->instant_play_client(endin_frame);
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
        this->reader.get()->set_pause_client(time);
    }

    void set_wait_after_load_client(timeval & time) {
        this->reader.get()->set_wait_after_load_client(time);
    }

    time_t get_real_time_movie_begin() {
        return this->in_trans.get()->get_meta_line().start_time;
    }

    time_t get_movie_time_length() {
        time_t start = this->in_trans.get()->get_meta_line().start_time;

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

        return (end - start);
    }

    void refresh(Rect /*rect*/) override {}

    void set_loop_on_movie(bool loop) {
        this->loop_on_movie = loop;
    }

    // event from back end (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    void draw_event(time_t now, gdi::GraphicApi & drawable) override {
        (void)now;
        (void)drawable;
        // TODO use system constants for sizes
        // TODO RZ: Support encrypted recorded file.
        if (!(this->sync_setted)) {
            this->sync_setted =  true;
            this->reader.get()->set_sync();
        }
        if (!this->end_of_data) {
            try
            {

                int i;
                for (i = 0; i < 500; i++) {

                    struct timeval now                = tvtime();
                    std::chrono::microseconds elapsed = difftimeval(now, this->reader.get()->start_synctime_now) ;
                    if (elapsed >= this->reader.get()->movie_elapsed_client) {
                        if (this->reader.get()->next_order()) {
                            this->reader.get()->interpret_order();
    //                         sleep(1);
                        } else {
                            if (this->loop_on_movie) {
                                this->in_trans = std::make_unique<InMetaSequenceTransport>(this->cctx, movie_path.prefix, movie_path.extension, InCryptoTransport::EncryptionMode::NotEncrypted);
                                this->reader =  std::make_unique<FileToGraphic>(*(this->in_trans.get()), timeval({0, 0}), timeval({0, 0}), true, FileToGraphic::Verbose::none);

                                //this->reader.get()->reinit();
                                this->reader.get()->add_consumer(&this->front, nullptr, nullptr, nullptr, nullptr);

//                                 if (this->reader.get()->next_order()) {
//                                     this->reader.get()->interpret_order();
//
//                                 }

                            } else {
                                TimeSystem timeobj;
                                this->disconnect(timeobj.get_time().tv_sec);
                                this->event.signal = BACK_EVENT_STOP;
                                this->event.set(1);
                                this->end_of_data = true;
                            }
                        }
                    }
                }
                if (i == 500) {
                    this->event.set(1);
                } else {
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
