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
    Copyright (C) Wallix 2017
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "acl/license_api.hpp"
#include "utils/fileutils.hpp"
#include "utils/log.hpp"
#include "utils/sugar/unique_fd.hpp"

#include <algorithm>
#include <string>
#include <cstring>

#include <sys/stat.h>

class FileSystemLicenseStore : public LicenseApi
{
public:
    FileSystemLicenseStore(std::string license_path_, bool use_target_ip_in_filename)
    : license_path(std::move(license_path_))
    , use_target_ip_in_filename(use_target_ip_in_filename)
    {
        LOG(LOG_INFO, "FileSystemLicenseStore::FileSystemLicenseStore(): UseTargetIPInFilename=%s", use_target_ip_in_filename ? "Yes" : "No");
    }

    // The functions shall return empty bytes_view to indicate the error.
    bytes_view get_license_v1(char const* client_name, char const* target_ip, uint32_t version, char const* scope, char const* company_name, char const* product_id, std::array<uint8_t, LIC::LICENSE_HWID_SIZE>& hwid, writable_bytes_view out, bool enable_log) override
    {
        char license_index[2048] = {};
        if (this->use_target_ip_in_filename)
        {
            ::snprintf(license_index, sizeof(license_index) - 1, "%s_0x%08X_%s_%s_%s", target_ip, version, scope, company_name, product_id);
        }
        else
        {
            ::snprintf(license_index, sizeof(license_index) - 1, "0.0.0.0_0x%08X_%s_%s_%s", version, scope, company_name, product_id);
        }
        license_index[sizeof(license_index) - 1] = '\0';
        std::replace(std::begin(license_index), std::end(license_index), ' ', '-');
        LOG_IF(enable_log, LOG_INFO, "FileSystemLicenseStore::get_license_v1(): LicenseIndex=\"%s\"", license_index);

        char filename[4096] = {};
        ::snprintf(filename, sizeof(filename) - 1, "%s/%s/%s",
            license_path.c_str(), client_name, license_index);
        filename[sizeof(filename) - 1] = '\0';
        LOG_IF(enable_log, LOG_INFO, "FileSystemLicenseStore::get_license_v1(): Filename=\"%s\"", filename);

        if (unique_fd ufd{::open(filename, O_RDONLY)}) {
            size_t number_of_bytes_read = ::read(ufd.fd(), hwid.data(), hwid.size());
            if (number_of_bytes_read != sizeof(hwid)) {
                LOG(LOG_ERR, "FileSystemLicenseStore::get_license_v1: license file truncated (1) : expected %zu, got %zu", sizeof(hwid), number_of_bytes_read);
            }
            else {
                uint32_t license_size = 0;
                number_of_bytes_read = ::read(ufd.fd(), &license_size, sizeof(license_size));
                if (number_of_bytes_read != sizeof(license_size)) {
                    LOG(LOG_ERR, "FileSystemLicenseStore::get_license_v1: license file truncated (2) : expected %zu, got %zu", sizeof(license_size), number_of_bytes_read);
                }
                else {
                    if (out.size() >= license_size)
                    {
                        number_of_bytes_read = ::read(ufd.fd(), out.data(), license_size);
                        if (number_of_bytes_read != license_size) {
                            LOG(LOG_ERR, "FileSystemLicenseStore::get_license_v1: license file truncated (3) : expected %u, got %zu", license_size, number_of_bytes_read);
                        }
                        else {
                            LOG(LOG_INFO, "FileSystemLicenseStore::get_license_v1: LicenseSize=%u", license_size);
                            if (enable_log)
                            {
                                hexdump(hwid.data(), hwid.size());
                            }

                            return bytes_view { out.data(), license_size };
                        }
                    }
                }
            }
        }
        else {
            LOG(LOG_WARNING, "FileSystemLicenseStore::get_license_v1: Failed to open license file! Path=\"%s\" errno=%s(%d)", filename, strerror(errno), errno);
        }

        return bytes_view { out.data(), 0 };
    }

    // The functions shall return empty bytes_view to indicate the error.
    bytes_view get_license_v0(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, writable_bytes_view out, bool enable_log) override
    {
        char license_index[2048] = {};
        ::snprintf(license_index, sizeof(license_index) - 1, "0x%08X_%s_%s_%s", version, scope, company_name, product_id);
        license_index[sizeof(license_index) - 1] = '\0';
        std::replace(std::begin(license_index), std::end(license_index), ' ', '-');
        LOG_IF(enable_log, LOG_INFO, "FileSystemLicenseStore::get_license_v0(): LicenseIndex=\"%s\"", license_index);

        char filename[4096] = {};
        ::snprintf(filename, sizeof(filename) - 1, "%s/%s/%s",
            license_path.c_str(), client_name, license_index);
        filename[sizeof(filename) - 1] = '\0';
        LOG_IF(enable_log, LOG_INFO, "FileSystemLicenseStore::get_license_v0(): Filename=\"%s\"", filename);

        if (unique_fd ufd{::open(filename, O_RDONLY)}) {
            uint32_t license_size = 0;
            size_t number_of_bytes_read = ::read(ufd.fd(), &license_size, sizeof(license_size));
            if (number_of_bytes_read != sizeof(license_size)) {
                LOG(LOG_ERR, "FileSystemLicenseStore::get_license_v0: license file truncated (1) : expected %zu, got %zu", sizeof(license_size), number_of_bytes_read);
            }
            else {
                if (out.size() >= license_size)
                {
                    number_of_bytes_read = ::read(ufd.fd(), out.data(), license_size);
                    if (number_of_bytes_read != license_size) {
                        LOG(LOG_ERR, "FileSystemLicenseStore::get_license_v0: license file truncated (2) : expected %u, got %zu", license_size, number_of_bytes_read);
                    }
                    else {
                        LOG(LOG_INFO, "FileSystemLicenseStore::get_license_v0: LicenseSize=%u", license_size);

                        return bytes_view { out.data(), license_size };
                    }
                }
            }
        }
        else {
            LOG(LOG_WARNING, "FileSystemLicenseStore::get_license_v0: Failed to open license file! Path=\"%s\" errno=%s(%d)", filename, strerror(errno), errno);
        }

        return bytes_view { out.data(), 0 };
    }

