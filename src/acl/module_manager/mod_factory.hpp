/*
SPDX-FileCopyrightText: 2022 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "acl/mod_wrapper.hpp"
#include "utils/redirection_info.hpp"
#include "utils/theme.hpp"
#include "acl/file_system_license_store.hpp"
#include "RAIL/client_execute.hpp"
#include "configs/autogen/enums.hpp"
#include "acl/module_manager/create_module_rdp.hpp"


class SocketTransport;


class ModFactory
{
public:
    ModFactory(EventContainer & events,
               CRef<TimeBase> time_base,
               CRef<ClientInfo> client_info,
               FrontAPI & front,
               gdi::GraphicApi & graphics,
               CRef<BGRPalette> palette,
               CRef<Font> glyphs,
               Inifile & ini,
               Keymap & keymap,
               Random & gen,
               CryptoContext & cctx
    );

    ~ModFactory();

    RedirectionInfo& get_redir_info() noexcept
    {
        return redir_info;
    }

    ModuleName mod_name() const noexcept
    {
        return current_mod;
    }

    mod_api& mod() noexcept
    {
        return mod_wrapper.get_mod();
    }

    mod_api const& mod() const noexcept
    {
        return mod_wrapper.get_mod();
    }

    SocketTransport* mod_sck_transport() const noexcept
    {
        return psocket_transport;
    }

    Callback& callback() noexcept
    {
        return mod_wrapper.get_callback();
    }

    void set_enable_osd_display_remote_target(bool enable) noexcept
    {
        mod_wrapper.set_enable_osd_display_remote_target(enable);
    }

    void display_osd_message(std::string_view message)
    {
        return mod_wrapper.display_osd_message(message);
    }

    void set_time_close(MonotonicTimePoint t)
    {
        mod_wrapper.set_time_close(t);
    }

    bool is_connected() const noexcept
    {
        return this->psocket_transport;
    }

    void disconnect();

    void create_mod_bouncer();

    void create_mod_replay();

    void create_widget_test_mod();

    void create_test_card_mod();

    void create_selector_mod();

    void create_close_mod();

    void create_close_mod_back_to_selector();

    void create_interactive_target_mod();

    void create_valid_message_mod();

    void create_display_message_mod();

    void create_dialog_challenge_mod();

    void create_display_link_mod();

    void create_wait_info_mod();

    void create_transition_mod();

    void create_login_mod();

    void create_rdp_mod(
        SessionLogApi& session_log,
        PerformAutomaticReconnection perform_automatic_reconnection
    );

    void create_vnc_mod(SessionLogApi& session_log);

private:
    class D;
    friend class D;

    SocketTransport * psocket_transport = nullptr;
    ModuleName current_mod = ModuleName::UNKNOWN;

    EventContainer & events;
    ClientInfo const& client_info;
    FrontAPI & front;
    gdi::GraphicApi & graphics;
    Inifile & ini;
    Font const & glyphs;
    Keymap & keymap;
    Random & gen;
    CryptoContext & cctx;
    std::array<uint8_t, 28> server_auto_reconnect_packet {};
    FileSystemLicenseStore file_system_license_store;

    Theme theme;
    ClientExecute rail_client_execute;
    ModWrapper mod_wrapper;
    RedirectionInfo redir_info;
};
