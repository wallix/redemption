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

    int sizeof_ts_request(int length) {
        length += BER::sizeof_integer(2);
        length += BER::sizeof_contextual_tag(3);
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
    long version;

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
    }

    virtual ~TSRequest() {
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
        ts_request_length = CredSSP::sizeof_ts_request(length);

        /* TSRequest */
        BER::write_sequence_tag(stream, ts_request_length);

        /* [0] version */
        BER::write_contextual_tag(stream, 0, 3, true);
        BER::write_integer(stream, this->version);

        /* [1] negoTokens (NegoData) */
	if (nego_tokens_length > 0) {
            length = nego_tokens_length;

            int sequence_length   = BER::sizeof_sequence_octet_string(nego_tokens_length);
            int sequenceof_length = BER::sizeof_sequence(sequence_length);
            int context_length    = BER::sizeof_sequence(sequenceof_length);

            length -= BER::write_contextual_tag(stream, 1, context_length, true);
            length -= BER::write_sequence_tag(stream, sequenceof_length);
            length -= BER::write_sequence_tag(stream, sequence_length);
            length -= BER::write_sequence_octet_string(stream, 0,
                                                       this->negoTokens.get_data(),
                                                       nego_tokens_length);

            // assert length == 0
        }

        /* [2] authInfo (OCTET STRING) */
	if (auth_info_length > 0) {
            length = auth_info_length;
            length -= BER::write_sequence_octet_string(stream, 2,
                                                       this->authInfo.get_data(),
                                                       auth_info_length);
            // assert length == 0
        }

        /* [3] pubKeyAuth (OCTET STRING) */
        if (pub_key_auth_length > 0) {
            length = pub_key_auth_length;
            length -= BER::write_sequence_octet_string(stream, 3,
                                                       this->pubKeyAuth.get_data(),
                                                       pub_key_auth_length);
            // assert length == 0
        }

        stream.mark_end();
    }

    int recv(Stream & stream) {
        int length;
        int status;
        uint32_t version;

        status = stream.size();

        if (status < 0) {
            LOG(LOG_ERR, "Credssp TSCredentials::recv() error: %d\n" , status);
            return -1;
        }

        /* TSRequest */
	if(!BER::read_sequence_tag(stream, length) ||
           !BER::read_contextual_tag(stream, 0, length, true) ||
           !BER::read_integer(stream, version)) {
            return -1;
	}

        /* [1] negoTokens (NegoData) */
	if (BER::read_contextual_tag(stream, 1, length, true) != false)	{
            if (!BER::read_sequence_tag(stream, length) || /* SEQUENCE OF NegoDataItem */
                !BER::read_sequence_tag(stream, length) || /* NegoDataItem */
                !BER::read_contextual_tag(stream, 0, length, true) || /* [0] negoToken */
                !BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
                !stream.in_check_rem(length)) {
                return -1;
            }

            this->negoTokens.init(length);
            this->negoTokens.out_copy_bytes(stream.p, length);
            this->negoTokens.mark_end();
            this->negoTokens.rewind();

            // // TODO
            // sspi_SecBufferAlloc(&credssp->negoToken, length);
            // Stream_Read(stream, credssp->negoToken.pvBuffer, length);
            // credssp->negoToken.cbBuffer = length;
	}

	/* [2] authInfo (OCTET STRING) */
	if (BER::read_contextual_tag(stream, 2, length, true) != false) {
            if(!BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
               !stream.in_check_rem(length)) {
                return -1;
            }

            this->authInfo.init(length);
            this->authInfo.out_copy_bytes(stream.p, length);
            this->authInfo.mark_end();
            this->authInfo.rewind();
            // // TODO
            // sspi_SecBufferAlloc(&credssp->authInfo, length);
            // Stream_Read(stream, credssp->authInfo.pvBuffer, length);
            // credssp->authInfo.cbBuffer = length;
	}

	/* [3] pubKeyAuth (OCTET STRING) */
	if (BER::read_contextual_tag(stream, 3, length, true) != false)	{
            if(!BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
               !stream.in_check_rem(length)) {
                return -1;
            }

            this->pubKeyAuth.init(length);
            this->pubKeyAuth.out_copy_bytes(stream.p, length);
            this->pubKeyAuth.mark_end();
            this->pubKeyAuth.rewind();

            // // TODO
            // sspi_SecBufferAlloc(&credssp->pubKeyAuth, length);
            // Stream_Read(stream, credssp->pubKeyAuth.pvBuffer, length);
            // credssp->pubKeyAuth.cbBuffer = length;
	}

        return 0;
    }

};


struct TSCredentials {
    int credType;
};

struct TSPasswordCreds : public TSCredentials {
    uint8_t domainName[256];
    uint8_t userName[256];
    uint8_t password[256];


    int emit(Stream & stream) {
	int size = 0;
	// int innerSize = CredSSP::sizeof_ts_password_creds(credssp);

	// /* TSPasswordCreds (SEQUENCE) */

	// size += BER::write_sequence_tag(stream, innerSize);

	// /* [0] domainName (OCTET STRING) */
	// size += BER::write_sequence_octet_string(stream, 0, (BYTE*) credssp->identity.Domain, credssp->identity.DomainLength * 2);

	// /* [1] userName (OCTET STRING) */
	// size += BER::write_sequence_octet_string(stream, 1, (BYTE*) credssp->identity.User, credssp->identity.UserLength * 2);

	// /* [2] password (OCTET STRING) */
	// size += BER::write_sequence_octet_string(stream, 2, (BYTE*) credssp->identity.Password, credssp->identity.PasswordLength * 2);

	return size;
    }


    void recv(Stream & stream) {
        int length;
        /* TSPasswordCreds (SEQUENCE) */
        BER::read_sequence_tag(stream, length);

	/* [0] domainName (OCTET STRING) */
	BER::read_contextual_tag(stream, 0, length, true);
	BER::read_octet_string_tag(stream, length);
	// credssp->identity.DomainLength = (UINT32) length;
	// credssp->identity.Domain = (UINT16*) malloc(length);
	// CopyMemory(credssp->identity.Domain, Stream_Pointer(s), credssp->identity.DomainLength);
	// Stream_Seek(stream, credssp->identity.DomainLength);
	// credssp->identity.DomainLength /= 2;

	/* [1] userName (OCTET STRING) */
	BER::read_contextual_tag(stream, 1, length, true);
	BER::read_octet_string_tag(stream, length);
	// credssp->identity.UserLength = (UINT32) length;
	// credssp->identity.User = (UINT16*) malloc(length);
	// CopyMemory(credssp->identity.User, Stream_Pointer(s), credssp->identity.UserLength);
	// Stream_Seek(stream, credssp->identity.UserLength);
	// credssp->identity.UserLength /= 2;

	/* [2] password (OCTET STRING) */
	BER::read_contextual_tag(stream, 2, length, true);
	BER::read_octet_string_tag(stream, length);
	// credssp->identity.PasswordLength = (UINT32) length;
	// credssp->identity.Password = (UINT16*) malloc(length);
	// CopyMemory(credssp->identity.Password, Stream_Pointer(s), credssp->identity.PasswordLength);
	// Stream_Seek(stream, credssp->identity.PasswordLength);
	// credssp->identity.PasswordLength /= 2;

	// credssp->identity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

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
