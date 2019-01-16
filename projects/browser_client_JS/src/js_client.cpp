/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2015
 *
 * free RDP client main program
 *
 */

#include "acl/auth_api.hpp"
#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "core/set_server_redirection_target.hpp"
#include "front/client_front.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
// #include "transport/socket_transport.hpp"
#include "utils/fixed_random.hpp"
#include "utils/genrandom.hpp"
#include "utils/redirection_info.hpp"
#include "utils/hexdump.hpp"
#include "utils/theme.hpp"

// #include <iostream>
#include <memory>
#include <string>

#include <emscripten/bind.h>


constexpr int FD_TRANS = 42;

struct BrowserTransport : Transport
{
    std::vector<std::string> in_buffers;
    std::vector<uint8_t> out_buffers;
    std::size_t current_pos = 0;

    TlsResult enable_client_tls(
        bool /*server_cert_store*/,
        ServerCertCheck /*server_cert_check*/,
        ServerNotifier & /*server_notifier*/,
        const char * /*certif_path*/
    ) override
    {
        LOG(LOG_INFO, "enable_client_tls");
        return TlsResult::Ok;
    }

    size_t do_partial_read(uint8_t * data, size_t len) override
    {
        if (in_buffers.empty()) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }

        auto remaining = len;

        while (len) {
            auto& s = in_buffers.front();
            auto s_len = s.size() - current_pos;
            if (s_len <= remaining) {
                memcpy(data, s.data() + current_pos, s_len);
                remaining -= s_len;
                data += s_len;
                current_pos += s_len;
                if (s_len != remaining) {
                    current_pos = 0;
                    in_buffers.erase(in_buffers.begin());
                }
            }

            if (in_buffers.empty()) {
                break;
            }
        }

        size_t const data_len = len - remaining;

        LOG(LOG_DEBUG, "BrowserTransport::read %zu bytes", data_len);

        // hexdump(data - data_len, data_len);

        return data_len;
    }

    void do_send(const uint8_t * buffer, size_t len) override
    {
        LOG(LOG_DEBUG, "BrowserTransport::send %zu bytes", len);
        // hexdump(buffer, len);
        out_buffers.insert(out_buffers.end(), buffer, buffer + len);
    }

    int get_fd() const override { return FD_TRANS; }
};

struct RdpClient
{
    std::unique_ptr<mod_api> mod;

    uint64_t verbose = 0xfffff;
    std::string target_device = "10.10.46.73";

    std::string screen_output;

    std::string username;
    std::string password;

    BrowserTransport browser_trans;

    ClientInfo client_info;

    ClientFront front = ClientFront(client_info.screen_info, verbose);
    NullReportMessage report_message;
    SessionReactor session_reactor;
    TimeSystem system_timeobj;

    Inifile ini;

    FixedRandom lcg_gen;
    LCGTime lcg_timeobj;
    NullAuthentifier authentifier;
    RedirectionInfo redir_info;

    std::array<unsigned char, 28> server_auto_reconnect_packet;
    std::string close_box_extra_message;
    Theme theme;
    Font font;


    RdpClient(unsigned long verbose)
    : verbose(verbose)
    {
        // TODO extra username/password from GET param

        client_info.screen_info.width = 800;
        client_info.screen_info.height = 600;
        client_info.screen_info.bpp = BitsPerPixel{24};

        ini.set<cfg::mod_rdp::server_redirection_support>(false);
        ini.set<cfg::mod_rdp::enable_nla>(false);
        ini.set<cfg::client::tls_fallback_legacy>(true);

        new_mod();
    }

