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

#include "utils/sugar/bytes_view.hpp"
#include "core/RDP/nla/ber.hpp"

#include <vector>


enum class PasswordCallback
{
    Error, Ok, Wait
};

namespace credssp {
    enum class State { Err, Cont, Finish, };
}


enum class SecIdFlag {
    SEC_WINNT_AUTH_IDENTITY_ANSI = 0x1,
    SEC_WINNT_AUTH_IDENTITY_UNICODE = 0x2
};

enum SEC_STATUS {
    SEC_E_OK                   = 0x00000000,
    SEC_E_INSUFFICIENT_MEMORY  = 0x80090300,
    SEC_E_INVALID_HANDLE       = 0x80090301,
    SEC_E_UNSUPPORTED_FUNCTION = 0x80090302,
    SEC_E_TARGET_UNKNOWN       = 0x80090303,
    SEC_E_INTERNAL_ERROR       = 0x80090304,
    SEC_E_SECPKG_NOT_FOUND     = 0x80090305,
    SEC_E_NOT_OWNER            = 0x80090306,
    SEC_E_CANNOT_INSTALL       = 0x80090307,
    SEC_E_INVALID_TOKEN        = 0x80090308,
    SEC_E_CANNOT_PACK          = 0x80090309,
    SEC_E_QOP_NOT_SUPPORTED    = 0x8009030A,
    SEC_E_NO_IMPERSONATION     = 0x8009030B,
    SEC_E_LOGON_DENIED         = 0x8009030C,
    SEC_E_UNKNOWN_CREDENTIALS  = 0x8009030D,
    SEC_E_NO_CREDENTIALS       = 0x8009030E,
    SEC_E_MESSAGE_ALTERED      = 0x8009030F,
    SEC_E_OUT_OF_SEQUENCE      = 0x80090310,
    SEC_E_NO_AUTHENTICATING_AUTHORITY = 0x80090311,
    SEC_E_BAD_PKGID            = 0x80090316,
    SEC_E_CONTEXT_EXPIRED      = 0x80090317,
    SEC_E_INCOMPLETE_MESSAGE   = 0x80090318,
    SEC_E_INCOMPLETE_CREDENTIALS = 0x80090320,
    SEC_E_BUFFER_TOO_SMALL     = 0x80090321,
    SEC_E_WRONG_PRINCIPAL      = 0x80090322,
    SEC_E_TIME_SKEW            = 0x80090324,
    SEC_E_UNTRUSTED_ROOT       = 0x80090325,
    SEC_E_ILLEGAL_MESSAGE      = 0x80090326,
    SEC_E_CERT_UNKNOWN         = 0x80090327,
    SEC_E_CERT_EXPIRED         = 0x80090328,
    SEC_E_ENCRYPT_FAILURE      = 0x80090329,
    SEC_E_DECRYPT_FAILURE      = 0x80090330,
    SEC_E_ALGORITHM_MISMATCH   = 0x80090331,
    SEC_E_SECURITY_QOS_FAILED  = 0x80090332,
    SEC_E_UNFINISHED_CONTEXT_DELETED = 0x80090333,
    SEC_E_NO_TGT_REPLY         = 0x80090334,
    SEC_E_NO_IP_ADDRESSES      = 0x80090335,
    SEC_E_WRONG_CREDENTIAL_HANDLE = 0x80090336,
    SEC_E_CRYPTO_SYSTEM_INVALID   = 0x80090337,
    SEC_E_MAX_REFERRALS_EXCEEDED  = 0x80090338,
    SEC_E_MUST_BE_KDC          = 0x80090339,
    SEC_E_STRONG_CRYPTO_NOT_SUPPORTED = 0x8009033A,
    SEC_E_TOO_MANY_PRINCIPALS  = 0x8009033B,
    SEC_E_NO_PA_DATA           = 0x8009033C,
    SEC_E_PKINIT_NAME_MISMATCH = 0x8009033D,
    SEC_E_SMARTCARD_LOGON_REQUIRED = 0x8009033E,
    SEC_E_SHUTDOWN_IN_PROGRESS = 0x8009033F,
    SEC_E_KDC_INVALID_REQUEST  = 0x80090340,
    SEC_E_KDC_UNABLE_TO_REFER  = 0x80090341,
    SEC_E_KDC_UNKNOWN_ETYPE    = 0x80090342,
    SEC_E_UNSUPPORTED_PREAUTH  = 0x80090343,
    SEC_E_DELEGATION_REQUIRED  = 0x80090345,
    SEC_E_BAD_BINDINGS         = 0x80090346,
    SEC_E_MULTIPLE_ACCOUNTS    = 0x80090347,
    SEC_E_NO_KERB_KEY          = 0x80090348,
    SEC_E_CERT_WRONG_USAGE     = 0x80090349,
    SEC_E_DOWNGRADE_DETECTED   = 0x80090350,
    SEC_E_SMARTCARD_CERT_REVOKED = 0x80090351,
    SEC_E_ISSUING_CA_UNTRUSTED = 0x80090352,
    SEC_E_REVOCATION_OFFLINE_C = 0x80090353,
    SEC_E_PKINIT_CLIENT_FAILURE = 0x80090354,
    SEC_E_SMARTCARD_CERT_EXPIRED = 0x80090355,
    SEC_E_NO_S4U_PROT_SUPPORT  = 0x80090356,
    SEC_E_CROSSREALM_DELEGATION_FAILURE = 0x80090357,
    SEC_E_REVOCATION_OFFLINE_KDC = 0x80090358,
    SEC_E_ISSUING_CA_UNTRUSTED_KDC = 0x80090359,
    SEC_E_KDC_CERT_EXPIRED     = 0x8009035A,
    SEC_E_KDC_CERT_REVOKED     = 0x8009035B,
    SEC_E_INVALID_PARAMETER    = 0x8009035D,
    SEC_E_DELEGATION_POLICY    = 0x8009035E,
    SEC_E_POLICY_NLTM_ONLY     = 0x8009035F,
    SEC_E_NO_CONTEXT           = 0x80090361,
    SEC_E_PKU2U_CERT_FAILURE   = 0x80090362,
    SEC_E_MUTUAL_AUTH_FAILED   = 0x80090363,

