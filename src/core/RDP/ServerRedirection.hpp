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
    Author(s): Christophe Grosjean, Jonathan Poelen,
               Meng Tan, Raphael Zhou
*/


#pragma once

#include <cinttypes>

#include "utils/log.hpp"
#include "core/error.hpp"
#include "utils/stream.hpp"
#include "utils/redirection_info.hpp"


// [MS-RDPBCGR] - 2.2.13.1 Server Redirection Packet
//  (RDP_SERVER_REDIRECTION_PACKET)
// ======================================================

// The RDP_SERVER_REDIRECTION_PACKET structure contains information to enable
//  a client to reconnect to a session on a specified server. This data is sent
//  to a client in a Redirection PDU to enable load-balancing of Remote Desktop
//  sessions across a collection of machines. For more information about the load
//  balancing of Remote Desktop sessions,
//  see [MSFT-SDLBTS] "Load-Balanced Configurations" and "Revectoring Clients".

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |            Flags              |            length             |
// +-------------------------------+-------------------------------+
// |                          SessionID                            |
// +---------------------------------------------------------------+
// |                          RedirFlags                           |
// +---------------------------------------------------------------+
// |               TargetNetAddressLength (optional)               |
// +---------------------------------------------------------------+
// |                  TargetNetAddress (variable)                  |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |                LoadBalanceInfoLength (optional)               |
// +---------------------------------------------------------------+
// |                   LoadBalanceInfo (variable)                  |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |                    UserNameLength (optional)                  |
// +---------------------------------------------------------------+
// |                       UserName (variable)                     |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |                     DomainLength (optional)                   |
// +---------------------------------------------------------------+
// |                        Domain (variable)                      |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |                    PasswordLength (optional)                  |
// +---------------------------------------------------------------+
// |                       Password (variable)                     |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |                  TargetFQDNLength (optional)                  |
// +---------------------------------------------------------------+
// |                     TargetFQDN (variable)                     |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |              TargetNetBiosNameLength (optional)               |
// +---------------------------------------------------------------+
// |                 TargetNetBiosName (variable)                  |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |                    TsvUrlLength (optional)                    |
// +---------------------------------------------------------------+
// |                       TsvUrl (variable)                       |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |              TargetNetAddressesLength (optional)              |
// +---------------------------------------------------------------+
// |                TargetNetAddresses (variable)                  |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |                         Pad (optional)                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+


// Flags (2 bytes): A 16-bit unsigned integer. The server redirection identifier.
//  This field MUST be set to SEC_REDIRECTION_PKT (0x0400).
enum {
    SEC_REDIRECTION_PKT = 0x0400
};

// Length (2 bytes): A 16-bit unsigned integer. The overall length, in bytes,
//  of the Server Redirection Packet structure.

// SessionID (4 bytes): A 32-bit unsigned integer. The session identifier to which
//  the client MUST reconnect. This identifier MUST be specified in the
//  RedirectSessionID field of the Client Cluster Data (section 2.2.1.3.5) if a
//  reconnect attempt takes place. The Client Cluster Data is transmitted as part
//  of the MCS Connect Initial PDU (section 2.2.1.3).

// RedirFlags (4 bytes): A 32-bit unsigned integer. A bit field that contains
//  redirection information flags, some of which indicate the presence of
//  additional data at the end of the packet.

