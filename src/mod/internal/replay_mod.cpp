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

#include "capture/cryptofile.hpp"
#include "core/app_path.hpp"
#include "core/front_api.hpp"
#include "core/session_reactor.hpp"
#include "keyboard/keymap2.hpp"
#include "mod/internal/replay_mod.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "transport/mwrm_reader.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/genfstat.hpp"

struct ReplayMod::Reader
{
    struct Path
    {
        char extension[128] {};
        char prefix[4096] {};

        //TODO: should be generalized to some wide use FilePath object
        // with basename, path, ext, etc. methods and use it for passing
        // around all of redemption pathes.
        explicit Path(const char * replay_path)
        {
            LOG(LOG_INFO, "Playing %s", replay_path);

            char path[1024];
            char basename[1024];
            utils::strlcpy(path, app_path(AppPath::Record)); // default value, actual one should come from movie_path
            utils::strlcpy(basename, "replay"); // default value actual one should come from movie_path
            utils::strlcpy(this->extension, ".mwrm"); // extension is currently ignored

            const bool res = canonical_path(
                replay_path,
                path, sizeof(path),
                basename, sizeof(basename),
                this->extension, sizeof(this->extension)
            );

            if (!res) {
                LOG(LOG_ERR, "Buffer Overflowed: Path too long");
                throw Error(ERR_RECORDER_FAILED_TO_FOUND_PATH);
            }

            std::snprintf(this->prefix, sizeof(this->prefix), "%s%s", path, basename);
        }
    } movie_path;
    CryptoContext cctx;
    Fstat         fstat;

    InMetaSequenceTransport in_trans;
    FileToGraphic reader;
    time_t balise_time_frame;

    Verbose debug_capture;

    Reader(
        Path path, timeval const& begin_read, timeval const& end_read,
        time_t balise_time_frame, bool play_video_with_corrupted_bitmap, Verbose debug_capture)
    : movie_path(path)
    // TODO RZ: Support encrypted recorded file.
    , in_trans(
        this->cctx,
        this->movie_path.prefix,
        this->movie_path.extension,
        InCryptoTransport::EncryptionMode::NotEncrypted,
        this->fstat)
    , reader(
        this->in_trans,
        begin_read,
        end_read,
        true,
        play_video_with_corrupted_bitmap,
        debug_capture)
    , balise_time_frame(balise_time_frame)
    , debug_capture(debug_capture)
    {
        time_t begin_file_read = begin_read.tv_sec + this->in_trans.get_meta_line().start_time - this->balise_time_frame;
        this->in_trans.set_begin_time(begin_file_read);
    }

    REDEMPTION_CXX_NODISCARD
    bool server_resize(FrontAPI& front)
    {
        bool is_resized = false;
        switch (front.server_resize(
            this->reader.info.width , this->reader.info.height , this->reader.info.bpp)
        ) {
            case FrontAPI::ResizeResult::no_need:
                // no resizing needed
                break;
            case FrontAPI::ResizeResult::instant_done:
            case FrontAPI::ResizeResult::remoteapp:
            case FrontAPI::ResizeResult::done:
                // resizing done;
                is_resized = true;
                break;
            case FrontAPI::ResizeResult::fail:
                // resizing failed
                // thow an Error ?
                LOG(LOG_WARNING, "Older RDP client can't resize to server asked resolution, disconnecting");
                throw Error(ERR_RDP_RESIZE_NOT_AVAILABLE);
        }

        this->reader.add_consumer(&front, nullptr, nullptr, nullptr, nullptr, nullptr);
        front.can_be_start_capture();
        return is_resized;
    }
};

ReplayMod::ReplayMod(
    SessionReactor& session_reactor
  , FrontAPI & front
  , const char * replay_path
  , uint16_t width
  , uint16_t height
  , std::string & auth_error_message
  , bool wait_for_escape
  , timeval const & begin_read
  , timeval const & end_read
  , time_t balise_time_frame
  , bool replay_on_loop
  , bool play_video_with_corrupted_bitmap
  , Verbose debug_capture)
: auth_error_message(auth_error_message)
, front_width(width)
, front_height(height)
, front(front)
, internal_reader(std::make_unique<Reader>(
    Reader::Path{replay_path}, begin_read, end_read, balise_time_frame, play_video_with_corrupted_bitmap, debug_capture))
, end_of_data(false)
, wait_for_escape(wait_for_escape)
, sync_setted(false)
, replay_on_loop(replay_on_loop)
, play_video_with_corrupted_bitmap(play_video_with_corrupted_bitmap)
, session_reactor(session_reactor)
{
    if (this->internal_reader->server_resize(front)) {
        this->front_width  = this->internal_reader->reader.info.width;
        this->front_height = this->internal_reader->reader.info.height;
    }

    this->timer = session_reactor.create_graphic_timer()
    .set_delay(std::chrono::seconds(0))
    .on_action([this](auto ctx, gdi::GraphicApi& gd){
        this->draw_event(0, gd);
        return ctx.ready_at(ctx.get_current_time());
    });
}


