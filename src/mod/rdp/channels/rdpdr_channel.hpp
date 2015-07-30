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
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef REDEMPTION_MOD_RDP_CHANNELS_RDPDRCHANNEL_HPP
#define REDEMPTION_MOD_RDP_CHANNELS_RDPDRCHANNEL_HPP

#include "apply_for_delim.hpp"
#include "base_channel.hpp"
#include "rdpdr_file_system_drive_manager.hpp"

class FileSystemVirtualChannel : public BaseVirtualChannel
{
public:
    VirtualChannelDataSender& to_server_sender;

private:
    rdpdr::SharedHeader client_message_header;
    rdpdr::SharedHeader server_message_header;

    rdpdr::DeviceIORequest server_device_io_request;

    FileSystemDriveManager& file_system_drive_manager;

    // DeviceId, CompletionId, MajorFunction, (extra data).
    typedef std::vector<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>>
        device_io_request_info_inventory_type;
    device_io_request_info_inventory_type device_io_request_info_inventory;

    bool device_capability_version_02_supported = false;

    const char* const param_client_name;
    const bool        param_file_system_read_authorized;
    const bool        param_file_system_write_authorized;
    const uint32_t    param_random_number;                  // For ClientId.

    bool proxy_managed_drives_announced = false;

    class ClientDeviceListAnnounceRequestProcessor {
        VirtualChannelDataSender* to_client_sender;
        VirtualChannelDataSender* to_server_sender;

        const bool param_file_system_authorized;
        const bool param_parallel_port_authorized;
        const bool param_print_authorized;
        const bool param_serial_port_authorized;
        const bool param_smart_card_authorized;

        uint32_t DeviceCount = 0;

        uint32_t length_of_remaining_device_data_to_be_processed = 0;
        uint32_t length_of_remaining_device_data_to_be_skipped   = 0;

        bool has_device_announced = false;

        uint8_t         chunked_virtual_channel_data[
            CHANNELS::MAX_CHANNEL_CHUNK_LENGTH];
        WriteOnlyStream chunked_virtual_channel_stream;

        uint32_t total_virtual_channel_data_length = 0;
        uint32_t channel_control_flags             = 0;

        const uint32_t verbose;

    public:
        ClientDeviceListAnnounceRequestProcessor(
            VirtualChannelDataSender* to_client_sender_,
            VirtualChannelDataSender* to_server_sender_,
            bool file_system_authorized,
            bool parallel_port_authorized,
            bool print_authorized,
            bool serial_port_authorized,
            bool smart_card_authorized,
            uint32_t channel_chunk_length,
            uint32_t verbose)
        : to_client_sender(to_client_sender_)
        , to_server_sender(to_server_sender_)
        , param_file_system_authorized(file_system_authorized)
        , param_parallel_port_authorized(parallel_port_authorized)
        , param_print_authorized(print_authorized)
        , param_serial_port_authorized(serial_port_authorized)
        , param_smart_card_authorized(smart_card_authorized)
        , chunked_virtual_channel_stream(
              chunked_virtual_channel_data,
              std::min<uint32_t>(sizeof(chunked_virtual_channel_data),
                                 channel_chunk_length))
        , verbose(verbose) {}

        void operator()(uint32_t total_length, uint32_t flags, Stream& chunk)
        {
            if (flags & CHANNELS::CHANNEL_FLAG_FIRST)
            {
                REDASSERT(!this->DeviceCount);
                REDASSERT(
                    !this->length_of_remaining_device_data_to_be_processed);
                REDASSERT(
                    !this->length_of_remaining_device_data_to_be_skipped);
                REDASSERT(!this->chunked_virtual_channel_stream.get_offset());
                REDASSERT(!this->total_virtual_channel_data_length);
                REDASSERT(!this->has_device_announced);

                this->DeviceCount = chunk.in_uint32_le();

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::ClientDeviceListAnnounceRequestProcessor::(): "
                            "DeviceCount=%u",
                        DeviceCount);
                }
            }