//  +-------------------------+---------------------------------------------------+
//  | Flag                    | Meaning                                           |
//  +-------------------------+---------------------------------------------------+
//  | LB_TARGET_NET_ADDRESS   | Indicates that the TargetNetAddressLength and     |
//  | 0x00000001              | TargetNetAddress fields are present.              |
//  +-------------------------+---------------------------------------------------+
//  | LB_LOAD_BALANCE_INFO    | Indicates that the LoadBalanceInfoLength and      |
//  | 0x00000002              | LoadBalanceInfo fields are present.               |
//  +-------------------------+---------------------------------------------------+
//  | LB_USERNAME             | Indicates that the UserNameLength and UserName    |
//  | 0x00000004              | fields are present.                               |
//  +-------------------------+---------------------------------------------------+
//  | LB_DOMAIN               | Indicates that the DomainLength and Domain        |
//  | 0x00000008              | fields are present.                               |
//  +-------------------------+---------------------------------------------------+
//  | LB_PASSWORD             | Indicates that the PasswordLength and Password    |
//  | 0x00000010              | fields are present.                               |
//  +-------------------------+---------------------------------------------------+
//  | LB_DONTSTOREUSERNAME    | Indicates that when reconnecting, the client MUST |
//  | 0x00000020              | send the username specified in the UserName field |
//  |                         | to the server in the Client Info PDU              |
//  |                         | (section 2.2.1.11.1.1).                           |
//  +-------------------------+---------------------------------------------------+
//  | LB_SMARTCARD_LOGON      | Indicates that the user can use a smart card for  |
//  | 0x00000040              | authentication.                                   |
//  +-------------------------+---------------------------------------------------+
//  | LB_NOREDIRECT           | Indicates that the contents of the PDU are for    |
//  | 0x00000080              | informational purposes only.                      |
//  |                         | No actual redirection is required.                |
//  +-------------------------+---------------------------------------------------+
//  | LB_TARGET_FQDN          | Indicates that the TargetFQDNLength and           |
//  | 0x00000100              | TargetFQDN fields are present.                    |
//  +-------------------------+---------------------------------------------------+
//  | LB_TARGET_NETBIOS_NAME  | Indicates that the TargetNetBiosNameLength and    |
//  | 0x00000200              | TargetNetBiosName fields are present.             |
//  +-------------------------+---------------------------------------------------+
//  | LB_TARGET_NET_ADDRESSES | Indicates that the TargetNetAddressesLength and   |
//  | 0x00000800              | TargetNetAddresses fields are present.            |
//  +-------------------------+---------------------------------------------------+
//  | LB_CLIENT_TSV_URL       | Indicates that the TsvUrlLength and TsvUrl        |
//  | 0x00001000              | fields are present.                               |
//  +-------------------------+---------------------------------------------------+
//  | LB_SERVER_TSV_CAPABLE   | Indicates that the server supports redirection    |
//  | 0x00002000              | based on the TsvUrl present in the                |
//  |                         | LoadBalanceInfo sent by the client.               |
//  +-------------------------+---------------------------------------------------+

enum {
    LB_TARGET_NET_ADDRESS   = 0x00000001,
    LB_LOAD_BALANCE_INFO    = 0x00000002,
    LB_USERNAME             = 0x00000004,
    LB_DOMAIN               = 0x00000008,
    LB_PASSWORD             = 0x00000010,
    LB_DONTSTOREUSERNAME    = 0x00000020,
    LB_SMARTCARD_LOGON      = 0x00000040,
    LB_NOREDIRECT           = 0x00000080,
    LB_TARGET_FQDN          = 0x00000100,
    LB_TARGET_NETBIOS_NAME  = 0x00000200,
    LB_TARGET_NET_ADDRESSES = 0x00000800,
    LB_CLIENT_TSV_URL       = 0x00001000,
    LB_SERVER_TSV_CAPABLE   = 0x00002000
};

// TargetNetAddressLength (4 bytes): A 32-bit unsigned integer. The length,
//  in bytes, of the TargetNetAddress field.

// TargetNetAddress (variable): A variable-length array of bytes containing the
//  IP address of the server (for example, "192.168.0.1" using dotted decimal
//  notation) in Unicode format, including a null-terminator.

// LoadBalanceInfoLength (4 bytes): A 32-bit unsigned integer. The length,
//  in bytes, of the LoadBalanceInfo field.

// LoadBalanceInfo (variable): A variable-length array of bytes containing
//  load balancing information that MUST be treated as opaque data by the client
//  and passed to the server if the LB_TARGET_NET_ADDRESS (0x00000001) flag is
//  not present in the RedirFlags field and a reconnection takes place.
//  See section 3.2.5.3.1 for details on populating the routingToken field of
//  the X.224 Connection Request PDU (section 2.2.1.1).

// UserNameLength (4 bytes): A 32-bit unsigned integer. The length, in bytes,
//  of the UserName field.

// UserName (variable): A variable-length array of bytes containing the username
//  of the user in Unicode format, including a null-terminator.

// DomainLength (4 bytes): A 32-bit unsigned integer. The length, in bytes,
//  of the Domain field.

// Domain (variable): A variable-length array of bytes containing the domain
//  to which the user connected in Unicode format, including a null-terminator.

// PasswordLength (4 bytes): A 32-bit unsigned integer. The length, in bytes,
//  of the Password field.

// Password (variable): A variable-length array of bytes containing the password
//  used by the user in Unicode format, including a null-terminator or a cookie
//  value that MUST be passed to the target server on successful connection.

// TargetFQDNLength (4 bytes): A 32-bit unsigned integer. The length, in bytes,
//  of the TargetFQDN field.

