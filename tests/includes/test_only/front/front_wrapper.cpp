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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "test_only/front/front_wrapper.hpp"
#include "front/front.hpp"


struct FrontWrapper::D
{
    struct MyFront : Front
    {
        friend class FrontWrapper;
        using Front::Front;
    };

    MyFront front;
};

const CHANNELS::ChannelDefArray & FrontWrapper::get_channel_list() const
{
    return d->front.get_channel_list();
}

void FrontWrapper::send_to_channel(
    const CHANNELS::ChannelDef & channel_def, bytes_view chunk_data,
    std::size_t total_length, int flags)
{
    return d->front.send_to_channel(channel_def, chunk_data, total_length, flags);
}

FrontWrapper::ResizeResult FrontWrapper::server_resize(ScreenInfo screen_server)
{
    return d->front.server_resize(screen_server);
}

void FrontWrapper::update_pointer_position(uint16_t x, uint16_t y)
{
    d->front.update_pointer_position(x, y);
}

void FrontWrapper::set_keyboard_indicators(uint16_t LedFlags)
{
    d->front.set_keyboard_indicators(LedFlags);
}

void FrontWrapper::session_probe_started(bool enable)
{
    d->front.session_probe_started(enable);
}

void FrontWrapper::set_keylayout(int LCID)
{
    d->front.set_keylayout(LCID);
}

void FrontWrapper::set_focus_on_password_textbox(bool set)
{
    d->front.set_focus_on_password_textbox(set);
}

void FrontWrapper::set_focus_on_unidentified_input_field(bool set)
{
    d->front.set_focus_on_unidentified_input_field(set);
}

void FrontWrapper::set_consent_ui_visible(bool set)
{
    d->front.set_consent_ui_visible(set);
}

void FrontWrapper::session_update(LogId id, KVList kv_list)
{
    d->front.session_update(id, kv_list);
}

void FrontWrapper::send_savesessioninfo()
{
    d->front.send_savesessioninfo();
}

int FrontWrapper::get_keylayout() const
{
    return d->front.get_keylayout();
}

bool FrontWrapper::is_up_and_running() const
{
    return d->front.state == Front::UP_AND_RUNNING;
}

void FrontWrapper::set_up_and_running()
{
    d->front.state = Front::UP_AND_RUNNING;
}

void FrontWrapper::incoming(Callback & cb)
{
    d->front.rbuf.load_data(d->front.trans);
    while (d->front.rbuf.next(TpduBuffer::PDU))
    {
        bytes_view tpdu = d->front.rbuf.current_pdu_buffer();
        uint8_t current_pdu_type = d->front.rbuf.current_pdu_get_type();
        d->front.incoming(tpdu, current_pdu_type, cb);
    }
}

void FrontWrapper::set_ignore_rdesktop_bogus_clip(bool set)
{
    d->front.ignore_rdesktop_bogus_clip = set;
}

ScreenInfo const& FrontWrapper::screen_info() const
{
    return d->front.client_info.screen_info;
}


gdi::GraphicApi& FrontWrapper::gd() noexcept
{
    return d->front;
}

CHANNELS::ChannelDefArray & FrontWrapper::get_mutable_channel_list()
{
    return d->front.channel_list;
}

FrontWrapper::FrontWrapper(
    SessionReactor& session_reactor,
    Transport & trans,
    Random & gen,
    Inifile & ini,
    CryptoContext & cctx,
    ReportMessageApi & report_message,
    bool fp_support, // If true, fast-path must be supported
    std::string server_capabilities_filename)
: d(new D{FrontWrapper::D::MyFront{
    session_reactor,
    trans, gen, ini, cctx, report_message,
    fp_support, std::move(server_capabilities_filename)
}})
{}

FrontWrapper::~FrontWrapper() = default;