            while (chunk.in_remain())
            {
                if (!this->length_of_remaining_device_data_to_be_processed &&
                    !this->length_of_remaining_device_data_to_be_skipped) {

                    const uint32_t DeviceType = chunk.in_uint32_le();
                    const uint32_t DeviceId   = chunk.in_uint32_le();

                    uint8_t PreferredDosName[
                              8 // PreferredDosName(8)
                            + 1
                        ];

                    chunk.in_copy_bytes(PreferredDosName,
                                        8   // PreferredDosName(8)
                                       );
                    PreferredDosName[8  // PreferredDosName(8)
                        ] = '\0';

                    uint32_t DeviceDataLength = chunk.in_uint32_le();

                    if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                        LOG(LOG_INFO,
                            "FileSystemVirtualChannel::ClientDeviceListAnnounceRequestProcessor::(): "
                                "DeviceType=%s(%u) DeviceId=%u "
                                "PreferredDosName=\"%s\" DeviceDataLength=%u",
                            rdpdr::DeviceAnnounceHeader::get_DeviceType_name(
                                DeviceType),
                            DeviceType, DeviceId, PreferredDosName,
                            DeviceDataLength);
                    }

                    REDASSERT(
                        !this->chunked_virtual_channel_stream.get_offset());
                    REDASSERT(!this->total_virtual_channel_data_length);

                    if (((DeviceType == rdpdr::RDPDR_DTYP_FILESYSTEM) &&
                         this->param_file_system_authorized) ||
                        ((DeviceType == rdpdr::RDPDR_DTYP_PARALLEL) &&
                         this->param_parallel_port_authorized) ||
                        ((DeviceType == rdpdr::RDPDR_DTYP_PRINT) &&
                         this->param_print_authorized) ||
                        ((DeviceType == rdpdr::RDPDR_DTYP_SERIAL) &&
                         this->param_serial_port_authorized) ||
                        ((DeviceType == rdpdr::RDPDR_DTYP_SMARTCARD) &&
                         this->param_smart_card_authorized))
                    {
                        this->length_of_remaining_device_data_to_be_processed =
                            DeviceDataLength;
                        this->length_of_remaining_device_data_to_be_skipped   = 0;

                        this->total_virtual_channel_data_length =
                              rdpdr::SharedHeader::size()
                            + 24    // DeviceCount(4)+ DeviceType(4) +
                                    //     DeviceId(4) + PreferredDosName(8) +
                                    //     DeviceDataLength(4)
                            + DeviceDataLength;
                        this->channel_control_flags             =
                            CHANNELS::CHANNEL_FLAG_FIRST;

                        rdpdr::SharedHeader client_message_header(
                            rdpdr::Component::RDPDR_CTYP_CORE,
                            rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE);

                        client_message_header.emit(
                            this->chunked_virtual_channel_stream);

                        this->chunked_virtual_channel_stream.out_uint32_le(
                             1  // DeviceCount(4)
                            );

                        this->chunked_virtual_channel_stream.out_uint32_le(
                            DeviceType);        // DeviceType(4)
                        this->chunked_virtual_channel_stream.out_uint32_le(
                            DeviceId);          // DeviceId(4)
                        this->chunked_virtual_channel_stream.out_copy_bytes(
                             PreferredDosName,
                             8                  // PreferredDosName(8)
                            );
                        this->chunked_virtual_channel_stream.out_uint32_le(
                            DeviceDataLength);  // DeviceDataLength(4)
                    }
                    else
                    {
                        this->length_of_remaining_device_data_to_be_processed = 0;
                        this->length_of_remaining_device_data_to_be_skipped   =
                            DeviceDataLength;

                        rdpdr::SharedHeader server_message_header(
                            rdpdr::Component::RDPDR_CTYP_CORE,
                            rdpdr::PacketId::PAKID_CORE_DEVICE_REPLY);

                        server_message_header.emit(
                            this->chunked_virtual_channel_stream);

                        rdpdr::ServerDeviceAnnounceResponse
                            server_device_announce_response(
                                    DeviceId,
                                    0xC0000001  // STATUS_UNSUCCESSFUL
                                );

                        server_device_announce_response.emit(
                            this->chunked_virtual_channel_stream);

                        if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                            LOG(LOG_INFO,
                                "FileSystemVirtualChannel::ClientDeviceListAnnounceRequestProcessor::(): "
                                    "Server Device Announce Response");
                            server_device_announce_response.log(LOG_INFO);
                        }

                        this->chunked_virtual_channel_stream.mark_end();

                        REDASSERT(this->to_client_sender);

                        (*this->to_client_sender)(
                            this->chunked_virtual_channel_stream.size(),
                            CHANNELS::CHANNEL_FLAG_FIRST |
                                CHANNELS::CHANNEL_FLAG_LAST,
                            this->chunked_virtual_channel_data,
                            this->chunked_virtual_channel_stream.size());

                        this->chunked_virtual_channel_stream.reset();
                    }
                }   // if (!this->length_of_remaining_device_data_to_be_processed &&

                if (this->chunked_virtual_channel_stream.get_offset() ||
                    this->length_of_remaining_device_data_to_be_processed) {
                    uint32_t length_of_device_data_can_be_processed =
                        std::min<uint32_t>(
                            std::min<uint32_t>(
                                this->length_of_remaining_device_data_to_be_processed,
                                chunk.in_remain()),
                            this->chunked_virtual_channel_stream.tailroom());

                    this->chunked_virtual_channel_stream.out_copy_bytes(
                        chunk.p, length_of_device_data_can_be_processed);
                    this->chunked_virtual_channel_stream.mark_end();

                    chunk.in_skip_bytes(
                        length_of_device_data_can_be_processed);
                    this->length_of_remaining_device_data_to_be_processed -=
                        length_of_device_data_can_be_processed;

                    if (!this->length_of_remaining_device_data_to_be_processed ||
                        (this->chunked_virtual_channel_stream.size() ==
                         this->chunked_virtual_channel_stream.get_capacity()))
                    {
                        if (!this->length_of_remaining_device_data_to_be_processed) {
                            this->channel_control_flags |=
                                CHANNELS::CHANNEL_FLAG_LAST;
                        }

                        REDASSERT(this->to_server_sender);

                        (*this->to_server_sender)(
                            this->total_virtual_channel_data_length,
                            this->channel_control_flags,
                            this->chunked_virtual_channel_data,
                            this->chunked_virtual_channel_stream.size());

                        this->has_device_announced = true;

                        this->channel_control_flags &=
                            ~CHANNELS::CHANNEL_FLAG_FIRST;

                        this->chunked_virtual_channel_stream.reset();

                        this->total_virtual_channel_data_length = 0;
                    }
                }
                else if (this->length_of_remaining_device_data_to_be_skipped) {
                    const uint32_t length_of_device_data_can_be_skipped =
                        std::min<uint32_t>(
                            this->length_of_remaining_device_data_to_be_skipped,
                            chunk.in_remain());

                    chunk.in_skip_bytes(length_of_device_data_can_be_skipped);
                    this->length_of_remaining_device_data_to_be_skipped -=
                        length_of_device_data_can_be_skipped;
                }
            }   // while (chunk.in_remain())

            if (flags & CHANNELS::CHANNEL_FLAG_LAST)
            {
                if (!this->has_device_announced)
                {
                    REDASSERT(
                        !this->chunked_virtual_channel_stream.get_offset());

                    rdpdr::SharedHeader client_message_header(
                        rdpdr::Component::RDPDR_CTYP_CORE,
                        rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE);

                    client_message_header.emit(
                        this->chunked_virtual_channel_stream);

                    this->chunked_virtual_channel_stream.out_uint32_le(
                         0  // DeviceCount(4)
                        );
                    this->chunked_virtual_channel_stream.mark_end();

                    REDASSERT(this->to_server_sender);

//                    (*this->to_server_sender)(
//                        this->chunked_virtual_channel_stream.size(),
//                        CHANNELS::CHANNEL_FLAG_FIRST |
//                            CHANNELS::CHANNEL_FLAG_LAST,
//                        this->chunked_virtual_channel_data,
//                        this->chunked_virtual_channel_stream.size());

                    if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                        LOG(LOG_INFO,
                            "FileSystemVirtualChannel::ClientDeviceListAnnounceRequestProcessor::(): "
                                "Client Device List Announce Request. "
                                "DeviceCount=0");
                    }

                    this->chunked_virtual_channel_stream.reset();
                }
                else {
                    this->has_device_announced = false;
                }
            }
        }   // operator()
    } client_device_list_announce_request_processor;

