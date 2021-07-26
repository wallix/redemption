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
    Copyright (C) Wallix 2020
    Author(s): 
*/

#include <cassert>
#include <cinttypes>
#include <memory>
#include <string>

#include "core/channel_list.hpp"
#include "core/events.hpp"
#include "core/ERREF/ntstatus.hpp"
#include "core/RDP/channels/rdpdr.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "mod/rdp/channels/virtual_channel_data_sender.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "utils/stream.hpp"

#include "scard/scard_rdp_channel.hpp"


///////////////////////////////////////////////////////////////////////////////


enum
{
    common_type_header_version      = 0x01,
    common_type_header_endianness   = 0x10,
    common_type_header_length       = 8,
    common_type_header_filler       = 0xCCCCCCCC,
    private_type_header_length      = 8,
    private_type_header_filler      = 0x00000000,
    default_output_buffer_length    = 2048
};

scard_rdp_channel::scard_rdp_channel(
    EventContainer &/*events*/,
    VirtualChannelDataSender *sender,
    scard_rdp_channel_handler *handler_ptr,
    RDPVerbose verbosity)
    :
    BaseVirtualChannel(sender, nullptr),
    _client_id(0),
    _device_id_ptr(nullptr),
    _output_buffer_length(default_output_buffer_length),
    _verbosity(verbosity),
    _handler_ptr(handler_ptr)
{
}

void scard_rdp_channel::establish()
{
    announce();
}

void scard_rdp_channel::send(scard_wrapped_call data)
{
    send_device_io_request(data.io_control_code, data.data);
}

void scard_rdp_channel::process_client_message(uint32_t total_length,
    uint32_t flags, bytes_view chunk_data)
{
    LOG_IF(bool(_verbosity & RDPVerbose::rdpdr), LOG_INFO,
        "scard_rdp_channel::process_client_message:"
        " total_length=%" PRIu32 " flags=0x%08X chunk_data_length=%zu",
        total_length, flags, chunk_data.size());

    if (bool(_verbosity & RDPVerbose::rdpdr_dump))
    {
        ::msgdump_c(false, true, total_length, flags, chunk_data);
    }

    InStream chunk(chunk_data);

    // receive packet header
    if (flags & CHANNELS::CHANNEL_FLAG_FIRST)
    {
        _packet_header.receive(chunk);
    }

    // receive packet body
    switch (_packet_header.packet_id)
    {
        case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
        {
            process_client_announce_reply(chunk, flags);
        } break;

        case rdpdr::PacketId::PAKID_CORE_CLIENT_NAME:
        {
            process_client_name_request(chunk, flags);
        } break;

        case rdpdr::PacketId::PAKID_CORE_CLIENT_CAPABILITY:
        {
            process_client_core_capability_response(chunk, flags);
        } break;

        case rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE:
        {
            process_client_device_list_announce_request(chunk, flags);
        } break;

        case rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION:
        {
            process_device_control_response(chunk, flags);
        } break;

        default:
            break;
    }
}

void scard_rdp_channel::process_server_message(uint32_t /*total_length*/,
    uint32_t /*flags*/, bytes_view /*chunk_data*/)
{
}

void scard_rdp_channel::announce()
{
    StaticOutStream<64> stream;

    rdpdr::SharedHeader header(
        rdpdr::Component::RDPDR_CTYP_CORE,
        rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE);
    header.emit(stream);

    rdpdr::ServerAnnounceRequest announce_request(
        0x0001,     // VersionMajor
        0x000D,     // VersionMinor
        9           // ClientID
    );
    announce_request.emit(stream);

    // send message to client
    send_message_to_client(stream.get_offset(),
        CHANNELS::CHANNEL_FLAG_FIRST|CHANNELS::CHANNEL_FLAG_LAST,
        stream.get_produced_bytes());
}

