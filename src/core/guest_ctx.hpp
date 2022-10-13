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
Copyright (C) Wallix 2022
Author(s): Proxies Team
*/

#pragma once

#include "front/front.hpp"
#include "transport/socket_transport.hpp"
#include "acl/auth_api.hpp"
#include "capture/cryptofile.hpp"
#include "utils/netutils.hpp"
#include "core/listen.hpp"
#include "utils/base64.hpp"

#include <sys/types.h>
#include <sys/socket.h>


inline void front_process(
    TpduBuffer& buffer, Front& front, InTransport front_trans,
    Callback& callback)
{
    buffer.load_data(front_trans);
    while (buffer.next(TpduBuffer::PDU)) // or TdpuBuffer::CredSSP in NLA
    {
        bytes_view tpdu = buffer.current_pdu_buffer();
        uint8_t current_pdu_type = buffer.current_pdu_get_type();
        front.incoming(tpdu, current_pdu_type, callback);
    }
}

struct GuestCtx
{
    bool is_started() const noexcept
    {
        return listen_event || guest_ptr;
    }

    bool has_front() const noexcept
    {
        return bool(guest_ptr);
    }

    SocketTransport& front_transport() const noexcept
    {
        return guest_ptr->get_transport();
    }

    zstring_view sck_path() const noexcept
    {
        return sck_path_;
    }

    zstring_view sck_password() const noexcept
    {
        return sck_password_;
    }

    struct ResultError
    {
        unsigned errnum;
        char const* errmsg;
    };

    ResultError start(
        chars_view sck_path_base, chars_view session_id,
        EventContainer& event_container, Front& front, Callback& callback,
        SessionLogApi& session_log, MonotonicTimePoint::duration invitation_delay,
        Random& rnd, Inifile& original_ini, bool enable_shared_control)
    {
        assert(!listen_event);

        // generate random socket path
        ++sck_counter_;
        str_assign(
            sck_path_, sck_path_base,
            "/front2_", session_id, '_',
            RandomText().text(rnd, 8), '_',
            int_to_decimal_chars(sck_counter_), ".sck"
        );

        // create a unix socket for guest front
        listen_sck = create_unix_server(sck_path_, EnableTransparentMode::No);
        if (!listen_sck.is_open()) {
            LOG(LOG_ERR, "Guest::start() create server error");
            int errnum = errno;
            return (errnum)
                ? ResultError{checked_int(errnum), strerror(errnum)}
                : ResultError{0xfff0u, "Create session sharing server error"};
        }

        // generate random password
        str_assign(sck_password_, RandomText().text(rnd, 32));

        LOG(LOG_INFO, "Guest::start(delay=%lds) start server",
            std::chrono::duration_cast<std::chrono::seconds>(invitation_delay).count());

        listen_event = &event_container.event_creator().create_event_fd_timeout(
            "GuestServer", this, listen_sck.fd(), invitation_delay,
            [
                this, &event_container, &front, &callback, &session_log, &rnd, &original_ini,
                enable_shared_control
            ](Event& /*event*/) mutable {
                int conn_sck = accept(listen_sck.fd(), nullptr, nullptr);
                int errnum = errno;
                close_listen_sck();

                if (conn_sck == -1) {
                    LOG(LOG_ERR, "Guest: accept error: %d %s", errnum, strerror(errnum));
                    return;
                }

                auto* kill_fn = +[](void* self){ static_cast<GuestCtx*>(self)->close_guest(); };
                guest_ptr = std::make_unique<Guest>(
                    unique_fd(conn_sck), event_container, front, original_ini, rnd,
                    enable_shared_control, kill_fn, this);

                guest_ptr->set_io_event([
                    this, &callback, &session_log, password = std::move(sck_password_)
                ](Event& /*event*/) {
                    // no input
                    NullCallback null_callback;
                    process_guest(null_callback);
                    // possibly closed by process_guest
                    if (!guest_ptr) {
                        return ;
                    }

                    if (guest_ptr->is_up_and_running()) {
                        // check credential
                        if (password != guest_ptr->get_client_info().password) {
                            LOG(LOG_ERR, "Guest: bad credential of session sharing");
                            session_log.log6(LogId::SESSION_SHARING_GUEST_CONNECTION_REJECTED, {
                                KVLog("name"_av, "guest-1"_av),
                                KVLog("reason"_av, "bad password"_av),
                            });
                            close_guest();
                            return;
                        }

                        guest_ptr->start_sharing(callback, session_log);
                        guest_ptr->set_io_event([this, &callback](Event& /*event*/) {
                            process_guest(callback);
                        });
                    }
                });
            },
            // timeout
            [this](Event& /*event*/) {
                LOG(LOG_INFO, "Guest::start() timeout");
                stop();
            }
        );

        return ResultError{0, nullptr};
    }