    SEC_I_CONTINUE_NEEDED      = 0x00090312,
    SEC_I_COMPLETE_NEEDED      = 0x00090313,
    SEC_I_COMPLETE_AND_CONTINUE = 0x00090314,
    SEC_I_LOCAL_LOGON          = 0x00090315,
    SEC_I_CONTEXT_EXPIRED      = 0x00090317,
    SEC_I_INCOMPLETE_CREDENTIALS = 0x00090320,
    SEC_I_RENEGOTIATE          = 0x00090321,
    SEC_I_NO_LSA_CONTEXT       = 0x00090323,
    SEC_I_SIGNATURE_NEEDED     = 0x0009035C,
    SEC_I_NO_RENEGOTIATION     = 0x00090360
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


/**
 * TSRequest ::= SEQUENCE {
 *     version    [0] INTEGER,
 *     negoTokens [1] NegoData OPTIONAL,
 *     authInfo   [2] OCTET STRING OPTIONAL,
 *     pubKeyAuth [3] OCTET STRING OPTIONAL,
 *     errorCode  [4] INTEGER OPTIONAL,
 *     clientNonce[5] OCTET STRING OPTIONAL
 * }
 *
 * NegoData ::= SEQUENCE OF NegoDataItem
 *
 * NegoDataItem ::= SEQUENCE {
 *     negoToken [0] OCTET STRING
 * }
 *
 * TSCredentials ::= SEQUENCE {
 *     credType    [0] INTEGER,
 *     credentials [1] OCTET STRING
 * }
 *
 * TSPasswordCreds ::= SEQUENCE {
 *     domainName  [0] OCTET STRING,
 *     userName    [1] OCTET STRING,
 *     password    [2] OCTET STRING
 * }
 *
 * TSSmartCardCreds ::= SEQUENCE {
 *     pin        [0] OCTET STRING,
 *     cspData    [1] TSCspDataDetail,
 *     userHint   [2] OCTET STRING OPTIONAL,
 *     domainHint [3] OCTET STRING OPTIONAL
 * }
 *
 * TSCspDataDetail ::= SEQUENCE {
 *     keySpec       [0] INTEGER,
 *     cardName      [1] OCTET STRING OPTIONAL,
 *     readerName    [2] OCTET STRING OPTIONAL,
 *     containerName [3] OCTET STRING OPTIONAL,
 *     cspName       [4] OCTET STRING OPTIONAL
 * }
 *
 */

static const int CLIENT_NONCE_LENGTH = 32;

struct ClientNonce {
    bool initialized = false;
    std::vector<uint8_t> clientNonce;