public:
    struct Params : public BaseVirtualChannel::Params
    {
        const char* client_name;

        bool file_system_read_authorized;
        bool file_system_write_authorized;

        bool parallel_port_authorized;
        bool print_authorized;
        bool serial_port_authorized;
        bool smart_card_authorized;

        uint32_t random_number;
    };

    FileSystemVirtualChannel(
        VirtualChannelDataSender* to_client_sender_,
        VirtualChannelDataSender* to_server_sender_,
        FileSystemDriveManager& file_system_drive_manager,
        const Params& params)
    : BaseVirtualChannel(to_client_sender_,
                         to_server_sender_,
                         params)
    , to_server_sender(*to_server_sender_)
    , file_system_drive_manager(file_system_drive_manager)
    , param_client_name(params.client_name)
    , param_file_system_read_authorized(params.file_system_read_authorized)
    , param_file_system_write_authorized(params.file_system_write_authorized)
    , param_random_number(params.random_number)
    , client_device_list_announce_request_processor(
          to_client_sender_,
          to_server_sender_,
          (params.file_system_read_authorized ||
           params.file_system_write_authorized),
          params.parallel_port_authorized,
          params.print_authorized,
          params.serial_port_authorized,
          params.smart_card_authorized,
          CHANNELS::CHANNEL_CHUNK_LENGTH,
          params.verbose)
    {}

    virtual ~FileSystemVirtualChannel()
    {
        if (!this->device_io_request_info_inventory.empty())
        {
            bool make_boom_in_debug_mode = false;

            for (device_io_request_info_inventory_type::iterator iter =
                     this->device_io_request_info_inventory.begin();
                 iter != this->device_io_request_info_inventory.end();
                 ++iter)
            {
                if (std::get<2>(*iter) != rdpdr::IRP_MJ_DIRECTORY_CONTROL)
                {
                    LOG(LOG_WARNING,
                        "FileSystemVirtualChannel::~FileSystemVirtualChannel: "
                            "There is Device I/O request information "
                            "remaining in inventory. "
                            "DeviceId=%u CompletionId=%u MajorFunction=%u "
                            "extra_data=%u",
                        std::get<0>(*iter), std::get<1>(*iter),
                        std::get<2>(*iter), std::get<3>(*iter));

                    make_boom_in_debug_mode = true;
                }
            }

            REDASSERT(!make_boom_in_debug_mode);
        }
    }

    inline virtual const char*
        get_reporting_reason_exchanged_data_limit_reached() const override
    {
        return "RDPDR_LIMIT";
    }

    void process_client_general_capability_set(uint32_t total_length,
            uint32_t flags, Stream& chunk, uint32_t Version) {
        rdpdr::GeneralCapabilitySet general_capability_set;

        uint8_t* const saved_general_capability_set_p = chunk.p;

        general_capability_set.receive(chunk, Version);

        const bool need_enable_user_loggedon_pdu =
            (!(general_capability_set.extendedPDU() &
               rdpdr::RDPDR_USER_LOGGEDON_PDU));

        const bool need_deny_asyncio =
            (general_capability_set.extraFlags1() & rdpdr::ENABLE_ASYNCIO);

        if ((this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) ||
            need_enable_user_loggedon_pdu || need_deny_asyncio) {
            LOG(LOG_INFO,
                "FileSystemVirtualChannel::process_client_general_capability_set: ");
            general_capability_set.log(LOG_INFO);
        }

        if (need_enable_user_loggedon_pdu || need_deny_asyncio) {
            WriteOnlyStream out_stream(
                saved_general_capability_set_p,
                rdpdr::GeneralCapabilitySet::size(Version));

            if (need_enable_user_loggedon_pdu) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_client_general_capability_set: "
                        "Allow the server to send a "
                        "Server User Logged On packet.");

                general_capability_set.set_extendedPDU(
                    general_capability_set.extendedPDU() |
                    rdpdr::RDPDR_USER_LOGGEDON_PDU);
            }

            if (need_deny_asyncio) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_client_general_capability_set: "
                        "Deny user to send multiple simultaneous "
                        "read or write requests on the same file from "
                        "a redirected file system.");

                general_capability_set.set_extraFlags1(
                    general_capability_set.extraFlags1() &
                    ~rdpdr::ENABLE_ASYNCIO);
            }

            general_capability_set.emit(out_stream, Version);

            chunk.p = saved_general_capability_set_p;

            general_capability_set.receive(chunk, Version);

            if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                general_capability_set.log(LOG_INFO);
            }
        }
    }   // process_client_general_capability_set

    bool process_client_core_capability_response(
        uint32_t total_length, uint32_t flags, Stream& chunk)
    {
        REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
            (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

        const uint16_t numCapabilities = chunk.in_uint16_le();
        if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
            LOG(LOG_INFO,
                "FileSystemVirtualChannel::process_client_core_capability_response: "
                    "numCapabilities=%u", numCapabilities);
        }

        chunk.in_skip_bytes(2); // Padding(2)

        for (uint16_t idx_capabilities = 0; idx_capabilities < numCapabilities;
             ++idx_capabilities) {
            const uint16_t CapabilityType   = chunk.in_uint16_le();
            const uint16_t CapabilityLength = chunk.in_uint16_le();
            const uint32_t Version          = chunk.in_uint32_le();

            if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_client_core_capability_response: "
                        "CapabilityType=0x%04X CapabilityLength=%u "
                        "Version=0x%X",
                    CapabilityType, CapabilityLength, Version);
            }

            switch (CapabilityType) {
                case rdpdr::CAP_GENERAL_TYPE:
                {
                    this->process_client_general_capability_set(total_length,
                        flags, chunk, Version);
                }
                break;

                default:
                    chunk.in_skip_bytes(CapabilityLength -
                        8 /* CapabilityType(2) + CapabilityLength(2) + Version(4) */);
                break;
            }

            if ((CapabilityType == rdpdr::CAP_DRIVE_TYPE) &&
                (Version == rdpdr::DRIVE_CAPABILITY_VERSION_02)) {
                this->device_capability_version_02_supported = true;
                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_core_capability_response: "
                            "Client supports DRIVE_CAPABILITY_VERSION_02.");
                }
            }
        }

        return true;
    }   // process_client_core_capability_response

    bool process_client_device_list_announce_request(uint32_t total_length,
        uint32_t flags, Stream& chunk)
    {
        this->client_device_list_announce_request_processor(total_length,
            flags, chunk);

        return false;
    }

    bool process_client_drive_query_information_response(
        uint32_t total_length, uint32_t flags, Stream& chunk,
        uint32_t FsInformationClass)
    {
        switch (FsInformationClass) {
            case rdpdr::FileBasicInformation:
            {
                chunk.in_skip_bytes(4); // Length(4)

                fscc::FileBasicInformation file_basic_information;

                file_basic_information.receive(chunk);
                file_basic_information.log(LOG_INFO);
            }
            break;

            default:
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_client_drive_query_information_response: "
                        "Undecoded FsInformationClass - %s(0x%X)",
                    rdpdr::ServerDriveQueryInformationRequest::get_FsInformationClass_name(
                        FsInformationClass),
                    FsInformationClass);
            break;
        }

        return true;
    }

    bool process_client_drive_query_volume_information_response(
        uint32_t total_length, uint32_t flags, Stream& chunk,
        uint32_t FsInformationClass)
    {
        switch (FsInformationClass) {
            case rdpdr::FileFsAttributeInformation:
            {
                chunk.in_skip_bytes(4); // Length(4)

                fscc::FileFsAttributeInformation
                    file_fs_Attribute_information;

                file_fs_Attribute_information.receive(chunk);
                file_fs_Attribute_information.log(LOG_INFO);
            }
            break;

            default:
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_client_drive_query_volume_information_response: "
                        "Undecoded FsInformationClass - %s(0x%X)",
                    rdpdr::ServerDriveQueryVolumeInformationRequest::get_FsInformationClass_name(
                        FsInformationClass),
                    FsInformationClass);
            break;
        }

        return true;
    }

    bool process_client_drive_io_response(uint32_t total_length,
        uint32_t flags, Stream& chunk)
    {
        REDASSERT(flags & CHANNELS::CHANNEL_FLAG_FIRST);
        REDASSERT(this->get_verbose() & MODRDP_LOGLEVEL_RDPDR);

        rdpdr::DeviceIOResponse device_io_response;

        device_io_response.receive(chunk);
        device_io_response.log(LOG_INFO);

        device_io_request_info_inventory_type::iterator iter;
        for (iter = this->device_io_request_info_inventory.begin();
             (iter != this->device_io_request_info_inventory.end()) &&
             ((std::get<0>(*iter) != device_io_response.DeviceId()) ||
              (std::get<1>(*iter) != device_io_response.CompletionId()));
             ++iter);

        if (iter == this->device_io_request_info_inventory.end()) {
            LOG(LOG_WARNING,
                "FileSystemVirtualChannel::process_client_drive_io_response: "
                    "The corresponding "
                    "Server Drive I/O Request is not found! "
                    "DeviceId=%u CompletionId=%u",
                device_io_response.DeviceId(),
                device_io_response.CompletionId());

            REDASSERT(false);

            return true;
        }

        uint32_t major_function = std::get<2>(*iter);
        uint32_t extra_data     = std::get<3>(*iter);

        LOG(LOG_INFO,
            "FileSystemVirtualChannel::process_client_drive_io_response: "
                "MajorFunction=%s(0x%08X) extra_data=0x%X",
            rdpdr::DeviceIORequest::get_MajorFunction_name(major_function),
            major_function, extra_data);

        this->device_io_request_info_inventory.erase(iter);

        switch (major_function)
        {
            case rdpdr::IRP_MJ_CREATE:
            {
                rdpdr::DeviceCreateResponse device_create_response;

                device_create_response.receive(chunk);
                device_create_response.log(LOG_INFO);
            }
            break;

            case rdpdr::IRP_MJ_READ:
            {
                uint32_t Length = chunk.in_uint32_le();

                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_client_drive_io_response: "
                        "Read request. Length=%u",
                    Length);

                this->update_exchanged_data(Length);
            }
            break;

            case rdpdr::IRP_MJ_QUERY_VOLUME_INFORMATION:
                this->process_client_drive_query_volume_information_response(
                    total_length, flags, chunk,
                    /* FsInformationClass = */extra_data);
            break;

            case rdpdr::IRP_MJ_QUERY_INFORMATION:
                this->process_client_drive_query_information_response(
                    total_length, flags, chunk,
                    /* FsInformationClass = */extra_data);
            break;

            default:
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_client_drive_io_response: "
                        "Undecoded Drive I/O Response - "
                        "MajorFunction=%s(0x%08X)",
                    rdpdr::DeviceIORequest::get_MajorFunction_name(
                        this->server_device_io_request.MajorFunction()),
                    this->server_device_io_request.MajorFunction());
            break;
        }   // switch (major_function)

        return true;
    }   // process_client_drive_io_response

    virtual void process_client_message(uint32_t total_length,
        uint32_t flags, uint8_t* chunk_data, uint32_t chunk_data_length)
            override
    {
        if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
            LOG(LOG_INFO,
                "FileSystemVirtualChannel::process_client_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR_DUMP) {
            LOG(LOG_INFO, "Recv done on rdpdr (-1) n bytes");
            const uint32_t dest = 0;    // Client
            hexdump_c(reinterpret_cast<const uint8_t*>(&dest),
                sizeof(dest));
            hexdump_c(reinterpret_cast<uint8_t*>(&total_length),
                sizeof(total_length));
            hexdump_c(reinterpret_cast<uint8_t*>(&flags), sizeof(flags));
            hexdump_c(reinterpret_cast<uint8_t*>(&chunk_data_length),
                sizeof(chunk_data_length));
            hexdump_c(chunk_data, chunk_data_length);
            LOG(LOG_INFO, "Dump done on rdpdr (-1) n bytes");
        }

        ReadOnlyStream chunk(chunk_data, chunk_data_length);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->client_message_header.receive(chunk);
        }

        bool send_message_to_server = true;

        switch (this->client_message_header.packet_id)
        {
            case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
                REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_message: "
                            "Client Announce Reply");

                    rdpdr::ClientAnnounceReply client_announce_reply;

                    client_announce_reply.receive(chunk);
                    client_announce_reply.log(LOG_INFO);
                }
            break;

            case rdpdr::PacketId::PAKID_CORE_CLIENT_NAME:
                REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_message: "
                            "Client Name Request");

                    rdpdr::ClientNameRequest client_name_request;

                    client_name_request.receive(chunk);
                    client_name_request.log(LOG_INFO);
                }
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE:
                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_message: "
                            "Client Device List Announce Request");
                }

                send_message_to_server =
                    this->process_client_device_list_announce_request(
                        total_length, flags, chunk);
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION:
                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_message: "
                            "Client Drive I/O Response");

                    if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                        send_message_to_server =
                            this->process_client_drive_io_response(
                                total_length, flags, chunk);
                    }
                }
            break;

            case rdpdr::PacketId::PAKID_CORE_CLIENT_CAPABILITY:
                REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_message: "
                            "Client Core Capability Response");
                }

                send_message_to_server =
                    this->process_client_core_capability_response(
                        total_length, flags, chunk);
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICELIST_REMOVE:
                REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_message: "
                            "Client Drive Device List Remove");
                }
            break;

            default:
                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_message: "
                            "Undecoded PDU - "
                            "Component=%s(0x%X) PacketId=0x%X",
                        rdpdr::SharedHeader::get_Component_name(
                            this->server_message_header.component),
                        static_cast<uint16_t>(
                            this->server_message_header.component),
                        rdpdr::SharedHeader::get_PacketId_name(
                            this->server_message_header.packet_id),
                        static_cast<uint16_t>(
                            this->server_message_header.packet_id));
                }
            break;
        }   // switch (this->client_message_header.packet_id)

        if (send_message_to_server) {
            this->send_message_to_server(total_length, flags, chunk_data,
                chunk_data_length);
        }
    }   // process_client_message

    bool process_server_announce_request(uint32_t total_length,
        uint32_t flags, Stream& chunk)
    {
        rdpdr::ServerAnnounceRequest server_announce_request;

        server_announce_request.receive(chunk);

        if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
            server_announce_request.log(LOG_INFO);
        }

        // Virtual channel is opened at client side and is authorized.
        if (this->has_valid_to_client_sender())
            return true;

        uint8_t message_buffer[1024];

        {
            WriteOnlyStream out_stream(message_buffer,
                sizeof(message_buffer));

            rdpdr::SharedHeader clent_message_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
            clent_message_header.emit(out_stream);

            rdpdr::ClientAnnounceReply client_announce_reply(
                0x0001, // VersionMajor, MUST be set to 0x0001.
                0x0006, // Windows XP SP3.
                // [MS-RDPEFS] - 3.2.5.1.3 Sending a Client Announce
                //     Reply Message.
                ((server_announce_request.VersionMinor() >= 12) ?
                 this->param_random_number :
                 server_announce_request.ClientId()));
            if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_server_announce_request:");
                client_announce_reply.log(LOG_INFO);
            }
            client_announce_reply.emit(out_stream);

            out_stream.mark_end();

            this->send_message_to_server(
                out_stream.size(),
                  CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_data(),
                out_stream.size());
        }

        {
            WriteOnlyStream out_stream(message_buffer,
                sizeof(message_buffer));

            rdpdr::SharedHeader clent_message_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_CLIENT_NAME);
            clent_message_header.emit(out_stream);

            rdpdr::ClientNameRequest client_name_request(
                this->param_client_name);
            if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_server_announce_request:");
                client_name_request.log(LOG_INFO);
            }
            client_name_request.emit(out_stream);

            out_stream.mark_end();

            this->send_message_to_server(
                out_stream.size(),
                  CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_data(),
                out_stream.size());
        }

        return false;
    }   // process_server_announce_request

    bool process_server_client_id_confirm(uint32_t total_length,
        uint32_t flags, Stream& chunk)
    {
        // Virtual channel is opened at client side and is authorized.
        if (this->has_valid_to_client_sender())
            return true;

        uint8_t message_buffer[1024];

        {
            WriteOnlyStream out_stream(message_buffer,
                sizeof(message_buffer));

            const rdpdr::SharedHeader clent_message_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_CLIENT_CAPABILITY);
            clent_message_header.emit(out_stream);

            out_stream.out_uint16_le(5);    // 5 capabilities.
            out_stream.out_clear_bytes(2);  // Padding(2)

            // General capability set
            const uint32_t general_capability_version =
                rdpdr::GENERAL_CAPABILITY_VERSION_02;
            out_stream.out_uint16_le(rdpdr::CAP_GENERAL_TYPE);
            out_stream.out_uint16_le(
                    rdpdr::GeneralCapabilitySet::size(
                        general_capability_version) +
                    8   // CapabilityType(2) + CapabilityLength(2) +
                        //     Version(4)
                );
            out_stream.out_uint32_le(general_capability_version);

            rdpdr::GeneralCapabilitySet general_capability_set(
                    0x2,        // osType
                    0x50001,    // osVersion
                    0x1,        // protocolMajorVersion
                    0xC,        // protocolMinorVersion -
                                //     RDP Client 6.0 and 6.1
                    0xFFFF,     // ioCode1
                    0x0,        // ioCode2
                    0x7,        // extendedPDU -
                                //     RDPDR_DEVICE_REMOVE_PDUS(1) |
                                //     RDPDR_CLIENT_DISPLAY_NAME_PDU(2) |
                                //     RDPDR_USER_LOGGEDON_PDU(4)
                    0x0,        // extraFlags1
                    0x0,        // extraFlags2
                    0           // SpecialTypeDeviceCap
                );
            if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_server_client_id_confirm:");
                general_capability_set.log(LOG_INFO);
            }
            general_capability_set.emit(out_stream,
                general_capability_version);

            // Print capability set
            out_stream.out_uint16_le(rdpdr::CAP_PRINTER_TYPE);
            out_stream.out_uint16_le(
                    8   // CapabilityType(2) + CapabilityLength(2) +
                        //     Version(4)
                );
            out_stream.out_uint32_le(rdpdr::PRINT_CAPABILITY_VERSION_01);

            // Port capability set
            out_stream.out_uint16_le(rdpdr::CAP_PORT_TYPE);
            out_stream.out_uint16_le(
                    8   // CapabilityType(2) + CapabilityLength(2) +
                        //     Version(4)
                );
            out_stream.out_uint32_le(rdpdr::PORT_CAPABILITY_VERSION_01);

            // Drive capability set
            out_stream.out_uint16_le(rdpdr::CAP_DRIVE_TYPE);
            out_stream.out_uint16_le(
                    8   // CapabilityType(2) + CapabilityLength(2) +
                        //     Version(4)
                );
            out_stream.out_uint32_le(rdpdr::DRIVE_CAPABILITY_VERSION_01);

            // Smart card capability set
            out_stream.out_uint16_le(rdpdr::CAP_SMARTCARD_TYPE);
            out_stream.out_uint16_le(
                    8   // CapabilityType(2) + CapabilityLength(2) +
                        //     Version(4)
                );
            out_stream.out_uint32_le(rdpdr::DRIVE_CAPABILITY_VERSION_01);

            out_stream.mark_end();

            this->send_message_to_server(
                out_stream.size(),
                  CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_data(),
                out_stream.size());
        }

        return false;
    }   // process_server_client_id_confirm

    bool process_server_create_drive_request(uint32_t total_length,
        uint32_t flags, Stream& chunk)
    {
        rdpdr::DeviceCreateRequest device_create_request;

        device_create_request.receive(chunk);

        if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
            device_create_request.log(LOG_INFO);
        }

              bool     access_ok     = true;
        const uint32_t DesiredAccess = device_create_request.DesiredAccess();

        if (!this->param_file_system_read_authorized &&
            smb2::read_access_is_required(DesiredAccess,
                                          /* strict_check = */false) &&
            !(device_create_request.CreateOptions() &
              smb2::FILE_DIRECTORY_FILE) &&
            ::strcmp(device_create_request.Path(), "/")) {
            access_ok = false;
        }
        if (!this->param_file_system_write_authorized &&
            smb2::write_access_is_required(DesiredAccess,
                                           /* strict_check = */false)) {
            access_ok = false;
        }

        if (!access_ok)
        {
            uint8_t message_buffer[1024];

            WriteOnlyStream out_stream(message_buffer,
                sizeof(message_buffer));

            const rdpdr::SharedHeader clent_message_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);

            clent_message_header.emit(out_stream);

            const rdpdr::DeviceIOResponse device_io_response(
                    this->server_device_io_request.DeviceId(),
                    this->server_device_io_request.CompletionId(),
                    0xC0000022  // STATUS_ACCESS_DENIED
                );

            if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                device_io_response.log(LOG_INFO);
            }
            device_io_response.emit(out_stream);

            const rdpdr::DeviceCreateResponse device_create_response(
                static_cast<uint32_t>(-1), 0);
            if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                device_create_response.log(LOG_INFO);
            }
            device_create_response.emit(out_stream);

            out_stream.mark_end();

            this->send_message_to_server(
                out_stream.size(),
                  CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_data(),
                out_stream.size());

            return false;
        }

        return true;
    }   // process_server_create_drive_request

    bool process_server_drive_io_request(uint32_t total_length,
        uint32_t flags, Stream& chunk,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task)
    {
        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->server_device_io_request.receive(chunk);
            if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                this->server_device_io_request.log(LOG_INFO);
            }
        }

        if (this->file_system_drive_manager.IsManagedDrive(
                this->server_device_io_request.DeviceId()))
        {
            this->file_system_drive_manager.ProcessDeviceIORequest(
                this->server_device_io_request,
                (flags & CHANNELS::CHANNEL_FLAG_FIRST),
                chunk,
                this->to_server_sender,
                out_asynchronous_task,
                this->get_verbose());

            return false;
        }

        if (!(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            return true;
        }

        uint32_t extra_data = 0;

        bool send_message_to_client = true;

        switch (this->server_device_io_request.MajorFunction())
        {
            case rdpdr::IRP_MJ_CREATE:
                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Device Create Request");
                }

                send_message_to_client =
                    this->process_server_create_drive_request(
                        total_length, flags, chunk);
            break;

            case rdpdr::IRP_MJ_CLOSE:
                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Device Close Request");
                }
            break;

            case rdpdr::IRP_MJ_WRITE:
                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    uint32_t Length = chunk.in_uint32_le();
                    uint64_t Offset = chunk.in_uint64_le();

                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Write Request. Length=%u Offset=%" PRIu64,
                        Length, Offset);

                    this->update_exchanged_data(Length);
                }
            break;

            case rdpdr::IRP_MJ_DEVICE_CONTROL:
                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Device control request");

                    rdpdr::DeviceControlRequest device_control_request;

                    device_control_request.receive(chunk);
                    device_control_request.log(LOG_INFO);
                }
            break;

            case rdpdr::IRP_MJ_QUERY_VOLUME_INFORMATION:
                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Query volume information request");

                    rdpdr::ServerDriveQueryVolumeInformationRequest
                        server_drive_query_volume_information_request;

                    server_drive_query_volume_information_request.receive(
                        chunk);
                    server_drive_query_volume_information_request.log(
                        LOG_INFO);

                    extra_data =
                        server_drive_query_volume_information_request.FsInformationClass();
                }
            break;

            case rdpdr::IRP_MJ_QUERY_INFORMATION:
                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Server Drive Query Information Request");

                    rdpdr::ServerDriveQueryInformationRequest
                        server_drive_query_information_request;

                    server_drive_query_information_request.receive(chunk);
                    server_drive_query_information_request.log(LOG_INFO);

                    extra_data =
                        server_drive_query_information_request.FsInformationClass();
                }
            break;

            case rdpdr::IRP_MJ_DIRECTORY_CONTROL:
                if (this->server_device_io_request.MinorFunction() ==
                    rdpdr::IRP_MN_QUERY_DIRECTORY) {
                    if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                        LOG(LOG_INFO,
                            "FileSystemVirtualChannel::process_server_drive_io_request: "
                                "Server Drive Query Directory Request");

                        rdpdr::ServerDriveQueryDirectoryRequest
                            server_drive_query_directory_request;

                        server_drive_query_directory_request.receive(chunk);
                        server_drive_query_directory_request.log(LOG_INFO);

                        extra_data =
                            server_drive_query_directory_request.FsInformationClass();
                    }
                }
            break;

            default:
                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Undecoded Drive I/O Request - "
                            "MajorFunction=%s(0x%08X) "
                            "MinorFunction=%s(0x%08X)",
                        rdpdr::DeviceIORequest::get_MajorFunction_name(
                            this->server_device_io_request.MajorFunction()),
                        this->server_device_io_request.MajorFunction(),
                        rdpdr::DeviceIORequest::get_MinorFunction_name(
                            this->server_device_io_request.MinorFunction()),
                        this->server_device_io_request.MinorFunction());
                }
            break;
        }   // switch (this->server_device_io_request.MajorFunction())

        if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
            this->device_io_request_info_inventory.push_back(
                std::make_tuple(
                    this->server_device_io_request.DeviceId(),
                    this->server_device_io_request.CompletionId(),
                    this->server_device_io_request.MajorFunction(),
                    extra_data));
        }

        return send_message_to_client;
    }   // process_server_drive_io_request

    virtual void process_server_message(uint32_t total_length,
        uint32_t flags, uint8_t* chunk_data, uint32_t chunk_data_length,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task)
            override
    {
        if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
            LOG(LOG_INFO,
                "FileSystemVirtualChannel::process_server_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR_DUMP) {
            LOG(LOG_INFO, "Recv done on rdpdr (-1) n bytes");
            const uint32_t dest = 1;    // Server
            hexdump_c(reinterpret_cast<const uint8_t*>(&dest),
                sizeof(dest));
            hexdump_c(reinterpret_cast<uint8_t*>(&total_length),
                sizeof(total_length));
            hexdump_c(reinterpret_cast<uint8_t*>(&flags), sizeof(flags));
            hexdump_c(reinterpret_cast<uint8_t*>(&chunk_data_length),
                sizeof(chunk_data_length));
            hexdump_c(chunk_data, chunk_data_length);
            LOG(LOG_INFO, "Dump done on rdpdr (-1) n bytes");
        }

        ReadOnlyStream chunk(chunk_data, chunk_data_length);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->server_message_header.receive(chunk);
        }

        bool send_message_to_client = this->has_valid_to_client_sender();

        switch (this->server_message_header.packet_id)
        {
            case rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE:
                if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) &&
                    (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR)) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_message: "
                            "Server Announce Request");
                }

                send_message_to_client =
                    this->process_server_announce_request(total_length,
                        flags, chunk);
            break;

            case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
                REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_message: "
                            "Server Client ID Confirm");
                }

                send_message_to_client =
                    this->process_server_client_id_confirm(total_length,
                        flags, chunk);
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICE_REPLY:
                REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_message: "
                            "Server Device Announce Response");

                    rdpdr::ServerDeviceAnnounceResponse
                        server_device_announce_response;

                    server_device_announce_response.receive(chunk);
                    server_device_announce_response.log(LOG_INFO);
                }
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST:
                REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_message: "
                            "Server Drive I/O Request");
                }

                send_message_to_client =
                    this->process_server_drive_io_request(total_length,
                        flags, chunk, out_asynchronous_task);
            break;

            case rdpdr::PacketId::PAKID_CORE_SERVER_CAPABILITY:
                REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_message: "
                            "Server Core Capability Request");
                }
            break;

            case rdpdr::PacketId::PAKID_CORE_USER_LOGGEDON:
                REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_message: "
                            "Server User Logged On");
                }

                if (!this->proxy_managed_drives_announced) {
                    this->file_system_drive_manager.AnnounceDrive(
                        this->device_capability_version_02_supported,
                        this->to_server_sender,
                        this->get_verbose());

                    this->proxy_managed_drives_announced = true;
                }
            break;

            case rdpdr::PacketId::PAKID_PRN_USING_XPS:
                REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_message: "
                            "Server Printer Set XPS Mode");
                }
            break;

            default:
                if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) &&
                    (this->get_verbose() & MODRDP_LOGLEVEL_RDPDR)) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_message: "
                            "Undecoded PDU - "
                            "Component=%s(0x%X) PacketId=0x%X",
                        rdpdr::SharedHeader::get_Component_name(
                            this->server_message_header.component),
                        static_cast<uint16_t>(
                            this->server_message_header.component),
                        rdpdr::SharedHeader::get_PacketId_name(
                            this->server_message_header.packet_id),
                        static_cast<uint16_t>(
                            this->server_message_header.packet_id));
                }
            break;
        }   // switch (this->server_message_header.packet_id)

        if (send_message_to_client) {
            this->send_message_to_client(total_length, flags, chunk_data,
                chunk_data_length);
        }
    }   // process_server_message
};  // class FileSystemVirtualChannel

#endif  // #ifndef REDEMPTION_MOD_RDP_CHANNELS_RDPDRCHANNEL_HPP