    bool put_license(char const* client_name, char const* target_ip, uint32_t version, char const* scope, char const* company_name, char const* product_id, std::array<uint8_t, LIC::LICENSE_HWID_SIZE> const& hwid, bytes_view in, bool enable_log) override
    {
        char license_index[2048] = {};
        if (this->use_target_ip_in_filename)
        {
            ::snprintf(license_index, sizeof(license_index) - 1, "%s_0x%08X_%s_%s_%s", target_ip, version, scope, company_name, product_id);
        }
        else
        {
            ::snprintf(license_index, sizeof(license_index) - 1, "0.0.0.0_0x%08X_%s_%s_%s", version, scope, company_name, product_id);
        }
        license_index[sizeof(license_index) - 1] = '\0';
        std::replace_if(std::begin(license_index), std::end(license_index),
                        [](unsigned char c) { return (' ' == c); }, '-');
        LOG_IF(enable_log, LOG_INFO, "FileSystemLicenseStore::put_license(): LicenseIndex=\"%s\"", license_index);

        char license_dir_path[4096] = {};
        ::snprintf(license_dir_path, sizeof(license_dir_path), "%s/%s",
            license_path.c_str(), client_name);
        license_dir_path[sizeof(license_dir_path) - 1] = '\0';
        if (::recursive_create_directory(license_dir_path, S_IRWXU | S_IRWXG, -1) != 0) {
            LOG(LOG_ERR, "FileSystemLicenseStore::put_license(): Failed to create directory: \"%s\"", license_dir_path);
            return false;
        }

        char filename_temporary[4096] = {};
        auto res = ::snprintf(filename_temporary, sizeof(filename_temporary) - 1, "%s/%s-XXXXXX.tmp",
            license_dir_path, license_index);
        if (res < 0 || (strlen(license_dir_path)+strlen(license_index)+12 >= sizeof(filename_temporary))){
            LOG(LOG_ERR, "FileSystemLicenseStore::put_license: temporary filename for Licence truncated: %s/%s",
                license_dir_path, license_index);
            return false;
        }
        filename_temporary[sizeof(filename_temporary) - 1] = '\0';

        int fd = ::mkostemps(filename_temporary, 4, O_CREAT | O_WRONLY);
        if (fd != -1) {
            unique_fd ufd{fd};
            uint32_t const license_size = in.size();
            LOG(LOG_INFO, "FileSystemLicenseStore::put_license: LicenseSize=%u", license_size);
            if (hwid.size() == ::write(ufd.fd(), hwid.data(), hwid.size())) {
                if (enable_log)
                {
                    hexdump(hwid.data(), hwid.size());
                }

                if (sizeof(license_size) == ::write(ufd.fd(), &license_size, sizeof(license_size))) {
                    if (license_size == ::write(ufd.fd(), in.data(), in.size())) {
                        char filename[6145] = {};
                        ::snprintf(filename, sizeof(filename) - 1, "%s/%s", license_dir_path, license_index);
                        filename[sizeof(filename) - 1] = '\0';
                        LOG_IF(enable_log, LOG_INFO, "FileSystemLicenseStore::put_license(): Filename=\"%s\"", filename);

                        if (::rename(filename_temporary, filename) == 0) {
                            return true;
                        }

                        LOG( LOG_ERR
                            , "FileSystemLicenseStore::put_license: failed to rename the (temporary) license file! "
                                "temporary_filename=\"%s\" filename=\"%s\" errno=%s(%d)"
                            , filename_temporary, filename, strerror(errno), errno);
                        ::unlink(filename_temporary);
                    }
                    else {
                        LOG( LOG_ERR
                           , "FileSystemLicenseStore::put_license: Failed to write (temporary) license file (1)! filename=\"%s\" errno=%s(%d)"
                           , filename_temporary, strerror(errno), errno);
                    }
                }
                else {
                    LOG( LOG_ERR
                       , "FileSystemLicenseStore::put_license: Failed to write (temporary) license file (2)! filename=\"%s\" errno=%s(%d)"
                       , filename_temporary, strerror(errno), errno);
                }
            }
            else {
                LOG( LOG_ERR
                   , "FileSystemLicenseStore::put_license: Failed to write (temporary) license file (3)! filename=\"%s\" errno=%s(%d)"
                   , filename_temporary, strerror(errno), errno);
            }
        }
        else {
            LOG( LOG_ERR
               , "FileSystemLicenseStore::put_license: Failed to open (temporary) license file for writing! filename=\"%s\" errno=%s(%d)"
               , filename_temporary, strerror(errno), errno);
        }

        return false;
    }

private:
    std::string const license_path;

    bool const use_target_ip_in_filename;
};