    ClientNonce() : clientNonce{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
        {}

    bool isset() const {
        return this->initialized;
    }

    void reset()
    {
        this->initialized = false;
    }

};

// 2.2.1 TSRequest
// ===============

// The TSRequest structure is the top-most structure used by the CredSSP client and
// CredSSP server. It contains the SPNEGO tokens and MAY<6> contain Kerberos/NTLM
// messages that are passed between the client and server, and either the public
// key authentication messages that are used to bind to the TLS session or the
// client credentials that are delegated to the server. The TSRequest message is
// always sent over the TLS-encrypted channel between the client and server in a
// CredSSP Protocol exchange (see step 1 in section 3.1.5).

//     TSRequest ::= SEQUENCE {
//             version    [0] INTEGER,
//             negoTokens [1] NegoData  OPTIONAL,
//             authInfo   [2] OCTET STRING OPTIONAL,
//             pubKeyAuth [3] OCTET STRING OPTIONAL,
//             errorCode  [4] INTEGER OPTIONAL,
//             clientNonce [5] OCTET STRING OPTIONAL
//     }

// version: This field specifies the supported version of the CredSSP Protocol.
// Valid values for this field are 2, 3, 4, 5, and 6.<7> If the version received is
// greater than the implementation understands, treat the peer as one that is
// compatible with the version of the CredSSP Protocol that the implementation
// understands.

// negoTokens: A NegoData structure, as defined in section 2.2.1.1, that contains
// the SPNEGO tokens or Kerberos/NTLM messages that are passed between the client
// and server.

// * NegoData ::= SEQUENCE OF NegoDataItem
// *
// * NegoDataItem ::= SEQUENCE {
// *     negoToken [0] OCTET STRING
// * }

// authInfo: A TSCredentials structure, as defined in section 2.2.1.2, that
// contains the user's credentials that are delegated to the server. The authInfo
// field MUST be encrypted under the encryption key that is negotiated under the
// SPNEGO package. The authInfo field carries the message signature and then the
// encrypted data.

// * TSCredentials ::= SEQUENCE {
// *     credType    [0] INTEGER,
// *     credentials [1] OCTET STRING
// * }


// pubKeyAuth: This field is used to assure that the public key that is used by the
// server during the TLS handshake belongs to the target server and not to a
// man-in-the-middle. This TLS session-binding is specified in section 3.1.5. After
// the client completes the SPNEGO phase of the CredSSP Protocol, it uses
// GSS_WrapEx() for the negotiated protocol to encrypt the server's public key.
// With version 4 or lower, the pubKeyAuth field carries the message signature and
// then the encrypted public key to the server. In response, the server uses the
// pubKeyAuth field to transmit to the client a modified version of the public key
// (as specified in section 3.1.5) that is encrypted under the encryption key that
// is negotiated under SPNEGO. In version 5 or higher, this field stores a computed
// hash of the public key.<8>

// errorCode: If the negotiated protocol version is 3, 4, or 6, and the SPNEGO
// exchange fails on the server, this field SHOULD<9> be used to send the NTSTATUS
// failure code ([MS-ERREF] section 2.3) to the client so that it knows what failed
// and be able to display a descriptive error to the user.

// clientNonce: A 32-byte array of cryptographically random bytes used to provide
// sufficient entropy during hash computation. This value is only used in version 5
// or higher of this protocol.




struct TSRequest final {
    /* TSRequest */

    /* [0] version */
    uint32_t version;
    uint32_t use_version;

    /* [1] negoTokens (NegoData) */
    std::vector<uint8_t> negoTokens;
    // BStream negoTokens;
    /* [2] authInfo (OCTET STRING) */
    std::vector<uint8_t> authInfo;
    // BStream authInfo;
    /* [3] pubKeyAuth (OCTET STRING) */
    std::vector<uint8_t> pubKeyAuth;
    // BStream pubKeyAuth;
    /* [4] errorCode (INTEGER OPTIONAL) */
    uint32_t error_code{0};
    /* [5] clientNonce (OCTET STRING OPTIONAL) */
    ClientNonce clientNonce;

