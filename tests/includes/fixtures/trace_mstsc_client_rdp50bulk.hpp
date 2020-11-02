const char outdata[] = /* NOLINT */
{
// Listen: binding socket 3 on 0.0.0.0:3389 |
// Listen: listening on socket 3 |
// SocketTransport: recv_timeout=1000 |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 39 bytes |
// /* 0000 */ "\x03\x00\x00\x27\x22\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" // ...'"......Cooki |
// /* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x78\x0d\x0a\x01" // e: mstshash=x... |
// /* 0020 */ "\x00\x08\x00\x0b\x00\x00\x00"                                     // ....... |
// Dump done on RDP Client (4) 39 bytes |
// Front::incoming: CONNECTION_INITIATION |
// Front::incoming: receiving x224 request PDU |
// CR Recv: PROTOCOL TLS |
// CR Recv: PROTOCOL HYBRID |
// CR Recv: PROTOCOL HYBRID EX |
// Front::incoming: sending x224 connection confirm PDU |
// -----------------> Front::incoming: TLS Support Enabled |
// CC Send: PROTOCOL TLS |
// Sending on RDP Client (4) 19 bytes |
/* 0000 */ "\x03\x00\x00\x13\x0e\xd0\x00\x00\x00\x00\x00\x02\x01\x08\x00\x01" // ................ |
/* 0010 */ "\x00\x00\x00"                                                     // ... |
// Sent dumped on RDP Client (4) 19 bytes |
// SocketTransport::enable_server_tls() start |
// TLSContext::enable_server_tls() set SSL options |
// TLSContext::enable_server_tls() set SSL cipher list |
// TLSContext::X509_get_pubkey() |
// TLSContext::i2d_PublicKey() |
// TLSContext::i2d_PublicKey() |
// Incoming connection to Bastion using TLS version TLSv1.2 |
// TLSContext::Negociated cipher used ECDHE-RSA-AES256-GCM-SHA384 |
// SocketTransport::enable_server_tls() done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 462 bytes |
// /* 0000 */ "\x03\x00\x01\xce\x02\xf0\x80\x7f\x65\x82\x01\xc2\x04\x01\x01\x04" // ........e....... |
// /* 0010 */ "\x01\x01\x01\x01\xff\x30\x19\x02\x01\x22\x02\x01\x02\x02\x01\x00" // .....0..."...... |
// /* 0020 */ "\x02\x01\x01\x02\x01\x00\x02\x01\x01\x02\x02\xff\xff\x02\x01\x02" // ................ |
// /* 0030 */ "\x30\x19\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02\x01" // 0............... |
// /* 0040 */ "\x00\x02\x01\x01\x02\x02\x04\x20\x02\x01\x02\x30\x1c\x02\x02\xff" // ....... ...0.... |
// /* 0050 */ "\xff\x02\x02\xfc\x17\x02\x02\xff\xff\x02\x01\x01\x02\x01\x00\x02" // ................ |
// /* 0060 */ "\x01\x01\x02\x02\xff\xff\x02\x01\x02\x04\x82\x01\x61\x00\x05\x00" // ............a... |
// /* 0070 */ "\x14\x7c\x00\x01\x81\x58\x00\x08\x00\x10\x00\x01\xc0\x00\x44\x75" // .|...X........Du |
// /* 0080 */ "\x63\x61\x81\x4a\x01\xc0\xea\x00\x0c\x00\x08\x00\xa0\x05\x84\x03" // ca.J............ |
// /* 0090 */ "\x01\xca\x03\xaa\x0c\x04\x00\x00\xba\x47\x00\x00\x43\x00\x4c\x00" // .........G..C.L. |
// /* 00a0 */ "\x54\x00\x30\x00\x32\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // T.0.2........... |
// /* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00" // ................ |
// /* 00c0 */ "\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x01\xca\x01\x00\x00\x00\x00\x00" // ................ |
// /* 0110 */ "\x18\x00\x0f\x00\xaf\x07\x33\x00\x35\x00\x36\x00\x61\x00\x37\x00" // ......3.5.6.a.7. |
// /* 0120 */ "\x37\x00\x38\x00\x38\x00\x2d\x00\x38\x00\x36\x00\x33\x00\x37\x00" // 7.8.8.-.8.6.3.7. |
// /* 0130 */ "\x2d\x00\x34\x00\x31\x00\x63\x00\x64\x00\x2d\x00\x39\x00\x39\x00" // -.4.1.c.d.-.9.9. |
// /* 0140 */ "\x37\x00\x64\x00\x2d\x00\x34\x00\x37\x00\x35\x00\x64\x00\x38\x00" // 7.d.-.4.7.5.d.8. |
// /* 0150 */ "\x31\x00\x61\x00\x00\x00\x07\x00\x01\x00\x00\x00\x00\x00\x00\x00" // 1.a............. |
// /* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\xc0" // ................ |
// /* 0170 */ "\x0c\x00\x15\x00\x00\x00\x00\x00\x00\x00\x02\xc0\x0c\x00\x1b\x00" // ................ |
// /* 0180 */ "\x00\x00\x00\x00\x00\x00\x03\xc0\x38\x00\x04\x00\x00\x00\x72\x64" // ........8.....rd |
// /* 0190 */ "\x70\x64\x72\x00\x00\x00\x00\x00\x80\x80\x72\x64\x70\x73\x6e\x64" // pdr.......rdpsnd |
// /* 01a0 */ "\x00\x00\x00\x00\x00\xc0\x63\x6c\x69\x70\x72\x64\x72\x00\x00\x00" // ......cliprdr... |
// /* 01b0 */ "\xa0\xc0\x64\x72\x64\x79\x6e\x76\x63\x00\x00\x00\x80\xc0\x06\xc0" // ..drdynvc....... |
// /* 01c0 */ "\x08\x00\x00\x00\x00\x00\x0a\xc0\x08\x00\x05\x03\x00\x00"         // .............. |
// Dump done on RDP Client (4) 462 bytes |
// Front::incoming: Basic Settings Exchange |
// GCC::UserData tag=c001 length=234 |
// Front::incoming: Received from Client GCC User Data CS_CORE (234 bytes) |
// cs_core::length [00ea] |
// cs_core::version [8000c] Unknown client |
// cs_core::desktopWidth  = 1440 |
// cs_core::desktopHeight = 900 |
// cs_core::colorDepth    = [ca01] [RNS_UD_COLOR_8BPP] superseded by postBeta2ColorDepth |
// cs_core::SASSequence   = [aa03] [RNS_UD_SAS_DEL] |
// cs_core::keyboardLayout= 040c |
// cs_core::clientBuild   = 18362 |
// cs_core::clientName    = CLT02 |
// cs_core::keyboardType  = [0004] IBM enhanced (101-key or 102-key) keyboard |
// cs_core::keyboardSubType      = [0000] OEM code |
// cs_core::keyboardFunctionKey  = 12 function keys |
// cs_core::imeFileName    = |
// cs_core::postBeta2ColorDepth  = [ca01] [8 bpp] |
// cs_core::clientProductId = 1 |
// cs_core::serialNumber = 0 |
// cs_core::highColorDepth  = [0018] [24-bit RGB mask] |
// cs_core::supportedColorDepths  = [000f] [24/16/15/32] |
// cs_core::earlyCapabilityFlags  = [07af] |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_ERRINFO_PDU |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_WANT_32BPP_SESSION |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_STATUSINFO_PDU |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_STRONG_ASYMMETRIC_KEYS |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_VALID_CONNECTION_TYPE |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_NETCHAR_AUTODETECT |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_DYNVC_GFX_PROTOCOL |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_DYNAMIC_TIME_ZONE |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_HEARTBEAT_PDU |
// cs_core::clientDigProductId=[330035003600610037003700380038002e0038003600330037002e0034003100630064002e003a003a00370064002e0034003700350064003800310061000000] |
// cs_core::connectionType = <unknown(greater than 6)> |
// cs_core::pad1octet = 0 |
// cs_core::serverSelectedProtocol = 1 |
// cs_core::desktopPhysicalWidth = 0 |
// cs_core::desktopPhysicalHeight = 0 |
// cs_core::desktopOrientation = 0 |
// cs_core::desktopScaleFactor = 0 |
// cs_core::deviceScaleFactor = 0 |
// GCC::UserData tag=c004 length=12 |
// Front::incoming: Receiving from Client GCC User Data CS_CLUSTER (12 bytes) |
// cs_cluster::flags [0015] |
// cs_cluster::flags::REDIRECTION_SUPPORTED |
// cs_cluster::flags::redirectionVersion = 6 |
// GCC::UserData tag=c002 length=12 |
// Front::incoming: Received from Client GCC User Data CS_SECURITY (12 bytes) |
// CSSecGccUserData::encryptionMethods 27 |
// CSSecGccUserData::extEncryptionMethods 0 |
// GCC::UserData tag=c003 length=56 |
// Front::incoming: Received from Client GCC User Data CS_NET (56 bytes) |
// cs_net::channelCount   = 4 |
// cs_net::channel 'rdpdr' [1004] INITIALIZED COMPRESS_RDP |
// cs_net::channel 'rdpsnd' [1005] INITIALIZED |
// cs_net::channel 'cliprdr' [1006] INITIALIZED COMPRESS_RDP SHOW_PROTOCOL |
// cs_net::channel 'drdynvc' [1007] INITIALIZED COMPRESS_RDP |
// GCC::UserData tag=c006 length=8 |
// Front::incoming: Receiving from Client GCC User Data CS_MCS_MSGCHANNEL (8 bytes) |
// CSMCSMsgChannel::flags 0 |
// GCC::UserData tag=c00a length=8 |
// Front::incoming: Receiving from Client GCC User Data CS_MULTITRANSPORT (8 bytes) |
// CSMultiTransport::flags 773 |
// Front::incoming: Sending to client GCC User Data SC_CORE (12 bytes) |
// sc_core::version [80004] RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 servers) |
// sc_core::clientRequestedProtocols  = 11 |
// Front::incoming: Sending to client GCC User Data SC_NET (16 bytes) |
// sc_net::MCSChannelId   = 1003 |
// sc_net::channelCount   = 4 |
// sc_net::channel[1004]::id = 1004 |
// sc_net::channel[1005]::id = 1005 |
// sc_net::channel[1006]::id = 1006 |
// sc_net::channel[1007]::id = 1007 |
// Front::incoming: Sending to client GCC User Data SC_SECURITY (12 bytes) |
// sc_security::encryptionMethod = 0 |
// sc_security::encryptionLevel  = 0 |
// Sending on RDP Client (4) 109 bytes |
/* 0000 */ "\x03\x00\x00\x6d\x02\xf0\x80\x7f\x66\x63\x0a\x01\x00\x02\x01\x00" // ...m....fc...... |
/* 0010 */ "\x30\x1a\x02\x01\x22\x02\x01\x03\x02\x01\x00\x02\x01\x01\x02\x01" // 0..."........... |
/* 0020 */ "\x00\x02\x01\x01\x02\x03\x00\xff\xf8\x02\x01\x02\x04\x3f\x00\x05" // .............?.. |
/* 0030 */ "\x00\x14\x7c\x00\x01\x2a\x14\x76\x0a\x01\x01\x00\x01\xc0\x00\x4d" // ..|..*.v.......M |
/* 0040 */ "\x63\x44\x6e\x80\x28\x01\x0c\x0c\x00\x04\x00\x08\x00\x0b\x00\x00" // cDn.(........... |
/* 0050 */ "\x00\x03\x0c\x10\x00\xeb\x03\x04\x00\xec\x03\xed\x03\xee\x03\xef" // ................ |
/* 0060 */ "\x03\x02\x0c\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00"             // ............. |
// Sent dumped on RDP Client (4) 109 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 12 bytes |
// /* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x04\x01\x00\x01\x00"                 // ............ |
// Dump done on RDP Client (4) 12 bytes |
// Front::incoming: Channel Connection |
// Front::incoming: Recv MCS::ErectDomainRequest |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 8 bytes |
// /* 0000 */ "\x03\x00\x00\x08\x02\xf0\x80\x28"                                 // .......( |
// Dump done on RDP Client (4) 8 bytes |
// Front::incoming: Recv MCS::AttachUserRequest |
// Front::incoming: Send MCS::AttachUserConfirm userid=0 |
// Sending on RDP Client (4) 11 bytes |
/* 0000 */ "\x03\x00\x00\x0b\x02\xf0\x80\x2e\x00\x00\x00"                     // ........... |
// Sent dumped on RDP Client (4) 11 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 12 bytes |
// /* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xe9"                 // .......8.... |
// Dump done on RDP Client (4) 12 bytes |
// Sending on RDP Client (4) 15 bytes |
/* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xe9\x03\xe9"     // .......>....... |
// Sent dumped on RDP Client (4) 15 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 12 bytes |
// /* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xeb"                 // .......8.... |
// Dump done on RDP Client (4) 12 bytes |
// Sending on RDP Client (4) 15 bytes |
/* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xeb\x03\xeb"     // .......>....... |
// Sent dumped on RDP Client (4) 15 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 12 bytes |
// /* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xec"                 // .......8.... |
// Dump done on RDP Client (4) 12 bytes |
// Front::incoming: cjrq[0] = 1004 -> cjcf |
// Sending on RDP Client (4) 15 bytes |
/* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xec\x03\xec"     // .......>....... |
// Sent dumped on RDP Client (4) 15 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 12 bytes |
// /* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xed"                 // .......8.... |
// Dump done on RDP Client (4) 12 bytes |
// Front::incoming: cjrq[1] = 1005 -> cjcf |
// Sending on RDP Client (4) 15 bytes |
/* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xed\x03\xed"     // .......>....... |
// Sent dumped on RDP Client (4) 15 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 12 bytes |
// /* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xee"                 // .......8.... |
// Dump done on RDP Client (4) 12 bytes |
// Front::incoming: cjrq[2] = 1006 -> cjcf |
// Sending on RDP Client (4) 15 bytes |
/* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xee\x03\xee"     // .......>....... |
// Sent dumped on RDP Client (4) 15 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 12 bytes |
// /* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xef"                 // .......8.... |
// Dump done on RDP Client (4) 12 bytes |
// Front::incoming: cjrq[3] = 1007 -> cjcf |
// Sending on RDP Client (4) 15 bytes |
/* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xef\x03\xef"     // .......>....... |
// Sent dumped on RDP Client (4) 15 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 329 bytes |
// /* 0000 */ "\x03\x00\x01\x49\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\x3a\x40" // ...I...d....p.:@ |
// /* 0010 */ "\x00\xf3\xa8\x09\x04\x0c\x04\xb3\x47\x01\x00\x00\x00\x02\x00\x00" // ........G....... |
// /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x78\x00\x00\x00\x00\x00\x00\x00\x00" // .......x........ |
// /* 0030 */ "\x00\x02\x00\x18\x00\x31\x00\x30\x00\x2e\x00\x31\x00\x30\x00\x2e" // .....1.0...1.0.. |
// /* 0040 */ "\x00\x34\x00\x34\x00\x2e\x00\x39\x00\x39\x00\x00\x00\x40\x00\x43" // .4.4...9.9...@.C |
// /* 0050 */ "\x00\x3a\x00\x5c\x00\x57\x00\x49\x00\x4e\x00\x44\x00\x4f\x00\x57" // .:...W.I.N.D.O.W |
// /* 0060 */ "\x00\x53\x00\x5c\x00\x73\x00\x79\x00\x73\x00\x74\x00\x65\x00\x6d" // .S...s.y.s.t.e.m |
// /* 0070 */ "\x00\x33\x00\x32\x00\x5c\x00\x6d\x00\x73\x00\x74\x00\x73\x00\x63" // .3.2...m.s.t.s.c |
// /* 0080 */ "\x00\x61\x00\x78\x00\x2e\x00\x64\x00\x6c\x00\x6c\x00\x00\x00\xc4" // .a.x...d.l.l.... |
// /* 0090 */ "\xff\xff\xff\x52\x00\x6f\x00\x6d\x00\x61\x00\x6e\x00\x63\x00\x65" // ...R.o.m.a.n.c.e |
// /* 00a0 */ "\x00\x20\x00\x53\x00\x74\x00\x61\x00\x6e\x00\x64\x00\x61\x00\x72" // . .S.t.a.n.d.a.r |
// /* 00b0 */ "\x00\x64\x00\x20\x00\x54\x00\x69\x00\x6d\x00\x65\x00\x00\x00\x00" // .d. .T.i.m.e.... |
// /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 00d0 */ "\x00\x00\x00\x00\x00\x0a\x00\x00\x00\x05\x00\x03\x00\x00\x00\x00" // ................ |
// /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x52\x00\x6f\x00\x6d\x00\x61\x00\x6e" // .......R.o.m.a.n |
// /* 00f0 */ "\x00\x63\x00\x65\x00\x20\x00\x44\x00\x61\x00\x79\x00\x6c\x00\x69" // .c.e. .D.a.y.l.i |
// /* 0100 */ "\x00\x67\x00\x68\x00\x74\x00\x20\x00\x54\x00\x69\x00\x6d\x00\x65" // .g.h.t. .T.i.m.e |
// /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x05\x00\x02" // ................ |
// /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\xc4\xff\xff\xff\x03\x00\x00\x00\x06" // ................ |
// /* 0140 */ "\x00\x00\x00\x00\x00\x64\x00\x00\x00"                             // .....d... |
// Dump done on RDP Client (4) 329 bytes |
// Front::incoming: RDP Security Commencement |
// Front::incoming: TLS mode: exchange packet disabled |
// Front::incoming: Secure Settings Exchange |
// RDP-5 Style logon |
// Receiving from client InfoPacket |
// InfoPacket::CodePage 67896329 |
// InfoPacket::flags 0x147b3 |
// InfoPacket::flags:INFO_MOUSE yes |
// InfoPacket::flags:INFO_DISABLECTRLALTDEL yes |
// InfoPacket::flags:INFO_AUTOLOGON no |
// InfoPacket::flags:INFO_UNICODE yes |
// InfoPacket::flags:INFO_MAXIMIZESHELL  yes |
// InfoPacket::flags:INFO_LOGONNOTIFY no |
// InfoPacket::flags:INFO_COMPRESSION yes |
// InfoPacket::flags:CompressionTypeMask yes |
// InfoPacket::flags:INFO_ENABLEWINDOWSKEY  yes |
// InfoPacket::flags:INFO_REMOTECONSOLEAUDIO no |
// InfoPacket::flags:INFO_FORCE_ENCRYPTED_CS_PDU yes |
// InfoPacket::flags:INFO_RAIL no |
// InfoPacket::flags:INFO_LOGONERRORS yes |
// InfoPacket::flags:INFO_MOUSE_HAS_WHEEL no |
// InfoPacket::flags:INFO_PASSWORD_IS_SC_PIN no |
// InfoPacket::flags:INFO_NOAUDIOPLAYBACK no |
// InfoPacket::flags:INFO_USING_SAVED_CREDS no |
// InfoPacket::flags:INFO_AUDIOCAPTURE no |
// InfoPacket::flags:INFO_VIDEO_DISABLE no |
// InfoPacket::flags:INFO_HIDEF_RAIL_SUPPORTED no |
// InfoPacket::cbDomain 2 |
// InfoPacket::cbUserName 4 |
// InfoPacket::cbPassword 2 |
// InfoPacket::cbAlternateShell 2 |
// InfoPacket::cbWorkingDir 2 |
// InfoPacket::Domain |
// InfoPacket::UserName x |
// InfoPacket::Password <hidden> |
// InfoPacket::AlternateShell |
// InfoPacket::WorkingDir |
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2 |
// InfoPacket::ExtendedInfoPacket::cbClientAddress 24 |
// InfoPacket::ExtendedInfoPacket::clientAddress 10.10.44.99 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 64 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\WINDOWS\system32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 3 |
// InfoPacket::ExtendedInfoPacket::performanceFlags 0x6 |
// InfoPacket::ExtendedInfoPacket::PERF_DISABLE_FULLWINDOWDRAG |
// InfoPacket::ExtendedInfoPacket::PERF_DISABLE_MENUANIMATIONS |
// InfoPacket::ExtendedInfoPacket::cbAutoReconnectLen 0 |
// InfoPacket::ExtendedInfoPacket::reserved1 100 |
// InfoPacket::ExtendedInfoPacket::reserved2 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::Bias 4294967236 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardName Romance Standard Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMonth 10 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDay 5 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wHour 3 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardBias 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightName Romance Daylight Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMonth 3 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDay 5 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wHour 2 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightBias 4294967236 |
// client info: performance flags before=0x00000006 after=0x0000002E default=0x00000080 present=0x00000028 not-present=0x00000000 |
// Front::incoming: Keyboard Layout = 0x40c |
// Front::incoming: licencing not client_info.is_mce |
// Front::incoming: licencing send_lic_initial |
// Sending on RDP Client (4) 337 bytes |
/* 0000 */ "\x03\x00\x01\x51\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x81\x42\x80" // ...Q...h....p.B. |
/* 0010 */ "\x00\x00\x00\x01\x02\x3e\x01\x7b\x3c\x31\xa6\xae\xe8\x74\xf6\xb4" // .....>.{<1...t.. |
/* 0020 */ "\xa5\x03\x90\xe7\xc2\xc7\x39\xba\x53\x1c\x30\x54\x6e\x90\x05\xd0" // ......9.S.0Tn... |
/* 0030 */ "\x05\xce\x44\x18\x91\x83\x81\x00\x00\x04\x00\x2c\x00\x00\x00\x4d" // ..D........,...M |
/* 0040 */ "\x00\x69\x00\x63\x00\x72\x00\x6f\x00\x73\x00\x6f\x00\x66\x00\x74" // .i.c.r.o.s.o.f.t |
/* 0050 */ "\x00\x20\x00\x43\x00\x6f\x00\x72\x00\x70\x00\x6f\x00\x72\x00\x61" // . .C.o.r.p.o.r.a |
/* 0060 */ "\x00\x74\x00\x69\x00\x6f\x00\x6e\x00\x00\x00\x08\x00\x00\x00\x32" // .t.i.o.n.......2 |
/* 0070 */ "\x00\x33\x00\x36\x00\x00\x00\x0d\x00\x04\x00\x01\x00\x00\x00\x03" // .3.6............ |
/* 0080 */ "\x00\xb8\x00\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x06" // ................ |
/* 0090 */ "\x00\x5c\x00\x52\x53\x41\x31\x48\x00\x00\x00\x00\x02\x00\x00\x3f" // ...RSA1H.......? |
/* 00a0 */ "\x00\x00\x00\x01\x00\x01\x00\x01\xc7\xc9\xf7\x8e\x5a\x38\xe4\x29" // ............Z8.) |
/* 00b0 */ "\xc3\x00\x95\x2d\xdd\x4c\x3e\x50\x45\x0b\x0d\x9e\x2a\x5d\x18\x63" // ...-.L>PE...*].c |
/* 00c0 */ "\x64\xc4\x2c\xf7\x8f\x29\xd5\x3f\xc5\x35\x22\x34\xff\xad\x3a\xe6" // d.,..).?.5"4..:. |
/* 00d0 */ "\xe3\x95\x06\xae\x55\x82\xe3\xc8\xc7\xb4\xa8\x47\xc8\x50\x71\x74" // ....U......G.Pqt |
/* 00e0 */ "\x29\x53\x89\x6d\x9c\xed\x70\x00\x00\x00\x00\x00\x00\x00\x00\x08" // )S.m..p......... |
/* 00f0 */ "\x00\x48\x00\xa8\xf4\x31\xb9\xab\x4b\xe6\xb4\xf4\x39\x89\xd6\xb1" // .H...1..K...9... |
/* 0100 */ "\xda\xf6\x1e\xec\xb1\xf0\x54\x3b\x5e\x3e\x6a\x71\xb4\xf7\x75\xc8" // ......T;^>jq..u. |
/* 0110 */ "\x16\x2f\x24\x00\xde\xe9\x82\x99\x5f\x33\x0b\xa9\xa6\x94\xaf\xcb" // ./$....._3...... |
/* 0120 */ "\x11\xc3\xf2\xdb\x09\x42\x68\x29\x56\x58\x01\x56\xdb\x59\x03\x69" // .....Bh)VX.V.Y.i |
/* 0130 */ "\xdb\x7d\x37\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x0e" // .}7............. |
/* 0140 */ "\x00\x0e\x00\x6d\x69\x63\x72\x6f\x73\x6f\x66\x74\x2e\x63\x6f\x6d" // ...microsoft.com |
/* 0150 */ "\x00"                                                             // . |
// Sent dumped on RDP Client (4) 337 bytes |
// Front::incoming: Waiting for answer to lic_initial |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 164 bytes |
// /* 0000 */ "\x03\x00\x00\xa4\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x95\x80" // .......d....p... |
// /* 0010 */ "\x00\x40\x02\x13\x83\x91\x00\x01\x00\x00\x00\x00\x00\x01\x08\xd8" // .@.............. |
// /* 0020 */ "\xf0\xee\x70\xaf\x7d\xbc\xcf\x68\x5e\xab\xfb\x4a\x67\x5e\x19\xb1" // ..p.}..h^..Jg^.. |
// /* 0030 */ "\x16\x52\xda\x63\x3e\xec\xbf\x0f\xd1\x03\xf1\xe5\xd6\x95\xb9\x00" // .R.c>........... |
// /* 0040 */ "\x00\x48\x00\x60\x03\x89\x08\xe9\x09\x29\xdd\x7e\x57\x18\xa5\x54" // .H.`.....).~W..T |
// /* 0050 */ "\x09\x49\x87\x81\x1c\xfe\x2a\x9b\x00\xdf\x85\xaa\xc5\xa0\x22\x1f" // .I....*.......". |
// /* 0060 */ "\xd5\x3b\x64\x9d\x2d\xe7\x0b\x3a\x6a\x56\x1a\x41\x44\xdb\x05\xaa" // .;d.-..:jV.AD... |
// /* 0070 */ "\xba\xd5\x24\xf2\x1d\x95\x0b\xe4\x01\x54\x4f\x65\x15\xf5\xb0\x3a" // ..$......TOe...: |
// /* 0080 */ "\xa9\x8a\x51\x00\x00\x00\x00\x00\x00\x00\x00\x0f\x00\x0b\x00\x52" // ..Q............R |
// /* 0090 */ "\x65\x64\x65\x6d\x70\x74\x69\x6f\x6e\x00\x10\x00\x06\x00\x43\x4c" // edemption.....CL |
// /* 00a0 */ "\x54\x30\x32\x00"                                                 // T02. |
// Dump done on RDP Client (4) 164 bytes |
// Front::incoming: WAITING_FOR_ANSWER_TO_LICENCE |
// Front::incoming: LIC::NEW_LICENSE_REQUEST |
// Sending on RDP Client (4) 34 bytes |
/* 0000 */ "\x03\x00\x00\x22\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x14\x80\x00" // ..."...h....p... |
/* 0010 */ "\x10\x00\xff\x02\x10\x00\x07\x00\x00\x00\x02\x00\x00\x00\x28\x14" // ..............(. |
/* 0020 */ "\x00\x00"                                                         // .. |
// Sent dumped on RDP Client (4) 34 bytes |
// Front::incoming: send_demand_active |
// Front::send_demand_active |
// Front::send_demand_active: Sending to client General caps (24 bytes) |
//      General caps::major 1 |
//      General caps::minor 3 |
//      General caps::protocol 512 |
//      General caps::pad2octetA 0 |
//      General caps::compression type 0 |
//      General caps::extra flags 1 |
//      General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED yes |
//      General caps::extraflags:LONG_CREDENTIALS_SUPPORTED no |
//      General caps::extraflags:AUTORECONNECT_SUPPORTED no |
//      General caps::extraflags:ENC_SALTED_CHECKSUM no |
//      General caps::extraflags:NO_BITMAP_COMPRESSION_HDR no |
//      General caps::updateCapability 0 |
//      General caps::remoteUnshare 0 |
//      General caps::compressionLevel 0 |
//      General caps::refreshRectSupport 0 |
//      General caps::suppressOutputSupport 0 |
// Front::send_demand_active: Sending to client Bitmap caps (28 bytes) |
//      Bitmap caps::preferredBitsPerPixel 24 |
//      Bitmap caps::receive1BitPerPixel 1 |
//      Bitmap caps::receive4BitsPerPixel 1 |
//      Bitmap caps::receive8BitsPerPixel 1 |
//      Bitmap caps::desktopWidth 1440 |
//      Bitmap caps::desktopHeight 900 |
//      Bitmap caps::pad2octets 0 |
//      Bitmap caps::desktopResizeFlag 1 (yes) |
//      Bitmap caps::bitmapCompressionFlag 1 yes |
//      Bitmap caps::highColorFlags 0 |
//      Bitmap caps::drawingFlags 8 |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY no |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_COLOR_SUBSAMPLING no |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_SKIP_ALPHA yes |
//      Bitmap caps::multipleRectangleSupport 1 |
//      Bitmap caps::pad2octetsB 0 |
// Front::send_demand_active: Sending to client Font caps (8 bytes) |
//      Font caps::fontSupportFlags 1 |
//      Font caps::pad2octets 0 |
// Front::send_demand_active: Sending to client Order caps (88 bytes) |
//      Order caps::terminalDescriptor 0 |
//      Order caps::pad4octetsA 1078071040 |
//      Order caps::desktopSaveXGranularity 1 |
//      Order caps::desktopSaveYGranularity 20 |
//      Order caps::pad2octetsA 0 |
//      Order caps::maximumOrderLevel 1 |
//      Order caps::numberFonts 47 |
//      Order caps::orderFlags 0x22 |
//      Order caps::orderSupport[DSTBLT] 1 |
//      Order caps::orderSupport[PATBLT] 1 |
//      Order caps::orderSupport[SCRBLT] 1 |
//      Order caps::orderSupport[MEMBLT] 1 |
//      Order caps::orderSupport[MEM3BLT] 0 |
//      Order caps::orderSupport[ATEXTOUT] 0 |
//      Order caps::orderSupport[AEXTTEXTOUT] 0 |
//      Order caps::orderSupport[DRAWNINEGRID] 0 |
//      Order caps::orderSupport[LINETO] 1 |
//      Order caps::orderSupport[MULTI_DRAWNINEGRID] 1 |
//      Order caps::orderSupport[OPAQUERECT] 1 |
//      Order caps::orderSupport[SAVEBITMAP] 0 |
//      Order caps::orderSupport[WTEXTOUT] 0 |
//      Order caps::orderSupport[MEMBLT_V2] 0 |
//      Order caps::orderSupport[MEM3BLT_V2] 0 |
//      Order caps::orderSupport[MULTIDSTBLT] 0 |
//      Order caps::orderSupport[MULTIPATBLT] 0 |
//      Order caps::orderSupport[MULTISCRBLT] 0 |
//      Order caps::orderSupport[MULTIOPAQUERECT] 0 |
//      Order caps::orderSupport[FAST_INDEX] 0 |
//      Order caps::orderSupport[POLYGON_SC] 0 |
//      Order caps::orderSupport[POLYGON_CB] 0 |
//      Order caps::orderSupport[POLYLINE] 1 |
//      Order caps::orderSupport[UnusedIndex7] 0 |
//      Order caps::orderSupport[FAST_GLYPH] 0 |
//      Order caps::orderSupport[ELLIPSE_SC] 1 |
//      Order caps::orderSupport[ELLIPSE_CB] 0 |
//      Order caps::orderSupport[GLYPH] 1 |
//      Order caps::orderSupport[GLYPH_WEXTTEXTOUT] 0 |
//      Order caps::orderSupport[GLYPH_WLONGTEXTOUT] 0 |
//      Order caps::orderSupport[GLYPH_WLONGEXTTEXTOUT] 0 |
//      Order caps::orderSupport[UnusedIndex11] 0 |
//      Order caps::textFlags 1697 |
//      Order caps::orderSupportExFlags 0x0 |
//      Order caps::pad4octetsB 1000000 |
//      Order caps::desktopSaveSize 1000000 |
//      Order caps::pad2octetsC 1 |
//      Order caps::pad2octetsD 0 |
//      Order caps::textANSICodePage 0 |
//      Order caps::pad2octetsE 0 |
// Front::send_demand_active: Sending to client Pointer caps (10 bytes) |
//      Pointer caps::colorPointerFlag 1 |
//      Pointer caps::colorPointerCacheSize 25 |
//      Pointer caps::pointerCacheSize 25 |
// Front::send_demand_active: Sending to client Input caps (88 bytes) |
//      Input caps::inputFlags 0x29 |
//      Input caps::pad2octetsA 0 |
//      Input caps::keyboardLayout 0 |
//      Input caps::keyboardType 0 |
//      Input caps::keyboardSubType 0 |
//      Input caps::keyboardFunctionKey 0 |
//      Input caps::imeFileName |
// Front::send_demand_active: Sending to client VirtualChannel caps (12 bytes) |
//      VirtualChannel caps::flags 0 |
//      VirtualChannel caps::VCChunkSize 1600 |
// Front::send_demand_active: Sending to client Share caps (8 bytes) |
//      Share caps::nodeId 1001 |
//      Share caps::pad2octets 46562 |
// Front::send_demand_active: Sending to client ColorCache caps (8 bytes) |
//      ColorCache caps::colorTableCacheSize 6 |
//      ColorCache caps::pad2octets 0 |
// Sending on RDP Client (4) 315 bytes |
/* 0000 */ "\x03\x00\x01\x3b\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x81\x2c\x2c" // ...;...h....p.,, |
/* 0010 */ "\x01\x11\x00\xe9\x03\x02\x00\x01\x00\x04\x00\x16\x01\x52\x44\x50" // .............RDP |
/* 0020 */ "\x00\x09\x00\x00\x00\x01\x00\x18\x00\x01\x00\x03\x00\x00\x02\x00" // ................ |
/* 0030 */ "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x1c" // ................ |
/* 0040 */ "\x00\x18\x00\x01\x00\x01\x00\x01\x00\xa0\x05\x84\x03\x00\x00\x01" // ................ |
/* 0050 */ "\x00\x01\x00\x00\x08\x01\x00\x00\x00\x0e\x00\x08\x00\x01\x00\x00" // ................ |
/* 0060 */ "\x00\x03\x00\x58\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ...X............ |
/* 0070 */ "\x00\x00\x00\x00\x00\x40\x42\x0f\x00\x01\x00\x14\x00\x00\x00\x01" // .....@B......... |
/* 0080 */ "\x00\x2f\x00\x22\x00\x01\x01\x01\x01\x00\x00\x00\x00\x01\x00\x01" // ./."............ |
/* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x01\x00" // ................ |
/* 00a0 */ "\x01\x00\x00\x00\x00\xa1\x06\x00\x00\x40\x42\x0f\x00\x40\x42\x0f" // .........@B..@B. |
/* 00b0 */ "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x08\x00\x0a\x00\x01\x00\x19" // ................ |
/* 00c0 */ "\x00\x19\x00\x0d\x00\x58\x00\x29\x00\x00\x00\x00\x00\x00\x00\x00" // .....X.)........ |
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x14\x00\x0c\x00\x00" // ................ |
/* 0120 */ "\x00\x00\x00\x40\x06\x00\x00\x09\x00\x08\x00\xe9\x03\xe2\xb5\x0a" // ...@............ |
/* 0130 */ "\x00\x08\x00\x06\x00\x00\x00\x00\x00\x00\x00"                     // ........... |
// Sent dumped on RDP Client (4) 315 bytes |
// Front::send_demand_active: done |
// Front::incoming: ACTIVATED (new license request) |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 479 bytes |
// /* 0000 */ "\x03\x00\x01\xdf\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\xd0\xd0" // .......d....p... |
// /* 0010 */ "\x01\x13\x00\xe9\x03\x02\x00\x01\x00\xe9\x03\x06\x00\xba\x01\x4d" // ...............M |
// /* 0020 */ "\x53\x54\x53\x43\x00\x12\x00\x00\x00\x01\x00\x18\x00\x01\x00\x03" // STSC............ |
// /* 0030 */ "\x00\x00\x02\x00\x00\x00\x00\x0d\x04\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0040 */ "\x00\x02\x00\x1c\x00\x20\x00\x01\x00\x01\x00\x01\x00\xa0\x05\x84" // ..... .......... |
// /* 0050 */ "\x03\x00\x00\x01\x00\x01\x00\x00\x1a\x01\x00\x00\x00\x03\x00\x58" // ...............X |
// /* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0070 */ "\x00\x00\x00\x00\x00\x01\x00\x14\x00\x00\x00\x01\x00\x00\x00\xaa" // ................ |
// /* 0080 */ "\x00\x01\x01\x01\x01\x01\x00\x00\x01\x01\x01\x00\x01\x00\x00\x00" // ................ |
// /* 0090 */ "\x01\x01\x01\x01\x01\x01\x01\x01\x00\x01\x01\x01\x00\x00\x00\x00" // ................ |
// /* 00a0 */ "\x00\xa1\x06\x06\x00\x00\x00\x00\x00\x00\x84\x03\x00\x00\x00\x00" // ................ |
// /* 00b0 */ "\x00\xe4\x04\x00\x00\x04\x00\x28\x00\x00\x00\x00\x00\x00\x00\x00" // .......(........ |
// /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 00d0 */ "\x00\x78\x00\x00\x03\x78\x00\x00\x0c\x51\x01\x00\x30\x0a\x00\x08" // .x...x...Q..0... |
// /* 00e0 */ "\x00\x06\x00\x00\x00\x07\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 00f0 */ "\x00\x05\x00\x0c\x00\x00\x00\x00\x00\x02\x00\x02\x00\x08\x00\x0a" // ................ |
// /* 0100 */ "\x00\x01\x00\x14\x00\x15\x00\x09\x00\x08\x00\x00\x00\x00\x00\x0d" // ................ |
// /* 0110 */ "\x00\x58\x00\x91\x00\x20\x00\x0c\x04\x00\x00\x04\x00\x00\x00\x00" // .X... .......... |
// /* 0120 */ "\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x08\x00\x01\x00\x00\x00\x0e" // ................ |
// /* 0170 */ "\x00\x08\x00\x01\x00\x00\x00\x10\x00\x34\x00\xfe\x00\x04\x00\xfe" // .........4...... |
// /* 0180 */ "\x00\x04\x00\xfe\x00\x08\x00\xfe\x00\x08\x00\xfe\x00\x10\x00\xfe" // ................ |
// /* 0190 */ "\x00\x20\x00\xfe\x00\x40\x00\xfe\x00\x80\x00\xfe\x00\x00\x01\x40" // . ...@.........@ |
// /* 01a0 */ "\x00\x00\x08\x00\x01\x00\x01\x03\x00\x00\x00\x0f\x00\x08\x00\x01" // ................ |
// /* 01b0 */ "\x00\x00\x00\x11\x00\x0c\x00\x01\x00\x00\x00\x00\x28\x64\x00\x14" // ............(d.. |
// /* 01c0 */ "\x00\x0c\x00\x01\x00\x00\x00\x00\x00\x00\x00\x15\x00\x0c\x00\x02" // ................ |
// /* 01d0 */ "\x00\x00\x00\x00\x0a\x00\x01\x1a\x00\x08\x00\x00\x00\x00\x00"     // ............... |
// Dump done on RDP Client (4) 479 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=0 |
// Front::incoming: Received CONFIRMACTIVEPDU |
// Front::process_confirm_active |
// Front::process_confirm_active: lengthSourceDescriptor = 6 |
// Front::process_confirm_active: lengthCombinedCapabilities = 442 |
// Front::capability 0 / 18 |
// Front::process_confirm_active: Receiving from client General caps (24 bytes) |
//      General caps::major 1 |
//      General caps::minor 3 |
//      General caps::protocol 512 |
//      General caps::pad2octetA 0 |
//      General caps::compression type 0 |
//      General caps::extra flags 40d |
//      General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED yes |
//      General caps::extraflags:LONG_CREDENTIALS_SUPPORTED yes |
//      General caps::extraflags:AUTORECONNECT_SUPPORTED yes |
//      General caps::extraflags:ENC_SALTED_CHECKSUM no |
//      General caps::extraflags:NO_BITMAP_COMPRESSION_HDR yes |
//      General caps::updateCapability 0 |
//      General caps::remoteUnshare 0 |
//      General caps::compressionLevel 0 |
//      General caps::refreshRectSupport 0 |
//      General caps::suppressOutputSupport 0 |
// Front::capability 1 / 18 |
// Front::process_confirm_active: Receiving from client Bitmap caps (28 bytes) |
//      Bitmap caps::preferredBitsPerPixel 32 |
//      Bitmap caps::receive1BitPerPixel 1 |
//      Bitmap caps::receive4BitsPerPixel 1 |
//      Bitmap caps::receive8BitsPerPixel 1 |
//      Bitmap caps::desktopWidth 1440 |
//      Bitmap caps::desktopHeight 900 |
//      Bitmap caps::pad2octets 0 |
//      Bitmap caps::desktopResizeFlag 1 (yes) |
//      Bitmap caps::bitmapCompressionFlag 1 yes |
//      Bitmap caps::highColorFlags 0 |
//      Bitmap caps::drawingFlags 26 |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY yes |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_COLOR_SUBSAMPLING no |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_SKIP_ALPHA yes |
//      Bitmap caps::multipleRectangleSupport 1 |
//      Bitmap caps::pad2octetsB 0 |
// Front::capability 2 / 18 |
// Front::process_confirm_active: Receiving from client Order caps (88 bytes) |
//      Order caps::terminalDescriptor 0 |
//      Order caps::pad4octetsA 0 |
//      Order caps::desktopSaveXGranularity 1 |
//      Order caps::desktopSaveYGranularity 20 |
//      Order caps::pad2octetsA 0 |
//      Order caps::maximumOrderLevel 1 |
//      Order caps::numberFonts 0 |
//      Order caps::orderFlags 0xAA |
//      Order caps::orderSupport[DSTBLT] 1 |
//      Order caps::orderSupport[PATBLT] 1 |
//      Order caps::orderSupport[SCRBLT] 1 |
//      Order caps::orderSupport[MEMBLT] 1 |
//      Order caps::orderSupport[MEM3BLT] 1 |
//      Order caps::orderSupport[ATEXTOUT] 0 |
//      Order caps::orderSupport[AEXTTEXTOUT] 0 |
//      Order caps::orderSupport[DRAWNINEGRID] 1 |
//      Order caps::orderSupport[LINETO] 1 |
//      Order caps::orderSupport[MULTI_DRAWNINEGRID] 1 |
//      Order caps::orderSupport[OPAQUERECT] 0 |
//      Order caps::orderSupport[SAVEBITMAP] 1 |
//      Order caps::orderSupport[WTEXTOUT] 0 |
//      Order caps::orderSupport[MEMBLT_V2] 0 |
//      Order caps::orderSupport[MEM3BLT_V2] 0 |
//      Order caps::orderSupport[MULTIDSTBLT] 1 |
//      Order caps::orderSupport[MULTIPATBLT] 1 |
//      Order caps::orderSupport[MULTISCRBLT] 1 |
//      Order caps::orderSupport[MULTIOPAQUERECT] 1 |
//      Order caps::orderSupport[FAST_INDEX] 1 |
//      Order caps::orderSupport[POLYGON_SC] 1 |
//      Order caps::orderSupport[POLYGON_CB] 1 |
//      Order caps::orderSupport[POLYLINE] 1 |
//      Order caps::orderSupport[UnusedIndex7] 0 |
//      Order caps::orderSupport[FAST_GLYPH] 1 |
//      Order caps::orderSupport[ELLIPSE_SC] 1 |
//      Order caps::orderSupport[ELLIPSE_CB] 1 |
//      Order caps::orderSupport[GLYPH] 0 |
//      Order caps::orderSupport[GLYPH_WEXTTEXTOUT] 0 |
//      Order caps::orderSupport[GLYPH_WLONGTEXTOUT] 0 |
//      Order caps::orderSupport[GLYPH_WLONGEXTTEXTOUT] 0 |
//      Order caps::orderSupport[UnusedIndex11] 0 |
//      Order caps::textFlags 1697 |
//      Order caps::orderSupportExFlags 0x6 |
//      Order caps::pad4octetsB 0 |
//      Order caps::desktopSaveSize 230400 |
//      Order caps::pad2octetsC 0 |
//      Order caps::pad2octetsD 0 |
//      Order caps::textANSICodePage 1252 |
//      Order caps::pad2octetsE 0 |
// Front::capability 3 / 18 |
// Front::process_confirm_active: Receiving from client BitmapCache caps (40 bytes) |
//      BitmapCache caps::pad1 0 |
//      BitmapCache caps::pad2 0 |
//      BitmapCache caps::pad3 0 |
//      BitmapCache caps::pad4 0 |
//      BitmapCache caps::pad5 0 |
//      BitmapCache caps::pad6 0 |
//      BitmapCache caps::cache0Entries 120 |
//      BitmapCache caps::cache0MaximumCellSize 768 |
//      BitmapCache caps::cache1Entries 120 |
//      BitmapCache caps::cache1MaximumCellSize 3072 |
//      BitmapCache caps::cache2Entries 337 |
//      BitmapCache caps::cache2MaximumCellSize 12288 |
// Front::capability 4 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_COLORCACHE |
// Front::capability 5 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_ACTIVATION |
// Front::capability 6 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_CONTROL |
// Front::capability 7 / 18 |
// Front::process_confirm_active: Receiving from client Pointer caps (10 bytes) |
//      Pointer caps::colorPointerFlag 1 |
//      Pointer caps::colorPointerCacheSize 20 |
//      Pointer caps::pointerCacheSize 21 |
// Front::capability 8 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_SHARE |
// Front::capability 9 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_INPUT |
// Front::capability 10 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_SOUND |
// Front::capability 11 / 18 |
// Front::capability 12 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_GLYPHCACHE |
// Front::process_confirm_active: Receiving from client GlyphCache caps (52 bytes) |
//      GlyphCache caps::GlyphCache[0].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[0].CacheMaximumCellSize=4 |
//      GlyphCache caps::GlyphCache[1].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[1].CacheMaximumCellSize=4 |
//      GlyphCache caps::GlyphCache[2].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[2].CacheMaximumCellSize=8 |
//      GlyphCache caps::GlyphCache[3].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[3].CacheMaximumCellSize=8 |
//      GlyphCache caps::GlyphCache[4].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[4].CacheMaximumCellSize=16 |
//      GlyphCache caps::GlyphCache[5].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[5].CacheMaximumCellSize=32 |
//      GlyphCache caps::GlyphCache[6].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[6].CacheMaximumCellSize=64 |
//      GlyphCache caps::GlyphCache[7].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[7].CacheMaximumCellSize=128 |
//      GlyphCache caps::GlyphCache[8].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[8].CacheMaximumCellSize=256 |
//      GlyphCache caps::GlyphCache[9].CacheEntries=64 |
//      GlyphCache caps::GlyphCache[9].CacheMaximumCellSize=2048 |
//      GlyphCache caps::FragCache 16777472 |
//      GlyphCache caps::GlyphSupportLevel 3 |
//      GlyphCache caps::pad2octets 0 |
// Front::capability 13 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_BRUSH |
// Front::process_confirm_active: Receiving from client BrushCache caps (8 bytes) |
//      BrushCacheCaps caps::brushSupportLevel 1 |
// Front::capability 14 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_OFFSCREENCACHE |
// Front::process_confirm_active: Receiving from client OffScreenCache caps (12 bytes) |
//      OffScreenCache caps::offscreenSupportLevel 1 |
//      OffScreenCache caps::offscreenCacheSize 10240 |
//      OffScreenCache caps::offscreenCacheEntries 100 |
// Front::capability 15 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_VIRTUALCHANNEL |
// Front::capability 16 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_DRAWNINEGRIDCACHE |
// Front::capability 17 / 18 |
// Front::process_confirm_active: Receiving from client MultifragmentUpdate caps (8 bytes) |
//      MultifragmentUpdate caps::MaxRequestSize 0 |
// Front::process_confirm_active: done p=0x7f95c8eeec85 end=0x7f95c8eeec85 |
// Front::reset: use_bitmap_comp=1 |
// Front::reset: use_compact_packets=1 |
// Front::reset: bitmap_cache_version=0 |
// Front::reset: Use RDP 5.0 Bulk compression |
// Front::incoming: Received CONFIRMACTIVEPDU done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 36 bytes |
// /* 0000 */ "\x03\x00\x00\x24\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x16\x16\x00" // ...$...d....p... |
// /* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x01\x08\x00\x1f\x00\x00\x00" // ................ |
// /* 0020 */ "\x01\x00\xe9\x03"                                                 // .... |
// Dump done on RDP Client (4) 36 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=0 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=31 sdata_in.len=8 sdata_in.compressedLen=0 remains=0 payload_len=4 |
// Front::process_data: PDUTYPE2_SYNCHRONIZE |
// Front::process_data: PDUTYPE2_SYNCHRONIZE messageType=1 controlId=1001 |
// Front::send_synchronize |
// Sec clear payload to send: |
// /* 0000 */ 0x16, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x16, 0x00, 0x1f, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x01, 0x00, 0xea, 0x03,                                                              // ...... |
// Sending on RDP Client (4) 36 bytes |
/* 0000 */ "\x03\x00\x00\x24\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x16\x16\x00" // ...$...h....p... |
/* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x16\x00\x1f\x00\x00\x00" // ................ |
/* 0020 */ "\x01\x00\xea\x03"                                                 // .... |
// Sent dumped on RDP Client (4) 36 bytes |
// Front::send_synchronize: done |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 40 bytes |
// /* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00" // ...(...d....p... |
// /* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x01\x0c\x00\x14\x00\x00\x00" // ................ |
// /* 0020 */ "\x04\x00\x00\x00\x00\x00\x00\x00"                                 // ........ |
// Dump done on RDP Client (4) 40 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=0 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=20 sdata_in.len=12 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// Front::process_data: PDUTYPE2_CONTROL |
// Front::send_control: action=4 |
// Sec clear payload to send: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x1a, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00,                                      // .......... |
// Sending on RDP Client (4) 40 bytes |
/* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x1a\x1a\x00" // ...(...h....p... |
/* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00" // ................ |
/* 0020 */ "\x04\x00\x00\x00\xea\x03\x00\x00"                                 // ........ |
// Sent dumped on RDP Client (4) 40 bytes |
// Front::send_control: done. action=4 |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 40 bytes |
// /* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00" // ...(...d....p... |
// /* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x01\x0c\x00\x14\x00\x00\x00" // ................ |
// /* 0020 */ "\x01\x00\x00\x00\x00\x00\x00\x00"                                 // ........ |
// Dump done on RDP Client (4) 40 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=0 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=20 sdata_in.len=12 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// Front::process_data: PDUTYPE2_CONTROL |
// Front::send_control: action=2 |
// Sec clear payload to send: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x1a, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00,                                      // .......... |
// Sending on RDP Client (4) 40 bytes |
/* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x1a\x1a\x00" // ...(...h....p... |
/* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00" // ................ |
/* 0020 */ "\x02\x00\x00\x00\xea\x03\x00\x00"                                 // ........ |
// Sent dumped on RDP Client (4) 40 bytes |
// Front::send_control: done. action=2 |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 30 bytes |
// /* 0000 */ "\x30\x1e\x01\x0f\x62\x01\x2a\x01\x36\x01\x1d\x03\x1d\x01\x0f\x01" // 0...b.*.6....... |
// /* 0010 */ "\x38\x01\x0f\x03\x38\x01\x0f\x20\x00\x08\xc4\x03\xde\x01"         // 8...8.. ...... |
// Dump done on RDP Client (4) 30 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0xF |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, sync eventFlags=0x2 |
// Front::incoming: (Fast-Path) Synchronize Event toggleFlags=0x2 |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0x2A |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0x36 |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0x1D |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x3 SPKeyboardFlags=0x8100, keyCode=0x1D |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0xF |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0x38 |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0xF |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x3 SPKeyboardFlags=0x8100, keyCode=0x38 |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0xF |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, mouse pointerFlags=0x800, xPos=0x3C4, yPos=0x1DE |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 40 bytes |
// /* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00" // ...(...d....p... |
// /* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x01\x00\x00\x27\x00\x00\x00" // ............'... |
// /* 0020 */ "\x00\x00\x00\x00\x03\x00\x32\x00"                                 // ......2. |
// Dump done on RDP Client (4) 40 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=0 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=39 sdata_in.len=0 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// Front::process_data: PDUTYPE2_FONTLIST |
// Front::send_fontmap |
// Sec clear payload to send: |
// /* 0000 */ 0xbe, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0xbe, 0x00, 0x28, 0x00,  // ..............(. |
// /* 0010 */ 0x00, 0x00, 0xff, 0x02, 0xb6, 0x00, 0x28, 0x00, 0x00, 0x00, 0x27, 0x00, 0x27, 0x00, 0x03, 0x00,  // ......(...'.'... |
// /* 0020 */ 0x04, 0x00, 0x00, 0x00, 0x26, 0x00, 0x01, 0x00, 0x1e, 0x00, 0x02, 0x00, 0x1f, 0x00, 0x03, 0x00,  // ....&........... |
// /* 0030 */ 0x1d, 0x00, 0x04, 0x00, 0x27, 0x00, 0x05, 0x00, 0x0b, 0x00, 0x06, 0x00, 0x28, 0x00, 0x08, 0x00,  // ....'.......(... |
// /* 0040 */ 0x21, 0x00, 0x09, 0x00, 0x20, 0x00, 0x0a, 0x00, 0x22, 0x00, 0x0b, 0x00, 0x25, 0x00, 0x0c, 0x00,  // !... ..."...%... |
// /* 0050 */ 0x24, 0x00, 0x0d, 0x00, 0x23, 0x00, 0x0e, 0x00, 0x19, 0x00, 0x0f, 0x00, 0x16, 0x00, 0x10, 0x00,  // $...#........... |
// /* 0060 */ 0x15, 0x00, 0x11, 0x00, 0x1c, 0x00, 0x12, 0x00, 0x1b, 0x00, 0x13, 0x00, 0x1a, 0x00, 0x14, 0x00,  // ................ |
// /* 0070 */ 0x17, 0x00, 0x15, 0x00, 0x18, 0x00, 0x16, 0x00, 0x0e, 0x00, 0x18, 0x00, 0x0c, 0x00, 0x19, 0x00,  // ................ |
// /* 0080 */ 0x0d, 0x00, 0x1a, 0x00, 0x12, 0x00, 0x1b, 0x00, 0x14, 0x00, 0x1f, 0x00, 0x13, 0x00, 0x20, 0x00,  // .............. . |
// /* 0090 */ 0x00, 0x00, 0x21, 0x00, 0x0a, 0x00, 0x22, 0x00, 0x06, 0x00, 0x23, 0x00, 0x07, 0x00, 0x24, 0x00,  // ..!..."...#...$. |
// /* 00a0 */ 0x08, 0x00, 0x25, 0x00, 0x09, 0x00, 0x26, 0x00, 0x04, 0x00, 0x27, 0x00, 0x03, 0x00, 0x28, 0x00,  // ..%...&...'...(. |
// /* 00b0 */ 0x02, 0x00, 0x29, 0x00, 0x01, 0x00, 0x2a, 0x00, 0x05, 0x00, 0x2b, 0x00, 0x2a, 0x00,              // ..)...*...+.*. |
// Sending on RDP Client (4) 205 bytes |
/* 0000 */ "\x03\x00\x00\xcd\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x80\xbe\xbe" // .......h....p... |
/* 0010 */ "\x00\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\xbe\x00\x28\x00\x00" // .............(.. |
/* 0020 */ "\x00\xff\x02\xb6\x00\x28\x00\x00\x00\x27\x00\x27\x00\x03\x00\x04" // .....(...'.'.... |
/* 0030 */ "\x00\x00\x00\x26\x00\x01\x00\x1e\x00\x02\x00\x1f\x00\x03\x00\x1d" // ...&............ |
/* 0040 */ "\x00\x04\x00\x27\x00\x05\x00\x0b\x00\x06\x00\x28\x00\x08\x00\x21" // ...'.......(...! |
/* 0050 */ "\x00\x09\x00\x20\x00\x0a\x00\x22\x00\x0b\x00\x25\x00\x0c\x00\x24" // ... ..."...%...$ |
/* 0060 */ "\x00\x0d\x00\x23\x00\x0e\x00\x19\x00\x0f\x00\x16\x00\x10\x00\x15" // ...#............ |
/* 0070 */ "\x00\x11\x00\x1c\x00\x12\x00\x1b\x00\x13\x00\x1a\x00\x14\x00\x17" // ................ |
/* 0080 */ "\x00\x15\x00\x18\x00\x16\x00\x0e\x00\x18\x00\x0c\x00\x19\x00\x0d" // ................ |
/* 0090 */ "\x00\x1a\x00\x12\x00\x1b\x00\x14\x00\x1f\x00\x13\x00\x20\x00\x00" // ............. .. |
/* 00a0 */ "\x00\x21\x00\x0a\x00\x22\x00\x06\x00\x23\x00\x07\x00\x24\x00\x08" // .!..."...#...$.. |
/* 00b0 */ "\x00\x25\x00\x09\x00\x26\x00\x04\x00\x27\x00\x03\x00\x28\x00\x02" // .%...&...'...(.. |
/* 00c0 */ "\x00\x29\x00\x01\x00\x2a\x00\x05\x00\x2b\x00\x2a\x00"             // .)...*...+.*. |
// Sent dumped on RDP Client (4) 205 bytes |
// Front::send_fontmap: done |
// Front::send_data_update_sync |
// send_server_update: fastpath_support=yes compression_support=yes shareId=65538 encryptionLevel=0 initiator=0 type=3 data_extra=0 |
// Sending on RDP Client (4) 5 bytes |
/* 0000 */ "\x00\x05\x03\x00\x00"                                             // ..... |
// Sent dumped on RDP Client (4) 5 bytes |
// send_server_update done |
// Front::process_data: --------------> UP AND RUNNING <-------------- |
// Front::process_data: asking for selector |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::begin_update: level=0 |
// Front::draw_tile(MemBlt)((400, 210, 32, 32) (0, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=0 in_wait_list=false |
// Front::draw_tile(MemBlt)((432, 210, 32, 32) (32, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=1 in_wait_list=false |
// Front::draw_tile(MemBlt)((464, 210, 32, 32) (64, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=2 in_wait_list=false |
// Front::draw_tile(MemBlt)((496, 210, 32, 32) (96, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=3 in_wait_list=false |
// Front::draw_tile(MemBlt)((528, 210, 32, 32) (128, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=4 in_wait_list=false |
// Front::draw_tile(MemBlt)((560, 210, 32, 32) (160, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=5 in_wait_list=false |
// Front::draw_tile(MemBlt)((592, 210, 32, 32) (192, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=6 in_wait_list=false |
// Front::draw_tile(MemBlt)((624, 210, 32, 32) (224, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=7 in_wait_list=false |
// Front::draw_tile(MemBlt)((656, 210, 32, 32) (256, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=8 in_wait_list=false |
// Front::draw_tile(MemBlt)((688, 210, 32, 32) (288, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=9 in_wait_list=false |
// Front::draw_tile(MemBlt)((720, 210, 32, 32) (320, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=10 in_wait_list=false |
// Front::draw_tile(MemBlt)((752, 210, 32, 32) (352, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=11 in_wait_list=false |
// Front::draw_tile(MemBlt)((784, 210, 32, 32) (384, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=12 in_wait_list=false |
// Front::draw_tile(MemBlt)((816, 210, 32, 32) (416, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=13 in_wait_list=false |
// Front::draw_tile(MemBlt)((848, 210, 32, 32) (448, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=5 in_wait_list=false |
// Front::draw_tile(MemBlt)((880, 210, 32, 32) (480, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=14 in_wait_list=false |
// Front::draw_tile(MemBlt)((912, 210, 32, 32) (512, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=15 in_wait_list=false |
// Front::draw_tile(MemBlt)((944, 210, 32, 32) (544, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=16 in_wait_list=false |
// Front::draw_tile(MemBlt)((976, 210, 32, 32) (576, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=17 in_wait_list=false |
// Front::draw_tile(MemBlt)((1008, 210, 32, 32) (608, 0, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=18 in_wait_list=false |
// Front::draw_tile(MemBlt)((400, 242, 32, 32) (0, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=19 in_wait_list=false |
// Front::draw_tile(MemBlt)((432, 242, 32, 32) (32, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=20 in_wait_list=false |
// Front::draw_tile(MemBlt)((464, 242, 32, 32) (64, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=21 in_wait_list=false |
// Front::draw_tile(MemBlt)((496, 242, 32, 32) (96, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=22 in_wait_list=false |
// Front::draw_tile(MemBlt)((528, 242, 32, 32) (128, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=23 in_wait_list=false |
// Front::draw_tile(MemBlt)((560, 242, 32, 32) (160, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=24 in_wait_list=false |
// Front::draw_tile(MemBlt)((592, 242, 32, 32) (192, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=25 in_wait_list=false |
// Front::draw_tile(MemBlt)((624, 242, 32, 32) (224, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=26 in_wait_list=false |
// Front::draw_tile(MemBlt)((656, 242, 32, 32) (256, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=27 in_wait_list=false |
// Front::draw_tile(MemBlt)((688, 242, 32, 32) (288, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=28 in_wait_list=false |
// Front::draw_tile(MemBlt)((720, 242, 32, 32) (320, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=28 in_wait_list=false |
// Front::draw_tile(MemBlt)((752, 242, 32, 32) (352, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=29 in_wait_list=false |
// Front::draw_tile(MemBlt)((784, 242, 32, 32) (384, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=30 in_wait_list=false |
// Front::draw_tile(MemBlt)((816, 242, 32, 32) (416, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=31 in_wait_list=false |
// Front::draw_tile(MemBlt)((848, 242, 32, 32) (448, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=24 in_wait_list=false |
// Front::draw_tile(MemBlt)((880, 242, 32, 32) (480, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=32 in_wait_list=false |
// Front::draw_tile(MemBlt)((912, 242, 32, 32) (512, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=33 in_wait_list=false |
// Front::draw_tile(MemBlt)((944, 242, 32, 32) (544, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=34 in_wait_list=false |
// Front::draw_tile(MemBlt)((976, 242, 32, 32) (576, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=35 in_wait_list=false |
// Front::draw_tile(MemBlt)((1008, 242, 32, 32) (608, 32, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=36 in_wait_list=false |
// Front::draw_tile(MemBlt)((400, 274, 32, 32) (0, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=37 in_wait_list=false |
// Front::draw_tile(MemBlt)((432, 274, 32, 32) (32, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=38 in_wait_list=false |
// Front::draw_tile(MemBlt)((464, 274, 32, 32) (64, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=39 in_wait_list=false |
// Front::draw_tile(MemBlt)((496, 274, 32, 32) (96, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=40 in_wait_list=false |
// Front::draw_tile(MemBlt)((528, 274, 32, 32) (128, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=41 in_wait_list=false |
// Front::draw_tile(MemBlt)((560, 274, 32, 32) (160, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=42 in_wait_list=false |
// Front::draw_tile(MemBlt)((592, 274, 32, 32) (192, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=43 in_wait_list=false |
// Front::draw_tile(MemBlt)((624, 274, 32, 32) (224, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=44 in_wait_list=false |
// Front::draw_tile(MemBlt)((656, 274, 32, 32) (256, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=45 in_wait_list=false |
// Front::draw_tile(MemBlt)((688, 274, 32, 32) (288, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=45 in_wait_list=false |
// Front::draw_tile(MemBlt)((720, 274, 32, 32) (320, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=45 in_wait_list=false |
// Front::draw_tile(MemBlt)((752, 274, 32, 32) (352, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=45 in_wait_list=false |
// Front::draw_tile(MemBlt)((784, 274, 32, 32) (384, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=46 in_wait_list=false |
// Front::draw_tile(MemBlt)((816, 274, 32, 32) (416, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=47 in_wait_list=false |
// Front::draw_tile(MemBlt)((848, 274, 32, 32) (448, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=48 in_wait_list=false |
// Front::draw_tile(MemBlt)((880, 274, 32, 32) (480, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=49 in_wait_list=false |
// Front::draw_tile(MemBlt)((912, 274, 32, 32) (512, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=50 in_wait_list=false |
// Front::draw_tile(MemBlt)((944, 274, 32, 32) (544, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=51 in_wait_list=false |
// Front::draw_tile(MemBlt)((976, 274, 32, 32) (576, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=52 in_wait_list=false |
// Front::draw_tile(MemBlt)((1008, 274, 32, 32) (608, 64, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=53 in_wait_list=false |
// Front::draw_tile(MemBlt)((400, 306, 32, 32) (0, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=54 in_wait_list=false |
// Front::draw_tile(MemBlt)((432, 306, 32, 32) (32, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=55 in_wait_list=false |
// Front::draw_tile(MemBlt)((464, 306, 32, 32) (64, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=56 in_wait_list=false |
// Front::draw_tile(MemBlt)((496, 306, 32, 32) (96, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=57 in_wait_list=false |
// Front::draw_tile(MemBlt)((528, 306, 32, 32) (128, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=58 in_wait_list=false |
// Front::draw_tile(MemBlt)((560, 306, 32, 32) (160, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=59 in_wait_list=false |
// Front::draw_tile(MemBlt)((592, 306, 32, 32) (192, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=60 in_wait_list=false |
// Front::draw_tile(MemBlt)((624, 306, 32, 32) (224, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=61 in_wait_list=false |
// Front::draw_tile(MemBlt)((656, 306, 32, 32) (256, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=62 in_wait_list=false |
// Front::draw_tile(MemBlt)((688, 306, 32, 32) (288, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=63 in_wait_list=false |
// Front::draw_tile(MemBlt)((720, 306, 32, 32) (320, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=64 in_wait_list=false |
// Front::draw_tile(MemBlt)((752, 306, 32, 32) (352, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=65 in_wait_list=false |
// Front::draw_tile(MemBlt)((784, 306, 32, 32) (384, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=66 in_wait_list=false |
// Front::draw_tile(MemBlt)((816, 306, 32, 32) (416, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=67 in_wait_list=false |
// Front::draw_tile(MemBlt)((848, 306, 32, 32) (448, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=68 in_wait_list=false |
// Front::draw_tile(MemBlt)((880, 306, 32, 32) (480, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=69 in_wait_list=false |
// Front::draw_tile(MemBlt)((912, 306, 32, 32) (512, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=70 in_wait_list=false |
// Front::draw_tile(MemBlt)((944, 306, 32, 32) (544, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=71 in_wait_list=false |
// Front::draw_tile(MemBlt)((976, 306, 32, 32) (576, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=72 in_wait_list=false |
// Front::draw_tile(MemBlt)((1008, 306, 32, 32) (608, 96, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=73 in_wait_list=false |
// Front::draw_tile(MemBlt)((400, 338, 32, 32) (0, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=74 in_wait_list=false |
// Front::draw_tile(MemBlt)((432, 338, 32, 32) (32, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=75 in_wait_list=false |
// Front::draw_tile(MemBlt)((464, 338, 32, 32) (64, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=76 in_wait_list=false |
// Front::draw_tile(MemBlt)((496, 338, 32, 32) (96, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=77 in_wait_list=false |
// Front::draw_tile(MemBlt)((528, 338, 32, 32) (128, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=78 in_wait_list=false |
// Front::draw_tile(MemBlt)((560, 338, 32, 32) (160, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=79 in_wait_list=false |
// Front::draw_tile(MemBlt)((592, 338, 32, 32) (192, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=80 in_wait_list=false |
// Front::draw_tile(MemBlt)((624, 338, 32, 32) (224, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=81 in_wait_list=false |
// Front::draw_tile(MemBlt)((656, 338, 32, 32) (256, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=82 in_wait_list=false |
// Front::draw_tile(MemBlt)((688, 338, 32, 32) (288, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=83 in_wait_list=false |
// Front::draw_tile(MemBlt)((720, 338, 32, 32) (320, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=84 in_wait_list=false |
// Front::draw_tile(MemBlt)((752, 338, 32, 32) (352, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=85 in_wait_list=false |
// Front::draw_tile(MemBlt)((784, 338, 32, 32) (384, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=86 in_wait_list=false |
// Front::draw_tile(MemBlt)((816, 338, 32, 32) (416, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=87 in_wait_list=false |
// Front::draw_tile(MemBlt)((848, 338, 32, 32) (448, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=88 in_wait_list=false |
// Front::draw_tile(MemBlt)((880, 338, 32, 32) (480, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=89 in_wait_list=false |
// Front::draw_tile(MemBlt)((912, 338, 32, 32) (512, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=90 in_wait_list=false |
// Front::draw_tile(MemBlt)((944, 338, 32, 32) (544, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=91 in_wait_list=false |
// Front::draw_tile(MemBlt)((976, 338, 32, 32) (576, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=92 in_wait_list=false |
// Front::draw_tile(MemBlt)((1008, 338, 32, 32) (608, 128, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=93 in_wait_list=false |
// Front::draw_tile(MemBlt)((400, 370, 32, 32) (0, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=94 in_wait_list=false |
// Front::draw_tile(MemBlt)((432, 370, 32, 32) (32, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=95 in_wait_list=false |
// Front::draw_tile(MemBlt)((464, 370, 32, 32) (64, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=96 in_wait_list=false |
// Front::draw_tile(MemBlt)((496, 370, 32, 32) (96, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=97 in_wait_list=false |
// Front::draw_tile(MemBlt)((528, 370, 32, 32) (128, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=98 in_wait_list=false |
// Front::draw_tile(MemBlt)((560, 370, 32, 32) (160, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=99 in_wait_list=false |
// Front::draw_tile(MemBlt)((592, 370, 32, 32) (192, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=100 in_wait_list=false |
// Front::draw_tile(MemBlt)((624, 370, 32, 32) (224, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=101 in_wait_list=false |
// Front::draw_tile(MemBlt)((656, 370, 32, 32) (256, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=102 in_wait_list=false |
// Front::draw_tile(MemBlt)((688, 370, 32, 32) (288, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=103 in_wait_list=false |
// Front::draw_tile(MemBlt)((720, 370, 32, 32) (320, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=104 in_wait_list=false |
// Front::draw_tile(MemBlt)((752, 370, 32, 32) (352, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=105 in_wait_list=false |
// Front::draw_tile(MemBlt)((784, 370, 32, 32) (384, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=106 in_wait_list=false |
// Front::draw_tile(MemBlt)((816, 370, 32, 32) (416, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=107 in_wait_list=false |
// Front::draw_tile(MemBlt)((848, 370, 32, 32) (448, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=108 in_wait_list=false |
// Front::draw_tile(MemBlt)((880, 370, 32, 32) (480, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=109 in_wait_list=false |
// Front::draw_tile(MemBlt)((912, 370, 32, 32) (512, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=110 in_wait_list=false |
// Front::draw_tile(MemBlt)((944, 370, 32, 32) (544, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=111 in_wait_list=false |
// Front::draw_tile(MemBlt)((976, 370, 32, 32) (576, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=112 in_wait_list=false |
// Front::draw_tile(MemBlt)((1008, 370, 32, 32) (608, 160, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=113 in_wait_list=false |
// Front::draw_tile(MemBlt)((400, 402, 32, 32) (0, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=114 in_wait_list=false |
// Front::draw_tile(MemBlt)((432, 402, 32, 32) (32, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=115 in_wait_list=false |
// Front::draw_tile(MemBlt)((464, 402, 32, 32) (64, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=116 in_wait_list=false |
// Front::draw_tile(MemBlt)((496, 402, 32, 32) (96, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=117 in_wait_list=false |
// Front::draw_tile(MemBlt)((528, 402, 32, 32) (128, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=118 in_wait_list=false |
// Front::draw_tile(MemBlt)((560, 402, 32, 32) (160, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=119 in_wait_list=false |
// Front::draw_tile(MemBlt)((592, 402, 32, 32) (192, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=0 in_wait_list=false |
// Front::draw_tile(MemBlt)((624, 402, 32, 32) (224, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=1 in_wait_list=false |
// Front::draw_tile(MemBlt)((656, 402, 32, 32) (256, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=2 in_wait_list=false |
// Front::draw_tile(MemBlt)((688, 402, 32, 32) (288, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=3 in_wait_list=false |
// Front::draw_tile(MemBlt)((720, 402, 32, 32) (320, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=4 in_wait_list=false |
// Front::draw_tile(MemBlt)((752, 402, 32, 32) (352, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=6 in_wait_list=false |
// Front::draw_tile(MemBlt)((784, 402, 32, 32) (384, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=7 in_wait_list=false |
// Front::draw_tile(MemBlt)((816, 402, 32, 32) (416, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=8 in_wait_list=false |
// Front::draw_tile(MemBlt)((848, 402, 32, 32) (448, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=9 in_wait_list=false |
// Front::draw_tile(MemBlt)((880, 402, 32, 32) (480, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=10 in_wait_list=false |
// Front::draw_tile(MemBlt)((912, 402, 32, 32) (512, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=11 in_wait_list=false |
// Front::draw_tile(MemBlt)((944, 402, 32, 32) (544, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=12 in_wait_list=false |
// Front::draw_tile(MemBlt)((976, 402, 32, 32) (576, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=13 in_wait_list=false |
// Front::draw_tile(MemBlt)((1008, 402, 32, 32) (608, 192, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=5 in_wait_list=false |
// Front::draw_tile(MemBlt)((400, 434, 32, 32) (0, 224, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=14 in_wait_list=false |
// Front::draw_tile(MemBlt)((432, 434, 32, 32) (32, 224, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=15 in_wait_list=false |
// Front::draw_tile(MemBlt)((464, 434, 32, 32) (64, 224, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=16 in_wait_list=false |
// Front::draw_tile(MemBlt)((496, 434, 32, 32) (96, 224, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=17 in_wait_list=false |
// Front::draw_tile(MemBlt)((528, 434, 32, 32) (128, 224, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=18 in_wait_list=false |
// Front::draw_tile(MemBlt)((560, 434, 32, 32) (160, 224, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=19 in_wait_list=false |
// Front::draw_tile(MemBlt)((592, 434, 32, 32) (192, 224, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=20 in_wait_list=false |
// Front::draw_tile(MemBlt)((624, 434, 32, 32) (224, 224, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=21 in_wait_list=false |
// Front::draw_tile(MemBlt)((656, 434, 32, 32) (256, 224, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=22 in_wait_list=false |
// Front::draw_tile(MemBlt)((688, 434, 32, 32) (288, 224, 32, 32)) |
// RDPSerializer: draw MemBlt, cache_id=1 cache_index=23 in_wait_list=false |
// send_server_update: fastpath_support=yes compression_support=yes shareId=65538 encryptionLevel=0 initiator=0 type=0 data_extra=298 |
// Sending on RDP Client (4) 8150 bytes |
} /* end outdata */;

