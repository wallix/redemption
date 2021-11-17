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

std::vector<uint8_t> emitTSPasswordCreds(bytes_view domain, bytes_view user, bytes_view password, bool verbose)
{
    // [0] domainName (OCTET STRING)
    auto ber_domain_name_header = BER::mkMandatoryOctetStringFieldHeader(domain.size(), 0);
    // [1] userName (OCTET STRING)
    auto ber_user_name_header = BER::mkMandatoryOctetStringFieldHeader(user.size(), 1);
    // [2] password (OCTET STRING)
    auto ber_password_header = BER::mkMandatoryOctetStringFieldHeader(password.size(), 2);

    // TSPasswordCreds (SEQUENCE)
    size_t ts_password_creds_length = ber_domain_name_header.size()+ domain.size()
                             + ber_user_name_header.size()+user.size()
                             + ber_password_header.size()+password.size();

    auto ber_ts_password_creds_header = BER::mkSequenceHeader(uint32_t(ts_password_creds_length));

    std::vector<uint8_t> result = std::move(ber_ts_password_creds_header);
    result << ber_domain_name_header << domain
           << ber_user_name_header << user
           << ber_password_header << password;

    if (verbose) {
        LOG(LOG_INFO, "emitPasswordsCreds full dump ------------");
        hexdump_d(result);
        LOG(LOG_INFO, "emitPasswordsCreds hexdump done ---------");
    }

    return result;
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


std::vector<uint8_t> emitTSCspDataDetail(uint32_t keySpec,
                                         bytes_view cardName,
                                         bytes_view readerName,
                                         bytes_view containerName,
                                         bytes_view cspName)
{
    auto ber_keySpec_Field        = BER::mkIntegerField(keySpec, 0);
    auto ber_cardName_Header      = BER::mkOptionalOctetStringFieldHeader(cardName.size(), 1);
    auto ber_readerName_Header    = BER::mkOptionalOctetStringFieldHeader(readerName.size(), 2);
    auto ber_containerName_Header = BER::mkOptionalOctetStringFieldHeader(containerName.size(), 3);
    auto ber_cspName_Header       = BER::mkOptionalOctetStringFieldHeader(cspName.size(), 4);

    auto innerSize = ber_keySpec_Field.size()
                  + ber_cardName_Header.size() + cardName.size()
                  + ber_readerName_Header.size() + readerName.size()
                  + ber_containerName_Header.size() + containerName.size()
                  + ber_cspName_Header.size() + cspName.size();

    // TSCspDataDetail (SEQUENCE)
    auto sequence_header = BER::mkSequenceHeader(innerSize);

    std::vector<uint8_t> result = std::move(sequence_header);
    result << ber_keySpec_Field
           << ber_cardName_Header      << cardName
           << ber_readerName_Header    << readerName
           << ber_containerName_Header << containerName
           << ber_cspName_Header       << cspName;

    return result;
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

std::vector<uint8_t> emitTSSmartCardCreds(
                  buffer_view pin, buffer_view userHint, bytes_view domainHint,
                  uint32_t keySpec,
                  bytes_view cardName,
                  bytes_view readerName,
                  bytes_view containerName,
                  bytes_view cspName)
{
    // [0] pin (OCTET STRING)
    auto ber_pin_header = BER::mkMandatoryOctetStringFieldHeader(pin.size(), 0);

    // [1] cspData (TSCspDataDetail)
    auto ber_TSCspDataDetail = emitTSCspDataDetail(keySpec, cardName, readerName, containerName, cspName);
    auto ber_CspDataDetail_header = BER::mkContextualFieldHeader(ber_TSCspDataDetail.size(), 1);

    /* [2] userHint (OCTET STRING OPTIONAL) */
    auto ber_userHint_header = BER::mkOptionalOctetStringFieldHeader(userHint.size(), 2);

    /* [3] domainHint (OCTET STRING OPTIONAL) */
    auto ber_domainHint_header = BER::mkOptionalOctetStringFieldHeader(domainHint.size(), 3);

    /* TSCredentials (SEQUENCE) */
    size_t ts_smartcards_creds_length = ber_pin_header.size() + pin.size()
                  + ber_CspDataDetail_header.size() + ber_TSCspDataDetail.size()
                  + ber_userHint_header.size() + userHint.size()
                  + ber_domainHint_header.size() + domainHint.size()
                  ;

    auto ber_ts_smartcards_creds_header = BER::mkSequenceHeader(ts_smartcards_creds_length);

    std::vector<uint8_t> result = std::move(ber_ts_smartcards_creds_header);
    result << ber_pin_header           << pin
           << ber_CspDataDetail_header << ber_TSCspDataDetail
           << ber_userHint_header      << userHint
           << ber_domainHint_header    << domainHint;

    return result;
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
    // [0] credType (INTEGER) : 1 means password
    auto ber_credtype_field = BER::mkSmallIntegerField(1, 0);

    // [1] credentials (OCTET STRING)
    std::vector<uint8_t> ber_credentials = emitTSPasswordCreds(domainName, userName, password, verbose);
    auto ber_credentials_header = BER::mkMandatoryOctetStringFieldHeader(ber_credentials.size(), 1);

    // TSCredentials (SEQUENCE)
    auto inner_size = ber_credtype_field.size() + ber_credentials_header.size() + ber_credentials.size();
    auto sequence_header = BER::mkSequenceHeader(inner_size);

    std::vector<uint8_t> result = std::move(sequence_header);
    result << ber_credtype_field
           << ber_credentials_header << ber_credentials;

    if (verbose) {
        LOG(LOG_INFO, "emitTSCredentialsPassword full dump ------------");
        hexdump_d(result);
        LOG(LOG_INFO, "emitTSCredentialsPassword hexdump done----------");
    }

    return result;
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
    // [0] credType (INTEGER): 2 means SmartCard
    auto ber_credtype_field = BER::mkSmallIntegerField(2, 0);

    // [1] credentials (OCTET STRING)
    std::vector<uint8_t> ber_credentials = emitTSSmartCardCreds(pin, userHint, domainHint, keySpec, cardName, readerName, containerName, cspName);
    auto ber_credentials_header = BER::mkMandatoryOctetStringFieldHeader(ber_credentials.size(), 1);

    // TSCredentials (SEQUENCE)
    auto inner_size = ber_credtype_field.size() + ber_credentials_header.size() + ber_credentials.size();
    auto sequence_header = BER::mkSequenceHeader(inner_size);

    std::vector<uint8_t> result = std::move(sequence_header);
    result << ber_credtype_field
           << ber_credentials_header << ber_credentials;

    if (verbose) {
        LOG(LOG_INFO, "emitTSCredentialsSmartCard full dump ------------");
        hexdump_d(result);
        LOG(LOG_INFO, "emitTSCredentialsSmartCard hexdump done----------");
    }

    return result;
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
    self.supportedMech = BER::read_optional_octet_string(stream, 1, "NegTokenInit::mechToken");

    /* mechToken [2] OCTET STRING  OPTIONAL, */
    self.responseToken = BER::read_optional_octet_string(stream, 2, "NegTokenInit::responseToken");

    /* mechListMIC [3] OCTET STRING  OPTIONAL, */
    self.mechListMic = BER::read_optional_octet_string(stream, 3, "NegTokenInit::mechListMIC");

    /* } */
    return self;
}


std::vector<uint8_t> emitMechTokensEnvelop(const std::vector<uint8_t> & mechTokens) {
    size_t cumulatedSize = mechTokens.size();
    std::vector<uint8_t> octetStringHeader = BER::mkOctetStringHeader(cumulatedSize);

    cumulatedSize += octetStringHeader.size();
    std::vector<uint8_t> innerCtxHeader = BER::mkContextualFieldHeader(cumulatedSize, 0);

    cumulatedSize += innerCtxHeader.size();
    std::vector<uint8_t> innerSeqHeader = BER::mkSequenceHeader(cumulatedSize);

    cumulatedSize += innerSeqHeader.size();
    std::vector<uint8_t> topSeqHeader = BER::mkSequenceHeader(cumulatedSize);

    std::vector<uint8_t> ret;
    ret << topSeqHeader
            << innerSeqHeader
               << innerCtxHeader
               << octetStringHeader << mechTokens;

    return ret;
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
    //  -- REQUIRED in the first reply from the target
    std::vector<uint8_t>  ber_negState;
    if (negState != SPNEGO_STATE_INVALID)
        ber_negState = BER::mkEnumeratedField(negState, 0);

    // supportedMech   [1] MechType      OPTIONAL,
    // -- present only in the first reply from the target
    std::vector<uint8_t> ber_supportedMech;
    bytes_view mechOidString = KnownOidHelper::oidData(supportedMech);
    if (mechOidString.size())
        ber_supportedMech = BER::mkOidField(mechOidString, 1);

    // responseToken   [2] OCTET STRING  OPTIONAL,
    auto ber_responseTokenHeader = BER::mkOptionalOctetStringFieldHeader(responseToken.size(), 2);

    // mechListMIC     [3] OCTET STRING  OPTIONAL,
    auto ber_mecListMIC = BER::mkOptionalOctetStringFieldHeader(mechListMIC.size(), 3);

    //
    auto inner_size = ber_negState.size() + ber_supportedMech.size() +
            ber_responseTokenHeader.size() + responseToken.size() +
            ber_mecListMIC.size() + mechListMIC.size();
    auto sequence_header = BER::mkSequenceHeader(inner_size);

    auto ctxt_header = BER::mkContextualFieldHeader(sequence_header.size() + inner_size, 1);

    std::vector<uint8_t> result;
    result  << ctxt_header
            << sequence_header
            << ber_negState
            << ber_supportedMech
            << ber_responseTokenHeader << responseToken
            << ber_mecListMIC << mechListMIC;

     if (verbose) {
         LOG(LOG_INFO, "emitNegTokenResp full dump ------------");
         hexdump_d(result);
         LOG(LOG_INFO, "emitNegTokenResp hexdump done----------");
     }

     return result;
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
