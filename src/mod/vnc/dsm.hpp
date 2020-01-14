/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): David Fort

*/

#pragma once

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>

#include "utils/stream.hpp"

/**
 *	@brief UltraVNC DSM plugin
 */
class UltraDSM {
	/** @brief plug-in state */
	typedef enum {
		DSM_WAITING_CHALLENGE,
	} UltraDsmState;

	/** various plug-in constants */
	enum {
		svncInvalid					= 0x00,
		svncCipherAES				= 0x01,
		svncCipherARC4				= 0x02,
		svncCipherBlowfish			= 0x04,
		svncCipherIDEA				= 0x08,
		svncCipherCAST5				= 0x10,
		svncCipherAESCFB			= 0x20,
		svncCipher3AESOFB			= 0x40,

		svncCipherMask				= 0xFF,

		svncKey128					= 0x1000,
		svncKey192					= 0x2000,
		svncKey256					= 0x4000,
		svncKey448					= 0x8000,
		svncKey56					= 0x0100,

		svncKeyMask					= 0xFF00,

		svncClientAuthRequired		= 0x00010000,

		svncOverridePassphrase		= 0x00020000,
		svncLowKey					= 0x00040000,
		svncNewKey					= 0x00800000,
		RC4_DROP_BYTES 				= 3072,
	};


public:
	UltraDSM(char *password);

	~UltraDSM();


	bool handleChallenge(InStream &instream);
	bool getResponse(OutStream &out);

public:
	static uint32_t checkBestSupportedFlags(uint32_t dwFlags);
	static const EVP_CIPHER* getCipher(uint32_t dwFlags, int &nKeyLength);

protected:
	UltraDsmState m_state;
	char *m_password;
	RSA *m_rsa;
	int m_nRSASize;
	uint32_t m_challengeFlags;
	uint32_t m_responseFlags;
	EVP_CIPHER_CTX *m_contextVS1;
	EVP_CIPHER_CTX *m_contextSV1;
	EVP_CIPHER_CTX *m_contextVS2;
	EVP_CIPHER_CTX *m_contextSV2;
	EVP_CIPHER_CTX *m_contextVS3;
	EVP_CIPHER_CTX *m_contextSV3;
	bool m_bTriple;
};
