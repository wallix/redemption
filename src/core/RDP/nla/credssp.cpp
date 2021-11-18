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


#include <cstring>

#include "utils/stream.hpp"
#include "core/RDP/nla/ntlm_message.hpp"
#include "core/RDP/nla/ber.hpp"
#include "core/RDP/nla/credssp.hpp"


std::vector<uint8_t> emitTSRequest(uint32_t version,
                                   bytes_view negoTokens,
                                   bytes_view authInfo,
                                   bytes_view pubKeyAuth,
                                   uint32_t error_code,
                                   bytes_view clientNonce,
                                   bool nonce_initialized,
                                   bool verbose)
{
    return BER::emitTSRequest(version, negoTokens, authInfo, pubKeyAuth, error_code, clientNonce, nonce_initialized, verbose);
}

TSRequest recvTSRequest(bytes_view data, bool verbose)
{
    if (verbose) {
        LOG(LOG_INFO, "recv TSRequest full dump++++++++++++++++++++++++++++++++");
        hexdump_d(data);
        LOG(LOG_INFO, "recv TSRequest hexdump - START PARSING DATA+++++++++++++");
    }

    InStream stream(data);
    TSRequest self(6);

    /* TSRequest */
    auto [length, queue] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT|BER::TAG_SEQUENCE_OF, "TS Request");
    stream.in_skip_bytes(stream.in_remain()-queue.size());
    (void)length;

    // version    [0] INTEGER,
    auto [value8, queue8] = BER::pop_integer_field(stream.remaining_bytes(), 0, "TS Request [0]");
    stream.in_skip_bytes(stream.in_remain()-queue8.size());
    self.use_version = value8;
    if (verbose) {
        LOG(LOG_INFO, "Credssp recvTSRequest() Remote Version %u", self.use_version);
    }

    // [1] negoTokens (NegoData) OPTIONAL
    if (BER::check_ber_ctxt_tag(stream.remaining_bytes(), 1)) {
        stream.in_skip_bytes(1);
        auto [len0, queue0] = BER::pop_length(stream.remaining_bytes(), "TS Request [1] negoTokens");
        stream.in_skip_bytes(stream.in_remain()-queue0.size());
        (void)len0;

        // * NegoData ::= SEQUENCE OF NegoDataItem
        // *
        // * NegoDataItem ::= SEQUENCE {
        // *     negoToken [0] OCTET STRING
        // * }

        // NegoData ::= SEQUENCE OF NegoDataItem
        auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "TS Request [1] negoTokens NegoData");
        stream.in_skip_bytes(stream.in_remain()-queue1.size());
        (void)length1;

        // NegoDataItem ::= SEQUENCE {
        auto [length2, queue2] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "TS Request [1] negoTokens NegoData NegoDataItem");
        stream.in_skip_bytes(stream.in_remain()-queue2.size());
        (void)length2;

        self.negoTokens = BER::read_mandatory_octet_string(stream, 0, "TS Request [1] negoTokens NegoData NegoDataItem [0] negoToken");
    }

    self.authInfo   = BER::read_optional_octet_string(stream, 2, "TSRequest [2] authInfo");
    self.pubKeyAuth = BER::read_optional_octet_string(stream, 3, "TSRequest [2] pubKeyAuth");

    /* [4] errorCode (INTEGER) */
    if (self.use_version >= 3 && self.use_version != 5){
        if (BER::check_ber_ctxt_tag(stream.remaining_bytes(), 4)){
            auto [value, queue] = BER::pop_integer_field(stream.remaining_bytes(), 4, "TS Request [4] errorCode");
            stream.in_skip_bytes(stream.in_remain()-queue.size());
            self.error_code = value;
            LOG(LOG_INFO, "Credssp recvTSCredentials() "
                "ErrorCode = %x, Facility = %x, Code = %x",
                self.error_code,
                (self.error_code >> 16) & 0x7FF,
                (self.error_code & 0xFFFF)
            );
        }
    }

    /* [5] clientNonce (OCTET STRING) */
    if (self.use_version >= 5){
        self.clientNonce.clientNonce = BER::read_optional_octet_string(stream, 5, "TSRequest [5] clientNonce");
        if (not self.clientNonce.clientNonce.empty()){
            self.clientNonce.initialized = true;
        }
    }
    return self;
}


