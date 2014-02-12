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

#ifndef _REDEMPTION_CORE_RDP_NLA_CREDSSP_HPP_
#define _REDEMPTION_CORE_RDP_NLA_CREDSSP_HPP_

#include "RDP/nla/asn1/ber.hpp"
#include "RDP/nla/sspi.hpp"

#include "RDP/nla/ntlm/ntlm_message_challenge.hpp"
/**
 * TSRequest ::= SEQUENCE {
 * 	version    [0] INTEGER,
 * 	negoTokens [1] NegoData OPTIONAL,
 * 	authInfo   [2] OCTET STRING OPTIONAL,
 * 	pubKeyAuth [3] OCTET STRING OPTIONAL
 * }
 *
 * NegoData ::= SEQUENCE OF NegoDataItem
 *
 * NegoDataItem ::= SEQUENCE {
 * 	negoToken [0] OCTET STRING
 * }
 *
 * TSCredentials ::= SEQUENCE {
 * 	credType    [0] INTEGER,
 * 	credentials [1] OCTET STRING
 * }
 *
 * TSPasswordCreds ::= SEQUENCE {
 * 	domainName  [0] OCTET STRING,
 * 	userName    [1] OCTET STRING,
 * 	password    [2] OCTET STRING
 * }
 *
 * TSSmartCardCreds ::= SEQUENCE {
 * 	pin        [0] OCTET STRING,
 * 	cspData    [1] TSCspDataDetail,
 * 	userHint   [2] OCTET STRING OPTIONAL,
 * 	domainHint [3] OCTET STRING OPTIONAL
 * }
 *
 * TSCspDataDetail ::= SEQUENCE {
 * 	keySpec       [0] INTEGER,
 * 	cardName      [1] OCTET STRING OPTIONAL,
 * 	readerName    [2] OCTET STRING OPTIONAL,
 * 	containerName [3] OCTET STRING OPTIONAL,
 * 	cspName       [4] OCTET STRING OPTIONAL
 * }
 *
 */

namespace CredSSP {

    int sizeof_nego_token(int length) {
        length = BER::sizeof_octet_string(length);
        length += BER::sizeof_contextual_tag(length);
        return length;
    }

    int sizeof_nego_tokens(int length) {
        length = sizeof_nego_token(length);
        length += BER::sizeof_sequence_tag(length);
        length += BER::sizeof_sequence_tag(length);
        length += BER::sizeof_contextual_tag(length);
        return length;
    }

    int sizeof_pub_key_auth(int length) {
        length = BER::sizeof_octet_string(length);
        length += BER::sizeof_contextual_tag(length);
        return length;
    }

    int sizeof_auth_info(int length) {
        length = BER::sizeof_octet_string(length);
        length += BER::sizeof_contextual_tag(length);
        return length;
    }

};

struct TSRequest {
    /* TSRequest */

    /* [0] version */
    uint32_t version;

    /* [1] negoTokens (NegoData) */
    Array negoTokens;
    // BStream negoTokens;
    /* [2] authInfo (OCTET STRING) */
    Array authInfo;
    // BStream authInfo;
    /* [3] pubKeyAuth (OCTET STRING) */
    Array pubKeyAuth;
    // BStream pubKeyAuth;

    TSRequest()
        : version(2)
        , negoTokens(Array(0))
        , authInfo(Array(0))
        , pubKeyAuth(Array(0))
    {
    }


    TSRequest(Stream & stream)
        : negoTokens(Array(0))
        , authInfo(Array(0))
        , pubKeyAuth(Array(0))
    {
        this->recv(stream);
        // LOG(LOG_INFO, "TSRequest recv %d", res);
    }

    virtual ~TSRequest() {
    }

    int ber_sizeof(int length) {
        length += BER::sizeof_integer(2);
        length += BER::sizeof_contextual_tag(3);
        return length;
    }

