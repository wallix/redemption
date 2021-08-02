#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <thread>

#include "core/channel_list.hpp"
#include "core/events.hpp"
#include "core/front_api.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "scard/scard_common.hpp"
#include "scard/scard_krb_client.hpp"
#include "scard/scard_pcsc_common.hpp"
#include "scard/scard_pcsc_environment.hpp"
#include "scard/scard_pcsc_server.hpp"
#include "scard/scard_pkcs11_enumerator.hpp"
#include "scard/scard_rdp_channel.hpp"
#include "scard/scard_utils.hpp"
#include "utils/fileutils.hpp"
#include "utils/log.hpp"

#include "scard/scard_bridge.hpp"

// TEST ONLY
#define TEST_PRINCIPAL_NAME "cyrille@INFERNO.PROXY"
#define TEST_SECURITY_CODE  "123456"
// TEST ONLY

#define SOCKET_FILE_NAME_PREFIX     "rdpcsc."
#define SOCKET_FILE_EXTENSION       "socket"


///////////////////////////////////////////////////////////////////////////////


scard_bridge::scard_bridge(FrontAPI &front, EventContainer &events)
    :
    _events_guard(events),
    _credentials(TEST_PRINCIPAL_NAME, TEST_SECURITY_CODE),
    _rdp_channel_established(false),
    _state(false)
{
    // create RDP channel sender
    {
        const auto &channel_list = front.get_channel_list();
        const auto channel_definition_ptr = channel_list.get_by_name(
            CHANNELS::channel_names::rdpdr);

        if (!channel_definition_ptr)
        {
            throw scard_exception("Failed to instantiate bridge: "
                "could not find RDPDR channel among the available front channels.");
        }

        _rdp_channel_sender_ptr = std::make_unique<scard_rdp_channel_sender>(
            front, *channel_definition_ptr, true);
    }

    // create RDP channel
    _rdp_channel_ptr = std::make_unique<scard_rdp_channel>(
        events, _rdp_channel_sender_ptr.get(), this,
        RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump);

    // find a unique socket path for the PC/SC server
    const auto server_socket_path = get_unique_socket_path("/tmp");
    
    // create PC/SC server
    _pcsc_server_ptr = std::make_unique<scard_pcsc_server>(
        server_socket_path.c_str(), this);
    
    // create PKCS#11 enumerator
    _pkcs11_enumerator_ptr = std::make_unique<scard_pkcs11_enumerator>(
        "/usr/lib/x86_64-linux-gnu/opensc-pkcs11.so", this);
    
    // create Kerberos client
    _krb_client_ptr = std::make_unique<scard_krb_client>(this);
    
    // set up PC/SC environment
    scard_pcsc_environment::set_socket_path(server_socket_path.c_str());

    // start PC/SC server
    _pcsc_server_future = std::async(std::launch::async,
        &scard_pcsc_server::serve, _pcsc_server_ptr.get());
    
    // establish RDP channel
    _rdp_channel_ptr->establish();
}

scard_bridge::~scard_bridge()
{
    LOG(LOG_INFO, "scard_bridge::~scard_bridge: "
        "destroying bridge...");

    // wait for threads termination
    if (_pkcs11_enumerator_future.valid())
    {
        _pkcs11_enumerator_future.get();
    }
    if (_krb_client_future.valid())
    {
        _krb_client_future.get();
    }

    // kill PC/SC server
    _pcsc_server_ptr.reset(nullptr);

    // wait for threads termination
    if (_pcsc_server_future.valid())
    {
        _pcsc_server_future.get();
    }
}

bool scard_bridge::process(uint32_t total_length, uint32_t flags, bytes_view chunk_data)
{
    LOG(LOG_INFO, "scard_bridge::process");
    
    // process RDP data
    _rdp_channel_ptr->process_client_message(
        total_length, flags, chunk_data);
    
    // skip if RDP channel not established yet
    if (!_rdp_channel_established)
    {
        return false;
    }

    std::unique_lock lock(_queue_mutex);

    // wait for data
    while (!_state && _queue.empty())
    {
        _queue_condition.wait(lock);
    }

    // send data over RDP channel
    while (!_queue.empty())
    {
        // do send data
        _rdp_channel_ptr->send(std::move(_queue.front()));

        // discard sent data
        _queue.pop_front();
    }

    return _state;
}

bool scard_bridge::process()
{
    LOG(LOG_INFO, "scard_bridge::process");

    std::unique_lock lock(_queue_mutex);

    // send data over RDP channel
    while (!_queue.empty())
    {
        // do send data
        _rdp_channel_ptr->send(std::move(_queue.front()));

        // discard sent data
        _queue.pop_front();
    }

    return _state;
}

void scard_bridge::stop()
{
    LOG(LOG_INFO, "scard_bridge::stop: "
        "stopping bridge...");

    // flag as stopped
    _state = true;

    // wake up thread
    _queue_condition.notify_one();
}

