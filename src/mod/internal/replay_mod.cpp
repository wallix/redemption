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
#include "capture/file_to_graphic.hpp"
#include "core/front_api.hpp"
#include "core/events.hpp"
#include "utils/timebase.hpp"
#include "utils/fileutils.hpp"
#include "utils/strutils.hpp"
#include "mod/internal/replay_mod.hpp"
#include "transport/in_multi_crypto_transport.hpp"
#include "transport/mwrm_file_data.hpp"


struct ReplayMod::Reader
{
    CryptoContext cctx;

    MonotonicTimePoint start_time_replay;

    InMultiCryptoTransport in_trans;
    FileToGraphic reader;

    Reader(
        gdi::GraphicApi & drawable,
        std::string const& mwrm_filename,
        bool play_video_with_corrupted_bitmap,
        Verbose debug_capture)
    // TODO RZ: Support encrypted recorded file.
    : in_trans(
        [&]{
            std::vector<std::string> filenames;
            MwrmFileData mwrm_data = load_mwrm_file_data(
                mwrm_filename.c_str(),
                this->cctx,
                InCryptoTransport::EncryptionMode::NotEncrypted);
            filenames.reserve(mwrm_data.wrms.size());
            for (auto const& wrm : mwrm_data.wrms) {
                if (file_exist(wrm.filename)) {
                    filenames.emplace_back(wrm.filename);
                    LOG(LOG_INFO, "ReplayMod::Reader: Found %s", filenames.back());
                }
                else {
                    auto pos = mwrm_filename.find_last_of('/');
                    if (pos != std::string::npos) {
                        filenames.emplace_back(str_concat(
                            chars_view(mwrm_filename).first(pos + 1),
                            wrm.filename));
                        LOG(LOG_INFO, "ReplayMod::Reader: Found %s -> %s",
                            wrm.filename, filenames.back());
                    }
                    else {
                        filenames.emplace_back(wrm.filename);
                        LOG(LOG_ERR, "ReplayMod::Reader: Not found %s", filenames.back());
                    }
                }
            }
            return filenames;
        }(),
        this->cctx,
        InCryptoTransport::EncryptionMode::NotEncrypted)
    , reader(this->in_trans, play_video_with_corrupted_bitmap, debug_capture)
    {
        this->start_time_replay = this->reader.get_monotonic_time();

        this->reader.add_consumer(
            &drawable, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr);
    }

    MonotonicTimePoint::duration current_duration() const
    {
        return reader.get_monotonic_time() - start_time_replay;
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
, replay_path(std::move(replay_path))
, debug_capture(debug_capture)
, wait_for_escape(wait_for_escape)
, replay_on_loop(replay_on_loop)
, play_video_with_corrupted_bitmap(play_video_with_corrupted_bitmap)
, timer_event(events.event_creator().create_event_timeout(
    "replay", this, events.get_monotonic_time(),
    [this](Event& ev){
        if (this->next_timestamp()) {
            const auto duration = this->internal_reader->current_duration();
            ev.alarm.reset_timeout(this->start_time + duration);
        }
        else if (this->replay_on_loop) {
            this->init_reader();
            ev.alarm.reset_timeout(this->start_time);
        }
        else if (!this->wait_for_escape) {
            ev.garbage = true;
            this->set_mod_signal(BACK_EVENT_STOP);
        }
    }))
, time_base_ref(events.get_time_base())
{
    this->init_reader();
}

ReplayMod::~ReplayMod() = default;

bool ReplayMod::next_timestamp()
{
    auto& reader = this->internal_reader->reader;
    auto previous = reader.get_monotonic_time();
    bool has_order;

    try {
        while ((has_order = reader.next_order())) {
            reader.interpret_order();
            if (previous != reader.get_monotonic_time()) {
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
    LOG(LOG_INFO, "Playing %s", this->replay_path);

    this->internal_reader = std::make_unique<Reader>(
        this->drawable,
        this->replay_path,
        this->play_video_with_corrupted_bitmap,
        this->debug_capture);

    this->start_time = this->time_base_ref.monotonic_time;

    auto& info = this->internal_reader->reader.get_wrm_info();
    switch (this->front.server_resize({info.width, info.height, info.bpp})) {
        case FrontAPI::ResizeResult::remoteapp_done:
        case FrontAPI::ResizeResult::done:
        case FrontAPI::ResizeResult::no_need:
        case FrontAPI::ResizeResult::instant_done:
        case FrontAPI::ResizeResult::remoteapp:
            break;

        case FrontAPI::ResizeResult::fail:
            LOG(LOG_ERR, "Older RDP client can't resize to server asked resolution, disconnecting");
            throw Error(ERR_RDP_RESIZE_NOT_AVAILABLE);
    }
}

void ReplayMod::rdp_gdi_up_and_running()
{
    if (auto* timer = this->timer_event.get_optional_event()) {
        LOG(LOG_DEBUG, "ReplayMod::rdp_gdi_up_and_running()");
        auto no_gdi_delay = this->time_base_ref.monotonic_time - this->gdi_down_time;
        this->start_time += no_gdi_delay;

        const auto duration = this->internal_reader->current_duration();
        timer->alarm.reset_timeout(this->start_time + duration);
    }
}

void ReplayMod::rdp_gdi_down()
{
    if (auto* timer = this->timer_event.get_optional_event()) {
        LOG(LOG_DEBUG, "ReplayMod::rdp_gdi_down()");
        timer->alarm.pause();
        this->gdi_down_time = this->time_base_ref.monotonic_time;
    }
}

void ReplayMod::rdp_input_scancode(
    KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap)
{
    (void)event_time;
    (void)keymap;
    if (pressed_scancode(flags, scancode) == Scancode::Esc) {
        this->set_mod_signal(BACK_EVENT_STOP);
    }
}

Dimension ReplayMod::get_dim() const
{
    auto const& info = this->internal_reader->reader.get_wrm_info();
    return Dimension(info.width , info.height);
}
