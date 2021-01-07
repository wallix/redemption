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


#pragma once

#include "acl/auth_api.hpp"
#include "core/events.hpp"
#include "core/log_id.hpp"
#include "core/RDP/channels/rdpdr_completion_id_manager.hpp"
#include "core/RDP/channels/rdpdr.hpp"
#include "core/channel_list.hpp"
#include "utils/timebase.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "mod/rdp/channels/rdpdr_file_system_drive_manager.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"
#include "system/ssl_sha256.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/strutils.hpp"
#include "utils/timebase.hpp"
#include "core/file_system_virtual_channel_params.hpp"
#include "core/stream_throw_helpers.hpp"
#include <deque>


class FileSystemVirtualChannel final : public BaseVirtualChannel
{
    SessionLogApi& session_log;
    const RDPVerbose verbose;

    VirtualChannelDataSender& to_server_sender;

    rdpdr::SharedHeader client_message_header;
    rdpdr::SharedHeader server_message_header;

    rdpdr::DeviceIORequest  server_device_io_request;
    rdpdr::DeviceIOResponse client_device_io_response;

    uint16_t serverVersionMinor;

    FileSystemDriveManager& file_system_drive_manager;

    FileSystemCompletionIdManager file_system_completion_id_manager;


    struct device_io_request_info_type
    {
        uint32_t device_id;
        uint32_t file_id;
        uint32_t completion_id;

        std::string file_path;

        // TODO IRP_MAJOR
        uint32_t major_function;
        uint32_t extra_data;

        uint64_t offset;
        uint32_t length;
        uint32_t remaining;
    };
    using device_io_request_info_inventory_type = std::vector<device_io_request_info_type>;
    device_io_request_info_inventory_type device_io_request_info_inventory;

    static constexpr const int64_t END_OF_FILE_READING = -1;  // Reading in progress.
    static constexpr const int64_t END_OF_FILE_UNREAD  = -2;

    struct device_io_target_info_type
    {
        uint32_t device_id;
        uint32_t file_id;

        std::string file_path;

        bool for_reading;
        bool for_writing;
        bool for_sequential_access;

        int64_t end_of_file;

        int64_t sequential_access_offset;

        SslSha256 sha256;
    };
    using device_io_target_info_inventory_type = std::vector<device_io_target_info_type>;
    device_io_target_info_inventory_type device_io_target_info_inventory;

    bool device_capability_version_02_supported = false;

    const char* const param_client_name;
    const bool        param_file_system_read_authorized;
    const bool        param_file_system_write_authorized;
    const uint32_t    param_random_number;                  // For ClientId.

    const bool        param_dont_log_data_into_syslog;

    const char* const param_proxy_managed_drive_prefix;

    bool user_logged_on = false;

    unsigned int enable_session_probe_drive_count = 0;

    class DeviceRedirectionManager
    {
        struct virtual_channel_data_type
        {
            std::size_t length;
            std::unique_ptr<uint8_t[]> data;

            bool can_be_removed;
        };
        using device_announce_collection_type = std::deque<virtual_channel_data_type>;
        device_announce_collection_type device_announces;

        struct device_info_type
        {
            uint32_t device_id;
            rdpdr::RDPDR_DTYP device_type;
            std::string preferred_dos_name;
        };
        using device_info_inventory_type = std::vector<device_info_type>;
        device_info_inventory_type device_info_inventory;

        FileSystemDriveManager& file_system_drive_manager;

        bool& user_logged_on;