void scard_bridge::handle_pcsc_server_start()
{
    LOG(LOG_INFO, "scard_bridge::handle_pcsc_server_start: "
        "PC/SC server started.");
}

void scard_bridge::handle_pcsc_client_connection(int /*socket*/)
{
    LOG(LOG_INFO, "scard_bridge::handle_pcsc_client_connection: "
        "client connected.");
}

void scard_bridge::handle_pcsc_server_data(
    scard_wrapped_call data)
{
    LOG(LOG_INFO, "scard_bridge::handle_pcsc_server_data: "
        "received PC/SC data from server.");

    {
        std::lock_guard lock(_queue_mutex);

        // enqueue data
        _queue.push_back(std::move(data));
    }

    // notify availability of server data
    _queue_condition.notify_one();
}

void scard_bridge::handle_pcsc_client_disconnection(int /*socket*/)
{
    LOG(LOG_INFO, "scard_bridge::handle_pcsc_client_connection: "
        "client disconnected.");
}

void scard_bridge::handle_pcsc_server_stop()
{
    LOG(LOG_INFO, "scard_bridge::handle_pcsc_server_stop: "
        "PC/SC server stopped.");
}

void scard_bridge::handle_rdp_channel_establishment()
{
    LOG(LOG_INFO, "scard_bridge::handle_rdp_channel_establishment: "
        "RDP channel established.");
    
    // flag RDP channel as established
    _rdp_channel_established = true;
    
    // enumerate on-card certificates
    _pkcs11_enumerator_future = std::async(std::launch::async,
        &scard_pkcs11_enumerator::enumerate, _pkcs11_enumerator_ptr.get());
}

void scard_bridge::handle_rdp_channel_data(
    scard_wrapped_return data)
{
    LOG(LOG_INFO, "scard_bridge::handle_rdp_channel_data: "
        "received RDP data from channel.");

    _pcsc_server_ptr->post(std::move(data));
}

void scard_bridge::handle_pkcs11_enumeration_start()
{
    LOG(LOG_INFO, "scard_bridge::handle_pkcs11_enumeration_start: "
        "PKCS#11 enumeration started.");
}

std::size_t scard_bridge::provide_pkcs11_security_code(
    writable_bytes_view buffer)
{
    const auto &security_code = std::get<
        credentials_entry_index_security_code>(_credentials);

    assert (security_code.size() <= buffer.size());

    // copy security code to buffer
    std::copy_n(security_code.begin(), security_code.size(),
        buffer.begin());

    return security_code.size();
}

void scard_bridge::handle_pkcs11_enumeration_end(
    const scard_pkcs11_identity_list &identities)
{
    LOG(LOG_INFO, "scard_bridge::handle_pkcs11_enumeration_end: "
        "PKCS#11 enumeration ended: found %lu suitable identitie(s).",
        identities.size());
    
    const auto &principal_name = std::get<
        credentials_entry_index_principal_name>(_credentials);
    const auto &security_code = std::get<
        credentials_entry_index_security_code>(_credentials);
    
    // filter matching identities
    auto it = std::find_if(identities.begin(), identities.end(),
        [&principal_name](const scard_pkcs11_identity &identity)
        {
            return compare_case_insensitive(identity.principal_name(),
                principal_name);
        }
    );
    if (it != identities.end())
    {
        LOG(LOG_INFO, "scard_bridge::handle_pkcs11_enumeration_end: "
            "matching identity found.");
        
        // get Kerberos initial credentials
        _krb_client_future = std::async(std::launch::async,
            &scard_krb_client::get_credentials, _krb_client_ptr.get(),
                it->string(), principal_name, security_code);
    }
    else
    {
        LOG(LOG_INFO, "scard_bridge::handle_pkcs11_enumeration_end: "
            "no matching identity found.");
        
        // stop
        stop();
    }
}

void scard_bridge::handle_krb_client_result(bool success)
{
    LOG(LOG_INFO, "scard_bridge::handle_krb_client_result: "
        "credentials retrieval %s.", (success ? "succeeded" : "failed"));
    
    // stop
    stop();
}

std::string scard_bridge::get_unique_socket_path(
    const char *socket_directory_path)
{
    // ensure the socket directory exists
    if (!dir_exist(socket_directory_path))
    {
        throw scard_exception("Failed to get unique socket path: "
            "provided socket directory does not exist.");
    }

    std::string socket_path;

    // build a unique socket file path
    {
        const unsigned max_index = -1;
        unsigned index = 0;

        do
        {
            std::string socket_file_name = std::string((SOCKET_FILE_NAME_PREFIX))
                + std::to_string(index);

            // build candidate socket path
            MakePath(socket_path, socket_directory_path,
                socket_file_name.c_str(), (SOCKET_FILE_EXTENSION));
            
            ++index;
        }
        while (file_exist(socket_path) && (index < max_index));

        // handle the (unlikely) case where no candidate path matches
        if (index == max_index)
        {
            throw scard_exception("Failed to get unique socket path: "
                "no path available in mask.");
        }
    }

    return socket_path;
}