TSPasswordCreds recvTSPasswordCreds(bytes_view data, bool verbose)
{
    if (verbose) {
        LOG(LOG_INFO, "recvTSPasswordCreds full dump--------------------------------");
        hexdump_d(data);
        LOG(LOG_INFO, "recvTSPasswordCreds hexdump - START PARSING DATA-------------");
    }

    InStream stream(data); // check all is consumed
    TSPasswordCreds self;
    /* TSPasswordCreds (SEQUENCE) */
    auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "TSPasswordCreds");
    stream.in_skip_bytes(stream.in_remain()-queue1.size());
    (void)length1;

    /* [0] domainName (OCTET STRING) */
    auto [length2, queue2] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_CTXT|BER::PC_CONSTRUCT|0, "TSPasswordCreds::domainName");
    stream.in_skip_bytes(stream.in_remain()-queue2.size());
    (void)length2;

    auto [length3, queue3] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_PRIMITIVE|BER::TAG_OCTET_STRING, "TSPasswordCreds::domainName");
    stream.in_skip_bytes(stream.in_remain()-queue3.size());

    self.domainName.resize(length3);
    stream.in_copy_bytes(self.domainName.data(), self.domainName.size());

    /* [1] userName (OCTET STRING) */
    auto [length4, queue4] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_CTXT|BER::PC_CONSTRUCT|1, "TSPasswordCreds::userName");
    stream.in_skip_bytes(stream.in_remain()-queue4.size());
    (void)length4;

    auto [length5, queue5] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_PRIMITIVE|BER::TAG_OCTET_STRING, "TSPasswordCreds::userName");
    stream.in_skip_bytes(stream.in_remain()-queue5.size());

    self.userName.resize(length5);
    stream.in_copy_bytes(self.userName.data(), self.userName.size());

    /* [2] password (OCTET STRING) */
    auto [length6, queue6] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_CTXT|BER::PC_CONSTRUCT|2, "TSPasswordCreds::password");
    stream.in_skip_bytes(stream.in_remain()-queue6.size());
    (void)length6;

    auto [length7, queue7] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_PRIMITIVE|BER::TAG_OCTET_STRING, "TSPasswordCreds::password");

    stream.in_skip_bytes(stream.in_remain()-queue7.size());

    self.password.resize(length7);
    stream.in_copy_bytes(self.password.data(), self.password.size());

    return self;
}


TSCspDataDetail recvTSCspDataDetail(bytes_view data)
{
    InStream stream(data);
    TSCspDataDetail self;
    // TSCspDataDetail ::= SEQUENCE
    auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "TSCspDataDetail Sequence");
    stream.in_skip_bytes(stream.in_remain()-queue1.size());
    (void)length1;

    auto [value2, queue2] = BER::pop_integer_field(queue1, 0, "TSCspDataDetail [0] keySpec");
    stream.in_skip_bytes(stream.in_remain()-queue2.size());
    self.keySpec       = value2;

    self.cardName      = BER::read_optional_octet_string(stream, 1, "TSCspDataDetail [1] cardName");
    self.readerName    = BER::read_optional_octet_string(stream, 2, "TSCspDataDetail [2] readerName");
    self.containerName = BER::read_optional_octet_string(stream, 3, "TSCspDataDetail [3] containerName");
    self.cspName       = BER::read_optional_octet_string(stream, 4, "TSCspDataDetail [4] cspName");
    return self;
}


TSSmartCardCreds recvTSSmartCardCreds(bytes_view data, bool verbose)
{
    if (verbose) {
        LOG(LOG_INFO, "recvTSSmartCardCreds full dump--------------------------------");
        hexdump_d(data);
        LOG(LOG_INFO, "recvTSSmartCardCreds hexdump - START PARSING DATA-------------");
    }

    InStream stream(data);
    TSSmartCardCreds self;
    /* TSSmartCardCreds (SEQUENCE) */
    auto [length0, queue0] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "TSSmartCardCreds");
    stream.in_skip_bytes(stream.in_remain()-queue0.size());
    (void)length0;

    /* [0] pin (OCTET STRING) */
    auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_CTXT|BER::PC_CONSTRUCT|0, "TSSmartCardCreds [0] pin");
    stream.in_skip_bytes(stream.in_remain()-queue1.size());
    (void)length1;

    auto [length2, queue2] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_PRIMITIVE|BER::TAG_OCTET_STRING, "TSSmartCardCreds [0] pin");
    stream.in_skip_bytes(stream.in_remain()-queue2.size());

    self.pin.resize(length2);
    stream.in_copy_bytes(self.pin);

    /* [1] cspData (TSCspDataDetail) */
    {
        auto [tSCspDataDetailLength, queue3] =  BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_CTXT|BER::PC_CONSTRUCT|1, "TSSmartCardCreds [1] cspData");
        stream.in_skip_bytes(stream.in_remain()-queue3.size());

        bytes_view data = stream.in_skip_bytes(tSCspDataDetailLength);
        self.cspData = recvTSCspDataDetail(data);
    }

    self.userHint   = BER::read_optional_octet_string(stream, 2, "TSSmartCardCreds [2] userHint");
    self.domainHint = BER::read_optional_octet_string(stream, 3, "TSSmartCardCreds [3] domainHint");
    return self;
}