    public:
        class ToDeviceAnnounceCollectionSender :
            public VirtualChannelDataSender
        {
            device_announce_collection_type& device_announces;

            std::unique_ptr<uint8_t[]> device_announce_data;
            OutStream                  device_announce_stream;

        public:
            explicit ToDeviceAnnounceCollectionSender(
                device_announce_collection_type& device_announces)
            : device_announces(device_announces) {}

            void operator()(uint32_t total_length, uint32_t flags,
                bytes_view chunk_data) override
            {
                assert((flags & CHANNELS::CHANNEL_FLAG_FIRST) ||
                          bool(this->device_announce_data));

                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                    this->device_announce_data =
                        std::make_unique<uint8_t[]>(total_length);

                    this->device_announce_stream = OutStream(
                        {this->device_announce_data.get(), total_length});
                }

                assert(this->device_announce_stream.tailroom() >=
                    chunk_data.size());

                this->device_announce_stream.out_copy_bytes(chunk_data);

                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    this->device_announces.push_back({
                        this->device_announce_stream.get_offset(),
                        std::move(this->device_announce_data),
                        false
                    });

                    this->device_announce_stream = OutStream();
                }
            }
        } to_device_announce_collection_sender;

    private:
        std::unique_ptr<uint8_t[]> current_device_announce_data;
        OutStream                  current_device_announce_stream;

        VirtualChannelDataSender* to_client_sender;
        VirtualChannelDataSender* to_server_sender;

        const bool param_file_system_authorized;
        const bool param_parallel_port_authorized;
        const bool param_print_authorized;
        const bool param_serial_port_authorized;
        const bool param_smart_card_authorized;

        uint32_t length_of_remaining_device_data_to_be_processed = 0;
        uint32_t length_of_remaining_device_data_to_be_skipped   = 0;

        bool waiting_for_server_device_announce_response = false;

        uint8_t         remaining_device_announce_request_header_data[
                20  // DeviceType(4) + DeviceId(4) +
                    //     PreferredDosName(8) +
                    //     DeviceDataLength(4)
            ];
        OutStream remaining_device_announce_request_header_stream;

        bool session_probe_drive_should_be_disable = false;

        FileSystemVirtualChannel& file_system_virtual_channel;

        const bool smartcard_passthrough;

        const RDPVerbose verbose;

    public:
        explicit DeviceRedirectionManager(
            FileSystemVirtualChannel& file_system_virtual_channel,
            FileSystemDriveManager& file_system_drive_manager,
            bool& user_logged_on,
            VirtualChannelDataSender* to_client_sender_,
            VirtualChannelDataSender* to_server_sender_,
            bool file_system_authorized,
            bool parallel_port_authorized,
            bool print_authorized,
            bool serial_port_authorized,
            bool smart_card_authorized,
            uint32_t channel_chunk_length,
            bool smartcard_passthrough,
            RDPVerbose verbose)
        : file_system_drive_manager(file_system_drive_manager)
        , user_logged_on(user_logged_on)
        , to_device_announce_collection_sender(device_announces)
        , to_client_sender(to_client_sender_)
        , to_server_sender(to_server_sender_)
        , param_file_system_authorized(file_system_authorized)
        , param_parallel_port_authorized(parallel_port_authorized)
        , param_print_authorized(print_authorized)
        , param_serial_port_authorized(serial_port_authorized)
        , param_smart_card_authorized(smart_card_authorized)
        , remaining_device_announce_request_header_stream(
              this->remaining_device_announce_request_header_data)
        , file_system_virtual_channel(file_system_virtual_channel)
        , smartcard_passthrough(smartcard_passthrough)
        , verbose(verbose) {
            (void)channel_chunk_length;
        }

    private:
        bool add_known_device(uint32_t DeviceId, rdpdr::RDPDR_DTYP DeviceType, const char* PreferredDosName) {
            for (device_info_type const & info : this->device_info_inventory) {
                if (info.device_id == DeviceId) {
                    LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                        "FileSystemVirtualChannel::DeviceRedirectionManager::add_known_device:"
                        " \"%s\"(DeviceId=%" PRIu32 " DeviceType=%" PRIu32 ") is already"
                        " in the device list. Old=\"%s\" (DeviceType=%" PRIu32 ")",
                        PreferredDosName, DeviceId, underlying_cast(DeviceType),
                        info.preferred_dos_name.c_str(), underlying_cast(info.device_type));

                    return false;
                }
            }

            this->device_info_inventory.push_back({ DeviceId, DeviceType, PreferredDosName });
            LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                "FileSystemVirtualChannel::DeviceRedirectionManager::add_known_device:"
                " Add \"%s\"(DeviceId=%" PRIu32 " DeviceType=%" PRIu32 ") to known device list.",
                PreferredDosName, DeviceId, underlying_cast(DeviceType));

            return true;
        }

    public:
        void DisableSessionProbeDrive() {
            if (!this->user_logged_on) {
                this->file_system_drive_manager.remove_session_probe_drive();

                return;
            }

            this->session_probe_drive_should_be_disable = true;

            this->effective_disable_session_probe_drive();
        }

        void effective_disable_session_probe_drive() {
            if (this->user_logged_on &&
                !this->waiting_for_server_device_announce_response &&
                this->device_announces.empty() &&
                this->session_probe_drive_should_be_disable) {
                this->file_system_drive_manager.disable_session_probe_drive(
                    *this->to_server_sender);

                this->session_probe_drive_should_be_disable = false;
            }
        }

        std::string const * get_device_name(uint32_t DeviceId) {
            for (device_info_type const & info : this->device_info_inventory) {
                if (info.device_id == DeviceId) {
                    return &info.preferred_dos_name;
                }
            }

            return nullptr;
        }

        rdpdr::RDPDR_DTYP get_device_type(uint32_t DeviceId) {
            for (device_info_type const & info : this->device_info_inventory) {
                if (info.device_id == DeviceId) {
                    return info.device_type;
                }
            }

            return rdpdr::RDPDR_DTYP_UNSPECIFIED;
        }

        bool is_known_device(uint32_t DeviceId) {
            for (device_info_type const & info : this->device_info_inventory) {
                if (info.device_id == DeviceId) {
                    return true;
                }
            }

            return false;
        }

    private:
        void remove_known_device(uint32_t DeviceId) {
            for (auto iter = this->device_info_inventory.begin();
                 iter != this->device_info_inventory.end(); ++iter) {
                if (iter->device_id == DeviceId) {
                    LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                        "FileSystemVirtualChannel::DeviceRedirectionManager::remove_known_device:"
                        " Remove \"%s\"(DeviceId=%" PRIu32 ") from known device list.",
                        iter->preferred_dos_name, DeviceId);

                    unordered_erase(this->device_info_inventory, iter);

                    break;
                }
            }
        }

        void announce_device(bool user_is_logged_on) {
            if (this->waiting_for_server_device_announce_response) {
                return;
            }

            for (auto iter = this->device_announces.begin();
                 !this->waiting_for_server_device_announce_response &&
                     (this->device_announces.end() != iter);
                 ++iter) {
                assert(this->to_server_sender);

                const uint32_t   total_length = iter->length;
                const uint8_t  * chunk_data = iter->data.get();

                uint32_t remaining_data_length = total_length;

                {
                    InStream chunk({chunk_data, total_length});

                    rdpdr::SharedHeader client_message_header;

                    client_message_header.receive(chunk);

                    assert(client_message_header.component ==
                        rdpdr::Component::RDPDR_CTYP_CORE);
                    assert(client_message_header.packet_id ==
                        rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE);

                    assert(chunk.in_remain() >=
                              4 // DeviceCount(4)
                             );
                    const uint32_t DeviceCount = chunk.in_uint32_le();
                    (void)DeviceCount;
                    assert(DeviceCount == 1);

                    rdpdr::DeviceAnnounceHeader_Recv device_announce_header;

                    device_announce_header.receive(chunk);

                    const rdpdr::RDPDR_DTYP DeviceType = device_announce_header.DeviceType();
                    if (!user_is_logged_on &&
                        (rdpdr::RDPDR_DTYP::RDPDR_DTYP_SERIAL != DeviceType) &&
                        (rdpdr::RDPDR_DTYP::RDPDR_DTYP_SMARTCARD != DeviceType)) {
                        continue;
                    }

                    if (!this->add_known_device(
                            device_announce_header.DeviceId(),
                            device_announce_header.DeviceType(),
                            device_announce_header.PreferredDosName())) {

                        iter->can_be_removed = true;

                        continue;
                    }
                }

                uint32_t flags = CHANNELS::CHANNEL_FLAG_FIRST;

                do {
                    uint32_t chunk_data_length = std::min<uint32_t>(
                        CHANNELS::CHANNEL_CHUNK_LENGTH,
                        remaining_data_length);

                    if (chunk_data_length <= CHANNELS::CHANNEL_CHUNK_LENGTH) {
                        flags |= CHANNELS::CHANNEL_FLAG_LAST;
                    }

                    if (bool(this->verbose & RDPVerbose::rdpdr_dump)) {
                        const bool send              = true;
                        const bool from_or_to_client = false;
                        ::msgdump_c(send, from_or_to_client,
                            total_length, flags,
                            {chunk_data, chunk_data_length});
                    }

                    (*this->to_server_sender)(
                        total_length, flags,
                        {chunk_data, chunk_data_length});

                    chunk_data            += chunk_data_length;
                    remaining_data_length -= chunk_data_length;

                    flags &= ~CHANNELS::CHANNEL_FLAG_FIRST;
                }
                while (remaining_data_length);

                iter->can_be_removed = true;

                this->waiting_for_server_device_announce_response = true;
            }

            this->device_announces.erase(
                std::remove_if(this->device_announces.begin(),
                               this->device_announces.end(),
                               [](const auto& device_announce) { return device_announce.can_be_removed; }),
                this->device_announces.end());

            this->effective_disable_session_probe_drive();
        }   // void announce_device(bool user_is_logged_on)

    public:
        void process_client_device_list_announce_request(
            uint32_t total_length, uint32_t flags, InStream& chunk)
        {
            (void)total_length;
            if (flags & CHANNELS::CHANNEL_FLAG_FIRST)
            {
                assert(!this->length_of_remaining_device_data_to_be_processed);
                assert(!this->length_of_remaining_device_data_to_be_skipped);

                // DeviceCount(4)
                ::check_throw(chunk, 4, "FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request", ERR_RDP_DATA_TRUNCATED);

                uint32_t DeviceCount = chunk.in_uint32_le();

                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request:"
                    " DeviceCount=%" PRIu32,
                    DeviceCount);

                this->remaining_device_announce_request_header_stream.rewind();
            }

            while (chunk.in_remain())
            {
                if (!this->length_of_remaining_device_data_to_be_processed &&
                    !this->length_of_remaining_device_data_to_be_skipped) {
                    // There is no device in process.

                    if ((chunk.in_remain() <
                             20 // DeviceType(4) + DeviceId(4) +
                                //     PreferredDosName(8) +
                                //     DeviceDataLength(4)
                        ) &&
                        !this->remaining_device_announce_request_header_stream.get_offset()) {

                        this->remaining_device_announce_request_header_stream.out_copy_bytes(
                            chunk.get_current(), chunk.in_remain());

                        LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                            "FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request:"
                            " %zu byte(s) of request header are saved.",
                            this->remaining_device_announce_request_header_stream.get_offset());

                        break;
                    }

                    InStream* device_announce_request_header_stream = &chunk;
                    InStream remaining_device_announce_request_header_stream_in;

                    if (this->remaining_device_announce_request_header_stream.get_offset()) {
                        const uint32_t needed_data_length =
                              20    // DeviceType(4) + DeviceId(4) +
                                    //     PreferredDosName(8) +
                                    //     DeviceDataLength(4)
                            - this->remaining_device_announce_request_header_stream.get_offset();

                        this->remaining_device_announce_request_header_stream.out_copy_bytes(
                            chunk.get_current(), needed_data_length);

                        chunk.in_skip_bytes(needed_data_length);

                        remaining_device_announce_request_header_stream_in = InStream(
                            this->remaining_device_announce_request_header_stream.get_produced_bytes());
                        device_announce_request_header_stream =
                            &remaining_device_announce_request_header_stream_in;

                        this->remaining_device_announce_request_header_stream.rewind();
                    }

                    const uint32_t DeviceType =
                        device_announce_request_header_stream->in_uint32_le();
                    const uint32_t DeviceId   =
                        device_announce_request_header_stream->in_uint32_le();

                    uint8_t PreferredDosName[
                              8 // PreferredDosName(8)
                            + 1
                        ];

                    device_announce_request_header_stream->in_copy_bytes(
                            PreferredDosName,
                            8   // PreferredDosName(8)
                        );
                    PreferredDosName[8  // PreferredDosName(8)
                        ] = '\0';

                    const uint32_t DeviceDataLength =
                        device_announce_request_header_stream->in_uint32_le();

                    this->remaining_device_announce_request_header_stream.rewind();

                    LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                        "FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request:"
                        " DeviceType=%s(%" PRIu32 ") DeviceId=%" PRIu32
                        " PreferredDosName=\"%s\" DeviceDataLength=%" PRIu32,
                        rdpdr::get_DeviceType_name(static_cast<rdpdr::RDPDR_DTYP>(DeviceType)),
                        DeviceType, DeviceId, PreferredDosName, DeviceDataLength);

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
                        assert(!bool(this->current_device_announce_data));

                        const uint32_t current_device_announce_data_length =
                              rdpdr::SharedHeader::size()
                            + 24                // DeviceCount(4) +
                                                //     DeviceType(4) +
                                                //     DeviceId(4) +
                                                //     PreferredDosName(8) +
                                                //     DeviceDataLength(4)
                            + DeviceDataLength;

                        if (this->current_device_announce_stream.get_capacity() <
                            current_device_announce_data_length) {
                            this->current_device_announce_data =
                                std::make_unique<uint8_t[]>(
                                    current_device_announce_data_length);

                            this->current_device_announce_stream = OutStream(
                                {this->current_device_announce_data.get(),
                                current_device_announce_data_length});
                        }
                        else {
                            this->current_device_announce_stream.rewind();
                        }

                        this->length_of_remaining_device_data_to_be_processed =
                            DeviceDataLength;
                        this->length_of_remaining_device_data_to_be_skipped   = 0;

                        rdpdr::SharedHeader client_message_header(
                            rdpdr::Component::RDPDR_CTYP_CORE,
                            rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE);

                        client_message_header.emit(
                            this->current_device_announce_stream);

                        this->current_device_announce_stream.out_uint32_le(
                             1  // DeviceCount(4)
                            );

                        this->current_device_announce_stream.out_uint32_le(
                            DeviceType);        // DeviceType(4)
                        this->current_device_announce_stream.out_uint32_le(
                            DeviceId);          // DeviceId(4)
                        this->current_device_announce_stream.out_copy_bytes(
                             PreferredDosName,
                             8                  // PreferredDosName(8)
                            );
                        this->current_device_announce_stream.out_uint32_le(
                            DeviceDataLength);  // DeviceDataLength(4)
                    }   // if (((DeviceType == rdpdr::RDPDR_DTYP_FILESYSTEM) &&
                    else
                    {
                        this->length_of_remaining_device_data_to_be_processed = 0;
                        this->length_of_remaining_device_data_to_be_skipped   =
                            DeviceDataLength;

                        StaticOutStream<512> out_stream;

                        rdpdr::SharedHeader server_message_header(
                            rdpdr::Component::RDPDR_CTYP_CORE,
                            rdpdr::PacketId::PAKID_CORE_DEVICE_REPLY);

                        server_message_header.emit(out_stream);

                        rdpdr::ServerDeviceAnnounceResponse
                            server_device_announce_response(
                                    DeviceId,
                                    erref::NTSTATUS::STATUS_UNSUCCESSFUL
                                );

                        server_device_announce_response.emit(out_stream);

                        if (bool(this->verbose & RDPVerbose::rdpdr)) {
                            LOG(LOG_INFO,
                                "FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request: "
                                    "Server Device Announce Response");
                            server_device_announce_response.log(LOG_INFO);
                        }

                        assert(this->to_client_sender);

                        const uint32_t total_length_      = out_stream.get_offset();
                        const uint32_t flags_             =
                            CHANNELS::CHANNEL_FLAG_FIRST |
                            CHANNELS::CHANNEL_FLAG_LAST;

                        if (bool(this->verbose & RDPVerbose::rdpdr_dump)) {
                            const bool send              = true;
                            const bool from_or_to_client = true;
                            ::msgdump_c(send,
                                from_or_to_client, total_length_, flags_,
                                out_stream.get_produced_bytes());
                        }

                        (*this->to_client_sender)(total_length_, flags_, out_stream.get_produced_bytes());
                    }
                }   // if (!this->length_of_remaining_device_data_to_be_processed &&

                if (this->current_device_announce_stream.get_offset() ||
                    this->length_of_remaining_device_data_to_be_processed) {

                    uint32_t length_of_device_data_can_be_processed =
                        std::min<uint32_t>(
                            this->length_of_remaining_device_data_to_be_processed,
                            chunk.in_remain());

                    this->current_device_announce_stream.out_copy_bytes(
                        chunk.get_current(), length_of_device_data_can_be_processed);

                    chunk.in_skip_bytes(
                        length_of_device_data_can_be_processed);
                    this->length_of_remaining_device_data_to_be_processed -=
                        length_of_device_data_can_be_processed;

                    if (!this->length_of_remaining_device_data_to_be_processed)
                    {
                        const uint32_t data_length =
                            this->current_device_announce_stream.get_offset();

                        this->current_device_announce_stream = OutStream();

                        this->device_announces.push_back({
                            data_length,
                            std::move(this->current_device_announce_data),
                            false
                        });
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

            if (this->smartcard_passthrough || this->user_logged_on) {
                this->announce_device(this->user_logged_on);
            }
        }   // process_client_device_list_announce_request()

        void process_client_drive_device_list_remove(uint32_t total_length,
                uint32_t flags, InStream& chunk) {
            (void)total_length;
            (void)flags;

            // DeviceCount(4)
            ::check_throw(chunk, 4,  "FileSystemVirtualChannel::DeviceRedirectionManager::process_client_drive_device_list_remove (1)", ERR_RDP_DATA_TRUNCATED);

            const uint32_t DeviceCount = chunk.in_uint32_le();

            const uint32_t max_number_of_removable_device = 128;

            StaticOutStream<
                // > rdpdr::SharedHeader::size()
                64
                // DeviceCount(4)
                + 4
                // DeviceId(4)
                + 4 * max_number_of_removable_device
            > client_drive_device_list_remove_stream;

            uint32_t number_of_removable_device = 0;

            rdpdr::SharedHeader client_message_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_DEVICELIST_REMOVE);

            client_message_header.emit(client_drive_device_list_remove_stream);

            const auto device_count_offset = client_drive_device_list_remove_stream.get_offset();

            // DeviceCount(4)
            client_drive_device_list_remove_stream.out_clear_bytes(4);

            // DeviceId(4)
            ::check_throw(chunk, DeviceCount * 4, "FileSystemVirtualChannel::DeviceRedirectionManager::process_client_drive_device_list_remove (2)", ERR_RDP_DATA_TRUNCATED);

            for (uint32_t device_index = 0; device_index < DeviceCount; device_index++) {
                const uint32_t DeviceId = chunk.in_uint32_le();

                bool device_removed = false;
                for (auto iter = this->device_announces.begin(); iter != this->device_announces.end(); ++iter) {
                    // DeviceCount(4) + DeviceType(4) + DeviceId(4)
                    InStream header_stream({iter->data.get(), rdpdr::SharedHeader::size() + 12});
                    // DeviceCount(4) + DeviceType(4)
                    header_stream.in_skip_bytes(rdpdr::SharedHeader::size() + 8);
                    const uint32_t current_device_id = header_stream.in_uint32_le();
                    if (DeviceId == current_device_id) {
                        unordered_erase(this->device_announces, iter);
                        device_removed = true;
                        break;
                    }
                }

                if (!device_removed
                && (number_of_removable_device < max_number_of_removable_device)) {
                    client_drive_device_list_remove_stream.out_uint32_le(DeviceId);
                    number_of_removable_device++;
                    this->remove_known_device(DeviceId);
                }
            }

            if (number_of_removable_device) {
                client_drive_device_list_remove_stream.stream_at(device_count_offset)
                  .out_uint32_le(number_of_removable_device);

                assert(this->to_server_sender);

                const uint32_t total_length_      =
                    client_drive_device_list_remove_stream.get_offset();
                const uint32_t flags_             =
                    CHANNELS::CHANNEL_FLAG_FIRST |
                    CHANNELS::CHANNEL_FLAG_LAST;

                if (bool(this->verbose & RDPVerbose::rdpdr_dump)) {
                    const bool send              = true;
                    const bool from_or_to_client = false;
                    ::msgdump_c(send, from_or_to_client, total_length_, flags_,
                        client_drive_device_list_remove_stream.get_produced_bytes());
                }

                (*this->to_server_sender)(
                    total_length_,
                    flags_,
                    client_drive_device_list_remove_stream.get_produced_bytes());
            }
        }

        void process_server_user_logged_on(uint32_t total_length,
                uint32_t flags, InStream& chunk) {
            (void)total_length;
            (void)flags;
            (void)chunk;
            this->announce_device(this->user_logged_on);
        }

        void process_server_device_announce_response(uint32_t total_length,
            uint32_t flags, InStream& chunk)
        {
            (void)total_length;
            (void)flags;
            this->waiting_for_server_device_announce_response = false;

            this->announce_device(this->user_logged_on);

            rdpdr::ServerDeviceAnnounceResponse
                server_device_announce_response;

            server_device_announce_response.receive(chunk);
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                server_device_announce_response.log(LOG_INFO);
            }

            if (server_device_announce_response.ResultCode() !=
                erref::NTSTATUS::STATUS_SUCCESS
               ) {
                this->remove_known_device(
                    server_device_announce_response.DeviceId());
            }

            if (server_device_announce_response.DeviceId() ==
                this->file_system_drive_manager.get_session_probe_drive_id()) {
                if (this->file_system_virtual_channel.session_probe_device_announce_responded_notifier) {
                    if (!this->file_system_virtual_channel.session_probe_device_announce_responded_notifier->on_device_announce_responded(
                            (server_device_announce_response.ResultCode() == erref::NTSTATUS::STATUS_SUCCESS))) {
                        this->file_system_virtual_channel.session_probe_device_announce_responded_notifier = nullptr;
                    }
                }
            }
        }
    } device_redirection_manager;

    SessionProbeLauncher* drive_redirection_initialize_notifier = nullptr;

    SessionProbeLauncher* session_probe_device_announce_responded_notifier = nullptr;

    const std::chrono::milliseconds initialization_timeout = std::chrono::milliseconds(5000);

    bool disable_client_sender = false;

    uint16_t server_major_version_number = 0;
    uint32_t server_generated_client_id = 0;

    template<class Cont, class ItFw>
    static void unordered_erase(Cont & cont, ItFw && pos)
    {
        if(pos + 1 != cont.end()) {
            *pos = std::move(cont.back());
        }
        cont.pop_back();
    }

    EventsGuard events_guard;
    EventRef initialization_timeout_event;

    struct NullVirtualChannelDataSender : VirtualChannelDataSender
    {
        void operator()(
            uint32_t /*total_length*/, uint32_t /*flags*/,
            bytes_view /*chunk_data*/) override
        {}
    };
    NullVirtualChannelDataSender null_virtual_channel_data_sender;

