const char outdata[] =
{
// connecting to 10.10.47.236:3389 |
// connection to 10.10.47.236:3389 succeeded : socket 4 |
// --------- CREATION OF MOD ------------------------ |
// Creation of new mod 'RDP' |
// ModRDPParams target_user="x" |
// ModRDPParams target_password="<hidden>" |
// ModRDPParams target_host="10.10.47.236" |
// ModRDPParams client_address="192.168.1.100" |
// ModRDPParams client_name="<null>" |
// ModRDPParams enable_tls=no |
// ModRDPParams enable_nla=no |
// ModRDPParams enable_krb=no |
// ModRDPParams enable_fastpath=no |
// ModRDPParams enable_mem3blt=no |
// ModRDPParams enable_bitmap_update=no |
// ModRDPParams enable_new_pointer=no |
// ModRDPParams enable_glyph_cache=no |
// ModRDPParams enable_wab_agent=no |
// ModRDPParams wab_agent_launch_timeout=0 |
// ModRDPParams wab_agent_on_launch_failure=0 |
// ModRDPParams wab_agent_keepalive_timeout=0 |
// ModRDPParams dsiable_clipboard_log=0x0 |
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
// ModRDPParams allow_channels=<none> |
// ModRDPParams deny_channels=<none> |
// ModRDPParams remote_program=no |
// ModRDPParams server_redirection_support=yes |
// ModRDPParams bogus_sc_net_size=yes |
// ModRDPParams client_device_announce_timeout=1000 |
// ModRDPParams proxy_managed_drives= |
// ModRDPParams verbose=0x000001FF |
// ModRDPParams cache_verbose=0x00000000 |
// RDP Extra orders="" |
// Remote RDP Server domain="" login="x" host="192-168-1-100" |
// Server key layout is 40c |
// Init with Redir_info: RedirectionInfo(valid=false, session_id=0, host='', username='', password='<null>', domain='', LoadBalanceInfoLength=0, dont_store_username=false, server_tsv_capable=false, smart_card_logon=false) |
// ServerRedirectionSupport=true |
// mod_rdp::Early TLS Security Exchange |
// RdpNego::NEGO_STATE_INITIAL |
// RdpNego::send_x224_connection_request_pdu |
// Send cookie: |
// /* 0000 */ "\x43\x6f\x6f\x6b\x69\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68" //Cookie: mstshash |
// /* 0010 */ "\x3d\x78\x0d\x0a"                                                 //=x.. |
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
// CS_Cluster: Server Redirection Supported |
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
/* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\xca\x01\x00" //................ |
/* 0010 */ "\x00\x00\x00\x00\x10\x00\x07\x00\x01\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0040 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0060 */ "\x04\xc0\x0c\x00\x09\x00\x00\x00\x00\x00\x00\x00\x02\xc0\x0c\x00" //................ |
/* 0070 */ "\x03\x00\x00\x00\x00\x00\x00\x00"                                 //........ |
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
// /* 0060 */ "\x00\x20\x00\x00\x00\xb8\x00\x00\x00\x6e\x4a\x14\xdd\xb1\xb1\xd6" //. .......nJ..... |
// /* 0070 */ "\xe4\x3b\x7c\x21\x16\xe4\xa6\x46\x1f\xcb\x4b\x8e\x0e\x04\x87\x84" //.;|!...F..K..... |
// /* 0080 */ "\xea\x37\x21\x7b\x76\x72\x97\x90\x02\x01\x00\x00\x00\x01\x00\x00" //.7!{vr.......... |
// /* 0090 */ "\x00\x01\x00\x00\x00\x06\x00\x5c\x00\x52\x53\x41\x31\x48\x00\x00" //.........RSA1H.. |
// /* 00a0 */ "\x00\x00\x02\x00\x00\x3f\x00\x00\x00\x01\x00\x01\x00\x67\xab\x0e" //.....?.......g.. |
// /* 00b0 */ "\x6a\x9f\xd6\x2b\xa3\x32\x2f\x41\xd1\xce\xee\x61\xc3\x76\x0b\x26" //j..+.2/A...a.v.& |
// /* 00c0 */ "\x11\x70\x48\x8a\x8d\x23\x81\x95\xa0\x39\xf7\x5b\xaa\x3e\xf1\xed" //.pH..#...9.[.>.. |
// /* 00d0 */ "\xb8\xc4\xee\xce\x5f\x6a\xf5\x43\xce\x5f\x60\xca\x6c\x06\x75\xae" //...._j.C._`.l.u. |
// /* 00e0 */ "\xc0\xd6\xa4\x0c\x92\xa4\xc6\x75\xea\x64\xb2\x50\x5b\x00\x00\x00" //.......u.d.P[... |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x08\x00\x48\x00\x6a\x41\xb1\x43\xcf\x47\x6f" //.......H.jA.C.Go |
// /* 0000 */ "\xf1\xe6\xcc\xa1\x72\x97\xd9\xe1\x85\x15\xb3\xc2\x39\xa0\xa6\x26" //....r.......9..& |
// /* 0010 */ "\x1a\xb6\x49\x01\xfa\xa6\xda\x60\xd7\x45\xf7\x2c\xee\xe4\x8e\x64" //..I....`.E.,...d |
// /* 0020 */ "\x2e\x37\x49\xf0\x4c\x94\x6f\x08\xf5\x63\x4c\x56\x29\x55\x5a\x63" //.7I.L.o..cLV)UZc |
// /* 0030 */ "\x41\x2c\x20\x65\x95\x99\xb1\x15\x7c\x00\x00\x00\x00\x00\x00\x00" //A, e....|....... |
// /* 0040 */ "\x00"                                                             //. |
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
// /* 0000 */ "\x02\xf0\x80\x2e\x00\x00\x20"                                     //......  |
// Dump done on RDP Wab Target (4) 7 bytes |
// cjrq[0] = 1033 |
// Sending on RDP Wab Target (4) 12 bytes |
/* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x20\x04\x09"                 //.......8. .. |
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
// /* 0000 */ "\x02\xf0\x80\x3e\x00\x00\x20\x04\x09\x04\x09"                     //...>.. .... |
// Dump done on RDP Wab Target (4) 11 bytes |
// cjcf[0] = 1033 |
// cjrq[1] = 1003 |
// Sending on RDP Wab Target (4) 12 bytes |
/* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x20\x03\xeb"                 //.......8. .. |
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
// /* 0000 */ "\x02\xf0\x80\x3e\x00\x00\x20\x03\xeb\x03\xeb"                     //...>.. .... |
// Dump done on RDP Wab Target (4) 11 bytes |
// cjcf[1] = 1003 |
// mod_rdp::RDP Security Commencement |
// mod_rdp::SecExchangePacket keylen=64 |
// send data request |
// Sending on RDP Wab Target (4) 94 bytes |
/* 0000 */ "\x03\x00\x00\x5e\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x50\x01\x00" //...^...d. ..pP.. |
/* 0010 */ "\x00\x00\x48\x00\x00\x00\xa4\xbe\x34\x2c\x91\x29\xd3\x65\x51\xf7" //..H.....4,.).eQ. |
/* 0020 */ "\xeb\xeb\xdf\x64\x31\x50\x0e\x1c\xb2\xfb\xbf\x02\x59\x28\x14\x0c" //...d1P......Y(.. |
/* 0030 */ "\x7b\x5e\x34\x95\x25\x19\xd6\xac\x5a\x5b\xf5\x90\xe5\x64\x10\x5e" //{^4.%...Z[...d.^ |
/* 0040 */ "\x77\x0c\xea\x1b\x4b\x49\x07\xd3\x02\xe6\xed\xee\xc6\x30\x2e\x2d" //w...KI.......0.- |
/* 0050 */ "\x8a\x53\x55\x1d\x28\x2a\x00\x00\x00\x00\x00\x00\x00\x00"         //.SU.(*........ |
// Sent dumped on RDP Wab Target (4) 94 bytes |
// send data request done |
// mod_rdp::Secure Settings Exchange |
// mod_rdp::send_client_info_pdu |
// send extended login info (RDP5-style) 1017b :x |
// Send data request InfoPacket |
// InfoPacket::CodePage 0 |
// InfoPacket::flags 0x1017b |
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
// send data request |
// Sending on RDP Wab Target (4) 339 bytes |
/* 0000 */ "\x03\x00\x01\x53\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x81\x44\x48" //...S...d. ..p.DH |
/* 0010 */ "\x00\x00\x00\x8e\xc0\x72\xeb\x87\x29\xd2\x6e\x31\xe3\x78\x37\xad" //.....r..).n1.x7. |
/* 0020 */ "\x9e\x8d\xf0\x76\x11\xb6\xb4\xc6\xc8\xa8\xc5\x8a\x46\x47\xb0\xd8" //...v........FG.. |
/* 0030 */ "\xeb\xad\xea\x71\x8f\x47\xd7\xbe\x8d\x26\xe4\xce\xa7\xd6\x41\x49" //...q.G...&....AI |
/* 0040 */ "\x60\x0e\xc7\x54\x58\xcc\x16\xd2\x9a\xa3\xbc\x36\x7a\xbd\x08\x9e" //`..TX......6z... |
/* 0050 */ "\xaa\x70\xf2\x83\x9d\x46\x4c\x02\x71\xbd\xe5\x4d\x82\x93\xd9\x3a" //.p...FL.q..M...: |
/* 0060 */ "\x20\x82\x55\xdf\x9c\x61\x15\xc0\x1b\x03\x6c\xc7\x01\x89\x81\x29" // .U..a....l....) |
/* 0070 */ "\x4b\xdb\xa8\x12\x94\x78\x2a\xd5\x90\x1b\x13\x97\xa0\x01\xbc\xf7" //K....x*......... |
/* 0080 */ "\xb4\x84\x3c\x0f\xa3\x66\xbe\x8c\x09\x32\x94\x11\x59\x90\x85\x06" //..<..f...2..Y... |
/* 0090 */ "\x59\x4e\xd5\x54\xa8\xba\x39\x01\xef\x16\x78\x82\x90\x53\x38\xcf" //YN.T..9...x..S8. |
/* 00a0 */ "\x6e\x78\xab\x49\x67\x34\x87\x63\x7c\xb3\xa7\x97\xb8\xdf\x7e\x03" //nx.Ig4.c|.....~. |
/* 00b0 */ "\x27\xd1\xfd\x04\xfa\x3f\x1c\x52\x3f\x47\x8d\x41\x9f\xb0\x6c\xd3" //'....?.R?G.A..l. |
/* 00c0 */ "\xbb\x36\xb7\x42\xad\x29\x40\x03\xd2\x61\x71\x1d\x81\x26\xd6\x1b" //.6.B.)@..aq..&.. |
/* 00d0 */ "\xe7\xe6\x15\xdb\xb2\x90\x66\x78\x00\xb8\x7d\xe9\x62\x56\x3a\x6c" //......fx..}.bV:l |
/* 00e0 */ "\x8d\x11\xb0\x22\xd7\xf5\xaa\x13\x89\xe4\x90\x2b\xc5\xe9\xe3\x70" //...".......+...p |
/* 00f0 */ "\x2c\xd6\xda\xf5\x4f\xcc\x49\x7c\xb3\x03\x8d\xa9\x4f\x6f\x72\xb7" //,...O.I|....Oor. |
/* 0000 */ "\x8d\xb1\xf8\xd8\xa5\x6a\x69\xcb\xcb\x80\xec\xb5\xf2\x29\xab\xe9" //.....ji......).. |
/* 0010 */ "\x48\x25\xbc\xdf\x32\xa1\xb8\xdd\x68\x4e\xa5\x50\xb7\x25\xf4\x35" //H%..2...hN.P.%.5 |
/* 0020 */ "\x50\x7e\x7b\xd7\xfb\x8d\xc5\xd3\x7d\x37\x6b\x76\x8d\x67\x18\x16" //P~{.....}7kv.g.. |
/* 0030 */ "\x42\xa1\x8a\xee\x53\x5e\xc9\xae\x6c\xe7\x7d\x55\x81\x42\xbf\x57" //B...S^..l.}U.B.W |
/* 0040 */ "\x24\x61\x79\xbf\xfe\x71\x71\x27\xa2\x1c\x37\xd1\x87\xee\xd3\x9d" //$ay..qq'..7..... |
/* 0050 */ "\x0c\x18\x06"                                                     //... |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x81\x42\x80\x00\x00\x00\x01" //...h. ..p.B..... |
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
// /* 0000 */ "\xb1\xf0\x54\x3b\x5e\x3e\x6a\x71\xb4\xf7\x75\xc8\x16\x2f\x24\x00" //..T;^>jq..u../$. |
// /* 0010 */ "\xde\xe9\x82\x99\x5f\x33\x0b\xa9\xa6\x94\xaf\xcb\x11\xc3\xf2\xdb" //...._3.......... |
// /* 0020 */ "\x09\x42\x68\x29\x56\x58\x01\x56\xdb\x59\x03\x69\xdb\x7d\x37\x00" //.Bh)VX.V.Y.i.}7. |
// /* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x0e\x00\x0e\x00\x6d" //...............m |
// /* 0040 */ "\x69\x63\x72\x6f\x73\x6f\x66\x74\x2e\x63\x6f\x6d\x00"             //icrosoft.com. |
// Dump done on RDP Wab Target (4) 333 bytes |
// Rdp::License Request |
// send data request |
// Sending on RDP Wab Target (4) 163 bytes |
/* 0000 */ "\x03\x00\x00\xa3\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x80\x94\x80" //.......d. ..p... |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x14\x80\x00\x10\x00\xff\x02" //...h. ..p....... |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x81\x34\x08\x00\x00\x00\x96" //...h. ..p.4..... |
// /* 0010 */ "\xd8\xde\xdc\xab\x9a\x1e\xc1\xdb\xb2\xe3\x1c\x0f\xd6\x86\xba\x12" //................ |
// /* 0020 */ "\x8c\xde\x75\x8e\x85\xce\x7d\x43\xac\x25\xd2\xe0\xcc\xa1\xb9\xe2" //..u...}C.%...... |
// /* 0030 */ "\x27\xf6\xd3\x88\x44\x11\xee\xdc\xec\x83\xdd\x29\x2e\x1a\xcf\xac" //'...D......).... |
// /* 0040 */ "\xe4\x92\x16\x32\x89\x20\x69\xc2\x49\xcf\x13\xc6\x38\x17\x1c\x38" //...2. i.I...8..8 |
// /* 0050 */ "\x6e\x74\xc9\x9d\x04\x59\x1b\x70\xae\x1d\x0d\xbb\x4e\xc3\xaf\x4f" //nt...Y.p....N..O |
// /* 0060 */ "\x7f\x97\x27\x35\x84\xd8\x2c\x07\xd7\xe9\xa3\x00\xba\x7d\x86\x3c" //..'5..,......}.< |
// /* 0070 */ "\x49\x9b\x11\x6e\xde\x4e\xc6\x91\x57\xfd\x99\xa8\x40\x16\x27\xe7" //I..n.N..W...@.'. |
// /* 0080 */ "\x12\xe2\xdb\x5f\x46\x23\x6d\x79\x24\x48\xee\xf9\x84\x0e\x8c\xba" //..._F#my$H...... |
// /* 0090 */ "\xe7\xd5\x1f\x52\x3d\xc4\xe5\x78\x94\xc5\x8c\x3d\x38\xbd\x31\xb8" //...R=..x...=8.1. |
// /* 00a0 */ "\xa5\xd0\xce\xc7\x24\x56\x71\x70\xe0\xae\x25\x13\x5c\x1f\x1c\xcb" //....$Vqp..%..... |
// /* 00b0 */ "\x3e\x44\x8f\xda\xeb\x2d\x0a\xdc\xd4\xb6\x08\x9f\xdc\x28\xef\x0c" //>D...-.......(.. |
// /* 00c0 */ "\xd4\xff\xd1\x3b\x62\x20\x21\xc1\x9e\x84\x7e\x23\x3e\xc8\x64\x45" //...;b !...~#>.dE |
// /* 00d0 */ "\x37\xf0\x2f\xe2\x1a\x29\xe7\x17\x60\xe3\xea\xd5\xd2\x5d\x70\x80" //7./..)..`....]p. |
// /* 00e0 */ "\x4c\x07\x8b\x70\xc8\x34\xe5\x76\xc6\x9a\xd6\xfa\x0c\x32\x7f\x7a" //L..p.4.v.....2.z |
// /* 00f0 */ "\xfa\x3f\xd5\x84\x18\xe1\xb0\x2f\x8b\xe5\xd7\x5b\x89\x07\xf8\xf1" //.?...../...[.... |
// /* 0000 */ "\x75\xdf\x64\x31\x2b\x5c\x3d\x85\xe5\xf6\x63\xb6\x5e\x26\x93\xc8" //u.d1+.=...c.^&.. |
// /* 0010 */ "\x2c\x7e\x86\x23\xcc\x6a\xbe\x45\x90\x13\x0b\x72\x30\x3a\x72\x22" //,~.#.j.E...r0:r" |
// /* 0020 */ "\x4e\x0e\x05\x27\xca\x83\x53\x38\x28\x40\x05\x8a\xbf\x8d\x26\xda" //N..'..S8(@....&. |
// /* 0030 */ "\x31\xb2\x7c\x3c\x0b\x77\xea\x48\xb8\x40\xf2\xb3\x4b\x0d\xf9"     //1.|<.w.H.@..K.. |
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
// Unprocessed Capability Set is encountered. capabilitySetType=Font Capability Set(14) |
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
// Unprocessed Capability Set is encountered. capabilitySetType=Bitmap Cache Host Support Capability Set(18) |
// Unprocessed Capability Set is encountered. capabilitySetType=Color Table Cache Capability Set(10) |
// Unprocessed Capability Set is encountered. capabilitySetType=Pointer Capability Set(8) |
// Unprocessed Capability Set is encountered. capabilitySetType=Share Capability Set(9) |
// Received from server Input caps (88 bytes) |
// Input caps::inputFlags 0x29 |
// Input caps::pad2octetsA 0 |
// Input caps::keyboardLayout 0 |
// Input caps::keyboardType 0 |
// Input caps::keyboardSubType 0 |
// Input caps::keyboardFunctionKey 0 |
// Input caps::imeFileName 2319485328 |
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
// Input caps::inputFlags 0x1 |
// Input caps::pad2octetsA 0 |
// Input caps::keyboardLayout 1033 |
// Input caps::keyboardType 4 |
// Input caps::keyboardSubType 0 |
// Input caps::keyboardFunctionKey 12 |
// Input caps::imeFileName 2319288560 |
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
/* 0000 */ "\x03\x00\x01\xb4\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x81\xa5\x08" //.......d. ..p... |
/* 0010 */ "\x00\x00\x00\x4f\x2a\xa3\x7e\x08\x50\x3e\x04\xe4\x13\xb6\xa8\x3e" //...O*.~.P>.....> |
/* 0020 */ "\x2d\x7b\xc0\x5b\xf9\xea\x76\xe7\x75\x47\x79\x6b\x34\xee\xd7\x93" //-{.[..v.uGyk4... |
/* 0030 */ "\x7d\x30\xe3\x86\xca\x89\xee\xa1\xdc\x62\x42\x6c\xcd\x36\x97\x10" //}0.......bBl.6.. |
/* 0040 */ "\x33\xe9\xff\x82\xae\x63\xfb\x49\xf5\xdd\x4f\x35\xf3\x07\x1f\x85" //3....c.I..O5.... |
/* 0050 */ "\x74\xbe\x92\xb8\x61\xae\x53\xc6\x1d\xfd\x81\x3c\x2c\xb4\x5f\x21" //t...a.S....<,._! |
/* 0060 */ "\x27\x21\xae\x27\x5c\x3f\xfa\x8f\x47\x98\x95\x4d\x4d\x78\xcb\x64" //'!.'.?..G..MMx.d |
/* 0070 */ "\xb6\xcd\x81\x86\x04\x29\xf8\x81\x24\x91\x7c\x70\xd2\xdd\x18\x95" //.....)..$.|p.... |
/* 0080 */ "\xf8\x2d\x6f\xeb\x01\x96\x76\x3c\x23\x8f\x25\xf0\xb8\xca\xf5\x64" //.-o...v<#.%....d |
/* 0090 */ "\x52\x25\xed\xe0\x0f\x57\xfa\x1c\x06\x9b\x15\x81\x15\x28\x78\x9b" //R%...W.......(x. |
/* 00a0 */ "\xad\xa0\x18\x93\xc7\x0d\x1a\x87\xe9\x0e\x3d\x90\xe7\xc6\x8e\xb2" //..........=..... |
/* 00b0 */ "\x0e\xe8\xa6\xd3\x87\xe3\x22\xcb\xb3\x68\x11\xb2\x3b\x58\x7f\x3f" //......"..h..;X.? |
/* 00c0 */ "\xf5\x57\xb0\xf6\x66\x75\x9c\xa1\xb6\x28\x53\x04\x69\x82\x3b\x36" //.W..fu...(S.i.;6 |
/* 00d0 */ "\xc7\xc1\x64\xc3\x3d\xc9\xb0\x4d\x16\x24\x39\x2d\x9b\x8d\xf7\x5e" //..d.=..M.$9-...^ |
/* 00e0 */ "\x50\xa3\xbc\xeb\xb0\x5b\xf5\xc8\x44\xac\x50\xeb\x35\x07\x4a\xe3" //P....[..D.P.5.J. |
/* 00f0 */ "\xec\x85\x31\x8a\xf9\x17\x96\xd2\x64\xca\x6f\x6c\xb7\x97\x45\x79" //..1.....d.ol..Ey |
/* 0000 */ "\xbd\x55\xa1\xc2\x61\x5e\x49\x94\xea\xe4\xf2\xda\x95\x6d\x65\xd7" //.U..a^I......me. |
/* 0010 */ "\x69\x43\xee\x23\xe0\x74\x96\xd4\x09\x68\x98\xcf\x04\xb9\xc6\xff" //iC.#.t...h...... |
/* 0020 */ "\x4d\x82\x2c\x69\x33\x2b\x40\xbf\x54\xa6\xa4\x17\x8d\x90\x9b\xb7" //M.,i3+@.T....... |
/* 0030 */ "\x92\xd0\xeb\x8d\xc0\xcb\xfa\xb7\xe0\x14\xef\xd4\xd8\xd8\x56\x09" //..............V. |
/* 0040 */ "\x43\x84\xea\xf0\x2a\x41\x73\x0a\x11\x3e\x70\x25\x70\xb8\x6a\x9c" //C...*As..>p%p.j. |
/* 0050 */ "\x26\x67\xb6\x15\x36\x0b\xdf\x05\xf3\x21\xdd\x0e\x5d\xf2\x3d\x8b" //&g..6....!..].=. |
/* 0060 */ "\x3b\xcc\x78\x73\x95\xc6\xaa\x0a\x62\x14\xc2\x8e\x8e\x93\x6d\xe2" //;.xs....b.....m. |
/* 0070 */ "\xe9\x6f\xfb\x34\x39\xfb\x6e\xff\x7d\xea\xe5\x67\x06\xb1\x76\xc4" //.o.49.n.}..g..v. |
/* 0080 */ "\x52\xef\xb1\x21\xdb\x2f\x35\xf0\x2a\xc5\xb6\x5e\x74\x80\x4d\x9e" //R..!./5.*..^t.M. |
/* 0090 */ "\x50\xe1\x26\x99\xee\x3b\xa8\x85\xb1\x7b\x25\xda\x73\x18\x44\x39" //P.&..;...{%.s.D9 |
/* 00a0 */ "\xe2\x79\x3c\xee\x66\xd9\x6c\xc3\xe1\x49\x76\xce\xcc\x03\x01\x91" //.y<.f.l..Iv..... |
/* 00b0 */ "\x1a\x71\x98\x53"                                                 //.q.S |
// Sent dumped on RDP Wab Target (4) 436 bytes |
// mod_rdp::send_confirm_active done |
// Waiting for answer to confirm active |
// mod_rdp::send_synchronise |
// Sending on RDP Wab Target (4) 48 bytes |
/* 0000 */ "\x03\x00\x00\x30\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x22\x08\x00" //...0...d. ..p".. |
/* 0010 */ "\x00\x00\xe9\xc4\x2f\x5a\x09\xbc\xb8\x9a\x8f\x5d\xd8\xba\x02\x00" //..../Z.....].... |
/* 0020 */ "\x88\x2a\xc5\xf0\xc2\x34\x86\x87\x8a\x62\x19\x16\x69\x60\x71\x42" //.*...4...b..i`qB |
// Sent dumped on RDP Wab Target (4) 48 bytes |
// mod_rdp::send_synchronise done |
// mod_rdp::send_control |
// Sending on RDP Wab Target (4) 52 bytes |
/* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x26\x08\x00" //...4...d. ..p&.. |
/* 0010 */ "\x00\x00\xf8\xd4\x61\xf8\x7a\xc9\x17\x51\x85\xf2\x16\x04\x35\x7f" //....a.z..Q....5. |
/* 0020 */ "\x3e\xf5\xe6\x04\x01\xc3\x19\xd1\xa3\x3a\xda\xc6\x53\x4c\x42\x5c" //>........:..SLB. |
/* 0030 */ "\x08\x5a\x43\x84"                                                 //.ZC. |
// Sent dumped on RDP Wab Target (4) 52 bytes |
// mod_rdp::send_control done |
// mod_rdp::send_control |
// Sending on RDP Wab Target (4) 52 bytes |
/* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x26\x08\x00" //...4...d. ..p&.. |
/* 0010 */ "\x00\x00\x8e\x9b\x5b\x89\xce\xef\x2c\xe5\x03\xbe\x1b\x8d\x33\xe6" //....[...,.....3. |
/* 0020 */ "\xc7\x48\xc9\x90\x80\x10\xa2\x81\xd2\xdb\x84\xff\x2b\x02\xef\x0e" //.H..........+... |
/* 0030 */ "\x48\x23\xd0\x0f"                                                 //H#.. |
// Sent dumped on RDP Wab Target (4) 52 bytes |
// mod_rdp::send_control done |
// use rdp5 |
// mod_rdp::send_fonts |
// Sending on RDP Wab Target (4) 52 bytes |
/* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x26\x08\x00" //...4...d. ..p&.. |
/* 0010 */ "\x00\x00\xc3\x8b\xf0\x06\x89\xd8\x02\x03\x17\xc9\xcd\xae\x67\xd9" //..............g. |
/* 0020 */ "\xdd\x4c\xe0\x47\x17\xf0\x4c\x76\x7d\x6d\xe5\xdc\xd5\xb9\xd9\x43" //.L.G..Lv}m.....C |
/* 0030 */ "\x14\x0c\xd4\x36"                                                 //...6 |
// Sent dumped on RDP Wab Target (4) 52 bytes |
// mod_rdp::send_fonts done |
// mod_rdp::send_input_slowpath |
// Sending on RDP Wab Target (4) 60 bytes |
/* 0000 */ "\x03\x00\x00\x3c\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x2e\x08\x00" //...<...d. ..p... |
/* 0010 */ "\x00\x00\xb3\xf0\x7b\x7d\x89\x95\x69\xd8\xc6\x76\x24\xfb\x07\x21" //....{}..i..v$..! |
/* 0020 */ "\x33\xbc\xe7\x20\x9c\xc5\xa1\xa5\x91\xc1\xd6\x85\x1e\x64\xe4\xe9" //3.. .........d.. |
/* 0030 */ "\x28\x41\x97\x6a\x93\x09\x2b\x11\x43\x0d\xac\x0b"                 //(A.j..+.C... |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x22\x08\x00\x00\x00\xe9\xc4" //...h. ..p"...... |
// /* 0010 */ "\x2f\x5a\x09\xbc\xb8\x9a\xc1\x1d\x96\xe5\x2b\xa0\x02\x4e\x46\x45" ///Z........+..NFE |
// /* 0020 */ "\xf4\x78\xa2\xa5\x6f\x9e\x1f\x59\x08\x0f\x34\x70"                 //.x..o..Y..4p |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x26\x08\x00\x00\x00\x3a\x47" //...h. ..p&....:G |
// /* 0010 */ "\x4f\xcf\xcb\x8d\x3b\x41\xb5\x69\x1a\x19\x80\x03\x92\x76\x06\x6f" //O...;A.i.....v.o |
// /* 0020 */ "\x92\x4a\xf4\xb9\x5b\xd4\xdc\xd3\x17\xfc\x14\xd2\xc2\xae\x66\x59" //.J..[.........fY |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x26\x08\x00\x00\x00\xa4\xbe" //...h. ..p&...... |
// /* 0010 */ "\x23\xe2\x7c\x15\xe9\xb2\x13\x1d\xa6\x15\xad\x8d\x5d\xd3\x56\xfd" //#.|.........].V. |
// /* 0020 */ "\xcd\x91\x97\x50\x7c\xbf\x31\x76\x9e\x9e\x32\xa8\x9d\xbf\x45\x1b" //...P|.1v..2...E. |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x80\xca\x08\x00\x00\x00\xa1" //...h. ..p....... |
// /* 0010 */ "\xfc\x30\x8f\x0f\xb6\x55\x87\x7b\xda\xd9\x90\x92\x05\xf7\x35\x45" //.0...U.{......5E |
// /* 0020 */ "\x96\xb2\x92\x5d\xa9\x6a\x16\x1e\x04\x0f\x4e\xaa\xa7\xeb\x58\xe8" //...].j....N...X. |
// /* 0030 */ "\xda\xeb\xc9\xac\x99\x44\x64\xf2\x51\xbf\x67\xec\x08\x75\x78\xd8" //.....Dd.Q.g..ux. |
// /* 0040 */ "\x15\x2f\x47\x06\xf2\x8e\x25\xbd\x19\xb1\xd6\x19\xc2\x29\x8b\x82" //./G...%......).. |
// /* 0050 */ "\x26\x49\x53\xc5\x8a\xfb\xcd\xb2\xcf\x6b\xec\x2e\x45\x8a\x9a\xc4" //&IS......k..E... |
// /* 0060 */ "\x17\x94\x9e\x1a\x01\x53\xb0\x2f\xe2\xaa\xe4\xdb\xff\x00\xea\x28" //.....S./.......( |
// /* 0070 */ "\xf8\x55\x1b\x78\x6e\x7c\xb2\xb3\x80\xb6\xe3\x9c\x10\x65\xf8\x04" //.U.xn|.......e.. |
// /* 0080 */ "\x66\xe9\xda\x66\x24\x73\x28\x01\xdc\xe4\xd3\x0d\x6f\xdf\x8c\x4d" //f..f$s(.....o..M |
// /* 0090 */ "\x43\x8d\xcc\xa0\xb6\x80\x49\xf7\x15\x3a\xc8\x05\x79\xf3\xc3\x17" //C.....I..:..y... |
// /* 00a0 */ "\x78\x2e\xbf\xb5\xe4\x56\xd2\x70\xf5\xfd\xa0\x7a\xac\x77\xb0\x8e" //x....V.p...z.w.. |
// /* 00b0 */ "\x81\x49\x08\xd3\xe0\x63\x53\x2e\x57\xce\x5d\x00\xf2\x3b\x8f\x67" //.I...cS.W.]..;.g |
// /* 00c0 */ "\x36\xb4\xef\x53\xea\x72\xfd\xca\xe1\x17\xe5\xd9\x52\x43\xb7\x15" //6..S.r......RC.. |
// /* 00d0 */ "\x61\xed\x60\x82\xb2"                                             //a.`.. |
// Dump done on RDP Wab Target (4) 213 bytes |
// LOOPING on PDUs: 190 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_FONTMAP |
// mod_rdp::send_input_slowpath |
// Sending on RDP Wab Target (4) 60 bytes |
/* 0000 */ "\x03\x00\x00\x3c\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x2e\x08\x00" //...<...d. ..p... |
/* 0010 */ "\x00\x00\xd2\xbc\x42\xae\xf4\xa1\x77\xd4\x66\x8e\x45\x9d\xee\x06" //....B...w.f.E... |
/* 0020 */ "\xf4\xc8\xf5\x84\x73\x8a\xe1\x79\xe0\x95\xe9\xbc\x26\x4d\x4a\x8b" //....s..y....&MJ. |
/* 0030 */ "\x81\xd9\x1e\xdd\xc4\x0c\x8e\x8d\xfb\x3f\xa2\x2d"                 //.........?.- |
// Sent dumped on RDP Wab Target (4) 60 bytes |
// mod_rdp::send_input_slowpath done |
// enable_wab_agent=no wab_agent_launch_timeout=0 wab_agent_on_launch_failure=0 |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x22\x08\x00\x00\x00\x5a\xda" //...h. ..p"....Z. |
// /* 0010 */ "\x3b\x7a\x03\xf7\x25\x6a\x2d\xab\xf7\xc9\xf0\xad\xf7\xc7\x63\x8b" //;z..%j-.......c. |
// /* 0020 */ "\x79\xde\xa5\x1c\x81\x50\xdc\x25\xb2\x6e\x66\xe4"                 //y....P.%.nf. |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x84\x6e\x08\x00\x00\x00\x95" //...h. ..p.n..... |
// /* 0010 */ "\x41\x1d\xe0\x4b\x6b\x0f\x1f\x27\x8d\x93\xd8\x43\xc3\xaa\x60\x14" //A..Kk..'...C..`. |
// /* 0020 */ "\xf2\xa4\xd4\xd9\x45\xfb\x2f\x23\x03\x7d\x67\x25\x15\xb3\x70\xf1" //....E./#.}g%..p. |
// /* 0030 */ "\xa4\x23\xfd\xaf\x3d\x6d\xba\xb9\x34\xe0\x5c\xbb\x43\xee\xb6\xcf" //.#..=m..4...C... |
// /* 0040 */ "\xf1\x0f\x03\xf5\x27\x39\xf2\xd7\x4b\xde\xe2\x52\x28\x62\xc4\x66" //....'9..K..R(b.f |
// /* 0050 */ "\x0d\x96\xa6\x11\x47\x5b\x3a\x46\x16\x1f\xb0\x25\xce\x69\x12\x7c" //....G[:F...%.i.| |
// /* 0060 */ "\xde\x1c\xfd\x3c\xe6\xef\x40\x66\x21\x28\xde\xd3\x10\x58\x79\x1a" //...<..@f!(...Xy. |
// /* 0070 */ "\xd3\x29\x4f\xee\x93\x5b\xc3\xd1\xef\xcc\xce\x0c\x3d\x11\x40\x67" //.)O..[......=.@g |
// /* 0080 */ "\x84\x91\xbf\x60\x0e\x40\xcb\x15\x46\x6d\x02\x9e\x66\x75\x57\xb1" //...`.@..Fm..fuW. |
// /* 0090 */ "\x80\x40\xe8\x83\x7d\xd6\x49\xdf\x92\xb5\x78\xcc\xb1\xc8\x59\xbc" //.@..}.I...x...Y. |
// /* 00a0 */ "\x61\xe4\xd6\x1a\xc7\xe7\xc9\x1d\x34\x9f\xd3\x28\xc1\xf7\x25\x06" //a.......4..(..%. |
// /* 00b0 */ "\x1d\x00\x77\xe2\xed\xb9\x6d\x5a\xe6\x92\xa8\x70\xb9\x23\x93\xae" //..w...mZ...p.#.. |
// /* 00c0 */ "\x85\xd3\xbb\xad\x46\xf0\x51\x02\xf1\xb8\x4a\xf0\xff\x09\x31\xb9" //....F.Q...J...1. |
// /* 00d0 */ "\x96\xdb\x8c\x40\xef\x8e\x69\x96\x7d\xdf\x86\x25\x83\x0c\x6a\xbd" //...@..i.}..%..j. |
// /* 00e0 */ "\xf3\x3a\x64\xa4\x7e\x8a\x50\x3b\x04\x2f\x5b\xed\x7c\xeb\xca\xd6" //.:d.~.P;./[.|... |
// /* 00f0 */ "\x53\x66\x6e\x16\x3c\xc3\x90\xd8\x88\x5a\x3c\xca\x1c\x28\x30\x23" //Sfn.<....Z<..(0# |
// /* 0000 */ "\xa2\x6d\x0c\xc4\xd6\x18\xfd\x65\xac\xa5\x22\xd2\x78\xa0\xc9\x65" //.m.....e..".x..e |
// /* 0010 */ "\x55\x6c\xcf\x31\x8a\xa4\xce\x35\xeb\xe4\xc8\x64\xb6\x03\x05\x21" //Ul.1...5...d...! |
// /* 0020 */ "\xaa\xbd\xfd\xed\xe8\x2c\x58\xce\xae\x57\xae\x78\x59\x52\xc7\x86" //.....,X..W.xYR.. |
// /* 0030 */ "\xe9\x55\x28\xc0\x59\xfe\x8c\xba\x05\xb3\xfc\xa2\xfd\x95\xc2\xe9" //.U(.Y........... |
// /* 0040 */ "\x92\xaa\xdb\xd6\x20\xf3\xfb\xb0\xc4\x91\x22\x95\xba\xef\x2a\xbc" //.... ....."...*. |
// /* 0050 */ "\x88\x84\x47\x5a\x54\x5b\xef\xef\x06\x62\x7c\x8d\xe7\x6b\x5a\x22" //..GZT[...b|..kZ" |
// /* 0060 */ "\x5d\x15\x79\x45\x9d\x8d\x42\xc8\x14\xf0\x31\x6a\x78\xdf\xd1\x96" //].yE..B...1jx... |
// /* 0070 */ "\xc6\xd5\x9f\x4b\x4d\xea\x51\xec\xbb\x3f\x19\x94\x92\x1d\x01\x49" //...KM.Q..?.....I |
// /* 0080 */ "\x7a\x6a\x2a\xe2\x64\x9f\x90\x91\xae\x8a\xa7\x30\x88\xd6\xf0\x32" //zj*.d......0...2 |
// /* 0090 */ "\xfb\x04\xbc\x11\x9d\x49\xf2\x27\xee\x7f\x92\x13\x59\x50\x80\xf5" //.....I.'....YP.. |
// /* 00a0 */ "\xc7\xb4\xf4\x2d\xef\x7c\x62\x1f\x61\x48\x4a\xff\xe6\x62\x84\xca" //...-.|b.aHJ..b.. |
// /* 00b0 */ "\xd4\x67\x75\xea\xc4\x4d\xbb\xa0\xda\x4a\x31\x4f\x5e\xec\x54\x58" //.gu..M...J1O^.TX |
// /* 00c0 */ "\xb0\xef\xfb\x26\x94\x98\x9b\xab\x50\x5f\x2c\x47\x74\x53\x55\x27" //...&....P_,GtSU' |
// /* 00d0 */ "\x29\x1e\x92\x9f\xf4\xfd\xcd\xd4\x90\x23\x9d\xf0\x06\x0c\x33\x7f" //)........#....3. |
// /* 00e0 */ "\x47\xe2\xb9\x12\xb2\xfb\x68\xb9\x4f\x63\xa6\x24\x5a\x51\x21\x0a" //G.....h.Oc.$ZQ!. |
// /* 00f0 */ "\xc7\x5f\x15\x3f\xfe\x02\xae\xc8\x96\x5b\x6b\xa7\xe8\xf1\x70\x9e" //._.?.....[k...p. |
// /* 0000 */ "\xd5\xb0\x31\xab\x6d\x53\xc0\x9a\x23\x71\x25\x74\x08\x75\xe9\x6c" //..1.mS..#q%t.u.l |
// /* 0010 */ "\xb6\xfd\xc5\x7d\x46\x38\x85\x63\x5a\x64\x62\xa9\xd4\xf6\xf8\x92" //...}F8.cZdb..... |
// /* 0020 */ "\xa7\x85\x8e\x58\xc9\xa2\x70\x63\x9d\x8b\xb6\xe9\xf8\x29\x95\x51" //...X..pc.....).Q |
// /* 0030 */ "\xfd\x27\x30\x0f\x22\xc1\x71\x22\xf4\x99\xb0\x76\x72\x4e\xc0\x1f" //.'0.".q"...vrN.. |
// /* 0040 */ "\x62\x64\x68\xbf\x5b\x70\x2c\x62\x30\x3a\x6c\x16\x0d\xeb\xa4\xb2" //bdh.[p,b0:l..... |
// /* 0050 */ "\x95\x30\xc1\x5f\xd4\x77\xab\x11\x82\x58\x08\x0a\x31\x26\x0d\xc4" //.0._.w...X..1&.. |
// /* 0060 */ "\x8e\x24\xd2\x27\x8d\xe7\x00\x9f\xb7\xb0\xe3\xb5\xc3\x48\x9c\x8f" //.$.'.........H.. |
// /* 0070 */ "\xdc\xc8\xbf\xee\xfd\xd5\xd0\x7a\x41\x34\x21\xf3\x33\xfb\xf3\xa7" //.......zA4!.3... |
// /* 0080 */ "\x51\xf7\x71\x62\xe5\x78\xac\x20\x35\x64\xed\x9d\x9e\x5b\x4d\x8d" //Q.qb.x. 5d...[M. |
// /* 0090 */ "\xf5\x81\xdc\xa2\x18\x51\x4c\x49\xb2\xa5\x83\x92\x3d\x1e\xbc\x5b" //.....QLI....=..[ |
// /* 00a0 */ "\xd8\x5b\x2c\xc2\xef\x7b\x85\x96\x0c\xa8\xe1\xc6\xfb\x32\x50\xae" //.[,..{.......2P. |
// /* 00b0 */ "\x93\x44\x7c\xf6\x5e\x4e\xe3\x28\x22\x15\xd7\xbe\x58\x72\x2a\x81" //.D|.^N.("...Xr*. |
// /* 00c0 */ "\xfc\x29\xb4\x72\x30\xfc\xa5\x2c\x4a\x49\xe9\x54\x5a\x77\x0e\x07" //.).r0..,JI.TZw.. |
// /* 00d0 */ "\xa9\x68\x7f\x33\x1b\x7e\xd4\x98\xbf\xe3\xf3\x1d\x44\xca\x2a\x2f" //.h.3.~......D.*/ |
// /* 00e0 */ "\xea\xf5\xcc\xf0\x7d\x9a\xef\x6e\xee\xf3\xc8\x90\x8f\x90\x59\x24" //....}..n......Y$ |
// /* 00f0 */ "\x36\x00\xd8\x21\xe2\x14\x66\x4f\xed\x53\x9d\x37\xc2\x82\x3c\x29" //6..!..fO.S.7..<) |
// /* 0000 */ "\x13\x00\x7f\xac\xf4\x07\x00\x14\x8d\x5f\x34\xea\xac\x91\xbd\x81" //........._4..... |
// /* 0010 */ "\x5d\xa6\x1f\x68\xa5\x0c\x1c\x62\x36\xea\x8b\x53\x10\xb6\xc8\xdf" //]..h...b6..S.... |
// /* 0020 */ "\x0e\x2f\x7d\xc1\xb6\x10\x56\x81\x55\xfb\x46\x76\xe7\x2f\x11\x9c" //./}...V.U.Fv./.. |
// /* 0030 */ "\xbb\xda\x97\xbe\x2c\x78\x2a\x0a\xaf\x8f\xa9\x28\xee\xf7\x48\xdb" //....,x*....(..H. |
// /* 0040 */ "\xdc\xd5\x89\xc0\xc9\x70\xf4\xf0\x69\xbc\x0c\xb1\x62\x06\xce\xd7" //.....p..i...b... |
// /* 0050 */ "\xef\x40\x33\x36\xb4\x57\xca\xa5\xeb\xf5\x23\x31\xfa\xb4\xef\x40" //.@36.W....#1...@ |
// /* 0060 */ "\x50\xee\x4e\x7c\x56\x52\xe5\x19\xd1\xb3\x46\x5d\xce\x89\x07\xb5" //P.N|VR....F].... |
// /* 0070 */ "\x89\x42\x1f\x88\x13\x87\xd6\x47\x22\x0d\x8e\xc3\x4f\xaf\x10\x96" //.B.....G"...O... |
// /* 0080 */ "\xcb\x88\xbe\xfe\xdf\x49\x3c\xc9\xee\x97\x7f\xc9\x21\x3a\x52\xe8" //.....I<.....!:R. |
// /* 0090 */ "\x5d\x70\x0c\xac\x5c\xff\x8a\xcd\x55\xd4\x3b\xb1\x83\x4d\x9e\x64" //]p......U.;..M.d |
// /* 00a0 */ "\xc9\x34\xbb\x9b\xdd\x7a\x54\x72\x2a\xcd\xe2\x98\x12\x4a\xf6\x0e" //.4...zTr*....J.. |
// /* 00b0 */ "\x05\x50\x54\x6e\x05\x8f\x7b\x75\x87\x2e\xce\xa3\xd5\x5f\x1e\x15" //.PTn..{u....._.. |
// /* 00c0 */ "\x27\x89\xc8\x82\x1c\x1e\x20\x9d\x43\x18\xd8\xff\x37\xd6\xb0\x3a" //'..... .C...7..: |
// /* 00d0 */ "\x81\x3e\x71\x85\xef\xcb\xa4\x21\x90\xdb\xde\xa4\xa6\xb1\x13\x82" //.>q....!........ |
// /* 00e0 */ "\xce\xb1\x2d\xa1\x42\x79\xca\x6c\x39\xe6\xfb\x86\x47\xb0\xa4\x70" //..-.By.l9...G..p |
// /* 00f0 */ "\x2a\x16\xe7\x5e\xc5\xac\x3c\x5a\x4b\xd0\x18\x57\x8c\x59\x60\x1b" //*..^..<ZK..W.Y`. |
// /* 0000 */ "\x00\x16\xcd\x3c\x8e\x32\x56\x85\x07\x11\x7a\x3d\xff\x04\x1d\x31" //...<.2V...z=...1 |
// /* 0010 */ "\x66\xce\x06\x1b\x19\x1a\xac\x47\x25\x5c\xaf\xd9\xea\x19\x89\xb3" //f......G%....... |
// /* 0020 */ "\xeb\x79\x60\x07\xa8\x84\x31\x26\x08\x46\x8d\xb4\x07\xab\xbe\x47" //.y`...1&.F.....G |
// /* 0030 */ "\x27\xe1\xbf\x68\x69\xdd\xe2\x8d\xf0\x4a\xd7\x62\x1f\xfe\x20\xde" //'..hi....J.b.. . |
// /* 0040 */ "\x65\x12\xab\x76\xc4\x1e\x19\xff\x68\x27\xf1\xb8\x0b\xba\xfb\xd8" //e..v....h'...... |
// /* 0050 */ "\x7c\xae\xca\x8d\xaa\x51\x04\x59\x9f\x77\xad\x12\x8a\x84\x88\x0a" //|....Q.Y.w...... |
// /* 0060 */ "\x3a\xbb\xfa\xa2\x18\x17\x36\x64\xd3\xd0\x5b\x93\xbb\x20\xb5\xa0" //:.....6d..[.. .. |
// /* 0070 */ "\x65\x0c\x54\x3c\xde\x66\xac\x39\x4a"                             //e.T<.f.9J |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x89\x9a\x08\x00\x00\x00\x82" //...h. ..p....... |
// /* 0010 */ "\xc6\x99\x40\xb8\x42\x13\xab\x91\x2b\x0b\x1e\x11\x4d\x52\x67\xcd" //..@.B...+...MRg. |
// /* 0020 */ "\x52\x73\x1d\x5a\x36\x53\x83\x9b\x6d\xb7\x6d\xe8\x68\xe8\x17\x7c" //Rs.Z6S..m.m.h..| |
// /* 0030 */ "\xa5\x14\xb8\x3f\x27\x7a\x77\xf4\x33\xac\x85\x3d\xb9\xf3\x44\x41" //...?'zw.3..=..DA |
// /* 0040 */ "\x92\x06\x6a\x73\xc5\x5b\x8f\x89\x7f\x2f\x4e\xe9\x9a\xec\xd5\x2d" //..js.[.../N....- |
// /* 0050 */ "\xff\x7c\xc6\xaa\xf2\x65\xa8\x07\xc0\x25\x68\x38\xdf\xe0\xb1\x9a" //.|...e...%h8.... |
// /* 0060 */ "\x2c\xbf\xa2\x52\x15\xae\xf4\x6d\xc7\x61\x77\xc8\xd3\xcc\x99\x1c" //,..R...m.aw..... |
// /* 0070 */ "\x25\xb3\xc7\x5e\x15\xb0\x7d\xb5\x38\xf4\x5b\x68\x67\xe9\xf2\x3e" //%..^..}.8.[hg..> |
// /* 0080 */ "\xc9\x1c\xab\xaf\x77\x41\x63\xd6\xac\xaf\x66\x91\x8b\xd6\xf4\xfe" //....wAc...f..... |
// /* 0090 */ "\x64\x92\xe0\x24\x6f\xe8\x5a\x46\x52\x76\x70\xc4\x75\x5c\x8c\xf3" //d..$o.ZFRvp.u... |
// /* 00a0 */ "\x4d\x11\x0a\x24\xe5\x57\x4a\x49\x26\x73\xbf\x91\x1f\xd5\xbd\x2b" //M..$.WJI&s.....+ |
// /* 00b0 */ "\xb6\xc7\xb3\xa9\xcb\x2a\xa9\xb2\x40\xb9\x60\xdc\x93\xd5\xc6\x93" //.....*..@.`..... |
// /* 00c0 */ "\x1c\x0e\x8a\x6c\xd5\xd3\x9a\x71\x0f\xe0\x60\x8f\xd4\xb1\xa5\x50" //...l...q..`....P |
// /* 00d0 */ "\x3c\xdd\xb1\x61\xee\xa1\x8d\x27\xd5\x19\xb7\x5b\x58\xfb\xb2\x20" //<..a...'...[X..  |
// /* 00e0 */ "\x52\x29\x65\x8a\xa5\xa4\xd0\x58\x25\xc9\x03\xf9\xe0\x85\xfd\xb3" //R)e....X%....... |
// /* 00f0 */ "\x58\x99\xb5\x48\xd3\x23\xfe\x95\x1a\x79\x7a\xff\xb8\x14\x0f\x16" //X..H.#...yz..... |
// /* 0000 */ "\x9e\x1c\x0a\x98\xfd\x71\xfb\xdb\xff\xc4\x44\x93\xb9\x41\xfc\xd1" //.....q....D..A.. |
// /* 0010 */ "\xfc\x2f\x38\x5e\x26\x21\xa1\x41\xb3\x92\x80\x4c\xdf\x5e\x02\xdd" //./8^&!.A...L.^.. |
// /* 0020 */ "\x37\x18\x25\x39\xed\x55\x16\x6d\x28\x02\x2b\x37\x50\xd2\xad\xf8" //7.%9.U.m(.+7P... |
// /* 0030 */ "\x20\x57\xf3\x9e\x53\xac\xe2\x3d\x51\x6e\xf6\xd2\x76\xf4\xbb\xfe" // W..S..=Qn..v... |
// /* 0040 */ "\x22\x5d\xe7\xce\x98\x44\x40\xd9\x78\x59\x1d\xaa\xb7\x41\x6b\x67" //"]...D@.xY...Akg |
// /* 0050 */ "\x27\x6a\xaf\xd2\xa7\x34\x0c\xbb\x0a\x6a\x8a\x52\xf9\x4f\xc8\x6b" //'j...4...j.R.O.k |
// /* 0060 */ "\x94\xfc\x0a\x0c\x69\x95\x94\x3d\x6b\x05\x3e\xb7\x46\xf5\x38\x5e" //....i..=k.>.F.8^ |
// /* 0070 */ "\x16\x18\xc6\x4f\x88\xbf\xfe\x46\xc1\xbc\x34\x3d\x41\x7a\xf9\x66" //...O...F..4=Az.f |
// /* 0080 */ "\xb6\x3f\x03\x94\xff\x4c\x01\x91\x87\x32\x19\xd8\xe2\xef\x63\x10" //.?...L...2....c. |
// /* 0090 */ "\x07\x79\x02\xb0\x02\x9a\x91\xa7\x3f\xe7\x50\xd9\x8c\x1e\x17\xdb" //.y......?.P..... |
// /* 00a0 */ "\x44\x71\x4e\x80\x34\x81\x0b\x5c\xc8\xed\x9e\x9d\x13\x32\x3a\x8c" //DqN.4........2:. |
// /* 00b0 */ "\x2f\x62\xda\x06\x22\x7b\xdf\x6a\xb1\xac\xd1\xff\xd2\x4c\x42\xbe" ///b.."{.j.....LB. |
// /* 00c0 */ "\xb3\xcf\x7e\x1e\x7e\xfe\x9d\x48\xb5\xd0\x7b\x7b\xc4\x53\xec\x56" //..~.~..H..{{.S.V |
// /* 00d0 */ "\x86\x10\x8a\x0c\xa6\x8a\xe0\xc5\x24\xab\x44\x02\x2a\x06\x5b\x58" //........$.D.*.[X |
// /* 00e0 */ "\x2a\x62\x48\xd7\x14\x34\xe0\x57\xa8\x08\x9c\xeb\xb1\x5d\x19\xaa" //*bH..4.W.....].. |
// /* 00f0 */ "\xfe\xef\x4f\x58\x33\x06\xf8\x04\xfb\xe4\xf0\x5b\xb5\xb0\x14\x57" //..OX3......[...W |
// /* 0000 */ "\x55\x92\x99\xba\x37\x33\x0b\x80\xb6\x0c\x1d\x7d\xf2\x86\xd2\x12" //U...73.....}.... |
// /* 0010 */ "\x67\xa7\xb0\x5b\x8a\xe4\xbc\xc8\x4f\x4c\xbc\x81\xf6\x0f\x71\xaa" //g..[....OL....q. |
// /* 0020 */ "\x15\xb9\xa5\x9f\xe3\x8b\xb8\xa4\x9f\xab\xe3\xe6\x8d\xc3\x97\x2a" //...............* |
// /* 0030 */ "\x00\x6c\x9a\xd9\x52\xc4\xf4\xf0\x9f\xfb\xdf\x7d\xa1\xa3\xeb\x27" //.l..R......}...' |
// /* 0040 */ "\x56\xf2\x1a\xb5\xc6\x49\xef\xe1\x25\x7b\xd5\xf0\xca\x8e\x8d\xfa" //V....I..%{...... |
// /* 0050 */ "\xe5\xeb\xec\x5c\xb7\xc0\x3a\x3f\x84\xf4\x4c\x53\x88\xf0\x00\x4b" //......:?..LS...K |
// /* 0060 */ "\x25\x47\x87\x64\x52\xdd\x23\x1e\xa9\x6e\x40\x9a\xa1\x36\x3a\x4d" //%G.dR.#..n@..6:M |
// /* 0070 */ "\x4f\xb7\xe8\x0d\xc6\x9f\x9a\x4f\xa9\xa7\x2a\x75\x6c\xa5\xeb\x8a" //O......O..*ul... |
// /* 0080 */ "\xa8\x54\x68\xbf\x18\x81\x63\xde\x9a\xba\x58\x89\xdd\xc8\xb5\x0c" //.Th...c...X..... |
// /* 0090 */ "\x8e\xbe\x9e\x96\x17\x29\x1d\xf7\xb8\xe6\x97\x44\xf4\x2a\x82\xb5" //.....).....D.*.. |
// /* 00a0 */ "\xf4\x20\x77\x28\x6b\x47\x88\x24\x94\x4f\xeb\x15\x06\xa6\x67\x6e" //. w(kG.$.O....gn |
// /* 00b0 */ "\xd6\x66\x7e\x66\x2d\xe0\xbd\xc7\x86\xd4\x2f\x8e\x39\xd8\x25\x35" //.f~f-...../.9.%5 |
// /* 00c0 */ "\x0e\x5b\x8c\x8e\x8a\xee\x56\xb9\x75\x97\xfc\x92\x01\xa3\x26\x1c" //.[....V.u.....&. |
// /* 00d0 */ "\x7a\xf0\x14\x51\x1e\xdc\xb1\xde\x0b\xc8\x89\xec\x3a\x97\xac\x19" //z..Q........:... |
// /* 00e0 */ "\x6c\x30\x6c\xfe\x00\x61\xef\xc6\x46\x1a\xb3\x2f\x49\x5b\xde\x40" //l0l..a..F../I[.@ |
// /* 00f0 */ "\x00\xb9\x34\x95\x43\xe1\x1d\x6a\x99\xea\x11\x90\xd4\x86\xbb\x98" //..4.C..j........ |
// /* 0000 */ "\xa5\x1b\x70\x37\x8e\xe9\x42\xb7\x6a\x26\x3e\x23\x43\x45\xa2\x0d" //..p7..B.j&>#CE.. |
// /* 0010 */ "\xbe\xf2\x2c\xb9\x5a\x4a\xa7\xdd\x8c\x91\x5e\x9f\xa3\xe9\xde\xec" //..,.ZJ....^..... |
// /* 0020 */ "\x78\xe5\xf2\xb5\xa0\xd5\x4f\x69\xe4\x8a\xd6\x41\x90\x08\x5d\xea" //x.....Oi...A..]. |
// /* 0030 */ "\x24\x2e\x21\x4e\xde\x6f\xc5\x1c\xb2\xdf\x61\x9c\x41\x34\xd5\x63" //$.!N.o....a.A4.c |
// /* 0040 */ "\x31\x0e\xc7\xb8\x9d\xa7\xc6\xbc\x9b\x10\x9c\x24\xd3\x7c\x1a\x86" //1..........$.|.. |
// /* 0050 */ "\x96\xa8\x2f\x8a\x08\x90\x66\x83\x93\x4c\x70\x43\xf3\x03\x09\x0f" //../...f..LpC.... |
// /* 0060 */ "\x04\x08\xa8\xb0\x4b\xd2\x28\x8a\x15\xd0\xf6\x1c\x82\xf9\x3d\xb5" //....K.(.......=. |
// /* 0070 */ "\xc7\xab\xbf\x80\x61\xed\xf4\x35\x2f\x39\xa4\x95\x4c\xa1\xbd\x9d" //....a..5/9..L... |
// /* 0080 */ "\xea\xd7\x60\x61\xf6\xa9\x23\x59\xab\xcd\xc2\x4f\xbd\x45\xb2\x64" //..`a..#Y...O.E.d |
// /* 0090 */ "\x41\x25\xfd\xcd\xb6\xd0\x17\xf4\xbe\x36\xdc\x10\x30\x64\xa7\x50" //A%.......6..0d.P |
// /* 00a0 */ "\xce\xb2\x83\x68\x06\x49\x8d\x7a\xb1\xc7\x98\x2d\x1c\x88\xdd\xa7" //...h.I.z...-.... |
// /* 00b0 */ "\x0f\xb5\x50\x40\x7b\x46\x66\xc2\x54\xd7\xfb\xdd\xe9\x12\xfa\xa2" //..P@{Ff.T....... |
// /* 00c0 */ "\x6d\xc9\x17\x3e\xaa\x25\x62\xbe\x53\xd5\x9a\x03\xf5\x42\xdb\x5f" //m..>.%b.S....B._ |
// /* 00d0 */ "\x9f\x73\x4a\x2e\xd2\xbb\x7f\x70\xc8\xb0\xf3\x4e\xbd\x57\x87\x76" //.sJ....p...N.W.v |
// /* 00e0 */ "\xfb\x2b\x64\x1a\x2c\x7e\xd0\x1d\xd5\xf6\xd3\x4b\x1f\x31\x0f\xac" //.+d.,~.....K.1.. |
// /* 00f0 */ "\xae\xc5\x03\x09\x10\x82\x10\x68\xf0\x57\x13\x50\xf8\xb0\x09\x5b" //.......h.W.P...[ |
// /* 0000 */ "\x71\x88\xca\x0d\x55\xbe\xf1\x48\x25\xeb\x68\x84\xdf\xa4\xb8\x65" //q...U..H%.h....e |
// /* 0010 */ "\x75\x99\xa3\xf4\x32\x73\xa9\x5a\xc7\x9d\x85\x23\xfe\xe9\xce\x93" //u...2s.Z...#.... |
// /* 0020 */ "\xc2\x3d\xc9\x15\x9e\xd4\xc8\xb6\x3b\x03\x2a\x18\x18\x26\xd9\x7c" //.=......;.*..&.| |
// /* 0030 */ "\x0b\x0d\x4f\xe3\x7f\x94\x2f\xdc\x1f\xb8\x65\x53\x12\x66\xab\x01" //..O.../...eS.f.. |
// /* 0040 */ "\xee\x7d\xed\x15\x71\xeb\xfa\x82\x19\x8e\x94\xe7\x0f\x83\x82\xce" //.}..q........... |
// /* 0050 */ "\x17\x2f\x87\xe0\xed\x2c\x85\x4e\xdb\x14\xff\xbb\x01\x3e\xd7\x96" //./...,.N.....>.. |
// /* 0060 */ "\x0b\xc6\xf2\xc1\xc5\xe4\xdb\xeb\xd5\xca\x22\x53\x44\x23\x4f\x70" //.........."SD#Op |
// /* 0070 */ "\xe5\x29\x06\x1a\x61\xb3\x7e\xc5\xca\x73\x8b\xde\xd6\xdb\x54\x35" //.)..a.~..s....T5 |
// /* 0080 */ "\x15\x79\x19\x35\x0f\x96\x1b\x43\x82\x98\x9b\x34\x60\xbe\x76\xd9" //.y.5...C...4`.v. |
// /* 0090 */ "\x29\x2a\xb8\x3a\x1c\x97\x86\xd4\x9d\xec\x06\x75\x68\xa5\xb6\x12" //)*.:.......uh... |
// /* 00a0 */ "\xb7\x7f\x04\x3f\xbc\xdf\x75\x7d\x78\xd7\x2f\xe1\xae\x7d\x59\x45" //...?..u}x./..}YE |
// /* 00b0 */ "\x33\xdb\x49\xb6\xdc\x4c\x53\x60\x21\xa6\xba\x87\xf8\x54\x1d\xb9" //3.I..LS`!....T.. |
// /* 00c0 */ "\xf6\xa7\xd1\x89\xaa\x32\xd1\x38\x05\xb7\xa0\x2c\x4d\xd9\x5b\x2b" //.....2.8...,M.[+ |
// /* 00d0 */ "\xbf\xe3\x72\xec\xc9\x40\xeb\x35\xc1\xda\x68\x01\x01\xf0\x21\xc6" //..r..@.5..h...!. |
// /* 00e0 */ "\x6f\xf0\xd6\x1b\x96\xa1\xe3\x1d\x70\xc2\xba\xf6\xbe\xd8\x9b\x9a" //o.......p....... |
// /* 00f0 */ "\x08\xfe\x56\xff\x27\x3c\x79\x40\xf4\xf5\x3a\x44\xbb\xb4\xdc\x28" //..V.'<y@..:D...( |
// /* 0000 */ "\xa7\x2c\xe3\x86\xf6\xfc\x74\x31\xbb\xfa\x1c\xb0\x69\x60\x2e\xc4" //.,....t1....i`.. |
// /* 0010 */ "\x70\x90\x92\xa2\x85\xd1\x14\x10\x82\x46\x9f\x19\xe6\x66\xf6\x01" //p........F...f.. |
// /* 0020 */ "\x46\x94\x7e\x9b\x1b\x98\x95\x23\x42\x75\xd7\xef\x9f\x21\xcc\x7e" //F.~....#Bu...!.~ |
// /* 0030 */ "\xfb\xe0\xc5\x15\x8a\x05\x0b\xd3\x19\x82\x1a\x09\xc3\xbb\xc5\x58" //...............X |
// /* 0040 */ "\xf6\x9c\x81\x1c\x37\x8c\xbf\xb2\x61\xd0\x3e\xe4\x0d\x84\xb9\xde" //....7...a.>..... |
// /* 0050 */ "\x39\x27\x6e\xa3\x51\x80\xe6\xfb\xf5\xcc\x06\x5b\xb5\xcc\x43\x16" //9'n.Q......[..C. |
// /* 0060 */ "\x69\xa1\x74\xb3\x75\xdb\xec\xfa\xec\x62\xce\xf8\x07\xe2\x7b\xcc" //i.t.u....b....{. |
// /* 0070 */ "\x07\x64\x7d\x63\x34\x38\x34\x12\xae\xf2\x15\xf5\x7c\x5c\x29\x44" //.d}c484.....|.)D |
// /* 0080 */ "\xce\x5a\x0a\x52\x4e\xbe\x4c\x62\xd2\x04\x17\x11\x09\xf1\x2a\xca" //.Z.RN.Lb......*. |
// /* 0090 */ "\x20\x32\xce\x42\xe4\x97\xcf\xde\x5f\xff\xdf\xac\xd8\x06\xb4\x2c" // 2.B...._......, |
// /* 00a0 */ "\x4c\x93\x3f\x2d\xf9\x8b\x46\x05\x9e\x51\x0f\x48\xf9\xd1\x2f\xc0" //L.?-..F..Q.H../. |
// /* 00b0 */ "\x1b\x90\x56\xb6\x7c\x25\xdf\xcd\xb3\x67\xcc\x34\x13\x58\x6d\x95" //..V.|%...g.4.Xm. |
// /* 00c0 */ "\xbd\xac\xf3\x01\xde\x11\x75\xa1\x61\x72\x49\x76\xd6\x3c\x39\xa9" //......u.arIv.<9. |
// /* 00d0 */ "\x80\x91\x4f\x0d\xc1\xac\xd1\xb8\x9e\xc6\x69\x92\xe1\x56\x0c\xf0" //..O.......i..V.. |
// /* 00e0 */ "\xad\x28\x05\x5e\xbd\xd6\x15\xdb\xed\xe0\x9f\x78\xd3\xa5\x4d\xd0" //.(.^.......x..M. |
// /* 00f0 */ "\xe9\xad\x13\xff\xe1\x4a\x1f\xcb\x15\xb1\xbc\x54\x1b\x52\x14\xfc" //.....J.....T.R.. |
// /* 0000 */ "\x94\xba\x9b\xf3\x2d\xbd\x26\x3a\x55\xf0\x7e\xaf\x47\xc2\x09\xa8" //....-.&:U.~.G... |
// /* 0010 */ "\x7c\x37\x62\x57\x2b\xca\x16\x96\xf6\xde\xb4\x18\x94\x3b\xef\x5c" //|7bW+........;.. |
// /* 0020 */ "\x75\x94\x56\xfd\xb7\x69\xec\x5b\x03\x4d\x54\xe8\x4a\x5d\x20\x26" //u.V..i.[.MT.J] & |
// /* 0030 */ "\x4d\x58\x27\xc1\x4b\x54\x9a\xe1\x1d\xf5\x7a\x7c\x90\x6c\xc6\x68" //MX'.KT....z|.l.h |
// /* 0040 */ "\x2d\xd5\xfc\x07\xd8\xe7\x89\x06\x25\x3e\xff\x8f\x8f\x17\xda\xaf" //-.......%>...... |
// /* 0050 */ "\xfa\xae\x02\x7e\xd9\x66\x47\xca\xf6\x0c\xbb\x23\x9d\xe2\xee\x44" //...~.fG....#...D |
// /* 0060 */ "\x06\x76\x12\xc7\x02\x56\x3d\x1a\xe0\x61\x23\x3a\x2b\x7b\x9b\xc0" //.v...V=..a#:+{.. |
// /* 0070 */ "\x76\xa6\xb9\x35\xd1\x6b\xe8\x0f\x78\xcb\xa2\x3d\xe2\x8c\x10\x75" //v..5.k..x..=...u |
// /* 0080 */ "\x75\xcb\x2d\x8f\x95\x3a\xfe\xfa\xf0\xbc\xdb\x28\x03\x98\xec\x05" //u.-..:.....(.... |
// /* 0090 */ "\xfd\x03\x98\x82\x9a\xbf\x36\x78\x68\x8a\xa6\x8b\x4e\x73\xbe\xe2" //......6xh...Ns.. |
// /* 00a0 */ "\xb8\xa8\xa3\xcc\x5f\xce\x58\xc1\x0d\x21\x4f\x9e\x5c\x4d\xcd\x71" //...._.X..!O..M.q |
// /* 00b0 */ "\x55\x2e\xd7\xa4\xaf\x33\xac\xd2\xd8\x2c\x71\xa7\x90\xff\x77\x98" //U....3...,q...w. |
// /* 00c0 */ "\x65\xf3\x28\xdf\xf0\xd0\xee\x1a\x32\xe6\x8a\x6c\xd2\xcf\x15\xc4" //e.(.....2..l.... |
// /* 00d0 */ "\x29\xbe\xda\xcc\xa3\x73\x9b\xae\x32\x5d\xfd\x65\x62\x93\x26\x60" //)....s..2].eb.&` |
// /* 00e0 */ "\x77\xda\xa9\xa2\x05\x79\xa7\xf0\x64\x6b\x9b\x86\xde\xcc\x99\x47" //w....y..dk.....G |
// /* 00f0 */ "\xe4\x5b\x91\x55\xbb\xf7\x27\x72\x4d\xe9\x04\x0a\x74\x9d\xee\xc3" //.[.U..'rM...t... |
// /* 0000 */ "\xad\x4c\x8b\xa7\x68\x87\x71\xee\x88\x33\x8c\x7d\x1f\xa1\x37\x15" //.L..h.q..3.}..7. |
// /* 0010 */ "\x3c\x66\x30\x7d\xa5\x22\x36\x83\xec\xb1\xaf\xa3\x09\x6a\x0f\x1f" //<f0}."6......j.. |
// /* 0020 */ "\x5a\xbc\x00\xb6\x4d\x23\x90\x5a\xcc\xbf\x6d\xfa\xdb\xc4\x13\xf9" //Z...M#.Z..m..... |
// /* 0030 */ "\x53\xf5\x88\xde\x9c\xe6\x61\xca\x1c\xb1\xe0\x9e\x18\x0e\xb0\x38" //S.....a........8 |
// /* 0040 */ "\xc2\xb2\x2a\x09\xa5\x69\xd0\xa4\x37\x8f\x3c\x9a\x12\x8f\x3c\x1f" //..*..i..7.<...<. |
// /* 0050 */ "\xf9\x69\x5b\xcd\x30\xaa\x28\xf5\x2f\x3c\xb3\xb3\x0e\xbb\x33\x46" //.i[.0.(./<....3F |
// /* 0060 */ "\xea\x43\x92\x76\x5a\x21\xed\x43\xdb\xb3\xc1\x13\x70\xc8\xc4\xb4" //.C.vZ!.C....p... |
// /* 0070 */ "\x53\x72\xbe\x09\x91\x43\x7c\x1d\x03\x46\xbc\x24\xd7\x49\xbd\x8b" //Sr...C|..F.$.I.. |
// /* 0080 */ "\x12\xef\x98\xf1\x98\x47\x12\xbc\x28\x08\x7c\x0d\x53\x60\xf2\xa9" //.....G..(.|.S`.. |
// /* 0090 */ "\xbc\xfb\xa4\xa7\xc5\xc3\x8c\x31\xac\x9c\xa2\x18\x1a\xdf\x8c\xe6" //.......1........ |
// /* 00a0 */ "\xf6\xe1\x21\xd8\x43\x6c\x39\xfd\x5a\x21\x43\x50\x25\xc0\xb7\xa5" //..!.Cl9.Z!CP%... |
// /* 00b0 */ "\x4f\x89\xf3\xb4\xdd\x21\x55\x3a\x4f\x92\x1d\xe8\x9b\x3c\x06\x18" //O....!U:O....<.. |
// /* 00c0 */ "\xe4\xfd\x13\x88\x84\x75\x58\xe0\x75\x23\xc2\x76\x43\x21\xcb\x24" //.....uX.u#.vC!.$ |
// /* 00d0 */ "\xa2\x68\x80\x00\xe4\x40\xf0\x93\x5b\xc5\x24\x99\xe4\xb9\x91\x7f" //.h...@..[.$..... |
// /* 00e0 */ "\x8b\x83\x14\xa0\xc1\x73\xd3\xb1\x28\x63\x96\xf2\x8e\xa6\x50\xc2" //.....s..(c....P. |
// /* 00f0 */ "\x12\x8b\x7a\xc9\x8f\xd8\x2e\xf3\x3e\x37\x85\x08\xef\x82\x77\xd8" //..z.....>7....w. |
// /* 0000 */ "\xcb\x15\x8e\x0c\xc0\x78\x0e\xca\x8a\x0b\x30\x52\x61\x47\x0d\x6a" //.....x....0RaG.j |
// /* 0010 */ "\x51\x2b\xb1\x6d\x1f\xab\x96\xe3\x24\x64\x42\x4e\x14\x4b\x51\xb3" //Q+.m....$dBN.KQ. |
// /* 0020 */ "\xf8\xc4\xe6\x9d\x24\xd3\x1f\x19\x6c\x52\xa1\xf5\x11\xbe\xdc\x98" //....$...lR...... |
// /* 0030 */ "\x06\x97\x5c\xe0\xf1\x7f\x17\x3b\xa4\x81\x5d\x86\x22\x77\xee\x2f" //.......;..]."w./ |
// /* 0040 */ "\x81\x7f\x7f\x22\x0d\x70\xd3\x5c\x5b\xbe\x9d\x86\xa9\x61\x53\xbf" //...".p..[....aS. |
// /* 0050 */ "\x43\xb3\x8a\x8e\x9a\xb7\x88\x15\x00\x2f\x10\xeb\xa1\xab\x64\xbc" //C......../....d. |
// /* 0060 */ "\x93\xb0\xab\x90\x50\x8d\x0a\xe4\xb6\x58\xb5\xff\xff\x05\x15\x9d" //....P....X...... |
// /* 0070 */ "\xb7\x22\xa6\x75\x90\x02\xad\x6d\x82\xf4\x9c\x74\x58\x9e\x46\xeb" //.".u...m...tX.F. |
// /* 0080 */ "\x71\xd6\xbc\x41\x26\x21\x13\xb1\xce\x11\xd5\xa6\x0f\x87\x3b\x09" //q..A&!........;. |
// /* 0090 */ "\xd7\xcf\x41\x75\x2f\x66\x32\x64\xdf\xfb\x38\x37\xde\x34\x32\x41" //..Au/f2d..87.42A |
// /* 00a0 */ "\xba\x90\xef\xe9\xaa\x7b\x57\xb4\x7e\x52\xf6\xb6\xa2\x52\x64\xd0" //.....{W.~R...Rd. |
// /* 00b0 */ "\xe0\x33\x03\x1b\xaa\x1c\xc7\x9c\x8e\xc7\x48\x1c\x7d\x32\xbf\x9d" //.3........H.}2.. |
// /* 00c0 */ "\xe5\xc7\xb4\xfd\x1c\x4d\x19\x1f\x19\x5a\x88\x03\x68\x61\xf2\x03" //.....M...Z..ha.. |
// /* 00d0 */ "\x4f\xbf\x40\x53\x88\x3a\xa8\x43\xbf\xea\x1c\x43\x83\x7c\x12\x03" //O.@S.:.C...C.|.. |
// /* 00e0 */ "\xcb\x39\x94\x86\xd1\x62\xd1\xa1\x8d\x49\x66\x8f\xac\x69\xf7\xa4" //.9...b...If..i.. |
// /* 00f0 */ "\x4c\x16\xca\xd1\xd2\xd2\xf9\x06\x23\x40\x22\x5d\x33\x87\x14\xde" //L.......#@"]3... |
// /* 0000 */ "\xb6\xd9\x90\x40\x6a\x26\xc2\x49\x8e\x35\xbd\xaf\x21\xf8\xab\xb4" //...@j&.I.5..!... |
// /* 0010 */ "\x57\x34\x26\xec\x3c\xa4\x01\x80\x9a\xcf\xac\xf2\x64\x88\x13\xb4" //W4&.<.......d... |
// /* 0020 */ "\xc4\xaa\x77\xc7\x2c\x74\x70\x15\xa6\xa0\xce\x09\x4a\xce\x89\x78" //..w.,tp.....J..x |
// /* 0030 */ "\x34\x49\x9d\xd5\x0d\xae\x65\xff\x34\x09\x02\x85\x4d\x48\x7d\xbe" //4I....e.4...MH}. |
// /* 0040 */ "\xdf\xdd\xbe\x2d\x15\xf3\x5c\xcb\x34\x12\x9f\x2a\xe5\x5b\x17\x22" //...-....4..*.[." |
// /* 0050 */ "\xf1\x85\x82\xe1\xa8\x46\xb1\xbe\xf7\x36\x23\xb1\x5f\x18\xa3\xdc" //.....F...6#._... |
// /* 0060 */ "\x76\x25\x9c\x8b\x88\x76\xc6\xb0\x62\x73\x29\x29\xe1\xff\xc0\xdf" //v%...v..bs)).... |
// /* 0070 */ "\x24\x55\x65\x75\x1f\xe1\x7b\x4e\x51\x6c\x89\xab\xb6\xdd\x8d\x24" //$Ueu..{NQl.....$ |
// /* 0080 */ "\xf9\xa5\x77\x3e\xa8\x38\xb8\x89\x7b\xb2\x9d\x78\xe9\xbe\x29\x4b" //..w>.8..{..x..)K |
// /* 0090 */ "\xde\xc6\xda\xeb\x6b\x40\x69\x5c\xed\x3b\x5d\xec\xa4\x81\x43\xd9" //....k@i..;]...C. |
// /* 00a0 */ "\x2e\x97\x3f\x91\xf2"                                             //..?.. |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(20,10,108,17) op=(20,10,108,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=26 data_len=26 [0x00 0x00 0x01 0x09 0x02 0x0e 0x03 0x09 0x04 0x09 0x02 0x04 0x03 0x09 0x04 0x09 0x05 0x04 0x06 0x09 0x04 0x09 0x06 0x04 0x06 0x09] |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(20,41,93,17) op=(20,41,93,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=57 data_len=24 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09 0x0d 0x05 0x0e 0x04 0x09 0x0b 0x0f 0x05 0x10 0x09 0x11 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,41,706,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(224,41,46,17) op=(224,41,46,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=57 data_len=12 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09] |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(930,41,58,17) op=(930,41,58,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=57 data_len=16 [0x12 0x00 0x09 0x09 0x0f 0x05 0x0c 0x09 0x0f 0x05 0x13 0x09 0x0f 0x08 0x14 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(963,13,34,17) op=(963,13,34,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=962 glyph_y=29 data_len=12 [0x15 0x00 0x16 0x08 0x14 0x03 0x0c 0x03 0x0b 0x05 0x09 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(794,705,19,17) op=(794,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=793 glyph_y=721 data_len=4 [0x17 0x00 0x18 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(835,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(836,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(842,705,12,17) op=(842,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=841 glyph_y=721 data_len=2 [0x17 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(877,704,23,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=876 glyph_y=721 data_len=2 [0x02 0x00] |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=900 glyph_y=721 data_len=4 [0x19 0x00 0x03 0x05] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(937,705,12,17) op=(937,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=936 glyph_y=721 data_len=2 [0x1a 0x00] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(978,705,19,17) op=(978,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=977 glyph_y=721 data_len=4 [0x1b 0x00 0x1a 0x07] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(817,738,49,17) op=(817,738,49,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=816 glyph_y=754 data_len=12 [0x1c 0x00 0x0f 0x07 0x0a 0x09 0x0f 0x09 0x10 0x09 0x0c 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(920,738,60,17) op=(920,738,60,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=919 glyph_y=754 data_len=14 [0x1d 0x00 0x0f 0x0a 0x1e 0x09 0x1e 0x09 0x0b 0x09 0x13 0x09 0x0c 0x08] |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x84\x4e\x08\x00\x00\x00\x0e" //...h. ..p.N..... |
// /* 0010 */ "\xec\xf3\x9f\xd4\x76\x1a\x6a\x7b\x13\x84\x30\xb3\xdf\xeb\x94\x90" //....v.j{..0..... |
// /* 0020 */ "\xc0\x65\xab\x70\xe3\x1e\x6a\xf3\x66\x5f\xed\x8a\x2b\xbd\x9b\x3a" //.e.p..j.f_..+..: |
// /* 0030 */ "\xc3\x07\x1d\x71\x5d\x91\x15\x17\x87\x78\xe5\x5c\x31\x95\x92\xdc" //...q]....x..1... |
// /* 0040 */ "\x4e\x2e\xcd\x32\x12\xd2\x0c\x38\xc1\xe4\x7f\x13\xd8\x33\x50\x44" //N..2...8.....3PD |
// /* 0050 */ "\x55\xcf\xf8\x8c\xf5\xd8\xac\x6c\x57\x40\xa5\xc9\x61\x1a\xf3\xf5" //U......lW@..a... |
// /* 0060 */ "\xd3\xae\x72\x21\xfe\x9a\xd2\x82\x82\xb3\x59\x8b\x98\xc3\x3a\x0a" //..r!......Y...:. |
// /* 0070 */ "\xa3\xde\xf1\x77\xe3\x29\x0f\xa5\xa1\x39\x0f\x09\x3a\x1c\xd9\xa0" //...w.)...9..:... |
// /* 0080 */ "\x96\x9d\x3c\x5c\x17\xc0\x1c\x94\x1f\xdd\x69\x68\x23\x31\xdc\x14" //..<.......ih#1.. |
// /* 0090 */ "\xa5\x2d\x36\x16\x88\x83\x1f\x39\xc1\x06\xef\x91\x83\x9e\xbf\xcf" //.-6....9........ |
// /* 00a0 */ "\x60\x20\x4e\xca\x94\x10\xcc\x45\x37\x64\xe4\x13\x16\x28\xda\xd8" //` N....E7d...(.. |
// /* 00b0 */ "\xf6\xec\xfd\x21\x8f\x35\x8c\x72\x83\x3c\x72\x7c\x9f\x19\xf7\x87" //...!.5.r.<r|.... |
// /* 00c0 */ "\xfd\xa0\x70\x1a\x80\xa1\x24\xd7\x29\xea\xf8\x86\xcd\xb4\xb6\x56" //..p...$.)......V |
// /* 00d0 */ "\x61\x86\xf8\x0f\x9a\xab\x02\x19\xea\xfc\xf0\xdc\x70\xe4\x77\xba" //a...........p.w. |
// /* 00e0 */ "\x03\x0c\x2d\xec\x5e\x0f\xb5\x41\xb1\xc3\x1c\x66\x2b\x1a\xba\x63" //..-.^..A...f+..c |
// /* 00f0 */ "\x42\xbe\x37\xcf\x8d\x61\x66\x2c\x55\xff\x7a\x84\x7f\x43\x11\x6b" //B.7..af,U.z..C.k |
// /* 0000 */ "\xda\x8b\x46\x49\x45\x64\xaa\x3b\x97\x7a\x28\xfb\xee\xa0\xc6\x7f" //..FIEd.;.z(..... |
// /* 0010 */ "\xb0\xbc\x23\x0e\x14\x63\x89\xe7\x74\xd6\xb8\x08\xcb\x09\x81\xde" //..#..c..t....... |
// /* 0020 */ "\x73\x90\xed\x8a\x18\x1d\xd8\xfb\x49\x68\xe6\x8e\x97\x3f\x73\x50" //s.......Ih...?sP |
// /* 0030 */ "\xa4\x0c\x81\x23\x21\x80\x77\xde\x5b\xe1\x62\x67\x1c\x6b\xbe\xb4" //...#!.w.[.bg.k.. |
// /* 0040 */ "\x3b\x6e\x56\x12\xd6\xfb\xcb\x98\xd3\xc5\xe6\x8d\x1b\x33\x53\xc5" //;nV..........3S. |
// /* 0050 */ "\xd0\x0e\x50\x37\xea\xd6\x8a\x55\xb3\xf4\x43\x7d\x7c\xca\xab\xa1" //..P7...U..C}|... |
// /* 0060 */ "\x09\x7f\xd0\x10\x2c\x1f\x8a\x0b\x17\xfa\xcb\x99\x1b\x93\x44\x9f" //....,.........D. |
// /* 0070 */ "\x63\xfa\x17\xb1\x07\xd2\xd2\xa2\xfc\xd4\xc5\x0c\x25\x6e\x46\x70" //c...........%nFp |
// /* 0080 */ "\x1f\xc2\x98\x21\x22\xb9\x8b\x6d\xe2\x9c\x7e\x9c\x29\xfa\x4c\x16" //...!"..m..~.).L. |
// /* 0090 */ "\xa0\x3a\x6d\x07\xd6\xc7\x8e\xab\x9a\x52\xdc\x9c\x27\x65\x14\x22" //.:m......R..'e." |
// /* 00a0 */ "\x90\xef\x3e\x35\x87\x25\x10\x59\x91\xa0\xd3\x61\xe3\x90\xa1\xcf" //..>5.%.Y...a.... |
// /* 00b0 */ "\xa1\x4b\xa9\xa8\x38\x4e\x9c\x1e\x0b\x31\xd1\x6d\x62\xbc\x20\x8e" //.K..8N...1.mb. . |
// /* 00c0 */ "\xe3\x5a\x5f\x28\x92\x60\x23\x4e\xeb\xef\x01\x1b\xc6\xa1\xf5\x28" //.Z_(.`#N.......( |
// /* 00d0 */ "\x43\xd6\x44\x90\x1f\x26\x59\x2b\x49\x6d\x07\x24\x3c\x34\xfa\xdb" //C.D..&Y+Im.$<4.. |
// /* 00e0 */ "\x00\x5b\xa2\xc6\xc6\xfb\x98\xb1\x5b\x5a\x90\x88\x99\xce\xc3\xa3" //.[......[Z...... |
// /* 00f0 */ "\xda\x9f\x45\xfb\x3f\x31\x05\x0d\x2b\xbc\x82\xfc\x9c\xa2\xed\xd5" //..E.?1..+....... |
// /* 0000 */ "\x9b\x8f\x3f\x75\x08\xa2\x5d\x5c\x3b\x07\xa2\xf3\xef\x74\xbd\x34" //..?u..].;....t.4 |
// /* 0010 */ "\x6c\xe7\x88\x38\xb8\x02\x8f\xfd\xbd\x15\xaa\xe9\x24\x2b\xcb\xf6" //l..8........$+.. |
// /* 0020 */ "\x51\x40\x3d\x2a\x3e\x7f\xcc\x90\x97\x45\x3c\xfa\x2e\x89\x5b\x2b" //Q@=*>....E<...[+ |
// /* 0030 */ "\xcd\xe8\xcc\xea\x99\x5b\x8e\xf2\xb1\x78\x25\x14\xd8\xdb\x75\x30" //.....[...x%...u0 |
// /* 0040 */ "\x17\x23\x2b\x60\xd6\xcc\x6e\x84\xca\xa8\x15\x4a\x51\x00\x8f\x25" //.#+`..n....JQ..% |
// /* 0050 */ "\x1c\xfa\x89\x54\x21\x34\x49\xdf\x64\xfd\x25\xf9\x33\x8a\x84\x5f" //...T!4I.d.%.3.._ |
// /* 0060 */ "\xde\xfb\xc2\x93\xf4\x6a\x41\x63\x10\x46\xec\xb1\x45\xe9\xa1\xf3" //.....jAc.F..E... |
// /* 0070 */ "\xf3\x17\x5a\xc3\x67\x21\x5e\x95\xfc\xa4\x88\xe2\x4c\xf3\xc4\xbd" //..Z.g!^.....L... |
// /* 0080 */ "\xfd\xf5\x86\x54\xb1\x9a\xf2\x52\xdd\xa4\xc3\x5f\xf9\x7a\xa5\x99" //...T...R..._.z.. |
// /* 0090 */ "\x31\x74\xb8\xb5\x9f\xa5\x30\x6f\xcb\x6b\xa1\x75\x23\xe6\x1f\x5c" //1t....0o.k.u#... |
// /* 00a0 */ "\x9b\xc5\xe4\x74\x58\xfa\x73\x92\x4d\xd1\x41\xd9\x40\xd6\x89\xe0" //...tX.s.M.A.@... |
// /* 00b0 */ "\x2a\x62\x77\xa9\x6b\x33\x38\x04\xf6\xcd\x9e\xfb\x64\x35\x8a\xed" //*bw.k38.....d5.. |
// /* 00c0 */ "\xad\xa5\x4a\x65\x1e\x93\x92\xcb\x7b\x02\xb5\x4e\x22\xe4\x59\xdf" //..Je....{..N".Y. |
// /* 00d0 */ "\x40\x9f\x86\x58\xb4\xd1\xf4\x51\x07\x20\xed\xba\x18\xec\xe8\x3c" //@..X...Q. .....< |
// /* 00e0 */ "\xb3\xc2\x55\xe4\xf9\xa1\x43\xdd\x17\x6e\x04\xd4\xb0\x07\xe0\x0f" //..U...C..n...... |
// /* 00f0 */ "\xba\x09\x02\xdb\xcf\xfe\xa7\x61\xe6\x2c\x9a\x99\x4b\x69\xc7\x06" //.......a.,..Ki.. |
// /* 0000 */ "\xde\x56\x1c\x30\xd9\xfd\x9f\xfb\xb1\x2d\x99\xa9\x38\xb9\xa4\xbb" //.V.0.....-..8... |
// /* 0010 */ "\x60\x73\x2e\x1e\x7d\xfe\x0d\x83\xaf\x97\x22\x86\xad\xf2\x50\xee" //`s..}....."...P. |
// /* 0020 */ "\x06\x9f\x42\x4f\x4d\x5a\x1b\xdf\x67\xce\xff\x8a\x93\x94\x5d\x33" //..BOMZ..g.....]3 |
// /* 0030 */ "\x54\x60\x83\xeb\x9f\x39\x1b\x37\xc4\x98\xf0\xe1\xe4\xbb\x4a\x31" //T`...9.7......J1 |
// /* 0040 */ "\xe6\x72\xf5\x4e\x02\x3a\x11\x3a\x79\x3d\xd8\xbd\x54\x03\x49\x03" //.r.N.:.:y=..T.I. |
// /* 0050 */ "\x03\x67\xf7\xac\xc8\x28\x96\xf8\x17\x5f\x2c\x75\xd4\x35\x94\x87" //.g...(..._,u.5.. |
// /* 0060 */ "\xd8\x9b\x69\x0c\xaa\xc1\x87\x7e\xf6\x39\xd2\xe1\x5f\x48\x3b\x2b" //..i....~.9.._H;+ |
// /* 0070 */ "\xb9\xaa\x8e\x50\xa4\x54\xfe\xc3\x23\x44\x1b\x03\xe7\xc3\xc0\xa5" //...P.T..#D...... |
// /* 0080 */ "\x2f\x1c\x41\xe5\x33\x36\x8e\x1a\x1b\x06\x11\xe0\x25\x72\x1d\x06" ///.A.36......%r.. |
// /* 0090 */ "\x11\x92\x2c\x37\xf8\x82\x6c\x67\x67\x2a\x8a\x48\x5d\xbc\x38\xd8" //..,7..lgg*.H].8. |
// /* 00a0 */ "\xc9\x85\xca\x90\x1f\x5d\x8c\xef\xb4\x0a\xdb\xcd\x36\x6b\xc2\x01" //.....]......6k.. |
// /* 00b0 */ "\x70\x64\xf5\x2f\x03\x33\xab\x6d\xd8\x22\xba\xe0\x4c\xa3\xc1\x70" //pd./.3.m."..L..p |
// /* 00c0 */ "\x3a\xa9\x44\xf6\x40\xb1\xe0\x7b\x20\xb8\xd5\xae\xb6\x45\x7e\x9a" //:.D.@..{ ....E~. |
// /* 00d0 */ "\xec\x7f\x2b\xca\x0e\xab\x49\x53\x60\x76\x6f\x0a\x1f\x5a\xd5\x21" //..+...IS`vo..Z.! |
// /* 00e0 */ "\x0d\xe0\xc4\x49\x14\x49\x12\x71\x60\x00\xd5\x89\xf2\xda\xb6\x05" //...I.I.q`....... |
// /* 00f0 */ "\x00\x69\x15\x36\xf6\x69\x28\xaa\x05\xd5\x90\x2e\xc4\x3c\x70\x0a" //.i.6.i(......<p. |
// /* 0000 */ "\x67\x31\x11\xbb\xd6\x46\xf1\x44\x17\xd3\x1b\x97\x24\x56\x4b\x0c" //g1...F.D....$VK. |
// /* 0010 */ "\x9d\xca\x03\x54\xb1\x23\xea\xb0\x72\x75\x91\xbd\xe3\xd2\x04\x7f" //...T.#..ru...... |
// /* 0020 */ "\x58\x1b\x49\xfa\x76\xca\x90\x3b\x22\xbf\xef\x21\xb2\x88\xed\x5e" //X.I.v..;"..!...^ |
// /* 0030 */ "\x00\x6f\xef\xb4\xcc\xfd\xc7\x41\x71\x8f\xa5\x5d\xfe\x95\xc4\xe4" //.o.....Aq..].... |
// /* 0040 */ "\xb9\xe2\x5d\xa0\x75\xa1\xdc\x21\xcc\x18\x84\xec\xc2\xe5\xba\xb0" //..].u..!........ |
// /* 0050 */ "\xf1\xf5\xae\x76\x6d\x7f\x2e\x7b\x44"                             //...vm..{D |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(20,10,108,17) op=(20,10,108,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=26 data_len=26 [0x00 0x00 0x01 0x09 0x02 0x0e 0x03 0x09 0x04 0x09 0x02 0x04 0x03 0x09 0x04 0x09 0x05 0x04 0x06 0x09 0x04 0x09 0x06 0x04 0x06 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,41,204,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(20,41,93,17) op=(20,41,93,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=57 data_len=24 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09 0x0d 0x05 0x0e 0x04 0x09 0x0b 0x0f 0x05 0x10 0x09 0x11 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,41,706,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(224,41,46,17) op=(224,41,46,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=57 data_len=12 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,41,84,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(930,41,58,17) op=(930,41,58,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=57 data_len=16 [0x12 0x00 0x09 0x09 0x0f 0x05 0x0c 0x09 0x0f 0x05 0x13 0x09 0x0f 0x08 0x14 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(963,13,34,17) op=(963,13,34,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=962 glyph_y=29 data_len=12 [0x15 0x00 0x16 0x08 0x14 0x03 0x0c 0x03 0x0b 0x05 0x09 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(794,705,19,17) op=(794,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=793 glyph_y=721 data_len=4 [0x17 0x00 0x18 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(835,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(836,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(842,705,12,17) op=(842,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=841 glyph_y=721 data_len=2 [0x17 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(877,704,23,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=876 glyph_y=721 data_len=2 [0x02 0x00] |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=900 glyph_y=721 data_len=4 [0x19 0x00 0x03 0x05] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(930,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(931,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(937,705,12,17) op=(937,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=936 glyph_y=721 data_len=2 [0x1a 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(971,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(972,703,30,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(978,705,19,17) op=(978,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=977 glyph_y=721 data_len=4 [0x1b 0x00 0x1a 0x07] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,63,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(811,736,60,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(817,738,49,17) op=(817,738,49,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=816 glyph_y=754 data_len=12 [0x1c 0x00 0x0f 0x07 0x0a 0x09 0x0f 0x09 0x10 0x09 0x0c 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(920,738,60,17) op=(920,738,60,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=919 glyph_y=754 data_len=14 [0x1d 0x00 0x0f 0x0a 0x1e 0x09 0x1e 0x09 0x0b 0x09 0x13 0x09 0x0c 0x08] |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x50\x08\x00\x00\x00\x30\x85" //...h. ..pP....0. |
// /* 0010 */ "\xa6\x69\x36\xed\xe1\x6d\x50\xc8\x0b\x64\x2b\x01\x50\x7f\xbb\x7e" //.i6..mP..d+.P..~ |
// /* 0020 */ "\xa3\xad\xdf\x6b\x3c\xd3\xc9\xe2\xb0\x3d\xc9\xf6\x37\xc2\x3b\xa2" //...k<....=..7.;. |
// /* 0030 */ "\xbb\x76\x4f\x49\x08\xa6\x53\x5c\xf6\x87\x94\x21\x38\x40\xed\x16" //.vOI..S....!8@.. |
// /* 0040 */ "\xce\x8c\xe2\x39\x2f\x0c\xc6\xcc\x10\x25\xa4\x89\xf7\x32\x70\x0b" //...9/....%...2p. |
// /* 0050 */ "\x1d\x81\xa3\x22\x85\xd8\x29\xd3\xfc\x15"                         //..."..)... |
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
// /* 0000 */ "\x00\x09\xea"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 2534 bytes |
// Recv done on RDP Wab Target (4) 2534 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x89\xdb\x08\x00\x00\x00\xa8" //...h. ..p....... |
// /* 0010 */ "\x3c\xa6\xc0\x4c\xf8\xd9\x30\x76\xab\x1c\xeb\x20\xdc\xb7\x51\xc1" //<..L..0v... ..Q. |
// /* 0020 */ "\xfc\xe9\x5c\x7a\x86\xc7\x35\x75\x60\x49\x2d\xdb\xa2\x30\xf7\xfd" //...z..5u`I-..0.. |
// /* 0030 */ "\x95\x80\x4a\x77\xb0\xde\xc2\xb3\xd1\xf4\xdf\xf4\x34\x93\x1e\x7b" //..Jw........4..{ |
// /* 0040 */ "\x15\xde\x0b\x2a\x30\x27\x09\xe6\x50\xc0\xb3\xf8\x0b\xa2\x1e\x8e" //...*0'..P....... |
// /* 0050 */ "\x46\xf9\xb4\xa3\xd5\xea\x40\x4d\xa3\x48\x4f\x9a\xc8\x3d\x92\xcd" //F.....@M.HO..=.. |
// /* 0060 */ "\xd3\x7a\x6c\xe1\x8d\x56\xf8\xe4\x80\x1c\x1f\xf8\xb3\xd2\x59\x8b" //.zl..V........Y. |
// /* 0070 */ "\x96\xc3\x68\x8a\xc9\xba\x3d\xd6\xcb\xbf\x25\xec\xc6\xde\xe0\xd0" //..h...=...%..... |
// /* 0080 */ "\xfb\x23\xb9\x0e\x50\xaf\xdb\xa2\xe1\xd3\x00\x9e\xf6\xa5\x48\x48" //.#..P.........HH |
// /* 0090 */ "\x72\x69\x01\x45\xbe\x9a\xf0\x82\xf5\x32\xd3\xd1\xe4\x6f\xac\x72" //ri.E.....2...o.r |
// /* 00a0 */ "\xdc\xa9\x33\x08\x08\xe8\xf8\x07\xe4\x9b\x58\xe9\xdf\xde\xe8\xde" //..3.......X..... |
// /* 00b0 */ "\x98\x49\x8c\xfc\x2e\x4e\x4e\x9d\x2e\x4c\x87\x6d\xd1\xf9\x6e\xe6" //.I...NN..L.m..n. |
// /* 00c0 */ "\xee\xc1\x97\x72\xc8\x14\xc4\x22\x12\xa6\xd2\x6a\x15\x5e\x1e\xde" //...r..."...j.^.. |
// /* 00d0 */ "\x45\xd6\xe4\xd1\x0f\xaa\xb6\x79\x80\x20\xfb\x01\xc1\x6e\xe5\xda" //E......y. ...n.. |
// /* 00e0 */ "\x59\x11\x93\x4d\x99\xc4\xca\xaf\x1a\x43\xe8\xaf\xf6\x50\x86\x30" //Y..M.....C...P.0 |
// /* 00f0 */ "\x50\x5f\xd8\x4f\xf9\x59\xfa\xd5\x17\x53\xdb\x83\x8d\xa0\xd9\xf5" //P_.O.Y...S...... |
// /* 0000 */ "\xfc\x92\xaa\xc4\xcb\x6b\x2a\x12\x01\xdc\x6b\x81\xbe\x98\x32\xc2" //.....k*...k...2. |
// /* 0010 */ "\x92\x36\x51\x73\x71\x42\x1d\x1e\xb9\x16\x3b\x7f\x1b\xc4\xaf\x7b" //.6QsqB....;....{ |
// /* 0020 */ "\xc6\x9f\x00\x7f\x79\xaf\x93\xd3\x7d\xc9\x80\xc3\x67\x27\x33\xd7" //....y...}...g'3. |
// /* 0030 */ "\xb6\x96\x1a\x3f\x03\x55\x47\x8c\xde\x3c\x99\x58\xb3\xb4\x2c\x88" //...?.UG..<.X..,. |
// /* 0040 */ "\x58\xa1\x26\x48\x7f\x69\xb9\x8e\xda\x3b\x7a\xd3\xfb\x75\xce\x91" //X.&H.i...;z..u.. |
// /* 0050 */ "\xa3\x8a\x2e\xc0\xbb\xc9\x16\xce\x54\xb3\x7a\x8b\xf8\x40\xdc\xe1" //........T.z..@.. |
// /* 0060 */ "\x92\xa9\x8d\x76\x42\xa5\xdf\xba\x13\x5f\x44\x77\xab\xd2\x01\xea" //...vB...._Dw.... |
// /* 0070 */ "\x3f\xd2\xe6\x27\x5c\x8a\xe5\xe1\xf5\xed\x4a\xeb\x75\xd9\x2f\xd3" //?..'......J.u./. |
// /* 0080 */ "\xe3\xbb\xef\x7c\x62\x94\x0a\x2f\x13\x6d\xc6\x53\x42\xf5\xb6\xe1" //...|b../.m.SB... |
// /* 0090 */ "\xd5\xb6\x72\x5d\xd5\x40\xa0\xac\x9f\xf9\xce\x2b\x9b\x46\x1a\xce" //..r].@.....+.F.. |
// /* 00a0 */ "\x26\xda\x31\x86\x77\x23\x7f\xd5\x16\xf3\x19\xe4\xda\xee\x10\x0b" //&.1.w#.......... |
// /* 00b0 */ "\x36\xf1\xd5\x3d\x98\x5b\x11\x73\x5d\x75\x34\x32\x16\x80\x59\x6f" //6..=.[.s]u42..Yo |
// /* 00c0 */ "\x39\x07\x24\x64\x28\xa1\x1a\xd0\xb2\x2e\xb2\x57\x36\x03\x85\xbf" //9.$d(......W6... |
// /* 00d0 */ "\x17\x7a\xcc\xf7\xda\xb4\x56\xc9\x30\xc5\x07\x68\x35\xbb\xaa\x23" //.z....V.0..h5..# |
// /* 00e0 */ "\xbf\x72\xcf\xef\xa7\xc0\xe7\x6f\xd8\x01\x6a\x66\x9b\x14\x33\xd3" //.r.....o..jf..3. |
// /* 00f0 */ "\xac\xec\xbb\x72\xfc\x52\x45\xd0\x24\xb5\xf9\x43\xf4\xd0\x99\x7a" //...r.RE.$..C...z |
// /* 0000 */ "\xcb\x2f\x42\x8c\x02\xa1\x1f\xe1\xc8\x54\x47\xd9\x3a\x0e\xbb\xe4" //./B......TG.:... |
// /* 0010 */ "\x6b\x5b\xba\x30\xa9\xf5\xc7\x70\x60\x0e\x4b\x6f\x87\x43\xfb\x69" //k[.0...p`.Ko.C.i |
// /* 0020 */ "\xc1\xde\x58\xe7\x69\x4f\x27\xb9\x4b\xdd\xad\x3b\x9d\x6e\x1c\xcb" //..X.iO'.K..;.n.. |
// /* 0030 */ "\xb4\x42\x8f\x0d\xa3\x58\x7a\x2e\x30\x0c\x83\x87\xac\x23\x26\x57" //.B...Xz.0....#&W |
// /* 0040 */ "\x4c\x0e\x81\x87\x02\x96\xd5\xaf\x71\x90\x11\xac\x10\x39\xb7\x79" //L.......q....9.y |
// /* 0050 */ "\xd8\x1e\x4f\x83\x91\x20\x32\xad\x7f\x8d\x29\xc6\xe4\x0a\x25\x16" //..O.. 2...)...%. |
// /* 0060 */ "\x2d\x3e\x53\x12\x15\xab\x03\x09\x2e\x30\x05\xf5\xd0\x3a\x8b\x84" //->S......0...:.. |
// /* 0070 */ "\xaf\xac\xa3\x85\xac\x00\xf9\xa0\x50\xaa\xe0\xcf\xff\xb5\xbc\x48" //........P......H |
// /* 0080 */ "\x0e\xc4\xab\xb6\xc2\xcd\xc5\xb6\xd2\x19\xcd\xea\xe1\x02\x87\x13" //................ |
// /* 0090 */ "\xb5\x12\x0f\x50\x7d\x38\xdc\x6e\x66\xa2\x1a\x53\x97\xe5\xca\x6a" //...P}8.nf..S...j |
// /* 00a0 */ "\x50\xed\x6c\x4e\x51\xf5\x09\xbf\x37\x02\xbb\x59\xb7\x7e\x71\xec" //P.lNQ...7..Y.~q. |
// /* 00b0 */ "\x56\x6e\x04\x84\xdc\x23\x15\x54\x84\x96\x9f\xf0\x60\x8a\x14\xc0" //Vn...#.T....`... |
// /* 00c0 */ "\xe2\x14\x5f\x09\xc7\xd6\x4e\x9c\x70\x8f\x77\x07\x44\x94\xad\x37" //.._...N.p.w.D..7 |
// /* 00d0 */ "\xd5\x9a\xba\x5d\x10\x83\x96\x6d\x81\x9b\xb9\xa4\x50\xbd\x16\xbb" //...]...m....P... |
// /* 00e0 */ "\x8a\x0e\x30\x99\x50\x9b\xd8\x46\xa6\x6a\x3e\x3c\x60\x83\xc1\x7b" //..0.P..F.j><`..{ |
// /* 00f0 */ "\xe0\x14\x55\x01\xbe\x8c\x01\x2f\x53\x3a\x9c\x30\x01\x32\x35\x9a" //..U..../S:.0.25. |
// /* 0000 */ "\x4f\x6c\x58\xa9\x02\xd5\xa2\x4f\xdd\xd5\x4e\x74\xf3\x42\xc6\xe4" //OlX....O..Nt.B.. |
// /* 0010 */ "\x72\xd9\xa4\xe1\x19\x8b\xd3\xcd\x36\x51\xbe\xb0\x60\x7e\xb7\x44" //r.......6Q..`~.D |
// /* 0020 */ "\x29\x05\xf0\x64\x97\xbb\x81\x01\x78\x7a\xb7\x00\x69\xbf\xbb\x3e" //)..d....xz..i..> |
// /* 0030 */ "\x9b\x20\xca\xf3\x16\x91\xf0\xdc\x90\xb8\xe5\x24\x11\xd8\x9c\x56" //. .........$...V |
// /* 0040 */ "\xea\x24\x33\xdd\x3a\xa9\x64\x70\xd2\xed\xf5\x53\x33\x72\x04\x45" //.$3.:.dp...S3r.E |
// /* 0050 */ "\xe7\x93\x9d\xa1\x1a\x13\x31\xb0\x34\x16\x0b\x7c\x22\x6e\x06\x52" //......1.4..|"n.R |
// /* 0060 */ "\x8d\xf6\x79\x4b\x1f\x9f\x1e\xe2\x13\xfc\x55\xdf\xf7\xff\xa9\x82" //..yK......U..... |
// /* 0070 */ "\x1d\x06\x2f\xea\x01\x0a\x50\x5f\x08\x2c\x03\xe1\xf3\x8d\x80\x27" //../...P_.,.....' |
// /* 0080 */ "\x4e\xf6\x32\xda\x2b\xac\x88\xed\x7b\x9f\x0b\x5d\x15\x1c\x2a\xbe" //N.2.+...{..]..*. |
// /* 0090 */ "\xa2\x26\x8f\x7c\xac\xbe\x8f\xb7\x1a\x59\xeb\xdb\xb8\xbb\x10\x67" //.&.|.....Y.....g |
// /* 00a0 */ "\xc9\xbc\xf2\x33\xa9\x74\xfc\x7a\xbc\x68\x2a\x13\x86\xe3\x64\xe8" //...3.t.z.h*...d. |
// /* 00b0 */ "\x1f\x14\x67\x5f\x69\x11\x49\x27\xd7\xeb\xf9\x56\xd2\xdf\x29\x35" //..g_i.I'...V..)5 |
// /* 00c0 */ "\xa7\x6e\x57\x35\xa2\x08\x28\xb1\xd3\xab\x71\x15\x35\xc4\xcd\x72" //.nW5..(...q.5..r |
// /* 00d0 */ "\xa8\x5d\xfe\xc5\xe6\x65\x9d\x19\xb6\xd5\xb2\x5e\x37\x89\x9f\xdf" //.]...e.....^7... |
// /* 00e0 */ "\x7e\xc1\xf6\xb7\xbc\x8c\xd4\x5a\x77\x86\x7c\xee\x84\x4b\x82\x24" //~......Zw.|..K.$ |
// /* 00f0 */ "\x81\x04\x12\x7f\x4f\xe5\x8e\xad\x78\xa4\x1d\x61\x6d\x7d\x5a\x67" //....O...x..am}Zg |
// /* 0000 */ "\xb7\x1a\xd5\x26\x92\x83\x38\x9c\xc5\x1e\xa8\x8d\x1a\x78\xd9\x8f" //...&..8......x.. |
// /* 0010 */ "\x4e\xef\x96\xb9\xa1\x66\x69\x0f\x8a\x13\x1f\xf4\x7e\x2c\xdc\xd1" //N....fi.....~,.. |
// /* 0020 */ "\x16\x49\x4b\x34\xa1\xa7\xe1\x98\x7a\xc2\x4a\x08\xb7\x4e\x3b\xd4" //.IK4....z.J..N;. |
// /* 0030 */ "\xca\x42\x6c\xa6\x3e\x44\xc2\xf3\x27\x11\x80\x72\x6d\x01\xbb\xf6" //.Bl.>D..'..rm... |
// /* 0040 */ "\x17\x3a\x74\x97\x70\x76\xa8\x1f\xab\x53\x2f\x40\x71\x19\x0d\xb2" //.:t.pv...S/@q... |
// /* 0050 */ "\xd6\x99\xb2\x14\x92\xd3\x99\xa4\xca\x03\x18\xe6\x8d\x76\x9a\xcd" //.............v.. |
// /* 0060 */ "\x46\x1e\x46\xf3\x0b\x6a\x4c\x6d\xee\xdd\x07\xbf\x79\x47\x68\x52" //F.F..jLm....yGhR |
// /* 0070 */ "\xbc\x0f\xaf\x63\x89\x29\x91\x3e\x37\x25\x03\x37\x1a\x3c\xde\x1c" //...c.).>7%.7.<.. |
// /* 0080 */ "\x5e\x75\x46\x61\x09\x34\x0d\x5a\x4e\xfe\x19\x1b\x62\x9b\x8f\x68" //^uFa.4.ZN...b..h |
// /* 0090 */ "\x52\xa5\xbe\x9d\x1d\x53\x63\x3a\x61\xa1\x5b\x00\x35\xf1\x5f\xb9" //R....Sc:a.[.5._. |
// /* 00a0 */ "\x63\x00\xa2\x83\x73\xc3\xd3\xcd\xd4\x07\x16\x39\x67\x57\xfb\x05" //c...s......9gW.. |
// /* 00b0 */ "\xd3\x69\x85\x77\x0f\xef\x66\x1b\x65\x97\xfe\x14\x7f\x80\x5e\xac" //.i.w..f.e.....^. |
// /* 00c0 */ "\x37\xa1\x0e\x31\xcf\x8a\x29\xe0\xc1\xe2\xc9\x88\x73\x35\xb3\x2d" //7..1..).....s5.- |
// /* 00d0 */ "\xf3\x69\xe0\xa3\x9c\x46\xfe\x74\x24\xb3\x78\x90\x58\x6d\x41\x0b" //.i...F.t$.x.XmA. |
// /* 00e0 */ "\x03\x14\x85\x09\x85\xe3\xe9\xb6\xae\xe0\xb2\xc5\x0e\x3f\x0f\x66" //.............?.f |
// /* 00f0 */ "\xb1\x94\xf7\xc7\xb3\x37\x34\xfe\xfd\x50\xdb\x0d\xb3\xf1\xdc\x41" //.....74..P.....A |
// /* 0000 */ "\xa1\xf8\xcb\x43\xa1\x42\xaf\xd3\x19\x35\xfc\x1e\x33\x1b\x46\x0c" //...C.B...5..3.F. |
// /* 0010 */ "\x38\xea\x2c\x38\x65\xd0\x09\x88\xf8\x3c\x3e\x69\xf1\x51\x6d\x45" //8.,8e....<>i.QmE |
// /* 0020 */ "\x23\x39\x23\x45\xd1\x3a\x98\x6b\x1b\x68\x52\x15\x72\xb5\xa5\x51" //#9#E.:.k.hR.r..Q |
// /* 0030 */ "\x3c\x86\x33\xd7\x99\x18\xa6\xe4\xe4\x06\x83\xde\xb6\xf5\x6a\x63" //<.3...........jc |
// /* 0040 */ "\x72\xd9\xa2\x1d\xec\xd0\x63\x22\xa2\x2b\x26\x4c\xbb\x55\x48\x9b" //r.....c".+&L.UH. |
// /* 0050 */ "\xc6\x2c\x44\xe9\x35\x42\x5a\x75\xbd\xd2\xf3\xe8\x64\x09\x92\x68" //.,D.5BZu....d..h |
// /* 0060 */ "\xe9\x61\xc0\xc6\x5c\xd7\x04\xca\x5c\xe4\x61\x96\x15\x19\xf5\x4e" //.a........a....N |
// /* 0070 */ "\x4c\xc9\xa0\xd6\x7b\x7d\xcf\xb1\x59\xe4\xd2\x57\x97\xbc\x38\x00" //L...{}..Y..W..8. |
// /* 0080 */ "\x76\x9a\x6c\xb5\x8d\x3c\xc6\x5e\xa5\x6e\x65\xac\x07\x37\x1f\xfa" //v.l..<.^.ne..7.. |
// /* 0090 */ "\x1c\x41\x7a\x3c\x02\x10\x59\x07\x7a\xf2\x0e\xb3\x98\xb3\xa5\x6c" //.Az<..Y.z......l |
// /* 00a0 */ "\x79\x3f\x25\x0a\xeb\xdd\x85\x17\x2c\x9f\x2c\x06\x4c\x46\x30\x71" //y?%.....,.,.LF0q |
// /* 00b0 */ "\x2d\x16\x47\xce\xcb\xc5\xbd\x64\x73\x86\xed\x8f\xbe\xf8\x5b\xa3" //-.G....ds.....[. |
// /* 00c0 */ "\x9c\x82\x38\x80\x30\x37\x77\x4a\x48\x73\xcd\x56\xe9\x5a\xaa\x3d" //..8.07wJHs.V.Z.= |
// /* 00d0 */ "\x10\x9f\x26\xde\x95\x37\xf8\x1f\x75\x25\xbe\x45\xca\x38\x7b\x5f" //..&..7..u%.E.8{_ |
// /* 00e0 */ "\x4e\x31\xf2\xfd\xbb\x08\xb4\x45\xf9\xdb\x3e\xa0\x91\x25\x43\x1a" //N1.....E..>..%C. |
// /* 00f0 */ "\x55\x4c\x0e\x24\x39\x40\x01\xda\xbe\x50\xc1\xfa\xef\xbd\x6c\x04" //UL.$9@...P....l. |
// /* 0000 */ "\xc2\x82\x38\x54\x35\x5c\x35\xdb\xd2\x22\x93\x0e\x0f\x9a\xa6\xaf" //..8T5.5.."...... |
// /* 0010 */ "\xb9\x9f\xbd\x95\xbf\x2a\x50\xdc\x9b\xa7\x47\x23\x64\x29\x78\xb7" //.....*P...G#d)x. |
// /* 0020 */ "\x6a\x9b\xe2\x16\x84\xcb\x78\xc6\xf7\xa7\x0e\xc8\xd9\xf9\xe2\xf7" //j.....x......... |
// /* 0030 */ "\x18\x02\xfa\x5f\x03\xe2\x03\x4e\x4d\x71\x04\x11\x52\x9e\x69\x62" //..._...NMq..R.ib |
// /* 0040 */ "\x76\x72\x92\x69\x42\xaa\x2e\x2f\xe4\x49\x86\x7f\x53\x8e\x81\xcd" //vr.iB../.I..S... |
// /* 0050 */ "\xc0\x9e\x0b\x9d\x7f\xf3\x1a\x3d\xae\x2d\xd3\xea\xec\x29\x3c\x90" //.......=.-...)<. |
// /* 0060 */ "\x99\x72\x16\x7f\x2a\x8c\x3c\x3b\x66\xa4\xeb\xe0\x34\x44\x95\xf9" //.r..*.<;f...4D.. |
// /* 0070 */ "\x3c\xef\x86\x38\x5f\xf4\xf2\x08\xf9\xf2\xc2\xf1\xf7\xa5\xd8\x16" //<..8_........... |
// /* 0080 */ "\x4e\xa1\x57\xcd\x1e\x32\xe8\xf0\xa0\x6b\x59\x53\xd9\x0f\x9c\x54" //N.W..2...kYS...T |
// /* 0090 */ "\xb8\x93\x47\xca\xa7\x4d\xc3\x68\x37\x0f\x49\x18\xf7\x5c\x13\x67" //..G..M.h7.I....g |
// /* 00a0 */ "\x2e\xe1\x34\x9d\x21\xbb\x24\x3b\x9c\x3a\x95\xad\xdc\x95\xc3\xc0" //..4.!.$;.:...... |
// /* 00b0 */ "\x69\x68\x76\x18\xb8\xba\x02\x2c\xac\x5f\xd7\xa6\x75\x38\x8b\x08" //ihv....,._..u8.. |
// /* 00c0 */ "\xf1\xe5\x40\x03\xd6\x1f\xc6\xc5\xf8\x43\x22\xce\xd6\x14\xf3\x78" //..@......C"....x |
// /* 00d0 */ "\x85\x8d\x6c\x4a\x83\x05\x4b\xe8\x81\xb8\x8e\x3a\x18\x5c\x47\xdd" //..lJ..K....:..G. |
// /* 00e0 */ "\x8a\xbc\xad\x64\x34\xa7\x68\xf1\xd6\x28\x4d\xc0\x5e\x3b\x38\x11" //...d4.h..(M.^;8. |
// /* 00f0 */ "\xdd\x55\x64\x70\xd7\x21\x8d\x5f\xe8\xc2\x57\x86\x9a\x89\xa3\x85" //.Udp.!._..W..... |
// /* 0000 */ "\x6e\x5d\x1f\x00\x35\xb3\x2a\xfc\xeb\x50\x1a\xc6\x4c\x24\x80\x51" //n]..5.*..P..L$.Q |
// /* 0010 */ "\xd2\xab\xe6\x0e\x85\xc4\x1e\x3f\xf7\x6d\x9b\xf0\xdd\xc1\xab\xc4" //.......?.m...... |
// /* 0020 */ "\x7a\xc7\xf6\x06\xb0\x5f\x3a\x72\x6d\xd6\x18\x50\x5f\xc9\xe6\xf7" //z...._:rm..P_... |
// /* 0030 */ "\x30\x15\xa9\x2b\x87\xa5\x2d\x8c\xb6\x83\x04\x3e\x8a\x2e\xae\x1f" //0..+..-....>.... |
// /* 0040 */ "\x29\x4d\x80\x98\x1a\xdc\x4d\x4a\x34\xf0\x30\x34\x58\xc8\xc8\x18" //)M....MJ4.04X... |
// /* 0050 */ "\x5b\xc8\x33\xb5\x2b\x8b\x37\x86\xf8\x0f\xd2\xfa\x0d\x2e\xc9\xc6" //[.3.+.7......... |
// /* 0060 */ "\x93\x86\x9b\x32\xf6\xa3\x2b\xad\x0d\xb2\xc3\x3d\x60\x40\xb8\x60" //...2..+....=`@.` |
// /* 0070 */ "\xae\xde\x2e\x20\xcb\x75\x3a\x99\x87\x00\x4c\xdc\x7e\x6e\x18\x46" //... .u:...L.~n.F |
// /* 0080 */ "\x86\x80\x16\x74\xfb\xe7\x8d\x4b\x21\x24\xe3\xbb\xda\xec\x7e\xc3" //...t...K!$....~. |
// /* 0090 */ "\x77\x07\x6c\xb3\x09\xe4\x47\x4f\xf0\x23\x14\x08\x0e\x60\xc0\x4b" //w.l...GO.#...`.K |
// /* 00a0 */ "\x73\xae\x9c\x96\xb5\x3a\x51\x58\xd1\x06\x4d\x14\xbb\x7f\xf4\x3b" //s....:QX..M....; |
// /* 00b0 */ "\xc4\xf2\x7c\xeb\x17\x50\x1a\x50\xf6\x68\x5c\x1d\xd0\x15\x4d\xb7" //..|..P.P.h....M. |
// /* 00c0 */ "\x91\x3a\x7f\x17\x5c\xb7\x28\xe3\x57\x4e\xd0\xcd\x27\xb9\xfe\xdf" //.:....(.WN..'... |
// /* 00d0 */ "\x25\x4c\x40\x58\x5a\xd5\xeb\xbe\x94\xf6\x88\x0d\x0e\xdc\x7b\xd5" //%L@XZ.........{. |
// /* 00e0 */ "\x5e\xe3\x0b\xb3\xbe\x4d\x6f\xc7\x2e\xff\xb2\xda\x54\x1a\xc4\x8d" //^....Mo.....T... |
// /* 00f0 */ "\x85\x09\xcd\x08\x27\x65\x54\x2d\xc7\x4f\x8f\xb4\xe1\xcc\x7b\xc2" //....'eT-.O....{. |
// /* 0000 */ "\xe2\x90\x8e\xa6\xbc\x74\x8a\xcb\xc5\xe3\x43\x39\x7b\xe5\x4d\x36" //.....t....C9{.M6 |
// /* 0010 */ "\x2f\x5e\x27\xf5\x41\xae\xe0\x74\x85\x65\x02\x3b\x4c\xfe\xf0\x1b" ///^'.A..t.e.;L... |
// /* 0020 */ "\x76\xb3\xf0\x5a\x48\x64\x3d\x73\x8f\x2e\xb0\x65\x13\xbc\xa0\x3b" //v..ZHd=s...e...; |
// /* 0030 */ "\xda\x08\x7d\x2e\x9a\x3c\x8b\x96\xc1\xf5\xc8\x16\xe5\x0a\x1c\x75" //..}..<.........u |
// /* 0040 */ "\x7f\x1c\x9d\x0c\x19\xbd\x42\x8c\x2d\x89\x35\x73\xd6\x15\x10\xfb" //......B.-.5s.... |
// /* 0050 */ "\xe1\xef\x5c\x10\xc9\x67\x04\xc7\x7e\xe6\x5a\xdd\xea\x92\x7a\x9c" //.....g..~.Z...z. |
// /* 0060 */ "\x83\xb6\xe3\x71\xe2\xc9\x54\x30\xbe\x10\xa2\x88\x67\xd5\xd3\x99" //...q..T0....g... |
// /* 0070 */ "\x6f\x7a\x24\xab\xf5\xc6\xe6\x56\xb2\x8a\x60\x28\xe1\x9a\x51\xc3" //oz$....V..`(..Q. |
// /* 0080 */ "\x82\x77\x29\x47\xc3\x9c\xb7\xdc\x74\x4c\x41\xff\xb4\x9a\xde\xa7" //.w)G....tLA..... |
// /* 0090 */ "\x33\x3d\x0f\x2e\x22\x99\x7d\x15\x27\xcd\x77\x6e\x37\x2f\x77\x53" //3=..".}.'.wn7/wS |
// /* 00a0 */ "\xf6\xf5\x9c\x81\xae\xdc\x7e\x68\x94\x1e\xf8\xcd\x9d\x89\xec\xd1" //......~h........ |
// /* 00b0 */ "\x1e\x9d\xd8\x03\x0a\xc8\xea\x2c\xcc\xec\x1d\x98\x6b\x56\x7a\x96" //.......,....kVz. |
// /* 00c0 */ "\xab\x36\xad\xc9\xa9\xf1\x4a\x21\x4b\x96\xb1\xb8\x34\xdc\xbe\x92" //.6....J!K...4... |
// /* 00d0 */ "\x35\xab\x08\xf8\x8d\x4e\x08\xa5\x27\x4d\x6f\x8a\xad\x59\x03\xaa" //5....N..'Mo..Y.. |
// /* 00e0 */ "\xff\xba\x90\x5a\x41\xe8\x93\x81\xc3\x1b\x0f\x60\x71\x11\x41\xfc" //...ZA......`q.A. |
// /* 00f0 */ "\x7c\xd4\x45\x11\x6e\x65\x90\x52\xfc\xbd\x9d\x8f\x4a\xf1\x66\x9b" //|.E.ne.R....J.f. |
// /* 0000 */ "\x16\x93\xfd\x4c\x65\x8e\x78\xa2\x25\x8f\xe0\xb0\x2e\xd3\x2f\x80" //...Le.x.%...../. |
// /* 0010 */ "\x2e\xe8\x98\xcc\xbf\xe1\x6e\xde\x54\x1b\xf1\x92\x2d\x57\x4b\x44" //......n.T...-WKD |
// /* 0020 */ "\x71\x49\xdf\x4e\x90\x36\x0d\x32\x7c\xcb\x35\xc4\x4f\x04\xfd\x14" //qI.N.6.2|.5.O... |
// /* 0030 */ "\x68\x8d\x91\x7d\xcf\x3e\xf9\xa0\x9e\xf5\xf0\x3f\xa4\x20\xe7\x3f" //h..}.>.....?. .? |
// /* 0040 */ "\x6f\x59\x3a\x98\xae\xee\xfc\x41\x0a\x24\x0c\x68\xe2\x15\x23\x25" //oY:....A.$.h..#% |
// /* 0050 */ "\x32\xcc\xbc\x7b\x12\x70\xd4\x76\x67\x92\xd1\xbf\xb3\x73\x44\xdf" //2..{.p.vg....sD. |
// /* 0060 */ "\x93\x5a\x99\x73\xf2\x05\x5e\x4b\x7d\x9f\x53\x5f\x91\x4c\xea\x17" //.Z.s..^K}.S_.L.. |
// /* 0070 */ "\xa9\x6a\xad\x87\x75\x8a\x23\xc4\x33\xb8\xdd\x6f\xb6\xbc\x1c\xc9" //.j..u.#.3..o.... |
// /* 0080 */ "\x70\x3c\xe4\x15\xb2\x97\xaf\xaa\xd9\xaf\x33\xfe\xc7\xe7\x63\xed" //p<........3...c. |
// /* 0090 */ "\xb0\xd4\x56\x74\x62\xb3\x63\x40\x3e\x2c\xd6\xc9\x4b\x1b\x28\x30" //..Vtb.c@>,..K.(0 |
// /* 00a0 */ "\xf3\x5b\x2e\xd5\xb5\x85\xb5\xf9\xb9\xae\xfe\x69\xfe\x28\x9a\xfb" //.[.........i.(.. |
// /* 00b0 */ "\x66\x6a\xaa\x0e\x67\x0d\x0c\xb3\xba\xcb\xa4\x1a\xf0\xba\xe9\x1b" //fj..g........... |
// /* 00c0 */ "\x5d\x61\x01\x11\xe8\xcc\xc0\xe1\x3b\x46\xd7\xff\x67\x65\xba\xe1" //]a......;F..ge.. |
// /* 00d0 */ "\x03\x89\x10\xc5\x02\xd8\x75\xc1\x61\x19\x2f\x17\x5f\x73\x13\x83" //......u.a./._s.. |
// /* 00e0 */ "\x5b\x6e\x43\x1b\xe8\x4d"                                         //[nC..M |
// Dump done on RDP Wab Target (4) 2534 bytes |
// LOOPING on PDUs: 2511 |
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
// order(10):opaquerect(rect(0,87,15,66) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,87,15,66) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,153,1024,549) color=0x0008ec) |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(20,10,108,17) op=(20,10,108,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=26 data_len=26 [0x00 0x00 0x01 0x09 0x02 0x0e 0x03 0x09 0x04 0x09 0x02 0x04 0x03 0x09 0x04 0x09 0x05 0x04 0x06 0x09 0x04 0x09 0x06 0x04 0x06 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,41,204,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(20,41,93,17) op=(20,41,93,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=57 data_len=24 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09 0x0d 0x05 0x0e 0x04 0x09 0x0b 0x0f 0x05 0x10 0x09 0x11 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,41,706,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(224,41,46,17) op=(224,41,46,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=57 data_len=12 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,41,84,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(930,41,58,17) op=(930,41,58,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=57 data_len=16 [0x12 0x00 0x09 0x09 0x0f 0x05 0x0c 0x09 0x0f 0x05 0x13 0x09 0x0f 0x08 0x14 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(963,13,34,17) op=(963,13,34,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=962 glyph_y=29 data_len=12 [0x15 0x00 0x16 0x08 0x14 0x03 0x0c 0x03 0x0b 0x05 0x09 0x09] |
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
// order(10):opaquerect(rect(15,87,994,22) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,89,200,18) color=0x000273) |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(20,90,86,17) op=(20,90,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=106 data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x21 0x0b 0x22 0x09 0x23 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,89,702,18) color=0x000273) |
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
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(224,90,254,17) op=(224,90,254,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=106 data_len=66 [0x24 0x00 0x25 0x09 0x26 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x27 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x20 0x0e 0x21 0x0b 0x22 0x09 0x23 0x08 0x28 0x09 0x11 0x05 0x09 0x09 0x16 0x05 0x28 0x03 0x05 0x05 0x29 0x09 0x04 0x09 0x06 0x04 0x2a 0x09 0x2b 0x09 0x2c 0x05 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,89,80,18) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(930,90,31,17) op=(930,90,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=106 data_len=6 [0x2c 0x00 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,109,994,22) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,111,200,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(20,112,95,17) op=(20,112,95,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=128 data_len=22 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x21 0x0b 0x22 0x09 0x02 0x08 0x21 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,111,702,18) color=0x00ef7e) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(224,112,296,17) op=(224,112,296,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=128 data_len=70 [0x2c 0x00 0x2e 0x0a 0x2d 0x09 0x2f 0x0b 0x07 0x05 0x08 0x09 0x09 0x08 0x0c 0x05 0x0b 0x05 0x26 0x09 0x11 0x0d 0x16 0x09 0x0f 0x03 0x1e 0x09 0x01 0x09 0x20 0x0e 0x21 0x0b 0x22 0x09 0x02 0x08 0x21 0x09 0x28 0x09 0x11 0x05 0x10 0x09 0x30 0x09 0x28 0x09 0x05 0x05 0x29 0x09 0x04 0x09 0x02 0x04 0x21 0x09 0x23 0x09 0x2b 0x09 0x2c 0x05 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,111,80,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(930,112,31,17) op=(930,112,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=128 data_len=6 [0x2c 0x00 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,131,994,22) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,133,200,18) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(20,134,107,17) op=(20,134,107,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=150 data_len=26 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x1e 0x07 0x0f 0x09 0x0c 0x09 0x0b 0x05 0x11 0x09 0x08 0x09 0x25 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,133,702,18) color=0x00cebd) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(224,134,230,17) op=(224,134,230,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=150 data_len=56 [0x08 0x00 0x13 0x08 0x13 0x08 0x0f 0x08 0x10 0x09 0x1e 0x09 0x0c 0x09 0x1f 0x05 0x31 0x07 0x08 0x04 0x22 0x08 0x0b 0x08 0x01 0x09 0x08 0x0e 0x11 0x08 0x11 0x09 0x1f 0x09 0x1e 0x07 0x0f 0x09 0x0c 0x09 0x0b 0x05 0x11 0x09 0x08 0x09 0x25 0x08 0x2b 0x09 0x24 0x05 0x12 0x09 0x12 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,133,80,18) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(930,134,28,17) op=(930,134,28,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=150 data_len=6 [0x24 0x00 0x12 0x09 0x12 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(787,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(788,703,30,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(794,705,19,17) op=(794,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=793 glyph_y=721 data_len=4 [0x17 0x00 0x18 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(835,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(836,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(842,705,12,17) op=(842,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=841 glyph_y=721 data_len=2 [0x17 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(877,704,23,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=876 glyph_y=721 data_len=2 [0x02 0x00] |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=900 glyph_y=721 data_len=4 [0x19 0x00 0x02 0x05] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(930,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(931,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(937,705,12,17) op=(937,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=936 glyph_y=721 data_len=2 [0x1a 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(971,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(972,703,30,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(978,705,19,17) op=(978,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=977 glyph_y=721 data_len=4 [0x1b 0x00 0x1a 0x07] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,63,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(811,736,60,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(817,738,49,17) op=(817,738,49,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=816 glyph_y=754 data_len=12 [0x1c 0x00 0x0f 0x07 0x0a 0x09 0x0f 0x09 0x10 0x09 0x0c 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(920,738,60,17) op=(920,738,60,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=919 glyph_y=754 data_len=14 [0x1d 0x00 0x0f 0x0a 0x1e 0x09 0x1e 0x09 0x0b 0x09 0x13 0x09 0x0c 0x08] |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x65\x08\x00\x00\x00\xbf\x84" //...h. ..pe...... |
// /* 0010 */ "\xdb\x0f\xb7\x8f\x6f\xe3\xbb\xe6\x1e\x1d\x48\x0a\xbb\x7d\x86\x49" //....o.....H..}.I |
// /* 0020 */ "\x4b\x54\x02\x1c\xa3\x23\x82\xf4\x74\xd6\xc3\x75\x5b\x32\x5f\x09" //KT...#..t..u[2_. |
// /* 0030 */ "\x51\x24\x2a\xb0\x42\x99\x0b\x43\xaf\x49\x77\xbc\x86\x50\x20\x7c" //Q$*.B..C.Iw..P | |
// /* 0040 */ "\x11\xeb\x57\xc9\xa1\x76\x42\x12\xe5\xed\xcc\x6c\xce\x7f\x50\x1b" //..W..vB....l..P. |
// /* 0050 */ "\x2d\xc1\x4d\x27\xfa\x1e\x5d\xbd\x83\x36\x1b\x3d\x9b\x1f\x32\x23" //-.M'..]..6.=..2# |
// /* 0060 */ "\x72\x90\x44\xf6\x46\xcf\x56\x94\xdb\xda\xab\x08\x4f\xcb\xb1"     //r.D.F.V.....O.. |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=876 glyph_y=721 data_len=2 [0x02 0x00] |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x48\x08\x00\x00\x00\x89\xfa" //...h. ..pH...... |
// /* 0010 */ "\xef\xa1\x71\x6e\xac\x6b\x0d\xcc\xa5\x2c\xab\x8f\xee\x6d\x05\xea" //..qn.k...,...m.. |
// /* 0020 */ "\xb5\x8c\x0a\x4c\x83\x09\x53\x72\x03\x00\xaa\xc6\x02\x77\xe2\x59" //...L..Sr.....w.Y |
// /* 0030 */ "\xb8\x68\x0f\x56\x4b\x79\x33\x61\x1a\x1b\xc7\x48\x8d\x27\x79\x38" //.h.VKy3a...H.'y8 |
// /* 0040 */ "\xff\xd2\x4a\xab\x8c\xac\x70\x30\x3c\x45\x94\x3b\xe4\x6c\xf7\x84" //..J...p0<E.;.l.. |
// /* 0050 */ "\xa1\xf2"                                                         //.. |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=900 glyph_y=721 data_len=4 [0x19 0x00 0x02 0x05] |
// ======================================== |
// process_orders done |
// ===================> count = 8 |
// ~mod_rdp(): Recv bmp cache count  = 0 |
// ~mod_rdp(): Recv order count      = 402 |
// ~mod_rdp(): Recv bmp update count = 0 |
// RDP Wab Target (0): total_received=9060, total_sent=1807 |
} /* end outdata */;

const char indata[] =
{
// connecting to 10.10.47.236:3389 |
// connection to 10.10.47.236:3389 succeeded : socket 4 |
// --------- CREATION OF MOD ------------------------ |
// Creation of new mod 'RDP' |
// ModRDPParams target_user="x" |
// ModRDPParams target_password="<hidden>" |
// ModRDPParams target_host="10.10.47.236" |
// ModRDPParams client_address="192.168.1.100" |
// ModRDPParams client_name="<null>" |
// ModRDPParams enable_tls=no |
// ModRDPParams enable_nla=no |
// ModRDPParams enable_krb=no |
// ModRDPParams enable_fastpath=no |
// ModRDPParams enable_mem3blt=no |
// ModRDPParams enable_bitmap_update=no |
// ModRDPParams enable_new_pointer=no |
// ModRDPParams enable_glyph_cache=no |
// ModRDPParams enable_wab_agent=no |
// ModRDPParams wab_agent_launch_timeout=0 |
// ModRDPParams wab_agent_on_launch_failure=0 |
// ModRDPParams wab_agent_keepalive_timeout=0 |
// ModRDPParams dsiable_clipboard_log=0x0 |
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
// ModRDPParams allow_channels=<none> |
// ModRDPParams deny_channels=<none> |
// ModRDPParams remote_program=no |
// ModRDPParams server_redirection_support=yes |
// ModRDPParams bogus_sc_net_size=yes |
// ModRDPParams client_device_announce_timeout=1000 |
// ModRDPParams proxy_managed_drives= |
// ModRDPParams verbose=0x000001FF |
// ModRDPParams cache_verbose=0x00000000 |
// RDP Extra orders="" |
// Remote RDP Server domain="" login="x" host="192-168-1-100" |
// Server key layout is 40c |
// Init with Redir_info: RedirectionInfo(valid=false, session_id=0, host='', username='', password='<null>', domain='', LoadBalanceInfoLength=0, dont_store_username=false, server_tsv_capable=false, smart_card_logon=false) |
// ServerRedirectionSupport=true |
// mod_rdp::Early TLS Security Exchange |
// RdpNego::NEGO_STATE_INITIAL |
// RdpNego::send_x224_connection_request_pdu |
// Send cookie: |
// /* 0000 */ "\x43\x6f\x6f\x6b\x69\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68" //Cookie: mstshash |
// /* 0010 */ "\x3d\x78\x0d\x0a"                                                 //=x.. |
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
// CS_Cluster: Server Redirection Supported |
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
// /* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\xca\x01\x00" //................ |
// /* 0010 */ "\x00\x00\x00\x00\x10\x00\x07\x00\x01\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0040 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0060 */ "\x04\xc0\x0c\x00\x09\x00\x00\x00\x00\x00\x00\x00\x02\xc0\x0c\x00" //................ |
// /* 0070 */ "\x03\x00\x00\x00\x00\x00\x00\x00"                                 //........ |
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
/* 0060 */ "\x00\x20\x00\x00\x00\xb8\x00\x00\x00\x6e\x4a\x14\xdd\xb1\xb1\xd6" //. .......nJ..... |
/* 0070 */ "\xe4\x3b\x7c\x21\x16\xe4\xa6\x46\x1f\xcb\x4b\x8e\x0e\x04\x87\x84" //.;|!...F..K..... |
/* 0080 */ "\xea\x37\x21\x7b\x76\x72\x97\x90\x02\x01\x00\x00\x00\x01\x00\x00" //.7!{vr.......... |
/* 0090 */ "\x00\x01\x00\x00\x00\x06\x00\x5c\x00\x52\x53\x41\x31\x48\x00\x00" //.........RSA1H.. |
/* 00a0 */ "\x00\x00\x02\x00\x00\x3f\x00\x00\x00\x01\x00\x01\x00\x67\xab\x0e" //.....?.......g.. |
/* 00b0 */ "\x6a\x9f\xd6\x2b\xa3\x32\x2f\x41\xd1\xce\xee\x61\xc3\x76\x0b\x26" //j..+.2/A...a.v.& |
/* 00c0 */ "\x11\x70\x48\x8a\x8d\x23\x81\x95\xa0\x39\xf7\x5b\xaa\x3e\xf1\xed" //.pH..#...9.[.>.. |
/* 00d0 */ "\xb8\xc4\xee\xce\x5f\x6a\xf5\x43\xce\x5f\x60\xca\x6c\x06\x75\xae" //...._j.C._`.l.u. |
/* 00e0 */ "\xc0\xd6\xa4\x0c\x92\xa4\xc6\x75\xea\x64\xb2\x50\x5b\x00\x00\x00" //.......u.d.P[... |
/* 00f0 */ "\x00\x00\x00\x00\x00\x08\x00\x48\x00\x6a\x41\xb1\x43\xcf\x47\x6f" //.......H.jA.C.Go |
/* 0000 */ "\xf1\xe6\xcc\xa1\x72\x97\xd9\xe1\x85\x15\xb3\xc2\x39\xa0\xa6\x26" //....r.......9..& |
/* 0010 */ "\x1a\xb6\x49\x01\xfa\xa6\xda\x60\xd7\x45\xf7\x2c\xee\xe4\x8e\x64" //..I....`.E.,...d |
/* 0020 */ "\x2e\x37\x49\xf0\x4c\x94\x6f\x08\xf5\x63\x4c\x56\x29\x55\x5a\x63" //.7I.L.o..cLV)UZc |
/* 0030 */ "\x41\x2c\x20\x65\x95\x99\xb1\x15\x7c\x00\x00\x00\x00\x00\x00\x00" //A, e....|....... |
/* 0040 */ "\x00"                                                             //. |
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
/* 0000 */ "\x02\xf0\x80\x2e\x00\x00\x20"                                     //......  |
// Dump done on RDP Wab Target (4) 7 bytes |
// cjrq[0] = 1033 |
// Sending on RDP Wab Target (4) 12 bytes |
// /* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x20\x04\x09"                 //.......8. .. |
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
/* 0000 */ "\x02\xf0\x80\x3e\x00\x00\x20\x04\x09\x04\x09"                     //...>.. .... |
// Dump done on RDP Wab Target (4) 11 bytes |
// cjcf[0] = 1033 |
// cjrq[1] = 1003 |
// Sending on RDP Wab Target (4) 12 bytes |
// /* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x20\x03\xeb"                 //.......8. .. |
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
/* 0000 */ "\x02\xf0\x80\x3e\x00\x00\x20\x03\xeb\x03\xeb"                     //...>.. .... |
// Dump done on RDP Wab Target (4) 11 bytes |
// cjcf[1] = 1003 |
// mod_rdp::RDP Security Commencement |
// mod_rdp::SecExchangePacket keylen=64 |
// send data request |
// Sending on RDP Wab Target (4) 94 bytes |
// /* 0000 */ "\x03\x00\x00\x5e\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x50\x01\x00" //...^...d. ..pP.. |
// /* 0010 */ "\x00\x00\x48\x00\x00\x00\xa4\xbe\x34\x2c\x91\x29\xd3\x65\x51\xf7" //..H.....4,.).eQ. |
// /* 0020 */ "\xeb\xeb\xdf\x64\x31\x50\x0e\x1c\xb2\xfb\xbf\x02\x59\x28\x14\x0c" //...d1P......Y(.. |
// /* 0030 */ "\x7b\x5e\x34\x95\x25\x19\xd6\xac\x5a\x5b\xf5\x90\xe5\x64\x10\x5e" //{^4.%...Z[...d.^ |
// /* 0040 */ "\x77\x0c\xea\x1b\x4b\x49\x07\xd3\x02\xe6\xed\xee\xc6\x30\x2e\x2d" //w...KI.......0.- |
// /* 0050 */ "\x8a\x53\x55\x1d\x28\x2a\x00\x00\x00\x00\x00\x00\x00\x00"         //.SU.(*........ |
// Sent dumped on RDP Wab Target (4) 94 bytes |
// send data request done |
// mod_rdp::Secure Settings Exchange |
// mod_rdp::send_client_info_pdu |
// send extended login info (RDP5-style) 1017b :x |
// Send data request InfoPacket |
// InfoPacket::CodePage 0 |
// InfoPacket::flags 0x1017b |
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
// send data request |
// Sending on RDP Wab Target (4) 339 bytes |
// /* 0000 */ "\x03\x00\x01\x53\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x81\x44\x48" //...S...d. ..p.DH |
// /* 0010 */ "\x00\x00\x00\x8e\xc0\x72\xeb\x87\x29\xd2\x6e\x31\xe3\x78\x37\xad" //.....r..).n1.x7. |
// /* 0020 */ "\x9e\x8d\xf0\x76\x11\xb6\xb4\xc6\xc8\xa8\xc5\x8a\x46\x47\xb0\xd8" //...v........FG.. |
// /* 0030 */ "\xeb\xad\xea\x71\x8f\x47\xd7\xbe\x8d\x26\xe4\xce\xa7\xd6\x41\x49" //...q.G...&....AI |
// /* 0040 */ "\x60\x0e\xc7\x54\x58\xcc\x16\xd2\x9a\xa3\xbc\x36\x7a\xbd\x08\x9e" //`..TX......6z... |
// /* 0050 */ "\xaa\x70\xf2\x83\x9d\x46\x4c\x02\x71\xbd\xe5\x4d\x82\x93\xd9\x3a" //.p...FL.q..M...: |
// /* 0060 */ "\x20\x82\x55\xdf\x9c\x61\x15\xc0\x1b\x03\x6c\xc7\x01\x89\x81\x29" // .U..a....l....) |
// /* 0070 */ "\x4b\xdb\xa8\x12\x94\x78\x2a\xd5\x90\x1b\x13\x97\xa0\x01\xbc\xf7" //K....x*......... |
// /* 0080 */ "\xb4\x84\x3c\x0f\xa3\x66\xbe\x8c\x09\x32\x94\x11\x59\x90\x85\x06" //..<..f...2..Y... |
// /* 0090 */ "\x59\x4e\xd5\x54\xa8\xba\x39\x01\xef\x16\x78\x82\x90\x53\x38\xcf" //YN.T..9...x..S8. |
// /* 00a0 */ "\x6e\x78\xab\x49\x67\x34\x87\x63\x7c\xb3\xa7\x97\xb8\xdf\x7e\x03" //nx.Ig4.c|.....~. |
// /* 00b0 */ "\x27\xd1\xfd\x04\xfa\x3f\x1c\x52\x3f\x47\x8d\x41\x9f\xb0\x6c\xd3" //'....?.R?G.A..l. |
// /* 00c0 */ "\xbb\x36\xb7\x42\xad\x29\x40\x03\xd2\x61\x71\x1d\x81\x26\xd6\x1b" //.6.B.)@..aq..&.. |
// /* 00d0 */ "\xe7\xe6\x15\xdb\xb2\x90\x66\x78\x00\xb8\x7d\xe9\x62\x56\x3a\x6c" //......fx..}.bV:l |
// /* 00e0 */ "\x8d\x11\xb0\x22\xd7\xf5\xaa\x13\x89\xe4\x90\x2b\xc5\xe9\xe3\x70" //...".......+...p |
// /* 00f0 */ "\x2c\xd6\xda\xf5\x4f\xcc\x49\x7c\xb3\x03\x8d\xa9\x4f\x6f\x72\xb7" //,...O.I|....Oor. |
// /* 0000 */ "\x8d\xb1\xf8\xd8\xa5\x6a\x69\xcb\xcb\x80\xec\xb5\xf2\x29\xab\xe9" //.....ji......).. |
// /* 0010 */ "\x48\x25\xbc\xdf\x32\xa1\xb8\xdd\x68\x4e\xa5\x50\xb7\x25\xf4\x35" //H%..2...hN.P.%.5 |
// /* 0020 */ "\x50\x7e\x7b\xd7\xfb\x8d\xc5\xd3\x7d\x37\x6b\x76\x8d\x67\x18\x16" //P~{.....}7kv.g.. |
// /* 0030 */ "\x42\xa1\x8a\xee\x53\x5e\xc9\xae\x6c\xe7\x7d\x55\x81\x42\xbf\x57" //B...S^..l.}U.B.W |
// /* 0040 */ "\x24\x61\x79\xbf\xfe\x71\x71\x27\xa2\x1c\x37\xd1\x87\xee\xd3\x9d" //$ay..qq'..7..... |
// /* 0050 */ "\x0c\x18\x06"                                                     //... |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x81\x42\x80\x00\x00\x00\x01" //...h. ..p.B..... |
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
/* 0000 */ "\xb1\xf0\x54\x3b\x5e\x3e\x6a\x71\xb4\xf7\x75\xc8\x16\x2f\x24\x00" //..T;^>jq..u../$. |
/* 0010 */ "\xde\xe9\x82\x99\x5f\x33\x0b\xa9\xa6\x94\xaf\xcb\x11\xc3\xf2\xdb" //...._3.......... |
/* 0020 */ "\x09\x42\x68\x29\x56\x58\x01\x56\xdb\x59\x03\x69\xdb\x7d\x37\x00" //.Bh)VX.V.Y.i.}7. |
/* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x0e\x00\x0e\x00\x6d" //...............m |
/* 0040 */ "\x69\x63\x72\x6f\x73\x6f\x66\x74\x2e\x63\x6f\x6d\x00"             //icrosoft.com. |
// Dump done on RDP Wab Target (4) 333 bytes |
// Rdp::License Request |
// send data request |
// Sending on RDP Wab Target (4) 163 bytes |
// /* 0000 */ "\x03\x00\x00\xa3\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x80\x94\x80" //.......d. ..p... |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x14\x80\x00\x10\x00\xff\x02" //...h. ..p....... |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x81\x34\x08\x00\x00\x00\x96" //...h. ..p.4..... |
/* 0010 */ "\xd8\xde\xdc\xab\x9a\x1e\xc1\xdb\xb2\xe3\x1c\x0f\xd6\x86\xba\x12" //................ |
/* 0020 */ "\x8c\xde\x75\x8e\x85\xce\x7d\x43\xac\x25\xd2\xe0\xcc\xa1\xb9\xe2" //..u...}C.%...... |
/* 0030 */ "\x27\xf6\xd3\x88\x44\x11\xee\xdc\xec\x83\xdd\x29\x2e\x1a\xcf\xac" //'...D......).... |
/* 0040 */ "\xe4\x92\x16\x32\x89\x20\x69\xc2\x49\xcf\x13\xc6\x38\x17\x1c\x38" //...2. i.I...8..8 |
/* 0050 */ "\x6e\x74\xc9\x9d\x04\x59\x1b\x70\xae\x1d\x0d\xbb\x4e\xc3\xaf\x4f" //nt...Y.p....N..O |
/* 0060 */ "\x7f\x97\x27\x35\x84\xd8\x2c\x07\xd7\xe9\xa3\x00\xba\x7d\x86\x3c" //..'5..,......}.< |
/* 0070 */ "\x49\x9b\x11\x6e\xde\x4e\xc6\x91\x57\xfd\x99\xa8\x40\x16\x27\xe7" //I..n.N..W...@.'. |
/* 0080 */ "\x12\xe2\xdb\x5f\x46\x23\x6d\x79\x24\x48\xee\xf9\x84\x0e\x8c\xba" //..._F#my$H...... |
/* 0090 */ "\xe7\xd5\x1f\x52\x3d\xc4\xe5\x78\x94\xc5\x8c\x3d\x38\xbd\x31\xb8" //...R=..x...=8.1. |
/* 00a0 */ "\xa5\xd0\xce\xc7\x24\x56\x71\x70\xe0\xae\x25\x13\x5c\x1f\x1c\xcb" //....$Vqp..%..... |
/* 00b0 */ "\x3e\x44\x8f\xda\xeb\x2d\x0a\xdc\xd4\xb6\x08\x9f\xdc\x28\xef\x0c" //>D...-.......(.. |
/* 00c0 */ "\xd4\xff\xd1\x3b\x62\x20\x21\xc1\x9e\x84\x7e\x23\x3e\xc8\x64\x45" //...;b !...~#>.dE |
/* 00d0 */ "\x37\xf0\x2f\xe2\x1a\x29\xe7\x17\x60\xe3\xea\xd5\xd2\x5d\x70\x80" //7./..)..`....]p. |
/* 00e0 */ "\x4c\x07\x8b\x70\xc8\x34\xe5\x76\xc6\x9a\xd6\xfa\x0c\x32\x7f\x7a" //L..p.4.v.....2.z |
/* 00f0 */ "\xfa\x3f\xd5\x84\x18\xe1\xb0\x2f\x8b\xe5\xd7\x5b\x89\x07\xf8\xf1" //.?...../...[.... |
/* 0000 */ "\x75\xdf\x64\x31\x2b\x5c\x3d\x85\xe5\xf6\x63\xb6\x5e\x26\x93\xc8" //u.d1+.=...c.^&.. |
/* 0010 */ "\x2c\x7e\x86\x23\xcc\x6a\xbe\x45\x90\x13\x0b\x72\x30\x3a\x72\x22" //,~.#.j.E...r0:r" |
/* 0020 */ "\x4e\x0e\x05\x27\xca\x83\x53\x38\x28\x40\x05\x8a\xbf\x8d\x26\xda" //N..'..S8(@....&. |
/* 0030 */ "\x31\xb2\x7c\x3c\x0b\x77\xea\x48\xb8\x40\xf2\xb3\x4b\x0d\xf9"     //1.|<.w.H.@..K.. |
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
// Unprocessed Capability Set is encountered. capabilitySetType=Font Capability Set(14) |
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
// Unprocessed Capability Set is encountered. capabilitySetType=Bitmap Cache Host Support Capability Set(18) |
// Unprocessed Capability Set is encountered. capabilitySetType=Color Table Cache Capability Set(10) |
// Unprocessed Capability Set is encountered. capabilitySetType=Pointer Capability Set(8) |
// Unprocessed Capability Set is encountered. capabilitySetType=Share Capability Set(9) |
// Received from server Input caps (88 bytes) |
// Input caps::inputFlags 0x29 |
// Input caps::pad2octetsA 0 |
// Input caps::keyboardLayout 0 |
// Input caps::keyboardType 0 |
// Input caps::keyboardSubType 0 |
// Input caps::keyboardFunctionKey 0 |
// Input caps::imeFileName 2319485328 |
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
// Input caps::inputFlags 0x1 |
// Input caps::pad2octetsA 0 |
// Input caps::keyboardLayout 1033 |
// Input caps::keyboardType 4 |
// Input caps::keyboardSubType 0 |
// Input caps::keyboardFunctionKey 12 |
// Input caps::imeFileName 2319288560 |
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
// /* 0000 */ "\x03\x00\x01\xb4\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x81\xa5\x08" //.......d. ..p... |
// /* 0010 */ "\x00\x00\x00\x4f\x2a\xa3\x7e\x08\x50\x3e\x04\xe4\x13\xb6\xa8\x3e" //...O*.~.P>.....> |
// /* 0020 */ "\x2d\x7b\xc0\x5b\xf9\xea\x76\xe7\x75\x47\x79\x6b\x34\xee\xd7\x93" //-{.[..v.uGyk4... |
// /* 0030 */ "\x7d\x30\xe3\x86\xca\x89\xee\xa1\xdc\x62\x42\x6c\xcd\x36\x97\x10" //}0.......bBl.6.. |
// /* 0040 */ "\x33\xe9\xff\x82\xae\x63\xfb\x49\xf5\xdd\x4f\x35\xf3\x07\x1f\x85" //3....c.I..O5.... |
// /* 0050 */ "\x74\xbe\x92\xb8\x61\xae\x53\xc6\x1d\xfd\x81\x3c\x2c\xb4\x5f\x21" //t...a.S....<,._! |
// /* 0060 */ "\x27\x21\xae\x27\x5c\x3f\xfa\x8f\x47\x98\x95\x4d\x4d\x78\xcb\x64" //'!.'.?..G..MMx.d |
// /* 0070 */ "\xb6\xcd\x81\x86\x04\x29\xf8\x81\x24\x91\x7c\x70\xd2\xdd\x18\x95" //.....)..$.|p.... |
// /* 0080 */ "\xf8\x2d\x6f\xeb\x01\x96\x76\x3c\x23\x8f\x25\xf0\xb8\xca\xf5\x64" //.-o...v<#.%....d |
// /* 0090 */ "\x52\x25\xed\xe0\x0f\x57\xfa\x1c\x06\x9b\x15\x81\x15\x28\x78\x9b" //R%...W.......(x. |
// /* 00a0 */ "\xad\xa0\x18\x93\xc7\x0d\x1a\x87\xe9\x0e\x3d\x90\xe7\xc6\x8e\xb2" //..........=..... |
// /* 00b0 */ "\x0e\xe8\xa6\xd3\x87\xe3\x22\xcb\xb3\x68\x11\xb2\x3b\x58\x7f\x3f" //......"..h..;X.? |
// /* 00c0 */ "\xf5\x57\xb0\xf6\x66\x75\x9c\xa1\xb6\x28\x53\x04\x69\x82\x3b\x36" //.W..fu...(S.i.;6 |
// /* 00d0 */ "\xc7\xc1\x64\xc3\x3d\xc9\xb0\x4d\x16\x24\x39\x2d\x9b\x8d\xf7\x5e" //..d.=..M.$9-...^ |
// /* 00e0 */ "\x50\xa3\xbc\xeb\xb0\x5b\xf5\xc8\x44\xac\x50\xeb\x35\x07\x4a\xe3" //P....[..D.P.5.J. |
// /* 00f0 */ "\xec\x85\x31\x8a\xf9\x17\x96\xd2\x64\xca\x6f\x6c\xb7\x97\x45\x79" //..1.....d.ol..Ey |
// /* 0000 */ "\xbd\x55\xa1\xc2\x61\x5e\x49\x94\xea\xe4\xf2\xda\x95\x6d\x65\xd7" //.U..a^I......me. |
// /* 0010 */ "\x69\x43\xee\x23\xe0\x74\x96\xd4\x09\x68\x98\xcf\x04\xb9\xc6\xff" //iC.#.t...h...... |
// /* 0020 */ "\x4d\x82\x2c\x69\x33\x2b\x40\xbf\x54\xa6\xa4\x17\x8d\x90\x9b\xb7" //M.,i3+@.T....... |
// /* 0030 */ "\x92\xd0\xeb\x8d\xc0\xcb\xfa\xb7\xe0\x14\xef\xd4\xd8\xd8\x56\x09" //..............V. |
// /* 0040 */ "\x43\x84\xea\xf0\x2a\x41\x73\x0a\x11\x3e\x70\x25\x70\xb8\x6a\x9c" //C...*As..>p%p.j. |
// /* 0050 */ "\x26\x67\xb6\x15\x36\x0b\xdf\x05\xf3\x21\xdd\x0e\x5d\xf2\x3d\x8b" //&g..6....!..].=. |
// /* 0060 */ "\x3b\xcc\x78\x73\x95\xc6\xaa\x0a\x62\x14\xc2\x8e\x8e\x93\x6d\xe2" //;.xs....b.....m. |
// /* 0070 */ "\xe9\x6f\xfb\x34\x39\xfb\x6e\xff\x7d\xea\xe5\x67\x06\xb1\x76\xc4" //.o.49.n.}..g..v. |
// /* 0080 */ "\x52\xef\xb1\x21\xdb\x2f\x35\xf0\x2a\xc5\xb6\x5e\x74\x80\x4d\x9e" //R..!./5.*..^t.M. |
// /* 0090 */ "\x50\xe1\x26\x99\xee\x3b\xa8\x85\xb1\x7b\x25\xda\x73\x18\x44\x39" //P.&..;...{%.s.D9 |
// /* 00a0 */ "\xe2\x79\x3c\xee\x66\xd9\x6c\xc3\xe1\x49\x76\xce\xcc\x03\x01\x91" //.y<.f.l..Iv..... |
// /* 00b0 */ "\x1a\x71\x98\x53"                                                 //.q.S |
// Sent dumped on RDP Wab Target (4) 436 bytes |
// mod_rdp::send_confirm_active done |
// Waiting for answer to confirm active |
// mod_rdp::send_synchronise |
// Sending on RDP Wab Target (4) 48 bytes |
// /* 0000 */ "\x03\x00\x00\x30\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x22\x08\x00" //...0...d. ..p".. |
// /* 0010 */ "\x00\x00\xe9\xc4\x2f\x5a\x09\xbc\xb8\x9a\x8f\x5d\xd8\xba\x02\x00" //..../Z.....].... |
// /* 0020 */ "\x88\x2a\xc5\xf0\xc2\x34\x86\x87\x8a\x62\x19\x16\x69\x60\x71\x42" //.*...4...b..i`qB |
// Sent dumped on RDP Wab Target (4) 48 bytes |
// mod_rdp::send_synchronise done |
// mod_rdp::send_control |
// Sending on RDP Wab Target (4) 52 bytes |
// /* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x26\x08\x00" //...4...d. ..p&.. |
// /* 0010 */ "\x00\x00\xf8\xd4\x61\xf8\x7a\xc9\x17\x51\x85\xf2\x16\x04\x35\x7f" //....a.z..Q....5. |
// /* 0020 */ "\x3e\xf5\xe6\x04\x01\xc3\x19\xd1\xa3\x3a\xda\xc6\x53\x4c\x42\x5c" //>........:..SLB. |
// /* 0030 */ "\x08\x5a\x43\x84"                                                 //.ZC. |
// Sent dumped on RDP Wab Target (4) 52 bytes |
// mod_rdp::send_control done |
// mod_rdp::send_control |
// Sending on RDP Wab Target (4) 52 bytes |
// /* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x26\x08\x00" //...4...d. ..p&.. |
// /* 0010 */ "\x00\x00\x8e\x9b\x5b\x89\xce\xef\x2c\xe5\x03\xbe\x1b\x8d\x33\xe6" //....[...,.....3. |
// /* 0020 */ "\xc7\x48\xc9\x90\x80\x10\xa2\x81\xd2\xdb\x84\xff\x2b\x02\xef\x0e" //.H..........+... |
// /* 0030 */ "\x48\x23\xd0\x0f"                                                 //H#.. |
// Sent dumped on RDP Wab Target (4) 52 bytes |
// mod_rdp::send_control done |
// use rdp5 |
// mod_rdp::send_fonts |
// Sending on RDP Wab Target (4) 52 bytes |
// /* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x26\x08\x00" //...4...d. ..p&.. |
// /* 0010 */ "\x00\x00\xc3\x8b\xf0\x06\x89\xd8\x02\x03\x17\xc9\xcd\xae\x67\xd9" //..............g. |
// /* 0020 */ "\xdd\x4c\xe0\x47\x17\xf0\x4c\x76\x7d\x6d\xe5\xdc\xd5\xb9\xd9\x43" //.L.G..Lv}m.....C |
// /* 0030 */ "\x14\x0c\xd4\x36"                                                 //...6 |
// Sent dumped on RDP Wab Target (4) 52 bytes |
// mod_rdp::send_fonts done |
// mod_rdp::send_input_slowpath |
// Sending on RDP Wab Target (4) 60 bytes |
// /* 0000 */ "\x03\x00\x00\x3c\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x2e\x08\x00" //...<...d. ..p... |
// /* 0010 */ "\x00\x00\xb3\xf0\x7b\x7d\x89\x95\x69\xd8\xc6\x76\x24\xfb\x07\x21" //....{}..i..v$..! |
// /* 0020 */ "\x33\xbc\xe7\x20\x9c\xc5\xa1\xa5\x91\xc1\xd6\x85\x1e\x64\xe4\xe9" //3.. .........d.. |
// /* 0030 */ "\x28\x41\x97\x6a\x93\x09\x2b\x11\x43\x0d\xac\x0b"                 //(A.j..+.C... |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x22\x08\x00\x00\x00\xe9\xc4" //...h. ..p"...... |
/* 0010 */ "\x2f\x5a\x09\xbc\xb8\x9a\xc1\x1d\x96\xe5\x2b\xa0\x02\x4e\x46\x45" ///Z........+..NFE |
/* 0020 */ "\xf4\x78\xa2\xa5\x6f\x9e\x1f\x59\x08\x0f\x34\x70"                 //.x..o..Y..4p |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x26\x08\x00\x00\x00\x3a\x47" //...h. ..p&....:G |
/* 0010 */ "\x4f\xcf\xcb\x8d\x3b\x41\xb5\x69\x1a\x19\x80\x03\x92\x76\x06\x6f" //O...;A.i.....v.o |
/* 0020 */ "\x92\x4a\xf4\xb9\x5b\xd4\xdc\xd3\x17\xfc\x14\xd2\xc2\xae\x66\x59" //.J..[.........fY |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x26\x08\x00\x00\x00\xa4\xbe" //...h. ..p&...... |
/* 0010 */ "\x23\xe2\x7c\x15\xe9\xb2\x13\x1d\xa6\x15\xad\x8d\x5d\xd3\x56\xfd" //#.|.........].V. |
/* 0020 */ "\xcd\x91\x97\x50\x7c\xbf\x31\x76\x9e\x9e\x32\xa8\x9d\xbf\x45\x1b" //...P|.1v..2...E. |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x80\xca\x08\x00\x00\x00\xa1" //...h. ..p....... |
/* 0010 */ "\xfc\x30\x8f\x0f\xb6\x55\x87\x7b\xda\xd9\x90\x92\x05\xf7\x35\x45" //.0...U.{......5E |
/* 0020 */ "\x96\xb2\x92\x5d\xa9\x6a\x16\x1e\x04\x0f\x4e\xaa\xa7\xeb\x58\xe8" //...].j....N...X. |
/* 0030 */ "\xda\xeb\xc9\xac\x99\x44\x64\xf2\x51\xbf\x67\xec\x08\x75\x78\xd8" //.....Dd.Q.g..ux. |
/* 0040 */ "\x15\x2f\x47\x06\xf2\x8e\x25\xbd\x19\xb1\xd6\x19\xc2\x29\x8b\x82" //./G...%......).. |
/* 0050 */ "\x26\x49\x53\xc5\x8a\xfb\xcd\xb2\xcf\x6b\xec\x2e\x45\x8a\x9a\xc4" //&IS......k..E... |
/* 0060 */ "\x17\x94\x9e\x1a\x01\x53\xb0\x2f\xe2\xaa\xe4\xdb\xff\x00\xea\x28" //.....S./.......( |
/* 0070 */ "\xf8\x55\x1b\x78\x6e\x7c\xb2\xb3\x80\xb6\xe3\x9c\x10\x65\xf8\x04" //.U.xn|.......e.. |
/* 0080 */ "\x66\xe9\xda\x66\x24\x73\x28\x01\xdc\xe4\xd3\x0d\x6f\xdf\x8c\x4d" //f..f$s(.....o..M |
/* 0090 */ "\x43\x8d\xcc\xa0\xb6\x80\x49\xf7\x15\x3a\xc8\x05\x79\xf3\xc3\x17" //C.....I..:..y... |
/* 00a0 */ "\x78\x2e\xbf\xb5\xe4\x56\xd2\x70\xf5\xfd\xa0\x7a\xac\x77\xb0\x8e" //x....V.p...z.w.. |
/* 00b0 */ "\x81\x49\x08\xd3\xe0\x63\x53\x2e\x57\xce\x5d\x00\xf2\x3b\x8f\x67" //.I...cS.W.]..;.g |
/* 00c0 */ "\x36\xb4\xef\x53\xea\x72\xfd\xca\xe1\x17\xe5\xd9\x52\x43\xb7\x15" //6..S.r......RC.. |
/* 00d0 */ "\x61\xed\x60\x82\xb2"                                             //a.`.. |
// Dump done on RDP Wab Target (4) 213 bytes |
// LOOPING on PDUs: 190 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_FONTMAP |
// mod_rdp::send_input_slowpath |
// Sending on RDP Wab Target (4) 60 bytes |
// /* 0000 */ "\x03\x00\x00\x3c\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x2e\x08\x00" //...<...d. ..p... |
// /* 0010 */ "\x00\x00\xd2\xbc\x42\xae\xf4\xa1\x77\xd4\x66\x8e\x45\x9d\xee\x06" //....B...w.f.E... |
// /* 0020 */ "\xf4\xc8\xf5\x84\x73\x8a\xe1\x79\xe0\x95\xe9\xbc\x26\x4d\x4a\x8b" //....s..y....&MJ. |
// /* 0030 */ "\x81\xd9\x1e\xdd\xc4\x0c\x8e\x8d\xfb\x3f\xa2\x2d"                 //.........?.- |
// Sent dumped on RDP Wab Target (4) 60 bytes |
// mod_rdp::send_input_slowpath done |
// enable_wab_agent=no wab_agent_launch_timeout=0 wab_agent_on_launch_failure=0 |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x22\x08\x00\x00\x00\x5a\xda" //...h. ..p"....Z. |
/* 0010 */ "\x3b\x7a\x03\xf7\x25\x6a\x2d\xab\xf7\xc9\xf0\xad\xf7\xc7\x63\x8b" //;z..%j-.......c. |
/* 0020 */ "\x79\xde\xa5\x1c\x81\x50\xdc\x25\xb2\x6e\x66\xe4"                 //y....P.%.nf. |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x84\x6e\x08\x00\x00\x00\x95" //...h. ..p.n..... |
/* 0010 */ "\x41\x1d\xe0\x4b\x6b\x0f\x1f\x27\x8d\x93\xd8\x43\xc3\xaa\x60\x14" //A..Kk..'...C..`. |
/* 0020 */ "\xf2\xa4\xd4\xd9\x45\xfb\x2f\x23\x03\x7d\x67\x25\x15\xb3\x70\xf1" //....E./#.}g%..p. |
/* 0030 */ "\xa4\x23\xfd\xaf\x3d\x6d\xba\xb9\x34\xe0\x5c\xbb\x43\xee\xb6\xcf" //.#..=m..4...C... |
/* 0040 */ "\xf1\x0f\x03\xf5\x27\x39\xf2\xd7\x4b\xde\xe2\x52\x28\x62\xc4\x66" //....'9..K..R(b.f |
/* 0050 */ "\x0d\x96\xa6\x11\x47\x5b\x3a\x46\x16\x1f\xb0\x25\xce\x69\x12\x7c" //....G[:F...%.i.| |
/* 0060 */ "\xde\x1c\xfd\x3c\xe6\xef\x40\x66\x21\x28\xde\xd3\x10\x58\x79\x1a" //...<..@f!(...Xy. |
/* 0070 */ "\xd3\x29\x4f\xee\x93\x5b\xc3\xd1\xef\xcc\xce\x0c\x3d\x11\x40\x67" //.)O..[......=.@g |
/* 0080 */ "\x84\x91\xbf\x60\x0e\x40\xcb\x15\x46\x6d\x02\x9e\x66\x75\x57\xb1" //...`.@..Fm..fuW. |
/* 0090 */ "\x80\x40\xe8\x83\x7d\xd6\x49\xdf\x92\xb5\x78\xcc\xb1\xc8\x59\xbc" //.@..}.I...x...Y. |
/* 00a0 */ "\x61\xe4\xd6\x1a\xc7\xe7\xc9\x1d\x34\x9f\xd3\x28\xc1\xf7\x25\x06" //a.......4..(..%. |
/* 00b0 */ "\x1d\x00\x77\xe2\xed\xb9\x6d\x5a\xe6\x92\xa8\x70\xb9\x23\x93\xae" //..w...mZ...p.#.. |
/* 00c0 */ "\x85\xd3\xbb\xad\x46\xf0\x51\x02\xf1\xb8\x4a\xf0\xff\x09\x31\xb9" //....F.Q...J...1. |
/* 00d0 */ "\x96\xdb\x8c\x40\xef\x8e\x69\x96\x7d\xdf\x86\x25\x83\x0c\x6a\xbd" //...@..i.}..%..j. |
/* 00e0 */ "\xf3\x3a\x64\xa4\x7e\x8a\x50\x3b\x04\x2f\x5b\xed\x7c\xeb\xca\xd6" //.:d.~.P;./[.|... |
/* 00f0 */ "\x53\x66\x6e\x16\x3c\xc3\x90\xd8\x88\x5a\x3c\xca\x1c\x28\x30\x23" //Sfn.<....Z<..(0# |
/* 0000 */ "\xa2\x6d\x0c\xc4\xd6\x18\xfd\x65\xac\xa5\x22\xd2\x78\xa0\xc9\x65" //.m.....e..".x..e |
/* 0010 */ "\x55\x6c\xcf\x31\x8a\xa4\xce\x35\xeb\xe4\xc8\x64\xb6\x03\x05\x21" //Ul.1...5...d...! |
/* 0020 */ "\xaa\xbd\xfd\xed\xe8\x2c\x58\xce\xae\x57\xae\x78\x59\x52\xc7\x86" //.....,X..W.xYR.. |
/* 0030 */ "\xe9\x55\x28\xc0\x59\xfe\x8c\xba\x05\xb3\xfc\xa2\xfd\x95\xc2\xe9" //.U(.Y........... |
/* 0040 */ "\x92\xaa\xdb\xd6\x20\xf3\xfb\xb0\xc4\x91\x22\x95\xba\xef\x2a\xbc" //.... ....."...*. |
/* 0050 */ "\x88\x84\x47\x5a\x54\x5b\xef\xef\x06\x62\x7c\x8d\xe7\x6b\x5a\x22" //..GZT[...b|..kZ" |
/* 0060 */ "\x5d\x15\x79\x45\x9d\x8d\x42\xc8\x14\xf0\x31\x6a\x78\xdf\xd1\x96" //].yE..B...1jx... |
/* 0070 */ "\xc6\xd5\x9f\x4b\x4d\xea\x51\xec\xbb\x3f\x19\x94\x92\x1d\x01\x49" //...KM.Q..?.....I |
/* 0080 */ "\x7a\x6a\x2a\xe2\x64\x9f\x90\x91\xae\x8a\xa7\x30\x88\xd6\xf0\x32" //zj*.d......0...2 |
/* 0090 */ "\xfb\x04\xbc\x11\x9d\x49\xf2\x27\xee\x7f\x92\x13\x59\x50\x80\xf5" //.....I.'....YP.. |
/* 00a0 */ "\xc7\xb4\xf4\x2d\xef\x7c\x62\x1f\x61\x48\x4a\xff\xe6\x62\x84\xca" //...-.|b.aHJ..b.. |
/* 00b0 */ "\xd4\x67\x75\xea\xc4\x4d\xbb\xa0\xda\x4a\x31\x4f\x5e\xec\x54\x58" //.gu..M...J1O^.TX |
/* 00c0 */ "\xb0\xef\xfb\x26\x94\x98\x9b\xab\x50\x5f\x2c\x47\x74\x53\x55\x27" //...&....P_,GtSU' |
/* 00d0 */ "\x29\x1e\x92\x9f\xf4\xfd\xcd\xd4\x90\x23\x9d\xf0\x06\x0c\x33\x7f" //)........#....3. |
/* 00e0 */ "\x47\xe2\xb9\x12\xb2\xfb\x68\xb9\x4f\x63\xa6\x24\x5a\x51\x21\x0a" //G.....h.Oc.$ZQ!. |
/* 00f0 */ "\xc7\x5f\x15\x3f\xfe\x02\xae\xc8\x96\x5b\x6b\xa7\xe8\xf1\x70\x9e" //._.?.....[k...p. |
/* 0000 */ "\xd5\xb0\x31\xab\x6d\x53\xc0\x9a\x23\x71\x25\x74\x08\x75\xe9\x6c" //..1.mS..#q%t.u.l |
/* 0010 */ "\xb6\xfd\xc5\x7d\x46\x38\x85\x63\x5a\x64\x62\xa9\xd4\xf6\xf8\x92" //...}F8.cZdb..... |
/* 0020 */ "\xa7\x85\x8e\x58\xc9\xa2\x70\x63\x9d\x8b\xb6\xe9\xf8\x29\x95\x51" //...X..pc.....).Q |
/* 0030 */ "\xfd\x27\x30\x0f\x22\xc1\x71\x22\xf4\x99\xb0\x76\x72\x4e\xc0\x1f" //.'0.".q"...vrN.. |
/* 0040 */ "\x62\x64\x68\xbf\x5b\x70\x2c\x62\x30\x3a\x6c\x16\x0d\xeb\xa4\xb2" //bdh.[p,b0:l..... |
/* 0050 */ "\x95\x30\xc1\x5f\xd4\x77\xab\x11\x82\x58\x08\x0a\x31\x26\x0d\xc4" //.0._.w...X..1&.. |
/* 0060 */ "\x8e\x24\xd2\x27\x8d\xe7\x00\x9f\xb7\xb0\xe3\xb5\xc3\x48\x9c\x8f" //.$.'.........H.. |
/* 0070 */ "\xdc\xc8\xbf\xee\xfd\xd5\xd0\x7a\x41\x34\x21\xf3\x33\xfb\xf3\xa7" //.......zA4!.3... |
/* 0080 */ "\x51\xf7\x71\x62\xe5\x78\xac\x20\x35\x64\xed\x9d\x9e\x5b\x4d\x8d" //Q.qb.x. 5d...[M. |
/* 0090 */ "\xf5\x81\xdc\xa2\x18\x51\x4c\x49\xb2\xa5\x83\x92\x3d\x1e\xbc\x5b" //.....QLI....=..[ |
/* 00a0 */ "\xd8\x5b\x2c\xc2\xef\x7b\x85\x96\x0c\xa8\xe1\xc6\xfb\x32\x50\xae" //.[,..{.......2P. |
/* 00b0 */ "\x93\x44\x7c\xf6\x5e\x4e\xe3\x28\x22\x15\xd7\xbe\x58\x72\x2a\x81" //.D|.^N.("...Xr*. |
/* 00c0 */ "\xfc\x29\xb4\x72\x30\xfc\xa5\x2c\x4a\x49\xe9\x54\x5a\x77\x0e\x07" //.).r0..,JI.TZw.. |
/* 00d0 */ "\xa9\x68\x7f\x33\x1b\x7e\xd4\x98\xbf\xe3\xf3\x1d\x44\xca\x2a\x2f" //.h.3.~......D.*/ |
/* 00e0 */ "\xea\xf5\xcc\xf0\x7d\x9a\xef\x6e\xee\xf3\xc8\x90\x8f\x90\x59\x24" //....}..n......Y$ |
/* 00f0 */ "\x36\x00\xd8\x21\xe2\x14\x66\x4f\xed\x53\x9d\x37\xc2\x82\x3c\x29" //6..!..fO.S.7..<) |
/* 0000 */ "\x13\x00\x7f\xac\xf4\x07\x00\x14\x8d\x5f\x34\xea\xac\x91\xbd\x81" //........._4..... |
/* 0010 */ "\x5d\xa6\x1f\x68\xa5\x0c\x1c\x62\x36\xea\x8b\x53\x10\xb6\xc8\xdf" //]..h...b6..S.... |
/* 0020 */ "\x0e\x2f\x7d\xc1\xb6\x10\x56\x81\x55\xfb\x46\x76\xe7\x2f\x11\x9c" //./}...V.U.Fv./.. |
/* 0030 */ "\xbb\xda\x97\xbe\x2c\x78\x2a\x0a\xaf\x8f\xa9\x28\xee\xf7\x48\xdb" //....,x*....(..H. |
/* 0040 */ "\xdc\xd5\x89\xc0\xc9\x70\xf4\xf0\x69\xbc\x0c\xb1\x62\x06\xce\xd7" //.....p..i...b... |
/* 0050 */ "\xef\x40\x33\x36\xb4\x57\xca\xa5\xeb\xf5\x23\x31\xfa\xb4\xef\x40" //.@36.W....#1...@ |
/* 0060 */ "\x50\xee\x4e\x7c\x56\x52\xe5\x19\xd1\xb3\x46\x5d\xce\x89\x07\xb5" //P.N|VR....F].... |
/* 0070 */ "\x89\x42\x1f\x88\x13\x87\xd6\x47\x22\x0d\x8e\xc3\x4f\xaf\x10\x96" //.B.....G"...O... |
/* 0080 */ "\xcb\x88\xbe\xfe\xdf\x49\x3c\xc9\xee\x97\x7f\xc9\x21\x3a\x52\xe8" //.....I<.....!:R. |
/* 0090 */ "\x5d\x70\x0c\xac\x5c\xff\x8a\xcd\x55\xd4\x3b\xb1\x83\x4d\x9e\x64" //]p......U.;..M.d |
/* 00a0 */ "\xc9\x34\xbb\x9b\xdd\x7a\x54\x72\x2a\xcd\xe2\x98\x12\x4a\xf6\x0e" //.4...zTr*....J.. |
/* 00b0 */ "\x05\x50\x54\x6e\x05\x8f\x7b\x75\x87\x2e\xce\xa3\xd5\x5f\x1e\x15" //.PTn..{u....._.. |
/* 00c0 */ "\x27\x89\xc8\x82\x1c\x1e\x20\x9d\x43\x18\xd8\xff\x37\xd6\xb0\x3a" //'..... .C...7..: |
/* 00d0 */ "\x81\x3e\x71\x85\xef\xcb\xa4\x21\x90\xdb\xde\xa4\xa6\xb1\x13\x82" //.>q....!........ |
/* 00e0 */ "\xce\xb1\x2d\xa1\x42\x79\xca\x6c\x39\xe6\xfb\x86\x47\xb0\xa4\x70" //..-.By.l9...G..p |
/* 00f0 */ "\x2a\x16\xe7\x5e\xc5\xac\x3c\x5a\x4b\xd0\x18\x57\x8c\x59\x60\x1b" //*..^..<ZK..W.Y`. |
/* 0000 */ "\x00\x16\xcd\x3c\x8e\x32\x56\x85\x07\x11\x7a\x3d\xff\x04\x1d\x31" //...<.2V...z=...1 |
/* 0010 */ "\x66\xce\x06\x1b\x19\x1a\xac\x47\x25\x5c\xaf\xd9\xea\x19\x89\xb3" //f......G%....... |
/* 0020 */ "\xeb\x79\x60\x07\xa8\x84\x31\x26\x08\x46\x8d\xb4\x07\xab\xbe\x47" //.y`...1&.F.....G |
/* 0030 */ "\x27\xe1\xbf\x68\x69\xdd\xe2\x8d\xf0\x4a\xd7\x62\x1f\xfe\x20\xde" //'..hi....J.b.. . |
/* 0040 */ "\x65\x12\xab\x76\xc4\x1e\x19\xff\x68\x27\xf1\xb8\x0b\xba\xfb\xd8" //e..v....h'...... |
/* 0050 */ "\x7c\xae\xca\x8d\xaa\x51\x04\x59\x9f\x77\xad\x12\x8a\x84\x88\x0a" //|....Q.Y.w...... |
/* 0060 */ "\x3a\xbb\xfa\xa2\x18\x17\x36\x64\xd3\xd0\x5b\x93\xbb\x20\xb5\xa0" //:.....6d..[.. .. |
/* 0070 */ "\x65\x0c\x54\x3c\xde\x66\xac\x39\x4a"                             //e.T<.f.9J |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x89\x9a\x08\x00\x00\x00\x82" //...h. ..p....... |
/* 0010 */ "\xc6\x99\x40\xb8\x42\x13\xab\x91\x2b\x0b\x1e\x11\x4d\x52\x67\xcd" //..@.B...+...MRg. |
/* 0020 */ "\x52\x73\x1d\x5a\x36\x53\x83\x9b\x6d\xb7\x6d\xe8\x68\xe8\x17\x7c" //Rs.Z6S..m.m.h..| |
/* 0030 */ "\xa5\x14\xb8\x3f\x27\x7a\x77\xf4\x33\xac\x85\x3d\xb9\xf3\x44\x41" //...?'zw.3..=..DA |
/* 0040 */ "\x92\x06\x6a\x73\xc5\x5b\x8f\x89\x7f\x2f\x4e\xe9\x9a\xec\xd5\x2d" //..js.[.../N....- |
/* 0050 */ "\xff\x7c\xc6\xaa\xf2\x65\xa8\x07\xc0\x25\x68\x38\xdf\xe0\xb1\x9a" //.|...e...%h8.... |
/* 0060 */ "\x2c\xbf\xa2\x52\x15\xae\xf4\x6d\xc7\x61\x77\xc8\xd3\xcc\x99\x1c" //,..R...m.aw..... |
/* 0070 */ "\x25\xb3\xc7\x5e\x15\xb0\x7d\xb5\x38\xf4\x5b\x68\x67\xe9\xf2\x3e" //%..^..}.8.[hg..> |
/* 0080 */ "\xc9\x1c\xab\xaf\x77\x41\x63\xd6\xac\xaf\x66\x91\x8b\xd6\xf4\xfe" //....wAc...f..... |
/* 0090 */ "\x64\x92\xe0\x24\x6f\xe8\x5a\x46\x52\x76\x70\xc4\x75\x5c\x8c\xf3" //d..$o.ZFRvp.u... |
/* 00a0 */ "\x4d\x11\x0a\x24\xe5\x57\x4a\x49\x26\x73\xbf\x91\x1f\xd5\xbd\x2b" //M..$.WJI&s.....+ |
/* 00b0 */ "\xb6\xc7\xb3\xa9\xcb\x2a\xa9\xb2\x40\xb9\x60\xdc\x93\xd5\xc6\x93" //.....*..@.`..... |
/* 00c0 */ "\x1c\x0e\x8a\x6c\xd5\xd3\x9a\x71\x0f\xe0\x60\x8f\xd4\xb1\xa5\x50" //...l...q..`....P |
/* 00d0 */ "\x3c\xdd\xb1\x61\xee\xa1\x8d\x27\xd5\x19\xb7\x5b\x58\xfb\xb2\x20" //<..a...'...[X..  |
/* 00e0 */ "\x52\x29\x65\x8a\xa5\xa4\xd0\x58\x25\xc9\x03\xf9\xe0\x85\xfd\xb3" //R)e....X%....... |
/* 00f0 */ "\x58\x99\xb5\x48\xd3\x23\xfe\x95\x1a\x79\x7a\xff\xb8\x14\x0f\x16" //X..H.#...yz..... |
/* 0000 */ "\x9e\x1c\x0a\x98\xfd\x71\xfb\xdb\xff\xc4\x44\x93\xb9\x41\xfc\xd1" //.....q....D..A.. |
/* 0010 */ "\xfc\x2f\x38\x5e\x26\x21\xa1\x41\xb3\x92\x80\x4c\xdf\x5e\x02\xdd" //./8^&!.A...L.^.. |
/* 0020 */ "\x37\x18\x25\x39\xed\x55\x16\x6d\x28\x02\x2b\x37\x50\xd2\xad\xf8" //7.%9.U.m(.+7P... |
/* 0030 */ "\x20\x57\xf3\x9e\x53\xac\xe2\x3d\x51\x6e\xf6\xd2\x76\xf4\xbb\xfe" // W..S..=Qn..v... |
/* 0040 */ "\x22\x5d\xe7\xce\x98\x44\x40\xd9\x78\x59\x1d\xaa\xb7\x41\x6b\x67" //"]...D@.xY...Akg |
/* 0050 */ "\x27\x6a\xaf\xd2\xa7\x34\x0c\xbb\x0a\x6a\x8a\x52\xf9\x4f\xc8\x6b" //'j...4...j.R.O.k |
/* 0060 */ "\x94\xfc\x0a\x0c\x69\x95\x94\x3d\x6b\x05\x3e\xb7\x46\xf5\x38\x5e" //....i..=k.>.F.8^ |
/* 0070 */ "\x16\x18\xc6\x4f\x88\xbf\xfe\x46\xc1\xbc\x34\x3d\x41\x7a\xf9\x66" //...O...F..4=Az.f |
/* 0080 */ "\xb6\x3f\x03\x94\xff\x4c\x01\x91\x87\x32\x19\xd8\xe2\xef\x63\x10" //.?...L...2....c. |
/* 0090 */ "\x07\x79\x02\xb0\x02\x9a\x91\xa7\x3f\xe7\x50\xd9\x8c\x1e\x17\xdb" //.y......?.P..... |
/* 00a0 */ "\x44\x71\x4e\x80\x34\x81\x0b\x5c\xc8\xed\x9e\x9d\x13\x32\x3a\x8c" //DqN.4........2:. |
/* 00b0 */ "\x2f\x62\xda\x06\x22\x7b\xdf\x6a\xb1\xac\xd1\xff\xd2\x4c\x42\xbe" ///b.."{.j.....LB. |
/* 00c0 */ "\xb3\xcf\x7e\x1e\x7e\xfe\x9d\x48\xb5\xd0\x7b\x7b\xc4\x53\xec\x56" //..~.~..H..{{.S.V |
/* 00d0 */ "\x86\x10\x8a\x0c\xa6\x8a\xe0\xc5\x24\xab\x44\x02\x2a\x06\x5b\x58" //........$.D.*.[X |
/* 00e0 */ "\x2a\x62\x48\xd7\x14\x34\xe0\x57\xa8\x08\x9c\xeb\xb1\x5d\x19\xaa" //*bH..4.W.....].. |
/* 00f0 */ "\xfe\xef\x4f\x58\x33\x06\xf8\x04\xfb\xe4\xf0\x5b\xb5\xb0\x14\x57" //..OX3......[...W |
/* 0000 */ "\x55\x92\x99\xba\x37\x33\x0b\x80\xb6\x0c\x1d\x7d\xf2\x86\xd2\x12" //U...73.....}.... |
/* 0010 */ "\x67\xa7\xb0\x5b\x8a\xe4\xbc\xc8\x4f\x4c\xbc\x81\xf6\x0f\x71\xaa" //g..[....OL....q. |
/* 0020 */ "\x15\xb9\xa5\x9f\xe3\x8b\xb8\xa4\x9f\xab\xe3\xe6\x8d\xc3\x97\x2a" //...............* |
/* 0030 */ "\x00\x6c\x9a\xd9\x52\xc4\xf4\xf0\x9f\xfb\xdf\x7d\xa1\xa3\xeb\x27" //.l..R......}...' |
/* 0040 */ "\x56\xf2\x1a\xb5\xc6\x49\xef\xe1\x25\x7b\xd5\xf0\xca\x8e\x8d\xfa" //V....I..%{...... |
/* 0050 */ "\xe5\xeb\xec\x5c\xb7\xc0\x3a\x3f\x84\xf4\x4c\x53\x88\xf0\x00\x4b" //......:?..LS...K |
/* 0060 */ "\x25\x47\x87\x64\x52\xdd\x23\x1e\xa9\x6e\x40\x9a\xa1\x36\x3a\x4d" //%G.dR.#..n@..6:M |
/* 0070 */ "\x4f\xb7\xe8\x0d\xc6\x9f\x9a\x4f\xa9\xa7\x2a\x75\x6c\xa5\xeb\x8a" //O......O..*ul... |
/* 0080 */ "\xa8\x54\x68\xbf\x18\x81\x63\xde\x9a\xba\x58\x89\xdd\xc8\xb5\x0c" //.Th...c...X..... |
/* 0090 */ "\x8e\xbe\x9e\x96\x17\x29\x1d\xf7\xb8\xe6\x97\x44\xf4\x2a\x82\xb5" //.....).....D.*.. |
/* 00a0 */ "\xf4\x20\x77\x28\x6b\x47\x88\x24\x94\x4f\xeb\x15\x06\xa6\x67\x6e" //. w(kG.$.O....gn |
/* 00b0 */ "\xd6\x66\x7e\x66\x2d\xe0\xbd\xc7\x86\xd4\x2f\x8e\x39\xd8\x25\x35" //.f~f-...../.9.%5 |
/* 00c0 */ "\x0e\x5b\x8c\x8e\x8a\xee\x56\xb9\x75\x97\xfc\x92\x01\xa3\x26\x1c" //.[....V.u.....&. |
/* 00d0 */ "\x7a\xf0\x14\x51\x1e\xdc\xb1\xde\x0b\xc8\x89\xec\x3a\x97\xac\x19" //z..Q........:... |
/* 00e0 */ "\x6c\x30\x6c\xfe\x00\x61\xef\xc6\x46\x1a\xb3\x2f\x49\x5b\xde\x40" //l0l..a..F../I[.@ |
/* 00f0 */ "\x00\xb9\x34\x95\x43\xe1\x1d\x6a\x99\xea\x11\x90\xd4\x86\xbb\x98" //..4.C..j........ |
/* 0000 */ "\xa5\x1b\x70\x37\x8e\xe9\x42\xb7\x6a\x26\x3e\x23\x43\x45\xa2\x0d" //..p7..B.j&>#CE.. |
/* 0010 */ "\xbe\xf2\x2c\xb9\x5a\x4a\xa7\xdd\x8c\x91\x5e\x9f\xa3\xe9\xde\xec" //..,.ZJ....^..... |
/* 0020 */ "\x78\xe5\xf2\xb5\xa0\xd5\x4f\x69\xe4\x8a\xd6\x41\x90\x08\x5d\xea" //x.....Oi...A..]. |
/* 0030 */ "\x24\x2e\x21\x4e\xde\x6f\xc5\x1c\xb2\xdf\x61\x9c\x41\x34\xd5\x63" //$.!N.o....a.A4.c |
/* 0040 */ "\x31\x0e\xc7\xb8\x9d\xa7\xc6\xbc\x9b\x10\x9c\x24\xd3\x7c\x1a\x86" //1..........$.|.. |
/* 0050 */ "\x96\xa8\x2f\x8a\x08\x90\x66\x83\x93\x4c\x70\x43\xf3\x03\x09\x0f" //../...f..LpC.... |
/* 0060 */ "\x04\x08\xa8\xb0\x4b\xd2\x28\x8a\x15\xd0\xf6\x1c\x82\xf9\x3d\xb5" //....K.(.......=. |
/* 0070 */ "\xc7\xab\xbf\x80\x61\xed\xf4\x35\x2f\x39\xa4\x95\x4c\xa1\xbd\x9d" //....a..5/9..L... |
/* 0080 */ "\xea\xd7\x60\x61\xf6\xa9\x23\x59\xab\xcd\xc2\x4f\xbd\x45\xb2\x64" //..`a..#Y...O.E.d |
/* 0090 */ "\x41\x25\xfd\xcd\xb6\xd0\x17\xf4\xbe\x36\xdc\x10\x30\x64\xa7\x50" //A%.......6..0d.P |
/* 00a0 */ "\xce\xb2\x83\x68\x06\x49\x8d\x7a\xb1\xc7\x98\x2d\x1c\x88\xdd\xa7" //...h.I.z...-.... |
/* 00b0 */ "\x0f\xb5\x50\x40\x7b\x46\x66\xc2\x54\xd7\xfb\xdd\xe9\x12\xfa\xa2" //..P@{Ff.T....... |
/* 00c0 */ "\x6d\xc9\x17\x3e\xaa\x25\x62\xbe\x53\xd5\x9a\x03\xf5\x42\xdb\x5f" //m..>.%b.S....B._ |
/* 00d0 */ "\x9f\x73\x4a\x2e\xd2\xbb\x7f\x70\xc8\xb0\xf3\x4e\xbd\x57\x87\x76" //.sJ....p...N.W.v |
/* 00e0 */ "\xfb\x2b\x64\x1a\x2c\x7e\xd0\x1d\xd5\xf6\xd3\x4b\x1f\x31\x0f\xac" //.+d.,~.....K.1.. |
/* 00f0 */ "\xae\xc5\x03\x09\x10\x82\x10\x68\xf0\x57\x13\x50\xf8\xb0\x09\x5b" //.......h.W.P...[ |
/* 0000 */ "\x71\x88\xca\x0d\x55\xbe\xf1\x48\x25\xeb\x68\x84\xdf\xa4\xb8\x65" //q...U..H%.h....e |
/* 0010 */ "\x75\x99\xa3\xf4\x32\x73\xa9\x5a\xc7\x9d\x85\x23\xfe\xe9\xce\x93" //u...2s.Z...#.... |
/* 0020 */ "\xc2\x3d\xc9\x15\x9e\xd4\xc8\xb6\x3b\x03\x2a\x18\x18\x26\xd9\x7c" //.=......;.*..&.| |
/* 0030 */ "\x0b\x0d\x4f\xe3\x7f\x94\x2f\xdc\x1f\xb8\x65\x53\x12\x66\xab\x01" //..O.../...eS.f.. |
/* 0040 */ "\xee\x7d\xed\x15\x71\xeb\xfa\x82\x19\x8e\x94\xe7\x0f\x83\x82\xce" //.}..q........... |
/* 0050 */ "\x17\x2f\x87\xe0\xed\x2c\x85\x4e\xdb\x14\xff\xbb\x01\x3e\xd7\x96" //./...,.N.....>.. |
/* 0060 */ "\x0b\xc6\xf2\xc1\xc5\xe4\xdb\xeb\xd5\xca\x22\x53\x44\x23\x4f\x70" //.........."SD#Op |
/* 0070 */ "\xe5\x29\x06\x1a\x61\xb3\x7e\xc5\xca\x73\x8b\xde\xd6\xdb\x54\x35" //.)..a.~..s....T5 |
/* 0080 */ "\x15\x79\x19\x35\x0f\x96\x1b\x43\x82\x98\x9b\x34\x60\xbe\x76\xd9" //.y.5...C...4`.v. |
/* 0090 */ "\x29\x2a\xb8\x3a\x1c\x97\x86\xd4\x9d\xec\x06\x75\x68\xa5\xb6\x12" //)*.:.......uh... |
/* 00a0 */ "\xb7\x7f\x04\x3f\xbc\xdf\x75\x7d\x78\xd7\x2f\xe1\xae\x7d\x59\x45" //...?..u}x./..}YE |
/* 00b0 */ "\x33\xdb\x49\xb6\xdc\x4c\x53\x60\x21\xa6\xba\x87\xf8\x54\x1d\xb9" //3.I..LS`!....T.. |
/* 00c0 */ "\xf6\xa7\xd1\x89\xaa\x32\xd1\x38\x05\xb7\xa0\x2c\x4d\xd9\x5b\x2b" //.....2.8...,M.[+ |
/* 00d0 */ "\xbf\xe3\x72\xec\xc9\x40\xeb\x35\xc1\xda\x68\x01\x01\xf0\x21\xc6" //..r..@.5..h...!. |
/* 00e0 */ "\x6f\xf0\xd6\x1b\x96\xa1\xe3\x1d\x70\xc2\xba\xf6\xbe\xd8\x9b\x9a" //o.......p....... |
/* 00f0 */ "\x08\xfe\x56\xff\x27\x3c\x79\x40\xf4\xf5\x3a\x44\xbb\xb4\xdc\x28" //..V.'<y@..:D...( |
/* 0000 */ "\xa7\x2c\xe3\x86\xf6\xfc\x74\x31\xbb\xfa\x1c\xb0\x69\x60\x2e\xc4" //.,....t1....i`.. |
/* 0010 */ "\x70\x90\x92\xa2\x85\xd1\x14\x10\x82\x46\x9f\x19\xe6\x66\xf6\x01" //p........F...f.. |
/* 0020 */ "\x46\x94\x7e\x9b\x1b\x98\x95\x23\x42\x75\xd7\xef\x9f\x21\xcc\x7e" //F.~....#Bu...!.~ |
/* 0030 */ "\xfb\xe0\xc5\x15\x8a\x05\x0b\xd3\x19\x82\x1a\x09\xc3\xbb\xc5\x58" //...............X |
/* 0040 */ "\xf6\x9c\x81\x1c\x37\x8c\xbf\xb2\x61\xd0\x3e\xe4\x0d\x84\xb9\xde" //....7...a.>..... |
/* 0050 */ "\x39\x27\x6e\xa3\x51\x80\xe6\xfb\xf5\xcc\x06\x5b\xb5\xcc\x43\x16" //9'n.Q......[..C. |
/* 0060 */ "\x69\xa1\x74\xb3\x75\xdb\xec\xfa\xec\x62\xce\xf8\x07\xe2\x7b\xcc" //i.t.u....b....{. |
/* 0070 */ "\x07\x64\x7d\x63\x34\x38\x34\x12\xae\xf2\x15\xf5\x7c\x5c\x29\x44" //.d}c484.....|.)D |
/* 0080 */ "\xce\x5a\x0a\x52\x4e\xbe\x4c\x62\xd2\x04\x17\x11\x09\xf1\x2a\xca" //.Z.RN.Lb......*. |
/* 0090 */ "\x20\x32\xce\x42\xe4\x97\xcf\xde\x5f\xff\xdf\xac\xd8\x06\xb4\x2c" // 2.B...._......, |
/* 00a0 */ "\x4c\x93\x3f\x2d\xf9\x8b\x46\x05\x9e\x51\x0f\x48\xf9\xd1\x2f\xc0" //L.?-..F..Q.H../. |
/* 00b0 */ "\x1b\x90\x56\xb6\x7c\x25\xdf\xcd\xb3\x67\xcc\x34\x13\x58\x6d\x95" //..V.|%...g.4.Xm. |
/* 00c0 */ "\xbd\xac\xf3\x01\xde\x11\x75\xa1\x61\x72\x49\x76\xd6\x3c\x39\xa9" //......u.arIv.<9. |
/* 00d0 */ "\x80\x91\x4f\x0d\xc1\xac\xd1\xb8\x9e\xc6\x69\x92\xe1\x56\x0c\xf0" //..O.......i..V.. |
/* 00e0 */ "\xad\x28\x05\x5e\xbd\xd6\x15\xdb\xed\xe0\x9f\x78\xd3\xa5\x4d\xd0" //.(.^.......x..M. |
/* 00f0 */ "\xe9\xad\x13\xff\xe1\x4a\x1f\xcb\x15\xb1\xbc\x54\x1b\x52\x14\xfc" //.....J.....T.R.. |
/* 0000 */ "\x94\xba\x9b\xf3\x2d\xbd\x26\x3a\x55\xf0\x7e\xaf\x47\xc2\x09\xa8" //....-.&:U.~.G... |
/* 0010 */ "\x7c\x37\x62\x57\x2b\xca\x16\x96\xf6\xde\xb4\x18\x94\x3b\xef\x5c" //|7bW+........;.. |
/* 0020 */ "\x75\x94\x56\xfd\xb7\x69\xec\x5b\x03\x4d\x54\xe8\x4a\x5d\x20\x26" //u.V..i.[.MT.J] & |
/* 0030 */ "\x4d\x58\x27\xc1\x4b\x54\x9a\xe1\x1d\xf5\x7a\x7c\x90\x6c\xc6\x68" //MX'.KT....z|.l.h |
/* 0040 */ "\x2d\xd5\xfc\x07\xd8\xe7\x89\x06\x25\x3e\xff\x8f\x8f\x17\xda\xaf" //-.......%>...... |
/* 0050 */ "\xfa\xae\x02\x7e\xd9\x66\x47\xca\xf6\x0c\xbb\x23\x9d\xe2\xee\x44" //...~.fG....#...D |
/* 0060 */ "\x06\x76\x12\xc7\x02\x56\x3d\x1a\xe0\x61\x23\x3a\x2b\x7b\x9b\xc0" //.v...V=..a#:+{.. |
/* 0070 */ "\x76\xa6\xb9\x35\xd1\x6b\xe8\x0f\x78\xcb\xa2\x3d\xe2\x8c\x10\x75" //v..5.k..x..=...u |
/* 0080 */ "\x75\xcb\x2d\x8f\x95\x3a\xfe\xfa\xf0\xbc\xdb\x28\x03\x98\xec\x05" //u.-..:.....(.... |
/* 0090 */ "\xfd\x03\x98\x82\x9a\xbf\x36\x78\x68\x8a\xa6\x8b\x4e\x73\xbe\xe2" //......6xh...Ns.. |
/* 00a0 */ "\xb8\xa8\xa3\xcc\x5f\xce\x58\xc1\x0d\x21\x4f\x9e\x5c\x4d\xcd\x71" //...._.X..!O..M.q |
/* 00b0 */ "\x55\x2e\xd7\xa4\xaf\x33\xac\xd2\xd8\x2c\x71\xa7\x90\xff\x77\x98" //U....3...,q...w. |
/* 00c0 */ "\x65\xf3\x28\xdf\xf0\xd0\xee\x1a\x32\xe6\x8a\x6c\xd2\xcf\x15\xc4" //e.(.....2..l.... |
/* 00d0 */ "\x29\xbe\xda\xcc\xa3\x73\x9b\xae\x32\x5d\xfd\x65\x62\x93\x26\x60" //)....s..2].eb.&` |
/* 00e0 */ "\x77\xda\xa9\xa2\x05\x79\xa7\xf0\x64\x6b\x9b\x86\xde\xcc\x99\x47" //w....y..dk.....G |
/* 00f0 */ "\xe4\x5b\x91\x55\xbb\xf7\x27\x72\x4d\xe9\x04\x0a\x74\x9d\xee\xc3" //.[.U..'rM...t... |
/* 0000 */ "\xad\x4c\x8b\xa7\x68\x87\x71\xee\x88\x33\x8c\x7d\x1f\xa1\x37\x15" //.L..h.q..3.}..7. |
/* 0010 */ "\x3c\x66\x30\x7d\xa5\x22\x36\x83\xec\xb1\xaf\xa3\x09\x6a\x0f\x1f" //<f0}."6......j.. |
/* 0020 */ "\x5a\xbc\x00\xb6\x4d\x23\x90\x5a\xcc\xbf\x6d\xfa\xdb\xc4\x13\xf9" //Z...M#.Z..m..... |
/* 0030 */ "\x53\xf5\x88\xde\x9c\xe6\x61\xca\x1c\xb1\xe0\x9e\x18\x0e\xb0\x38" //S.....a........8 |
/* 0040 */ "\xc2\xb2\x2a\x09\xa5\x69\xd0\xa4\x37\x8f\x3c\x9a\x12\x8f\x3c\x1f" //..*..i..7.<...<. |
/* 0050 */ "\xf9\x69\x5b\xcd\x30\xaa\x28\xf5\x2f\x3c\xb3\xb3\x0e\xbb\x33\x46" //.i[.0.(./<....3F |
/* 0060 */ "\xea\x43\x92\x76\x5a\x21\xed\x43\xdb\xb3\xc1\x13\x70\xc8\xc4\xb4" //.C.vZ!.C....p... |
/* 0070 */ "\x53\x72\xbe\x09\x91\x43\x7c\x1d\x03\x46\xbc\x24\xd7\x49\xbd\x8b" //Sr...C|..F.$.I.. |
/* 0080 */ "\x12\xef\x98\xf1\x98\x47\x12\xbc\x28\x08\x7c\x0d\x53\x60\xf2\xa9" //.....G..(.|.S`.. |
/* 0090 */ "\xbc\xfb\xa4\xa7\xc5\xc3\x8c\x31\xac\x9c\xa2\x18\x1a\xdf\x8c\xe6" //.......1........ |
/* 00a0 */ "\xf6\xe1\x21\xd8\x43\x6c\x39\xfd\x5a\x21\x43\x50\x25\xc0\xb7\xa5" //..!.Cl9.Z!CP%... |
/* 00b0 */ "\x4f\x89\xf3\xb4\xdd\x21\x55\x3a\x4f\x92\x1d\xe8\x9b\x3c\x06\x18" //O....!U:O....<.. |
/* 00c0 */ "\xe4\xfd\x13\x88\x84\x75\x58\xe0\x75\x23\xc2\x76\x43\x21\xcb\x24" //.....uX.u#.vC!.$ |
/* 00d0 */ "\xa2\x68\x80\x00\xe4\x40\xf0\x93\x5b\xc5\x24\x99\xe4\xb9\x91\x7f" //.h...@..[.$..... |
/* 00e0 */ "\x8b\x83\x14\xa0\xc1\x73\xd3\xb1\x28\x63\x96\xf2\x8e\xa6\x50\xc2" //.....s..(c....P. |
/* 00f0 */ "\x12\x8b\x7a\xc9\x8f\xd8\x2e\xf3\x3e\x37\x85\x08\xef\x82\x77\xd8" //..z.....>7....w. |
/* 0000 */ "\xcb\x15\x8e\x0c\xc0\x78\x0e\xca\x8a\x0b\x30\x52\x61\x47\x0d\x6a" //.....x....0RaG.j |
/* 0010 */ "\x51\x2b\xb1\x6d\x1f\xab\x96\xe3\x24\x64\x42\x4e\x14\x4b\x51\xb3" //Q+.m....$dBN.KQ. |
/* 0020 */ "\xf8\xc4\xe6\x9d\x24\xd3\x1f\x19\x6c\x52\xa1\xf5\x11\xbe\xdc\x98" //....$...lR...... |
/* 0030 */ "\x06\x97\x5c\xe0\xf1\x7f\x17\x3b\xa4\x81\x5d\x86\x22\x77\xee\x2f" //.......;..]."w./ |
/* 0040 */ "\x81\x7f\x7f\x22\x0d\x70\xd3\x5c\x5b\xbe\x9d\x86\xa9\x61\x53\xbf" //...".p..[....aS. |
/* 0050 */ "\x43\xb3\x8a\x8e\x9a\xb7\x88\x15\x00\x2f\x10\xeb\xa1\xab\x64\xbc" //C......../....d. |
/* 0060 */ "\x93\xb0\xab\x90\x50\x8d\x0a\xe4\xb6\x58\xb5\xff\xff\x05\x15\x9d" //....P....X...... |
/* 0070 */ "\xb7\x22\xa6\x75\x90\x02\xad\x6d\x82\xf4\x9c\x74\x58\x9e\x46\xeb" //.".u...m...tX.F. |
/* 0080 */ "\x71\xd6\xbc\x41\x26\x21\x13\xb1\xce\x11\xd5\xa6\x0f\x87\x3b\x09" //q..A&!........;. |
/* 0090 */ "\xd7\xcf\x41\x75\x2f\x66\x32\x64\xdf\xfb\x38\x37\xde\x34\x32\x41" //..Au/f2d..87.42A |
/* 00a0 */ "\xba\x90\xef\xe9\xaa\x7b\x57\xb4\x7e\x52\xf6\xb6\xa2\x52\x64\xd0" //.....{W.~R...Rd. |
/* 00b0 */ "\xe0\x33\x03\x1b\xaa\x1c\xc7\x9c\x8e\xc7\x48\x1c\x7d\x32\xbf\x9d" //.3........H.}2.. |
/* 00c0 */ "\xe5\xc7\xb4\xfd\x1c\x4d\x19\x1f\x19\x5a\x88\x03\x68\x61\xf2\x03" //.....M...Z..ha.. |
/* 00d0 */ "\x4f\xbf\x40\x53\x88\x3a\xa8\x43\xbf\xea\x1c\x43\x83\x7c\x12\x03" //O.@S.:.C...C.|.. |
/* 00e0 */ "\xcb\x39\x94\x86\xd1\x62\xd1\xa1\x8d\x49\x66\x8f\xac\x69\xf7\xa4" //.9...b...If..i.. |
/* 00f0 */ "\x4c\x16\xca\xd1\xd2\xd2\xf9\x06\x23\x40\x22\x5d\x33\x87\x14\xde" //L.......#@"]3... |
/* 0000 */ "\xb6\xd9\x90\x40\x6a\x26\xc2\x49\x8e\x35\xbd\xaf\x21\xf8\xab\xb4" //...@j&.I.5..!... |
/* 0010 */ "\x57\x34\x26\xec\x3c\xa4\x01\x80\x9a\xcf\xac\xf2\x64\x88\x13\xb4" //W4&.<.......d... |
/* 0020 */ "\xc4\xaa\x77\xc7\x2c\x74\x70\x15\xa6\xa0\xce\x09\x4a\xce\x89\x78" //..w.,tp.....J..x |
/* 0030 */ "\x34\x49\x9d\xd5\x0d\xae\x65\xff\x34\x09\x02\x85\x4d\x48\x7d\xbe" //4I....e.4...MH}. |
/* 0040 */ "\xdf\xdd\xbe\x2d\x15\xf3\x5c\xcb\x34\x12\x9f\x2a\xe5\x5b\x17\x22" //...-....4..*.[." |
/* 0050 */ "\xf1\x85\x82\xe1\xa8\x46\xb1\xbe\xf7\x36\x23\xb1\x5f\x18\xa3\xdc" //.....F...6#._... |
/* 0060 */ "\x76\x25\x9c\x8b\x88\x76\xc6\xb0\x62\x73\x29\x29\xe1\xff\xc0\xdf" //v%...v..bs)).... |
/* 0070 */ "\x24\x55\x65\x75\x1f\xe1\x7b\x4e\x51\x6c\x89\xab\xb6\xdd\x8d\x24" //$Ueu..{NQl.....$ |
/* 0080 */ "\xf9\xa5\x77\x3e\xa8\x38\xb8\x89\x7b\xb2\x9d\x78\xe9\xbe\x29\x4b" //..w>.8..{..x..)K |
/* 0090 */ "\xde\xc6\xda\xeb\x6b\x40\x69\x5c\xed\x3b\x5d\xec\xa4\x81\x43\xd9" //....k@i..;]...C. |
/* 00a0 */ "\x2e\x97\x3f\x91\xf2"                                             //..?.. |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(20,10,108,17) op=(20,10,108,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=26 data_len=26 [0x00 0x00 0x01 0x09 0x02 0x0e 0x03 0x09 0x04 0x09 0x02 0x04 0x03 0x09 0x04 0x09 0x05 0x04 0x06 0x09 0x04 0x09 0x06 0x04 0x06 0x09] |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(20,41,93,17) op=(20,41,93,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=57 data_len=24 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09 0x0d 0x05 0x0e 0x04 0x09 0x0b 0x0f 0x05 0x10 0x09 0x11 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,41,706,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(224,41,46,17) op=(224,41,46,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=57 data_len=12 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09] |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(930,41,58,17) op=(930,41,58,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=57 data_len=16 [0x12 0x00 0x09 0x09 0x0f 0x05 0x0c 0x09 0x0f 0x05 0x13 0x09 0x0f 0x08 0x14 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(963,13,34,17) op=(963,13,34,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=962 glyph_y=29 data_len=12 [0x15 0x00 0x16 0x08 0x14 0x03 0x0c 0x03 0x0b 0x05 0x09 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(794,705,19,17) op=(794,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=793 glyph_y=721 data_len=4 [0x17 0x00 0x18 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(835,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(836,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(842,705,12,17) op=(842,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=841 glyph_y=721 data_len=2 [0x17 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(877,704,23,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=876 glyph_y=721 data_len=2 [0x02 0x00] |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=900 glyph_y=721 data_len=4 [0x19 0x00 0x03 0x05] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(937,705,12,17) op=(937,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=936 glyph_y=721 data_len=2 [0x1a 0x00] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(978,705,19,17) op=(978,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=977 glyph_y=721 data_len=4 [0x1b 0x00 0x1a 0x07] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(817,738,49,17) op=(817,738,49,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=816 glyph_y=754 data_len=12 [0x1c 0x00 0x0f 0x07 0x0a 0x09 0x0f 0x09 0x10 0x09 0x0c 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(920,738,60,17) op=(920,738,60,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=919 glyph_y=754 data_len=14 [0x1d 0x00 0x0f 0x0a 0x1e 0x09 0x1e 0x09 0x0b 0x09 0x13 0x09 0x0c 0x08] |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x84\x4e\x08\x00\x00\x00\x0e" //...h. ..p.N..... |
/* 0010 */ "\xec\xf3\x9f\xd4\x76\x1a\x6a\x7b\x13\x84\x30\xb3\xdf\xeb\x94\x90" //....v.j{..0..... |
/* 0020 */ "\xc0\x65\xab\x70\xe3\x1e\x6a\xf3\x66\x5f\xed\x8a\x2b\xbd\x9b\x3a" //.e.p..j.f_..+..: |
/* 0030 */ "\xc3\x07\x1d\x71\x5d\x91\x15\x17\x87\x78\xe5\x5c\x31\x95\x92\xdc" //...q]....x..1... |
/* 0040 */ "\x4e\x2e\xcd\x32\x12\xd2\x0c\x38\xc1\xe4\x7f\x13\xd8\x33\x50\x44" //N..2...8.....3PD |
/* 0050 */ "\x55\xcf\xf8\x8c\xf5\xd8\xac\x6c\x57\x40\xa5\xc9\x61\x1a\xf3\xf5" //U......lW@..a... |
/* 0060 */ "\xd3\xae\x72\x21\xfe\x9a\xd2\x82\x82\xb3\x59\x8b\x98\xc3\x3a\x0a" //..r!......Y...:. |
/* 0070 */ "\xa3\xde\xf1\x77\xe3\x29\x0f\xa5\xa1\x39\x0f\x09\x3a\x1c\xd9\xa0" //...w.)...9..:... |
/* 0080 */ "\x96\x9d\x3c\x5c\x17\xc0\x1c\x94\x1f\xdd\x69\x68\x23\x31\xdc\x14" //..<.......ih#1.. |
/* 0090 */ "\xa5\x2d\x36\x16\x88\x83\x1f\x39\xc1\x06\xef\x91\x83\x9e\xbf\xcf" //.-6....9........ |
/* 00a0 */ "\x60\x20\x4e\xca\x94\x10\xcc\x45\x37\x64\xe4\x13\x16\x28\xda\xd8" //` N....E7d...(.. |
/* 00b0 */ "\xf6\xec\xfd\x21\x8f\x35\x8c\x72\x83\x3c\x72\x7c\x9f\x19\xf7\x87" //...!.5.r.<r|.... |
/* 00c0 */ "\xfd\xa0\x70\x1a\x80\xa1\x24\xd7\x29\xea\xf8\x86\xcd\xb4\xb6\x56" //..p...$.)......V |
/* 00d0 */ "\x61\x86\xf8\x0f\x9a\xab\x02\x19\xea\xfc\xf0\xdc\x70\xe4\x77\xba" //a...........p.w. |
/* 00e0 */ "\x03\x0c\x2d\xec\x5e\x0f\xb5\x41\xb1\xc3\x1c\x66\x2b\x1a\xba\x63" //..-.^..A...f+..c |
/* 00f0 */ "\x42\xbe\x37\xcf\x8d\x61\x66\x2c\x55\xff\x7a\x84\x7f\x43\x11\x6b" //B.7..af,U.z..C.k |
/* 0000 */ "\xda\x8b\x46\x49\x45\x64\xaa\x3b\x97\x7a\x28\xfb\xee\xa0\xc6\x7f" //..FIEd.;.z(..... |
/* 0010 */ "\xb0\xbc\x23\x0e\x14\x63\x89\xe7\x74\xd6\xb8\x08\xcb\x09\x81\xde" //..#..c..t....... |
/* 0020 */ "\x73\x90\xed\x8a\x18\x1d\xd8\xfb\x49\x68\xe6\x8e\x97\x3f\x73\x50" //s.......Ih...?sP |
/* 0030 */ "\xa4\x0c\x81\x23\x21\x80\x77\xde\x5b\xe1\x62\x67\x1c\x6b\xbe\xb4" //...#!.w.[.bg.k.. |
/* 0040 */ "\x3b\x6e\x56\x12\xd6\xfb\xcb\x98\xd3\xc5\xe6\x8d\x1b\x33\x53\xc5" //;nV..........3S. |
/* 0050 */ "\xd0\x0e\x50\x37\xea\xd6\x8a\x55\xb3\xf4\x43\x7d\x7c\xca\xab\xa1" //..P7...U..C}|... |
/* 0060 */ "\x09\x7f\xd0\x10\x2c\x1f\x8a\x0b\x17\xfa\xcb\x99\x1b\x93\x44\x9f" //....,.........D. |
/* 0070 */ "\x63\xfa\x17\xb1\x07\xd2\xd2\xa2\xfc\xd4\xc5\x0c\x25\x6e\x46\x70" //c...........%nFp |
/* 0080 */ "\x1f\xc2\x98\x21\x22\xb9\x8b\x6d\xe2\x9c\x7e\x9c\x29\xfa\x4c\x16" //...!"..m..~.).L. |
/* 0090 */ "\xa0\x3a\x6d\x07\xd6\xc7\x8e\xab\x9a\x52\xdc\x9c\x27\x65\x14\x22" //.:m......R..'e." |
/* 00a0 */ "\x90\xef\x3e\x35\x87\x25\x10\x59\x91\xa0\xd3\x61\xe3\x90\xa1\xcf" //..>5.%.Y...a.... |
/* 00b0 */ "\xa1\x4b\xa9\xa8\x38\x4e\x9c\x1e\x0b\x31\xd1\x6d\x62\xbc\x20\x8e" //.K..8N...1.mb. . |
/* 00c0 */ "\xe3\x5a\x5f\x28\x92\x60\x23\x4e\xeb\xef\x01\x1b\xc6\xa1\xf5\x28" //.Z_(.`#N.......( |
/* 00d0 */ "\x43\xd6\x44\x90\x1f\x26\x59\x2b\x49\x6d\x07\x24\x3c\x34\xfa\xdb" //C.D..&Y+Im.$<4.. |
/* 00e0 */ "\x00\x5b\xa2\xc6\xc6\xfb\x98\xb1\x5b\x5a\x90\x88\x99\xce\xc3\xa3" //.[......[Z...... |
/* 00f0 */ "\xda\x9f\x45\xfb\x3f\x31\x05\x0d\x2b\xbc\x82\xfc\x9c\xa2\xed\xd5" //..E.?1..+....... |
/* 0000 */ "\x9b\x8f\x3f\x75\x08\xa2\x5d\x5c\x3b\x07\xa2\xf3\xef\x74\xbd\x34" //..?u..].;....t.4 |
/* 0010 */ "\x6c\xe7\x88\x38\xb8\x02\x8f\xfd\xbd\x15\xaa\xe9\x24\x2b\xcb\xf6" //l..8........$+.. |
/* 0020 */ "\x51\x40\x3d\x2a\x3e\x7f\xcc\x90\x97\x45\x3c\xfa\x2e\x89\x5b\x2b" //Q@=*>....E<...[+ |
/* 0030 */ "\xcd\xe8\xcc\xea\x99\x5b\x8e\xf2\xb1\x78\x25\x14\xd8\xdb\x75\x30" //.....[...x%...u0 |
/* 0040 */ "\x17\x23\x2b\x60\xd6\xcc\x6e\x84\xca\xa8\x15\x4a\x51\x00\x8f\x25" //.#+`..n....JQ..% |
/* 0050 */ "\x1c\xfa\x89\x54\x21\x34\x49\xdf\x64\xfd\x25\xf9\x33\x8a\x84\x5f" //...T!4I.d.%.3.._ |
/* 0060 */ "\xde\xfb\xc2\x93\xf4\x6a\x41\x63\x10\x46\xec\xb1\x45\xe9\xa1\xf3" //.....jAc.F..E... |
/* 0070 */ "\xf3\x17\x5a\xc3\x67\x21\x5e\x95\xfc\xa4\x88\xe2\x4c\xf3\xc4\xbd" //..Z.g!^.....L... |
/* 0080 */ "\xfd\xf5\x86\x54\xb1\x9a\xf2\x52\xdd\xa4\xc3\x5f\xf9\x7a\xa5\x99" //...T...R..._.z.. |
/* 0090 */ "\x31\x74\xb8\xb5\x9f\xa5\x30\x6f\xcb\x6b\xa1\x75\x23\xe6\x1f\x5c" //1t....0o.k.u#... |
/* 00a0 */ "\x9b\xc5\xe4\x74\x58\xfa\x73\x92\x4d\xd1\x41\xd9\x40\xd6\x89\xe0" //...tX.s.M.A.@... |
/* 00b0 */ "\x2a\x62\x77\xa9\x6b\x33\x38\x04\xf6\xcd\x9e\xfb\x64\x35\x8a\xed" //*bw.k38.....d5.. |
/* 00c0 */ "\xad\xa5\x4a\x65\x1e\x93\x92\xcb\x7b\x02\xb5\x4e\x22\xe4\x59\xdf" //..Je....{..N".Y. |
/* 00d0 */ "\x40\x9f\x86\x58\xb4\xd1\xf4\x51\x07\x20\xed\xba\x18\xec\xe8\x3c" //@..X...Q. .....< |
/* 00e0 */ "\xb3\xc2\x55\xe4\xf9\xa1\x43\xdd\x17\x6e\x04\xd4\xb0\x07\xe0\x0f" //..U...C..n...... |
/* 00f0 */ "\xba\x09\x02\xdb\xcf\xfe\xa7\x61\xe6\x2c\x9a\x99\x4b\x69\xc7\x06" //.......a.,..Ki.. |
/* 0000 */ "\xde\x56\x1c\x30\xd9\xfd\x9f\xfb\xb1\x2d\x99\xa9\x38\xb9\xa4\xbb" //.V.0.....-..8... |
/* 0010 */ "\x60\x73\x2e\x1e\x7d\xfe\x0d\x83\xaf\x97\x22\x86\xad\xf2\x50\xee" //`s..}....."...P. |
/* 0020 */ "\x06\x9f\x42\x4f\x4d\x5a\x1b\xdf\x67\xce\xff\x8a\x93\x94\x5d\x33" //..BOMZ..g.....]3 |
/* 0030 */ "\x54\x60\x83\xeb\x9f\x39\x1b\x37\xc4\x98\xf0\xe1\xe4\xbb\x4a\x31" //T`...9.7......J1 |
/* 0040 */ "\xe6\x72\xf5\x4e\x02\x3a\x11\x3a\x79\x3d\xd8\xbd\x54\x03\x49\x03" //.r.N.:.:y=..T.I. |
/* 0050 */ "\x03\x67\xf7\xac\xc8\x28\x96\xf8\x17\x5f\x2c\x75\xd4\x35\x94\x87" //.g...(..._,u.5.. |
/* 0060 */ "\xd8\x9b\x69\x0c\xaa\xc1\x87\x7e\xf6\x39\xd2\xe1\x5f\x48\x3b\x2b" //..i....~.9.._H;+ |
/* 0070 */ "\xb9\xaa\x8e\x50\xa4\x54\xfe\xc3\x23\x44\x1b\x03\xe7\xc3\xc0\xa5" //...P.T..#D...... |
/* 0080 */ "\x2f\x1c\x41\xe5\x33\x36\x8e\x1a\x1b\x06\x11\xe0\x25\x72\x1d\x06" ///.A.36......%r.. |
/* 0090 */ "\x11\x92\x2c\x37\xf8\x82\x6c\x67\x67\x2a\x8a\x48\x5d\xbc\x38\xd8" //..,7..lgg*.H].8. |
/* 00a0 */ "\xc9\x85\xca\x90\x1f\x5d\x8c\xef\xb4\x0a\xdb\xcd\x36\x6b\xc2\x01" //.....]......6k.. |
/* 00b0 */ "\x70\x64\xf5\x2f\x03\x33\xab\x6d\xd8\x22\xba\xe0\x4c\xa3\xc1\x70" //pd./.3.m."..L..p |
/* 00c0 */ "\x3a\xa9\x44\xf6\x40\xb1\xe0\x7b\x20\xb8\xd5\xae\xb6\x45\x7e\x9a" //:.D.@..{ ....E~. |
/* 00d0 */ "\xec\x7f\x2b\xca\x0e\xab\x49\x53\x60\x76\x6f\x0a\x1f\x5a\xd5\x21" //..+...IS`vo..Z.! |
/* 00e0 */ "\x0d\xe0\xc4\x49\x14\x49\x12\x71\x60\x00\xd5\x89\xf2\xda\xb6\x05" //...I.I.q`....... |
/* 00f0 */ "\x00\x69\x15\x36\xf6\x69\x28\xaa\x05\xd5\x90\x2e\xc4\x3c\x70\x0a" //.i.6.i(......<p. |
/* 0000 */ "\x67\x31\x11\xbb\xd6\x46\xf1\x44\x17\xd3\x1b\x97\x24\x56\x4b\x0c" //g1...F.D....$VK. |
/* 0010 */ "\x9d\xca\x03\x54\xb1\x23\xea\xb0\x72\x75\x91\xbd\xe3\xd2\x04\x7f" //...T.#..ru...... |
/* 0020 */ "\x58\x1b\x49\xfa\x76\xca\x90\x3b\x22\xbf\xef\x21\xb2\x88\xed\x5e" //X.I.v..;"..!...^ |
/* 0030 */ "\x00\x6f\xef\xb4\xcc\xfd\xc7\x41\x71\x8f\xa5\x5d\xfe\x95\xc4\xe4" //.o.....Aq..].... |
/* 0040 */ "\xb9\xe2\x5d\xa0\x75\xa1\xdc\x21\xcc\x18\x84\xec\xc2\xe5\xba\xb0" //..].u..!........ |
/* 0050 */ "\xf1\xf5\xae\x76\x6d\x7f\x2e\x7b\x44"                             //...vm..{D |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(20,10,108,17) op=(20,10,108,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=26 data_len=26 [0x00 0x00 0x01 0x09 0x02 0x0e 0x03 0x09 0x04 0x09 0x02 0x04 0x03 0x09 0x04 0x09 0x05 0x04 0x06 0x09 0x04 0x09 0x06 0x04 0x06 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,41,204,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(20,41,93,17) op=(20,41,93,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=57 data_len=24 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09 0x0d 0x05 0x0e 0x04 0x09 0x0b 0x0f 0x05 0x10 0x09 0x11 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,41,706,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(224,41,46,17) op=(224,41,46,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=57 data_len=12 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,41,84,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(930,41,58,17) op=(930,41,58,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=57 data_len=16 [0x12 0x00 0x09 0x09 0x0f 0x05 0x0c 0x09 0x0f 0x05 0x13 0x09 0x0f 0x08 0x14 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(963,13,34,17) op=(963,13,34,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=962 glyph_y=29 data_len=12 [0x15 0x00 0x16 0x08 0x14 0x03 0x0c 0x03 0x0b 0x05 0x09 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(794,705,19,17) op=(794,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=793 glyph_y=721 data_len=4 [0x17 0x00 0x18 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(835,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(836,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(842,705,12,17) op=(842,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=841 glyph_y=721 data_len=2 [0x17 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(877,704,23,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=876 glyph_y=721 data_len=2 [0x02 0x00] |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=900 glyph_y=721 data_len=4 [0x19 0x00 0x03 0x05] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(930,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(931,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(937,705,12,17) op=(937,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=936 glyph_y=721 data_len=2 [0x1a 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(971,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(972,703,30,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(978,705,19,17) op=(978,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=977 glyph_y=721 data_len=4 [0x1b 0x00 0x1a 0x07] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,63,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(811,736,60,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(817,738,49,17) op=(817,738,49,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=816 glyph_y=754 data_len=12 [0x1c 0x00 0x0f 0x07 0x0a 0x09 0x0f 0x09 0x10 0x09 0x0c 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(920,738,60,17) op=(920,738,60,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=919 glyph_y=754 data_len=14 [0x1d 0x00 0x0f 0x0a 0x1e 0x09 0x1e 0x09 0x0b 0x09 0x13 0x09 0x0c 0x08] |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x50\x08\x00\x00\x00\x30\x85" //...h. ..pP....0. |
/* 0010 */ "\xa6\x69\x36\xed\xe1\x6d\x50\xc8\x0b\x64\x2b\x01\x50\x7f\xbb\x7e" //.i6..mP..d+.P..~ |
/* 0020 */ "\xa3\xad\xdf\x6b\x3c\xd3\xc9\xe2\xb0\x3d\xc9\xf6\x37\xc2\x3b\xa2" //...k<....=..7.;. |
/* 0030 */ "\xbb\x76\x4f\x49\x08\xa6\x53\x5c\xf6\x87\x94\x21\x38\x40\xed\x16" //.vOI..S....!8@.. |
/* 0040 */ "\xce\x8c\xe2\x39\x2f\x0c\xc6\xcc\x10\x25\xa4\x89\xf7\x32\x70\x0b" //...9/....%...2p. |
/* 0050 */ "\x1d\x81\xa3\x22\x85\xd8\x29\xd3\xfc\x15"                         //..."..)... |
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
/* 0000 */ "\x00\x09\xea"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 2534 bytes |
// Recv done on RDP Wab Target (4) 2534 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x89\xdb\x08\x00\x00\x00\xa8" //...h. ..p....... |
/* 0010 */ "\x3c\xa6\xc0\x4c\xf8\xd9\x30\x76\xab\x1c\xeb\x20\xdc\xb7\x51\xc1" //<..L..0v... ..Q. |
/* 0020 */ "\xfc\xe9\x5c\x7a\x86\xc7\x35\x75\x60\x49\x2d\xdb\xa2\x30\xf7\xfd" //...z..5u`I-..0.. |
/* 0030 */ "\x95\x80\x4a\x77\xb0\xde\xc2\xb3\xd1\xf4\xdf\xf4\x34\x93\x1e\x7b" //..Jw........4..{ |
/* 0040 */ "\x15\xde\x0b\x2a\x30\x27\x09\xe6\x50\xc0\xb3\xf8\x0b\xa2\x1e\x8e" //...*0'..P....... |
/* 0050 */ "\x46\xf9\xb4\xa3\xd5\xea\x40\x4d\xa3\x48\x4f\x9a\xc8\x3d\x92\xcd" //F.....@M.HO..=.. |
/* 0060 */ "\xd3\x7a\x6c\xe1\x8d\x56\xf8\xe4\x80\x1c\x1f\xf8\xb3\xd2\x59\x8b" //.zl..V........Y. |
/* 0070 */ "\x96\xc3\x68\x8a\xc9\xba\x3d\xd6\xcb\xbf\x25\xec\xc6\xde\xe0\xd0" //..h...=...%..... |
/* 0080 */ "\xfb\x23\xb9\x0e\x50\xaf\xdb\xa2\xe1\xd3\x00\x9e\xf6\xa5\x48\x48" //.#..P.........HH |
/* 0090 */ "\x72\x69\x01\x45\xbe\x9a\xf0\x82\xf5\x32\xd3\xd1\xe4\x6f\xac\x72" //ri.E.....2...o.r |
/* 00a0 */ "\xdc\xa9\x33\x08\x08\xe8\xf8\x07\xe4\x9b\x58\xe9\xdf\xde\xe8\xde" //..3.......X..... |
/* 00b0 */ "\x98\x49\x8c\xfc\x2e\x4e\x4e\x9d\x2e\x4c\x87\x6d\xd1\xf9\x6e\xe6" //.I...NN..L.m..n. |
/* 00c0 */ "\xee\xc1\x97\x72\xc8\x14\xc4\x22\x12\xa6\xd2\x6a\x15\x5e\x1e\xde" //...r..."...j.^.. |
/* 00d0 */ "\x45\xd6\xe4\xd1\x0f\xaa\xb6\x79\x80\x20\xfb\x01\xc1\x6e\xe5\xda" //E......y. ...n.. |
/* 00e0 */ "\x59\x11\x93\x4d\x99\xc4\xca\xaf\x1a\x43\xe8\xaf\xf6\x50\x86\x30" //Y..M.....C...P.0 |
/* 00f0 */ "\x50\x5f\xd8\x4f\xf9\x59\xfa\xd5\x17\x53\xdb\x83\x8d\xa0\xd9\xf5" //P_.O.Y...S...... |
/* 0000 */ "\xfc\x92\xaa\xc4\xcb\x6b\x2a\x12\x01\xdc\x6b\x81\xbe\x98\x32\xc2" //.....k*...k...2. |
/* 0010 */ "\x92\x36\x51\x73\x71\x42\x1d\x1e\xb9\x16\x3b\x7f\x1b\xc4\xaf\x7b" //.6QsqB....;....{ |
/* 0020 */ "\xc6\x9f\x00\x7f\x79\xaf\x93\xd3\x7d\xc9\x80\xc3\x67\x27\x33\xd7" //....y...}...g'3. |
/* 0030 */ "\xb6\x96\x1a\x3f\x03\x55\x47\x8c\xde\x3c\x99\x58\xb3\xb4\x2c\x88" //...?.UG..<.X..,. |
/* 0040 */ "\x58\xa1\x26\x48\x7f\x69\xb9\x8e\xda\x3b\x7a\xd3\xfb\x75\xce\x91" //X.&H.i...;z..u.. |
/* 0050 */ "\xa3\x8a\x2e\xc0\xbb\xc9\x16\xce\x54\xb3\x7a\x8b\xf8\x40\xdc\xe1" //........T.z..@.. |
/* 0060 */ "\x92\xa9\x8d\x76\x42\xa5\xdf\xba\x13\x5f\x44\x77\xab\xd2\x01\xea" //...vB...._Dw.... |
/* 0070 */ "\x3f\xd2\xe6\x27\x5c\x8a\xe5\xe1\xf5\xed\x4a\xeb\x75\xd9\x2f\xd3" //?..'......J.u./. |
/* 0080 */ "\xe3\xbb\xef\x7c\x62\x94\x0a\x2f\x13\x6d\xc6\x53\x42\xf5\xb6\xe1" //...|b../.m.SB... |
/* 0090 */ "\xd5\xb6\x72\x5d\xd5\x40\xa0\xac\x9f\xf9\xce\x2b\x9b\x46\x1a\xce" //..r].@.....+.F.. |
/* 00a0 */ "\x26\xda\x31\x86\x77\x23\x7f\xd5\x16\xf3\x19\xe4\xda\xee\x10\x0b" //&.1.w#.......... |
/* 00b0 */ "\x36\xf1\xd5\x3d\x98\x5b\x11\x73\x5d\x75\x34\x32\x16\x80\x59\x6f" //6..=.[.s]u42..Yo |
/* 00c0 */ "\x39\x07\x24\x64\x28\xa1\x1a\xd0\xb2\x2e\xb2\x57\x36\x03\x85\xbf" //9.$d(......W6... |
/* 00d0 */ "\x17\x7a\xcc\xf7\xda\xb4\x56\xc9\x30\xc5\x07\x68\x35\xbb\xaa\x23" //.z....V.0..h5..# |
/* 00e0 */ "\xbf\x72\xcf\xef\xa7\xc0\xe7\x6f\xd8\x01\x6a\x66\x9b\x14\x33\xd3" //.r.....o..jf..3. |
/* 00f0 */ "\xac\xec\xbb\x72\xfc\x52\x45\xd0\x24\xb5\xf9\x43\xf4\xd0\x99\x7a" //...r.RE.$..C...z |
/* 0000 */ "\xcb\x2f\x42\x8c\x02\xa1\x1f\xe1\xc8\x54\x47\xd9\x3a\x0e\xbb\xe4" //./B......TG.:... |
/* 0010 */ "\x6b\x5b\xba\x30\xa9\xf5\xc7\x70\x60\x0e\x4b\x6f\x87\x43\xfb\x69" //k[.0...p`.Ko.C.i |
/* 0020 */ "\xc1\xde\x58\xe7\x69\x4f\x27\xb9\x4b\xdd\xad\x3b\x9d\x6e\x1c\xcb" //..X.iO'.K..;.n.. |
/* 0030 */ "\xb4\x42\x8f\x0d\xa3\x58\x7a\x2e\x30\x0c\x83\x87\xac\x23\x26\x57" //.B...Xz.0....#&W |
/* 0040 */ "\x4c\x0e\x81\x87\x02\x96\xd5\xaf\x71\x90\x11\xac\x10\x39\xb7\x79" //L.......q....9.y |
/* 0050 */ "\xd8\x1e\x4f\x83\x91\x20\x32\xad\x7f\x8d\x29\xc6\xe4\x0a\x25\x16" //..O.. 2...)...%. |
/* 0060 */ "\x2d\x3e\x53\x12\x15\xab\x03\x09\x2e\x30\x05\xf5\xd0\x3a\x8b\x84" //->S......0...:.. |
/* 0070 */ "\xaf\xac\xa3\x85\xac\x00\xf9\xa0\x50\xaa\xe0\xcf\xff\xb5\xbc\x48" //........P......H |
/* 0080 */ "\x0e\xc4\xab\xb6\xc2\xcd\xc5\xb6\xd2\x19\xcd\xea\xe1\x02\x87\x13" //................ |
/* 0090 */ "\xb5\x12\x0f\x50\x7d\x38\xdc\x6e\x66\xa2\x1a\x53\x97\xe5\xca\x6a" //...P}8.nf..S...j |
/* 00a0 */ "\x50\xed\x6c\x4e\x51\xf5\x09\xbf\x37\x02\xbb\x59\xb7\x7e\x71\xec" //P.lNQ...7..Y.~q. |
/* 00b0 */ "\x56\x6e\x04\x84\xdc\x23\x15\x54\x84\x96\x9f\xf0\x60\x8a\x14\xc0" //Vn...#.T....`... |
/* 00c0 */ "\xe2\x14\x5f\x09\xc7\xd6\x4e\x9c\x70\x8f\x77\x07\x44\x94\xad\x37" //.._...N.p.w.D..7 |
/* 00d0 */ "\xd5\x9a\xba\x5d\x10\x83\x96\x6d\x81\x9b\xb9\xa4\x50\xbd\x16\xbb" //...]...m....P... |
/* 00e0 */ "\x8a\x0e\x30\x99\x50\x9b\xd8\x46\xa6\x6a\x3e\x3c\x60\x83\xc1\x7b" //..0.P..F.j><`..{ |
/* 00f0 */ "\xe0\x14\x55\x01\xbe\x8c\x01\x2f\x53\x3a\x9c\x30\x01\x32\x35\x9a" //..U..../S:.0.25. |
/* 0000 */ "\x4f\x6c\x58\xa9\x02\xd5\xa2\x4f\xdd\xd5\x4e\x74\xf3\x42\xc6\xe4" //OlX....O..Nt.B.. |
/* 0010 */ "\x72\xd9\xa4\xe1\x19\x8b\xd3\xcd\x36\x51\xbe\xb0\x60\x7e\xb7\x44" //r.......6Q..`~.D |
/* 0020 */ "\x29\x05\xf0\x64\x97\xbb\x81\x01\x78\x7a\xb7\x00\x69\xbf\xbb\x3e" //)..d....xz..i..> |
/* 0030 */ "\x9b\x20\xca\xf3\x16\x91\xf0\xdc\x90\xb8\xe5\x24\x11\xd8\x9c\x56" //. .........$...V |
/* 0040 */ "\xea\x24\x33\xdd\x3a\xa9\x64\x70\xd2\xed\xf5\x53\x33\x72\x04\x45" //.$3.:.dp...S3r.E |
/* 0050 */ "\xe7\x93\x9d\xa1\x1a\x13\x31\xb0\x34\x16\x0b\x7c\x22\x6e\x06\x52" //......1.4..|"n.R |
/* 0060 */ "\x8d\xf6\x79\x4b\x1f\x9f\x1e\xe2\x13\xfc\x55\xdf\xf7\xff\xa9\x82" //..yK......U..... |
/* 0070 */ "\x1d\x06\x2f\xea\x01\x0a\x50\x5f\x08\x2c\x03\xe1\xf3\x8d\x80\x27" //../...P_.,.....' |
/* 0080 */ "\x4e\xf6\x32\xda\x2b\xac\x88\xed\x7b\x9f\x0b\x5d\x15\x1c\x2a\xbe" //N.2.+...{..]..*. |
/* 0090 */ "\xa2\x26\x8f\x7c\xac\xbe\x8f\xb7\x1a\x59\xeb\xdb\xb8\xbb\x10\x67" //.&.|.....Y.....g |
/* 00a0 */ "\xc9\xbc\xf2\x33\xa9\x74\xfc\x7a\xbc\x68\x2a\x13\x86\xe3\x64\xe8" //...3.t.z.h*...d. |
/* 00b0 */ "\x1f\x14\x67\x5f\x69\x11\x49\x27\xd7\xeb\xf9\x56\xd2\xdf\x29\x35" //..g_i.I'...V..)5 |
/* 00c0 */ "\xa7\x6e\x57\x35\xa2\x08\x28\xb1\xd3\xab\x71\x15\x35\xc4\xcd\x72" //.nW5..(...q.5..r |
/* 00d0 */ "\xa8\x5d\xfe\xc5\xe6\x65\x9d\x19\xb6\xd5\xb2\x5e\x37\x89\x9f\xdf" //.]...e.....^7... |
/* 00e0 */ "\x7e\xc1\xf6\xb7\xbc\x8c\xd4\x5a\x77\x86\x7c\xee\x84\x4b\x82\x24" //~......Zw.|..K.$ |
/* 00f0 */ "\x81\x04\x12\x7f\x4f\xe5\x8e\xad\x78\xa4\x1d\x61\x6d\x7d\x5a\x67" //....O...x..am}Zg |
/* 0000 */ "\xb7\x1a\xd5\x26\x92\x83\x38\x9c\xc5\x1e\xa8\x8d\x1a\x78\xd9\x8f" //...&..8......x.. |
/* 0010 */ "\x4e\xef\x96\xb9\xa1\x66\x69\x0f\x8a\x13\x1f\xf4\x7e\x2c\xdc\xd1" //N....fi.....~,.. |
/* 0020 */ "\x16\x49\x4b\x34\xa1\xa7\xe1\x98\x7a\xc2\x4a\x08\xb7\x4e\x3b\xd4" //.IK4....z.J..N;. |
/* 0030 */ "\xca\x42\x6c\xa6\x3e\x44\xc2\xf3\x27\x11\x80\x72\x6d\x01\xbb\xf6" //.Bl.>D..'..rm... |
/* 0040 */ "\x17\x3a\x74\x97\x70\x76\xa8\x1f\xab\x53\x2f\x40\x71\x19\x0d\xb2" //.:t.pv...S/@q... |
/* 0050 */ "\xd6\x99\xb2\x14\x92\xd3\x99\xa4\xca\x03\x18\xe6\x8d\x76\x9a\xcd" //.............v.. |
/* 0060 */ "\x46\x1e\x46\xf3\x0b\x6a\x4c\x6d\xee\xdd\x07\xbf\x79\x47\x68\x52" //F.F..jLm....yGhR |
/* 0070 */ "\xbc\x0f\xaf\x63\x89\x29\x91\x3e\x37\x25\x03\x37\x1a\x3c\xde\x1c" //...c.).>7%.7.<.. |
/* 0080 */ "\x5e\x75\x46\x61\x09\x34\x0d\x5a\x4e\xfe\x19\x1b\x62\x9b\x8f\x68" //^uFa.4.ZN...b..h |
/* 0090 */ "\x52\xa5\xbe\x9d\x1d\x53\x63\x3a\x61\xa1\x5b\x00\x35\xf1\x5f\xb9" //R....Sc:a.[.5._. |
/* 00a0 */ "\x63\x00\xa2\x83\x73\xc3\xd3\xcd\xd4\x07\x16\x39\x67\x57\xfb\x05" //c...s......9gW.. |
/* 00b0 */ "\xd3\x69\x85\x77\x0f\xef\x66\x1b\x65\x97\xfe\x14\x7f\x80\x5e\xac" //.i.w..f.e.....^. |
/* 00c0 */ "\x37\xa1\x0e\x31\xcf\x8a\x29\xe0\xc1\xe2\xc9\x88\x73\x35\xb3\x2d" //7..1..).....s5.- |
/* 00d0 */ "\xf3\x69\xe0\xa3\x9c\x46\xfe\x74\x24\xb3\x78\x90\x58\x6d\x41\x0b" //.i...F.t$.x.XmA. |
/* 00e0 */ "\x03\x14\x85\x09\x85\xe3\xe9\xb6\xae\xe0\xb2\xc5\x0e\x3f\x0f\x66" //.............?.f |
/* 00f0 */ "\xb1\x94\xf7\xc7\xb3\x37\x34\xfe\xfd\x50\xdb\x0d\xb3\xf1\xdc\x41" //.....74..P.....A |
/* 0000 */ "\xa1\xf8\xcb\x43\xa1\x42\xaf\xd3\x19\x35\xfc\x1e\x33\x1b\x46\x0c" //...C.B...5..3.F. |
/* 0010 */ "\x38\xea\x2c\x38\x65\xd0\x09\x88\xf8\x3c\x3e\x69\xf1\x51\x6d\x45" //8.,8e....<>i.QmE |
/* 0020 */ "\x23\x39\x23\x45\xd1\x3a\x98\x6b\x1b\x68\x52\x15\x72\xb5\xa5\x51" //#9#E.:.k.hR.r..Q |
/* 0030 */ "\x3c\x86\x33\xd7\x99\x18\xa6\xe4\xe4\x06\x83\xde\xb6\xf5\x6a\x63" //<.3...........jc |
/* 0040 */ "\x72\xd9\xa2\x1d\xec\xd0\x63\x22\xa2\x2b\x26\x4c\xbb\x55\x48\x9b" //r.....c".+&L.UH. |
/* 0050 */ "\xc6\x2c\x44\xe9\x35\x42\x5a\x75\xbd\xd2\xf3\xe8\x64\x09\x92\x68" //.,D.5BZu....d..h |
/* 0060 */ "\xe9\x61\xc0\xc6\x5c\xd7\x04\xca\x5c\xe4\x61\x96\x15\x19\xf5\x4e" //.a........a....N |
/* 0070 */ "\x4c\xc9\xa0\xd6\x7b\x7d\xcf\xb1\x59\xe4\xd2\x57\x97\xbc\x38\x00" //L...{}..Y..W..8. |
/* 0080 */ "\x76\x9a\x6c\xb5\x8d\x3c\xc6\x5e\xa5\x6e\x65\xac\x07\x37\x1f\xfa" //v.l..<.^.ne..7.. |
/* 0090 */ "\x1c\x41\x7a\x3c\x02\x10\x59\x07\x7a\xf2\x0e\xb3\x98\xb3\xa5\x6c" //.Az<..Y.z......l |
/* 00a0 */ "\x79\x3f\x25\x0a\xeb\xdd\x85\x17\x2c\x9f\x2c\x06\x4c\x46\x30\x71" //y?%.....,.,.LF0q |
/* 00b0 */ "\x2d\x16\x47\xce\xcb\xc5\xbd\x64\x73\x86\xed\x8f\xbe\xf8\x5b\xa3" //-.G....ds.....[. |
/* 00c0 */ "\x9c\x82\x38\x80\x30\x37\x77\x4a\x48\x73\xcd\x56\xe9\x5a\xaa\x3d" //..8.07wJHs.V.Z.= |
/* 00d0 */ "\x10\x9f\x26\xde\x95\x37\xf8\x1f\x75\x25\xbe\x45\xca\x38\x7b\x5f" //..&..7..u%.E.8{_ |
/* 00e0 */ "\x4e\x31\xf2\xfd\xbb\x08\xb4\x45\xf9\xdb\x3e\xa0\x91\x25\x43\x1a" //N1.....E..>..%C. |
/* 00f0 */ "\x55\x4c\x0e\x24\x39\x40\x01\xda\xbe\x50\xc1\xfa\xef\xbd\x6c\x04" //UL.$9@...P....l. |
/* 0000 */ "\xc2\x82\x38\x54\x35\x5c\x35\xdb\xd2\x22\x93\x0e\x0f\x9a\xa6\xaf" //..8T5.5.."...... |
/* 0010 */ "\xb9\x9f\xbd\x95\xbf\x2a\x50\xdc\x9b\xa7\x47\x23\x64\x29\x78\xb7" //.....*P...G#d)x. |
/* 0020 */ "\x6a\x9b\xe2\x16\x84\xcb\x78\xc6\xf7\xa7\x0e\xc8\xd9\xf9\xe2\xf7" //j.....x......... |
/* 0030 */ "\x18\x02\xfa\x5f\x03\xe2\x03\x4e\x4d\x71\x04\x11\x52\x9e\x69\x62" //..._...NMq..R.ib |
/* 0040 */ "\x76\x72\x92\x69\x42\xaa\x2e\x2f\xe4\x49\x86\x7f\x53\x8e\x81\xcd" //vr.iB../.I..S... |
/* 0050 */ "\xc0\x9e\x0b\x9d\x7f\xf3\x1a\x3d\xae\x2d\xd3\xea\xec\x29\x3c\x90" //.......=.-...)<. |
/* 0060 */ "\x99\x72\x16\x7f\x2a\x8c\x3c\x3b\x66\xa4\xeb\xe0\x34\x44\x95\xf9" //.r..*.<;f...4D.. |
/* 0070 */ "\x3c\xef\x86\x38\x5f\xf4\xf2\x08\xf9\xf2\xc2\xf1\xf7\xa5\xd8\x16" //<..8_........... |
/* 0080 */ "\x4e\xa1\x57\xcd\x1e\x32\xe8\xf0\xa0\x6b\x59\x53\xd9\x0f\x9c\x54" //N.W..2...kYS...T |
/* 0090 */ "\xb8\x93\x47\xca\xa7\x4d\xc3\x68\x37\x0f\x49\x18\xf7\x5c\x13\x67" //..G..M.h7.I....g |
/* 00a0 */ "\x2e\xe1\x34\x9d\x21\xbb\x24\x3b\x9c\x3a\x95\xad\xdc\x95\xc3\xc0" //..4.!.$;.:...... |
/* 00b0 */ "\x69\x68\x76\x18\xb8\xba\x02\x2c\xac\x5f\xd7\xa6\x75\x38\x8b\x08" //ihv....,._..u8.. |
/* 00c0 */ "\xf1\xe5\x40\x03\xd6\x1f\xc6\xc5\xf8\x43\x22\xce\xd6\x14\xf3\x78" //..@......C"....x |
/* 00d0 */ "\x85\x8d\x6c\x4a\x83\x05\x4b\xe8\x81\xb8\x8e\x3a\x18\x5c\x47\xdd" //..lJ..K....:..G. |
/* 00e0 */ "\x8a\xbc\xad\x64\x34\xa7\x68\xf1\xd6\x28\x4d\xc0\x5e\x3b\x38\x11" //...d4.h..(M.^;8. |
/* 00f0 */ "\xdd\x55\x64\x70\xd7\x21\x8d\x5f\xe8\xc2\x57\x86\x9a\x89\xa3\x85" //.Udp.!._..W..... |
/* 0000 */ "\x6e\x5d\x1f\x00\x35\xb3\x2a\xfc\xeb\x50\x1a\xc6\x4c\x24\x80\x51" //n]..5.*..P..L$.Q |
/* 0010 */ "\xd2\xab\xe6\x0e\x85\xc4\x1e\x3f\xf7\x6d\x9b\xf0\xdd\xc1\xab\xc4" //.......?.m...... |
/* 0020 */ "\x7a\xc7\xf6\x06\xb0\x5f\x3a\x72\x6d\xd6\x18\x50\x5f\xc9\xe6\xf7" //z...._:rm..P_... |
/* 0030 */ "\x30\x15\xa9\x2b\x87\xa5\x2d\x8c\xb6\x83\x04\x3e\x8a\x2e\xae\x1f" //0..+..-....>.... |
/* 0040 */ "\x29\x4d\x80\x98\x1a\xdc\x4d\x4a\x34\xf0\x30\x34\x58\xc8\xc8\x18" //)M....MJ4.04X... |
/* 0050 */ "\x5b\xc8\x33\xb5\x2b\x8b\x37\x86\xf8\x0f\xd2\xfa\x0d\x2e\xc9\xc6" //[.3.+.7......... |
/* 0060 */ "\x93\x86\x9b\x32\xf6\xa3\x2b\xad\x0d\xb2\xc3\x3d\x60\x40\xb8\x60" //...2..+....=`@.` |
/* 0070 */ "\xae\xde\x2e\x20\xcb\x75\x3a\x99\x87\x00\x4c\xdc\x7e\x6e\x18\x46" //... .u:...L.~n.F |
/* 0080 */ "\x86\x80\x16\x74\xfb\xe7\x8d\x4b\x21\x24\xe3\xbb\xda\xec\x7e\xc3" //...t...K!$....~. |
/* 0090 */ "\x77\x07\x6c\xb3\x09\xe4\x47\x4f\xf0\x23\x14\x08\x0e\x60\xc0\x4b" //w.l...GO.#...`.K |
/* 00a0 */ "\x73\xae\x9c\x96\xb5\x3a\x51\x58\xd1\x06\x4d\x14\xbb\x7f\xf4\x3b" //s....:QX..M....; |
/* 00b0 */ "\xc4\xf2\x7c\xeb\x17\x50\x1a\x50\xf6\x68\x5c\x1d\xd0\x15\x4d\xb7" //..|..P.P.h....M. |
/* 00c0 */ "\x91\x3a\x7f\x17\x5c\xb7\x28\xe3\x57\x4e\xd0\xcd\x27\xb9\xfe\xdf" //.:....(.WN..'... |
/* 00d0 */ "\x25\x4c\x40\x58\x5a\xd5\xeb\xbe\x94\xf6\x88\x0d\x0e\xdc\x7b\xd5" //%L@XZ.........{. |
/* 00e0 */ "\x5e\xe3\x0b\xb3\xbe\x4d\x6f\xc7\x2e\xff\xb2\xda\x54\x1a\xc4\x8d" //^....Mo.....T... |
/* 00f0 */ "\x85\x09\xcd\x08\x27\x65\x54\x2d\xc7\x4f\x8f\xb4\xe1\xcc\x7b\xc2" //....'eT-.O....{. |
/* 0000 */ "\xe2\x90\x8e\xa6\xbc\x74\x8a\xcb\xc5\xe3\x43\x39\x7b\xe5\x4d\x36" //.....t....C9{.M6 |
/* 0010 */ "\x2f\x5e\x27\xf5\x41\xae\xe0\x74\x85\x65\x02\x3b\x4c\xfe\xf0\x1b" ///^'.A..t.e.;L... |
/* 0020 */ "\x76\xb3\xf0\x5a\x48\x64\x3d\x73\x8f\x2e\xb0\x65\x13\xbc\xa0\x3b" //v..ZHd=s...e...; |
/* 0030 */ "\xda\x08\x7d\x2e\x9a\x3c\x8b\x96\xc1\xf5\xc8\x16\xe5\x0a\x1c\x75" //..}..<.........u |
/* 0040 */ "\x7f\x1c\x9d\x0c\x19\xbd\x42\x8c\x2d\x89\x35\x73\xd6\x15\x10\xfb" //......B.-.5s.... |
/* 0050 */ "\xe1\xef\x5c\x10\xc9\x67\x04\xc7\x7e\xe6\x5a\xdd\xea\x92\x7a\x9c" //.....g..~.Z...z. |
/* 0060 */ "\x83\xb6\xe3\x71\xe2\xc9\x54\x30\xbe\x10\xa2\x88\x67\xd5\xd3\x99" //...q..T0....g... |
/* 0070 */ "\x6f\x7a\x24\xab\xf5\xc6\xe6\x56\xb2\x8a\x60\x28\xe1\x9a\x51\xc3" //oz$....V..`(..Q. |
/* 0080 */ "\x82\x77\x29\x47\xc3\x9c\xb7\xdc\x74\x4c\x41\xff\xb4\x9a\xde\xa7" //.w)G....tLA..... |
/* 0090 */ "\x33\x3d\x0f\x2e\x22\x99\x7d\x15\x27\xcd\x77\x6e\x37\x2f\x77\x53" //3=..".}.'.wn7/wS |
/* 00a0 */ "\xf6\xf5\x9c\x81\xae\xdc\x7e\x68\x94\x1e\xf8\xcd\x9d\x89\xec\xd1" //......~h........ |
/* 00b0 */ "\x1e\x9d\xd8\x03\x0a\xc8\xea\x2c\xcc\xec\x1d\x98\x6b\x56\x7a\x96" //.......,....kVz. |
/* 00c0 */ "\xab\x36\xad\xc9\xa9\xf1\x4a\x21\x4b\x96\xb1\xb8\x34\xdc\xbe\x92" //.6....J!K...4... |
/* 00d0 */ "\x35\xab\x08\xf8\x8d\x4e\x08\xa5\x27\x4d\x6f\x8a\xad\x59\x03\xaa" //5....N..'Mo..Y.. |
/* 00e0 */ "\xff\xba\x90\x5a\x41\xe8\x93\x81\xc3\x1b\x0f\x60\x71\x11\x41\xfc" //...ZA......`q.A. |
/* 00f0 */ "\x7c\xd4\x45\x11\x6e\x65\x90\x52\xfc\xbd\x9d\x8f\x4a\xf1\x66\x9b" //|.E.ne.R....J.f. |
/* 0000 */ "\x16\x93\xfd\x4c\x65\x8e\x78\xa2\x25\x8f\xe0\xb0\x2e\xd3\x2f\x80" //...Le.x.%...../. |
/* 0010 */ "\x2e\xe8\x98\xcc\xbf\xe1\x6e\xde\x54\x1b\xf1\x92\x2d\x57\x4b\x44" //......n.T...-WKD |
/* 0020 */ "\x71\x49\xdf\x4e\x90\x36\x0d\x32\x7c\xcb\x35\xc4\x4f\x04\xfd\x14" //qI.N.6.2|.5.O... |
/* 0030 */ "\x68\x8d\x91\x7d\xcf\x3e\xf9\xa0\x9e\xf5\xf0\x3f\xa4\x20\xe7\x3f" //h..}.>.....?. .? |
/* 0040 */ "\x6f\x59\x3a\x98\xae\xee\xfc\x41\x0a\x24\x0c\x68\xe2\x15\x23\x25" //oY:....A.$.h..#% |
/* 0050 */ "\x32\xcc\xbc\x7b\x12\x70\xd4\x76\x67\x92\xd1\xbf\xb3\x73\x44\xdf" //2..{.p.vg....sD. |
/* 0060 */ "\x93\x5a\x99\x73\xf2\x05\x5e\x4b\x7d\x9f\x53\x5f\x91\x4c\xea\x17" //.Z.s..^K}.S_.L.. |
/* 0070 */ "\xa9\x6a\xad\x87\x75\x8a\x23\xc4\x33\xb8\xdd\x6f\xb6\xbc\x1c\xc9" //.j..u.#.3..o.... |
/* 0080 */ "\x70\x3c\xe4\x15\xb2\x97\xaf\xaa\xd9\xaf\x33\xfe\xc7\xe7\x63\xed" //p<........3...c. |
/* 0090 */ "\xb0\xd4\x56\x74\x62\xb3\x63\x40\x3e\x2c\xd6\xc9\x4b\x1b\x28\x30" //..Vtb.c@>,..K.(0 |
/* 00a0 */ "\xf3\x5b\x2e\xd5\xb5\x85\xb5\xf9\xb9\xae\xfe\x69\xfe\x28\x9a\xfb" //.[.........i.(.. |
/* 00b0 */ "\x66\x6a\xaa\x0e\x67\x0d\x0c\xb3\xba\xcb\xa4\x1a\xf0\xba\xe9\x1b" //fj..g........... |
/* 00c0 */ "\x5d\x61\x01\x11\xe8\xcc\xc0\xe1\x3b\x46\xd7\xff\x67\x65\xba\xe1" //]a......;F..ge.. |
/* 00d0 */ "\x03\x89\x10\xc5\x02\xd8\x75\xc1\x61\x19\x2f\x17\x5f\x73\x13\x83" //......u.a./._s.. |
/* 00e0 */ "\x5b\x6e\x43\x1b\xe8\x4d"                                         //[nC..M |
// Dump done on RDP Wab Target (4) 2534 bytes |
// LOOPING on PDUs: 2511 |
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
// order(10):opaquerect(rect(0,87,15,66) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,87,15,66) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,153,1024,549) color=0x0008ec) |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(20,10,108,17) op=(20,10,108,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=26 data_len=26 [0x00 0x00 0x01 0x09 0x02 0x0e 0x03 0x09 0x04 0x09 0x02 0x04 0x03 0x09 0x04 0x09 0x05 0x04 0x06 0x09 0x04 0x09 0x06 0x04 0x06 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,41,204,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(20,41,93,17) op=(20,41,93,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=57 data_len=24 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09 0x0d 0x05 0x0e 0x04 0x09 0x0b 0x0f 0x05 0x10 0x09 0x11 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(219,41,706,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(224,41,46,17) op=(224,41,46,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=57 data_len=12 [0x07 0x00 0x08 0x09 0x09 0x08 0x0a 0x05 0x0b 0x09 0x0c 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(925,41,84,16) color=0x004398) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=004398 bk=(930,41,58,17) op=(930,41,58,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=57 data_len=16 [0x12 0x00 0x09 0x09 0x0f 0x05 0x0c 0x09 0x0f 0x05 0x13 0x09 0x0f 0x08 0x14 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(963,13,34,17) op=(963,13,34,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=962 glyph_y=29 data_len=12 [0x15 0x00 0x16 0x08 0x14 0x03 0x0c 0x03 0x0b 0x05 0x09 0x09] |
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
// order(10):opaquerect(rect(15,87,994,22) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,89,200,18) color=0x000273) |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(20,90,86,17) op=(20,90,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=106 data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x21 0x0b 0x22 0x09 0x23 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,89,702,18) color=0x000273) |
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
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(224,90,254,17) op=(224,90,254,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=106 data_len=66 [0x24 0x00 0x25 0x09 0x26 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x27 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x20 0x0e 0x21 0x0b 0x22 0x09 0x23 0x08 0x28 0x09 0x11 0x05 0x09 0x09 0x16 0x05 0x28 0x03 0x05 0x05 0x29 0x09 0x04 0x09 0x06 0x04 0x2a 0x09 0x2b 0x09 0x2c 0x05 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,89,80,18) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(930,90,31,17) op=(930,90,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=106 data_len=6 [0x2c 0x00 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,109,994,22) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,111,200,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(20,112,95,17) op=(20,112,95,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=128 data_len=22 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x20 0x07 0x21 0x0b 0x22 0x09 0x02 0x08 0x21 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,111,702,18) color=0x00ef7e) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(224,112,296,17) op=(224,112,296,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=128 data_len=70 [0x2c 0x00 0x2e 0x0a 0x2d 0x09 0x2f 0x0b 0x07 0x05 0x08 0x09 0x09 0x08 0x0c 0x05 0x0b 0x05 0x26 0x09 0x11 0x0d 0x16 0x09 0x0f 0x03 0x1e 0x09 0x01 0x09 0x20 0x0e 0x21 0x0b 0x22 0x09 0x02 0x08 0x21 0x09 0x28 0x09 0x11 0x05 0x10 0x09 0x30 0x09 0x28 0x09 0x05 0x05 0x29 0x09 0x04 0x09 0x02 0x04 0x21 0x09 0x23 0x09 0x2b 0x09 0x2c 0x05 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,111,80,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(930,112,31,17) op=(930,112,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=128 data_len=6 [0x2c 0x00 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,131,994,22) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,133,200,18) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(20,134,107,17) op=(20,134,107,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=150 data_len=26 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x1f 0x09 0x1e 0x07 0x0f 0x09 0x0c 0x09 0x0b 0x05 0x11 0x09 0x08 0x09 0x25 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,133,702,18) color=0x00cebd) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(224,134,230,17) op=(224,134,230,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=150 data_len=56 [0x08 0x00 0x13 0x08 0x13 0x08 0x0f 0x08 0x10 0x09 0x1e 0x09 0x0c 0x09 0x1f 0x05 0x31 0x07 0x08 0x04 0x22 0x08 0x0b 0x08 0x01 0x09 0x08 0x0e 0x11 0x08 0x11 0x09 0x1f 0x09 0x1e 0x07 0x0f 0x09 0x0c 0x09 0x0b 0x05 0x11 0x09 0x08 0x09 0x25 0x08 0x2b 0x09 0x24 0x05 0x12 0x09 0x12 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,133,80,18) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(930,134,28,17) op=(930,134,28,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=150 data_len=6 [0x24 0x00 0x12 0x09 0x12 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(787,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(788,703,30,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(794,705,19,17) op=(794,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=793 glyph_y=721 data_len=4 [0x17 0x00 0x18 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(835,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(836,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(842,705,12,17) op=(842,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=841 glyph_y=721 data_len=2 [0x17 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(877,704,23,18) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=876 glyph_y=721 data_len=2 [0x02 0x00] |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=900 glyph_y=721 data_len=4 [0x19 0x00 0x02 0x05] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(930,702,26,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(931,703,23,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(937,705,12,17) op=(937,705,12,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=936 glyph_y=721 data_len=2 [0x1a 0x00] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(971,702,33,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(972,703,30,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(978,705,19,17) op=(978,705,19,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=977 glyph_y=721 data_len=4 [0x1b 0x00 0x1a 0x07] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(810,735,63,23) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(811,736,60,20) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(817,738,49,17) op=(817,738,49,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=816 glyph_y=754 data_len=12 [0x1c 0x00 0x0f 0x07 0x0a 0x09 0x0f 0x09 0x10 0x09 0x0c 0x09] |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(920,738,60,17) op=(920,738,60,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=919 glyph_y=754 data_len=14 [0x1d 0x00 0x0f 0x0a 0x1e 0x09 0x1e 0x09 0x0b 0x09 0x13 0x09 0x0c 0x08] |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x65\x08\x00\x00\x00\xbf\x84" //...h. ..pe...... |
/* 0010 */ "\xdb\x0f\xb7\x8f\x6f\xe3\xbb\xe6\x1e\x1d\x48\x0a\xbb\x7d\x86\x49" //....o.....H..}.I |
/* 0020 */ "\x4b\x54\x02\x1c\xa3\x23\x82\xf4\x74\xd6\xc3\x75\x5b\x32\x5f\x09" //KT...#..t..u[2_. |
/* 0030 */ "\x51\x24\x2a\xb0\x42\x99\x0b\x43\xaf\x49\x77\xbc\x86\x50\x20\x7c" //Q$*.B..C.Iw..P | |
/* 0040 */ "\x11\xeb\x57\xc9\xa1\x76\x42\x12\xe5\xed\xcc\x6c\xce\x7f\x50\x1b" //..W..vB....l..P. |
/* 0050 */ "\x2d\xc1\x4d\x27\xfa\x1e\x5d\xbd\x83\x36\x1b\x3d\x9b\x1f\x32\x23" //-.M'..]..6.=..2# |
/* 0060 */ "\x72\x90\x44\xf6\x46\xcf\x56\x94\xdb\xda\xab\x08\x4f\xcb\xb1"     //r.D.F.V.....O.. |
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
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ffff bk=(878,705,10,17) op=(878,705,10,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=876 glyph_y=721 data_len=2 [0x02 0x00] |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x48\x08\x00\x00\x00\x89\xfa" //...h. ..pH...... |
/* 0010 */ "\xef\xa1\x71\x6e\xac\x6b\x0d\xcc\xa5\x2c\xab\x8f\xee\x6d\x05\xea" //..qn.k...,...m.. |
/* 0020 */ "\xb5\x8c\x0a\x4c\x83\x09\x53\x72\x03\x00\xaa\xc6\x02\x77\xe2\x59" //...L..Sr.....w.Y |
/* 0030 */ "\xb8\x68\x0f\x56\x4b\x79\x33\x61\x1a\x1b\xc7\x48\x8d\x27\x79\x38" //.h.VKy3a...H.'y8 |
/* 0040 */ "\xff\xd2\x4a\xab\x8c\xac\x70\x30\x3c\x45\x94\x3b\xe4\x6c\xf7\x84" //..J...p0<E.;.l.. |
/* 0050 */ "\xa1\xf2"                                                         //.. |
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
// order(27):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(901,705,15,17) op=(901,705,15,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=900 glyph_y=721 data_len=4 [0x19 0x00 0x02 0x05] |
// ======================================== |
// process_orders done |
// ===================> count = 8 |
// ~mod_rdp(): Recv bmp cache count  = 0 |
// ~mod_rdp(): Recv order count      = 402 |
// ~mod_rdp(): Recv bmp update count = 0 |
// RDP Wab Target (0): total_received=9060, total_sent=1807 |
} /* end indata */;

