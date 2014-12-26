const char outdata[] =
{
// connecting to 10.10.47.86:3389 |
// connection to 10.10.47.86:3389 succeeded : socket 4 |
// --------- CREATION OF MOD ------------------------ |
// Creation of new mod 'RDP' |
// ModRDPParams target_user="x" |
// ModRDPParams target_password="<hidden>" |
// ModRDPParams target_host="10.10.47.86" |
// ModRDPParams client_address="192.168.1.100" |
// ModRDPParams enable_tls=no |
// ModRDPParams enable_nla=no |
// ModRDPParams enable_krb=no |
// ModRDPParams enable_fastpath=no |
// ModRDPParams enable_mem3blt=no |
// ModRDPParams enable_bitmap_update=no |
// ModRDPParams enable_new_pointer=no |
// ModRDPParams enable_glyph_cache=no |
// ModRDPParams enable_transparent_mode=no |
// ModRDPParams output_filename="" |
// ModRDPParams persistent_key_list_transport=<(nil)> |
// ModRDPParams transparent_recorder_transport=<(nil)> |
// ModRDPParams key_flags=7 |
// ModRDPParams acl=<(nil)> |
// ModRDPParams auth_channel="" |
// ModRDPParams alternate_shell="" |
// ModRDPParams shell_working_directory="" |
// ModRDPParams target_application_account="" |
// ModRDPParams target_application_password="<hidden>" |
// ModRDPParams rdp_compression=0 |
// ModRDPParams error_message=<(nil)> |
// ModRDPParams disconnect_on_logon_user_change=no |
// ModRDPParams open_session_timeout=0 |
// ModRDPParams certificate_change_action=0 |
// ModRDPParams extra_orders= |
// ModRDPParams enable_persistent_disk_bitmap_cache=no |
// ModRDPParams enable_cache_waiting_list=no |
// ModRDPParams persist_bitmap_cache_on_disk=no |
// ModRDPParams password_printing_mode=0 |
// ModRDPParams allow_channels=<(nil)> |
// ModRDPParams deny_channels=<(nil)> |
// ModRDPParams verbose=0x000001FF |
// ModRDPParams cache_verbose=0x00000000 |
// RDP Extra orders="" |
// Remote RDP Server domain="" login="x" host="192-168-1-100" |
// Server key layout is 40c |
// mod_rdp::Early TLS Security Exchange |
// RdpNego::NEGO_STATE_INITIAL |
// RdpNego::send_x224_connection_request_pdu |
// Sending on RDP Wab Target (4) 31 bytes |
/* 0000 */ "\x03\x00\x00\x1f\x1a\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...........Cooki |
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x78\x0d\x0a"     //e: mstshash=x.. |
// Sent dumped on RDP Wab Target (4) 31 bytes |
// RdpNego::send_x224_connection_request_pdu done |
// mod_rdp::Early TLS Security Exchange |
// RdpNego::NEGO_STATE_RDP |
// RdpNego::recv_connection_confirm |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x0b"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 7 bytes |
// Recv done on RDP Wab Target (4) 7 bytes |
// /* 0000 */ "\x06\xd0\x00\x00\x00\x00\x00"                                     //....... |
// Dump done on RDP Wab Target (4) 7 bytes |
// mod_rdp::Early TLS Security Exchange |
// Sending to Server GCC User Data CS_CORE (216 bytes) |
// cs_core::version [80004] RDP 5.0, 5.1, 5.2, and 6.0 clients) |
// cs_core::desktopWidth  = 1024 |
// cs_core::desktopHeight = 768 |
// cs_core::colorDepth    = [ca01] [RNS_UD_COLOR_8BPP] superseded by postBeta2ColorDepth |
// cs_core::SASSequence   = [aa03] [Unknown] |
// cs_core::keyboardLayout= 040c |
// cs_core::clientBuild   = 2600 |
// cs_core::clientName    = 192-168-1-100 |
// cs_core::keyboardType  = [0004] IBM enhanced (101-key or 102-key) keyboard |
// cs_core::keyboardSubType      = [0000] OEM code |
// cs_core::keyboardFunctionKey  = 12 function keys |
// cs_core::imeFileName    =  |
// cs_core::postBeta2ColorDepth  = [ca01] [8 bpp] |
// cs_core::clientProductId = 1 |
// cs_core::serialNumber = 0 |
// cs_core::highColorDepth  = [0010] [16-bit 565 RGB mask] |
// cs_core::supportedColorDepths  = [0007] [24/16/15/] |
// cs_core::earlyCapabilityFlags  = [0001] |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_ERRINFO_PDU |
// cs_core::clientDigProductId=[00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 |
// cs_core::connectionType  = 0 |
// cs_core::pad1octet  = 0 |
// cs_core::serverSelectedProtocol = 0 |
// Sending to server GCC User Data CS_CLUSTER (12 bytes) |
// cs_cluster::flags [0009] |
// cs_cluster::redirectedSessionID = 0 |
// Sending to server GCC User Data CS_SECURITY (12 bytes) |
// CSSecGccUserData::encryptionMethods 3 |
// CSSecGccUserData::extEncryptionMethods 0 |
// Sending on RDP Wab Target (4) 376 bytes |
/* 0000 */ "\x03\x00\x01\x78\x02\xf0\x80\x7f\x65\x82\x01\x6c\x04\x01\x01\x04" //...x....e..l.... |
/* 0010 */ "\x01\x01\x01\x01\xff\x30\x1a\x02\x01\x22\x02\x01\x02\x02\x01\x00" //.....0..."...... |
/* 0020 */ "\x02\x01\x01\x02\x01\x00\x02\x01\x01\x02\x03\x00\xff\xff\x02\x01" //................ |
/* 0030 */ "\x02\x30\x19\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02" //.0.............. |
/* 0040 */ "\x01\x00\x02\x01\x01\x02\x02\x04\x20\x02\x01\x02\x30\x1f\x02\x03" //........ ...0... |
/* 0050 */ "\x00\xff\xff\x02\x02\xfc\x17\x02\x03\x00\xff\xff\x02\x01\x01\x02" //................ |
/* 0060 */ "\x01\x00\x02\x01\x01\x02\x03\x00\xff\xff\x02\x01\x02\x04\x82\x01" //................ |
/* 0070 */ "\x07\x00\x05\x00\x14\x7c\x00\x01\x80\xfe\x00\x08\x00\x10\x00\x01" //.....|.......... |
/* 0080 */ "\xc0\x00\x44\x75\x63\x61\x80\xf0\x01\xc0\xd8\x00\x04\x00\x08\x00" //..Duca.......... |
/* 0090 */ "\x00\x04\x00\x03\x01\xca\x03\xaa\x0c\x04\x00\x00\x28\x0a\x00\x00" //............(... |
/* 00a0 */ "\x31\x00\x39\x00\x32\x00\x2d\x00\x31\x00\x36\x00\x38\x00\x2d\x00" //1.9.2.-.1.6.8.-. |
/* 00b0 */ "\x31\x00\x2d\x00\x31\x00\x30\x00\x30\x00\x00\x00\x00\x00\x00\x00" //1.-.1.0.0....... |
/* 00c0 */ "\x04\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\xca\x01\x00" //................ |
/* 0110 */ "\x00\x00\x00\x00\x10\x00\x07\x00\x01\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0160 */ "\x04\xc0\x0c\x00\x09\x00\x00\x00\x00\x00\x00\x00\x02\xc0\x0c\x00" //................ |
/* 0170 */ "\x03\x00\x00\x00\x00\x00\x00\x00"                                 //........ |
// Sent dumped on RDP Wab Target (4) 376 bytes |
// mod_rdp::Basic Settings Exchange |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x01\x45"                                                     //..E |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 321 bytes |
// Recv done on RDP Wab Target (4) 321 bytes |
// /* 0000 */ "\x02\xf0\x80\x7f\x66\x82\x01\x39\x0a\x01\x00\x02\x01\x00\x30\x1a" //....f..9......0. |
// /* 0010 */ "\x02\x01\x22\x02\x01\x03\x02\x01\x00\x02\x01\x01\x02\x01\x00\x02" //.."............. |
// /* 0020 */ "\x01\x01\x02\x03\x00\xff\xf8\x02\x01\x02\x04\x82\x01\x13\x00\x05" //................ |
// /* 0030 */ "\x00\x14\x7c\x00\x01\x2a\x14\x76\x0a\x01\x01\x00\x01\xc0\x00\x4d" //..|..*.v.......M |
// /* 0040 */ "\x63\x44\x6e\x80\xfc\x01\x0c\x08\x00\x04\x00\x08\x00\x03\x0c\x08" //cDn............. |
// /* 0050 */ "\x00\xeb\x03\x00\x00\x02\x0c\xec\x00\x01\x00\x00\x00\x01\x00\x00" //................ |
// /* 0060 */ "\x00\x20\x00\x00\x00\xb8\x00\x00\x00\x53\x50\x66\x7f\xc1\x9d\xd7" //. .......SPf.... |
// /* 0070 */ "\x5e\xab\x02\xd1\x9c\x81\x1c\xc2\x25\x54\x03\xc8\xca\x18\x33\x86" //^.......%T....3. |
// /* 0080 */ "\x27\x09\x22\xdf\x33\x0b\x29\xab\x45\x01\x00\x00\x00\x01\x00\x00" //'.".3.).E....... |
// /* 0090 */ "\x00\x01\x00\x00\x00\x06\x00\x5c\x00\x52\x53\x41\x31\x48\x00\x00" //.........RSA1H.. |
// /* 00a0 */ "\x00\x00\x02\x00\x00\x3f\x00\x00\x00\x01\x00\x01\x00\x67\xab\x0e" //.....?.......g.. |
// /* 00b0 */ "\x6a\x9f\xd6\x2b\xa3\x32\x2f\x41\xd1\xce\xee\x61\xc3\x76\x0b\x26" //j..+.2/A...a.v.& |
// /* 00c0 */ "\x11\x70\x48\x8a\x8d\x23\x81\x95\xa0\x39\xf7\x5b\xaa\x3e\xf1\xed" //.pH..#...9.[.>.. |
// /* 00d0 */ "\xb8\xc4\xee\xce\x5f\x6a\xf5\x43\xce\x5f\x60\xca\x6c\x06\x75\xae" //...._j.C._`.l.u. |
// /* 00e0 */ "\xc0\xd6\xa4\x0c\x92\xa4\xc6\x75\xea\x64\xb2\x50\x5b\x00\x00\x00" //.......u.d.P[... |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x08\x00\x48\x00\x6a\x41\xb1\x43\xcf\x47\x6f" //.......H.jA.C.Go |
// /* 0100 */ "\xf1\xe6\xcc\xa1\x72\x97\xd9\xe1\x85\x15\xb3\xc2\x39\xa0\xa6\x26" //....r.......9..& |
// /* 0110 */ "\x1a\xb6\x49\x01\xfa\xa6\xda\x60\xd7\x45\xf7\x2c\xee\xe4\x8e\x64" //..I....`.E.,...d |
// /* 0120 */ "\x2e\x37\x49\xf0\x4c\x94\x6f\x08\xf5\x63\x4c\x56\x29\x55\x5a\x63" //.7I.L.o..cLV)UZc |
// /* 0130 */ "\x41\x2c\x20\x65\x95\x99\xb1\x15\x7c\x00\x00\x00\x00\x00\x00\x00" //A, e....|....... |
// /* 0140 */ "\x00"                                                             //. |
// Dump done on RDP Wab Target (4) 321 bytes |
// GCC::UserData tag=0c01 length=8 |
// Received from server GCC User Data SC_CORE (8 bytes) |
// sc_core::version [80004] RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 servers) |
// GCC::UserData tag=0c03 length=8 |
// server_channels_count=0 sent_channels_count=0 |
// Received from server GCC User Data SC_NET (8 bytes) |
// sc_net::MCSChannelId   = 1003 |
// sc_net::channelCount   = 0 |
// GCC::UserData tag=0c02 length=236 |
// RSA Key blob len in certificate is 92 |
// Received from server GCC User Data SC_SECURITY (236 bytes) |
// sc_security::encryptionMethod = 1 |
// sc_security::encryptionLevel  = 1 |
// sc_security::serverRandomLen  = 32 |
// sc_security::serverCertLen    = 184 |
// sc_security::dwVersion = 1 |
// sc_security::temporary = false |
// sc_security::RDP4-style encryption |
// sc_security::proprietaryCertificate::dwSigAlgId = 1 |
// sc_security::proprietaryCertificate::dwKeyAlgId = 1 |
// sc_security::proprietaryCertificate::wPublicKeyBlobType = 6 |
// sc_security::proprietaryCertificate::wPublicKeyBlobLen = 92 |
// sc_security::proprietaryCertificate::RSAPK::magic = 826364754 |
// sc_security::proprietaryCertificate::RSAPK::keylen = 72 |
// sc_security::proprietaryCertificate::RSAPK::bitlen = 512 |
// sc_security::proprietaryCertificate::RSAPK::datalen = 63 |
// mod_rdp::Channel Connection |
// Send MCS::ErectDomainRequest |
// Sending on RDP Wab Target (4) 12 bytes |
/* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x04\x01\x00\x01\x00"                 //............ |
// Sent dumped on RDP Wab Target (4) 12 bytes |
// Send MCS::AttachUserRequest |
// Sending on RDP Wab Target (4) 8 bytes |
/* 0000 */ "\x03\x00\x00\x08\x02\xf0\x80\x28"                                 //.......( |
// Sent dumped on RDP Wab Target (4) 8 bytes |
// mod_rdp::Channel Connection Attach User |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x0b"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 7 bytes |
// Recv done on RDP Wab Target (4) 7 bytes |
// /* 0000 */ "\x02\xf0\x80\x2e\x00\x00\x00"                                     //....... |
// Dump done on RDP Wab Target (4) 7 bytes |
// cjrq[0] = 1001 |
// Sending on RDP Wab Target (4) 12 bytes |
/* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xe9"                 //.......8.... |
// Sent dumped on RDP Wab Target (4) 12 bytes |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x0f"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 11 bytes |
// Recv done on RDP Wab Target (4) 11 bytes |
// /* 0000 */ "\x02\xf0\x80\x3e\x00\x00\x00\x03\xe9\x03\xe9"                     //...>....... |
// Dump done on RDP Wab Target (4) 11 bytes |
// cjcf[0] = 1001 |
// cjrq[1] = 1003 |
// Sending on RDP Wab Target (4) 12 bytes |
/* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xeb"                 //.......8.... |
// Sent dumped on RDP Wab Target (4) 12 bytes |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x0f"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 11 bytes |
// Recv done on RDP Wab Target (4) 11 bytes |
// /* 0000 */ "\x02\xf0\x80\x3e\x00\x00\x00\x03\xeb\x03\xeb"                     //...>....... |
// Dump done on RDP Wab Target (4) 11 bytes |
// cjcf[1] = 1003 |
// mod_rdp::RDP Security Commencement |
// mod_rdp::SecExchangePacket keylen=64 |
// send data request |
// Sending on RDP Wab Target (4) 94 bytes |
/* 0000 */ "\x03\x00\x00\x5e\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x50\x01\x00" //...^...d....pP.. |
/* 0010 */ "\x00\x00\x48\x00\x00\x00\xa4\xbe\x34\x2c\x91\x29\xd3\x65\x51\xf7" //..H.....4,.).eQ. |
/* 0020 */ "\xeb\xeb\xdf\x64\x31\x50\x0e\x1c\xb2\xfb\xbf\x02\x59\x28\x14\x0c" //...d1P......Y(.. |
/* 0030 */ "\x7b\x5e\x34\x95\x25\x19\xd6\xac\x5a\x5b\xf5\x90\xe5\x64\x10\x5e" //{^4.%...Z[...d.^ |
/* 0040 */ "\x77\x0c\xea\x1b\x4b\x49\x07\xd3\x02\xe6\xed\xee\xc6\x30\x2e\x2d" //w...KI.......0.- |
/* 0050 */ "\x8a\x53\x55\x1d\x28\x2a\x00\x00\x00\x00\x00\x00\x00\x00"         //.SU.(*........ |
// Sent dumped on RDP Wab Target (4) 94 bytes |
// send data request done |
// mod_rdp::Secure Settings Exchange |
// mod_rdp::send_client_info_pdu |
// Sending to server:  InfoPacket |
// InfoPacket::CodePage 0 |
// InfoPacket::flags 0x173 |
// InfoPacket::flags:INFO_MOUSE yes |
// InfoPacket::flags:INFO_DISABLECTRLALTDEL yes |
// InfoPacket::flags:INFO_AUTOLOGON yes |
// InfoPacket::flags:INFO_UNICODE yes |
// InfoPacket::flags:INFO_MAXIMIZESHELL  yes |
// InfoPacket::flags:INFO_LOGONNOTIFY yes |
// InfoPacket::flags:INFO_COMPRESSION no |
// InfoPacket::flags:CompressionTypeMask no |
// InfoPacket::flags:INFO_ENABLEWINDOWSKEY  yes |
// InfoPacket::flags:INFO_REMOTECONSOLEAUDIO no |
// InfoPacket::flags:FORCE_ENCRYPTED_CS_PDU no |
// InfoPacket::flags:INFO_RAIL no |
// InfoPacket::flags:INFO_LOGONERRORS no |
// InfoPacket::flags:INFO_MOUSE_HAS_WHEEL no |
// InfoPacket::flags:INFO_PASSWORD_IS_SC_PIN no |
// InfoPacket::flags:INFO_NOAUDIOPLAYBACK no |
// InfoPacket::flags:INFO_USING_SAVED_CREDS no |
// InfoPacket::flags:RNS_INFO_AUDIOCAPTURE no |
// InfoPacket::flags:RNS_INFO_VIDEO_DISABLE no |
// InfoPacket::cbDomain 0 |
// InfoPacket::cbUserName 2 |
// InfoPacket::cbPassword 2 |
// InfoPacket::cbAlternateShell 0 |
// InfoPacket::cbWorkingDir 0 |
// InfoPacket::Domain  |
// InfoPacket::UserName x |
// InfoPacket::Password <hidden> |
// InfoPacket::AlternateShell  |
// InfoPacket::WorkingDir  |
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2 |
// InfoPacket::ExtendedInfoPacket::cbClientAddress 28 |
// InfoPacket::ExtendedInfoPacket::clientAddress 192.168.1.100 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 64 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\Windows\System32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 0 |
// InfoPacket::ExtendedInfoPacket::performanceFlags 7 |
// InfoPacket::ExtendedInfoPacket::cbAutoReconnectLen 0 |
// InfoPacket::ExtendedInfoPacket::autoReconnectCookie  |
// InfoPacket::ExtendedInfoPacket::reserved1 0 |
// InfoPacket::ExtendedInfoPacket::reserved2 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::Bias 120 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardName GMT Standard Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMonth 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDay 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wHour 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardBias 60 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightName GMT Daylight Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMonth 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDay 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wHour 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMilliseconds 0 |
// send extended login info (RDP5-style) 9017b :x |
// Preparing sec header  InfoPacket |
// InfoPacket::CodePage 0 |
// InfoPacket::flags 0x9017b |
// InfoPacket::flags:INFO_MOUSE yes |
// InfoPacket::flags:INFO_DISABLECTRLALTDEL yes |
// InfoPacket::flags:INFO_AUTOLOGON yes |
// InfoPacket::flags:INFO_UNICODE yes |
// InfoPacket::flags:INFO_MAXIMIZESHELL  yes |
// InfoPacket::flags:INFO_LOGONNOTIFY yes |
// InfoPacket::flags:INFO_COMPRESSION no |
// InfoPacket::flags:CompressionTypeMask no |
// InfoPacket::flags:INFO_ENABLEWINDOWSKEY  yes |
// InfoPacket::flags:INFO_REMOTECONSOLEAUDIO no |
// InfoPacket::flags:FORCE_ENCRYPTED_CS_PDU no |
// InfoPacket::flags:INFO_RAIL no |
// InfoPacket::flags:INFO_LOGONERRORS yes |
// InfoPacket::flags:INFO_MOUSE_HAS_WHEEL no |
// InfoPacket::flags:INFO_PASSWORD_IS_SC_PIN no |
// InfoPacket::flags:INFO_NOAUDIOPLAYBACK yes |
// InfoPacket::flags:INFO_USING_SAVED_CREDS no |
// InfoPacket::flags:RNS_INFO_AUDIOCAPTURE no |
// InfoPacket::flags:RNS_INFO_VIDEO_DISABLE no |
// InfoPacket::cbDomain 0 |
// InfoPacket::cbUserName 2 |
// InfoPacket::cbPassword 2 |
// InfoPacket::cbAlternateShell 0 |
// InfoPacket::cbWorkingDir 0 |
// InfoPacket::Domain  |
// InfoPacket::UserName x |
// InfoPacket::Password <hidden> |
// InfoPacket::AlternateShell  |
// InfoPacket::WorkingDir  |
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2 |
// InfoPacket::ExtendedInfoPacket::cbClientAddress 28 |
// InfoPacket::ExtendedInfoPacket::clientAddress 192.168.1.100 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 64 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\Windows\System32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 0 |
// InfoPacket::ExtendedInfoPacket::performanceFlags 7 |
// InfoPacket::ExtendedInfoPacket::cbAutoReconnectLen 0 |
// InfoPacket::ExtendedInfoPacket::autoReconnectCookie  |
// InfoPacket::ExtendedInfoPacket::reserved1 0 |
// InfoPacket::ExtendedInfoPacket::reserved2 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::Bias 120 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardName GMT Standard Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMonth 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDay 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wHour 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardBias 60 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightName GMT Daylight Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMonth 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDay 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wHour 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMilliseconds 0 |
// Send data request InfoPacket |
// InfoPacket::CodePage 0 |
// InfoPacket::flags 0x9017b |
// InfoPacket::flags:INFO_MOUSE yes |
// InfoPacket::flags:INFO_DISABLECTRLALTDEL yes |
// InfoPacket::flags:INFO_AUTOLOGON yes |
// InfoPacket::flags:INFO_UNICODE yes |
// InfoPacket::flags:INFO_MAXIMIZESHELL  yes |
// InfoPacket::flags:INFO_LOGONNOTIFY yes |
// InfoPacket::flags:INFO_COMPRESSION no |
// InfoPacket::flags:CompressionTypeMask no |
// InfoPacket::flags:INFO_ENABLEWINDOWSKEY  yes |
// InfoPacket::flags:INFO_REMOTECONSOLEAUDIO no |
// InfoPacket::flags:FORCE_ENCRYPTED_CS_PDU no |
// InfoPacket::flags:INFO_RAIL no |
// InfoPacket::flags:INFO_LOGONERRORS yes |
// InfoPacket::flags:INFO_MOUSE_HAS_WHEEL no |
// InfoPacket::flags:INFO_PASSWORD_IS_SC_PIN no |
// InfoPacket::flags:INFO_NOAUDIOPLAYBACK yes |
// InfoPacket::flags:INFO_USING_SAVED_CREDS no |
// InfoPacket::flags:RNS_INFO_AUDIOCAPTURE no |
// InfoPacket::flags:RNS_INFO_VIDEO_DISABLE no |
// InfoPacket::cbDomain 0 |
// InfoPacket::cbUserName 2 |
// InfoPacket::cbPassword 2 |
// InfoPacket::cbAlternateShell 0 |
// InfoPacket::cbWorkingDir 0 |
// InfoPacket::Domain  |
// InfoPacket::UserName x |
// InfoPacket::Password <hidden> |
// InfoPacket::AlternateShell  |
// InfoPacket::WorkingDir  |
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2 |
// InfoPacket::ExtendedInfoPacket::cbClientAddress 28 |
// InfoPacket::ExtendedInfoPacket::clientAddress 192.168.1.100 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 64 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\Windows\System32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 0 |
// InfoPacket::ExtendedInfoPacket::performanceFlags 7 |
// InfoPacket::ExtendedInfoPacket::cbAutoReconnectLen 0 |
// InfoPacket::ExtendedInfoPacket::autoReconnectCookie  |
// InfoPacket::ExtendedInfoPacket::reserved1 0 |
// InfoPacket::ExtendedInfoPacket::reserved2 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::Bias 120 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardName GMT Standard Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMonth 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDay 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wHour 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardBias 60 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightName GMT Daylight Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMonth 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDay 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wHour 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMilliseconds 0 |
// send data request |
// Sending on RDP Wab Target (4) 339 bytes |
/* 0000 */ "\x03\x00\x01\x53\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\x44\x48" //...S...d....p.DH |
/* 0010 */ "\x00\x00\x00\x12\xdd\x4c\x90\x84\x1e\x0d\x32\xce\x3b\xec\xc9\xc5" //.....L....2.;... |
/* 0020 */ "\xf3\xbe\xeb\x13\x44\x3a\x25\x0e\x9f\xbd\xec\x15\x32\x35\x13\xc7" //....D:%.....25.. |
/* 0030 */ "\xad\x69\x63\x60\xd6\x70\xb4\x36\x34\xdf\x0a\xb7\xec\xc7\x6b\x49" //.ic`.p.64.....kI |
/* 0040 */ "\xc4\xcd\xb4\x42\x2c\xed\x40\x9a\xdc\x8f\xa6\x62\x9d\x38\x8e\xbb" //...B,.@....b.8.. |
/* 0050 */ "\x40\x32\x76\x95\x1d\x87\xe7\x30\x22\x90\xd7\xe5\x1d\xb6\xc9\xa9" //@2v....0"....... |
/* 0060 */ "\x9d\x0b\xb0\xf0\x34\x62\x93\xf8\x70\x1e\x26\x62\x3d\x91\x49\x5b" //....4b..p.&b=.I[ |
/* 0070 */ "\x3b\xe8\x52\x82\x94\x76\x8d\x34\x99\xc5\x49\x5f\xec\x7a\x43\x44" //;.R..v.4..I_.zCD |
/* 0080 */ "\x6e\x54\xda\xe1\xbf\x56\xcb\x52\xe0\x37\x3a\xc0\x41\x2d\xcd\x79" //nT...V.R.7:.A-.y |
/* 0090 */ "\xeb\x49\xfc\x46\xec\xb5\xbb\x04\x56\xd1\x02\xae\x19\xda\xed\xd6" //.I.F....V....... |
/* 00a0 */ "\x60\xc2\xe3\x3a\x0e\x6f\xf9\x26\x59\x78\x1a\xc8\x32\x6c\x2d\x40" //`..:.o.&Yx..2l-@ |
/* 00b0 */ "\xe3\xfd\xba\x08\xbb\x41\x62\x23\xdc\x76\xe6\xbb\x7e\x46\x4f\x52" //.....Ab#.v..~FOR |
/* 00c0 */ "\x98\x53\x03\xe8\x00\xf6\x2c\x4b\x05\x29\xd3\x39\xad\x35\x6f\x7a" //.S....,K.).9.5oz |
/* 00d0 */ "\x21\x4c\xf5\x90\x6e\x56\x08\x9e\x16\x5c\x0a\xf5\xbd\x9c\x0a\xa4" //!L..nV.......... |
/* 00e0 */ "\x8a\x8c\x4e\xbb\xcf\xe2\xf8\x37\x79\x96\xb3\x99\x04\x67\x44\xcf" //..N....7y....gD. |
/* 00f0 */ "\xc5\xb5\x75\xb2\xcb\xcd\x18\x8b\xd8\x58\x2f\xc7\xcf\xb5\x4d\x04" //..u......X/...M. |
/* 0100 */ "\x5c\x63\x64\xfd\xb5\xf6\x1c\x00\x1d\xf6\x96\x2e\x3a\xef\x13\xf3" //.cd.........:... |
/* 0110 */ "\xe9\xfc\x5e\xbb\x2b\xfb\xce\x48\xa2\xcb\xc4\xd4\x73\xd5\xa7\xfc" //..^.+..H....s... |
/* 0120 */ "\x2e\x82\x04\xb5\x45\x08\x06\x1a\x42\x26\x80\xcb\xcb\x87\xfb\x6c" //....E...B&.....l |
/* 0130 */ "\x82\xca\x17\x30\xbc\x68\x9f\xf7\xad\x1e\x19\xd5\x18\xb3\xb5\xf7" //...0.h.......... |
/* 0140 */ "\x44\xd5\x0a\x63\xfd\x7f\x20\x7a\x2d\x26\x60\xeb\xe3\xe1\xd4\xce" //D..c.. z-&`..... |
/* 0150 */ "\xdb\x27\xb4"                                                     //.'. |
// Sent dumped on RDP Wab Target (4) 339 bytes |
// send data request done |
// mod_rdp::send_client_info_pdu done |
// mod_rdp::Licensing |
// Rdp::Get license: username="x" |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x01\x51"                                                     //..Q |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 333 bytes |
// Recv done on RDP Wab Target (4) 333 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x81\x42\x80\x00\x00\x00\x01" //...h....p.B..... |
// /* 0010 */ "\x02\x3e\x01\x7b\x3c\x31\xa6\xae\xe8\x74\xf6\xb4\xa5\x03\x90\xe7" //.>.{<1...t...... |
// /* 0020 */ "\xc2\xc7\x39\xba\x53\x1c\x30\x54\x6e\x90\x05\xd0\x05\xce\x44\x18" //..9.S.0Tn.....D. |
// /* 0030 */ "\x91\x83\x81\x00\x00\x04\x00\x2c\x00\x00\x00\x4d\x00\x69\x00\x63" //.......,...M.i.c |
// /* 0040 */ "\x00\x72\x00\x6f\x00\x73\x00\x6f\x00\x66\x00\x74\x00\x20\x00\x43" //.r.o.s.o.f.t. .C |
// /* 0050 */ "\x00\x6f\x00\x72\x00\x70\x00\x6f\x00\x72\x00\x61\x00\x74\x00\x69" //.o.r.p.o.r.a.t.i |
// /* 0060 */ "\x00\x6f\x00\x6e\x00\x00\x00\x08\x00\x00\x00\x32\x00\x33\x00\x36" //.o.n.......2.3.6 |
// /* 0070 */ "\x00\x00\x00\x0d\x00\x04\x00\x01\x00\x00\x00\x03\x00\xb8\x00\x01" //................ |
// /* 0080 */ "\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x06\x00\x5c\x00\x52" //...............R |
// /* 0090 */ "\x53\x41\x31\x48\x00\x00\x00\x00\x02\x00\x00\x3f\x00\x00\x00\x01" //SA1H.......?.... |
// /* 00a0 */ "\x00\x01\x00\x01\xc7\xc9\xf7\x8e\x5a\x38\xe4\x29\xc3\x00\x95\x2d" //........Z8.)...- |
// /* 00b0 */ "\xdd\x4c\x3e\x50\x45\x0b\x0d\x9e\x2a\x5d\x18\x63\x64\xc4\x2c\xf7" //.L>PE...*].cd.,. |
// /* 00c0 */ "\x8f\x29\xd5\x3f\xc5\x35\x22\x34\xff\xad\x3a\xe6\xe3\x95\x06\xae" //.).?.5"4..:..... |
// /* 00d0 */ "\x55\x82\xe3\xc8\xc7\xb4\xa8\x47\xc8\x50\x71\x74\x29\x53\x89\x6d" //U......G.Pqt)S.m |
// /* 00e0 */ "\x9c\xed\x70\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x48\x00\xa8" //..p..........H.. |
// /* 00f0 */ "\xf4\x31\xb9\xab\x4b\xe6\xb4\xf4\x39\x89\xd6\xb1\xda\xf6\x1e\xec" //.1..K...9....... |
// /* 0100 */ "\xb1\xf0\x54\x3b\x5e\x3e\x6a\x71\xb4\xf7\x75\xc8\x16\x2f\x24\x00" //..T;^>jq..u../$. |
// /* 0110 */ "\xde\xe9\x82\x99\x5f\x33\x0b\xa9\xa6\x94\xaf\xcb\x11\xc3\xf2\xdb" //...._3.......... |
// /* 0120 */ "\x09\x42\x68\x29\x56\x58\x01\x56\xdb\x59\x03\x69\xdb\x7d\x37\x00" //.Bh)VX.V.Y.i.}7. |
// /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x0e\x00\x0e\x00\x6d" //...............m |
// /* 0140 */ "\x69\x63\x72\x6f\x73\x6f\x66\x74\x2e\x63\x6f\x6d\x00"             //icrosoft.com. |
// Dump done on RDP Wab Target (4) 333 bytes |
// Rdp::License Request |
// send data request |
// Sending on RDP Wab Target (4) 163 bytes |
/* 0000 */ "\x03\x00\x00\xa3\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x94\x80" //.......d....p... |
/* 0010 */ "\x00\x00\x00\x13\x03\x90\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02" //................ |
/* 0040 */ "\x00\x48\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //.H.............. |
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0f\x00\x02\x00\x78" //...............x |
/* 0090 */ "\x00\x10\x00\x0e\x00\x31\x39\x32\x2d\x31\x36\x38\x2d\x31\x2d\x31" //.....192-168-1-1 |
/* 00a0 */ "\x30\x30\x00"                                                     //00. |
// Sent dumped on RDP Wab Target (4) 163 bytes |
// send data request done |
// mod_rdp::Licensing |
// Rdp::Get license: username="x" |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x22"                                                     //.." |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 30 bytes |
// Recv done on RDP Wab Target (4) 30 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x14\x80\x00\x10\x00\xff\x02" //...h....p....... |
// /* 0010 */ "\x10\x00\x07\x00\x00\x00\x02\x00\x00\x00\x28\x14\x00\x00"         //..........(... |
// Dump done on RDP Wab Target (4) 30 bytes |
// Rdp::Get license status |
// /* 0000 */ 0xff, 0x02, 0x10, 0x00, 0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x28, 0x14, 0x00, 0x00,  // ............(... |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x01\x43"                                                     //..C |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 319 bytes |
// Recv done on RDP Wab Target (4) 319 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x81\x34\x08\x00\x00\x00\x4b" //...h....p.4....K |
// /* 0010 */ "\x8a\x10\x64\x02\x2b\x08\xb5\x24\x8a\x82\x0d\x68\x83\x0a\xf1\xab" //..d.+..$...h.... |
// /* 0020 */ "\x13\x59\x72\xbd\xf9\x5f\xbe\xaf\x72\xcc\x18\xe7\x1e\x65\xc9\xb9" //.Yr.._..r....e.. |
// /* 0030 */ "\xec\xe7\x2b\x99\xe4\x9c\x59\x3a\x24\xca\x31\x34\x6c\xb9\x99\xc9" //..+...Y:$.14l... |
// /* 0040 */ "\xa7\x15\x52\x16\x81\x64\xaa\x61\x7f\xfe\xad\xee\xe2\x8b\xb8\x42" //..R..d.a.......B |
// /* 0050 */ "\x23\xbe\x98\x19\xf3\x39\x1a\x16\x52\x5e\x55\x4b\xab\xc9\x6d\x8e" //#....9..R^UK..m. |
// /* 0060 */ "\x53\xb8\x28\xe8\xb5\x12\x9c\x03\x0d\x34\x23\x0a\xac\xaf\xa9\x2a" //S.(......4#....* |
// /* 0070 */ "\xf4\x0c\xa4\x2c\x4e\x57\x64\x79\xb9\xdb\xd8\x5b\x6d\x29\x42\xd4" //...,NWdy...[m)B. |
// /* 0080 */ "\xc7\x03\x43\x4d\x2b\x29\xc7\x89\x78\xcc\xa4\x07\xd9\x11\x9c\x9e" //..CM+)..x....... |
// /* 0090 */ "\x28\x78\x42\xd4\x16\x05\x4a\x06\xe1\x64\x88\xc0\x4c\x41\xc7\xd3" //(xB...J..d..LA.. |
// /* 00a0 */ "\x7a\xc7\x9e\xc0\x3d\xe0\x56\xbd\xa5\xdb\x49\xca\x68\xcd\x5c\x0f" //z...=.V...I.h... |
// /* 00b0 */ "\x61\x35\xad\x83\x48\xc7\xf9\x6d\x71\x1b\x09\x21\x45\xc7\x4f\x3f" //a5..H..mq..!E.O? |
// /* 00c0 */ "\xe8\x2f\x3a\x31\xb8\x41\x0d\x30\xcb\xd9\x15\xa2\xb1\xa4\x6b\x7f" //./:1.A.0......k. |
// /* 00d0 */ "\x7e\xc5\xf3\x1a\xdc\x72\x36\x22\x4d\xe1\x80\x78\xaa\x2b\x42\xa9" //~....r6"M..x.+B. |
// /* 00e0 */ "\x9c\xc4\xc6\x84\x7b\x25\x18\x9d\x34\x9c\x1f\xcb\x80\x09\xf1\xcd" //....{%..4....... |
// /* 00f0 */ "\x96\x96\x7c\x15\xe6\xc2\x5f\x48\xca\x75\x5f\x4e\x1c\x1b\x49\xc4" //..|..._H.u_N..I. |
// /* 0100 */ "\xa1\x46\xeb\x4f\x3a\xae\x47\x7c\x61\xfa\x17\x56\xa9\x69\x4c\x9d" //.F.O:.G|a..V.iL. |
// /* 0110 */ "\xdf\xd5\xa3\x4a\x5c\x7c\x63\xf4\x08\x3c\x89\xe6\x82\x8e\x96\x3d" //...J.|c..<.....= |
// /* 0120 */ "\xc1\x81\xfe\x22\xfa\x04\xa4\x3e\x32\xc1\x9a\x17\x45\xbf\x0d\x9c" //..."...>2...E... |
// /* 0130 */ "\xae\x30\xce\x5f\xe6\x7d\xf1\xd9\x9c\x8a\x3a\xa1\x06\x48\xd2"     //.0._.}....:..H. |
// Dump done on RDP Wab Target (4) 319 bytes |
// LOOPING on PDUs: 296 |
// PDUTYPE_DEMANDACTIVEPDU |
// mod_rdp::process_server_caps |
// Received from server General caps (24 bytes) |
// General caps::major 1 |
// General caps::minor 3 |
// General caps::protocol 512 |
// General caps::pad2octetA 0 |
// General caps::compression type 0 |
// General caps::extra flags 1 |
// General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED yes |
// General caps::extraflags:LONG_CREDENTIALS_SUPPORTED no |
// General caps::extraflags:AUTORECONNECT_SUPPORTED no |
// General caps::extraflags:ENC_SALTED_CHECKSUM no |
// General caps::extraflags:NO_BITMAP_COMPRESSION_HDR no |
// General caps::updateCapability 0 |
// General caps::remoteUnshare 0 |
// General caps::compressionLevel 0 |
// General caps::refreshRectSupport 0 |
// General caps::suppressOutputSupport 0 |
// Received from server Bitmap caps (28 bytes) |
// Bitmap caps::preferredBitsPerPixel 16 |
// Bitmap caps::receive1BitPerPixel 1 |
// Bitmap caps::receive4BitsPerPixel 1 |
// Bitmap caps::receive8BitsPerPixel 1 |
// Bitmap caps::desktopWidth 1024 |
// Bitmap caps::desktopHeight 768 |
// Bitmap caps::pad2octets 0 |
// Bitmap caps::desktopResizeFlag 1 (yes) |
// Bitmap caps::bitmapCompressionFlag 1 yes |
// Bitmap caps::highColorFlags 0 |
// Bitmap caps::drawingFlags 8 |
// Bitmap caps::drawingFlags:DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY no |
// Bitmap caps::drawingFlags:DRAW_ALLOW_COLOR_SUBSAMPLING no |
// Bitmap caps::drawingFlags:DRAW_ALLOW_SKIP_ALPHA yes |
// Bitmap caps::multipleRectangleSupport 1 |
// Bitmap caps::pad2octetsB 0 |
// Received from server Order caps (88 bytes) |
// Order caps::terminalDescriptor 0 |
// Order caps::pad4octetsA 1000000 |
// Order caps::desktopSaveXGranularity 1 |
// Order caps::desktopSaveYGranularity 20 |
// Order caps::pad2octetsA 0 |
// Order caps::maximumOrderLevel 1 |
// Order caps::numberFonts 47 |
// Order caps::orderFlags 34 |
// Order caps::orderSupport[TS_NEG_DSTBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_PATBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_SCRBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MEMBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MEM3BLT_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex1] 0 |
// Order caps::orderSupport[UnusedIndex2] 0 |
// Order caps::orderSupport[TS_NEG_DRAWNINEGRID_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_LINETO_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex3] 1 |
// Order caps::orderSupport[TS_NEG_SAVEBITMAP_INDEX] 0 |
// Order caps::orderSupport[UnusedIndex4] 0 |
// Order caps::orderSupport[UnusedIndex5] 0 |
// Order caps::orderSupport[UnusedIndex6] 0 |
// Order caps::orderSupport[TS_NEG_MULTIDSTBLT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_MULTIPATBLT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_MULTISCRBLT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_FAST_INDEX_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_POLYGON_SC_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_POLYGON_CB_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_POLYLINE_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex7] 0 |
// Order caps::orderSupport[TS_NEG_FAST_GLYPH_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_ELLIPSE_SC_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_ELLIPSE_CB_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_INDEX_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex8] 0 |
// Order caps::orderSupport[UnusedIndex9] 0 |
// Order caps::orderSupport[UnusedIndex10] 0 |
// Order caps::orderSupport[UnusedIndex11] 0 |
// Order caps::textFlags 1697 |
// Order caps::orderSupportExFlags 0 |
// Order caps::pad4octetsB 1000000 |
// Order caps::desktopSaveSize 1000000 |
// Order caps::pad2octetsC 1 |
// Order caps::pad2octetsD 0 |
// Order caps::textANSICodePage 0 |
// Order caps::pad2octetsE 0 |
// Received from server Input caps (88 bytes) |
// Input caps::inputFlags 41 |
// Input caps::pad2octetsA 0 |
// Input caps::keyboardLayout 0 |
// Input caps::keyboardType 0 |
// Input caps::keyboardSubType 0 |
// Input caps::keyboardFunctionKey 0 |
// Input caps::imeFileName 1207414960 |
// mod_rdp::process_server_caps done |
// mod_rdp::send_confirm_active |
// Sending to server General caps (24 bytes) |
// General caps::major 1 |
// General caps::minor 3 |
// General caps::protocol 512 |
// General caps::pad2octetA 0 |
// General caps::compression type 0 |
// General caps::extra flags 40c |
// General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED no |
// General caps::extraflags:LONG_CREDENTIALS_SUPPORTED yes |
// General caps::extraflags:AUTORECONNECT_SUPPORTED yes |
// General caps::extraflags:ENC_SALTED_CHECKSUM no |
// General caps::extraflags:NO_BITMAP_COMPRESSION_HDR yes |
// General caps::updateCapability 0 |
// General caps::remoteUnshare 0 |
// General caps::compressionLevel 0 |
// General caps::refreshRectSupport 0 |
// General caps::suppressOutputSupport 0 |
// Sending to server Bitmap caps (28 bytes) |
// Bitmap caps::preferredBitsPerPixel 16 |
// Bitmap caps::receive1BitPerPixel 1 |
// Bitmap caps::receive4BitsPerPixel 1 |
// Bitmap caps::receive8BitsPerPixel 1 |
// Bitmap caps::desktopWidth 1024 |
// Bitmap caps::desktopHeight 768 |
// Bitmap caps::pad2octets 0 |
// Bitmap caps::desktopResizeFlag 1 (yes) |
// Bitmap caps::bitmapCompressionFlag 1 yes |
// Bitmap caps::highColorFlags 0 |
// Bitmap caps::drawingFlags 8 |
// Bitmap caps::drawingFlags:DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY no |
// Bitmap caps::drawingFlags:DRAW_ALLOW_COLOR_SUBSAMPLING no |
// Bitmap caps::drawingFlags:DRAW_ALLOW_SKIP_ALPHA yes |
// Bitmap caps::multipleRectangleSupport 1 |
// Bitmap caps::pad2octetsB 0 |
// Sending to server Order caps (88 bytes) |
// Order caps::terminalDescriptor 0 |
// Order caps::pad4octetsA 0 |
// Order caps::desktopSaveXGranularity 1 |
// Order caps::desktopSaveYGranularity 20 |
// Order caps::pad2octetsA 0 |
// Order caps::maximumOrderLevel 1 |
// Order caps::numberFonts 0 |
// Order caps::orderFlags 170 |
// Order caps::orderSupport[TS_NEG_DSTBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_PATBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_SCRBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MEMBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MEM3BLT_INDEX] 0 |
// Order caps::orderSupport[UnusedIndex1] 0 |
// Order caps::orderSupport[UnusedIndex2] 0 |
// Order caps::orderSupport[TS_NEG_DRAWNINEGRID_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_LINETO_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] 0 |
// Order caps::orderSupport[UnusedIndex3] 1 |
// Order caps::orderSupport[TS_NEG_SAVEBITMAP_INDEX] 0 |
// Order caps::orderSupport[UnusedIndex4] 0 |
// Order caps::orderSupport[UnusedIndex5] 1 |
// Order caps::orderSupport[UnusedIndex6] 0 |
// Order caps::orderSupport[TS_NEG_MULTIDSTBLT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_MULTIPATBLT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_MULTISCRBLT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_FAST_INDEX_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_POLYGON_SC_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_POLYGON_CB_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_POLYLINE_INDEX] 0 |
// Order caps::orderSupport[UnusedIndex7] 0 |
// Order caps::orderSupport[TS_NEG_FAST_GLYPH_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_ELLIPSE_SC_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_ELLIPSE_CB_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_INDEX_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex8] 0 |
// Order caps::orderSupport[UnusedIndex9] 0 |
// Order caps::orderSupport[UnusedIndex10] 0 |
// Order caps::orderSupport[UnusedIndex11] 0 |
// Order caps::textFlags 1697 |
// Order caps::orderSupportExFlags 4 |
// Order caps::pad4octetsB 0 |
// Order caps::desktopSaveSize 0 |
// Order caps::pad2octetsC 0 |
// Order caps::pad2octetsD 0 |
// Order caps::textANSICodePage 1252 |
// Order caps::pad2octetsE 0 |
// Sending to server BitmapCache caps (40 bytes) |
// BitmapCache caps::pad1 0 |
// BitmapCache caps::pad2 0 |
// BitmapCache caps::pad3 0 |
// BitmapCache caps::pad4 0 |
// BitmapCache caps::pad5 0 |
// BitmapCache caps::pad6 0 |
// BitmapCache caps::cache0Entries 600 |
// BitmapCache caps::cache0MaximumCellSize 512 |
// BitmapCache caps::cache1Entries 300 |
// BitmapCache caps::cache1MaximumCellSize 2048 |
// BitmapCache caps::cache2Entries 262 |
// BitmapCache caps::cache2MaximumCellSize 8192 |
// Sending to server ColorCache caps (8 bytes) |
// ColorCache caps::colorTableCacheSize 6 |
// ColorCache caps::pad2octets 0 |
// Sending to server Activation caps (12 bytes) |
// Activation caps::helpKeyFlag 0 |
// Activation caps::helpKeyIndexFlag 0 |
// Activation caps::helpExtendedKeyFlag 0 |
// Activation caps::windowManagerKeyFlag 0 |
// Sending to server ControlCaps caps (12 bytes) |
// ControlCaps caps::controlFlags 0 |
// ControlCaps caps::remoteDetachFlag 0 |
// ControlCaps caps::controlInterest 2 |
// ControlCaps caps::detachInterest 2 |
// Sending to server Pointer caps (8 bytes) |
// Pointer caps::colorPointerFlag 1 |
// Pointer caps::colorPointerCacheSize 20 |
// Sending to server Share caps (8 bytes) |
// Share caps::nodeId 0 |
// Share caps::pad2octets 0 |
// Sending to server Input caps (88 bytes) |
// Input caps::inputFlags 1 |
// Input caps::pad2octetsA 0 |
// Input caps::keyboardLayout 1033 |
// Input caps::keyboardType 4 |
// Input caps::keyboardSubType 0 |
// Input caps::keyboardFunctionKey 12 |
// Input caps::imeFileName 1207218208 |
// Sending to server SoundCaps caps (8 bytes) |
// SoundCaps caps::soundFlags 1 |
// SoundCaps caps::pad2octetsA 0 |
// Sending to server Font caps (8 bytes) |
// Font caps::fontSupportFlags 1 |
// Font caps::pad2octets 0 |
// Sending to server GlyphCache caps (52 bytes) |
// GlyphCache caps::GlyphCache[0].CacheEntries=254 |
// GlyphCache caps::GlyphCache[0].CacheMaximumCellSize=4 |
// GlyphCache caps::GlyphCache[1].CacheEntries=254 |
// GlyphCache caps::GlyphCache[1].CacheMaximumCellSize=4 |
// GlyphCache caps::GlyphCache[2].CacheEntries=254 |
// GlyphCache caps::GlyphCache[2].CacheMaximumCellSize=8 |
// GlyphCache caps::GlyphCache[3].CacheEntries=254 |
// GlyphCache caps::GlyphCache[3].CacheMaximumCellSize=8 |
// GlyphCache caps::GlyphCache[4].CacheEntries=254 |
// GlyphCache caps::GlyphCache[4].CacheMaximumCellSize=16 |
// GlyphCache caps::GlyphCache[5].CacheEntries=254 |
// GlyphCache caps::GlyphCache[5].CacheMaximumCellSize=32 |
// GlyphCache caps::GlyphCache[6].CacheEntries=254 |
// GlyphCache caps::GlyphCache[6].CacheMaximumCellSize=64 |
// GlyphCache caps::GlyphCache[7].CacheEntries=254 |
// GlyphCache caps::GlyphCache[7].CacheMaximumCellSize=128 |
// GlyphCache caps::GlyphCache[8].CacheEntries=254 |
// GlyphCache caps::GlyphCache[8].CacheMaximumCellSize=256 |
// GlyphCache caps::GlyphCache[9].CacheEntries=64 |
// GlyphCache caps::GlyphCache[9].CacheMaximumCellSize=2048 |
// GlyphCache caps::FragCache 16777472 |
// GlyphCache caps::GlyphSupportLevel 0 |
// GlyphCache caps::pad2octets 0 |
// Sending on RDP Wab Target (4) 436 bytes |
/* 0000 */ "\x03\x00\x01\xb4\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\xa5\x08" //.......d....p... |
/* 0010 */ "\x00\x00\x00\x0f\x3e\x15\xec\x08\x01\x2a\xfd\x95\x1f\x9f\xb1\xc9" //....>....*...... |
/* 0020 */ "\x1c\x23\xd5\xe8\x5c\x64\xc3\xda\x83\xfd\x70\x7a\xed\x0e\x8d\xe7" //.#...d....pz.... |
/* 0030 */ "\xa1\x2e\xfa\x63\x68\x09\x7c\x3a\xbf\x76\x7c\x61\xe6\xae\xfa\x6d" //...ch.|:.v|a...m |
/* 0040 */ "\xef\xeb\x0a\x99\xa8\x39\x6d\xe8\x59\x42\x57\x27\x20\xa1\xcf\xa2" //.....9m.YBW' ... |
/* 0050 */ "\x7f\xa7\x6c\x16\xfd\x4e\x71\xe6\xa4\xf8\xb2\x83\x8c\x4f\x32\xaa" //..l..Nq......O2. |
/* 0060 */ "\x53\x47\x26\xf4\x9e\x9d\x62\x04\xe4\x61\x60\x0e\x3d\x9e\x8e\x48" //SG&...b..a`.=..H |
/* 0070 */ "\x74\x5a\x32\x46\x38\x9c\x94\xb7\x8d\xe2\xee\xf9\x5a\x29\xe7\x3a" //tZ2F8.......Z).: |
/* 0080 */ "\x22\x72\x7a\x4f\xc4\x90\x07\x1c\x44\xb6\x34\x6b\x7d\x38\x96\x52" //"rzO....D.4k}8.R |
/* 0090 */ "\x4e\x07\x39\x93\x48\x0b\x18\x1b\x33\x7c\x51\xc6\x7e\xcb\xc4\x94" //N.9.H...3|Q.~... |
/* 00a0 */ "\x2f\x7a\x06\xcc\xef\x78\xea\x16\x6e\x98\x29\x48\xcd\xcd\x37\x25" ///z...x..n.)H..7% |
/* 00b0 */ "\x55\x3c\xa5\x5d\xab\xbb\x3e\xb0\x1d\x90\xf4\x4e\x97\x8c\xef\xfd" //U<.]..>....N.... |
/* 00c0 */ "\xeb\x2a\xef\xfc\x5b\xe7\x0e\x8e\xba\x56\xbe\xf4\x42\x71\x65\xaa" //.*..[....V..Bqe. |
/* 00d0 */ "\x3b\x9b\x2e\x94\x0e\x9a\x0b\xd0\x09\x98\x26\xeb\x49\x5c\x58\x2d" //;.........&.I.X- |
/* 00e0 */ "\x9c\x81\x57\x5a\x81\xf5\x7c\xa5\xbc\x26\x1f\x0d\xe4\x82\x83\x1b" //..WZ..|..&...... |
/* 00f0 */ "\xbd\x09\xb0\x07\x00\xd2\x8f\x4b\x79\xb6\x1f\x65\x6e\xdd\x96\x57" //.......Ky..en..W |
/* 0100 */ "\x4d\x9f\xa3\x8a\xfe\x09\x39\xd2\x4f\xf1\x48\x4c\x27\xb6\x8a\xdb" //M.....9.O.HL'... |
/* 0110 */ "\x17\x6c\xb1\x8e\xf9\xc5\x0e\x87\x36\xd0\xf7\xab\x39\x99\xed\x55" //.l......6...9..U |
/* 0120 */ "\xaa\x71\x0a\x33\xc6\x39\x57\x0b\x1b\xd1\xe3\x9f\xee\xe4\x39\xe6" //.q.3.9W.......9. |
/* 0130 */ "\xca\x89\x2a\xa4\xbd\xfb\x38\xf2\x35\x22\x37\x5a\x46\x2f\x45\x09" //..*...8.5"7ZF/E. |
/* 0140 */ "\xf2\xea\x57\x7b\xe7\x0d\xee\xa6\x81\xf7\x99\x3a\xfe\x45\x63\x39" //..W{.......:.Ec9 |
/* 0150 */ "\xdf\xf0\x1b\x1c\xe4\xaf\x52\x70\xd1\x0b\xc5\xbd\x23\x87\x23\xff" //......Rp....#.#. |
/* 0160 */ "\x5f\x41\x73\x07\xbe\x05\x06\x62\x4a\x5b\xda\xa3\x6e\x85\xc0\x96" //_As....bJ[..n... |
/* 0170 */ "\x0d\xf2\xc1\x68\x1e\xe2\x46\x3b\x63\x0b\x97\xa9\xc6\xda\x43\x28" //...h..F;c.....C( |
/* 0180 */ "\x05\x4c\x63\x85\x34\xa1\x6a\x71\x5d\xaf\x4e\x65\x53\x30\xb3\xcf" //.Lc.4.jq].NeS0.. |
/* 0190 */ "\xde\x70\x4f\x7e\x6f\xb3\x7c\xd4\xc5\x98\x72\x52\x01\x97\xae\x35" //.pO~o.|...rR...5 |
/* 01a0 */ "\xa2\xa5\xc3\xd0\x2e\x7f\x0c\x1f\xa5\xb1\xc7\x12\xa6\x43\x8b\x8c" //.............C.. |
/* 01b0 */ "\x95\xa9\x4a\x1a"                                                 //..J. |
// Sent dumped on RDP Wab Target (4) 436 bytes |
// mod_rdp::send_confirm_active done |
// Waiting for answer to confirm active |
// mod_rdp::send_synchronise |
// Sending on RDP Wab Target (4) 48 bytes |
/* 0000 */ "\x03\x00\x00\x30\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x22\x08\x00" //...0...d....p".. |
/* 0010 */ "\x00\x00\x27\xf4\xec\x82\xb2\x3a\xbb\x4d\x17\xb1\x7d\xbe\x82\x27" //..'....:.M..}..' |
/* 0020 */ "\xc1\xa2\x84\xaa\x0c\xef\x99\x8f\x80\x69\x00\xb9\xd2\xaa\xf1\xef" //.........i...... |
// Sent dumped on RDP Wab Target (4) 48 bytes |
// mod_rdp::send_synchronise done |
// mod_rdp::send_control |
// Sending on RDP Wab Target (4) 52 bytes |
/* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x26\x08\x00" //...4...d....p&.. |
/* 0010 */ "\x00\x00\x50\x48\x19\x37\x31\xb6\xae\xeb\xa3\xe0\xfe\xca\x11\xe7" //..PH.71......... |
/* 0020 */ "\x7e\x0d\x28\x44\xf4\x94\xd4\xb4\xea\xa9\x9d\xb9\x25\x3e\x34\x53" //~.(D........%>4S |
/* 0030 */ "\xf4\x04\x06\x8c"                                                 //.... |
// Sent dumped on RDP Wab Target (4) 52 bytes |
// mod_rdp::send_control done |
// mod_rdp::send_control |
// Sending on RDP Wab Target (4) 52 bytes |
/* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x26\x08\x00" //...4...d....p&.. |
/* 0010 */ "\x00\x00\xac\x2d\x2b\x62\x76\xb8\x06\x8a\x5e\x9d\x95\xcf\xb4\xbb" //...-+bv...^..... |
/* 0020 */ "\xf3\xf9\x83\xe2\xa7\x50\xab\x1c\x57\xbf\x96\x94\xc3\x7b\x92\x40" //.....P..W....{.@ |
/* 0030 */ "\x80\x28\x37\x0e"                                                 //.(7. |
// Sent dumped on RDP Wab Target (4) 52 bytes |
// mod_rdp::send_control done |
// use rdp5 |
// mod_rdp::send_fonts |
// Sending on RDP Wab Target (4) 52 bytes |
/* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x26\x08\x00" //...4...d....p&.. |
/* 0010 */ "\x00\x00\xba\x2f\xdc\x17\xee\x50\x08\xc9\x36\x54\x65\x3e\x50\xce" //.../...P..6Te>P. |
/* 0020 */ "\xc0\x62\x9f\xa3\x70\x0e\x20\x3d\x7b\x09\x4b\xbb\xa3\x2c\x7d\xd7" //.b..p. ={.K..,}. |
/* 0030 */ "\xe0\xe4\x32\xce"                                                 //..2. |
// Sent dumped on RDP Wab Target (4) 52 bytes |
// mod_rdp::send_fonts done |
// mod_rdp::send_input_slowpath |
// Sending on RDP Wab Target (4) 60 bytes |
/* 0000 */ "\x03\x00\x00\x3c\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x2e\x08\x00" //...<...d....p... |
/* 0010 */ "\x00\x00\x1a\x23\xc5\xad\xf9\xea\x5e\x29\xa4\xbc\xb2\x1f\x9a\xae" //...#....^)...... |
/* 0020 */ "\xe3\xd1\x56\xef\xf3\x45\xce\x7c\x0d\xee\xf2\xb3\x7a\x01\x5a\x13" //..V..E.|....z.Z. |
/* 0030 */ "\x09\x2b\x95\x47\x9e\x12\xc9\xde\x0b\x3d\x15\xe4"                 //.+.G.....=.. |
// Sent dumped on RDP Wab Target (4) 60 bytes |
// mod_rdp::send_input_slowpath done |
// Resizing to 1024x768x16 |
// --------- FRONT ------------------------ |
// server_resize(width=1024, height=768, bpp=16 |
// ======================================== |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x30"                                                     //..0 |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 44 bytes |
// Recv done on RDP Wab Target (4) 44 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x22\x08\x00\x00\x00\x27\xf4" //...h....p"....'. |
// /* 0010 */ "\xec\x82\xb2\x3a\xbb\x4d\xb4\xa6\x96\xc7\x7f\x91\x41\xee\xb4\xd6" //...:.M......A... |
// /* 0020 */ "\x4f\xb5\xa9\xf7\x85\x26\x2c\x34\xb9\x05\x68\x51"                 //O....&,4..hQ |
// Dump done on RDP Wab Target (4) 44 bytes |
// LOOPING on PDUs: 22 |
// PDUTYPE_DATAPDU |
// WAITING_SYNCHRONIZE |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x34"                                                     //..4 |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 48 bytes |
// Recv done on RDP Wab Target (4) 48 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x26\x08\x00\x00\x00\xa4\x40" //...h....p&.....@ |
// /* 0010 */ "\xbb\x02\xb9\xc2\x91\x89\x6d\xc9\x44\xd8\x6e\x3a\x18\xbc\xc4\xee" //......m.D.n:.... |
// /* 0020 */ "\x04\x0d\x46\x52\xd5\x6d\x0b\x1f\xcf\xec\x83\x71\xff\xd0\xe6\xb9" //..FR.m.....q.... |
// Dump done on RDP Wab Target (4) 48 bytes |
// LOOPING on PDUs: 26 |
// PDUTYPE_DATAPDU |
// WAITING_CTL_COOPERATE |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x34"                                                     //..4 |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 48 bytes |
// Recv done on RDP Wab Target (4) 48 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x26\x08\x00\x00\x00\x26\x6a" //...h....p&....&j |
// /* 0010 */ "\x56\x5b\x5c\x3c\xb0\x04\x7d\x14\x97\x9a\xab\x01\x43\xa5\x61\x71" //V[.<..}.....C.aq |
// /* 0020 */ "\x6a\x9d\xb9\x6b\xda\xb7\x71\x80\x0e\xff\x47\xce\x65\xba\xe6\x7d" //j..k..q...G.e..} |
// Dump done on RDP Wab Target (4) 48 bytes |
// LOOPING on PDUs: 26 |
// PDUTYPE_DATAPDU |
// WAITING_GRANT_CONTROL_COOPERATE |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\xd9"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 213 bytes |
// Recv done on RDP Wab Target (4) 213 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x80\xca\x08\x00\x00\x00\x3a" //...h....p......: |
// /* 0010 */ "\x1d\xea\xe8\xfa\xf9\x93\x66\x9e\x8e\xc1\x08\x21\x3f\x6d\xd4\x22" //......f....!?m." |
// /* 0020 */ "\x95\x10\xea\x56\x46\x51\xbc\xdb\xbc\x04\xbe\xc0\xea\xd9\xf3\x6b" //...VFQ.........k |
// /* 0030 */ "\x5b\x32\x52\x50\x00\xf5\x31\xcb\x61\x50\xfa\xd7\x38\xcd\x04\xd9" //[2RP..1.aP..8... |
// /* 0040 */ "\x30\xdf\xef\xbc\x99\x8e\x39\x6c\x49\x9a\x85\xdc\xc1\x94\x63\xac" //0.....9lI.....c. |
// /* 0050 */ "\x10\xc5\x90\xc6\x27\xca\x39\x26\x62\xd0\x9b\xf9\xb2\xd9\xa9\xcb" //....'.9&b....... |
// /* 0060 */ "\x07\x73\xc7\xe1\x59\xce\x9e\xbb\x5b\xf1\x63\xb6\xeb\x1c\x15\x34" //.s..Y...[.c....4 |
// /* 0070 */ "\x65\x70\x3b\xb2\x79\xd6\xf3\x3e\xc8\x5a\xa6\x14\x9a\x4b\x53\x92" //ep;.y..>.Z...KS. |
// /* 0080 */ "\x97\xeb\x87\x0d\xdb\x51\xa9\xdd\x4d\xa8\xcb\x2b\x9c\xef\x9b\x72" //.....Q..M..+...r |
// /* 0090 */ "\x61\x7d\x5c\xd5\xe7\xec\xa6\x7c\xa6\x62\x60\x20\xdf\x51\x76\x52" //a}.....|.b` .QvR |
// /* 00a0 */ "\x53\xf9\x64\xb2\xd4\xfe\x05\x90\x44\x21\x7d\x68\xbd\x15\xbb\x31" //S.d.....D!}h...1 |
// /* 00b0 */ "\x83\x7a\x5b\x76\x8c\xe5\x47\x99\xa3\xf8\xca\x82\x04\x84\x25\x5d" //.z[v..G.......%] |
// /* 00c0 */ "\x59\x6c\xa3\xf5\x06\x40\x19\x19\x7f\x5d\xee\x2c\xfb\x0a\xba\x4c" //Yl...@...].,...L |
// /* 00d0 */ "\x9f\x70\x85\x90\xe7"                                             //.p... |
// Dump done on RDP Wab Target (4) 213 bytes |
// LOOPING on PDUs: 190 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_FONTMAP |
// mod_rdp::send_input_slowpath |
// Sending on RDP Wab Target (4) 60 bytes |
/* 0000 */ "\x03\x00\x00\x3c\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x2e\x08\x00" //...<...d....p... |
/* 0010 */ "\x00\x00\x89\x07\x58\x7c\x97\x5b\xb0\x16\x41\x26\x15\xac\x95\x9a" //....X|.[..A&.... |
/* 0020 */ "\x88\xd3\x41\xa8\x75\x36\xbd\x26\xf4\xf4\xe7\x88\xaa\xbb\x1f\xa4" //..A.u6.&........ |
/* 0030 */ "\xd6\xe3\xa8\x90\x6a\xb4\xf8\xd5\x45\xa7\x7b\x43"                 //....j...E.{C |
// Sent dumped on RDP Wab Target (4) 60 bytes |
// mod_rdp::send_input_slowpath done |
// ========= CREATION OF MOD DONE ==================== |
// ===================> count = 0 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x30"                                                     //..0 |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 44 bytes |
// Recv done on RDP Wab Target (4) 44 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x22\x08\x00\x00\x00\xcf\x26" //...h....p".....& |
// /* 0010 */ "\xea\x88\xff\x88\xb7\x15\xb5\x0e\xd7\x34\x86\x72\x9a\x96\x8b\x8b" //.........4.r.... |
// /* 0020 */ "\xdf\x79\x7c\xc0\x63\x15\x21\x81\x55\x06\x51\x4b"                 //.y|.c.!.U.QK |
// Dump done on RDP Wab Target (4) 44 bytes |
// LOOPING on PDUs: 22 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_SYNCHRONIZE |
// ===================> count = 1 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x04\x7d"                                                     //..} |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 1145 bytes |
// Recv done on RDP Wab Target (4) 1145 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x84\x6e\x08\x00\x00\x00\x5d" //...h....p.n....] |
// /* 0010 */ "\x58\xb4\x81\x3e\xc8\xe7\xdf\xb1\x23\x88\xea\xa6\xfb\x13\x48\x10" //X..>....#.....H. |
// /* 0020 */ "\x01\xcc\x59\xe3\xc3\x3c\x3a\x67\x74\xe5\x06\x67\x8f\x0a\x0f\xcc" //..Y..<:gt..g.... |
// /* 0030 */ "\x07\x8e\xae\x5a\xcd\xb5\xe7\xb7\x7a\x17\xf8\x33\x20\xa1\xde\xa7" //...Z....z..3 ... |
// /* 0040 */ "\x30\x73\x5e\x74\x7b\xf8\xf3\x71\xf1\xda\x4a\x31\x5c\x49\x90\xac" //0s^t{..q..J1.I.. |
// /* 0050 */ "\x92\x68\x2f\x73\x7b\x3b\x9b\x91\x47\x14\xa5\xd8\xf1\xcb\xa7\x18" //.h/s{;..G....... |
// /* 0060 */ "\xf5\x1c\x3f\x1a\x25\x76\x0c\x73\x7c\x19\xe5\x3b\x0c\x8f\xc6\x5e" //..?.%v.s|..;...^ |
// /* 0070 */ "\xce\x31\xfc\x04\x9c\x51\x25\x6b\x89\x0f\xfa\xad\xee\xe5\x39\x9b" //.1...Q%k......9. |
// /* 0080 */ "\x88\x67\x22\xac\x8d\xd7\x10\x1c\xb9\xdd\x2d\xb7\x93\x81\x1c\x8c" //.g".......-..... |
// /* 0090 */ "\x05\xfe\xa6\xb9\x58\x41\x46\xc0\x52\x5a\x4b\x04\x26\x07\x05\xf3" //....XAF.RZK.&... |
// /* 00a0 */ "\xdf\xbd\x2f\x15\xc3\x2b\xa7\xaf\xc5\x50\xc5\x71\x54\x45\x49\x6a" //../..+...P.qTEIj |
// /* 00b0 */ "\x64\x88\xa7\xf2\x11\x98\x1e\x8a\x2d\x88\xec\xcf\xd0\x1a\x8d\xe8" //d.......-....... |
// /* 00c0 */ "\x31\x5f\x48\xdf\x4a\x72\x14\x85\xdd\xbf\x57\xcf\x98\x5d\x61\xaf" //1_H.Jr....W..]a. |
// /* 00d0 */ "\x09\x3a\x82\x6f\xe7\x3d\xa1\x91\x90\xe8\x38\xb6\x19\xb7\x28\xc2" //.:.o.=....8...(. |
// /* 00e0 */ "\xcf\xfd\xe0\xf7\xa5\x4d\x8a\x6c\x98\x0b\x82\x20\x06\x7a\x3a\xa0" //.....M.l... .z:. |
// /* 00f0 */ "\xa0\xe5\x86\x7d\xf2\x27\x3b\x6c\xbc\x64\xb5\x3e\x51\x07\x60\xaf" //...}.';l.d.>Q.`. |
// /* 0100 */ "\x25\x05\x03\x9f\x03\x67\x75\x71\x53\xd8\x68\xe0\xe2\x46\xc4\xa9" //%....guqS.h..F.. |
// /* 0110 */ "\xb2\xbc\x3a\x3b\xb7\xb7\xfb\xe4\x3b\x8d\x3b\xb4\xcd\xe8\x7b\xef" //..:;....;.;...{. |
// /* 0120 */ "\xb7\xd3\x3e\x0b\x93\xac\x75\x84\xc3\x60\xf5\x3f\xed\xbf\x25\xd8" //..>...u..`.?..%. |
// /* 0130 */ "\xba\x15\xdc\x5d\x64\x58\x54\xf1\xb1\xbe\xdd\x31\xb9\xdc\xa2\xcf" //...]dXT....1.... |
// /* 0140 */ "\x63\x54\x4b\x09\xdf\x86\x9e\x4b\x03\x77\xa2\x30\x4c\xdf\xaa\xda" //cTK....K.w.0L... |
// /* 0150 */ "\xa9\x5a\x86\xcf\x74\x89\x04\xd3\xaf\x70\x44\x5a\x27\x1e\x7c\x8b" //.Z..t....pDZ'.|. |
// /* 0160 */ "\xda\x64\x69\x1f\xbf\x91\x60\x85\xb5\x13\xd3\xb1\xfb\x1d\x0a\xdb" //.di...`......... |
// /* 0170 */ "\x42\xa9\x94\xf7\x3c\x0c\x83\x64\xbf\x2e\x99\x01\x75\xd0\x57\xf5" //B...<..d....u.W. |
// /* 0180 */ "\x4d\x59\x7b\x6b\x3c\xb1\xb9\x4b\x61\xc8\x9a\xc2\xc1\x9c\x23\xe1" //MY{k<..Ka.....#. |
// /* 0190 */ "\x06\x24\x05\x18\x74\xfd\x39\xaa\x52\x0f\x94\xa3\x90\x32\xe0\x7b" //.$..t.9.R....2.{ |
// /* 01a0 */ "\x70\x34\x35\x8a\xf0\xfd\x48\xd3\x62\x72\x1b\xf5\x93\xeb\xb1\x8c" //p45...H.br...... |
// /* 01b0 */ "\xcb\x59\x40\x54\xd0\x77\xa3\xe4\x2e\x26\x62\x61\x9b\x0a\xef\x7c" //.Y@T.w...&ba...| |
// /* 01c0 */ "\x3e\x1a\xa2\xd9\xde\x6d\xc7\xdd\x0f\x9d\x84\x33\x43\xb6\xc3\xea" //>....m.....3C... |
// /* 01d0 */ "\x6b\x97\xa7\x45\x6f\xcb\x89\x3f\xfc\xc8\xd6\x18\xc5\x0e\x93\xd9" //k..Eo..?........ |
// /* 01e0 */ "\x76\xa9\x88\xda\xcc\x82\xae\xd0\xe2\xd6\xba\xc8\x81\x40\x83\x58" //v............@.X |
// /* 01f0 */ "\x81\x62\x52\x62\xa7\xd7\x2e\xc8\x6e\xe8\x7b\xb8\x1f\xc4\xaf\xcf" //.bRb....n.{..... |
// /* 0200 */ "\xf3\x10\x70\x03\x4a\xb0\x63\x91\x03\x33\xea\xc3\xde\x88\xd0\x27" //..p.J.c..3.....' |
// /* 0210 */ "\x4e\x3a\x30\x16\x21\x6c\x9f\x12\xa7\x8c\xa6\x7d\xd5\x41\xc2\xce" //N:0.!l.....}.A.. |
// /* 0220 */ "\x1a\x7f\x72\xa4\x41\x20\x94\xd1\xcc\xd3\x13\xec\x76\xa1\x17\xf0" //..r.A ......v... |
// /* 0230 */ "\x4c\x19\x55\xbb\x23\xad\x8d\xc5\x72\xed\x42\x3e\xaf\x3f\x9f\x2c" //L.U.#...r.B>.?., |
// /* 0240 */ "\x14\x0f\xef\x52\x58\x36\x2c\x5b\x96\x60\xf3\xf7\x05\xb9\xeb\x9b" //...RX6,[.`...... |
// /* 0250 */ "\x32\x5c\x75\x41\xf2\x4b\x85\x18\x2c\x14\xf6\xc4\x85\x82\xed\x3b" //2.uA.K..,......; |
// /* 0260 */ "\xd0\x9d\xcf\xa4\x00\xca\x63\xb5\x4c\xcc\x01\x8b\x30\x58\x48\x33" //......c.L...0XH3 |
// /* 0270 */ "\x07\xa7\xdc\x7c\xec\xb1\x3a\xf4\xeb\xcd\x82\xe3\xca\xa2\xbc\x8e" //...|..:......... |
// /* 0280 */ "\xa6\xd7\x1b\x82\x21\x27\x21\xe3\x43\x0f\x85\x82\x25\x01\x77\x8a" //....!'!.C...%.w. |
// /* 0290 */ "\x87\x74\x38\xcc\xe5\xa1\x96\xfd\x68\x6d\xaa\xea\x28\x44\xb7\x0c" //.t8.....hm..(D.. |
// /* 02a0 */ "\x84\x7a\xab\x53\x8f\x97\xa5\x13\x70\x56\xd5\x26\xc2\xcc\x24\xc6" //.z.S....pV.&..$. |
// /* 02b0 */ "\x20\x72\xb5\x2a\x68\xeb\xa7\xe4\xcf\x81\x2c\x43\x9c\x16\xe3\xc6" // r.*h.....,C.... |
// /* 02c0 */ "\xc0\x76\xf3\x25\xc2\x62\x0d\x03\xd4\x99\xa5\xa6\x9b\x53\x7e\x61" //.v.%.b.......S~a |
// /* 02d0 */ "\x67\xc5\xdc\xb5\xd2\x83\x51\xd1\x57\x98\x98\x58\x74\xb4\x48\x63" //g.....Q.W..Xt.Hc |
// /* 02e0 */ "\xd8\xf0\x28\xff\x6e\x3e\xb7\x6d\x39\xa1\x98\x6d\x37\x95\xbf\x86" //..(.n>.m9..m7... |
// /* 02f0 */ "\x16\x40\x42\xff\x1c\x9b\x2a\x07\x9b\xc9\x42\xcc\xac\xef\x87\xcc" //.@B...*...B..... |
// /* 0300 */ "\x29\x34\xf8\xe4\x56\x79\xf1\xa0\x7a\x64\x6c\x67\xf1\x69\x2c\x41" //)4..Vy..zdlg.i,A |
// /* 0310 */ "\xcb\xb1\xba\xc8\x9f\x20\x81\x18\x91\x26\x08\x8a\x20\xf8\x61\x64" //..... ...&.. .ad |
// /* 0320 */ "\xab\xe4\xf3\x86\x7b\xb8\x44\x73\xb6\x18\x8b\x28\x69\x6f\x79\xfe" //....{.Ds...(ioy. |
// /* 0330 */ "\xa1\xda\x3e\xbc\x9c\x8f\xb5\xfc\x83\xd6\x5b\x72\x1f\x6c\x66\xbf" //..>.......[r.lf. |
// /* 0340 */ "\x75\x04\xf4\x40\xc9\x6f\x8a\x97\x78\x3c\xd6\x78\x8c\x06\xca\xc5" //u..@.o..x<.x.... |
// /* 0350 */ "\x0c\x96\x35\xd6\xbe\x9a\x0e\x0d\x48\x01\xf5\xd4\xf2\xe8\x81\x2a" //..5.....H......* |
// /* 0360 */ "\xb3\x38\x14\x14\x6b\xb3\x00\xd4\x86\xd8\x9c\x52\xd4\x1d\x64\x81" //.8..k......R..d. |
// /* 0370 */ "\x00\x38\x0b\x7b\x7b\x41\x14\x4f\xff\x88\x2f\x36\x64\xb5\x3b\x78" //.8.{{A.O../6d.;x |
// /* 0380 */ "\xfd\x16\x9a\xfd\xcc\x0c\x7e\x69\xb7\x80\x1e\x49\xa5\x01\x36\x64" //......~i...I..6d |
// /* 0390 */ "\x6e\xf7\x7d\x6a\x89\xc7\x8a\x36\x20\x4a\x75\x91\xdc\x44\xad\x3a" //n.}j...6 Ju..D.: |
// /* 03a0 */ "\xf5\x60\x40\x82\xf0\xcd\x0f\x36\x9b\xb3\x33\x17\x69\x03\xa2\x1e" //.`@....6..3.i... |
// /* 03b0 */ "\xd8\x36\x48\x0f\x10\x5f\x2a\x30\x29\xd1\x76\xf1\x17\x10\xf1\x1c" //.6H.._*0).v..... |
// /* 03c0 */ "\x2e\x94\x4c\x28\x1b\xd6\x9b\x3b\xf2\x2e\x31\x00\x2b\xd7\xc0\x61" //..L(...;..1.+..a |
// /* 03d0 */ "\xce\xe9\xe6\x4f\xfd\x18\x63\x00\x72\xa8\x21\xd2\xa3\xf8\xb2\xe7" //...O..c.r.!..... |
// /* 03e0 */ "\x5a\x54\x0a\xf3\xa6\x55\xc9\x04\x52\x54\xf7\xae\x4d\xef\x00\x44" //ZT...U..RT..M..D |
// /* 03f0 */ "\xb3\x6d\x6c\x22\x1a\x54\xcd\xce\x9c\x57\x82\xb2\x71\xb2\xcb\xe7" //.ml".T...W..q... |
// /* 0400 */ "\x9c\xcf\x50\xb0\xb0\xd6\xb0\xef\xe6\x2e\xb0\x10\x1d\xca\x6a\xcc" //..P...........j. |
// /* 0410 */ "\xe3\x79\xad\x84\x7c\xf9\x3e\xeb\xad\xcd\x74\x3e\xfb\x7d\xf6\x4f" //.y..|.>...t>.}.O |
// /* 0420 */ "\xff\x2b\x16\xa5\x74\x22\x33\x91\x4a\x9a\xe1\xe4\xfe\x77\x19\x2f" //.+..t"3.J....w./ |
// /* 0430 */ "\x66\x44\x27\x73\x39\xda\x28\xec\xf7\x54\xaf\x9c\xc8\x24\x66\xe6" //fD's9.(..T...$f. |
// /* 0440 */ "\xa2\x86\x4f\x5c\xb3\xa1\x86\x30\xb0\x70\xf4\x43\xc6\x19\x77\xef" //..O....0.p.C..w. |
// /* 0450 */ "\xba\xf8\xbe\xb5\xa3\x68\xdf\x5d\x2a\xb1\x8f\x35\xe2\xcc\x65\xb8" //.....h.]*..5..e. |
// /* 0460 */ "\xe6\x9d\x40\x17\xc6\xe4\x08\x28\x1f\xe2\x83\xc6\xe5\xfc\x42\x4b" //..@....(......BK |
// /* 0470 */ "\x6d\xa9\x37\x24\x8c\x6e\x11\x66\xbc"                             //m.7$.n.f. |
// Dump done on RDP Wab Target (4) 1145 bytes |
// LOOPING on PDUs: 1122 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_ORDERS |
// process_orders bpp=16 |
// process_colormap |
// receiving colormap 0 |
// process_colormap done |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(16,63,197,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(18,64,1,15) color=0x008c51) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,62,198,1) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,63,1,18) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(213,62,1,20) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,81,199,1) color=0x000273) |
// ======================================== |
// process_orders done |
// ===================> count = 2 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x09\xa9"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 2469 bytes |
// Recv done on RDP Wab Target (4) 2469 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x89\x9a\x08\x00\x00\x00\x2b" //...h....p......+ |
// /* 0010 */ "\x18\x93\x18\x25\x96\xd7\x03\xac\x46\xa1\xb2\x81\x1b\x62\x1b\xed" //...%....F....b.. |
// /* 0020 */ "\x49\xf3\x08\x9c\x34\x90\x15\xbb\xb6\x42\xbd\xbb\x54\xf1\x8f\x8c" //I...4....B..T... |
// /* 0030 */ "\xa0\x35\x65\x73\x4f\x6f\x21\x81\x27\xfc\xd9\x39\xd9\x8e\x08\x78" //.5esOo!.'..9...x |
// /* 0040 */ "\x96\x33\x5b\x31\x13\x5b\x87\x8c\xbc\x2d\x1e\x57\x88\xc5\x50\x76" //.3[1.[...-.W..Pv |
// /* 0050 */ "\x6d\xc2\xe1\xd0\x45\xa5\x18\xef\x28\x56\xf9\xee\xb3\xcb\x55\x46" //m...E...(V....UF |
// /* 0060 */ "\x9e\x25\x58\x14\x85\x14\xda\x0d\x6e\x1b\xdc\x5c\xeb\xd9\x6c\x19" //.%X.....n.....l. |
// /* 0070 */ "\x6c\x81\xcd\x3d\x6b\x8b\x97\x7a\x8c\x29\x65\xb9\x6b\x7b\xc4\x25" //l..=k..z.)e.k{.% |
// /* 0080 */ "\xc3\x23\xa1\x0e\x16\x77\x39\x94\x5c\xee\x90\xf0\x45\xea\x62\xbf" //.#...w9.....E.b. |
// /* 0090 */ "\x19\x09\x68\x14\x33\x9c\x89\xe1\x43\x82\xce\x0b\xc8\xc7\x03\x57" //..h.3...C......W |
// /* 00a0 */ "\xe2\xd4\x8f\xbf\x3c\xe8\x11\xb6\x52\x7d\x93\x79\x37\xc8\x1d\x8a" //....<...R}.y7... |
// /* 00b0 */ "\xba\x33\x86\x60\xc9\xae\x79\x11\xea\xd2\x71\x42\x81\x32\x72\x85" //.3.`..y...qB.2r. |
// /* 00c0 */ "\x71\x7d\xe1\x9b\xc1\x99\x1a\xea\x78\x37\x63\x5a\x4c\x48\xf0\x67" //q}......x7cZLH.g |
// /* 00d0 */ "\xeb\x75\x7e\xd0\xf4\xa8\x7a\x37\x86\xfe\x0c\xfd\x84\x74\x81\x73" //.u~...z7.....t.s |
// /* 00e0 */ "\xdd\x99\x8b\x44\xf0\x11\xc8\x8f\xcd\xa0\x96\x98\x22\xb9\x0c\xbc" //...D........"... |
// /* 00f0 */ "\x31\x19\xac\x60\x2f\xa0\x1c\xa7\x20\x11\x19\xc3\x67\x63\xa8\x7e" //1..`/... ...gc.~ |
// /* 0100 */ "\x42\x0b\xf2\x2f\xd1\x80\x60\x76\x99\x9b\xa1\x4c\xd5\x3a\xb2\x14" //B../..`v...L.:.. |
// /* 0110 */ "\x6d\xee\xc4\xcc\x0e\xb1\x8c\x54\xb5\x8b\x65\x19\x6c\xf0\x17\x17" //m......T..e.l... |
// /* 0120 */ "\xfd\xc4\x70\x87\x45\x7e\x18\xe3\x4e\xe9\x2c\x12\x83\x11\x87\x1c" //..p.E~..N.,..... |
// /* 0130 */ "\x19\x95\x42\xa9\x2c\x20\x28\x6b\x8f\xc1\xa8\x14\x7a\x1b\x7f\x38" //..B., (k....z..8 |
// /* 0140 */ "\x87\x3a\x9a\x1e\xd0\x09\xbc\xc4\xa1\x78\xdd\xc2\xbc\x50\x73\xef" //.:.......x...Ps. |
// /* 0150 */ "\xfe\x62\x55\x25\xe4\x6b\xa6\xb5\xd3\x2e\x73\x0f\x14\xd7\x24\x57" //.bU%.k....s...$W |
// /* 0160 */ "\x34\xfd\x4f\xe7\xfb\xc7\xab\xcb\x0c\x1f\x62\x55\x2c\x8f\xbc\x7c" //4.O.......bU,..| |
// /* 0170 */ "\x2b\x98\x5a\x7f\x1a\x7b\xff\x5c\xc3\x2c\x43\x91\x4a\x1b\x16\xa8" //+.Z..{...,C.J... |
// /* 0180 */ "\x52\x50\x99\x5d\x99\x20\x31\x23\x69\x54\x56\x29\x57\x73\x5d\xde" //RP.]. 1#iTV)Ws]. |
// /* 0190 */ "\x24\x7b\x21\x62\x7a\x3c\xea\x80\xf9\xd2\x47\xe6\x2a\x6c\x4e\x99" //${!bz<....G.*lN. |
// /* 01a0 */ "\xa0\xee\xdb\xc8\x66\x77\x74\x38\xb4\x9b\x89\x21\xb5\xe5\x49\x70" //....fwt8...!..Ip |
// /* 01b0 */ "\x35\xea\x7d\x94\x48\xf9\xa0\x08\xdc\x6e\xce\x62\x5c\xe0\x4d\xc7" //5.}.H....n.b..M. |
// /* 01c0 */ "\x06\x4e\xff\x42\xc0\xa6\x13\xf8\xab\xd2\x0e\xce\x6a\x77\x18\x9f" //.N.B........jw.. |
// /* 01d0 */ "\xfb\x83\x85\x76\x0e\x25\x90\x1d\xb8\x6b\x5b\xb4\x13\xb6\x79\x3d" //...v.%...k[...y= |
// /* 01e0 */ "\xee\xcd\xd6\x5a\x0b\xd9\xa1\x31\x4a\x53\x28\x5f\x6a\xcc\x27\xdb" //...Z...1JS(_j.'. |
// /* 01f0 */ "\x85\x77\x73\xae\x85\xae\xb8\x17\x5c\x0a\x33\xdb\xc1\xc3\xaa\xab" //.ws.......3..... |
// /* 0200 */ "\x8c\x88\x89\x89\x11\x25\x50\xcc\xf7\x90\x81\x4b\x91\x38\xab\x69" //.....%P....K.8.i |
// /* 0210 */ "\x22\x9c\x44\x84\xc7\x8b\x2d\x41\x58\x6f\x1a\xfb\x54\x79\x38\xc1" //".D...-AXo..Ty8. |
// /* 0220 */ "\xe6\xfc\x94\x92\x7f\xf6\x4d\x6d\x62\xfb\x8e\x34\x88\xe9\x51\xaa" //......Mmb..4..Q. |
// /* 0230 */ "\xcb\xd0\xc9\x33\xf9\xe4\xdc\x3c\x57\x92\xe2\xc0\x12\x79\xcc\x5e" //...3...<W....y.^ |
// /* 0240 */ "\xac\xd4\x43\x87\xa2\xe3\xd7\xf7\x9b\xc3\xd6\xb2\xc6\x24\xea\x83" //..C..........$.. |
// /* 0250 */ "\xe4\xc7\xe9\xdf\x8a\x9f\x83\x02\x1e\xa2\x4a\x0a\x00\x26\x82\xfa" //..........J..&.. |
// /* 0260 */ "\xc3\xcb\xbf\x65\xa5\x00\x45\x41\x8d\xc7\x30\x62\x71\x67\xa1\x55" //...e..EA..0bqg.U |
// /* 0270 */ "\xe1\x33\x10\x54\x6c\x72\xec\xf9\x47\x3f\xcd\xcd\x3f\x7a\xda\x69" //.3.Tlr..G?..?z.i |
// /* 0280 */ "\xb2\x18\x62\x7b\x4f\x46\x52\x36\x3c\x52\xcb\x27\x97\x2e\x8f\x6c" //..b{OFR6<R.'...l |
// /* 0290 */ "\xd6\xbc\xb1\xc3\x65\x2d\x95\x59\xca\x09\xfd\xca\x42\xd0\xbe\xbf" //....e-.Y....B... |
// /* 02a0 */ "\x07\xb5\xff\x04\x4c\x7b\xbd\x39\x27\x05\xa5\x13\xf0\x3f\x5d\x9c" //....L{.9'....?]. |
// /* 02b0 */ "\xfc\x04\xce\x46\x64\x87\x7e\xa5\xf5\xf3\xfa\x54\x69\xbe\x3e\x4e" //...Fd.~....Ti.>N |
// /* 02c0 */ "\xf9\x6b\xe3\x3b\x4b\x0c\x16\xb1\xd7\x14\x82\x33\x77\xff\xd8\xbd" //.k.;K......3w... |
// /* 02d0 */ "\x90\x01\x58\x86\x6f\xd0\x41\x73\xd5\xd3\xee\x77\x19\x68\xfc\xb8" //..X.o.As...w.h.. |
// /* 02e0 */ "\x66\x72\xc0\x27\x29\xd9\x8f\x0f\xdc\x99\xdd\xa1\x48\x82\xd9\xb6" //fr.').......H... |
// /* 02f0 */ "\xba\xb4\xcf\xc2\xb7\x1c\x82\xd1\xd4\x34\xd8\xae\x14\x77\xc6\x13" //.........4...w.. |
// /* 0300 */ "\x6d\xff\x71\x3b\xb6\x8f\x33\xbf\xf5\x91\x14\x54\xb0\x6a\xe5\x6d" //m.q;..3....T.j.m |
// /* 0310 */ "\xe6\x32\x49\xe4\xdf\xa2\xd4\x56\x49\x41\x0d\x10\x0c\x4e\xc1\x87" //.2I....VIA...N.. |
// /* 0320 */ "\x90\x00\x34\x05\x47\x5b\x31\xb0\x8d\x3a\xb8\xc9\x1c\xaf\x9b\x89" //..4.G[1..:...... |
// /* 0330 */ "\x48\x4c\x58\xf1\xab\x40\xaf\x0f\xf1\xdf\x01\xae\x6c\x73\x5e\x82" //HLX..@......ls^. |
// /* 0340 */ "\xa0\x40\x41\x1b\x3d\x4a\x73\xab\x1d\x6d\xfc\xf2\x4a\xfa\x5f\x0f" //.@A.=Js..m..J._. |
// /* 0350 */ "\xee\xd8\x54\x37\x1b\x3e\x6b\xdc\xae\xd8\x7a\x8d\x75\x0b\x2d\x4e" //..T7.>k...z.u.-N |
// /* 0360 */ "\xd3\x3c\xd7\x33\x4e\x77\x1b\x37\xc3\xe9\x78\xc7\x37\xdd\xba\x6a" //.<.3Nw.7..x.7..j |
// /* 0370 */ "\xd8\x67\x3a\xb5\x42\x8e\xbc\xa7\xbf\x56\xbc\xac\x76\xbf\x78\x98" //.g:.B....V..v.x. |
// /* 0380 */ "\xd4\xf7\x5f\xea\xe7\x15\x1b\x74\xfb\xbe\x59\xcf\xdf\x68\x86\xf6" //.._....t..Y..h.. |
// /* 0390 */ "\x8e\xdf\x16\xea\xe6\x0e\x9e\x52\x3b\x42\xde\x10\xd1\xfb\xca\xb4" //.......R;B...... |
// /* 03a0 */ "\x0b\xcc\x14\x22\xd9\x55\x0d\x66\x4d\xf7\xf9\x09\xf8\xcd\x37\xf5" //...".U.fM.....7. |
// /* 03b0 */ "\x27\x17\x63\x7e\x0e\x9a\x18\x51\x99\x64\xdf\xe6\xaf\x36\x04\xe2" //'.c~...Q.d...6.. |
// /* 03c0 */ "\x55\x01\x51\xde\xc2\x25\x26\xc4\xc6\x93\xab\x5d\x2a\xa0\xa6\x99" //U.Q..%&....]*... |
// /* 03d0 */ "\x3d\x4b\x39\x36\x04\xa6\x1f\x93\xe4\xe1\xb2\x3a\xf4\x04\x84\xd2" //=K96.......:.... |
// /* 03e0 */ "\xb8\x4f\x28\x63\x58\x35\x7a\x53\xcc\x79\x26\x8c\x7d\x00\x4c\x9b" //.O(cX5zS.y&.}.L. |
// /* 03f0 */ "\x3a\x94\x78\x15\x91\x86\xb4\xde\x44\x32\x57\x38\x29\x49\xd4\x9a" //:.x.....D2W8)I.. |
// /* 0400 */ "\xba\xc2\x1e\x5f\xf4\x37\xfa\x39\xd7\x8f\xc1\xe2\xd7\xd1\x12\xbf" //..._.7.9........ |
// /* 0410 */ "\x77\xfd\xc6\x4b\xe7\x09\x9d\xf2\x29\x1e\xd5\x49\x2c\x6f\x7f\x7c" //w..K....)..I,o.| |
// /* 0420 */ "\x8f\x11\x46\x22\x1e\x44\x30\x9b\x65\xcc\x52\x03\x2d\xa7\x70\xb4" //..F".D0.e.R.-.p. |
// /* 0430 */ "\xfb\x4b\x84\x9b\x3d\x3c\x1e\xc0\xac\xf5\xe0\x3c\xee\x1b\x38\x94" //.K..=<.....<..8. |
// /* 0440 */ "\x63\x78\x2f\x4b\x9f\xeb\x7b\x22\xb0\x59\x8a\xfb\x59\xea\x83\x0e" //cx/K..{".Y..Y... |
// /* 0450 */ "\x29\xcb\x19\x82\xbf\xe9\x73\x2b\xa7\xcd\xe4\xca\x78\xd0\xc1\x4d" //).....s+....x..M |
// /* 0460 */ "\x39\xe6\x45\x7c\x05\x77\x35\x11\x88\x28\x38\x6f\xc3\xe9\x6f\xed" //9.E|.w5..(8o..o. |
// /* 0470 */ "\xe8\xf9\xe5\x6c\x4b\x51\xb2\x00\xff\xe8\x62\xb6\xbd\xa3\xf9\x2d" //...lKQ....b....- |
// /* 0480 */ "\xcb\xae\x0d\xd3\x7b\xaf\x9a\xe4\x64\x4a\x61\x7e\x0b\x63\xa8\x0b" //....{...dJa~.c.. |
// /* 0490 */ "\x69\x01\x0c\x85\xb7\x13\x25\x20\x27\x62\xaf\x01\x7a\xb6\xfc\x9b" //i.....% 'b..z... |
// /* 04a0 */ "\x4f\xd9\xb6\xa5\x4f\x60\xda\x93\xb6\x40\xdc\x01\xf1\x76\x63\xe8" //O...O`...@...vc. |
// /* 04b0 */ "\x5c\x1b\x8b\xf2\x19\xb4\xc3\x2d\x60\x69\xfe\xad\xd5\x95\xce\xeb" //.......-`i...... |
// /* 04c0 */ "\x30\x79\xdb\xf4\xa0\x8d\xad\x12\x11\x61\xb2\x8d\x98\x9b\xc9\x90" //0y.......a...... |
// /* 04d0 */ "\x53\xf4\x53\x83\x07\xdc\x12\xfe\x6c\xaf\xe3\xf1\xb0\x7f\x8f\xf0" //S.S.....l....... |
// /* 04e0 */ "\x81\x41\xb1\x96\xd5\xe4\x2b\xdb\x51\x75\x00\x81\x1d\x97\xdf\x30" //.A....+.Qu.....0 |
// /* 04f0 */ "\x6e\xf6\x6e\x25\xa0\xd1\xb7\x95\xdb\xc1\x27\x3e\xca\x5b\x70\xf8" //n.n%......'>.[p. |
// /* 0500 */ "\xcf\xdf\xc4\x1d\x28\x2f\x4f\x8c\x04\x0d\x48\x53\xed\x80\x3c\xa6" //....(/O...HS..<. |
// /* 0510 */ "\x16\x6b\xb9\xd0\x20\xcc\xc1\xe8\x6f\x6c\x2f\xe0\xe6\x29\xcc\x6e" //.k.. ...ol/..).n |
// /* 0520 */ "\x6d\x3f\xde\x2b\xbe\xe7\x0a\x51\x9f\xbc\x68\xca\x3b\x6c\x73\x68" //m?.+...Q..h.;lsh |
// /* 0530 */ "\xad\x60\x68\xdb\xbf\xf3\xf7\x25\x75\xe3\xfa\x69\x81\xd7\x15\xc1" //.`h....%u..i.... |
// /* 0540 */ "\xc3\x7f\x2c\xb7\x81\x7d\xfe\x96\x0d\x53\x47\x29\x2b\xd6\xba\x30" //..,..}...SG)+..0 |
// /* 0550 */ "\xaa\x4d\x00\x33\xdb\x1c\x0f\x1a\x4e\x26\xb9\xce\x51\x77\x56\xbf" //.M.3....N&..QwV. |
// /* 0560 */ "\xfd\x33\x40\x5b\x4f\x34\x13\xc0\x4d\x19\x90\x13\x68\x4c\x7d\x06" //.3@[O4..M...hL}. |
// /* 0570 */ "\x14\x24\xdf\x6f\x7e\x01\x64\x9c\x56\x0c\x52\x0b\xca\x0d\x08\x81" //.$.o~.d.V.R..... |
// /* 0580 */ "\xc7\x0c\x7a\x6c\x96\x9f\x43\xc0\xba\x9c\x44\xa0\x63\x2e\xb4\x95" //..zl..C...D.c... |
// /* 0590 */ "\xa5\x46\x3c\xe9\xbf\x3e\xc0\x58\x5c\xee\x20\x82\x9e\x40\x1c\x67" //.F<..>.X.. ..@.g |
// /* 05a0 */ "\x91\x80\xc3\xde\x13\xb0\xec\x28\xda\x1b\x29\xe2\xa2\x28\xb8\x04" //.......(..)..(.. |
// /* 05b0 */ "\x1c\xc7\x2d\xfc\x2a\x91\x4b\x46\x4c\x88\x85\x78\x19\x78\x8c\x36" //..-.*.KFL..x.x.6 |
// /* 05c0 */ "\x57\xd7\xfe\xdf\x9f\x23\x79\x1c\xb3\x49\xe8\x15\xad\x72\xb9\x8d" //W....#y..I...r.. |
// /* 05d0 */ "\xe8\x9a\x72\xa0\xd5\x9f\x6f\x53\x59\x64\xc1\x95\xa0\x25\x91\x13" //..r...oSYd...%.. |
// /* 05e0 */ "\x9e\x7f\xd1\x73\xe3\x65\x71\x3a\xab\x55\xb4\x7c\xad\xfa\x00\x1b" //...s.eq:.U.|.... |
// /* 05f0 */ "\x8d\xee\xd4\x6e\x41\x2a\xd6\x9c\x09\x34\x93\x24\x64\xcc\x9f\x16" //...nA*...4.$d... |
// /* 0600 */ "\xf5\x39\x45\xfe\x04\x1e\x2f\x6f\x5c\xda\x3b\xab\xa1\x55\x62\xb0" //.9E.../o..;..Ub. |
// /* 0610 */ "\xe8\x7f\x32\xe4\x01\x6b\xb0\x10\xf2\x1a\x83\xfe\x1e\xf9\xb7\x61" //..2..k.........a |
// /* 0620 */ "\xa8\xe9\xf2\x44\xbf\x43\x2a\xe0\x34\x11\xc9\x7e\xf8\x45\x94\xba" //...D.C*.4..~.E.. |
// /* 0630 */ "\xae\x9c\x62\xca\x4c\xf9\xcc\x48\x32\x86\xeb\x70\xc7\xbe\x26\x55" //..b.L..H2..p..&U |
// /* 0640 */ "\x1e\xfb\x8f\x32\xcf\xdb\xa5\xfe\x63\xe4\x20\xae\x39\xa1\x4b\x31" //...2....c. .9.K1 |
// /* 0650 */ "\x82\x06\x81\x83\x36\xf8\x9c\xb2\xae\xa5\xf9\x95\x81\xe5\x10\x69" //....6..........i |
// /* 0660 */ "\xb4\x5c\x90\x36\xf1\xe4\x95\x19\xdb\xd6\x1d\xfe\xf3\x75\xb6\x89" //...6.........u.. |
// /* 0670 */ "\x0a\x70\x71\x4e\x7c\xd2\x95\x06\xd5\xaf\x8b\x01\x1a\x0f\xf5\x30" //.pqN|..........0 |
// /* 0680 */ "\xee\xa1\xea\xef\xd4\xc5\xca\xfc\x51\x4b\x6b\x1b\xe5\x4a\xab\xee" //........QKk..J.. |
// /* 0690 */ "\x4e\x0d\x19\xc7\x41\x5e\x19\xf2\x09\x73\x7c\x10\xfe\xaa\x9e\xeb" //N...A^...s|..... |
// /* 06a0 */ "\x8a\x96\x00\x0f\xd1\x7c\x09\x67\x28\x8a\x15\x72\xe0\x02\xc1\xd3" //.....|.g(..r.... |
// /* 06b0 */ "\x67\x3a\xa3\xef\x83\x03\xd1\x83\x9a\x7b\xb7\x16\xdd\x3a\x78\x72" //g:.......{...:xr |
// /* 06c0 */ "\xc1\x6d\x87\x7f\xe1\x40\xa0\xed\xe9\x1b\xe1\xac\x86\x41\x44\xb0" //.m...@.......AD. |
// /* 06d0 */ "\xfb\xb3\x06\xfa\xca\x55\xee\x90\x72\xea\x20\x90\x48\x82\xa1\x0b" //.....U..r. .H... |
// /* 06e0 */ "\x72\xcd\x56\x75\xaa\x03\x09\xaf\x3e\x4a\x91\x43\x61\xc6\x90\x4b" //r.Vu....>J.Ca..K |
// /* 06f0 */ "\x66\xb7\xd6\x6a\x67\xe8\x1c\xfc\xb9\x30\xf4\x5a\x4c\x90\x23\x7e" //f..jg....0.ZL.#~ |
// /* 0700 */ "\xed\xd2\xcb\x46\x19\xd4\xd3\x59\xcc\xc4\xc2\x27\xeb\x4a\x43\xcd" //...F...Y...'.JC. |
// /* 0710 */ "\x6d\x96\xa3\xdc\x19\x44\x70\xaa\xe8\x8a\x89\x48\x9c\x6d\x92\x4c" //m....Dp....H.m.L |
// /* 0720 */ "\x51\xbb\xeb\xc6\x1b\xa6\xf2\x68\x07\xc6\xb8\x83\xd8\x7a\x1f\xa6" //Q......h.....z.. |
// /* 0730 */ "\x89\x34\x73\x21\x94\x04\x2a\xf7\xf7\x71\x8b\x8a\xf0\xc2\xb3\x3c" //.4s!..*..q.....< |
// /* 0740 */ "\xb3\xa0\x9e\xd5\xb2\x2a\x17\xd9\x35\xc8\x01\x7c\xa9\x17\xb3\xb3" //.....*..5..|.... |
// /* 0750 */ "\x05\xb2\xe2\x07\x13\x36\xd3\x46\x2c\x1b\xa6\x95\x45\x82\x53\xee" //.....6.F,...E.S. |
// /* 0760 */ "\x6a\x81\xcb\x09\xd1\x52\xf3\x1e\x9b\x11\x55\x67\x43\x55\x9b\xd7" //j....R....UgCU.. |
// /* 0770 */ "\xf9\xfc\x6b\xf6\x94\x50\x2c\x41\xa6\x84\x31\x8b\x71\x50\x20\x7d" //..k..P,A..1.qP } |
// /* 0780 */ "\x29\xe3\x12\xd4\xbc\xdc\x8a\x84\x0d\x46\x89\x20\x32\x8c\x39\x73" //)........F. 2.9s |
// /* 0790 */ "\x53\x7e\x9b\x5a\x41\x23\x0d\x3d\xee\xbd\x7f\xf5\x17\xcf\xd4\x21" //S~.ZA#.=.......! |
// /* 07a0 */ "\x02\xa0\xdc\xd6\x13\xdd\xe9\x40\x7c\xdc\x9a\x20\x85\xaf\x7d\x6d" //.......@|.. ..}m |
// /* 07b0 */ "\x80\x89\x44\xf9\x8c\xe9\xd4\x65\x18\x00\xf8\x2c\x38\x09\x4d\x4a" //..D....e...,8.MJ |
// /* 07c0 */ "\x5a\x1a\x1d\x06\x5b\x52\xbd\x23\x69\x0e\xc4\x1f\xae\xbf\xe4\xa1" //Z...[R.#i....... |
// /* 07d0 */ "\x94\xd5\x98\x98\x2b\x06\xb7\xd2\xa0\xd4\xe1\x5b\x34\x70\x61\xd3" //....+......[4pa. |
// /* 07e0 */ "\x2b\x1d\x5b\x72\xc9\x1c\xd5\xb4\x3f\x2f\x72\x86\xc0\xe0\xcd\xae" //+.[r....?/r..... |
// /* 07f0 */ "\x66\x2b\x23\xe6\x4d\xf6\x9d\x7d\xba\x91\x40\x0f\x29\x7a\x73\x35" //f+#.M..}..@.)zs5 |
// /* 0800 */ "\xca\x7b\x7e\xd4\x4d\x14\x9b\x05\x6e\x93\xb4\x6d\x1a\x34\x44\xda" //.{~.M...n..m.4D. |
// /* 0810 */ "\x64\x7b\xd8\x8a\xba\x00\xa0\xce\x5c\xe0\xe2\x05\x4a\xad\x3a\xf3" //d{..........J.:. |
// /* 0820 */ "\x85\x18\x14\x98\x1b\x8d\x4f\x30\xca\x39\x9e\x77\x74\x44\xc0\x12" //......O0.9.wtD.. |
// /* 0830 */ "\x65\xfd\x83\xeb\x06\xee\x63\x6f\x5a\xff\x7c\xd9\xc6\xff\x1f\xac" //e.....coZ.|..... |
// /* 0840 */ "\x1d\x1d\x6d\x6b\xef\xbc\x66\x41\x78\xae\x11\x7e\x49\x87\x78\x34" //..mk..fAx..~I.x4 |
// /* 0850 */ "\xcd\x71\xc8\x0d\x38\x6e\x55\xa6\xd0\x39\x57\xef\x18\x44\x2f\x58" //.q..8nU..9W..D/X |
// /* 0860 */ "\x9e\x4a\xe4\x6d\x60\x14\xca\xc4\x29\x39\x83\x9c\xa6\x9f\xde\x15" //.J.m`...)9...... |
// /* 0870 */ "\xb2\xec\x77\xfb\x9b\xa7\xa4\xf3\x59\x9d\xe3\x40\x0e\xfd\x68\x8e" //..w.....Y..@..h. |
// /* 0880 */ "\xbc\xc4\x60\x34\x5f\xf2\x8b\x5c\x60\x66\x2b\x3f\x7b\x6e\x45\xe3" //..`4_...`f+?{nE. |
// /* 0890 */ "\xb3\xcc\x53\xfa\x09\x3f\x7a\xb2\xcf\x5b\x4d\xb7\x5b\x51\x9c\x10" //..S..?z..[M.[Q.. |
// /* 08a0 */ "\x66\x31\x2d\xb2\xdf\x79\xaf\x8d\xdb\xf7\x7c\x03\xb2\x20\x92\xeb" //f1-..y....|.. .. |
// /* 08b0 */ "\xc3\xfe\x19\xab\x34\x65\xc9\xed\x00\xa4\xd6\xb0\xd0\x97\x9e\xdd" //....4e.......... |
// /* 08c0 */ "\x2b\xf7\x2d\xae\xdf\x9e\x95\xec\x6d\xe4\x78\x6c\x23\x8b\x83\xa7" //+.-.....m.xl#... |
// /* 08d0 */ "\xec\x1c\xe3\x13\x14\x32\xbc\x4e\x48\xc6\x93\xe8\x4d\xf5\xf5\xb9" //.....2.NH...M... |
// /* 08e0 */ "\x6e\xce\xe7\x90\x2f\x75\x0a\x01\x9f\x42\x13\x18\xe1\x49\x08\x43" //n.../u...B...I.C |
// /* 08f0 */ "\x6e\xf1\x5c\x34\x7d\x0e\xf0\x42\xc1\x03\x2b\x94\xdc\xef\x6b\xea" //n..4}..B..+...k. |
// /* 0900 */ "\xb0\xc5\x30\xa8\x7a\x20\x35\x68\x2c\xef\xb0\x61\xa1\x80\x12\x05" //..0.z 5h,..a.... |
// /* 0910 */ "\x5b\x64\xce\xf6\xac\xa8\xcb\x6e\x9d\xac\x19\xd0\x80\x67\x41\x86" //[d.....n.....gA. |
// /* 0920 */ "\x77\x71\x26\x4e\xa3\x77\xf1\x14\x8b\xf5\xd8\xd5\x6a\x2a\x36\x8d" //wq&N.w......j*6. |
// /* 0930 */ "\xda\x78\x60\xa1\xe9\xf1\x00\x03\xca\xa7\x2e\x6f\x2c\xec\x09\xa2" //.x`........o,... |
// /* 0940 */ "\xcc\x72\xe4\xf3\x2b\xda\xfc\x87\xa7\x1c\x02\x8b\xb7\x3e\x82\xf5" //.r..+........>.. |
// /* 0950 */ "\x56\xe2\x05\xd4\x87\x8c\xe7\x2f\x47\x2a\x3d\xcf\xed\x2f\x31\x0f" //V....../G*=../1. |
// /* 0960 */ "\x1b\x86\xdd\x72\x7d\x41\x84\xd3\x70\x7f\x57\xd0\xdc\x3f\x1b\xf9" //...r}A..p.W..?.. |
// /* 0970 */ "\x87\xa4\x56\x6f\xf5\xff\x35\xdb\xe8\xfb\xe6\x6e\x6c\x2a\x6e\x6c" //..Vo..5....nl*nl |
// /* 0980 */ "\x38\x1a\x0c\x53\xd9\x20\x0d\x41\x29\x65\x82\x8b\x49\x1d\x6b\xc7" //8..S. .A)e..I.k. |
// /* 0990 */ "\x99\xd9\xa6\xcb\xa4\x0c\x54\xfc\x01\x81\x40\xa2\xbb\xef\x43\xb9" //......T...@...C. |
// /* 09a0 */ "\x36\x3d\x00\x66\x1e"                                             //6=.f. |
// Dump done on RDP Wab Target (4) 2469 bytes |
// LOOPING on PDUs: 2446 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_ORDERS |
// process_orders bpp=16 |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,0,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,10,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(127,10,829,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,10,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,26,956,7) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,26,20,7) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,33,1024,8) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,41,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,41,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,57,1024,5) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,62,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(214,62,5,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(920,62,5,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,62,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,82,1024,620) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,702,787,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(820,702,15,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,702,69,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,702,15,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,702,20,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,703,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,703,29,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,703,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,703,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(915,705,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,705,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,705,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,721,29,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,721,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,721,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,723,69,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,723,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,723,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,725,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,735,810,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(873,735,40,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(987,735,37,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,758,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(20,10,107,16) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(20,10,108,17) op=(20,10,108,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=1a data_len=26 [0x00 0x00 0x01 0x09 0x02 0x0e 0x03 0x09 0x04 0x09 0x02 0x04 0x03 0x09 0x04 0x09 0x05 0x04 0x06 0x09 0x04 0x09 0x06 0x04 0x06 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,41,204,16) color=0x004398) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(20,41,93,17) op=(20,41,93,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=39 data_len=24 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09 0x0d 0x05 0x0e 0x04 0x09 0x0b 0x0f 0x05 0x10 0x09 0x11 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,41,706,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(224,41,46,17) op=(224,41,46,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=39 data_len=12 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,41,84,16) color=0x004398) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(930,41,58,17) op=(930,41,58,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=39 data_len=16 [0x12 0x00 0x09 0x09 0x0f 0x05 0x0c 0x09 0x0f 0x05 0x13 0x09 0x0f 0x08 0x14 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(16,63,197,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(18,64,1,15) color=0x008c51) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,62,198,1) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,63,1,18) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(213,62,1,20) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,81,199,1) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(220,63,699,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,62,700,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(919,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,81,701,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(926,63,82,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,62,83,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1008,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,81,84,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,10,48,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(957,11,45,20) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(963,13,34,17) op=(963,13,34,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3c3 glyph_y=1d data_len=12 [0x15 0x00 0x16 0x08 0x14 0x03 0x0c 0x03 0x0b 0x05 0x09 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,10,46,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,12,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1002,10,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,31,48,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(787,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(788,703,30,20) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(794,705,19,17) op=(794,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=31a glyph_y=2d1 data_len=4 [0x17 0x00 0x18 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(835,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(836,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(842,705,12,17) op=(842,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=34a glyph_y=2d1 data_len=2 [0x17 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(877,704,23,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=36e glyph_y=2d1 data_len=2 [0x02 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,703,24,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,704,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(900,703,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,722,25,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,705,14,16) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=385 glyph_y=2d1 data_len=4 [0x19 0x00 0x03 0x05] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(930,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(931,703,23,20) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(937,705,12,17) op=(937,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a9 glyph_y=2d1 data_len=2 [0x1a 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(971,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(972,703,30,20) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(978,705,19,17) op=(978,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3d2 glyph_y=2d1 data_len=4 [0x1b 0x00 0x1a 0x07] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,63,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(811,736,60,20) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(817,738,49,17) op=(817,738,49,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=331 glyph_y=2f2 data_len=12 [0x1c 0x00 0x0f 0x07 0x0a 0x09 0x0f 0x09 0x10 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,61,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,737,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(871,735,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,756,63,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,735,74,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(914,736,71,20) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(920,738,60,17) op=(920,738,60,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=398 glyph_y=2f2 data_len=14 [0x1d 0x00 0x0f 0x0a 0x1e 0x09 0x1e 0x09 0x0b 0x09 0x13 0x09 0x0c 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,735,72,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,737,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(985,735,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,756,74,2) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 3 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x04\x5d"                                                     //..] |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 1113 bytes |
// Recv done on RDP Wab Target (4) 1113 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x84\x4e\x08\x00\x00\x00\x24" //...h....p.N....$ |
// /* 0010 */ "\x7f\xd4\xef\x1a\x94\xfb\xb8\x2b\x6f\xcd\xfe\x66\x38\x37\xaa\x3d" //.......+o..f87.= |
// /* 0020 */ "\xbe\x08\x75\x85\xbc\x60\x3c\x05\x0f\x16\xd3\x23\xf6\xc6\xd9\x64" //..u..`<....#...d |
// /* 0030 */ "\x44\x95\xc9\xd5\x88\xbc\x88\x43\x13\x7b\xc0\xe2\x9a\xef\x3b\xae" //D......C.{....;. |
// /* 0040 */ "\x17\x5a\x48\xc2\xff\x95\xdb\xb5\x9d\x23\xe8\x97\xc2\xfe\x46\x3d" //.ZH......#....F= |
// /* 0050 */ "\x7d\x0c\xb4\xd0\x96\x47\xe6\x18\xa9\x8b\xbc\x89\x5e\xcc\xb9\x5d" //}....G......^..] |
// /* 0060 */ "\xc3\x3d\xab\x27\x57\x70\xcf\xfe\x0d\x9c\x83\xce\xb5\x3f\x25\x86" //.=.'Wp.......?%. |
// /* 0070 */ "\x8e\x44\x7e\x08\xba\x8a\xd4\x14\x0e\xe2\x7a\x69\xfb\x60\x20\x55" //.D~.......zi.` U |
// /* 0080 */ "\xab\x6b\x37\xb9\xf3\x7b\x13\xf7\x63\x39\x8e\x71\xb0\xa5\x47\xdc" //.k7..{..c9.q..G. |
// /* 0090 */ "\x1c\x7e\x7f\x73\xd3\x9c\xf4\x76\x32\xe3\x52\xa9\x4e\x79\x8e\xe8" //.~.s...v2.R.Ny.. |
// /* 00a0 */ "\x4b\x44\x71\x34\xc7\x8a\xf5\x8f\x6b\xb9\x71\xc3\xb4\x39\x52\xd2" //KDq4....k.q..9R. |
// /* 00b0 */ "\xe2\xf7\xc5\x0f\xaf\xcd\xf9\x38\xeb\x4e\x3f\xff\x65\x05\xb0\xcb" //.......8.N?.e... |
// /* 00c0 */ "\xa8\x90\x31\x63\x5f\x55\xea\x48\xd0\xb0\x36\x2b\x00\x43\x4b\x9d" //..1c_U.H..6+.CK. |
// /* 00d0 */ "\x5b\x45\xaf\x67\x50\x66\x31\xde\x90\x34\x4c\x18\x9b\x7c\x05\xd7" //[E.gPf1..4L..|.. |
// /* 00e0 */ "\x73\x1b\xe4\x9a\x3d\x7b\xd3\x85\x5f\x6e\x8b\x3c\x38\x72\x73\x5e" //s...={.._n.<8rs^ |
// /* 00f0 */ "\xb7\x93\xf1\xf9\x7c\x17\x17\x77\xdc\x77\xf6\x14\xdf\xee\x7d\xe6" //....|..w.w....}. |
// /* 0100 */ "\x91\xf8\x1d\x05\x44\x1a\x76\x87\x56\xd7\x3d\x37\xac\x8c\xb0\x4e" //....D.v.V.=7...N |
// /* 0110 */ "\x0e\xea\xfe\xf6\x7f\xc0\x58\xff\x60\x3b\x59\x90\x86\xb8\xe6\x19" //......X.`;Y..... |
// /* 0120 */ "\xdb\x62\xbc\xdd\x37\x85\x42\x05\x7e\xf2\x57\xf1\x00\x84\x08\x12" //.b..7.B.~.W..... |
// /* 0130 */ "\x47\x57\xa8\x2b\x6c\x85\x3d\x2e\x49\xec\xdb\xff\xe2\xf2\xdf\x00" //GW.+l.=.I....... |
// /* 0140 */ "\x11\xc6\xd6\x91\xea\x7c\xea\x7c\x21\x68\xa3\xb9\xf6\x6c\x79\x35" //.....|.|!h...ly5 |
// /* 0150 */ "\xaa\x43\x86\x91\x04\xf2\xe6\x3d\x3f\x92\x98\x07\x05\x11\x8d\x15" //.C.....=?....... |
// /* 0160 */ "\x45\xbf\xf1\x27\x43\xf5\xa2\xf4\x72\x5d\xde\x80\x01\x49\xbb\x11" //E..'C...r]...I.. |
// /* 0170 */ "\x56\x0a\x0f\xed\x44\x8b\x08\xd1\xc0\xcd\x5b\xc9\xca\x14\x5e\x1c" //V...D.....[...^. |
// /* 0180 */ "\xc3\xb9\xce\x79\x46\x2d\xae\xdb\xf1\x7c\x0e\x3f\x3c\x5c\x4d\x25" //...yF-...|.?<.M% |
// /* 0190 */ "\xa7\x28\xeb\xe6\x9a\xe2\xf3\x4c\x31\x42\x49\xab\xfc\xe1\x4b\x97" //.(.....L1BI...K. |
// /* 01a0 */ "\x50\xb8\x88\x1f\x1a\x65\xa9\xeb\xab\x20\x89\x90\xa0\x11\xef\x93" //P....e... ...... |
// /* 01b0 */ "\x6c\xa3\x95\xf6\xd1\xbf\x8e\xbb\x50\x11\x6c\x03\x65\xc2\x23\xec" //l.......P.l.e.#. |
// /* 01c0 */ "\xdb\x9b\x44\x32\xc7\xbc\x11\xd3\xa7\x0c\x81\x8a\xef\x7e\x32\x88" //..D2.........~2. |
// /* 01d0 */ "\xb7\x7e\x29\x02\x68\x05\xf9\x26\x42\x38\x33\xa4\xd0\xc1\x2a\x4c" //.~).h..&B83...*L |
// /* 01e0 */ "\xab\x98\x12\x99\x7e\x7c\x38\x24\x2d\x62\x6e\xae\xe7\x66\x17\xcc" //....~|8$-bn..f.. |
// /* 01f0 */ "\xd2\x21\x23\x6a\xb6\x7a\x6a\xe7\xb1\x5a\x6f\xaf\x44\x15\x0e\xee" //.!#j.zj..Zo.D... |
// /* 0200 */ "\xe9\x78\xfc\xb6\x31\x35\x64\xf8\xc3\x06\xf8\x47\x2b\x15\x9a\x9b" //.x..15d....G+... |
// /* 0210 */ "\x8a\x43\x95\xac\xa1\xdb\x3c\xa2\xf8\xf3\x19\x68\xbf\xd8\xc5\x59" //.C....<....h...Y |
// /* 0220 */ "\x9f\x4e\xb8\xc4\x38\xc8\xaa\x79\xb1\x01\x00\xba\xc6\x2a\x43\x5a" //.N..8..y.....*CZ |
// /* 0230 */ "\x02\x3b\x1d\x00\x4a\xa6\xe4\x65\x9a\x79\x32\x51\x46\x51\x06\x93" //.;..J..e.y2QFQ.. |
// /* 0240 */ "\xed\xeb\x0e\x3f\x33\x7a\xf7\x41\x89\x22\xcb\x66\x73\xe5\x99\x82" //...?3z.A.".fs... |
// /* 0250 */ "\x03\x93\x65\xdd\xec\xa8\x42\x13\x2b\x76\x34\xd2\xb8\x57\xbb\x7b" //..e...B.+v4..W.{ |
// /* 0260 */ "\xb6\xfb\x2e\xdb\x92\x63\x6a\x47\xef\xcb\xf2\xed\xf3\x0e\xcf\xc4" //.....cjG........ |
// /* 0270 */ "\xc4\xa6\x22\x5e\x57\xfe\x88\x28\xf1\xc0\x3a\x9d\x62\x2b\x52\xfa" //.."^W..(..:.b+R. |
// /* 0280 */ "\x12\x7d\x2c\x1f\x2b\x4f\x20\x3c\xb5\x10\x78\x98\xcf\xe1\x6d\xa5" //.},.+O <..x...m. |
// /* 0290 */ "\xfc\x6e\x3a\x24\x6e\xc6\xa1\xa4\xa8\xbd\x2b\x6f\x86\x5f\x9f\xc3" //.n:$n.....+o._.. |
// /* 02a0 */ "\xc2\xfd\x96\x42\xa1\xd3\x82\xf0\xd3\xe6\x3a\xb7\xc5\x44\x0d\x8c" //...B......:..D.. |
// /* 02b0 */ "\xd7\x9b\x44\x19\x10\xc0\x72\xff\xb4\xc6\x28\x88\x8a\x1a\x7c\xf3" //..D...r...(...|. |
// /* 02c0 */ "\x87\x5f\x26\x52\xab\x6d\xc1\x0b\x7e\xe3\x77\x97\xac\xda\x9a\x29" //._&R.m..~.w....) |
// /* 02d0 */ "\x2e\x61\x11\x05\x0d\x41\x05\x37\xc2\x24\x50\x3f\xe4\x13\xb6\x4c" //.a...A.7.$P?...L |
// /* 02e0 */ "\x0e\x4a\x67\x0d\xd5\xf5\x3a\x6a\xc0\x62\x4f\x07\x1c\x65\x9e\xed" //.Jg...:j.bO..e.. |
// /* 02f0 */ "\x0a\x0c\xea\x5f\xb8\x63\x75\x9f\x08\x36\xf5\xa7\xa8\xca\x5f\xca" //..._.cu..6...._. |
// /* 0300 */ "\x5e\xda\x57\x55\x95\x8b\x19\xd9\x54\xf4\x63\x12\x92\x09\x92\x42" //^.WU....T.c....B |
// /* 0310 */ "\xc9\xd9\x68\xcc\x7a\x16\xfd\x0d\xaf\xb3\xd9\xbb\x8d\xfe\x1a\xff" //..h.z........... |
// /* 0320 */ "\x34\x69\xa7\xad\xa8\xf8\xc3\x4d\x82\xe4\xbf\x4c\xb3\x9c\x1d\xc9" //4i.....M...L.... |
// /* 0330 */ "\x89\xcc\xf2\xb3\x5b\x71\x45\x96\xa7\x2e\x99\x44\x64\x82\xc5\x2d" //....[qE....Dd..- |
// /* 0340 */ "\xcd\x0c\xe8\xba\xaa\x57\xbc\x19\xe9\xe3\xda\x16\x73\x76\x24\xcf" //.....W......sv$. |
// /* 0350 */ "\x31\x57\x0c\x21\x56\xa3\x1c\x58\x0b\x16\x10\x77\x61\x75\x3c\x56" //1W.!V..X...wau<V |
// /* 0360 */ "\x2f\xbe\x78\x07\xfe\x18\xc0\x98\x43\xe6\x18\x55\xf6\x6c\x1e\xa3" ///.x.....C..U.l.. |
// /* 0370 */ "\xe2\x6d\xd3\x4e\x5f\x9b\x51\x05\x5d\xcd\x93\x3a\x94\xb7\x40\x85" //.m.N_.Q.]..:..@. |
// /* 0380 */ "\xd3\x99\xea\x43\x4c\x66\xd0\x01\x79\x3a\xd2\xc7\xd2\xde\x9a\x0e" //...CLf..y:...... |
// /* 0390 */ "\x41\x9b\x80\x3f\x8f\xb3\x1e\xd4\xfe\x26\xd4\x3b\x9e\x3f\xf5\x34" //A..?.....&.;.?.4 |
// /* 03a0 */ "\x34\xac\x5a\xc2\x88\x55\x1d\x2a\x85\x6b\x69\x05\x8c\xf9\x4f\xd8" //4.Z..U.*.ki...O. |
// /* 03b0 */ "\x61\xad\x2a\xd4\xf3\x0c\xb3\x62\x18\x1b\x81\x0d\x69\x35\xe7\x85" //a.*....b....i5.. |
// /* 03c0 */ "\x3e\xfe\x70\x93\x50\x51\x57\x98\xe9\xa4\xc7\xd7\x37\xde\x10\xe4" //>.p.PQW.....7... |
// /* 03d0 */ "\x7d\x08\x5f\x99\xdf\x8e\xb7\x92\xc5\x96\x05\xfc\x64\xd6\x0e\xf1" //}._.........d... |
// /* 03e0 */ "\x70\xc8\x7b\xae\xd7\x98\x9a\xca\x63\xa8\xa5\xd4\x88\xc8\x7e\xd7" //p.{.....c.....~. |
// /* 03f0 */ "\x53\x07\xc7\x51\xbe\x2c\x05\x46\x92\x54\xd9\xd9\x5c\x33\xa5\x94" //S..Q.,.F.T...3.. |
// /* 0400 */ "\x69\xe5\xe1\x42\xdd\x9c\xad\xc5\xb9\x7e\x34\xc7\x2c\xee\x45\x45" //i..B.....~4.,.EE |
// /* 0410 */ "\xa2\xea\x3e\xde\x92\x78\x7f\x4b\x29\x8f\xca\x54\x0f\xe0\x75\x1c" //..>..x.K)..T..u. |
// /* 0420 */ "\x5a\x23\x7f\x7a\x2e\x13\x5c\xca\x0c\x39\x10\x41\x85\xaa\xdf\x9e" //Z#.z.....9.A.... |
// /* 0430 */ "\x42\x5a\x9d\xab\x1d\xc7\x57\x00\xa7\xf4\x34\x69\xcb\xce\x0d\x66" //BZ....W...4i...f |
// /* 0440 */ "\x78\xae\x61\xda\x66\xdf\xf8\xde\x38\xc6\x7e\x0b\x25\x96\x74\xe2" //x.a.f...8.~.%.t. |
// /* 0450 */ "\x0e\x8c\xb7\xd4\xd4\x45\xe2\x2b\x5f"                             //.....E.+_ |
// Dump done on RDP Wab Target (4) 1113 bytes |
// LOOPING on PDUs: 1090 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_ORDERS |
// process_orders bpp=16 |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,0,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,10,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(127,10,829,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,10,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,26,956,7) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,26,20,7) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,33,1024,8) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,41,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,41,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,57,1024,5) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,62,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(214,62,5,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(920,62,5,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,62,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,82,1024,620) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,702,787,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(820,702,15,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,702,69,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,702,15,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,702,20,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,703,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,703,29,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,703,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,703,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(915,705,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,705,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,705,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,721,29,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,721,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,721,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,723,69,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,723,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,723,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,725,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,735,810,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(873,735,40,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(987,735,37,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,758,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(20,10,107,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(20,10,108,17) op=(20,10,108,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=1a data_len=26 [0x00 0x00 0x01 0x09 0x02 0x0e 0x03 0x09 0x04 0x09 0x02 0x04 0x03 0x09 0x04 0x09 0x05 0x04 0x06 0x09 0x04 0x09 0x06 0x04 0x06 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,41,204,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(20,41,93,17) op=(20,41,93,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=39 data_len=24 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09 0x0d 0x05 0x0e 0x04 0x09 0x0b 0x0f 0x05 0x10 0x09 0x11 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,41,706,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(224,41,46,17) op=(224,41,46,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=39 data_len=12 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,41,84,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(930,41,58,17) op=(930,41,58,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=39 data_len=16 [0x12 0x00 0x09 0x09 0x0f 0x05 0x0c 0x09 0x0f 0x05 0x13 0x09 0x0f 0x08 0x14 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(16,63,197,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(18,64,1,15) color=0x008c51) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,62,198,1) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,63,1,18) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(213,62,1,20) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,81,199,1) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(220,63,699,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,62,700,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(919,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,81,701,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(926,63,82,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,62,83,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1008,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,81,84,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,10,48,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(957,11,45,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(963,13,34,17) op=(963,13,34,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3c3 glyph_y=1d data_len=12 [0x15 0x00 0x16 0x08 0x14 0x03 0x0c 0x03 0x0b 0x05 0x09 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,10,46,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,12,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1002,10,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,31,48,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(787,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(788,703,30,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(794,705,19,17) op=(794,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=31a glyph_y=2d1 data_len=4 [0x17 0x00 0x18 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(835,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(836,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(842,705,12,17) op=(842,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=34a glyph_y=2d1 data_len=2 [0x17 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(877,704,23,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=36e glyph_y=2d1 data_len=2 [0x02 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,703,24,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,704,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(900,703,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,722,25,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,705,14,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=385 glyph_y=2d1 data_len=4 [0x19 0x00 0x03 0x05] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(930,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(931,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(937,705,12,17) op=(937,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a9 glyph_y=2d1 data_len=2 [0x1a 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(971,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(972,703,30,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(978,705,19,17) op=(978,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3d2 glyph_y=2d1 data_len=4 [0x1b 0x00 0x1a 0x07] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,63,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(811,736,60,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(817,738,49,17) op=(817,738,49,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=331 glyph_y=2f2 data_len=12 [0x1c 0x00 0x0f 0x07 0x0a 0x09 0x0f 0x09 0x10 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,61,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,737,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(871,735,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,756,63,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,735,74,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(914,736,71,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(920,738,60,17) op=(920,738,60,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=398 glyph_y=2f2 data_len=14 [0x1d 0x00 0x0f 0x0a 0x1e 0x09 0x1e 0x09 0x0b 0x09 0x13 0x09 0x0c 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,735,72,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,737,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(985,735,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,756,74,2) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 4 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x5e"                                                     //..^ |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 90 bytes |
// Recv done on RDP Wab Target (4) 90 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x50\x08\x00\x00\x00\x30\x76" //...h....pP....0v |
// /* 0010 */ "\xed\xf1\x02\x34\xd7\xe2\xab\xcb\x96\x8f\x58\x9b\x35\xab\x8e\x31" //...4......X.5..1 |
// /* 0020 */ "\x69\x2e\x56\x5d\xec\x77\xf7\x30\x94\x36\x89\x43\xe2\xa4\x90\x46" //i.V].w.0.6.C...F |
// /* 0030 */ "\x04\x54\x92\xe1\x5e\x56\x61\x83\x41\x94\xcf\xc0\x0b\x36\x0a\x49" //.T..^Va.A....6.I |
// /* 0040 */ "\x68\x36\xd3\xe9\x9b\x7b\x9f\x33\x84\xa7\x2d\x9b\x25\x1e\xc7\x11" //h6...{.3..-.%... |
// /* 0050 */ "\x1d\xba\x8a\x3a\x24\x69\x0c\xbc\x17\x22"                         //...:$i..." |
// Dump done on RDP Wab Target (4) 90 bytes |
// LOOPING on PDUs: 68 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_ORDERS |
// process_orders bpp=16 |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(16,63,197,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,62,198,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(213,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,81,199,1) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 5 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x0d\xb8"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 3508 bytes |
// Recv done on RDP Wab Target (4) 3508 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x8d\xa9\x08\x00\x00\x00\x44" //...h....p......D |
// /* 0010 */ "\x06\x57\x55\x72\xdb\xd6\xe2\x2a\xfd\xa2\xf5\x89\xff\x99\x6e\xf3" //.WUr...*......n. |
// /* 0020 */ "\x70\x11\xff\x27\xbf\x6f\x9b\x65\x11\x18\x60\x50\x58\x37\xe2\xbe" //p..'.o.e..`PX7.. |
// /* 0030 */ "\x1d\xf6\x92\x11\xa3\x2a\x1a\xa7\xcf\xba\x6c\x0c\xab\x68\xa3\x0f" //.....*....l..h.. |
// /* 0040 */ "\x04\x0b\x5c\xc9\x24\x11\xdd\x97\x13\x50\xa4\xf8\xcf\x11\xbc\xac" //....$....P...... |
// /* 0050 */ "\xe7\x15\x1a\xae\x27\x89\xbc\xaf\x41\x3c\xa6\xea\xab\xd5\x67\xac" //....'...A<....g. |
// /* 0060 */ "\xcf\x08\x92\x58\xe8\xaf\xec\x14\x7a\x06\xc3\xab\x5a\x56\x54\xf7" //...X....z...ZVT. |
// /* 0070 */ "\x68\x28\x90\x35\xc9\x1e\xd6\xda\xfa\xb2\x98\x88\x8b\xa9\x9e\xf2" //h(.5............ |
// /* 0080 */ "\xbc\x99\x52\x93\x01\x08\xca\x55\xd5\xda\x99\xb6\xeb\xe3\x4b\x5e" //..R....U......K^ |
// /* 0090 */ "\x6c\xab\x4b\xd8\x12\x60\x14\x34\xf3\xa4\xf2\x91\x43\x63\x40\xee" //l.K..`.4....Cc@. |
// /* 00a0 */ "\xc6\x64\x3c\x99\xc9\xde\x10\x92\x42\x57\xdf\x38\xd7\x6a\xa0\xd2" //.d<.....BW.8.j.. |
// /* 00b0 */ "\x09\x9f\x30\x5b\xf8\x99\xa9\x4d\xa5\xf7\x0b\xca\xe1\x78\x4d\xef" //..0[...M.....xM. |
// /* 00c0 */ "\x1f\x39\xe0\x6c\x84\x8d\xc5\x2c\x51\x6f\xba\xc1\xc8\xda\x5e\x80" //.9.l...,Qo....^. |
// /* 00d0 */ "\xfb\x16\xa2\x55\xd8\xac\x32\x94\xa1\x19\xdc\x72\xb5\xa0\x2b\x64" //...U..2....r..+d |
// /* 00e0 */ "\xd6\x31\xd6\x34\x8d\x6b\x6e\x91\xd6\x4f\x65\xcf\x08\xd0\xd6\x4d" //.1.4.kn..Oe....M |
// /* 00f0 */ "\x96\x67\x21\x64\xaf\xe9\x90\xc4\xe8\xcd\xc2\x25\xdf\x61\xc8\x70" //.g!d.......%.a.p |
// /* 0100 */ "\x69\xce\x20\x33\x00\x80\x9a\x3f\xc0\xc2\x77\xce\xaf\xa6\x84\x7a" //i. 3...?..w....z |
// /* 0110 */ "\x5a\x10\xad\x67\x26\x6c\xf2\xcc\xb8\x58\x69\xb0\x65\xb0\xaf\xaf" //Z..g&l...Xi.e... |
// /* 0120 */ "\xa1\xe2\x02\xa3\x79\x46\x7b\xd8\xfb\xcf\x25\x24\x03\x39\x08\xcd" //....yF{...%$.9.. |
// /* 0130 */ "\x2a\xd1\xc3\xe8\x32\x12\xfb\x5a\x33\x99\x2c\x73\x93\xcc\x5c\xff" //*...2..Z3.,s.... |
// /* 0140 */ "\xa2\xc2\xbe\xd8\xeb\x4e\x2f\x09\x37\xc5\x6a\xd5\xc9\x40\xfa\x33" //.....N/.7.j..@.3 |
// /* 0150 */ "\x8a\x75\x29\xfd\x8b\xc8\xbd\x95\xed\x60\xb0\x49\x6c\xbc\x7c\x1d" //.u)......`.Il.|. |
// /* 0160 */ "\x8a\x82\x25\x7f\x8a\x00\xb7\x6b\x5a\x07\x64\x76\xb9\x74\x96\x04" //..%....kZ.dv.t.. |
// /* 0170 */ "\xa0\x3e\xee\x64\xcb\xc1\xb0\x39\xc8\x92\x96\xab\xbb\x6e\x40\xf1" //.>.d...9.....n@. |
// /* 0180 */ "\xf3\x1d\x26\x56\xdb\x5b\xda\xd8\x0d\x5d\xdf\x6a\xd4\xaa\x10\x81" //..&V.[...].j.... |
// /* 0190 */ "\x39\x5a\xad\x34\x12\x0b\x53\xb6\x35\x8f\x3b\x8d\x8e\xe5\xa5\xfb" //9Z.4..S.5.;..... |
// /* 01a0 */ "\xd7\xef\xf2\x71\x50\x9c\xc5\xbb\x7e\xc9\x36\xdc\x1e\xe4\xee\x6f" //...qP...~.6....o |
// /* 01b0 */ "\x28\x66\x09\xfa\xdc\x81\xff\xff\x5b\x9a\x26\x71\x95\xf7\xd3\x5b" //(f......[.&q...[ |
// /* 01c0 */ "\xa5\x91\xce\x38\xa1\x68\xf3\x41\x4c\x18\xfb\x39\x3b\x33\x6f\xc9" //...8.h.AL..9;3o. |
// /* 01d0 */ "\xd3\x8b\x56\xdb\x62\x80\xe5\xa1\x4e\x93\x48\x7e\x8b\xbd\x8e\x83" //..V.b...N.H~.... |
// /* 01e0 */ "\x49\x50\x89\xdf\x0e\x6f\x12\x20\xe5\x09\x4a\xe4\xf7\x18\x0a\xad" //IP...o. ..J..... |
// /* 01f0 */ "\xea\xe3\x9b\x9c\x24\x21\xa8\x4c\x4c\x99\x77\x39\x86\x08\xa6\xcb" //....$!.LL.w9.... |
// /* 0200 */ "\x78\xdc\x80\x36\xd8\x50\xc8\x74\xae\x4e\xbe\xb7\x77\x83\x62\xf4" //x..6.P.t.N..w.b. |
// /* 0210 */ "\xf6\xf6\x21\x71\x0a\x80\xc8\x8a\xb9\xed\xce\xe8\x5a\xa9\xe5\xe9" //..!q........Z... |
// /* 0220 */ "\x55\x3f\x3b\x24\xf5\xcf\x72\xbf\x30\x83\x7e\x43\xea\x24\xc1\x65" //U?;$..r.0.~C.$.e |
// /* 0230 */ "\xf2\x85\x1e\xad\x53\x5e\x0f\xde\x72\xfe\x25\xcc\xad\x85\x1f\x44" //....S^..r.%....D |
// /* 0240 */ "\x61\x98\x14\x77\x56\xcd\x6d\xb7\x29\x0e\xfc\x7b\x64\x17\x78\x5e" //a..wV.m.)..{d.x^ |
// /* 0250 */ "\x8d\xbf\x09\xa1\x55\x60\x08\xee\x25\x6a\xc4\xd9\xdf\x97\x4a\x98" //....U`..%j....J. |
// /* 0260 */ "\xcd\xa1\x67\x7c\x60\x59\xfa\x49\x71\x22\x09\xd0\xed\x83\x4f\xc0" //..g|`Y.Iq"....O. |
// /* 0270 */ "\xdd\x73\x58\x9a\x2e\xf9\xcc\x13\xda\xcc\xe5\x95\x6b\x0d\x53\x26" //.sX.........k.S& |
// /* 0280 */ "\x96\xad\xc9\x37\x91\xfd\x98\x89\x74\x58\x12\x34\x30\x42\x11\xbe" //...7....tX.40B.. |
// /* 0290 */ "\x51\xda\xa2\xbe\xca\x8d\xd8\x56\xac\x05\x2c\x79\x6c\xad\x53\x06" //Q......V..,yl.S. |
// /* 02a0 */ "\xd7\xcf\xb3\x26\xff\x08\x1c\x18\x25\xf9\x6c\x27\xd4\xaf\xa8\x38" //...&....%.l'...8 |
// /* 02b0 */ "\xc7\xb5\x45\x9e\xd9\x39\xca\x0a\xe0\x63\x69\x49\x79\xcc\xa5\x69" //..E..9...ciIy..i |
// /* 02c0 */ "\x91\x69\xbf\xf3\x4c\xc7\x05\x47\xc1\xbf\xb5\x56\x4d\xe8\xb4\xbc" //.i..L..G...VM... |
// /* 02d0 */ "\xc8\x9f\x08\x2e\xa7\xe1\x0d\x68\x76\x17\xaa\x85\x48\xda\x98\x21" //.......hv...H..! |
// /* 02e0 */ "\x21\x1e\xf1\x00\x63\x69\xb1\x63\x14\x14\x65\x4e\x24\x6b\x81\xba" //!...ci.c..eN$k.. |
// /* 02f0 */ "\x10\x83\x3f\x7c\xd3\xd0\xbb\x7e\xb5\xf2\x4e\x9f\x00\xc6\xe5\x41" //..?|...~..N....A |
// /* 0300 */ "\xc4\x2c\x00\x0d\x86\xed\x39\x3f\xac\xf3\x78\x1d\xeb\x9d\x7f\x55" //.,....9?..x....U |
// /* 0310 */ "\xbc\xd4\xac\xcd\xa3\x6d\x08\x53\xf4\xdd\xe3\xb8\x14\x47\x8c\x43" //.....m.S.....G.C |
// /* 0320 */ "\x7e\x8d\x49\x8e\x8f\x5e\x89\xc9\x39\x94\xdb\x95\xbc\x2c\xbf\x6a" //~.I..^..9....,.j |
// /* 0330 */ "\x33\x44\xfa\x30\x07\xb8\x60\x5b\x67\xf4\xe2\xd7\xa1\x47\xb4\x8e" //3D.0..`[g....G.. |
// /* 0340 */ "\x0b\x03\x6d\xef\xb5\xcd\x2b\x41\x5d\x0a\xbc\xa4\x15\xa7\xcb\x36" //..m...+A]......6 |
// /* 0350 */ "\xeb\xed\x4a\x43\x7d\xa2\xfd\x47\xa5\x9e\x87\x29\x20\x38\x3c\xd9" //..JC}..G...) 8<. |
// /* 0360 */ "\x56\x69\x3b\x15\xfc\x3e\x1e\x27\x23\x9c\x7e\x17\x9a\x8d\x3e\xed" //Vi;..>.'#.~...>. |
// /* 0370 */ "\x79\xa4\x3c\xdf\x37\x2e\xf3\x06\xcb\x9b\xd5\xbd\x3e\xbd\xf0\xc4" //y.<.7.......>... |
// /* 0380 */ "\x12\xb6\x78\x63\x60\xd4\xb7\x0c\x83\xf5\x54\xb5\xaa\xeb\x45\x4b" //..xc`.....T...EK |
// /* 0390 */ "\xb0\x6f\x39\x04\x14\x2f\xe5\x37\xe8\x35\xba\x61\x71\x19\xcc\xde" //.o9../.7.5.aq... |
// /* 03a0 */ "\xd3\x9e\x92\xbf\xfe\x82\x92\x86\xc4\x89\x4d\x3d\xce\x34\xd8\xd7" //..........M=.4.. |
// /* 03b0 */ "\x6b\x4f\xcd\x1e\x54\x9d\x18\x04\x6f\x9d\x6d\x68\x68\x32\xf6\x11" //kO..T...o.mhh2.. |
// /* 03c0 */ "\xa5\x2a\xb5\xf7\x0a\x06\xdb\x25\x71\xf7\x16\xd0\xa8\x6b\xde\x81" //.*.....%q....k.. |
// /* 03d0 */ "\x94\xcd\x37\xad\x23\x22\xc7\xeb\xad\xc5\x35\x9d\x70\xf2\x91\x1c" //..7.#"....5.p... |
// /* 03e0 */ "\x34\x71\x3f\xbd\x6a\x51\xeb\x6b\x35\x45\x51\xb7\xb1\x54\x4c\x33" //4q?.jQ.k5EQ..TL3 |
// /* 03f0 */ "\x7f\x4f\x68\xd9\x9b\xbe\xff\xab\x0d\x7b\xa2\xe9\xab\x12\xfe\x80" //.Oh......{...... |
// /* 0400 */ "\xca\x69\x75\x99\x77\xac\x98\x8c\x3a\xd4\x52\x22\x2a\x8f\x7b\x6e" //.iu.w...:.R"*.{n |
// /* 0410 */ "\x4b\x28\x56\x6a\xef\xd7\x78\x4f\x52\xcb\x1e\x64\xf3\xa2\xbe\x0d" //K(Vj..xOR..d.... |
// /* 0420 */ "\xad\x3f\xb5\x36\x8d\x9f\x41\xde\x8f\x56\xba\x3b\x43\xaf\x8c\xc8" //.?.6..A..V.;C... |
// /* 0430 */ "\x3b\x22\x86\xbe\x8b\xd2\x33\x6b\xd6\xd2\x87\xd3\xf6\x7c\x9a\x47" //;"....3k.....|.G |
// /* 0440 */ "\x13\x53\x62\x89\x63\xb6\xa2\x81\x88\x28\x8f\xc3\x3d\x91\x19\x8a" //.Sb.c....(..=... |
// /* 0450 */ "\xc8\x91\x64\x25\xed\xed\x41\xfa\x19\xba\x1b\x24\xc6\xa7\x45\xef" //..d%..A....$..E. |
// /* 0460 */ "\xad\x5d\x4c\x1c\x9f\x63\x08\x19\x18\xaf\x50\xbe\xd8\x61\xc7\x6e" //.]L..c....P..a.n |
// /* 0470 */ "\x09\x4b\xee\x1b\x6b\xbe\xbe\x1f\x7c\x48\x60\x45\x3c\xfb\x59\x6b" //.K..k...|H`E<.Yk |
// /* 0480 */ "\x0b\xca\x84\x60\x0a\xe4\xc9\x11\x91\xc3\x2f\x51\x27\xb9\x26\x2c" //...`....../Q'.&, |
// /* 0490 */ "\xda\x45\x36\xf5\xfd\xad\x7f\xd5\x91\x60\x7d\x9c\x7e\xf8\x5b\x9c" //.E6......`}.~.[. |
// /* 04a0 */ "\xb9\xb1\xa7\x77\x95\x1c\x3a\x40\x6c\xdf\x76\x29\x8f\x77\xe9\x10" //...w..:@l.v).w.. |
// /* 04b0 */ "\x4c\x2d\x56\xd5\x4e\x6a\x69\xb1\x18\xac\x2d\xa5\x65\x59\xb7\xd2" //L-V.Nji...-.eY.. |
// /* 04c0 */ "\xcd\x72\x27\xad\x0a\x93\x74\x0e\xcc\xa5\x96\x79\x8d\x82\x4d\xe7" //.r'...t....y..M. |
// /* 04d0 */ "\x56\x66\x58\x74\x54\x0a\xd1\x7b\x39\x79\x00\x3b\x7b\x8d\x3c\xa9" //VfXtT..{9y.;{.<. |
// /* 04e0 */ "\x70\xd9\x83\x44\xe6\x46\x67\x87\x9b\x7a\x3c\x87\x51\x7e\x4e\xb5" //p..D.Fg..z<.Q~N. |
// /* 04f0 */ "\xfe\x00\x86\x8f\x59\x07\xed\xc1\x63\x3b\xd6\x39\x3b\x97\x9c\xc5" //....Y...c;.9;... |
// /* 0500 */ "\x00\x18\x52\xbd\x01\x52\xf1\xea\x60\x76\xb2\xef\x23\x0d\x2c\x50" //..R..R..`v..#.,P |
// /* 0510 */ "\xeb\x72\x4c\xce\x87\x72\x64\xee\xcf\x04\x5b\xf9\x57\x03\x25\x97" //.rL..rd...[.W.%. |
// /* 0520 */ "\xd1\x24\x40\x12\xa3\xa0\x62\xea\x9e\x0f\xde\xdb\x87\xbd\xca\x32" //.$@...b........2 |
// /* 0530 */ "\x46\x34\x5e\x54\xcf\xcb\xd2\x1a\x2c\x5a\xbd\x24\x4c\xef\xf6\x12" //F4^T....,Z.$L... |
// /* 0540 */ "\x9a\xb8\x57\x5b\x6c\x19\xb2\x91\x38\x00\x4f\x26\x5e\x03\x1d\xf5" //..W[l...8.O&^... |
// /* 0550 */ "\x0c\x07\x74\x8d\xbc\xdc\xf8\xb3\x25\xec\xf6\xb4\x38\xff\xe8\x95" //..t.....%...8... |
// /* 0560 */ "\x52\x40\x66\xf0\x22\xd8\x87\x33\x03\x3e\xb2\x32\xf8\x32\xa5\x12" //R@f."..3.>.2.2.. |
// /* 0570 */ "\xe4\x5b\x24\x0e\x06\x78\x19\x7f\x4b\x50\x51\x15\x01\xb0\xf9\xf6" //.[$..x..KPQ..... |
// /* 0580 */ "\x51\x41\x43\xfb\x36\x15\xee\xd1\x29\xff\x4f\x2c\x74\xf5\x07\xe8" //QAC.6...).O,t... |
// /* 0590 */ "\xdd\xa9\x0f\x2d\x05\x90\x7b\x36\xca\xde\xc6\x50\xcb\x75\xcf\xcf" //...-..{6...P.u.. |
// /* 05a0 */ "\x28\x79\x44\xf7\xbd\xda\x46\xd0\xbe\x1c\x98\xc5\x1a\x1e\xb0\x7f" //(yD...F......... |
// /* 05b0 */ "\xa9\x82\x1b\xc8\x60\x15\x87\x86\xbd\x54\x3c\xc1\x25\xe6\xc0\xc1" //....`....T<.%... |
// /* 05c0 */ "\x80\xc7\xfd\xb9\x0f\x8d\x31\x03\x40\x06\x34\x9a\x20\x12\x85\xfc" //......1.@.4. ... |
// /* 05d0 */ "\x9a\x00\x34\xb8\x21\x83\x23\x18\x0a\xcd\x4e\x4d\x7e\x19\x40\x90" //..4.!.#...NM~.@. |
// /* 05e0 */ "\xaa\xa8\x8e\xbe\xf2\x9d\x6e\x4d\x5c\x17\x3b\x91\xa6\x13\x29\x00" //......nM..;...). |
// /* 05f0 */ "\xbb\x67\x6a\x23\xf7\x38\xb6\x42\xab\x7c\x5d\x06\x0c\x87\x93\x47" //.gj#.8.B.|]....G |
// /* 0600 */ "\x2f\x34\xa4\x12\x67\xf1\xf0\x8d\x73\x17\x52\xf4\xdd\xfe\x3c\x3a" ///4..g...s.R...<: |
// /* 0610 */ "\xf1\xd0\x42\x57\xbe\xa5\x96\xe1\x9d\x1e\x2e\xaa\xbe\x1f\x57\x09" //..BW..........W. |
// /* 0620 */ "\xc0\x47\x1c\xed\x3b\x18\x2b\xbf\x0d\xc5\x20\x48\x01\xe6\xba\x50" //.G..;.+... H...P |
// /* 0630 */ "\xae\x4e\xf4\x67\x1a\x19\x60\x3d\x3b\x06\x2e\x70\x75\x0b\x75\xad" //.N.g..`=;..pu.u. |
// /* 0640 */ "\x20\xf3\x6f\x84\xcc\x6d\xac\x20\xfa\x53\x82\x3d\x53\x52\xf6\x5a" // .o..m. .S.=SR.Z |
// /* 0650 */ "\xe6\x2e\xf3\x54\x00\xcd\xd3\x78\x77\x6d\x50\x12\xab\x15\x46\x1b" //...T...xwmP...F. |
// /* 0660 */ "\xaa\x5b\x52\xd6\x7b\x39\xd9\xea\x10\xce\xaa\xe2\x0b\x0a\x61\x82" //.[R.{9........a. |
// /* 0670 */ "\x84\xda\xf5\x39\x82\x27\x66\x19\xd3\x8d\xc5\xff\xe6\x4f\xd9\x0b" //...9.'f......O.. |
// /* 0680 */ "\xa1\x5f\xc8\xdc\x0f\xed\x71\xd0\x14\xf2\x21\xbb\x84\x9e\xcc\x1d" //._....q...!..... |
// /* 0690 */ "\x7c\x71\xed\xe0\xc4\x42\xe5\x00\x64\x91\x45\x64\x85\xd0\x9b\x84" //|q...B..d.Ed.... |
// /* 06a0 */ "\x03\x18\xa3\xae\x27\x5a\x3b\x8d\xcb\xbd\xc9\x3a\x38\xbc\xc4\x59" //....'Z;....:8..Y |
// /* 06b0 */ "\xdc\xa1\x8f\x2a\x3d\x06\xcf\x07\x1c\x6f\x26\xa6\x31\x7b\xeb\x19" //...*=....o&.1{.. |
// /* 06c0 */ "\x24\x87\xdb\x9e\x3e\x85\xf2\x78\x1f\x93\xf3\x31\xbe\x7b\xd0\xe0" //$...>..x...1.{.. |
// /* 06d0 */ "\xec\x5b\xe5\xd4\x2e\x6c\x81\x61\x1f\xbb\x91\xab\xe2\xbf\x4b\x1a" //.[...l.a......K. |
// /* 06e0 */ "\xa2\x8b\xd5\xc5\xf7\xab\x63\x66\x93\x75\xea\x9b\xa7\x12\x93\x85" //......cf.u...... |
// /* 06f0 */ "\xfe\xe5\xcd\x18\xe1\xc8\x6a\xdf\xaf\x16\x75\xd2\xc8\x47\xf5\xa0" //......j...u..G.. |
// /* 0700 */ "\x5d\x08\x42\xa1\x04\x06\x68\x90\xa9\x0b\x38\xa5\xf5\xf0\x3e\x41" //].B...h...8...>A |
// /* 0710 */ "\xb3\x48\x85\x89\xab\x33\x7b\xde\x4a\x52\xe7\x4b\x5a\x25\xbe\x5d" //.H...3{.JR.KZ%.] |
// /* 0720 */ "\x7e\x28\xb7\x8c\x13\x53\xaf\x13\xa1\x0b\x09\x5f\xa6\x41\xd4\x62" //~(...S....._.A.b |
// /* 0730 */ "\x4a\xac\x8f\x11\x9a\x9f\xd2\x78\x8a\x76\xd4\x04\xbb\x65\xc4\x21" //J......x.v...e.! |
// /* 0740 */ "\x3b\x76\x78\x9e\xc1\xe5\x1c\x2f\xe3\x1a\x9d\xec\x6c\xf6\x59\xcf" //;vx..../....l.Y. |
// /* 0750 */ "\x09\xe9\xd1\x51\x9c\xac\xab\xa9\xee\x95\x8f\xa9\xc5\x21\x25\x07" //...Q.........!%. |
// /* 0760 */ "\xdf\xc3\x6a\x65\x80\x8a\x22\xcc\x20\xc0\x77\x2d\xca\xa7\x52\xb1" //..je..". .w-..R. |
// /* 0770 */ "\x3d\xe5\x48\x64\x23\x25\x3e\x54\xa9\xba\x97\xaf\xb0\x96\xdc\xa6" //=.Hd#%>T........ |
// /* 0780 */ "\x09\xb6\x1b\xed\xf6\x5d\xf3\x96\xd7\xb7\xd4\xec\x8e\xed\xcc\x2a" //.....].........* |
// /* 0790 */ "\x71\x76\xb7\x3d\xe2\x3c\x5f\x2c\x63\x54\x97\x31\xdd\x09\xf1\x52" //qv.=.<_,cT.1...R |
// /* 07a0 */ "\xa4\x9b\xe5\x6c\x07\x26\xe8\x3d\x32\x85\x27\x3b\x9c\xc9\x66\xbf" //...l.&.=2.';..f. |
// /* 07b0 */ "\xa2\x8d\x69\x1f\x66\x84\x12\xad\x08\x86\x3a\x29\xb1\x81\x8a\xd5" //..i.f.....:).... |
// /* 07c0 */ "\x54\x41\xbb\xe3\x4b\x10\x81\x78\x24\xff\x38\xe2\x7b\x1b\x9e\xc7" //TA..K..x$.8.{... |
// /* 07d0 */ "\xc2\x94\x13\x2d\x57\xe9\xec\xf6\x68\xf1\x97\xab\xa4\x17\xe5\xe5" //...-W...h....... |
// /* 07e0 */ "\x64\xa3\x8e\xb1\xf1\x6a\x73\x6e\xbf\x2d\xe9\x31\x7d\x2a\x9a\xbb" //d....jsn.-.1}*.. |
// /* 07f0 */ "\xa4\x2c\xe8\x7a\xf5\x4e\x9d\x7e\x2b\x51\x75\x1e\x15\xba\xe3\x61" //.,.z.N.~+Qu....a |
// /* 0800 */ "\x35\x67\xde\xa2\x1d\x37\xdf\xde\x77\x77\x3a\x1f\x2a\x49\xbe\xf7" //5g...7..ww:.*I.. |
// /* 0810 */ "\xd3\x4f\x05\x9d\x87\x7c\xe2\x60\xcf\x3a\x2b\x32\x08\x89\x3e\x49" //.O...|.`.:+2..>I |
// /* 0820 */ "\x09\x03\xac\x68\xf0\x65\x0b\x30\x80\x62\x45\xc9\xac\x8f\x8a\x63" //...h.e.0.bE....c |
// /* 0830 */ "\x74\x86\x50\x62\x9f\xf4\xd2\x5f\x40\x87\xbf\xce\xac\x81\xda\x34" //t.Pb..._@......4 |
// /* 0840 */ "\xb5\xd2\xcc\xad\x86\xf8\x37\x46\x19\x32\x00\x45\xdf\xe4\xe0\x5c" //......7F.2.E.... |
// /* 0850 */ "\x64\x72\x85\xf5\x28\xa4\x8a\xe1\x7a\xd0\x33\xd0\xb2\xf2\x92\x36" //dr..(...z.3....6 |
// /* 0860 */ "\xe4\xec\x16\x2b\xef\x60\xa7\xd0\x73\x4a\x91\x44\xcb\xff\x48\x34" //...+.`..sJ.D..H4 |
// /* 0870 */ "\x06\xef\x04\x99\xe1\x05\xf5\xcd\xe1\x68\x87\xb7\x22\xd4\xa4\x0f" //.........h.."... |
// /* 0880 */ "\x6f\xb3\x33\xc9\xd2\x6e\xc0\xa0\x29\x44\xb9\x1b\x56\x6c\xb1\xb4" //o.3..n..)D..Vl.. |
// /* 0890 */ "\x32\xc2\xa7\x72\x2a\x66\x49\x7b\xe0\x68\x5f\x17\xc7\xcc\x6d\xb8" //2..r*fI{.h_...m. |
// /* 08a0 */ "\xff\x6e\xcf\x14\xb3\x37\x89\x59\x98\x31\xec\xe2\xe7\x13\x8c\x3c" //.n...7.Y.1.....< |
// /* 08b0 */ "\x41\x7e\x06\xf5\x2f\x49\xc1\xc2\x7d\xc4\x3f\x97\x15\xc0\xa9\xa1" //A~../I..}.?..... |
// /* 08c0 */ "\x4f\x55\x20\xaa\x66\x15\x90\xe3\xa5\x71\xcb\x65\x05\x6a\x0f\xbe" //OU .f....q.e.j.. |
// /* 08d0 */ "\x18\xef\x72\x7c\x5b\xfe\x17\x19\x1f\x9b\x1e\xf6\x80\xe4\x80\xfe" //..r|[........... |
// /* 08e0 */ "\x06\x65\xa0\x1e\x6a\xd4\x0f\x43\xc7\xeb\x7a\xee\x69\x54\xcb\xf9" //.e..j..C..z.iT.. |
// /* 08f0 */ "\x4d\xc2\xbf\x17\xf6\x61\xb8\x91\xe4\xc7\x6e\xec\x8a\xe6\x5c\x13" //M....a....n..... |
// /* 0900 */ "\xb2\xd9\x62\xec\x1f\x88\xbb\x72\x4c\xf6\x83\xa8\x06\x8e\xe7\x20" //..b....rL......  |
// /* 0910 */ "\x65\x1e\x3f\x9a\x69\x4a\x11\xea\x53\xbc\xd5\xf0\xd6\x91\xfe\x1b" //e.?.iJ..S....... |
// /* 0920 */ "\x1e\x79\x07\xe3\x13\xc3\x9a\xf1\x8f\x69\xdd\x5c\x05\x14\x8d\x65" //.y.......i.....e |
// /* 0930 */ "\x25\x53\x97\xe5\x67\x71\xc9\xd8\x1d\x5c\xba\xef\x70\x89\xe0\xae" //%S..gq......p... |
// /* 0940 */ "\xbc\x28\x71\x22\x71\x2a\x02\x2c\x65\x8b\x6d\x8d\x84\x7a\x3a\x5b" //.(q"q*.,e.m..z:[ |
// /* 0950 */ "\xbd\x2c\xbc\x56\x42\x40\x57\x72\x18\x9d\xd0\x7c\x8d\xf7\x11\x92" //.,.VB@Wr...|.... |
// /* 0960 */ "\x8a\x73\xf9\x9e\x46\xb4\xb6\x59\x36\xc5\x50\x7c\x1e\xd2\x87\x1d" //.s..F..Y6.P|.... |
// /* 0970 */ "\xce\x4e\x8d\xfa\x80\x40\x59\xf8\x03\x02\x56\x94\x34\xe1\x1b\xf1" //.N...@Y...V.4... |
// /* 0980 */ "\xfc\x06\x35\xb6\xec\xce\x93\xd4\x51\xeb\xcd\xd1\x15\xf3\xa7\x88" //..5.....Q....... |
// /* 0990 */ "\x4e\xfa\x5e\x45\x65\x4f\x20\x33\x27\x63\x79\xc6\xdb\x13\xf5\x88" //N.^EeO 3'cy..... |
// /* 09a0 */ "\x2c\xf4\xb8\x3b\xaa\x43\x97\x1c\x76\x42\xf0\x2d\x6b\x75\xa3\x0a" //,..;.C..vB.-ku.. |
// /* 09b0 */ "\x22\x86\xff\xbc\x5a\xc8\x0c\xe5\x08\x68\xbe\x92\xb2\x88\x1a\x1b" //"...Z....h...... |
// /* 09c0 */ "\xbb\x54\xc8\x31\xaf\x3b\xbc\xf4\xcc\xa8\x75\x7d\x39\xe5\xa3\x7e" //.T.1.;....u}9..~ |
// /* 09d0 */ "\xeb\x10\xa5\xec\x34\x70\x06\x12\xc6\xf8\xcf\x23\x66\xc1\xd6\x38" //....4p.....#f..8 |
// /* 09e0 */ "\x49\x1d\x2f\x37\xe2\xa9\xe0\x7b\x5b\x38\xbd\x8e\x51\x7b\xb5\xc2" //I./7...{[8..Q{.. |
// /* 09f0 */ "\xd2\x1f\xe5\x0f\xcf\xb7\xa5\xe5\x48\x65\x4f\xb9\x0c\x33\x63\x50" //........HeO..3cP |
// /* 0a00 */ "\x66\x09\xa7\x7b\xe4\xf1\xe7\x23\xd1\x72\x97\xb8\x12\x34\x86\x43" //f..{...#.r...4.C |
// /* 0a10 */ "\xcb\x22\x42\x41\x47\x37\x0c\x1f\x5f\xba\xbf\x39\x78\x0d\x78\xd5" //."BAG7.._..9x.x. |
// /* 0a20 */ "\xb6\xe5\x12\x4d\x4c\x78\xc3\x6a\xf3\x64\x4b\x90\x64\x24\xfd\x9c" //...MLx.j.dK.d$.. |
// /* 0a30 */ "\x6f\xac\x72\xbb\x18\x14\x79\x16\xc4\x5f\x68\x9a\xe3\x74\x1e\xed" //o.r...y.._h..t.. |
// /* 0a40 */ "\xbc\x98\xe9\xae\xe0\x76\xd4\x59\x97\x52\xa9\x60\x89\x58\x00\x55" //.....v.Y.R.`.X.U |
// /* 0a50 */ "\x54\xb6\x8c\x48\xe7\x5a\x1c\xa8\x51\xce\x32\x7d\xa7\x78\x4c\xbd" //T..H.Z..Q.2}.xL. |
// /* 0a60 */ "\xd0\x27\x8b\xe7\x71\xef\xcc\xf4\x5e\x79\xce\xaf\x49\x3c\xdb\x1c" //.'..q...^y..I<.. |
// /* 0a70 */ "\xd9\x91\x8e\x3b\xba\x43\x8a\xe8\x74\xfb\x26\xad\xc3\xd0\xa7\xf5" //...;.C..t.&..... |
// /* 0a80 */ "\xff\x7d\x82\x46\x95\x9c\x6b\xd7\xce\x90\xd8\x14\xe1\xea\x71\xad" //.}.F..k.......q. |
// /* 0a90 */ "\x83\x88\x0f\x34\x8f\x7b\x18\xf9\xc4\x36\x47\xfc\x31\xd9\x1f\xc7" //...4.{...6G.1... |
// /* 0aa0 */ "\x15\xa1\xf4\xbd\xd2\xd5\x89\xe0\xc8\x13\x75\x37\xac\xed\x2f\x3a" //..........u7../: |
// /* 0ab0 */ "\x86\xea\x56\xc1\xa7\xbe\x27\x4a\x65\x88\x9b\x6c\xb4\x3c\xbf\x98" //..V...'Je..l.<.. |
// /* 0ac0 */ "\x6b\xcc\x71\xda\x82\x30\x91\x18\x18\xca\x2e\xb2\xad\xb6\xbb\x9b" //k.q..0.......... |
// /* 0ad0 */ "\x43\xc2\x4b\x29\x92\xca\xf7\x1c\x71\x44\x81\x54\xa7\x57\xfa\x16" //C.K)....qD.T.W.. |
// /* 0ae0 */ "\xc9\x0c\xd5\x56\x59\x3f\xe3\xd9\xc0\x03\xea\x25\x36\x54\x6d\xc4" //...VY?.....%6Tm. |
// /* 0af0 */ "\x72\x42\x4a\x5b\x03\x0e\x12\xa0\x90\xd9\x64\xb4\x91\x4d\x0f\x75" //rBJ[......d..M.u |
// /* 0b00 */ "\x05\xe8\x1a\x3e\x7b\x71\x54\xf0\xb0\xc4\xdd\x27\x85\x5f\x80\x66" //...>{qT....'._.f |
// /* 0b10 */ "\x49\xfe\x0c\xa8\x53\xd2\x20\xbb\xbc\xe6\xe3\xca\x5c\x24\x2a\x64" //I...S. ......$*d |
// /* 0b20 */ "\x14\x71\x9b\xdd\x09\xef\xe7\x50\x9c\x8a\x3c\xe6\x16\x63\x9f\xce" //.q.....P..<..c.. |
// /* 0b30 */ "\x9d\x5f\xde\x07\x8f\x96\xfe\xe9\x56\x6e\xda\xe2\x82\x23\x92\xc3" //._......Vn...#.. |
// /* 0b40 */ "\x47\xc2\xcd\x2b\x93\x1f\x77\x86\xbd\x3b\xb2\x38\xdd\xc1\xb2\x0e" //G..+..w..;.8.... |
// /* 0b50 */ "\x8c\x43\xff\x20\xc3\x8e\x8f\x16\x3e\x17\x16\x42\x24\x46\xbd\xfe" //.C. ....>..B$F.. |
// /* 0b60 */ "\x9d\x68\xaf\x6a\x4e\x6d\xd8\xd0\x39\xca\x3a\x2d\x92\xfc\x26\x71" //.h.jNm..9.:-..&q |
// /* 0b70 */ "\x4a\x0f\xe1\x30\x91\xa5\x9c\xce\xb5\x6d\x12\x9c\x8d\x80\x91\xa5" //J..0.....m...... |
// /* 0b80 */ "\x26\x8d\x1a\x91\x29\xdd\xad\x1c\x03\xfa\x55\x41\xec\x8a\x0c\x9f" //&...).....UA.... |
// /* 0b90 */ "\x5d\x02\x86\x40\x38\x6f\x27\x61\x7f\x21\xe0\x8b\xfd\xb2\x8b\x86" //]..@8o'a.!...... |
// /* 0ba0 */ "\x4e\xd3\x79\x60\xa5\x19\x1c\xf2\xc7\x19\xff\x50\xd9\x78\xc3\x09" //N.y`.......P.x.. |
// /* 0bb0 */ "\xe8\xec\x74\xea\x1f\xfe\x9e\xe6\xa3\xa3\x11\x94\xaf\x00\xd7\x10" //..t............. |
// /* 0bc0 */ "\x51\xeb\x1e\x6c\xfc\x34\x34\xdd\x96\x5e\x57\x4c\x3a\x13\x8d\xc2" //Q..l.44..^WL:... |
// /* 0bd0 */ "\x27\x04\xb9\xb4\x13\x23\x31\xa1\x3c\xd1\x44\x5c\xec\x27\xad\x7e" //'....#1.<.D..'.~ |
// /* 0be0 */ "\x2d\x9d\x09\x76\xf4\x5e\xf2\x14\xe0\xba\x7d\xa0\xb1\xdc\x44\xe8" //-..v.^....}...D. |
// /* 0bf0 */ "\xff\x92\xef\xfb\x20\xcc\x56\x39\x86\x3a\x45\x94\x32\xe2\x47\xbf" //.... .V9.:E.2.G. |
// /* 0c00 */ "\x36\x8b\x04\xe5\x61\x94\xb9\x71\xca\xdc\xd7\xf9\xf3\xe1\x07\xe5" //6...a..q........ |
// /* 0c10 */ "\xec\xd4\x1b\x64\xb6\x07\x63\xde\x29\xe0\x0e\x08\x90\x11\xbe\x44" //...d..c.)......D |
// /* 0c20 */ "\x78\xf3\x92\x04\x05\x7a\x19\x00\xa0\xb3\xbb\x4f\x1d\x81\x22\xc6" //x....z.....O..". |
// /* 0c30 */ "\x00\x5f\xcc\xec\x3d\x58\xd4\xe0\x25\x4f\xa6\xbe\x6b\x4d\xc7\x9d" //._..=X..%O..kM.. |
// /* 0c40 */ "\x55\x6e\x01\x9f\xb2\xb3\x80\x35\x46\x22\x3d\x85\x04\xba\xf5\x73" //Un.....5F"=....s |
// /* 0c50 */ "\xb4\x2f\xe3\x72\xd0\xb7\xad\x9a\xa0\xa9\x77\xdc\x0a\x5d\x81\x05" //./.r......w..].. |
// /* 0c60 */ "\xd2\x17\x9c\x36\x8c\x35\x0e\xaa\x78\x98\xb2\x16\x30\x4e\x9b\x6e" //...6.5..x...0N.n |
// /* 0c70 */ "\xf0\x8d\x49\xa9\xf7\xf0\x80\xd3\x6a\xdc\xf6\xa4\x40\x52\x98\x11" //..I.....j...@R.. |
// /* 0c80 */ "\xfe\xff\xe6\x3c\x0b\xa7\x62\x22\xb4\x1e\x2f\x24\x00\x16\x2e\x58" //...<..b"../$...X |
// /* 0c90 */ "\x13\x96\x8a\xef\x25\xd9\x1a\x13\xd1\x30\x75\x81\xd8\xa7\x34\x27" //....%....0u...4' |
// /* 0ca0 */ "\x82\x06\x41\x78\xcb\xfa\x50\x7d\xdc\x8e\xad\xdb\xf6\x36\x50\x52" //..Ax..P}.....6PR |
// /* 0cb0 */ "\x03\x69\x11\x87\xbd\xfd\x48\x57\x0e\x07\x21\xc3\xe3\x03\x01\xbf" //.i....HW..!..... |
// /* 0cc0 */ "\x39\x8c\x79\x34\x60\xa2\xee\x03\x6a\xdd\x0b\x26\x1d\x4a\xdb\x7f" //9.y4`...j..&.J.. |
// /* 0cd0 */ "\x9e\x1f\x85\x9e\xcd\x8a\x8d\x76\x8c\xf0\x30\xeb\x32\xa5\x2a\x22" //.......v..0.2.*" |
// /* 0ce0 */ "\x60\x38\xc4\x64\x3b\x65\x15\x26\x7c\xe0\x19\x37\xf2\x07\xdc\x7b" //`8.d;e.&|..7...{ |
// /* 0cf0 */ "\xe3\xc2\x86\x50\xd3\xe9\x23\x22\x10\xc1\xba\xdd\xad\x0b\x63\xf4" //...P..#"......c. |
// /* 0d00 */ "\x33\xa9\xb9\xba\x2e\x68\x93\x0f\xd1\x0f\x2d\xbd\x6a\xf1\x77\xe0" //3....h....-.j.w. |
// /* 0d10 */ "\xc4\xc9\xcb\xca\x5a\x56\x15\x56\x40\x3c\x3a\xeb\x63\x54\x47\xc9" //....ZV.V@<:.cTG. |
// /* 0d20 */ "\x9e\xe7\x8d\xf4\x01\x34\xe1\x81\x5d\x01\xb8\xf2\xd4\xaf\x7c\xb4" //.....4..].....|. |
// /* 0d30 */ "\xfb\xc6\x90\x50\x9c\x4b\xff\x9f\x71\x62\x9a\x21\xdd\xd5\x73\x56" //...P.K..qb.!..sV |
// /* 0d40 */ "\xf1\x7c\x87\xbc\xb2\xab\xdd\x2a\x4b\xea\x73\x14\x4f\x4c\xd7\xed" //.|.....*K.s.OL.. |
// /* 0d50 */ "\x83\x7b\x29\xab\xfa\x3b\x81\x55\x47\x79\x95\x61\xb0\x1f\x20\x12" //.{)..;.UGy.a.. . |
// /* 0d60 */ "\x20\x28\x32\x0e\x1e\x79\xef\xfa\x4d\x30\xde\x76\x6a\xec\x56\x83" // (2..y..M0.vj.V. |
// /* 0d70 */ "\x3f\x5c\x24\xee\x68\xd5\xc6\x9e\x43\xa5\x14\x1c\x96\x1a\xf2\x8f" //?.$.h...C....... |
// /* 0d80 */ "\xb5\x89\x55\xf2\x23\x6e\x2f\x32\xa7\xa3\x29\x65\xef\xb6\x91\x86" //..U.#n/2..)e.... |
// /* 0d90 */ "\x96\x09\xac\x56\x33\x38\x3b\x5a\x32\xc0\x90\xe7\x80\xb1\x87\x8f" //...V38;Z2....... |
// /* 0da0 */ "\xaa\x4f\xa0\xb7\xff\x71\xb3\xbb\x63\xe7\x79\xb1\xf7\xbc\xa8\xd8" //.O...q..c.y..... |
// /* 0db0 */ "\xa9\x6c\x80\x6f"                                                 //.l.o |
// Dump done on RDP Wab Target (4) 3508 bytes |
// LOOPING on PDUs: 3485 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_ORDERS |
// process_orders bpp=16 |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,0,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,10,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(127,10,829,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,10,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,26,956,7) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,26,20,7) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,33,1024,8) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,41,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,41,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,57,1024,5) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,62,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(214,62,5,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(920,62,5,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,62,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,82,1024,5) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,87,15,140) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,87,15,140) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,227,1024,475) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,702,787,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(820,702,15,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,702,69,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,702,15,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,702,20,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,703,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,703,29,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,703,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,703,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(915,705,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,705,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,705,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,721,29,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,721,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,721,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,723,69,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,723,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,723,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,725,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,735,810,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(873,735,40,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(987,735,37,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,758,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(20,10,107,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(20,10,108,17) op=(20,10,108,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=1a data_len=26 [0x00 0x00 0x01 0x09 0x02 0x0e 0x03 0x09 0x04 0x09 0x02 0x04 0x03 0x09 0x04 0x09 0x05 0x04 0x06 0x09 0x04 0x09 0x06 0x04 0x06 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,41,204,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(20,41,93,17) op=(20,41,93,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=39 data_len=24 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09 0x0d 0x05 0x0e 0x04 0x09 0x0b 0x0f 0x05 0x10 0x09 0x11 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,41,706,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(224,41,46,17) op=(224,41,46,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=39 data_len=12 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,41,84,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(930,41,58,17) op=(930,41,58,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=39 data_len=16 [0x12 0x00 0x09 0x09 0x0f 0x05 0x0c 0x09 0x0f 0x05 0x13 0x09 0x0f 0x08 0x14 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(16,63,197,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,62,198,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(213,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,81,199,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(220,63,699,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,62,700,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(919,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,81,701,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(926,63,82,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,62,83,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1008,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,81,84,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,10,48,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(957,11,45,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(963,13,34,17) op=(963,13,34,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3c3 glyph_y=1d data_len=12 [0x15 0x00 0x16 0x08 0x14 0x03 0x0c 0x03 0x0b 0x05 0x09 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,10,46,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,12,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1002,10,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,31,48,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,87,994,20) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,89,200,16) color=0x000273) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(20,90,86,17) op=(20,90,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=6a data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x21 0x0b 0x22 0x09 0x23 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,89,702,16) color=0x000273) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(224,90,263,17) op=(224,90,263,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=6a data_len=68 [0x24 0x00 0x25 0x09 0x26 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x27 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x20 0x0e 0x21 0x0b 0x22 0x09 0x23 0x08 0x28 0x09 0x11 0x05 0x09 0x09 0x16 0x05 0x28 0x03 0x05 0x05 0x29 0x09 0x04 0x09 0x02 0x04 0x29 0x09 0x23 0x09 0x2a 0x09 0x2b 0x05 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,89,80,16) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(930,90,31,17) op=(930,90,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=6a data_len=6 [0x2b 0x00 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,107,994,20) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,109,200,16) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(20,110,78,17) op=(20,110,78,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=7e data_len=18 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x00 0x0b 0x11 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,109,702,16) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(224,110,255,17) op=(224,110,255,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=7e data_len=66 [0x24 0x00 0x25 0x09 0x26 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x27 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x20 0x0e 0x00 0x0b 0x11 0x09 0x28 0x09 0x11 0x05 0x09 0x09 0x16 0x05 0x28 0x03 0x05 0x05 0x29 0x09 0x04 0x09 0x21 0x04 0x02 0x09 0x05 0x09 0x2a 0x09 0x2b 0x05 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,109,80,16) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(930,110,31,17) op=(930,110,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=7e data_len=6 [0x2b 0x00 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,127,994,20) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,129,200,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(20,130,77,17) op=(20,130,77,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=92 data_len=18 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x21 0x0b 0x22 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,129,702,16) color=0x00cebd) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(224,130,254,17) op=(224,130,254,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=92 data_len=64 [0x08 0x00 0x25 0x08 0x26 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x27 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x20 0x0e 0x21 0x0b 0x22 0x09 0x28 0x08 0x11 0x05 0x10 0x09 0x2d 0x09 0x28 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x2e 0x04 0x05 0x09 0x2a 0x09 0x2b 0x05 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,129,80,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(930,130,31,17) op=(930,130,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=92 data_len=6 [0x2b 0x00 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,147,994,20) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,149,200,16) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(20,150,86,17) op=(20,150,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=a6 data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x21 0x0b 0x22 0x09 0x06 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,149,702,16) color=0x00ef7e) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(224,150,189,17) op=(224,150,189,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=a6 data_len=44 [0x08 0x00 0x1e 0x08 0x2f 0x09 0x01 0x09 0x20 0x0e 0x21 0x0b 0x22 0x09 0x06 0x08 0x28 0x09 0x11 0x05 0x10 0x09 0x2d 0x09 0x28 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x29 0x04 0x03 0x09 0x2a 0x09 0x30 0x05 0x31 0x09 0x1d 0x0a] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,149,80,16) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(930,150,30,17) op=(930,150,30,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=a6 data_len=6 [0x30 0x00 0x31 0x09 0x1d 0x0a] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,167,994,20) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,169,200,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(20,170,107,17) op=(20,170,107,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=ba data_len=26 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x1e 0x07 0x0f 0x09 0x0c 0x09 0x0b 0x05 0x11 0x09 0x08 0x09 0x25 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,169,702,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(224,170,259,17) op=(224,170,259,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=ba data_len=62 [0x1e 0x00 0x0f 0x09 0x0c 0x09 0x0b 0x05 0x11 0x09 0x08 0x09 0x25 0x08 0x1f 0x09 0x08 0x07 0x13 0x08 0x13 0x08 0x0f 0x08 0x10 0x09 0x1e 0x09 0x0c 0x09 0x01 0x05 0x08 0x0e 0x11 0x08 0x11 0x09 0x1f 0x09 0x1e 0x07 0x0f 0x09 0x0c 0x09 0x0b 0x05 0x11 0x09 0x08 0x09 0x25 0x08 0x2a 0x09 0x24 0x05 0x12 0x09 0x12 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,169,80,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(930,170,28,17) op=(930,170,28,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=ba data_len=6 [0x24 0x00 0x12 0x09 0x12 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,187,994,20) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,189,200,16) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(20,190,86,17) op=(20,190,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=ce data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x21 0x0b 0x22 0x09 0x06 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,189,702,16) color=0x00ef7e) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(224,190,286,17) op=(224,190,286,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=ce data_len=72 [0x32 0x00 0x08 0x09 0x33 0x08 0x24 0x05 0x25 0x09 0x26 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x27 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x20 0x0e 0x21 0x0b 0x22 0x09 0x06 0x08 0x28 0x09 0x11 0x05 0x10 0x09 0x2d 0x09 0x28 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x29 0x04 0x03 0x09 0x2a 0x09 0x2b 0x05 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,189,80,16) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(930,190,31,17) op=(930,190,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=ce data_len=6 [0x2b 0x00 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,207,994,20) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,209,200,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(20,210,86,17) op=(20,210,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=e2 data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x21 0x0b 0x22 0x09 0x23 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,209,702,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(224,210,286,17) op=(224,210,286,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=e2 data_len=72 [0x32 0x00 0x08 0x09 0x33 0x08 0x24 0x05 0x25 0x09 0x26 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x27 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x20 0x0e 0x21 0x0b 0x22 0x09 0x23 0x08 0x28 0x09 0x11 0x05 0x10 0x09 0x2d 0x09 0x28 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x23 0x04 0x23 0x09 0x2a 0x09 0x2b 0x05 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,209,80,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(930,210,31,17) op=(930,210,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=e2 data_len=6 [0x2b 0x00 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(787,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(788,703,30,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(794,705,19,17) op=(794,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=31a glyph_y=2d1 data_len=4 [0x17 0x00 0x18 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(835,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(836,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(842,705,12,17) op=(842,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=34a glyph_y=2d1 data_len=2 [0x17 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(877,704,23,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=36e glyph_y=2d1 data_len=2 [0x02 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,703,24,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,704,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(900,703,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,722,25,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,705,14,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=385 glyph_y=2d1 data_len=4 [0x19 0x00 0x02 0x05] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(930,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(931,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(937,705,12,17) op=(937,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a9 glyph_y=2d1 data_len=2 [0x1a 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(971,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(972,703,30,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(978,705,19,17) op=(978,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3d2 glyph_y=2d1 data_len=4 [0x1b 0x00 0x1a 0x07] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,63,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(811,736,60,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(817,738,49,17) op=(817,738,49,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=331 glyph_y=2f2 data_len=12 [0x1c 0x00 0x0f 0x07 0x0a 0x09 0x0f 0x09 0x10 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,61,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,737,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(871,735,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,756,63,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,735,74,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(914,736,71,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(920,738,60,17) op=(920,738,60,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=398 glyph_y=2f2 data_len=14 [0x1d 0x00 0x0f 0x0a 0x1e 0x09 0x1e 0x09 0x0b 0x09 0x13 0x09 0x0c 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,735,72,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,737,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(985,735,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,756,74,2) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 6 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x73"                                                     //..s |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 111 bytes |
// Recv done on RDP Wab Target (4) 111 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x65\x08\x00\x00\x00\x83\xb3" //...h....pe...... |
// /* 0010 */ "\x0d\xbb\x42\xb1\xc5\x85\xc0\x58\xbd\x45\x93\x28\x58\x07\x83\x90" //..B....X.E.(X... |
// /* 0020 */ "\xb3\x37\xd1\x83\xf5\xf9\xec\x37\xf7\x94\x31\xfa\x54\xba\xbb\x15" //.7.....7..1.T... |
// /* 0030 */ "\x1f\x60\xd5\xef\xbc\x82\xc5\x29\x0b\x93\x5b\xdd\xae\xf7\xb3\x64" //.`.....)..[....d |
// /* 0040 */ "\x14\xc0\x66\xda\x20\x57\x5f\x78\x3e\x59\xb7\xa8\x0b\x2b\x4d\x04" //..f. W_x>Y...+M. |
// /* 0050 */ "\x46\x9a\x9e\xe7\xf3\xc3\x5f\x41\x39\x60\x07\x3b\x3a\xa5\x82\xcf" //F....._A9`.;:... |
// /* 0060 */ "\x6f\x2c\xf5\x0c\x60\xf1\x8b\xf4\x52\x9d\xc2\xf2\xf7\x04\x86"     //o,..`...R...... |
// Dump done on RDP Wab Target (4) 111 bytes |
// LOOPING on PDUs: 89 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_ORDERS |
// process_orders bpp=16 |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(877,704,23,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=36e glyph_y=2d1 data_len=2 [0x02 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,703,24,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,704,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(900,703,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,722,25,1) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 7 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x56"                                                     //..V |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 82 bytes |
// Recv done on RDP Wab Target (4) 82 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x48\x08\x00\x00\x00\x8a\x8d" //...h....pH...... |
// /* 0010 */ "\x2b\x3c\x2c\xcd\xdc\x84\x51\xc5\xf5\x84\xf4\xb7\x6a\x33\x63\x19" //+<,...Q.....j3c. |
// /* 0020 */ "\x27\x78\x21\xa9\xbe\x14\x77\xcf\x66\x67\x45\xdb\x1f\xb1\x01\x20" //'x!...w.fgE....  |
// /* 0030 */ "\x92\xc0\x34\xeb\x57\x92\xf4\x1f\xb3\x4f\x9f\xbd\x82\x2b\xfd\x12" //..4.W....O...+.. |
// /* 0040 */ "\x9a\x09\xfc\x45\xd2\xcd\x60\x34\x72\x4d\x60\x44\x3f\x42\x90\x6d" //...E..`4rM`D?B.m |
// /* 0050 */ "\x8f\x5d"                                                         //.] |
// Dump done on RDP Wab Target (4) 82 bytes |
// LOOPING on PDUs: 60 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_ORDERS |
// process_orders bpp=16 |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,705,14,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=385 glyph_y=2d1 data_len=4 [0x19 0x00 0x02 0x05] |
// ======================================== |
// process_orders done |
// ===================> count = 8 |
// ~mod_rdp(): Recv bmp cache count  = 0 |
// ~mod_rdp(): Recv order count      = 432 |
// ~mod_rdp(): Recv bmp update count = 0 |
// Socket RDP Wab Target (4) : closing connection |
// RDP Wab Target (0): total_received=10034, total_sent=1807 |
} /* end outdata */;

const char indata[] =
{
// connecting to 10.10.47.86:3389 |
// connection to 10.10.47.86:3389 succeeded : socket 4 |
// --------- CREATION OF MOD ------------------------ |
// Creation of new mod 'RDP' |
// ModRDPParams target_user="x" |
// ModRDPParams target_password="<hidden>" |
// ModRDPParams target_host="10.10.47.86" |
// ModRDPParams client_address="192.168.1.100" |
// ModRDPParams enable_tls=no |
// ModRDPParams enable_nla=no |
// ModRDPParams enable_krb=no |
// ModRDPParams enable_fastpath=no |
// ModRDPParams enable_mem3blt=no |
// ModRDPParams enable_bitmap_update=no |
// ModRDPParams enable_new_pointer=no |
// ModRDPParams enable_glyph_cache=no |
// ModRDPParams enable_transparent_mode=no |
// ModRDPParams output_filename="" |
// ModRDPParams persistent_key_list_transport=<(nil)> |
// ModRDPParams transparent_recorder_transport=<(nil)> |
// ModRDPParams key_flags=7 |
// ModRDPParams acl=<(nil)> |
// ModRDPParams auth_channel="" |
// ModRDPParams alternate_shell="" |
// ModRDPParams shell_working_directory="" |
// ModRDPParams target_application_account="" |
// ModRDPParams target_application_password="<hidden>" |
// ModRDPParams rdp_compression=0 |
// ModRDPParams error_message=<(nil)> |
// ModRDPParams disconnect_on_logon_user_change=no |
// ModRDPParams open_session_timeout=0 |
// ModRDPParams certificate_change_action=0 |
// ModRDPParams extra_orders= |
// ModRDPParams enable_persistent_disk_bitmap_cache=no |
// ModRDPParams enable_cache_waiting_list=no |
// ModRDPParams persist_bitmap_cache_on_disk=no |
// ModRDPParams password_printing_mode=0 |
// ModRDPParams allow_channels=<(nil)> |
// ModRDPParams deny_channels=<(nil)> |
// ModRDPParams verbose=0x000001FF |
// ModRDPParams cache_verbose=0x00000000 |
// RDP Extra orders="" |
// Remote RDP Server domain="" login="x" host="192-168-1-100" |
// Server key layout is 40c |
// mod_rdp::Early TLS Security Exchange |
// RdpNego::NEGO_STATE_INITIAL |
// RdpNego::send_x224_connection_request_pdu |
// Sending on RDP Wab Target (4) 31 bytes |
// /* 0000 */ "\x03\x00\x00\x1f\x1a\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...........Cooki |
// /* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x78\x0d\x0a"     //e: mstshash=x.. |
// Sent dumped on RDP Wab Target (4) 31 bytes |
// RdpNego::send_x224_connection_request_pdu done |
// mod_rdp::Early TLS Security Exchange |
// RdpNego::NEGO_STATE_RDP |
// RdpNego::recv_connection_confirm |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x0b"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 7 bytes |
// Recv done on RDP Wab Target (4) 7 bytes |
/* 0000 */ "\x06\xd0\x00\x00\x00\x00\x00"                                     //....... |
// Dump done on RDP Wab Target (4) 7 bytes |
// mod_rdp::Early TLS Security Exchange |
// Sending to Server GCC User Data CS_CORE (216 bytes) |
// cs_core::version [80004] RDP 5.0, 5.1, 5.2, and 6.0 clients) |
// cs_core::desktopWidth  = 1024 |
// cs_core::desktopHeight = 768 |
// cs_core::colorDepth    = [ca01] [RNS_UD_COLOR_8BPP] superseded by postBeta2ColorDepth |
// cs_core::SASSequence   = [aa03] [Unknown] |
// cs_core::keyboardLayout= 040c |
// cs_core::clientBuild   = 2600 |
// cs_core::clientName    = 192-168-1-100 |
// cs_core::keyboardType  = [0004] IBM enhanced (101-key or 102-key) keyboard |
// cs_core::keyboardSubType      = [0000] OEM code |
// cs_core::keyboardFunctionKey  = 12 function keys |
// cs_core::imeFileName    =  |
// cs_core::postBeta2ColorDepth  = [ca01] [8 bpp] |
// cs_core::clientProductId = 1 |
// cs_core::serialNumber = 0 |
// cs_core::highColorDepth  = [0010] [16-bit 565 RGB mask] |
// cs_core::supportedColorDepths  = [0007] [24/16/15/] |
// cs_core::earlyCapabilityFlags  = [0001] |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_ERRINFO_PDU |
// cs_core::clientDigProductId=[00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 |
// cs_core::connectionType  = 0 |
// cs_core::pad1octet  = 0 |
// cs_core::serverSelectedProtocol = 0 |
// Sending to server GCC User Data CS_CLUSTER (12 bytes) |
// cs_cluster::flags [0009] |
// cs_cluster::redirectedSessionID = 0 |
// Sending to server GCC User Data CS_SECURITY (12 bytes) |
// CSSecGccUserData::encryptionMethods 3 |
// CSSecGccUserData::extEncryptionMethods 0 |
// Sending on RDP Wab Target (4) 376 bytes |
// /* 0000 */ "\x03\x00\x01\x78\x02\xf0\x80\x7f\x65\x82\x01\x6c\x04\x01\x01\x04" //...x....e..l.... |
// /* 0010 */ "\x01\x01\x01\x01\xff\x30\x1a\x02\x01\x22\x02\x01\x02\x02\x01\x00" //.....0..."...... |
// /* 0020 */ "\x02\x01\x01\x02\x01\x00\x02\x01\x01\x02\x03\x00\xff\xff\x02\x01" //................ |
// /* 0030 */ "\x02\x30\x19\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02" //.0.............. |
// /* 0040 */ "\x01\x00\x02\x01\x01\x02\x02\x04\x20\x02\x01\x02\x30\x1f\x02\x03" //........ ...0... |
// /* 0050 */ "\x00\xff\xff\x02\x02\xfc\x17\x02\x03\x00\xff\xff\x02\x01\x01\x02" //................ |
// /* 0060 */ "\x01\x00\x02\x01\x01\x02\x03\x00\xff\xff\x02\x01\x02\x04\x82\x01" //................ |
// /* 0070 */ "\x07\x00\x05\x00\x14\x7c\x00\x01\x80\xfe\x00\x08\x00\x10\x00\x01" //.....|.......... |
// /* 0080 */ "\xc0\x00\x44\x75\x63\x61\x80\xf0\x01\xc0\xd8\x00\x04\x00\x08\x00" //..Duca.......... |
// /* 0090 */ "\x00\x04\x00\x03\x01\xca\x03\xaa\x0c\x04\x00\x00\x28\x0a\x00\x00" //............(... |
// /* 00a0 */ "\x31\x00\x39\x00\x32\x00\x2d\x00\x31\x00\x36\x00\x38\x00\x2d\x00" //1.9.2.-.1.6.8.-. |
// /* 00b0 */ "\x31\x00\x2d\x00\x31\x00\x30\x00\x30\x00\x00\x00\x00\x00\x00\x00" //1.-.1.0.0....... |
// /* 00c0 */ "\x04\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\xca\x01\x00" //................ |
// /* 0110 */ "\x00\x00\x00\x00\x10\x00\x07\x00\x01\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0160 */ "\x04\xc0\x0c\x00\x09\x00\x00\x00\x00\x00\x00\x00\x02\xc0\x0c\x00" //................ |
// /* 0170 */ "\x03\x00\x00\x00\x00\x00\x00\x00"                                 //........ |
// Sent dumped on RDP Wab Target (4) 376 bytes |
// mod_rdp::Basic Settings Exchange |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x01\x45"                                                     //..E |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 321 bytes |
// Recv done on RDP Wab Target (4) 321 bytes |
/* 0000 */ "\x02\xf0\x80\x7f\x66\x82\x01\x39\x0a\x01\x00\x02\x01\x00\x30\x1a" //....f..9......0. |
/* 0010 */ "\x02\x01\x22\x02\x01\x03\x02\x01\x00\x02\x01\x01\x02\x01\x00\x02" //.."............. |
/* 0020 */ "\x01\x01\x02\x03\x00\xff\xf8\x02\x01\x02\x04\x82\x01\x13\x00\x05" //................ |
/* 0030 */ "\x00\x14\x7c\x00\x01\x2a\x14\x76\x0a\x01\x01\x00\x01\xc0\x00\x4d" //..|..*.v.......M |
/* 0040 */ "\x63\x44\x6e\x80\xfc\x01\x0c\x08\x00\x04\x00\x08\x00\x03\x0c\x08" //cDn............. |
/* 0050 */ "\x00\xeb\x03\x00\x00\x02\x0c\xec\x00\x01\x00\x00\x00\x01\x00\x00" //................ |
/* 0060 */ "\x00\x20\x00\x00\x00\xb8\x00\x00\x00\x53\x50\x66\x7f\xc1\x9d\xd7" //. .......SPf.... |
/* 0070 */ "\x5e\xab\x02\xd1\x9c\x81\x1c\xc2\x25\x54\x03\xc8\xca\x18\x33\x86" //^.......%T....3. |
/* 0080 */ "\x27\x09\x22\xdf\x33\x0b\x29\xab\x45\x01\x00\x00\x00\x01\x00\x00" //'.".3.).E....... |
/* 0090 */ "\x00\x01\x00\x00\x00\x06\x00\x5c\x00\x52\x53\x41\x31\x48\x00\x00" //.........RSA1H.. |
/* 00a0 */ "\x00\x00\x02\x00\x00\x3f\x00\x00\x00\x01\x00\x01\x00\x67\xab\x0e" //.....?.......g.. |
/* 00b0 */ "\x6a\x9f\xd6\x2b\xa3\x32\x2f\x41\xd1\xce\xee\x61\xc3\x76\x0b\x26" //j..+.2/A...a.v.& |
/* 00c0 */ "\x11\x70\x48\x8a\x8d\x23\x81\x95\xa0\x39\xf7\x5b\xaa\x3e\xf1\xed" //.pH..#...9.[.>.. |
/* 00d0 */ "\xb8\xc4\xee\xce\x5f\x6a\xf5\x43\xce\x5f\x60\xca\x6c\x06\x75\xae" //...._j.C._`.l.u. |
/* 00e0 */ "\xc0\xd6\xa4\x0c\x92\xa4\xc6\x75\xea\x64\xb2\x50\x5b\x00\x00\x00" //.......u.d.P[... |
/* 00f0 */ "\x00\x00\x00\x00\x00\x08\x00\x48\x00\x6a\x41\xb1\x43\xcf\x47\x6f" //.......H.jA.C.Go |
/* 0100 */ "\xf1\xe6\xcc\xa1\x72\x97\xd9\xe1\x85\x15\xb3\xc2\x39\xa0\xa6\x26" //....r.......9..& |
/* 0110 */ "\x1a\xb6\x49\x01\xfa\xa6\xda\x60\xd7\x45\xf7\x2c\xee\xe4\x8e\x64" //..I....`.E.,...d |
/* 0120 */ "\x2e\x37\x49\xf0\x4c\x94\x6f\x08\xf5\x63\x4c\x56\x29\x55\x5a\x63" //.7I.L.o..cLV)UZc |
/* 0130 */ "\x41\x2c\x20\x65\x95\x99\xb1\x15\x7c\x00\x00\x00\x00\x00\x00\x00" //A, e....|....... |
/* 0140 */ "\x00"                                                             //. |
// Dump done on RDP Wab Target (4) 321 bytes |
// GCC::UserData tag=0c01 length=8 |
// Received from server GCC User Data SC_CORE (8 bytes) |
// sc_core::version [80004] RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 servers) |
// GCC::UserData tag=0c03 length=8 |
// server_channels_count=0 sent_channels_count=0 |
// Received from server GCC User Data SC_NET (8 bytes) |
// sc_net::MCSChannelId   = 1003 |
// sc_net::channelCount   = 0 |
// GCC::UserData tag=0c02 length=236 |
// RSA Key blob len in certificate is 92 |
// Received from server GCC User Data SC_SECURITY (236 bytes) |
// sc_security::encryptionMethod = 1 |
// sc_security::encryptionLevel  = 1 |
// sc_security::serverRandomLen  = 32 |
// sc_security::serverCertLen    = 184 |
// sc_security::dwVersion = 1 |
// sc_security::temporary = false |
// sc_security::RDP4-style encryption |
// sc_security::proprietaryCertificate::dwSigAlgId = 1 |
// sc_security::proprietaryCertificate::dwKeyAlgId = 1 |
// sc_security::proprietaryCertificate::wPublicKeyBlobType = 6 |
// sc_security::proprietaryCertificate::wPublicKeyBlobLen = 92 |
// sc_security::proprietaryCertificate::RSAPK::magic = 826364754 |
// sc_security::proprietaryCertificate::RSAPK::keylen = 72 |
// sc_security::proprietaryCertificate::RSAPK::bitlen = 512 |
// sc_security::proprietaryCertificate::RSAPK::datalen = 63 |
// mod_rdp::Channel Connection |
// Send MCS::ErectDomainRequest |
// Sending on RDP Wab Target (4) 12 bytes |
// /* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x04\x01\x00\x01\x00"                 //............ |
// Sent dumped on RDP Wab Target (4) 12 bytes |
// Send MCS::AttachUserRequest |
// Sending on RDP Wab Target (4) 8 bytes |
// /* 0000 */ "\x03\x00\x00\x08\x02\xf0\x80\x28"                                 //.......( |
// Sent dumped on RDP Wab Target (4) 8 bytes |
// mod_rdp::Channel Connection Attach User |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x0b"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 7 bytes |
// Recv done on RDP Wab Target (4) 7 bytes |
/* 0000 */ "\x02\xf0\x80\x2e\x00\x00\x00"                                     //....... |
// Dump done on RDP Wab Target (4) 7 bytes |
// cjrq[0] = 1001 |
// Sending on RDP Wab Target (4) 12 bytes |
// /* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xe9"                 //.......8.... |
// Sent dumped on RDP Wab Target (4) 12 bytes |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x0f"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 11 bytes |
// Recv done on RDP Wab Target (4) 11 bytes |
/* 0000 */ "\x02\xf0\x80\x3e\x00\x00\x00\x03\xe9\x03\xe9"                     //...>....... |
// Dump done on RDP Wab Target (4) 11 bytes |
// cjcf[0] = 1001 |
// cjrq[1] = 1003 |
// Sending on RDP Wab Target (4) 12 bytes |
// /* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xeb"                 //.......8.... |
// Sent dumped on RDP Wab Target (4) 12 bytes |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x0f"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 11 bytes |
// Recv done on RDP Wab Target (4) 11 bytes |
/* 0000 */ "\x02\xf0\x80\x3e\x00\x00\x00\x03\xeb\x03\xeb"                     //...>....... |
// Dump done on RDP Wab Target (4) 11 bytes |
// cjcf[1] = 1003 |
// mod_rdp::RDP Security Commencement |
// mod_rdp::SecExchangePacket keylen=64 |
// send data request |
// Sending on RDP Wab Target (4) 94 bytes |
// /* 0000 */ "\x03\x00\x00\x5e\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x50\x01\x00" //...^...d....pP.. |
// /* 0010 */ "\x00\x00\x48\x00\x00\x00\xa4\xbe\x34\x2c\x91\x29\xd3\x65\x51\xf7" //..H.....4,.).eQ. |
// /* 0020 */ "\xeb\xeb\xdf\x64\x31\x50\x0e\x1c\xb2\xfb\xbf\x02\x59\x28\x14\x0c" //...d1P......Y(.. |
// /* 0030 */ "\x7b\x5e\x34\x95\x25\x19\xd6\xac\x5a\x5b\xf5\x90\xe5\x64\x10\x5e" //{^4.%...Z[...d.^ |
// /* 0040 */ "\x77\x0c\xea\x1b\x4b\x49\x07\xd3\x02\xe6\xed\xee\xc6\x30\x2e\x2d" //w...KI.......0.- |
// /* 0050 */ "\x8a\x53\x55\x1d\x28\x2a\x00\x00\x00\x00\x00\x00\x00\x00"         //.SU.(*........ |
// Sent dumped on RDP Wab Target (4) 94 bytes |
// send data request done |
// mod_rdp::Secure Settings Exchange |
// mod_rdp::send_client_info_pdu |
// Sending to server:  InfoPacket |
// InfoPacket::CodePage 0 |
// InfoPacket::flags 0x173 |
// InfoPacket::flags:INFO_MOUSE yes |
// InfoPacket::flags:INFO_DISABLECTRLALTDEL yes |
// InfoPacket::flags:INFO_AUTOLOGON yes |
// InfoPacket::flags:INFO_UNICODE yes |
// InfoPacket::flags:INFO_MAXIMIZESHELL  yes |
// InfoPacket::flags:INFO_LOGONNOTIFY yes |
// InfoPacket::flags:INFO_COMPRESSION no |
// InfoPacket::flags:CompressionTypeMask no |
// InfoPacket::flags:INFO_ENABLEWINDOWSKEY  yes |
// InfoPacket::flags:INFO_REMOTECONSOLEAUDIO no |
// InfoPacket::flags:FORCE_ENCRYPTED_CS_PDU no |
// InfoPacket::flags:INFO_RAIL no |
// InfoPacket::flags:INFO_LOGONERRORS no |
// InfoPacket::flags:INFO_MOUSE_HAS_WHEEL no |
// InfoPacket::flags:INFO_PASSWORD_IS_SC_PIN no |
// InfoPacket::flags:INFO_NOAUDIOPLAYBACK no |
// InfoPacket::flags:INFO_USING_SAVED_CREDS no |
// InfoPacket::flags:RNS_INFO_AUDIOCAPTURE no |
// InfoPacket::flags:RNS_INFO_VIDEO_DISABLE no |
// InfoPacket::cbDomain 0 |
// InfoPacket::cbUserName 2 |
// InfoPacket::cbPassword 2 |
// InfoPacket::cbAlternateShell 0 |
// InfoPacket::cbWorkingDir 0 |
// InfoPacket::Domain  |
// InfoPacket::UserName x |
// InfoPacket::Password <hidden> |
// InfoPacket::AlternateShell  |
// InfoPacket::WorkingDir  |
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2 |
// InfoPacket::ExtendedInfoPacket::cbClientAddress 28 |
// InfoPacket::ExtendedInfoPacket::clientAddress 192.168.1.100 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 64 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\Windows\System32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 0 |
// InfoPacket::ExtendedInfoPacket::performanceFlags 7 |
// InfoPacket::ExtendedInfoPacket::cbAutoReconnectLen 0 |
// InfoPacket::ExtendedInfoPacket::autoReconnectCookie  |
// InfoPacket::ExtendedInfoPacket::reserved1 0 |
// InfoPacket::ExtendedInfoPacket::reserved2 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::Bias 120 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardName GMT Standard Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMonth 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDay 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wHour 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardBias 60 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightName GMT Daylight Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMonth 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDay 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wHour 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMilliseconds 0 |
// send extended login info (RDP5-style) 9017b :x |
// Preparing sec header  InfoPacket |
// InfoPacket::CodePage 0 |
// InfoPacket::flags 0x9017b |
// InfoPacket::flags:INFO_MOUSE yes |
// InfoPacket::flags:INFO_DISABLECTRLALTDEL yes |
// InfoPacket::flags:INFO_AUTOLOGON yes |
// InfoPacket::flags:INFO_UNICODE yes |
// InfoPacket::flags:INFO_MAXIMIZESHELL  yes |
// InfoPacket::flags:INFO_LOGONNOTIFY yes |
// InfoPacket::flags:INFO_COMPRESSION no |
// InfoPacket::flags:CompressionTypeMask no |
// InfoPacket::flags:INFO_ENABLEWINDOWSKEY  yes |
// InfoPacket::flags:INFO_REMOTECONSOLEAUDIO no |
// InfoPacket::flags:FORCE_ENCRYPTED_CS_PDU no |
// InfoPacket::flags:INFO_RAIL no |
// InfoPacket::flags:INFO_LOGONERRORS yes |
// InfoPacket::flags:INFO_MOUSE_HAS_WHEEL no |
// InfoPacket::flags:INFO_PASSWORD_IS_SC_PIN no |
// InfoPacket::flags:INFO_NOAUDIOPLAYBACK yes |
// InfoPacket::flags:INFO_USING_SAVED_CREDS no |
// InfoPacket::flags:RNS_INFO_AUDIOCAPTURE no |
// InfoPacket::flags:RNS_INFO_VIDEO_DISABLE no |
// InfoPacket::cbDomain 0 |
// InfoPacket::cbUserName 2 |
// InfoPacket::cbPassword 2 |
// InfoPacket::cbAlternateShell 0 |
// InfoPacket::cbWorkingDir 0 |
// InfoPacket::Domain  |
// InfoPacket::UserName x |
// InfoPacket::Password <hidden> |
// InfoPacket::AlternateShell  |
// InfoPacket::WorkingDir  |
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2 |
// InfoPacket::ExtendedInfoPacket::cbClientAddress 28 |
// InfoPacket::ExtendedInfoPacket::clientAddress 192.168.1.100 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 64 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\Windows\System32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 0 |
// InfoPacket::ExtendedInfoPacket::performanceFlags 7 |
// InfoPacket::ExtendedInfoPacket::cbAutoReconnectLen 0 |
// InfoPacket::ExtendedInfoPacket::autoReconnectCookie  |
// InfoPacket::ExtendedInfoPacket::reserved1 0 |
// InfoPacket::ExtendedInfoPacket::reserved2 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::Bias 120 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardName GMT Standard Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMonth 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDay 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wHour 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardBias 60 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightName GMT Daylight Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMonth 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDay 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wHour 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMilliseconds 0 |
// Send data request InfoPacket |
// InfoPacket::CodePage 0 |
// InfoPacket::flags 0x9017b |
// InfoPacket::flags:INFO_MOUSE yes |
// InfoPacket::flags:INFO_DISABLECTRLALTDEL yes |
// InfoPacket::flags:INFO_AUTOLOGON yes |
// InfoPacket::flags:INFO_UNICODE yes |
// InfoPacket::flags:INFO_MAXIMIZESHELL  yes |
// InfoPacket::flags:INFO_LOGONNOTIFY yes |
// InfoPacket::flags:INFO_COMPRESSION no |
// InfoPacket::flags:CompressionTypeMask no |
// InfoPacket::flags:INFO_ENABLEWINDOWSKEY  yes |
// InfoPacket::flags:INFO_REMOTECONSOLEAUDIO no |
// InfoPacket::flags:FORCE_ENCRYPTED_CS_PDU no |
// InfoPacket::flags:INFO_RAIL no |
// InfoPacket::flags:INFO_LOGONERRORS yes |
// InfoPacket::flags:INFO_MOUSE_HAS_WHEEL no |
// InfoPacket::flags:INFO_PASSWORD_IS_SC_PIN no |
// InfoPacket::flags:INFO_NOAUDIOPLAYBACK yes |
// InfoPacket::flags:INFO_USING_SAVED_CREDS no |
// InfoPacket::flags:RNS_INFO_AUDIOCAPTURE no |
// InfoPacket::flags:RNS_INFO_VIDEO_DISABLE no |
// InfoPacket::cbDomain 0 |
// InfoPacket::cbUserName 2 |
// InfoPacket::cbPassword 2 |
// InfoPacket::cbAlternateShell 0 |
// InfoPacket::cbWorkingDir 0 |
// InfoPacket::Domain  |
// InfoPacket::UserName x |
// InfoPacket::Password <hidden> |
// InfoPacket::AlternateShell  |
// InfoPacket::WorkingDir  |
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2 |
// InfoPacket::ExtendedInfoPacket::cbClientAddress 28 |
// InfoPacket::ExtendedInfoPacket::clientAddress 192.168.1.100 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 64 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\Windows\System32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 0 |
// InfoPacket::ExtendedInfoPacket::performanceFlags 7 |
// InfoPacket::ExtendedInfoPacket::cbAutoReconnectLen 0 |
// InfoPacket::ExtendedInfoPacket::autoReconnectCookie  |
// InfoPacket::ExtendedInfoPacket::reserved1 0 |
// InfoPacket::ExtendedInfoPacket::reserved2 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::Bias 120 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardName GMT Standard Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMonth 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDay 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wHour 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardBias 60 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightName GMT Daylight Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMonth 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDay 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wHour 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMilliseconds 0 |
// send data request |
// Sending on RDP Wab Target (4) 339 bytes |
// /* 0000 */ "\x03\x00\x01\x53\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\x44\x48" //...S...d....p.DH |
// /* 0010 */ "\x00\x00\x00\x12\xdd\x4c\x90\x84\x1e\x0d\x32\xce\x3b\xec\xc9\xc5" //.....L....2.;... |
// /* 0020 */ "\xf3\xbe\xeb\x13\x44\x3a\x25\x0e\x9f\xbd\xec\x15\x32\x35\x13\xc7" //....D:%.....25.. |
// /* 0030 */ "\xad\x69\x63\x60\xd6\x70\xb4\x36\x34\xdf\x0a\xb7\xec\xc7\x6b\x49" //.ic`.p.64.....kI |
// /* 0040 */ "\xc4\xcd\xb4\x42\x2c\xed\x40\x9a\xdc\x8f\xa6\x62\x9d\x38\x8e\xbb" //...B,.@....b.8.. |
// /* 0050 */ "\x40\x32\x76\x95\x1d\x87\xe7\x30\x22\x90\xd7\xe5\x1d\xb6\xc9\xa9" //@2v....0"....... |
// /* 0060 */ "\x9d\x0b\xb0\xf0\x34\x62\x93\xf8\x70\x1e\x26\x62\x3d\x91\x49\x5b" //....4b..p.&b=.I[ |
// /* 0070 */ "\x3b\xe8\x52\x82\x94\x76\x8d\x34\x99\xc5\x49\x5f\xec\x7a\x43\x44" //;.R..v.4..I_.zCD |
// /* 0080 */ "\x6e\x54\xda\xe1\xbf\x56\xcb\x52\xe0\x37\x3a\xc0\x41\x2d\xcd\x79" //nT...V.R.7:.A-.y |
// /* 0090 */ "\xeb\x49\xfc\x46\xec\xb5\xbb\x04\x56\xd1\x02\xae\x19\xda\xed\xd6" //.I.F....V....... |
// /* 00a0 */ "\x60\xc2\xe3\x3a\x0e\x6f\xf9\x26\x59\x78\x1a\xc8\x32\x6c\x2d\x40" //`..:.o.&Yx..2l-@ |
// /* 00b0 */ "\xe3\xfd\xba\x08\xbb\x41\x62\x23\xdc\x76\xe6\xbb\x7e\x46\x4f\x52" //.....Ab#.v..~FOR |
// /* 00c0 */ "\x98\x53\x03\xe8\x00\xf6\x2c\x4b\x05\x29\xd3\x39\xad\x35\x6f\x7a" //.S....,K.).9.5oz |
// /* 00d0 */ "\x21\x4c\xf5\x90\x6e\x56\x08\x9e\x16\x5c\x0a\xf5\xbd\x9c\x0a\xa4" //!L..nV.......... |
// /* 00e0 */ "\x8a\x8c\x4e\xbb\xcf\xe2\xf8\x37\x79\x96\xb3\x99\x04\x67\x44\xcf" //..N....7y....gD. |
// /* 00f0 */ "\xc5\xb5\x75\xb2\xcb\xcd\x18\x8b\xd8\x58\x2f\xc7\xcf\xb5\x4d\x04" //..u......X/...M. |
// /* 0100 */ "\x5c\x63\x64\xfd\xb5\xf6\x1c\x00\x1d\xf6\x96\x2e\x3a\xef\x13\xf3" //.cd.........:... |
// /* 0110 */ "\xe9\xfc\x5e\xbb\x2b\xfb\xce\x48\xa2\xcb\xc4\xd4\x73\xd5\xa7\xfc" //..^.+..H....s... |
// /* 0120 */ "\x2e\x82\x04\xb5\x45\x08\x06\x1a\x42\x26\x80\xcb\xcb\x87\xfb\x6c" //....E...B&.....l |
// /* 0130 */ "\x82\xca\x17\x30\xbc\x68\x9f\xf7\xad\x1e\x19\xd5\x18\xb3\xb5\xf7" //...0.h.......... |
// /* 0140 */ "\x44\xd5\x0a\x63\xfd\x7f\x20\x7a\x2d\x26\x60\xeb\xe3\xe1\xd4\xce" //D..c.. z-&`..... |
// /* 0150 */ "\xdb\x27\xb4"                                                     //.'. |
// Sent dumped on RDP Wab Target (4) 339 bytes |
// send data request done |
// mod_rdp::send_client_info_pdu done |
// mod_rdp::Licensing |
// Rdp::Get license: username="x" |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x01\x51"                                                     //..Q |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 333 bytes |
// Recv done on RDP Wab Target (4) 333 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x81\x42\x80\x00\x00\x00\x01" //...h....p.B..... |
/* 0010 */ "\x02\x3e\x01\x7b\x3c\x31\xa6\xae\xe8\x74\xf6\xb4\xa5\x03\x90\xe7" //.>.{<1...t...... |
/* 0020 */ "\xc2\xc7\x39\xba\x53\x1c\x30\x54\x6e\x90\x05\xd0\x05\xce\x44\x18" //..9.S.0Tn.....D. |
/* 0030 */ "\x91\x83\x81\x00\x00\x04\x00\x2c\x00\x00\x00\x4d\x00\x69\x00\x63" //.......,...M.i.c |
/* 0040 */ "\x00\x72\x00\x6f\x00\x73\x00\x6f\x00\x66\x00\x74\x00\x20\x00\x43" //.r.o.s.o.f.t. .C |
/* 0050 */ "\x00\x6f\x00\x72\x00\x70\x00\x6f\x00\x72\x00\x61\x00\x74\x00\x69" //.o.r.p.o.r.a.t.i |
/* 0060 */ "\x00\x6f\x00\x6e\x00\x00\x00\x08\x00\x00\x00\x32\x00\x33\x00\x36" //.o.n.......2.3.6 |
/* 0070 */ "\x00\x00\x00\x0d\x00\x04\x00\x01\x00\x00\x00\x03\x00\xb8\x00\x01" //................ |
/* 0080 */ "\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x06\x00\x5c\x00\x52" //...............R |
/* 0090 */ "\x53\x41\x31\x48\x00\x00\x00\x00\x02\x00\x00\x3f\x00\x00\x00\x01" //SA1H.......?.... |
/* 00a0 */ "\x00\x01\x00\x01\xc7\xc9\xf7\x8e\x5a\x38\xe4\x29\xc3\x00\x95\x2d" //........Z8.)...- |
/* 00b0 */ "\xdd\x4c\x3e\x50\x45\x0b\x0d\x9e\x2a\x5d\x18\x63\x64\xc4\x2c\xf7" //.L>PE...*].cd.,. |
/* 00c0 */ "\x8f\x29\xd5\x3f\xc5\x35\x22\x34\xff\xad\x3a\xe6\xe3\x95\x06\xae" //.).?.5"4..:..... |
/* 00d0 */ "\x55\x82\xe3\xc8\xc7\xb4\xa8\x47\xc8\x50\x71\x74\x29\x53\x89\x6d" //U......G.Pqt)S.m |
/* 00e0 */ "\x9c\xed\x70\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x48\x00\xa8" //..p..........H.. |
/* 00f0 */ "\xf4\x31\xb9\xab\x4b\xe6\xb4\xf4\x39\x89\xd6\xb1\xda\xf6\x1e\xec" //.1..K...9....... |
/* 0100 */ "\xb1\xf0\x54\x3b\x5e\x3e\x6a\x71\xb4\xf7\x75\xc8\x16\x2f\x24\x00" //..T;^>jq..u../$. |
/* 0110 */ "\xde\xe9\x82\x99\x5f\x33\x0b\xa9\xa6\x94\xaf\xcb\x11\xc3\xf2\xdb" //...._3.......... |
/* 0120 */ "\x09\x42\x68\x29\x56\x58\x01\x56\xdb\x59\x03\x69\xdb\x7d\x37\x00" //.Bh)VX.V.Y.i.}7. |
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x0e\x00\x0e\x00\x6d" //...............m |
/* 0140 */ "\x69\x63\x72\x6f\x73\x6f\x66\x74\x2e\x63\x6f\x6d\x00"             //icrosoft.com. |
// Dump done on RDP Wab Target (4) 333 bytes |
// Rdp::License Request |
// send data request |
// Sending on RDP Wab Target (4) 163 bytes |
// /* 0000 */ "\x03\x00\x00\xa3\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x94\x80" //.......d....p... |
// /* 0010 */ "\x00\x00\x00\x13\x03\x90\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02" //................ |
// /* 0040 */ "\x00\x48\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //.H.............. |
// /* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0f\x00\x02\x00\x78" //...............x |
// /* 0090 */ "\x00\x10\x00\x0e\x00\x31\x39\x32\x2d\x31\x36\x38\x2d\x31\x2d\x31" //.....192-168-1-1 |
// /* 00a0 */ "\x30\x30\x00"                                                     //00. |
// Sent dumped on RDP Wab Target (4) 163 bytes |
// send data request done |
// mod_rdp::Licensing |
// Rdp::Get license: username="x" |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x22"                                                     //.." |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 30 bytes |
// Recv done on RDP Wab Target (4) 30 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x14\x80\x00\x10\x00\xff\x02" //...h....p....... |
/* 0010 */ "\x10\x00\x07\x00\x00\x00\x02\x00\x00\x00\x28\x14\x00\x00"         //..........(... |
// Dump done on RDP Wab Target (4) 30 bytes |
// Rdp::Get license status |
// /* 0000 */ 0xff, 0x02, 0x10, 0x00, 0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x28, 0x14, 0x00, 0x00,  // ............(... |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x01\x43"                                                     //..C |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 319 bytes |
// Recv done on RDP Wab Target (4) 319 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x81\x34\x08\x00\x00\x00\x4b" //...h....p.4....K |
/* 0010 */ "\x8a\x10\x64\x02\x2b\x08\xb5\x24\x8a\x82\x0d\x68\x83\x0a\xf1\xab" //..d.+..$...h.... |
/* 0020 */ "\x13\x59\x72\xbd\xf9\x5f\xbe\xaf\x72\xcc\x18\xe7\x1e\x65\xc9\xb9" //.Yr.._..r....e.. |
/* 0030 */ "\xec\xe7\x2b\x99\xe4\x9c\x59\x3a\x24\xca\x31\x34\x6c\xb9\x99\xc9" //..+...Y:$.14l... |
/* 0040 */ "\xa7\x15\x52\x16\x81\x64\xaa\x61\x7f\xfe\xad\xee\xe2\x8b\xb8\x42" //..R..d.a.......B |
/* 0050 */ "\x23\xbe\x98\x19\xf3\x39\x1a\x16\x52\x5e\x55\x4b\xab\xc9\x6d\x8e" //#....9..R^UK..m. |
/* 0060 */ "\x53\xb8\x28\xe8\xb5\x12\x9c\x03\x0d\x34\x23\x0a\xac\xaf\xa9\x2a" //S.(......4#....* |
/* 0070 */ "\xf4\x0c\xa4\x2c\x4e\x57\x64\x79\xb9\xdb\xd8\x5b\x6d\x29\x42\xd4" //...,NWdy...[m)B. |
/* 0080 */ "\xc7\x03\x43\x4d\x2b\x29\xc7\x89\x78\xcc\xa4\x07\xd9\x11\x9c\x9e" //..CM+)..x....... |
/* 0090 */ "\x28\x78\x42\xd4\x16\x05\x4a\x06\xe1\x64\x88\xc0\x4c\x41\xc7\xd3" //(xB...J..d..LA.. |
/* 00a0 */ "\x7a\xc7\x9e\xc0\x3d\xe0\x56\xbd\xa5\xdb\x49\xca\x68\xcd\x5c\x0f" //z...=.V...I.h... |
/* 00b0 */ "\x61\x35\xad\x83\x48\xc7\xf9\x6d\x71\x1b\x09\x21\x45\xc7\x4f\x3f" //a5..H..mq..!E.O? |
/* 00c0 */ "\xe8\x2f\x3a\x31\xb8\x41\x0d\x30\xcb\xd9\x15\xa2\xb1\xa4\x6b\x7f" //./:1.A.0......k. |
/* 00d0 */ "\x7e\xc5\xf3\x1a\xdc\x72\x36\x22\x4d\xe1\x80\x78\xaa\x2b\x42\xa9" //~....r6"M..x.+B. |
/* 00e0 */ "\x9c\xc4\xc6\x84\x7b\x25\x18\x9d\x34\x9c\x1f\xcb\x80\x09\xf1\xcd" //....{%..4....... |
/* 00f0 */ "\x96\x96\x7c\x15\xe6\xc2\x5f\x48\xca\x75\x5f\x4e\x1c\x1b\x49\xc4" //..|..._H.u_N..I. |
/* 0100 */ "\xa1\x46\xeb\x4f\x3a\xae\x47\x7c\x61\xfa\x17\x56\xa9\x69\x4c\x9d" //.F.O:.G|a..V.iL. |
/* 0110 */ "\xdf\xd5\xa3\x4a\x5c\x7c\x63\xf4\x08\x3c\x89\xe6\x82\x8e\x96\x3d" //...J.|c..<.....= |
/* 0120 */ "\xc1\x81\xfe\x22\xfa\x04\xa4\x3e\x32\xc1\x9a\x17\x45\xbf\x0d\x9c" //..."...>2...E... |
/* 0130 */ "\xae\x30\xce\x5f\xe6\x7d\xf1\xd9\x9c\x8a\x3a\xa1\x06\x48\xd2"     //.0._.}....:..H. |
// Dump done on RDP Wab Target (4) 319 bytes |
// LOOPING on PDUs: 296 |
// PDUTYPE_DEMANDACTIVEPDU |
// mod_rdp::process_server_caps |
// Received from server General caps (24 bytes) |
// General caps::major 1 |
// General caps::minor 3 |
// General caps::protocol 512 |
// General caps::pad2octetA 0 |
// General caps::compression type 0 |
// General caps::extra flags 1 |
// General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED yes |
// General caps::extraflags:LONG_CREDENTIALS_SUPPORTED no |
// General caps::extraflags:AUTORECONNECT_SUPPORTED no |
// General caps::extraflags:ENC_SALTED_CHECKSUM no |
// General caps::extraflags:NO_BITMAP_COMPRESSION_HDR no |
// General caps::updateCapability 0 |
// General caps::remoteUnshare 0 |
// General caps::compressionLevel 0 |
// General caps::refreshRectSupport 0 |
// General caps::suppressOutputSupport 0 |
// Received from server Bitmap caps (28 bytes) |
// Bitmap caps::preferredBitsPerPixel 16 |
// Bitmap caps::receive1BitPerPixel 1 |
// Bitmap caps::receive4BitsPerPixel 1 |
// Bitmap caps::receive8BitsPerPixel 1 |
// Bitmap caps::desktopWidth 1024 |
// Bitmap caps::desktopHeight 768 |
// Bitmap caps::pad2octets 0 |
// Bitmap caps::desktopResizeFlag 1 (yes) |
// Bitmap caps::bitmapCompressionFlag 1 yes |
// Bitmap caps::highColorFlags 0 |
// Bitmap caps::drawingFlags 8 |
// Bitmap caps::drawingFlags:DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY no |
// Bitmap caps::drawingFlags:DRAW_ALLOW_COLOR_SUBSAMPLING no |
// Bitmap caps::drawingFlags:DRAW_ALLOW_SKIP_ALPHA yes |
// Bitmap caps::multipleRectangleSupport 1 |
// Bitmap caps::pad2octetsB 0 |
// Received from server Order caps (88 bytes) |
// Order caps::terminalDescriptor 0 |
// Order caps::pad4octetsA 1000000 |
// Order caps::desktopSaveXGranularity 1 |
// Order caps::desktopSaveYGranularity 20 |
// Order caps::pad2octetsA 0 |
// Order caps::maximumOrderLevel 1 |
// Order caps::numberFonts 47 |
// Order caps::orderFlags 34 |
// Order caps::orderSupport[TS_NEG_DSTBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_PATBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_SCRBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MEMBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MEM3BLT_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex1] 0 |
// Order caps::orderSupport[UnusedIndex2] 0 |
// Order caps::orderSupport[TS_NEG_DRAWNINEGRID_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_LINETO_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex3] 1 |
// Order caps::orderSupport[TS_NEG_SAVEBITMAP_INDEX] 0 |
// Order caps::orderSupport[UnusedIndex4] 0 |
// Order caps::orderSupport[UnusedIndex5] 0 |
// Order caps::orderSupport[UnusedIndex6] 0 |
// Order caps::orderSupport[TS_NEG_MULTIDSTBLT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_MULTIPATBLT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_MULTISCRBLT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_FAST_INDEX_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_POLYGON_SC_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_POLYGON_CB_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_POLYLINE_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex7] 0 |
// Order caps::orderSupport[TS_NEG_FAST_GLYPH_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_ELLIPSE_SC_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_ELLIPSE_CB_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_INDEX_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex8] 0 |
// Order caps::orderSupport[UnusedIndex9] 0 |
// Order caps::orderSupport[UnusedIndex10] 0 |
// Order caps::orderSupport[UnusedIndex11] 0 |
// Order caps::textFlags 1697 |
// Order caps::orderSupportExFlags 0 |
// Order caps::pad4octetsB 1000000 |
// Order caps::desktopSaveSize 1000000 |
// Order caps::pad2octetsC 1 |
// Order caps::pad2octetsD 0 |
// Order caps::textANSICodePage 0 |
// Order caps::pad2octetsE 0 |
// Received from server Input caps (88 bytes) |
// Input caps::inputFlags 41 |
// Input caps::pad2octetsA 0 |
// Input caps::keyboardLayout 0 |
// Input caps::keyboardType 0 |
// Input caps::keyboardSubType 0 |
// Input caps::keyboardFunctionKey 0 |
// Input caps::imeFileName 1207414960 |
// mod_rdp::process_server_caps done |
// mod_rdp::send_confirm_active |
// Sending to server General caps (24 bytes) |
// General caps::major 1 |
// General caps::minor 3 |
// General caps::protocol 512 |
// General caps::pad2octetA 0 |
// General caps::compression type 0 |
// General caps::extra flags 40c |
// General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED no |
// General caps::extraflags:LONG_CREDENTIALS_SUPPORTED yes |
// General caps::extraflags:AUTORECONNECT_SUPPORTED yes |
// General caps::extraflags:ENC_SALTED_CHECKSUM no |
// General caps::extraflags:NO_BITMAP_COMPRESSION_HDR yes |
// General caps::updateCapability 0 |
// General caps::remoteUnshare 0 |
// General caps::compressionLevel 0 |
// General caps::refreshRectSupport 0 |
// General caps::suppressOutputSupport 0 |
// Sending to server Bitmap caps (28 bytes) |
// Bitmap caps::preferredBitsPerPixel 16 |
// Bitmap caps::receive1BitPerPixel 1 |
// Bitmap caps::receive4BitsPerPixel 1 |
// Bitmap caps::receive8BitsPerPixel 1 |
// Bitmap caps::desktopWidth 1024 |
// Bitmap caps::desktopHeight 768 |
// Bitmap caps::pad2octets 0 |
// Bitmap caps::desktopResizeFlag 1 (yes) |
// Bitmap caps::bitmapCompressionFlag 1 yes |
// Bitmap caps::highColorFlags 0 |
// Bitmap caps::drawingFlags 8 |
// Bitmap caps::drawingFlags:DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY no |
// Bitmap caps::drawingFlags:DRAW_ALLOW_COLOR_SUBSAMPLING no |
// Bitmap caps::drawingFlags:DRAW_ALLOW_SKIP_ALPHA yes |
// Bitmap caps::multipleRectangleSupport 1 |
// Bitmap caps::pad2octetsB 0 |
// Sending to server Order caps (88 bytes) |
// Order caps::terminalDescriptor 0 |
// Order caps::pad4octetsA 0 |
// Order caps::desktopSaveXGranularity 1 |
// Order caps::desktopSaveYGranularity 20 |
// Order caps::pad2octetsA 0 |
// Order caps::maximumOrderLevel 1 |
// Order caps::numberFonts 0 |
// Order caps::orderFlags 170 |
// Order caps::orderSupport[TS_NEG_DSTBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_PATBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_SCRBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MEMBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MEM3BLT_INDEX] 0 |
// Order caps::orderSupport[UnusedIndex1] 0 |
// Order caps::orderSupport[UnusedIndex2] 0 |
// Order caps::orderSupport[TS_NEG_DRAWNINEGRID_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_LINETO_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] 0 |
// Order caps::orderSupport[UnusedIndex3] 1 |
// Order caps::orderSupport[TS_NEG_SAVEBITMAP_INDEX] 0 |
// Order caps::orderSupport[UnusedIndex4] 0 |
// Order caps::orderSupport[UnusedIndex5] 1 |
// Order caps::orderSupport[UnusedIndex6] 0 |
// Order caps::orderSupport[TS_NEG_MULTIDSTBLT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_MULTIPATBLT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_MULTISCRBLT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_FAST_INDEX_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_POLYGON_SC_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_POLYGON_CB_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_POLYLINE_INDEX] 0 |
// Order caps::orderSupport[UnusedIndex7] 0 |
// Order caps::orderSupport[TS_NEG_FAST_GLYPH_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_ELLIPSE_SC_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_ELLIPSE_CB_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_INDEX_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex8] 0 |
// Order caps::orderSupport[UnusedIndex9] 0 |
// Order caps::orderSupport[UnusedIndex10] 0 |
// Order caps::orderSupport[UnusedIndex11] 0 |
// Order caps::textFlags 1697 |
// Order caps::orderSupportExFlags 4 |
// Order caps::pad4octetsB 0 |
// Order caps::desktopSaveSize 0 |
// Order caps::pad2octetsC 0 |
// Order caps::pad2octetsD 0 |
// Order caps::textANSICodePage 1252 |
// Order caps::pad2octetsE 0 |
// Sending to server BitmapCache caps (40 bytes) |
// BitmapCache caps::pad1 0 |
// BitmapCache caps::pad2 0 |
// BitmapCache caps::pad3 0 |
// BitmapCache caps::pad4 0 |
// BitmapCache caps::pad5 0 |
// BitmapCache caps::pad6 0 |
// BitmapCache caps::cache0Entries 600 |
// BitmapCache caps::cache0MaximumCellSize 512 |
// BitmapCache caps::cache1Entries 300 |
// BitmapCache caps::cache1MaximumCellSize 2048 |
// BitmapCache caps::cache2Entries 262 |
// BitmapCache caps::cache2MaximumCellSize 8192 |
// Sending to server ColorCache caps (8 bytes) |
// ColorCache caps::colorTableCacheSize 6 |
// ColorCache caps::pad2octets 0 |
// Sending to server Activation caps (12 bytes) |
// Activation caps::helpKeyFlag 0 |
// Activation caps::helpKeyIndexFlag 0 |
// Activation caps::helpExtendedKeyFlag 0 |
// Activation caps::windowManagerKeyFlag 0 |
// Sending to server ControlCaps caps (12 bytes) |
// ControlCaps caps::controlFlags 0 |
// ControlCaps caps::remoteDetachFlag 0 |
// ControlCaps caps::controlInterest 2 |
// ControlCaps caps::detachInterest 2 |
// Sending to server Pointer caps (8 bytes) |
// Pointer caps::colorPointerFlag 1 |
// Pointer caps::colorPointerCacheSize 20 |
// Sending to server Share caps (8 bytes) |
// Share caps::nodeId 0 |
// Share caps::pad2octets 0 |
// Sending to server Input caps (88 bytes) |
// Input caps::inputFlags 1 |
// Input caps::pad2octetsA 0 |
// Input caps::keyboardLayout 1033 |
// Input caps::keyboardType 4 |
// Input caps::keyboardSubType 0 |
// Input caps::keyboardFunctionKey 12 |
// Input caps::imeFileName 1207218208 |
// Sending to server SoundCaps caps (8 bytes) |
// SoundCaps caps::soundFlags 1 |
// SoundCaps caps::pad2octetsA 0 |
// Sending to server Font caps (8 bytes) |
// Font caps::fontSupportFlags 1 |
// Font caps::pad2octets 0 |
// Sending to server GlyphCache caps (52 bytes) |
// GlyphCache caps::GlyphCache[0].CacheEntries=254 |
// GlyphCache caps::GlyphCache[0].CacheMaximumCellSize=4 |
// GlyphCache caps::GlyphCache[1].CacheEntries=254 |
// GlyphCache caps::GlyphCache[1].CacheMaximumCellSize=4 |
// GlyphCache caps::GlyphCache[2].CacheEntries=254 |
// GlyphCache caps::GlyphCache[2].CacheMaximumCellSize=8 |
// GlyphCache caps::GlyphCache[3].CacheEntries=254 |
// GlyphCache caps::GlyphCache[3].CacheMaximumCellSize=8 |
// GlyphCache caps::GlyphCache[4].CacheEntries=254 |
// GlyphCache caps::GlyphCache[4].CacheMaximumCellSize=16 |
// GlyphCache caps::GlyphCache[5].CacheEntries=254 |
// GlyphCache caps::GlyphCache[5].CacheMaximumCellSize=32 |
// GlyphCache caps::GlyphCache[6].CacheEntries=254 |
// GlyphCache caps::GlyphCache[6].CacheMaximumCellSize=64 |
// GlyphCache caps::GlyphCache[7].CacheEntries=254 |
// GlyphCache caps::GlyphCache[7].CacheMaximumCellSize=128 |
// GlyphCache caps::GlyphCache[8].CacheEntries=254 |
// GlyphCache caps::GlyphCache[8].CacheMaximumCellSize=256 |
// GlyphCache caps::GlyphCache[9].CacheEntries=64 |
// GlyphCache caps::GlyphCache[9].CacheMaximumCellSize=2048 |
// GlyphCache caps::FragCache 16777472 |
// GlyphCache caps::GlyphSupportLevel 0 |
// GlyphCache caps::pad2octets 0 |
// Sending on RDP Wab Target (4) 436 bytes |
// /* 0000 */ "\x03\x00\x01\xb4\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\xa5\x08" //.......d....p... |
// /* 0010 */ "\x00\x00\x00\x0f\x3e\x15\xec\x08\x01\x2a\xfd\x95\x1f\x9f\xb1\xc9" //....>....*...... |
// /* 0020 */ "\x1c\x23\xd5\xe8\x5c\x64\xc3\xda\x83\xfd\x70\x7a\xed\x0e\x8d\xe7" //.#...d....pz.... |
// /* 0030 */ "\xa1\x2e\xfa\x63\x68\x09\x7c\x3a\xbf\x76\x7c\x61\xe6\xae\xfa\x6d" //...ch.|:.v|a...m |
// /* 0040 */ "\xef\xeb\x0a\x99\xa8\x39\x6d\xe8\x59\x42\x57\x27\x20\xa1\xcf\xa2" //.....9m.YBW' ... |
// /* 0050 */ "\x7f\xa7\x6c\x16\xfd\x4e\x71\xe6\xa4\xf8\xb2\x83\x8c\x4f\x32\xaa" //..l..Nq......O2. |
// /* 0060 */ "\x53\x47\x26\xf4\x9e\x9d\x62\x04\xe4\x61\x60\x0e\x3d\x9e\x8e\x48" //SG&...b..a`.=..H |
// /* 0070 */ "\x74\x5a\x32\x46\x38\x9c\x94\xb7\x8d\xe2\xee\xf9\x5a\x29\xe7\x3a" //tZ2F8.......Z).: |
// /* 0080 */ "\x22\x72\x7a\x4f\xc4\x90\x07\x1c\x44\xb6\x34\x6b\x7d\x38\x96\x52" //"rzO....D.4k}8.R |
// /* 0090 */ "\x4e\x07\x39\x93\x48\x0b\x18\x1b\x33\x7c\x51\xc6\x7e\xcb\xc4\x94" //N.9.H...3|Q.~... |
// /* 00a0 */ "\x2f\x7a\x06\xcc\xef\x78\xea\x16\x6e\x98\x29\x48\xcd\xcd\x37\x25" ///z...x..n.)H..7% |
// /* 00b0 */ "\x55\x3c\xa5\x5d\xab\xbb\x3e\xb0\x1d\x90\xf4\x4e\x97\x8c\xef\xfd" //U<.]..>....N.... |
// /* 00c0 */ "\xeb\x2a\xef\xfc\x5b\xe7\x0e\x8e\xba\x56\xbe\xf4\x42\x71\x65\xaa" //.*..[....V..Bqe. |
// /* 00d0 */ "\x3b\x9b\x2e\x94\x0e\x9a\x0b\xd0\x09\x98\x26\xeb\x49\x5c\x58\x2d" //;.........&.I.X- |
// /* 00e0 */ "\x9c\x81\x57\x5a\x81\xf5\x7c\xa5\xbc\x26\x1f\x0d\xe4\x82\x83\x1b" //..WZ..|..&...... |
// /* 00f0 */ "\xbd\x09\xb0\x07\x00\xd2\x8f\x4b\x79\xb6\x1f\x65\x6e\xdd\x96\x57" //.......Ky..en..W |
// /* 0100 */ "\x4d\x9f\xa3\x8a\xfe\x09\x39\xd2\x4f\xf1\x48\x4c\x27\xb6\x8a\xdb" //M.....9.O.HL'... |
// /* 0110 */ "\x17\x6c\xb1\x8e\xf9\xc5\x0e\x87\x36\xd0\xf7\xab\x39\x99\xed\x55" //.l......6...9..U |
// /* 0120 */ "\xaa\x71\x0a\x33\xc6\x39\x57\x0b\x1b\xd1\xe3\x9f\xee\xe4\x39\xe6" //.q.3.9W.......9. |
// /* 0130 */ "\xca\x89\x2a\xa4\xbd\xfb\x38\xf2\x35\x22\x37\x5a\x46\x2f\x45\x09" //..*...8.5"7ZF/E. |
// /* 0140 */ "\xf2\xea\x57\x7b\xe7\x0d\xee\xa6\x81\xf7\x99\x3a\xfe\x45\x63\x39" //..W{.......:.Ec9 |
// /* 0150 */ "\xdf\xf0\x1b\x1c\xe4\xaf\x52\x70\xd1\x0b\xc5\xbd\x23\x87\x23\xff" //......Rp....#.#. |
// /* 0160 */ "\x5f\x41\x73\x07\xbe\x05\x06\x62\x4a\x5b\xda\xa3\x6e\x85\xc0\x96" //_As....bJ[..n... |
// /* 0170 */ "\x0d\xf2\xc1\x68\x1e\xe2\x46\x3b\x63\x0b\x97\xa9\xc6\xda\x43\x28" //...h..F;c.....C( |
// /* 0180 */ "\x05\x4c\x63\x85\x34\xa1\x6a\x71\x5d\xaf\x4e\x65\x53\x30\xb3\xcf" //.Lc.4.jq].NeS0.. |
// /* 0190 */ "\xde\x70\x4f\x7e\x6f\xb3\x7c\xd4\xc5\x98\x72\x52\x01\x97\xae\x35" //.pO~o.|...rR...5 |
// /* 01a0 */ "\xa2\xa5\xc3\xd0\x2e\x7f\x0c\x1f\xa5\xb1\xc7\x12\xa6\x43\x8b\x8c" //.............C.. |
// /* 01b0 */ "\x95\xa9\x4a\x1a"                                                 //..J. |
// Sent dumped on RDP Wab Target (4) 436 bytes |
// mod_rdp::send_confirm_active done |
// Waiting for answer to confirm active |
// mod_rdp::send_synchronise |
// Sending on RDP Wab Target (4) 48 bytes |
// /* 0000 */ "\x03\x00\x00\x30\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x22\x08\x00" //...0...d....p".. |
// /* 0010 */ "\x00\x00\x27\xf4\xec\x82\xb2\x3a\xbb\x4d\x17\xb1\x7d\xbe\x82\x27" //..'....:.M..}..' |
// /* 0020 */ "\xc1\xa2\x84\xaa\x0c\xef\x99\x8f\x80\x69\x00\xb9\xd2\xaa\xf1\xef" //.........i...... |
// Sent dumped on RDP Wab Target (4) 48 bytes |
// mod_rdp::send_synchronise done |
// mod_rdp::send_control |
// Sending on RDP Wab Target (4) 52 bytes |
// /* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x26\x08\x00" //...4...d....p&.. |
// /* 0010 */ "\x00\x00\x50\x48\x19\x37\x31\xb6\xae\xeb\xa3\xe0\xfe\xca\x11\xe7" //..PH.71......... |
// /* 0020 */ "\x7e\x0d\x28\x44\xf4\x94\xd4\xb4\xea\xa9\x9d\xb9\x25\x3e\x34\x53" //~.(D........%>4S |
// /* 0030 */ "\xf4\x04\x06\x8c"                                                 //.... |
// Sent dumped on RDP Wab Target (4) 52 bytes |
// mod_rdp::send_control done |
// mod_rdp::send_control |
// Sending on RDP Wab Target (4) 52 bytes |
// /* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x26\x08\x00" //...4...d....p&.. |
// /* 0010 */ "\x00\x00\xac\x2d\x2b\x62\x76\xb8\x06\x8a\x5e\x9d\x95\xcf\xb4\xbb" //...-+bv...^..... |
// /* 0020 */ "\xf3\xf9\x83\xe2\xa7\x50\xab\x1c\x57\xbf\x96\x94\xc3\x7b\x92\x40" //.....P..W....{.@ |
// /* 0030 */ "\x80\x28\x37\x0e"                                                 //.(7. |
// Sent dumped on RDP Wab Target (4) 52 bytes |
// mod_rdp::send_control done |
// use rdp5 |
// mod_rdp::send_fonts |
// Sending on RDP Wab Target (4) 52 bytes |
// /* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x26\x08\x00" //...4...d....p&.. |
// /* 0010 */ "\x00\x00\xba\x2f\xdc\x17\xee\x50\x08\xc9\x36\x54\x65\x3e\x50\xce" //.../...P..6Te>P. |
// /* 0020 */ "\xc0\x62\x9f\xa3\x70\x0e\x20\x3d\x7b\x09\x4b\xbb\xa3\x2c\x7d\xd7" //.b..p. ={.K..,}. |
// /* 0030 */ "\xe0\xe4\x32\xce"                                                 //..2. |
// Sent dumped on RDP Wab Target (4) 52 bytes |
// mod_rdp::send_fonts done |
// mod_rdp::send_input_slowpath |
// Sending on RDP Wab Target (4) 60 bytes |
// /* 0000 */ "\x03\x00\x00\x3c\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x2e\x08\x00" //...<...d....p... |
// /* 0010 */ "\x00\x00\x1a\x23\xc5\xad\xf9\xea\x5e\x29\xa4\xbc\xb2\x1f\x9a\xae" //...#....^)...... |
// /* 0020 */ "\xe3\xd1\x56\xef\xf3\x45\xce\x7c\x0d\xee\xf2\xb3\x7a\x01\x5a\x13" //..V..E.|....z.Z. |
// /* 0030 */ "\x09\x2b\x95\x47\x9e\x12\xc9\xde\x0b\x3d\x15\xe4"                 //.+.G.....=.. |
// Sent dumped on RDP Wab Target (4) 60 bytes |
// mod_rdp::send_input_slowpath done |
// Resizing to 1024x768x16 |
// --------- FRONT ------------------------ |
// server_resize(width=1024, height=768, bpp=16 |
// ======================================== |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x30"                                                     //..0 |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 44 bytes |
// Recv done on RDP Wab Target (4) 44 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x22\x08\x00\x00\x00\x27\xf4" //...h....p"....'. |
/* 0010 */ "\xec\x82\xb2\x3a\xbb\x4d\xb4\xa6\x96\xc7\x7f\x91\x41\xee\xb4\xd6" //...:.M......A... |
/* 0020 */ "\x4f\xb5\xa9\xf7\x85\x26\x2c\x34\xb9\x05\x68\x51"                 //O....&,4..hQ |
// Dump done on RDP Wab Target (4) 44 bytes |
// LOOPING on PDUs: 22 |
// PDUTYPE_DATAPDU |
// WAITING_SYNCHRONIZE |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x34"                                                     //..4 |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 48 bytes |
// Recv done on RDP Wab Target (4) 48 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x26\x08\x00\x00\x00\xa4\x40" //...h....p&.....@ |
/* 0010 */ "\xbb\x02\xb9\xc2\x91\x89\x6d\xc9\x44\xd8\x6e\x3a\x18\xbc\xc4\xee" //......m.D.n:.... |
/* 0020 */ "\x04\x0d\x46\x52\xd5\x6d\x0b\x1f\xcf\xec\x83\x71\xff\xd0\xe6\xb9" //..FR.m.....q.... |
// Dump done on RDP Wab Target (4) 48 bytes |
// LOOPING on PDUs: 26 |
// PDUTYPE_DATAPDU |
// WAITING_CTL_COOPERATE |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x34"                                                     //..4 |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 48 bytes |
// Recv done on RDP Wab Target (4) 48 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x26\x08\x00\x00\x00\x26\x6a" //...h....p&....&j |
/* 0010 */ "\x56\x5b\x5c\x3c\xb0\x04\x7d\x14\x97\x9a\xab\x01\x43\xa5\x61\x71" //V[.<..}.....C.aq |
/* 0020 */ "\x6a\x9d\xb9\x6b\xda\xb7\x71\x80\x0e\xff\x47\xce\x65\xba\xe6\x7d" //j..k..q...G.e..} |
// Dump done on RDP Wab Target (4) 48 bytes |
// LOOPING on PDUs: 26 |
// PDUTYPE_DATAPDU |
// WAITING_GRANT_CONTROL_COOPERATE |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\xd9"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 213 bytes |
// Recv done on RDP Wab Target (4) 213 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x80\xca\x08\x00\x00\x00\x3a" //...h....p......: |
/* 0010 */ "\x1d\xea\xe8\xfa\xf9\x93\x66\x9e\x8e\xc1\x08\x21\x3f\x6d\xd4\x22" //......f....!?m." |
/* 0020 */ "\x95\x10\xea\x56\x46\x51\xbc\xdb\xbc\x04\xbe\xc0\xea\xd9\xf3\x6b" //...VFQ.........k |
/* 0030 */ "\x5b\x32\x52\x50\x00\xf5\x31\xcb\x61\x50\xfa\xd7\x38\xcd\x04\xd9" //[2RP..1.aP..8... |
/* 0040 */ "\x30\xdf\xef\xbc\x99\x8e\x39\x6c\x49\x9a\x85\xdc\xc1\x94\x63\xac" //0.....9lI.....c. |
/* 0050 */ "\x10\xc5\x90\xc6\x27\xca\x39\x26\x62\xd0\x9b\xf9\xb2\xd9\xa9\xcb" //....'.9&b....... |
/* 0060 */ "\x07\x73\xc7\xe1\x59\xce\x9e\xbb\x5b\xf1\x63\xb6\xeb\x1c\x15\x34" //.s..Y...[.c....4 |
/* 0070 */ "\x65\x70\x3b\xb2\x79\xd6\xf3\x3e\xc8\x5a\xa6\x14\x9a\x4b\x53\x92" //ep;.y..>.Z...KS. |
/* 0080 */ "\x97\xeb\x87\x0d\xdb\x51\xa9\xdd\x4d\xa8\xcb\x2b\x9c\xef\x9b\x72" //.....Q..M..+...r |
/* 0090 */ "\x61\x7d\x5c\xd5\xe7\xec\xa6\x7c\xa6\x62\x60\x20\xdf\x51\x76\x52" //a}.....|.b` .QvR |
/* 00a0 */ "\x53\xf9\x64\xb2\xd4\xfe\x05\x90\x44\x21\x7d\x68\xbd\x15\xbb\x31" //S.d.....D!}h...1 |
/* 00b0 */ "\x83\x7a\x5b\x76\x8c\xe5\x47\x99\xa3\xf8\xca\x82\x04\x84\x25\x5d" //.z[v..G.......%] |
/* 00c0 */ "\x59\x6c\xa3\xf5\x06\x40\x19\x19\x7f\x5d\xee\x2c\xfb\x0a\xba\x4c" //Yl...@...].,...L |
/* 00d0 */ "\x9f\x70\x85\x90\xe7"                                             //.p... |
// Dump done on RDP Wab Target (4) 213 bytes |
// LOOPING on PDUs: 190 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_FONTMAP |
// mod_rdp::send_input_slowpath |
// Sending on RDP Wab Target (4) 60 bytes |
// /* 0000 */ "\x03\x00\x00\x3c\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x2e\x08\x00" //...<...d....p... |
// /* 0010 */ "\x00\x00\x89\x07\x58\x7c\x97\x5b\xb0\x16\x41\x26\x15\xac\x95\x9a" //....X|.[..A&.... |
// /* 0020 */ "\x88\xd3\x41\xa8\x75\x36\xbd\x26\xf4\xf4\xe7\x88\xaa\xbb\x1f\xa4" //..A.u6.&........ |
// /* 0030 */ "\xd6\xe3\xa8\x90\x6a\xb4\xf8\xd5\x45\xa7\x7b\x43"                 //....j...E.{C |
// Sent dumped on RDP Wab Target (4) 60 bytes |
// mod_rdp::send_input_slowpath done |
// ========= CREATION OF MOD DONE ==================== |
// ===================> count = 0 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x30"                                                     //..0 |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 44 bytes |
// Recv done on RDP Wab Target (4) 44 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x22\x08\x00\x00\x00\xcf\x26" //...h....p".....& |
/* 0010 */ "\xea\x88\xff\x88\xb7\x15\xb5\x0e\xd7\x34\x86\x72\x9a\x96\x8b\x8b" //.........4.r.... |
/* 0020 */ "\xdf\x79\x7c\xc0\x63\x15\x21\x81\x55\x06\x51\x4b"                 //.y|.c.!.U.QK |
// Dump done on RDP Wab Target (4) 44 bytes |
// LOOPING on PDUs: 22 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_SYNCHRONIZE |
// ===================> count = 1 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x04\x7d"                                                     //..} |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 1145 bytes |
// Recv done on RDP Wab Target (4) 1145 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x84\x6e\x08\x00\x00\x00\x5d" //...h....p.n....] |
/* 0010 */ "\x58\xb4\x81\x3e\xc8\xe7\xdf\xb1\x23\x88\xea\xa6\xfb\x13\x48\x10" //X..>....#.....H. |
/* 0020 */ "\x01\xcc\x59\xe3\xc3\x3c\x3a\x67\x74\xe5\x06\x67\x8f\x0a\x0f\xcc" //..Y..<:gt..g.... |
/* 0030 */ "\x07\x8e\xae\x5a\xcd\xb5\xe7\xb7\x7a\x17\xf8\x33\x20\xa1\xde\xa7" //...Z....z..3 ... |
/* 0040 */ "\x30\x73\x5e\x74\x7b\xf8\xf3\x71\xf1\xda\x4a\x31\x5c\x49\x90\xac" //0s^t{..q..J1.I.. |
/* 0050 */ "\x92\x68\x2f\x73\x7b\x3b\x9b\x91\x47\x14\xa5\xd8\xf1\xcb\xa7\x18" //.h/s{;..G....... |
/* 0060 */ "\xf5\x1c\x3f\x1a\x25\x76\x0c\x73\x7c\x19\xe5\x3b\x0c\x8f\xc6\x5e" //..?.%v.s|..;...^ |
/* 0070 */ "\xce\x31\xfc\x04\x9c\x51\x25\x6b\x89\x0f\xfa\xad\xee\xe5\x39\x9b" //.1...Q%k......9. |
/* 0080 */ "\x88\x67\x22\xac\x8d\xd7\x10\x1c\xb9\xdd\x2d\xb7\x93\x81\x1c\x8c" //.g".......-..... |
/* 0090 */ "\x05\xfe\xa6\xb9\x58\x41\x46\xc0\x52\x5a\x4b\x04\x26\x07\x05\xf3" //....XAF.RZK.&... |
/* 00a0 */ "\xdf\xbd\x2f\x15\xc3\x2b\xa7\xaf\xc5\x50\xc5\x71\x54\x45\x49\x6a" //../..+...P.qTEIj |
/* 00b0 */ "\x64\x88\xa7\xf2\x11\x98\x1e\x8a\x2d\x88\xec\xcf\xd0\x1a\x8d\xe8" //d.......-....... |
/* 00c0 */ "\x31\x5f\x48\xdf\x4a\x72\x14\x85\xdd\xbf\x57\xcf\x98\x5d\x61\xaf" //1_H.Jr....W..]a. |
/* 00d0 */ "\x09\x3a\x82\x6f\xe7\x3d\xa1\x91\x90\xe8\x38\xb6\x19\xb7\x28\xc2" //.:.o.=....8...(. |
/* 00e0 */ "\xcf\xfd\xe0\xf7\xa5\x4d\x8a\x6c\x98\x0b\x82\x20\x06\x7a\x3a\xa0" //.....M.l... .z:. |
/* 00f0 */ "\xa0\xe5\x86\x7d\xf2\x27\x3b\x6c\xbc\x64\xb5\x3e\x51\x07\x60\xaf" //...}.';l.d.>Q.`. |
/* 0100 */ "\x25\x05\x03\x9f\x03\x67\x75\x71\x53\xd8\x68\xe0\xe2\x46\xc4\xa9" //%....guqS.h..F.. |
/* 0110 */ "\xb2\xbc\x3a\x3b\xb7\xb7\xfb\xe4\x3b\x8d\x3b\xb4\xcd\xe8\x7b\xef" //..:;....;.;...{. |
/* 0120 */ "\xb7\xd3\x3e\x0b\x93\xac\x75\x84\xc3\x60\xf5\x3f\xed\xbf\x25\xd8" //..>...u..`.?..%. |
/* 0130 */ "\xba\x15\xdc\x5d\x64\x58\x54\xf1\xb1\xbe\xdd\x31\xb9\xdc\xa2\xcf" //...]dXT....1.... |
/* 0140 */ "\x63\x54\x4b\x09\xdf\x86\x9e\x4b\x03\x77\xa2\x30\x4c\xdf\xaa\xda" //cTK....K.w.0L... |
/* 0150 */ "\xa9\x5a\x86\xcf\x74\x89\x04\xd3\xaf\x70\x44\x5a\x27\x1e\x7c\x8b" //.Z..t....pDZ'.|. |
/* 0160 */ "\xda\x64\x69\x1f\xbf\x91\x60\x85\xb5\x13\xd3\xb1\xfb\x1d\x0a\xdb" //.di...`......... |
/* 0170 */ "\x42\xa9\x94\xf7\x3c\x0c\x83\x64\xbf\x2e\x99\x01\x75\xd0\x57\xf5" //B...<..d....u.W. |
/* 0180 */ "\x4d\x59\x7b\x6b\x3c\xb1\xb9\x4b\x61\xc8\x9a\xc2\xc1\x9c\x23\xe1" //MY{k<..Ka.....#. |
/* 0190 */ "\x06\x24\x05\x18\x74\xfd\x39\xaa\x52\x0f\x94\xa3\x90\x32\xe0\x7b" //.$..t.9.R....2.{ |
/* 01a0 */ "\x70\x34\x35\x8a\xf0\xfd\x48\xd3\x62\x72\x1b\xf5\x93\xeb\xb1\x8c" //p45...H.br...... |
/* 01b0 */ "\xcb\x59\x40\x54\xd0\x77\xa3\xe4\x2e\x26\x62\x61\x9b\x0a\xef\x7c" //.Y@T.w...&ba...| |
/* 01c0 */ "\x3e\x1a\xa2\xd9\xde\x6d\xc7\xdd\x0f\x9d\x84\x33\x43\xb6\xc3\xea" //>....m.....3C... |
/* 01d0 */ "\x6b\x97\xa7\x45\x6f\xcb\x89\x3f\xfc\xc8\xd6\x18\xc5\x0e\x93\xd9" //k..Eo..?........ |
/* 01e0 */ "\x76\xa9\x88\xda\xcc\x82\xae\xd0\xe2\xd6\xba\xc8\x81\x40\x83\x58" //v............@.X |
/* 01f0 */ "\x81\x62\x52\x62\xa7\xd7\x2e\xc8\x6e\xe8\x7b\xb8\x1f\xc4\xaf\xcf" //.bRb....n.{..... |
/* 0200 */ "\xf3\x10\x70\x03\x4a\xb0\x63\x91\x03\x33\xea\xc3\xde\x88\xd0\x27" //..p.J.c..3.....' |
/* 0210 */ "\x4e\x3a\x30\x16\x21\x6c\x9f\x12\xa7\x8c\xa6\x7d\xd5\x41\xc2\xce" //N:0.!l.....}.A.. |
/* 0220 */ "\x1a\x7f\x72\xa4\x41\x20\x94\xd1\xcc\xd3\x13\xec\x76\xa1\x17\xf0" //..r.A ......v... |
/* 0230 */ "\x4c\x19\x55\xbb\x23\xad\x8d\xc5\x72\xed\x42\x3e\xaf\x3f\x9f\x2c" //L.U.#...r.B>.?., |
/* 0240 */ "\x14\x0f\xef\x52\x58\x36\x2c\x5b\x96\x60\xf3\xf7\x05\xb9\xeb\x9b" //...RX6,[.`...... |
/* 0250 */ "\x32\x5c\x75\x41\xf2\x4b\x85\x18\x2c\x14\xf6\xc4\x85\x82\xed\x3b" //2.uA.K..,......; |
/* 0260 */ "\xd0\x9d\xcf\xa4\x00\xca\x63\xb5\x4c\xcc\x01\x8b\x30\x58\x48\x33" //......c.L...0XH3 |
/* 0270 */ "\x07\xa7\xdc\x7c\xec\xb1\x3a\xf4\xeb\xcd\x82\xe3\xca\xa2\xbc\x8e" //...|..:......... |
/* 0280 */ "\xa6\xd7\x1b\x82\x21\x27\x21\xe3\x43\x0f\x85\x82\x25\x01\x77\x8a" //....!'!.C...%.w. |
/* 0290 */ "\x87\x74\x38\xcc\xe5\xa1\x96\xfd\x68\x6d\xaa\xea\x28\x44\xb7\x0c" //.t8.....hm..(D.. |
/* 02a0 */ "\x84\x7a\xab\x53\x8f\x97\xa5\x13\x70\x56\xd5\x26\xc2\xcc\x24\xc6" //.z.S....pV.&..$. |
/* 02b0 */ "\x20\x72\xb5\x2a\x68\xeb\xa7\xe4\xcf\x81\x2c\x43\x9c\x16\xe3\xc6" // r.*h.....,C.... |
/* 02c0 */ "\xc0\x76\xf3\x25\xc2\x62\x0d\x03\xd4\x99\xa5\xa6\x9b\x53\x7e\x61" //.v.%.b.......S~a |
/* 02d0 */ "\x67\xc5\xdc\xb5\xd2\x83\x51\xd1\x57\x98\x98\x58\x74\xb4\x48\x63" //g.....Q.W..Xt.Hc |
/* 02e0 */ "\xd8\xf0\x28\xff\x6e\x3e\xb7\x6d\x39\xa1\x98\x6d\x37\x95\xbf\x86" //..(.n>.m9..m7... |
/* 02f0 */ "\x16\x40\x42\xff\x1c\x9b\x2a\x07\x9b\xc9\x42\xcc\xac\xef\x87\xcc" //.@B...*...B..... |
/* 0300 */ "\x29\x34\xf8\xe4\x56\x79\xf1\xa0\x7a\x64\x6c\x67\xf1\x69\x2c\x41" //)4..Vy..zdlg.i,A |
/* 0310 */ "\xcb\xb1\xba\xc8\x9f\x20\x81\x18\x91\x26\x08\x8a\x20\xf8\x61\x64" //..... ...&.. .ad |
/* 0320 */ "\xab\xe4\xf3\x86\x7b\xb8\x44\x73\xb6\x18\x8b\x28\x69\x6f\x79\xfe" //....{.Ds...(ioy. |
/* 0330 */ "\xa1\xda\x3e\xbc\x9c\x8f\xb5\xfc\x83\xd6\x5b\x72\x1f\x6c\x66\xbf" //..>.......[r.lf. |
/* 0340 */ "\x75\x04\xf4\x40\xc9\x6f\x8a\x97\x78\x3c\xd6\x78\x8c\x06\xca\xc5" //u..@.o..x<.x.... |
/* 0350 */ "\x0c\x96\x35\xd6\xbe\x9a\x0e\x0d\x48\x01\xf5\xd4\xf2\xe8\x81\x2a" //..5.....H......* |
/* 0360 */ "\xb3\x38\x14\x14\x6b\xb3\x00\xd4\x86\xd8\x9c\x52\xd4\x1d\x64\x81" //.8..k......R..d. |
/* 0370 */ "\x00\x38\x0b\x7b\x7b\x41\x14\x4f\xff\x88\x2f\x36\x64\xb5\x3b\x78" //.8.{{A.O../6d.;x |
/* 0380 */ "\xfd\x16\x9a\xfd\xcc\x0c\x7e\x69\xb7\x80\x1e\x49\xa5\x01\x36\x64" //......~i...I..6d |
/* 0390 */ "\x6e\xf7\x7d\x6a\x89\xc7\x8a\x36\x20\x4a\x75\x91\xdc\x44\xad\x3a" //n.}j...6 Ju..D.: |
/* 03a0 */ "\xf5\x60\x40\x82\xf0\xcd\x0f\x36\x9b\xb3\x33\x17\x69\x03\xa2\x1e" //.`@....6..3.i... |
/* 03b0 */ "\xd8\x36\x48\x0f\x10\x5f\x2a\x30\x29\xd1\x76\xf1\x17\x10\xf1\x1c" //.6H.._*0).v..... |
/* 03c0 */ "\x2e\x94\x4c\x28\x1b\xd6\x9b\x3b\xf2\x2e\x31\x00\x2b\xd7\xc0\x61" //..L(...;..1.+..a |
/* 03d0 */ "\xce\xe9\xe6\x4f\xfd\x18\x63\x00\x72\xa8\x21\xd2\xa3\xf8\xb2\xe7" //...O..c.r.!..... |
/* 03e0 */ "\x5a\x54\x0a\xf3\xa6\x55\xc9\x04\x52\x54\xf7\xae\x4d\xef\x00\x44" //ZT...U..RT..M..D |
/* 03f0 */ "\xb3\x6d\x6c\x22\x1a\x54\xcd\xce\x9c\x57\x82\xb2\x71\xb2\xcb\xe7" //.ml".T...W..q... |
/* 0400 */ "\x9c\xcf\x50\xb0\xb0\xd6\xb0\xef\xe6\x2e\xb0\x10\x1d\xca\x6a\xcc" //..P...........j. |
/* 0410 */ "\xe3\x79\xad\x84\x7c\xf9\x3e\xeb\xad\xcd\x74\x3e\xfb\x7d\xf6\x4f" //.y..|.>...t>.}.O |
/* 0420 */ "\xff\x2b\x16\xa5\x74\x22\x33\x91\x4a\x9a\xe1\xe4\xfe\x77\x19\x2f" //.+..t"3.J....w./ |
/* 0430 */ "\x66\x44\x27\x73\x39\xda\x28\xec\xf7\x54\xaf\x9c\xc8\x24\x66\xe6" //fD's9.(..T...$f. |
/* 0440 */ "\xa2\x86\x4f\x5c\xb3\xa1\x86\x30\xb0\x70\xf4\x43\xc6\x19\x77\xef" //..O....0.p.C..w. |
/* 0450 */ "\xba\xf8\xbe\xb5\xa3\x68\xdf\x5d\x2a\xb1\x8f\x35\xe2\xcc\x65\xb8" //.....h.]*..5..e. |
/* 0460 */ "\xe6\x9d\x40\x17\xc6\xe4\x08\x28\x1f\xe2\x83\xc6\xe5\xfc\x42\x4b" //..@....(......BK |
/* 0470 */ "\x6d\xa9\x37\x24\x8c\x6e\x11\x66\xbc"                             //m.7$.n.f. |
// Dump done on RDP Wab Target (4) 1145 bytes |
// LOOPING on PDUs: 1122 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_ORDERS |
// process_orders bpp=16 |
// process_colormap |
// receiving colormap 0 |
// process_colormap done |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(16,63,197,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(18,64,1,15) color=0x008c51) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,62,198,1) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,63,1,18) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(213,62,1,20) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,81,199,1) color=0x000273) |
// ======================================== |
// process_orders done |
// ===================> count = 2 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x09\xa9"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 2469 bytes |
// Recv done on RDP Wab Target (4) 2469 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x89\x9a\x08\x00\x00\x00\x2b" //...h....p......+ |
/* 0010 */ "\x18\x93\x18\x25\x96\xd7\x03\xac\x46\xa1\xb2\x81\x1b\x62\x1b\xed" //...%....F....b.. |
/* 0020 */ "\x49\xf3\x08\x9c\x34\x90\x15\xbb\xb6\x42\xbd\xbb\x54\xf1\x8f\x8c" //I...4....B..T... |
/* 0030 */ "\xa0\x35\x65\x73\x4f\x6f\x21\x81\x27\xfc\xd9\x39\xd9\x8e\x08\x78" //.5esOo!.'..9...x |
/* 0040 */ "\x96\x33\x5b\x31\x13\x5b\x87\x8c\xbc\x2d\x1e\x57\x88\xc5\x50\x76" //.3[1.[...-.W..Pv |
/* 0050 */ "\x6d\xc2\xe1\xd0\x45\xa5\x18\xef\x28\x56\xf9\xee\xb3\xcb\x55\x46" //m...E...(V....UF |
/* 0060 */ "\x9e\x25\x58\x14\x85\x14\xda\x0d\x6e\x1b\xdc\x5c\xeb\xd9\x6c\x19" //.%X.....n.....l. |
/* 0070 */ "\x6c\x81\xcd\x3d\x6b\x8b\x97\x7a\x8c\x29\x65\xb9\x6b\x7b\xc4\x25" //l..=k..z.)e.k{.% |
/* 0080 */ "\xc3\x23\xa1\x0e\x16\x77\x39\x94\x5c\xee\x90\xf0\x45\xea\x62\xbf" //.#...w9.....E.b. |
/* 0090 */ "\x19\x09\x68\x14\x33\x9c\x89\xe1\x43\x82\xce\x0b\xc8\xc7\x03\x57" //..h.3...C......W |
/* 00a0 */ "\xe2\xd4\x8f\xbf\x3c\xe8\x11\xb6\x52\x7d\x93\x79\x37\xc8\x1d\x8a" //....<...R}.y7... |
/* 00b0 */ "\xba\x33\x86\x60\xc9\xae\x79\x11\xea\xd2\x71\x42\x81\x32\x72\x85" //.3.`..y...qB.2r. |
/* 00c0 */ "\x71\x7d\xe1\x9b\xc1\x99\x1a\xea\x78\x37\x63\x5a\x4c\x48\xf0\x67" //q}......x7cZLH.g |
/* 00d0 */ "\xeb\x75\x7e\xd0\xf4\xa8\x7a\x37\x86\xfe\x0c\xfd\x84\x74\x81\x73" //.u~...z7.....t.s |
/* 00e0 */ "\xdd\x99\x8b\x44\xf0\x11\xc8\x8f\xcd\xa0\x96\x98\x22\xb9\x0c\xbc" //...D........"... |
/* 00f0 */ "\x31\x19\xac\x60\x2f\xa0\x1c\xa7\x20\x11\x19\xc3\x67\x63\xa8\x7e" //1..`/... ...gc.~ |
/* 0100 */ "\x42\x0b\xf2\x2f\xd1\x80\x60\x76\x99\x9b\xa1\x4c\xd5\x3a\xb2\x14" //B../..`v...L.:.. |
/* 0110 */ "\x6d\xee\xc4\xcc\x0e\xb1\x8c\x54\xb5\x8b\x65\x19\x6c\xf0\x17\x17" //m......T..e.l... |
/* 0120 */ "\xfd\xc4\x70\x87\x45\x7e\x18\xe3\x4e\xe9\x2c\x12\x83\x11\x87\x1c" //..p.E~..N.,..... |
/* 0130 */ "\x19\x95\x42\xa9\x2c\x20\x28\x6b\x8f\xc1\xa8\x14\x7a\x1b\x7f\x38" //..B., (k....z..8 |
/* 0140 */ "\x87\x3a\x9a\x1e\xd0\x09\xbc\xc4\xa1\x78\xdd\xc2\xbc\x50\x73\xef" //.:.......x...Ps. |
/* 0150 */ "\xfe\x62\x55\x25\xe4\x6b\xa6\xb5\xd3\x2e\x73\x0f\x14\xd7\x24\x57" //.bU%.k....s...$W |
/* 0160 */ "\x34\xfd\x4f\xe7\xfb\xc7\xab\xcb\x0c\x1f\x62\x55\x2c\x8f\xbc\x7c" //4.O.......bU,..| |
/* 0170 */ "\x2b\x98\x5a\x7f\x1a\x7b\xff\x5c\xc3\x2c\x43\x91\x4a\x1b\x16\xa8" //+.Z..{...,C.J... |
/* 0180 */ "\x52\x50\x99\x5d\x99\x20\x31\x23\x69\x54\x56\x29\x57\x73\x5d\xde" //RP.]. 1#iTV)Ws]. |
/* 0190 */ "\x24\x7b\x21\x62\x7a\x3c\xea\x80\xf9\xd2\x47\xe6\x2a\x6c\x4e\x99" //${!bz<....G.*lN. |
/* 01a0 */ "\xa0\xee\xdb\xc8\x66\x77\x74\x38\xb4\x9b\x89\x21\xb5\xe5\x49\x70" //....fwt8...!..Ip |
/* 01b0 */ "\x35\xea\x7d\x94\x48\xf9\xa0\x08\xdc\x6e\xce\x62\x5c\xe0\x4d\xc7" //5.}.H....n.b..M. |
/* 01c0 */ "\x06\x4e\xff\x42\xc0\xa6\x13\xf8\xab\xd2\x0e\xce\x6a\x77\x18\x9f" //.N.B........jw.. |
/* 01d0 */ "\xfb\x83\x85\x76\x0e\x25\x90\x1d\xb8\x6b\x5b\xb4\x13\xb6\x79\x3d" //...v.%...k[...y= |
/* 01e0 */ "\xee\xcd\xd6\x5a\x0b\xd9\xa1\x31\x4a\x53\x28\x5f\x6a\xcc\x27\xdb" //...Z...1JS(_j.'. |
/* 01f0 */ "\x85\x77\x73\xae\x85\xae\xb8\x17\x5c\x0a\x33\xdb\xc1\xc3\xaa\xab" //.ws.......3..... |
/* 0200 */ "\x8c\x88\x89\x89\x11\x25\x50\xcc\xf7\x90\x81\x4b\x91\x38\xab\x69" //.....%P....K.8.i |
/* 0210 */ "\x22\x9c\x44\x84\xc7\x8b\x2d\x41\x58\x6f\x1a\xfb\x54\x79\x38\xc1" //".D...-AXo..Ty8. |
/* 0220 */ "\xe6\xfc\x94\x92\x7f\xf6\x4d\x6d\x62\xfb\x8e\x34\x88\xe9\x51\xaa" //......Mmb..4..Q. |
/* 0230 */ "\xcb\xd0\xc9\x33\xf9\xe4\xdc\x3c\x57\x92\xe2\xc0\x12\x79\xcc\x5e" //...3...<W....y.^ |
/* 0240 */ "\xac\xd4\x43\x87\xa2\xe3\xd7\xf7\x9b\xc3\xd6\xb2\xc6\x24\xea\x83" //..C..........$.. |
/* 0250 */ "\xe4\xc7\xe9\xdf\x8a\x9f\x83\x02\x1e\xa2\x4a\x0a\x00\x26\x82\xfa" //..........J..&.. |
/* 0260 */ "\xc3\xcb\xbf\x65\xa5\x00\x45\x41\x8d\xc7\x30\x62\x71\x67\xa1\x55" //...e..EA..0bqg.U |
/* 0270 */ "\xe1\x33\x10\x54\x6c\x72\xec\xf9\x47\x3f\xcd\xcd\x3f\x7a\xda\x69" //.3.Tlr..G?..?z.i |
/* 0280 */ "\xb2\x18\x62\x7b\x4f\x46\x52\x36\x3c\x52\xcb\x27\x97\x2e\x8f\x6c" //..b{OFR6<R.'...l |
/* 0290 */ "\xd6\xbc\xb1\xc3\x65\x2d\x95\x59\xca\x09\xfd\xca\x42\xd0\xbe\xbf" //....e-.Y....B... |
/* 02a0 */ "\x07\xb5\xff\x04\x4c\x7b\xbd\x39\x27\x05\xa5\x13\xf0\x3f\x5d\x9c" //....L{.9'....?]. |
/* 02b0 */ "\xfc\x04\xce\x46\x64\x87\x7e\xa5\xf5\xf3\xfa\x54\x69\xbe\x3e\x4e" //...Fd.~....Ti.>N |
/* 02c0 */ "\xf9\x6b\xe3\x3b\x4b\x0c\x16\xb1\xd7\x14\x82\x33\x77\xff\xd8\xbd" //.k.;K......3w... |
/* 02d0 */ "\x90\x01\x58\x86\x6f\xd0\x41\x73\xd5\xd3\xee\x77\x19\x68\xfc\xb8" //..X.o.As...w.h.. |
/* 02e0 */ "\x66\x72\xc0\x27\x29\xd9\x8f\x0f\xdc\x99\xdd\xa1\x48\x82\xd9\xb6" //fr.').......H... |
/* 02f0 */ "\xba\xb4\xcf\xc2\xb7\x1c\x82\xd1\xd4\x34\xd8\xae\x14\x77\xc6\x13" //.........4...w.. |
/* 0300 */ "\x6d\xff\x71\x3b\xb6\x8f\x33\xbf\xf5\x91\x14\x54\xb0\x6a\xe5\x6d" //m.q;..3....T.j.m |
/* 0310 */ "\xe6\x32\x49\xe4\xdf\xa2\xd4\x56\x49\x41\x0d\x10\x0c\x4e\xc1\x87" //.2I....VIA...N.. |
/* 0320 */ "\x90\x00\x34\x05\x47\x5b\x31\xb0\x8d\x3a\xb8\xc9\x1c\xaf\x9b\x89" //..4.G[1..:...... |
/* 0330 */ "\x48\x4c\x58\xf1\xab\x40\xaf\x0f\xf1\xdf\x01\xae\x6c\x73\x5e\x82" //HLX..@......ls^. |
/* 0340 */ "\xa0\x40\x41\x1b\x3d\x4a\x73\xab\x1d\x6d\xfc\xf2\x4a\xfa\x5f\x0f" //.@A.=Js..m..J._. |
/* 0350 */ "\xee\xd8\x54\x37\x1b\x3e\x6b\xdc\xae\xd8\x7a\x8d\x75\x0b\x2d\x4e" //..T7.>k...z.u.-N |
/* 0360 */ "\xd3\x3c\xd7\x33\x4e\x77\x1b\x37\xc3\xe9\x78\xc7\x37\xdd\xba\x6a" //.<.3Nw.7..x.7..j |
/* 0370 */ "\xd8\x67\x3a\xb5\x42\x8e\xbc\xa7\xbf\x56\xbc\xac\x76\xbf\x78\x98" //.g:.B....V..v.x. |
/* 0380 */ "\xd4\xf7\x5f\xea\xe7\x15\x1b\x74\xfb\xbe\x59\xcf\xdf\x68\x86\xf6" //.._....t..Y..h.. |
/* 0390 */ "\x8e\xdf\x16\xea\xe6\x0e\x9e\x52\x3b\x42\xde\x10\xd1\xfb\xca\xb4" //.......R;B...... |
/* 03a0 */ "\x0b\xcc\x14\x22\xd9\x55\x0d\x66\x4d\xf7\xf9\x09\xf8\xcd\x37\xf5" //...".U.fM.....7. |
/* 03b0 */ "\x27\x17\x63\x7e\x0e\x9a\x18\x51\x99\x64\xdf\xe6\xaf\x36\x04\xe2" //'.c~...Q.d...6.. |
/* 03c0 */ "\x55\x01\x51\xde\xc2\x25\x26\xc4\xc6\x93\xab\x5d\x2a\xa0\xa6\x99" //U.Q..%&....]*... |
/* 03d0 */ "\x3d\x4b\x39\x36\x04\xa6\x1f\x93\xe4\xe1\xb2\x3a\xf4\x04\x84\xd2" //=K96.......:.... |
/* 03e0 */ "\xb8\x4f\x28\x63\x58\x35\x7a\x53\xcc\x79\x26\x8c\x7d\x00\x4c\x9b" //.O(cX5zS.y&.}.L. |
/* 03f0 */ "\x3a\x94\x78\x15\x91\x86\xb4\xde\x44\x32\x57\x38\x29\x49\xd4\x9a" //:.x.....D2W8)I.. |
/* 0400 */ "\xba\xc2\x1e\x5f\xf4\x37\xfa\x39\xd7\x8f\xc1\xe2\xd7\xd1\x12\xbf" //..._.7.9........ |
/* 0410 */ "\x77\xfd\xc6\x4b\xe7\x09\x9d\xf2\x29\x1e\xd5\x49\x2c\x6f\x7f\x7c" //w..K....)..I,o.| |
/* 0420 */ "\x8f\x11\x46\x22\x1e\x44\x30\x9b\x65\xcc\x52\x03\x2d\xa7\x70\xb4" //..F".D0.e.R.-.p. |
/* 0430 */ "\xfb\x4b\x84\x9b\x3d\x3c\x1e\xc0\xac\xf5\xe0\x3c\xee\x1b\x38\x94" //.K..=<.....<..8. |
/* 0440 */ "\x63\x78\x2f\x4b\x9f\xeb\x7b\x22\xb0\x59\x8a\xfb\x59\xea\x83\x0e" //cx/K..{".Y..Y... |
/* 0450 */ "\x29\xcb\x19\x82\xbf\xe9\x73\x2b\xa7\xcd\xe4\xca\x78\xd0\xc1\x4d" //).....s+....x..M |
/* 0460 */ "\x39\xe6\x45\x7c\x05\x77\x35\x11\x88\x28\x38\x6f\xc3\xe9\x6f\xed" //9.E|.w5..(8o..o. |
/* 0470 */ "\xe8\xf9\xe5\x6c\x4b\x51\xb2\x00\xff\xe8\x62\xb6\xbd\xa3\xf9\x2d" //...lKQ....b....- |
/* 0480 */ "\xcb\xae\x0d\xd3\x7b\xaf\x9a\xe4\x64\x4a\x61\x7e\x0b\x63\xa8\x0b" //....{...dJa~.c.. |
/* 0490 */ "\x69\x01\x0c\x85\xb7\x13\x25\x20\x27\x62\xaf\x01\x7a\xb6\xfc\x9b" //i.....% 'b..z... |
/* 04a0 */ "\x4f\xd9\xb6\xa5\x4f\x60\xda\x93\xb6\x40\xdc\x01\xf1\x76\x63\xe8" //O...O`...@...vc. |
/* 04b0 */ "\x5c\x1b\x8b\xf2\x19\xb4\xc3\x2d\x60\x69\xfe\xad\xd5\x95\xce\xeb" //.......-`i...... |
/* 04c0 */ "\x30\x79\xdb\xf4\xa0\x8d\xad\x12\x11\x61\xb2\x8d\x98\x9b\xc9\x90" //0y.......a...... |
/* 04d0 */ "\x53\xf4\x53\x83\x07\xdc\x12\xfe\x6c\xaf\xe3\xf1\xb0\x7f\x8f\xf0" //S.S.....l....... |
/* 04e0 */ "\x81\x41\xb1\x96\xd5\xe4\x2b\xdb\x51\x75\x00\x81\x1d\x97\xdf\x30" //.A....+.Qu.....0 |
/* 04f0 */ "\x6e\xf6\x6e\x25\xa0\xd1\xb7\x95\xdb\xc1\x27\x3e\xca\x5b\x70\xf8" //n.n%......'>.[p. |
/* 0500 */ "\xcf\xdf\xc4\x1d\x28\x2f\x4f\x8c\x04\x0d\x48\x53\xed\x80\x3c\xa6" //....(/O...HS..<. |
/* 0510 */ "\x16\x6b\xb9\xd0\x20\xcc\xc1\xe8\x6f\x6c\x2f\xe0\xe6\x29\xcc\x6e" //.k.. ...ol/..).n |
/* 0520 */ "\x6d\x3f\xde\x2b\xbe\xe7\x0a\x51\x9f\xbc\x68\xca\x3b\x6c\x73\x68" //m?.+...Q..h.;lsh |
/* 0530 */ "\xad\x60\x68\xdb\xbf\xf3\xf7\x25\x75\xe3\xfa\x69\x81\xd7\x15\xc1" //.`h....%u..i.... |
/* 0540 */ "\xc3\x7f\x2c\xb7\x81\x7d\xfe\x96\x0d\x53\x47\x29\x2b\xd6\xba\x30" //..,..}...SG)+..0 |
/* 0550 */ "\xaa\x4d\x00\x33\xdb\x1c\x0f\x1a\x4e\x26\xb9\xce\x51\x77\x56\xbf" //.M.3....N&..QwV. |
/* 0560 */ "\xfd\x33\x40\x5b\x4f\x34\x13\xc0\x4d\x19\x90\x13\x68\x4c\x7d\x06" //.3@[O4..M...hL}. |
/* 0570 */ "\x14\x24\xdf\x6f\x7e\x01\x64\x9c\x56\x0c\x52\x0b\xca\x0d\x08\x81" //.$.o~.d.V.R..... |
/* 0580 */ "\xc7\x0c\x7a\x6c\x96\x9f\x43\xc0\xba\x9c\x44\xa0\x63\x2e\xb4\x95" //..zl..C...D.c... |
/* 0590 */ "\xa5\x46\x3c\xe9\xbf\x3e\xc0\x58\x5c\xee\x20\x82\x9e\x40\x1c\x67" //.F<..>.X.. ..@.g |
/* 05a0 */ "\x91\x80\xc3\xde\x13\xb0\xec\x28\xda\x1b\x29\xe2\xa2\x28\xb8\x04" //.......(..)..(.. |
/* 05b0 */ "\x1c\xc7\x2d\xfc\x2a\x91\x4b\x46\x4c\x88\x85\x78\x19\x78\x8c\x36" //..-.*.KFL..x.x.6 |
/* 05c0 */ "\x57\xd7\xfe\xdf\x9f\x23\x79\x1c\xb3\x49\xe8\x15\xad\x72\xb9\x8d" //W....#y..I...r.. |
/* 05d0 */ "\xe8\x9a\x72\xa0\xd5\x9f\x6f\x53\x59\x64\xc1\x95\xa0\x25\x91\x13" //..r...oSYd...%.. |
/* 05e0 */ "\x9e\x7f\xd1\x73\xe3\x65\x71\x3a\xab\x55\xb4\x7c\xad\xfa\x00\x1b" //...s.eq:.U.|.... |
/* 05f0 */ "\x8d\xee\xd4\x6e\x41\x2a\xd6\x9c\x09\x34\x93\x24\x64\xcc\x9f\x16" //...nA*...4.$d... |
/* 0600 */ "\xf5\x39\x45\xfe\x04\x1e\x2f\x6f\x5c\xda\x3b\xab\xa1\x55\x62\xb0" //.9E.../o..;..Ub. |
/* 0610 */ "\xe8\x7f\x32\xe4\x01\x6b\xb0\x10\xf2\x1a\x83\xfe\x1e\xf9\xb7\x61" //..2..k.........a |
/* 0620 */ "\xa8\xe9\xf2\x44\xbf\x43\x2a\xe0\x34\x11\xc9\x7e\xf8\x45\x94\xba" //...D.C*.4..~.E.. |
/* 0630 */ "\xae\x9c\x62\xca\x4c\xf9\xcc\x48\x32\x86\xeb\x70\xc7\xbe\x26\x55" //..b.L..H2..p..&U |
/* 0640 */ "\x1e\xfb\x8f\x32\xcf\xdb\xa5\xfe\x63\xe4\x20\xae\x39\xa1\x4b\x31" //...2....c. .9.K1 |
/* 0650 */ "\x82\x06\x81\x83\x36\xf8\x9c\xb2\xae\xa5\xf9\x95\x81\xe5\x10\x69" //....6..........i |
/* 0660 */ "\xb4\x5c\x90\x36\xf1\xe4\x95\x19\xdb\xd6\x1d\xfe\xf3\x75\xb6\x89" //...6.........u.. |
/* 0670 */ "\x0a\x70\x71\x4e\x7c\xd2\x95\x06\xd5\xaf\x8b\x01\x1a\x0f\xf5\x30" //.pqN|..........0 |
/* 0680 */ "\xee\xa1\xea\xef\xd4\xc5\xca\xfc\x51\x4b\x6b\x1b\xe5\x4a\xab\xee" //........QKk..J.. |
/* 0690 */ "\x4e\x0d\x19\xc7\x41\x5e\x19\xf2\x09\x73\x7c\x10\xfe\xaa\x9e\xeb" //N...A^...s|..... |
/* 06a0 */ "\x8a\x96\x00\x0f\xd1\x7c\x09\x67\x28\x8a\x15\x72\xe0\x02\xc1\xd3" //.....|.g(..r.... |
/* 06b0 */ "\x67\x3a\xa3\xef\x83\x03\xd1\x83\x9a\x7b\xb7\x16\xdd\x3a\x78\x72" //g:.......{...:xr |
/* 06c0 */ "\xc1\x6d\x87\x7f\xe1\x40\xa0\xed\xe9\x1b\xe1\xac\x86\x41\x44\xb0" //.m...@.......AD. |
/* 06d0 */ "\xfb\xb3\x06\xfa\xca\x55\xee\x90\x72\xea\x20\x90\x48\x82\xa1\x0b" //.....U..r. .H... |
/* 06e0 */ "\x72\xcd\x56\x75\xaa\x03\x09\xaf\x3e\x4a\x91\x43\x61\xc6\x90\x4b" //r.Vu....>J.Ca..K |
/* 06f0 */ "\x66\xb7\xd6\x6a\x67\xe8\x1c\xfc\xb9\x30\xf4\x5a\x4c\x90\x23\x7e" //f..jg....0.ZL.#~ |
/* 0700 */ "\xed\xd2\xcb\x46\x19\xd4\xd3\x59\xcc\xc4\xc2\x27\xeb\x4a\x43\xcd" //...F...Y...'.JC. |
/* 0710 */ "\x6d\x96\xa3\xdc\x19\x44\x70\xaa\xe8\x8a\x89\x48\x9c\x6d\x92\x4c" //m....Dp....H.m.L |
/* 0720 */ "\x51\xbb\xeb\xc6\x1b\xa6\xf2\x68\x07\xc6\xb8\x83\xd8\x7a\x1f\xa6" //Q......h.....z.. |
/* 0730 */ "\x89\x34\x73\x21\x94\x04\x2a\xf7\xf7\x71\x8b\x8a\xf0\xc2\xb3\x3c" //.4s!..*..q.....< |
/* 0740 */ "\xb3\xa0\x9e\xd5\xb2\x2a\x17\xd9\x35\xc8\x01\x7c\xa9\x17\xb3\xb3" //.....*..5..|.... |
/* 0750 */ "\x05\xb2\xe2\x07\x13\x36\xd3\x46\x2c\x1b\xa6\x95\x45\x82\x53\xee" //.....6.F,...E.S. |
/* 0760 */ "\x6a\x81\xcb\x09\xd1\x52\xf3\x1e\x9b\x11\x55\x67\x43\x55\x9b\xd7" //j....R....UgCU.. |
/* 0770 */ "\xf9\xfc\x6b\xf6\x94\x50\x2c\x41\xa6\x84\x31\x8b\x71\x50\x20\x7d" //..k..P,A..1.qP } |
/* 0780 */ "\x29\xe3\x12\xd4\xbc\xdc\x8a\x84\x0d\x46\x89\x20\x32\x8c\x39\x73" //)........F. 2.9s |
/* 0790 */ "\x53\x7e\x9b\x5a\x41\x23\x0d\x3d\xee\xbd\x7f\xf5\x17\xcf\xd4\x21" //S~.ZA#.=.......! |
/* 07a0 */ "\x02\xa0\xdc\xd6\x13\xdd\xe9\x40\x7c\xdc\x9a\x20\x85\xaf\x7d\x6d" //.......@|.. ..}m |
/* 07b0 */ "\x80\x89\x44\xf9\x8c\xe9\xd4\x65\x18\x00\xf8\x2c\x38\x09\x4d\x4a" //..D....e...,8.MJ |
/* 07c0 */ "\x5a\x1a\x1d\x06\x5b\x52\xbd\x23\x69\x0e\xc4\x1f\xae\xbf\xe4\xa1" //Z...[R.#i....... |
/* 07d0 */ "\x94\xd5\x98\x98\x2b\x06\xb7\xd2\xa0\xd4\xe1\x5b\x34\x70\x61\xd3" //....+......[4pa. |
/* 07e0 */ "\x2b\x1d\x5b\x72\xc9\x1c\xd5\xb4\x3f\x2f\x72\x86\xc0\xe0\xcd\xae" //+.[r....?/r..... |
/* 07f0 */ "\x66\x2b\x23\xe6\x4d\xf6\x9d\x7d\xba\x91\x40\x0f\x29\x7a\x73\x35" //f+#.M..}..@.)zs5 |
/* 0800 */ "\xca\x7b\x7e\xd4\x4d\x14\x9b\x05\x6e\x93\xb4\x6d\x1a\x34\x44\xda" //.{~.M...n..m.4D. |
/* 0810 */ "\x64\x7b\xd8\x8a\xba\x00\xa0\xce\x5c\xe0\xe2\x05\x4a\xad\x3a\xf3" //d{..........J.:. |
/* 0820 */ "\x85\x18\x14\x98\x1b\x8d\x4f\x30\xca\x39\x9e\x77\x74\x44\xc0\x12" //......O0.9.wtD.. |
/* 0830 */ "\x65\xfd\x83\xeb\x06\xee\x63\x6f\x5a\xff\x7c\xd9\xc6\xff\x1f\xac" //e.....coZ.|..... |
/* 0840 */ "\x1d\x1d\x6d\x6b\xef\xbc\x66\x41\x78\xae\x11\x7e\x49\x87\x78\x34" //..mk..fAx..~I.x4 |
/* 0850 */ "\xcd\x71\xc8\x0d\x38\x6e\x55\xa6\xd0\x39\x57\xef\x18\x44\x2f\x58" //.q..8nU..9W..D/X |
/* 0860 */ "\x9e\x4a\xe4\x6d\x60\x14\xca\xc4\x29\x39\x83\x9c\xa6\x9f\xde\x15" //.J.m`...)9...... |
/* 0870 */ "\xb2\xec\x77\xfb\x9b\xa7\xa4\xf3\x59\x9d\xe3\x40\x0e\xfd\x68\x8e" //..w.....Y..@..h. |
/* 0880 */ "\xbc\xc4\x60\x34\x5f\xf2\x8b\x5c\x60\x66\x2b\x3f\x7b\x6e\x45\xe3" //..`4_...`f+?{nE. |
/* 0890 */ "\xb3\xcc\x53\xfa\x09\x3f\x7a\xb2\xcf\x5b\x4d\xb7\x5b\x51\x9c\x10" //..S..?z..[M.[Q.. |
/* 08a0 */ "\x66\x31\x2d\xb2\xdf\x79\xaf\x8d\xdb\xf7\x7c\x03\xb2\x20\x92\xeb" //f1-..y....|.. .. |
/* 08b0 */ "\xc3\xfe\x19\xab\x34\x65\xc9\xed\x00\xa4\xd6\xb0\xd0\x97\x9e\xdd" //....4e.......... |
/* 08c0 */ "\x2b\xf7\x2d\xae\xdf\x9e\x95\xec\x6d\xe4\x78\x6c\x23\x8b\x83\xa7" //+.-.....m.xl#... |
/* 08d0 */ "\xec\x1c\xe3\x13\x14\x32\xbc\x4e\x48\xc6\x93\xe8\x4d\xf5\xf5\xb9" //.....2.NH...M... |
/* 08e0 */ "\x6e\xce\xe7\x90\x2f\x75\x0a\x01\x9f\x42\x13\x18\xe1\x49\x08\x43" //n.../u...B...I.C |
/* 08f0 */ "\x6e\xf1\x5c\x34\x7d\x0e\xf0\x42\xc1\x03\x2b\x94\xdc\xef\x6b\xea" //n..4}..B..+...k. |
/* 0900 */ "\xb0\xc5\x30\xa8\x7a\x20\x35\x68\x2c\xef\xb0\x61\xa1\x80\x12\x05" //..0.z 5h,..a.... |
/* 0910 */ "\x5b\x64\xce\xf6\xac\xa8\xcb\x6e\x9d\xac\x19\xd0\x80\x67\x41\x86" //[d.....n.....gA. |
/* 0920 */ "\x77\x71\x26\x4e\xa3\x77\xf1\x14\x8b\xf5\xd8\xd5\x6a\x2a\x36\x8d" //wq&N.w......j*6. |
/* 0930 */ "\xda\x78\x60\xa1\xe9\xf1\x00\x03\xca\xa7\x2e\x6f\x2c\xec\x09\xa2" //.x`........o,... |
/* 0940 */ "\xcc\x72\xe4\xf3\x2b\xda\xfc\x87\xa7\x1c\x02\x8b\xb7\x3e\x82\xf5" //.r..+........>.. |
/* 0950 */ "\x56\xe2\x05\xd4\x87\x8c\xe7\x2f\x47\x2a\x3d\xcf\xed\x2f\x31\x0f" //V....../G*=../1. |
/* 0960 */ "\x1b\x86\xdd\x72\x7d\x41\x84\xd3\x70\x7f\x57\xd0\xdc\x3f\x1b\xf9" //...r}A..p.W..?.. |
/* 0970 */ "\x87\xa4\x56\x6f\xf5\xff\x35\xdb\xe8\xfb\xe6\x6e\x6c\x2a\x6e\x6c" //..Vo..5....nl*nl |
/* 0980 */ "\x38\x1a\x0c\x53\xd9\x20\x0d\x41\x29\x65\x82\x8b\x49\x1d\x6b\xc7" //8..S. .A)e..I.k. |
/* 0990 */ "\x99\xd9\xa6\xcb\xa4\x0c\x54\xfc\x01\x81\x40\xa2\xbb\xef\x43\xb9" //......T...@...C. |
/* 09a0 */ "\x36\x3d\x00\x66\x1e"                                             //6=.f. |
// Dump done on RDP Wab Target (4) 2469 bytes |
// LOOPING on PDUs: 2446 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_ORDERS |
// process_orders bpp=16 |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,0,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,10,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(127,10,829,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,10,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,26,956,7) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,26,20,7) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,33,1024,8) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,41,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,41,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,57,1024,5) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,62,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(214,62,5,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(920,62,5,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,62,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,82,1024,620) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,702,787,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(820,702,15,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,702,69,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,702,15,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,702,20,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,703,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,703,29,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,703,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,703,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(915,705,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,705,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,705,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,721,29,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,721,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,721,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,723,69,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,723,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,723,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,725,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,735,810,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(873,735,40,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(987,735,37,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,758,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(20,10,107,16) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(20,10,108,17) op=(20,10,108,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=1a data_len=26 [0x00 0x00 0x01 0x09 0x02 0x0e 0x03 0x09 0x04 0x09 0x02 0x04 0x03 0x09 0x04 0x09 0x05 0x04 0x06 0x09 0x04 0x09 0x06 0x04 0x06 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,41,204,16) color=0x004398) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(20,41,93,17) op=(20,41,93,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=39 data_len=24 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09 0x0d 0x05 0x0e 0x04 0x09 0x0b 0x0f 0x05 0x10 0x09 0x11 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,41,706,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(224,41,46,17) op=(224,41,46,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=39 data_len=12 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,41,84,16) color=0x004398) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(930,41,58,17) op=(930,41,58,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=39 data_len=16 [0x12 0x00 0x09 0x09 0x0f 0x05 0x0c 0x09 0x0f 0x05 0x13 0x09 0x0f 0x08 0x14 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(16,63,197,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(18,64,1,15) color=0x008c51) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,62,198,1) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,63,1,18) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(213,62,1,20) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,81,199,1) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(220,63,699,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,62,700,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(919,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,81,701,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(926,63,82,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,62,83,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1008,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,81,84,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,10,48,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(957,11,45,20) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(963,13,34,17) op=(963,13,34,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3c3 glyph_y=1d data_len=12 [0x15 0x00 0x16 0x08 0x14 0x03 0x0c 0x03 0x0b 0x05 0x09 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,10,46,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,12,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1002,10,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,31,48,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(787,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(788,703,30,20) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(794,705,19,17) op=(794,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=31a glyph_y=2d1 data_len=4 [0x17 0x00 0x18 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(835,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(836,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(842,705,12,17) op=(842,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=34a glyph_y=2d1 data_len=2 [0x17 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(877,704,23,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=36e glyph_y=2d1 data_len=2 [0x02 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,703,24,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,704,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(900,703,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,722,25,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,705,14,16) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=385 glyph_y=2d1 data_len=4 [0x19 0x00 0x03 0x05] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(930,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(931,703,23,20) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(937,705,12,17) op=(937,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a9 glyph_y=2d1 data_len=2 [0x1a 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(971,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(972,703,30,20) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(978,705,19,17) op=(978,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3d2 glyph_y=2d1 data_len=4 [0x1b 0x00 0x1a 0x07] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,63,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(811,736,60,20) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(817,738,49,17) op=(817,738,49,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=331 glyph_y=2f2 data_len=12 [0x1c 0x00 0x0f 0x07 0x0a 0x09 0x0f 0x09 0x10 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,61,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,737,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(871,735,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,756,63,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,735,74,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(914,736,71,20) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(920,738,60,17) op=(920,738,60,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=398 glyph_y=2f2 data_len=14 [0x1d 0x00 0x0f 0x0a 0x1e 0x09 0x1e 0x09 0x0b 0x09 0x13 0x09 0x0c 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,735,72,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,737,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(985,735,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,756,74,2) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 3 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x04\x5d"                                                     //..] |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 1113 bytes |
// Recv done on RDP Wab Target (4) 1113 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x84\x4e\x08\x00\x00\x00\x24" //...h....p.N....$ |
/* 0010 */ "\x7f\xd4\xef\x1a\x94\xfb\xb8\x2b\x6f\xcd\xfe\x66\x38\x37\xaa\x3d" //.......+o..f87.= |
/* 0020 */ "\xbe\x08\x75\x85\xbc\x60\x3c\x05\x0f\x16\xd3\x23\xf6\xc6\xd9\x64" //..u..`<....#...d |
/* 0030 */ "\x44\x95\xc9\xd5\x88\xbc\x88\x43\x13\x7b\xc0\xe2\x9a\xef\x3b\xae" //D......C.{....;. |
/* 0040 */ "\x17\x5a\x48\xc2\xff\x95\xdb\xb5\x9d\x23\xe8\x97\xc2\xfe\x46\x3d" //.ZH......#....F= |
/* 0050 */ "\x7d\x0c\xb4\xd0\x96\x47\xe6\x18\xa9\x8b\xbc\x89\x5e\xcc\xb9\x5d" //}....G......^..] |
/* 0060 */ "\xc3\x3d\xab\x27\x57\x70\xcf\xfe\x0d\x9c\x83\xce\xb5\x3f\x25\x86" //.=.'Wp.......?%. |
/* 0070 */ "\x8e\x44\x7e\x08\xba\x8a\xd4\x14\x0e\xe2\x7a\x69\xfb\x60\x20\x55" //.D~.......zi.` U |
/* 0080 */ "\xab\x6b\x37\xb9\xf3\x7b\x13\xf7\x63\x39\x8e\x71\xb0\xa5\x47\xdc" //.k7..{..c9.q..G. |
/* 0090 */ "\x1c\x7e\x7f\x73\xd3\x9c\xf4\x76\x32\xe3\x52\xa9\x4e\x79\x8e\xe8" //.~.s...v2.R.Ny.. |
/* 00a0 */ "\x4b\x44\x71\x34\xc7\x8a\xf5\x8f\x6b\xb9\x71\xc3\xb4\x39\x52\xd2" //KDq4....k.q..9R. |
/* 00b0 */ "\xe2\xf7\xc5\x0f\xaf\xcd\xf9\x38\xeb\x4e\x3f\xff\x65\x05\xb0\xcb" //.......8.N?.e... |
/* 00c0 */ "\xa8\x90\x31\x63\x5f\x55\xea\x48\xd0\xb0\x36\x2b\x00\x43\x4b\x9d" //..1c_U.H..6+.CK. |
/* 00d0 */ "\x5b\x45\xaf\x67\x50\x66\x31\xde\x90\x34\x4c\x18\x9b\x7c\x05\xd7" //[E.gPf1..4L..|.. |
/* 00e0 */ "\x73\x1b\xe4\x9a\x3d\x7b\xd3\x85\x5f\x6e\x8b\x3c\x38\x72\x73\x5e" //s...={.._n.<8rs^ |
/* 00f0 */ "\xb7\x93\xf1\xf9\x7c\x17\x17\x77\xdc\x77\xf6\x14\xdf\xee\x7d\xe6" //....|..w.w....}. |
/* 0100 */ "\x91\xf8\x1d\x05\x44\x1a\x76\x87\x56\xd7\x3d\x37\xac\x8c\xb0\x4e" //....D.v.V.=7...N |
/* 0110 */ "\x0e\xea\xfe\xf6\x7f\xc0\x58\xff\x60\x3b\x59\x90\x86\xb8\xe6\x19" //......X.`;Y..... |
/* 0120 */ "\xdb\x62\xbc\xdd\x37\x85\x42\x05\x7e\xf2\x57\xf1\x00\x84\x08\x12" //.b..7.B.~.W..... |
/* 0130 */ "\x47\x57\xa8\x2b\x6c\x85\x3d\x2e\x49\xec\xdb\xff\xe2\xf2\xdf\x00" //GW.+l.=.I....... |
/* 0140 */ "\x11\xc6\xd6\x91\xea\x7c\xea\x7c\x21\x68\xa3\xb9\xf6\x6c\x79\x35" //.....|.|!h...ly5 |
/* 0150 */ "\xaa\x43\x86\x91\x04\xf2\xe6\x3d\x3f\x92\x98\x07\x05\x11\x8d\x15" //.C.....=?....... |
/* 0160 */ "\x45\xbf\xf1\x27\x43\xf5\xa2\xf4\x72\x5d\xde\x80\x01\x49\xbb\x11" //E..'C...r]...I.. |
/* 0170 */ "\x56\x0a\x0f\xed\x44\x8b\x08\xd1\xc0\xcd\x5b\xc9\xca\x14\x5e\x1c" //V...D.....[...^. |
/* 0180 */ "\xc3\xb9\xce\x79\x46\x2d\xae\xdb\xf1\x7c\x0e\x3f\x3c\x5c\x4d\x25" //...yF-...|.?<.M% |
/* 0190 */ "\xa7\x28\xeb\xe6\x9a\xe2\xf3\x4c\x31\x42\x49\xab\xfc\xe1\x4b\x97" //.(.....L1BI...K. |
/* 01a0 */ "\x50\xb8\x88\x1f\x1a\x65\xa9\xeb\xab\x20\x89\x90\xa0\x11\xef\x93" //P....e... ...... |
/* 01b0 */ "\x6c\xa3\x95\xf6\xd1\xbf\x8e\xbb\x50\x11\x6c\x03\x65\xc2\x23\xec" //l.......P.l.e.#. |
/* 01c0 */ "\xdb\x9b\x44\x32\xc7\xbc\x11\xd3\xa7\x0c\x81\x8a\xef\x7e\x32\x88" //..D2.........~2. |
/* 01d0 */ "\xb7\x7e\x29\x02\x68\x05\xf9\x26\x42\x38\x33\xa4\xd0\xc1\x2a\x4c" //.~).h..&B83...*L |
/* 01e0 */ "\xab\x98\x12\x99\x7e\x7c\x38\x24\x2d\x62\x6e\xae\xe7\x66\x17\xcc" //....~|8$-bn..f.. |
/* 01f0 */ "\xd2\x21\x23\x6a\xb6\x7a\x6a\xe7\xb1\x5a\x6f\xaf\x44\x15\x0e\xee" //.!#j.zj..Zo.D... |
/* 0200 */ "\xe9\x78\xfc\xb6\x31\x35\x64\xf8\xc3\x06\xf8\x47\x2b\x15\x9a\x9b" //.x..15d....G+... |
/* 0210 */ "\x8a\x43\x95\xac\xa1\xdb\x3c\xa2\xf8\xf3\x19\x68\xbf\xd8\xc5\x59" //.C....<....h...Y |
/* 0220 */ "\x9f\x4e\xb8\xc4\x38\xc8\xaa\x79\xb1\x01\x00\xba\xc6\x2a\x43\x5a" //.N..8..y.....*CZ |
/* 0230 */ "\x02\x3b\x1d\x00\x4a\xa6\xe4\x65\x9a\x79\x32\x51\x46\x51\x06\x93" //.;..J..e.y2QFQ.. |
/* 0240 */ "\xed\xeb\x0e\x3f\x33\x7a\xf7\x41\x89\x22\xcb\x66\x73\xe5\x99\x82" //...?3z.A.".fs... |
/* 0250 */ "\x03\x93\x65\xdd\xec\xa8\x42\x13\x2b\x76\x34\xd2\xb8\x57\xbb\x7b" //..e...B.+v4..W.{ |
/* 0260 */ "\xb6\xfb\x2e\xdb\x92\x63\x6a\x47\xef\xcb\xf2\xed\xf3\x0e\xcf\xc4" //.....cjG........ |
/* 0270 */ "\xc4\xa6\x22\x5e\x57\xfe\x88\x28\xf1\xc0\x3a\x9d\x62\x2b\x52\xfa" //.."^W..(..:.b+R. |
/* 0280 */ "\x12\x7d\x2c\x1f\x2b\x4f\x20\x3c\xb5\x10\x78\x98\xcf\xe1\x6d\xa5" //.},.+O <..x...m. |
/* 0290 */ "\xfc\x6e\x3a\x24\x6e\xc6\xa1\xa4\xa8\xbd\x2b\x6f\x86\x5f\x9f\xc3" //.n:$n.....+o._.. |
/* 02a0 */ "\xc2\xfd\x96\x42\xa1\xd3\x82\xf0\xd3\xe6\x3a\xb7\xc5\x44\x0d\x8c" //...B......:..D.. |
/* 02b0 */ "\xd7\x9b\x44\x19\x10\xc0\x72\xff\xb4\xc6\x28\x88\x8a\x1a\x7c\xf3" //..D...r...(...|. |
/* 02c0 */ "\x87\x5f\x26\x52\xab\x6d\xc1\x0b\x7e\xe3\x77\x97\xac\xda\x9a\x29" //._&R.m..~.w....) |
/* 02d0 */ "\x2e\x61\x11\x05\x0d\x41\x05\x37\xc2\x24\x50\x3f\xe4\x13\xb6\x4c" //.a...A.7.$P?...L |
/* 02e0 */ "\x0e\x4a\x67\x0d\xd5\xf5\x3a\x6a\xc0\x62\x4f\x07\x1c\x65\x9e\xed" //.Jg...:j.bO..e.. |
/* 02f0 */ "\x0a\x0c\xea\x5f\xb8\x63\x75\x9f\x08\x36\xf5\xa7\xa8\xca\x5f\xca" //..._.cu..6...._. |
/* 0300 */ "\x5e\xda\x57\x55\x95\x8b\x19\xd9\x54\xf4\x63\x12\x92\x09\x92\x42" //^.WU....T.c....B |
/* 0310 */ "\xc9\xd9\x68\xcc\x7a\x16\xfd\x0d\xaf\xb3\xd9\xbb\x8d\xfe\x1a\xff" //..h.z........... |
/* 0320 */ "\x34\x69\xa7\xad\xa8\xf8\xc3\x4d\x82\xe4\xbf\x4c\xb3\x9c\x1d\xc9" //4i.....M...L.... |
/* 0330 */ "\x89\xcc\xf2\xb3\x5b\x71\x45\x96\xa7\x2e\x99\x44\x64\x82\xc5\x2d" //....[qE....Dd..- |
/* 0340 */ "\xcd\x0c\xe8\xba\xaa\x57\xbc\x19\xe9\xe3\xda\x16\x73\x76\x24\xcf" //.....W......sv$. |
/* 0350 */ "\x31\x57\x0c\x21\x56\xa3\x1c\x58\x0b\x16\x10\x77\x61\x75\x3c\x56" //1W.!V..X...wau<V |
/* 0360 */ "\x2f\xbe\x78\x07\xfe\x18\xc0\x98\x43\xe6\x18\x55\xf6\x6c\x1e\xa3" ///.x.....C..U.l.. |
/* 0370 */ "\xe2\x6d\xd3\x4e\x5f\x9b\x51\x05\x5d\xcd\x93\x3a\x94\xb7\x40\x85" //.m.N_.Q.]..:..@. |
/* 0380 */ "\xd3\x99\xea\x43\x4c\x66\xd0\x01\x79\x3a\xd2\xc7\xd2\xde\x9a\x0e" //...CLf..y:...... |
/* 0390 */ "\x41\x9b\x80\x3f\x8f\xb3\x1e\xd4\xfe\x26\xd4\x3b\x9e\x3f\xf5\x34" //A..?.....&.;.?.4 |
/* 03a0 */ "\x34\xac\x5a\xc2\x88\x55\x1d\x2a\x85\x6b\x69\x05\x8c\xf9\x4f\xd8" //4.Z..U.*.ki...O. |
/* 03b0 */ "\x61\xad\x2a\xd4\xf3\x0c\xb3\x62\x18\x1b\x81\x0d\x69\x35\xe7\x85" //a.*....b....i5.. |
/* 03c0 */ "\x3e\xfe\x70\x93\x50\x51\x57\x98\xe9\xa4\xc7\xd7\x37\xde\x10\xe4" //>.p.PQW.....7... |
/* 03d0 */ "\x7d\x08\x5f\x99\xdf\x8e\xb7\x92\xc5\x96\x05\xfc\x64\xd6\x0e\xf1" //}._.........d... |
/* 03e0 */ "\x70\xc8\x7b\xae\xd7\x98\x9a\xca\x63\xa8\xa5\xd4\x88\xc8\x7e\xd7" //p.{.....c.....~. |
/* 03f0 */ "\x53\x07\xc7\x51\xbe\x2c\x05\x46\x92\x54\xd9\xd9\x5c\x33\xa5\x94" //S..Q.,.F.T...3.. |
/* 0400 */ "\x69\xe5\xe1\x42\xdd\x9c\xad\xc5\xb9\x7e\x34\xc7\x2c\xee\x45\x45" //i..B.....~4.,.EE |
/* 0410 */ "\xa2\xea\x3e\xde\x92\x78\x7f\x4b\x29\x8f\xca\x54\x0f\xe0\x75\x1c" //..>..x.K)..T..u. |
/* 0420 */ "\x5a\x23\x7f\x7a\x2e\x13\x5c\xca\x0c\x39\x10\x41\x85\xaa\xdf\x9e" //Z#.z.....9.A.... |
/* 0430 */ "\x42\x5a\x9d\xab\x1d\xc7\x57\x00\xa7\xf4\x34\x69\xcb\xce\x0d\x66" //BZ....W...4i...f |
/* 0440 */ "\x78\xae\x61\xda\x66\xdf\xf8\xde\x38\xc6\x7e\x0b\x25\x96\x74\xe2" //x.a.f...8.~.%.t. |
/* 0450 */ "\x0e\x8c\xb7\xd4\xd4\x45\xe2\x2b\x5f"                             //.....E.+_ |
// Dump done on RDP Wab Target (4) 1113 bytes |
// LOOPING on PDUs: 1090 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_ORDERS |
// process_orders bpp=16 |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,0,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,10,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(127,10,829,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,10,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,26,956,7) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,26,20,7) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,33,1024,8) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,41,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,41,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,57,1024,5) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,62,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(214,62,5,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(920,62,5,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,62,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,82,1024,620) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,702,787,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(820,702,15,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,702,69,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,702,15,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,702,20,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,703,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,703,29,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,703,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,703,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(915,705,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,705,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,705,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,721,29,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,721,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,721,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,723,69,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,723,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,723,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,725,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,735,810,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(873,735,40,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(987,735,37,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,758,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(20,10,107,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(20,10,108,17) op=(20,10,108,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=1a data_len=26 [0x00 0x00 0x01 0x09 0x02 0x0e 0x03 0x09 0x04 0x09 0x02 0x04 0x03 0x09 0x04 0x09 0x05 0x04 0x06 0x09 0x04 0x09 0x06 0x04 0x06 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,41,204,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(20,41,93,17) op=(20,41,93,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=39 data_len=24 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09 0x0d 0x05 0x0e 0x04 0x09 0x0b 0x0f 0x05 0x10 0x09 0x11 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,41,706,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(224,41,46,17) op=(224,41,46,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=39 data_len=12 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,41,84,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(930,41,58,17) op=(930,41,58,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=39 data_len=16 [0x12 0x00 0x09 0x09 0x0f 0x05 0x0c 0x09 0x0f 0x05 0x13 0x09 0x0f 0x08 0x14 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(16,63,197,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(18,64,1,15) color=0x008c51) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,62,198,1) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,63,1,18) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(213,62,1,20) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,81,199,1) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(220,63,699,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,62,700,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(919,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,81,701,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(926,63,82,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,62,83,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1008,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,81,84,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,10,48,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(957,11,45,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(963,13,34,17) op=(963,13,34,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3c3 glyph_y=1d data_len=12 [0x15 0x00 0x16 0x08 0x14 0x03 0x0c 0x03 0x0b 0x05 0x09 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,10,46,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,12,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1002,10,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,31,48,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(787,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(788,703,30,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(794,705,19,17) op=(794,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=31a glyph_y=2d1 data_len=4 [0x17 0x00 0x18 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(835,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(836,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(842,705,12,17) op=(842,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=34a glyph_y=2d1 data_len=2 [0x17 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(877,704,23,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=36e glyph_y=2d1 data_len=2 [0x02 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,703,24,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,704,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(900,703,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,722,25,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,705,14,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=385 glyph_y=2d1 data_len=4 [0x19 0x00 0x03 0x05] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(930,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(931,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(937,705,12,17) op=(937,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a9 glyph_y=2d1 data_len=2 [0x1a 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(971,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(972,703,30,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(978,705,19,17) op=(978,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3d2 glyph_y=2d1 data_len=4 [0x1b 0x00 0x1a 0x07] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,63,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(811,736,60,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(817,738,49,17) op=(817,738,49,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=331 glyph_y=2f2 data_len=12 [0x1c 0x00 0x0f 0x07 0x0a 0x09 0x0f 0x09 0x10 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,61,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,737,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(871,735,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,756,63,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,735,74,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(914,736,71,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(920,738,60,17) op=(920,738,60,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=398 glyph_y=2f2 data_len=14 [0x1d 0x00 0x0f 0x0a 0x1e 0x09 0x1e 0x09 0x0b 0x09 0x13 0x09 0x0c 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,735,72,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,737,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(985,735,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,756,74,2) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 4 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x5e"                                                     //..^ |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 90 bytes |
// Recv done on RDP Wab Target (4) 90 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x50\x08\x00\x00\x00\x30\x76" //...h....pP....0v |
/* 0010 */ "\xed\xf1\x02\x34\xd7\xe2\xab\xcb\x96\x8f\x58\x9b\x35\xab\x8e\x31" //...4......X.5..1 |
/* 0020 */ "\x69\x2e\x56\x5d\xec\x77\xf7\x30\x94\x36\x89\x43\xe2\xa4\x90\x46" //i.V].w.0.6.C...F |
/* 0030 */ "\x04\x54\x92\xe1\x5e\x56\x61\x83\x41\x94\xcf\xc0\x0b\x36\x0a\x49" //.T..^Va.A....6.I |
/* 0040 */ "\x68\x36\xd3\xe9\x9b\x7b\x9f\x33\x84\xa7\x2d\x9b\x25\x1e\xc7\x11" //h6...{.3..-.%... |
/* 0050 */ "\x1d\xba\x8a\x3a\x24\x69\x0c\xbc\x17\x22"                         //...:$i..." |
// Dump done on RDP Wab Target (4) 90 bytes |
// LOOPING on PDUs: 68 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_ORDERS |
// process_orders bpp=16 |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(16,63,197,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,62,198,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(213,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,81,199,1) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 5 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x0d\xb8"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 3508 bytes |
// Recv done on RDP Wab Target (4) 3508 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x8d\xa9\x08\x00\x00\x00\x44" //...h....p......D |
/* 0010 */ "\x06\x57\x55\x72\xdb\xd6\xe2\x2a\xfd\xa2\xf5\x89\xff\x99\x6e\xf3" //.WUr...*......n. |
/* 0020 */ "\x70\x11\xff\x27\xbf\x6f\x9b\x65\x11\x18\x60\x50\x58\x37\xe2\xbe" //p..'.o.e..`PX7.. |
/* 0030 */ "\x1d\xf6\x92\x11\xa3\x2a\x1a\xa7\xcf\xba\x6c\x0c\xab\x68\xa3\x0f" //.....*....l..h.. |
/* 0040 */ "\x04\x0b\x5c\xc9\x24\x11\xdd\x97\x13\x50\xa4\xf8\xcf\x11\xbc\xac" //....$....P...... |
/* 0050 */ "\xe7\x15\x1a\xae\x27\x89\xbc\xaf\x41\x3c\xa6\xea\xab\xd5\x67\xac" //....'...A<....g. |
/* 0060 */ "\xcf\x08\x92\x58\xe8\xaf\xec\x14\x7a\x06\xc3\xab\x5a\x56\x54\xf7" //...X....z...ZVT. |
/* 0070 */ "\x68\x28\x90\x35\xc9\x1e\xd6\xda\xfa\xb2\x98\x88\x8b\xa9\x9e\xf2" //h(.5............ |
/* 0080 */ "\xbc\x99\x52\x93\x01\x08\xca\x55\xd5\xda\x99\xb6\xeb\xe3\x4b\x5e" //..R....U......K^ |
/* 0090 */ "\x6c\xab\x4b\xd8\x12\x60\x14\x34\xf3\xa4\xf2\x91\x43\x63\x40\xee" //l.K..`.4....Cc@. |
/* 00a0 */ "\xc6\x64\x3c\x99\xc9\xde\x10\x92\x42\x57\xdf\x38\xd7\x6a\xa0\xd2" //.d<.....BW.8.j.. |
/* 00b0 */ "\x09\x9f\x30\x5b\xf8\x99\xa9\x4d\xa5\xf7\x0b\xca\xe1\x78\x4d\xef" //..0[...M.....xM. |
/* 00c0 */ "\x1f\x39\xe0\x6c\x84\x8d\xc5\x2c\x51\x6f\xba\xc1\xc8\xda\x5e\x80" //.9.l...,Qo....^. |
/* 00d0 */ "\xfb\x16\xa2\x55\xd8\xac\x32\x94\xa1\x19\xdc\x72\xb5\xa0\x2b\x64" //...U..2....r..+d |
/* 00e0 */ "\xd6\x31\xd6\x34\x8d\x6b\x6e\x91\xd6\x4f\x65\xcf\x08\xd0\xd6\x4d" //.1.4.kn..Oe....M |
/* 00f0 */ "\x96\x67\x21\x64\xaf\xe9\x90\xc4\xe8\xcd\xc2\x25\xdf\x61\xc8\x70" //.g!d.......%.a.p |
/* 0100 */ "\x69\xce\x20\x33\x00\x80\x9a\x3f\xc0\xc2\x77\xce\xaf\xa6\x84\x7a" //i. 3...?..w....z |
/* 0110 */ "\x5a\x10\xad\x67\x26\x6c\xf2\xcc\xb8\x58\x69\xb0\x65\xb0\xaf\xaf" //Z..g&l...Xi.e... |
/* 0120 */ "\xa1\xe2\x02\xa3\x79\x46\x7b\xd8\xfb\xcf\x25\x24\x03\x39\x08\xcd" //....yF{...%$.9.. |
/* 0130 */ "\x2a\xd1\xc3\xe8\x32\x12\xfb\x5a\x33\x99\x2c\x73\x93\xcc\x5c\xff" //*...2..Z3.,s.... |
/* 0140 */ "\xa2\xc2\xbe\xd8\xeb\x4e\x2f\x09\x37\xc5\x6a\xd5\xc9\x40\xfa\x33" //.....N/.7.j..@.3 |
/* 0150 */ "\x8a\x75\x29\xfd\x8b\xc8\xbd\x95\xed\x60\xb0\x49\x6c\xbc\x7c\x1d" //.u)......`.Il.|. |
/* 0160 */ "\x8a\x82\x25\x7f\x8a\x00\xb7\x6b\x5a\x07\x64\x76\xb9\x74\x96\x04" //..%....kZ.dv.t.. |
/* 0170 */ "\xa0\x3e\xee\x64\xcb\xc1\xb0\x39\xc8\x92\x96\xab\xbb\x6e\x40\xf1" //.>.d...9.....n@. |
/* 0180 */ "\xf3\x1d\x26\x56\xdb\x5b\xda\xd8\x0d\x5d\xdf\x6a\xd4\xaa\x10\x81" //..&V.[...].j.... |
/* 0190 */ "\x39\x5a\xad\x34\x12\x0b\x53\xb6\x35\x8f\x3b\x8d\x8e\xe5\xa5\xfb" //9Z.4..S.5.;..... |
/* 01a0 */ "\xd7\xef\xf2\x71\x50\x9c\xc5\xbb\x7e\xc9\x36\xdc\x1e\xe4\xee\x6f" //...qP...~.6....o |
/* 01b0 */ "\x28\x66\x09\xfa\xdc\x81\xff\xff\x5b\x9a\x26\x71\x95\xf7\xd3\x5b" //(f......[.&q...[ |
/* 01c0 */ "\xa5\x91\xce\x38\xa1\x68\xf3\x41\x4c\x18\xfb\x39\x3b\x33\x6f\xc9" //...8.h.AL..9;3o. |
/* 01d0 */ "\xd3\x8b\x56\xdb\x62\x80\xe5\xa1\x4e\x93\x48\x7e\x8b\xbd\x8e\x83" //..V.b...N.H~.... |
/* 01e0 */ "\x49\x50\x89\xdf\x0e\x6f\x12\x20\xe5\x09\x4a\xe4\xf7\x18\x0a\xad" //IP...o. ..J..... |
/* 01f0 */ "\xea\xe3\x9b\x9c\x24\x21\xa8\x4c\x4c\x99\x77\x39\x86\x08\xa6\xcb" //....$!.LL.w9.... |
/* 0200 */ "\x78\xdc\x80\x36\xd8\x50\xc8\x74\xae\x4e\xbe\xb7\x77\x83\x62\xf4" //x..6.P.t.N..w.b. |
/* 0210 */ "\xf6\xf6\x21\x71\x0a\x80\xc8\x8a\xb9\xed\xce\xe8\x5a\xa9\xe5\xe9" //..!q........Z... |
/* 0220 */ "\x55\x3f\x3b\x24\xf5\xcf\x72\xbf\x30\x83\x7e\x43\xea\x24\xc1\x65" //U?;$..r.0.~C.$.e |
/* 0230 */ "\xf2\x85\x1e\xad\x53\x5e\x0f\xde\x72\xfe\x25\xcc\xad\x85\x1f\x44" //....S^..r.%....D |
/* 0240 */ "\x61\x98\x14\x77\x56\xcd\x6d\xb7\x29\x0e\xfc\x7b\x64\x17\x78\x5e" //a..wV.m.)..{d.x^ |
/* 0250 */ "\x8d\xbf\x09\xa1\x55\x60\x08\xee\x25\x6a\xc4\xd9\xdf\x97\x4a\x98" //....U`..%j....J. |
/* 0260 */ "\xcd\xa1\x67\x7c\x60\x59\xfa\x49\x71\x22\x09\xd0\xed\x83\x4f\xc0" //..g|`Y.Iq"....O. |
/* 0270 */ "\xdd\x73\x58\x9a\x2e\xf9\xcc\x13\xda\xcc\xe5\x95\x6b\x0d\x53\x26" //.sX.........k.S& |
/* 0280 */ "\x96\xad\xc9\x37\x91\xfd\x98\x89\x74\x58\x12\x34\x30\x42\x11\xbe" //...7....tX.40B.. |
/* 0290 */ "\x51\xda\xa2\xbe\xca\x8d\xd8\x56\xac\x05\x2c\x79\x6c\xad\x53\x06" //Q......V..,yl.S. |
/* 02a0 */ "\xd7\xcf\xb3\x26\xff\x08\x1c\x18\x25\xf9\x6c\x27\xd4\xaf\xa8\x38" //...&....%.l'...8 |
/* 02b0 */ "\xc7\xb5\x45\x9e\xd9\x39\xca\x0a\xe0\x63\x69\x49\x79\xcc\xa5\x69" //..E..9...ciIy..i |
/* 02c0 */ "\x91\x69\xbf\xf3\x4c\xc7\x05\x47\xc1\xbf\xb5\x56\x4d\xe8\xb4\xbc" //.i..L..G...VM... |
/* 02d0 */ "\xc8\x9f\x08\x2e\xa7\xe1\x0d\x68\x76\x17\xaa\x85\x48\xda\x98\x21" //.......hv...H..! |
/* 02e0 */ "\x21\x1e\xf1\x00\x63\x69\xb1\x63\x14\x14\x65\x4e\x24\x6b\x81\xba" //!...ci.c..eN$k.. |
/* 02f0 */ "\x10\x83\x3f\x7c\xd3\xd0\xbb\x7e\xb5\xf2\x4e\x9f\x00\xc6\xe5\x41" //..?|...~..N....A |
/* 0300 */ "\xc4\x2c\x00\x0d\x86\xed\x39\x3f\xac\xf3\x78\x1d\xeb\x9d\x7f\x55" //.,....9?..x....U |
/* 0310 */ "\xbc\xd4\xac\xcd\xa3\x6d\x08\x53\xf4\xdd\xe3\xb8\x14\x47\x8c\x43" //.....m.S.....G.C |
/* 0320 */ "\x7e\x8d\x49\x8e\x8f\x5e\x89\xc9\x39\x94\xdb\x95\xbc\x2c\xbf\x6a" //~.I..^..9....,.j |
/* 0330 */ "\x33\x44\xfa\x30\x07\xb8\x60\x5b\x67\xf4\xe2\xd7\xa1\x47\xb4\x8e" //3D.0..`[g....G.. |
/* 0340 */ "\x0b\x03\x6d\xef\xb5\xcd\x2b\x41\x5d\x0a\xbc\xa4\x15\xa7\xcb\x36" //..m...+A]......6 |
/* 0350 */ "\xeb\xed\x4a\x43\x7d\xa2\xfd\x47\xa5\x9e\x87\x29\x20\x38\x3c\xd9" //..JC}..G...) 8<. |
/* 0360 */ "\x56\x69\x3b\x15\xfc\x3e\x1e\x27\x23\x9c\x7e\x17\x9a\x8d\x3e\xed" //Vi;..>.'#.~...>. |
/* 0370 */ "\x79\xa4\x3c\xdf\x37\x2e\xf3\x06\xcb\x9b\xd5\xbd\x3e\xbd\xf0\xc4" //y.<.7.......>... |
/* 0380 */ "\x12\xb6\x78\x63\x60\xd4\xb7\x0c\x83\xf5\x54\xb5\xaa\xeb\x45\x4b" //..xc`.....T...EK |
/* 0390 */ "\xb0\x6f\x39\x04\x14\x2f\xe5\x37\xe8\x35\xba\x61\x71\x19\xcc\xde" //.o9../.7.5.aq... |
/* 03a0 */ "\xd3\x9e\x92\xbf\xfe\x82\x92\x86\xc4\x89\x4d\x3d\xce\x34\xd8\xd7" //..........M=.4.. |
/* 03b0 */ "\x6b\x4f\xcd\x1e\x54\x9d\x18\x04\x6f\x9d\x6d\x68\x68\x32\xf6\x11" //kO..T...o.mhh2.. |
/* 03c0 */ "\xa5\x2a\xb5\xf7\x0a\x06\xdb\x25\x71\xf7\x16\xd0\xa8\x6b\xde\x81" //.*.....%q....k.. |
/* 03d0 */ "\x94\xcd\x37\xad\x23\x22\xc7\xeb\xad\xc5\x35\x9d\x70\xf2\x91\x1c" //..7.#"....5.p... |
/* 03e0 */ "\x34\x71\x3f\xbd\x6a\x51\xeb\x6b\x35\x45\x51\xb7\xb1\x54\x4c\x33" //4q?.jQ.k5EQ..TL3 |
/* 03f0 */ "\x7f\x4f\x68\xd9\x9b\xbe\xff\xab\x0d\x7b\xa2\xe9\xab\x12\xfe\x80" //.Oh......{...... |
/* 0400 */ "\xca\x69\x75\x99\x77\xac\x98\x8c\x3a\xd4\x52\x22\x2a\x8f\x7b\x6e" //.iu.w...:.R"*.{n |
/* 0410 */ "\x4b\x28\x56\x6a\xef\xd7\x78\x4f\x52\xcb\x1e\x64\xf3\xa2\xbe\x0d" //K(Vj..xOR..d.... |
/* 0420 */ "\xad\x3f\xb5\x36\x8d\x9f\x41\xde\x8f\x56\xba\x3b\x43\xaf\x8c\xc8" //.?.6..A..V.;C... |
/* 0430 */ "\x3b\x22\x86\xbe\x8b\xd2\x33\x6b\xd6\xd2\x87\xd3\xf6\x7c\x9a\x47" //;"....3k.....|.G |
/* 0440 */ "\x13\x53\x62\x89\x63\xb6\xa2\x81\x88\x28\x8f\xc3\x3d\x91\x19\x8a" //.Sb.c....(..=... |
/* 0450 */ "\xc8\x91\x64\x25\xed\xed\x41\xfa\x19\xba\x1b\x24\xc6\xa7\x45\xef" //..d%..A....$..E. |
/* 0460 */ "\xad\x5d\x4c\x1c\x9f\x63\x08\x19\x18\xaf\x50\xbe\xd8\x61\xc7\x6e" //.]L..c....P..a.n |
/* 0470 */ "\x09\x4b\xee\x1b\x6b\xbe\xbe\x1f\x7c\x48\x60\x45\x3c\xfb\x59\x6b" //.K..k...|H`E<.Yk |
/* 0480 */ "\x0b\xca\x84\x60\x0a\xe4\xc9\x11\x91\xc3\x2f\x51\x27\xb9\x26\x2c" //...`....../Q'.&, |
/* 0490 */ "\xda\x45\x36\xf5\xfd\xad\x7f\xd5\x91\x60\x7d\x9c\x7e\xf8\x5b\x9c" //.E6......`}.~.[. |
/* 04a0 */ "\xb9\xb1\xa7\x77\x95\x1c\x3a\x40\x6c\xdf\x76\x29\x8f\x77\xe9\x10" //...w..:@l.v).w.. |
/* 04b0 */ "\x4c\x2d\x56\xd5\x4e\x6a\x69\xb1\x18\xac\x2d\xa5\x65\x59\xb7\xd2" //L-V.Nji...-.eY.. |
/* 04c0 */ "\xcd\x72\x27\xad\x0a\x93\x74\x0e\xcc\xa5\x96\x79\x8d\x82\x4d\xe7" //.r'...t....y..M. |
/* 04d0 */ "\x56\x66\x58\x74\x54\x0a\xd1\x7b\x39\x79\x00\x3b\x7b\x8d\x3c\xa9" //VfXtT..{9y.;{.<. |
/* 04e0 */ "\x70\xd9\x83\x44\xe6\x46\x67\x87\x9b\x7a\x3c\x87\x51\x7e\x4e\xb5" //p..D.Fg..z<.Q~N. |
/* 04f0 */ "\xfe\x00\x86\x8f\x59\x07\xed\xc1\x63\x3b\xd6\x39\x3b\x97\x9c\xc5" //....Y...c;.9;... |
/* 0500 */ "\x00\x18\x52\xbd\x01\x52\xf1\xea\x60\x76\xb2\xef\x23\x0d\x2c\x50" //..R..R..`v..#.,P |
/* 0510 */ "\xeb\x72\x4c\xce\x87\x72\x64\xee\xcf\x04\x5b\xf9\x57\x03\x25\x97" //.rL..rd...[.W.%. |
/* 0520 */ "\xd1\x24\x40\x12\xa3\xa0\x62\xea\x9e\x0f\xde\xdb\x87\xbd\xca\x32" //.$@...b........2 |
/* 0530 */ "\x46\x34\x5e\x54\xcf\xcb\xd2\x1a\x2c\x5a\xbd\x24\x4c\xef\xf6\x12" //F4^T....,Z.$L... |
/* 0540 */ "\x9a\xb8\x57\x5b\x6c\x19\xb2\x91\x38\x00\x4f\x26\x5e\x03\x1d\xf5" //..W[l...8.O&^... |
/* 0550 */ "\x0c\x07\x74\x8d\xbc\xdc\xf8\xb3\x25\xec\xf6\xb4\x38\xff\xe8\x95" //..t.....%...8... |
/* 0560 */ "\x52\x40\x66\xf0\x22\xd8\x87\x33\x03\x3e\xb2\x32\xf8\x32\xa5\x12" //R@f."..3.>.2.2.. |
/* 0570 */ "\xe4\x5b\x24\x0e\x06\x78\x19\x7f\x4b\x50\x51\x15\x01\xb0\xf9\xf6" //.[$..x..KPQ..... |
/* 0580 */ "\x51\x41\x43\xfb\x36\x15\xee\xd1\x29\xff\x4f\x2c\x74\xf5\x07\xe8" //QAC.6...).O,t... |
/* 0590 */ "\xdd\xa9\x0f\x2d\x05\x90\x7b\x36\xca\xde\xc6\x50\xcb\x75\xcf\xcf" //...-..{6...P.u.. |
/* 05a0 */ "\x28\x79\x44\xf7\xbd\xda\x46\xd0\xbe\x1c\x98\xc5\x1a\x1e\xb0\x7f" //(yD...F......... |
/* 05b0 */ "\xa9\x82\x1b\xc8\x60\x15\x87\x86\xbd\x54\x3c\xc1\x25\xe6\xc0\xc1" //....`....T<.%... |
/* 05c0 */ "\x80\xc7\xfd\xb9\x0f\x8d\x31\x03\x40\x06\x34\x9a\x20\x12\x85\xfc" //......1.@.4. ... |
/* 05d0 */ "\x9a\x00\x34\xb8\x21\x83\x23\x18\x0a\xcd\x4e\x4d\x7e\x19\x40\x90" //..4.!.#...NM~.@. |
/* 05e0 */ "\xaa\xa8\x8e\xbe\xf2\x9d\x6e\x4d\x5c\x17\x3b\x91\xa6\x13\x29\x00" //......nM..;...). |
/* 05f0 */ "\xbb\x67\x6a\x23\xf7\x38\xb6\x42\xab\x7c\x5d\x06\x0c\x87\x93\x47" //.gj#.8.B.|]....G |
/* 0600 */ "\x2f\x34\xa4\x12\x67\xf1\xf0\x8d\x73\x17\x52\xf4\xdd\xfe\x3c\x3a" ///4..g...s.R...<: |
/* 0610 */ "\xf1\xd0\x42\x57\xbe\xa5\x96\xe1\x9d\x1e\x2e\xaa\xbe\x1f\x57\x09" //..BW..........W. |
/* 0620 */ "\xc0\x47\x1c\xed\x3b\x18\x2b\xbf\x0d\xc5\x20\x48\x01\xe6\xba\x50" //.G..;.+... H...P |
/* 0630 */ "\xae\x4e\xf4\x67\x1a\x19\x60\x3d\x3b\x06\x2e\x70\x75\x0b\x75\xad" //.N.g..`=;..pu.u. |
/* 0640 */ "\x20\xf3\x6f\x84\xcc\x6d\xac\x20\xfa\x53\x82\x3d\x53\x52\xf6\x5a" // .o..m. .S.=SR.Z |
/* 0650 */ "\xe6\x2e\xf3\x54\x00\xcd\xd3\x78\x77\x6d\x50\x12\xab\x15\x46\x1b" //...T...xwmP...F. |
/* 0660 */ "\xaa\x5b\x52\xd6\x7b\x39\xd9\xea\x10\xce\xaa\xe2\x0b\x0a\x61\x82" //.[R.{9........a. |
/* 0670 */ "\x84\xda\xf5\x39\x82\x27\x66\x19\xd3\x8d\xc5\xff\xe6\x4f\xd9\x0b" //...9.'f......O.. |
/* 0680 */ "\xa1\x5f\xc8\xdc\x0f\xed\x71\xd0\x14\xf2\x21\xbb\x84\x9e\xcc\x1d" //._....q...!..... |
/* 0690 */ "\x7c\x71\xed\xe0\xc4\x42\xe5\x00\x64\x91\x45\x64\x85\xd0\x9b\x84" //|q...B..d.Ed.... |
/* 06a0 */ "\x03\x18\xa3\xae\x27\x5a\x3b\x8d\xcb\xbd\xc9\x3a\x38\xbc\xc4\x59" //....'Z;....:8..Y |
/* 06b0 */ "\xdc\xa1\x8f\x2a\x3d\x06\xcf\x07\x1c\x6f\x26\xa6\x31\x7b\xeb\x19" //...*=....o&.1{.. |
/* 06c0 */ "\x24\x87\xdb\x9e\x3e\x85\xf2\x78\x1f\x93\xf3\x31\xbe\x7b\xd0\xe0" //$...>..x...1.{.. |
/* 06d0 */ "\xec\x5b\xe5\xd4\x2e\x6c\x81\x61\x1f\xbb\x91\xab\xe2\xbf\x4b\x1a" //.[...l.a......K. |
/* 06e0 */ "\xa2\x8b\xd5\xc5\xf7\xab\x63\x66\x93\x75\xea\x9b\xa7\x12\x93\x85" //......cf.u...... |
/* 06f0 */ "\xfe\xe5\xcd\x18\xe1\xc8\x6a\xdf\xaf\x16\x75\xd2\xc8\x47\xf5\xa0" //......j...u..G.. |
/* 0700 */ "\x5d\x08\x42\xa1\x04\x06\x68\x90\xa9\x0b\x38\xa5\xf5\xf0\x3e\x41" //].B...h...8...>A |
/* 0710 */ "\xb3\x48\x85\x89\xab\x33\x7b\xde\x4a\x52\xe7\x4b\x5a\x25\xbe\x5d" //.H...3{.JR.KZ%.] |
/* 0720 */ "\x7e\x28\xb7\x8c\x13\x53\xaf\x13\xa1\x0b\x09\x5f\xa6\x41\xd4\x62" //~(...S....._.A.b |
/* 0730 */ "\x4a\xac\x8f\x11\x9a\x9f\xd2\x78\x8a\x76\xd4\x04\xbb\x65\xc4\x21" //J......x.v...e.! |
/* 0740 */ "\x3b\x76\x78\x9e\xc1\xe5\x1c\x2f\xe3\x1a\x9d\xec\x6c\xf6\x59\xcf" //;vx..../....l.Y. |
/* 0750 */ "\x09\xe9\xd1\x51\x9c\xac\xab\xa9\xee\x95\x8f\xa9\xc5\x21\x25\x07" //...Q.........!%. |
/* 0760 */ "\xdf\xc3\x6a\x65\x80\x8a\x22\xcc\x20\xc0\x77\x2d\xca\xa7\x52\xb1" //..je..". .w-..R. |
/* 0770 */ "\x3d\xe5\x48\x64\x23\x25\x3e\x54\xa9\xba\x97\xaf\xb0\x96\xdc\xa6" //=.Hd#%>T........ |
/* 0780 */ "\x09\xb6\x1b\xed\xf6\x5d\xf3\x96\xd7\xb7\xd4\xec\x8e\xed\xcc\x2a" //.....].........* |
/* 0790 */ "\x71\x76\xb7\x3d\xe2\x3c\x5f\x2c\x63\x54\x97\x31\xdd\x09\xf1\x52" //qv.=.<_,cT.1...R |
/* 07a0 */ "\xa4\x9b\xe5\x6c\x07\x26\xe8\x3d\x32\x85\x27\x3b\x9c\xc9\x66\xbf" //...l.&.=2.';..f. |
/* 07b0 */ "\xa2\x8d\x69\x1f\x66\x84\x12\xad\x08\x86\x3a\x29\xb1\x81\x8a\xd5" //..i.f.....:).... |
/* 07c0 */ "\x54\x41\xbb\xe3\x4b\x10\x81\x78\x24\xff\x38\xe2\x7b\x1b\x9e\xc7" //TA..K..x$.8.{... |
/* 07d0 */ "\xc2\x94\x13\x2d\x57\xe9\xec\xf6\x68\xf1\x97\xab\xa4\x17\xe5\xe5" //...-W...h....... |
/* 07e0 */ "\x64\xa3\x8e\xb1\xf1\x6a\x73\x6e\xbf\x2d\xe9\x31\x7d\x2a\x9a\xbb" //d....jsn.-.1}*.. |
/* 07f0 */ "\xa4\x2c\xe8\x7a\xf5\x4e\x9d\x7e\x2b\x51\x75\x1e\x15\xba\xe3\x61" //.,.z.N.~+Qu....a |
/* 0800 */ "\x35\x67\xde\xa2\x1d\x37\xdf\xde\x77\x77\x3a\x1f\x2a\x49\xbe\xf7" //5g...7..ww:.*I.. |
/* 0810 */ "\xd3\x4f\x05\x9d\x87\x7c\xe2\x60\xcf\x3a\x2b\x32\x08\x89\x3e\x49" //.O...|.`.:+2..>I |
/* 0820 */ "\x09\x03\xac\x68\xf0\x65\x0b\x30\x80\x62\x45\xc9\xac\x8f\x8a\x63" //...h.e.0.bE....c |
/* 0830 */ "\x74\x86\x50\x62\x9f\xf4\xd2\x5f\x40\x87\xbf\xce\xac\x81\xda\x34" //t.Pb..._@......4 |
/* 0840 */ "\xb5\xd2\xcc\xad\x86\xf8\x37\x46\x19\x32\x00\x45\xdf\xe4\xe0\x5c" //......7F.2.E.... |
/* 0850 */ "\x64\x72\x85\xf5\x28\xa4\x8a\xe1\x7a\xd0\x33\xd0\xb2\xf2\x92\x36" //dr..(...z.3....6 |
/* 0860 */ "\xe4\xec\x16\x2b\xef\x60\xa7\xd0\x73\x4a\x91\x44\xcb\xff\x48\x34" //...+.`..sJ.D..H4 |
/* 0870 */ "\x06\xef\x04\x99\xe1\x05\xf5\xcd\xe1\x68\x87\xb7\x22\xd4\xa4\x0f" //.........h.."... |
/* 0880 */ "\x6f\xb3\x33\xc9\xd2\x6e\xc0\xa0\x29\x44\xb9\x1b\x56\x6c\xb1\xb4" //o.3..n..)D..Vl.. |
/* 0890 */ "\x32\xc2\xa7\x72\x2a\x66\x49\x7b\xe0\x68\x5f\x17\xc7\xcc\x6d\xb8" //2..r*fI{.h_...m. |
/* 08a0 */ "\xff\x6e\xcf\x14\xb3\x37\x89\x59\x98\x31\xec\xe2\xe7\x13\x8c\x3c" //.n...7.Y.1.....< |
/* 08b0 */ "\x41\x7e\x06\xf5\x2f\x49\xc1\xc2\x7d\xc4\x3f\x97\x15\xc0\xa9\xa1" //A~../I..}.?..... |
/* 08c0 */ "\x4f\x55\x20\xaa\x66\x15\x90\xe3\xa5\x71\xcb\x65\x05\x6a\x0f\xbe" //OU .f....q.e.j.. |
/* 08d0 */ "\x18\xef\x72\x7c\x5b\xfe\x17\x19\x1f\x9b\x1e\xf6\x80\xe4\x80\xfe" //..r|[........... |
/* 08e0 */ "\x06\x65\xa0\x1e\x6a\xd4\x0f\x43\xc7\xeb\x7a\xee\x69\x54\xcb\xf9" //.e..j..C..z.iT.. |
/* 08f0 */ "\x4d\xc2\xbf\x17\xf6\x61\xb8\x91\xe4\xc7\x6e\xec\x8a\xe6\x5c\x13" //M....a....n..... |
/* 0900 */ "\xb2\xd9\x62\xec\x1f\x88\xbb\x72\x4c\xf6\x83\xa8\x06\x8e\xe7\x20" //..b....rL......  |
/* 0910 */ "\x65\x1e\x3f\x9a\x69\x4a\x11\xea\x53\xbc\xd5\xf0\xd6\x91\xfe\x1b" //e.?.iJ..S....... |
/* 0920 */ "\x1e\x79\x07\xe3\x13\xc3\x9a\xf1\x8f\x69\xdd\x5c\x05\x14\x8d\x65" //.y.......i.....e |
/* 0930 */ "\x25\x53\x97\xe5\x67\x71\xc9\xd8\x1d\x5c\xba\xef\x70\x89\xe0\xae" //%S..gq......p... |
/* 0940 */ "\xbc\x28\x71\x22\x71\x2a\x02\x2c\x65\x8b\x6d\x8d\x84\x7a\x3a\x5b" //.(q"q*.,e.m..z:[ |
/* 0950 */ "\xbd\x2c\xbc\x56\x42\x40\x57\x72\x18\x9d\xd0\x7c\x8d\xf7\x11\x92" //.,.VB@Wr...|.... |
/* 0960 */ "\x8a\x73\xf9\x9e\x46\xb4\xb6\x59\x36\xc5\x50\x7c\x1e\xd2\x87\x1d" //.s..F..Y6.P|.... |
/* 0970 */ "\xce\x4e\x8d\xfa\x80\x40\x59\xf8\x03\x02\x56\x94\x34\xe1\x1b\xf1" //.N...@Y...V.4... |
/* 0980 */ "\xfc\x06\x35\xb6\xec\xce\x93\xd4\x51\xeb\xcd\xd1\x15\xf3\xa7\x88" //..5.....Q....... |
/* 0990 */ "\x4e\xfa\x5e\x45\x65\x4f\x20\x33\x27\x63\x79\xc6\xdb\x13\xf5\x88" //N.^EeO 3'cy..... |
/* 09a0 */ "\x2c\xf4\xb8\x3b\xaa\x43\x97\x1c\x76\x42\xf0\x2d\x6b\x75\xa3\x0a" //,..;.C..vB.-ku.. |
/* 09b0 */ "\x22\x86\xff\xbc\x5a\xc8\x0c\xe5\x08\x68\xbe\x92\xb2\x88\x1a\x1b" //"...Z....h...... |
/* 09c0 */ "\xbb\x54\xc8\x31\xaf\x3b\xbc\xf4\xcc\xa8\x75\x7d\x39\xe5\xa3\x7e" //.T.1.;....u}9..~ |
/* 09d0 */ "\xeb\x10\xa5\xec\x34\x70\x06\x12\xc6\xf8\xcf\x23\x66\xc1\xd6\x38" //....4p.....#f..8 |
/* 09e0 */ "\x49\x1d\x2f\x37\xe2\xa9\xe0\x7b\x5b\x38\xbd\x8e\x51\x7b\xb5\xc2" //I./7...{[8..Q{.. |
/* 09f0 */ "\xd2\x1f\xe5\x0f\xcf\xb7\xa5\xe5\x48\x65\x4f\xb9\x0c\x33\x63\x50" //........HeO..3cP |
/* 0a00 */ "\x66\x09\xa7\x7b\xe4\xf1\xe7\x23\xd1\x72\x97\xb8\x12\x34\x86\x43" //f..{...#.r...4.C |
/* 0a10 */ "\xcb\x22\x42\x41\x47\x37\x0c\x1f\x5f\xba\xbf\x39\x78\x0d\x78\xd5" //."BAG7.._..9x.x. |
/* 0a20 */ "\xb6\xe5\x12\x4d\x4c\x78\xc3\x6a\xf3\x64\x4b\x90\x64\x24\xfd\x9c" //...MLx.j.dK.d$.. |
/* 0a30 */ "\x6f\xac\x72\xbb\x18\x14\x79\x16\xc4\x5f\x68\x9a\xe3\x74\x1e\xed" //o.r...y.._h..t.. |
/* 0a40 */ "\xbc\x98\xe9\xae\xe0\x76\xd4\x59\x97\x52\xa9\x60\x89\x58\x00\x55" //.....v.Y.R.`.X.U |
/* 0a50 */ "\x54\xb6\x8c\x48\xe7\x5a\x1c\xa8\x51\xce\x32\x7d\xa7\x78\x4c\xbd" //T..H.Z..Q.2}.xL. |
/* 0a60 */ "\xd0\x27\x8b\xe7\x71\xef\xcc\xf4\x5e\x79\xce\xaf\x49\x3c\xdb\x1c" //.'..q...^y..I<.. |
/* 0a70 */ "\xd9\x91\x8e\x3b\xba\x43\x8a\xe8\x74\xfb\x26\xad\xc3\xd0\xa7\xf5" //...;.C..t.&..... |
/* 0a80 */ "\xff\x7d\x82\x46\x95\x9c\x6b\xd7\xce\x90\xd8\x14\xe1\xea\x71\xad" //.}.F..k.......q. |
/* 0a90 */ "\x83\x88\x0f\x34\x8f\x7b\x18\xf9\xc4\x36\x47\xfc\x31\xd9\x1f\xc7" //...4.{...6G.1... |
/* 0aa0 */ "\x15\xa1\xf4\xbd\xd2\xd5\x89\xe0\xc8\x13\x75\x37\xac\xed\x2f\x3a" //..........u7../: |
/* 0ab0 */ "\x86\xea\x56\xc1\xa7\xbe\x27\x4a\x65\x88\x9b\x6c\xb4\x3c\xbf\x98" //..V...'Je..l.<.. |
/* 0ac0 */ "\x6b\xcc\x71\xda\x82\x30\x91\x18\x18\xca\x2e\xb2\xad\xb6\xbb\x9b" //k.q..0.......... |
/* 0ad0 */ "\x43\xc2\x4b\x29\x92\xca\xf7\x1c\x71\x44\x81\x54\xa7\x57\xfa\x16" //C.K)....qD.T.W.. |
/* 0ae0 */ "\xc9\x0c\xd5\x56\x59\x3f\xe3\xd9\xc0\x03\xea\x25\x36\x54\x6d\xc4" //...VY?.....%6Tm. |
/* 0af0 */ "\x72\x42\x4a\x5b\x03\x0e\x12\xa0\x90\xd9\x64\xb4\x91\x4d\x0f\x75" //rBJ[......d..M.u |
/* 0b00 */ "\x05\xe8\x1a\x3e\x7b\x71\x54\xf0\xb0\xc4\xdd\x27\x85\x5f\x80\x66" //...>{qT....'._.f |
/* 0b10 */ "\x49\xfe\x0c\xa8\x53\xd2\x20\xbb\xbc\xe6\xe3\xca\x5c\x24\x2a\x64" //I...S. ......$*d |
/* 0b20 */ "\x14\x71\x9b\xdd\x09\xef\xe7\x50\x9c\x8a\x3c\xe6\x16\x63\x9f\xce" //.q.....P..<..c.. |
/* 0b30 */ "\x9d\x5f\xde\x07\x8f\x96\xfe\xe9\x56\x6e\xda\xe2\x82\x23\x92\xc3" //._......Vn...#.. |
/* 0b40 */ "\x47\xc2\xcd\x2b\x93\x1f\x77\x86\xbd\x3b\xb2\x38\xdd\xc1\xb2\x0e" //G..+..w..;.8.... |
/* 0b50 */ "\x8c\x43\xff\x20\xc3\x8e\x8f\x16\x3e\x17\x16\x42\x24\x46\xbd\xfe" //.C. ....>..B$F.. |
/* 0b60 */ "\x9d\x68\xaf\x6a\x4e\x6d\xd8\xd0\x39\xca\x3a\x2d\x92\xfc\x26\x71" //.h.jNm..9.:-..&q |
/* 0b70 */ "\x4a\x0f\xe1\x30\x91\xa5\x9c\xce\xb5\x6d\x12\x9c\x8d\x80\x91\xa5" //J..0.....m...... |
/* 0b80 */ "\x26\x8d\x1a\x91\x29\xdd\xad\x1c\x03\xfa\x55\x41\xec\x8a\x0c\x9f" //&...).....UA.... |
/* 0b90 */ "\x5d\x02\x86\x40\x38\x6f\x27\x61\x7f\x21\xe0\x8b\xfd\xb2\x8b\x86" //]..@8o'a.!...... |
/* 0ba0 */ "\x4e\xd3\x79\x60\xa5\x19\x1c\xf2\xc7\x19\xff\x50\xd9\x78\xc3\x09" //N.y`.......P.x.. |
/* 0bb0 */ "\xe8\xec\x74\xea\x1f\xfe\x9e\xe6\xa3\xa3\x11\x94\xaf\x00\xd7\x10" //..t............. |
/* 0bc0 */ "\x51\xeb\x1e\x6c\xfc\x34\x34\xdd\x96\x5e\x57\x4c\x3a\x13\x8d\xc2" //Q..l.44..^WL:... |
/* 0bd0 */ "\x27\x04\xb9\xb4\x13\x23\x31\xa1\x3c\xd1\x44\x5c\xec\x27\xad\x7e" //'....#1.<.D..'.~ |
/* 0be0 */ "\x2d\x9d\x09\x76\xf4\x5e\xf2\x14\xe0\xba\x7d\xa0\xb1\xdc\x44\xe8" //-..v.^....}...D. |
/* 0bf0 */ "\xff\x92\xef\xfb\x20\xcc\x56\x39\x86\x3a\x45\x94\x32\xe2\x47\xbf" //.... .V9.:E.2.G. |
/* 0c00 */ "\x36\x8b\x04\xe5\x61\x94\xb9\x71\xca\xdc\xd7\xf9\xf3\xe1\x07\xe5" //6...a..q........ |
/* 0c10 */ "\xec\xd4\x1b\x64\xb6\x07\x63\xde\x29\xe0\x0e\x08\x90\x11\xbe\x44" //...d..c.)......D |
/* 0c20 */ "\x78\xf3\x92\x04\x05\x7a\x19\x00\xa0\xb3\xbb\x4f\x1d\x81\x22\xc6" //x....z.....O..". |
/* 0c30 */ "\x00\x5f\xcc\xec\x3d\x58\xd4\xe0\x25\x4f\xa6\xbe\x6b\x4d\xc7\x9d" //._..=X..%O..kM.. |
/* 0c40 */ "\x55\x6e\x01\x9f\xb2\xb3\x80\x35\x46\x22\x3d\x85\x04\xba\xf5\x73" //Un.....5F"=....s |
/* 0c50 */ "\xb4\x2f\xe3\x72\xd0\xb7\xad\x9a\xa0\xa9\x77\xdc\x0a\x5d\x81\x05" //./.r......w..].. |
/* 0c60 */ "\xd2\x17\x9c\x36\x8c\x35\x0e\xaa\x78\x98\xb2\x16\x30\x4e\x9b\x6e" //...6.5..x...0N.n |
/* 0c70 */ "\xf0\x8d\x49\xa9\xf7\xf0\x80\xd3\x6a\xdc\xf6\xa4\x40\x52\x98\x11" //..I.....j...@R.. |
/* 0c80 */ "\xfe\xff\xe6\x3c\x0b\xa7\x62\x22\xb4\x1e\x2f\x24\x00\x16\x2e\x58" //...<..b"../$...X |
/* 0c90 */ "\x13\x96\x8a\xef\x25\xd9\x1a\x13\xd1\x30\x75\x81\xd8\xa7\x34\x27" //....%....0u...4' |
/* 0ca0 */ "\x82\x06\x41\x78\xcb\xfa\x50\x7d\xdc\x8e\xad\xdb\xf6\x36\x50\x52" //..Ax..P}.....6PR |
/* 0cb0 */ "\x03\x69\x11\x87\xbd\xfd\x48\x57\x0e\x07\x21\xc3\xe3\x03\x01\xbf" //.i....HW..!..... |
/* 0cc0 */ "\x39\x8c\x79\x34\x60\xa2\xee\x03\x6a\xdd\x0b\x26\x1d\x4a\xdb\x7f" //9.y4`...j..&.J.. |
/* 0cd0 */ "\x9e\x1f\x85\x9e\xcd\x8a\x8d\x76\x8c\xf0\x30\xeb\x32\xa5\x2a\x22" //.......v..0.2.*" |
/* 0ce0 */ "\x60\x38\xc4\x64\x3b\x65\x15\x26\x7c\xe0\x19\x37\xf2\x07\xdc\x7b" //`8.d;e.&|..7...{ |
/* 0cf0 */ "\xe3\xc2\x86\x50\xd3\xe9\x23\x22\x10\xc1\xba\xdd\xad\x0b\x63\xf4" //...P..#"......c. |
/* 0d00 */ "\x33\xa9\xb9\xba\x2e\x68\x93\x0f\xd1\x0f\x2d\xbd\x6a\xf1\x77\xe0" //3....h....-.j.w. |
/* 0d10 */ "\xc4\xc9\xcb\xca\x5a\x56\x15\x56\x40\x3c\x3a\xeb\x63\x54\x47\xc9" //....ZV.V@<:.cTG. |
/* 0d20 */ "\x9e\xe7\x8d\xf4\x01\x34\xe1\x81\x5d\x01\xb8\xf2\xd4\xaf\x7c\xb4" //.....4..].....|. |
/* 0d30 */ "\xfb\xc6\x90\x50\x9c\x4b\xff\x9f\x71\x62\x9a\x21\xdd\xd5\x73\x56" //...P.K..qb.!..sV |
/* 0d40 */ "\xf1\x7c\x87\xbc\xb2\xab\xdd\x2a\x4b\xea\x73\x14\x4f\x4c\xd7\xed" //.|.....*K.s.OL.. |
/* 0d50 */ "\x83\x7b\x29\xab\xfa\x3b\x81\x55\x47\x79\x95\x61\xb0\x1f\x20\x12" //.{)..;.UGy.a.. . |
/* 0d60 */ "\x20\x28\x32\x0e\x1e\x79\xef\xfa\x4d\x30\xde\x76\x6a\xec\x56\x83" // (2..y..M0.vj.V. |
/* 0d70 */ "\x3f\x5c\x24\xee\x68\xd5\xc6\x9e\x43\xa5\x14\x1c\x96\x1a\xf2\x8f" //?.$.h...C....... |
/* 0d80 */ "\xb5\x89\x55\xf2\x23\x6e\x2f\x32\xa7\xa3\x29\x65\xef\xb6\x91\x86" //..U.#n/2..)e.... |
/* 0d90 */ "\x96\x09\xac\x56\x33\x38\x3b\x5a\x32\xc0\x90\xe7\x80\xb1\x87\x8f" //...V38;Z2....... |
/* 0da0 */ "\xaa\x4f\xa0\xb7\xff\x71\xb3\xbb\x63\xe7\x79\xb1\xf7\xbc\xa8\xd8" //.O...q..c.y..... |
/* 0db0 */ "\xa9\x6c\x80\x6f"                                                 //.l.o |
// Dump done on RDP Wab Target (4) 3508 bytes |
// LOOPING on PDUs: 3485 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_ORDERS |
// process_orders bpp=16 |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,0,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,10,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(127,10,829,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,10,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,26,956,7) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,26,20,7) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,33,1024,8) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,41,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,41,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,57,1024,5) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,62,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(214,62,5,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(920,62,5,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,62,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,82,1024,5) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,87,15,140) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,87,15,140) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,227,1024,475) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,702,787,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(820,702,15,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,702,69,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,702,15,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,702,20,1) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,703,15,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,703,29,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,703,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,703,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(915,705,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,705,15,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,705,20,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,721,29,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,721,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,721,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(861,723,69,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,723,15,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1004,723,20,2) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,725,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,735,810,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(873,735,40,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(987,735,37,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,758,1024,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(20,10,107,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(20,10,108,17) op=(20,10,108,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=1a data_len=26 [0x00 0x00 0x01 0x09 0x02 0x0e 0x03 0x09 0x04 0x09 0x02 0x04 0x03 0x09 0x04 0x09 0x05 0x04 0x06 0x09 0x04 0x09 0x06 0x04 0x06 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,41,204,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(20,41,93,17) op=(20,41,93,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=39 data_len=24 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09 0x0d 0x05 0x0e 0x04 0x09 0x0b 0x0f 0x05 0x10 0x09 0x11 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,41,706,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(224,41,46,17) op=(224,41,46,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=39 data_len=12 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,41,84,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(930,41,58,17) op=(930,41,58,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=39 data_len=16 [0x12 0x00 0x09 0x09 0x0f 0x05 0x0c 0x09 0x0f 0x05 0x13 0x09 0x0f 0x08 0x14 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(16,63,197,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,62,198,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(213,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,81,199,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(220,63,699,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,62,700,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(919,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,81,701,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(926,63,82,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,62,83,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,63,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1008,62,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,81,84,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,10,48,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(957,11,45,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(963,13,34,17) op=(963,13,34,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3c3 glyph_y=1d data_len=12 [0x15 0x00 0x16 0x08 0x14 0x03 0x0c 0x03 0x0b 0x05 0x09 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,10,46,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,12,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1002,10,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(956,31,48,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,87,994,20) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,89,200,16) color=0x000273) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(20,90,86,17) op=(20,90,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=6a data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x21 0x0b 0x22 0x09 0x23 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,89,702,16) color=0x000273) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(224,90,263,17) op=(224,90,263,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=6a data_len=68 [0x24 0x00 0x25 0x09 0x26 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x27 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x20 0x0e 0x21 0x0b 0x22 0x09 0x23 0x08 0x28 0x09 0x11 0x05 0x09 0x09 0x16 0x05 0x28 0x03 0x05 0x05 0x29 0x09 0x04 0x09 0x02 0x04 0x29 0x09 0x23 0x09 0x2a 0x09 0x2b 0x05 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,89,80,16) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(930,90,31,17) op=(930,90,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=6a data_len=6 [0x2b 0x00 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,107,994,20) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,109,200,16) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(20,110,78,17) op=(20,110,78,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=7e data_len=18 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x00 0x0b 0x11 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,109,702,16) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(224,110,255,17) op=(224,110,255,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=7e data_len=66 [0x24 0x00 0x25 0x09 0x26 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x27 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x20 0x0e 0x00 0x0b 0x11 0x09 0x28 0x09 0x11 0x05 0x09 0x09 0x16 0x05 0x28 0x03 0x05 0x05 0x29 0x09 0x04 0x09 0x21 0x04 0x02 0x09 0x05 0x09 0x2a 0x09 0x2b 0x05 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,109,80,16) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(930,110,31,17) op=(930,110,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=7e data_len=6 [0x2b 0x00 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,127,994,20) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,129,200,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(20,130,77,17) op=(20,130,77,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=92 data_len=18 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x21 0x0b 0x22 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,129,702,16) color=0x00cebd) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(224,130,254,17) op=(224,130,254,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=92 data_len=64 [0x08 0x00 0x25 0x08 0x26 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x27 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x20 0x0e 0x21 0x0b 0x22 0x09 0x28 0x08 0x11 0x05 0x10 0x09 0x2d 0x09 0x28 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x2e 0x04 0x05 0x09 0x2a 0x09 0x2b 0x05 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,129,80,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(930,130,31,17) op=(930,130,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=92 data_len=6 [0x2b 0x00 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,147,994,20) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,149,200,16) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(20,150,86,17) op=(20,150,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=a6 data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x21 0x0b 0x22 0x09 0x06 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,149,702,16) color=0x00ef7e) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(224,150,189,17) op=(224,150,189,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=a6 data_len=44 [0x08 0x00 0x1e 0x08 0x2f 0x09 0x01 0x09 0x20 0x0e 0x21 0x0b 0x22 0x09 0x06 0x08 0x28 0x09 0x11 0x05 0x10 0x09 0x2d 0x09 0x28 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x29 0x04 0x03 0x09 0x2a 0x09 0x30 0x05 0x31 0x09 0x1d 0x0a] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,149,80,16) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(930,150,30,17) op=(930,150,30,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=a6 data_len=6 [0x30 0x00 0x31 0x09 0x1d 0x0a] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,167,994,20) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,169,200,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(20,170,107,17) op=(20,170,107,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=ba data_len=26 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x1e 0x07 0x0f 0x09 0x0c 0x09 0x0b 0x05 0x11 0x09 0x08 0x09 0x25 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,169,702,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(224,170,259,17) op=(224,170,259,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=ba data_len=62 [0x1e 0x00 0x0f 0x09 0x0c 0x09 0x0b 0x05 0x11 0x09 0x08 0x09 0x25 0x08 0x1f 0x09 0x08 0x07 0x13 0x08 0x13 0x08 0x0f 0x08 0x10 0x09 0x1e 0x09 0x0c 0x09 0x01 0x05 0x08 0x0e 0x11 0x08 0x11 0x09 0x1f 0x09 0x1e 0x07 0x0f 0x09 0x0c 0x09 0x0b 0x05 0x11 0x09 0x08 0x09 0x25 0x08 0x2a 0x09 0x24 0x05 0x12 0x09 0x12 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,169,80,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(930,170,28,17) op=(930,170,28,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=ba data_len=6 [0x24 0x00 0x12 0x09 0x12 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,187,994,20) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,189,200,16) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(20,190,86,17) op=(20,190,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=ce data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x21 0x0b 0x22 0x09 0x06 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,189,702,16) color=0x00ef7e) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(224,190,286,17) op=(224,190,286,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=ce data_len=72 [0x32 0x00 0x08 0x09 0x33 0x08 0x24 0x05 0x25 0x09 0x26 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x27 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x20 0x0e 0x21 0x0b 0x22 0x09 0x06 0x08 0x28 0x09 0x11 0x05 0x10 0x09 0x2d 0x09 0x28 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x29 0x04 0x03 0x09 0x2a 0x09 0x2b 0x05 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,189,80,16) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(930,190,31,17) op=(930,190,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=ce data_len=6 [0x2b 0x00 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,207,994,20) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,209,200,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(20,210,86,17) op=(20,210,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=14 glyph_y=e2 data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x21 0x0b 0x22 0x09 0x23 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,209,702,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(224,210,286,17) op=(224,210,286,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=e0 glyph_y=e2 data_len=72 [0x32 0x00 0x08 0x09 0x33 0x08 0x24 0x05 0x25 0x09 0x26 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x27 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x20 0x0e 0x21 0x0b 0x22 0x09 0x23 0x08 0x28 0x09 0x11 0x05 0x10 0x09 0x2d 0x09 0x28 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x23 0x04 0x23 0x09 0x2a 0x09 0x2b 0x05 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,209,80,16) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(930,210,31,17) op=(930,210,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a2 glyph_y=e2 data_len=6 [0x2b 0x00 0x2c 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(787,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(788,703,30,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(794,705,19,17) op=(794,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=31a glyph_y=2d1 data_len=4 [0x17 0x00 0x18 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(835,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(836,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(842,705,12,17) op=(842,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=34a glyph_y=2d1 data_len=2 [0x17 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(877,704,23,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=36e glyph_y=2d1 data_len=2 [0x02 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,703,24,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,704,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(900,703,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,722,25,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,705,14,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=385 glyph_y=2d1 data_len=4 [0x19 0x00 0x02 0x05] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(930,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(931,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(937,705,12,17) op=(937,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3a9 glyph_y=2d1 data_len=2 [0x1a 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(971,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(972,703,30,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(978,705,19,17) op=(978,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=3d2 glyph_y=2d1 data_len=4 [0x1b 0x00 0x1a 0x07] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,63,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(811,736,60,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(817,738,49,17) op=(817,738,49,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=331 glyph_y=2f2 data_len=12 [0x1c 0x00 0x0f 0x07 0x0a 0x09 0x0f 0x09 0x10 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,61,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,737,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(871,735,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,756,63,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,735,74,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(914,736,71,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(920,738,60,17) op=(920,738,60,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=398 glyph_y=2f2 data_len=14 [0x1d 0x00 0x0f 0x0a 0x1e 0x09 0x1e 0x09 0x0b 0x09 0x13 0x09 0x0c 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,735,72,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,737,2,21) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(985,735,2,23) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(913,756,74,2) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 6 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x73"                                                     //..s |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 111 bytes |
// Recv done on RDP Wab Target (4) 111 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x65\x08\x00\x00\x00\x83\xb3" //...h....pe...... |
/* 0010 */ "\x0d\xbb\x42\xb1\xc5\x85\xc0\x58\xbd\x45\x93\x28\x58\x07\x83\x90" //..B....X.E.(X... |
/* 0020 */ "\xb3\x37\xd1\x83\xf5\xf9\xec\x37\xf7\x94\x31\xfa\x54\xba\xbb\x15" //.7.....7..1.T... |
/* 0030 */ "\x1f\x60\xd5\xef\xbc\x82\xc5\x29\x0b\x93\x5b\xdd\xae\xf7\xb3\x64" //.`.....)..[....d |
/* 0040 */ "\x14\xc0\x66\xda\x20\x57\x5f\x78\x3e\x59\xb7\xa8\x0b\x2b\x4d\x04" //..f. W_x>Y...+M. |
/* 0050 */ "\x46\x9a\x9e\xe7\xf3\xc3\x5f\x41\x39\x60\x07\x3b\x3a\xa5\x82\xcf" //F....._A9`.;:... |
/* 0060 */ "\x6f\x2c\xf5\x0c\x60\xf1\x8b\xf4\x52\x9d\xc2\xf2\xf7\x04\x86"     //o,..`...R...... |
// Dump done on RDP Wab Target (4) 111 bytes |
// LOOPING on PDUs: 89 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_ORDERS |
// process_orders bpp=16 |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(877,704,23,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=36e glyph_y=2d1 data_len=2 [0x02 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,703,24,1) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,704,1,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(900,703,1,20) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(876,722,25,1) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 7 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x56"                                                     //..V |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 82 bytes |
// Recv done on RDP Wab Target (4) 82 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x48\x08\x00\x00\x00\x8a\x8d" //...h....pH...... |
/* 0010 */ "\x2b\x3c\x2c\xcd\xdc\x84\x51\xc5\xf5\x84\xf4\xb7\x6a\x33\x63\x19" //+<,...Q.....j3c. |
/* 0020 */ "\x27\x78\x21\xa9\xbe\x14\x77\xcf\x66\x67\x45\xdb\x1f\xb1\x01\x20" //'x!...w.fgE....  |
/* 0030 */ "\x92\xc0\x34\xeb\x57\x92\xf4\x1f\xb3\x4f\x9f\xbd\x82\x2b\xfd\x12" //..4.W....O...+.. |
/* 0040 */ "\x9a\x09\xfc\x45\xd2\xcd\x60\x34\x72\x4d\x60\x44\x3f\x42\x90\x6d" //...E..`4rM`D?B.m |
/* 0050 */ "\x8f\x5d"                                                         //.] |
// Dump done on RDP Wab Target (4) 82 bytes |
// LOOPING on PDUs: 60 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_UPDATE |
// RDP_UPDATE_ORDERS |
// process_orders bpp=16 |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(901,705,14,16) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=385 glyph_y=2d1 data_len=4 [0x19 0x00 0x02 0x05] |
// ======================================== |
// process_orders done |
// ===================> count = 8 |
// ~mod_rdp(): Recv bmp cache count  = 0 |
// ~mod_rdp(): Recv order count      = 432 |
// ~mod_rdp(): Recv bmp update count = 0 |
// Socket RDP Wab Target (4) : closing connection |
// RDP Wab Target (0): total_received=10034, total_sent=1807 |
} /* end indata */;

