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
  Copyright (C) Wallix 2010
  Author(s): Christophe Grosjean, Raphael Zhou

  rdp channel maanger
*/

#pragma once

class RDPChannelManagerMod
{
    FrontAPI& front;

public:
    std::unique_ptr<VirtualChannelDataSender>   file_system_to_client_sender;
    std::unique_ptr<VirtualChannelDataSender>   file_system_to_server_sender;

    std::unique_ptr<FileSystemVirtualChannel>   file_system_virtual_channel;

    std::unique_ptr<VirtualChannelDataSender>   clipboard_to_client_sender;
    std::unique_ptr<VirtualChannelDataSender>   clipboard_to_server_sender;

    std::unique_ptr<ClipboardVirtualChannel>    clipboard_virtual_channel;

    std::unique_ptr<VirtualChannelDataSender>   session_probe_to_server_sender;

    std::unique_ptr<SessionProbeVirtualChannel> session_probe_virtual_channel;

    FileSystemDriveManager file_system_drive_manager;

    class ToClientSender : public VirtualChannelDataSender
    {
        FrontAPI& front;

        const CHANNELS::ChannelDef& channel;

        uint32_t verbose;

    public:
        ToClientSender(FrontAPI& front,
                       const CHANNELS::ChannelDef& channel,
                       uint32_t verbose)
        : front(front)
        , channel(channel)
        , verbose(verbose) {}

        void operator()(uint32_t total_length, uint32_t flags,
            const uint8_t* chunk_data, uint32_t chunk_data_length)
                override
        {
            if ((this->verbose & MODRDP_LOGLEVEL_CLIPRDR_DUMP) ||
                (this->verbose & MODRDP_LOGLEVEL_RDPDR_DUMP)) {
                const bool send              = true;
                const bool from_or_to_client = true;
                ::msgdump_c(send, from_or_to_client, total_length, flags,
                    chunk_data, chunk_data_length);
            }

            this->front.send_to_channel(this->channel,
                chunk_data, total_length, chunk_data_length, flags);
        }
    };

    class ToServerSender : public VirtualChannelDataSender
    {
        Transport&      transport;
        CryptContext&   encrypt;
        int             encryption_level;
        uint16_t        user_id;
        uint16_t        channel_id;
        bool            show_protocol;

        uint32_t verbose;

    public:
        ToServerSender(Transport& transport,
                       CryptContext& encrypt,
                       int encryption_level,
                       uint16_t user_id,
                       uint16_t channel_id,
                       bool show_protocol,
                       uint32_t verbose)
        : transport(transport)
        , encrypt(encrypt)
        , encryption_level(encryption_level)
        , user_id(user_id)
        , channel_id(channel_id)
        , show_protocol(show_protocol)
        , verbose(verbose) {}

        void operator()(uint32_t total_length, uint32_t flags,
            const uint8_t* chunk_data, uint32_t chunk_data_length)
                override {
            CHANNELS::VirtualChannelPDU virtual_channel_pdu;

            if (this->show_protocol) {
                flags |= CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
            }

            if ((this->verbose & MODRDP_LOGLEVEL_CLIPRDR_DUMP) ||
                (this->verbose & MODRDP_LOGLEVEL_RDPDR_DUMP)) {
                const bool send              = true;
                const bool from_or_to_client = false;
                ::msgdump_c(send, from_or_to_client, total_length, flags,
                    chunk_data, chunk_data_length);
            }

            virtual_channel_pdu.send_to_server(this->transport,
                this->encrypt, this->encryption_level, this->user_id,
                this->channel_id, total_length, flags, chunk_data,
                chunk_data_length);
        }
    };

    RDPChannelManagerMod(FrontAPI& front)
    : front(front) {}

};  // RDPChannelManagerMod

