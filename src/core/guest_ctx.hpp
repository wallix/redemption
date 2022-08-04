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
        return event || front2_ptr;
    }

    bool has_front() const noexcept
    {
        return bool(front2_ptr);
    }

    SocketTransport& front_transport() const noexcept
    {
        return front2_ptr->get_transport();
    }

    void start(
        EventContainer& event_container, Front& front, Callback& callback,
        UdevRandom& rnd, Inifile& original_ini)
    {
        assert(!event);

        listen_sck = create_unix_server(sck_patch, EnableTransparentMode::No);
        if (!listen_sck.is_open()) {
            LOG(LOG_DEBUG, "Guest::start() create server error");
            int errnum = errno;
            original_ini.set_acl<cfg::context::session_sharing_invitation_error_code>(checked_int(errnum));
            original_ini.set_acl<cfg::context::session_sharing_invitation_error_message>(strerror(errnum));
            return ;
        }

        original_ini.set_acl<cfg::context::session_sharing_invitation_error_code>(0u);
        std::string session_sharing_invitation_id = generate_password(rnd);
        original_ini.set_acl<cfg::context::session_sharing_invitation_id>(session_sharing_invitation_id);
        original_ini.set_acl<cfg::context::session_sharing_invitation_addr>(sck_patch);

        LOG(LOG_DEBUG, "start ok");

        // TODO add timer before auto-close

        event = &event_container.event_creator().create_event_fd_without_timeout(
            "GuestServer", this, listen_sck.fd(),
            [
                this, &event_container, &front, &callback, &rnd, &original_ini,
                password = std::move(session_sharing_invitation_id)
            ](Event& /*event*/) {
                LOG(LOG_DEBUG, "guest connection");

                int conn_sck = accept(listen_sck.fd(), nullptr, nullptr);
                close_listen_sck();

                garbage_valid_event();

                if (conn_sck == -1) {
                    // TODO replace with log ?
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }

                auto* kill_fn = +[](void* self){ static_cast<GuestCtx*>(self)->close_front2(); };
                front2_ptr = std::make_unique<Guest>(
                    unique_fd(conn_sck), event_container, front, original_ini, rnd, kill_fn, this);

                front2_ptr->set_io_event([this, &callback, password = std::move(password)](Event& /*event*/) {
                    // no input
                    NullCallback null_callback;
                    process_front2(null_callback);

                    if (front2_ptr->is_up_and_running()) {
                        // check credential
                        if (password != front2_ptr->get_client_info().password) {
                            LOG(LOG_ERR, "Guest: bad credential of session sharing");
                            close_front2();
                            return;
                        }

                        front2_ptr->start_sharing(callback);
                        front2_ptr->set_io_event([this, &callback](Event& /*event*/) {
                            process_front2(callback);
                        });
                    }
                });
            }
        );
    }

    void stop()
    {
        if (listen_sck.is_open()) {
            close_listen_sck();
            if (event) {
                garbage_valid_event();
            }
        }

        if (front2_ptr) {
            close_front2();
        }
    }

    static std::string generate_password(Random& rnd)
    {
        // TODO
        (void)rnd;
        return "abc";
    }

private:
    struct GuestData
    {
        Inifile ini;
        TpduBuffer rbuf;
        Front& user_front;
        bool is_synchronized = false;
        Event* event = nullptr;
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
            UdevRandom& rnd,
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
            this->event = &event_container.event_creator()
              .create_event_fd_without_timeout("GuestClient", this, get_fd(), f);
        }

        void start_sharing(Callback& callback)
        {
            user_front.add_guest(*this);
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
            if (this->event) {
                event->garbage = true;
                event = nullptr;
            }
        }

        Front& get_front()
        {
            return *this;
        }

        NullAclReport null_acl_report;
        CryptoContext cctx;
    };

    void process_front2(Callback& callback)
    {
        try {
            front2_ptr->process(callback);
        }
        catch (...) {
            garbage_valid_event();
            close_front2();
        }
    }

    void garbage_valid_event() noexcept
    {
        event->garbage = true;
        event = nullptr;
    }

    void close_listen_sck()
    {
        listen_sck.close();
        remove(sck_patch);
    }

    void close_front2()
    {
        front2_ptr->stop_sharing();
        front2_ptr.reset();
    }

    std::unique_ptr<Guest> front2_ptr;
    unique_fd listen_sck = invalid_fd();
    Event* event = nullptr;

    static constexpr zstring_view sck_patch = "/tmp/front2.sck"_zv;
};
