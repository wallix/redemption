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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/


#pragma once

#include "core/error.hpp"
#include "utils/utf.hpp"
#include "utils/sugar/buf_maker.hpp"
#include "utils/sugar/cast.hpp"

#include <cstring>


// TODO: CGR. This array here should be replaced by a plain std::vector<uint8_t>
class Array
{
    enum {
        AUTOSIZE = 65536
    };

    BufMaker<AUTOSIZE> buf_maker;
    array_view_u8 avbuf;

public:
    explicit Array(size_t size = AUTOSIZE)
      : avbuf(this->buf_maker.dyn_array(size))
    {}

public:
    Array(Array const &) = delete;
    Array& operator=(Array const &) = delete;

    size_t size() const {
        return this->avbuf.size();
    }

    uint8_t * get_data() {
        return this->avbuf.data();
    }

    uint8_t const * get_data() const {
        return this->avbuf.data();
    }

    array_view_const_u8 av() const {
        return this->avbuf;
    }

    array_view_u8 av() {
        return this->avbuf;
    }

    void copy(Array const& other) {
        this->init(other.size());
        memcpy(this->get_data(), other.get_data(), this->size());
    }

    // a default buffer of 65536 bytes is allocated automatically, we will only allocate dynamic memory if we need more.
    void init(size_t v) {
        this->avbuf = this->buf_maker.dyn_array(v);
    }

    void copy(const_bytes_view source, uint32_t offset = 0) {
        assert(this->size() >= source.size() + offset);
        if (source.size()) {
            memcpy(this->avbuf.data() + offset, source.data(), source.size());
        }
    }
};


enum SecIdFlag {
    SEC_WINNT_AUTH_IDENTITY_ANSI = 0x1,
    SEC_WINNT_AUTH_IDENTITY_UNICODE = 0x2
};

struct SEC_WINNT_AUTH_IDENTITY
{
    // kerberos only
    //@{
    char princname[256];
    char princpass[256];
    //@}
    // ntlm only
    //@{
    Array User;
    Array Domain;
    Array Password;
    //@}

    SEC_WINNT_AUTH_IDENTITY()
        : User(0)
        , Domain(0)
        , Password(0)
    {
        this->princname[0] = 0;
        this->princpass[0] = 0;
    }

    void SetUserFromUtf8(const uint8_t * user)
    {
        this->copyFromUtf8(this->User, user);
    }

    void SetDomainFromUtf8(const uint8_t * domain)
    {
        this->copyFromUtf8(this->Domain, domain);
    }

    void SetPasswordFromUtf8(const uint8_t * password)
    {
        this->copyFromUtf8(this->Password, password);
    }

    void SetKrbAuthIdentity(const uint8_t * user, const uint8_t * pass)
    {
        auto copy = [](char (&arr)[256], uint8_t const* data){
            if (data) {
                const char * p = char_ptr_cast(data);
                const size_t length = p ? strnlen(p, 255) : 0;
                memcpy(arr, data, length);
                arr[length] = 0;
            }
        };

        copy(this->princname, user);
        copy(this->princpass, pass);
    }

    void clear()
    {
        this->User.init(0);
        this->Domain.init(0);
        this->Password.init(0);
    }

    void CopyAuthIdentity(SEC_WINNT_AUTH_IDENTITY const& src)
    {
        this->User.copy(src.User);
        this->Domain.copy(src.Domain);
        this->Password.copy(src.Password);
    }

private:
    static void copyFromUtf8(Array& arr, uint8_t const* data)
    {
        if (data) {
            size_t user_len = UTF8Len(data);
            arr.init(user_len * 2);
            UTF8toUTF16({data, strlen(char_ptr_cast(data))}, arr.get_data(), user_len * 2);
        }
        else {
            arr.init(0);
        }
    }
};