TSCredentials recvTSCredentials(bytes_view data, bool verbose)
{
    if (verbose) {
        LOG(LOG_INFO, "recvTSCredentials full dump--------------------------------");
        hexdump_d(data);
        LOG(LOG_INFO, "recvTSCredentials hexdump - START PARSING DATA-------------");
    }
    InStream stream(data);
    TSCredentials self;
    // stream is decrypted and should be decrypted before calling recv

    // TSCredentials (SEQUENCE)
    auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "TSCredentials");
    stream.in_skip_bytes(stream.in_remain()-queue1.size());
    (void)length1;

    // [0] credType (INTEGER)
    auto [value2, queue2] = BER::pop_integer_field(stream.remaining_bytes(), 0, "TSCredentials [0] credType ");
    stream.in_skip_bytes(stream.in_remain()-queue2.size());
    self.credType = value2;


    // [1] credentials (OCTET STRING)
    auto [length3, queue3] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_CTXT|BER::PC_CONSTRUCT|1, "TSCredentials");
    stream.in_skip_bytes(stream.in_remain()-queue3.size());
    (void)length3;

    auto [creds_length, queue4] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_PRIMITIVE|BER::TAG_OCTET_STRING, "TSSmartCardCreds [3] domainHint");
    stream.in_skip_bytes(stream.in_remain()-queue4.size());


    if (self.credType == 2) {
        bytes_view data = stream.in_skip_bytes(creds_length);
        self.smartcardCreds = recvTSSmartCardCreds(data, verbose);
    } else {
        bytes_view data = stream.in_skip_bytes(creds_length);
        self.passCreds = recvTSPasswordCreds(data, verbose);
    }
    return self;
}

std::vector<uint8_t> emitTSCredentialsPassword(bytes_view domainName, bytes_view userName, bytes_view password, bool verbose)
{
    return BER::emitTSCredentialsPassword(domainName, userName, password, verbose);
}

std::vector<uint8_t> emitTSCredentialsSmartCard(
                  buffer_view pin, buffer_view userHint, bytes_view domainHint,
                  uint32_t keySpec,
                  bytes_view cardName,
                  bytes_view readerName,
                  bytes_view containerName,
                  bytes_view cspName,
                  bool verbose)
{
    return BER::emitTSCredentialsSmartCard(pin, userHint, domainHint, keySpec, cardName, readerName, containerName, cspName, verbose);
}


bool check_sp_nego(bytes_view data, bool verbose, bytes_view & body)
{
    bytes_view appBody;
    BER::BerOID oid;

    if (!BER::check_ber_app_tag(data, 0, verbose, "", appBody))
        return false;

    if (!BER::peek_oid(appBody, verbose, "", oid, body))
        return false;

    switch (KnownOidHelper::resolve(oid)) {
    case OID_SPNEGO:
        return true;

    case OID_SPNEGOEX: // TODO: handle it ?
    case OID_UNKNOWN:
    case OID_NTLM:
    case OID_KRB5:
    case OID_KRB5_KILE:
    case OID_KRB5_U2U:
        return false;
    }
}

KnownOid guessAuthTokenType(bytes_view data)
{
    bytes_view body;

    /* first check a FreeRDP token */
    const uint8_t ntlmSignature[] = {'N', 'T', 'L', 'M', 'S', 'S', 'P', 0x00};
    if (data.size() > sizeof(ntlmSignature) && memcmp(data.data(), ntlmSignature, sizeof(ntlmSignature)) == 0)
        return OID_NTLM;

    /* check for a raw kerberos token */
    if (BER::check_ber_app_tag(data, 0, false, "", body)) {
        BER::BerOID oid;

        if (BER::peek_oid(body, false, "", oid, body)) {
            KnownOid r = KnownOidHelper::resolve(oid);
            switch (r) {
            case OID_KRB5:
            case OID_KRB5_KILE:
            case OID_KRB5_U2U:
                return r;

            case OID_SPNEGO:
            case OID_SPNEGOEX:
            case OID_NTLM:
            case OID_UNKNOWN:
                break;
            }
        }
    }

    /* finally look if it's SPnego */
    if (check_sp_nego(data, false, body))
        return OID_SPNEGO;

    return OID_UNKNOWN;
}


