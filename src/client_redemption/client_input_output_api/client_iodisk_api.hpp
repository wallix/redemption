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
   Copyright (C) Wallix 2010-2013
   Author(s): Clément Moroldo, David Fort
*/

#pragma once

#include "utils/log.hpp"


#include "client_redemption/client_redemption_api.hpp"



#include "core/FSCC/FileInformation.hpp"

constexpr long long WINDOWS_TICK = 10000000;
constexpr long long SEC_TO_UNIX_EPOCH = 11644473600LL;

class ClientIODiskAPI : public ClientIO {


public:
    virtual ~ClientIODiskAPI() = default;

    struct FileStat {

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

    struct FileStatvfs {

        uint64_t VolumeCreationTime             = 0;
        const char * VolumeLabel                = "";
        const char * FileSystemName             = "ext4";

        uint32_t FileSystemAttributes           = fscc::NEW_FILE_ATTRIBUTES;
        uint32_t SectorsPerAllocationUnit       = 8;

        uint32_t BytesPerSector                 = 0;
        uint32_t MaximumComponentNameLength     = 0;
        uint64_t TotalAllocationUnits           = 0;
        uint64_t CallerAvailableAllocationUnits = 0;
        uint64_t AvailableAllocationUnits       = 0;
        uint64_t ActualAvailableAllocationUnits = 0;
    };




    unsigned WindowsTickToUnixSeconds(long long windowsTicks) {
        return unsigned((windowsTicks / WINDOWS_TICK) - SEC_TO_UNIX_EPOCH);
    }

    long long UnixSecondsToWindowsTick(unsigned unixSeconds) {
        return ((unixSeconds + SEC_TO_UNIX_EPOCH) * WINDOWS_TICK);
    }

    uint32_t string_to_hex32(unsigned char * str) {
        size_t size = sizeof(str);
        uint32_t hex32(0);
        for (size_t i = 0; i < size; i++) {
            int s = str[i];
            if(s > 47 && s < 58) {                      //this covers 0-9
                hex32 += (s - 48) << (size - i - 1);
            } else if (s > 64 && s < 71) {              // this covers A-F
                hex32 += (s - 55) << (size - i - 1);
            } else if (s > 'a'-1 && s < 'f'+1) {        // this covers a-f
                hex32 += (s - 'a') << (size - i - 1);
            }
        }
        return hex32;
    }

    virtual bool ifile_good(const char * new_path) = 0;

    virtual bool ofile_good(const char * new_path) = 0;

    virtual int get_file_size(const char * path) = 0;

    virtual bool dir_good(const char * new_path) = 0;

    virtual void marke_dir(const char * new_path) = 0;

    virtual FileStat get_file_stat(const char * file_to_request) = 0;

    virtual FileStatvfs get_file_statvfs(const char * file_to_request) = 0;

    virtual void read_data(
        std::string const& file_to_tread, int offset, byte_array data) = 0;

    virtual bool set_elem_from_dir(std::vector<std::string> & elem_list, const std::string & str_dir_path) = 0;

    virtual int get_device(const char * file_path) = 0;

    virtual uint32_t get_volume_serial_number(int device) = 0;

    virtual bool write_file(const char * file_to_write, const char * data, int data_len) = 0;

    virtual bool remove_file(const char * file_to_remove) = 0;

    virtual bool rename_file(const char * file_to_rename,  const char * new_name) = 0;
};

