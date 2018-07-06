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

#include "utils/log.hpp"
#include "core/error.hpp"
#include "utils/utf.hpp"

#include <new>
#include <cstdio>


// TODO: CGR. This array here should be replaced by a plain std::vector<uint8_t>
class Array {
    enum {
        AUTOSIZE = 65536
    };

    uint8_t* data{nullptr};
    size_t capacity{0};
    private:
    uint8_t autobuffer[AUTOSIZE]{};

    public:
    explicit Array(size_t size = AUTOSIZE)

    {
        this->data = this->autobuffer;
        this->init(size);
    }

    ~Array() {
        // <this->data> is allocated dynamically.
        if (this->capacity > AUTOSIZE) {
            delete [] this->data;
        }
    }

private:
    Array(Array const &) /* = delete*/;
    Array& operator=(Array const &) /* = delete*/;

public:
    size_t size() const {
        return this->capacity;
    }

    uint8_t * get_data() const {
        return this->data;
    }

    void copy(Array const& other) {
        this->init(other.size());
        memcpy(this->get_data(), other.get_data(), this->size());
    }

    // a default buffer of 65536 bytes is allocated automatically, we will only allocate dynamic memory if we need more.
    void init(size_t v) {
        if (v != this->capacity) {
            // <this->data> is allocated dynamically.
            if (this->capacity > AUTOSIZE){
                delete [] this->data;
            }

            this->capacity = v;
            if (v > AUTOSIZE){
                this->data = new(std::nothrow) uint8_t[v];
                if (!this->data) {
                    this->capacity = 0;
                    LOG(LOG_ERR, "failed to allocate buffer : size asked = %d\n", static_cast<int>(v));
                    throw Error(ERR_STREAM_MEMORY_ALLOCATION_ERROR);
                }
            }
            else {
                this->data = &(this->autobuffer[0]);
            }
        }
    }

    void copy(const uint8_t * source, size_t size, uint32_t offset = 0) {
        assert(this->capacity >= size + offset);
        memcpy(this->data + offset, source, size);
    }
};

#define NTLMSP_NAME "NTLM"
#define SECBUFFER_VERSION 0

/* Buffer Types */
enum buffer_type {
    SECBUFFER_EMPTY = 0,
    SECBUFFER_DATA = 1,
    SECBUFFER_TOKEN = 2,
    SECBUFFER_PKG_PARAMS = 3,
    SECBUFFER_MISSING = 4,
    SECBUFFER_EXTRA = 5,
    SECBUFFER_STREAM_TRAILER = 6,
    SECBUFFER_STREAM_HEADER = 7,
    SECBUFFER_NEGOTIATION_INFO = 8,
    SECBUFFER_PADDING = 9,
    SECBUFFER_STREAM = 10,
    SECBUFFER_MECHLIST = 11,
    SECBUFFER_MECHLIST_SIGNATURE = 12,
    SECBUFFER_TARGET = 13,
    SECBUFFER_CHANNEL_BINDINGS = 14,
    SECBUFFER_CHANGE_PASS_RESPONSE = 15,
    SECBUFFER_TARGET_HOST = 16,
    SECBUFFER_ALERT = 17
};

struct SecBuffer {
    unsigned long BufferType;
    Array         Buffer;

    void setzero() {
        this->Buffer.init(0);
        this->BufferType = SECBUFFER_EMPTY;
    }
};

using PSecBuffer = SecBuffer *;
struct SecBufferDesc
{
    unsigned long ulVersion;
    unsigned long cBuffers;
    SecBuffer *   pBuffers;

    PSecBuffer FindSecBuffer(buffer_type BufferType)
    {
        unsigned long index;
        PSecBuffer pSecBuffer = nullptr;

        for (index = 0; index < this->cBuffers; index++) {
            if (this->pBuffers[index].BufferType == BufferType) {
                pSecBuffer = &(this->pBuffers[index]);
                break;
            }
        }

        return pSecBuffer;
    }

};

struct TimeStamp {
    uint32_t LowPart;
    int32_t HighPart;
};

struct SecPkgInfo {
    uint32_t fCapabilities;
    uint16_t wVersion;
    uint16_t wRPCID;
    uint32_t cbMaxToken;
    const char* Name;
    const char* Comment;
};


struct SEC_CHANNEL_BINDINGS
{
    uint32_t dwInitiatorAddrType;
    uint32_t cbInitiatorLength;
    uint32_t dwInitiatorOffset;
    uint32_t dwAcceptorAddrType;
    uint32_t cbAcceptorLength;
    uint32_t dwAcceptorOffset;
    uint32_t cbApplicationDataLength;
    uint32_t dwApplicationDataOffset;
};
struct SecPkgContext_Bindings
{
        uint32_t BindingsLength;
        SEC_CHANNEL_BINDINGS* Bindings;
};

