/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2016
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef REDEMPTION_MOD_RDP_CHANNELS_SESPROALTERNATESHELLBASEDLAUNCHER_HPP
#define REDEMPTION_MOD_RDP_CHANNELS_SESPROALTERNATESHELLBASEDLAUNCHER_HPP

#include "mod/rdp/channels/sespro_channel.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"

class SessionProbeAlternateShellBasedLauncher : public SessionProbeLauncher {
private:
    SessionProbeVirtualChannel* channel = nullptr;

    bool stopped = false;

    uint32_t verbose;

public:
    SessionProbeAlternateShellBasedLauncher(uint32_t verbose)
    : verbose(verbose) {}

    wait_obj* get_event() override { return nullptr; }

    bool on_clipboard_initialize() override { return false; }

    bool on_clipboard_monitor_ready() override { return false; }

    bool on_drive_access() override {
        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_LAUNCHER) {
            LOG(LOG_INFO,
                "SessionProbeAlternateShellBasedLauncher :=> on_drive_access");
        }

        if (this->stopped) {
            return false;
        }

        if (this->channel) {
            this->channel->give_additional_launch_time();
        }

        return false;
    }

    bool on_event() override { return false; }

    bool on_image_read(uint64_t offset, uint32_t length) override {
        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_LAUNCHER) {
            LOG(LOG_INFO,
                "SessionProbeAlternateShellBasedLauncher :=> on_image_read");
        }

        if (this->stopped) {
            return false;
        }

        if (this->channel) {
            this->channel->give_additional_launch_time();
        }

        return false;
    }

    bool on_server_format_data_request() override { return false; }

    bool on_server_format_list_response() override { return false; }

    // Returns false to prevent message to be sent to server.
    bool process_client_cliprdr_message(InStream & chunk,
        uint32_t length, uint32_t flags) override { return true; }

    void set_clipboard_virtual_channel(
            BaseVirtualChannel* channel) override {
    }

    void set_session_probe_virtual_channel(
        BaseVirtualChannel* channel) override {
        this->channel = static_cast<SessionProbeVirtualChannel*>(channel);
    }

    void stop() override {
        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_LAUNCHER) {
            LOG(LOG_INFO,
                "SessionProbeAlternateShellBasedLauncher :=> stop");
        }

        this->stopped = true;
    }
};

#endif  // #endif REDEMPTION_MOD_RDP_CHANNELS_SESPROALTERNATESHELLBASEDLAUNCHER_HPP
