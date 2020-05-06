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
   Author(s): Clément Moroldo

   Unit test to writing RDP orders to file and rereading them
*/


#pragma once

#include "utils/sugar/bytes_view.hpp"
#include "mod/rdp/rdp_verbose.hpp"

#include <unordered_map>
#include <memory>
#include <vector>
#include <string>


constexpr long long WINDOWS_TICK = 10000000;
constexpr long long SEC_TO_UNIX_EPOCH = 11644473600LL;

class ClientIODiskAPI
{
public:
    virtual ~ClientIODiskAPI() = default;

    struct FileStat
    {
        uint64_t LastAccessTime = 0;
        uint64_t LastWriteTime  = 0;
        uint64_t CreationTime   = 0;
        uint64_t ChangeTime     = 0;
        uint32_t FileAttributes = 0;

        int64_t  AllocationSize = 0;
        int64_t  EndOfFile      = 0;
        uint32_t NumberOfLinks  = 0;
        uint8_t  DeletePending  = 0;
        uint8_t  Directory      = 0;
    };

    struct FileStatvfs
    {
    private:
        static uint32_t _default_FileSystemAttributes() noexcept;

    public:
        uint64_t VolumeCreationTime             = 0;
        chars_view VolumeLabel       = ""_av;
        chars_view FileSystemName    = "ext4"_av;

        uint32_t FileSystemAttributes           = _default_FileSystemAttributes();
        uint32_t SectorsPerAllocationUnit       = 8;

        uint32_t BytesPerSector                 = 0;
        uint32_t MaximumComponentNameLength     = 0;
        uint64_t TotalAllocationUnits           = 0;
        uint64_t CallerAvailableAllocationUnits = 0;
        uint64_t AvailableAllocationUnits       = 0;
        uint64_t ActualAvailableAllocationUnits = 0;
    };

    static unsigned WindowsTickToUnixSeconds(long long windowsTicks) {
        return unsigned((windowsTicks / WINDOWS_TICK) - SEC_TO_UNIX_EPOCH);
    }

    static long long UnixSecondsToWindowsTick(unsigned unixSeconds) {
        return ((unixSeconds + SEC_TO_UNIX_EPOCH) * WINDOWS_TICK);
    }

    virtual bool ifile_good(const char * new_path) = 0;

    virtual bool ofile_good(const char * new_path) = 0;

    virtual int get_file_size(const char * path) = 0;

    virtual bool dir_good(const char * new_path) = 0;

    virtual void marke_dir(const char * new_path) = 0;

    virtual FileStat get_file_stat(const char * file_to_request) = 0;

    virtual FileStatvfs get_file_statvfs(const char * file_to_request) = 0;

    virtual void read_data(
        std::string const& file_to_tread, int offset, writable_bytes_view data) = 0;

    virtual bool set_elem_from_dir(std::vector<std::string> & elem_list, const std::string & str_dir_path) = 0;

    virtual int get_device(const char * file_path) = 0;

    virtual uint32_t get_volume_serial_number(int device) = 0;

    virtual bool write_file(const char * file_to_write, bytes_view data) = 0;

    virtual bool remove_file(const char * file_to_remove) = 0;

    virtual bool rename_file(const char * file_to_rename,  const char * new_name) = 0;
};


namespace rdpdr
{
    enum RDPDR_DTYP : uint32_t;
    class DeviceIOResponse;
}

class RDPDiskConfig;
class ClientChannelMod;
class InStream;
class OutStream;


class ClientRDPDRChannel
{
    RDPVerbose verbose;

    ClientIODiskAPI * impl_io_disk;

    ClientChannelMod * callback;

public:

    bool drives_created = false;
    uint16_t protocol_minor_version = 0;
    uint32_t next_file_id = 0;

    std::unique_ptr<uint8_t[]> ReadData;
    int last_read_data_portion_length = 0;

    uint32_t get_file_id() {
        this->next_file_id++;
        return this->next_file_id;
    }

    std::unordered_map<int, std::string> paths;

    std::string share_dir;


    struct DeviceData {
        char name[8] = {0};
        uint32_t ID;
        rdpdr::RDPDR_DTYP type;

        DeviceData(const char * name, uint32_t ID, rdpdr::RDPDR_DTYP type)
        : ID(ID)
        , type(type)
        {
            for (int i = 0; i < 8; i++) {
                this->name[i] = name[i];
            }
        }
    };

    std::vector<DeviceData> device_list;

    bool fileSystemCapacity[6] = { true, true, false, false, false, false };


    int writeData_to_wait = 0;
    int file_to_write_id = 0;

    uint32_t current_dir_id = 0;
    std::vector<std::string> elem_in_path;
    uint16_t server_capability_number = 0;

    uint32_t ioCode1 = 0;
    uint32_t extendedPDU = 0;
    uint32_t extraFlags1 = 0;
    uint32_t SpecialTypeDeviceCap = 0;
    uint32_t general_capability_version = 0;

    const uint32_t channel_flags;



    ClientRDPDRChannel(RDPVerbose verbose, ClientChannelMod * callback, RDPDiskConfig & config);

    ~ClientRDPDRChannel();

    void set_api(ClientIODiskAPI * impl_io_disk);

    void receive(InStream & chunk) /*NOLINT*/;

    void FremoveDriveDevice(const DeviceData * devices, const size_t deviceCount);

    void set_share_dir(const std::string & share_dir);



    void process_server_annnounce_request(InStream & chunk);

    void process_core_server_capabilities(InStream & chunk);

    void process_core_clientID_confirm();

    void process_core_device_reply(InStream & chunk);

    void process_core_device_iorequest(InStream & chunk);



    void process_iorequest_lock_control(rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream);

    void process_iorequest_create(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id);

    void process_iorequest_query_information(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id);

    void process_iorequest_close(rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id);

    void process_iorequest_read(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id);

    void process_iorequest_directory_control(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id, uint32_t minor_function);

    void process_iorequest_query_volume_information(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id);

    void provess_iorequest_write(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id);

    void process_iorequest_set_information(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id);

    void process_iorequest_device_control(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream);
};