enum SEC_STATUS {
    SEC_E_OK = 0x00000000,
    SEC_E_INSUFFICIENT_MEMORY = 0x80090300,
    SEC_E_INVALID_HANDLE = 0x80090301,
    SEC_E_UNSUPPORTED_FUNCTION = 0x80090302,
    SEC_E_TARGET_UNKNOWN = 0x80090303,
    SEC_E_INTERNAL_ERROR = 0x80090304,
    SEC_E_SECPKG_NOT_FOUND = 0x80090305,
    SEC_E_NOT_OWNER = 0x80090306,
    SEC_E_CANNOT_INSTALL = 0x80090307,
    SEC_E_INVALID_TOKEN = 0x80090308,
    SEC_E_CANNOT_PACK = 0x80090309,
    SEC_E_QOP_NOT_SUPPORTED = 0x8009030A,
    SEC_E_NO_IMPERSONATION = 0x8009030B,
    SEC_E_LOGON_DENIED = 0x8009030C,
    SEC_E_UNKNOWN_CREDENTIALS = 0x8009030D,
    SEC_E_NO_CREDENTIALS = 0x8009030E,
    SEC_E_MESSAGE_ALTERED = 0x8009030F,
    SEC_E_OUT_OF_SEQUENCE = 0x80090310,
    SEC_E_NO_AUTHENTICATING_AUTHORITY = 0x80090311,
    SEC_E_BAD_PKGID = 0x80090316,
    SEC_E_CONTEXT_EXPIRED = 0x80090317,
    SEC_E_INCOMPLETE_MESSAGE = 0x80090318,
    SEC_E_INCOMPLETE_CREDENTIALS = 0x80090320,
    SEC_E_BUFFER_TOO_SMALL = 0x80090321,
    SEC_E_WRONG_PRINCIPAL = 0x80090322,
    SEC_E_TIME_SKEW = 0x80090324,
    SEC_E_UNTRUSTED_ROOT = 0x80090325,
    SEC_E_ILLEGAL_MESSAGE = 0x80090326,
    SEC_E_CERT_UNKNOWN = 0x80090327,
    SEC_E_CERT_EXPIRED = 0x80090328,
    SEC_E_ENCRYPT_FAILURE = 0x80090329,
    SEC_E_DECRYPT_FAILURE = 0x80090330,
    SEC_E_ALGORITHM_MISMATCH = 0x80090331,
    SEC_E_SECURITY_QOS_FAILED = 0x80090332,
    SEC_E_UNFINISHED_CONTEXT_DELETED = 0x80090333,
    SEC_E_NO_TGT_REPLY = 0x80090334,
    SEC_E_NO_IP_ADDRESSES = 0x80090335,
    SEC_E_WRONG_CREDENTIAL_HANDLE = 0x80090336,
    SEC_E_CRYPTO_SYSTEM_INVALID = 0x80090337,
    SEC_E_MAX_REFERRALS_EXCEEDED = 0x80090338,
    SEC_E_MUST_BE_KDC = 0x80090339,
    SEC_E_STRONG_CRYPTO_NOT_SUPPORTED = 0x8009033A,
    SEC_E_TOO_MANY_PRINCIPALS = 0x8009033B,
    SEC_E_NO_PA_DATA = 0x8009033C,
    SEC_E_PKINIT_NAME_MISMATCH = 0x8009033D,
    SEC_E_SMARTCARD_LOGON_REQUIRED = 0x8009033E,
    SEC_E_SHUTDOWN_IN_PROGRESS = 0x8009033F,
    SEC_E_KDC_INVALID_REQUEST = 0x80090340,
    SEC_E_KDC_UNABLE_TO_REFER = 0x80090341,
    SEC_E_KDC_UNKNOWN_ETYPE = 0x80090342,
    SEC_E_UNSUPPORTED_PREAUTH = 0x80090343,
    SEC_E_DELEGATION_REQUIRED = 0x80090345,
    SEC_E_BAD_BINDINGS = 0x80090346,
    SEC_E_MULTIPLE_ACCOUNTS = 0x80090347,
    SEC_E_NO_KERB_KEY = 0x80090348,
    SEC_E_CERT_WRONG_USAGE = 0x80090349,
    SEC_E_DOWNGRADE_DETECTED = 0x80090350,
    SEC_E_SMARTCARD_CERT_REVOKED = 0x80090351,
    SEC_E_ISSUING_CA_UNTRUSTED = 0x80090352,
    SEC_E_REVOCATION_OFFLINE_C = 0x80090353,
    SEC_E_PKINIT_CLIENT_FAILURE = 0x80090354,
    SEC_E_SMARTCARD_CERT_EXPIRED = 0x80090355,
    SEC_E_NO_S4U_PROT_SUPPORT = 0x80090356,
    SEC_E_CROSSREALM_DELEGATION_FAILURE = 0x80090357,
    SEC_E_REVOCATION_OFFLINE_KDC = 0x80090358,
    SEC_E_ISSUING_CA_UNTRUSTED_KDC = 0x80090359,
    SEC_E_KDC_CERT_EXPIRED = 0x8009035A,
    SEC_E_KDC_CERT_REVOKED = 0x8009035B,
    SEC_E_INVALID_PARAMETER = 0x8009035D,
    SEC_E_DELEGATION_POLICY = 0x8009035E,
    SEC_E_POLICY_NLTM_ONLY = 0x8009035F,
    SEC_E_NO_CONTEXT = 0x80090361,
    SEC_E_PKU2U_CERT_FAILURE = 0x80090362,
    SEC_E_MUTUAL_AUTH_FAILED = 0x80090363,

