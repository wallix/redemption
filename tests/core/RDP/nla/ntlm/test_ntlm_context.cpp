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

#define RED_TEST_MODULE TestNtlm_context
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/nla/ntlm/ntlm_context.hpp"

#include "test_only/lcg_random.hpp"


RED_AUTO_TEST_CASE(TestNtlmContext)
{
    LCGRandom rand(0);
    LCGTime timeobj;

    NTLMContext context(false, rand, timeobj, 0x400);
    // context.init();
    context.ntlm_set_negotiate_flags();
    // context.hardcoded_tests = true;

    // NtlmNegotiateFlags ntlm_nego_flag;

    // ntlm_nego_flag.flags = context.ConfigFlags;
    // ntlm_nego_flag.flags |= NTLMSSP_NEGOTIATE_56;  // W
    // ntlm_nego_flag.flags |= NTLMSSP_NEGOTIATE_VERSION;  // T
    // ntlm_nego_flag.flags |= NTLMSSP_NEGOTIATE_LM_KEY;  // G
    // ntlm_nego_flag.flags |= NTLMSSP_NEGOTIATE_SEAL;  // E
    // ntlm_nego_flag.flags |= NTLMSSP_NEGOTIATE_OEM;  // B

    // hexdump_c((uint8_t*)&ntlm_nego_flag.flags, 4);

    uint8_t nego_string[] =
        /* 0000 */ "\x4e\x54\x4c\x4d\x53\x53\x50\x00\x01\x00\x00\x00\xb7\x82\x08\xe2"
        /* 0010 */ "\x00\x00\x00\x00\x28\x00\x00\x00\x00\x00\x00\x00\x28\x00\x00\x00"
        /* 0020 */ "\x05\x01\x28\x0a\x00\x00\x00\x0f";

    uint8_t challenge_string[] =
        /* 0000 */ "\x4e\x54\x4c\x4d\x53\x53\x50\x00\x02\x00\x00\x00\x08\x00\x08\x00"
        /* 0010 */ "\x38\x00\x00\x00\x35\x82\x8a\xe2\x26\x6e\xcd\x75\xaa\x41\xe7\x6f"
        /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x40\x00\x40\x00\x00\x00"
        /* 0030 */ "\x06\x01\xb0\x1d\x00\x00\x00\x0f\x57\x00\x49\x00\x4e\x00\x37\x00"
        /* 0040 */ "\x02\x00\x08\x00\x57\x00\x49\x00\x4e\x00\x37\x00\x01\x00\x08\x00"
        /* 0050 */ "\x57\x00\x49\x00\x4e\x00\x37\x00\x04\x00\x08\x00\x77\x00\x69\x00"
        /* 0060 */ "\x6e\x00\x37\x00\x03\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00"
        /* 0070 */ "\x07\x00\x08\x00\xa9\x8d\x9b\x1a\x6c\xb0\xcb\x01\x00\x00\x00\x00";

    InStream s(nego_string, sizeof(nego_string) - 1);
    context.NEGOTIATE_MESSAGE.recv(s);

    s = InStream(challenge_string, sizeof(challenge_string) - 1);
    context.CHALLENGE_MESSAGE.recv(s);

    const uint8_t password[] = {
        // 0x50, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
        // 0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00
        0x70, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
        0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00
    };
    const uint8_t userName[] = {
        0x75, 0x00, 0x73, 0x00, 0x65, 0x00, 0x72, 0x00,
        0x6e, 0x00, 0x61, 0x00, 0x6d, 0x00, 0x65, 0x00
    };
    // const uint8_t userUpper[] = {
    //     // 0x55, 0x00, 0x53, 0x00, 0x45, 0x00, 0x52, 0x00
    //     0x55, 0x00, 0x53, 0x00, 0x45, 0x00, 0x52, 0x00,
    //     0x4e, 0x00, 0x41, 0x00, 0x4d, 0x00, 0x45, 0x00
    // };
    const uint8_t userDomain[] = {
        // 0x44, 0x00, 0x6f, 0x00, 0x6d, 0x00, 0x61, 0x00,
        // 0x69, 0x00, 0x6e, 0x00
        0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00
    };



    context.ntlmv2_compute_response_from_challenge(make_array_view(password),
                                                   make_array_view(userName),
                                                   make_array_view(userDomain));

    auto & LmChallengeResponse = context.AUTHENTICATE_MESSAGE.LmChallengeResponse.buffer;
    RED_CHECK_MEM_C(
        make_array_view(LmChallengeResponse.get_data(), LmChallengeResponse.size()),
        /* 0000 */ "\x11\x1b\x69\x4b\xdb\x30\x53\x91\xef\x94\x8b\x20\x83\xbd\x07\x43" //..iK.0S.... ...C
        /* 0010 */ "\xb8\x6c\xda\xa6\xf0\xf6\x30\x8d"                                 //.l....0.
    );
    auto & NtChallengeResponse = context.AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer;

    RED_CHECK_MEM_C(
        make_array_view(NtChallengeResponse.get_data(), NtChallengeResponse.size()),
        "\x54\x01\x2a\xc9\x4e\x20\x30\x7d\xed\x6a\xcf\xb8\x6b\xb0\x45\xc5" //T.*.N 0}.j..k.E. !
        "\x01\x01\x00\x00\x00\x00\x00\x00\x67\x95\x0e\x5a\x4e\x56\x76\xd6" //........g..ZNVv. !
        "\xb8\x6c\xda\xa6\xf0\xf6\x30\x8d\x00\x00\x00\x00\x02\x00\x08\x00" //.l....0......... !
        "\x57\x00\x49\x00\x4e\x00\x37\x00\x01\x00\x08\x00\x57\x00\x49\x00" //W.I.N.7.....W.I. !
        "\x4e\x00\x37\x00\x04\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00" //N.7.....w.i.n.7. !
        "\x03\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x07\x00\x08\x00" //....w.i.n.7..... !
        "\xa9\x8d\x9b\x1a\x6c\xb0\xcb\x01\x00\x00\x00\x00\x00\x00\x00\x00" //....l........... !
    );
    RED_CHECK_MEM_AC(
        context.SessionBaseKey,
        "\x1b\x76\xfd\xe3\x46\x77\x60\x04\x39\x7a\x47\x8a\x60\x92\x0c\x4c"
    );

    context.ntlm_encrypt_random_session_key();

    RED_CHECK_MEM_AC(
        context.EncryptedRandomSessionKey,
        "\x5a\xcb\x6c\xba\x58\x07\xb5\xd4\xf4\x61\x65\xfb\xb0\x9a\xe7\xc6"
    );

    context.ntlm_generate_client_signing_key();
    RED_CHECK_MEM_AC(
        context.ClientSigningKey,
        /* 0000 */ "\xb6\x18\x61\x5b\xdb\x97\x6c\x62\xfd\xd5\x72\xab\x37\x24\xd1\x38" //..a[..lb..r.7$.8
    );
    context.ntlm_generate_client_sealing_key();
    RED_CHECK_MEM_AC(
        context.ClientSealingKey,
        /* 0000 */ "\x02\x46\xea\x18\xc8\xba\x71\xf3\xc1\x06\xb9\xf0\x54\x37\x44\x01" //.F....q.....T7D.
    );
    context.ntlm_generate_server_signing_key();
    RED_CHECK_MEM_AC(
        context.ServerSigningKey,
      /* 0000 */ "\x56\x66\xbd\xc3\x82\xda\xb7\x70\x08\x36\xb3\xed\xcd\x67\x8b\x5a" //Vf.....p.6...g.Z
    );
    context.ntlm_generate_server_sealing_key();
    RED_CHECK_MEM_AC(
        context.ServerSealingKey,
        /* 0000 */ "\x19\x3a\x3f\x24\x89\x27\xd3\x8b\x4b\xf5\x63\x2d\xa4\xc2\xb2\x78" //.:?$.'..K.c-...x
    );
}
RED_AUTO_TEST_CASE(TestNTOWFv2)
{
    LCGRandom rand(0);
    LCGTime timeobj;

    NTLMContext context(false, rand, timeobj);
    uint8_t buff[16];

    constexpr auto password = "Password"_av;
    constexpr auto user = "User"_av;
    constexpr auto domain = "Domain"_av;

    uint8_t upassword[password.size() * 2];
    uint8_t uuser[user.size() * 2];
    uint8_t udomain[domain.size() * 2];
    UTF8toUTF16(password, upassword, sizeof(upassword));
    UTF8toUTF16(user, uuser, sizeof(uuser));
    UTF8toUTF16(domain, udomain, sizeof(udomain));

    context.NTOWFv2(make_array_view(upassword),
                    make_array_view(uuser),
                    make_array_view(udomain),
                    make_array_view(buff));
    RED_CHECK_MEM_AC(buff, "\x0c\x86\x8a\x40\x3b\xfd\x7a\x93\xa3\x00\x1e\xf2\x2e\xf0\x2e\x3f");
}