public:
    bool channel_filter_on;
    const std::string channel_files_directory;

    FileSystemVirtualChannel(
        EventContainer & events,
        VirtualChannelDataSender* to_client_sender_,
        VirtualChannelDataSender* to_server_sender_,
        FileSystemDriveManager& file_system_drive_manager,
        const bool channel_filter_on,
        std::string channel_files_directory,
        const char * client_name,
        uint32_t random_number,
        const char * proxy_managed_drive_prefix,
        const FileSystemVirtualChannelParams& params,
        SessionLogApi& session_log,
        RDPVerbose verbose)
    : BaseVirtualChannel(to_client_sender_, to_server_sender_)
    , session_log(session_log)
    , verbose(verbose)
    , to_server_sender(to_server_sender_ ? *to_server_sender_  : null_virtual_channel_data_sender)
    , serverVersionMinor(0xC)
    , file_system_drive_manager(file_system_drive_manager)
    , param_client_name(client_name)
    , param_file_system_read_authorized(params.file_system_read_authorized)
    , param_file_system_write_authorized(params.file_system_write_authorized)
    , param_random_number(random_number)
    , param_dont_log_data_into_syslog(params.dont_log_data_into_syslog)
    , param_proxy_managed_drive_prefix(proxy_managed_drive_prefix)
    , device_redirection_manager(
          *this,
          file_system_drive_manager,
          user_logged_on,
          to_client_sender_,
          to_server_sender_,
          (params.file_system_read_authorized || params.file_system_write_authorized),
          params.parallel_port_authorized,
          params.print_authorized,
          params.serial_port_authorized,
          params.smart_card_authorized,
          CHANNELS::CHANNEL_CHUNK_LENGTH,
          params.smartcard_passthrough,
          verbose)
    , events_guard(events)
    , channel_filter_on(channel_filter_on)
    , channel_files_directory(std::move(channel_files_directory))
    {}

    ~FileSystemVirtualChannel() override
    {
#ifndef NDEBUG
        for (device_io_request_info_type & request_info : this->device_io_request_info_inventory)
        {
            assert(request_info.major_function != rdpdr::IRP_MJ_DIRECTORY_CONTROL);

            LOG(LOG_WARNING,
                "FileSystemVirtualChannel::~FileSystemVirtualChannel: "
                    "There is Device I/O request information "
                    "remaining in inventory. "
                    "DeviceId=%" PRIu32 " FileId=%" PRIu32 " CompletionId=%" PRIu32 " "
                    "MajorFunction=%" PRIu32 " extra_data=%" PRIu32 " file_path=\"%s\"",
                request_info.device_id,
                request_info.file_id,
                request_info.completion_id,
                request_info.major_function,
                request_info.extra_data,
                request_info.file_path.c_str()
            );
        }
#endif  // #ifndef NDEBUG

#ifndef NDEBUG
        for (device_io_target_info_type & target_info : this->device_io_target_info_inventory)
        {
            LOG(LOG_WARNING,
                "FileSystemVirtualChannel::~FileSystemVirtualChannel: "
                    "There is Device I/O target information "
                    "remaining in inventory. "
                    "DeviceId=%" PRIu32 " FileId=%" PRIu32 " file_path=\"%s\" for_reading=%s "
                    "for_writing=%s",
                target_info.device_id,
                target_info.file_id,
                target_info.file_path,
                target_info.for_reading ? "yes" : "no",
                target_info.for_writing ? "yes" : "no"
            );
        }
#endif  // #ifndef NDEBUG
    }

    void disable_session_probe_drive() {
        if (this->enable_session_probe_drive_count) {
            this->enable_session_probe_drive_count--;

            LOG(LOG_INFO, "FileSystemVirtualChannel::disable_session_probe_drive: count=%u", this->enable_session_probe_drive_count);

            if (!this->enable_session_probe_drive_count) {
                this->device_redirection_manager.DisableSessionProbeDrive();
            }
        }
    }

    void enable_session_probe_drive() {
        this->enable_session_probe_drive_count++;

        LOG(LOG_INFO, "FileSystemVirtualChannel::enable_session_probe_drive: count=%u", this->enable_session_probe_drive_count);

        if (1 == this->enable_session_probe_drive_count) {
            this->file_system_drive_manager.enable_session_probe_drive(
                this->param_proxy_managed_drive_prefix);
        }
    }

    device_io_request_info_inventory_type::iterator
    find_request_info(uint32_t device_id, uint32_t completion_id)
    {
        return std::find_if(
            this->device_io_request_info_inventory.begin(),
            this->device_io_request_info_inventory.end(),
            [device_id, completion_id](device_io_request_info_type const & request_info) {
                return (request_info.device_id == device_id)
                    && (request_info.completion_id == completion_id);
            }
        );
    }



    device_io_target_info_inventory_type::iterator
    find_target_info(uint32_t device_id, uint32_t file_id)
    {
        return std::find_if(
            this->device_io_target_info_inventory.begin(),
            this->device_io_target_info_inventory.end(),
            [device_id, file_id](device_io_target_info_type const & target_info){
                return (target_info.device_id == device_id)
                    && (target_info.file_id == file_id);
            }
        );
    }

