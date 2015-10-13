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

#include <deque>

#include "apply_for_delim.hpp"
#include "base_channel.hpp"
#include "front_api.hpp"
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

    // DeviceId, FileId, CompletionId, MajorFunction, (extra data), Path.
    typedef std::vector<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t,
                                   uint32_t, std::unique_ptr<std::string>>>
        device_io_request_info_inventory_type;
    device_io_request_info_inventory_type device_io_request_info_inventory;

    // DeviceId, FileId, file_path, for_reading, for_writing.
    typedef std::vector<std::tuple<uint32_t, uint32_t,
                                   std::unique_ptr<std::string>, bool, bool>>
        device_io_target_info_inventory_type;
    device_io_target_info_inventory_type device_io_target_info_inventory;

    bool device_capability_version_02_supported = false;

    const char* const param_client_name;
    const bool        param_file_system_read_authorized;
    const bool        param_file_system_write_authorized;
    const uint32_t    param_random_number;                  // For ClientId.

    const bool        param_dont_log_data_into_syslog;
    const bool        param_dont_log_data_into_wrm;

    auth_api*         param_acl;

    bool proxy_managed_drives_announced = false;

    class DeviceRedirectionManager {
        // (Virtual Channel) data_length, (Virtual Channel) data.
        typedef std::deque<std::tuple<uint32_t, std::unique_ptr<uint8_t[]>>>
            device_announce_collection_type;
        device_announce_collection_type device_announces;

        // DeviceId, PreferredDosName.
        typedef std::vector<std::tuple<uint32_t,
                            std::unique_ptr<std::string>>>
            device_info_inventory_type;
        device_info_inventory_type device_info_inventory;

    public:
        class ToDeviceAnnounceCollectionSender :
            public VirtualChannelDataSender
        {
            device_announce_collection_type& device_announces;

            std::unique_ptr<uint8_t[]> device_announce_data;
            OutStream                  device_announce_stream;

        public:
            ToDeviceAnnounceCollectionSender(
                device_announce_collection_type& device_announces)
            : device_announces(device_announces) {}

            void operator()(uint32_t total_length, uint32_t flags,
                const uint8_t* chunk_data, uint32_t chunk_data_length)
                    override {
                REDASSERT((flags & CHANNELS::CHANNEL_FLAG_FIRST) ||
                          bool(this->device_announce_data));

                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                    this->device_announce_data =
                        std::make_unique<uint8_t[]>(total_length);

                    this->device_announce_stream = OutStream(
                        this->device_announce_data.get(), total_length);
                }

                REDASSERT(this->device_announce_stream.tailroom() >=
                    chunk_data_length);

                this->device_announce_stream.out_copy_bytes(chunk_data,
                    chunk_data_length);

                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    this->device_announces.push_back(
                        std::make_tuple(
                            this->device_announce_stream.get_offset(),
                            std::move(this->device_announce_data)));

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

        const uint32_t verbose;

    public:
        DeviceRedirectionManager(
            VirtualChannelDataSender* to_client_sender_,
            VirtualChannelDataSender* to_server_sender_,
            bool file_system_authorized,
            bool parallel_port_authorized,
            bool print_authorized,
            bool serial_port_authorized,
            bool smart_card_authorized,
            uint32_t channel_chunk_length,
            uint32_t verbose)
        : to_device_announce_collection_sender(device_announces)
        , to_client_sender(to_client_sender_)
        , to_server_sender(to_server_sender_)
        , param_file_system_authorized(file_system_authorized)
        , param_parallel_port_authorized(parallel_port_authorized)
        , param_print_authorized(print_authorized)
        , param_serial_port_authorized(serial_port_authorized)
        , param_smart_card_authorized(smart_card_authorized)
        , remaining_device_announce_request_header_stream(
              this->remaining_device_announce_request_header_data)
        , verbose(verbose) {
        }

    private:
        void add_known_device(uint32_t DeviceId,
                              const char* PreferredDosName) {
            this->device_info_inventory.push_back(
                std::make_tuple(DeviceId,
                    std::make_unique<std::string>(PreferredDosName)));
            if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::DeviceRedirectionManager::add_known_device: "
                        "Add \"%s\"(DeviceId=%u) to known dervice list.",
                    PreferredDosName, DeviceId);
            }
        }

    public:
        const char* get_device_name(uint32_t DeviceId) {
            for (device_info_inventory_type::const_iterator iter =
                     this->device_info_inventory.cbegin();
                 iter != this->device_info_inventory.cend(); ++iter) {
                if (std::get<0>(*iter) == DeviceId) {
                    return std::get<1>(*iter)->c_str();
                }
            }

            return nullptr;
        }

    private:
        void remove_known_device(uint32_t DeviceId) {
            for (device_info_inventory_type::const_iterator iter =
                     this->device_info_inventory.cbegin();
                 iter != this->device_info_inventory.cend(); ++iter) {
                if (std::get<0>(*iter) == DeviceId) {
                    if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                        LOG(LOG_INFO,
                            "FileSystemVirtualChannel::DeviceRedirectionManager::remove_known_device: "
                                "Remove \"%s\"(DeviceId=%u) from known dervice list.",
                            std::get<1>(*iter)->c_str(),
                            DeviceId);
                    }

                    break;
                }
            }
        }

        void announce_device() {
            if (!this->waiting_for_server_device_announce_response &&
                this->device_announces.size()) {
                REDASSERT(this->to_server_sender);

                const uint32_t total_length =
                    std::get<0>(this->device_announces.front());

                uint32_t remaining_data_length = total_length;

                uint8_t const * chunk_data =
                    std::get<1>(this->device_announces.front()).get();

                {
                    InStream chunk(chunk_data, total_length);

                    rdpdr::SharedHeader client_message_header;

                    client_message_header.receive(chunk);

                    REDASSERT(client_message_header.component ==
                        rdpdr::Component::RDPDR_CTYP_CORE);
                    REDASSERT(client_message_header.packet_id ==
                        rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE);

                    REDASSERT(chunk.in_remain() >=
                              4 // DeviceCount(4)
                             );
                    const uint32_t DeviceCount = chunk.in_uint32_le();
                    (void)DeviceCount;
                    REDASSERT(DeviceCount == 1);

                    rdpdr::DeviceAnnounceHeader device_announce_header;

                    device_announce_header.receive(chunk);

                    if (device_announce_header.DeviceType() ==
                        rdpdr::RDPDR_DTYP_FILESYSTEM) {
                        this->add_known_device(
                            device_announce_header.DeviceId(),
                            device_announce_header.PreferredDosName());
                        this->device_info_inventory.push_back(
                            std::make_tuple(device_announce_header.DeviceId(),
                                std::make_unique<std::string>(
                                    device_announce_header.PreferredDosName()))
                            );
                        if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                            LOG(LOG_INFO,
                                "FileSystemVirtualChannel::DeviceRedirectionManager::announce_device: "
                                    "Add \"%s\"(DeviceId=%u) to known dervice list.",
                                device_announce_header.PreferredDosName(),
                                device_announce_header.DeviceId());
                        }
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

                    if (this->verbose & MODRDP_LOGLEVEL_RDPDR_DUMP) {
                        const bool send              = true;
                        const bool from_or_to_client = false;
                        ::msgdump_c(send, from_or_to_client,
                            total_length, flags, chunk_data,
                            chunk_data_length);
                    }

                    (*this->to_server_sender)(
                        total_length,
                        flags,
                        chunk_data,
                        chunk_data_length);

                    chunk_data            += chunk_data_length;
                    remaining_data_length -= chunk_data_length;

                    flags &= ~CHANNELS::CHANNEL_FLAG_FIRST;
                }
                while (remaining_data_length);

                this->device_announces.pop_front();

                this->waiting_for_server_device_announce_response = true;
            }
        }

    public:
        void process_client_device_list_announce_request(
            uint32_t total_length, uint32_t flags, InStream& chunk)
        {
            if (flags & CHANNELS::CHANNEL_FLAG_FIRST)
            {
                REDASSERT(
                    !this->length_of_remaining_device_data_to_be_processed);
                REDASSERT(
                    !this->length_of_remaining_device_data_to_be_skipped);

                {
                    const unsigned int expected = 4;    // DeviceCount(4)
                    if (!chunk.in_check_rem(expected)) {
                        LOG(LOG_ERR,
                            "FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request: "
                                "Truncated DR_DEVICELIST_ANNOUNCE, "
                                "need=%u remains=%u",
                            expected, chunk.in_remain());
                        throw Error(ERR_RDP_DATA_TRUNCATED);
                    }
                }

                uint32_t DeviceCount = chunk.in_uint32_le();

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request: "
                            "DeviceCount=%u",
                        DeviceCount);
                }

                this->remaining_device_announce_request_header_stream.rewind();
            }

            while (chunk.in_remain())
            {
                if (!this->length_of_remaining_device_data_to_be_processed &&
                    !this->length_of_remaining_device_data_to_be_skipped) {
                    // There is no device in process.

                    if (chunk.in_remain() <
                            20  // DeviceType(4) + DeviceId(4) +
                                //     PreferredDosName(8) +
                                //     DeviceDataLength(4)
                       ) {
                        REDASSERT(
                            !this->remaining_device_announce_request_header_stream.get_offset());

                        this->remaining_device_announce_request_header_stream.out_copy_bytes(
                            chunk.get_current(), chunk.in_remain());

                        if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                            LOG(LOG_INFO,
                                "FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request: "
                                    "%u byte(s) of request header are saved.",
                                uint32_t(
                                    this->remaining_device_announce_request_header_stream.get_offset()));
                        }

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
                            this->remaining_device_announce_request_header_stream.get_data(),
                            this->remaining_device_announce_request_header_stream.get_offset());
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

                    if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                        LOG(LOG_INFO,
                            "FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request: "
                                "DeviceType=%s(%u) DeviceId=%u "
                                "PreferredDosName=\"%s\" DeviceDataLength=%u",
                            rdpdr::DeviceAnnounceHeader::get_DeviceType_name(
                                DeviceType),
                            DeviceType, DeviceId, PreferredDosName,
                            DeviceDataLength);
                    }

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
                        REDASSERT(!(bool)this->current_device_announce_data);

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
                                this->current_device_announce_data.get(),
                                current_device_announce_data_length);
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

                        uint8_t out_data[512];
                        OutStream out_stream(out_data);

                        rdpdr::SharedHeader server_message_header(
                            rdpdr::Component::RDPDR_CTYP_CORE,
                            rdpdr::PacketId::PAKID_CORE_DEVICE_REPLY);

                        server_message_header.emit(out_stream);

                        rdpdr::ServerDeviceAnnounceResponse
                            server_device_announce_response(
                                    DeviceId,
                                    0xC0000001  // STATUS_UNSUCCESSFUL
                                );

                        server_device_announce_response.emit(out_stream);

                        if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                            LOG(LOG_INFO,
                                "FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request: "
                                    "Server Device Announce Response");
                            server_device_announce_response.log(LOG_INFO);
                        }

                        REDASSERT(this->to_client_sender);

                        const uint32_t total_length_      = out_stream.get_offset();
                        const uint32_t flags_             =
                            CHANNELS::CHANNEL_FLAG_FIRST |
                            CHANNELS::CHANNEL_FLAG_LAST;
                        const uint8_t* chunk_data_        = out_data;
                        const uint32_t chunk_data_length_ = total_length_;

                        if (this->verbose & MODRDP_LOGLEVEL_RDPDR_DUMP) {
                            const bool send              = true;
                            const bool from_or_to_client = true;
                            ::msgdump_c(send,
                                from_or_to_client, total_length_, flags_,
                                chunk_data_, chunk_data_length_);
                        }

                        (*this->to_client_sender)(
                            total_length_,
                            flags_,
                            chunk_data_,
                            chunk_data_length_);
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

                        this->device_announces.push_back(
                            std::make_tuple(
                                data_length,
                                std::move(this->current_device_announce_data)));
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

            this->announce_device();
        }   // process_client_device_list_announce_request()

        void process_client_drive_device_list_remove(uint32_t total_length,
                uint32_t flags, InStream& chunk) {
            {
                const unsigned int expected = 4;    // DeviceCount(4)
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "FileSystemVirtualChannel::DeviceRedirectionManager::process_client_drive_device_list_remove: "
                            "Truncated DR_DEVICELIST_REMOVE (1), "
                            "need=%u remains=%u",
                        expected, chunk.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }
            }

            const uint32_t DeviceCount = chunk.in_uint32_le();

            {
                const unsigned int expected = DeviceCount *
                                              4 // DeviceId(4)
                                            ;
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "FileSystemVirtualChannel::DeviceRedirectionManager::process_client_drive_device_list_remove: "
                            "Truncated DR_DEVICELIST_REMOVE (2), "
                            "need=%u remains=%u",
                        expected, chunk.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }
            }

            auto remove_device =
                    [](device_announce_collection_type& device_announces,
                        const uint32_t DeviceId) -> bool {
                bool device_removed = false;

                for (device_announce_collection_type::const_iterator iter =
                         device_announces.cbegin();
                     iter != device_announces.cend(); ++iter) {
                    uint8_t const * data =
                        std::get<1>(*iter).get();

                    InStream header_stream(data,
                            rdpdr::SharedHeader::size() +
                            12  // DeviceCount(4) + DeviceType(4) + DeviceId(4)
                        );

                    header_stream.in_skip_bytes(
                            rdpdr::SharedHeader::size() +
                            8  // DeviceCount(4) + DeviceType(4)
                        );

                    const uint32_t current_device_id =
                        header_stream.in_uint32_le();

                    if (DeviceId == current_device_id) {
                        device_announces.erase(iter);
                        device_removed = true;
                        break;
                    }
                }

                return device_removed;
            };

            const uint32_t max_number_of_removable_device = 128;

            uint8_t client_drive_device_list_remove_data[
                      64    // > rdpdr::SharedHeader::size()
                    + 4     // DeviceCount(4)
                    + 4     // DeviceId(4)
                    * max_number_of_removable_device
                ];
            OutStream client_drive_device_list_remove_stream(
                client_drive_device_list_remove_data);

            uint32_t number_of_removable_device = 0;

            rdpdr::SharedHeader client_message_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_DEVICELIST_REMOVE);

            client_message_header.emit(client_drive_device_list_remove_stream);

            const auto device_count_offset =
                client_drive_device_list_remove_stream.get_offset();

            client_drive_device_list_remove_stream.out_clear_bytes(
                    4   // DeviceCount(4)
                );

            for (uint32_t device_index = 0; device_index < DeviceCount;
                 device_index++) {
                const uint32_t DeviceId = chunk.in_uint32_le();

                if (!remove_device(this->device_announces, DeviceId) &&
                    (number_of_removable_device < max_number_of_removable_device)) {
                    client_drive_device_list_remove_stream.out_uint32_le(
                        DeviceId);
                    number_of_removable_device++;

                    this->remove_known_device(DeviceId);
                }
            }

            if (number_of_removable_device) {
                client_drive_device_list_remove_stream.set_out_uint32_le(
                    number_of_removable_device, device_count_offset);

                REDASSERT(this->to_server_sender);

                const uint32_t total_length_      =
                    client_drive_device_list_remove_stream.get_offset();
                const uint32_t flags_             =
                    CHANNELS::CHANNEL_FLAG_FIRST |
                    CHANNELS::CHANNEL_FLAG_LAST;
                const uint8_t* chunk_data_        =
                    client_drive_device_list_remove_data;
                const uint32_t chunk_data_length_ = total_length_;

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR_DUMP) {
                    const bool send              = true;
                    const bool from_or_to_client = false;
                    ::msgdump_c(send, from_or_to_client,
                        total_length_, flags_, chunk_data_,
                        chunk_data_length_);
                }

                (*this->to_server_sender)(
                    total_length_,
                    flags_,
                    chunk_data_,
                    chunk_data_length_);
            }
        }

        void process_server_user_logged_on(uint32_t total_length,
                uint32_t flags, InStream& chunk) {
            this->announce_device();
        }

        void process_server_device_announce_response(uint32_t total_length,
                uint32_t flags, InStream& chunk) {
            this->waiting_for_server_device_announce_response = false;

            this->announce_device();

            rdpdr::ServerDeviceAnnounceResponse
                server_device_announce_response;

            server_device_announce_response.receive(chunk);
            if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                server_device_announce_response.log(LOG_INFO);
            }

            if (server_device_announce_response.ResultCode() !=
                0x00000000  // STATUS_SUCCESS
               ) {
                this->remove_known_device(
                    server_device_announce_response.DeviceId());
            }
        }
    } device_redirection_manager;

    FrontAPI& front;

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

        bool dont_log_data_into_syslog;
        bool dont_log_data_into_wrm;

        auth_api* acl;
    };

    FileSystemVirtualChannel(
        VirtualChannelDataSender* to_client_sender_,
        VirtualChannelDataSender* to_server_sender_,
        FileSystemDriveManager& file_system_drive_manager,
        FrontAPI& front,
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
    , param_dont_log_data_into_syslog(params.dont_log_data_into_syslog)
    , param_dont_log_data_into_wrm(params.dont_log_data_into_wrm)
    , param_acl(params.acl)
    , device_redirection_manager(
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
    , front(front) {}

    ~FileSystemVirtualChannel() override
    {
#ifndef NDEBUG
        bool make_boom_in_debug_mode = false;
#endif  // #ifndef NDEBUG

        if (!this->device_io_request_info_inventory.empty())
        {
            for (device_io_request_info_inventory_type::iterator iter =
                     this->device_io_request_info_inventory.begin();
                 iter != this->device_io_request_info_inventory.end();
                 ++iter)
            {
                REDASSERT(std::get<3>(*iter) !=
                    rdpdr::IRP_MJ_DIRECTORY_CONTROL);

                LOG(LOG_WARNING,
                    "FileSystemVirtualChannel::~FileSystemVirtualChannel: "
                        "There is Device I/O request information "
                        "remaining in inventory. "
                        "DeviceId=%u FileId=%u CompletionId=%u "
                        "MajorFunction=%u extra_data=%u file_path=\"%s\"",
                    std::get<0>(*iter), std::get<1>(*iter),
                    std::get<2>(*iter), std::get<3>(*iter),
                    std::get<4>(*iter),
                    (std::get<5>(*iter) ?
                     std::get<5>(*iter)->c_str() :
                     ""));

#ifndef NDEBUG
                make_boom_in_debug_mode = true;
#endif  // #ifndef NDEBUG
            }
        }

        if (!this->device_io_target_info_inventory.empty())
        {
            for (device_io_target_info_inventory_type::iterator iter =
                     this->device_io_target_info_inventory.begin();
                 iter != this->device_io_target_info_inventory.end();
                 ++iter)
            {
                LOG(LOG_WARNING,
                    "FileSystemVirtualChannel::~FileSystemVirtualChannel: "
                        "There is Device I/O target information "
                        "remaining in inventory. "
                        "DeviceId=%u FileId=%u file_path=\"%s\" for_reading=%s "
                        "for_writing=%s",
                    std::get<0>(*iter), std::get<1>(*iter),
                    (std::get<2>(*iter) ? std::get<2>(*iter)->c_str() : ""),
                    (std::get<3>(*iter) ? "yes" : "no"),
                    (std::get<4>(*iter) ? "yes" : "no"));

#ifndef NDEBUG
                make_boom_in_debug_mode = true;
#endif  // #ifndef NDEBUG
            }
        }

        REDASSERT(!make_boom_in_debug_mode);
    }

    const char* get_reporting_reason_exchanged_data_limit_reached() const override
    {
        return "RDPDR_LIMIT";
    }

    void process_client_general_capability_set(uint32_t total_length,
            uint32_t flags, InStream& chunk, uint32_t Version) {
        rdpdr::GeneralCapabilitySet general_capability_set;

        InStream tmp_chunk = chunk.clone();
        general_capability_set.receive(tmp_chunk, Version);

        const bool need_enable_user_loggedon_pdu =
            (!(general_capability_set.extendedPDU() &
               rdpdr::RDPDR_USER_LOGGEDON_PDU));

        const bool need_deny_asyncio =
            (general_capability_set.extraFlags1() & rdpdr::ENABLE_ASYNCIO);

        if ((this->verbose & MODRDP_LOGLEVEL_RDPDR) &&
            (need_enable_user_loggedon_pdu || need_deny_asyncio)) {
            LOG(LOG_INFO,
                "FileSystemVirtualChannel::process_client_general_capability_set:");
            general_capability_set.log(LOG_INFO);
        }

        if (need_enable_user_loggedon_pdu || need_deny_asyncio) {
            OutStream out_stream(
                const_cast<uint8_t*>(chunk.get_current()),
                rdpdr::GeneralCapabilitySet::size(Version));

            if ((this->verbose & MODRDP_LOGLEVEL_RDPDR) &&
                need_enable_user_loggedon_pdu) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_client_general_capability_set:"
                        "Allow the server to send a "
                        "Server User Logged On packet.");

                general_capability_set.set_extendedPDU(
                    general_capability_set.extendedPDU() |
                    rdpdr::RDPDR_USER_LOGGEDON_PDU);
            }

            if ((this->verbose & MODRDP_LOGLEVEL_RDPDR) &&
                need_deny_asyncio) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_client_general_capability_set:"
                        "Deny user to send multiple simultaneous "
                        "read or write requests on the same file from "
                        "a redirected file system.");

                general_capability_set.set_extraFlags1(
                    general_capability_set.extraFlags1() &
                    ~rdpdr::ENABLE_ASYNCIO);
            }

            general_capability_set.emit(out_stream, Version);

            general_capability_set.receive(chunk, Version);

            if ((this->verbose & MODRDP_LOGLEVEL_RDPDR) &&
                (need_enable_user_loggedon_pdu || need_deny_asyncio)) {
                general_capability_set.log(LOG_INFO);
            }
        }
    }   // process_client_general_capability_set

    bool process_client_core_capability_response(
        uint32_t total_length, uint32_t flags, InStream& chunk)
    {
        REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
            (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

        {
            const unsigned int expected = 4;    // numCapabilities(2) +
                                                //     Padding(2)
            if (!chunk.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "FileSystemVirtualChannel::process_client_core_capability_response: "
                        "Truncated DR_CORE_CAPABILITY_RSP (1), "
                        "need=%u remains=%u",
                    expected, chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }
        }

        const uint16_t numCapabilities = chunk.in_uint16_le();
        if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
            LOG(LOG_INFO,
                "FileSystemVirtualChannel::process_client_core_capability_response: "
                    "numCapabilities=%u", numCapabilities);
        }

        chunk.in_skip_bytes(2); // Padding(2)

        for (uint16_t idx_capabilities = 0; idx_capabilities < numCapabilities;
             ++idx_capabilities) {
            {
                const unsigned int expected = 8;    // CapabilityType(2) +
                                                    //     CapabilityLength(2) +
                                                    //     Version(4)
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "FileSystemVirtualChannel::process_client_core_capability_response: "
                            "Truncated DR_CORE_CAPABILITY_RSP (2), "
                            "need=%u remains=%u",
                        expected, chunk.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }
            }

            const uint16_t CapabilityType   = chunk.in_uint16_le();
            const uint16_t CapabilityLength = chunk.in_uint16_le();
            const uint32_t Version          = chunk.in_uint32_le();

            if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
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
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_core_capability_response: "
                            "Client supports DRIVE_CAPABILITY_VERSION_02.");
                }
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

    bool process_client_drive_query_information_response(
        uint32_t total_length, uint32_t flags, InStream& chunk,
        uint32_t FsInformationClass)
    {
        switch (FsInformationClass) {
            case rdpdr::FileBasicInformation:
            {
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    {
                        const unsigned int expected = 4;    // Length(4)
                        if (!chunk.in_check_rem(expected)) {
                            LOG(LOG_ERR,
                                "FileSystemVirtualChannel::process_client_drive_query_information_response: "
                                    "Truncated DR_DRIVE_QUERY_INFORMATION_RSP - "
                                    "FileBasicInformation, "
                                    "need=%u remains=%u",
                                expected, chunk.in_remain());
                            throw Error(ERR_RDP_DATA_TRUNCATED);
                        }
                    }

                    if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                        chunk.in_skip_bytes(4); // Length(4)

                        fscc::FileBasicInformation file_basic_information;

                        file_basic_information.receive(chunk);
                        file_basic_information.log(LOG_INFO);
                    }
                }
            }
            break;

            case rdpdr::FileStandardInformation:
            {
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    {
                        const unsigned int expected = 4;    // Length(4)
                        if (!chunk.in_check_rem(expected)) {
                            LOG(LOG_ERR,
                                "FileSystemVirtualChannel::process_client_drive_query_information_response: "
                                    "Truncated DR_DRIVE_QUERY_INFORMATION_RSP - "
                                    "FileStandardInformation, "
                                    "need=%u remains=%u",
                                expected, chunk.in_remain());
                            throw Error(ERR_RDP_DATA_TRUNCATED);
                        }
                    }

                    if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                        chunk.in_skip_bytes(4); // Length(4)

                        fscc::FileStandardInformation file_standard_information;

                        file_standard_information.receive(chunk);
                        file_standard_information.log(LOG_INFO);
                    }
                }
            }
            break;

            default:
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_WARNING,
                        "FileSystemVirtualChannel::process_client_drive_query_information_response: "
                            "Undecoded FsInformationClass - %s(0x%X)",
                        rdpdr::ServerDriveQueryInformationRequest::get_FsInformationClass_name(
                            FsInformationClass),
                        FsInformationClass);
                }
            break;
        }

        return true;
    }

    bool process_client_drive_query_volume_information_response(
        uint32_t total_length, uint32_t flags, InStream& chunk,
        uint32_t FsInformationClass)
    {
        switch (FsInformationClass) {
            case rdpdr::FileFsAttributeInformation:
            {
                {
                    const unsigned int expected = 4;    // Length(4)
                    if (!chunk.in_check_rem(expected)) {
                        LOG(LOG_ERR,
                            "FileSystemVirtualChannel::process_client_drive_query_volume_information_response: "
                                "Truncated DR_DRIVE_QUERY_VOLUME_INFORMATION_RSP - "
                                "FileFsAttributeInformation, "
                                "need=%u remains=%u",
                            expected, chunk.in_remain());
                        throw Error(ERR_RDP_DATA_TRUNCATED);
                    }
                }

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    chunk.in_skip_bytes(4); // Length(4)

                    fscc::FileFsAttributeInformation
                        file_fs_Attribute_information;

                    file_fs_Attribute_information.receive(chunk);
                    file_fs_Attribute_information.log(LOG_INFO);
                }
            }
            break;

            default:
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_WARNING,
                        "FileSystemVirtualChannel::process_client_drive_query_volume_information_response: "
                            "Undecoded FsInformationClass - %s(0x%X)",
                        rdpdr::ServerDriveQueryVolumeInformationRequest::get_FsInformationClass_name(
                            FsInformationClass),
                        FsInformationClass);
                }
            break;
        }

        return true;
    }

    bool process_client_drive_io_response(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        REDASSERT(flags & CHANNELS::CHANNEL_FLAG_FIRST);

        rdpdr::DeviceIOResponse device_io_response;

        device_io_response.receive(chunk);
        if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
            device_io_response.log(LOG_INFO);
        }

        device_io_request_info_inventory_type::iterator iter;
        for (iter = this->device_io_request_info_inventory.begin();
             (iter != this->device_io_request_info_inventory.end()) &&
             ((std::get<0>(*iter) != device_io_response.DeviceId()) ||
              (std::get<2>(*iter) != device_io_response.CompletionId()));
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

        const uint32_t FileId        = std::get<1>(*iter);
        const uint32_t MajorFunction = std::get<3>(*iter);
        const uint32_t extra_data    = std::get<4>(*iter);
        const char*    file_path     = (std::get<5>(*iter) ?
                                        std::get<5>(*iter)->c_str() :
                                        "");

        if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
            LOG(LOG_INFO,
                "FileSystemVirtualChannel::process_client_drive_io_response: "
                    "FileId=%u MajorFunction=%s(0x%08X) extra_data=0x%X "
                    "file_path=\"%s\"",
                FileId,
                rdpdr::DeviceIORequest::get_MajorFunction_name(MajorFunction),
                MajorFunction, extra_data, file_path);
        }

        switch (MajorFunction)
        {
            case rdpdr::IRP_MJ_CREATE:
            {
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_drive_io_response: "
                            "Create request.");
                }

                rdpdr::DeviceCreateResponse device_create_response;

                device_create_response.receive(chunk);
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    device_create_response.log(LOG_INFO);
                }

                if (device_io_response.IoStatus() ==
                    0x00000000  // STATUS_SUCCESS
                   ) {
                    const char* device_name =
                        this->device_redirection_manager.get_device_name(
                            device_io_response.DeviceId());

                    if (device_name) {
                        std::unique_ptr<std::string> target_file_name =
                            std::make_unique<std::string>(device_name);

                        *target_file_name += file_path;

                        if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                            LOG(LOG_INFO,
                                "FileSystemVirtualChannel::process_client_drive_io_response: "
                                    "Add \"%s\" to known file list. "
                                    "DeviceId=%u FileId=%u",
                                target_file_name->c_str(),
                                device_io_response.DeviceId(),
                                device_create_response.FileId());
                        }

                        this->device_io_target_info_inventory.push_back(
                            std::make_tuple(
                                device_io_response.DeviceId(),
                                device_create_response.FileId(),
                                std::move(target_file_name),    // file_path
                                false,                          // for_reading
                                false                           // for_writing
                            ));
                    }
                    else {
                        LOG(LOG_WARNING,
                            "FileSystemVirtualChannel::process_client_drive_io_response: "
                                "Device not found. DeviceId=%u",
                            device_io_response.DeviceId());

                        REDASSERT(false);
                    }
                }
            }
            break;

            case rdpdr::IRP_MJ_CLOSE:
            {
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_drive_io_response: "
                            "Close request.");
                }

                for (device_io_target_info_inventory_type::const_iterator
                         target_iter =
                             this->device_io_target_info_inventory.cbegin();
                     target_iter !=
                         this->device_io_target_info_inventory.cend();
                     ++target_iter) {
                    if ((std::get<0>(*target_iter) ==
                         device_io_response.DeviceId()) &&
                        (std::get<1>(*target_iter) == FileId)) {

                        if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                            LOG(LOG_INFO,
                                "FileSystemVirtualChannel::process_client_drive_io_response: "
                                    "Remove \"%s\" from known file list. "
                                    "DeviceId=%u FileId=%u",
                                std::get<2>(*target_iter)->c_str(),
                                device_io_response.DeviceId(),
                                FileId);
                        }

                        this->device_io_target_info_inventory.erase(
                            target_iter);

                        break;
                    }
                }
            }
            break;

            case rdpdr::IRP_MJ_READ:
            {
                {
                    const unsigned int expected = 4;    // Length(4)
                    if (!chunk.in_check_rem(expected)) {
                        LOG(LOG_ERR,
                            "FileSystemVirtualChannel::process_client_drive_io_response: "
                                "Truncated IRP_MJ_READ, need=%u remains=%u",
                            expected, chunk.in_remain());
                        throw Error(ERR_RDP_DATA_TRUNCATED);
                    }
                }

                const uint32_t Length = chunk.in_uint32_le();

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_drive_io_response: "
                            "Read request. Length=%u",
                        Length);
                }

                if (device_io_response.IoStatus() ==
                    0x00000000  // STATUS_SUCCESS
                   ) {
                    this->update_exchanged_data(Length);

                    for (device_io_request_info_inventory_type::const_iterator
                             request_iter =
                                 this->device_io_request_info_inventory.cbegin();
                         request_iter !=
                             this->device_io_request_info_inventory.cend();
                         ++request_iter) {
                        //LOG(LOG_INFO, "DeviceId=%u:%u CompletionId=%u:%u",
                        //    device_io_response.DeviceId(), std::get<0>(*request_iter),
                        //    device_io_response.CompletionId(), std::get<2>(*request_iter)
                        //    );
                        if ((std::get<0>(*request_iter) ==
                             device_io_response.DeviceId()) &&
                            (std::get<2>(*request_iter) ==
                             device_io_response.CompletionId())) {
                            const uint32_t FileId = std::get<1>(*request_iter);

                            for (device_io_target_info_inventory_type::iterator
                                     target_iter =
                                         this->device_io_target_info_inventory.begin();
                                 target_iter != this->device_io_target_info_inventory.end();
                                 ++target_iter) {
                                //LOG(LOG_INFO, "--> DeviceId=%u:%u FileId=%u:%u",
                                //    device_io_response.DeviceId(), std::get<0>(*target_iter),
                                //    FileId, std::get<1>(*target_iter)
                                //    );

                                if ((device_io_response.DeviceId() == std::get<0>(*target_iter)) &&
                                    (FileId == std::get<1>(*target_iter))) {
                                    if (!std::get<3>(*target_iter)) {
                                        if (!this->param_dont_log_data_into_syslog &&
                                            this->param_acl) {
                                            this->param_acl->log2("CNT event",
                                                "DR_READ",
                                                std::get<2>(*target_iter)->c_str());
                                        }

                                        if (!this->param_dont_log_data_into_wrm) {
                                            std::string message("ReadRedirectedFileSystem=");
                                            message += std::get<2>(*target_iter)->c_str();

                                            bool contian_window_title = false;
                                            this->front.session_update(message.c_str(),
                                                contian_window_title);
                                        }

                                        std::get<3>(*target_iter) = true;

                                        if (std::get<3>(*target_iter) ==
                                            std::get<4>(*target_iter)) {
                                            this->device_io_target_info_inventory.erase(
                                                target_iter);
                                        }
                                    }

                                    break;
                                }
                            }

                            break;
                        }
                    }
                }
            }
            break;

            case rdpdr::IRP_MJ_WRITE:
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_drive_io_response: "
                            "Write request.");
                }

                if (device_io_response.IoStatus() ==
                    0x00000000  // STATUS_SUCCESS
                   ) {
                    for (device_io_request_info_inventory_type::const_iterator
                             request_iter =
                                 this->device_io_request_info_inventory.cbegin();
                         request_iter !=
                             this->device_io_request_info_inventory.cend();
                         ++request_iter) {
                        if ((std::get<0>(*request_iter) ==
                             device_io_response.DeviceId()) &&
                            (std::get<2>(*request_iter) ==
                             device_io_response.CompletionId())) {
                            const uint32_t FileId = std::get<1>(*request_iter);

                            for (device_io_target_info_inventory_type::iterator
                                     target_iter =
                                         this->device_io_target_info_inventory.begin();
                                 target_iter != this->device_io_target_info_inventory.end();
                                 ++target_iter) {
                                if ((device_io_response.DeviceId() == std::get<0>(*target_iter)) &&
                                    (FileId == std::get<1>(*target_iter))) {
                                    if (!std::get<4>(*target_iter)) {
                                        if (!this->param_dont_log_data_into_syslog &&
                                            this->param_acl) {
                                            this->param_acl->log2("CNT event",
                                                "DR_WRITE",
                                                std::get<2>(*target_iter)->c_str());
                                        }

                                        if (!this->param_dont_log_data_into_wrm) {
                                            std::string message("WriteRedirectedFileSystem=");
                                            message += std::get<2>(*target_iter)->c_str();

                                            bool contian_window_title = false;
                                            this->front.session_update(message.c_str(),
                                                contian_window_title);
                                        }

                                        std::get<4>(*target_iter) = true;

                                        if (std::get<3>(*target_iter) ==
                                            std::get<4>(*target_iter)) {
                                            this->device_io_target_info_inventory.erase(
                                                target_iter);
                                        }
                                    }

                                    break;
                                }
                            }

                            break;
                        }
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
                    /* FsInformationClass = */extra_data);
            break;

            default:
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_WARNING,
                        "FileSystemVirtualChannel::process_client_drive_io_response: "
                            "Undecoded Drive I/O Response - "
                            "MajorFunction=%s(0x%08X)",
                        rdpdr::DeviceIORequest::get_MajorFunction_name(
                            this->server_device_io_request.MajorFunction()),
                        this->server_device_io_request.MajorFunction());
                }
            break;
        }   // switch (MajorFunction)

        this->device_io_request_info_inventory.erase(iter);

        return true;
    }   // process_client_drive_io_response

    void process_client_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data, uint32_t chunk_data_length)
            override
    {
        if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
            LOG(LOG_INFO,
                "FileSystemVirtualChannel::process_client_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (this->verbose & MODRDP_LOGLEVEL_RDPDR_DUMP) {
            const bool send              = false;
            const bool from_or_to_client = true;
            ::msgdump_c(send, from_or_to_client, total_length, flags,
                chunk_data, chunk_data_length);
        }

        InStream chunk(chunk_data, chunk_data_length);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->client_message_header.receive(chunk);
        }

        bool send_message_to_server = true;

        switch (this->client_message_header.packet_id)
        {
            case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
                REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
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

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_message: "
                            "Client Name Request");

                    rdpdr::ClientNameRequest client_name_request;

                    client_name_request.receive(chunk);
                    client_name_request.log(LOG_INFO);
                }
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE:
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_message: "
                            "Client Device List Announce Request");
                }

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
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_message: "
                            "Client Drive I/O Response");
                }

                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                    send_message_to_server =
                        this->process_client_drive_io_response(
                            total_length, flags, chunk);
                }
            break;

            case rdpdr::PacketId::PAKID_CORE_CLIENT_CAPABILITY:
                REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
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

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_client_message: "
                            "Client Drive Device List Remove");
                }

                this->device_redirection_manager.process_client_drive_device_list_remove(
                    total_length, flags, chunk);

                send_message_to_server = false;
            break;

            default:
                if ((this->verbose & MODRDP_LOGLEVEL_RDPDR) &&
                    (flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
                    LOG(LOG_WARNING,
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
        uint32_t flags, InStream& chunk)
    {
        rdpdr::ServerAnnounceRequest server_announce_request;

        server_announce_request.receive(chunk);

        if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
            server_announce_request.log(LOG_INFO);
        }

        // Virtual channel is opened at client side and is authorized.
        if (this->has_valid_to_client_sender()) {
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
            if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_server_announce_request:");
                client_announce_reply.log(LOG_INFO);
            }
            client_announce_reply.emit(out_stream);

            this->send_message_to_server(
                out_stream.get_offset(),
                  CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_data(),
                out_stream.get_offset());
        }

        {
            OutStream out_stream(message_buffer);

            rdpdr::SharedHeader clent_message_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_CLIENT_NAME);
            clent_message_header.emit(out_stream);

            rdpdr::ClientNameRequest client_name_request(
                this->param_client_name);
            if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_server_announce_request:");
                client_name_request.log(LOG_INFO);
            }
            client_name_request.emit(out_stream);

            this->send_message_to_server(
                out_stream.get_offset(),
                  CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_data(),
                out_stream.get_offset());
        }

        return false;
    }   // process_server_announce_request

    bool process_server_client_id_confirm(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        // Virtual channel is opened at client side and is authorized.
        if (this->has_valid_to_client_sender())
            return true;

        uint8_t message_buffer[1024];

        {
            OutStream out_stream(message_buffer);

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
            if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                LOG(LOG_INFO,
                    "FileSystemVirtualChannel::process_server_client_id_confirm:");
                general_capability_set.log(LOG_INFO);
            }
            general_capability_set.emit(out_stream, general_capability_version);

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
                out_stream.get_data(),
                out_stream.get_offset());
        }

        return false;
    }   // process_server_client_id_confirm

    bool process_server_create_drive_request(uint32_t total_length,
        uint32_t flags, InStream& chunk,
        std::unique_ptr<std::string>& file_path)
    {
        rdpdr::DeviceCreateRequest device_create_request;

        device_create_request.receive(chunk);

        if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
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

            OutStream out_stream(message_buffer);

            const rdpdr::SharedHeader clent_message_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);

            clent_message_header.emit(out_stream);

            const rdpdr::DeviceIOResponse device_io_response(
                    this->server_device_io_request.DeviceId(),
                    this->server_device_io_request.CompletionId(),
                    0xC0000022  // STATUS_ACCESS_DENIED
                );

            if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                device_io_response.log(LOG_INFO);
            }
            device_io_response.emit(out_stream);

            const rdpdr::DeviceCreateResponse device_create_response(
                static_cast<uint32_t>(-1), 0);
            if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                device_create_response.log(LOG_INFO);
            }
            device_create_response.emit(out_stream);

            this->send_message_to_server(
                out_stream.get_offset(),
                  CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_data(),
                out_stream.get_offset());

            return false;
        }

        file_path = std::make_unique<std::string>(
            device_create_request.Path());

        return true;
    }   // process_server_create_drive_request

    bool process_server_drive_io_request(uint32_t total_length,
        uint32_t flags, InStream& chunk,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task)
    {
        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->server_device_io_request.receive(chunk);
            if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
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
                this->verbose);

            return false;
        }

        if (!(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            return true;
        }

        uint32_t extra_data = 0;

        bool send_message_to_client  = true;
        bool do_not_add_to_inventory = false;

        std::unique_ptr<std::string> file_path;

        switch (this->server_device_io_request.MajorFunction())
        {
            case rdpdr::IRP_MJ_CREATE:
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Device Create Request");
                }

                send_message_to_client =
                    this->process_server_create_drive_request(
                        total_length, flags, chunk, file_path);
            break;

            case rdpdr::IRP_MJ_CLOSE:
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Device Close Request");
                }
            break;

            case rdpdr::IRP_MJ_WRITE:
            {
                const uint32_t Length = chunk.in_uint32_le();

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    const uint64_t Offset = chunk.in_uint64_le();

                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Write Request. Length=%u Offset=%" PRIu64,
                        Length, Offset);
                }

                this->update_exchanged_data(Length);
            }
            break;

            case rdpdr::IRP_MJ_DEVICE_CONTROL:
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
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
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Query volume information request");
                }

                rdpdr::ServerDriveQueryVolumeInformationRequest
                    server_drive_query_volume_information_request;

                server_drive_query_volume_information_request.receive(
                    chunk);
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    server_drive_query_volume_information_request.log(
                        LOG_INFO);
                }

                extra_data =
                    server_drive_query_volume_information_request.FsInformationClass();
            }
            break;

            case rdpdr::IRP_MJ_QUERY_INFORMATION:
            {
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_drive_io_request: "
                            "Server Drive Query Information Request");
                }

                rdpdr::ServerDriveQueryInformationRequest
                    server_drive_query_information_request;

                server_drive_query_information_request.receive(chunk);
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    server_drive_query_information_request.log(LOG_INFO);
                }

                extra_data =
                    server_drive_query_information_request.FsInformationClass();
            }
            break;

            case rdpdr::IRP_MJ_DIRECTORY_CONTROL:
                if (this->server_device_io_request.MinorFunction() ==
                    rdpdr::IRP_MN_QUERY_DIRECTORY) {
                    if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                        LOG(LOG_INFO,
                            "FileSystemVirtualChannel::process_server_drive_io_request: "
                                "Server Drive Query Directory Request");
                    }

                    rdpdr::ServerDriveQueryDirectoryRequest
                        server_drive_query_directory_request;

                    server_drive_query_directory_request.receive(chunk);
                    if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                        server_drive_query_directory_request.log(LOG_INFO);
                    }

                    extra_data =
                        server_drive_query_directory_request.FsInformationClass();
                }
                else {
                    do_not_add_to_inventory = true;
                }
            break;

            default:
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_WARNING,
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

        if (send_message_to_client && !do_not_add_to_inventory) {
            this->device_io_request_info_inventory.push_back(
                std::make_tuple(
                    this->server_device_io_request.DeviceId(),
                    this->server_device_io_request.FileId(),
                    this->server_device_io_request.CompletionId(),
                    this->server_device_io_request.MajorFunction(),
                    extra_data,
                    std::move(file_path)));
        }

        return send_message_to_client;
    }   // process_server_drive_io_request

    void process_server_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data, uint32_t chunk_data_length,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task)
            override
    {
        if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
            LOG(LOG_INFO,
                "FileSystemVirtualChannel::process_server_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (this->verbose & MODRDP_LOGLEVEL_RDPDR_DUMP) {
            const bool send              = false;
            const bool from_or_to_client = false;
            ::msgdump_c(send, from_or_to_client, total_length, flags,
                chunk_data, chunk_data_length);
        }

        InStream chunk(chunk_data, chunk_data_length);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->server_message_header.receive(chunk);
        }

        bool send_message_to_client = this->has_valid_to_client_sender();

        switch (this->server_message_header.packet_id)
        {
            case rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE:
                if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) &&
                    (this->verbose & MODRDP_LOGLEVEL_RDPDR)) {
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

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
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

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_message: "
                            "Server Device Announce Response");
                }

                this->device_redirection_manager.process_server_device_announce_response(
                    total_length, flags, chunk);
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST:
                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
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

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_message: "
                            "Server Core Capability Request");
                }
            break;

            case rdpdr::PacketId::PAKID_CORE_USER_LOGGEDON:
                REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_message: "
                            "Server User Logged On");
                }

                if (!this->proxy_managed_drives_announced) {
                    this->file_system_drive_manager.AnnounceDrive(
                        this->device_capability_version_02_supported,
                        this->device_redirection_manager.to_device_announce_collection_sender,
                        this->verbose);

                    this->proxy_managed_drives_announced = true;

                    this->device_redirection_manager.process_server_user_logged_on(
                        total_length, flags, chunk);
                }
            break;

            case rdpdr::PacketId::PAKID_PRN_USING_XPS:
                REDASSERT((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) ==
                    (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST));

                if (this->verbose & MODRDP_LOGLEVEL_RDPDR) {
                    LOG(LOG_INFO,
                        "FileSystemVirtualChannel::process_server_message: "
                            "Server Printer Set XPS Mode");
                }
            break;

            default:
                if ((this->verbose & MODRDP_LOGLEVEL_RDPDR) &&
                    (flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
                    LOG(LOG_WARNING,
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
