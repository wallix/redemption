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
    char* Name;
    char* Comment;
};

struct SecPkgContext_AccessToken {
    void* AccessToken;
};
struct SecPkgContext_Authority
{
    char* sAuthorityName;
};
struct SecPkgContext_ClientSpecifiedTarget
{
    char* sTargetName;
};

struct SecPkgContext_ConnectionInfo
{
    uint32_t dwProtocol;
    uint32_t aiCipher;
    uint32_t dwCipherStrength;
    uint32_t aiHash;
    uint32_t dwHashStrength;
    uint32_t aiExch;
    uint32_t dwExchStrength;
};
struct SecPkgContext_ClientCreds
{
    uint32_t AuthBufferLen;
    uint8_t* AuthBuffer;
};
struct SecPkgContex_DceInfo
{
    uint32_t AuthzSvc;
    void* pPac;
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
struct SecPkgContext_EapKeyBlock
{
    uint8_t rgbKeys[128];
    uint8_t rgbIVs[64];
};
struct SecPkgContext_Flags
{
    uint32_t Flags;
};
struct SecPkgContext_KeyInfo
{
    char*    sSignatureAlgorithmName;
    char*    sEncryptAlgorithmName;
    uint32_t KeySize;
    uint32_t SignatureAlgorithm;
    uint32_t EncryptAlgorithm;
};
struct SecPkgContext_Lifespan
{
    TimeStamp tsStart;
    TimeStamp tsExpiry;
};
struct SecPkgContext_Names
{
    char* sUserName;
};
struct SecPkgContext_NativeNames
{
    char* sClientName;
    char* sServerName;
};

struct SecPkgContext_NegotiationInfo
{
    SecPkgInfo* PackageInfo;
    uint32_t NegotiationState;
};

struct SecPkgContext_PackageInfo
{
    SecPkgInfo* PackageInfo;
};
struct SecPkgContext_PasswordExpiry
{
    TimeStamp tsPasswordExpires;
};
struct SecPkgContext_SessionKey
{
    uint32_t SessionKeyLength;
    uint8_t* SessionKey;
};
struct SecPkgContext_SessionInfo
{
    uint32_t dwFlags;
    uint32_t cbSessionId;
    uint8_t  rgbSessionId[32];
};
struct SecPkgContext_Sizes
{
    uint32_t cbMaxToken;
    uint32_t cbMaxSignature;
    uint32_t cbBlockSize;
    uint32_t cbSecurityTrailer;
};
struct SecPkgContext_StreamSizes
{
    uint32_t cbHeader;
    uint32_t cbTrailer;
    uint32_t cbMaximumMessage;
    uint32_t cBuffers;
    uint32_t cbBlockSize;
};
struct SecPkgContext_SubjectAttributes
{
    void* AttributeInfo;
};
struct SecPkgContext_SupportedSignatures
{
    uint16_t  cSignatureAndHashAlgorithms;
    uint16_t* pSignatureAndHashAlgorithms;
};
struct SecPkgContext_TargetInformation
{
    uint32_t MarshalledTargetInfoLength;
    uint8_t* MarshalledTargetInfo;
};

struct SecPkgCredentials_Names
{
    char* sUserName;
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
struct SecHandle
{
    unsigned long * dwLower;
    unsigned long * dwUpper;
};

struct SecurityFunctionTable {

    virtual ~SecurityFunctionTable() {}
    uint32_t dwVersion;

    // ENUMERATE_SECURITY_PACKAGES_FN EnumerateSecurityPackages;
    virtual uint32_t enumerate_security_package(unsigned long * pcPackages,
                                                SecPkgInfo ** ppPackageInfo) = 0;

    // QUERY_CREDENTIALS_ATTRIBUTES_FN QueryCredentialsAttributes;
    virtual uint32_t query_credentials_attributes(SecHandle * phCredential,
                                                  unsigned long ulAttribute,
                                                  void* pBuffer) = 0;

    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
    // virtual uint32_t acquire_credentials_handle(LPSTR pszPrincipal, LPSTR pszPackage,
    //                                     unsigned long fCredentialUse, void* pvLogonID,
    //                                     void* pAuthData, SEC_GET_KEY_FN pGetKeyFn,
    //                                     void* pvGetKeyArgument, SecHandle * phCredential,
    //                                     TimeStamp * ptsExpiry) = 0;

    // FREE_CREDENTIALS_HANDLE_FN FreeCredentialsHandle;
    virtual uint32_t free_credentials_handle(SecHandle * phCredential) = 0;

