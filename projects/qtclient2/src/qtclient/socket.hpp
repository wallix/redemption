/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "transport/socket_transport.hpp"


namespace qtclient
{

struct SocketIO final : SocketTransport
{
    using SocketTransport::SocketTransport;

    size_t do_partial_read(uint8_t * buffer, size_t len) override;
};

}
