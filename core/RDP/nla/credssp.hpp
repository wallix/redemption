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


    void ap_integer_increment_le(uint8_t * number, int size) {
	int index;

	for (index = 0; index < size; index++) {
            if (number[index] < 0xFF) {
                number[index]++;
                break;
            }
            else {
                number[index] = 0;
                continue;
            }
        }
    }

    void ap_integer_decrement_le(uint8_t * number, int size) {
	int index;

	for (index = 0; index < size; index++) {
            if (number[index] > 0) {
                number[index]--;
                break;
            }
            else {
                number[index] = 0xFF;
                continue;
            }
        }
    }

};

struct OctetString {
    uint32_t length;
    uint8_t * data;

    OctetString() {
        this->length = 0;
        this->data = NULL;
    }


    virtual ~OctetString() {
        if (this->data) {
            delete [] this->data;
        }
    }

    void alloc(uint32_t size) {
        this->data = new uint8_t[size];
        this->length = size;
    }
};



struct TSRequest {
    /* TSRequest */

    /* [0] version */
    uint32_t version;

    /* [1] negoTokens (NegoData) */
    BStream negoTokens;
    /* [2] authInfo (OCTET STRING) */
    BStream authInfo;
    /* [3] pubKeyAuth (OCTET STRING) */
    BStream pubKeyAuth;

    TSRequest()
        : version(2)
        , negoTokens(BStream())
        , authInfo(BStream())
        , pubKeyAuth(BStream())
    {
    }


    TSRequest(Stream & stream) {
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
            length = auth_info_length;
            length -= BER::write_sequence_octet_string(stream, 2,
                                                       this->authInfo.get_data(),
                                                       this->authInfo.size());
            // assert length == 0
        }

        /* [3] pubKeyAuth (OCTET STRING) */
        if (pub_key_auth_length > 0) {
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
            LOG(LOG_ERR, "Credssp TSCredentials::recv() error: %d\n" , status);
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
            LOG(LOG_INFO, "Credssp TSCredentials::recv() NEGOTOKENS\n");

            if (!BER::read_sequence_tag(stream, length) || /* SEQUENCE OF NegoDataItem */
                !BER::read_sequence_tag(stream, length) || /* NegoDataItem */
                !BER::read_contextual_tag(stream, 0, length, true) || /* [0] negoToken */
                !BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
                !stream.in_check_rem(length)) {
                return -1;
            }

            this->negoTokens.init(length);
            this->negoTokens.out_copy_bytes(stream.p, length);
            stream.in_skip_bytes(length);
            this->negoTokens.mark_end();
            this->negoTokens.rewind();
	}

	/* [2] authInfo (OCTET STRING) */
	if (BER::read_contextual_tag(stream, 2, length, true) != false) {
            LOG(LOG_INFO, "Credssp TSCredentials::recv() AUTHINFO\n");
            if(!BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
               !stream.in_check_rem(length)) {
                return -1;
            }

            this->authInfo.init(length);
            this->authInfo.out_copy_bytes(stream.p, length);
            stream.in_skip_bytes(length);
            this->authInfo.mark_end();
            this->authInfo.rewind();
	}

	/* [3] pubKeyAuth (OCTET STRING) */
	if (BER::read_contextual_tag(stream, 3, length, true) != false)	{
            LOG(LOG_INFO, "Credssp TSCredentials::recv() PUBKEYAUTH\n");
            if(!BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
               !stream.in_check_rem(length)) {
                return -1;
            }
            this->pubKeyAuth.init(length);
            this->pubKeyAuth.out_copy_bytes(stream.p, length);
            stream.in_skip_bytes(length);
            this->pubKeyAuth.mark_end();
            this->pubKeyAuth.rewind();
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
    int domainName_length;
    uint8_t userName[256];
    int userName_length;
    uint8_t password[256];
    int password_length;

    TSPasswordCreds()
        : domainName_length(0)
        , userName_length(0)
        , password_length(0)
    {

    }

    TSPasswordCreds(const uint8_t * domain, int domain_length, const uint8_t * user, int user_length, const uint8_t * pass, int pass_length) {
        memcpy(this->domainName, domain, domain_length);
        this->domainName_length = domain_length;

        memcpy(this->userName, user, user_length);
        this->userName_length = user_length;

        memcpy(this->password, pass, pass_length);
        this->password_length = pass_length;
    }

    TSPasswordCreds(Stream & stream) {
        this->recv(stream);
    }

    ~TSPasswordCreds() {}


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


    TSCredentials()
        : credType(1)
        , passCreds(TSPasswordCreds())
    {}

    TSCredentials(const uint8_t * domain, int domain_length, const uint8_t * user, int user_length, const uint8_t * pass, int pass_length)
        : credType(1)
        , passCreds(TSPasswordCreds(domain, domain_length,
                                    user, user_length,
                                    pass, pass_length))
    {

    }

    ~TSCredentials() {}

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


    void encode(Stream & ts_credentials) {
	int length;

        // should check Restricted Admin Mode and emit credentials with empty fields
	length = BER::sizeof_sequence(this->ber_sizeof());
        ts_credentials.init(length);

        this->emit(ts_credentials);
    }

    int encrypt() {
        return 0;
    }

    int decrypt() {
        return 0;
    }

};


// struct TSCspDataDetail : public TSCredentials {
//     int keySpec;
//     char cardName[256];
//     char readerName[256];
//     char containerName[256];
//     char cspName[256];

// };
// struct TSSmartCardCreds : public TSCredentials {
//     char pin[256];
//     TSCspDataDetail capData;
//     char userHint[256];
//     char domainHint[256];

// };


#endif