    void* Reserved2;
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;
    virtual uint32_t initialize_security_context(SecHandle * phCredential, SecHandle * phContext,
                                         char* pszTargetName, unsigned long fContextReq,
                                         unsigned long Reserved1, unsigned long TargetDataRep,
                                         SecBufferDesc * pInput, unsigned long Reserved2,
                                         SecHandle * phNewContext, SecBufferDesc * pOutput,
                                         unsigned long * pfContextAttr, TimeStamp * ptsExpiry) = 0;

    // ACCEPT_SECURITY_CONTEXT AcceptSecurityContext;
    virtual uint32_t accept_security_context(SecHandle * phCredential, SecHandle * phContext,
                                     SecBufferDesc * pInput, unsigned long fContextReq,
                                     unsigned long TargetDataRep, SecHandle * phNewContext,
                                     SecBufferDesc * pOutput, unsigned long * pfContextAttr,
                                     TimeStamp * ptsTimeStamp) = 0;

    // COMPLETE_AUTH_TOKEN CompleteAuthToken;
    virtual uint32_t complete_auth_token(SecHandle * phContext, SecBufferDesc * pToken) = 0;

    // DELETE_SECURITY_CONTEXT DeleteSecurityContext;
    virtual uint32_t delete_security_context(SecHandle * phContext) = 0;

    // APPLY_CONTROL_TOKEN ApplyControlToken;
    virtual uint32_t apply_control_token(SecHandle * phContext, SecBufferDesc * pInput) = 0;

    // QUERY_CONTEXT_ATTRIBUTES QueryContextAttributes;
    virtual uint32_t query_context_attributes(SecHandle * phContext, unsigned long ulAttribute,
                                              void* pBuffer) = 0;

    // IMPERSONATE_SECURITY_CONTEXT ImpersonateSecurityContext;
    virtual uint32_t impersonate_security_context(SecHandle * phContext) = 0;

    // REVERT_SECURITY_CONTEXT RevertSecurityContext;
    virtual uint32_t revert_security_context(SecHandle * phContext) = 0;

    // MAKE_SIGNATURE MakeSignature;
    virtual uint32_t make_signature(SecHandle * phContext, unsigned long fQOP,
                                    SecBufferDesc * pMessage, unsigned long MessageSeqNo) = 0;

    // VERIFY_SIGNATURE VerifySignature;
    virtual uint32_t verify_signature(SecHandle * phContext, SecBufferDesc * pMessage,
                                      unsigned long MessageSeqNo, unsigned long * pfQOP) = 0;

    // FREE_CONTEXT_BUFFER FreeContextBuffer;
    virtual uint32_t free_context_buffer(void* pvContextBuffer) = 0;

    // QUERY_SECURITY_PACKAGE_INFO QuerySecurityPackageInfo;
    virtual uint32_t query_security_package_info(char* pszPackageName,
                                                 SecPkgInfo ** ppPackageInfo) = 0;

    void* Reserved3;
    void* Reserved4;

    // EXPORT_SECURITY_CONTEXT ExportSecurityContext;
    // virtual uint32_t export_security_context(SecHandle * phContext, unsigned long fFlags,
    //                                  SecBuffer * pPackedContext, HANDLE* pToken) = 0;

    // IMPORT_SECURITY_CONTEXT ImportSecurityContext;
    // virtual uint32_t import_security_context(char* pszPackage, SecBuffer * pPackedContext,
    //                                  HANDLE pToken, SecHandle * phContext) = 0;

    // ADD_CREDENTIALS AddCredentials;
    // virtual uint32_t add_credentials(SecHandle * hCredentials, char* pszPrincipal,
    //                                  char* pszPackage, uint32_t fCredentialUse,
    //                                  void* pAuthData, SEC_GET_KEY_FN pGetKeyFn,
    //                                  void* pvGetKeyArgument, TimeStamp * ptsExpiry) = 0;

    void* Reserved8;

    // QUERY_SECURITY_CONTEXT_TOKEN QuerySecurityContextToken;
    // virtual uint32_t query_security_context_token(SecHandle * phContext, HANDLE* phToken) = 0;

    // ENCRYPT_MESSAGE EncryptMessage;
    virtual uint32_t encrypt_message(SecHandle * phContext, unsigned long fQOP,
                                     SecBufferDesc * pMessage, unsigned long MessageSeqNo) = 0;

    // DECRYPT_MESSAGE DecryptMessage;
    virtual uint32_t decrypt_message(SecHandle * phContext, SecBufferDesc * pMessage,
                                     unsigned long MessageSeqNo, unsigned long * pfQOP) = 0;

    // SET_CONTEXT_ATTRIBUTES SetContextAttributes;
    virtual uint32_t set_context_attributes(SecHandle * phContext, unsigned long ulAttribute,
                                            void* pBuffer, unsigned long cbBuffer) = 0;

};

#endif
