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
    Copyright (C) Wallix 2021
    Author(s): Florent Plard
*/

#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <tuple>

#include "scard/scard_krb_client.hpp"
#include "scard/scard_pcsc_common.hpp"
#include "scard/scard_pcsc_server.hpp"
#include "scard/scard_pkcs11_enumerator.hpp"
#include "scard/scard_rdp_channel.hpp"
#include "scard/scard_x509.hpp"
#include "utils/sugar/bytes_view.hpp"


///////////////////////////////////////////////////////////////////////////////


class EventContainer;
class FrontAPI;

class scard_bridge
    :
    public scard_krb_client_handler,
    public scard_pcsc_server_handler,
    public scard_pkcs11_enumerator_handler,
    public scard_rdp_channel_handler
{
public:
    ///
    scard_bridge(FrontAPI &front, EventContainer &events);

    ///
    ~scard_bridge();
    
    ///
    bool process(uint32_t total_length,  uint32_t flags,
        bytes_view chunk_data);
    
    ///
    bool process();
 
private:
    ///
    typedef std::tuple<
        std::string, std::string
    > credentials_container;

    ///
    enum
    {
        credentials_entry_index_principal_name = 0,
        credentials_entry_index_security_code = 1
    };

    ///
    EventsGuard _events_guard;

    ///
    credentials_container _credentials;
    
    ///
    std::unique_ptr<scard_rdp_channel_sender> _rdp_channel_sender_ptr;

    ///
    std::unique_ptr<scard_rdp_channel> _rdp_channel_ptr;

    ///
    std::atomic<bool> _rdp_channel_established;

    ///
    std::unique_ptr<scard_pcsc_server> _pcsc_server_ptr;

    ///
    std::unique_ptr<scard_pkcs11_enumerator> _pkcs11_enumerator_ptr;

    ///
    std::unique_ptr<scard_krb_client> _krb_client_ptr;

    ///
    std::atomic<bool> _state;

    ///
    std::deque<scard_wrapped_call> _queue;

    ///
    std::mutex _queue_mutex;

    ///
    std::condition_variable _queue_condition;

    ///
    std::future<void> _pcsc_server_future;

    ///
    std::future<scard_pkcs11_identity_list> _pkcs11_enumerator_future;

    ///
    std::future<bool> _krb_client_future;

    ///
    void stop();

    ///
    virtual void handle_pcsc_server_start() override;

    ///
    virtual void handle_pcsc_client_connection(int socket) override;

    ///
    virtual void handle_pcsc_server_data(scard_wrapped_call data) override;

    ///
    virtual void handle_pcsc_client_disconnection(int socket) override;

    ///
    virtual void handle_pcsc_server_stop() override;

    ///
    virtual void handle_rdp_channel_establishment() override;

    ///
    virtual void handle_rdp_channel_data(
        scard_wrapped_return data) override;

    ///
    virtual void handle_pkcs11_enumeration_start() override;

    ///
    virtual std::size_t provide_pkcs11_security_code(
        writable_bytes_view buffer) override;

    ///
    virtual void handle_pkcs11_enumeration_end(
        const scard_pkcs11_identity_list &identities) override;

    ///
    virtual void handle_krb_client_result(bool success) override;

    ///
    static std::string get_unique_socket_path(
        const char *socket_directory_path);
};