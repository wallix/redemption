#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

#include <array>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <deque>
#include <mutex>

#include "utils/log.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "scard/scard_pcsc_common.hpp"
#include "scard/scard_pcsc_server.hpp"


///////////////////////////////////////////////////////////////////////////////


scard_pcsc_server::scard_pcsc_server(const char *socket_path,
    scard_pcsc_server_handler *handler_ptr)
    :
    _socket(0),
    _handler_ptr(handler_ptr),
    _state(false)
{
    // create server socket
    auto socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket == 0) 
    { 
        throw scard_pcsc_exception(
            "Failed to create scard server socket.");
    }

    // store created socket
    _socket.reset(socket);

    // bind socket
    {
        struct sockaddr_un address;

        // initialize address structure
        memset(&address, 0, sizeof(address));

        // fill in address structure
        address.sun_family = AF_UNIX;
        strncpy(address.sun_path, socket_path,
            sizeof(address.sun_path) - 1);

        // bind address
        if (bind(_socket.fd(), reinterpret_cast<struct sockaddr *>(&address),
            sizeof(address)) < 0) 
        { 
            throw scard_pcsc_exception(
                "Failed to bind scard server socket.");
        }
    }

    // prepare to accept connections
    if (listen(_socket.fd(), 1) < 0) 
    { 
        throw scard_pcsc_exception(
            "Failed to listen to scard client connections.");
    }
}

scard_pcsc_server::~scard_pcsc_server()
{
    // disconnect
    disconnect();

    // destroy socket
    ::unlink(_socket_path.c_str());
}

void scard_pcsc_server::disconnect()
{
    // flag as stopped
    _state = true;

    // shutdown socket
    ::shutdown(_socket.fd(), SHUT_RDWR);

    // close socket and reset file descriptor to an invalid value
    _socket.reset();
}

void scard_pcsc_server::serve()
{
    std::array<uint8_t, 1024> buffer;

    while (!_state)
    {
        LOG(LOG_INFO, "scard_pcsc_server::serve: "
            "waiting for incoming connection...");
        
        // notify start
        if (_handler_ptr)
        {
            _handler_ptr->handle_pcsc_server_start();
        }

        // accept incoming connection
        auto socket = ::accept(_socket.fd(), nullptr, nullptr);
        if (socket < 0)
        {
            LOG(LOG_ERR, "scard_pcsc_server::serve: "
                "failed to accept incoming connection.");
            
            continue;
        }

        LOG(LOG_INFO, "scard_pcsc_server::serve: "
            "accepted incoming connection.");

        unique_fd client_socket(socket);

        // check peer permissions
        {
            struct ucred credentials;
            socklen_t credentials_length = sizeof(struct ucred);

            // retrieve peer credentials
            int ret = getsockopt(socket, SOL_SOCKET, SO_PEERCRED,
                &credentials, &credentials_length);
            if (ret < 0)
            {
                LOG(LOG_ERR, "scard_pcsc_server::serve: "
                    "failed to check peer credentials.");
                
                continue;
            }

            const pid_t pid = getpid();
            const uid_t uid = getuid();
            const gid_t gid = getgid();

            // check peer credentials
            if ((credentials.pid != pid) || (credentials.uid != uid)
                    || (credentials.gid != gid))
            {
                LOG(LOG_ERR, "scard_pcsc_server::serve: "
                    "peer not allowed to connect; disconnecting peer...");

                continue;
            }
        }

        // notify client connection
        if (_handler_ptr)
        {
            _handler_ptr->handle_pcsc_client_connection(socket);
        }

        struct timeval timeout;

        // define polling frequency
        timeout.tv_sec  = 0;
        timeout.tv_usec = 1 * 1000;

        fd_set fds;
        int n_fds;

        while (!_state)
        {
            FD_ZERO(&fds);
            FD_SET(socket, &fds);

            // check if data can be read from socket and receive incoming call
            n_fds = ::select(socket + 1, &fds, NULL, NULL, &timeout);
            if (n_fds < 0)
            {
                LOG(LOG_ERR, "scard_pcsc_server::serve: "
                    "failed to poll socket for reading.");
                
                break;
            }
            else
            if (n_fds > 0)
            {
                // do receive
                scard_wrapped_call wrapped_call = receive(
                    socket, buffer.data(), buffer.size());

                // defer handling of incoming data
                if (_handler_ptr)
                {
                    _handler_ptr->handle_pcsc_server_data(std::move(wrapped_call));
                }
            }

            FD_ZERO(&fds);
            FD_SET(socket, &fds);

            // check if data can be written to socket and send outgoing return
            n_fds = ::select(socket + 1, NULL, &fds, NULL, &timeout);
            if (n_fds < 0)
            {
                LOG(LOG_ERR, "scard_pcsc_server::serve: "
                    "failed to poll socket for writing.");
                
                break;
            }
            else
            if (n_fds > 0)
            {
                scard_wrapped_return wrapped_return;

                // pop return
                {
                    std::lock_guard lock(_queue_mutex);

                    // skip if no return to be sent
                    if (_queue.empty())
                    {
                        continue;
                    }

                    // unqueue wrapped return
                    wrapped_return = std::move(_queue.front());
                    _queue.pop_front();
                }

                // do send
                send(socket, std::move(wrapped_return));
            }
        }

        // notify client disconnection
        if (_handler_ptr)
        {
            _handler_ptr->handle_pcsc_client_disconnection(socket);
        }
    }

    // notify stop
    if (_handler_ptr)
    {
        _handler_ptr->handle_pcsc_server_stop();
    }
}

