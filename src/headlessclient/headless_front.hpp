/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <memory>

#include "core/front_api.hpp"
#include "core/channel_list.hpp"
#include "gdi/graphic_dispatcher.hpp"
#include "headlessclient/headless_command.hpp"


class Inifile;
class TimeBase;
class ClientInfo;
class HeadlessGraphics;
class HeadlessWrmCapture;

struct HeadlessFront final : FrontAPI
{
    struct Recording
    {
        bool png = false;
        bool wrm = false;
    };

    HeadlessFront(TimeBase& time_base, Inifile& ini, ClientInfo& client_info);

    ~HeadlessFront();

    HeadlessCommand& command()
    {
        return cmd_ctx;
    }

    gdi::GraphicApi& prepare_gd();

    char const* dump_png(zstring_view filename, uint16_t mouse_x, uint16_t mouse_y);

    bool can_be_start_capture(SessionLogApi& session_log) override;

    void must_flush_capture() override;

    bool must_be_stop_capture() override;

    bool is_capture_in_progress() const override
    {
        return false;
    }

    ResizeResult server_resize(ScreenInfo screen_server) override;

    const CHANNELS::ChannelDefArray& get_channel_list() const override
    {
        return cl;
    }

    void send_to_channel(
        CHANNELS::ChannelDef const& channel_def, bytes_view chunk_data,
        std::size_t total_data_len, uint32_t flags) override
    {
        (void)channel_def;
        (void)chunk_data;
        (void)total_data_len;
        (void)flags;
    }

    void update_pointer_position(uint16_t x, uint16_t y) override
    {
        cmd_ctx.mouse_x = x;
        cmd_ctx.mouse_y = y;
    }

    void session_update(MonotonicTimePoint now, LogId id, KVLogList kv_list) override
    {
        (void)now;
        (void)id;
        (void)kv_list;
    }

    void possible_active_window_change() override
    {}

    void print_output_resut();
    void print_fail_result();
    void read_config_str();
    void read_config_file();

private:
    HeadlessCommand cmd_ctx;
    CHANNELS::ChannelDefArray cl;
    // capture variable members
    // -----------------
    TimeBase& time_base;
    Inifile& ini;
    ClientInfo& client_info;
    gdi::GraphicDispatcher gds;
    std::unique_ptr<HeadlessGraphics> drawable;
    std::unique_ptr<HeadlessWrmCapture> wrm_gd;
};