    SEC_I_CONTINUE_NEEDED = 0x00090312,
    SEC_I_COMPLETE_NEEDED = 0x00090313,
    SEC_I_COMPLETE_AND_CONTINUE = 0x00090314,
    SEC_I_LOCAL_LOGON = 0x00090315,
    SEC_I_CONTEXT_EXPIRED = 0x00090317,
    SEC_I_INCOMPLETE_CREDENTIALS = 0x00090320,
    SEC_I_RENEGOTIATE = 0x00090321,
    SEC_I_NO_LSA_CONTEXT = 0x00090323,
    SEC_I_SIGNATURE_NEEDED = 0x0009035C,
    SEC_I_NO_RENEGOTIATION = 0x00090360
};

/* InitializeSecurityContext Flags */
enum ISC_REQ {
    ISC_REQ_DELEGATE = 0x00000001,
    ISC_REQ_MUTUAL_AUTH = 0x00000002,
    ISC_REQ_REPLAY_DETECT = 0x00000004,
    ISC_REQ_SEQUENCE_DETECT = 0x00000008,
    ISC_REQ_CONFIDENTIALITY = 0x00000010,
    ISC_REQ_USE_SESSION_KEY = 0x00000020,
    ISC_REQ_PROMPT_FOR_CREDS = 0x00000040,
    ISC_REQ_USE_SUPPLIED_CREDS = 0x00000080,
    ISC_REQ_ALLOCATE_MEMORY = 0x00000100,
    ISC_REQ_USE_DCE_STYLE = 0x00000200,
    ISC_REQ_DATAGRAM = 0x00000400,
    ISC_REQ_CONNECTION = 0x00000800,
    ISC_REQ_CALL_LEVEL = 0x00001000,
    ISC_REQ_FRAGMENT_SUPPLIED = 0x00002000,
    ISC_REQ_EXTENDED_ERROR = 0x00004000,
    ISC_REQ_STREAM = 0x00008000,
    ISC_REQ_INTEGRITY = 0x00010000,
    ISC_REQ_IDENTIFY = 0x00020000,
    ISC_REQ_NULL_SESSION = 0x00040000,
    ISC_REQ_MANUAL_CRED_VALIDATION = 0x00080000,
    ISC_REQ_RESERVED1 = 0x00100000,
    ISC_REQ_FRAGMENT_TO_FIT = 0x00200000,
    ISC_REQ_FORWARD_CREDENTIALS = 0x00400000,
    ISC_REQ_NO_INTEGRITY = 0x00800000,
    ISC_REQ_USE_HTTP_STYLE = 0x01000000
};

/* AcceptSecurityContext Flags */
enum ASC_REQ {
    ASC_REQ_DELEGATE = 0x00000001,
    ASC_REQ_MUTUAL_AUTH = 0x00000002,
    ASC_REQ_REPLAY_DETECT = 0x00000004,
    ASC_REQ_SEQUENCE_DETECT = 0x00000008,
    ASC_REQ_CONFIDENTIALITY = 0x00000010,
    ASC_REQ_USE_SESSION_KEY = 0x00000020,
    ASC_REQ_ALLOCATE_MEMORY = 0x00000100,
    ASC_REQ_USE_DCE_STYLE = 0x00000200,
    ASC_REQ_DATAGRAM = 0x00000400,
    ASC_REQ_CONNECTION = 0x00000800,
    ASC_REQ_CALL_LEVEL = 0x00001000,
    ASC_REQ_EXTENDED_ERROR = 0x00008000,
    ASC_REQ_STREAM = 0x00010000,
    ASC_REQ_INTEGRITY = 0x00020000,
    ASC_REQ_LICENSING = 0x00040000,
    ASC_REQ_IDENTIFY = 0x00080000,
    ASC_REQ_ALLOW_NULL_SESSION = 0x00100000,
    ASC_REQ_ALLOW_NON_USER_LOGONS = 0x00200000,
    ASC_REQ_ALLOW_CONTEXT_REPLAY = 0x00400000,
    ASC_REQ_FRAGMENT_TO_FIT = 0x00800000,
    ASC_REQ_FRAGMENT_SUPPLIED = 0x00002000,
    ASC_REQ_NO_TOKEN = 0x01000000,
    ASC_REQ_PROXY_BINDINGS = 0x04000000,
    ASC_REQ_ALLOW_MISSING_BINDINGS = 0x10000000
};

