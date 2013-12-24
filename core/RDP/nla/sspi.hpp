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

#ifndef _REDEMPTION_CORE_RDP_NLA_SSPI_HPP_
#define _REDEMPTION_CORE_RDP_NLA_SSPI_HPP_

#include <stdio.h>

struct SecBuffer {
    unsigned long cbBuffer;
    unsigned long BufferType;
    void *        pvBuffer;
};
struct SecBufferDesc
{
    unsigned long ulVersion;
    unsigned long cBuffers;
    SecBuffer *   pBuffers;
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
const char Ntlm_Name[] = "NTLM";
const char Ntlm_Comment[] = "NTLM Security Package";
const SecPkgInfo NTLM_SecPkgInfo = {
    0x00082B37,             // fCapabilities
    1,                      // wVersion
    0x000A,                 // wRPCID
    0x00000B48,             // cbMaxToken
    Ntlm_Name,              // Name
    Ntlm_Comment            // Comment
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


struct _SEC_WINNT_AUTH_IDENTITY
{
    uint16_t* User;
    uint32_t UserLength;
    uint16_t* Domain;
    uint32_t DomainLength;
    uint16_t* Password;
    uint32_t PasswordLength;
    uint32_t Flags;
};
typedef struct _SEC_WINNT_AUTH_IDENTITY SEC_WINNT_AUTH_IDENTITY;

typedef struct _SecHandle {
    unsigned long * dwLower;
    unsigned long * dwUpper;
} SecHandle, *PSecHandle;

typedef SecHandle CredHandle;
typedef PSecHandle PCredHandle;

typedef SecHandle CtxtHandle;
typedef PSecHandle PCtxtHandle;



const SecPkgInfo* SecPkgInfo_LIST[] = {
    // &NTLM_SecPkgInfoA,
    // &CREDSSP_SecPkgInfoA,
    // &SCHANNEL_SecPkgInfoA
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
    SEC_I_NO_RENEGOTIATION = 0x00090360,
};

typedef void (*SEC_GET_KEY_FN)(void* Arg, void* Principal, uint32_t KeyVer, void** Key, SEC_STATUS* pStatus);
typedef void* HANDLE, *PHANDLE, *LPHANDLE;

struct SecurityFunctionTable {

    virtual ~SecurityFunctionTable() {}
    uint32_t dwVersion;

    // ENUMERATE_SECURITY_PACKAGES_FN EnumerateSecurityPackages;
    virtual SEC_STATUS EnumerateSecurityPackages(unsigned long * pcPackages,
                                                 SecPkgInfo ** ppPackageInfo) {
        // int index;
        // uint32_t cPackages = sizeof(SecPkgInfo_LIST) / sizeof(*(SecPkgInfo_LIST));
        // size_t size = sizeof(SecPkgInfo) * cPackages;
        return SEC_E_OK;
    }

    // QUERY_CREDENTIALS_ATTRIBUTES_FN QueryCredentialsAttributes;
    virtual SEC_STATUS QueryCredentialsAttributes(SecHandle * phCredential,
                                                  unsigned long ulAttribute,
                                                  void* pBuffer) {
        return SEC_E_OK;
    }

    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
    virtual SEC_STATUS AcquireCredentialsHandle(char * pszPrincipal, char * pszPackage,
                                                unsigned long fCredentialUse, void* pvLogonID,
                                                void* pAuthData, SEC_GET_KEY_FN pGetKeyFn,
                                                void* pvGetKeyArgument, SecHandle * phCredential,
                                                TimeStamp * ptsExpiry) {
         return SEC_E_OK;
    }

    // FREE_CREDENTIALS_HANDLE_FN FreeCredentialsHandle;
    virtual SEC_STATUS FreeCredentialsHandle(SecHandle * phCredential) {
        return SEC_E_OK;
    }

    // void * Reserved2;

    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;
    virtual SEC_STATUS InitializeSecurityContext(SecHandle * phCredential, SecHandle * phContext,
                                                 char* pszTargetName, unsigned long fContextReq,
                                                 unsigned long Reserved1,
                                                 unsigned long TargetDataRep,
                                                 SecBufferDesc * pInput, unsigned long Reserved2,
                                                 SecHandle * phNewContext, SecBufferDesc * pOutput,
                                                 unsigned long * pfContextAttr,
                                                 TimeStamp * ptsExpiry) {
        return SEC_E_OK;
    }

    // ACCEPT_SECURITY_CONTEXT AcceptSecurityContext;
    virtual SEC_STATUS AcceptSecurityContext(SecHandle * phCredential, SecHandle * phContext,
                                             SecBufferDesc * pInput, unsigned long fContextReq,
                                             unsigned long TargetDataRep, SecHandle * phNewContext,
                                             SecBufferDesc * pOutput,
                                             unsigned long * pfContextAttr,
                                             TimeStamp * ptsTimeStamp) {
        return SEC_E_OK;
    }


    // COMPLETE_AUTH_TOKEN CompleteAuthToken;
    virtual SEC_STATUS CompleteAuthToken(SecHandle * phContext, SecBufferDesc * pToken) {
        return SEC_E_OK;
    }

    // DELETE_SECURITY_CONTEXT DeleteSecurityContext;
    virtual SEC_STATUS DeleteSecurityContext(SecHandle * phContext) {
        return SEC_E_OK;
    }

    // APPLY_CONTROL_TOKEN ApplyControlToken;
    virtual SEC_STATUS ApplyControlToken(SecHandle * phContext, SecBufferDesc * pInput) {
        return SEC_E_OK;
    }

    // QUERY_CONTEXT_ATTRIBUTES QueryContextAttributes;
    virtual SEC_STATUS QueryContextAttributes(SecHandle * phContext, unsigned long ulAttribute,
                                              void* pBuffer) {
        return SEC_E_OK;
    }

    // IMPERSONATE_SECURITY_CONTEXT ImpersonateSecurityContext;
    virtual SEC_STATUS ImpersonateSecurityContext(SecHandle * phContext) {
        return SEC_E_OK;
    }

    // REVERT_SECURITY_CONTEXT RevertSecurityContext;
    virtual SEC_STATUS RevertSecurityContext(SecHandle * phContext) {
        return SEC_E_OK;
    }

    // MAKE_SIGNATURE MakeSignature;
    virtual SEC_STATUS MakeSignature(SecHandle * phContext, unsigned long fQOP,
                                      SecBufferDesc * pMessage, unsigned long MessageSeqNo) {
        return SEC_E_OK;
    }

    // VERIFY_SIGNATURE VerifySignature;
    virtual SEC_STATUS VerifySignature(SecHandle * phContext, SecBufferDesc * pMessage,
                                        unsigned long MessageSeqNo, unsigned long * pfQOP) {
        return SEC_E_OK;
    }

    // FREE_CONTEXT_BUFFER FreeContextBuffer;
    virtual SEC_STATUS FreeContextBuffer(void* pvContextBuffer) {
        return SEC_E_OK;
    }

    // QUERY_SECURITY_PACKAGE_INFO QuerySecurityPackageInfo;
    virtual SEC_STATUS QuerySecurityPackageInfo(char* pszPackageName,
                                                   SecPkgInfo ** ppPackageInfo) {
        return SEC_E_OK;
    }

    // void* Reserved3;
    // void* Reserved4;

    // EXPORT_SECURITY_CONTEXT ExportSecurityContext;
    virtual SEC_STATUS ExportSecurityContext(SecHandle * phContext, unsigned long fFlags,
                                             SecBuffer * pPackedContext, HANDLE* pToken) {
        return SEC_E_OK;
    }

    // IMPORT_SECURITY_CONTEXT ImportSecurityContext;
    virtual SEC_STATUS ImportSecurityContext(char* pszPackage, SecBuffer * pPackedContext,
                                             HANDLE pToken, SecHandle * phContext) {
        return SEC_E_OK;
    }

    // ADD_CREDENTIALS AddCredentials;
    virtual SEC_STATUS AddCredentials(SecHandle * hCredentials, char* pszPrincipal,
                                     char* pszPackage, uint32_t fCredentialUse,
                                     void* pAuthData, SEC_GET_KEY_FN pGetKeyFn,
                                     void* pvGetKeyArgument, TimeStamp * ptsExpiry) {
        return SEC_E_OK;
    }

    // void* Reserved8;

    // QUERY_SECURITY_CONTEXT_TOKEN QuerySecurityContextToken;
    virtual SEC_STATUS QuerySecurityContextToken(SecHandle * phContext, HANDLE* phToken) {
        return SEC_E_OK;
    }

    // ENCRYPT_MESSAGE EncryptMessage;
    virtual SEC_STATUS EncryptMessage(SecHandle * phContext, unsigned long fQOP,
                                       SecBufferDesc * pMessage, unsigned long MessageSeqNo) {
        return SEC_E_OK;
    }

    // DECRYPT_MESSAGE DecryptMessage;
    virtual SEC_STATUS DecryptMessage(SecHandle * phContext, SecBufferDesc * pMessage,
                                       unsigned long MessageSeqNo, unsigned long * pfQOP) {
        return SEC_E_OK;
    }

    // SET_CONTEXT_ATTRIBUTES SetContextAttributes;
    virtual SEC_STATUS SetContextAttributes(SecHandle * phContext, unsigned long ulAttribute,
                                            void* pBuffer, unsigned long cbBuffer) {
        return SEC_E_OK;
    }

};

enum SecInterface {
    NTLM_Interface,
    Kerberos_Interface,
    SChannel_Interface
};

SecurityFunctionTable * InitSecurityInterface(SecInterface secInter) {

    return NULL;
}


#endif