struct SecPkgContext_Sizes
{
    uint32_t cbMaxToken;
    uint32_t cbMaxSignature;
    uint32_t cbBlockSize;
    uint32_t cbSecurityTrailer;
};

enum SecIdFlag {
    SEC_WINNT_AUTH_IDENTITY_ANSI = 0x1,
    SEC_WINNT_AUTH_IDENTITY_UNICODE = 0x2
};

struct SEC_WINNT_AUTH_IDENTITY
{
    char princname[256];
    char princpass[256];
    Array User;
    Array Domain;
    Array Password;
    uint32_t Flags{0};

    SEC_WINNT_AUTH_IDENTITY()
        : User(0)
        , Domain(0)
        , Password(0)

    {
        this->princname[0] = 0;
        this->princpass[0] = 0;
    }

    void SetUserFromUtf8(const uint8_t * user) {
        if (user) {
            size_t user_len = UTF8Len(user);
            this->User.init(user_len * 2);
            UTF8toUTF16(user, this->User.get_data(), user_len * 2);
        }
        else {
            this->User.init(0);
        }
    }

    void SetDomainFromUtf8(const uint8_t * domain) {
        if (domain) {
            size_t domain_len = UTF8Len(domain);
            this->Domain.init(domain_len * 2);
            UTF8toUTF16(domain, this->Domain.get_data(), domain_len * 2);
        }
        else {
            this->Domain.init(0);
        }
    }

    void SetPasswordFromUtf8(const uint8_t * password) {
        if (password) {
            size_t password_len = UTF8Len(password);
            this->Password.init(password_len * 2);
            UTF8toUTF16(password, this->Password.get_data(), password_len * 2);
        }
        else {
            this->Password.init(0);
        }
    }
    void SetKrbAuthIdentity(const uint8_t * user, const uint8_t * pass) {
        if (user) {
            const char * p = char_ptr_cast(user);
            size_t length = 0;
            if (p) {
                length = strlen(p);
                if (length > 256) {
                    length = 255;
                }
            }
            memcpy(this->princname, user, length);
            this->princname[length] = 0;
        }
        if (pass) {
            const char * p = char_ptr_cast(pass);
            size_t length = 0;
            if (p) {
                length = strlen(p);
                if (length > 256) {
                    length = 255;
                }
            }
            memcpy(this->princpass, pass, length);
            this->princpass[length] = 0;
        }
    }

    void SetAuthIdentityFromUtf8(const uint8_t * user, const uint8_t * domain,
                                const uint8_t * password) {
       this->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
       if (user) {
           size_t user_len = UTF8Len(user);
           this->User.init(user_len * 2);
           UTF8toUTF16(user, this->User.get_data(), user_len * 2);
       }
       else {
           this->User.init(0);
       }

       if (domain) {
           size_t domain_len = UTF8Len(domain);
           this->Domain.init(domain_len * 2);
           UTF8toUTF16(domain, this->Domain.get_data(), domain_len * 2);
       }
       else {
           this->Domain.init(0);
       }

       if (password) {
           size_t password_len = UTF8Len(password);
           this->Password.init(password_len * 2);
           UTF8toUTF16(password, this->Password.get_data(), password_len * 2);
       }
       else {
           this->Password.init(0);
       }

    }

    void clear() {
        this->User.init(0);
        this->Domain.init(0);
        this->Password.init(0);
        this->Flags = 0;
    }

    void CopyAuthIdentity(SEC_WINNT_AUTH_IDENTITY & src) {
        this->User.init(src.User.size());
        this->User.copy(src.User.get_data(), src.User.size());
        this->Domain.init(src.Domain.size());
        this->Domain.copy(src.Domain.get_data(), src.Domain.size());
        this->Password.init(src.Password.size());
        this->Password.copy(src.Password.get_data(), src.Password.size());
        this->Flags = src.Flags;
    }
};

struct CREDENTIALS {
    uint32_t flags;
    SEC_WINNT_AUTH_IDENTITY identity;
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

enum SecDrep {
    SECURITY_NATIVE_DREP = 0x00000010,
    SECURITY_NETWORK_DREP = 0x00000000
};

enum CredentialUse {
    SECPKG_CRED_INBOUND = 0x00000001,
    SECPKG_CRED_OUTBOUND = 0x00000002,
    SECPKG_CRED_BOTH = 0x00000003,
    SECPKG_CRED_AUTOLOGON_RESTRICTED = 0x00000010,
    SECPKG_CRED_PROCESS_POLICY_ONLY = 0x00000020
};

enum SecPkg_Att {
/* Security Context Attributes */

