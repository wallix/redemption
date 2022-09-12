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

    void start(
        std::string_view session_id,
        EventContainer& event_container, Front& front, Callback& callback,
        SessionLogApi& session_log, UdevRandom& rnd,
        Inifile& original_ini, std::string_view user_data,
        bool enable_shared_control)
    {
        assert(!listen_event);

        sck_path = generate_sck_path(session_id);

        original_ini.set_acl<cfg::context::session_sharing_userdata>(user_data);

        listen_sck = create_unix_server(sck_path, EnableTransparentMode::No);
        if (!listen_sck.is_open()) {
            int errnum = errno;
            LOG(LOG_ERR, "Guest::start() create server error");
            original_ini.set_acl<cfg::context::session_sharing_invitation_error_code>(checked_int(errnum));
            original_ini.set_acl<cfg::context::session_sharing_invitation_error_message>(strerror(errnum));
            return ;
        }

        original_ini.set_acl<cfg::context::session_sharing_invitation_error_code>(0u);
        std::string session_sharing_invitation_id = generate_password(rnd);
        original_ini.set_acl<cfg::context::session_sharing_invitation_id>(session_sharing_invitation_id);
        original_ini.set_acl<cfg::context::session_sharing_invitation_addr>(sck_path);

        LOG(LOG_INFO, "Guest::start() create server error");

        // TODO add timer before auto-close

        listen_event = &event_container.event_creator().create_event_fd_without_timeout(
            "GuestServer", this, listen_sck.fd(),
            [
                this, &event_container, &front, &callback, &session_log, &rnd, &original_ini,
                password = std::move(session_sharing_invitation_id),
                enable_shared_control
            ](Event& /*event*/) mutable {
                LOG(LOG_DEBUG, "guest connection");

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
                    this, &callback, &session_log, password = std::move(password)
                ](Event& /*event*/) {
                    // no input
                    NullCallback null_callback;
                    process_guest(null_callback);

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
            }
        );
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

    static std::string generate_password(Random& rnd)
    {
        uint8_t rnd_data[32];
        rnd.random(rnd_data, sizeof(rnd_data));

        std::array<uint8_t, 64> password_rep;
        base64_encode(make_array_view(rnd_data), writable_bytes_view(password_rep));
        std::string password(char_ptr_cast(password_rep.data()), base64_encode_size(sizeof(rnd_data)));

        return password;
    }

    static std::string generate_sck_path(std::string_view session_id)
    {
        return str_concat("/tmp/front2_", session_id, ".sck");
    }

private:
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
            UdevRandom& rnd,
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
        listen_sck.close();
        remove(sck_path.c_str());
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

    std::string sck_path;
};