enum ASC_RET {
    ASC_RET_DELEGATE = 0x00000001,
    ASC_RET_MUTUAL_AUTH = 0x00000002,
    ASC_RET_REPLAY_DETECT = 0x00000004,
    ASC_RET_SEQUENCE_DETECT = 0x00000008,
    ASC_RET_CONFIDENTIALITY = 0x00000010,
    ASC_RET_USE_SESSION_KEY = 0x00000020,
    ASC_RET_ALLOCATED_MEMORY = 0x00000100,
    ASC_RET_USED_DCE_STYLE = 0x00000200,
    ASC_RET_DATAGRAM = 0x00000400,
    ASC_RET_CONNECTION = 0x00000800,
    ASC_RET_CALL_LEVEL = 0x00002000,
    ASC_RET_THIRD_LEG_FAILED = 0x00004000,
    ASC_RET_EXTENDED_ERROR = 0x00008000,
    ASC_RET_STREAM = 0x00010000,
    ASC_RET_INTEGRITY = 0x00020000,
    ASC_RET_LICENSING = 0x00040000,
    ASC_RET_IDENTIFY = 0x00080000,
    ASC_RET_NULL_SESSION = 0x00100000,
    ASC_RET_ALLOW_NON_USER_LOGONS = 0x00200000,
    ASC_RET_FRAGMENT_ONLY = 0x00800000,
    ASC_RET_NO_TOKEN = 0x01000000,
    ASC_RET_NO_PROXY_BINDINGS = 0x04000000,
    ASC_RET_MISSING_BINDINGS = 0x10000000
};

enum CredentialUse {
    SECPKG_CRED_INBOUND = 0x00000001,
    SECPKG_CRED_OUTBOUND = 0x00000002,
    SECPKG_CRED_BOTH = 0x00000003,
    SECPKG_CRED_AUTOLOGON_RESTRICTED = 0x00000010,
    SECPKG_CRED_PROCESS_POLICY_ONLY = 0x00000020
};


// using SEC_GET_KEY_FN = void (*)(void* Arg, void* Principal, uint32_t KeyVer, void** Key, SEC_STATUS* pStatus);
// using HANDLE = void*;
// using PHANDLE = void*;
// using LPHANDLE = void*;

struct SecurityFunctionTable
{
    virtual ~SecurityFunctionTable() = default;

    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
    virtual SEC_STATUS AcquireCredentialsHandle(const char * pszPrincipal,
                                                Array * pvLogonID,
                                                SEC_WINNT_AUTH_IDENTITY const* pAuthData) = 0;

    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;
    virtual SEC_STATUS InitializeSecurityContext(array_view_const_char pszTargetName,
                                                 array_view_const_u8 input_buffer,
                                                 Array& output_buffer) = 0;

    // GSS_Accept_sec_context
    // ACCEPT_SECURITY_CONTEXT AcceptSecurityContext;
    virtual SEC_STATUS AcceptSecurityContext(array_view_const_u8 input_buffer,
                                             Array& output_buffer) = 0;

    // GSS_Wrap
    // ENCRYPT_MESSAGE EncryptMessage;
    virtual SEC_STATUS EncryptMessage(array_view_const_u8 data_in,
                                      Array& data_out,
                                      unsigned long messageSeqNo) = 0;

    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;
    virtual SEC_STATUS DecryptMessage(array_view_const_u8 data_in,
                                      Array& data_out,
                                      unsigned long messageSeqNo) = 0;
};

struct UnimplementedSecurityFunctionTable : SecurityFunctionTable
{
    SEC_STATUS AcquireCredentialsHandle(
        const char * /*pszPrincipal*/,
        Array * /*pvLogonID*/,
        SEC_WINNT_AUTH_IDENTITY const* /*pAuthData*/
    ) override
    {
        return SEC_E_UNSUPPORTED_FUNCTION;
    }

    SEC_STATUS InitializeSecurityContext(
        array_view_const_char /*pszTargetName*/,
        array_view_const_u8 /*input_buffer*/,
        Array& /*output_buffer*/
    ) override
    {
        return SEC_E_UNSUPPORTED_FUNCTION;
    }

    SEC_STATUS AcceptSecurityContext(
        array_view_const_u8 /*input_buffer*/,
        Array& /*output_buffer*/
    ) override
    {
        return SEC_E_UNSUPPORTED_FUNCTION;
    }

    SEC_STATUS EncryptMessage(
        array_view_const_u8 /*data_in*/, Array& /*data_out*/,
        unsigned long /*messageSeqNo*/
    ) override
    {
        return SEC_E_UNSUPPORTED_FUNCTION;
    }

    SEC_STATUS DecryptMessage(
        array_view_const_u8 /*data_in*/, Array& /*data_out*/,
        unsigned long /*messageSeqNo*/
    ) override
    {
        return SEC_E_UNSUPPORTED_FUNCTION;
    }
};

enum SecInterface {
    NTLM_Interface,
    Kerberos_Interface,
    //SChannel_Interface
};