// TargetFQDN (variable): A variable-length array of bytes containing the fully
//  qualified domain name (FQDN) of the target machine, including a null-terminator.

// TargetNetBiosNameLength (4 bytes): A 32-bit unsigned integer. The length,
//  in bytes, of the TargetNetBiosName field.

// TargetNetBiosName (variable): A variable-length array of bytes containing
//  the NETBIOS name of the target machine, including a null-terminator.

// TsvUrlLength (4 bytes): The length, in bytes, of the TsvUrl field.

// TsvUrl (variable): A variable-length array of bytes. If the client has
//  previously sent a TsvUrl field in the LoadBalanceInfo to the server in
//  the expected format, then the server will return the same TsvUrl to the
//  client in this field. The client verifies that it is the same as the one
//  that it previously passed to the server and if they don’t match, the client
//  immediately disconnects the connection.

// TargetNetAddressesLength (4 bytes): A 32-bit unsigned integer. The length,
//  in bytes, of the TargetNetAddresses field.

// TargetNetAddresses (variable): A variable-length array of bytes containing
//  the target IP addresses of the server to connect against, stored in a
//  Target Net Addresses structure (section 2.2.13.1.1).

// Pad (8 bytes): An optional 8-element array of 8-bit unsigned integers. Padding.
//  Values in this field MUST be ignored.


struct ServerRedirectionPDU {
    uint16_t Flags{SEC_REDIRECTION_PKT};
    uint16_t Length{0};
    uint32_t SessionID{0};
    uint32_t RedirFlags{0};
    uint32_t TargetNetAddressLength{0};
    uint8_t  TargetNetAddress[256]{};
    uint32_t LoadBalanceInfoLength{0};
    uint8_t  LoadBalanceInfo[1024]{};
    uint32_t UserNameLength{0};
    uint8_t  UserName[512]{};
    uint32_t DomainLength{0};
    uint8_t  Domain[512]{};
    uint32_t PasswordLength{0};
    uint8_t  Password[512]{};
    uint32_t TargetFQDNLength{0};
    uint8_t  TargetFQDN[1024]{};
    uint32_t TargetNetBiosNameLength{0};
    uint8_t  TargetNetBiosName[512]{};
    uint32_t TsvUrlLength{0};
    uint8_t  TsvUrl[1024]{};
    uint32_t TargetNetAddressesLength{0};
    uint8_t  TargetNetAddresses[1024]{};

    ServerRedirectionPDU()
         
    = default;

    unsigned recv_field_process(InStream & stream, uint8_t* field, size_t field_size,
                                uint32_t & field_length) {
        unsigned offset = 0;
        field_length = stream.in_uint32_le();
        offset += 4;
        if ((field_length > field_size) ||
            !stream.in_check_rem(field_length)) {
            LOG(LOG_INFO, "error recv_field_process field_length=%" PRIu32 ", sizeof(field)=%zu",
                field_length, field_size);
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }
        stream.in_copy_bytes(field, field_length);
        offset += field_length;
        return offset;
    }

