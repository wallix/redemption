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
   Copyright (C) Wallix 2017-2018
   Author(s): Clément Moroldo
*/


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"

#include <csignal>

#include "utils/log.hpp"

#include "utils/timebase.hpp"
#include "client_redemption/client_redemption.hpp"
#include "utils/set_exception_handler_pretty_message.hpp"
#include "system/scoped_ssl_init.hpp"
#include "core/events.hpp"

#pragma GCC diagnostic pop

#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

class ClientRedemptionHeadless : public ClientRedemption
{
public:
    ClientRedemptionHeadless(EventContainer& events,
                             ClientRedemptionConfig & config)
        : ClientRedemption(events, config)
    {
        this->cmd_launch_conn();
    }

    ~ClientRedemptionHeadless() = default;

    void session_update(timeval /*now*/, LogId /*id*/, KVLogList /*kv_list*/) override {}
    void possible_active_window_change() override {}

    void close() override {}

    void connect(const std::string& ip, const std::string& name, const std::string& pwd, const int port) override
    {
        ClientRedemption::connect(ip, name, pwd, port);

        if (this->config.connected) {
            mod_api* mod = this->_callback.get_mod();

            while (!mod->is_up_and_running()) {
                if (int err = this->wait_and_draw_event(3s)) {
                    std::cout << " Error: wait_and_draw_event() fail during negociation (" << err << ").\n";
                }
            }

            this->start_wab_session_time = tvtime();
        }
    }

    int wait_and_draw_event(std::chrono::milliseconds timeout)
    {
        timeval now = tvtime();
        int max = 0;
        fd_set rfds;
        io_fd_zero(rfds);

        this->events.get_fds([&rfds,&max](int fd){
            io_fd_set(fd, rfds);
            max = std::max(max, fd);
        });

        auto next_timeout = this->events.next_timeout();
        timeval ultimatum = (next_timeout == timeval{})
            ? now + timeout
            : (now < next_timeout)
            ? to_timeval(next_timeout - now)
            : timeval{};

        int num = select(max + 1, &rfds, nullptr, nullptr, &ultimatum);

        if (num < 0) {
            if (errno == EINTR) {
                // ExecuteEventsResult::Continue;
                return 0;
            }
            // ExecuteEventsResult::Error
            LOG(LOG_ERR, "RDP CLIENT :: errno = %s", strerror(errno));
            return 9;
        }

        this->events.set_current_time(tvtime());
        this->events.execute_events([&rfds](int fd){
            return io_fd_isset(fd, rfds);
        }, false);

        return 0;
    }
};

static int run_mod(
    ClientRedemptionHeadless& front,
    ClientRedemptionConfig & config,
    ClientCallback & callback,
    timeval start_win_session_time)
{
    const timeval time_stop = addusectimeval(config.time_out_disconnection, tvtime());
    const auto time_mark = 50ms;

    if (callback.get_mod()) {
        auto & mod = *(callback.get_mod());

        bool logged = false;

        while (true)
        {
            if (!logged && mod.is_up_and_running()) {
                logged = true;

                std::cout << "RDP Session Log On.\n";
                if (config.quick_connection_test) {

                    std::cout << "quick_connection_test\n";
                    front.disconnect("", false);
                    return 0;
                }
            }

            if (time_stop < tvtime() && !config.persist) {
                std::cerr << " Exit timeout (timeout = " << config.time_out_disconnection.count() << " ms)" << std::endl;
                front.disconnect("", false);
                return 8;
            }

            if (int err = front.wait_and_draw_event(time_mark)) {
                return err;
            }

            // send key to keep alive
            if (config.keep_alive_freq) {
                std::chrono::microseconds duration = difftimeval(tvtime(), start_win_session_time);

                if ( ((duration.count() / 1000000) % config.keep_alive_freq) == 0) {
                    callback.send_rdp_scanCode(0x1e, KBD_FLAG_UP);
                    callback.send_rdp_scanCode(0x1e, 0);
                }
            }
        }
    }

    return 0;
}

int main(int argc, char const** argv)
{
    set_exception_handler_pretty_message();
    openlog("rdpproxy", LOG_CONS | LOG_PERROR, LOG_USER);

    {
        struct sigaction sa;
        sa.sa_flags = 0;
        sigaddset(&sa.sa_mask, SIGPIPE);
        sa.sa_handler = [](int sig){
            std::cout << "got SIGPIPE(" << sig << ") : ignoring\n";
        };
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
        REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
        #if REDEMPTION_COMP_CLANG_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
            REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wzero-as-null-pointer-constant")
        #endif
        sigaction(SIGPIPE, &sa, nullptr);
        REDEMPTION_DIAGNOSTIC_POP
    }

    ClientRedemptionConfig config(RDPVerbose(0), CLIENT_REDEMPTION_MAIN_PATH);
    ClientConfig::set_config(argc, argv, config);
    EventContainer events;
    events.set_current_time(tvtime());
    ScopedSslInit scoped_ssl;

    ClientRedemptionHeadless client(events, config);

    return run_mod(client, client.config, client._callback, client.start_win_session_time);
}