    void emit(Stream & stream) {
        int length;
        int ts_request_length;
        int nego_tokens_length;
        int pub_key_auth_length;
        int auth_info_length;

        nego_tokens_length = (this->negoTokens.size() > 0)
            ? CredSSP::sizeof_nego_tokens(this->negoTokens.size())
            : 0;
        pub_key_auth_length = (this->pubKeyAuth.size() > 0)
            ? CredSSP::sizeof_pub_key_auth(this->pubKeyAuth.size())
            : 0;
        auth_info_length = (this->authInfo.size() > 0)
            ? CredSSP::sizeof_auth_info(this->authInfo.size())
            : 0;

        length = nego_tokens_length + pub_key_auth_length + auth_info_length;
        ts_request_length = this->ber_sizeof(length);

        /* TSRequest */
        BER::write_sequence_tag(stream, ts_request_length);

        /* [0] version */
        BER::write_contextual_tag(stream, 0, 3, true);
        BER::write_integer(stream, this->version);

        /* [1] negoTokens (NegoData) */
	if (nego_tokens_length > 0) {
            LOG(LOG_INFO, "Credssp: TSCredentials::emit() NegoToken");
            length = nego_tokens_length;

            int sequence_length   = BER::sizeof_sequence_octet_string(this->negoTokens.size());
            int sequenceof_length = BER::sizeof_sequence(sequence_length);
            int context_length    = BER::sizeof_sequence(sequenceof_length);

            length -= BER::write_contextual_tag(stream, 1, context_length, true);
            length -= BER::write_sequence_tag(stream, sequenceof_length);
            length -= BER::write_sequence_tag(stream, sequence_length);
            length -= BER::write_sequence_octet_string(stream, 0,
                                                       this->negoTokens.get_data(),
                                                       this->negoTokens.size());

            // assert length == 0
        }

        /* [2] authInfo (OCTET STRING) */
	if (auth_info_length > 0) {
            LOG(LOG_INFO, "Credssp: TSCredentials::emit() AuthInfo");
            length = auth_info_length;
            length -= BER::write_sequence_octet_string(stream, 2,
                                                       this->authInfo.get_data(),
                                                       this->authInfo.size());
            // assert length == 0
        }

        /* [3] pubKeyAuth (OCTET STRING) */
        if (pub_key_auth_length > 0) {
            LOG(LOG_INFO, "Credssp: TSCredentials::emit() pubKeyAuth");
            length = pub_key_auth_length;
            length -= BER::write_sequence_octet_string(stream, 3,
                                                       this->pubKeyAuth.get_data(),
                                                       this->pubKeyAuth.size());
            // assert length == 0
        }

        stream.mark_end();
    }

    int recv(Stream & stream) {
        int length;
        int status;
        // uint32_t version;

        status = stream.size();

        if (status < 0) {
            LOG(LOG_ERR, "Credssp TSCredentials::recv() error: %d" , status);
            return -1;
        }

        /* TSRequest */
	if(!BER::read_sequence_tag(stream, length) ||
           !BER::read_contextual_tag(stream, 0, length, true) ||
           !BER::read_integer(stream, this->version)) {
            return -1;
	}

        /* [1] negoTokens (NegoData) */
	if (BER::read_contextual_tag(stream, 1, length, true) != false)	{
            LOG(LOG_INFO, "Credssp TSCredentials::recv() NEGOTOKENS");

            if (!BER::read_sequence_tag(stream, length) || /* SEQUENCE OF NegoDataItem */
                !BER::read_sequence_tag(stream, length) || /* NegoDataItem */
                !BER::read_contextual_tag(stream, 0, length, true) || /* [0] negoToken */
                !BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
                !stream.in_check_rem(length)) {
                return -1;
            }

            this->negoTokens.init(length);
            stream.in_copy_bytes(this->negoTokens.get_data(), length);
	}

	/* [2] authInfo (OCTET STRING) */
	if (BER::read_contextual_tag(stream, 2, length, true) != false) {
            LOG(LOG_INFO, "Credssp TSCredentials::recv() AUTHINFO");
            if(!BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
               !stream.in_check_rem(length)) {
                return -1;
            }

            this->authInfo.init(length);
            stream.in_copy_bytes(this->authInfo.get_data(), length);
	}

	/* [3] pubKeyAuth (OCTET STRING) */
	if (BER::read_contextual_tag(stream, 3, length, true) != false)	{
            LOG(LOG_INFO, "Credssp TSCredentials::recv() PUBKEYAUTH");
            if(!BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
               !stream.in_check_rem(length)) {
                return -1;
            }
            this->pubKeyAuth.init(length);
            stream.in_copy_bytes(this->pubKeyAuth.get_data(), length);
	}

        return 0;
    }

};


/*
 * TSPasswordCreds ::= SEQUENCE {
 * 	domainName  [0] OCTET STRING,
 * 	userName    [1] OCTET STRING,
 * 	password    [2] OCTET STRING
 * }
 */
struct TSPasswordCreds {
    uint8_t domainName[256];
    size_t domainName_length;
    uint8_t userName[256];
    size_t userName_length;
    uint8_t password[256];
    size_t password_length;

    TSPasswordCreds()
        : domainName_length(0)
        , userName_length(0)
        , password_length(0)
    {

    }

    TSPasswordCreds(const uint8_t * domain, size_t domain_length, const uint8_t * user, size_t user_length, const uint8_t * pass, size_t pass_length) {
        this->domainName_length = (domain_length < sizeof(this->domainName))
            ? domain_length
            : sizeof(this->domainName);
        memcpy(this->domainName, domain, this->domainName_length);

        this->userName_length = (user_length < sizeof(this->userName))
            ? user_length
            : sizeof(this->userName);
        memcpy(this->userName, user, this->userName_length);

        this->password_length = (pass_length < sizeof(this->password))
            ? pass_length
            : sizeof(this->password);
        memcpy(this->password, pass, this->password_length);
    }

    // TSPasswordCreds(Stream & stream) {
    //     this->recv(stream);
    // }

    virtual ~TSPasswordCreds() {}


    int ber_sizeof() {
        int length = 0;
        // TO COMPLETE
        length += BER::sizeof_sequence_octet_string(domainName_length);
        length += BER::sizeof_sequence_octet_string(userName_length);
        length += BER::sizeof_sequence_octet_string(password_length);
        return length;
    }