public:
    void process_client_general_capability_set(uint32_t total_length,
        uint32_t flags, InStream& chunk, uint32_t Version
    ) {
        (void)total_length;
        (void)flags;

        rdpdr::GeneralCapabilitySet general_capability_set;

        InStream tmp_chunk = chunk.clone();
        general_capability_set.receive(tmp_chunk, Version);

        const bool need_enable_user_loggedon_pdu =
            (!(general_capability_set.extendedPDU() &
               rdpdr::RDPDR_USER_LOGGEDON_PDU));

        const bool need_deny_asyncio =
            (general_capability_set.extraFlags1() & rdpdr::ENABLE_ASYNCIO);

        if (bool(this->verbose & RDPVerbose::rdpdr) &&
            (need_enable_user_loggedon_pdu || need_deny_asyncio)) {
            LOG(LOG_INFO,
                "FileSystemVirtualChannel::process_client_general_capability_set:");
            general_capability_set.log(LOG_INFO);
        }

        if (need_enable_user_loggedon_pdu || need_deny_asyncio) {
            OutStream out_stream(
                {const_cast<uint8_t*>(chunk.get_current()),
                rdpdr::GeneralCapabilitySet::size(Version)});

            if (need_enable_user_loggedon_pdu) {
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_client_general_capability_set:"
                    " Allow the server to send a Server User Logged On packet.");

                general_capability_set.set_extendedPDU(
                    general_capability_set.extendedPDU() |
                    rdpdr::RDPDR_USER_LOGGEDON_PDU);
            }

            if (need_deny_asyncio) {
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_client_general_capability_set:"
                    " Deny user to send multiple simultaneous"
                    " read or write requests on the same file from"
                    " a redirected file system.");

                general_capability_set.set_extraFlags1(
                    general_capability_set.extraFlags1() &
                    ~rdpdr::ENABLE_ASYNCIO);
            }

            general_capability_set.emit(out_stream);

            general_capability_set.receive(chunk, Version);

            if (bool(this->verbose & RDPVerbose::rdpdr) &&
                (need_enable_user_loggedon_pdu || need_deny_asyncio)) {
                general_capability_set.log(LOG_INFO);
            }
        }
        else {
            chunk.rewind(tmp_chunk.get_offset());
        }
    }   // process_client_general_capability_set

    bool process_client_core_capability_response(
        uint32_t total_length, uint32_t flags, InStream& chunk)
    {
        assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
            (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

        // numCapabilities(2) + Padding(2)
        ::check_throw(chunk, 4, "FileSystemVirtualChannel::process_client_core_capability_response:DR_CORE_CAPABILITY_RSP (1)", ERR_RDP_DATA_TRUNCATED);

        const uint16_t numCapabilities = chunk.in_uint16_le();
        LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
            "FileSystemVirtualChannel::process_client_core_capability_response:"
            " numCapabilities=%u", numCapabilities);

        chunk.in_skip_bytes(2); // Padding(2)

        for (uint16_t idx_capabilities = 0; idx_capabilities < numCapabilities;
             ++idx_capabilities) {

            // CapabilityType(2) + CapabilityLength(2) + Version(4)
             ::check_throw(chunk, 8, "FileSystemVirtualChannel::process_client_core_capability_response:DR_CORE_CAPABILITY_RSP (2)", ERR_RDP_DATA_TRUNCATED);

            auto chunk_p = chunk.get_current();

            const uint16_t CapabilityType   = chunk.in_uint16_le();
            const uint16_t CapabilityLength = chunk.in_uint16_le();
            const uint32_t Version          = chunk.in_uint32_le();

            LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                "FileSystemVirtualChannel::process_client_core_capability_response:"
                " CapabilityType=0x%04X CapabilityLength=%u Version=0x%X",
                CapabilityType, CapabilityLength, Version);

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

            chunk.in_skip_bytes(CapabilityLength - (chunk.get_current() - chunk_p));

            if ((CapabilityType == rdpdr::CAP_DRIVE_TYPE) &&
                (Version == rdpdr::DRIVE_CAPABILITY_VERSION_02)) {
                this->device_capability_version_02_supported = true;
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_client_core_capability_response:"
                    " Client supports DRIVE_CAPABILITY_VERSION_02.");
            }
        }

        return true;
    }   // process_client_core_capability_response

    bool process_client_device_list_announce_request(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        this->device_redirection_manager.process_client_device_list_announce_request(
            total_length, flags, chunk);

        return false;
    }

    bool process_client_drive_directory_control_response(
        uint32_t total_length, uint32_t flags, InStream& chunk,
        uint32_t FsInformationClass)
    {
        (void)total_length;
        (void)flags;
        switch (FsInformationClass) {
            case rdpdr::FileFullDirectoryInformation:
            {
                // Length(4)
                 ::check_throw(chunk, 4, "FileSystemVirtualChannel::process_client_drive_directory_control_response:DR_DRIVE_QUERY_DIRECTORY_REQ FileFullDirectoryInformation", ERR_RDP_DATA_TRUNCATED);

                 uint32_t Length = chunk.in_uint32_le(); // Length(4)

                if (Length) {
                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                        fscc::FileFullDirectoryInformation
                            file_full_directory_information;

                        //auto chunk_p = chunk.get_current();

                        file_full_directory_information.receive(chunk);

                        //LOG(LOG_INFO, "FileFullDirectoryInformation: size=%u",
                        //    (unsigned int)(chunk.get_current() - chunk_p));
                        //hexdump(chunk_p, chunk.get_current() - chunk_p);

                        file_full_directory_information.log(LOG_INFO);
                    }
                }
            }
            break;

            case rdpdr::FileBothDirectoryInformation:
            {
                // Length(4)
                ::check_throw(chunk, 4, "FileSystemVirtualChannel::process_client_drive_directory_control_response:DR_DRIVE_QUERY_DIRECTORY_REQ - FileBothDirectoryInformation", ERR_RDP_DATA_TRUNCATED);

                uint32_t Length = chunk.in_uint32_le(); // Length(4)

                if (Length) {
                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                        fscc::FileBothDirectoryInformation
                            file_both_directory_information;

                        //auto chunk_p = chunk.get_current();

                        file_both_directory_information.receive(chunk);

                        //LOG(LOG_INFO, "FileBothDirectoryInformation: size=%u",
                        //    (unsigned int)(chunk.get_current() - chunk_p));
                        //hexdump(chunk_p, chunk.get_current() - chunk_p);

                        file_both_directory_information.log(LOG_INFO);
                    }
                }
            }
            break;

            case rdpdr::FileNamesInformation:
            {
                // Length(4)
                ::check_throw(chunk, 4, "FileSystemVirtualChannel::process_client_drive_directory_control_response:DR_DRIVE_QUERY_DIRECTORY_REQ - FileNamesInformation", ERR_RDP_DATA_TRUNCATED);

                uint32_t Length = chunk.in_uint32_le(); // Length(4)

                if (Length) {
/*                    if (bool(this->verbose & RDPVerbose::rdpdr))*/ {
                        fscc::FileNamesInformation
                            file_names_information;

                        auto chunk_p = chunk.get_current();

                        file_names_information.receive(chunk);

                        LOG(LOG_INFO, "FileNamesInformation: size=%u",
                            static_cast<unsigned>(chunk.get_current() - chunk_p));
                        hexdump(chunk_p, chunk.get_current() - chunk_p);

                        file_names_information.log(LOG_INFO);
                    }
                }
            }
            break;

            default:
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_WARNING,
                    "FileSystemVirtualChannel::process_client_drive_directory_control_response:"
                    " Undecoded FsInformationClass - %s(0x%X)",
                    rdpdr::ServerDriveQueryDirectoryRequest::get_FsInformationClass_name(
                        FsInformationClass),
                    FsInformationClass);
            break;
        }

        return true;
    }   // process_client_drive_directory_control_response

    bool process_client_drive_query_information_response(
        uint32_t total_length, uint32_t flags, InStream& chunk,
        uint32_t FsInformationClass, uint32_t DeviceId, uint32_t FileId)
    {
        (void)total_length;
        (void)flags;
        switch (FsInformationClass) {
            case rdpdr::FileBasicInformation:
            {
                // Length(4)
                ::check_throw(chunk, 4, "FileSystemVirtualChannel::process_client_drive_query_information_response DR_DRIVE_QUERY_INFORMATION_RSP FileBasicInformation", ERR_RDP_DATA_TRUNCATED);

                uint32_t Length = chunk.in_uint32_le(); // Length(4)

                if (Length) {
                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                        fscc::FileBasicInformation file_basic_information;

                        file_basic_information.receive(chunk);
                        file_basic_information.log(LOG_INFO);
                    }
                }
            }
            break;

            case rdpdr::FileStandardInformation:
            {
                // Length(4)
                ::check_throw(chunk, 4, "FileSystemVirtualChannel::process_client_drive_query_information_response DR_DRIVE_QUERY_INFORMATION_RSP FileStandardInformation", ERR_RDP_DATA_TRUNCATED);

                uint32_t Length = chunk.in_uint32_le(); // Length(4)

                if (Length) {
                    fscc::FileStandardInformation file_standard_information;
                    file_standard_information.receive(chunk);

                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                        file_standard_information.log(LOG_INFO);
                    }

                    auto iter = this->find_target_info(DeviceId, FileId);
                    if (iter != this->device_io_target_info_inventory.end()) {
                        iter->end_of_file = file_standard_information.EndOfFile;
                    }
                }
            }
            break;

            default:
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_WARNING,
                    "FileSystemVirtualChannel::process_client_drive_query_information_response:"
                    " Undecoded FsInformationClass - %s(0x%X)",
                    rdpdr::ServerDriveQueryInformationRequest::get_FsInformationClass_name(
                        FsInformationClass),
                    FsInformationClass);
            break;
        }

        return true;
    }   // process_client_drive_query_information_response

    bool process_client_drive_query_volume_information_response(
        uint32_t total_length, uint32_t flags, InStream& chunk,
        uint32_t FsInformationClass)
    {
        (void)total_length;
        (void)flags;
        switch (FsInformationClass) {
            case rdpdr::FileFsVolumeInformation:
            {
                // Length(4)
                ::check_throw(chunk, 4, "FileSystemVirtualChannel::process_client_drive_query_volume_information_response DR_DRIVE_QUERY_VOLUME_INFORMATION_RSP FileFsVolumeInformation", ERR_RDP_DATA_TRUNCATED);

                uint32_t Length = chunk.in_uint32_le(); // Length(4)

                if (Length) {
                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                        fscc::FileFsVolumeInformation
                            file_fs_volume_information;

                        //auto chunk_p = chunk.get_current();

                        file_fs_volume_information.receive(chunk);

                        //LOG(LOG_INFO, "FileFsVolumeInformation: size=%u",
                        //    (unsigned int)(chunk.get_current() - chunk_p));
                        //hexdump(chunk_p, chunk.get_current() - chunk_p);

                        file_fs_volume_information.log(LOG_INFO);
                    }
                }
            }
            break;

            case rdpdr::FileFsAttributeInformation:
            {
                // Length(4)
                ::check_throw(chunk, 4, "FileSystemVirtualChannel::process_client_drive_query_volume_information_response DR_DRIVE_QUERY_VOLUME_INFORMATION_RSP FileFsAttributeInformation", ERR_RDP_DATA_TRUNCATED);

                uint32_t Length = chunk.in_uint32_le(); // Length(4)

                if (Length) {
                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                        fscc::FileFsAttributeInformation
                            file_fs_Attribute_information;

                        //auto chunk_p = chunk.get_current();

                        file_fs_Attribute_information.receive(chunk);

                        //LOG(LOG_INFO, "FileFsAttributeInformation: size=%u",
                        //    (unsigned int)(chunk.get_current() - chunk_p));
                        //hexdump(chunk_p, chunk.get_current() - chunk_p);

                        file_fs_Attribute_information.log(LOG_INFO);
                    }
                }
            }
            break;

            default:
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_WARNING,
                    "FileSystemVirtualChannel::process_client_drive_query_volume_information_response:"
                    " Undecoded FsInformationClass - %s(0x%X)",
                    rdpdr::get_FsInformationClass_name(FsInformationClass),
                    FsInformationClass);
            break;
        }

        return true;
    }

    bool process_client_drive_io_response(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        size_t const chunk_offset = chunk.get_offset();

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->client_device_io_response.receive(chunk);
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                this->client_device_io_response.log(LOG_INFO);
            }
        }

        auto request_iter = this->find_request_info(this->client_device_io_response.DeviceId(), this->client_device_io_response.CompletionId());
        if (request_iter == this->device_io_request_info_inventory.end()) {
            LOG(LOG_WARNING,
                "FileSystemVirtualChannel::process_client_drive_io_response: "
                    "The corresponding "
                    "Server Drive I/O Request is not found! "
                    "DeviceId=%" PRIu32 " CompletionId=%" PRIu32,
                this->client_device_io_response.DeviceId(),
                this->client_device_io_response.CompletionId());

            return true;
        }

        auto target_iter = this->find_target_info(this->client_device_io_response.DeviceId(), request_iter->file_id);

        if (!(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            if (rdpdr::IRP_MJ_READ == request_iter->major_function) {
                if (this->device_io_target_info_inventory.end() != target_iter) {
                    if (target_iter->for_sequential_access) {
                        uint32_t const effective_chunked_data_length = std::min<uint32_t>(chunk.in_remain(), request_iter->remaining);

                        if ((flags & CHANNELS::CHANNEL_FLAG_LAST) &&
                            (chunk.in_remain() != request_iter->remaining)) {
                            LOG(LOG_ERR,
                                "FileSystemVirtualChannel::process_client_drive_io_response: "
                                    "in_remain(%zu) != remaining=(%" PRIu32 ")",
                                chunk.in_remain(), request_iter->remaining);
                            assert(false);
                        }

                        target_iter->sha256.update({ chunk.get_current(), effective_chunked_data_length });

                        request_iter->remaining -= effective_chunked_data_length;
                    }
                }
                else {
                    LOG(LOG_WARNING,
                        "FileSystemVirtualChannel::process_client_drive_io_response: "
                            "Target not found! (1)");
                    assert(false);
                }
            }

            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                unordered_erase(this->device_io_request_info_inventory, request_iter);
            }

            return true;
        }

        const uint32_t     FileId        = request_iter->file_id;
        const uint32_t     MajorFunction = request_iter->major_function;
        const uint32_t     extra_data    = request_iter->extra_data;
        const std::string& file_path     = request_iter->file_path;

        LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
            "FileSystemVirtualChannel::process_client_drive_io_response:"
            " FileId=%u MajorFunction=%s(0x%08X) extra_data=0x%X"
            " file_path=\"%s\"",
            FileId,
            rdpdr::get_MajorFunction_name(MajorFunction),
            MajorFunction, extra_data, file_path);

        switch (MajorFunction)
        {
            case rdpdr::IRP_MJ_CREATE:
            {
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_client_drive_io_response:"
                    " Create request.");

                rdpdr::DeviceCreateResponse device_create_response;

                device_create_response.receive(chunk,
                    this->client_device_io_response.IoStatus());
                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                    device_create_response.log(LOG_INFO);
                }

                if (this->client_device_io_response.IoStatus() == erref::NTSTATUS::STATUS_SUCCESS) {
                    std::string const * p_device_name =
                        this->device_redirection_manager.get_device_name(
                            this->client_device_io_response.DeviceId());

                    {
                        rdpdr::RDPDR_DTYP device_type =
                            this->device_redirection_manager.get_device_type(
                                this->client_device_io_response.DeviceId());
                        if (rdpdr::RDPDR_DTYP_FILESYSTEM != device_type) {
                            auto device_name = (p_device_name) ? *p_device_name : ""_av;
                            auto device_type_name = rdpdr::DeviceAnnounceHeader_get_DeviceType_friendly_name(device_type);

                            this->session_log.log6(
                                LogId::DRIVE_REDIRECTION_USE, {
                                KVLog("device_name"_av, device_name),
                                KVLog("device_type"_av, device_type_name),
                            });

                            LOG_IF(!this->param_dont_log_data_into_syslog, LOG_INFO,
                                "type=DRIVE_REDIRECTION_USE device_name=%s device_type=%s",
                                device_name.data(), device_type_name.data());
                        }
                    }

                    if (p_device_name) {
                        std::string target_file_name = *p_device_name + file_path;

                        LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                            "FileSystemVirtualChannel::process_client_drive_io_response:"
                            " Add \"%s\" to known file list."
                            " DeviceId=%" PRIu32 " FileId=%" PRIu32,
                            target_file_name,
                            this->client_device_io_response.DeviceId(),
                            device_create_response.FileId());

                        this->device_io_target_info_inventory.push_back({
                            this->client_device_io_response.DeviceId(),
                            device_create_response.FileId(),
                            std::move(target_file_name),    // file_path
                            false,                          // for_reading
                            false,                          // for_writing
                            true,                           // for_sequential_access
                            END_OF_FILE_UNREAD,             // end_of_file
                            0,                              // sequential_access_offset
                            {}
                        });
                    }
                    else {
                        LOG(LOG_WARNING,
                            "FileSystemVirtualChannel::process_client_drive_io_response: "
                                "Device not found. DeviceId=%" PRIu32,
                            this->client_device_io_response.DeviceId());

                        //assert(false);
                    }
                }
            }
            break;

            case rdpdr::IRP_MJ_CLOSE:
            {
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_client_drive_io_response:"
                    " Close request.");

                if (this->device_io_target_info_inventory.end() != target_iter) {
                    rdpdr::RDPDR_DTYP const device_type =
                        this->device_redirection_manager.get_device_type(
                            this->server_device_io_request.DeviceId());
                    if ((rdpdr::RDPDR_DTYP_FILESYSTEM == device_type) &&
                        (target_iter->for_reading != target_iter->for_writing)) {
                        auto const DESKTOP_INI_FILENAME = "/desktop.ini"_av;
                        if (target_iter->sequential_access_offset &&
                            !::utils::ends_case_with(file_path, DESKTOP_INI_FILENAME)) {
                            if (target_iter->for_reading) {
                                if (target_iter->sequential_access_offset == target_iter->end_of_file) {
                                    uint8_t digest[SslSha256::DIGEST_LENGTH] = { 0 };

                                    target_iter->sha256.final(digest);

                                    char digest_s[128];
                                    size_t digest_s_len = snprintf(digest_s, sizeof(digest_s),
                                        "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
                                        "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                                        digest[ 0], digest[ 1], digest[ 2], digest[ 3], digest[ 4], digest[ 5], digest[ 6], digest[ 7],
                                        digest[ 8], digest[ 9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15],
                                        digest[16], digest[17], digest[18], digest[19], digest[20], digest[21], digest[22], digest[23],
                                        digest[24], digest[25], digest[26], digest[27], digest[28], digest[29], digest[30], digest[31]);

                                    LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                                        "FileSystemVirtualChannel::process_client_drive_io_response:"
                                        " File reading. Length=%" PRId64 " SHA-256=%s",
                                        target_iter->end_of_file, digest_s);

                                    auto const file_size_str = std::to_string(target_iter->end_of_file);
                                    this->session_log.log6(
                                        LogId::DRIVE_REDIRECTION_READ_EX, {
                                        KVLog("file_name"_av, file_path),
                                        KVLog("size"_av, file_size_str),
                                        KVLog("sha256"_av, {digest_s, digest_s_len}),
                                    });

                                    LOG_IF(!this->param_dont_log_data_into_syslog, LOG_INFO,
                                        "type=DRIVE_REDIRECTION_READ_EX file_name=%s"
                                        "size=%s sha256=%s",
                                        file_path, file_size_str, digest_s);
                                }
                                else {
                                    this->session_log.log6(
                                        LogId::DRIVE_REDIRECTION_READ, {
                                        KVLog("file_name"_av, file_path),
                                    });

                                    LOG_IF(!this->param_dont_log_data_into_syslog, LOG_INFO,
                                        "type=DRIVE_REDIRECTION_READ file_name=%s", file_path);
                                }
                            }
                            else if (target_iter->for_writing) {
                                if (target_iter->sequential_access_offset >= target_iter->end_of_file) {
                                    uint8_t digest[SslSha256::DIGEST_LENGTH] = { 0 };

                                    target_iter->sha256.final(digest);

                                    char digest_s[128];
                                    size_t digest_s_len = snprintf(digest_s, sizeof(digest_s),
                                        "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
                                        "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                                        digest[ 0], digest[ 1], digest[ 2], digest[ 3], digest[ 4], digest[ 5], digest[ 6], digest[ 7],
                                        digest[ 8], digest[ 9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15],
                                        digest[16], digest[17], digest[18], digest[19], digest[20], digest[21], digest[22], digest[23],
                                        digest[24], digest[25], digest[26], digest[27], digest[28], digest[29], digest[30], digest[31]);

                                    auto const file_size_str = std::to_string(target_iter->end_of_file);

                                    this->session_log.log6(
                                        LogId::DRIVE_REDIRECTION_WRITE_EX, {
                                        KVLog("file_name"_av, file_path),
                                        KVLog("size"_av, file_size_str),
                                        KVLog("sha256"_av, {digest_s, digest_s_len}),
                                    });

                                    LOG_IF(!this->param_dont_log_data_into_syslog, LOG_INFO,
                                        "type=DRIVE_REDIRECTION_WRITE_EX file_name=%s"
                                        "size=%s sha256=%s",
                                        file_path, file_size_str, digest_s);
                                }
                                else if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                    this->session_log.log6(
                                        LogId::DRIVE_REDIRECTION_WRITE, {
                                        KVLog("file_name"_av, file_path),
                                    });

                                    LOG_IF(!this->param_dont_log_data_into_syslog, LOG_INFO,
                                        "type=DRIVE_REDIRECTION_WRITE file_name=%s", file_path);
                                }
                            }
                        }
                    }

                    LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                        "FileSystemVirtualChannel::process_client_drive_io_response:"
                        " Remove \"%s\" from known file list."
                        " DeviceId=%" PRIu32 " FileId=%" PRIu32 " EndOfFile=%" PRId64,
                        file_path.c_str(),
                        this->client_device_io_response.DeviceId(),
                        FileId, target_iter->end_of_file);

                    unordered_erase(this->device_io_target_info_inventory, target_iter);
                }
            }
            break;

            case rdpdr::IRP_MJ_READ:
            {
                // Length(4)
                ::check_throw(chunk, 4, "FileSystemVirtualChannel::process_client_drive_io_response Device Read Response", ERR_RDP_DATA_TRUNCATED);

                const uint32_t Length = chunk.in_uint32_le();

                request_iter->remaining = Length;

                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_client_drive_io_response:"
                    " Read request. Length=%" PRIu32,
                    Length);

                if (this->client_device_io_response.IoStatus() == erref::NTSTATUS::STATUS_SUCCESS) {
                    if (this->device_io_target_info_inventory.end() != target_iter) {
                        if (target_iter->for_sequential_access) {
                            if (!target_iter->for_writing &&
                                (static_cast<uint64_t>(target_iter->sequential_access_offset) == request_iter->offset)) {

                                uint32_t const effective_chunked_data_length = std::min<uint32_t>(chunk.in_remain(), request_iter->remaining);

                                target_iter->sha256.update({ chunk.get_current(), effective_chunked_data_length });

                                request_iter->remaining -= effective_chunked_data_length;

                                target_iter->sequential_access_offset += Length;
                            }
                            else {
                                target_iter->for_sequential_access = false;
                            }
                        }

                        target_iter->for_reading = true;
                    }
                    else {
                        LOG(LOG_WARNING,
                            "FileSystemVirtualChannel::process_client_drive_io_response: "
                                "Target not found! (2)");
                        assert(false);
                    }
                }
            }
            break;

            case rdpdr::IRP_MJ_WRITE:
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_client_drive_io_response:"
                    " Write request.");

                if (this->client_device_io_response.IoStatus() == erref::NTSTATUS::STATUS_SUCCESS) {
                    if (this->device_io_target_info_inventory.end() != target_iter) {
                        target_iter->for_writing = true;
                    }
                    else {
                        LOG(LOG_WARNING,
                            "FileSystemVirtualChannel::process_client_drive_io_response: "
                                "Target not found! (3)");
                        assert(false);
                    }
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
                    /* FsInformationClass = */extra_data, this->client_device_io_response.DeviceId(), FileId);
            break;

            case rdpdr::IRP_MJ_DIRECTORY_CONTROL:
                this->process_client_drive_directory_control_response(
                    total_length, flags, chunk,
                    /* FsInformationClass = */extra_data);
            break;

            case rdpdr::IRP_MJ_SET_INFORMATION:
                if (this->client_device_io_response.IoStatus() == erref::NTSTATUS::STATUS_SUCCESS) {
                    switch (extra_data) {
                        case rdpdr::FileDispositionInformation:
                        {
                            if (this->device_io_target_info_inventory.end() != target_iter) {
                                this->session_log.log6(
                                    LogId::DRIVE_REDIRECTION_DELETE, {
                                    KVLog("file_name"_av, file_path),
                                });

                                LOG_IF(!this->param_dont_log_data_into_syslog, LOG_INFO,
                                    "type=DRIVE_REDIRECTION_DELETE file_name=%s", file_path);
                            }
                            else {
                                LOG(LOG_WARNING,
                                    "FileSystemVirtualChannel::process_client_drive_io_response: "
                                        "Target not found! (4)");
                                assert(false);
                            }
                        }
                        break;

                        case rdpdr::FileRenameInformation:
                        {
                            if (this->device_io_target_info_inventory.end() != target_iter) {
                                this->session_log.log6(
                                    LogId::DRIVE_REDIRECTION_RENAME, {
                                    KVLog("old_file_name"_av, target_iter->file_path),
                                    KVLog("new_file_name"_av, file_path),
                                });

                                LOG_IF(!this->param_dont_log_data_into_syslog, LOG_INFO,
                                    "type=DRIVE_REDIRECTION_RENAME old_file_name=%s new_file_name=%s",
                                    target_iter->file_path, file_path);
                            }
                            else {
                                LOG(LOG_WARNING,
                                    "FileSystemVirtualChannel::process_client_drive_io_response: "
                                        "Target not found! (5)");
                                assert(false);
                            }
                        }
                        break;
                    }
                }
            break;

            default:
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_WARNING,
                    "FileSystemVirtualChannel::process_client_drive_io_response:"
                    " Undecoded Drive I/O Response - MajorFunction=%s(0x%08X)",
                    rdpdr::get_MajorFunction_name(MajorFunction),
                    MajorFunction);
            break;
        }   // switch (MajorFunction)

        if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
            unordered_erase(this->device_io_request_info_inventory, request_iter);
        }

        bool send_message_to_server = true;

        uint32_t completion_id = this->client_device_io_response.CompletionId();

        if (this->file_system_completion_id_manager.is_dest_only_id(completion_id)) {
            this->file_system_completion_id_manager.unreg_dest_only_id(completion_id);

            send_message_to_server = false;
        }
        else if (this->client_device_io_response.map_completion_id(this->file_system_completion_id_manager)) {
            StaticOutStream<65536> out_chunk;
            out_chunk.out_copy_bytes(chunk.get_data(), chunk.get_capacity());

            OutStream out_stream(out_chunk.get_produced_bytes());
            out_stream.rewind(chunk_offset);
            this->client_device_io_response.emit(out_stream);

            this->send_message_to_server(total_length, flags, out_chunk.get_produced_bytes());

            send_message_to_server = false;
        }

        return send_message_to_server;
    }   // process_client_drive_io_response

    void process_client_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data) override
    {
        LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
            "FileSystemVirtualChannel::process_client_message:"
            " total_length=%" PRIu32 " flags=0x%08X chunk_data_length=%zu",
            total_length, flags, chunk_data.size());

        if (bool(this->verbose & RDPVerbose::rdpdr_dump)) {
            const bool send              = false;
            const bool from_or_to_client = true;
            ::msgdump_c(send, from_or_to_client, total_length, flags, chunk_data);
        }

        InStream chunk(chunk_data);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->client_message_header.receive(chunk);
        }

        bool send_message_to_server = true;

        switch (this->client_message_header.packet_id)
        {
            case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
                assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_message: "
                            "Client Announce Reply");

                    rdpdr::ClientAnnounceReply client_announce_reply;

                    client_announce_reply.receive(chunk);
                    client_announce_reply.log(LOG_INFO);
                }

                this->initialization_timeout_event.garbage();
            break;

            case rdpdr::PacketId::PAKID_CORE_CLIENT_NAME:
                assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_message: "
                            "Client Name Request");

                    rdpdr::ClientNameRequest client_name_request;

                    //auto chunk_p = chunk.get_current();

                    client_name_request.receive(chunk);

                    //LOG(LOG_INFO, "ClientNameRequest: size=%u",
                    //    (unsigned int)(chunk.get_current() - chunk_p));
                    //hexdump(chunk_p, chunk.get_current() - chunk_p);

                    client_name_request.log(LOG_INFO);
                }
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE:
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_client_message:"
                    " Client Device List Announce Request");

                send_message_to_server =
                    this->process_client_device_list_announce_request(
                        total_length, flags, chunk);

                //{
                //    uint8_t out_data[256];
                //    WriteOnlyStream out_stream(out_data, sizeof(out_data));
                //
                //    rdpdr::SharedHeader client_message_header(
                //        rdpdr::Component::RDPDR_CTYP_CORE,
                //        rdpdr::PacketId::PAKID_CORE_DEVICELIST_REMOVE);
                //
                //    client_message_header.emit(out_stream);
                //
                //    out_stream.out_uint32_le(1);    // DeviceCount(4)
                //
                //    out_stream.out_uint32_le(1);    // DeviceId(4)
                //
                //    out_stream.mark_end();
                //
                //    this->process_client_message(out_stream.size(),
                //        CHANNELS::CHANNEL_FLAG_FIRST |
                //            CHANNELS::CHANNEL_FLAG_LAST,
                //        out_data, out_stream.size());
                //}
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION:
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_client_message:"
                    " Client Drive I/O Response");

                send_message_to_server =
                    this->process_client_drive_io_response(
                        total_length, flags, chunk);
            break;

            case rdpdr::PacketId::PAKID_CORE_CLIENT_CAPABILITY:
                assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_client_message:"
                    " Client Core Capability Response");

                send_message_to_server =
                    this->process_client_core_capability_response(
                        total_length, flags, chunk);
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICELIST_REMOVE:
                assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_client_message:"
                    " Client Drive Device List Remove");

                this->device_redirection_manager.process_client_drive_device_list_remove(
                    total_length, flags, chunk);

                send_message_to_server = false;
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICE_REPLY:
            case rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE:
            case rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST:
            case rdpdr::PacketId::PAKID_CORE_SERVER_CAPABILITY:
            case rdpdr::PacketId::PAKID_PRN_CACHE_DATA:
            case rdpdr::PacketId::PAKID_CORE_USER_LOGGEDON:
            case rdpdr::PacketId::PAKID_PRN_USING_XPS:
            default:
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr)
                    && (flags & CHANNELS::CHANNEL_FLAG_FIRST), LOG_WARNING,
                    "FileSystemVirtualChannel::process_client_message: "
                        "Undecoded PDU - Component=%s(0x%" PRIX16 ") PacketId=%s(0x%" PRIX16 ")",
                    rdpdr::SharedHeader::get_Component_name(
                        this->server_message_header.component),
                    static_cast<uint16_t>(this->server_message_header.component),
                    rdpdr::SharedHeader::get_PacketId_name(
                        this->server_message_header.packet_id),
                    static_cast<uint16_t>(this->server_message_header.packet_id));
            break;
        }   // switch (this->client_message_header.packet_id)

        if (send_message_to_server) {
            this->send_message_to_server(total_length, flags, chunk_data);
        }
    }   // process_client_message

    bool process_server_announce_request(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        rdpdr::ServerAnnounceRequest server_announce_request;

        server_announce_request.receive(chunk);

        serverVersionMinor = server_announce_request.VersionMinor();

        if (bool(this->verbose & RDPVerbose::rdpdr)) {
            server_announce_request.log(LOG_INFO);
        }

        this->server_major_version_number = server_announce_request.VersionMajor();
        this->server_generated_client_id = server_announce_request.ClientId();

        // Virtual channel is opened at client side and is authorized.
        if (this->has_valid_to_client_sender()) {
            this->initialization_timeout_event = this->events_guard.create_event_timeout(
                "Initialisation timeout Event",
                this->initialization_timeout,
                [this](Event&)
                {
                    this->process_event();
                });
            return true;
        }

        uint8_t message_buffer[1024];

        {
            OutStream out_stream(message_buffer);

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
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_server_announce_request:");
                client_announce_reply.log(LOG_INFO);
            }
            client_announce_reply.emit(out_stream);

            this->send_message_to_server(
                out_stream.get_offset(),
                  CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_produced_bytes());
        }

        {
            OutStream out_stream(message_buffer);

            rdpdr::SharedHeader clent_message_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_CLIENT_NAME);
            clent_message_header.emit(out_stream);

            rdpdr::ClientNameRequest client_name_request(
                this->param_client_name);
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_server_announce_request:");
                client_name_request.log(LOG_INFO);
            }
            client_name_request.emit(out_stream);

            this->send_message_to_server(
                out_stream.get_offset(),
                  CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_produced_bytes());
        }

        return false;
    }   // process_server_announce_request

    bool process_server_client_id_confirm(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;
        (void)chunk;

        if (this->drive_redirection_initialize_notifier) {
            if (!this->drive_redirection_initialize_notifier->on_drive_redirection_initialize()) {
                this->drive_redirection_initialize_notifier = nullptr;
            }
        }

        // Virtual channel is opened at client side and is authorized.
        if (this->has_valid_to_client_sender() && !this->disable_client_sender) {
            return true;
        }

        {
            StaticOutStream<1024> out_stream;

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
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_server_client_id_confirm:");
                general_capability_set.log(LOG_INFO);
            }
            general_capability_set.emit(out_stream);

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

            this->send_message_to_server(
                out_stream.get_offset(),
                  CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_produced_bytes());
        }

        return false;
    }   // process_server_client_id_confirm

    bool process_server_create_drive_request(uint32_t total_length,
        uint32_t flags, InStream& chunk,
        std::string& file_path)
    {
        (void)total_length;
        (void)flags;

        rdpdr::DeviceCreateRequest device_create_request;

        //auto chunk_p = chunk.get_current();

        device_create_request.receive(chunk);

        //LOG(LOG_INFO, "DeviceCreateRequest: size=%u",
        //    (unsigned int)(chunk.get_current() - chunk_p));
        //hexdump(chunk_p, chunk.get_current() - chunk_p);

        if (bool(this->verbose & RDPVerbose::rdpdr)) {
            device_create_request.log(LOG_INFO);
        }

        bool access_ok = true;

        if (this->device_redirection_manager.is_known_device(
                this->server_device_io_request.DeviceId())) {
            // Is a File system device.
            const uint32_t DesiredAccess =
                device_create_request.DesiredAccess();

            if (!this->param_file_system_read_authorized &&
                smb2::read_access_is_required(DesiredAccess,
                                              /* strict_check = */false) &&
                !(device_create_request.CreateOptions() &
                  smb2::FILE_DIRECTORY_FILE) &&
                0 != ::strcmp(device_create_request.Path().data(), "/")) {
                access_ok = false;
            }
            if (!this->param_file_system_write_authorized &&
                smb2::write_access_is_required(DesiredAccess,
                                               /* strict_check = */false)) {
                access_ok = false;
            }
        }

        if (!access_ok)
        {
            StaticOutStream<1024> out_stream;

            const rdpdr::SharedHeader clent_message_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);

            clent_message_header.emit(out_stream);

            const rdpdr::DeviceIOResponse device_io_response(
                    this->server_device_io_request.DeviceId(),
                    this->server_device_io_request.CompletionId(),
                    erref::NTSTATUS::STATUS_ACCESS_DENIED
                );

            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                device_io_response.log(LOG_INFO);
            }
            device_io_response.emit(out_stream);

            const rdpdr::DeviceCreateResponse device_create_response(
                static_cast<uint32_t>(-1), 0);
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                device_create_response.log(LOG_INFO);
            }
            device_create_response.emit(out_stream);

            this->send_message_to_server(
                out_stream.get_offset(),
                  CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_produced_bytes());

            return false;
        }

        auto av = device_create_request.Path();
        file_path.assign(av.begin(), av.end());

        return true;
    }   // process_server_create_drive_request

    bool process_server_drive_io_request(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        size_t const chunk_offset = chunk.get_offset();

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->server_device_io_request.receive(chunk);
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                this->server_device_io_request.log(LOG_INFO);
            }
        }

        if (this->file_system_drive_manager.is_managed_drive(
                this->server_device_io_request.DeviceId()))
        {
            this->file_system_drive_manager.process_device_IO_request(
                this->server_device_io_request,
                (flags & CHANNELS::CHANNEL_FLAG_FIRST),
                chunk,
                this->to_server_sender);

            return false;
        }

        auto target_iter = this->device_io_target_info_inventory.end();

        if (this->server_device_io_request.MajorFunction() != rdpdr::IRP_MJ_CREATE) {
            target_iter = this->find_target_info(
                    this->server_device_io_request.DeviceId(),
                    this->server_device_io_request.FileId()
                );
        }

        if (!(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            if (this->server_device_io_request.MajorFunction() == rdpdr::IRP_MJ_WRITE) {
                if (this->device_io_target_info_inventory.end() != target_iter) {
                    if (target_iter->for_sequential_access) {
                        auto request_iter = this->find_request_info(this->server_device_io_request.DeviceId(), this->server_device_io_request.CompletionId());
                        if (this->device_io_request_info_inventory.end() != request_iter) {
                            uint32_t const effective_chunked_data_length = std::min<uint32_t>(chunk.in_remain(), request_iter->remaining);

                            if ((flags & CHANNELS::CHANNEL_FLAG_LAST) &&
                                (chunk.in_remain() != request_iter->remaining)) {
                                LOG(LOG_WARNING,
                                    "FileSystemVirtualChannel::process_server_drive_io_request: "
                                        "in_remain(%zu) != remaining=(%" PRIu32 ")",
                                    chunk.in_remain(), request_iter->remaining);
                                assert(false);
                            }

                            target_iter->sha256.update({ chunk.get_current(), effective_chunked_data_length });

                            request_iter->remaining -= effective_chunked_data_length;
                        }
                        else {
                            LOG(LOG_WARNING,
                                "FileSystemVirtualChannel::process_server_drive_io_request: "
                                    "Request not found!");
                            assert(false);
                        }
                    }
                }
                else {
                    LOG(LOG_WARNING,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Target not found! (1)");
                    assert(false);
                }
            }

            return true;
        }

        uint32_t extra_data = 0;
        uint64_t offset     = 0;
        uint32_t length     = 0;
        uint32_t remaining  = 0;

        bool send_message_to_client  = true;
        bool do_not_add_to_inventory = false;

        std::string file_path;

        if (this->device_io_target_info_inventory.end() != target_iter) {
            file_path = target_iter->file_path;
        }

        if ((this->server_device_io_request.MajorFunction() == rdpdr::IRP_MJ_READ) ||
            (this->server_device_io_request.MajorFunction() == rdpdr::IRP_MJ_WRITE)) {

            rdpdr::RDPDR_DTYP device_type =
                this->device_redirection_manager.get_device_type(
                    this->server_device_io_request.DeviceId());
            if ((rdpdr::RDPDR_DTYP_FILESYSTEM == device_type) &&
                (this->device_io_target_info_inventory.end() != target_iter) &&
                (target_iter->end_of_file == END_OF_FILE_UNREAD))
            {
                const rdpdr::SharedHeader shared_header(
                        rdpdr::Component::RDPDR_CTYP_CORE,
                        rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST
                    );
                const uint32_t completion_id = this->file_system_completion_id_manager.reg_dest_only_id();
                const uint32_t MinorFunction = 0;
                const rdpdr::DeviceIORequest device_io_request(
                        this->server_device_io_request.DeviceId(),
                        this->server_device_io_request.FileId(),
                        completion_id,
                        rdpdr::IRP_MJ_QUERY_INFORMATION,
                        MinorFunction
                    );
                const rdpdr::ServerDriveQueryInformationRequest
                    server_drive_query_information_request(
                           rdpdr::FileStandardInformation
                        );
                StaticOutStream<65536> out_stream;

                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_server_drive_io_request: "
                        "Send proxy-managed Server Drive Query Information Request. "
                        "FilePath=\"%s\"",
                    file_path);

                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                    shared_header.log(LOG_INFO);
                }
                shared_header.emit(out_stream);

                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                    device_io_request.log(LOG_INFO);
                }
                device_io_request.emit(out_stream);

                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                    server_drive_query_information_request.log(LOG_INFO);
                }
                server_drive_query_information_request.emit(out_stream);

                this->send_message_to_client(out_stream.get_offset(), flags, out_stream.get_produced_bytes());

                this->device_io_request_info_inventory.push_back({
                    this->server_device_io_request.DeviceId(),
                    this->server_device_io_request.FileId(),
                    completion_id,
                    file_path,
                    rdpdr::IRP_MJ_QUERY_INFORMATION,
                    rdpdr::FileStandardInformation,
                    0,                                          // offset
                    0,                                          // length
                    0                                           // remaining
                });

                target_iter->end_of_file = END_OF_FILE_READING;
            }
        }

        switch (this->server_device_io_request.MajorFunction())
        {
            case rdpdr::IRP_MJ_CREATE:
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_server_drive_io_request: "
                        "Device Create Request");

                send_message_to_client =
                    this->process_server_create_drive_request(
                        total_length, flags, chunk, file_path);
            break;

            case rdpdr::IRP_MJ_CLOSE:
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_server_drive_io_request: "
                        "Device Close Request");
            break;

            case rdpdr::IRP_MJ_READ:
                {
                    // Length(4) + Offset(8)
                    ::check_throw(chunk, 12, "FileSystemVirtualChannel::process_server_drive_io_request Device Read Request", ERR_RDPDR_PDU_TRUNCATED);


                length    = chunk.in_uint32_le();
                offset    = chunk.in_uint64_le();
                remaining = length;

                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_server_drive_io_request: "
                        "Read request. Length=%u Offset=%" PRIu64,
                    length, offset);
                }
            break;

            case rdpdr::IRP_MJ_WRITE:
            {
                // Length(4) + Offset(8) + Padding(20)
                ::check_throw(chunk, 32, "FileSystemVirtualChannel::process_server_drive_io_request Device Write Request", ERR_RDPDR_PDU_TRUNCATED);

                length    = chunk.in_uint32_le();
                offset    = chunk.in_uint64_le();
                remaining = length;

                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_server_drive_io_request: "
                        "Write Request. Length=%" PRIu32 " Offset=%" PRIu64,
                    length, offset);

                chunk.in_skip_bytes(20);    // Padding(20)

                if (this->device_io_target_info_inventory.end() != target_iter) {
                    if (target_iter->for_sequential_access) {
                        if (!target_iter->for_reading &&
                            (static_cast<uint64_t>(target_iter->sequential_access_offset) == offset)) {

                            uint32_t const effective_chunked_data_length = std::min<uint32_t>(chunk.in_remain(), remaining);

                            target_iter->sha256.update({ chunk.get_current(), effective_chunked_data_length });

                            remaining -= effective_chunked_data_length;

                            target_iter->sequential_access_offset += length;
                        }
                        else {
                            target_iter->for_sequential_access = false;
                        }
                    }
                }
                else {
                    LOG(LOG_WARNING,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Target not found! (2)");
                    assert(false);
                }
            }
            break;

            case rdpdr::IRP_MJ_DEVICE_CONTROL:
                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Device control request");

                    rdpdr::DeviceControlRequest device_control_request;

                    device_control_request.receive(chunk);
                    device_control_request.log(LOG_INFO);
                }
            break;

            case rdpdr::IRP_MJ_QUERY_VOLUME_INFORMATION:
            {
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_server_drive_io_request: "
                        "Query volume information request");

                rdpdr::ServerDriveQueryVolumeInformationRequest
                    server_drive_query_volume_information_request;

                server_drive_query_volume_information_request.receive(
                    chunk);
                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                    server_drive_query_volume_information_request.log(
                        LOG_INFO);
                }

                extra_data =
                    server_drive_query_volume_information_request.FsInformationClass();
            }
            break;

            case rdpdr::IRP_MJ_QUERY_INFORMATION:
            {
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_server_drive_io_request: "
                        "Server Drive Query Information Request");

                rdpdr::ServerDriveQueryInformationRequest
                    server_drive_query_information_request;

                server_drive_query_information_request.receive(chunk);
                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                    server_drive_query_information_request.log(LOG_INFO);
                }

                extra_data =
                    server_drive_query_information_request.FsInformationClass();
            }
            break;

            case rdpdr::IRP_MJ_DIRECTORY_CONTROL:
                if (this->server_device_io_request.MinorFunction() ==
                    rdpdr::IRP_MN_QUERY_DIRECTORY) {
                    LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Server Drive Query Directory Request");

                    rdpdr::ServerDriveQueryDirectoryRequest
                        server_drive_query_directory_request;

                    server_drive_query_directory_request.receive(chunk);
                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                        server_drive_query_directory_request.log(LOG_INFO);
                    }

                    extra_data =
                        server_drive_query_directory_request.FsInformationClass();
                }
                else {
                    do_not_add_to_inventory = true;
                }
            break;

            case rdpdr::IRP_MJ_SET_INFORMATION:
            {
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_server_drive_io_request: "
                        "Server Drive Set Information Request");

                rdpdr::ServerDriveSetInformationRequest
                    server_drive_set_information_request;

                server_drive_set_information_request.receive(chunk);
                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                    server_drive_set_information_request.log(LOG_INFO);
                }

                extra_data =
                    server_drive_set_information_request.FsInformationClass();

                switch (server_drive_set_information_request.FsInformationClass())
                {
                    case rdpdr::FileEndOfFileInformation: {

                        // EndOfFile(8)
                        ::check_throw(chunk, 8, "FileSystemVirtualChannel::process_server_drive_io_request FILE_END_OF_FILE_INFORMATION", ERR_RDPDR_PDU_TRUNCATED);

                        int64_t EndOfFile = chunk.in_sint64_le();

                        LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                            "FileSystemVirtualChannel::process_server_drive_io_request: "
                                "EndOfFile=%" PRId64,
                            EndOfFile);

                        if (this->device_io_target_info_inventory.end() != target_iter) {
                            target_iter->end_of_file = EndOfFile;
                        }
                        else {
                            LOG(LOG_WARNING,
                                "FileSystemVirtualChannel::process_server_drive_io_request: "
                                    "Target not found! (3)");
                            assert(false);
                        }
                    }
                    break;

                    case rdpdr::FileRenameInformation: {
                        rdpdr::RDPFileRenameInformation rdp_file_rename_information;

                        rdp_file_rename_information.receive(chunk);

                        if (bool(verbose & RDPVerbose::rdpdr)) {
                            rdp_file_rename_information.log(LOG_INFO);
                        }

                        std::string const * device_name =
                            this->device_redirection_manager.get_device_name(
                                this->server_device_io_request.DeviceId());
                        if (device_name) {
                            file_path =
                                *device_name + rdp_file_rename_information.FileName();
                            LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                                "FileSystemVirtualChannel::process_server_drive_io_request: "
                                    "FileName=\"%s\"",
                                file_path);
                        }
                    }
                    break;
                }
            }
            break;

            default:
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_WARNING,
                    "FileSystemVirtualChannel::process_server_drive_io_request: "
                        "Undecoded Drive I/O Request - "
                        "MajorFunction=%s(0x%08X) "
                        "MinorFunction=%s(0x%08X)",
                    rdpdr::get_MajorFunction_name(
                        this->server_device_io_request.MajorFunction()),
                    this->server_device_io_request.MajorFunction(),
                    rdpdr::get_MinorFunction_name(
                        this->server_device_io_request.MinorFunction()),
                    this->server_device_io_request.MinorFunction());
            break;
        }   // switch (this->server_device_io_request.MajorFunction())

        bool send_replaced_message_to_client = false;

        if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) && send_message_to_client &&
            this->server_device_io_request.map_completion_id(this->file_system_completion_id_manager)) {

            StaticOutStream<65536> out_chunk;
            out_chunk.out_copy_bytes(chunk.get_data(), chunk.get_capacity());

            OutStream out_stream(out_chunk.get_produced_bytes());
            out_stream.rewind(chunk_offset);
            this->server_device_io_request.emit(out_stream);

            this->send_message_to_client(total_length, flags, out_chunk.get_produced_bytes());

            send_message_to_client          = false;
            send_replaced_message_to_client = true;
        }

        if ((send_message_to_client || send_replaced_message_to_client) &&
            !do_not_add_to_inventory) {

            this->device_io_request_info_inventory.push_back({
                this->server_device_io_request.DeviceId(),
                this->server_device_io_request.FileId(),
                this->server_device_io_request.CompletionId(),
                std::move(file_path),
                this->server_device_io_request.MajorFunction(),
                extra_data,
                offset,                                         // offset
                length,                                         // length
                remaining                                       // remaining
            });
        }

        return send_message_to_client;
    }   // process_server_drive_io_request

    void process_server_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data) override
    {
        LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
            "FileSystemVirtualChannel::process_server_message: "
                "total_length=%" PRIu32 " flags=0x%08X chunk_data_length=%zu",
            total_length, flags, chunk_data.size());

        if (bool(this->verbose & RDPVerbose::rdpdr_dump)) {
            const bool send              = false;
            const bool from_or_to_client = false;
            ::msgdump_c(send, from_or_to_client, total_length, flags, chunk_data);
        }

        if (flags && !(flags &~ (CHANNELS::CHANNEL_FLAG_SUSPEND | CHANNELS::CHANNEL_FLAG_RESUME))) {
            this->send_message_to_client(total_length, flags, chunk_data);
            return;
        }

        InStream chunk(chunk_data);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->server_message_header.receive(chunk);
        }

        bool send_message_to_client = (this->has_valid_to_client_sender() && !this->disable_client_sender);

        switch (this->server_message_header.packet_id)
        {
            case rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE:
                if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) &&
                    bool(this->verbose & RDPVerbose::rdpdr)) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_message: "
                            "Server Announce Request");
                }

                send_message_to_client =
                    this->process_server_announce_request(total_length,
                        flags, chunk);
            break;

            case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
                assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_server_message: "
                        "Server Client ID Confirm");

                send_message_to_client =
                    this->process_server_client_id_confirm(total_length,
                        flags, chunk);
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICE_REPLY:
                assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_server_message: "
                        "Server Device Announce Response");

                this->device_redirection_manager.process_server_device_announce_response(
                    total_length, flags, chunk);
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST:
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_server_message: "
                        "Server Drive I/O Request");

                send_message_to_client =
                    this->process_server_drive_io_request(total_length, flags, chunk);
            break;

            case rdpdr::PacketId::PAKID_CORE_SERVER_CAPABILITY:
                assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_server_message: "
                        "Server Core Capability Request");
            break;

            case rdpdr::PacketId::PAKID_CORE_USER_LOGGEDON:
                assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_server_message: "
                        "Server User Logged On");

                if (!this->user_logged_on) {
                    this->file_system_drive_manager.announce_drive(
                        this->device_capability_version_02_supported,
                        this->device_redirection_manager.to_device_announce_collection_sender);
                }

                this->user_logged_on = true;

                this->device_redirection_manager.process_server_user_logged_on(
                    total_length, flags, chunk);
            break;

            case rdpdr::PacketId::PAKID_PRN_USING_XPS:
                assert((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr), LOG_INFO,
                    "FileSystemVirtualChannel::process_server_message: "
                        "Server Printer Set XPS Mode");
            break;

            case rdpdr::PacketId::PAKID_CORE_CLIENT_NAME:
            case rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE:
            case rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION:
            case rdpdr::PacketId::PAKID_CORE_CLIENT_CAPABILITY:
            case rdpdr::PacketId::PAKID_CORE_DEVICELIST_REMOVE:
            case rdpdr::PacketId::PAKID_PRN_CACHE_DATA:
            default:
                LOG_IF(bool(this->verbose & RDPVerbose::rdpdr)
                    && (flags & CHANNELS::CHANNEL_FLAG_FIRST),
                    LOG_WARNING,
                    "FileSystemVirtualChannel::process_server_message: "
                        "Undecoded PDU - "
                        "Component=%s(0x%" PRIX16 ") PacketId=%s(0x%" PRIX16 ")",
                    rdpdr::SharedHeader::get_Component_name(
                        this->server_message_header.component),
                    static_cast<uint16_t>(this->server_message_header.component),
                    rdpdr::SharedHeader::get_PacketId_name(
                        this->server_message_header.packet_id),
                    static_cast<uint16_t>(this->server_message_header.packet_id));
            break;
        }   // switch (this->server_message_header.packet_id)

        if (send_message_to_client) {
            this->send_message_to_client(total_length, flags, chunk_data);
        }
    }   // process_server_message

    void set_session_probe_launcher(SessionProbeLauncher* launcher) {
        this->drive_redirection_initialize_notifier = launcher;
        this->session_probe_device_announce_responded_notifier = launcher;
    }