void scard_rdp_channel::process_client_announce_reply(InStream &chunk, uint32_t flags)
{
    assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
        (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

    if(bool(_verbosity & RDPVerbose::rdpdr))
    {
        LOG(LOG_INFO,
            "scard_rdp_channel::process_client_announce_reply: "
            "Client Announce Reply");
    }

    rdpdr::ClientAnnounceReply client_announce_reply;

    // receive packet body
    client_announce_reply.receive(chunk);

    if (bool(_verbosity & RDPVerbose::rdpdr_dump))
    {
        client_announce_reply.log(LOG_INFO);
    }

    // save ClientId for later use
    _client_id = client_announce_reply.ClientId;
}

void scard_rdp_channel::process_client_name_request(InStream &chunk, uint32_t flags)
{
    assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
        (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

    if (bool(_verbosity & RDPVerbose::rdpdr))
    {
        LOG(LOG_INFO,
            "scard_rdp_channel::process_client_name_request: "
                "Client Name Request");
    }

    rdpdr::ClientNameRequest client_name_request;

    // receive packet body
    client_name_request.receive(chunk);

    if (bool(_verbosity & RDPVerbose::rdpdr_dump))
    {
        client_name_request.log(LOG_INFO);
    }

    // send Server Core Capability Request message
    send_server_core_capability_request();

    // send Server Client ID Confirm message
    send_server_client_id_confirm();
}

void scard_rdp_channel::process_client_core_capability_response(InStream &chunk, uint32_t flags)
{
    assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
        (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));
    
    // numCapabilities(2) + Padding(2)
    ::check_throw(chunk, 4,
        "scard_rdp_channel::process_client_core_capability_response:DR_CORE_CAPABILITY_RSP (1)",
        ERR_RDP_DATA_TRUNCATED);

    const uint16_t num_capabilities = chunk.in_uint16_le();

    if (bool(_verbosity & RDPVerbose::rdpdr))
    {
        LOG(LOG_INFO,
            "scard_rdp_channel::process_client_core_capability_response: "
            "numCapabilities=%u", num_capabilities);
    }
}

void scard_rdp_channel::process_client_device_list_announce_request(
    InStream &chunk, uint32_t flags)
{
    assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
        (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

    
    ::check_throw(chunk, 4,
        "scard_rdp_channel::process_client_device_list_announce_request",
        ERR_RDP_DATA_TRUNCATED);

    // DeviceCount (4 bytes)
    uint32_t device_count = chunk.in_uint32_le();

    while (device_count)
    {
        // DeviceAnnounceHeader (var. bytes)
        rdpdr::DeviceAnnounceHeader_Recv device_announce_header;
        device_announce_header.receive(chunk);

        const auto device_type = device_announce_header.DeviceType();

        // skip non-smartcard device
        if (device_type != rdpdr::RDPDR_DTYP_SMARTCARD)
        {
            continue;
        }

        const auto device_id = device_announce_header.DeviceId();

        // save smartcard device ID for later use
        // note: there is only one device ID for ALL smartcard devices
        _device_id_ptr = std::make_unique<uint32_t>(device_id);

        const auto device_name = device_announce_header.PreferredDosName();

        LOG(LOG_INFO,
            "scard_rdp_channel::process_client_device_list_announce_request: "
            "smartcard device '%s' announced", device_name);

        // send Server Device Announce Response messages
        send_server_device_announce_response();

        --device_count;
    }

    // notify establishment
    if (_handler_ptr)
    {
        _handler_ptr->handle_rdp_channel_establishment();
    }
}

void scard_rdp_channel::send_server_core_capability_request()
{
    StaticOutStream<1024> stream;

    rdpdr::SharedHeader header(
        rdpdr::Component::RDPDR_CTYP_CORE,
        rdpdr::PacketId::PAKID_CORE_SERVER_CAPABILITY);
    header.emit(stream);

    rdpdr::ServerCoreCapabilityRequest server_core_capability_request(1);
    server_core_capability_request.emit(stream);

    const uint32_t general_capability_version = rdpdr::GENERAL_CAPABILITY_VERSION_02;
    stream.out_uint16_le(rdpdr::CAP_GENERAL_TYPE);
    stream.out_uint16_le(
        rdpdr::GeneralCapabilitySet::size(general_capability_version) +
        8   // CapabilityType(2) + CapabilityLength(2) + Version(4)
    );
    stream.out_uint32_le(general_capability_version);

    rdpdr::GeneralCapabilitySet general_capability_set(
        0x2,        // osType
        // 0x50001,                               // osVersion
        // 0x1,                                   // protocolMajorVersion
        0xc,                                    //this->serverVersionMinor,         // protocolMinorVersion -
                    //     RDP Client 6.0 and 6.1
        0xFFFF,     // ioCode1
        // 0x0,                                   // ioCode2
        0x7,        // extendedPDU -
                    //     RDPDR_DEVICE_REMOVE_PDUS(1) |
                    //     RDPDR_CLIENT_DISPLAY_NAME_PDU(2) |
                    //     RDPDR_USER_LOGGEDON_PDU(4)
        0x0,        // extraFlags1
        // 0x0,                                   // extraFlags2
        0,                                      // SpecialTypeDeviceCap
        general_capability_version
    );
    if (bool(_verbosity & RDPVerbose::rdpdr))
    {
        LOG(LOG_INFO,
            "scard_rdp_channel::send_server_core_capability_request: "
        );
        general_capability_set.log(LOG_INFO);
    }
    general_capability_set.emit(stream);

    // Smart card capability set
    stream.out_uint16_le(rdpdr::CAP_SMARTCARD_TYPE);
    stream.out_uint16_le(
        8   // CapabilityType(2) + CapabilityLength(2) +
            //     Version(4)
    );

    // send message
    send_message_to_client(stream.get_offset(),
        CHANNELS::CHANNEL_FLAG_FIRST|CHANNELS::CHANNEL_FLAG_LAST,
        stream.get_produced_bytes());
}

void scard_rdp_channel::send_server_client_id_confirm()
{
    StaticOutStream<1024> stream;

    rdpdr::SharedHeader header(
        rdpdr::Component::RDPDR_CTYP_CORE,
        rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
    header.emit(stream);
    
    stream.out_uint16_le(0x0001);   // VersionMajor (must be set to 0x0001)
    stream.out_uint16_le(0x000C);   // VersionMinor (must be set to either 0x0002, 0x0005, 0x000A, 0x000C, 0x000D)
    stream.out_uint32_le(_client_id);

    // send message to client
    send_message_to_client(stream.get_offset(),
        CHANNELS::CHANNEL_FLAG_FIRST|CHANNELS::CHANNEL_FLAG_LAST,
        stream.get_produced_bytes());
}

void scard_rdp_channel::send_server_device_announce_response()
{
    assert(_device_id_ptr);

    StaticOutStream<1024> stream;

    rdpdr::SharedHeader header(
        rdpdr::Component::RDPDR_CTYP_CORE,
        rdpdr::PacketId::PAKID_CORE_DEVICE_REPLY);
    header.emit(stream);

    rdpdr::ServerDeviceAnnounceResponse server_device_announce_response(
        *_device_id_ptr, erref::NTSTATUS::STATUS_SUCCESS);
    server_device_announce_response.emit(stream);

    // send message to client
    send_message_to_client(stream.get_offset(),
        CHANNELS::CHANNEL_FLAG_FIRST|CHANNELS::CHANNEL_FLAG_LAST,
        stream.get_produced_bytes());
}

void scard_rdp_channel::send_device_io_request(uint32_t io_control_code, bytes_view data)
{
    assert(_device_id_ptr);

    StaticOutStream<1024> stream;

    // encode header
    {
        rdpdr::SharedHeader header(
            rdpdr::Component::RDPDR_CTYP_CORE,
            rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
        
        // do encode
        header.emit(stream); // 4 bytes
    }

    // encode Device IO Request
    {
        const uint32_t device_id = *_device_id_ptr;
        const uint32_t file_id = 0;
        const uint32_t completion_id = _io_completion_ids.allocate();
        const uint32_t major_function = rdpdr::IRP_MJ_DEVICE_CONTROL;
        const uint32_t minor_function = 0x00000000; // must be set to this fixed value

        rdpdr::DeviceIORequest device_io_request(
            device_id, file_id, completion_id,
            major_function, minor_function
        );

        // do encode
        device_io_request.emit(stream); // 20 bytes

        // store the request
        const auto result =_io_requests.emplace(completion_id, io_control_code);

        assert(result.second);
    }

    // OutputBufferLength (4 bytes)
    // 0x0800 is the recommended value (see [MS-RDPESC] §3.2.5.1)
    stream.out_uint32_le(_output_buffer_length);

    const uint32_t input_buffer_length = (
        common_type_header_length +     // CommonTypeHeader
        private_type_header_length +    // PrivateTypeHeader
        data.size()                     // ObjectBuffer
    );

    // InputBufferLength (4 bytes)
    stream.out_uint32_le(input_buffer_length);

    // IoControlCode (4 bytes)
    stream.out_uint32_le(io_control_code);

    // Padding (20 bytes)
    stream.out_skip_bytes(20);

    /* InputBuffer */

    /* CommonTypeHeader (8 bytes) */

    // Version (1 byte)
    stream.out_uint8(common_type_header_version);

    // Endianness (1 byte)
    stream.out_uint8(common_type_header_endianness);

    // CommonHeaderLength (2 bytes)
    stream.out_uint16_le(common_type_header_length);

    // Filler (4 bytes)
    stream.out_uint32_le(common_type_header_filler);

    /* PrivateTypeHeader (8 bytes + padding size) */

    // ObjectBufferLength (4 bytes)
    stream.out_uint32_le(data.size());

    // Filler (4 bytes)
    stream.out_uint32_le(private_type_header_filler);

    /* Payload */
    stream.out_copy_bytes(data);

    // send message to client
    send_message_to_client(stream.get_offset(),
        CHANNELS::CHANNEL_FLAG_FIRST|CHANNELS::CHANNEL_FLAG_LAST,
        stream.get_produced_bytes());
}

void scard_rdp_channel::process_device_control_response(InStream &chunk, uint32_t flags)
{
    assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
        (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));
    assert(_device_id_ptr);

    if (bool(_verbosity & RDPVerbose::rdpdr))
    {
        LOG(LOG_INFO,
            "scard_channel::process_device_control_response: "
                "Device Control Response");
    }

    uint32_t io_control_code;

    // receive and check Device IO Response
    {
        rdpdr::DeviceIOResponse device_io_response;
        device_io_response.receive(chunk);

        // check device ID
        {
            const uint32_t device_id = device_io_response.DeviceId();

            // ensure the device ID matches the advertised one
            if (device_id != *_device_id_ptr)
            {
                LOG(LOG_ERR,
                    "scard_channel::process_device_control_response: "
                        "unknown device ID %" PRIu32, device_id);
                
                return;
            }
        }

        // check completion ID and get the corresponding IO control code
        {
            const uint32_t completion_id = device_io_response.CompletionId();

            // ensure a request associated to this completion ID exists
            auto it = _io_requests.find(completion_id);
            if (it == _io_requests.end())
            {
                LOG(LOG_ERR,
                    "scard_channel::process_device_control_response: "
                        "unknown request with completion ID %" PRIu32, completion_id);

                return;
            }

            // save IO control code
            io_control_code = it->second;

            // remove request
            _io_requests.erase(it);
        }

        // check IO status
        {
            const auto io_status = device_io_response.IoStatus();

            if (io_status == erref::NTSTATUS::STATUS_BUFFER_TOO_SMALL)
            {
                LOG(LOG_WARNING,
                    "scard_channel::process_device_control_response: "
                        "unexpected IO status 0x08%" PRIX32, io_status);

                // TODO Double output buffer size and re-send previous message

                return;
            }
            if (io_status != erref::NTSTATUS::STATUS_SUCCESS)
            {
                LOG(LOG_ERR,
                    "scard_channel::process_device_control_response: "
                        "unexpected IO status 0x08%" PRIX32, io_status);

                return;
            }
        }
    }

    // OutputBufferLength
    const auto output_buffer_length = chunk.in_uint32_le();

    ::check_throw(chunk, output_buffer_length,
        "scard_channel::process_device_control_response: ",
        ERR_RDP_DATA_TRUNCATED);
    
    /* CommonTypeHeader (8 bytes) */

    // Version (1 byte)
    const auto version = chunk.in_uint8();

    assert(version == common_type_header_version);

    // Endianness (1 byte)
    const auto endianness = chunk.in_uint8();

    assert(endianness == common_type_header_endianness);

    // CommonHeaderLength (2 bytes)
    const auto length = chunk.in_uint16_le();

    assert(length == common_type_header_length);

    // Filler (4 bytes)
    chunk.in_skip_bytes(4);

    /* PrivateTypeHeader (8 bytes) */

    // ObjectBufferLength (4 bytes)
    const auto object_buffer_length = chunk.in_uint32_le();

    // Filler (4 bytes)
    chunk.in_skip_bytes(4);

    // defer handling of data
    if (_handler_ptr)
    {
        scard_wrapped_return wrapped_return(io_control_code,
            bytes_view(chunk.get_current(), object_buffer_length)
        );

        _handler_ptr->handle_rdp_channel_data(
            std::move(wrapped_return));
    }
}

///
scard_rdp_channel_sender::scard_rdp_channel_sender(FrontAPI &front,
    const CHANNELS::ChannelDef &channel_definition, bool verbose) noexcept
    :
    _front(front),
    _channel_definition(channel_definition),
    _verbose(verbose)
{
}

void scard_rdp_channel_sender::operator()(
    uint32_t total_length, uint32_t flags,
    bytes_view chunk_data)
{
    if (_verbose)
    {
        const bool send = true;
        const bool from_or_to_client = true;

        ::msgdump_c(send, from_or_to_client,
            total_length, flags, chunk_data);
    }

    _front.send_to_channel(_channel_definition,
        chunk_data, total_length, flags);
}