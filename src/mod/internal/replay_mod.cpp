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


#include "core/app_path.hpp"
#include "mod/internal/replay_mod.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "core/front_api.hpp"
#include "keyboard/keymap2.hpp"

ReplayMod::TemporaryCtxPath::TemporaryCtxPath(const char * replay_path, const char * movie)
{
    char path_movie[1024];
    std::snprintf(path_movie, sizeof(path_movie)-1, "%s%s", replay_path, movie);
    path_movie[sizeof(path_movie)-1] = 0;
    LOG(LOG_INFO, "Playing %s", path_movie);

    char path[1024];
    char basename[1024];
    strcpy(path, app_path(AppPath::Record)); // default value, actual one should come from movie_path
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

ReplayMod::ReplayMod(
    FrontAPI & front
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
  , bool replay_on_loop
  , Verbose debug_capture)
: InternalMod(front, width, height, font, Theme{}, true)
, auth_error_message(auth_error_message)
, movie_path(replay_path, movie)
// TODO RZ: Support encrypted recorded file.
, in_trans(std::make_unique<InMetaSequenceTransport>(
    this->cctx,
    this->movie_path.prefix,
    this->movie_path.extension,
    InCryptoTransport::EncryptionMode::NotEncrypted,
    this->fstat))
, reader(std::make_unique<FileToGraphic>(
    *this->in_trans,
    begin_read,
    end_read,
    true,
    debug_capture))
, end_of_data(false)
, wait_for_escape(wait_for_escape)
, balise_time_frame(balise_time_frame)
, sync_setted(false)
, replay_on_loop(replay_on_loop)
{
    switch (this->front.server_resize( this->reader->info_width
                                     , this->reader->info_height
                                     , this->reader->info_bpp)) {
    case FrontAPI::ResizeResult::no_need:
        // no resizing needed
        break;
    case FrontAPI::ResizeResult::instant_done:
    case FrontAPI::ResizeResult::remoteapp:
    case FrontAPI::ResizeResult::done:
        // resizing done;
        this->front_width  = this->reader->info_width;
        this->front_height = this->reader->info_height;

        this->screen.set_wh(this->reader->info_width, this->reader->info_height);

        break;
    case FrontAPI::ResizeResult::fail:
        // resizing failed
        // thow an Error ?
        LOG(LOG_WARNING, "Older RDP client can't resize to server asked resolution, disconnecting");
        throw Error(ERR_VNC_OLDER_RDP_CLIENT_CANT_RESIZE);
    }

    this->reader->add_consumer(&this->front, nullptr, nullptr, nullptr, nullptr, nullptr);
    time_t begin_file_read = begin_read.tv_sec+this->in_trans->get_meta_line().start_time - this->balise_time_frame;
    this->in_trans->set_begin_time(begin_file_read);
    this->front.can_be_start_capture();
}

ReplayMod::~ReplayMod()
{
    this->screen.clear();
}

void ReplayMod::add_consumer(
    gdi::GraphicApi * graphic_ptr,
    gdi::CaptureApi * capture_ptr,
    gdi::KbdInputApi * kbd_input_ptr,
    gdi::CaptureProbeApi * capture_probe_ptr,
    gdi::ExternalCaptureApi * external_event_ptr,
    gdi::ResizeApi * resize_ptr
) {
    this->reader->add_consumer(
        graphic_ptr,
        capture_ptr,
        kbd_input_ptr,
        capture_probe_ptr,
        external_event_ptr,
        resize_ptr
    );
}

void ReplayMod::play()
{
    this->reader->play(false);
}

bool ReplayMod::play_client()
{
    return this->reader->play_client();
}

void ReplayMod::set_sync()
{
    this->reader->set_sync();
}

WrmVersion ReplayMod::get_wrm_version()
{
    return this->in_trans->get_wrm_version();
}

bool ReplayMod::get_break_privplay_client()
{
    return this->reader->break_privplay_client;
}

void ReplayMod::instant_play_client(std::chrono::microseconds endin_frame)
{
    this->reader->instant_play_client(endin_frame);
}

void ReplayMod::rdp_input_scancode(
    long /*param1*/, long /*param2*/,
    long /*param3*/, long /*param4*/, Keymap2 * keymap)
{
    if (keymap->nb_kevent_available() > 0
        && keymap->get_kevent() == Keymap2::KEVENT_ESC) {
        this->event.signal = BACK_EVENT_STOP;
        this->event.set_trigger_time(wait_obj::NOW);
    }
}

void ReplayMod::set_pause(timeval & time)
{
    this->reader->set_pause_client(time);
}

void ReplayMod::set_wait_after_load_client(timeval & time)
{
    this->reader->set_wait_after_load_client(time);
}

time_t ReplayMod::get_real_time_movie_begin()
{
    return this->in_trans->get_meta_line().start_time;
}

// event from back end (draw event from remote or internal server)
// returns module continuation status, 0 if module want to continue
// non 0 if it wants to stop (to run another module)
void ReplayMod::draw_event(time_t now, gdi::GraphicApi & drawable)
{
    (void)now;
    (void)drawable;
    // TODO use system constants for sizes
    if (!this->sync_setted) {
        this->sync_setted = true;
        this->reader->set_sync();
    }

    if (this->end_of_data) {
        timespec wtime = {1, 0};
        nanosleep(&wtime, nullptr);
        this->event.set_trigger_time(std::chrono::seconds(1));
        return;
    }

    try
    {
        // unnecessary loop
        for (int i = 0; i < 500; i++) {
            const std::chrono::microseconds elapsed
                = difftimeval(tvtime(), this->reader->start_synctime_now);

            if (elapsed < this->reader->movie_elapsed_client) {
                using std::chrono::duration_cast;
                auto const diff_time = this->reader->movie_elapsed_client - elapsed;
                auto const sec_diff = duration_cast<std::chrono::seconds>(diff_time);
                auto const microsec_diff = diff_time - sec_diff;
                auto const nano_diff = duration_cast<std::chrono::nanoseconds>(microsec_diff);

                timespec wtime = {sec_diff.count(), nano_diff.count()};
                timespec wtime_rem = { 0, 0 };
                while (nanosleep(&wtime, &wtime_rem) == -1 && errno == EINTR) {
                    wtime = wtime_rem;
                }
            }

            if (this->reader->next_order()) {
                this->reader->interpret_order();

            } else {
                if (this->replay_on_loop) {

                    this->in_trans.reset(new InMetaSequenceTransport(
                                            this->cctx,
                                            this->movie_path.prefix,
                                            this->movie_path.extension,
                                            InCryptoTransport::EncryptionMode::NotEncrypted,
                                            this->fstat));

                    timeval const begin_read = {0, 0};
                    time_t begin_file_read = begin_read.tv_sec+this->in_trans->get_meta_line().start_time - this->balise_time_frame;
                    this->in_trans->set_begin_time(begin_file_read);
                    this->sync_setted = false;

                    this->reader.reset(new FileToGraphic(
                                            *this->in_trans,
                                            begin_read, {0, 0},
                                            true,
                                            to_verbose_flags(0)));


                    switch (this->front.server_resize( this->reader->info_width
                                     , this->reader->info_height
                                     , this->reader->info_bpp)) {
                    case FrontAPI::ResizeResult::no_need:
                        // no resizing needed
                        break;
                    case FrontAPI::ResizeResult::instant_done:
                    case FrontAPI::ResizeResult::remoteapp:
                    case FrontAPI::ResizeResult::done:
                        // resizing done;
                        this->front_width  = this->reader->info_width;
                        this->front_height = this->reader->info_height;

                        this->screen.set_wh(this->reader->info_width, this->reader->info_height);

                        break;
                    case FrontAPI::ResizeResult::fail:
                        // resizing failed
                        // thow an Error ?
                        LOG(LOG_WARNING, "Older RDP client can't resize to server asked resolution, disconnecting");
                        throw Error(ERR_VNC_OLDER_RDP_CLIENT_CANT_RESIZE);
                    }

                    this->reader->add_consumer(&this->front, nullptr, nullptr, nullptr, nullptr, nullptr);

                    this->front.can_be_start_capture();

                } else {

                    this->end_of_data = true;
                    this->disconnect(tvtime().tv_sec);
                    this->front.sync();

                    if (!this->wait_for_escape) {
                        this->event.signal = BACK_EVENT_STOP;
                    }

                    break;
                }
            }
        }

        this->event.set_trigger_time(wait_obj::NOW);
    }
    catch (Error const & e) {
        if (e.id == ERR_TRANSPORT_OPEN_FAILED) {
            this->auth_error_message = "The recorded file is inaccessible or corrupted!";

            this->event.signal = BACK_EVENT_NEXT;
            this->event.set_trigger_time(wait_obj::NOW);
        }
        else {
            throw;
        }
    }
}