    void stop()
    {
        if (listen_sck.is_open()) {
            close_listen_sck();
        }

        if (guest_ptr) {
            close_guest();
        }
    }

    ~GuestCtx()
    {
        if (listen_sck.is_open()) {
            close_listen_sck();
        }
    }

private:
    struct RandomText
    {
        char text_buffer[64];

        chars_view text(Random& rnd, std::size_t bytes_len)
        {
            char buffer[32];
            assert(sizeof(buffer) >= bytes_len);
            rnd.random(buffer, bytes_len);
            return base64_encode({buffer, bytes_len}, make_writable_array_view(text_buffer)).as_chars();
        }
    };

    struct GuestData
    {
        Inifile ini;
        TpduBuffer rbuf;
        Front& user_front;
        bool is_synchronized = false;
        Event* io_event = nullptr;
        EventContainer& event_container;

        GuestData(
            EventContainer& event_container,
            Front& user_front,
            Inifile const& original_ini)
        : user_front(user_front)
        , event_container(event_container)
        {
            original_ini.copy_variables_to(ini);
        }

        Inifile& get_ini()
        {
            return ini;
        }
    };

    class Guest final : GuestData, SocketTransport, public Front
    {
    public:
        Guest(
            unique_fd&& conn_sck,
            EventContainer& event_container,
            Front& user_front,
            Inifile const& original_ini,
            Random& rnd,
            bool enable_shared_control,
            void(*kill_fn)(void*),
            void* fn_ctx)
        : GuestData(event_container, user_front, original_ini)
        , SocketTransport(
            "Guest"_sck_name, std::move(conn_sck), ""_av, 0,
            std::chrono::milliseconds(1000),
            std::chrono::milliseconds::zero(),
            std::chrono::milliseconds(1000),
            SocketTransport::Verbose()/*TODO debug::session_sharing_front*/,
            nullptr)
        , Front(event_container, null_acl_report,
            *this, rnd, get_ini(), cctx,
            Front::GuestParameters{
                .is_guest = true,
                .enable_shared_control = enable_shared_control,
                .screen_info = user_front.get_client_info().screen_info,
                .kill_fn = kill_fn,
                .fn_ctx = fn_ctx,
            }
        )
        {}

        ~Guest()
        {
            this->remove_io_event();
        }

        void process(Callback& callback)
        {
            front_process(rbuf, get_front(), get_transport(), callback);
        }

        template<class F>
        void set_io_event(F f)
        {
            this->remove_io_event();
            this->io_event = &event_container.event_creator()
              .create_event_fd_without_timeout("GuestClient", this, get_fd(), f);
        }

        void start_sharing(Callback& callback, SessionLogApi& session_log)
        {
            user_front.add_guest(*this, session_log);
            is_synchronized = true;
            auto screen = user_front.get_client_info().screen_info;
            callback.rdp_input_invalidate({0, 0, screen.width, screen.height});
        }

        void stop_sharing()
        {
            this->remove_io_event();

            if (is_synchronized) {
                user_front.remove_guest(*this);
            }

            try {
                this->get_front().disconnect();
                this->get_transport().disconnect();
            }
            catch (...) {
            }
        }

        SocketTransport& get_transport()
        {
            return *this;
        }

    private:
        void remove_io_event() noexcept
        {
            if (this->io_event) {
                io_event->garbage = true;
                io_event = nullptr;
            }
        }

        Front& get_front()
        {
            return *this;
        }

        NullAclReport null_acl_report;
        CryptoContext cctx;
    };

    void process_guest(Callback& callback)
    {
        try {
            guest_ptr->process(callback);
        }
        catch (...) {
            close_guest();
        }
    }

    void close_listen_sck()
    {
        LOG(LOG_INFO, "Guest::stop()");
        listen_sck.close();
        remove(sck_path_.c_str());
        listen_event->garbage = true;
        listen_event = nullptr;
    }

    void close_guest()
    {
        guest_ptr->stop_sharing();
        guest_ptr.reset();
    }

    std::unique_ptr<Guest> guest_ptr;
    unique_fd listen_sck = invalid_fd();
    Event* listen_event = nullptr;

    unsigned sck_counter_ = 0;
    std::string sck_path_;
    std::string sck_password_;
};