SpNegoTokenInit recvSpNegoTokenInit(bytes_view data, bool verbose)
{
    InStream stream(data);
    SpNegoTokenInit self;

    /* NegTokenInit ::= SEQUENCE { */
    auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "NegTokenInit");
    stream.in_skip_bytes(stream.in_remain()-queue1.size());
    (void)length1;

    /* mechTypes [0] MechTypeList, */
    auto [length2, queue2] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_CTXT|BER::PC_CONSTRUCT|0, "NegTokenInit::mechTypes");
    stream.in_skip_bytes(stream.in_remain()-queue2.size());

    auto [length3, queue3] = BER::pop_tag_length(stream.remaining_bytes(), BER::PC_CONSTRUCT|BER::TAG_SEQUENCE_OF, "NegTokenInit::mechTypes");
    (void)length3;

    stream.in_skip_bytes(stream.in_remain()-queue3.size());
    auto mechs = stream.in_skip_bytes(length3);
    InStream mechs_stream(mechs);
    while (mechs_stream.in_remain()) {
        SpNegoMech mech;
        mech.oid = BER::pop_oid(mechs_stream, "NegTokenInit::mechTypes");
        mech.mechType = SpNegoMech::resolve_mech(mech.oid);
        LOG_IF(mech.mechType == OID_UNKNOWN && verbose, LOG_INFO, "recvSpNegoTokenInit mechUnknown");
        self.mechTypes.push_back(mech);
    }

    /* reqFlags [1] ContextFlags  OPTIONAL */
    self.reqFlags = BER::read_optional_integer(stream, 1, "NegTokenInit::reqFlags");

    /* mechToken [2] OCTET STRING  OPTIONAL, */
    self.mechToken = BER::read_optional_octet_string(stream, 2, "NegTokenInit::mechToken");

    /* mechListMIC [3] OCTET STRING  OPTIONAL, */
    self.mechListMic = BER::read_optional_octet_string(stream, 3, "NegTokenInit::mechListMIC");

    /* } */
    return self;
}


SpNegoTokenResp recvSpNegoTokenResp(bytes_view data)
{
    InStream stream(data);
    SpNegoTokenResp self { SPNEGO_STATE_ACCEPT_COMPLETED };

    /* NegTokenResp ::= SEQUENCE { */
    auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "NegTokenResp");
    stream.in_skip_bytes(stream.in_remain()-queue1.size());
    (void)length1;

    /* negState       [0] ENUMERATED {
              accept-completed    (0),
              accept-incomplete   (1),
              reject              (2),
              request-mic         (3)
          }                                 OPTIONAL,
    */
    uint32_t enum_value = SPNEGO_STATE_ACCEPT_COMPLETED;
    BER::read_optional_enumerated(stream, 0, enum_value, "NegTokenInit::negState");
    self.negState = SpNegoNegstate(enum_value);

    /* supportedMech   [1] MechType      OPTIONAL, */
    self.supportedMech = BER::read_optional_oid(stream, 1, "NegTokenInit::mechToken");

    /* mechToken [2] OCTET STRING  OPTIONAL, */
    self.responseToken = BER::read_optional_octet_string(stream, 2, "NegTokenInit::responseToken");

    /* mechListMIC [3] OCTET STRING  OPTIONAL, */
    self.mechListMic = BER::read_optional_octet_string(stream, 3, "NegTokenInit::mechListMIC");

    /* } */
    return self;
}


std::vector<uint8_t> emitMechTokensEnvelop(bytes_view mechTokens)
{
    return BER::emitMechTokensEnvelop(mechTokens);
}


std::vector<uint8_t> emitNegTokenResp(
        SpNegoNegstate negState,
        KnownOid supportedMech,
        buffer_view responseToken,
        buffer_view mechListMIC,
        bool verbose)
{
    // negState       [0] ENUMERATED {
    //      accept-completed    (0),
    //      accept-incomplete   (1),
    //      reject              (2),
    //      request-mic         (3)
    //  }                                 OPTIONAL,
    return BER::emitNegTokenResp(underlying_cast(negState), negState != SPNEGO_STATE_INVALID, KnownOidHelper::oidData(supportedMech), responseToken, mechListMIC, verbose);
}


SpNegoToken recvSpNego(bytes_view data, bool verbose) {
    SpNegoToken ret;
    bytes_view appBody;
    uint8_t tag;

    if (!BER::peek_construct_tag(data, tag, verbose, "NegotiationToken", appBody))
        return ret;

    ret.isError = false;
    if (tag == 0) {
        /* NegTokenInit */
        ret.isInit = true;

        auto [spNegoOid, queue] = BER::pop_oid(appBody, "oid");

        if (!BER::check_ber_ctxt_tag(queue, 0)) {
            ret.isError = true;
            return ret;
        }

        auto [len, body] = BER::pop_tag_length(queue, BER::CLASS_CTXT|BER::PC_CONSTRUCT|0, "NegTokenInit");
        ret.isInit = true;
        ret.negTokenInit = recvSpNegoTokenInit(body, verbose);

    } else if (tag == 1) {
        /* NegTokenResp */
        ret.isInit = false;
        ret.negTokenResp = recvSpNegoTokenResp(appBody);

    } else {
        ret.isError = true;
    }

    return ret;
}