void scard_pcsc_server::post(scard_wrapped_return data)
{
    std::lock_guard lock(_queue_mutex);

    // enqueue data
    _queue.push_back(std::move(data));
}


///////////////////////////////////////////////////////////////////////////////


scard_wrapped_call scard_pcsc_server::receive(int fd,
    uint8_t *buffer, std::size_t buffer_size)
{
    int ret;
    uint32_t length;

    // receive message length
    ret = read(fd, &length, sizeof(length));
    if (ret != sizeof(length))
    {
        throw scard_pcsc_exception(
            "Failed to receive message: "
            "failed to receive message length."
        );
    }

    assert(length <= buffer_size);

    LOG(LOG_INFO, "scard_pcsc_server::receive: "
        "received message length: %" PRIu32 " (%lu bytes)",
        length, sizeof(length));

    uint32_t io_control_code;

    // receive IO control code
    ret = read(fd, &io_control_code, sizeof(io_control_code));
    if (ret != sizeof(io_control_code))
    {
        throw scard_pcsc_exception(
            "Failed to receive message: "
            "failed to receive IO control code."
        );
    }
    length -= sizeof(io_control_code);

    LOG(LOG_INFO, "scard_pcsc_server::receive: "
        "received IO control code: 0x%08" PRIX32 " (%lu bytes)",
        io_control_code, sizeof(io_control_code));

    // receive payload
    ret = read(fd, buffer, length);
    if (ret != static_cast<int>(length))
    {
        throw scard_pcsc_exception(
            "Failed to receive message: "
            "failed to receive payload."
        );
    }

    LOG(LOG_INFO, "scard_pcsc_server::receive: "
        "received payload (%u bytes).",
        length);

    return scard_wrapped_call(
        io_control_code, bytes_view(buffer, length)
    );
}

void scard_pcsc_server::send(int fd, scard_wrapped_return &&wrapped_return)
{
    int ret;

    uint32_t length = 
        sizeof(wrapped_return.io_control_code) +
        wrapped_return.data.size()
    ;

    // send message length
    ret = write(fd, &length, sizeof(length));
    if (ret != sizeof(length))
    {
        throw scard_pcsc_exception(
            "Failed to send message: "
            "failed to send message length."
        );
    }

    LOG(LOG_INFO, "scard_pcsc_server::send: "
        "sent message length: %" PRIu32 " (%lu bytes)",
        length, sizeof(length));

    const uint32_t io_control_code = wrapped_return.io_control_code;

    // send IO control code
    ret = write(fd, &io_control_code, sizeof(io_control_code));
    if (ret != sizeof(io_control_code))
    {
        throw scard_pcsc_exception(
            "Failed to send message: "
            "failed to send IO control code."
        );
    }
    length -= sizeof(io_control_code);

    LOG(LOG_INFO, "scard_pcsc_server::send: "
        "sent IO control code: 0x%08" PRIX32 " (%lu bytes)",
        io_control_code, sizeof(io_control_code));

    const auto buffer = wrapped_return.data.data();

    // send payload
    ret = write(fd, buffer, length);
    if (ret != static_cast<int>(length))
    {
        throw scard_pcsc_exception(
            "Failed to send message: "
            "failed to send payload."
        );
    }

    LOG(LOG_INFO, "scard_pcsc_server::send: "
        "sent message payload (%u bytes).",
        length);
}