    SECPKG_ATTR_SIZES = 0,
    SECPKG_ATTR_NAMES = 1,
    SECPKG_ATTR_LIFESPAN = 2,
    SECPKG_ATTR_DCE_INFO = 3,
    SECPKG_ATTR_STREAM_SIZES = 4,
    SECPKG_ATTR_KEY_INFO = 5,
    SECPKG_ATTR_AUTHORITY = 6,
    SECPKG_ATTR_PROTO_INFO = 7,
    SECPKG_ATTR_PASSWORD_EXPIRY = 8,
    SECPKG_ATTR_SESSION_KEY = 9,
    SECPKG_ATTR_PACKAGE_INFO = 10,
    SECPKG_ATTR_USER_FLAGS = 11,
    SECPKG_ATTR_NEGOTIATION_INFO = 12,
    SECPKG_ATTR_NATIVE_NAMES = 13,
    SECPKG_ATTR_FLAGS = 14,
    SECPKG_ATTR_USE_VALIDATED = 15,
    SECPKG_ATTR_CREDENTIAL_NAME = 16,
    SECPKG_ATTR_TARGET_INFORMATION = 17,
    SECPKG_ATTR_ACCESS_TOKEN = 18,
    SECPKG_ATTR_TARGET = 19,
    SECPKG_ATTR_AUTHENTICATION_ID = 20,
    SECPKG_ATTR_LOGOFF_TIME = 21,
    SECPKG_ATTR_NEGO_KEYS = 22,
    SECPKG_ATTR_PROMPTING_NEEDED = 24,
    SECPKG_ATTR_UNIQUE_BINDINGS = 25,
    SECPKG_ATTR_ENDPOINT_BINDINGS = 26,
    SECPKG_ATTR_CLIENT_SPECIFIED_TARGET = 27,
    SECPKG_ATTR_LAST_CLIENT_TOKEN_STATUS = 30,
    SECPKG_ATTR_NEGO_PKG_INFO = 31,
    SECPKG_ATTR_NEGO_STATUS = 32,
    SECPKG_ATTR_CONTEXT_DELETED = 33
};


using SEC_GET_KEY_FN = void (*)(void* Arg, void* Principal, uint32_t KeyVer, void** Key, SEC_STATUS* pStatus);
typedef void* HANDLE, *PHANDLE, *LPHANDLE;

struct SecurityFunctionTable
{
    virtual ~SecurityFunctionTable() = default;
    uint32_t dwVersion;

    // ENUMERATE_SECURITY_PACKAGES_FN EnumerateSecurityPackages;
    //virtual SEC_STATUS EnumerateSecurityPackages(unsigned long * pcPackages,
    //                                             SecPkgInfo ** ppPackageInfo) {
    //    // int index;
    //    // uint32_t cPackages = sizeof(SecPkgInfo_LIST) / sizeof(*(SecPkgInfo_LIST));
    //    // size_t size = sizeof(SecPkgInfo) * cPackages;
    //    return SEC_E_UNSUPPORTED_FUNCTION;
    //}

    // QUERY_CREDENTIALS_ATTRIBUTES_FN QueryCredentialsAttributes;
    //virtual SEC_STATUS QueryCredentialsAttributes(unsigned long ulAttribute,
    //                                              void* pBuffer) {
    //    return SEC_E_UNSUPPORTED_FUNCTION;
    //}

    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
    virtual SEC_STATUS AcquireCredentialsHandle(const char * pszPrincipal,
                                                unsigned long fCredentialUse,
                                                Array * pvLogonID,
                                                SEC_WINNT_AUTH_IDENTITY * pAuthData) {

        (void)pszPrincipal;
        (void)fCredentialUse;
        (void)pvLogonID;
        (void)pAuthData;
         return SEC_E_UNSUPPORTED_FUNCTION;
    }

    // void * Reserved2;

    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;
    virtual SEC_STATUS InitializeSecurityContext(char* pszTargetName,
                                                 unsigned long fContextReq,
                                                 SecBufferDesc * pInput,
                                                 unsigned long Reserved2,
                                                 SecBufferDesc * pOutput) {

        (void)pszTargetName;
        (void)fContextReq;
        (void)pInput;
        (void)Reserved2;
        (void)pOutput;
        return SEC_E_UNSUPPORTED_FUNCTION;
    }

    // GSS_Accept_sec_context
    // ACCEPT_SECURITY_CONTEXT AcceptSecurityContext;
    virtual SEC_STATUS AcceptSecurityContext(SecBufferDesc& pInput,
                                             unsigned long fContextReq,
                                             SecBufferDesc& pOutput) {
        (void)pInput;
        (void)fContextReq;
        (void)pOutput;
        return SEC_E_UNSUPPORTED_FUNCTION;
    }


    // GSS_Process_context_token ?
    // COMPLETE_AUTH_TOKEN CompleteAuthToken;
    virtual SEC_STATUS CompleteAuthToken(SecBufferDesc& Token) {
        (void)Token;
        return SEC_E_UNSUPPORTED_FUNCTION;
    }

