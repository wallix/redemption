/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "core/events.hpp"
#include "core/front_api.hpp"
#include "core/callback.hpp"
#include "gdi/graphic_dispatcher.hpp"
#include "headlessclient/headless_wrm_capture.hpp"
#include "headlessclient/headless_graphics.hpp"
#include "headlessclient/headless_command.hpp"
#include "headlessclient/headless_path.hpp"
#include "headlessclient/input_collector.hpp"
#include "utils/uninit_buffer.hpp"
#include "utils/basic_notifier_function.hpp"


struct HeadlessRepl final : FrontAPI, SessionLogApi, private RdpInput
{
    struct SessionEventGuard : noncopyable
    {
        SessionEventGuard(HeadlessRepl& repl, RdpInput& mod);

        ~SessionEventGuard();

        bool execute_timer(Event& e, InputCollector& input_collector, Keymap& keymap);

    private:
        friend class HeadlessRepl;

        HeadlessRepl& repl;

        EventRef repetition_cmd_event{};
        EventRef ipng_event{};
    };

    HeadlessRepl(chars_view home);

    bool execute_delayed_command(RdpInput& mod)
    {
        return execute_command(mod, delayed_cmd);
    }

    bool execute_command(RdpInput& mod, chars_view cmd_line);

    gdi::GraphicApi& prepare_gd();

    ImageView get_image_view();


    /*
     * SessionLogApi
     */

    void log6(LogId id, KVLogList kv_list) override;

    void report(const char * reason, const char * message) override;

    void set_control_owner_ctx(chars_view name) override
    {
        (void)name;
    }


    /*
     * FrontApi
     */

    bool can_be_start_capture(SessionLogApi& session_log) override;

    void must_flush_capture() override;

    bool must_be_stop_capture() override;

    bool is_capture_in_progress() const override
    {
        return false;
    }

    ResizeResult server_resize(ScreenInfo screen_server) override;

    CHANNELS::ChannelDefArrayView get_channel_list() const override
    {
        return CHANNELS::ChannelDefArrayView();
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

private:
    template<class MakePath>
    void screenshot(MakePath make_path);

    bool make_ipng_capture_event();

    void make_repetition_command_event(RdpInput& mod);


    /*
     * RdpInput
     */

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override
    {
        input_mod->rdp_input_scancode(flags, scancode, event_time, keymap);
    }

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override
    {
        input_mod->rdp_input_unicode(flag, unicode);
    }

    void rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y) override
    {
        x = std::min(x, client_info.screen_info.width);
        y = std::min(y, client_info.screen_info.height);
        cmd_ctx.mouse_x = x;
        cmd_ctx.mouse_x = y;
        input_mod->rdp_input_mouse(device_flags, x, y);
    }

    void rdp_input_mouse_ex(uint16_t device_flags, uint16_t x, uint16_t y) override
    {
        x = std::min(x, client_info.screen_info.width);
        y = std::min(y, client_info.screen_info.height);
        cmd_ctx.mouse_x = x;
        cmd_ctx.mouse_x = y;
        input_mod->rdp_input_mouse_ex(device_flags, x, y);
    }

    void rdp_input_synchronize(KeyLocks locks) override
    {
        input_mod->rdp_input_synchronize(locks);
    }

    void rdp_input_invalidate(Rect /*r*/) override
    {}

    void rdp_gdi_up_and_running() override
    {}

    void rdp_gdi_down() override
    {}

public:
    bool quit = false;
    bool start_connection = false;
    bool disconnection = false;

private:
    bool has_delay_cmd = false;
    bool first_png = true;
    bool gd_is_ready = false;

public:
    bool enable_wrm = false;
    bool enable_png = false;
    bool enable_record_transport = false;
private:
    bool is_regular_png_path = false;

    MonotonicTimePoint::duration cmd_delay;
    MonotonicTimePoint::duration key_delay;
    MonotonicTimePoint::duration mouse_delay;
    MonotonicTimePoint::duration sleep_delay;
    MonotonicTimePoint::duration ipng_delay;
    std::string delayed_cmd;

    struct Counters
    {
        unsigned wrm = 0;
        unsigned png = 0;
        unsigned ipng = 0;
        unsigned total = 0;
    };

    Counters counters;

public:
    std::string png_path;
    std::string ipng_suffix;
    std::string wrm_path;
    std::string record_transport_path;
    std::string ip_address;
    std::string username;
    std::string password;
private:
    std::string home_variable;

    HeadlessPath prefix_path;
    HeadlessPath screen_repetition_prefix_path;

public:
    ClientInfo client_info {};
    Inifile ini;
    EventManager event_manager;

    HeadlessCommand cmd_ctx;

    enum class PathType : uint8_t
    {
        Png,
        Wrm,
        RecorderTransport,
    };

    BasicNotifierFunction<PathType, zstring_view> path_notifier;

private:
    SessionEventGuard* session_event = nullptr;

    RdpInput* input_mod;

    UninitDynamicBuffer buffer;
    // capture variable members
    // -----------------
    gdi::GraphicDispatcher gds;
    std::unique_ptr<HeadlessGraphics> drawable;
    std::unique_ptr<HeadlessWrmCapture> wrm_gd;
};