ReplayMod::~ReplayMod() = default;

void ReplayMod::add_consumer(
    gdi::GraphicApi * graphic_ptr,
    gdi::CaptureApi * capture_ptr,
    gdi::KbdInputApi * kbd_input_ptr,
    gdi::CaptureProbeApi * capture_probe_ptr,
    gdi::ExternalCaptureApi * external_event_ptr,
    gdi::ResizeApi * resize_ptr
) {
    this->internal_reader->reader.add_consumer(
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
    this->internal_reader->reader.play(false);
}

bool ReplayMod::play_client()
{
    return this->internal_reader->reader.play_client();
}

void ReplayMod::set_sync()
{
    this->internal_reader->reader.set_sync();
}

WrmVersion ReplayMod::get_wrm_version() const
{
    return this->internal_reader->in_trans.get_wrm_version();
}

bool ReplayMod::get_break_privplay_client()
{
    return this->internal_reader->reader.break_privplay_client;
}

void ReplayMod::instant_play_client(std::chrono::microseconds endin_frame)
{
    this->internal_reader->reader.instant_play_client(endin_frame);
}

void ReplayMod::rdp_input_scancode(
    long /*param1*/, long /*param2*/,
    long /*param3*/, long /*param4*/, Keymap2 * keymap)
{
    if (keymap->nb_kevent_available() > 0
        && keymap->get_kevent() == Keymap2::KEVENT_ESC) {
        this->session_reactor.set_next_event(BACK_EVENT_STOP);
    }
}

Dimension ReplayMod::get_dim() const {
     Dimension dim(this->front_width , this->front_height);

     return dim;
}

void ReplayMod::set_pause(timeval & time)
{
    this->internal_reader->reader.set_pause_client(time);
}

void ReplayMod::set_wait_after_load_client(timeval & time)
{
    this->internal_reader->reader.set_wait_after_load_client(time);
}

time_t ReplayMod::get_real_time_movie_begin()
{
    return this->internal_reader->in_trans.get_meta_line().start_time;
}

void ReplayMod::draw_event(time_t /*now*/, gdi::GraphicApi & /*gd*/)
{
    // TODO use system constants for sizes
    if (!this->sync_setted) {
        this->sync_setted = true;
        this->internal_reader->reader.set_sync();
    }

    if (this->end_of_data) {
        timespec wtime = {1, 0};
        nanosleep(&wtime, nullptr);
        return;
    }

    try
    {
        // unnecessary loop
        for (int i = 0; i < 500; i++) {
            const std::chrono::microseconds elapsed
                = difftimeval(tvtime(), this->internal_reader->reader.start_synctime_now);

            if (elapsed < this->internal_reader->reader.movie_elapsed_client) {
                using std::chrono::duration_cast;
                auto const diff_time = this->internal_reader->reader.movie_elapsed_client - elapsed;
                auto const sec_diff = duration_cast<std::chrono::seconds>(diff_time);
                auto const microsec_diff = diff_time - sec_diff;
                auto const nano_diff = duration_cast<std::chrono::nanoseconds>(microsec_diff);

                timespec wtime = {sec_diff.count(), nano_diff.count()};
                timespec wtime_rem = { 0, 0 };
                while (nanosleep(&wtime, &wtime_rem) == -1 && errno == EINTR) {
                    wtime = wtime_rem;
                }
            }

            if (this->internal_reader->reader.next_order()) {
                this->internal_reader->reader.interpret_order();
            }
            else {
                if (this->replay_on_loop) {
                    timeval const begin_read = {0, 0};
                    timeval const end_read = {0, 0};
                    this->internal_reader = std::make_unique<Reader>(
                        this->internal_reader->movie_path,
                        begin_read, end_read,
                        this->internal_reader->balise_time_frame,
                        this->play_video_with_corrupted_bitmap,
                        this->internal_reader->debug_capture);

                    if (this->internal_reader->server_resize(this->front)) {
                        this->front_width  = this->internal_reader->reader.info.width;
                        this->front_height = this->internal_reader->reader.info.height;
                    }

                    this->sync_setted = false;
                }
                else {
                    this->end_of_data = true;
                    this->timer->set_delay(std::chrono::seconds(1));

                    this->disconnect(tvtime().tv_sec);
                    this->front.sync();

                    if (!this->wait_for_escape) {
                        this->session_reactor.set_next_event(BACK_EVENT_STOP);
                    }

                    break;
                }
            }
        }
    }
    catch (Error const & e) {
        if (e.id == ERR_TRANSPORT_OPEN_FAILED) {
            this->auth_error_message = "The recorded file is inaccessible or corrupted!";
            this->session_reactor.set_next_event(BACK_EVENT_NEXT);
        }
        else {
            throw;
        }
    }
}

std::string ReplayMod::get_mwrm_path() const
{
    return str_concat(this->internal_reader->movie_path.prefix, ".mwrm");
}