private:
    void process_event() {
        this->initialization_timeout_event.garbage();
        uint8_t message_buffer[1024];

        {
            OutStream out_stream(message_buffer);

            rdpdr::SharedHeader clent_message_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
            clent_message_header.emit(out_stream);

            rdpdr::ClientAnnounceReply client_announce_reply(
                0x0001, // VersionMajor, MUST be set to 0x0001.
                0x0006, // Windows XP SP3.
                // [MS-RDPEFS] - 3.2.5.1.3 Sending a Client Announce
                //     Reply Message.
                ((this->server_major_version_number >= 12) ?
                 this->param_random_number :
                 this->server_generated_client_id));
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_event:");
                client_announce_reply.log(LOG_INFO);
            }
            client_announce_reply.emit(out_stream);

            this->send_message_to_server(
                out_stream.get_offset(),
                  CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_produced_bytes());
        }

        {
            OutStream out_stream(message_buffer);

            rdpdr::SharedHeader clent_message_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_CLIENT_NAME);
            clent_message_header.emit(out_stream);

            rdpdr::ClientNameRequest client_name_request(
                this->param_client_name);
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_event:");
                client_name_request.log(LOG_INFO);
            }
            client_name_request.emit(out_stream);

            this->send_message_to_server(
                out_stream.get_offset(),
                  CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_produced_bytes());
        }

        LOG(LOG_INFO,
            "FileSystemVirtualChannel::process_event:"
                "Initialization timeout reached.");
        this->disable_client_sender = true;
    }
};  // class FileSystemVirtualChannel