    // GSS_Delete_sec_context
    // DELETE_SECURITY_CONTEXT DeleteSecurityContext;
    //virtual SEC_STATUS DeleteSecurityContext() {
    //    return SEC_E_UNSUPPORTED_FUNCTION;
    //}

    // APPLY_CONTROL_TOKEN ApplyControlToken;
    //virtual SEC_STATUS ApplyControlToken(SecBufferDesc * pInput) {
    //    return SEC_E_UNSUPPORTED_FUNCTION;
    //}

    // QUERY_CONTEXT_ATTRIBUTES QueryContextAttributes;
    virtual SecPkgContext_Sizes QueryContextSizes() {
        throw Error(ERR_SEC);
    }

    // IMPERSONATE_SECURITY_CONTEXT ImpersonateSecurityContext;
    virtual SEC_STATUS ImpersonateSecurityContext() {
        return SEC_E_UNSUPPORTED_FUNCTION;
    }

    // REVERT_SECURITY_CONTEXT RevertSecurityContext;
    virtual SEC_STATUS RevertSecurityContext() {
        return SEC_E_UNSUPPORTED_FUNCTION;
    }

    // MAKE_SIGNATURE MakeSignature;
    //virtual SEC_STATUS MakeSignature(unsigned long fQOP,
    //                                 SecBufferDesc * pMessage, unsigned long MessageSeqNo) {
    //    return SEC_E_UNSUPPORTED_FUNCTION;
    //}

    // VERIFY_SIGNATURE VerifySignature;
    //virtual SEC_STATUS VerifySignature(SecBufferDesc * pMessage,
    //                                    unsigned long MessageSeqNo, unsigned long * pfQOP) {
    //    return SEC_E_UNSUPPORTED_FUNCTION;
    //}

    // QUERY_SECURITY_PACKAGE_INFO QuerySecurityPackageInfo;
    virtual SecPkgInfo QuerySecurityPackageInfo()
    {
        throw Error(ERR_SEC);
        // return SecPkgInfo{};
    }

    // void* Reserved3;
    // void* Reserved4;

    // GSS_Export_sec_context
    // EXPORT_SECURITY_CONTEXT ExportSecurityContext;
    //virtual SEC_STATUS ExportSecurityContext(unsigned long fFlags,
    //                                         SecBuffer * pPackedContext, HANDLE* pToken) {
    //    return SEC_E_UNSUPPORTED_FUNCTION;
    //}

    // GSS_Import_sec_context
    // IMPORT_SECURITY_CONTEXT ImportSecurityContext;
    //virtual SEC_STATUS ImportSecurityContext(char* pszPackage, SecBuffer * pPackedContext,
    //                                         HANDLE pToken) {
    //    (void)pszPackage;
    //    (void)pPackedContext;
    //    (void)pToken;
    //    return SEC_E_UNSUPPORTED_FUNCTION;
    //}

    // GSS_Add_cred
    // ADD_CREDENTIALS AddCredentials;
    //virtual SEC_STATUS AddCredentials(char* pszPrincipal,
    //                                 char* pszPackage, uint32_t fCredentialUse,
    //                                 void* pAuthData, SEC_GET_KEY_FN pGetKeyFn,
    //                                 void* pvGetKeyArgument, TimeStamp * ptsExpiry) {
    //    return SEC_E_UNSUPPORTED_FUNCTION;
    //}

    // void* Reserved8;

    // QUERY_SECURITY_CONTEXT_TOKEN QuerySecurityContextToken;
    //virtual SEC_STATUS QuerySecurityContextToken(HANDLE* phToken) {
    //    return SEC_E_UNSUPPORTED_FUNCTION;
    //}

    // GSS_Wrap
    // ENCRYPT_MESSAGE EncryptMessage;
    virtual SEC_STATUS EncryptMessage(SecBufferDesc& Message, unsigned long MessageSeqNo) {
        (void)Message;
        (void)MessageSeqNo;
        return SEC_E_UNSUPPORTED_FUNCTION;
    }

    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;
    virtual SEC_STATUS DecryptMessage(SecBufferDesc& Message, unsigned long MessageSeqNo) {
        (void)Message;
        (void)MessageSeqNo;
        return SEC_E_UNSUPPORTED_FUNCTION;
    }

    // SET_CONTEXT_ATTRIBUTES SetContextAttributes;
    //virtual SEC_STATUS SetContextAttributes(unsigned long ulAttribute,
    //                                        void* pBuffer, unsigned long cbBuffer) {
    //    return SEC_E_UNSUPPORTED_FUNCTION;
    //}

};

enum SecInterface {
    NTLM_Interface,
    Kerberos_Interface,
    //SChannel_Interface
};