    TSRequest(uint32_t version = 6): version(version), use_version(this->version)
    {
    }
};


std::vector<uint8_t> emitTSRequest(uint32_t version,
                                   bytes_view negoTokens,
                                   bytes_view authInfo,
                                   bytes_view pubKeyAuth,
                                   uint32_t error_code,
                                   bytes_view clientNonce,
                                   bool nonce_initialized,
                                   bool verbose);

TSRequest recvTSRequest(bytes_view data, bool verbose);

/*
 * TSPasswordCreds ::= SEQUENCE {
 *     domainName  [0] OCTET STRING,
 *     userName    [1] OCTET STRING,
 *     password    [2] OCTET STRING
 * }
 */
struct TSPasswordCreds {
    std::vector<uint8_t> domainName;
    std::vector<uint8_t> userName;
    std::vector<uint8_t> password;
};


TSPasswordCreds recvTSPasswordCreds(bytes_view data, bool verbose);

std::vector<uint8_t> emitTSPasswordCreds(bytes_view domain, bytes_view user, bytes_view password, bool verbose);


/* TSCspDataDetail ::= SEQUENCE {
 *     keySpec       [0] INTEGER,
 *     cardName      [1] OCTET STRING OPTIONAL,
 *     readerName    [2] OCTET STRING OPTIONAL,
 *     containerName [3] OCTET STRING OPTIONAL,
 *     cspName       [4] OCTET STRING OPTIONAL
 * }
 *
 */
struct TSCspDataDetail {
    uint32_t keySpec{0};
    std::vector<uint8_t> cardName;
    std::vector<uint8_t> readerName;
    std::vector<uint8_t> containerName;
    std::vector<uint8_t> cspName;
};


TSCspDataDetail recvTSCspDataDetail(bytes_view data);


std::vector<uint8_t> emitTSCspDataDetail(uint32_t keySpec,
                                         bytes_view cardName,
                                         bytes_view readerName,
                                         bytes_view containerName,
                                         bytes_view cspName);


/*
 * TSSmartCardCreds ::= SEQUENCE {
 *     pin        [0] OCTET STRING,
 *     cspData    [1] TSCspDataDetail,
 *     userHint   [2] OCTET STRING OPTIONAL,
 *     domainHint [3] OCTET STRING OPTIONAL
 * }
 */

struct TSSmartCardCreds {
    std::vector<uint8_t> pin;
    TSCspDataDetail cspData;
    std::vector<uint8_t> userHint;
    std::vector<uint8_t> domainHint;
};

TSSmartCardCreds recvTSSmartCardCreds(bytes_view data, bool verbose);

std::vector<uint8_t> emitTSSmartCardCreds(
                  buffer_view pin, buffer_view userHint, bytes_view domainHint,
                  uint32_t keySpec,
                  bytes_view cardName,
                  bytes_view readerName,
                  bytes_view containerName,
                  bytes_view cspName);


/*
 * TSCredentials ::= SEQUENCE {
 *     credType    [0] INTEGER,
 *     credentials [1] OCTET STRING
 * }
 */
struct TSCredentials
{
    uint32_t credType{1};
    TSPasswordCreds passCreds;
    TSSmartCardCreds smartcardCreds;
    // For now, TSCredentials only contains TSPasswordCreds (not TSSmartCardCreds)
};

TSCredentials recvTSCredentials(bytes_view data, bool verbose);

std::vector<uint8_t> emitTSCredentialsPassword(bytes_view domainName, bytes_view userName, bytes_view password, bool verbose);

std::vector<uint8_t> emitTSCredentialsSmartCard(
                  buffer_view pin, buffer_view userHint, bytes_view domainHint,
                  uint32_t keySpec,
                  bytes_view cardName,
                  bytes_view readerName,
                  bytes_view containerName,
                  bytes_view cspName,
                  bool verbose);

/** @brief an identified OID */
enum KnownOid {
    OID_UNKNOWN,
    OID_NTLM,
    OID_SPNEGO,
    OID_SPNEGOEX,
    OID_KRB5,
    OID_KRB5_KILE,
    OID_KRB5_U2U
};

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

/** @brief operations on known OIDs */
class KnownOidHelper {
public:
    static constexpr uint8_t spNegoOid[] = { 0x2B, 0x06, 0x01, 0x05, 0x05, 0x02 };
    static constexpr uint8_t kileOid[] = { 0x2A, 0x86, 0x48, 0x82, 0xF7, 0x12, 0x01, 0x02, 0x02 };
    static constexpr uint8_t krb5Oid[] = { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x12, 0x01, 0x02, 0x02 };
    static constexpr uint8_t user2userOid[] = { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x12, 0x01, 0x02, 0x02, 0x03 };
    static constexpr uint8_t spnegoexOid[] = { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x02, 0x02, 0x1E };
    static constexpr uint8_t ntlmOid[] = { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x02, 0x02, 0x0A };
    struct KnownOidDef {
        const uint8_t *oid;
        size_t oidLen;
        KnownOid flag;
    };

