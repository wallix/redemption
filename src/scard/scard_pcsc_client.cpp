#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

#include <cinttypes>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "scard/scard_pcsc_client.hpp"
#include "scard/scard_pcsc_common.hpp"
#include "scard/scard_utils.hpp"
#include "utils/log.hpp"


///////////////////////////////////////////////////////////////////////////////


scard_pcsc_client::scard_pcsc_client(const char *socket_file_path,
    std::size_t buffer_size_hint)
    :
    _socket(0),
    _buffer(buffer_size_hint)
{
    // create socket
    auto socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket < 0)
    {
        throw std::runtime_error("Failed to create scard client socket.");
    }

    // initialize address structure
    memset(&_socket_address, 0, sizeof(_socket_address));

    // fill in address structure
    _socket_address.sun_family = AF_UNIX;
    strncpy(_socket_address.sun_path, socket_file_path,
        sizeof(_socket_address.sun_path) - 1);

    // store created socket
    _socket.reset(socket);
}

void scard_pcsc_client::connect()
{
    int ret;

    LOG(LOG_INFO, "scard_pcsc_client::connect: "
        "client not connected; connecting..."
    );

    // connect socket
    ret = ::connect(_socket.fd(), reinterpret_cast<struct sockaddr *>(
        &_socket_address), sizeof(_socket_address));
    if (ret < 0) 
    { 
        throw std::runtime_error("Failed to connect socket.");
    }

    LOG(LOG_INFO, "scard_pcsc_client::connect: "
        "client connected."
    );
}

std::size_t scard_pcsc_client::send(uint32_t io_control_code, bytes_view data)
{
    int ret;
    
    uint32_t length = 
        sizeof(io_control_code) +
        data.size()
    ;
    
    // send message length
    ret = write(_socket.fd(), &length, sizeof(length));
    if (ret != sizeof(length))
    {
        // try to (re-)connect
        connect();

        // try to send message length, again
        ret = write(_socket.fd(), &length, sizeof(length));
        if (ret != sizeof(length))
        {
            throw std::runtime_error("Failed to send message: "
                "failed to send message length.");
        }
    }

    LOG(LOG_INFO, "scard_pcsc_client::send: "
        "sent message length %" PRIu32 " (%lu bytes).",
        length, sizeof(length));

    // send IO control code
    ret = write(_socket.fd(), &io_control_code, sizeof(io_control_code));
    if (ret != sizeof(io_control_code))
    {
        throw std::runtime_error("Failed to send message: "
            "failed to send IO control code.");
    }
    length -= sizeof(io_control_code);

    LOG(LOG_INFO, "scard_pcsc_client::send: "
        "sent IO control code 0x%08" PRIX32 " (%lu bytes).",
        io_control_code, sizeof(io_control_code));

    // send payload
    ret = write(_socket.fd(), data.data(), length);
    if (ret != static_cast<int>(length))
    {
        throw std::runtime_error("Failed to send message: "
            "failed to send payload.");
    }

    LOG(LOG_INFO, "scard_pcsc_client::send: "
        "sent payload (%u bytes).",
        length);

    return length;
}

bytes_view scard_pcsc_client::receive()
{
    int ret;

    uint32_t length;

    // receive message length
    ret = read(_socket.fd(), &length, sizeof(length));
    if (ret != sizeof(length))
    {
        // try to (re-)connect
        connect();

        // try to receive message length, again
        ret = read(_socket.fd(), &length, sizeof(length));
        if (ret != sizeof(length))
        {
            throw std::runtime_error("Failed to receive message: "
                "failed to receive message length.");
        }
    }

    // allocate memory for receipt
    _buffer.reserve(length);

    LOG(LOG_INFO, "scard_pcsc_client::receive: "
        "received message length %" PRIu32 " (%lu bytes).",
        length, sizeof(length));

    uint32_t io_control_code_;

    // receive IO control code
    ret = read(_socket.fd(), &io_control_code_, sizeof(io_control_code_));
    if (ret != sizeof(io_control_code_))
    {
        throw std::runtime_error("Failed to receive message: "
            "failed to receive IO control code.");
    }
    length -= sizeof(io_control_code_);

    LOG(LOG_INFO, "scard_pcsc_client::receive: "
        "received IO control code 0x%08" PRIX32 " (%lu bytes).",
        io_control_code_, sizeof(io_control_code_));

    // receive payload
    ret = read(_socket.fd(), _buffer.data(), length);
    if (ret != static_cast<int>(length))
    {
        throw std::runtime_error("Failed to receive message: "
            "failed to receive payload.");
    }

    LOG(LOG_INFO, "scard_pcsc_client::receive: "
        "received payload (%u bytes).",
        length);

    return bytes_view(_buffer.data(), length);
}