    int emit(Stream & stream) {
	int size = 0;
	int innerSize = this->ber_sizeof();

	// /* TSPasswordCreds (SEQUENCE) */

	size += BER::write_sequence_tag(stream, innerSize);

	// /* [0] domainName (OCTET STRING) */
	size += BER::write_sequence_octet_string(stream, 0, this->domainName,
                                                 this->domainName_length);

	// /* [1] userName (OCTET STRING) */
	size += BER::write_sequence_octet_string(stream, 1, this->userName,
                                                 this->userName_length);

	// /* [2] password (OCTET STRING) */
	size += BER::write_sequence_octet_string(stream, 2, this->password,
                                                 this->password_length);

        stream.mark_end();
	return size;
    }


    void recv(Stream & stream) {
        int length = 0;
        /* TSPasswordCreds (SEQUENCE) */
        BER::read_sequence_tag(stream, length);

	/* [0] domainName (OCTET STRING) */
	BER::read_contextual_tag(stream, 0, length, true);
	BER::read_octet_string_tag(stream, length);

        this->domainName_length = length;
        stream.in_copy_bytes(this->domainName, length);

	/* [1] userName (OCTET STRING) */
	BER::read_contextual_tag(stream, 1, length, true);
	BER::read_octet_string_tag(stream, length);

        this->userName_length = length;
        stream.in_copy_bytes(this->userName, length);

	/* [2] password (OCTET STRING) */
	BER::read_contextual_tag(stream, 2, length, true);
	BER::read_octet_string_tag(stream, length);

        this->password_length = length;
        stream.in_copy_bytes(this->password, length);

    }
};

/*
 * TSCredentials ::= SEQUENCE {
 * 	credType    [0] INTEGER,
 * 	credentials [1] OCTET STRING
 * }
 */
struct TSCredentials {
    int credType;
    TSPasswordCreds passCreds;
    // For now, TSCredentials can only contains TSPasswordCreds (not TSSmartCardCreds)

    TSCredentials()
        : credType(1)
        , passCreds(TSPasswordCreds())
    {}

    TSCredentials(const uint8_t * domain, size_t domain_length, const uint8_t * user, size_t user_length, const uint8_t * pass, size_t pass_length)
        : credType(1)
        , passCreds(TSPasswordCreds(domain, domain_length,
                                    user, user_length,
                                    pass, pass_length))
    {

    }

    virtual ~TSCredentials() {}

    void set_credentials(const uint8_t * domain, int domain_length, const uint8_t * user,
                         int user_length, const uint8_t * pass, int pass_length) {
        this->passCreds = TSPasswordCreds(domain, domain_length, user, user_length, pass, pass_length);
    }

    int ber_sizeof() {
	int size = 0;
	size += BER::sizeof_integer(1);
	size += BER::sizeof_contextual_tag(BER::sizeof_integer(1));
	size += BER::sizeof_sequence_octet_string(BER::sizeof_sequence(this->passCreds.ber_sizeof()));
	return size;
    }

    int emit(Stream & ts_credentials) {
        // ts_credentials is the authInfo Stream field of TSRequest before it is sent
        // ts_credentials will not be encrypted and should be encrypted after calling emit
        int size = 0;

	int innerSize = this->ber_sizeof();
	int passwordSize;

	/* TSCredentials (SEQUENCE) */
	size += BER::write_sequence_tag(ts_credentials, innerSize);

	/* [0] credType (INTEGER) */
	size += BER::write_contextual_tag(ts_credentials, 0, BER::sizeof_integer(1), true);
	size += BER::write_integer(ts_credentials, 1);

	/* [1] credentials (OCTET STRING) */

	passwordSize = BER::sizeof_sequence(this->passCreds.ber_sizeof());

	size += BER::write_contextual_tag(ts_credentials, 1, BER::sizeof_octet_string(passwordSize), true);
	size += BER::write_octet_string_tag(ts_credentials, passwordSize);
	size += this->passCreds.emit(ts_credentials);

        ts_credentials.mark_end();
	return size;
    }


    void recv(Stream & ts_credentials) {
        // ts_credentials is decrypted and should be decrypted before calling recv
	int length;
	uint32_t integer_length;
	int ts_password_creds_length;

	/* TSCredentials (SEQUENCE) */
        BER::read_sequence_tag(ts_credentials, length);

	/* [0] credType (INTEGER) */
        BER::read_contextual_tag(ts_credentials, 0, length, true);
        BER::read_integer(ts_credentials, integer_length);

	/* [1] credentials (OCTET STRING) */
        BER::read_contextual_tag(ts_credentials, 1, length, true);
        BER::read_octet_string_tag(ts_credentials, ts_password_creds_length);

        this->passCreds.recv(ts_credentials);
    }


};


// struct TSCspDataDetail : public TSCredentials {
//     int keySpec;
//     uint8_t cardName[256];
//     uint8_t readerName[256];
//     uint8_t containerName[256];
//     uint8_t cspName[256];

// };
// struct TSSmartCardCreds : public TSCredentials {
//     uint8_t pin[256];
//     TSCspDataDetail capData;
//     uint8_t userHint[256];
//     uint8_t domainHint[256];

// };


#endif
