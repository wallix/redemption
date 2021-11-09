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

#include "utils/stream.hpp"

/**
 *    @brief UltraVNC DSM plugin
 */
class UltraDSM
{
    /** @brief plug-in state */
    // enum UltraDsmState {
    //     DSM_WAITING_CHALLENGE,
    // };

public:
    UltraDSM() = default;

    ~UltraDSM();

    bool handleChallenge(InStream &instream, uint16_t &challengeLen, uint8_t &passphraseused);
    bool getResponse(OutStream &out);
    bool encrypt(byte_ptr buffer, size_t len, writable_bytes_view & out);
    bool decrypt(const uint8_t *buffer, size_t len, writable_bytes_view out);

public:
    static uint32_t checkBestSupportedFlags(uint32_t dwFlags);
    static const EVP_CIPHER* getCipher(uint32_t dwFlags, int &nKeyLength);

private:
    // UltraDsmState m_state;
    // char *m_password;
    RSA *m_rsa = nullptr;
    int m_nRSASize = 0;
    uint32_t m_challengeFlags = 0;
    uint32_t m_responseFlags = 0;
    EVP_CIPHER_CTX *m_contextVS1 = nullptr;
    EVP_CIPHER_CTX *m_contextSV1 = nullptr;
    EVP_CIPHER_CTX *m_contextVS2 = nullptr;
    EVP_CIPHER_CTX *m_contextSV2 = nullptr;
    EVP_CIPHER_CTX *m_contextVS3 = nullptr;
    EVP_CIPHER_CTX *m_contextSV3 = nullptr;
    bool m_bTriple = false;
};