RED_AUTO_TEST_CASE(TestSetters)
{
    LCGRandom rand(0);
    LCGTime timeobj;

    NTLMContext context(false, rand, timeobj);
    // context.init();

    auto work = "Carpe Diem"_av;
    auto spn = "Sustine et abstine"_av;

    RED_CHECK_EQUAL(context.Workstation.size(), 0);
    context.ntlm_SetContextWorkstation(work);
    RED_CHECK_EQUAL(context.Workstation.size(), work.size() * 2);
    // TODO TEST bad test
    RED_CHECK(memcmp(work.data(), context.Workstation.get_data(), work.size()+1));

    RED_CHECK_EQUAL(context.ServicePrincipalName.size(), 0);
    context.ntlm_SetContextServicePrincipalName(spn);
    RED_CHECK_EQUAL(context.ServicePrincipalName.size(), spn.size() * 2);
    // TODO TEST bad test
    RED_CHECK(memcmp(spn.data(), context.ServicePrincipalName.get_data(), spn.size()+1));

}


RED_AUTO_TEST_CASE(TestNtlmScenario)
{
    LCGRandom rand(0);
    LCGTime timeobj;

    NTLMContext client_context(false, rand, timeobj, 0x400);
    NTLMContext server_context(true, rand, timeobj, 0x400);
    const uint8_t password[] = {
        0x50, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
        0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00,
        0x70, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
        0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00
    };
    const uint8_t userName[] = {
        0x75, 0x00, 0x73, 0x00, 0x65, 0x00, 0x72, 0x00,
        0x75, 0x00, 0x73, 0x00, 0x65, 0x00, 0x72, 0x00,
        0x6e, 0x00, 0x61, 0x00, 0x6d, 0x00, 0x65, 0x00
    };
    // const uint8_t userUpper[] = {
    //     0x55, 0x00, 0x53, 0x00, 0x45, 0x00, 0x52, 0x00,
    //     0x55, 0x00, 0x53, 0x00, 0x45, 0x00, 0x52, 0x00,
    //     0x4e, 0x00, 0x41, 0x00, 0x4d, 0x00, 0x45, 0x00
    // };
    const uint8_t userDomain[] = {
        0x44, 0x00, 0x6f, 0x00, 0x6d, 0x00, 0x61, 0x00,
        0x69, 0x00, 0x6e, 0x00,
        0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00
    };

    const uint8_t workstation[] = {
        0x57, 0x00, 0x49, 0x00, 0x4e, 0x00, 0x58, 0x00, 0x50, 0x00
    };

    // Initialization
    uint8_t client_to_server_buf[65535];
    InStream in_client_to_server(client_to_server_buf);
    OutStream out_client_to_server(client_to_server_buf);
    uint8_t server_to_client_buf[65535];
    OutStream out_server_to_client(server_to_client_buf);

    bool result;

    // CLIENT BUILDS NEGOTIATE
    client_context.ntlm_set_negotiate_flags();
    client_context.NEGOTIATE_MESSAGE.negoFlags.flags = client_context.NegotiateFlags;
    if (client_context.NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
        client_context.NEGOTIATE_MESSAGE.version.ntlm_get_version_info();

    // send NEGOTIATE MESSAGE
    client_context.NEGOTIATE_MESSAGE.emit(out_client_to_server);
    server_context.NEGOTIATE_MESSAGE.recv(in_client_to_server);

    // SERVER RECV NEGOTIATE AND BUILD CHALLENGE
    result = server_context.ntlm_check_nego();
    RED_CHECK(result);
    server_context.ntlm_generate_server_challenge();
    memcpy(server_context.ServerChallenge, server_context.CHALLENGE_MESSAGE.serverChallenge, 8);
    server_context.ntlm_generate_timestamp();
    server_context.ntlm_construct_challenge_target_info();

    server_context.CHALLENGE_MESSAGE.negoFlags.flags = server_context.NegotiateFlags;
    if (server_context.NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
        server_context.CHALLENGE_MESSAGE.version.ntlm_get_version_info();

    // send CHALLENGE MESSAGE
    server_context.CHALLENGE_MESSAGE.emit(out_server_to_client);
    InStream in_server_to_client(out_server_to_client.get_bytes());
    client_context.CHALLENGE_MESSAGE.recv(in_server_to_client);

    // CLIENT RECV CHALLENGE AND BUILD AUTHENTICATE

    client_context.ntlmv2_compute_response_from_challenge(make_array_view(password),
                                                          make_array_view(userName),
                                                          make_array_view(userDomain));
    client_context.ntlm_encrypt_random_session_key();
    client_context.ntlm_generate_client_signing_key();
    client_context.ntlm_generate_client_sealing_key();
    client_context.ntlm_generate_server_signing_key();
    client_context.ntlm_generate_server_sealing_key();
    client_context.AUTHENTICATE_MESSAGE.negoFlags.flags = client_context.NegotiateFlags;

    uint32_t const flag = client_context.AUTHENTICATE_MESSAGE.negoFlags.flags;
    if (flag & NTLMSSP_NEGOTIATE_VERSION)
        client_context.AUTHENTICATE_MESSAGE.version.ntlm_get_version_info();

    if (flag & NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED) {
        auto & workstationbuff = client_context.AUTHENTICATE_MESSAGE.Workstation.buffer;
        workstationbuff.reset();
        workstationbuff.ostream.out_copy_bytes(workstation, sizeof(workstation));
        workstationbuff.mark_end();
    }

    auto & domain = client_context.AUTHENTICATE_MESSAGE.DomainName.buffer;
    domain.reset();
    domain.ostream.out_copy_bytes(userDomain, sizeof(userDomain));
    domain.mark_end();

    auto & user = client_context.AUTHENTICATE_MESSAGE.UserName.buffer;

    user.reset();
    user.ostream.out_copy_bytes(userName, sizeof(userName));
    user.mark_end();

    client_context.AUTHENTICATE_MESSAGE.version.ntlm_get_version_info();


    // send AUTHENTICATE MESSAGE
    out_client_to_server.rewind();
    client_context.AUTHENTICATE_MESSAGE.emit(out_client_to_server);
    in_client_to_server.rewind();
    server_context.AUTHENTICATE_MESSAGE.recv(in_client_to_server);

    // SERVER PROCEED RESPONSE CHECKING
    uint8_t hash[16] = {};
    server_context.hash_password(make_array_view(password), hash);

    result = server_context.ntlm_check_nt_response_from_authenticate(make_array_view(hash));
    RED_CHECK(result);
    result = server_context.ntlm_check_lm_response_from_authenticate(make_array_view(hash));
    RED_CHECK(result);
    // SERVER COMPUTE SHARED KEY WITH CLIENT
    server_context.ntlm_compute_session_base_key(make_array_view(hash));
    server_context.ntlm_decrypt_exported_session_key();

    server_context.ntlm_generate_client_signing_key();
    server_context.ntlm_generate_client_sealing_key();
    server_context.ntlm_generate_server_signing_key();
    server_context.ntlm_generate_server_sealing_key();

    RED_CHECK_MEM_AA(server_context.SessionBaseKey, client_context.SessionBaseKey);
    RED_CHECK_MEM_AA(server_context.ExportedSessionKey, client_context.ExportedSessionKey);
    RED_CHECK_MEM_AA(server_context.ClientSigningKey, client_context.ClientSigningKey);
    RED_CHECK_MEM_AA(server_context.ClientSealingKey, client_context.ClientSealingKey);
    RED_CHECK_MEM_AA(server_context.ServerSigningKey, client_context.ServerSigningKey);
    RED_CHECK_MEM_AA(server_context.ServerSealingKey, client_context.ServerSealingKey);
}


RED_AUTO_TEST_CASE(TestNtlmScenario2)
{
    LCGRandom rand(0);
    LCGTime timeobj;

    NTLMContext client_context(false, rand, timeobj);
    NTLMContext server_context(true, rand, timeobj);

    const uint8_t password[] = {
        0x50, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
        0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00,
        0x70, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
        0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00
    };
    uint8_t userName[] = {
        0x75, 0x00, 0x73, 0x00, 0x65, 0x00, 0x72, 0x00,
        0x6e, 0x00, 0x61, 0x00, 0x6d, 0x00, 0x65, 0x00,
        0x6e, 0x00, 0x61, 0x00, 0x6d, 0x00, 0x65, 0x00
    };
    const uint8_t userDomain[] = {
        0x44, 0x00, 0x6f, 0x00, 0x6d, 0x00, 0x61, 0x00,
        0x69, 0x00, 0x6e, 0x00,
        0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00
    };

    const uint8_t workstation[] = {
        0x57, 0x00, 0x49, 0x00, 0x4e, 0x00, 0x58, 0x00, 0x50, 0x00
    };

    // Initialization
    uint8_t client_to_server_buf[65535];
    OutStream out_client_to_server(client_to_server_buf);
    uint8_t server_to_client_buf[65535];
    OutStream out_server_to_client(server_to_client_buf);

    // CLIENT BUILDS NEGOTIATE
    client_context.ntlm_client_build_negotiate();

    // send NEGOTIATE MESSAGE
    client_context.NEGOTIATE_MESSAGE.emit(out_client_to_server);

    client_context.SavedNegotiateMessage.init(out_client_to_server.get_offset());
    memcpy(client_context.SavedNegotiateMessage.get_data(),
           out_client_to_server.get_data(), out_client_to_server.get_offset());

    InStream in_client_to_server(out_client_to_server.get_bytes());
    server_context.NEGOTIATE_MESSAGE.recv(in_client_to_server);
    server_context.SavedNegotiateMessage.init(in_client_to_server.get_offset());
    memcpy(server_context.SavedNegotiateMessage.get_data(),
           in_client_to_server.get_data(), in_client_to_server.get_offset());
    // SERVER RECV NEGOTIATE AND BUILD CHALLENGE
    server_context.ntlm_server_build_challenge();

    // send CHALLENGE MESSAGE
    server_context.CHALLENGE_MESSAGE.emit(out_server_to_client);
    server_context.SavedChallengeMessage.init(out_server_to_client.get_offset());
    memcpy(server_context.SavedChallengeMessage.get_data(),
           out_server_to_client.get_data(), out_server_to_client.get_offset());

    InStream in_server_to_client(out_server_to_client.get_bytes());
    client_context.CHALLENGE_MESSAGE.recv(in_server_to_client);
    client_context.SavedChallengeMessage.init(in_server_to_client.get_offset());
    memcpy(client_context.SavedChallengeMessage.get_data(),
           in_server_to_client.get_data(), in_server_to_client.get_offset());
    // CLIENT RECV CHALLENGE AND BUILD AUTHENTICATE

    client_context.ntlm_client_build_authenticate(make_array_view(password),
                                                  make_array_view(userName),
                                                  make_array_view(userDomain),
                                                  make_array_view(workstation));

    // send AUTHENTICATE MESSAGE
    out_client_to_server.rewind();
    /*client_context.UseMIC*/ {
        client_context.AUTHENTICATE_MESSAGE.ignore_mic = true;
        client_context.AUTHENTICATE_MESSAGE.emit(out_client_to_server);
        client_context.AUTHENTICATE_MESSAGE.ignore_mic = false;

        client_context.SavedAuthenticateMessage.init(out_client_to_server.get_offset());
        memcpy(client_context.SavedAuthenticateMessage.get_data(), out_client_to_server.get_data(),
               out_client_to_server.get_offset());
        client_context.ntlm_compute_MIC();
        memcpy(client_context.AUTHENTICATE_MESSAGE.MIC, client_context.MessageIntegrityCheck, 16);
    }
    out_client_to_server.rewind();
    client_context.AUTHENTICATE_MESSAGE.emit(out_client_to_server);
    in_client_to_server = InStream(out_client_to_server.get_bytes());
    server_context.AUTHENTICATE_MESSAGE.recv(in_client_to_server);
    if (server_context.AUTHENTICATE_MESSAGE.has_mic) {
        memset(client_to_server_buf +
               server_context.AUTHENTICATE_MESSAGE.PayloadOffset, 0, 16);
        server_context.SavedAuthenticateMessage.init(in_client_to_server.get_offset());
        memcpy(server_context.SavedAuthenticateMessage.get_data(),
               in_client_to_server.get_data(), in_client_to_server.get_offset());
    }

    // SERVER PROCEED RESPONSE CHECKING
    uint8_t hash[16] = {};
    server_context.hash_password(make_array_view(password), hash);
    server_context.ntlm_server_proceed_authenticate(hash);

    RED_CHECK_MEM_AA(server_context.SessionBaseKey, client_context.SessionBaseKey);
    RED_CHECK_MEM_AA(server_context.ExportedSessionKey, client_context.ExportedSessionKey);
    RED_CHECK_MEM_AA(server_context.ClientSigningKey, client_context.ClientSigningKey);
    RED_CHECK_MEM_AA(server_context.ClientSealingKey, client_context.ClientSealingKey);
    RED_CHECK_MEM_AA(server_context.ServerSigningKey, client_context.ServerSigningKey);
    RED_CHECK_MEM_AA(server_context.ServerSealingKey, client_context.ServerSealingKey);
    RED_CHECK_MEM_AA(client_context.MessageIntegrityCheck, server_context.MessageIntegrityCheck);
}



RED_AUTO_TEST_CASE(TestWrittersReaders)
{
    LCGRandom rand(0);
    LCGTime timeobj;

    NTLMContext context_write(false, rand, timeobj, 0x400);
    context_write.NegotiateFlags |= NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED;
    context_write.NegotiateFlags |= NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED;
    NTLMContext context_read(true, rand, timeobj, 0x400);
    SEC_STATUS status;

    Array nego;
    status = context_write.write_negotiate(nego);
    RED_CHECK_EQUAL(status, SEC_I_CONTINUE_NEEDED);
    RED_CHECK_EQUAL(context_write.state, NTLM_STATE_CHALLENGE);
    status = context_read.read_negotiate(nego.av());
    RED_CHECK_EQUAL(status, SEC_I_CONTINUE_NEEDED);
    RED_CHECK_EQUAL(context_read.state, NTLM_STATE_CHALLENGE);

    Array chal;
    status = context_write.write_challenge(chal);
    RED_CHECK_EQUAL(status, SEC_I_CONTINUE_NEEDED);
    RED_CHECK_EQUAL(context_write.state, NTLM_STATE_AUTHENTICATE);
    status = context_read.read_challenge(chal.av());
    RED_CHECK_EQUAL(status, SEC_I_CONTINUE_NEEDED);
    RED_CHECK_EQUAL(context_read.state, NTLM_STATE_AUTHENTICATE);

    Array auth;
    status = context_write.write_authenticate(auth);
    RED_CHECK_EQUAL(status, SEC_I_COMPLETE_NEEDED);
    RED_CHECK_EQUAL(context_write.state, NTLM_STATE_FINAL);
    status = context_read.read_authenticate(auth.av());
    RED_CHECK_EQUAL(status, SEC_E_LOGON_DENIED);
}