const char indata[] = /* NOLINT */
{
// Listen: binding socket 3 on 0.0.0.0:3389 |
// Listen: listening on socket 3 |
// SocketTransport: recv_timeout=1000 |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 39 bytes |
/* 0000 */ "\x03\x00\x00\x27\x22\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" // ...'"......Cooki |
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x78\x0d\x0a\x01" // e: mstshash=x... |
/* 0020 */ "\x00\x08\x00\x0b\x00\x00\x00"                                     // ....... |
// Dump done on RDP Client (4) 39 bytes |
// Front::incoming: CONNECTION_INITIATION |
// Front::incoming: receiving x224 request PDU |
// CR Recv: PROTOCOL TLS |
// CR Recv: PROTOCOL HYBRID |
// CR Recv: PROTOCOL HYBRID EX |
// Front::incoming: sending x224 connection confirm PDU |
// -----------------> Front::incoming: TLS Support Enabled |
// CC Send: PROTOCOL TLS |
// Sending on RDP Client (4) 19 bytes |
// /* 0000 */ "\x03\x00\x00\x13\x0e\xd0\x00\x00\x00\x00\x00\x02\x01\x08\x00\x01" // ................ |
// /* 0010 */ "\x00\x00\x00"                                                     // ... |
// Sent dumped on RDP Client (4) 19 bytes |
// SocketTransport::enable_server_tls() start |
// TLSContext::enable_server_tls() set SSL options |
// TLSContext::enable_server_tls() set SSL cipher list |
// TLSContext::X509_get_pubkey() |
// TLSContext::i2d_PublicKey() |
// TLSContext::i2d_PublicKey() |
// Incoming connection to Bastion using TLS version TLSv1.2 |
// TLSContext::Negociated cipher used ECDHE-RSA-AES256-GCM-SHA384 |
// SocketTransport::enable_server_tls() done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 462 bytes |
/* 0000 */ "\x03\x00\x01\xce\x02\xf0\x80\x7f\x65\x82\x01\xc2\x04\x01\x01\x04" // ........e....... |
/* 0010 */ "\x01\x01\x01\x01\xff\x30\x19\x02\x01\x22\x02\x01\x02\x02\x01\x00" // .....0..."...... |
/* 0020 */ "\x02\x01\x01\x02\x01\x00\x02\x01\x01\x02\x02\xff\xff\x02\x01\x02" // ................ |
/* 0030 */ "\x30\x19\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02\x01" // 0............... |
/* 0040 */ "\x00\x02\x01\x01\x02\x02\x04\x20\x02\x01\x02\x30\x1c\x02\x02\xff" // ....... ...0.... |
/* 0050 */ "\xff\x02\x02\xfc\x17\x02\x02\xff\xff\x02\x01\x01\x02\x01\x00\x02" // ................ |
/* 0060 */ "\x01\x01\x02\x02\xff\xff\x02\x01\x02\x04\x82\x01\x61\x00\x05\x00" // ............a... |
/* 0070 */ "\x14\x7c\x00\x01\x81\x58\x00\x08\x00\x10\x00\x01\xc0\x00\x44\x75" // .|...X........Du |
/* 0080 */ "\x63\x61\x81\x4a\x01\xc0\xea\x00\x0c\x00\x08\x00\xa0\x05\x84\x03" // ca.J............ |
/* 0090 */ "\x01\xca\x03\xaa\x0c\x04\x00\x00\xba\x47\x00\x00\x43\x00\x4c\x00" // .........G..C.L. |
/* 00a0 */ "\x54\x00\x30\x00\x32\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // T.0.2........... |
/* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00" // ................ |
/* 00c0 */ "\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x01\xca\x01\x00\x00\x00\x00\x00" // ................ |
/* 0110 */ "\x18\x00\x0f\x00\xaf\x07\x33\x00\x35\x00\x36\x00\x61\x00\x37\x00" // ......3.5.6.a.7. |
/* 0120 */ "\x37\x00\x38\x00\x38\x00\x2d\x00\x38\x00\x36\x00\x33\x00\x37\x00" // 7.8.8.-.8.6.3.7. |
/* 0130 */ "\x2d\x00\x34\x00\x31\x00\x63\x00\x64\x00\x2d\x00\x39\x00\x39\x00" // -.4.1.c.d.-.9.9. |
/* 0140 */ "\x37\x00\x64\x00\x2d\x00\x34\x00\x37\x00\x35\x00\x64\x00\x38\x00" // 7.d.-.4.7.5.d.8. |
/* 0150 */ "\x31\x00\x61\x00\x00\x00\x07\x00\x01\x00\x00\x00\x00\x00\x00\x00" // 1.a............. |
/* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\xc0" // ................ |
/* 0170 */ "\x0c\x00\x15\x00\x00\x00\x00\x00\x00\x00\x02\xc0\x0c\x00\x1b\x00" // ................ |
/* 0180 */ "\x00\x00\x00\x00\x00\x00\x03\xc0\x38\x00\x04\x00\x00\x00\x72\x64" // ........8.....rd |
/* 0190 */ "\x70\x64\x72\x00\x00\x00\x00\x00\x80\x80\x72\x64\x70\x73\x6e\x64" // pdr.......rdpsnd |
/* 01a0 */ "\x00\x00\x00\x00\x00\xc0\x63\x6c\x69\x70\x72\x64\x72\x00\x00\x00" // ......cliprdr... |
/* 01b0 */ "\xa0\xc0\x64\x72\x64\x79\x6e\x76\x63\x00\x00\x00\x80\xc0\x06\xc0" // ..drdynvc....... |
/* 01c0 */ "\x08\x00\x00\x00\x00\x00\x0a\xc0\x08\x00\x05\x03\x00\x00"         // .............. |
// Dump done on RDP Client (4) 462 bytes |
// Front::incoming: Basic Settings Exchange |
// GCC::UserData tag=c001 length=234 |
// Front::incoming: Received from Client GCC User Data CS_CORE (234 bytes) |
// cs_core::length [00ea] |
// cs_core::version [8000c] Unknown client |
// cs_core::desktopWidth  = 1440 |
// cs_core::desktopHeight = 900 |
// cs_core::colorDepth    = [ca01] [RNS_UD_COLOR_8BPP] superseded by postBeta2ColorDepth |
// cs_core::SASSequence   = [aa03] [RNS_UD_SAS_DEL] |
// cs_core::keyboardLayout= 040c |
// cs_core::clientBuild   = 18362 |
// cs_core::clientName    = CLT02 |
// cs_core::keyboardType  = [0004] IBM enhanced (101-key or 102-key) keyboard |
// cs_core::keyboardSubType      = [0000] OEM code |
// cs_core::keyboardFunctionKey  = 12 function keys |
// cs_core::imeFileName    = |
// cs_core::postBeta2ColorDepth  = [ca01] [8 bpp] |
// cs_core::clientProductId = 1 |
// cs_core::serialNumber = 0 |
// cs_core::highColorDepth  = [0018] [24-bit RGB mask] |
// cs_core::supportedColorDepths  = [000f] [24/16/15/32] |
// cs_core::earlyCapabilityFlags  = [07af] |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_ERRINFO_PDU |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_WANT_32BPP_SESSION |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_STATUSINFO_PDU |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_STRONG_ASYMMETRIC_KEYS |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_VALID_CONNECTION_TYPE |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_NETCHAR_AUTODETECT |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_DYNVC_GFX_PROTOCOL |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_DYNAMIC_TIME_ZONE |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_HEARTBEAT_PDU |
// cs_core::clientDigProductId=[330035003600610037003700380038002e0038003600330037002e0034003100630064002e003a003a00370064002e0034003700350064003800310061000000] |
// cs_core::connectionType = <unknown(greater than 6)> |
// cs_core::pad1octet = 0 |
// cs_core::serverSelectedProtocol = 1 |
// cs_core::desktopPhysicalWidth = 0 |
// cs_core::desktopPhysicalHeight = 0 |
// cs_core::desktopOrientation = 0 |
// cs_core::desktopScaleFactor = 0 |
// cs_core::deviceScaleFactor = 0 |
// GCC::UserData tag=c004 length=12 |
// Front::incoming: Receiving from Client GCC User Data CS_CLUSTER (12 bytes) |
// cs_cluster::flags [0015] |
// cs_cluster::flags::REDIRECTION_SUPPORTED |
// cs_cluster::flags::redirectionVersion = 6 |
// GCC::UserData tag=c002 length=12 |
// Front::incoming: Received from Client GCC User Data CS_SECURITY (12 bytes) |
// CSSecGccUserData::encryptionMethods 27 |
// CSSecGccUserData::extEncryptionMethods 0 |
// GCC::UserData tag=c003 length=56 |
// Front::incoming: Received from Client GCC User Data CS_NET (56 bytes) |
// cs_net::channelCount   = 4 |
// cs_net::channel 'rdpdr' [1004] INITIALIZED COMPRESS_RDP |
// cs_net::channel 'rdpsnd' [1005] INITIALIZED |
// cs_net::channel 'cliprdr' [1006] INITIALIZED COMPRESS_RDP SHOW_PROTOCOL |
// cs_net::channel 'drdynvc' [1007] INITIALIZED COMPRESS_RDP |
// GCC::UserData tag=c006 length=8 |
// Front::incoming: Receiving from Client GCC User Data CS_MCS_MSGCHANNEL (8 bytes) |
// CSMCSMsgChannel::flags 0 |
// GCC::UserData tag=c00a length=8 |
// Front::incoming: Receiving from Client GCC User Data CS_MULTITRANSPORT (8 bytes) |
// CSMultiTransport::flags 773 |
// Front::incoming: Sending to client GCC User Data SC_CORE (12 bytes) |
// sc_core::version [80004] RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 servers) |
// sc_core::clientRequestedProtocols  = 11 |
// Front::incoming: Sending to client GCC User Data SC_NET (16 bytes) |
// sc_net::MCSChannelId   = 1003 |
// sc_net::channelCount   = 4 |
// sc_net::channel[1004]::id = 1004 |
// sc_net::channel[1005]::id = 1005 |
// sc_net::channel[1006]::id = 1006 |
// sc_net::channel[1007]::id = 1007 |
// Front::incoming: Sending to client GCC User Data SC_SECURITY (12 bytes) |
// sc_security::encryptionMethod = 0 |
// sc_security::encryptionLevel  = 0 |
// Sending on RDP Client (4) 109 bytes |
// /* 0000 */ "\x03\x00\x00\x6d\x02\xf0\x80\x7f\x66\x63\x0a\x01\x00\x02\x01\x00" // ...m....fc...... |
// /* 0010 */ "\x30\x1a\x02\x01\x22\x02\x01\x03\x02\x01\x00\x02\x01\x01\x02\x01" // 0..."........... |
// /* 0020 */ "\x00\x02\x01\x01\x02\x03\x00\xff\xf8\x02\x01\x02\x04\x3f\x00\x05" // .............?.. |
// /* 0030 */ "\x00\x14\x7c\x00\x01\x2a\x14\x76\x0a\x01\x01\x00\x01\xc0\x00\x4d" // ..|..*.v.......M |
// /* 0040 */ "\x63\x44\x6e\x80\x28\x01\x0c\x0c\x00\x04\x00\x08\x00\x0b\x00\x00" // cDn.(........... |
// /* 0050 */ "\x00\x03\x0c\x10\x00\xeb\x03\x04\x00\xec\x03\xed\x03\xee\x03\xef" // ................ |
// /* 0060 */ "\x03\x02\x0c\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00"             // ............. |
// Sent dumped on RDP Client (4) 109 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 12 bytes |
/* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x04\x01\x00\x01\x00"                 // ............ |
// Dump done on RDP Client (4) 12 bytes |
// Front::incoming: Channel Connection |
// Front::incoming: Recv MCS::ErectDomainRequest |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 8 bytes |
/* 0000 */ "\x03\x00\x00\x08\x02\xf0\x80\x28"                                 // .......( |
// Dump done on RDP Client (4) 8 bytes |
// Front::incoming: Recv MCS::AttachUserRequest |
// Front::incoming: Send MCS::AttachUserConfirm userid=0 |
// Sending on RDP Client (4) 11 bytes |
// /* 0000 */ "\x03\x00\x00\x0b\x02\xf0\x80\x2e\x00\x00\x00"                     // ........... |
// Sent dumped on RDP Client (4) 11 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 12 bytes |
/* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xe9"                 // .......8.... |
// Dump done on RDP Client (4) 12 bytes |
// Sending on RDP Client (4) 15 bytes |
// /* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xe9\x03\xe9"     // .......>....... |
// Sent dumped on RDP Client (4) 15 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 12 bytes |
/* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xeb"                 // .......8.... |
// Dump done on RDP Client (4) 12 bytes |
// Sending on RDP Client (4) 15 bytes |
// /* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xeb\x03\xeb"     // .......>....... |
// Sent dumped on RDP Client (4) 15 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 12 bytes |
/* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xec"                 // .......8.... |
// Dump done on RDP Client (4) 12 bytes |
// Front::incoming: cjrq[0] = 1004 -> cjcf |
// Sending on RDP Client (4) 15 bytes |
// /* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xec\x03\xec"     // .......>....... |
// Sent dumped on RDP Client (4) 15 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 12 bytes |
/* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xed"                 // .......8.... |
// Dump done on RDP Client (4) 12 bytes |
// Front::incoming: cjrq[1] = 1005 -> cjcf |
// Sending on RDP Client (4) 15 bytes |
// /* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xed\x03\xed"     // .......>....... |
// Sent dumped on RDP Client (4) 15 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 12 bytes |
/* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xee"                 // .......8.... |
// Dump done on RDP Client (4) 12 bytes |
// Front::incoming: cjrq[2] = 1006 -> cjcf |
// Sending on RDP Client (4) 15 bytes |
// /* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xee\x03\xee"     // .......>....... |
// Sent dumped on RDP Client (4) 15 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 12 bytes |
/* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x38\x00\x00\x03\xef"                 // .......8.... |
// Dump done on RDP Client (4) 12 bytes |
// Front::incoming: cjrq[3] = 1007 -> cjcf |
// Sending on RDP Client (4) 15 bytes |
// /* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xef\x03\xef"     // .......>....... |
// Sent dumped on RDP Client (4) 15 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 329 bytes |
/* 0000 */ "\x03\x00\x01\x49\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\x3a\x40" // ...I...d....p.:@ |
/* 0010 */ "\x00\xf3\xa8\x09\x04\x0c\x04\xb3\x47\x01\x00\x00\x00\x02\x00\x00" // ........G....... |
/* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x78\x00\x00\x00\x00\x00\x00\x00\x00" // .......x........ |
/* 0030 */ "\x00\x02\x00\x18\x00\x31\x00\x30\x00\x2e\x00\x31\x00\x30\x00\x2e" // .....1.0...1.0.. |
/* 0040 */ "\x00\x34\x00\x34\x00\x2e\x00\x39\x00\x39\x00\x00\x00\x40\x00\x43" // .4.4...9.9...@.C |
/* 0050 */ "\x00\x3a\x00\x5c\x00\x57\x00\x49\x00\x4e\x00\x44\x00\x4f\x00\x57" // .:...W.I.N.D.O.W |
/* 0060 */ "\x00\x53\x00\x5c\x00\x73\x00\x79\x00\x73\x00\x74\x00\x65\x00\x6d" // .S...s.y.s.t.e.m |
/* 0070 */ "\x00\x33\x00\x32\x00\x5c\x00\x6d\x00\x73\x00\x74\x00\x73\x00\x63" // .3.2...m.s.t.s.c |
/* 0080 */ "\x00\x61\x00\x78\x00\x2e\x00\x64\x00\x6c\x00\x6c\x00\x00\x00\xc4" // .a.x...d.l.l.... |
/* 0090 */ "\xff\xff\xff\x52\x00\x6f\x00\x6d\x00\x61\x00\x6e\x00\x63\x00\x65" // ...R.o.m.a.n.c.e |
/* 00a0 */ "\x00\x20\x00\x53\x00\x74\x00\x61\x00\x6e\x00\x64\x00\x61\x00\x72" // . .S.t.a.n.d.a.r |
/* 00b0 */ "\x00\x64\x00\x20\x00\x54\x00\x69\x00\x6d\x00\x65\x00\x00\x00\x00" // .d. .T.i.m.e.... |
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 00d0 */ "\x00\x00\x00\x00\x00\x0a\x00\x00\x00\x05\x00\x03\x00\x00\x00\x00" // ................ |
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x52\x00\x6f\x00\x6d\x00\x61\x00\x6e" // .......R.o.m.a.n |
/* 00f0 */ "\x00\x63\x00\x65\x00\x20\x00\x44\x00\x61\x00\x79\x00\x6c\x00\x69" // .c.e. .D.a.y.l.i |
/* 0100 */ "\x00\x67\x00\x68\x00\x74\x00\x20\x00\x54\x00\x69\x00\x6d\x00\x65" // .g.h.t. .T.i.m.e |
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x05\x00\x02" // ................ |
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\xc4\xff\xff\xff\x03\x00\x00\x00\x06" // ................ |
/* 0140 */ "\x00\x00\x00\x00\x00\x64\x00\x00\x00"                             // .....d... |
// Dump done on RDP Client (4) 329 bytes |
// Front::incoming: RDP Security Commencement |
// Front::incoming: TLS mode: exchange packet disabled |
// Front::incoming: Secure Settings Exchange |
// RDP-5 Style logon |
// Receiving from client InfoPacket |
// InfoPacket::CodePage 67896329 |
// InfoPacket::flags 0x147b3 |
// InfoPacket::flags:INFO_MOUSE yes |
// InfoPacket::flags:INFO_DISABLECTRLALTDEL yes |
// InfoPacket::flags:INFO_AUTOLOGON no |
// InfoPacket::flags:INFO_UNICODE yes |
// InfoPacket::flags:INFO_MAXIMIZESHELL  yes |
// InfoPacket::flags:INFO_LOGONNOTIFY no |
// InfoPacket::flags:INFO_COMPRESSION yes |
// InfoPacket::flags:CompressionTypeMask yes |
// InfoPacket::flags:INFO_ENABLEWINDOWSKEY  yes |
// InfoPacket::flags:INFO_REMOTECONSOLEAUDIO no |
// InfoPacket::flags:INFO_FORCE_ENCRYPTED_CS_PDU yes |
// InfoPacket::flags:INFO_RAIL no |
// InfoPacket::flags:INFO_LOGONERRORS yes |
// InfoPacket::flags:INFO_MOUSE_HAS_WHEEL no |
// InfoPacket::flags:INFO_PASSWORD_IS_SC_PIN no |
// InfoPacket::flags:INFO_NOAUDIOPLAYBACK no |
// InfoPacket::flags:INFO_USING_SAVED_CREDS no |
// InfoPacket::flags:INFO_AUDIOCAPTURE no |
// InfoPacket::flags:INFO_VIDEO_DISABLE no |
// InfoPacket::flags:INFO_HIDEF_RAIL_SUPPORTED no |
// InfoPacket::cbDomain 2 |
// InfoPacket::cbUserName 4 |
// InfoPacket::cbPassword 2 |
// InfoPacket::cbAlternateShell 2 |
// InfoPacket::cbWorkingDir 2 |
// InfoPacket::Domain |
// InfoPacket::UserName x |
// InfoPacket::Password <hidden> |
// InfoPacket::AlternateShell |
// InfoPacket::WorkingDir |
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2 |
// InfoPacket::ExtendedInfoPacket::cbClientAddress 24 |
// InfoPacket::ExtendedInfoPacket::clientAddress 10.10.44.99 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 64 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\WINDOWS\system32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 3 |
// InfoPacket::ExtendedInfoPacket::performanceFlags 0x6 |
// InfoPacket::ExtendedInfoPacket::PERF_DISABLE_FULLWINDOWDRAG |
// InfoPacket::ExtendedInfoPacket::PERF_DISABLE_MENUANIMATIONS |
// InfoPacket::ExtendedInfoPacket::cbAutoReconnectLen 0 |
// InfoPacket::ExtendedInfoPacket::reserved1 100 |
// InfoPacket::ExtendedInfoPacket::reserved2 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::Bias 4294967236 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardName Romance Standard Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMonth 10 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDay 5 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wHour 3 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardBias 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightName Romance Daylight Time |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMonth 3 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDay 5 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wHour 2 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightBias 4294967236 |
// client info: performance flags before=0x00000006 after=0x0000002E default=0x00000080 present=0x00000028 not-present=0x00000000 |
// Front::incoming: Keyboard Layout = 0x40c |
// Front::incoming: licencing not client_info.is_mce |
// Front::incoming: licencing send_lic_initial |
// Sending on RDP Client (4) 337 bytes |
// /* 0000 */ "\x03\x00\x01\x51\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x81\x42\x80" // ...Q...h....p.B. |
// /* 0010 */ "\x00\x00\x00\x01\x02\x3e\x01\x7b\x3c\x31\xa6\xae\xe8\x74\xf6\xb4" // .....>.{<1...t.. |
// /* 0020 */ "\xa5\x03\x90\xe7\xc2\xc7\x39\xba\x53\x1c\x30\x54\x6e\x90\x05\xd0" // ......9.S.0Tn... |
// /* 0030 */ "\x05\xce\x44\x18\x91\x83\x81\x00\x00\x04\x00\x2c\x00\x00\x00\x4d" // ..D........,...M |
// /* 0040 */ "\x00\x69\x00\x63\x00\x72\x00\x6f\x00\x73\x00\x6f\x00\x66\x00\x74" // .i.c.r.o.s.o.f.t |
// /* 0050 */ "\x00\x20\x00\x43\x00\x6f\x00\x72\x00\x70\x00\x6f\x00\x72\x00\x61" // . .C.o.r.p.o.r.a |
// /* 0060 */ "\x00\x74\x00\x69\x00\x6f\x00\x6e\x00\x00\x00\x08\x00\x00\x00\x32" // .t.i.o.n.......2 |
// /* 0070 */ "\x00\x33\x00\x36\x00\x00\x00\x0d\x00\x04\x00\x01\x00\x00\x00\x03" // .3.6............ |
// /* 0080 */ "\x00\xb8\x00\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x06" // ................ |
// /* 0090 */ "\x00\x5c\x00\x52\x53\x41\x31\x48\x00\x00\x00\x00\x02\x00\x00\x3f" // ...RSA1H.......? |
// /* 00a0 */ "\x00\x00\x00\x01\x00\x01\x00\x01\xc7\xc9\xf7\x8e\x5a\x38\xe4\x29" // ............Z8.) |
// /* 00b0 */ "\xc3\x00\x95\x2d\xdd\x4c\x3e\x50\x45\x0b\x0d\x9e\x2a\x5d\x18\x63" // ...-.L>PE...*].c |
// /* 00c0 */ "\x64\xc4\x2c\xf7\x8f\x29\xd5\x3f\xc5\x35\x22\x34\xff\xad\x3a\xe6" // d.,..).?.5"4..:. |
// /* 00d0 */ "\xe3\x95\x06\xae\x55\x82\xe3\xc8\xc7\xb4\xa8\x47\xc8\x50\x71\x74" // ....U......G.Pqt |
// /* 00e0 */ "\x29\x53\x89\x6d\x9c\xed\x70\x00\x00\x00\x00\x00\x00\x00\x00\x08" // )S.m..p......... |
// /* 00f0 */ "\x00\x48\x00\xa8\xf4\x31\xb9\xab\x4b\xe6\xb4\xf4\x39\x89\xd6\xb1" // .H...1..K...9... |
// /* 0100 */ "\xda\xf6\x1e\xec\xb1\xf0\x54\x3b\x5e\x3e\x6a\x71\xb4\xf7\x75\xc8" // ......T;^>jq..u. |
// /* 0110 */ "\x16\x2f\x24\x00\xde\xe9\x82\x99\x5f\x33\x0b\xa9\xa6\x94\xaf\xcb" // ./$....._3...... |
// /* 0120 */ "\x11\xc3\xf2\xdb\x09\x42\x68\x29\x56\x58\x01\x56\xdb\x59\x03\x69" // .....Bh)VX.V.Y.i |
// /* 0130 */ "\xdb\x7d\x37\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x0e" // .}7............. |
// /* 0140 */ "\x00\x0e\x00\x6d\x69\x63\x72\x6f\x73\x6f\x66\x74\x2e\x63\x6f\x6d" // ...microsoft.com |
// /* 0150 */ "\x00"                                                             // . |
// Sent dumped on RDP Client (4) 337 bytes |
// Front::incoming: Waiting for answer to lic_initial |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 164 bytes |
/* 0000 */ "\x03\x00\x00\xa4\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x95\x80" // .......d....p... |
/* 0010 */ "\x00\x40\x02\x13\x83\x91\x00\x01\x00\x00\x00\x00\x00\x01\x08\xd8" // .@.............. |
/* 0020 */ "\xf0\xee\x70\xaf\x7d\xbc\xcf\x68\x5e\xab\xfb\x4a\x67\x5e\x19\xb1" // ..p.}..h^..Jg^.. |
/* 0030 */ "\x16\x52\xda\x63\x3e\xec\xbf\x0f\xd1\x03\xf1\xe5\xd6\x95\xb9\x00" // .R.c>........... |
/* 0040 */ "\x00\x48\x00\x60\x03\x89\x08\xe9\x09\x29\xdd\x7e\x57\x18\xa5\x54" // .H.`.....).~W..T |
/* 0050 */ "\x09\x49\x87\x81\x1c\xfe\x2a\x9b\x00\xdf\x85\xaa\xc5\xa0\x22\x1f" // .I....*.......". |
/* 0060 */ "\xd5\x3b\x64\x9d\x2d\xe7\x0b\x3a\x6a\x56\x1a\x41\x44\xdb\x05\xaa" // .;d.-..:jV.AD... |
/* 0070 */ "\xba\xd5\x24\xf2\x1d\x95\x0b\xe4\x01\x54\x4f\x65\x15\xf5\xb0\x3a" // ..$......TOe...: |
/* 0080 */ "\xa9\x8a\x51\x00\x00\x00\x00\x00\x00\x00\x00\x0f\x00\x0b\x00\x52" // ..Q............R |
/* 0090 */ "\x65\x64\x65\x6d\x70\x74\x69\x6f\x6e\x00\x10\x00\x06\x00\x43\x4c" // edemption.....CL |
/* 00a0 */ "\x54\x30\x32\x00"                                                 // T02. |
// Dump done on RDP Client (4) 164 bytes |
// Front::incoming: WAITING_FOR_ANSWER_TO_LICENCE |
// Front::incoming: LIC::NEW_LICENSE_REQUEST |
// Sending on RDP Client (4) 34 bytes |
// /* 0000 */ "\x03\x00\x00\x22\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x14\x80\x00" // ..."...h....p... |
// /* 0010 */ "\x10\x00\xff\x02\x10\x00\x07\x00\x00\x00\x02\x00\x00\x00\x28\x14" // ..............(. |
// /* 0020 */ "\x00\x00"                                                         // .. |
// Sent dumped on RDP Client (4) 34 bytes |
// Front::incoming: send_demand_active |
// Front::send_demand_active |
// Front::send_demand_active: Sending to client General caps (24 bytes) |
//      General caps::major 1 |
//      General caps::minor 3 |
//      General caps::protocol 512 |
//      General caps::pad2octetA 0 |
//      General caps::compression type 0 |
//      General caps::extra flags 1 |
//      General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED yes |
//      General caps::extraflags:LONG_CREDENTIALS_SUPPORTED no |
//      General caps::extraflags:AUTORECONNECT_SUPPORTED no |
//      General caps::extraflags:ENC_SALTED_CHECKSUM no |
//      General caps::extraflags:NO_BITMAP_COMPRESSION_HDR no |
//      General caps::updateCapability 0 |
//      General caps::remoteUnshare 0 |
//      General caps::compressionLevel 0 |
//      General caps::refreshRectSupport 0 |
//      General caps::suppressOutputSupport 0 |
// Front::send_demand_active: Sending to client Bitmap caps (28 bytes) |
//      Bitmap caps::preferredBitsPerPixel 24 |
//      Bitmap caps::receive1BitPerPixel 1 |
//      Bitmap caps::receive4BitsPerPixel 1 |
//      Bitmap caps::receive8BitsPerPixel 1 |
//      Bitmap caps::desktopWidth 1440 |
//      Bitmap caps::desktopHeight 900 |
//      Bitmap caps::pad2octets 0 |
//      Bitmap caps::desktopResizeFlag 1 (yes) |
//      Bitmap caps::bitmapCompressionFlag 1 yes |
//      Bitmap caps::highColorFlags 0 |
//      Bitmap caps::drawingFlags 8 |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY no |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_COLOR_SUBSAMPLING no |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_SKIP_ALPHA yes |
//      Bitmap caps::multipleRectangleSupport 1 |
//      Bitmap caps::pad2octetsB 0 |
// Front::send_demand_active: Sending to client Font caps (8 bytes) |
//      Font caps::fontSupportFlags 1 |
//      Font caps::pad2octets 0 |
// Front::send_demand_active: Sending to client Order caps (88 bytes) |
//      Order caps::terminalDescriptor 0 |
//      Order caps::pad4octetsA 1078071040 |
//      Order caps::desktopSaveXGranularity 1 |
//      Order caps::desktopSaveYGranularity 20 |
//      Order caps::pad2octetsA 0 |
//      Order caps::maximumOrderLevel 1 |
//      Order caps::numberFonts 47 |
//      Order caps::orderFlags 0x22 |
//      Order caps::orderSupport[DSTBLT] 1 |
//      Order caps::orderSupport[PATBLT] 1 |
//      Order caps::orderSupport[SCRBLT] 1 |
//      Order caps::orderSupport[MEMBLT] 1 |
//      Order caps::orderSupport[MEM3BLT] 0 |
//      Order caps::orderSupport[ATEXTOUT] 0 |
//      Order caps::orderSupport[AEXTTEXTOUT] 0 |
//      Order caps::orderSupport[DRAWNINEGRID] 0 |
//      Order caps::orderSupport[LINETO] 1 |
//      Order caps::orderSupport[MULTI_DRAWNINEGRID] 1 |
//      Order caps::orderSupport[OPAQUERECT] 1 |
//      Order caps::orderSupport[SAVEBITMAP] 0 |
//      Order caps::orderSupport[WTEXTOUT] 0 |
//      Order caps::orderSupport[MEMBLT_V2] 0 |
//      Order caps::orderSupport[MEM3BLT_V2] 0 |
//      Order caps::orderSupport[MULTIDSTBLT] 0 |
//      Order caps::orderSupport[MULTIPATBLT] 0 |
//      Order caps::orderSupport[MULTISCRBLT] 0 |
//      Order caps::orderSupport[MULTIOPAQUERECT] 0 |
//      Order caps::orderSupport[FAST_INDEX] 0 |
//      Order caps::orderSupport[POLYGON_SC] 0 |
//      Order caps::orderSupport[POLYGON_CB] 0 |
//      Order caps::orderSupport[POLYLINE] 1 |
//      Order caps::orderSupport[UnusedIndex7] 0 |
//      Order caps::orderSupport[FAST_GLYPH] 0 |
//      Order caps::orderSupport[ELLIPSE_SC] 1 |
//      Order caps::orderSupport[ELLIPSE_CB] 0 |
//      Order caps::orderSupport[GLYPH] 1 |
//      Order caps::orderSupport[GLYPH_WEXTTEXTOUT] 0 |
//      Order caps::orderSupport[GLYPH_WLONGTEXTOUT] 0 |
//      Order caps::orderSupport[GLYPH_WLONGEXTTEXTOUT] 0 |
//      Order caps::orderSupport[UnusedIndex11] 0 |
//      Order caps::textFlags 1697 |
//      Order caps::orderSupportExFlags 0x0 |
//      Order caps::pad4octetsB 1000000 |
//      Order caps::desktopSaveSize 1000000 |
//      Order caps::pad2octetsC 1 |
//      Order caps::pad2octetsD 0 |
//      Order caps::textANSICodePage 0 |
//      Order caps::pad2octetsE 0 |
// Front::send_demand_active: Sending to client Pointer caps (10 bytes) |
//      Pointer caps::colorPointerFlag 1 |
//      Pointer caps::colorPointerCacheSize 25 |
//      Pointer caps::pointerCacheSize 25 |
// Front::send_demand_active: Sending to client Input caps (88 bytes) |
//      Input caps::inputFlags 0x29 |
//      Input caps::pad2octetsA 0 |
//      Input caps::keyboardLayout 0 |
//      Input caps::keyboardType 0 |
//      Input caps::keyboardSubType 0 |
//      Input caps::keyboardFunctionKey 0 |
//      Input caps::imeFileName |
// Front::send_demand_active: Sending to client VirtualChannel caps (12 bytes) |
//      VirtualChannel caps::flags 0 |
//      VirtualChannel caps::VCChunkSize 1600 |
// Front::send_demand_active: Sending to client Share caps (8 bytes) |
//      Share caps::nodeId 1001 |
//      Share caps::pad2octets 46562 |
// Front::send_demand_active: Sending to client ColorCache caps (8 bytes) |
//      ColorCache caps::colorTableCacheSize 6 |
//      ColorCache caps::pad2octets 0 |
// Sending on RDP Client (4) 315 bytes |
// /* 0000 */ "\x03\x00\x01\x3b\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x81\x2c\x2c" // ...;...h....p.,, |
// /* 0010 */ "\x01\x11\x00\xe9\x03\x02\x00\x01\x00\x04\x00\x16\x01\x52\x44\x50" // .............RDP |
// /* 0020 */ "\x00\x09\x00\x00\x00\x01\x00\x18\x00\x01\x00\x03\x00\x00\x02\x00" // ................ |
// /* 0030 */ "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x1c" // ................ |
// /* 0040 */ "\x00\x18\x00\x01\x00\x01\x00\x01\x00\xa0\x05\x84\x03\x00\x00\x01" // ................ |
// /* 0050 */ "\x00\x01\x00\x00\x08\x01\x00\x00\x00\x0e\x00\x08\x00\x01\x00\x00" // ................ |
// /* 0060 */ "\x00\x03\x00\x58\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ...X............ |
// /* 0070 */ "\x00\x00\x00\x00\x00\x40\x42\x0f\x00\x01\x00\x14\x00\x00\x00\x01" // .....@B......... |
// /* 0080 */ "\x00\x2f\x00\x22\x00\x01\x01\x01\x01\x00\x00\x00\x00\x01\x01\x01" // ./."............ |
// /* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x01\x00" // ................ |
// /* 00a0 */ "\x01\x00\x00\x00\x00\xa1\x06\x00\x00\x40\x42\x0f\x00\x40\x42\x0f" // .........@B..@B. |
// /* 00b0 */ "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x08\x00\x0a\x00\x01\x00\x19" // ................ |
// /* 00c0 */ "\x00\x19\x00\x0d\x00\x58\x00\x29\x00\x00\x00\x00\x00\x00\x00\x00" // .....X.)........ |
// /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x14\x00\x0c\x00\x00" // ................ |
// /* 0120 */ "\x00\x00\x00\x40\x06\x00\x00\x09\x00\x08\x00\xe9\x03\xe2\xb5\x0a" // ...@............ |
// /* 0130 */ "\x00\x08\x00\x06\x00\x00\x00\x00\x00\x00\x00"                     // ........... |
// Sent dumped on RDP Client (4) 315 bytes |
// Front::send_demand_active: done |
// Front::incoming: ACTIVATED (new license request) |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 479 bytes |
/* 0000 */ "\x03\x00\x01\xdf\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\xd0\xd0" // .......d....p... |
/* 0010 */ "\x01\x13\x00\xe9\x03\x02\x00\x01\x00\xe9\x03\x06\x00\xba\x01\x4d" // ...............M |
/* 0020 */ "\x53\x54\x53\x43\x00\x12\x00\x00\x00\x01\x00\x18\x00\x01\x00\x03" // STSC............ |
/* 0030 */ "\x00\x00\x02\x00\x00\x00\x00\x0d\x04\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0040 */ "\x00\x02\x00\x1c\x00\x20\x00\x01\x00\x01\x00\x01\x00\xa0\x05\x84" // ..... .......... |
/* 0050 */ "\x03\x00\x00\x01\x00\x01\x00\x00\x1a\x01\x00\x00\x00\x03\x00\x58" // ...............X |
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0070 */ "\x00\x00\x00\x00\x00\x01\x00\x14\x00\x00\x00\x01\x00\x00\x00\xaa" // ................ |
/* 0080 */ "\x00\x01\x01\x01\x01\x01\x00\x00\x01\x01\x01\x00\x01\x00\x00\x00" // ................ |
/* 0090 */ "\x01\x01\x01\x01\x01\x01\x01\x01\x00\x01\x01\x01\x00\x00\x00\x00" // ................ |
/* 00a0 */ "\x00\xa1\x06\x06\x00\x00\x00\x00\x00\x00\x84\x03\x00\x00\x00\x00" // ................ |
/* 00b0 */ "\x00\xe4\x04\x00\x00\x04\x00\x28\x00\x00\x00\x00\x00\x00\x00\x00" // .......(........ |
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 00d0 */ "\x00\x78\x00\x00\x03\x78\x00\x00\x0c\x51\x01\x00\x30\x0a\x00\x08" // .x...x...Q..0... |
/* 00e0 */ "\x00\x06\x00\x00\x00\x07\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 00f0 */ "\x00\x05\x00\x0c\x00\x00\x00\x00\x00\x02\x00\x02\x00\x08\x00\x0a" // ................ |
/* 0100 */ "\x00\x01\x00\x14\x00\x15\x00\x09\x00\x08\x00\x00\x00\x00\x00\x0d" // ................ |
/* 0110 */ "\x00\x58\x00\x91\x00\x20\x00\x0c\x04\x00\x00\x04\x00\x00\x00\x00" // .X... .......... |
/* 0120 */ "\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x08\x00\x01\x00\x00\x00\x0e" // ................ |
/* 0170 */ "\x00\x08\x00\x01\x00\x00\x00\x10\x00\x34\x00\xfe\x00\x04\x00\xfe" // .........4...... |
/* 0180 */ "\x00\x04\x00\xfe\x00\x08\x00\xfe\x00\x08\x00\xfe\x00\x10\x00\xfe" // ................ |
/* 0190 */ "\x00\x20\x00\xfe\x00\x40\x00\xfe\x00\x80\x00\xfe\x00\x00\x01\x40" // . ...@.........@ |
/* 01a0 */ "\x00\x00\x08\x00\x01\x00\x01\x03\x00\x00\x00\x0f\x00\x08\x00\x01" // ................ |
/* 01b0 */ "\x00\x00\x00\x11\x00\x0c\x00\x01\x00\x00\x00\x00\x28\x64\x00\x14" // ............(d.. |
/* 01c0 */ "\x00\x0c\x00\x01\x00\x00\x00\x00\x00\x00\x00\x15\x00\x0c\x00\x02" // ................ |
/* 01d0 */ "\x00\x00\x00\x00\x0a\x00\x01\x1a\x00\x08\x00\x00\x00\x00\x00"     // ............... |
// Dump done on RDP Client (4) 479 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=0 |
// Front::incoming: Received CONFIRMACTIVEPDU |
// Front::process_confirm_active |
// Front::process_confirm_active: lengthSourceDescriptor = 6 |
// Front::process_confirm_active: lengthCombinedCapabilities = 442 |
// Front::capability 0 / 18 |
// Front::process_confirm_active: Receiving from client General caps (24 bytes) |
//      General caps::major 1 |
//      General caps::minor 3 |
//      General caps::protocol 512 |
//      General caps::pad2octetA 0 |
//      General caps::compression type 0 |
//      General caps::extra flags 40d |
//      General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED yes |
//      General caps::extraflags:LONG_CREDENTIALS_SUPPORTED yes |
//      General caps::extraflags:AUTORECONNECT_SUPPORTED yes |
//      General caps::extraflags:ENC_SALTED_CHECKSUM no |
//      General caps::extraflags:NO_BITMAP_COMPRESSION_HDR yes |
//      General caps::updateCapability 0 |
//      General caps::remoteUnshare 0 |
//      General caps::compressionLevel 0 |
//      General caps::refreshRectSupport 0 |
//      General caps::suppressOutputSupport 0 |
// Front::capability 1 / 18 |
// Front::process_confirm_active: Receiving from client Bitmap caps (28 bytes) |
//      Bitmap caps::preferredBitsPerPixel 32 |
//      Bitmap caps::receive1BitPerPixel 1 |
//      Bitmap caps::receive4BitsPerPixel 1 |
//      Bitmap caps::receive8BitsPerPixel 1 |
//      Bitmap caps::desktopWidth 1440 |
//      Bitmap caps::desktopHeight 900 |
//      Bitmap caps::pad2octets 0 |
//      Bitmap caps::desktopResizeFlag 1 (yes) |
//      Bitmap caps::bitmapCompressionFlag 1 yes |
//      Bitmap caps::highColorFlags 0 |
//      Bitmap caps::drawingFlags 26 |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY yes |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_COLOR_SUBSAMPLING no |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_SKIP_ALPHA yes |
//      Bitmap caps::multipleRectangleSupport 1 |
//      Bitmap caps::pad2octetsB 0 |
// Front::capability 2 / 18 |
// Front::process_confirm_active: Receiving from client Order caps (88 bytes) |
//      Order caps::terminalDescriptor 0 |
//      Order caps::pad4octetsA 0 |
//      Order caps::desktopSaveXGranularity 1 |
//      Order caps::desktopSaveYGranularity 20 |
//      Order caps::pad2octetsA 0 |
//      Order caps::maximumOrderLevel 1 |
//      Order caps::numberFonts 0 |
//      Order caps::orderFlags 0xAA |
//      Order caps::orderSupport[DSTBLT] 1 |
//      Order caps::orderSupport[PATBLT] 1 |
//      Order caps::orderSupport[SCRBLT] 1 |
//      Order caps::orderSupport[MEMBLT] 1 |
//      Order caps::orderSupport[MEM3BLT] 1 |
//      Order caps::orderSupport[ATEXTOUT] 0 |
//      Order caps::orderSupport[AEXTTEXTOUT] 0 |
//      Order caps::orderSupport[DRAWNINEGRID] 1 |
//      Order caps::orderSupport[LINETO] 1 |
//      Order caps::orderSupport[MULTI_DRAWNINEGRID] 1 |
//      Order caps::orderSupport[OPAQUERECT] 0 |
//      Order caps::orderSupport[SAVEBITMAP] 1 |
//      Order caps::orderSupport[WTEXTOUT] 0 |
//      Order caps::orderSupport[MEMBLT_V2] 0 |
//      Order caps::orderSupport[MEM3BLT_V2] 0 |
//      Order caps::orderSupport[MULTIDSTBLT] 1 |
//      Order caps::orderSupport[MULTIPATBLT] 1 |
//      Order caps::orderSupport[MULTISCRBLT] 1 |
//      Order caps::orderSupport[MULTIOPAQUERECT] 1 |
//      Order caps::orderSupport[FAST_INDEX] 1 |
//      Order caps::orderSupport[POLYGON_SC] 1 |
//      Order caps::orderSupport[POLYGON_CB] 1 |
//      Order caps::orderSupport[POLYLINE] 1 |
//      Order caps::orderSupport[UnusedIndex7] 0 |
//      Order caps::orderSupport[FAST_GLYPH] 1 |
//      Order caps::orderSupport[ELLIPSE_SC] 1 |
//      Order caps::orderSupport[ELLIPSE_CB] 1 |
//      Order caps::orderSupport[GLYPH] 0 |
//      Order caps::orderSupport[GLYPH_WEXTTEXTOUT] 0 |
//      Order caps::orderSupport[GLYPH_WLONGTEXTOUT] 0 |
//      Order caps::orderSupport[GLYPH_WLONGEXTTEXTOUT] 0 |
//      Order caps::orderSupport[UnusedIndex11] 0 |
//      Order caps::textFlags 1697 |
//      Order caps::orderSupportExFlags 0x6 |
//      Order caps::pad4octetsB 0 |
//      Order caps::desktopSaveSize 230400 |
//      Order caps::pad2octetsC 0 |
//      Order caps::pad2octetsD 0 |
//      Order caps::textANSICodePage 1252 |
//      Order caps::pad2octetsE 0 |
// Front::capability 3 / 18 |
// Front::process_confirm_active: Receiving from client BitmapCache caps (40 bytes) |
//      BitmapCache caps::pad1 0 |
//      BitmapCache caps::pad2 0 |
//      BitmapCache caps::pad3 0 |
//      BitmapCache caps::pad4 0 |
//      BitmapCache caps::pad5 0 |
//      BitmapCache caps::pad6 0 |
//      BitmapCache caps::cache0Entries 120 |
//      BitmapCache caps::cache0MaximumCellSize 768 |
//      BitmapCache caps::cache1Entries 120 |
//      BitmapCache caps::cache1MaximumCellSize 3072 |
//      BitmapCache caps::cache2Entries 337 |
//      BitmapCache caps::cache2MaximumCellSize 12288 |
// Front::capability 4 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_COLORCACHE |
// Front::capability 5 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_ACTIVATION |
// Front::capability 6 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_CONTROL |
// Front::capability 7 / 18 |
// Front::process_confirm_active: Receiving from client Pointer caps (10 bytes) |
//      Pointer caps::colorPointerFlag 1 |
//      Pointer caps::colorPointerCacheSize 20 |
//      Pointer caps::pointerCacheSize 21 |
// Front::capability 8 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_SHARE |
// Front::capability 9 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_INPUT |
// Front::capability 10 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_SOUND |
// Front::capability 11 / 18 |
// Front::capability 12 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_GLYPHCACHE |
// Front::process_confirm_active: Receiving from client GlyphCache caps (52 bytes) |
//      GlyphCache caps::GlyphCache[0].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[0].CacheMaximumCellSize=4 |
//      GlyphCache caps::GlyphCache[1].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[1].CacheMaximumCellSize=4 |
//      GlyphCache caps::GlyphCache[2].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[2].CacheMaximumCellSize=8 |
//      GlyphCache caps::GlyphCache[3].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[3].CacheMaximumCellSize=8 |
//      GlyphCache caps::GlyphCache[4].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[4].CacheMaximumCellSize=16 |
//      GlyphCache caps::GlyphCache[5].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[5].CacheMaximumCellSize=32 |
//      GlyphCache caps::GlyphCache[6].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[6].CacheMaximumCellSize=64 |
//      GlyphCache caps::GlyphCache[7].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[7].CacheMaximumCellSize=128 |
//      GlyphCache caps::GlyphCache[8].CacheEntries=254 |
//      GlyphCache caps::GlyphCache[8].CacheMaximumCellSize=256 |
//      GlyphCache caps::GlyphCache[9].CacheEntries=64 |
//      GlyphCache caps::GlyphCache[9].CacheMaximumCellSize=2048 |
//      GlyphCache caps::FragCache 16777472 |
//      GlyphCache caps::GlyphSupportLevel 3 |
//      GlyphCache caps::pad2octets 0 |
// Front::capability 13 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_BRUSH |
// Front::process_confirm_active: Receiving from client BrushCache caps (8 bytes) |
//      BrushCacheCaps caps::brushSupportLevel 1 |
// Front::capability 14 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_OFFSCREENCACHE |
// Front::process_confirm_active: Receiving from client OffScreenCache caps (12 bytes) |
//      OffScreenCache caps::offscreenSupportLevel 1 |
//      OffScreenCache caps::offscreenCacheSize 10240 |
//      OffScreenCache caps::offscreenCacheEntries 100 |
// Front::capability 15 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_VIRTUALCHANNEL |
// Front::capability 16 / 18 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_DRAWNINEGRIDCACHE |
// Front::capability 17 / 18 |
// Front::process_confirm_active: Receiving from client MultifragmentUpdate caps (8 bytes) |
//      MultifragmentUpdate caps::MaxRequestSize 0 |
// Front::process_confirm_active: done p=0x7f95c8eeec85 end=0x7f95c8eeec85 |
// Front::reset: use_bitmap_comp=1 |
// Front::reset: use_compact_packets=1 |
// Front::reset: bitmap_cache_version=0 |
// Front::reset: Use RDP 5.0 Bulk compression |
// Front::incoming: Received CONFIRMACTIVEPDU done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 36 bytes |
/* 0000 */ "\x03\x00\x00\x24\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x16\x16\x00" // ...$...d....p... |
/* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x01\x08\x00\x1f\x00\x00\x00" // ................ |
/* 0020 */ "\x01\x00\xe9\x03"                                                 // .... |
// Dump done on RDP Client (4) 36 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=0 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=31 sdata_in.len=8 sdata_in.compressedLen=0 remains=0 payload_len=4 |
// Front::process_data: PDUTYPE2_SYNCHRONIZE |
// Front::process_data: PDUTYPE2_SYNCHRONIZE messageType=1 controlId=1001 |
// Front::send_synchronize |
// Sec clear payload to send: |
// /* 0000 */ 0x16, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x16, 0x00, 0x1f, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x01, 0x00, 0xea, 0x03,                                                              // ...... |
// Sending on RDP Client (4) 36 bytes |
// /* 0000 */ "\x03\x00\x00\x24\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x16\x16\x00" // ...$...h....p... |
// /* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x16\x00\x1f\x00\x00\x00" // ................ |
// /* 0020 */ "\x01\x00\xea\x03"                                                 // .... |
// Sent dumped on RDP Client (4) 36 bytes |
// Front::send_synchronize: done |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 40 bytes |
/* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00" // ...(...d....p... |
/* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x01\x0c\x00\x14\x00\x00\x00" // ................ |
/* 0020 */ "\x04\x00\x00\x00\x00\x00\x00\x00"                                 // ........ |
// Dump done on RDP Client (4) 40 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=0 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=20 sdata_in.len=12 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// Front::process_data: PDUTYPE2_CONTROL |
// Front::send_control: action=4 |
// Sec clear payload to send: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x1a, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00,                                      // .......... |
// Sending on RDP Client (4) 40 bytes |
// /* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x1a\x1a\x00" // ...(...h....p... |
// /* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00" // ................ |
// /* 0020 */ "\x04\x00\x00\x00\xea\x03\x00\x00"                                 // ........ |
// Sent dumped on RDP Client (4) 40 bytes |
// Front::send_control: done. action=4 |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 40 bytes |
/* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00" // ...(...d....p... |
/* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x01\x0c\x00\x14\x00\x00\x00" // ................ |
/* 0020 */ "\x01\x00\x00\x00\x00\x00\x00\x00"                                 // ........ |
// Dump done on RDP Client (4) 40 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=0 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=20 sdata_in.len=12 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// Front::process_data: PDUTYPE2_CONTROL |
// Front::send_control: action=2 |
// Sec clear payload to send: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x1a, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00,                                      // .......... |
// Sending on RDP Client (4) 40 bytes |
// /* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x1a\x1a\x00" // ...(...h....p... |
// /* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00" // ................ |
// /* 0020 */ "\x02\x00\x00\x00\xea\x03\x00\x00"                                 // ........ |
// Sent dumped on RDP Client (4) 40 bytes |
// Front::send_control: done. action=2 |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 30 bytes |
/* 0000 */ "\x30\x1e\x01\x0f\x62\x01\x2a\x01\x36\x01\x1d\x03\x1d\x01\x0f\x01" // 0...b.*.6....... |
/* 0010 */ "\x38\x01\x0f\x03\x38\x01\x0f\x20\x00\x08\xc4\x03\xde\x01"         // 8...8.. ...... |
// Dump done on RDP Client (4) 30 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0xF |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, sync eventFlags=0x2 |
// Front::incoming: (Fast-Path) Synchronize Event toggleFlags=0x2 |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0x2A |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0x36 |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0x1D |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x3 SPKeyboardFlags=0x8100, keyCode=0x1D |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0xF |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0x38 |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0xF |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x3 SPKeyboardFlags=0x8100, keyCode=0x38 |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, scancode eventCode=0x1 SPKeyboardFlags=0x8000, keyCode=0xF |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming: Received Fast-Path PDU, mouse pointerFlags=0x800, xPos=0x3C4, yPos=0x1DE |
// Front::incoming: Received Fast-Path PUD done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 40 bytes |
/* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00" // ...(...d....p... |
/* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x01\x00\x00\x27\x00\x00\x00" // ............'... |
/* 0020 */ "\x00\x00\x00\x00\x03\x00\x32\x00"                                 // ......2. |
} /* end indata */;