    void receive(InStream & stream) {
        unsigned offset = 0;
        unsigned expected = 12; /* Flags(2) + Length(2) +
                                   SessionID(4) + RedirFlags(4) */
        if (!stream.in_check_rem(expected)) {
            LOG( LOG_ERR
               , "ServerRedirection::receive RDP_SERVER_REDIRECTION_PACKET"
                 " - Truncated data, need=%u, remains=%zu"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }
        this->Flags = stream.in_uint16_le();
        this->Length = stream.in_uint16_le();
        this->SessionID = stream.in_uint32_le();
        this->RedirFlags = stream.in_uint32_le();
        offset += 12;
        if (this->RedirFlags & LB_TARGET_NET_ADDRESS) {
            offset += this->recv_field_process(stream, this->TargetNetAddress,
                                               sizeof(this->TargetNetAddress),
                                               this->TargetNetAddressLength);
        }
        if (this->RedirFlags & LB_LOAD_BALANCE_INFO) {
            offset += this->recv_field_process(stream, this->LoadBalanceInfo,
                                               sizeof(this->LoadBalanceInfo),
                                               this->LoadBalanceInfoLength);
        }
        if (this->RedirFlags & LB_USERNAME) {
            offset += this->recv_field_process(stream, this->UserName,
                                               sizeof(this->UserName),
                                               this->UserNameLength);
        }
        if (this->RedirFlags & LB_DOMAIN) {
            offset += this->recv_field_process(stream, this->Domain,
                                               sizeof(this->Domain),
                                               this->DomainLength);
        }
        if (this->RedirFlags & LB_PASSWORD) {
            offset += this->recv_field_process(stream, this->Password,
                                               sizeof(this->Password),
                                               this->PasswordLength);
        }
        if (this->RedirFlags & LB_TARGET_FQDN) {
            offset += this->recv_field_process(stream, this->TargetFQDN,
                                               sizeof(this->TargetFQDN),
                                               this->TargetFQDNLength);
        }
        if (this->RedirFlags & LB_TARGET_NETBIOS_NAME) {
            offset += this->recv_field_process(stream, this->TargetNetBiosName,
                                               sizeof(this->TargetNetBiosName),
                                               this->TargetNetBiosNameLength);
        }
        if (this->RedirFlags & LB_CLIENT_TSV_URL) {
            offset += this->recv_field_process(stream, this->TsvUrl,
                                               sizeof(this->TsvUrl),
                                               this->TsvUrlLength);
        }
        if (this->RedirFlags & LB_TARGET_NET_ADDRESSES) {
            offset += this->recv_field_process(stream, this->TargetNetAddresses,
                                               sizeof(this->TargetNetAddresses),
                                               this->TargetNetAddressesLength);
        }
        int remains = this->Length - offset;
        // LOG(LOG_INFO, "receive RDP_SERVER_REDIRECTION_PACKET pad = %d", remains);
        if ((remains <= 8) && (remains >= 0)) {
            stream.in_skip_bytes(remains);
        }
    }

    void emit(OutStream & stream) const {
        unsigned expected = 12; /* Flags(2) + Length(2) +
                                   SessionID(4) + RedirFlags(4) */
        if (!stream.has_room(expected)) {
            LOG( LOG_ERR
               , "ServerRedirectionPDU::emit - stream too small, need=%u, remains=%u"
               , expected
               , static_cast<unsigned>(stream.tailroom()));
            throw Error(ERR_STREAM_MEMORY_TOO_SMALL);
        }
        stream.out_uint16_le(this->Flags);
        stream.out_uint16_le(this->Length);
        stream.out_uint32_le(this->SessionID);
        stream.out_uint32_le(this->RedirFlags);
        if (this->RedirFlags & LB_TARGET_NET_ADDRESS) {
            stream.out_uint32_le(this->TargetNetAddressLength);
            stream.out_copy_bytes(this->TargetNetAddress,
                                  this->TargetNetAddressLength);
        }
        if (this->RedirFlags & LB_LOAD_BALANCE_INFO) {
            stream.out_uint32_le(this->LoadBalanceInfoLength);
            stream.out_copy_bytes(this->LoadBalanceInfo, this->LoadBalanceInfoLength);
        }
        if (this->RedirFlags & LB_USERNAME) {
            stream.out_uint32_le(this->UserNameLength);
            stream.out_copy_bytes(this->UserName, this->UserNameLength);
        }
        if (this->RedirFlags & LB_DOMAIN) {
            stream.out_uint32_le(this->DomainLength);
            stream.out_copy_bytes(this->Domain, this->DomainLength);
        }
        if (this->RedirFlags & LB_PASSWORD) {
            stream.out_uint32_le(this->PasswordLength);
            stream.out_copy_bytes(this->Password, this->PasswordLength);
        }
        if (this->RedirFlags & LB_TARGET_FQDN) {
            stream.out_uint32_le(this->TargetFQDNLength);
            stream.out_copy_bytes(this->TargetFQDN, this->TargetFQDNLength);
        }
        if (this->RedirFlags & LB_TARGET_NETBIOS_NAME) {
            stream.out_uint32_le(this->TargetNetBiosNameLength);
            stream.out_copy_bytes(this->TargetNetBiosName,
                                  this->TargetNetBiosNameLength);
        }
        if (this->RedirFlags & LB_CLIENT_TSV_URL) {
            stream.out_uint32_le(this->TsvUrlLength);
            stream.out_copy_bytes(this->TsvUrl, this->TsvUrlLength);
        }
        if (this->RedirFlags & LB_TARGET_NET_ADDRESSES) {
            stream.out_uint32_le(this->TargetNetAddressesLength);
            stream.out_copy_bytes(this->TargetNetAddresses,
                                  this->TargetNetAddressesLength);
        }
    }

    bool DontStoreUsername() {
        return (this->RedirFlags & LB_DONTSTOREUSERNAME);
    }
    bool ServerTsvCapable() {
        return (this->RedirFlags & LB_SERVER_TSV_CAPABLE);
    }
    bool SmartcardLogon() {
        return (this->RedirFlags & LB_SMARTCARD_LOGON);
    }
    bool Noredirect() {
        return (this->RedirFlags & LB_NOREDIRECT);
    }
    bool UseLoadBalanceInfo() {
        return (!(this->RedirFlags & LB_TARGET_NET_ADDRESS) &&
                (this->RedirFlags & LB_LOAD_BALANCE_INFO) &&
                (!this->Noredirect()));
    }

    void log(int level, const char * message) const {
        LOG(level
            , "%s: ServerRedirectionPDU(Flags=%u, Length=%u, SessionID=%u, RedirFlags=%u)"
            , message
            , this->Flags
            , this->Length
            , this->SessionID
            , this->RedirFlags
            );

        LOG(level, "RedirectionFlags {");
        if (this->RedirFlags & LB_TARGET_NET_ADDRESS) {
            LOG(level, "LB_TARGET_NET_ADDRESS,");
            LOG(level, "length = %u", this->TargetNetAddressLength);
            hexdump_c(this->TargetNetAddress, this->TargetNetAddressLength);
        }
        if (this->RedirFlags & LB_LOAD_BALANCE_INFO) {
            LOG(level, "LB_LOAD_BALANCE_INFO,");
            LOG(level, "length = %u", this->LoadBalanceInfoLength);
            hexdump_c(this->LoadBalanceInfo, this->LoadBalanceInfoLength);
        }
        if (this->RedirFlags & LB_USERNAME) {
            LOG(level, "LB_USERNAME,");
            LOG(level, "length = %u", this->UserNameLength);
            hexdump_c(this->UserName, this->UserNameLength);
        }
        if (this->RedirFlags & LB_DOMAIN) {
            LOG(level, "LB_DOMAIN,");
            LOG(level, "length = %u", this->DomainLength);
            hexdump_c(this->Domain, this->DomainLength);
        }
        if (this->RedirFlags & LB_PASSWORD) {
            LOG(level, "LB_PASSWORD,");
            LOG(level, "length = %u", this->PasswordLength);
        }
        if (this->RedirFlags & LB_DONTSTOREUSERNAME) {
            LOG(level, "LB_DONTSTOREUSERNAME,");
        }
        if (this->RedirFlags & LB_SMARTCARD_LOGON) {
            LOG(level, "LB_SMARTCARD_LOGON,");
        }
        if (this->RedirFlags & LB_NOREDIRECT) {
            LOG(level, "LB_NOREDIRECT,");
        }
        if (this->RedirFlags & LB_TARGET_FQDN) {
            LOG(level, "LB_TARGET_FQDN,");
            LOG(level, "length = %u", this->TargetFQDNLength);
            hexdump_c(this->TargetFQDN, this->TargetFQDNLength);
        }
        if (this->RedirFlags & LB_TARGET_NETBIOS_NAME) {
            LOG(level, "LB_TARGET_NETBIOS_NAME,");
            LOG(level, "length = %u", this->TargetNetBiosNameLength);
            hexdump_c(this->TargetNetBiosName, this->TargetNetBiosNameLength);
        }
        if (this->RedirFlags & LB_TARGET_NET_ADDRESSES) {
            LOG(level, "LB_TARGET_NET_ADDRESSES,");
            LOG(level, "length = %u", this->TargetNetAddressesLength);
            hexdump_c(this->TargetNetAddresses, this->TargetNetAddressesLength);
        }
        if (this->RedirFlags & LB_CLIENT_TSV_URL) {
            LOG(level, "LB_CLIENT_TSV_URL,");
            LOG(level, "length = %u", this->TsvUrlLength);
            hexdump_c(this->TsvUrl, this->TsvUrlLength);
        }
        if (this->RedirFlags & LB_SERVER_TSV_CAPABLE) {
            LOG(level, "LB_SERVER_TSV_CAPABLE,");
        }
        LOG(level, "%s", "}");
    }
    void export_to_redirection_info(RedirectionInfo & rinfo) {

        rinfo.valid = !(this->Noredirect());
        rinfo.dont_store_username = this->DontStoreUsername();
        rinfo.server_tsv_capable = this->ServerTsvCapable();
        rinfo.smart_card_logon = this->SmartcardLogon();
        rinfo.session_id = this->SessionID;
        if (this->TargetNetAddressLength > 0) {
            UTF16toUTF8(this->TargetNetAddress, this->TargetNetAddressLength / 2,
                        rinfo.host, sizeof(rinfo.host));
            rinfo.host_is_fqdn = false;
        }
        else if (this->TargetFQDNLength > 0) {
            UTF16toUTF8(this->TargetFQDN, this->TargetFQDNLength / 2,
                        rinfo.host, sizeof(rinfo.host));
            rinfo.host_is_fqdn = true;
        }
        else if (this->TargetNetAddressesLength > 0) {
            LOG(LOG_ERR, "ServerRedirection: Unsupported TargetNetAddresses List");
            throw Error(ERR_RDP_UNSUPPORTED);
        }
        if (this->UserNameLength > 0) {
            UTF16toUTF8(this->UserName, this->UserNameLength / 2,
                        rinfo.username, sizeof(rinfo.username));
        }
        if (this->PasswordLength > 0) {
            UTF16toUTF8(this->Password, this->PasswordLength / 2,
                        rinfo.password, sizeof(rinfo.password));
        }
        if (this->DomainLength > 0) {
            UTF16toUTF8(this->Domain, this->DomainLength / 2,
                        rinfo.domain, sizeof(rinfo.domain));
        }
        if (this->UseLoadBalanceInfo()) {
            memcpy(rinfo.lb_info, this->LoadBalanceInfo,
                   this->LoadBalanceInfoLength);
            rinfo.lb_info_length = this->LoadBalanceInfoLength;
        }
    }

    void import_from_redirection_info(RedirectionInfo & rinfo) {
        this->Flags = SEC_REDIRECTION_PKT;
        this->RedirFlags = 0;
        if (!rinfo.valid) {
            this->RedirFlags |= LB_NOREDIRECT;
        }
        if (rinfo.dont_store_username) {
            this->RedirFlags |= LB_DONTSTOREUSERNAME;
        }
        if (rinfo.server_tsv_capable) {
            this->RedirFlags |= LB_SERVER_TSV_CAPABLE;
        }
        if (rinfo.smart_card_logon) {
            this->RedirFlags |= LB_SMARTCARD_LOGON;
        }
        this->SessionID = rinfo.session_id;
        size_t field_length = 0;
        size_t utf8_len = 0;
        if (rinfo.host[0] != 0) {
            utf8_len = UTF8Len(rinfo.host);
            std::string rinfo_host(char_ptr_cast(rinfo.host));
            if (rinfo.host_is_fqdn) {
                field_length = UTF8toUTF16(rinfo_host, this->TargetFQDN, utf8_len * 2);
                this->TargetFQDNLength = field_length;
                this->RedirFlags |= LB_TARGET_FQDN;
            } else {
                field_length = UTF8toUTF16(rinfo_host, this->TargetNetAddress,
                                           utf8_len * 2);
                this->TargetNetAddressLength = field_length;
                this->RedirFlags |= LB_TARGET_NET_ADDRESS;
            }
        }
        if (rinfo.username[0] != 0) {
            utf8_len = UTF8Len(rinfo.username);
            std::string rinfo_username(char_ptr_cast(rinfo.username));
            field_length = UTF8toUTF16(rinfo_username, this->UserName,
                                       utf8_len * 2);
            this->UserNameLength = field_length;
            this->RedirFlags |= LB_USERNAME;
        }
        if (rinfo.password[0] != 0) {
            utf8_len = UTF8Len(rinfo.password);
            std::string rinfo_password(char_ptr_cast(rinfo.password));
            field_length = UTF8toUTF16(rinfo_password, this->Password, utf8_len * 2);
            this->PasswordLength = field_length;
            this->RedirFlags |= LB_PASSWORD;
        }
        if (rinfo.domain[0] != 0) {
            utf8_len = UTF8Len(rinfo.domain);
            std::string rinfo_domain(char_ptr_cast(rinfo.domain));
            field_length = UTF8toUTF16(rinfo_domain, this->Domain, utf8_len * 2);
            this->DomainLength = field_length;
            this->RedirFlags |= LB_DOMAIN;
        }
        if (rinfo.lb_info_length > 0) {
            memcpy(this->LoadBalanceInfo, rinfo.lb_info, rinfo.lb_info_length);
            this->LoadBalanceInfoLength = rinfo.lb_info_length;
            this->RedirFlags |= LB_LOAD_BALANCE_INFO;
        }
    }

};

