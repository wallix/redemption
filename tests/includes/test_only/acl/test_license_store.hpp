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

#include "acl/file_system_license_store.hpp"

#include "utils/hexdump.hpp"
#include "utils/log.hpp"
#include "utils/sugar/bytes_view.hpp"

class CaptureLicenseStore : public LicenseApi
{
public:
    // The functions shall return empty bytes_view to indicate the error.
    bytes_view get_license(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, writable_bytes_view out, bool enable_log) override
    {
        (void)client_name;
        (void)version;
        (void)scope;
        (void)company_name;
        (void)product_id;
        (void)out;
        (void)enable_log;

        return bytes_view { out.data(), 0 };
    }

    bool put_license(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, bytes_view in, bool enable_log) override
    {
        (void)enable_log;

        LOG(LOG_INFO, "/*CaptureLicenseStore */ const char license_client_name[] =");
        hexdump_c(client_name, ::strlen(client_name));
        LOG(LOG_INFO, "/*CaptureLicenseStore */ ;");

        LOG(LOG_INFO, "/*CaptureLicenseStore */ uint32_t license_version = %u", version);
        LOG(LOG_INFO, "/*CaptureLicenseStore */ ;");

        LOG(LOG_INFO, "/*CaptureLicenseStore */ const char license_scope[] =");
        hexdump_c(scope, ::strlen(scope));
        LOG(LOG_INFO, "/*CaptureLicenseStore */ ;");

        LOG(LOG_INFO, "/*CaptureLicenseStore */ const char license_company_name[] =");
        hexdump_c(company_name, ::strlen(company_name));
        LOG(LOG_INFO, "/*CaptureLicenseStore */ ;");

        LOG(LOG_INFO, "/*CaptureLicenseStore */ const char license_product_id[] =");
        hexdump_c(product_id, ::strlen(product_id));
        LOG(LOG_INFO, "/*CaptureLicenseStore */ ;");

        LOG(LOG_INFO, "/*CaptureLicenseStore */ const uint8_t license_data[%zu] = {", in.size());
        hexdump_d(in.data(), in.size());
        LOG(LOG_INFO, "/*CaptureLicenseStore */ };");

        return true;
    }
};

class CompareLicenseStore : public CaptureLicenseStore
{
public:
    CompareLicenseStore(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, bytes_view license_data) :
        expected_client_name(client_name),
        expected_version(version),
        expected_scope(scope),
        expected_company_name(company_name),
        expected_product_id(product_id),
        expected_license_data(license_data) {}

    bool put_license(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, bytes_view in, bool enable_log) override
    {
        (void)enable_log;

        RED_CHECK_EQ(client_name,  this->expected_client_name);
        RED_CHECK_EQ(version,      this->expected_version);
        RED_CHECK_EQ(scope,        this->expected_scope);
        RED_CHECK_EQ(company_name, this->expected_company_name);
        RED_CHECK_EQ(product_id,   this->expected_product_id);

        RED_REQUIRE_EQ(in.size(), this->expected_license_data.size());

        RED_CHECK_MEM(in, this->expected_license_data);

        return true;
    }

private:
    char const* expected_client_name;
    uint32_t    expected_version;
    char const* expected_scope;
    char const* expected_company_name;
    char const* expected_product_id;
    bytes_view  expected_license_data;
};


class ReplayLicenseStore : public LicenseApi
{
public:
    ReplayLicenseStore(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, bytes_view license_data) :
        expected_client_name(client_name),
        expected_version(version),
        expected_scope(scope),
        expected_company_name(company_name),
        expected_product_id(product_id),
        expected_license_data(license_data) {}

    // The functions shall return empty bytes_view to indicate the error.
    bytes_view get_license(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, writable_bytes_view out, bool enable_log) override
    {
        (void)enable_log;

        RED_CHECK_EQ(client_name,  this->expected_client_name);
        RED_CHECK_EQ(version,      this->expected_version);
        RED_CHECK_EQ(scope,        this->expected_scope);
        RED_CHECK_EQ(company_name, this->expected_company_name);
        RED_CHECK_EQ(product_id,   this->expected_product_id);

        RED_REQUIRE_GE(out.size(), this->expected_license_data.size());

        size_t const effective_license_size = std::min(out.size(), this->expected_license_data.size());

        ::memcpy(out.data(), this->expected_license_data.data(), effective_license_size);

        return bytes_view { out.data(), effective_license_size };
    }

    bool put_license(char const* client_name, uint32_t version, char const* scope, char const* company_name, char const* product_id, bytes_view in, bool enable_log) override
    {
        (void)enable_log;
        (void)in;

        RED_CHECK_EQ(client_name,  this->expected_client_name);
        RED_CHECK_EQ(version,      this->expected_version);
        RED_CHECK_EQ(scope,        this->expected_scope);
        RED_CHECK_EQ(company_name, this->expected_company_name);
        RED_CHECK_EQ(product_id,   this->expected_product_id);

        return true;
    }

private:
    char const* expected_client_name;
    uint32_t    expected_version;
    char const* expected_scope;
    char const* expected_company_name;
    char const* expected_product_id;
    bytes_view  expected_license_data;
};