    void new_mod()
    {
        ModRDPParams mod_rdp_params(
            username.c_str(),
            password.c_str(),
            target_device.c_str(),
            "0.0.0.0", // client ip is silenced
            /*front.keymap.key_flags*/ 0,
            font,
            theme,
            server_auto_reconnect_packet,
            close_box_extra_message,
            to_verbose_flags(verbose)
        );

        mod_rdp_params.device_id                  = "device_id";
        mod_rdp_params.enable_tls                 = true;
        mod_rdp_params.enable_nla                 = false;
        mod_rdp_params.enable_fastpath            = true;
        mod_rdp_params.enable_mem3blt             = true;
        mod_rdp_params.enable_new_pointer         = true;
        mod_rdp_params.enable_glyph_cache         = true;
        mod_rdp_params.enable_ninegrid_bitmap     = true;
        std::string allow_channels                = "*";
        mod_rdp_params.allow_channels             = &allow_channels;
        mod_rdp_params.deny_channels              = nullptr;
        mod_rdp_params.enable_rdpdr_data_analysis = false;
        mod_rdp_params.server_cert_check          = ServerCertCheck::always_succeed;

        if (verbose > 128) {
            mod_rdp_params.log();
        }

        this->mod = new_mod_rdp(
            browser_trans, session_reactor, front, client_info, redir_info, lcg_gen,
            lcg_timeobj, mod_rdp_params, authentifier, report_message, ini, nullptr);
    }

    // long long is not embind type. Use long or double (safe for 53 bits)
    long next_timeout()
    {
        session_reactor.set_current_time(tvtime());
        std::chrono::microseconds us =
            session_reactor.get_next_timeout(SessionReactor::EnableGraphics{true}, 5s)
          - session_reactor.get_current_time();
        return us.count();
    }

    void update()
    {
        session_reactor.execute_timers_at(
            SessionReactor::EnableGraphics{true}, tvtime(),
            [&]() -> gdi::GraphicApi& { return front; });
    }

    emscripten::val get_data()
    {
        auto& out = browser_trans.out_buffers;
        return emscripten::val(emscripten::typed_memory_view(out.size(), out.data()));
    }

    void clear_data()
    {
        browser_trans.out_buffers.clear();
    }

    void next_message(std::string data)
    {
        browser_trans.in_buffers.emplace_back(std::move(data));
        // browser_trans.out_buffers.insert(browser_trans.out_buffers.end(), data.begin(), data.end());
        session_reactor.execute_callbacks(*mod);
        auto fd_isset = [](int fd, auto& /*e*/){ return fd == FD_TRANS; };
        session_reactor.execute_graphics(fd_isset, front);
    }
};

// Binding code
EMSCRIPTEN_BINDINGS(client) {
    emscripten::class_<RdpClient>("RdpClient")
        .constructor<unsigned long>()
        .function("next_timeout", &RdpClient::next_timeout)
        .function("update", &RdpClient::update)
        .function("get_data", &RdpClient::get_data)
        .function("clear_data", &RdpClient::clear_data)
        .function("next_message", &RdpClient::next_message)
    ;
}

// int run_main()
// {
//
//     auto run_rdp = [&]{
//         auto mod = new_mod_rdp(
//             mod_trans, session_reactor, front, client_info, redir_info, lcg_gen,
//             lcg_timeobj, mod_rdp_params, authentifier, report_message, ini, nullptr);
//         using Ms = std::chrono::milliseconds;
//         return run_test_client(
//             "RDP", session_reactor, *mod, front,
//             Ms(inactivity_time_ms), Ms(max_time_ms), screen_output);
//     };
//
//     int eid = run_rdp();
//
//     if (ERR_RDP_SERVER_REDIR != eid) {
//         return eid ? 1 : 0;
//     }
//
//     set_server_redirection_target(ini, report_message);
//
//     return run_rdp() ? 2 : 0;
// }
//
// int main(int argc, char** argv)
// {
//     try {
//         run_main();
//     }
//     catch (Error const& e) {
//         if (e.errnum) {
//             std::cerr << e.errmsg() << " - " << strerror(e.errnum) << std::endl;
//             return e.errnum;
//         }
//         std::cerr << e.errmsg() << std::endl;
//         return 1;
//     }
//     catch (std::exception const& e) {
//         std::cerr << e.what() << std::endl;
//         return 1;
//     }
// }
