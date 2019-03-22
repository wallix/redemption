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
   Copyright (C) Wallix 2018
   Author(s): David Fort

   A proxy that will capture all the traffic to the target
*/

#pragma once

#include "core/RDP/nla/nla.hpp"
#include "transport/recorder_transport.hpp"

/// @brief Wrap a Transport and a RecorderFile for a nla negociation
struct NlaTeeTransport : Transport
{
    using PacketType = RecorderFile::PacketType;
    enum class Type : bool { Client, Server };

    NlaTeeTransport(Transport& trans, RecorderFile& outFile, Type type)
    : trans(trans)
    , outFile(outFile)
    , is_server(type == Type::Server)
    {}

    TlsResult enable_client_tls(
        bool server_cert_store,
        ServerCertCheck server_cert_check,
        ServerNotifier & server_notifier,
        const char * certif_path
    ) override
    {
        return this->trans.enable_client_tls(
            server_cert_store, server_cert_check, server_notifier, certif_path);
    }

    array_view_const_u8 get_public_key() const override
    {
        return this->trans.get_public_key();
    }

    bool disconnect() override
    {
        return this->trans.disconnect();
    }

    bool connect() override
    {
        return this->trans.connect();
    }

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override
    {
        auto r = this->trans.atomic_read(buffer, len);
        outFile.write_packet(
            this->is_server ? PacketType::NlaServerIn : PacketType::NlaClientIn,
            {buffer, len});
        return r;
    }

    size_t do_partial_read(uint8_t * buffer, size_t len) override
    {
        len = this->trans.partial_read(buffer, len);
        outFile.write_packet(
            this->is_server ? PacketType::NlaServerIn : PacketType::NlaClientIn,
            {buffer, len});
        return len;
    }

    void do_send(const uint8_t * buffer, size_t len) override
    {
        outFile.write_packet(
            this->is_server ? PacketType::NlaServerOut : PacketType::NlaClientOut,
            {buffer, len});
        this->trans.send(buffer, len);
    }

private:
    Transport& trans;
    RecorderFile& outFile;
    bool is_server;
};