    static constexpr KnownOidDef knownOids[] = {
        { spNegoOid, sizeof(spNegoOid), OID_SPNEGO },
        { kileOid, sizeof(kileOid), OID_KRB5_KILE },
        { krb5Oid, sizeof(krb5Oid), OID_KRB5 },
        { spnegoexOid, sizeof(spnegoexOid), OID_SPNEGOEX },
        { ntlmOid, sizeof(ntlmOid), OID_NTLM },
        { user2userOid, sizeof(user2userOid), OID_KRB5_U2U },
    };

    /** given an oid buffer try to match a known OID
     * @param oid OID string
     * @return the resolved KnownOid, OID_UNKNOWN if not found
     */
    static KnownOid resolve(const BER::BerOID & oid)
    {
        for (KnownOidDef const& knowOid : knownOids) {
            if (oid.size() != knowOid.oidLen)
                continue;
            if (memcmp(oid.data(), knowOid.oid, knowOid.oidLen) == 0)
                return knowOid.flag;
        }

        return OID_UNKNOWN;
    }

    /** returns the OID bytes for the given known OID
     * @param oid known OID item
     * @return a bytes_view on the OID bytes, an empty bytes_view if not found
     */
    static bytes_view oidData(KnownOid oid)
    {
        if (oid == OID_UNKNOWN)
            return bytes_view();

        for (KnownOidDef const& knowOid : knownOids) {
            if (knowOid.flag == oid)
                return bytes_view(knowOid.oid, knowOid.oidLen);
        }

        return bytes_view();
    }
};


bool check_sp_nego(bytes_view data, bool verbose, bytes_view & body);

KnownOid guessAuthTokenType(bytes_view data);



/** @brief SPNEGO mech */
struct SpNegoMech {
    KnownOid mechType;
    BER::BerOID oid;

    static KnownOid resolve_mech(const std::vector<uint8_t> & oid) {
        KnownOid ret = KnownOidHelper::resolve(oid);
        switch(ret) {
        case OID_UNKNOWN:
        case OID_NTLM:
        case OID_SPNEGO:
        case OID_SPNEGOEX:
        case OID_KRB5:
        case OID_KRB5_KILE:
            return ret;

        case OID_KRB5_U2U:
            return OID_UNKNOWN;
        }
    }
};



/** @brief SPNEGO negTokenInit packet */
struct SpNegoTokenInit final {
    std::vector<SpNegoMech> mechTypes;
    uint32_t reqFlags;
    std::vector<uint8_t> mechToken;
    std::vector<uint8_t> mechListMic;
};

SpNegoTokenInit recvSpNegoTokenInit(bytes_view data, bool verbose);

/** @brief negState in negTokenResp */
enum SpNegoNegstate : uint32_t {
    SPNEGO_STATE_ACCEPT_COMPLETED = 0,
    SPNEGO_STATE_ACCEPT_INCOMPLETE = 1,
    SPNEGO_STATE_REJECT = 2,
    SPNEGO_STATE_REQUEST_MIC = 3,

    SPNEGO_STATE_INVALID = 0xffffffffu,
};

/** @brief SPNEGO negTokenResp packet */
struct SpNegoTokenResp final {
    SpNegoNegstate negState;
    std::vector<uint8_t> supportedMech {};
    std::vector<uint8_t> responseToken {};
    std::vector<uint8_t> mechListMic {};
};


SpNegoTokenResp recvSpNegoTokenResp(bytes_view data);


std::vector<uint8_t> emitMechTokensEnvelop(const std::vector<uint8_t> & mechTokens);


std::vector<uint8_t> emitNegTokenResp(
        SpNegoNegstate negState,
        KnownOid supportedMech,
        buffer_view responseToken,
        buffer_view mechListMIC,
        bool verbose);


struct SpNegoToken {
    bool isInit;
    bool isError;
    SpNegoTokenInit negTokenInit;
    SpNegoTokenResp negTokenResp;
};

SpNegoToken recvSpNego(bytes_view data, bool verbose);
