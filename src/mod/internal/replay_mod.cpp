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
Copyright (C) Wallix 2010-2020
Author(s): Wallix Team
*/

#include "capture/cryptofile.hpp"
#include "core/front_api.hpp"
#include "core/events.hpp"
#include "utils/timebase.hpp"
#include "keyboard/keymap2.hpp"
#include "mod/internal/replay_mod.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "utils/genfstat.hpp"


struct ReplayMod::Reader
{
    CryptoContext cctx;
    Fstat         fstat;

    InMetaSequenceTransport in_trans;
    FileToGraphic reader;

    Reader(
        char const* prefix,
        char const* extension,
        bool play_video_with_corrupted_bitmap,
        Verbose debug_capture)
    // TODO RZ: Support encrypted recorded file.
    : in_trans(
        this->cctx,
        prefix,
        extension,
        InCryptoTransport::EncryptionMode::NotEncrypted,
        this->fstat)
    , reader(
        this->in_trans,
        timeval{0, 0},
        timeval{0, 0},
        play_video_with_corrupted_bitmap,
        debug_capture)
    {}

    void server_resize(gdi::GraphicApi & drawable, FrontAPI & front)
    {
        auto& info = this->reader.get_wrm_info();
        switch (front.server_resize({info.width , info.height , info.bpp})) {
            case FrontAPI::ResizeResult::no_need:
            case FrontAPI::ResizeResult::instant_done:
            case FrontAPI::ResizeResult::remoteapp:
            case FrontAPI::ResizeResult::done:
                break;
            case FrontAPI::ResizeResult::fail:
                // resizing failed
                LOG(LOG_WARNING, "Older RDP client can't resize to server asked resolution, disconnecting");
                throw Error(ERR_RDP_RESIZE_NOT_AVAILABLE);
        }

        this->reader.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    }
};

ReplayMod::ReplayMod(
    EventContainer & events,
    gdi::GraphicApi & drawable,
    FrontAPI & front,
    std::string replay_path,
    std::string & auth_error_message,
    bool wait_for_escape,
    bool replay_on_loop,
    bool play_video_with_corrupted_bitmap,
    Verbose debug_capture)
: auth_error_message(auth_error_message)
, drawable(drawable)
, front(front)
, prefix_path([&]() -> std::string&& {
    auto pos = replay_path.find_last_of('.');
    if (pos != std::string::npos) {
        replay_path.resize(pos);
    }
    return std::move(replay_path);
}())
, debug_capture(debug_capture)
, wait_for_escape(wait_for_escape)
, replay_on_loop(replay_on_loop)
, play_video_with_corrupted_bitmap(play_video_with_corrupted_bitmap)
, events_guards(events)
{
    this->init_reader();

    auto action = [this](Event& ev){
        if (this->next_timestamp()) {
            const auto now = this->events_guards.get_current_time();

            const auto& reader = this->internal_reader->reader;
            const auto replay_delay = reader.get_current_time() - this->start_time_replay;
            const auto real_delay = now - this->start_time;

            const auto next_time = (replay_delay <= real_delay)
                ? now
                : now + (replay_delay - real_delay);
            ev.alarm.set_timeout(next_time);
        }
        else if (this->replay_on_loop) {
            this->init_reader();
            ev.alarm.set_timeout(this->start_time);
        }
        else if (!this->wait_for_escape) {
            ev.garbage = true;
            this->set_mod_signal(BACK_EVENT_STOP);
            // throw Error(ERR_BACK_EVENT_NEXT);
        }
    };

    this->events_guards.create_event_timeout("replay", this->events_guards.get_current_time(), action);
}

ReplayMod::~ReplayMod() = default;

bool ReplayMod::next_timestamp()
{
    auto& reader = this->internal_reader->reader;
    auto previous = reader.get_current_time();
    bool has_order;

    try {
        while ((has_order = reader.next_order())) {
            reader.interpret_order();
            if (previous != reader.get_current_time()) {
                break;
            }
        }
    }
    catch (Error const& e) {
        if (e.id == ERR_TRANSPORT_OPEN_FAILED) {
            this->auth_error_message = "The recorded file is inaccessible or corrupted!";
            this->set_mod_signal(BACK_EVENT_NEXT);
            // throw Error(ERR_BACK_EVENT_NEXT);
            has_order = false;
        }
        else {
            throw;
        }
    }

    return has_order;
}

void ReplayMod::init_reader()
{
    LOG(LOG_INFO, "Playing %s.mwrm", this->prefix_path);

    this->internal_reader = std::make_unique<Reader>(
        this->prefix_path.c_str(),
        ".mwrm",
        this->play_video_with_corrupted_bitmap,
        this->debug_capture);
    this->start_time = this->events_guards.get_current_time();
    this->start_time_replay = this->internal_reader->reader.get_current_time();
    this->internal_reader->server_resize(this->drawable, this->front);
}

void ReplayMod::rdp_input_scancode(
    long /*param1*/, long /*param2*/,
    long /*param3*/, long /*param4*/, Keymap2 * keymap)
{
    if (keymap->nb_kevent_available() > 0
     && keymap->get_kevent() == Keymap2::KEVENT_ESC
    ) {
        this->set_mod_signal(BACK_EVENT_STOP);
    }
}

Dimension ReplayMod::get_dim() const
{
    auto const& info = this->internal_reader->reader.get_wrm_info();
    return Dimension(info.width , info.height);
}
