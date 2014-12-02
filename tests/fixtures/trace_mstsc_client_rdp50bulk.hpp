const char outdata[] =
{
// Listen: binding socket 4 on 0.0.0.0:3389 |
// Listen: listening on socket 4 |
// Incoming socket to 5 (ip=10.10.47.175) |
// Reading font file ./tests/fixtures/sans-10.fv1 |
// font name <Bitstream Vera Sans> size <10> |
// Font file ./tests/fixtures/sans-10.fv1 defines glyphs up to 256 |
// Front::incoming |
// Front::incoming:CONNECTION_INITIATION |
// Front::incoming::receiving x224 request PDU |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x00\x13"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 15 bytes |
// Recv done on RDP Client (5) 15 bytes |
// /* 0000 */ "\x0e\xe0\x00\x00\x00\x00\x00\x01\x00\x08\x00\x01\x00\x00\x00"     //............... |
// Dump done on RDP Client (5) 15 bytes |
// CR Recv: PROTOCOL TLS 1.0 |
// Front::incoming::sending x224 connection confirm PDU |
// -----------------> Front::TLS Support Enabled |
// CC Send: PROTOCOL TLS 1.0 |
// Sending on RDP Client (5) 19 bytes |
/* 0000 */ "\x03\x00\x00\x13\x0e\xd0\x00\x00\x00\x00\x00\x02\x01\x08\x00\x01" //................ |
/* 0010 */ "\x00\x00\x00"                                                     //... |
// Sent dumped on RDP Client (5) 19 bytes |
// RIO *::enable_server_tls() start |
// RIO *::SSL_CTX_set_options() |
// RIO *::enable_server_tls() done |
// Front::incoming::Basic Settings Exchange |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x01\xac"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 424 bytes |
// Recv done on RDP Client (5) 424 bytes |
// /* 0000 */ "\x02\xf0\x80\x7f\x65\x82\x01\xa0\x04\x01\x01\x04\x01\x01\x01\x01" //....e........... |
// /* 0010 */ "\xff\x30\x19\x02\x01\x22\x02\x01\x02\x02\x01\x00\x02\x01\x01\x02" //.0...".......... |
// /* 0020 */ "\x01\x00\x02\x01\x01\x02\x02\xff\xff\x02\x01\x02\x30\x19\x02\x01" //............0... |
// /* 0030 */ "\x01\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02\x01\x00\x02\x01\x01" //................ |
// /* 0040 */ "\x02\x02\x04\x20\x02\x01\x02\x30\x1c\x02\x02\xff\xff\x02\x02\xfc" //... ...0........ |
// /* 0050 */ "\x17\x02\x02\xff\xff\x02\x01\x01\x02\x01\x00\x02\x01\x01\x02\x02" //................ |
// /* 0060 */ "\xff\xff\x02\x01\x02\x04\x82\x01\x3f\x00\x05\x00\x14\x7c\x00\x01" //........?....|.. |
// /* 0070 */ "\x81\x36\x00\x08\x00\x10\x00\x01\xc0\x00\x44\x75\x63\x61\x81\x28" //.6........Duca.( |
// /* 0080 */ "\x01\xc0\xd8\x00\x04\x00\x08\x00\x00\x04\x00\x03\x01\xca\x03\xaa" //................ |
// /* 0090 */ "\x0c\x04\x00\x00\xb0\x1d\x00\x00\x52\x00\x44\x00\x50\x00\x2d\x00" //........R.D.P.-. |
// /* 00a0 */ "\x54\x00\x45\x00\x53\x00\x54\x00\x00\x00\x00\x00\x00\x00\x00\x00" //T.E.S.T......... |
// /* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00c0 */ "\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0100 */ "\x00\x00\x00\x00\x01\xca\x01\x00\x00\x00\x00\x00\x10\x00\x0f\x00" //................ |
// /* 0110 */ "\x2d\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //-............... |
// /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0150 */ "\x00\x00\x02\x00\x01\x00\x00\x00\x04\xc0\x0c\x00\x11\x00\x00\x00" //................ |
// /* 0160 */ "\x00\x00\x00\x00\x02\xc0\x0c\x00\x1b\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0170 */ "\x03\xc0\x38\x00\x04\x00\x00\x00\x72\x64\x70\x64\x72\x00\x00\x00" //..8.....rdpdr... |
// /* 0180 */ "\x00\x00\x80\x80\x72\x64\x70\x73\x6e\x64\x00\x00\x00\x00\x00\xc0" //....rdpsnd...... |
// /* 0190 */ "\x64\x72\x64\x79\x6e\x76\x63\x00\x00\x00\x80\xc0\x63\x6c\x69\x70" //drdynvc.....clip |
// /* 01a0 */ "\x72\x64\x72\x00\x00\x00\xa0\xc0"                                 //rdr..... |
// Dump done on RDP Client (5) 424 bytes |
// GCC::UserData tag=c001 length=216 |
// Received from Client GCC User Data CS_CORE (216 bytes) |
// cs_core::version [80004] RDP 5.0, 5.1, 5.2, and 6.0 clients) |
// cs_core::desktopWidth  = 1024 |
// cs_core::desktopHeight = 768 |
// cs_core::colorDepth    = [ca01] [RNS_UD_COLOR_8BPP] superseded by postBeta2ColorDepth |
// cs_core::SASSequence   = [aa03] [Unknown] |
// cs_core::keyboardLayout= 040c |
// cs_core::clientBuild   = 7600 |
// cs_core::clientName    = RDP-TEST |
// cs_core::keyboardType  = [0004] IBM enhanced (101-key or 102-key) keyboard |
// cs_core::keyboardSubType      = [0000] OEM code |
// cs_core::keyboardFunctionKey  = 12 function keys |
// cs_core::imeFileName    =  |
// cs_core::postBeta2ColorDepth  = [ca01] [8 bpp] |
// cs_core::clientProductId = 1 |
// cs_core::serialNumber = 0 |
// cs_core::highColorDepth  = [0010] [16-bit 565 RGB mask] |
// cs_core::supportedColorDepths  = [000f] [24/16/15/32] |
// cs_core::earlyCapabilityFlags  = [002d] |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_ERRINFO_PDU |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_STATUSINFO_PDU |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_STRONG_ASYMMETRIC_KEYS |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_VALID_CONNECTION_TYPE |
// cs_core::clientDigProductId=[00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 |
// cs_core::connectionType  = 2 |
// cs_core::pad1octet  = 0 |
// cs_core::serverSelectedProtocol = 1 |
// GCC::UserData tag=c004 length=12 |
// Receiving from Client GCC User Data CS_CLUSTER (12 bytes) |
// cs_cluster::flags [0011] |
// cs_cluster::redirectedSessionID = 0 |
// GCC::UserData tag=c002 length=12 |
// Received from Client GCC User Data CS_SECURITY (12 bytes) |
// CSSecGccUserData::encryptionMethods 27 |
// CSSecGccUserData::extEncryptionMethods 0 |
// GCC::UserData tag=c003 length=56 |
// Received from Client GCC User Data CS_NET (56 bytes) |
// cs_net::channelCount   = 4 |
// cs_net::channel '   rdpdr' [1004] INITIALIZED COMPRESS_RDP |
// cs_net::channel '  rdpsnd' [1005] INITIALIZED |
// cs_net::channel ' drdynvc' [1006] INITIALIZED COMPRESS_RDP |
// cs_net::channel ' cliprdr' [1007] INITIALIZED COMPRESS_RDP SHOW_PROTOCOL |
// Sending to client GCC User Data SC_CORE (12 bytes) |
// sc_core::version [80004] RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 servers) |
// sc_core::clientRequestedProtocols  = 1 |
// Sending to client GCC User Data SC_NET (16 bytes) |
// sc_net::MCSChannelId   = 1003 |
// sc_net::channelCount   = 4 |
// sc_net::channel[1004]::id = 1004 |
// sc_net::channel[1005]::id = 1005 |
// sc_net::channel[1006]::id = 1006 |
// sc_net::channel[1007]::id = 1007 |
// Sending to client GCC User Data SC_SECURITY (12 bytes) |
// sc_security::encryptionMethod = 0 |
// sc_security::encryptionLevel  = 0 |
// Sending on RDP Client (5) 109 bytes |
/* 0000 */ "\x03\x00\x00\x6d\x02\xf0\x80\x7f\x66\x63\x0a\x01\x00\x02\x01\x00" //...m....fc...... |
/* 0010 */ "\x30\x1a\x02\x01\x22\x02\x01\x03\x02\x01\x00\x02\x01\x01\x02\x01" //0..."........... |
/* 0020 */ "\x00\x02\x01\x01\x02\x03\x00\xff\xf8\x02\x01\x02\x04\x3f\x00\x05" //.............?.. |
/* 0030 */ "\x00\x14\x7c\x00\x01\x2a\x14\x76\x0a\x01\x01\x00\x01\xc0\x00\x4d" //..|..*.v.......M |
/* 0040 */ "\x63\x44\x6e\x80\x28\x01\x0c\x0c\x00\x04\x00\x08\x00\x01\x00\x00" //cDn.(........... |
/* 0050 */ "\x00\x03\x0c\x10\x00\xeb\x03\x04\x00\xec\x03\xed\x03\xee\x03\xef" //................ |
/* 0060 */ "\x03\x02\x0c\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00"             //............. |
// Sent dumped on RDP Client (5) 109 bytes |
// Front::incoming::Channel Connection |
// Front::incoming::Recv MCS::ErectDomainRequest |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x00\x0c"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 8 bytes |
// Recv done on RDP Client (5) 8 bytes |
// /* 0000 */ "\x02\xf0\x80\x04\x01\x00\x01\x00"                                 //........ |
// Dump done on RDP Client (5) 8 bytes |
// Front::incoming::Recv MCS::AttachUserRequest |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x00\x08"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 4 bytes |
// Recv done on RDP Client (5) 4 bytes |
// /* 0000 */ "\x02\xf0\x80\x28"                                                 //...( |
// Dump done on RDP Client (5) 4 bytes |
// Front::incoming::Send MCS::AttachUserConfirm |
// Sending on RDP Client (5) 11 bytes |
/* 0000 */ "\x03\x00\x00\x0b\x02\xf0\x80\x2e\x00\x00\x00"                     //........... |
// Sent dumped on RDP Client (5) 11 bytes |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x00\x0c"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 8 bytes |
// Recv done on RDP Client (5) 8 bytes |
// /* 0000 */ "\x02\xf0\x80\x38\x00\x00\x03\xe9"                                 //...8.... |
// Dump done on RDP Client (5) 8 bytes |
// Sending on RDP Client (5) 15 bytes |
/* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xe9\x03\xe9"     //.......>....... |
// Sent dumped on RDP Client (5) 15 bytes |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x00\x0c"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 8 bytes |
// Recv done on RDP Client (5) 8 bytes |
// /* 0000 */ "\x02\xf0\x80\x38\x00\x00\x03\xeb"                                 //...8.... |
// Dump done on RDP Client (5) 8 bytes |
// Sending on RDP Client (5) 15 bytes |
/* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xeb\x03\xeb"     //.......>....... |
// Sent dumped on RDP Client (5) 15 bytes |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x00\x0c"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 8 bytes |
// Recv done on RDP Client (5) 8 bytes |
// /* 0000 */ "\x02\xf0\x80\x38\x00\x00\x03\xec"                                 //...8.... |
// Dump done on RDP Client (5) 8 bytes |
// cjrq[0] = 1004 -> cjcf |
// Sending on RDP Client (5) 15 bytes |
/* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xec\x03\xec"     //.......>....... |
// Sent dumped on RDP Client (5) 15 bytes |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x00\x0c"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 8 bytes |
// Recv done on RDP Client (5) 8 bytes |
// /* 0000 */ "\x02\xf0\x80\x38\x00\x00\x03\xed"                                 //...8.... |
// Dump done on RDP Client (5) 8 bytes |
// cjrq[1] = 1005 -> cjcf |
// Sending on RDP Client (5) 15 bytes |
/* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xed\x03\xed"     //.......>....... |
// Sent dumped on RDP Client (5) 15 bytes |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x00\x0c"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 8 bytes |
// Recv done on RDP Client (5) 8 bytes |
// /* 0000 */ "\x02\xf0\x80\x38\x00\x00\x03\xee"                                 //...8.... |
// Dump done on RDP Client (5) 8 bytes |
// cjrq[2] = 1006 -> cjcf |
// Sending on RDP Client (5) 15 bytes |
/* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xee\x03\xee"     //.......>....... |
// Sent dumped on RDP Client (5) 15 bytes |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x00\x0c"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 8 bytes |
// Recv done on RDP Client (5) 8 bytes |
// /* 0000 */ "\x02\xf0\x80\x38\x00\x00\x03\xef"                                 //...8.... |
// Dump done on RDP Client (5) 8 bytes |
// cjrq[3] = 1007 -> cjcf |
// Sending on RDP Client (5) 15 bytes |
/* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xef\x03\xef"     //.......>....... |
// Sent dumped on RDP Client (5) 15 bytes |
// Front::incoming::RDP Security Commencement |
// TLS mode: exchange packet disabled |
// Front::incoming |
// Front::incoming::Secure Settings Exchange |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x01\x4b"                                                     //..K |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 327 bytes |
// Recv done on RDP Client (5) 327 bytes |
// /* 0000 */ "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\x3c\x40\x00\x00\x00\x00" //...d....p.<@.... |
// /* 0010 */ "\x00\x00\x00\xb3\x47\x0b\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00" //....G........... |
// /* 0020 */ "\x00\x00\x00\x78\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x1a" //...x............ |
// /* 0030 */ "\x00\x31\x00\x30\x00\x2e\x00\x31\x00\x30\x00\x2e\x00\x34\x00\x37" //.1.0...1.0...4.7 |
// /* 0040 */ "\x00\x2e\x00\x31\x00\x37\x00\x35\x00\x00\x00\x40\x00\x43\x00\x3a" //...1.7.5...@.C.: |
// /* 0050 */ "\x00\x5c\x00\x57\x00\x49\x00\x4e\x00\x44\x00\x4f\x00\x57\x00\x53" //...W.I.N.D.O.W.S |
// /* 0060 */ "\x00\x5c\x00\x73\x00\x79\x00\x73\x00\x74\x00\x65\x00\x6d\x00\x33" //...s.y.s.t.e.m.3 |
// /* 0070 */ "\x00\x32\x00\x5c\x00\x6d\x00\x73\x00\x74\x00\x73\x00\x63\x00\x61" //.2...m.s.t.s.c.a |
// /* 0080 */ "\x00\x78\x00\x2e\x00\x64\x00\x6c\x00\x6c\x00\x00\x00\xc4\xff\xff" //.x...d.l.l...... |
// /* 0090 */ "\xff\x50\x00\x61\x00\x72\x00\x69\x00\x73\x00\x2c\x00\x20\x00\x4d" //.P.a.r.i.s.,. .M |
// /* 00a0 */ "\x00\x61\x00\x64\x00\x72\x00\x69\x00\x64\x00\x00\x00\x00\x00\x00" //.a.d.r.i.d...... |
// /* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00d0 */ "\x00\x00\x00\x0a\x00\x00\x00\x05\x00\x03\x00\x00\x00\x00\x00\x00" //................ |
// /* 00e0 */ "\x00\x00\x00\x00\x00\x50\x00\x61\x00\x72\x00\x69\x00\x73\x00\x2c" //.....P.a.r.i.s., |
// /* 00f0 */ "\x00\x20\x00\x4d\x00\x61\x00\x64\x00\x72\x00\x69\x00\x64\x00\x00" //. .M.a.d.r.i.d.. |
// /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x05\x00\x02\x00\x00" //................ |
// /* 0130 */ "\x00\x00\x00\x00\x00\xc4\xff\xff\xff\x00\x00\x00\x00\x07\x00\x00" //................ |
// /* 0140 */ "\x00\x00\x00\x64\x00\x00\x00"                                     //...d... |
// Dump done on RDP Client (5) 327 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x00, 0x00, 0x00, 0x00, 0xb3, 0x47, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,  // .....G.......... |
// /* 0010 */ 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,  // ....x........... |
// /* 0020 */ 0x1a, 0x00, 0x31, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x34, 0x00,  // ..1.0...1.0...4. |
// /* 0030 */ 0x37, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x37, 0x00, 0x35, 0x00, 0x00, 0x00, 0x40, 0x00, 0x43, 0x00,  // 7...1.7.5...@.C. |
// /* 0040 */ 0x3a, 0x00, 0x5c, 0x00, 0x57, 0x00, 0x49, 0x00, 0x4e, 0x00, 0x44, 0x00, 0x4f, 0x00, 0x57, 0x00,  // :...W.I.N.D.O.W. |
// /* 0050 */ 0x53, 0x00, 0x5c, 0x00, 0x73, 0x00, 0x79, 0x00, 0x73, 0x00, 0x74, 0x00, 0x65, 0x00, 0x6d, 0x00,  // S...s.y.s.t.e.m. |
// /* 0060 */ 0x33, 0x00, 0x32, 0x00, 0x5c, 0x00, 0x6d, 0x00, 0x73, 0x00, 0x74, 0x00, 0x73, 0x00, 0x63, 0x00,  // 3.2...m.s.t.s.c. |
// /* 0070 */ 0x61, 0x00, 0x78, 0x00, 0x2e, 0x00, 0x64, 0x00, 0x6c, 0x00, 0x6c, 0x00, 0x00, 0x00, 0xc4, 0xff,  // a.x...d.l.l..... |
// /* 0080 */ 0xff, 0xff, 0x50, 0x00, 0x61, 0x00, 0x72, 0x00, 0x69, 0x00, 0x73, 0x00, 0x2c, 0x00, 0x20, 0x00,  // ..P.a.r.i.s.,. . |
// /* 0090 */ 0x4d, 0x00, 0x61, 0x00, 0x64, 0x00, 0x72, 0x00, 0x69, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00,  // M.a.d.r.i.d..... |
// /* 00a0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00b0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00c0 */ 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00d0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x00, 0x61, 0x00, 0x72, 0x00, 0x69, 0x00, 0x73, 0x00,  // ......P.a.r.i.s. |
// /* 00e0 */ 0x2c, 0x00, 0x20, 0x00, 0x4d, 0x00, 0x61, 0x00, 0x64, 0x00, 0x72, 0x00, 0x69, 0x00, 0x64, 0x00,  // ,. .M.a.d.r.i.d. |
// /* 00f0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0100 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0110 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 0x00, 0x02, 0x00,  // ................ |
// /* 0120 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc4, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00,  // ................ |
// /* 0130 */ 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,                          // ....d... |
// RDP-5 Style logon |
// Receiving from client InfoPacket |
// InfoPacket::CodePage 0 |
// InfoPacket::flags 0xb47b3 |
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
// InfoPacket::flags:FORCE_ENCRYPTED_CS_PDU yes |
// InfoPacket::flags:INFO_RAIL no |
// InfoPacket::flags:INFO_LOGONERRORS yes |
// InfoPacket::flags:INFO_MOUSE_HAS_WHEEL yes |
// InfoPacket::flags:INFO_PASSWORD_IS_SC_PIN no |
// InfoPacket::flags:INFO_NOAUDIOPLAYBACK yes |
// InfoPacket::flags:INFO_USING_SAVED_CREDS no |
// InfoPacket::flags:RNS_INFO_AUDIOCAPTURE no |
// InfoPacket::flags:RNS_INFO_VIDEO_DISABLE no |
// InfoPacket::cbDomain 2 |
// InfoPacket::cbUserName 4 |
// InfoPacket::cbPassword 2 |
// InfoPacket::cbAlternateShell 2 |
// InfoPacket::cbWorkingDir 2 |
// InfoPacket::Domain  |
// InfoPacket::UserName x |
// InfoPacket::Password <hidden> |
// InfoPacket::AlternateShell  |
// InfoPacket::WorkingDir  |
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2 |
// InfoPacket::ExtendedInfoPacket::cbClientAddress 26 |
// InfoPacket::ExtendedInfoPacket::clientAddress 10.10.47.175 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 64 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\WINDOWS\system32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 0 |
// InfoPacket::ExtendedInfoPacket::performanceFlags 7 |
// InfoPacket::ExtendedInfoPacket::cbAutoReconnectLen 0 |
// InfoPacket::ExtendedInfoPacket::autoReconnectCookie  |
// InfoPacket::ExtendedInfoPacket::reserved1 100 |
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
// client info: performance flags before=0x00000007 after=0x00000007 default=0x00000000 present=0x00000000 not-present=0x00000000 |
// Front Keyboard Layout = 0x40c |
// Front::incoming::licencing not client_info.is_mce |
// Front::incoming::licencing send_lic_initial |
// Sec clear payload to send: |
// /* 0000 */ 0x01, 0x02, 0x3e, 0x01, 0x7b, 0x3c, 0x31, 0xa6, 0xae, 0xe8, 0x74, 0xf6, 0xb4, 0xa5, 0x03, 0x90,  // ..>.{<1...t..... |
// /* 0010 */ 0xe7, 0xc2, 0xc7, 0x39, 0xba, 0x53, 0x1c, 0x30, 0x54, 0x6e, 0x90, 0x05, 0xd0, 0x05, 0xce, 0x44,  // ...9.S.0Tn.....D |
// /* 0020 */ 0x18, 0x91, 0x83, 0x81, 0x00, 0x00, 0x04, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x4d, 0x00, 0x69, 0x00,  // ........,...M.i. |
// /* 0030 */ 0x63, 0x00, 0x72, 0x00, 0x6f, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x66, 0x00, 0x74, 0x00, 0x20, 0x00,  // c.r.o.s.o.f.t. . |
// /* 0040 */ 0x43, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x70, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x61, 0x00, 0x74, 0x00,  // C.o.r.p.o.r.a.t. |
// /* 0050 */ 0x69, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x32, 0x00, 0x33, 0x00,  // i.o.n.......2.3. |
// /* 0060 */ 0x36, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0xb8, 0x00,  // 6............... |
// /* 0070 */ 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x5c, 0x00,  // ................ |
// /* 0080 */ 0x52, 0x53, 0x41, 0x31, 0x48, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00,  // RSA1H.......?... |
// /* 0090 */ 0x01, 0x00, 0x01, 0x00, 0x01, 0xc7, 0xc9, 0xf7, 0x8e, 0x5a, 0x38, 0xe4, 0x29, 0xc3, 0x00, 0x95,  // .........Z8.)... |
// /* 00a0 */ 0x2d, 0xdd, 0x4c, 0x3e, 0x50, 0x45, 0x0b, 0x0d, 0x9e, 0x2a, 0x5d, 0x18, 0x63, 0x64, 0xc4, 0x2c,  // -.L>PE...*].cd., |
// /* 00b0 */ 0xf7, 0x8f, 0x29, 0xd5, 0x3f, 0xc5, 0x35, 0x22, 0x34, 0xff, 0xad, 0x3a, 0xe6, 0xe3, 0x95, 0x06,  // ..).?.5"4..:.... |
// /* 00c0 */ 0xae, 0x55, 0x82, 0xe3, 0xc8, 0xc7, 0xb4, 0xa8, 0x47, 0xc8, 0x50, 0x71, 0x74, 0x29, 0x53, 0x89,  // .U......G.Pqt)S. |
// /* 00d0 */ 0x6d, 0x9c, 0xed, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x48, 0x00,  // m..p..........H. |
// /* 00e0 */ 0xa8, 0xf4, 0x31, 0xb9, 0xab, 0x4b, 0xe6, 0xb4, 0xf4, 0x39, 0x89, 0xd6, 0xb1, 0xda, 0xf6, 0x1e,  // ..1..K...9...... |
// /* 00f0 */ 0xec, 0xb1, 0xf0, 0x54, 0x3b, 0x5e, 0x3e, 0x6a, 0x71, 0xb4, 0xf7, 0x75, 0xc8, 0x16, 0x2f, 0x24,  // ...T;^>jq..u../$ |
// /* 0100 */ 0x00, 0xde, 0xe9, 0x82, 0x99, 0x5f, 0x33, 0x0b, 0xa9, 0xa6, 0x94, 0xaf, 0xcb, 0x11, 0xc3, 0xf2,  // ....._3......... |
// /* 0110 */ 0xdb, 0x09, 0x42, 0x68, 0x29, 0x56, 0x58, 0x01, 0x56, 0xdb, 0x59, 0x03, 0x69, 0xdb, 0x7d, 0x37,  // ..Bh)VX.V.Y.i.}7 |
// /* 0120 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x0e, 0x00,  // ................ |
// /* 0130 */ 0x6d, 0x69, 0x63, 0x72, 0x6f, 0x73, 0x6f, 0x66, 0x74, 0x2e, 0x63, 0x6f, 0x6d, 0x00,        // microsoft.com. |
// Sending on RDP Client (5) 337 bytes |
/* 0000 */ "\x03\x00\x01\x51\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x81\x42\x80" //...Q...h....p.B. |
/* 0010 */ "\x00\x00\x00\x01\x02\x3e\x01\x7b\x3c\x31\xa6\xae\xe8\x74\xf6\xb4" //.....>.{<1...t.. |
/* 0020 */ "\xa5\x03\x90\xe7\xc2\xc7\x39\xba\x53\x1c\x30\x54\x6e\x90\x05\xd0" //......9.S.0Tn... |
/* 0030 */ "\x05\xce\x44\x18\x91\x83\x81\x00\x00\x04\x00\x2c\x00\x00\x00\x4d" //..D........,...M |
/* 0040 */ "\x00\x69\x00\x63\x00\x72\x00\x6f\x00\x73\x00\x6f\x00\x66\x00\x74" //.i.c.r.o.s.o.f.t |
/* 0050 */ "\x00\x20\x00\x43\x00\x6f\x00\x72\x00\x70\x00\x6f\x00\x72\x00\x61" //. .C.o.r.p.o.r.a |
/* 0060 */ "\x00\x74\x00\x69\x00\x6f\x00\x6e\x00\x00\x00\x08\x00\x00\x00\x32" //.t.i.o.n.......2 |
/* 0070 */ "\x00\x33\x00\x36\x00\x00\x00\x0d\x00\x04\x00\x01\x00\x00\x00\x03" //.3.6............ |
/* 0080 */ "\x00\xb8\x00\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x06" //................ |
/* 0090 */ "\x00\x5c\x00\x52\x53\x41\x31\x48\x00\x00\x00\x00\x02\x00\x00\x3f" //...RSA1H.......? |
/* 00a0 */ "\x00\x00\x00\x01\x00\x01\x00\x01\xc7\xc9\xf7\x8e\x5a\x38\xe4\x29" //............Z8.) |
/* 00b0 */ "\xc3\x00\x95\x2d\xdd\x4c\x3e\x50\x45\x0b\x0d\x9e\x2a\x5d\x18\x63" //...-.L>PE...*].c |
/* 00c0 */ "\x64\xc4\x2c\xf7\x8f\x29\xd5\x3f\xc5\x35\x22\x34\xff\xad\x3a\xe6" //d.,..).?.5"4..:. |
/* 00d0 */ "\xe3\x95\x06\xae\x55\x82\xe3\xc8\xc7\xb4\xa8\x47\xc8\x50\x71\x74" //....U......G.Pqt |
/* 00e0 */ "\x29\x53\x89\x6d\x9c\xed\x70\x00\x00\x00\x00\x00\x00\x00\x00\x08" //)S.m..p......... |
/* 00f0 */ "\x00\x48\x00\xa8\xf4\x31\xb9\xab\x4b\xe6\xb4\xf4\x39\x89\xd6\xb1" //.H...1..K...9... |
/* 0100 */ "\xda\xf6\x1e\xec\xb1\xf0\x54\x3b\x5e\x3e\x6a\x71\xb4\xf7\x75\xc8" //......T;^>jq..u. |
/* 0110 */ "\x16\x2f\x24\x00\xde\xe9\x82\x99\x5f\x33\x0b\xa9\xa6\x94\xaf\xcb" //./$....._3...... |
/* 0120 */ "\x11\xc3\xf2\xdb\x09\x42\x68\x29\x56\x58\x01\x56\xdb\x59\x03\x69" //.....Bh)VX.V.Y.i |
/* 0130 */ "\xdb\x7d\x37\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x0e" //.}7............. |
/* 0140 */ "\x00\x0e\x00\x6d\x69\x63\x72\x6f\x73\x6f\x66\x74\x2e\x63\x6f\x6d" //...microsoft.com |
/* 0150 */ "\x00"                                                             //. |
// Sent dumped on RDP Client (5) 337 bytes |
// Front::incoming::waiting for answer to lic_initial |
// Front::incoming |
// Front::incoming::WAITING_FOR_ANSWER_TO_LICENCE |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x00\xab"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 167 bytes |
// Recv done on RDP Client (5) 167 bytes |
// /* 0000 */ "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x9c\x80\x00\xf9\x1e\x13" //...d....p....... |
// /* 0010 */ "\x83\x98\x00\x01\x00\x00\x00\x00\x00\x01\x03\xb6\xbf\xf9\xc7\x44" //...............D |
// /* 0020 */ "\x71\x16\xea\x72\x2a\xeb\xc0\xb0\x5f\x60\x31\x04\x6c\x1c\x95\xae" //q..r*..._`1.l... |
// /* 0030 */ "\x1e\xda\xd1\xba\x94\x1a\xaa\xc3\xb2\x36\xac\x00\x00\x48\x00\x9f" //.........6...H.. |
// /* 0040 */ "\x09\xee\xb6\x56\x80\xb2\x43\xb2\xdd\x34\xed\x89\x53\x65\x55\x72" //...V..C..4..SeUr |
// /* 0050 */ "\x77\x3b\xc2\x79\xf5\x71\x9e\x2f\x6a\x56\x09\x85\x5b\xff\xd0\xb4" //w;.y.q./jV..[... |
// /* 0060 */ "\x0f\x85\x0c\x68\x30\x05\x34\x4c\xd5\x1f\x58\x29\x26\xae\xd2\xfe" //...h0.4L..X)&... |
// /* 0070 */ "\xa8\x83\x9d\x72\xe5\x5f\x0d\x49\x51\x74\x5b\x21\x78\x22\x58\x00" //...r._.IQt[!x"X. |
// /* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x0f\x00\x0f\x00\x41\x64\x6d\x69\x6e" //...........Admin |
// /* 0090 */ "\x69\x73\x74\x72\x61\x74\x65\x75\x72\x00\x10\x00\x09\x00\x52\x44" //istrateur.....RD |
// /* 00a0 */ "\x50\x2d\x54\x45\x53\x54\x00"                                     //P-TEST. |
// Dump done on RDP Client (5) 167 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x13, 0x83, 0x98, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0xb6, 0xbf, 0xf9, 0xc7,  // ................ |
// /* 0010 */ 0x44, 0x71, 0x16, 0xea, 0x72, 0x2a, 0xeb, 0xc0, 0xb0, 0x5f, 0x60, 0x31, 0x04, 0x6c, 0x1c, 0x95,  // Dq..r*..._`1.l.. |
// /* 0020 */ 0xae, 0x1e, 0xda, 0xd1, 0xba, 0x94, 0x1a, 0xaa, 0xc3, 0xb2, 0x36, 0xac, 0x00, 0x00, 0x48, 0x00,  // ..........6...H. |
// /* 0030 */ 0x9f, 0x09, 0xee, 0xb6, 0x56, 0x80, 0xb2, 0x43, 0xb2, 0xdd, 0x34, 0xed, 0x89, 0x53, 0x65, 0x55,  // ....V..C..4..SeU |
// /* 0040 */ 0x72, 0x77, 0x3b, 0xc2, 0x79, 0xf5, 0x71, 0x9e, 0x2f, 0x6a, 0x56, 0x09, 0x85, 0x5b, 0xff, 0xd0,  // rw;.y.q./jV..[.. |
// /* 0050 */ 0xb4, 0x0f, 0x85, 0x0c, 0x68, 0x30, 0x05, 0x34, 0x4c, 0xd5, 0x1f, 0x58, 0x29, 0x26, 0xae, 0xd2,  // ....h0.4L..X)&.. |
// /* 0060 */ 0xfe, 0xa8, 0x83, 0x9d, 0x72, 0xe5, 0x5f, 0x0d, 0x49, 0x51, 0x74, 0x5b, 0x21, 0x78, 0x22, 0x58,  // ....r._.IQt[!x"X |
// /* 0070 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x41, 0x64, 0x6d, 0x69,  // ............Admi |
// /* 0080 */ 0x6e, 0x69, 0x73, 0x74, 0x72, 0x61, 0x74, 0x65, 0x75, 0x72, 0x00, 0x10, 0x00, 0x09, 0x00, 0x52,  // nistrateur.....R |
// /* 0090 */ 0x44, 0x50, 0x2d, 0x54, 0x45, 0x53, 0x54, 0x00,                          // DP-TEST. |
// Front::NEW_LICENSE_REQUEST |
// Sec clear payload to send: |
// /* 0000 */ 0xff, 0x02, 0x10, 0x00, 0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x28, 0x14, 0x00, 0x00,  // ............(... |
// Sending on RDP Client (5) 34 bytes |
/* 0000 */ "\x03\x00\x00\x22\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x14\x80\x00" //..."...h....p... |
/* 0010 */ "\x10\x00\xff\x02\x10\x00\x07\x00\x00\x00\x02\x00\x00\x00\x28\x14" //..............(. |
/* 0020 */ "\x00\x00"                                                         //.. |
// Sent dumped on RDP Client (5) 34 bytes |
// Front::incoming::send_demand_active |
// Front::send_demand_active |
// Sending to client General caps (24 bytes) |
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
// Sending to client Bitmap caps (28 bytes) |
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
// Sending to client Font caps (8 bytes) |
// Font caps::fontSupportFlags 1 |
// Font caps::pad2octets 0 |
// Sending to client Order caps (88 bytes) |
// Order caps::terminalDescriptor 0 |
// Order caps::pad4octetsA 1078071040 |
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
// Order caps::orderSupport[TS_NEG_MEM3BLT_INDEX] 0 |
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
// Sending to client ColorCache caps (8 bytes) |
// ColorCache caps::colorTableCacheSize 6 |
// ColorCache caps::pad2octets 0 |
// Sending to client Pointer caps (10 bytes) |
// Pointer caps::colorPointerFlag 1 |
// Pointer caps::colorPointerCacheSize 25 |
// Pointer caps::pointerCacheSize 25 |
// Sending to client Share caps (8 bytes) |
// Share caps::nodeId 1001 |
// Share caps::pad2octets 46562 |
// Sending to client Input caps (88 bytes) |
// Input caps::inputFlags 41 |
// Input caps::pad2octetsA 0 |
// Input caps::keyboardLayout 0 |
// Input caps::keyboardType 0 |
// Input caps::keyboardSubType 0 |
// Input caps::keyboardFunctionKey 0 |
// Input caps::imeFileName 3952044240 |
// Sec clear payload to send: |
// /* 0000 */ 0x20, 0x01, 0x11, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x04, 0x00, 0x0a, 0x01, 0x52, 0x44,  //  .............RD |
// /* 0010 */ 0x50, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x01, 0x00, 0x03, 0x00, 0x00, 0x02,  // P............... |
// /* 0020 */ 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,  // ................ |
// /* 0030 */ 0x1c, 0x00, 0x10, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x04, 0x00, 0x03, 0x00, 0x00,  // ................ |
// /* 0040 */ 0x01, 0x00, 0x01, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x08, 0x00, 0x01, 0x00,  // ................ |
// /* 0050 */ 0x00, 0x00, 0x03, 0x00, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ....X........... |
// /* 0060 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x42, 0x0f, 0x00, 0x01, 0x00, 0x14, 0x00, 0x00, 0x00,  // ......@B........ |
// /* 0070 */ 0x01, 0x00, 0x2f, 0x00, 0x22, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,  // ../."........... |
// /* 0080 */ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01,  // ................ |
// /* 0090 */ 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0xa1, 0x06, 0x00, 0x00, 0x40, 0x42, 0x0f, 0x00, 0x40, 0x42,  // ..........@B..@B |
// /* 00a0 */ 0x0f, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x08, 0x00, 0x06, 0x00,  // ................ |
// /* 00b0 */ 0x00, 0x00, 0x08, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x19, 0x00, 0x19, 0x00, 0x09, 0x00, 0x08, 0x00,  // ................ |
// /* 00c0 */ 0xe9, 0x03, 0xe2, 0xb5, 0x0d, 0x00, 0x58, 0x00, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ......X.)....... |
// /* 00d0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00e0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00f0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0100 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0110 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// Sending on RDP Client (5) 303 bytes |
/* 0000 */ "\x03\x00\x01\x2f\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x81\x20\x20" //.../...h....p.   |
/* 0010 */ "\x01\x11\x00\xe9\x03\x02\x00\x01\x00\x04\x00\x0a\x01\x52\x44\x50" //.............RDP |
/* 0020 */ "\x00\x08\x00\x00\x00\x01\x00\x18\x00\x01\x00\x03\x00\x00\x02\x00" //................ |
/* 0030 */ "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x1c" //................ |
/* 0040 */ "\x00\x10\x00\x01\x00\x01\x00\x01\x00\x00\x04\x00\x03\x00\x00\x01" //................ |
/* 0050 */ "\x00\x01\x00\x00\x08\x01\x00\x00\x00\x0e\x00\x08\x00\x01\x00\x00" //................ |
/* 0060 */ "\x00\x03\x00\x58\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //...X............ |
/* 0070 */ "\x00\x00\x00\x00\x00\x40\x42\x0f\x00\x01\x00\x14\x00\x00\x00\x01" //.....@B......... |
/* 0080 */ "\x00\x2f\x00\x22\x00\x01\x01\x01\x01\x00\x00\x00\x00\x01\x01\x01" //./."............ |
/* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x01\x00" //................ |
/* 00a0 */ "\x01\x00\x00\x00\x00\xa1\x06\x00\x00\x40\x42\x0f\x00\x40\x42\x0f" //.........@B..@B. |
/* 00b0 */ "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x08\x00\x06\x00\x00" //................ |
/* 00c0 */ "\x00\x08\x00\x0a\x00\x01\x00\x19\x00\x19\x00\x09\x00\x08\x00\xe9" //................ |
/* 00d0 */ "\x03\xe2\xb5\x0d\x00\x58\x00\x29\x00\x00\x00\x00\x00\x00\x00\x00" //.....X.)........ |
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"     //............... |
// Sent dumped on RDP Client (5) 303 bytes |
// Front::incoming::ACTIVATED (new license request) |
// Front::incoming |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x02\x07"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 515 bytes |
// Recv done on RDP Client (5) 515 bytes |
// /* 0000 */ "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\xf8\xf8\x01\x13\x00\xe9" //...d....p....... |
// /* 0010 */ "\x03\x02\x00\x01\x00\xe9\x03\x06\x00\xe2\x01\x4d\x53\x54\x53\x43" //...........MSTSC |
// /* 0020 */ "\x00\x13\x00\x00\x00\x01\x00\x18\x00\x01\x00\x03\x00\x00\x02\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x0d\x04\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x1c" //................ |
// /* 0040 */ "\x00\x10\x00\x01\x00\x01\x00\x01\x00\x00\x04\x00\x03\x00\x00\x01" //................ |
// /* 0050 */ "\x00\x01\x00\x00\x08\x01\x00\x00\x00\x03\x00\x58\x00\x00\x00\x00" //...........X.... |
// /* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0070 */ "\x00\x01\x00\x14\x00\x00\x00\x01\x00\x00\x00\xaa\x00\x01\x01\x01" //................ |
// /* 0080 */ "\x01\x01\x00\x00\x01\x01\x01\x00\x01\x00\x00\x00\x01\x01\x01\x01" //................ |
// /* 0090 */ "\x01\x01\x01\x01\x00\x01\x01\x01\x00\x00\x00\x00\x00\xa1\x06\x06" //................ |
// /* 00a0 */ "\x00\x00\x00\x00\x00\x00\x84\x03\x00\x00\x00\x00\x00\xe4\x04\x00" //................ |
// /* 00b0 */ "\x00\x04\x00\x28\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //...(............ |
// /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x78\x00\x00" //.............x.. |
// /* 00d0 */ "\x02\x78\x00\x00\x08\x51\x01\x00\x20\x0a\x00\x08\x00\x06\x00\x00" //.x...Q.. ....... |
// /* 00e0 */ "\x00\x07\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\x00\x0c" //................ |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x02\x00\x02\x00\x08\x00\x0a\x00\x01\x00\x14" //................ |
// /* 0100 */ "\x00\x15\x00\x09\x00\x08\x00\x00\x00\x00\x00\x0d\x00\x58\x00\x91" //.............X.. |
// /* 0110 */ "\x00\x20\x00\x0c\x04\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x0c" //. .............. |
// /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0160 */ "\x00\x00\x00\x0c\x00\x08\x00\x01\x00\x00\x00\x0e\x00\x08\x00\x01" //................ |
// /* 0170 */ "\x00\x00\x00\x10\x00\x34\x00\xfe\x00\x04\x00\xfe\x00\x04\x00\xfe" //.....4.......... |
// /* 0180 */ "\x00\x08\x00\xfe\x00\x08\x00\xfe\x00\x10\x00\xfe\x00\x20\x00\xfe" //............. .. |
// /* 0190 */ "\x00\x40\x00\xfe\x00\x80\x00\xfe\x00\x00\x01\x40\x00\x00\x08\x00" //.@.........@.... |
// /* 01a0 */ "\x01\x00\x01\x03\x00\x00\x00\x0f\x00\x08\x00\x01\x00\x00\x00\x11" //................ |
// /* 01b0 */ "\x00\x0c\x00\x01\x00\x00\x00\x00\x14\x64\x00\x14\x00\x0c\x00\x01" //.........d...... |
// /* 01c0 */ "\x00\x00\x00\x00\x00\x00\x00\x15\x00\x0c\x00\x02\x00\x00\x00\x00" //................ |
// /* 01d0 */ "\x0a\x00\x01\x16\x00\x28\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //.....(.......... |
// /* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1a\x00\x08\x00\x00" //................ |
// /* 0200 */ "\x00\x00\x00"                                                     //... |
// Dump done on RDP Client (5) 515 bytes |
// sec decrypted payload: |
// /* 0000 */ 0xf8, 0x01, 0x13, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0xe9, 0x03, 0x06, 0x00, 0xe2, 0x01,  // ................ |
// /* 0010 */ 0x4d, 0x53, 0x54, 0x53, 0x43, 0x00, 0x13, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x01, 0x00,  // MSTSC........... |
// /* 0020 */ 0x03, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0030 */ 0x00, 0x00, 0x02, 0x00, 0x1c, 0x00, 0x10, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x04,  // ................ |
// /* 0040 */ 0x00, 0x03, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00,  // ................ |
// /* 0050 */ 0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // X............... |
// /* 0060 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,  // ................ |
// /* 0070 */ 0xaa, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00,  // ................ |
// /* 0080 */ 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,  // ................ |
// /* 0090 */ 0x00, 0x00, 0xa1, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x03, 0x00, 0x00, 0x00,  // ................ |
// /* 00a0 */ 0x00, 0x00, 0xe4, 0x04, 0x00, 0x00, 0x04, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........(....... |
// /* 00b0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00c0 */ 0x00, 0x00, 0x78, 0x00, 0x00, 0x02, 0x78, 0x00, 0x00, 0x08, 0x51, 0x01, 0x00, 0x20, 0x0a, 0x00,  // ..x...x...Q.. .. |
// /* 00d0 */ 0x08, 0x00, 0x06, 0x00, 0x00, 0x00, 0x07, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00e0 */ 0x00, 0x00, 0x05, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x08, 0x00,  // ................ |
// /* 00f0 */ 0x0a, 0x00, 0x01, 0x00, 0x14, 0x00, 0x15, 0x00, 0x09, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0100 */ 0x0d, 0x00, 0x58, 0x00, 0x91, 0x00, 0x20, 0x00, 0x0c, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,  // ..X... ......... |
// /* 0110 */ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0120 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0130 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0140 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0150 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00,  // ................ |
// /* 0160 */ 0x0e, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x34, 0x00, 0xfe, 0x00, 0x04, 0x00,  // ..........4..... |
// /* 0170 */ 0xfe, 0x00, 0x04, 0x00, 0xfe, 0x00, 0x08, 0x00, 0xfe, 0x00, 0x08, 0x00, 0xfe, 0x00, 0x10, 0x00,  // ................ |
// /* 0180 */ 0xfe, 0x00, 0x20, 0x00, 0xfe, 0x00, 0x40, 0x00, 0xfe, 0x00, 0x80, 0x00, 0xfe, 0x00, 0x00, 0x01,  // .. ...@......... |
// /* 0190 */ 0x40, 0x00, 0x00, 0x08, 0x00, 0x01, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x08, 0x00,  // @............... |
// /* 01a0 */ 0x01, 0x00, 0x00, 0x00, 0x11, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x14, 0x64, 0x00,  // ..............d. |
// /* 01b0 */ 0x14, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x0c, 0x00,  // ................ |
// /* 01c0 */ 0x02, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x16, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00,  // ..........(..... |
// /* 01d0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 01e0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 01f0 */ 0x1a, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,                          // ........ |
// Front::incoming::sec_flags=0 |
// Front received CONFIRMACTIVEPDU |
// process_confirm_active |
// lengthSourceDescriptor = 6 |
// lengthCombinedCapabilities = 482 |
// Front::capability 0 / 19 |
// Receiving from client General caps (24 bytes) |
// General caps::major 1 |
// General caps::minor 3 |
// General caps::protocol 512 |
// General caps::pad2octetA 0 |
// General caps::compression type 0 |
// General caps::extra flags 40d |
// General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED yes |
// General caps::extraflags:LONG_CREDENTIALS_SUPPORTED yes |
// General caps::extraflags:AUTORECONNECT_SUPPORTED yes |
// General caps::extraflags:ENC_SALTED_CHECKSUM no |
// General caps::extraflags:NO_BITMAP_COMPRESSION_HDR yes |
// General caps::updateCapability 0 |
// General caps::remoteUnshare 0 |
// General caps::compressionLevel 0 |
// General caps::refreshRectSupport 0 |
// General caps::suppressOutputSupport 0 |
// Front::capability 1 / 19 |
// Receiving from client Bitmap caps (28 bytes) |
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
// Front::capability 2 / 19 |
// Receiving from client Order caps (88 bytes) |
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
// Order caps::orderSupport[TS_NEG_MEM3BLT_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex1] 0 |
// Order caps::orderSupport[UnusedIndex2] 0 |
// Order caps::orderSupport[TS_NEG_DRAWNINEGRID_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_LINETO_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex3] 0 |
// Order caps::orderSupport[TS_NEG_SAVEBITMAP_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex4] 0 |
// Order caps::orderSupport[UnusedIndex5] 0 |
// Order caps::orderSupport[UnusedIndex6] 0 |
// Order caps::orderSupport[TS_NEG_MULTIDSTBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MULTIPATBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MULTISCRBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_FAST_INDEX_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_POLYGON_SC_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_POLYGON_CB_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_POLYLINE_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex7] 0 |
// Order caps::orderSupport[TS_NEG_FAST_GLYPH_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_ELLIPSE_SC_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_ELLIPSE_CB_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_INDEX_INDEX] 0 |
// Order caps::orderSupport[UnusedIndex8] 0 |
// Order caps::orderSupport[UnusedIndex9] 0 |
// Order caps::orderSupport[UnusedIndex10] 0 |
// Order caps::orderSupport[UnusedIndex11] 0 |
// Order caps::textFlags 1697 |
// Order caps::orderSupportExFlags 6 |
// Order caps::pad4octetsB 0 |
// Order caps::desktopSaveSize 230400 |
// Order caps::pad2octetsC 0 |
// Order caps::pad2octetsD 0 |
// Order caps::textANSICodePage 1252 |
// Order caps::pad2octetsE 0 |
// Front::capability 3 / 19 |
// Receiving from client BitmapCache caps (40 bytes) |
// BitmapCache caps::pad1 0 |
// BitmapCache caps::pad2 0 |
// BitmapCache caps::pad3 0 |
// BitmapCache caps::pad4 0 |
// BitmapCache caps::pad5 0 |
// BitmapCache caps::pad6 0 |
// BitmapCache caps::cache0Entries 120 |
// BitmapCache caps::cache0MaximumCellSize 512 |
// BitmapCache caps::cache1Entries 120 |
// BitmapCache caps::cache1MaximumCellSize 2048 |
// BitmapCache caps::cache2Entries 337 |
// BitmapCache caps::cache2MaximumCellSize 8192 |
// Front::capability 4 / 19 |
// Receiving from client CAPSTYPE_COLORCACHE |
// Front::capability 5 / 19 |
// Receiving from client CAPSTYPE_ACTIVATION |
// Front::capability 6 / 19 |
// Receiving from client CAPSTYPE_CONTROL |
// Front::capability 7 / 19 |
// Receiving from client Pointer caps (10 bytes) |
// Pointer caps::colorPointerFlag 1 |
// Pointer caps::colorPointerCacheSize 20 |
// Pointer caps::pointerCacheSize 21 |
// Front::capability 8 / 19 |
// Receiving from client CAPSTYPE_SHARE |
// Front::capability 9 / 19 |
// Receiving from client CAPSTYPE_INPUT |
// Front::capability 10 / 19 |
// Receiving from client CAPSTYPE_SOUND |
// Front::capability 11 / 19 |
// Front::capability 12 / 19 |
// Receiving from client CAPSTYPE_GLYPHCACHE |
// Front::capability 13 / 19 |
// Receiving from client CAPSTYPE_BRUSH |
// Receiving from client BrushCache caps (8 bytes) |
// BrushCacheCaps caps::brushSupportLevel 1 |
// Front::capability 14 / 19 |
// Receiving from client CAPSTYPE_OFFSCREENCACHE |
// Receiving from client OffScreenCache caps (12 bytes) |
// OffScreenCache caps::offscreenSupportLevel 1 |
// OffScreenCache caps::offscreenCacheSize 5120 |
// OffScreenCache caps::offscreenCacheEntries 100 |
// Front::capability 15 / 19 |
// Receiving from client CAPSTYPE_VIRTUALCHANNEL |
// Front::capability 16 / 19 |
// Receiving from client CAPSTYPE_DRAWNINEGRIDCACHE |
// Front::capability 17 / 19 |
// Receiving from client CAPSTYPE_DRAWGDIPLUS |
// Front::capability 18 / 19 |
// Receiving from client MultifragmentUpdate caps (8 bytes) |
// MultifragmentUpdate caps::MaxRequestSize 0 |
// process_confirm_active done p=0x7fffeb977b57 end=0x7fffeb977b57 |
// Front::reset::use_bitmap_comp=1 |
// Front::reset::use_compact_packets=1 |
// Front::reset::bitmap_cache_version=0 |
// Front: Use RDP 5.0 Bulk compression |
// GraphicsUpdatePDU::init::Initializing orders batch mcs_userid=0 shareid=65538 |
// GraphicsUpdatePDU::init::Initializing bitmaps batch mcs_userid=0 shareid=65538 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[0](2000) used=0 free=16278 |
// Front received CONFIRMACTIVEPDU done |
// Front::incoming |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x00\x24"                                                     //..$ |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 32 bytes |
// Recv done on RDP Client (5) 32 bytes |
// /* 0000 */ "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x16\x16\x00\x17\x00\xe9\x03" //...d....p....... |
// /* 0010 */ "\x02\x00\x01\x00\x00\x01\x08\x00\x1f\x00\x00\x00\x01\x00\xe9\x03" //................ |
// Dump done on RDP Client (5) 32 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x16, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x08, 0x00, 0x1f, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x01, 0x00, 0xe9, 0x03,                                // ...... |
// Front::incoming::sec_flags=0 |
// Front received DATAPDU |
// Front::process_data(...) |
// sdata_in.pdutype2=31 sdata_in.len=8 sdata_in.compressedLen=0 remains=0 payload_len=4 |
// PDUTYPE2_SYNCHRONIZE |
// PDUTYPE2_SYNCHRONIZE messageType=1 controlId=1001 |
// send_synchronize |
// Sec clear payload to send: |
// /* 0000 */ 0x16, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x16, 0x00, 0x1f, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x01, 0x00, 0xea, 0x03,                                // ...... |
// Sending on RDP Client (5) 36 bytes |
/* 0000 */ "\x03\x00\x00\x24\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x16\x16\x00" //...$...h....p... |
/* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x16\x00\x1f\x00\x00\x00" //................ |
/* 0020 */ "\x01\x00\xea\x03"                                                 //.... |
// Sent dumped on RDP Client (5) 36 bytes |
// send_synchronize done |
// process_data done |
// Front received DATAPDU done |
// Front::incoming |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x00\x28"                                                     //..( |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 36 bytes |
// Recv done on RDP Client (5) 36 bytes |
// /* 0000 */ "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00\x17\x00\xe9\x03" //...d....p....... |
// /* 0010 */ "\x02\x00\x01\x00\x00\x01\x0c\x00\x14\x00\x00\x00\x04\x00\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00"                                                 //.... |
// Dump done on RDP Client (5) 36 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x0c, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                    // .......... |
// Front::incoming::sec_flags=0 |
// Front received DATAPDU |
// Front::process_data(...) |
// sdata_in.pdutype2=20 sdata_in.len=12 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// PDUTYPE2_CONTROL |
// send_control action=4 |
// Sec clear payload to send: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x1a, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00,                    // .......... |
// Sending on RDP Client (5) 40 bytes |
/* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x1a\x1a\x00" //...(...h....p... |
/* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00" //................ |
/* 0020 */ "\x04\x00\x00\x00\xea\x03\x00\x00"                                 //........ |
// Sent dumped on RDP Client (5) 40 bytes |
// send_control done. action=4 |
// process_data done |
// Front received DATAPDU done |
// Front::incoming |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x00\x28"                                                     //..( |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 36 bytes |
// Recv done on RDP Client (5) 36 bytes |
// /* 0000 */ "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00\x17\x00\xe9\x03" //...d....p....... |
// /* 0010 */ "\x02\x00\x01\x00\x00\x01\x0c\x00\x14\x00\x00\x00\x01\x00\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00"                                                 //.... |
// Dump done on RDP Client (5) 36 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x0c, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                    // .......... |
// Front::incoming::sec_flags=0 |
// Front received DATAPDU |
// Front::process_data(...) |
// sdata_in.pdutype2=20 sdata_in.len=12 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// PDUTYPE2_CONTROL |
// send_control action=2 |
// Sec clear payload to send: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x1a, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00,                    // .......... |
// Sending on RDP Client (5) 40 bytes |
/* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x1a\x1a\x00" //...(...h....p... |
/* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00" //................ |
/* 0020 */ "\x02\x00\x00\x00\xea\x03\x00\x00"                                 //........ |
// Sent dumped on RDP Client (5) 40 bytes |
// send_control done. action=2 |
// process_data done |
// Front received DATAPDU done |
// Front::incoming |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
// /* 0000 */ "\x00\x00\x28"                                                     //..( |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 36 bytes |
// Recv done on RDP Client (5) 36 bytes |
// /* 0000 */ "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00\x17\x00\xe9\x03" //...d....p....... |
// /* 0010 */ "\x02\x00\x01\x00\x00\x01\x00\x00\x27\x00\x00\x00\x00\x00\x00\x00" //........'....... |
// /* 0020 */ "\x03\x00\x32\x00"                                                 //..2. |
// Dump done on RDP Client (5) 36 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x27, 0x00,  // ..............'. |
// /* 0010 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x32, 0x00,                    // ........2. |
// Front::incoming::sec_flags=0 |
// Front received DATAPDU |
// Front::process_data(...) |
// sdata_in.pdutype2=39 sdata_in.len=0 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// PDUTYPE2_FONTLIST |
// send_fontmap |
// Sec clear payload to send: |
// /* 0000 */ 0x9a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0xbe, 0x00, 0x28, 0x61,  // ..............(a |
// /* 0010 */ 0x9a, 0x00, 0xbf, 0x81, 0x4d, 0x80, 0x0a, 0x00, 0x00, 0x00, 0x09, 0xfe, 0x10, 0x0c, 0x00, 0x13,  // ....M........... |
// /* 0020 */ 0xe5, 0x09, 0x80, 0x00, 0x40, 0x07, 0x80, 0x00, 0x80, 0x07, 0xfe, 0x80, 0x77, 0xe9, 0x09, 0xc0,  // ....@.......w... |
// /* 0030 */ 0x01, 0x40, 0x02, 0xc0, 0x01, 0xbf, 0x30, 0x20, 0x00, 0x84, 0x00, 0x24, 0x00, 0x80, 0x00, 0x28,  // .@....0 ...$...( |
// /* 0040 */ 0x00, 0x8b, 0xe9, 0x09, 0x40, 0x03, 0x00, 0x09, 0x00, 0x03, 0x40, 0x08, 0xc0, 0x03, 0x80, 0x06,  // ....@.....@..... |
// /* 0050 */ 0x40, 0x03, 0xc0, 0x05, 0x80, 0x04, 0x00, 0x05, 0x40, 0x04, 0x40, 0x07, 0x00, 0x04, 0x80, 0x06,  // @.......@.@..... |
// /* 0060 */ 0xc0, 0x04, 0xc0, 0x06, 0x80, 0x05, 0x00, 0x05, 0xfe, 0x90, 0x63, 0xed, 0x03, 0xbe, 0x30, 0x33,  // ..........c...03 |
// /* 0070 */ 0xf3, 0x03, 0x7e, 0xb3, 0xef, 0x40, 0xa7, 0x87, 0x82, 0x7e, 0x0a, 0x00, 0x78, 0x49, 0xe0, 0xe8,  // ..~..@...~..xI.. |
// /* 0080 */ 0x06, 0xf0, 0x30, 0x1f, 0xc1, 0xc0, 0x8f, 0x0b, 0x02, 0x7c, 0x84, 0xf1, 0xb4, 0x01, 0xf8, 0xc8,  // ..0......|...... |
// /* 0090 */ 0x04, 0x00, 0x53, 0xe4, 0xe1, 0x57, 0x90, 0x85, 0x7f, 0x18,                    // ..S..W.... |
// Sending on RDP Client (5) 169 bytes |
/* 0000 */ "\x03\x00\x00\xa9\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x80\x9a\x9a" //.......h....p... |
/* 0010 */ "\x00\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\xbe\x00\x28\x61\x9a" //.............(a. |
/* 0020 */ "\x00\xbf\x81\x4d\x80\x0a\x00\x00\x00\x09\xfe\x10\x0c\x00\x13\xe5" //...M............ |
/* 0030 */ "\x09\x80\x00\x40\x07\x80\x00\x80\x07\xfe\x80\x77\xe9\x09\xc0\x01" //...@.......w.... |
/* 0040 */ "\x40\x02\xc0\x01\xbf\x30\x20\x00\x84\x00\x24\x00\x80\x00\x28\x00" //@....0 ...$...(. |
/* 0050 */ "\x8b\xe9\x09\x40\x03\x00\x09\x00\x03\x40\x08\xc0\x03\x80\x06\x40" //...@.....@.....@ |
/* 0060 */ "\x03\xc0\x05\x80\x04\x00\x05\x40\x04\x40\x07\x00\x04\x80\x06\xc0" //.......@.@...... |
/* 0070 */ "\x04\xc0\x06\x80\x05\x00\x05\xfe\x90\x63\xed\x03\xbe\x30\x33\xf3" //.........c...03. |
/* 0080 */ "\x03\x7e\xb3\xef\x40\xa7\x87\x82\x7e\x0a\x00\x78\x49\xe0\xe8\x06" //.~..@...~..xI... |
/* 0090 */ "\xf0\x30\x1f\xc1\xc0\x8f\x0b\x02\x7c\x84\xf1\xb4\x01\xf8\xc8\x04" //.0......|....... |
/* 00a0 */ "\x00\x53\xe4\xe1\x57\x90\x85\x7f\x18"                             //.S..W.... |
// Sent dumped on RDP Client (5) 169 bytes |
// send_fontmap done |
// Front::send_data_update_sync |
// send_server_update: fastpath_support=yes compression_support=yes shareId=65538 encryptionLevel=0 initiator=0 type=3 data_extra=0 |
// Sending on RDP Client (5) 5 bytes |
/* 0000 */ "\x00\x05\x03\x00\x00"                                             //..... |
// Sent dumped on RDP Client (5) 5 bytes |
// send_server_update done |
// --------------> UP AND RUNNING <---------------- |
// asking for selector |
// process_data done |
// Front received DATAPDU done |
// Front::begin_update |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[1](23) used=1033 free=15245 |
// order(10 clip(0,0,1,1)):opaquerect(rect(0,0,1024,768) color=0x00ffff) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[2](23) used=1042 free=15236 |
// order(10 clip(0,0,1,1)):opaquerect(rect(5,5,1014,758) color=0x00f800) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[3](23) used=1050 free=15228 |
// order(10 clip(0,0,1,1)):opaquerect(rect(10,10,1004,748) color=0x0007e0) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[4](23) used=1058 free=15220 |
// order(10 clip(0,0,1,1)):opaquerect(rect(15,15,994,738) color=0x00001f) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[5](23) used=1066 free=15212 |
// order(10 clip(0,0,1,1)):opaquerect(rect(20,20,984,728) color=0x000000) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[6](23) used=1073 free=15205 |
// order(10 clip(0,0,1,1)):opaquerect(rect(30,30,964,708) color=0x000273) |
// Widget_load: image file [./tests/fixtures/Philips_PM5544_640.png] is PNG file |
// front::draw:draw_tile((192, 144, 64, 64) (0, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[7](8208) used=1081 free=15197 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[8](30) used=1209 free=15069 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 144, 64, 64) (64, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[9](8208) used=1223 free=15055 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[10](30) used=1374 free=14904 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=1) |
// front::draw:draw_tile((320, 144, 64, 64) (128, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[11](8208) used=1380 free=14898 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[12](30) used=1487 free=14791 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=2) |
// front::draw:draw_tile((384, 144, 64, 64) (192, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[13](8208) used=1493 free=14785 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[14](30) used=1935 free=14343 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=3) |
// front::draw:draw_tile((448, 144, 64, 64) (256, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[15](8208) used=1941 free=14337 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[16](30) used=2193 free=14085 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=4) |
// front::draw:draw_tile((512, 144, 64, 64) (320, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[17](8208) used=2199 free=14079 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[18](30) used=2462 free=13816 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=5) |
// front::draw:draw_tile((576, 144, 64, 64) (384, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[19](8208) used=2468 free=13810 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[20](30) used=2922 free=13356 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=6) |
// front::draw:draw_tile((640, 144, 64, 64) (448, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[21](8208) used=2928 free=13350 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[22](30) used=3021 free=13257 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=7) |
// front::draw:draw_tile((704, 144, 64, 64) (512, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[23](8208) used=3027 free=13251 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[24](30) used=3181 free=13097 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=8) |
// front::draw:draw_tile((768, 144, 64, 64) (576, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[25](8208) used=3187 free=13091 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[26](30) used=3332 free=12946 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=9) |
// front::draw:draw_tile((192, 208, 64, 64) (0, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[27](8208) used=3338 free=12940 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[28](30) used=3441 free=12837 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=10) |
// front::draw:draw_tile((256, 208, 64, 64) (64, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[29](8208) used=3450 free=12828 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[30](30) used=3583 free=12695 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=11) |
// front::draw:draw_tile((320, 208, 64, 64) (128, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[31](8208) used=3589 free=12689 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[32](30) used=4141 free=12137 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=12) |
// front::draw:draw_tile((384, 208, 64, 64) (192, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[33](8208) used=4147 free=12131 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[34](30) used=4260 free=12018 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=13) |
// front::draw:draw_tile((448, 208, 64, 64) (256, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[35](8208) used=4266 free=12012 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[36](30) used=4306 free=11972 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=14) |
// front::draw:draw_tile((512, 208, 64, 64) (320, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[37](8208) used=4312 free=11966 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[38](30) used=4353 free=11925 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=15) |
// front::draw:draw_tile((576, 208, 64, 64) (384, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[39](8208) used=4359 free=11919 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[40](30) used=4450 free=11828 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=16) |
// front::draw:draw_tile((640, 208, 64, 64) (448, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[41](8208) used=4456 free=11822 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[42](30) used=5036 free=11242 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=17) |
// front::draw:draw_tile((704, 208, 64, 64) (512, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[43](8208) used=5042 free=11236 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[44](30) used=5175 free=11103 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=18) |
// front::draw:draw_tile((768, 208, 64, 64) (576, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[45](8208) used=5181 free=11097 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[46](30) used=5287 free=10991 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=19) |
// front::draw:draw_tile((192, 272, 64, 64) (0, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[47](8208) used=5293 free=10985 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[48](30) used=5398 free=10880 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=20) |
// front::draw:draw_tile((256, 272, 64, 64) (64, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[49](8208) used=5407 free=10871 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[50](30) used=5805 free=10473 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=21) |
// front::draw:draw_tile((320, 272, 64, 64) (128, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[51](8208) used=5811 free=10467 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[52](30) used=5957 free=10321 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=22) |
// front::draw:draw_tile((384, 272, 64, 64) (192, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[53](8208) used=5963 free=10315 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[54](30) used=6036 free=10242 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=23) |
// front::draw:draw_tile((448, 272, 64, 64) (256, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[55](8208) used=6042 free=10236 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[56](30) used=6116 free=10162 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=24) |
// front::draw:draw_tile((512, 272, 64, 64) (320, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[57](8208) used=6122 free=10156 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[58](30) used=6191 free=10087 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=25) |
// front::draw:draw_tile((576, 272, 64, 64) (384, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[59](8208) used=6197 free=10081 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[60](30) used=6268 free=10010 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=26) |
// front::draw:draw_tile((640, 272, 64, 64) (448, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[61](8208) used=6274 free=10004 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[62](30) used=6417 free=9861 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=27) |
// front::draw:draw_tile((704, 272, 64, 64) (512, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[63](8208) used=6423 free=9855 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[64](30) used=6842 free=9436 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=28) |
// front::draw:draw_tile((768, 272, 64, 64) (576, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[65](8208) used=6848 free=9430 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[66](30) used=6956 free=9322 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=29) |
// front::draw:draw_tile((192, 336, 64, 64) (0, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[67](8208) used=6962 free=9316 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[68](30) used=7047 free=9231 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=30) |
// front::draw:draw_tile((256, 336, 64, 64) (64, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[69](8208) used=7056 free=9222 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[70](30) used=7496 free=8782 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=31) |
// front::draw:draw_tile((320, 336, 64, 64) (128, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[71](8208) used=7502 free=8776 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[72](30) used=7616 free=8662 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=32) |
// front::draw:draw_tile((384, 336, 64, 64) (192, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[73](8208) used=7622 free=8656 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[74](30) used=7773 free=8505 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=33) |
// front::draw:draw_tile((448, 336, 64, 64) (256, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[75](8208) used=7779 free=8499 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[76](30) used=7953 free=8325 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=34) |
// front::draw:draw_tile((512, 336, 64, 64) (320, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[77](8208) used=7959 free=8319 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[78](30) used=8128 free=8150 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=35) |
// front::draw:draw_tile((576, 336, 64, 64) (384, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[79](8208) used=8134 free=8144 |
// GraphicsUpdatePDU::flush_orders: order_count=79 |
// send_server_update: fastpath_support=yes compression_support=yes shareId=65538 encryptionLevel=0 initiator=0 type=0 data_extra=79 |
// Sending on RDP Client (5) 5514 bytes |
/* 0000 */ "\x00\x95\x8a\x80\x21\x83\x15\x4f\x00\x03\xbe\x01\x80\x7d\x17\xc3" //....!..O.....}.. |
/* 0010 */ "\x80\x00\x05\x5f\x88\x95\x7c\x45\xff\xe2\x3f\xe0\x01\x57\xe2\x25" //..._..|E..?..W.% |
/* 0020 */ "\x5f\x11\x7f\xf8\x80\x04\x9f\xa0\xf8\x89\x57\xc4\x5f\xfe\x20\x01" //_.........W._. . |
/* 0030 */ "\xb7\xe8\x3e\x22\x55\xf1\x17\xff\x88\x00\x89\x7d\x07\xc4\x4a\xbe" //..>"U......}..J. |
/* 0040 */ "\x22\xff\xf1\x00\x13\x6f\xa0\xf8\x89\x57\xc4\x5f\xfe\x20\x02\xb7" //"....o...W._. .. |
/* 0050 */ "\xf4\x1f\x11\x2a\xf8\x8b\xff\xc4\x78\x96\x15\x7e\x22\x55\xf1\x17" //...*....x..~"U.. |
/* 0060 */ "\xff\x88\xf1\x8c\x2a\xfc\x44\xab\xe2\x2f\xff\x10\x00\x49\xf4\x1f" //....*.D../...I.. |
/* 0070 */ "\x11\x2a\xf8\x8b\xff\xc4\x00\x24\xfd\x07\xc4\x4a\xbe\x22\xff\xf1" //.*.....$...J.".. |
/* 0080 */ "\x00\x0d\xbf\x41\xf1\x12\xaf\x88\xbf\xfc\x40\x04\x4b\xe8\x3e\x22" //...A......@.K.>" |
/* 0090 */ "\x55\xf1\x17\xff\x88\x00\x9b\x7d\x07\xc4\x4a\xbe\x22\xff\xf1\x00" //U......}..J."... |
/* 00a0 */ "\x15\xbf\xa0\xf8\x89\x57\xc4\x5f\xfe\x23\xc2\xaf\x20\x3e\x83\xe2" //.....W._.#.. >.. |
/* 00b0 */ "\x2f\xff\x11\xe2\x17\x90\x09\x3e\x40\x24\xf9\x01\xea\x11\x27\xd0" ///......>@$....'. |
/* 00c0 */ "\x7c\x44\xab\xe2\x2f\xff\x10\x00\x93\xf4\x1f\x11\x2a\xf8\x8b\xff" //|D../.......*... |
/* 00d0 */ "\xc4\x00\x36\xfd\x07\xc4\x4a\xbe\x22\xff\xf1\x00\x11\x2f\xa0\xf8" //..6...J."..../.. |
/* 00e0 */ "\x89\x57\xc4\x5f\xfe\x20\x02\x6d\xf4\x1f\x11\x2a\xf8\x8b\xff\xc4" //.W._. .m...*.... |
/* 00f0 */ "\x00\x56\xfe\x83\xe2\x25\x5f\x11\x7f\xf8\x8f\x02\xbc\x80\xfa\x0f" //.V...%_......... |
/* 0100 */ "\x88\xbf\xfc\x47\x84\x5e\x40\x36\xf9\x00\xdb\xe4\x07\x01\x11\x27" //...G.^@6.......' |
/* 0110 */ "\xd0\x7c\x44\xab\xe2\x2f\xff\x10\x00\x93\xf4\x1f\x11\x2a\xf8\x8b" //.|D../.......*.. |
/* 0120 */ "\xff\xc4\x00\x36\xfd\x07\xc4\x4a\xbe\x22\xff\xf1\x00\x11\x2f\xa0" //...6...J."..../. |
/* 0130 */ "\xf8\x89\x57\xc4\x5f\xfe\x20\x02\x6d\xf4\x1f\x11\x2a\xf8\x8b\xff" //..W._. .m...*... |
/* 0140 */ "\xc4\x00\x56\xfe\x83\xe2\x25\x5f\x11\x7f\xf8\x8f\xea\xf2\x03\xe8" //..V...%_........ |
/* 0150 */ "\x3e\x22\xff\xf1\x1e\x01\x79\x01\x12\xf2\x02\x25\xe4\x07\x08\x11" //>"....y....%.... |
/* 0160 */ "\x27\xd0\x7c\x44\xab\xe2\x2f\xff\x10\x00\x93\xf4\x1f\x11\x2a\xf8" //'.|D../.......*. |
/* 0170 */ "\x8b\xff\xc4\x00\x36\xfd\x07\xc4\x4a\xbe\x22\xff\xf1\x00\x11\x2f" //....6...J."..../ |
/* 0180 */ "\xa0\xf8\x89\x57\xc4\x5f\xfe\x20\x02\x6d\xf4\x1f\x11\x2a\xf8\x8b" //...W._. .m...*.. |
/* 0190 */ "\xff\xc4\x00\x56\xfe\x83\xe2\x25\x5f\x11\x7f\xf8\x8f\xca\xf2\x03" //...V...%_....... |
/* 01a0 */ "\xe8\x3e\x22\xff\xf1\x1f\xc5\xe4\x04\xdb\xc8\x09\xb7\x90\x1c\x3c" //.>"............< |
/* 01b0 */ "\x44\x9f\x41\xf1\x12\xaf\x88\xbf\xfc\x40\x02\x4f\xd0\x7c\x44\xab" //D.A......@.O.|D. |
/* 01c0 */ "\xe2\x2f\xff\x10\x00\xdb\xf4\x1f\x11\x2a\xf8\x8b\xff\xc4\x00\x44" //./.......*.....D |
/* 01d0 */ "\xbe\x83\xe2\x25\x5f\x11\x7f\xf8\x80\x09\xb7\xd0\x7c\x44\xab\xe2" //...%_.......|D.. |
/* 01e0 */ "\x2f\xff\x10\x01\x5b\xfa\x0f\x88\x95\x7c\x45\xff\xe2\x3e\xab\xc8" ///...[....|E..>.. |
/* 01f0 */ "\x0f\xa0\xf8\x8b\xff\xc4\x7e\x17\x90\x15\xbf\x20\x2b\x7e\x40\x71" //......~.... +~@q |
/* 0200 */ "\x61\x12\x7d\x07\xc4\x4a\xbe\x22\xff\xf1\x00\x09\x3f\x41\xf1\x12" //a.}..J."....?A.. |
/* 0210 */ "\xaf\x88\xbf\xfc\x40\x03\x6f\xd0\x7c\x44\xab\xe2\x2f\xff\x10\x01" //....@.o.|D../... |
/* 0220 */ "\x12\xfa\x0f\x88\x95\x7c\x45\xff\xe2\x00\x26\xdf\x41\xf1\x12\xaf" //.....|E...&.A... |
/* 0230 */ "\x88\xbf\xfc\x40\x05\x6f\xe8\x3e\x22\x55\xf1\x17\xff\x88\xf8\xaf" //...@.o.>"U...... |
/* 0240 */ "\x20\x3e\x83\xe2\x2f\xff\x11\xf4\x5e\x40\x5f\xfc\x80\xbf\xfc\x29" // >../...^@_....) |
/* 0250 */ "\x00\x24\xfa\x0f\x88\x95\x7c\x45\xff\xe2\x00\x12\x7e\x83\xe2\x25" //.$....|E....~..% |
/* 0260 */ "\x5f\x11\x7f\xf8\x80\x06\xdf\xa0\xf8\x89\x57\xc4\x5f\xfe\x20\x02" //_.........W._. . |
/* 0270 */ "\x25\xf4\x1f\x11\x2a\xf8\x8b\xff\xc4\x00\x4d\xbe\x83\xe2\x25\x5f" //%...*.....M...%_ |
/* 0280 */ "\x11\x7f\xf8\x80\x0a\xdf\xd0\x7c\x44\xab\xe2\x2f\xff\x11\xe3\x57" //.......|D../...W |
/* 0290 */ "\x90\x1f\x41\xf1\x17\xff\x88\x09\x0a\x3c\xe5\xca\x03\xbf\xdf\xc4" //..A......<...... |
/* 02a0 */ "\x4f\xc1\x41\x6e\xd7\x60\x0b\xc7\xc8\xab\x00\x3f\xc8\xa1\xf0\x01" //O.An.`.....?.... |
/* 02b0 */ "\x11\xff\x91\x00\x02\x27\xe1\x41\x56\xcb\x63\x98\x10\x1b\x98\x00" //.....'.AV.c..... |
/* 02c0 */ "\x00\x20\x10\x10\x02\x02\x00\x83\x8f\x9f\x46\x10\x18\xd5\x5a\x80" //. ........F...Z. |
/* 02d0 */ "\x22\x21\x04\xa1\x5f\xef\xf0\x17\x78\xf5\x2e\xde\x69\x70\x88\x02" //"!.._...x...ip.. |
/* 02e0 */ "\x40\x6b\x75\xea\x00\x8c\x08\x40\x0a\x03\x38\x97\x52\xe3\xcc\x2c" //@ku....@..8.R.., |
/* 02f0 */ "\x0c\x17\xfb\xfd\x39\x56\x5b\x50\x01\xa1\x8c\x00\xc0\x47\x2c\xf4" //....9V[P.....G,. |
/* 0300 */ "\x01\x19\x52\x90\x14\x06\x75\x56\x39\xa8\x01\x41\x8c\x41\x68\x02" //..R...uV9..A.Ah. |
/* 0310 */ "\x28\xa5\x20\x50\x1b\xbc\xf4\xae\xe1\x40\x07\xa7\x4a\x94\xbe\xa6" //(. P.....@..J... |
/* 0320 */ "\x73\xaa\xb1\x5b\x03\x87\x85\x53\x69\xad\x00\x46\xb7\x5e\x1e\x80" //s..[...Si..F.^.. |
/* 0330 */ "\x9a\x49\x68\x02\x24\x84\x20\x16\xe4\x90\xd3\xf0\x20\x0a\x00\x04" //.Ih.$. ..... ... |
/* 0340 */ "\x40\x01\x03\xe1\x29\x80\x70\xaf\x27\x63\x08\xe7\x18\xa0\x02\xdb" //@...).p.'c...... |
/* 0350 */ "\xcd\x20\x26\x6a\xb4\x01\x3a\xea\x9c\x04\xfa\x1b\xe5\x48\x89\x45" //. &j..:......H.E |
/* 0360 */ "\x28\x62\x10\x4b\x84\x00\x12\x80\x0b\x1e\x61\x80\xc5\x5b\x68\x02" //(b.K......a..[h. |
/* 0370 */ "\x72\x03\xfa\x80\x14\x00\x40\x07\xc2\x0a\x1c\x08\x43\xcb\x91\x40" //r.....@.....C..@ |
/* 0380 */ "\x05\xf1\xc4\x14\x21\x00\xc0\x56\x69\xd4\x01\x0f\x99\x12\x63\x10" //....!..Vi.....c. |
/* 0390 */ "\x18\x0d\x5e\x7b\x43\x95\x29\x50\x01\xbc\x95\x0a\xb2\xd2\x00\x0a" //..^{C.)P........ |
/* 03a0 */ "\x80\x10\x63\x18\x0d\x11\x14\xa7\xcc\xe3\x60\x2f\xd2\xfa\x59\x30" //..c.......`/..Y0 |
/* 03b0 */ "\x3c\x78\x99\x0e\x73\xa0\x08\x56\xeb\xdf\x1c\x3d\x40\x12\xf3\x2a" //<x..s..V...=@..* |
/* 03c0 */ "\xcc\x22\x04\x02\x80\x02\x00\x06\xbd\xe5\xdc\x57\x3a\x20\xa8\x01" //.".........W: .. |
/* 03d0 */ "\xa2\x10\x4f\x55\xbc\xac\xa0\x08\x79\x52\x83\x60\x42\x15\x00\x29" //..OU....yR.`B..) |
/* 03e0 */ "\x73\x98\x0d\x00\x43\xc9\x76\x1b\xe3\xf8\x45\x29\xfa\x2f\x1e\x83" //s...C.v...E)./.. |
/* 03f0 */ "\x51\x4a\x6a\x00\x49\xad\x60\x2c\x03\xbf\xb9\x23\x07\xfa\x18\xd0" //QJj.I.`,...#.... |
/* 0400 */ "\x04\x3c\x6c\x80\x7e\xe7\x8c\xc0\x00\x14\x01\x0f\x18\xac\x1e\x31" //.<l.~..........1 |
/* 0410 */ "\x80\x20\x00\x39\x68\x0f\x8c\x70\x95\x80\x81\x80\x40\x84\x20\x91" //. .9h..p....@. . |
/* 0420 */ "\xaa\xb1\x00\xb5\x45\xb5\xac\x04\x0e\x04\xfb\x6f\x5d\xb7\x80\x0f" //....E......o]... |
/* 0430 */ "\x40\x57\x6d\xe4\x3c\x1c\x80\x03\xcc\x42\x10\x48\x14\x5a\x6b\x5f" //@Wm.<....B.H.Zk_ |
/* 0440 */ "\xbf\x80\x0f\x40\xc4\x20\x8e\x30\xcf\x05\x3e\x9c\xba\xcb\x67\xd7" //...@. .0..>...g. |
/* 0450 */ "\x7f\xa7\x2a\x30\xcf\xcd\x80\xa4\x96\x3d\xf4\xe8\xc2\x08\xb3\x4e" //..*0.....=.....N |
/* 0460 */ "\xfa\x75\x53\x8e\x1e\x59\xfe\x55\x3a\xcb\x7e\xee\x23\x14\x6f\xa7" //.uS..Y.U:.~.#.o. |
/* 0470 */ "\x24\x3c\x1e\x9f\x26\x7f\xcb\xc6\x51\xf2\xa2\x2d\x35\xfc\xd1\x04" //$<..&...Q..-5... |
/* 0480 */ "\xfc\xd1\x57\x9e\xf8\x84\x3b\xf5\x50\x3f\x99\xb5\xdf\xe6\x62\xa3" //..W...;.P?....b. |
/* 0490 */ "\x0c\x18\xa3\x57\xef\xe3\xd0\x48\x94\x52\xb7\x5e\xbf\xdf\xdf\x6d" //...W...H.R.^...m |
/* 04a0 */ "\xef\xb1\x28\xa4\x0e\xa7\x1c\x59\xa7\x4f\xae\xe3\x8c\x03\x1c\x5c" //..(....Y.O...... |
/* 04b0 */ "\x44\x1c\x34\xd4\x7b\x7b\xdc\x11\x56\x5a\x39\x67\xbf\xdf\xe3\xd7" //D.4.{{..VZ9g.... |
/* 04c0 */ "\x7e\x22\x31\x50\x01\x61\x08\x01\x81\x8e\x59\xee\xf3\xd1\xcb\x3b" //~"1P.a....Y....; |
/* 04d0 */ "\xb5\xfe\xff\xe2\xa4\x6a\xad\x7f\xbf\xae\xdb\xf8\x85\xc7\x13\x9a" //.....j.......... |
/* 04e0 */ "\x8d\x55\xbc\x39\x0e\x30\xc3\x8a\x37\x8a\x30\x30\x82\x3a\xf8\x08" //.U.9.0..7.00.:.. |
/* 04f0 */ "\x00\x3a\xc7\xf3\x11\x28\xa7\xd8\xaf\x98\x81\x2a\xcb\x6f\x7e\xcd" //.:...(.....*.o~. |
/* 0500 */ "\xaf\xac\x18\xa3\x3e\xdb\xfe\xb4\xf9\x08\x71\x46\xf4\x2b\xe6\x4d" //....>.....qF.+.M |
/* 0510 */ "\xcf\xe6\x6a\xb3\x4e\xf9\x90\xc8\x21\x04\xb8\x4f\xc1\x60\x7e\x55" //..j.N...!..O.`~U |
/* 0520 */ "\xa2\x1f\x1c\x65\xc6\xc4\x11\xc1\xbd\xc2\x75\x02\x24\x21\x9b\x4d" //...e......u.$!.M |
/* 0530 */ "\x40\x01\x7c\xfc\xa0\x04\x56\xeb\xd0\x20\x00\x03\x49\x2f\xce\x1a" //@.|...V.. ..I/.. |
/* 0540 */ "\x49\x5e\x79\x03\x2b\xfc\x20\x20\x0c\x00\x0e\xdf\xc3\xc7\x1b\x6f" //I^y.+.  .......o |
/* 0550 */ "\x25\x50\x01\x05\xc0\x00\x16\x80\x61\x6f\x7b\xb9\xa0\x01\x5f\xef" //%P......ao{..._. |
/* 0560 */ "\xfe\x43\x7a\x7e\x3c\x6f\x3c\x17\xca\xb2\xd7\x9f\x1c\xc0\xe7\x8e" //.Cz~<o<......... |
/* 0570 */ "\x50\x29\x94\x52\xaf\x3d\xf3\x9c\x81\x41\x1e\x69\xd5\x79\xec\xea" //P).R.=...A.i.y.. |
/* 0580 */ "\xad\xe8\xc0\xe7\x12\xb7\xea\x88\x94\x51\xd7\xf5\x38\x25\x3f\x54" //.........Q..8%?T |
/* 0590 */ "\x41\x54\x61\x82\x92\x5a\xbc\xf6\x35\x56\xfa\x01\xb6\x6f\xa0\x99" //ATa..Z..5V...o.. |
/* 05a0 */ "\x7d\xe0\xc5\x1a\xb7\x5e\xf0\x50\x6d\x94\x42\x09\x07\x51\x86\x5c" //}....^.Pm.B..Q.. |
/* 05b0 */ "\xe3\x8e\xb2\xdb\xbc\xf5\xe6\x9d\x7b\xaf\x7a\xec\x32\xd1\xf5\x11" //........{.z.2... |
/* 05c0 */ "\x28\xa0\xa4\x96\x2d\x35\x18\xa3\x3c\xd3\xa7\x55\x6e\x17\x01\x9f" //(...-5..<..Un... |
/* 05d0 */ "\xf0\xe2\xed\x30\x85\x59\x69\x87\x87\xd5\x38\xe0\xa4\x95\xd6\x5b" //...0.Yi...8....[ |
/* 05e0 */ "\x16\x9a\xcd\xa6\xb4\x22\x84\x28\x1d\xde\x7a\x31\x46\x38\xa3\x50" //.....".(..z1F8.P |
/* 05f0 */ "\xa4\xc6\x30\x27\x9a\x77\x10\x20\x83\xcf\xb6\x06\xe0\x74\x9e\x41" //..0'.w. .....t.A |
/* 0600 */ "\x11\x3a\xab\x10\x00\xf1\xfc\x83\x0f\x67\x45\x27\x89\x90\x1b\x8f" //.:.......gE'.... |
/* 0610 */ "\x88\xc2\x93\xc5\xfc\x0c\x63\xc6\x0a\xb9\xa4\x2f\x5e\xc2\xc6\xf1" //......c..../^... |
/* 0620 */ "\xdd\xb1\xec\x2c\x9f\x7d\x85\x1c\x28\x62\xa7\x1c\x00\x19\x82\x47" //...,.}..(b.....G |
/* 0630 */ "\xae\xef\x3c\xc4\xca\x30\xcf\x50\x80\x05\xe0\x55\x79\xec\xca\x28" //..<..0.P...Uy..( |
/* 0640 */ "\xcf\xf2\x82\x30\x39\xf5\xde\x75\x56\xab\xcf\x79\x83\x40\x0b\x4f" //...09..uV..y.@.O |
/* 0650 */ "\xd1\x06\xf1\xaf\x8e\x2c\x88\x94\x53\xf4\xbd\xa0\x00\x13\x82\xae" //.....,..S....... |
/* 0660 */ "\xdb\xe3\x5f\x88\x29\x25\x51\x86\x7f\x28\x0e\x82\xc7\xae\xf5\xba" //.._.)%Q..(...... |
/* 0670 */ "\xf0\xc5\x19\xd6\x5b\x32\x8a\x36\xfd\x00\x3f\x1e\x75\xee\xbc\x43" //....[2.6..?.u..C |
/* 0680 */ "\xc5\x38\xeb\x2d\xb9\xc7\x14\x61\x8e\x42\x10\x41\xf8\x11\xe5\x9e" //.8.-...a.B.A.... |
/* 0690 */ "\x75\x56\x21\xe3\x3e\x8b\x4d\x45\x24\xb1\x28\xa3\xd0\x84\x10\x33" //uV!.>.ME$.(....3 |
/* 06a0 */ "\x88\x70\x87\xd4\x68\x19\xc5\x18\x62\x8d\xca\x44\x50\x85\x07\x9b" //.p..h...b..DP... |
/* 06b0 */ "\x4d\x62\xd3\x57\x59\x68\xa4\x96\xa7\x1c\x89\x45\x0c\x3d\xbe\x95" //Mb.WYh.....E.=.. |
/* 06c0 */ "\x29\x00\xf3\x3b\x30\x00\x7e\xd3\x87\xb0\x8e\x00\x74\x00\x0e\x1a" //)..;0.~.....t... |
/* 06d0 */ "\x4e\x5e\x71\xc3\x6e\x79\x43\xc7\xb5\x1e\xc9\x8a\x00\x07\x2f\x99" //N^q.nyC......./. |
/* 06e0 */ "\x00\x06\x29\xa0\x00\xfb\x2e\x38\x67\xc5\x43\xd9\xb0\x0a\x00\x07" //..)....8g.C..... |
/* 06f0 */ "\x39\xe5\x1f\x8c\xdc\x04\x18\x00\x1e\x02\xda\xd6\x40\x2d\x02\x8d" //9...........@-.. |
/* 0700 */ "\x55\xbc\xd2\x1b\x81\x2e\xdb\xf8\x30\x07\x60\x71\xaa\xb1\x84\x13" //U.......0.`q.... |
/* 0710 */ "\x84\x6c\x40\xab\xf7\xf8\xb4\xd7\x86\x13\xe5\x2b\x75\xe7\x1f\x6c" //.l@........+u..l |
/* 0720 */ "\xc8\xcc\x0a\x7d\x77\x75\x96\xf0\x96\x00\x0c\x7f\x52\x8c\x33\x83" //...}wu......R.3. |
/* 0730 */ "\xc0\x00\x5a\x05\x1e\xbb\x8a\x49\x78\x4b\x00\x05\x60\x5c\x12\x90" //..Z....IxK..`... |
/* 0740 */ "\x4f\x73\x40\x9f\xa8\x95\x38\xe3\x5f\xca\x08\xfd\xe3\xac\xb7\xd9" //Os@...8._....... |
/* 0750 */ "\x30\x21\x02\x18\xa3\x18\x41\x1b\xbe\x50\x3a\x05\x3a\xab\x44\xa2" //0!....A..P:.:.D. |
/* 0760 */ "\x9e\x85\x00\x06\x78\xd0\x29\x25\xf6\x98\x00\x2b\xf4\x22\xd3\x57" //....x.)%...+.".W |
/* 0770 */ "\x4f\x94\x08\xe0\x66\xab\xcf\x79\x05\xbe\x60\x1b\xe6\x5b\xbf\xcc" //O...f..y..`..[.. |
/* 0780 */ "\x02\x7c\xc5\xdf\xe6\x38\x5f\x80\x00\xa0\x75\xfb\xf8\xc5\x19\x46" //.|...8_...u....F |
/* 0790 */ "\x18\xf7\xf3\x00\x1f\x60\xb3\x4e\xa9\xc7\x39\x4a\xaa\x00\x24\x00" //.....`.N..9J..$. |
/* 07a0 */ "\x00\x3e\x6d\x44\xa2\x83\x8c\x00\x97\xfb\xff\x29\x37\x1d\xb3\x01" //.>mD.......)7... |
/* 07b0 */ "\x75\xfe\xff\x05\xe1\x4f\x1c\x81\xe3\x94\xe6\x70\x21\xcb\x3a\xac" //u....O.....p!.:. |
/* 07c0 */ "\xb4\x4f\xfe\x8e\x28\xce\x30\xcf\x53\xaa\x00\x3c\x28\xdc\x27\x94" //.O..(.0.S..<(.'. |
/* 07d0 */ "\x0e\x01\xd1\xc8\xa6\x3d\x44\x10\x82\x08\xf0\xe2\x3a\xf1\x55\x71" //.....=D.....:.Uq |
/* 07e0 */ "\xaa\x00\x0b\x7e\xd9\x89\x70\xd9\x91\x48\x15\xee\xbc\xab\x2d\x6a" //...~..p..H....-j |
/* 07f0 */ "\xf9\x41\x18\x1b\xed\xbc\x63\xeb\x1b\x3e\x60\x3a\x07\x1e\xfc\x4a" //.A....c..>`:...J |
/* 0800 */ "\x9c\x71\xbf\xe6\x02\xfe\x64\xdc\xe3\x9e\x9e\x80\x8f\x98\x59\xa7" //.q....d.......Y. |
/* 0810 */ "\x71\xc8\x7c\xc8\x78\xe0\xfc\xb4\x02\xd0\x02\x3c\x4f\x1e\x33\x8f" //q.|.x......<O.3. |
/* 0820 */ "\x2a\x47\xcc\x8f\x19\xc7\x95\x23\x84\x1c\xf2\x90\x79\xf1\xca\x39" //*G.....#....y..9 |
/* 0830 */ "\x36\x9a\x90\xd0\x02\x09\x08\x79\x4b\x3e\x6b\x94\xdd\x52\x38\x46" //6......yK>k..R8F |
/* 0840 */ "\x40\x30\x00\x1a\x80\x07\x08\xcc\x13\xb0\x13\x0b\xa6\x14\x74\xc9" //@0............t. |
/* 0850 */ "\x71\x54\x74\xd6\x8e\x99\x91\xd3\x0d\xc7\x76\x0f\x7c\x23\x3d\x2f" //qTt.......v.|#=/ |
/* 0860 */ "\xdc\x7c\x20\xf7\xa5\x5c\x75\xdf\x8e\x98\x73\x20\x07\x20\x00\xfa" //.| ...u...s . .. |
/* 0870 */ "\xd3\xf8\xeb\x90\x0b\xc0\xe2\x42\x1b\x81\x00\x0f\x88\xe0\x1c\x00" //.......B........ |
/* 0880 */ "\x0e\x1b\xe2\x3c\x61\x78\x0b\x01\xd6\xe1\xc5\x0d\xe2\x58\x3e\x86" //...<ax.......X>. |
/* 0890 */ "\xf5\xd1\x9b\x44\x12\x80\x0b\x00\x7c\xf1\x63\xad\xcb\xad\x2d\x4f" //...D....|.c...-O |
/* 08a0 */ "\xa3\xbd\x74\x22\x02\x97\x15\xa0\x02\xc1\x6e\x3e\x16\x8e\xb6\x8e" //..t"......n>.... |
/* 08b0 */ "\xb9\x7c\x9d\x78\xe4\x15\x14\xa4\x3f\xae\x66\x5d\x76\xe3\xae\x69" //.|.x....?.f]v..i |
/* 08c0 */ "\x00\x14\x3f\xf6\x3e\xa6\x6c\x0c\xde\x31\x75\xcf\xde\xf1\xe6\x08" //..?.>.l..1u..... |
/* 08d0 */ "\x08\x00\x03\xe1\x7e\xf3\x05\xc0\xb0\x1a\xe2\x97\x00\x7c\x80\xa8" //....~........|.. |
/* 08e0 */ "\x4f\xbc\xc0\xed\xff\x1c\x84\x94\x0e\xe9\xef\x2e\xa9\x70\x5b\x8d" //O............p[. |
/* 08f0 */ "\x40\x0a\x84\xf1\xc7\xa1\x02\x10\xee\x12\xed\x89\x14\xbf\x1f\x3a" //@..............: |
/* 0900 */ "\xeb\x4f\x0c\xe9\xb1\xf4\x16\x1e\x59\xf9\x5f\xa9\x7c\x12\x1c\x82" //.O......Y._.|... |
/* 0910 */ "\x63\x82\x8c\xf2\xb3\x02\xf7\x01\x79\x4d\xf5\xf8\x68\x01\x32\xa5" //c.......yM..h.2. |
/* 0920 */ "\x26\x06\x3f\x14\x47\x95\x13\x69\xa8\xf7\x70\x88\xe0\xba\x00\xb8" //&.?.G..i..p..... |
/* 0930 */ "\x79\x5e\x02\x40\x00\xd6\xbc\xaf\x8a\xc0\x00\x57\x79\x1e\x3a\x80" //y^.@.......Wy.:. |
/* 0940 */ "\x14\x06\xbf\xff\x49\xad\x63\xdc\x5a\x20\x32\x9d\x4b\x5a\xa8\x01" //....I.c.Z 2.KZ.. |
/* 0950 */ "\x30\x84\x77\x67\x32\x94\x87\x53\x8d\x24\xb5\x0e\x4e\xee\xa4\x11" //0.wg2..S.$..N... |
/* 0960 */ "\x40\x11\x85\x29\x5c\x03\x82\x18\x05\x3a\xb2\x0f\x38\xad\xc8\x42" //@..).....:..8..B |
/* 0970 */ "\x10\xf7\xd8\x9b\x39\xcd\xc0\x38\x0c\x04\x18\x06\x80\x01\x50\xf1" //....9..8......P. |
/* 0980 */ "\x08\x0d\xe3\xc6\x18\xbb\x60\x4b\xd7\x77\x39\x71\x55\xf1\x7c\x2a" //......`K.w9qU.|* |
/* 0990 */ "\xcb\x78\x6b\xc7\xc7\x7c\xd8\xee\xf3\x55\x80\x2e\x26\x50\x20\x9a" //.xk..|...U..&P . |
/* 09a0 */ "\xd7\x50\x02\xa5\xad\x00\xc0\x5f\x6d\xf4\x34\xd6\xb1\x86\x00\x8b" //.P....._m.4..... |
/* 09b0 */ "\xfd\xfc\x56\x02\xa5\xde\x9c\xc3\x80\x01\x96\x02\x00\x1f\x74\x28" //..V...........t( |
/* 09c0 */ "\x6c\xa5\x22\xbd\xee\xa5\x0e\x2b\x50\x02\x21\xe6\x7d\x34\x3f\x28" //l."....+P.!.}4?( |
/* 09d0 */ "\x0f\x02\x19\x3c\x9e\x5c\xe0\xc8\xa1\x84\x21\x74\xd4\x7d\x67\x93" //...<......!t.}g. |
/* 09e0 */ "\x80\x00\x62\x3e\xbb\x05\x80\x01\x8d\x81\x79\x2f\x01\x90\x20\xc0" //..b>......y/.. . |
/* 09f0 */ "\x0a\x37\x8f\x45\x8c\x61\x90\x16\xe6\xba\x00\x35\xe2\x7a\x2d\xc7" //.7.E.a.....5.z-. |
/* 0a00 */ "\xe9\x01\x93\x18\x80\x0f\xc0\xa9\xef\x70\x21\x00\x01\xee\xe1\xcb" //.........p!..... |
/* 0a10 */ "\x5a\xd7\xce\x08\x41\x19\x4a\x7e\x3a\x5c\xe6\x82\x11\xf5\x66\x54" //Z...A.J~:.....fT |
/* 0a20 */ "\xa5\xf3\x82\x10\x4e\x15\x01\xf8\x14\x39\xcf\x02\xa5\x3e\xac\x42" //....N....9...>.B |
/* 0a30 */ "\xbe\xad\x41\xbf\xfd\x48\x84\x00\x04\x5b\x97\x6d\xfc\xc5\x1f\xd0" //..A..H...[.m.... |
/* 0a40 */ "\x20\xdf\x7f\xbf\xd4\x00\xe6\x51\x40\x16\x00\xd0\x00\x07\x08\xd5" // ......Q@....... |
/* 0a50 */ "\x5a\x02\xd2\x94\x00\x1f\x40\x00\x85\x29\x7e\x9a\xee\x1b\x09\xad" //Z.....@..)~..... |
/* 0a60 */ "\x7e\x07\xbf\xb6\xf0\x3c\x78\xc6\x7c\x8d\x5f\x7b\x8d\x17\xef\x08" //~....<x.|._{.... |
/* 0a70 */ "\x60\x3f\x76\xcb\x18\xfe\xed\xa4\xfd\xda\xa3\x9c\xfc\x75\xbd\xef" //`?v..........u.. |
/* 0a80 */ "\xc0\x41\xe8\x0f\x75\xd8\x04\x21\xfc\xf3\x4f\x7b\xa1\x8c\x7c\x99" //.A..u..!..O{..|. |
/* 0a90 */ "\x97\x39\xd8\xfe\x4d\x60\x0d\x07\xc9\x8b\xbf\x5d\xd3\x0f\xa6\x50" //.9..M`.....]...P |
/* 0aa0 */ "\x3b\xff\x37\x47\x82\xe2\x03\xda\x3d\xe0\x37\x8b\xb1\xe0\x0e\xf8" //;.7G....=.7..... |
/* 0ab0 */ "\xf1\xed\x99\xb7\xbd\xf8\xf1\xff\x62\xde\xf7\xe1\x8e\x2f\x93\x74" //........b..../.t |
/* 0ac0 */ "\xff\xb7\x87\xed\xcf\xfe\xbb\x37\x44\x07\xc1\x97\xe1\xa8\xfb\x09" //.......7D....... |
/* 0ad0 */ "\x52\x91\x00\x00\x8b\x00\x77\x6e\xa4\x6a\xad\x80\x2f\xc0\x09\x29" //R.....wn.j../..) |
/* 0ae0 */ "\x4c\x04\x1c\x5d\x81\xa1\x08\x55\x00\x20\x42\x1e\x1f\xc0\x84\x28" //L..]...U. B....( |
/* 0af0 */ "\x17\x76\x78\x0e\xa0\x06\xd9\xf8\x5c\x39\x18\x9c\x7b\x5e\x14\x61" //.vx......9..{^.a |
/* 0b00 */ "\x9d\x95\x1e\x9c\xae\xef\x17\xc4\xd1\xe1\x79\xc0\x0d\xc1\xfa\x17" //..........y..... |
/* 0b10 */ "\xb1\xe3\xc3\x8e\x86\x4c\x00\x98\xf9\x23\x08\x26\x00\x5f\xb2\x1e" //.....L...#.&._.. |
/* 0b20 */ "\x32\xaf\xa5\x3f\x6f\x3f\xf2\xfb\x71\x80\x1c\x1f\x72\x3c\x06\x99" //2..?o?..q...r<.. |
/* 0b30 */ "\x45\x00\x07\xfc\x36\xe1\xdd\x01\x80\x00\x6c\x9c\x5e\x71\x62\xc0" //E...6.....l.^qb. |
/* 0b40 */ "\xe8\x00\x5e\x1d\x16\x37\xf2\x05\x17\x00\x50\x3c\x0a\x16\x80\xd8" //..^..7....P<.... |
/* 0b50 */ "\xc6\x1c\x78\x4e\xb9\x81\x4d\xad\x6b\x00\xec\x80\x0c\xb8\x18\xc0" //..xN..M.k....... |
/* 0b60 */ "\xfc\x07\x9b\x32\xda\x0f\x40\x02\x49\x4a\xe2\x40\x11\x80\xcc\xa2" //...2..@.IJ.@.... |
/* 0b70 */ "\x94\x24\xd6\xb8\x08\xa4\x94\x43\x8f\x32\x8f\xcb\xcd\x80\x25\x9d" //.$.....C.2....%. |
/* 0b80 */ "\x66\xd4\x79\xd5\x5a\xe0\x00\x07\xc6\x8d\xe7\xd1\x84\x13\xe3\x87" //f.y.Z........... |
/* 0b90 */ "\xbc\x6f\x01\xbc\x59\x5e\x37\xc4\x67\x03\xdf\xa9\xde\x32\xa3\x6f" //.o..Y^7.g....2.o |
/* 0ba0 */ "\xd8\xb9\xd0\x02\x3d\xc0\x30\x45\x29\xb9\xd0\x30\x3c\x6b\xf7\x40" //....=.0E)..0<k.@ |
/* 0bb0 */ "\x70\x00\x18\xe7\xee\xc4\x6b\x04\x05\xbf\x5c\x79\x96\x03\xbf\x7f" //p.....k....y.... |
/* 0bc0 */ "\x28\x78\x00\x1a\x77\xef\xc5\x33\x04\x1b\x03\x7f\x7d\xff\x78\xfe" //(x..w..3....}.x. |
/* 0bd0 */ "\x46\x00\xdf\x2d\x11\x4a\x5c\xba\x43\x93\x60\x3c\x40\xc3\xf0\x14" //F..-.J..C.`<@... |
/* 0be0 */ "\x94\xab\x85\x48\x1f\xce\x8f\x3d\x3d\x82\xa7\xbd\xfa\x52\x3c\xf2" //...H...==....R<. |
/* 0bf0 */ "\xf3\xfb\xd0\xd1\x70\x3f\xf4\x04\x8f\x77\x06\x18\x7b\xd6\x44\x00" //....p?...w..{.D. |
/* 0c00 */ "\x00\xcd\xf9\x11\xe2\x6a\x04\x22\x01\xe1\xdc\x09\xe0\x2e\x30\xce" //.....j."......0. |
/* 0c10 */ "\x45\xe0\x01\x8f\x63\xb9\x14\x00\x05\xe0\x33\x79\xaf\x65\x68\x17" //E...c.....3y.eh. |
/* 0c20 */ "\xe4\xc4\xe1\x1c\x35\x92\xf6\x59\x81\x5e\xf3\x43\x08\x27\xcf\x4c" //....5..Y.^.C.'.L |
/* 0c30 */ "\xa2\x9d\x5f\x20\x52\x02\xeb\x2d\xec\xcd\x02\x7f\x5f\x09\x9f\x58" //.._ R..-...._..X |
/* 0c40 */ "\x79\x12\xfd\xce\xce\x70\x23\xd0\xc1\xd9\xc6\x04\x20\x48\x21\x13" //y....p#..... H!. |
/* 0c50 */ "\xaa\xb5\x39\x35\xad\x80\x43\x82\x92\xe7\x1c\xe4\x5e\x00\x0f\xa1" //..95..C.....^... |
/* 0c60 */ "\x34\xa5\x53\xfd\x02\x0e\x81\x48\x84\x3c\xea\xe4\x5a\x00\x0d\x81" //4.S....H.<..Z... |
/* 0c70 */ "\x10\x00\x26\xb5\xf1\x02\x7a\xd4\xd2\x95\xd6\xe2\x03\x3d\x46\x73" //..&...z......=Fs |
/* 0c80 */ "\xf9\x00\xb8\x14\x0b\xf2\x80\x07\x7f\x0c\xf6\xa5\x80\xaf\xc2\xeb" //................ |
/* 0c90 */ "\x9b\x01\x3e\xcc\x3b\x7c\x80\x47\x22\xd1\x0f\xe0\x92\xc6\x3e\xd5" //..>.;|.G".....>. |
/* 0ca0 */ "\xb0\x0f\xed\x77\x5e\x86\x00\x90\x51\xce\x7b\x58\xc0\x2c\x09\x94" //...w^...Q.{X.,.. |
/* 0cb0 */ "\xa6\xdf\xe0\x17\x0b\xc1\x0f\x7e\xf0\x00\x1c\x0a\x86\x31\x4f\x7b" //.......~.....1O{ |
/* 0cc0 */ "\xed\x7f\x00\x50\x2b\x18\xc4\xb9\xcd\xf3\xe5\x00\x7e\xa0\xd2\x4a" //...P+.......~..J |
/* 0cd0 */ "\xfb\xf2\x80\x10\x36\x10\x88\xd5\x5b\x00\x2e\x87\x20\x64\x00\x03" //....6...[... d.. |
/* 0ce0 */ "\x14\x6f\x37\x07\x48\x58\xbe\x30\x72\x04\xd2\x95\xf7\xc3\xbc\x79" //.o7.HX.0r......y |
/* 0cf0 */ "\x9c\x55\x1c\xdf\x12\x63\x16\x3f\x90\x1b\x81\x22\xf8\x76\x60\x40" //.U...c.?...".v`@ |
/* 0d00 */ "\x84\x36\xf7\xbe\xd5\x30\x33\xe0\xa5\x3d\xef\xbd\x3d\x50\xfc\x3c" //.6...03..=..=P.< |
/* 0d10 */ "\xef\x4f\x02\xff\x79\x4f\x7b\xe0\x33\x02\xdf\xbe\x78\x0d\xc0\xac" //.O..yO{.3...x... |
/* 0d20 */ "\x0a\x4c\x63\x6f\x7b\xe0\x31\x02\x9e\x13\x70\x09\x9f\x4c\x38\x01" //.Lco{.1...p..L8. |
/* 0d30 */ "\x40\x8f\xa2\x92\xa5\x2e\x00\x91\x81\x46\x49\x29\x50\x29\x52\x94" //@........FI)P)R. |
/* 0d40 */ "\x6a\xac\xc0\xb9\xc9\x21\xdc\x0a\x39\x1b\xc0\x17\x21\xb1\xe8\x6f" //j....!..9...!..o |
/* 0d50 */ "\x7e\x72\x8c\x33\xde\x2e\xca\x31\xc9\x0e\x03\x3b\x8b\x0c\x20\x9e" //~r.3...1...;.. . |
/* 0d60 */ "\xfa\x80\xbe\x46\x6f\x74\xd2\xcd\x3b\xdd\x33\xc8\xde\x07\x38\x63" //...Fot..;.3...8c |
/* 0d70 */ "\x2a\x71\xcf\x70\x40\x36\x00\x03\x18\x41\x3d\xc5\x00\x9c\x8d\x5e" //*q.p@6...A=....^ |
/* 0d80 */ "\xd3\xbf\xbb\x1e\xd3\xac\x01\x07\x1e\xd3\x80\x0c\x01\x09\x2f\xdf" //............../. |
/* 0d90 */ "\x2a\x38\x59\x1f\xfe\x44\x60\x10\x18\xe2\xd7\x42\x3d\x3e\x36\x12" //*8Y..D`....B=>6. |
/* 0da0 */ "\xed\x3f\x9f\x4f\x7d\x41\x61\xbd\xa9\x66\x80\x05\xd3\xa5\x50\x02" //.?.O}Aa..f....P. |
/* 0db0 */ "\xb7\xbd\xf7\x72\x9e\xa0\x0f\x8e\x18\x60\x0a\xbf\xdf\xc5\xba\x74" //...r.....`.....t |
/* 0dc0 */ "\xaa\x00\x5d\x5c\xa0\x7e\xbf\xd4\x02\x20\x20\xd0\xd1\x5e\x9d\x2a" //..]..~...  ..^.* |
/* 0dd0 */ "\x87\x18\x80\x0f\x6e\x99\x43\x8a\xf6\xe9\x5c\x05\x42\x3b\x44\x00" //....n.C.....B;D. |
/* 0de0 */ "\x16\xba\x7a\xbd\x26\x00\x1c\xe9\xea\xf1\x6a\xe0\x79\xd3\xe2\x44" //..z.&.....j.y..D |
/* 0df0 */ "\x80\x00\xd7\x7c\x97\x8a\xde\x0b\x3e\x76\x8a\xe5\xa7\x01\x75\xf3" //...|....>v....u. |
/* 0e00 */ "\x8f\x58\x07\x98\x04\xaf\xf7\xf0\xee\xbd\xd1\xd7\xde\x7a\xff\xbe" //.X...........z.. |
/* 0e10 */ "\x6b\xc6\x40\x1e\x72\x8e\xf1\xce\xc0\x4e\xbe\xd1\xf9\x00\x05\xdd" //k.@.r....N...... |
/* 0e20 */ "\x7c\xe3\x90\xd3\xb3\x13\xc6\x08\xd5\x5a\xe0\x21\xe3\x09\x13\x00" //|........Z.!.... |
/* 0e30 */ "\x03\x5c\xf1\x86\x2b\x58\x2e\xc9\xd9\x8f\x84\x02\x61\xfd\x9f\xd6" //....+X......a... |
/* 0e40 */ "\x3b\x31\x48\x01\x65\x3b\x9a\x28\x07\xb7\x8b\xf4\x4d\xbd\xef\xc4" //;1H.e;.(....M... |
/* 0e50 */ "\x0f\x13\x3b\x34\x57\xb3\x9e\x3d\x07\x76\x66\x81\xcf\x2e\xa8\x02" //..;4W..=.vf..... |
/* 0e60 */ "\x32\x21\x0e\xcc\xb4\x9a\xd7\x70\x0e\x05\xec\xcb\x22\x00\x22\x9c" //2!.....p....".". |
/* 0e70 */ "\xd2\x18\x08\xef\x17\xf0\x11\x5e\xcc\xbb\x81\xfa\x99\x4b\x8e\x50" //.......^.....K.P |
/* 0e80 */ "\x00\xec\x17\xfe\x3e\x8e\x0c\x22\x08\x0c\x02\xef\xcb\x71\x43\xaf" //....>..".....qC. |
/* 0e90 */ "\x70\xc1\x4e\xaa\xd4\x00\x4f\x3e\xa1\x95\x60\x00\xa7\x5f\xdc\xbe" //p.N...O>..`.._.. |
/* 0ea0 */ "\xdf\xf9\xc7\x3d\xb2\xc1\x05\x0f\xf5\x00\xb8\x06\x95\x63\xaf\x7e" //...=.........c.~ |
/* 0eb0 */ "\xc1\x54\x97\xe2\xbf\xdc\xb0\xf3\xb0\x40\x41\xa8\xdf\x14\x5d\x04" //.T.......@A...]. |
/* 0ec0 */ "\xfd\x81\x08\x24\x83\xc1\x96\x3f\x01\xb5\x47\x3e\x28\x3f\xeb\x51" //...$...?..G>(?.Q |
/* 0ed0 */ "\x0d\x9f\xe6\x25\x13\x5f\x8a\xfa\x37\xfe\x26\x8f\x3d\xf8\xaf\xa3" //...%._..7.&.=... |
/* 0ee0 */ "\x7f\x8a\x4a\x7c\x30\xf9\x8f\x18\x3c\x09\x72\xce\x90\x4f\xa2\xf0" //..J|0...<.r..O.. |
/* 0ef0 */ "\xac\x92\xf8\x9a\x3f\x75\x81\x7e\xe8\xa5\xf8\x85\xf3\x1f\x76\x7c" //....?u.~......v| |
/* 0f00 */ "\x87\x3f\x77\x4a\x67\xdd\xa9\x0f\xdd\x86\xaa\x70\xf6\x05\x8a\x81" //.?wJg......p.... |
/* 0f10 */ "\x08\x52\x97\xc7\x0f\x30\x60\x44\xc4\x06\x65\x56\xa2\x91\x31\x8c" //.R...0`D..eV..1. |
/* 0f20 */ "\x70\x10\x81\x09\x10\x86\x01\xf2\x60\xc0\xc1\x0d\x12\x85\x1c\x18" //p.......`....... |
/* 0f30 */ "\xf5\x3c\x71\x30\x54\x00\x7b\xb8\x47\x50\x08\xe4\x0c\xec\x23\x1e" //.<q0T.{.GP....#. |
/* 0f40 */ "\xc6\x71\xd5\x0f\xf5\x70\x76\x16\x88\x14\x4a\x29\x67\xec\xb5\x14" //.q...pv...J)g... |
/* 0f50 */ "\xf7\xbe\xda\x4e\xa5\x51\xd7\x2e\x7e\x76\x25\x1f\x39\xd0\x79\xf1" //...N.Q..~v%.9.y. |
/* 0f60 */ "\x5d\xf1\x10\x00\x2e\x18\xd0\x2a\x00\x07\x05\xe1\x8d\x8c\x0f\x06" //]......*........ |
/* 0f70 */ "\xe5\x28\x04\x16\x0b\xfc\x06\x89\x5f\xa7\xdf\xc1\xee\x00\x01\x54" //.(......_......T |
/* 0f80 */ "\x00\x32\x26\x38\x08\x3d\x1a\x80\x0d\x6b\x18\xa3\x7c\xa5\x11\xce" //.2&8.=...k..|... |
/* 0f90 */ "\x9e\xea\x4b\x40\x03\xa1\x47\x3a\x15\xa8\x00\xd7\x35\xa6\x00\x90" //..K@..G:....5... |
/* 0fa0 */ "\x00\x0b\x81\x68\x14\x07\xc3\xe1\xf8\x0f\x87\x63\xb3\xdb\xca\xb3" //...h.......c.... |
/* 0fb0 */ "\xd8\xf6\xbe\x73\xb9\xd4\x7c\xe7\x7a\x2b\xec\xd4\x1f\x81\x60\x42" //...s..|.z+....`B |
/* 0fc0 */ "\xe7\x9a\xa0\x42\x09\xe6\xab\xfe\x37\xcf\xf1\xfc\x62\x86\x30\x00" //...B....7...b.0. |
/* 0fd0 */ "\x33\xe5\x88\x16\x00\x03\x3c\xf2\xc6\x27\x58\x39\xa1\x40\x28\xbf" //3.....<..'X9.@(. |
/* 0fe0 */ "\xe0\x46\xa0\xbd\x1e\xc1\xe5\x88\xa2\xba\x92\xc0\x66\x4d\x94\x00" //.F..........fM.. |
/* 0ff0 */ "\x69\x34\x66\x80\x16\xea\x4b\x02\x99\x36\x2b\x49\xa8\xf3\x48\xcd" //i4f...K..6+I..H. |
/* 1000 */ "\x16\x14\x45\xe5\x99\x90\x91\xd0\x02\xfe\x5c\xb4\x5e\x27\xb7\x04" //..E.........^'.. |
/* 1010 */ "\x00\x5f\x07\xc0\xb8\x00\x19\xef\x83\xf1\x3b\xc1\xd5\xaa\x00\xf5" //._........;..... |
/* 1020 */ "\x80\x1e\x03\x40\x00\x0f\x81\x04\x21\x72\x16\x4b\x80\x5c\x7e\x7a" //...@....!r.K..~z |
/* 1030 */ "\x00\x0f\x70\x40\x08\xa7\x4d\x3c\x28\x64\x25\x80\xcb\x5b\x28\x00" //..p@..M<(d%..[(. |
/* 1040 */ "\xb3\x48\xcc\x05\x52\x4d\x4e\x96\x78\x3d\x04\xf0\x7c\x30\x02\x78" //.H..RMN.x=..|0.x |
/* 1050 */ "\x40\x65\xe1\x09\x18\x00\x03\x38\xf0\x86\x26\xd8\x3b\xe4\x43\xce" //@e.....8..&.;.C. |
/* 1060 */ "\xf4\x1f\x01\x02\x10\xa0\x0f\x0f\xde\x2e\x10\x00\x1b\x9c\x00\x20" //...............  |
/* 1070 */ "\x7e\xf1\x01\x29\x49\xa0\x05\x1a\xca\x60\x2a\x51\x3a\x00\x35\x98" //~..)I....`*Q:.5. |
/* 1080 */ "\xd2\x9c\x6b\x29\x7b\xe6\x86\xf3\x0c\x00\xb8\xfb\xea\x49\x7c\x17" //..k){........I|. |
/* 1090 */ "\x23\x20\x00\x67\x5e\x0b\xc4\xe3\x07\xa0\x28\xff\xe9\x37\xb4\x50" //# .g^.....(..7.P |
/* 10a0 */ "\x0a\x00\x2f\x1e\xaa\x51\xfc\x00\x28\x49\xa2\x92\x4a\x60\x29\xbe" //../..Q..(I..J`). |
/* 10b0 */ "\x06\x5d\x0d\x28\x01\x7f\x2e\x7b\x6b\x47\x82\xa0\x31\xc7\xe8\x94" //.].(...{kG..1... |
/* 10c0 */ "\x7e\x5b\xd5\x16\x35\xcf\x0d\xe0\xdc\x60\x18\x3e\x25\x4f\x00\xbc" //~[..5....`.>%O.. |
/* 10d0 */ "\x40\x62\xe8\x14\x02\x01\xf0\x0f\x06\xa9\xf7\x43\x48\x08\xbc\xb6" //@b.........CH... |
/* 10e0 */ "\x8d\x66\x8e\x50\x02\xcd\x48\x60\x2a\x4d\x74\xeb\x34\x71\xda\xf1" //.f.P..H`*Mt.4q.. |
/* 10f0 */ "\x3d\xa0\x05\x95\x10\xf8\x09\xe9\xe9\x69\x02\xc0\x6b\x0e\x11\xac" //=........i..k... |
/* 1100 */ "\x0d\x9e\x1e\xeb\xa0\xe4\x16\x1e\xf9\x93\x01\xc0\xb8\xd4\x3b\xcd" //..............;. |
/* 1110 */ "\xbd\xf3\x4a\xb2\xde\x9e\x75\x77\x9e\xa8\xc3\x3e\x78\x52\x4b\xd3" //..J...uw...>xRK. |
/* 1120 */ "\xc6\x06\xbf\xd3\xa7\x7c\x0d\x76\x01\x7d\x54\x5a\x6b\x50\x05\x91" //.....|.v.}TZkP.. |
/* 1130 */ "\x88\x60\x40\x49\x1b\x00\x03\x8b\x72\xb8\xc6\x28\x02\x38\x01\x40" //.`@I....r..(.8.@ |
/* 1140 */ "\x03\x1f\x00\x80\xa8\xb5\xf2\xb2\x9e\x84\x1a\x09\x40\x47\x80\x3a" //............@G.: |
/* 1150 */ "\x58\xc0\x5c\x06\x36\x21\xe4\x52\xc7\x0e\xf4\x06\xf2\xf7\xca\xea" //X...6!.R........ |
/* 1160 */ "\x70\x00\xf8\x2b\x98\xc9\xdf\x9a\x40\x1e\x03\x5d\x18\xef\x8b\x00" //p..+....@..].... |
/* 1170 */ "\xf0\x17\x72\x9e\x57\x72\xab\x2d\xe5\x8e\x12\x63\xf1\xa8\x74\x03" //..r.Wr.-...c..t. |
/* 1180 */ "\xb0\x4c\x04\xc0\x6b\x43\x1f\xe8\x6b\xc5\x45\x01\xa8\x3d\xfd\x72" //.L..kC..k.E..=.r |
/* 1190 */ "\xe0\x47\x4f\x08\x08\xf0\x8d\x5c\x96\xbd\x36\x1e\x00\xb8\x7d\x98" //.GO.......6...}. |
/* 11a0 */ "\xbc\x8c\x74\xff\x80\x78\x0c\xcd\x4b\xea\x07\x7e\xa5\x0c\x90\xf6" //..t..x..K..~.... |
/* 11b0 */ "\x2d\x80\x68\x0d\x4f\xf4\x41\xef\xd4\xa3\x8e\xa3\xd2\x0b\xff\x26" //-.h.O.A........& |
/* 11c0 */ "\x02\x7a\x01\xd4\x86\x01\x3c\x1d\x4a\x2d\x7f\x48\xf8\x11\xd4\xa6" //.z....<.J-.H.... |
/* 11d0 */ "\x00\xe0\x22\xa8\x5f\xcd\xe1\xbb\x19\x21\xec\x8d\x00\x50\x13\x2f" //.."._....!...P./ |
/* 11e0 */ "\x2c\x43\xd8\x47\x54\x08\x01\xfc\x3a\x9c\x10\xf5\x4c\x98\x0b\x8f" //,C.GT...:...L... |
/* 11f0 */ "\x80\xa2\xb8\x31\xd5\x52\x07\xe0\x33\x77\x3f\xc2\x9f\x08\x72\xd5" //...1.R..3w?...r. |
/* 1200 */ "\xc4\x62\x8b\x66\x8e\x40\x47\x6c\xfd\x10\x20\x5a\x88\x02\x43\x03" //.b.f.@Gl.. Z..C. |
/* 1210 */ "\x59\xe8\x88\xe5\xb9\x5c\xb8\xc3\x95\x00\x16\x5e\x89\xce\x5b\x55" //Y..........^..[U |
/* 1220 */ "\xf1\xc3\x7c\x10\x9d\xa8\xe0\x77\xe6\x3c\x0e\xab\xe0\x88\x07\xc7" //..|....w.<...... |
/* 1230 */ "\x60\x81\xe8\xed\x30\xef\x3b\x50\x2b\xa4\x73\xc6\xa1\xdf\xd3\x1f" //`...0.;P+.s..... |
/* 1240 */ "\x22\x6e\x7c\xd3\x9c\x23\x17\x0b\xef\x2c\xd9\x1c\x00\x03\x5f\xe1" //"n|..#...,...._. |
/* 1250 */ "\xfd\x8a\xee\x12\x1d\x75\x43\xe4\x00\x97\x50\x3a\x69\xba\x6d\x3a" //.....uC...P:i.m: |
/* 1260 */ "\xa1\x4f\x51\xe8\xb7\xbd\xf9\x26\xba\x9b\x39\x06\x03\x82\x3a\x65" //.OQ....&..9...:e |
/* 1270 */ "\xc7\x54\x8a\x8f\x5d\xfa\x71\x03\xbf\x44\x75\x5d\x79\xd1\xd5\xdd" //.T..].q..Du]y... |
/* 1280 */ "\xd3\x4e\x3c\x14\xb8\x08\x71\x70\x91\xd0\x00\x34\x8f\x19\x62\x8d" //.N<...qp...4..b. |
/* 1290 */ "\x84\xa5\xc1\xde\x06\x60\xc0\xb8\xf1\x8c\x78\xe1\x01\x57\xae\xd2" //.....`....x..W.. |
/* 12a0 */ "\xe9\xf3\xab\x85\xd0\x0f\xaa\x94\x10\x43\xae\x14\x00\x3c\x0a\x9c" //.........C...<.. |
/* 12b0 */ "\xc0\xa7\xf5\x81\x52\x1d\x4c\x27\x02\x6b\x30\x6e\xcc\x36\xb5\xf5" //....R.L'.k0n.6.. |
/* 12c0 */ "\x31\xa8\x10\x85\xc2\x9c\x0c\x03\xd4\xc6\x14\x00\x47\x80\x00\xe5" //1...........G... |
/* 12d0 */ "\x7c\x4e\x71\x94\x80\x8f\xf5\x31\x53\xe7\x50\x18\x8d\x2f\xc3\x30" //|Nq....1S.P../.0 |
/* 12e0 */ "\xab\x2d\xa7\x7a\x4d\x4f\x7b\x11\x80\xd2\xd6\xbe\x82\xa0\x07\xd6" //.-.zMO{......... |
/* 12f0 */ "\x65\xc3\x29\x82\x3c\xc3\xf8\x08\x39\xf1\x5c\xdd\x22\x03\xcd\x5a" //e.).<...9..."..Z |
/* 1300 */ "\x3b\xc7\xbe\x2b\xa1\x34\x73\xa4\x85\xfe\x55\x2f\x94\xe6\xfb\x18" //;..+.4s...U/.... |
/* 1310 */ "\x01\x61\xf1\x58\x0c\xd8\x3f\xf5\x14\x00\x1a\x18\xc7\x96\x60\x07" //.a.X..?.......`. |
/* 1320 */ "\xed\xa8\x7c\xe1\x80\xf0\x17\x9a\x77\x16\x6a\x8d\x55\x86\x28\x00" //..|.....w.j.U.(. |
/* 1330 */ "\xa4\x10\xee\xb5\x2c\x02\x19\x14\x00\x19\x94\x53\x01\x93\x4f\x44" //....,......S..OD |
/* 1340 */ "\x45\x0f\x83\x08\x1e\x03\x06\x31\xa0\x00\xf4\x9a\x01\x1c\x51\x80" //E......1......Q. |
/* 1350 */ "\x70\x06\x53\xb9\x0b\xea\x3c\x2a\x5f\xf0\xfd\x78\xa6\x7c\x67\x3e" //p.S...<*_..x.|g> |
/* 1360 */ "\x3d\x9f\x23\xcf\x94\x67\xcb\x73\xe6\x59\xf3\x7c\xf9\xe6\x7a\x4e" //=.#..g.s.Y.|..zN |
/* 1370 */ "\x04\x3b\x8d\x1b\xf8\x60\x04\x30\x35\x12\x03\x10\x8a\x50\xe0\x55" //.;...`.05....P.U |
/* 1380 */ "\x6a\x6c\x8a\xad\xd7\x79\x42\x9d\x08\x40\x68\x55\xea\x1c\x8a\xad" //jl...yB..@hU.... |
/* 1390 */ "\xc7\x21\x7c\xbf\xbb\xd8\x2f\xe3\x1c\x31\x54\xf6\xd6\xb8\x09\xd5" //.!|.../..1T..... |
/* 13a0 */ "\x9f\x02\x74\x46\x06\xd6\x56\x7a\xcc\xc4\x63\x01\xd2\xa0\x26\xfd" //..tF..Vz..c...&. |
/* 13b0 */ "\x5d\x62\x2b\xd7\x5a\x05\x3a\xdc\xb9\x5a\x40\xf9\x0f\x61\x47\x5b" //]b+.Z.:..Z@..aG[ |
/* 13c0 */ "\xf9\xeb\x2d\x3d\x79\xa7\xac\x7c\xc1\x3f\x84\x49\xff\x5c\x36\xc3" //..-=y..|.?.I..6. |
/* 13d0 */ "\x3e\x2b\x88\xab\xf4\x5f\xe3\x99\x27\xde\x88\x4f\xde\x82\xfe\xb8" //>+..._..'..O.... |
/* 13e0 */ "\x75\xf6\xf2\xec\x0f\x3f\xe6\x7b\x30\xdf\xdb\x94\x1f\xb7\x08\xb4" //u....?.{0....... |
/* 13f0 */ "\xe1\x3e\x1b\x91\x85\x28\x0c\xbc\x37\x31\x63\xc2\x89\xa1\xde\x15" //.>...(..71c..... |
/* 1400 */ "\x22\x1e\x0f\x2e\x7f\x60\x00\x3a\x00\x6f\x03\x06\x09\xac\x8a\x00" //"....`.:.o...... |
/* 1410 */ "\x8d\x0c\x63\x04\x12\xc5\x00\x37\x80\xc8\x06\x45\x83\xd1\x18\x20" //..c....7...E...  |
/* 1420 */ "\x9a\x26\x51\x4a\x00\x6f\x01\x56\x09\x1d\x9c\x12\x1e\x1d\xfe\x89" //.&QJ.o.V........ |
/* 1430 */ "\x1f\xf4\x48\xaa\xd0\x32\x5b\x3a\x35\xbc\x12\xcf\xeb\x25\x5a\x03" //..H..2[:5....%Z. |
/* 1440 */ "\x27\x92\xf5\x90\x30\x08\x75\xb1\xa8\x51\x15\xc1\x00\x1f\xc7\x10" //'...0.u..Q...... |
/* 1450 */ "\x40\x01\x83\xef\xf5\x21\x00\xe1\xc4\x22\xe0\xb4\xe4\xd8\xf2\x05" //@....!..."...... |
/* 1460 */ "\x0c\xf8\xfd\xc2\xaf\x0a\x00\x87\x91\xf0\x0a\xf2\x3d\x70\x9c\xc3" //............=p.. |
/* 1470 */ "\xc8\x71\xe4\xfc\x42\x7e\x63\x85\x05\xf2\x6c\x8f\x2b\xe1\x40\x09" //.q..B~c...l.+.@. |
/* 1480 */ "\xef\xbe\xa0\x01\xc0\x6e\xe6\xb4\x24\x59\x0c\x06\x6c\x5b\x40\x10" //.....n..$Y..l[@. |
/* 1490 */ "\x75\x25\x81\xc6\x95\xa7\x4b\xcb\x28\xc6\x30\x1b\x59\xa5\x1d\x90" //u%....K.(.0.Y... |
/* 14a0 */ "\x97\x01\xdf\xba\xdb\x4c\x13\xcb\xb8\x10\x80\x01\xc8\x7c\xbb\x8c" //.....L.......|.. |
/* 14b0 */ "\x7f\x0a\xad\x01\xfc\xbd\xc0\x21\xc3\xb8\x43\xe6\x25\x1f\xcb\x2f" //.......!..C.%../ |
/* 14c0 */ "\x32\xf0\xf1\x65\x80\x18\x10\x80\x0f\x9a\xa2\x8f\xe5\x58\xf3\x6f" //2..e.........X.o |
/* 14d0 */ "\x18\x07\xcc\x88\x73\x98\x0d\x1f\xcd\x2f\x3a\xf2\x60\x37\xf7\x88" //....s..../:.`7.. |
/* 14e0 */ "\xd7\x0a\x08\x1f\x98\x10\x1b\x31\xad\x09\x52\x43\x01\x92\xf9\xec" //.......1..RC.... |
/* 14f0 */ "\xc8\x4b\x04\x85\x2b\x4a\xd7\x92\x11\x98\x30\x31\x7c\x76\x0f\x50" //.K..+J....01|v.P |
/* 1500 */ "\x00\x2e\x0a\x3e\x74\x92\x20\x00\x38\xe7\x9d\x31\x8d\x00\x11\xf5" //...>t. .8..1.... |
/* 1510 */ "\xed\x91\x0f\x05\x88\x7c\xe6\xf3\xc1\x14\xa4\x8a\xa3\xde\x5d\x70" //.....|........]p |
/* 1520 */ "\x01\x03\xe3\x2a\xf3\x1c\x00\xe0\x2d\x96\x09\xbd\x8f\x28\xc2\x6b" //...*....-....(.k |
/* 1530 */ "\x5d\x46\x86\x30\xff\xe7\x08\xc0\x93\x8f\x2e\x56\x09\xfe\x8c\x02" //]F.0.......V.... |
/* 1540 */ "\xb2\xf3\xa4\x60\x24\xb9\xb9\xc7\x2a\x38\x0a\x65\xe1\x47\x96\xe2" //...`$...*8.e.G.. |
/* 1550 */ "\xa0\x03\xe9\x38\xe5\xa4\xf3\x9c\x01\x2a\xcb\x40\x7d\xc0\xf4\x52" //...8.....*.@}..R |
/* 1560 */ "\xe1\x51\x03\x82\x10\x84\x1c\xd2\xab\x55\x28\xd5\x4a\x80\x20\x2b" //.Q.......U(.J. + |
/* 1570 */ "\x29\x80\xbb\x4c\x68\x44\xa5\xb0\x19\xbc\xe6\x8e\x2b\x29\x11\xec" //)..LhD......+).. |
/* 1580 */ "\xf0\x60\x0e\xec\xf0\xe0\x47\x44\x60\x00"                         //.`....GD`. |
// Sent dumped on RDP Client (5) 5514 bytes |
// send_server_update done |
// GraphicsUpdatePDU::init::Initializing orders batch mcs_userid=0 shareid=65538 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[1](30) used=149 free=16129 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=36) |
// front::draw:draw_tile((640, 336, 64, 64) (448, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[2](8208) used=155 free=16123 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[3](30) used=265 free=16013 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=37) |
// front::draw:draw_tile((704, 336, 64, 64) (512, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[4](8208) used=271 free=16007 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[5](30) used=721 free=15557 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=38) |
// front::draw:draw_tile((768, 336, 64, 64) (576, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[6](8208) used=727 free=15551 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[7](30) used=812 free=15466 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=39) |
// front::draw:draw_tile((192, 400, 64, 64) (0, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[8](8208) used=818 free=15460 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[9](30) used=911 free=15367 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=40) |
// front::draw:draw_tile((256, 400, 64, 64) (64, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[10](8208) used=920 free=15358 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[11](30) used=1370 free=14908 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=41) |
// front::draw:draw_tile((320, 400, 64, 64) (128, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[12](8208) used=1376 free=14902 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[13](30) used=1631 free=14647 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=42) |
// front::draw:draw_tile((384, 400, 64, 64) (192, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[14](8208) used=1637 free=14641 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[15](30) used=1936 free=14342 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=43) |
// front::draw:draw_tile((448, 400, 64, 64) (256, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[16](8208) used=1942 free=14336 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[17](30) used=2265 free=14013 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=44) |
// front::draw:draw_tile((512, 400, 64, 64) (320, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[18](8208) used=2271 free=14007 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[19](30) used=2592 free=13686 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=45) |
// front::draw:draw_tile((576, 400, 64, 64) (384, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[20](8208) used=2598 free=13680 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[21](30) used=2897 free=13381 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=46) |
// front::draw:draw_tile((640, 400, 64, 64) (448, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[22](8208) used=2903 free=13375 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[23](30) used=3141 free=13137 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=47) |
// front::draw:draw_tile((704, 400, 64, 64) (512, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[24](8208) used=3147 free=13131 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[25](30) used=3582 free=12696 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=48) |
// front::draw:draw_tile((768, 400, 64, 64) (576, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[26](8208) used=3588 free=12690 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[27](30) used=3680 free=12598 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=49) |
// front::draw:draw_tile((192, 464, 64, 64) (0, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[28](8208) used=3686 free=12592 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[29](30) used=3806 free=12472 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=50) |
// front::draw:draw_tile((256, 464, 64, 64) (64, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[30](8208) used=3815 free=12463 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[31](30) used=4091 free=12187 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=51) |
// front::draw:draw_tile((320, 464, 64, 64) (128, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[32](8208) used=4097 free=12181 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[33](30) used=4603 free=11675 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=52) |
// front::draw:draw_tile((384, 464, 64, 64) (192, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[34](8208) used=4609 free=11669 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[35](30) used=4815 free=11463 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=53) |
// front::draw:draw_tile((448, 464, 64, 64) (256, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[36](8208) used=4821 free=11457 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[37](30) used=4983 free=11295 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=54) |
// front::draw:draw_tile((512, 464, 64, 64) (320, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[38](8208) used=4989 free=11289 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[39](30) used=5149 free=11129 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=55) |
// front::draw:draw_tile((576, 464, 64, 64) (384, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[40](8208) used=5155 free=11123 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[41](30) used=5346 free=10932 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=56) |
// front::draw:draw_tile((640, 464, 64, 64) (448, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[42](8208) used=5352 free=10926 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[43](30) used=5804 free=10474 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=57) |
// front::draw:draw_tile((704, 464, 64, 64) (512, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[44](8208) used=5810 free=10468 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[45](30) used=6069 free=10209 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=58) |
// front::draw:draw_tile((768, 464, 64, 64) (576, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[46](8208) used=6075 free=10203 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[47](30) used=6195 free=10083 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=59) |
// front::draw:draw_tile((192, 528, 64, 64) (0, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[48](8208) used=6201 free=10077 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[49](30) used=6325 free=9953 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=60) |
// front::draw:draw_tile((256, 528, 64, 64) (64, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[50](8208) used=6334 free=9944 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[51](30) used=6472 free=9806 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=61) |
// front::draw:draw_tile((320, 528, 64, 64) (128, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[52](8208) used=6478 free=9800 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[53](30) used=6832 free=9446 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=62) |
// front::draw:draw_tile((384, 528, 64, 64) (192, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[54](8208) used=6838 free=9440 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[55](30) used=7365 free=8913 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=63) |
// front::draw:draw_tile((448, 528, 64, 64) (256, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[56](8208) used=7371 free=8907 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[57](30) used=7453 free=8825 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=64) |
// front::draw:draw_tile((512, 528, 64, 64) (320, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[58](8208) used=7459 free=8819 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[59](30) used=7518 free=8760 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=65) |
// front::draw:draw_tile((576, 528, 64, 64) (384, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[60](8208) used=7524 free=8754 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[61](30) used=8003 free=8275 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=66) |
// front::draw:draw_tile((640, 528, 64, 64) (448, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[62](8208) used=8009 free=8269 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[63](30) used=8324 free=7954 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=67) |
// front::draw:draw_tile((704, 528, 64, 64) (512, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[64](8208) used=8330 free=7948 |
// GraphicsUpdatePDU::flush_orders: order_count=64 |
// send_server_update: fastpath_support=yes compression_support=yes shareId=65538 encryptionLevel=0 initiator=0 type=0 data_extra=64 |
// Sending on RDP Client (5) 5230 bytes |
/* 0000 */ "\x00\x94\x6e\x80\x21\x67\x14\x40\x00\x03\x84\x79\xc7\x18\x36\x15" //..n.!g.@...y..6. |
/* 0010 */ "\xfe\x0c\xf9\xb7\x5f\x09\x38\x10\x38\x58\xc7\x9a\x76\xf3\x2c\x70" //...._.8.8X..v.,p |
/* 0020 */ "\xab\x9f\x31\x71\xf0\xab\x8f\x2d\xf2\x70\xbf\x79\x09\x01\x87\xc6" //..1q...-.p.y.... |
/* 0030 */ "\x68\x00\x20\x92\x98\x19\x45\x2a\x97\x52\x85\x54\x88\x00\x20\x2c" //h. ...E*.R.T.. , |
/* 0040 */ "\xb3\x1a\x11\xa1\x6c\x06\x4d\x39\xa3\x85\x28\xf6\xcc\x30\x0c\x76" //....l.M9..(..0.v |
/* 0050 */ "\xcc\x79\x76\x89\x00\x00\xd8\x7c\xb7\x8a\xfe\x17\x4e\x79\x4d\xe5" //.yv....|....NyM. |
/* 0060 */ "\xca\x1d\xf2\xa4\x71\xcc\x79\x52\x30\x5c\x65\xf8\xfd\xe4\xbc\x38" //....q.yR0.e....8 |
/* 0070 */ "\xee\xbc\x6f\x3e\x43\xbe\x3a\x33\x20\x03\xa7\x82\x4a\x60\x2d\xea" //..o>C.:3 ...J`-. |
/* 0080 */ "\x34\x00\x01\x52\x18\x19\xca\x15\x69\x6a\x34\xa4\x61\x4d\x35\x21" //4..R....ij4.aM5! |
/* 0090 */ "\xa7\x82\x28\xf6\xf3\x30\x08\x76\xf3\x78\xd2\x89\x40\x00\xe6\xbd" //..(..0.v.x..@... |
/* 00a0 */ "\x01\xf1\x99\x80\x93\x75\xaa\x7b\x1a\x84\x79\x98\xba\x03\x47\x68" //.....u.{..y...Gh |
/* 00b0 */ "\x4a\xaf\x42\xe8\x00\xa3\xcf\x63\x1c\xd2\xab\x9a\xa4\xf3\x5e\x9e" //J.B....c......^. |
/* 00c0 */ "\x6d\x13\xcd\xca\x79\xbe\x4f\x38\x69\xe7\x21\x3c\xe6\xa7\x9d\x24" //m...y.O8i.!<...$ |
/* 00d0 */ "\xf3\xae\x03\xf3\xf4\x70\x08\xc3\x86\x1c\x7e\x38\x07\xe8\x00\xe8" //.....p....~8.... |
/* 00e0 */ "\x0c\xfc\x2c\x10\x04\x07\xe7\xda\xe0\x1b\x8f\x40\x47\x89\xc2\x3d" //..,........@G..= |
/* 00f0 */ "\xbd\xe6\x3c\x0f\xcf\xcd\xc0\x3f\x0e\x81\x51\x01\xc1\x3b\xf1\xe1" //..<....?..Q..;.. |
/* 0100 */ "\x48\xe8\x05\xeb\x8f\xf3\xff\x71\xc8\x0c\x50\xfb\xf2\x1d\x8a\x68" //H......q..P....h |
/* 0110 */ "\x40\xb2\x58\x0d\x5c\xe6\x98\x6a\x43\x05\x54\x29\x73\x29\x88\x98" //@.X....jC.T)s).. |
/* 0120 */ "\xcc\x0c\x54\x12\x9c\xbb\x27\x1d\xfb\xc1\x97\xb0\x0d\xc4\x3e\x04" //..T...'.......>. |
/* 0130 */ "\x74\x06\x8e\xfe\xc0\xfe\xee\xce\x62\xe1\x0f\x92\xbc\xf7\x40\x8e" //t.......b.....@. |
/* 0140 */ "\xdc\x13\x76\xf1\xec\x44\xfc\x53\x1d\xaa\x03\xf0\x1e\xd7\x52\x3c" //..v..D.S......R< |
/* 0150 */ "\x01\x83\xae\x20\x3f\xba\xbb\xe9\x27\x5d\xea\xb1\xed\xba\xee\xbd" //... ?...']...... |
/* 0160 */ "\x23\xf0\x19\x87\x6c\x67\x72\x76\x05\xd5\x75\xd0\x77\xcb\x91\x9c" //#...lgrv..u.w... |
/* 0170 */ "\x11\x5f\x1e\x0c\x9e\xf5\x0f\x03\xd9\xe8\x06\x05\xe5\x80\xd8\x0c" //._.............. |
/* 0180 */ "\xef\xf0\xc3\xbf\x80\xa3\xc5\x13\xde\xca\x4f\x00\x30\x39\x9c\x06" //..........O.09.. |
/* 0190 */ "\x77\xc9\xaf\xf0\x0c\xe1\x11\x1c\x1e\x5d\x8b\x74\x27\x00\xa5\x8f" //w........].t'... |
/* 01a0 */ "\x3d\xc0\x63\xf8\x4f\xea\x7b\x17\x30\x7d\xa2\xff\x7f\x9d\x55\x83" //=.c.O.{.0}....U. |
/* 01b0 */ "\xb8\xa9\x29\x71\xe0\x7e\xd6\xeb\x80\x10\x27\x62\x58\xe6\x0b\x18" //..)q.~....'bX... |
/* 01c0 */ "\x33\x12\x3b\x43\xcf\x6a\xf8\xf5\x73\x0a\x7d\x3b\x15\x47\x05\x85" //3.;C.j..s.};.G.. |
/* 01d0 */ "\xc2\xa0\x0d\xe1\xb8\xc3\x39\xcc\x1c\x95\xa1\x47\x9e\x1d\xa2\x39" //......9....G...9 |
/* 01e0 */ "\x00\x01\xbb\x39\x5f\x15\x26\x05\x55\x76\x72\xac\x17\xf4\xaf\x03" //...9_.&.Uvr..... |
/* 01f0 */ "\xf8\x4d\xe1\x28\xc0\xa5\x58\x61\x59\x9d\x83\xbb\x48\x4c\x40\x04" //.M.(..XaY...HL@. |
/* 0200 */ "\xa3\x8a\x4e\xc5\x3d\xa4\x36\xc1\xd6\x71\xc2\xc8\x13\x81\xd4\xd6" //..N.=.6..q...... |
/* 0210 */ "\xfc\x07\x65\x30\x19\x52\x93\x57\x1e\x00\xae\x07\xf1\x7c\xd3\x6b" //..e0.R.W.....|.k |
/* 0220 */ "\x5b\x5f\xcd\x0a\x70\xcf\xae\x0c\xc1\xee\x16\xc6\xcf\xa4\x13\xfe" //[_..p........... |
/* 0230 */ "\x8e\xfb\x2c\x06\xf5\x50\x10\xc6\x3c\x04\x36\xc1\x17\xaf\xa9\xb3" //..,..P..<.6..... |
/* 0240 */ "\x77\xd4\x08\xf1\x13\x8f\xa9\x89\x63\x1b\x7f\xd4\x74\x2c\x8f\x07" //w.......c...t,.. |
/* 0250 */ "\x2c\x78\x2d\x51\xc3\xe8\xe1\xf4\x81\xfe\x35\x7d\x27\x14\x39\xfd" //,x-Q......5}'.9. |
/* 0260 */ "\x33\x82\x33\x5f\x5c\x1d\xc4\xf2\x02\x70\xa0\x23\xf8\xc0\xf6\x07" //3.3_.....p.#.... |
/* 0270 */ "\x52\x79\x80\x13\xee\x35\x00\x13\x03\x2d\xa6\x51\x4c\x13\x96\xc0" //Ry...5...-.QL... |
/* 0280 */ "\x94\x53\x80\x6a\xa3\xf4\xba\xe1\xb8\x47\xc7\x80\x25\xb5\xaf\x0b" //.S.j.....G..%... |
/* 0290 */ "\x6d\x00\x03\x38\x60\x47\x85\x67\x02\x09\x47\x8b\x0e\x9f\x54\x17" //m..8`G.g..G...T. |
/* 02a0 */ "\xc5\xf6\xbf\x8c\xe0\x94\x67\x61\x47\x92\x22\xde\xf6\xeb\xf7\x01" //......gaG."..... |
/* 02b0 */ "\x5e\x39\x7d\x27\x1a\x39\xf5\x94\xe0\xd0\x98\xf0\x95\x4b\x9c\xdd" //^9}'.9.......K.. |
/* 02c0 */ "\xbf\x08\x27\xd0\x6f\xa4\xfc\x33\xe5\x47\x87\xaf\xdd\x4f\x6e\xdd" //..'.o..3.G...On. |
/* 02d0 */ "\xbd\xed\xdf\xf3\x02\x38\xeb\x57\x8a\x27\x8f\xa4\xf8\x32\x7f\x5c" //.....8.W.'...2.. |
/* 02e0 */ "\xf9\x8d\xdb\xe2\xbf\xa0\x3f\xb9\x6c\x22\x94\x88\x7b\x29\x42\xa5" //......?.l"..{)B. |
/* 02f0 */ "\xad\x78\x9e\x02\xfc\xd2\x10\xe4\xa2\xa1\x1a\x49\x7e\xc8\x6f\x90" //.x.........I~.o. |
/* 0300 */ "\x82\x9f\x2a\x52\xf6\x53\x98\x8c\x94\x80\x01\xdc\xb8\x77\x63\x70" //..*R.S.......wcp |
/* 0310 */ "\xc3\x7f\x20\xa8\x43\x08\x43\x29\x4b\x4a\x52\x6b\x5a\x8c\x30\xd2" //.. .C.C)KJRkZ.0. |
/* 0320 */ "\x4a\x71\x46\x1c\xb3\x90\xc0\x03\x6f\x78\x4c\x08\x29\x25\x30\xc1" //JqF.....oxL.)%0. |
/* 0330 */ "\xa7\x82\x84\x24\x52\x94\x10\x88\x10\x84\x18\xc6\x14\xa4\xda\xd6" //...$R........... |
/* 0340 */ "\xe3\x85\x19\x4b\x34\xe3\xcb\x3d\x7e\xff\x5b\xaf\x5d\xe7\xb7\x3c" //...K4..=~.[.]..< |
/* 0350 */ "\x03\x22\x0c\x63\x08\x42\x24\x21\x20\x00\xa1\x8c\x49\x8c\x65\x4a" //.".c.B$! ...I.eJ |
/* 0360 */ "\x50\xe7\x38\x96\x14\x3f\x61\x45\xd8\xc2\x6f\x15\x53\x8e\x53\xde" //P.8..?aE..o.S.S. |
/* 0370 */ "\xed\x6b\x56\x73\x9f\x34\x00\x2c\x63\x14\x73\x9e\x86\xa0\xa0\xf8" //.kVs.4.,c.s..... |
/* 0380 */ "\x00\x6a\x08\x45\x8c\x62\x0c\x62\x50\x85\x4b\x5a\x6d\xc1\x2e\xc9" //.j.E.b.bP.KZm... |
/* 0390 */ "\x94\x50\xd3\x00\xf4\x2d\xef\x69\xad\x72\xa5\x20\x84\x26\x52\x9a" //.P...-.i.r. .&R. |
/* 03a0 */ "\x18\xc1\x00\x03\x08\x40\x21\x0b\x6b\x59\x86\x15\xb7\x43\x9c\xcc" //.....@!.kY...C.. |
/* 03b0 */ "\x3f\x10\x52\x4a\xe3\x0c\xa7\xbd\x96\x31\xfe\x2c\xa0\x42\x00\x84" //?.RJ.....1.,.B.. |
/* 03c0 */ "\x32\x63\x14\x94\xab\x5a\xd7\xe2\x8a\x9e\x14\x73\x7e\x28\x21\x84" //2c...Z.....s~(!. |
/* 03d0 */ "\x42\x94\xac\xe7\x28\xa5\x3c\x1a\xdf\x88\x48\x43\x47\x39\x7d\xc1" //B...(.<...HCG9}. |
/* 03e0 */ "\x49\xec\x2c\x6f\x1d\xd2\xab\x07\x74\x51\x50\x00\x18\xf7\x18\xdc" //I.,o....tQP..... |
/* 03f0 */ "\x47\x00\x4a\xc0\x30\x6c\xea\x65\x14\x8b\x4d\x63\x55\x6a\x14\x7a" //G.J.0l.e..McUj.z |
/* 0400 */ "\xef\x00\x13\x8d\x55\xa2\xd3\x59\x94\x52\x5c\xe7\xd5\xf8\x73\xc4" //....U..Y.R....s. |
/* 0410 */ "\x80\x00\x08\x43\x67\x39\xb5\xad\x53\xde\xea\x71\xc9\xb4\xd6\x75" //...Cg9..S..q...u |
/* 0420 */ "\x85\x52\x8f\xb6\xf7\x9a\x76\x14\x43\x88\x73\x9c\xa9\x4b\xde\xf1" //.R....v.C.s..K.. |
/* 0430 */ "\x3a\xab\x5e\xf0\x95\xa0\xa1\x08\x80\x02\x14\xa5\x73\x8e\x2e\xdb" //:.^.........s... |
/* 0440 */ "\xd6\x69\xd6\xf7\xb8\x31\x8d\x0c\x60\xb1\x8e\x2d\x35\xc3\x4a\x29" //.i...1..`..-5.J) |
/* 0450 */ "\xaa\x6b\x5d\x8c\x61\x94\xa6\x9e\xf6\x71\x85\x8f\x02\x92\x59\x52" //.k].a....q....YR |
/* 0460 */ "\x92\x08\x40\x42\x13\x8c\x32\x85\x5b\xaf\x41\x5c\x61\x81\x08\x48" //..@B..2.[.A.a..H |
/* 0470 */ "\x21\x12\xa5\x21\x49\x2d\xc0\x00\x3f\x80\x62\xa7\x1c\x34\xf4\x3a" //!..!I-..?.b..4.: |
/* 0480 */ "\x84\xda\xd7\x15\x51\x86\x4b\x9c\xc3\x18\xda\x52\xa8\xe7\x24\x84" //....Q.K....R..$. |
/* 0490 */ "\x3c\x35\x40\x12\x2c\x23\xf0\xb3\x9c\xc2\x94\x8b\x18\xe9\xef\x6e" //<5@.,#.........n |
/* 04a0 */ "\x38\x56\xcd\x53\x8e\x18\x7b\x6d\x6b\x5a\x49\x4a\x21\x84\xf0\x09" //8V.S..{mkZIJ!... |
/* 04b0 */ "\x87\x18\x64\x4a\x29\x6b\x5a\x65\x29\x20\x00\x93\x18\xe0\xd4\xe4" //..dJ)kZe) ...... |
/* 04c0 */ "\xa9\x4a\x25\x0a\x52\x04\xff\x40\x84\x29\x73\x99\xa4\x96\x5c\xe6" //.J%.R..@.)s..... |
/* 04d0 */ "\x50\x85\x02\x10\xb1\x8c\x42\x94\xb0\x6e\xf4\x26\xb5\xd1\xce\x48" //P.....B..n.&...H |
/* 04e0 */ "\x30\xa3\xf2\xd6\xb4\x45\x06\xb5\xad\x22\xff\x45\x1c\xe5\x35\xae" //0....E...".E..5. |
/* 04f0 */ "\x96\xb5\x80\xe0\x5c\x3c\x4c\x1e\xe6\x7b\xca\xf0\x95\x74\x10\x7b" //.....<L..{...t.{ |
/* 0500 */ "\xc6\x0a\xc0\x00\xcd\xbd\xe3\x89\xa0\x09\x60\x04\x1d\xf6\xde\x71" //..........`....q |
/* 0510 */ "\x46\xc1\x6c\x82\x9b\x18\xc2\x6b\x5e\x12\xe4\x40\x01\x22\xd3\x52" //F.l....k^..@.".R |
/* 0520 */ "\xc6\x3a\x1f\x09\x65\xbd\xed\x67\x85\xfa\xe7\x1c\x85\x60\xeb\xc1" //.:..e..g.....`.. |
/* 0530 */ "\x42\x14\x4a\x29\x7b\xaf\x4e\xb0\xa3\x52\xce\x72\xc2\x11\x4b\x5a" //B.J){.N..R.r..KZ |
/* 0540 */ "\x34\xc3\x07\x21\x8a\x36\x10\x28\x8b\x5a\xd1\xc5\x1a\x35\x56\x6d" //4..!.6.(.Z...5Vm |
/* 0550 */ "\xc0\xea\x00\xc6\x3f\x95\x1c\x7c\x9f\x28\xb3\x4e\x79\xa7\x54\x00" //....?..|.(.Ny.T. |
/* 0560 */ "\x8b\x18\xc1\x70\xb1\x59\x66\x9c\x80\x02\x03\x0d\x4d\x6b\xf9\x50" //...p.Yf.....Mk.P |
/* 0570 */ "\x19\x83\xf0\xc9\xad\x71\xaa\xb1\xc5\x1a\xd6\xb4\x40\x00\xda\xd7" //.....q......@... |
/* 0580 */ "\x3a\xab\x0c\x51\xae\x16\xe0\x5e\x46\xa6\x2b\x1e\x00\x15\x61\x1d" //:..Q...^F.+...a. |
/* 0590 */ "\xb4\x37\x86\x7c\x58\xc6\x38\xdf\x38\x5b\xbe\x56\x4d\x29\x58\x3c" //.7.|X.8.8[.VM)X< |
/* 05a0 */ "\x08\x1d\x81\xf9\x5c\xbc\x98\x1c\xbe\xa8\x01\x11\x2a\x9f\x17\x73" //............*..s |
/* 05b0 */ "\x4a\x55\x8c\x62\x44\x20\x51\xed\xd4\x5a\x6a\x69\xc6\xa5\x63\x18" //JU.bD Q..Zji..c. |
/* 05c0 */ "\x04\x21\x85\x29\x71\x08\x82\xc6\x39\x31\x89\x00\x01\x0c\x6e\xd6" //.!.)q...91....n. |
/* 05d0 */ "\xb5\x12\x8a\x38\xc3\x29\x7e\xdf\x20\x84\x51\xce\x5b\x5a\xee\x71" //...8.)~. .Q.[Z.q |
/* 05e0 */ "\xcb\x7b\xdf\xf6\x80\xc6\x37\x18\x64\xca\x28\x9a\xd7\x82\xfb\x9f" //.{....7.d.(..... |
/* 05f0 */ "\x29\x12\x8a\x16\x31\xd1\xce\x4b\x18\xe2\x5c\x3f\x9f\xf2\x04\x99" //)...1..K...?.... |
/* 0600 */ "\xfb\x6a\xb8\x05\x68\x43\x82\x39\x8a\xb2\xde\x6b\xf0\x45\xc0\x26" //.j..hC.9...k.E.& |
/* 0610 */ "\x0b\x00\x00\xcd\x38\x04\xe2\x64\x02\x5a\x01\x00\x11\xb5\xad\x20" //....8..d.Z.....  |
/* 0620 */ "\x84\x43\x9c\xfd\x27\x14\xb5\xa6\x1f\x94\xe4\x35\x12\xa5\x26\x10" //.C..'......5..&. |
/* 0630 */ "\x89\x73\x9d\xee\xbd\x57\xf9\x4c\x1a\xf7\x1f\x2a\x21\x4a\x58\x36" //.s...W.L...*!JX6 |
/* 0640 */ "\xaa\x07\x2c\xf3\x69\xaf\xca\x8b\x7b\xd9\xe5\x9e\x2d\x35\x69\x4a" //..,.i...{...-5iJ |
/* 0650 */ "\xa1\x8c\x08\x41\x3e\x54\x24\xf9\x6c\x21\x85\x7e\x0e\xb2\xd4\x94" //...A>T$.l!.~.... |
/* 0660 */ "\xa8\x31\x8b\xcd\x3b\x01\xcf\x80\x2e\x78\x62\x29\x06\x31\x4d\x6b" //.1..;....xb).1Mk |
/* 0670 */ "\x21\x81\xfa\x56\x38\x0d\xc5\x4e\x38\x0b\x07\xfa\x90\x59\x84\xbf" //!..V8..N8....Y.. |
/* 0680 */ "\x45\x8c\x7f\xa2\x08\x6e\x14\x00\x58\x95\x6e\xbd\x47\x39\xf7\xbe" //E....n..X.n.G9.. |
/* 0690 */ "\x9f\x2b\x8b\x8e\x38\x3b\xd1\x40\x84\x2e\x01\x2e\x01\xeb\x80\x00" //.+..8;.@........ |
/* 06a0 */ "\x4b\x14\x5a\x6b\x00\x02\x51\xe3\x00\x6d\x6b\xa9\xe1\x71\x85\xad" //K.Zk..Q..mk..q.. |
/* 06b0 */ "\x6b\xe5\x51\x84\x12\x97\xf2\x90\xe7\x3b\x9f\xca\x92\x8f\x95\x8a" //k.Q......;...... |
/* 06c0 */ "\x29\x4a\x8c\x30\x31\x8c\x03\x07\x10\x0c\x20\x80\x84\x25\x08\x46" //)J.01..... ..%.F |
/* 06d0 */ "\x92\x50\x84\x20\x42\x13\x08\x23\x6f\x70\x05\x2d\x68\x17\x06\xcf" //.P. B..#op.-h... |
/* 06e0 */ "\x30\x0f\xdb\x04\xfa\x3e\xa4\x61\x01\x26\x06\x5e\x2a\xcb\x70\xe8" //0....>.a.&.^*.p. |
/* 06f0 */ "\x6a\xbd\xc0\x12\x38\x05\xb8\x08\xc5\x2d\x6b\x80\xeb\x8a\x85\x16" //j...8....-k..... |
/* 0700 */ "\xf2\x81\xd8\xb8\x00\x07\x83\xd1\xe6\x9d\x26\xf4\xc8\x86\x12\x9a" //..........&..... |
/* 0710 */ "\xd7\x06\x31\xc2\xca\x50\x01\x80\x5c\x9f\x52\x03\x18\xc8\x60\x18" //..1..P....R...`. |
/* 0720 */ "\x01\x00\xca\x53\x85\x70\xe0\x83\xea\xa3\x55\x6a\x3f\x07\xd5\x7e" //...S.p....Uj?..~ |
/* 0730 */ "\xff\xf5\x23\x0a\xf1\xc1\x00\x04\x4b\xea\x4d\xad\x6a\xbf\x7f\x6d" //..#.....K.M.j..m |
/* 0740 */ "\xfa\x92\x29\x25\xfa\x91\x3a\xfa\x93\x3e\xbb\xb6\xb5\xa2\x94\xf3" //..)%..:..>...... |
/* 0750 */ "\xd0\x84\x10\x8f\xa9\x1c\xb3\x22\x7d\x77\xf4\xc4\x00\x00\x46\xaa" //......."}w....F. |
/* 0760 */ "\xdd\x5d\x4d\x78\x4b\x16\x2e\x00\x21\xe4\x30\x7f\x4d\xc0\x47\xa6" //.]MxK...!.0.M.G. |
/* 0770 */ "\xc5\x1e\x87\x9e\x30\x08\x00\xa8\x63\x1a\x8c\x33\xea\xe1\xc1\x8a" //....0...c..3.... |
/* 0780 */ "\x3a\x5d\x38\xd4\x94\x61\x84\x9f\x51\x81\xb6\x22\xd6\xb4\x21\x84" //:]8..a..Q.."..!. |
/* 0790 */ "\xaf\xe7\xea\x55\x9c\xe4\x53\xf4\x5f\xd4\x8b\x5a\xd7\xd4\xc3\x84" //...U..S._..Z.... |
/* 07a0 */ "\xf5\x7d\x4a\x71\x86\x72\xca\x8f\xa9\x1e\x0c\x85\x18\x62\x4a\x56" //.}Jq.r.......bJV |
/* 07b0 */ "\x17\x6d\x4b\x49\x08\x79\x36\xb0\xd3\x6b\xde\x9c\x3e\xf1\xa2\xe0" //.mKI.y6..k..>... |
/* 07c0 */ "\x00\x3b\x0f\x38\x97\xbc\x6a\xbe\x00\x5e\x01\x19\xad\xd7\xa9\x6b" //.;.8..j..^.....k |
/* 07d0 */ "\x52\xe7\x3a\xbc\xf6\x05\x81\xec\x90\xa5\x2b\x7b\xdc\x5a\x6b\x43" //R.:.......+{.ZkC |
/* 07e0 */ "\x18\x5d\xb7\xf8\x36\x15\x65\xa0\x84\x31\xeb\xbf\x36\x38\x14\x92" //.]..6.e..1..68.. |
/* 07f0 */ "\xa2\x94\xd7\xef\xf4\x73\x99\x94\x52\xa7\x1c\x83\x18\x90\xc6\x19" //.....s..R....... |
/* 0800 */ "\x4a\x6e\x78\x63\x39\x67\x39\x9f\x5d\xc9\x08\x4d\x30\xc9\x88\xa1" //Jnxc9g9.]..M0... |
/* 0810 */ "\x09\xf6\xdf\x63\x18\x75\x96\xf2\x10\x83\xcb\x3c\x88\x42\x6d\x35" //...c.u.....<.Bm5 |
/* 0820 */ "\xa7\xbd\xca\x94\xaf\x75\xe6\x10\x8b\xbc\xf6\x1c\xe7\x02\x10\xc2" //.....u.......... |
/* 0830 */ "\x2c\x80\xf8\xbb\x8c\x32\x8e\x73\x84\x1b\x1d\x9f\x11\x28\xa4\x9b" //,....2.s.....(.. |
/* 0840 */ "\xa8\x32\x5d\xf6\x55\x38\xe3\x29\x4f\x4b\xd8\x91\x08\x5c\xe3\x9c" //.2].U8.)OK...... |
/* 0850 */ "\xd2\x23\x9f\x14\x0a\x49\x49\x08\x78\x35\xa3\x40\x9e\x1b\x58\xf9" //.#...II.x5.@..X. |
/* 0860 */ "\xfe\x03\x18\xc9\x08\x45\x30\x9c\x1b\x91\x2c\x26\xf1\x1a\x48\x84" //.....E0...,&..H. |
/* 0870 */ "\x3a\x18\x44\x1b\x9c\xf1\x84\x22\x9e\xf7\x0a\x52\x93\x18\xc4\xa2" //:.D...."...R.... |
/* 0880 */ "\x88\x21\x16\xf7\xb4\x94\xa6\x10\x86\x00\xef\x5e\xe5\xd2\x9a\x30" //.!.........^...0 |
/* 0890 */ "\x79\x0c\x97\x80\x01\xc9\xb8\x72\xe3\x24\x01\x30\x00\x09\xee\x36" //y......r.$.0...6 |
/* 08a0 */ "\xee\xd5\x38\x07\xd7\x40\x9c\x22\x3a\xc2\x07\xc1\xec\x14\xf8\xed" //..8..@.":....... |
/* 08b0 */ "\xff\x3d\x5a\xc3\xff\x30\xdb\xda\x0e\x3b\x19\x4f\x43\xe7\xd1\x8e" //.=Z..0...;.OC... |
/* 08c0 */ "\x05\xac\x03\xe7\x50\x26\x1f\xe1\x87\x19\x07\xd3\xe5\x93\x76\x64" //....P&........vd |
/* 08d0 */ "\x7d\x3c\xbf\xcc\xf6\xa7\x9e\x40\xaf\x05\x46\x01\xa8\x8e\xed\x7b" //}<.....@..F....{ |
/* 08e0 */ "\x7d\x68\xe7\x84\xee\x21\x61\x08\x58\x08\xf4\x00\x0f\xe5\x86\x13" //}h...!a.X....... |
/* 08f0 */ "\x74\x9e\x30\xf1\xdd\xb2\x70\x52\x44\x94\x21\x77\x6a\x9c\x0c\x94" //t.0...pRD.!wj... |
/* 0900 */ "\x0a\xe1\x46\xfd\xcf\xff\x7e\x70\x8f\x3d\x3c\x27\x5d\x6f\x9f\x52" //..F...~p.=<']o.R |
/* 0910 */ "\x3c\x53\x3d\xc0\x27\xc3\x13\xdd\xa3\x0f\xb7\x1e\x5c\x0f\xf7\x67" //<S=.'..........g |
/* 0920 */ "\xe8\xfc\x11\x2e\xee\x77\x1e\x7a\x9f\xc1\x3e\x7f\x9f\x0d\xcf\xb7" //.....w.z..>..... |
/* 0930 */ "\xe3\xda\x78\x78\x16\x7b\xb2\xf6\x7e\xf8\xee\xd1\x03\x7e\x17\x77" //..xx.{..~....~.w |
/* 0940 */ "\x5d\x8e\x30\xaf\xae\xc0\x50\x61\x9f\x8e\xee\xbf\x14\xb5\xa1\x0f" //].0...Pa........ |
/* 0950 */ "\x36\xcc\x07\xcd\xa9\x52\x97\x76\xa0\xc7\x07\x98\x30\x00\x03\x4f" //6....R.v....0..O |
/* 0960 */ "\xe2\xdb\x8a\x68\x01\x8e\x01\xed\xe7\x0e\x06\xc0\x3a\xf8\x7b\x75" //...h........:.{u |
/* 0970 */ "\x82\x75\x18\x06\x3e\x30\x0c\xb3\x00\xc9\x30\x0c\x7c\x60\x19\x66" //.u..>0....0.|`.f |
/* 0980 */ "\x09\xe6\xc7\x04\xbc\x30\xc1\x2e\x5e\x22\x83\x10\x00\x3e\xa6\x2f" //.....0..^"...>./ |
/* 0990 */ "\x11\x5b\x48\x01\x97\x4e\x46\x01\xf4\xe5\x04\xc1\x40\x8c\x20\x4d" //.[H..NF.....@. M |
/* 09a0 */ "\xe1\x0f\x04\x7d\x21\x00\x0e\x02\x01\x9c\x30\x1b\xdd\x4e\x01\xc0" //...}!.....0..N.. |
/* 09b0 */ "\x41\xf3\x50\x01\x32\x00\x03\x07\xe2\xb9\x88\x28\x09\x9e\x02\x0f" //A.P.2......(.... |
/* 09c0 */ "\xa8\x7d\x0e\x80\x27\x41\x91\xc0\x4a\xe8\xbe\x43\x88\x96\x17\xef" //.}..'A..J..C.... |
/* 09d0 */ "\x91\x50\x48\x0c\x43\xeb\x15\x11\x6a\x00\x46\x45\x58\x13\xfe\x30" //.PH.C...j.FEX..0 |
/* 09e0 */ "\x8c\xd8\x57\x01\x1c\x30\x8d\xd4\x60\xf6\xcc\x3c\x61\x18\x60\x2a" //..W..0..`..<a.`* |
/* 09f0 */ "\x64\x42\x18\x06\x2e\x79\xd6\xba\x0a\xc0\x0f\x80\xe3\xcb\xe9\xfe" //dB...y.......... |
/* 0a00 */ "\x14\x97\x8e\x04\x8e\x18\xa1\xec\x1d\x6a\xc0\x37\xe1\xee\xb9\x8e" //.........j.7.... |
/* 0a10 */ "\x3c\xa1\xdf\x9d\x7c\x1d\x4c\x5f\x3c\x39\x83\xe2\xaf\x02\x33\xb9" //<...|.L_<9....3. |
/* 0a20 */ "\x8c\xb0\x69\x23\x07\xd3\xd6\x01\xe4\x61\x5e\x77\x8b\x20\x83\xcf" //..i#.....a^w. .. |
/* 0a30 */ "\x2f\x0b\xc1\x21\x08\xe6\x0f\xf1\x98\xb4\xd7\x04\x72\x86\x30\x17" ///..!........r.0. |
/* 0a40 */ "\x4c\x08\x32\x94\xe1\xaf\xc5\x9c\xe7\x05\xdf\xc6\x0b\xff\x9c\x38" //L.2............8 |
/* 0a50 */ "\xb8\xf0\x18\x67\x8e\xf4\x0f\x70\xe3\x74\x79\x81\x8f\x9b\x38\x0a" //...g...p.ty...8. |
/* 0a60 */ "\x15\xe3\x39\x85\x29\x34\x7d\x40\xbf\x0e\x17\xcd\x74\xc5\x60\x0b" //..9.)4}@....t.`. |
/* 0a70 */ "\x00\x0f\x27\x69\x33\x00\x03\xb6\xfb\x6b\x1b\x58\x09\xa6\x21\x25" //..'i3....k.X..!% |
/* 0a80 */ "\xa3\x8a\x35\x03\x84\x63\xcc\x51\x87\xb2\x43\xb8\x58\x38\x79\x67" //..5..c.Q..C.X8yg |
/* 0a90 */ "\x60\xc4\x54\x41\xbc\x0c\x62\xe0\x6d\x03\x5c\x08\x4c\x00\xbc\x46" //`.TA..b.m...L..F |
/* 0aa0 */ "\x21\x66\x9c\xc0\x0d\xed\xa6\x1d\x8c\x60\x97\x45\x4e\x39\x86\x46" //!f.......`.EN9.F |
/* 0ab0 */ "\x60\x5b\xf7\xb0\xac\x24\x70\xa3\x30\x04\xe1\x92\x81\x0f\xbf\x20" //`[...$p.0......  |
/* 0ac0 */ "\x52\x05\x53\x8e\x72\x1f\x78\x2e\x1f\x6d\xec\x01\x77\xfb\xf8\x9f" //R.S.r.x..m..w... |
/* 0ad0 */ "\x80\xe0\x32\xff\x7f\x12\xf4\xbb\x0a\xf3\x40\x8e\x1f\x44\x7e\x1c" //..2.......@..D~. |
/* 0ae0 */ "\xb0\xbc\x22\x80\x01\xe9\x87\xcb\x0f\x7a\xe9\x77\x9e\xb0\x72\xee" //.."......z.w..r. |
/* 0af0 */ "\x60\x28\xf5\x3c\xd2\x4b\x4e\x91\x61\xd1\xb8\x41\x08\xc3\x63\xf1" //`(.<.KN.a..A..c. |
/* 0b00 */ "\xc0\x29\x40\x0c\x4d\x6b\xe0\x30\x91\x08\x30\x62\x74\x90\xef\x1c" //.)@.Mk.0..0bt... |
/* 0b10 */ "\xb3\x00\x17\x01\xb7\xac\x0c\x18\x59\x8c\x37\x86\x44\x2f\x80\x19" //........Y.7.D/.. |
/* 0b20 */ "\x83\x0f\xe7\x06\xb9\x63\x98\x30\xf6\xa0\x67\x0c\x9e\x5f\x26\x33" //.....c.0..g.._&3 |
/* 0b30 */ "\x00\x37\xba\xa2\xd6\xb4\xc0\x0f\xed\xe7\x82\x1c\x8a\xc3\xdc\xc4" //.7.............. |
/* 0b40 */ "\x4c\x1d\x91\x02\xdc\x94\x4c\x18\x3d\xf5\x81\x09\x23\x94\x3b\x80" //L.....L.=...#.;. |
/* 0b50 */ "\x24\x30\x05\x00\x61\xcc\xb7\x41\xc8\xf9\xeb\x39\xcb\x00\x58\x06" //$0..a..A...9..X. |
/* 0b60 */ "\x16\x0c\x60\x01\x68\xf9\xee\xe8\x00\xcc\x08\x1f\x1e\x0c\x61\x95" //..`.h.........a. |
/* 0b70 */ "\x70\x8f\xf5\x72\x2a\x0f\x72\x66\x73\xed\x81\x0f\x9e\xfe\xb2\x50" //p..r*.rfs......P |
/* 0b80 */ "\x84\xc0\x1f\xf3\xdd\xa5\x00\x03\xf8\x58\x7e\x2e\x14\x0c\x12\x6b" //.........X~....k |
/* 0b90 */ "\x1c\x3b\x9c\xb3\x87\x38\x56\x21\x80\xb6\xe0\x86\x00\x22\xa3\x80" //.;...8V!.....".. |
/* 0ba0 */ "\xb7\x2e\xff\x7f\xc0\x5b\x7f\x30\x43\x70\x40\xc1\xb9\x32\x68\x00" //.....[.0Cp@..2h. |
/* 0bb0 */ "\x07\x41\xe4\xb7\x8c\xfc\x00\xd7\x0c\x18\xab\xfc\x15\x55\xba\xf1" //.A...........U.. |
/* 0bc0 */ "\x14\x04\x42\x08\x2f\x70\x12\x04\x80\xcf\xae\xe2\x58\x5b\xe1\x42" //..B./p......X[.B |
/* 0bd0 */ "\x65\x29\xe7\x02\x17\xa0\x01\x9a\x49\x58\x30\xe4\x4a\x05\xd9\x5e" //e)......IX0.J..^ |
/* 0be0 */ "\x28\x01\x18\x23\x28\x0d\xd5\xa8\xce\x58\xc7\x70\x80\x04\x60\x29" //(..#(....X.p..`) |
/* 0bf0 */ "\x6c\x86\x28\xd8\x0a\x5b\xea\x6e\xf3\xdc\x05\x2d\xa7\xa5\x28\xd4" //l.(..[.n...-..(. |
/* 0c00 */ "\x00\x0e\x2b\xe3\x17\xa5\x2a\x26\x00\x6d\xde\x19\x70\x40\x04\xa0" //..+...*&.m..p@.. |
/* 0c10 */ "\x18\x26\xb5\xe1\x88\x21\x63\x1c\x63\x01\x06\xed\x7f\xc2\xbe\x8c" //.&...!c.c....... |
/* 0c20 */ "\x04\x19\xdc\x43\x00\x07\xc0\x41\xbe\x37\x48\x27\x54\x06\x02\x03" //...C...A.7H'T... |
/* 0c30 */ "\x8f\x95\x18\x08\x37\xaf\xcd\x14\x6c\x00\x07\x13\xf3\x46\x31\x10" //....7...l....F1. |
/* 0c40 */ "\x03\x7f\x34\x60\xe3\x0c\xf3\x42\xa7\x1c\x8a\xe0\x16\xbf\x28\x87" //..4`...B......(. |
/* 0c50 */ "\x0d\x73\x30\x0b\x5d\x60\x2d\xb6\x01\x6c\xf3\x30\x0a\xc4\x60\x16" //.s0.]`-..l.0..`. |
/* 0c60 */ "\xc8\xc0\x0f\xf1\x88\xf9\x23\x00\xb6\x69\x70\x41\xf3\x34\x9b\x80" //......#..ipA.4.. |
/* 0c70 */ "\x01\xcc\xbc\xcd\x8c\xc0\x03\x09\x66\x83\x79\x4d\x8c\xbc\xd4\xa3" //........f.yM.... |
/* 0c80 */ "\xb8\xc3\x30\xee\x3a\x80\x2a\xeb\x2d\x01\xa0\x61\xb2\x15\x1f\x07" //..0.:.*.-..a.... |
/* 0c90 */ "\xdb\x86\x28\xcc\x03\x1f\xe8\x8e\x11\x82\x5f\x00\x18\x0b\xb1\x53" //..(......._....S |
/* 0ca0 */ "\x8e\x00\xc0\x07\xb0\x06\xb4\x78\x5e\x30\x07\x57\x17\xd4\x8c\x01" //.......x^0.W.... |
/* 0cb0 */ "\xd3\xf3\xfc\x0a\xe8\xe0\x00\x0e\x6f\xce\x9f\x19\xa8\x09\xc8\x01" //........o....... |
/* 0cc0 */ "\x00\x17\x3a\x58\xe7\x58\xc3\x00\x09\xf5\xdf\x09\x4e\xc6\x1c\x71" //..:X.X......N..q |
/* 0cd0 */ "\x1e\x59\xf0\x87\xd4\xf2\x36\x8e\xc6\x4b\xfd\xff\x05\x26\xbe\x3c" //.Y....6..K...&.< |
/* 0ce0 */ "\x2c\xe0\xa5\x97\x3a\x28\xf9\x91\xe7\xe0\xfe\x18\x3b\x7e\x8b\xed" //,...:(......;~.. |
/* 0cf0 */ "\xc8\x1f\xc3\x94\x17\x5f\xfb\x71\xe0\x58\xfb\xb8\xff\xeb\x8b\x60" //....._.q.X.....` |
/* 0d00 */ "\xeb\xa7\xde\x70\x36\x7d\xed\x5f\x6d\xfc\xe8\xa2\x9e\x09\x87\x01" //...p6}._m....... |
/* 0d10 */ "\xe1\x80\x42\xff\x7f\xa7\xf3\x3b\x01\x10\xf3\x9e\x72\xfc\x2e\xfe" //..B....;....r... |
/* 0d20 */ "\x55\x37\x88\xb8\x47\xf4\x3c\x0d\xb3\x8f\x9a\x5f\x39\x83\x18\x1f" //U7..G.<...._9... |
/* 0d30 */ "\x56\x1d\x65\xbf\x1d\xe7\xea\xf9\xcf\xd5\x78\x3e\x26\x31\x10\xfb" //V.e.......x>&1.. |
/* 0d40 */ "\xc4\xf7\xe7\xc8\xf8\xef\x2e\x07\xfb\x28\x3e\x64\xff\x69\xfc\x38" //.........(>d.i.8 |
/* 0d50 */ "\x8c\x60\xfc\x27\xc7\x60\xfb\xa8\xe1\x2e\xf9\x93\xe0\x28\xed\x44" //.`.'.`.......(.D |
/* 0d60 */ "\xf9\x93\xe3\x89\xfd\xb8\xe0\xab\xc0\xff\x2d\x17\xc5\x60\x49\xc0" //..........-..`I. |
/* 0d70 */ "\x78\x17\xcb\x81\x08\xb6\x01\x9b\xf3\x08\x63\x78\x06\x6e\x2f\xf7" //x.........cx.n/. |
/* 0d80 */ "\xfc\x03\x37\xe8\x61\x22\xf8\xe1\x15\x47\x20\x00\x77\x6e\x29\xf8" //..7.a"...G .wn). |
/* 0d90 */ "\xdd\x00\x0e\xb0\xcc\x46\x65\x74\x5e\xd2\x13\x5a\x89\xe0\x12\x98" //.....Fet^..Z.... |
/* 0da0 */ "\xee\x1c\xec\xf8\xc3\x2a\xf4\x33\x6f\xed\x27\x10\x1a\xec\x2c\x57" //.....*.3o.'...,W |
/* 0db0 */ "\xaa\x64\x60\x14\xa0\xaf\x67\xc5\x40\x0b\x07\x6f\x3b\x13\xbb\x49" //.d`...g.@..o;..I |
/* 0dc0 */ "\xc7\xc7\x0c\x76\x7c\x60\xc3\x6a\xec\x29\x18\x30\x70\xab\x2d\xb8" //...v|`.j.).0p.-. |
/* 0dd0 */ "\x46\x01\x60\x3e\xb9\x0f\xe0\xbb\x38\xe0\x51\x1f\x31\xec\x58\xbb" //F.`>....8.Q.1.X. |
/* 0de0 */ "\xfc\xea\xe0\x6d\x7c\x00\x27\x85\xed\x7c\xe7\x02\xf8\xf6\xb4\x0f" //...m|.'..|...... |
/* 0df0 */ "\xf1\x9b\x61\x0e\x07\x68\x58\xf8\xa1\x8a\x2f\x8d\x9d\x6a\x1d\x7e" //..a..hX.../..j.~ |
/* 0e00 */ "\xc3\x7c\xfe\x9d\xa0\xab\xb4\x48\x77\x00\x4b\x14\x61\x9d\xa0\xa5" //.|.....Hw.K.a... |
/* 0e10 */ "\x56\x5a\x3d\xd9\xc6\x30\x05\x44\x60\x0a\x39\xf9\x93\x80\x7a\xf1" //VZ=..0.D`.9...z. |
/* 0e20 */ "\xc0\x1a\xd3\xd1\xed\xf7\xe3\x00\x9d\x85\xbd\x93\x07\x58\x00\x57" //.............X.W |
/* 0e30 */ "\x63\xbc\x00\xef\xd1\x82\xe6\xde\xc8\x61\x36\x5f\x00\x78\x23\x10" //c........a6_.x#. |
/* 0e40 */ "\x63\x3e\x83\xee\xde\x1a\x2c\xeb\xa0\x61\x5a\xa9\x3b\x00\x03\xc0" //c>....,..aZ.;... |
/* 0e50 */ "\xab\x8f\x1f\x5b\x68\x01\xe6\x00\x93\xe1\xbb\xcf\x69\x62\x10\x4b" //...[h.......ib.K |
/* 0e60 */ "\x80\x00\x16\x0c\x10\x60\xc1\x89\xc1\x84\x31\x88\xcf\x63\x15\x2a" //.....`....1..c.* |
/* 0e70 */ "\xd5\x6f\x05\xe2\x1b\x5a\xc0\xa0\x35\x8b\x2d\x2d\xb8\xd0\x43\xfb" //.o...Z..5.--..C. |
/* 0e80 */ "\xce\x23\x7f\x8c\x01\x90\x5e\x71\xb8\x54\x01\xf8\x9f\x30\x08\xfc" //.#....^q.T...0.. |
/* 0e90 */ "\x60\x0a\x39\xc1\x7c\x8c\x22\x58\xc3\x49\xda\x53\xb1\x0e\x65\x08" //`.9.|."X.I.S..e. |
/* 0ea0 */ "\x01\x1e\x00\x01\xe0\x93\x07\x91\x6d\xec\x00\xf7\x00\x4b\xb8\x66" //........m....K.f |
/* 0eb0 */ "\xe3\xc2\x1a\x3e\x6b\x01\x9e\x56\x00\x5a\x79\x36\x68\x18\x8c\x85" //...>k..V.Zy6h... |
/* 0ec0 */ "\x88\xf9\x60\x83\x07\xc5\x30\x76\xfb\x15\x1f\x46\x0e\xff\x80\x56" //..`...0v...F...V |
/* 0ed0 */ "\xe3\x48\x09\x8d\x32\x80\x27\x19\x4b\xa8\x01\x28\x19\x62\xa9\xb9" //.H..2.'.K..(.b.. |
/* 0ee0 */ "\x6d\x6b\xa0\x02\xaf\x38\xe0\x0e\x39\x93\x8b\xd4\x00\xa0\x98\x02" //mk...8..9....... |
/* 0ef0 */ "\xc9\xe0\x56\x14\x33\x5f\x01\xfc\x00\xc3\x71\xe0\x01\xcc\x43\xb3" //..V.3_....q...C. |
/* 0f00 */ "\x5f\x59\xe5\x35\x00\x0f\xda\xb0\x7a\x00\x06\xab\xcc\x9b\x15\x30" //_Y.5....z......0 |
/* 0f10 */ "\x13\xec\x00\x1b\x58\x9a\x85\xe2\x47\x93\x5e\x41\x8c\x54\x89\xaf" //....X...G.^A.T.. |
/* 0f20 */ "\x8f\x20\x37\x00\x27\x00\xb0\x7b\xda\x80\x21\xe3\x36\x2a\x90\xc3" //. 7.'..{..!.6*.. |
/* 0f30 */ "\x12\x37\xcd\x88\x13\x0d\x2f\xc0\xec\x0d\x3a\x18\x07\x67\xc3\x5d" //.7..../...:..g.] |
/* 0f40 */ "\x41\xc8\x11\x5c\x96\x41\x77\x62\xf9\x41\xb8\x0d\x9e\x7a\xc9\xf2" //A....Awb.A...z.. |
/* 0f50 */ "\x03\x30\x20\xf8\x25\x0e\xbc\xcb\xf2\x83\x10\x26\xd8\x63\x0d\xbd" //.0 .%......&.c.. |
/* 0f60 */ "\x9b\xe5\x05\xe0\x41\x5f\x98\xcf\xf2\x82\xd0\x1b\x34\xf5\xa7\xe4" //....A_......4... |
/* 0f70 */ "\x05\x3f\x32\x47\x56\x6a\xf9\x41\x38\x12\x74\x30\x86\xce\xd7\xf2" //.?2GVj.A8.t0.... |
/* 0f80 */ "\x82\x50\x29\x6c\x52\x01\x77\x6c\xf9\x41\x18\x11\x54\x96\x81\x7f" //.P)lR.wl.A..T... |
/* 0f90 */ "\x6d\xb0\x79\x25\x88\x35\x2e\x80\x01\x80\xa6\x12\xca\xb3\x7d\x82" //m.y%.5........}. |
/* 0fa0 */ "\xfe\xc0\x1f\x87\x60\xd5\xdb\xfc\x3e\xef\x4f\x6e\x08\xc4\x21\x55" //....`...>.On..!U |
/* 0fb0 */ "\x85\x70\x00\x41\x08\x40\x34\xdb\x7e\x17\x4b\x9c\x2f\x77\x0e\x7e" //.p.A.@4.~.K./w.~ |
/* 0fc0 */ "\xc5\x05\xfa\x8c\xe9\xf2\x18\x75\xfa\x7e\x43\x06\x57\xc6\x0c\xc1" //.......u.~C.W... |
/* 0fd0 */ "\x00\x03\xe0\x08\xd5\xee\xbd\x85\xe2\x60\x1b\x00\x85\x17\x6d\xf8" //.........`....m. |
/* 0fe0 */ "\x10\xa2\x01\x7e\xa1\x76\xde\xf3\xf2\xa3\x00\x86\x9e\xbe\x40\x13" //...~.v........@. |
/* 0ff0 */ "\xf7\x1e\xfe\x40\x0f\xff\x1f\x3e\x43\x0e\xef\x43\xef\xc8\x00\xf8" //...@...>C..C.... |
/* 1000 */ "\x78\xfd\xf2\x18\x42\xda\x1f\xfe\x44\x30\x00\x7c\x66\x06\x1f\x93" //x...B...D0.|f... |
/* 1010 */ "\xe0\x00\x3e\x04\x8e\x05\x16\x00\x04\x7f\x80\xed\x18\x84\xec\x43" //..>............C |
/* 1020 */ "\xc3\xda\x78\xdd\x97\x6d\xfc\xf3\x60\xdf\x0d\xa6\xc0\xe4\x10\x3e" //..x..m..`......> |
/* 1030 */ "\x08\x53\x25\x81\xd5\xb0\xe1\x00\x67\xe5\x9a\x25\xb4\x2a\xf4\x82" //.S%.....g..%.*.. |
/* 1040 */ "\xee\xc7\xeb\x80\xbc\x0c\x2a\x4b\x43\xaf\x50\x2f\xfe\xf4\x81\x48" //......*KC.P/...H |
/* 1050 */ "\x1c\x32\x28\x89\x4e\x80\x5d\xfd\xdd\x02\x30\x30\xf8\x24\x9e\x6a" //.2(.N.]...00.$.j |
/* 1060 */ "\x43\x67\xf6\xe4\x07\xc0\xe9\x71\xc9\x1d\x5a\x81\x7f\x6e\xf9\x80" //Cg.....q..Z..n.. |
/* 1070 */ "\xce\x08\xeb\x34\xf5\x82\xde\xe3\xf3\x01\x50\x39\x64\x53\xec\x43" //...4......P9dS.C |
/* 1080 */ "\x9d\x82\xff\xce\x8d\x80\x19\xa3\x98\x33\x8c\xfb\x07\x44\xce\x27" //.........3...D.' |
/* 1090 */ "\x02\x5b\xa7\xa4\x56\x77\x6f\x7f\xb0\x0e\xd2\x05\x10\xbb\xc0\xae" //.[..Vwo......... |
/* 10a0 */ "\xef\x1f\xd0\x07\x03\xbf\x5f\xda\xed\xf2\x0b\xbb\xd7\xf4\x44\xb1" //......_.......D. |
/* 10b0 */ "\x5a\xcd\xf0\xbc\x88\x1d\xbe\x39\x04\xf0\x59\xf3\xeb\xbd\x09\x18" //Z......9..Y..... |
/* 10c0 */ "\xac\xd2\x42\xe0\x42\x10\x28\x6f\x88\x8f\xf6\x0b\xf8\xef\x07\x34" //..B.B.(o.......4 |
/* 10d0 */ "\x0e\xac\xc0\x03\x60\xbf\x8d\xc0\xcb\xa0\x88\x94\xea\x05\xfd\x80" //....`........... |
/* 10e0 */ "\x0a\xc1\x7f\x19\xf3\x28\x16\x8d\x20\xbb\xb0\x02\x58\x2f\xe2\xff" //.....(.. ...X/.. |
/* 10f0 */ "\x31\x46\x9e\xc0\x2e\xec\x00\xd6\x0c\x04\x76\x49\x96\xb0\x5b\xd8" //1F........vI..[. |
/* 1100 */ "\x02\x2c\x18\x08\xc1\x6b\x8e\x10\xd9\xd8\x02\xac\x17\xf1\x2f\x01" //.,...k......../. |
/* 1110 */ "\x28\x75\xe6\x00\xcb\x05\xfc\x43\xf5\x2c\xd3\xdf\x01\x03\x6c\x17" //(u.....C.,....l. |
/* 1120 */ "\xf0\xf8\x15\xaa\x38\x81\x6f\x60\x0f\xb0\x5f\xc3\x60\x45\xd0\x4f" //....8.o`.._.`E.O |
/* 1130 */ "\x11\x04\x6c\x17\xf0\xc8\x13\x44\xb7\xc2\x81\x2b\x05\xfc\x2b\x88" //..l....D...+..+. |
/* 1140 */ "\x7a\xc5\x5e\x60\x14\xb0\x5f\xc2\x3f\xa6\x02\xb6\x0b\xf8\x3c\x0a" //z.^`.._.?.....<. |
/* 1150 */ "\x99\x1c\x61\xb7\xb0\x0b\xd8\x2f\xe0\xd0\x10\x6a\x33\x00\xcd\x82" //..a..../...j3... |
/* 1160 */ "\xfe\x09\x02\x6e\x86\x00\xd9\xd8\x06\xac\x17\xf0\x38\x12\x97\xa7" //...n........8... |
/* 1170 */ "\x4c\x03\x76\x0b\xfc\x0c\x42\x09\x39\xab\x4a\x6e\xec\x03\xb6\x0b" //L.v...B.9.Jn.... |
/* 1180 */ "\xfc\x0a\x59\x14\x90\xdd\xee\x6f\xd8\x76\x9d\x44\x6e\x08\xa4\x36" //..Y....o.v.Dn..6 |
/* 1190 */ "\x84\x51\x9b\x82\x20\x45\x96\x7a\x03\xdd\x11\x02\x19\x43\x60\x86" //.Q.. E.z.....C`. |
/* 11a0 */ "\x50\xe0\x11\x4a\x24\x82\x29\x43\xa0\x47\x20\x26\xb6\x7a\x10\xc8" //P..J$.)C.G &.z.. |
/* 11b0 */ "\x04\x06\x1b\x35\xa1\x50\x23\x9f\x4a\x20\x11\x4f\xbc\xfc\x90\x41" //...5.P#.J .O...A |
/* 11c0 */ "\x04\xa1\x80\x41\x20\x20\x10\x4c\x0b\x04\x80\xc6\xee\xf8\x3b\xe9" //...A  .L......;. |
/* 11d0 */ "\xcb\xdd\x42\x06\x00\xc0\x2b\x2c\x1c\xe3\x13\x28\x0c\xf7\xa1\x43" //..B...+,...(...C |
/* 11e0 */ "\x9b\xa2\xe1\x6d\xe9\xb3\x27\xe0\x00\x68\xbc\xb3\x71\x43\xc5\xd3" //...m..'..h..qC.. |
/* 11f0 */ "\x8e\x0f\xc4\x4a\x72\x0d\xbe\x81\xe5\xf4\x04\x86\xeb\x40\x02\x00" //...Jr........@.. |
/* 1200 */ "\xbc\x7a\xea\x00\x2c\x10\x10\x1b\xad\xc1\xfc\x06\x03\x61\xba\x5c" //.z..,........a.. |
/* 1210 */ "\x27\xe0\x30\x1a\x0d\xd2\xe1\x52\x06\xa1\x58\x1e\xe8\x94\x07\xba" //'.0....R..X..... |
/* 1220 */ "\x1c\x81\xce\x80\x29\x00\x73\x80\x80\xe2\x25\x50\x04\x62\xe7\x8a" //....).s...%P.b.. |
/* 1230 */ "\xc3\x58\x0f\x8a\x70\x9f\x18\xbb\xc3\x18\x96\x00\x20\x80\x7d\xe5" //.X..p....... .}. |
/* 1240 */ "\xe2\x02\x45\x6c\xa0\x7b\x05\x85\xa2\xc0\x36\x7b\x2c\xf6\x19\x14" //..El.{....6{,... |
/* 1250 */ "\xe5\x9c\xb0\x5f\xee\x00\x01\x7f\x71\x18\x80\xd8\x98\x68\x03\xb4" //..._....q....h.. |
/* 1260 */ "\x08\xe7\x80\x71\x41\x00\x03\xa9\x72\xd6\xc6\xa0\x02\x84\x01\xc5" //...qA...r....... |
/* 1270 */ "\x65\x40\x07\xe2\xd0\xe1\x5f\x44\x00\x3c\x5c\x6c\x86\x08\x81\x6f" //e@...._D.<.l...o |
/* 1280 */ "\x24\xa3\x4a\xe2\x9c\x5d\xe3\x8b\x1a\x08\xc1\x6f\x44\xa7\x52\xe3" //$.J..].....oD.R. |
/* 1290 */ "\x9f\x3a\xe0\x70\x0b\xbb\x1f\xce\x62\xc7\xd7\xcc\x20\x5b\xec\xb3" //.:.p....b... [.. |
/* 12a0 */ "\xd4\xd8\x66\x2d\x46\x7e\x65\x02\xff\x42\xaf\x14\x49\x7f\x36\x0c" //..f-F~e..B..I.6. |
/* 12b0 */ "\x1e\x02\x36\x7d\xa7\xcc\x80\x1b\x3b\x44\xb7\xec\x3f\x34\x40\xe4" //..6}....;D..?4@. |
/* 12c0 */ "\x17\x74\x4a\x76\x17\x77\x7c\xc8\x83\x50\x2f\xf6\x3a\xf4\xbe\x6a" //.tJv.w|..P/.:..j |
/* 12d0 */ "\x39\xc5\xdf\x6c\x0a\x20\x55\x96\xf1\x69\x40\x98\xed\xee\x56\xfe" //9..l. U..i@...V. |
/* 12e0 */ "\x2f\x51\xc5\xa5\x03\x61\xb7\xa7\x5b\xef\xd8\x11\x55\xfd\x9e\x1a" ///Q...a..[...U... |
/* 12f0 */ "\x03\x51\xa6\x18\x5c\xad\xcd\xbd\x85\x85\xe3\x15\x1c\x4b\xf7\xd8" //.Q...........K.. |
/* 1300 */ "\x48\x2e\xe4\x94\x62\xe8\x21\xdd\x43\x0b\x1a\xc6\x21\x4a\x08\x41" //H...b.!.C...!J.A |
/* 1310 */ "\x18\x00\x3e\xb2\x81\x7f\x25\xa3\x61\x7c\x97\xfa\x99\x05\xde\xcb" //..>...%.a|...... |
/* 1320 */ "\x3d\xf2\x46\x10\x0b\xbc\x9a\x6b\xf3\x30\x40\xb7\xa3\xf9\x83\x08" //=.F....k.0@..... |
/* 1330 */ "\x11\x0d\x9f\x0f\xb6\x07\xbe\x54\x50\xab\xdc\x03\x83\xd0\x4e\x6a" //.......TP.....Nj |
/* 1340 */ "\x2b\xfb\x25\x3b\x0b\xf8\xbe\xfc\x78\x0d\x07\xfe\xf8\x40\x80\x6c" //+.%;....x....@.l |
/* 1350 */ "\xf2\xbf\xc3\x81\x25\x1b\xea\xe0\xc1\x6f\x09\x92\xf8\x02\xa4\x55" //....%....o.....U |
/* 1360 */ "\x64\xee\xcb\x28\x1d\x82\xff\xfd\x23\xe9\xd0\x0d\x9e\x1b\xfc\x9f" //d..(....#....... |
/* 1370 */ "\x4d\x40\xbf\xfd\x3c\x24\x55\x6f\x0d\xbe\x9c\x40\x2e\xe6\x98\x96" //M@..<$Uo...@.... |
/* 1380 */ "\xf9\xf4\xd0\x18\x5f\x10\xe7\xd1\x31\xdb\xdb\xa1\x8c\x03\xff\x41" //...._...1......A |
/* 1390 */ "\x20\xbb\xdc\xc0\x38\xbd\xb8\x45\xc7\x3c\xf5\x00\x0a\x22\x21\x40" // ...8..E.<..."!@ |
/* 13a0 */ "\x1d\x20\x63\x80\xbb\x53\xa8\x00\x50\xe1\x00\x0a\x6c\x06\x73\xdf" //. c..S..P...l.s. |
/* 13b0 */ "\xb5\xcd\xed\x70\xa9\x71\x3e\x94\x2e\x1c\xce\x14\xb8\x96\x00\xa1" //...p.q>......... |
/* 13c0 */ "\xe2\xa1\xa7\x5c\x8c\x3a\x62\xc5\xc3\xa5\xdb\xa5\x93\xae\x0c\x61" //.....:b........a |
/* 13d0 */ "\x57\x2b\xeb\x7d\x18\xa9\xb9\x8a\x9d\xeb\x02\xd0\xba\x59\x44\x80" //W+.}.........YD. |
/* 13e0 */ "\x12\x02\x93\x80\x17\x5e\x46\x32\x53\xbd\x34\xf3\x04\x30\xdb\xe5" //.....^F2S.4..0.. |
/* 13f0 */ "\xbe\x7c\xc3\xed\xbc\xd3\x38\x81\x1a\x3e\x8a\xc5\x5e\xf4\x1d\xb0" //.|....8..>..^... |
/* 1400 */ "\x5b\xfd\xa3\x1f\x25\x20\xbb\xa3\xfe\x3a\x81\x02\x7b\x66\x3e\x9c" //[...% ...:..{f>. |
/* 1410 */ "\x40\xea\xdf\x26\xc0\x6c\xff\xbb\x1f\x66\xe2\xe1\x85\xf0\x96\x73" //@..&.l...f.....s |
/* 1420 */ "\x9c\x46\xb9\x2d\xee\x36\xf8\x13\x9d\xbe\xbb\x7f\x60\x61\xf3\x99" //.F.-.6......`a.. |
/* 1430 */ "\x56\x5b\xd2\x85\xe5\x58\x3d\xe9\x3c\xe8\x8c\xee\x6e\xef\x6d\x7d" //V[...X=.<...n.m} |
/* 1440 */ "\xe8\x6b\xe3\xb0\x07\x54\x60\x18\xe6\x23\x80\x8f\x06\xb8\xc3\xc7" //.k...T`..#...... |
/* 1450 */ "\x7c\xf8\xf9\x33\xf4\xa3\xf1\x30\x10\x1d\xfe\x98\xf8\xef\xef\x7e" //|..3...0.......~ |
/* 1460 */ "\x05\xbf\x09\xdf\x50\x37\x7e\x89\x3e\xe0\x03\x08\x60\x00"         //....P7~.>...`. |
// Sent dumped on RDP Client (5) 5230 bytes |
// send_server_update done |
// GraphicsUpdatePDU::init::Initializing orders batch mcs_userid=0 shareid=65538 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[1](30) used=139 free=16139 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=68) |
// front::draw:draw_tile((768, 528, 64, 64) (576, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[2](8208) used=145 free=16133 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[3](30) used=275 free=16003 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=69) |
// front::draw:draw_tile((192, 592, 64, 32) (0, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[4](4112) used=281 free=15997 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[5](30) used=353 free=15925 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=70) |
// front::draw:draw_tile((256, 592, 64, 32) (64, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[6](4112) used=364 free=15914 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[7](30) used=447 free=15831 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=71) |
// front::draw:draw_tile((320, 592, 64, 32) (128, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[8](4112) used=453 free=15825 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[9](30) used=512 free=15766 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=72) |
// front::draw:draw_tile((384, 592, 64, 32) (192, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[10](4112) used=518 free=15760 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[11](30) used=615 free=15663 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=73) |
// front::draw:draw_tile((448, 592, 64, 32) (256, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[12](4112) used=621 free=15657 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[13](30) used=879 free=15399 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=74) |
// front::draw:draw_tile((512, 592, 64, 32) (320, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[14](4112) used=885 free=15393 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[15](30) used=1153 free=15125 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=75) |
// front::draw:draw_tile((576, 592, 64, 32) (384, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[16](4112) used=1159 free=15119 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[17](30) used=1260 free=15018 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=76) |
// front::draw:draw_tile((640, 592, 64, 32) (448, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[18](4112) used=1266 free=15012 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[19](30) used=1319 free=14959 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=77) |
// front::draw:draw_tile((704, 592, 64, 32) (512, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[20](4112) used=1325 free=14953 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[21](30) used=1409 free=14869 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=78) |
// front::draw:draw_tile((768, 592, 64, 32) (576, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[22](4112) used=1415 free=14863 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[23](30) used=1496 free=14782 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=79) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[24](32) used=1502 free=14776 |
// order(9 clip(200,145,1,110)):lineto(back_mode=01 startx=200 starty=1198 endx=200 endy=145 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[25](32) used=1530 free=14748 |
// order(9 clip(200,145,1,110)):lineto(back_mode=01 startx=200 starty=145 endx=200 endy=1198 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[26](32) used=1536 free=14742 |
// order(9 clip(200,145,1,110)):lineto(back_mode=01 startx=201 starty=1198 endx=200 endy=145 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[27](32) used=1544 free=14734 |
// order(9 clip(200,145,1,110)):lineto(back_mode=01 startx=200 starty=145 endx=201 endy=1198 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[28](32) used=1554 free=14724 |
// order(9 clip(145,200,110,1)):lineto(back_mode=01 startx=1198 starty=200 endx=145 endy=200 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[29](32) used=1569 free=14709 |
// order(9 clip(145,200,110,1)):lineto(back_mode=01 startx=145 starty=200 endx=1198 endy=200 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[30](32) used=1575 free=14703 |
// order(9 clip(145,200,110,1)):lineto(back_mode=01 startx=1198 starty=201 endx=145 endy=200 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[31](32) used=1583 free=14695 |
// order(9 clip(145,200,110,1)):lineto(back_mode=01 startx=145 starty=200 endx=1198 endy=201 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[32](82) used=1593 free=14685 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[33](66) used=1659 free=14619 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[34](66) used=1709 free=14569 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[35](66) used=1759 free=14519 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[36](66) used=1809 free=14469 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[37](20) used=1859 free=14419 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[38](297) used=1879 free=14399 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[39](66) used=1929 free=14349 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[40](66) used=1979 free=14299 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[41](66) used=2029 free=14249 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[42](297) used=2079 free=14199 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[43](82) used=2111 free=14167 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[44](66) used=2177 free=14101 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[45](66) used=2227 free=14051 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[46](297) used=2277 free=14001 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[47](82) used=2309 free=13969 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[48](66) used=2375 free=13903 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[49](66) used=2425 free=13853 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[50](297) used=2475 free=13803 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[51](66) used=2507 free=13771 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[52](66) used=2557 free=13721 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[53](66) used=2607 free=13671 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[54](297) used=2657 free=13621 |
// Widget_load: image file [./tests/fixtures/xrdp24b-redemption.png] is PNG file |
// front::draw:draw_tile((738, 613, 64, 64) (0, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[55](8208) used=2692 free=13586 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[56](30) used=3170 free=13108 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(738,613,64,64) rop=cc srcx=0 srcy=0 cache_idx=80) |
// front::draw:draw_tile((802, 613, 64, 64) (64, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[57](8208) used=3179 free=13099 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[58](30) used=3890 free=12388 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(802,613,64,64) rop=cc srcx=0 srcy=0 cache_idx=81) |
// front::draw:draw_tile((866, 613, 64, 64) (128, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[59](8208) used=3896 free=12382 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[60](30) used=4667 free=11611 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(866,613,64,64) rop=cc srcx=0 srcy=0 cache_idx=82) |
// front::draw:draw_tile((930, 613, 64, 64) (192, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[61](8208) used=4673 free=11605 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[62](30) used=5355 free=10923 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(930,613,64,64) rop=cc srcx=0 srcy=0 cache_idx=83) |
// front::draw:draw_tile((738, 677, 64, 61) (0, 64, 64, 61)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[63](7824) used=5361 free=10917 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[64](30) used=6300 free=9978 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(738,677,64,61) rop=cc srcx=0 srcy=0 cache_idx=84) |
// front::draw:draw_tile((802, 677, 64, 61) (64, 64, 64, 61)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[65](7824) used=6311 free=9967 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[66](30) used=7623 free=8655 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(802,677,64,61) rop=cc srcx=0 srcy=0 cache_idx=85) |
// front::draw:draw_tile((866, 677, 64, 61) (128, 64, 64, 61)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[67](7824) used=7629 free=8649 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[68](30) used=9025 free=7253 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(866,677,64,61) rop=cc srcx=0 srcy=0 cache_idx=86) |
// front::draw:draw_tile((930, 677, 64, 61) (192, 64, 64, 61)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[69](7824) used=9031 free=7247 |
// GraphicsUpdatePDU::flush_orders: order_count=69 |
// send_server_update: fastpath_support=yes compression_support=yes shareId=65538 encryptionLevel=0 initiator=0 type=0 data_extra=69 |
// Sending on RDP Client (5) 6454 bytes |
/* 0000 */ "\x00\x99\x36\x80\x21\x2f\x19\x45\x00\x03\x7e\xe4\x53\x8b\xe0\x02" //..6.!/.E..~.S... |
/* 0010 */ "\x26\x00\x9a\xcf\xb4\x6e\xfd\xa9\xe3\x80\x92\xa1\x81\x52\x58\xab" //&....n.......RX. |
/* 0020 */ "\xfb\x83\x80\xe2\x87\xc5\x8d\x82\x14\x26\x5f\x4d\xed\x27\x10\x17" //.........&_M.'.. |
/* 0030 */ "\xb9\x25\x00\x14\x85\xd3\xb3\xe3\xb1\x44\xe0\x0e\x69\x93\x8b\xe0" //.%.......D..i... |
/* 0040 */ "\x0e\x69\xa0\x5c\x31\x55\x7b\xb4\x63\xc0\x80\x01\xcc\x46\xe1\x5d" //.i..1U{.c....F.] |
/* 0050 */ "\x66\x91\x28\xa5\xc2\xf9\xda\x3e\x51\x00\x00\xf0\x99\x0b\xca\x36" //f.(....>Q......6 |
/* 0060 */ "\xe6\x00\x8a\x01\xde\x49\xed\x67\x91\x1d\x49\xe7\x93\xfc\x60\x02" //.....I.g..I...`. |
/* 0070 */ "\x28\x00\x02\xc5\x5e\x5c\xd8\xcf\x95\x4b\x21\x74\xa8\x01\x00\x1d" //(...^....K!t.... |
/* 0080 */ "\x7d\x96\x15\x0b\x91\x0a\x5d\x03\x15\x66\x98\x31\x5a\xe5\xe0\x03" //}.....]..f.1Z... |
/* 0090 */ "\x66\x09\xfc\x8f\x1f\xb0\x50\x73\x00\x1c\x46\x09\xfc\x8e\xc4\x4c" //f.....Ps..F....L |
/* 00a0 */ "\x3a\x45\x9f\x71\x7a\xf6\xc8\xef\xc9\x16\x40\x20\x72\x00\x8d\x88" //:E.qz.....@ r... |
/* 00b0 */ "\xe0\x18\xac\xf9\x83\x91\xd7\x0c\x56\x9c\xc5\x69\xb0\xc0\xdd\x8c" //........V..i.... |
/* 00c0 */ "\xf8\xbc\x58\x00\x28\x08\xb7\x69\xf2\x80\x11\x07\x81\x00\x68\xde" //..X.(..i......h. |
/* 00d0 */ "\x13\xc5\x10\x01\x1c\x00\xe3\x03\xee\x3a\x10\xa5\x4a\x44\xc0\x17" //.........:..JD.. |
/* 00e0 */ "\x8c\x58\x69\x18\xb1\x1e\x31\x63\xb4\xe2\x84\xc9\xc3\x3d\x8c\x10" //.Xi...1c.....=.. |
/* 00f0 */ "\x5e\xe0\x01\x58\xce\x67\xa2\x01\x87\x69\xe5\x1f\x8e\x43\xc3\x38" //^..X.g...i...C.8 |
/* 0100 */ "\x96\x00\x24\x61\xb8\xd0\x00\x06\x2d\x22\x62\xd2\x98\xe0\xbb\x18" //..$a....-"b..... |
/* 0110 */ "\xcd\x8b\xad\x8f\xc0\x32\x90\x00\x06\xa9\xe0\x1c\x54\x80\x12\x7b" //.....2......T..{ |
/* 0120 */ "\x64\x0d\x24\xa4\x00\xbf\x14\x30\x01\x60\xc6\x38\x7a\x78\x42\xbb" //d.$....0.`.8zxB. |
/* 0130 */ "\x20\xe7\xe1\x96\x2e\xaa\x8f\x60\xc6\x06\xce\x76\x0f\xcb\xd8\x0f" // ......`...v.... |
/* 0140 */ "\x6d\x54\x6a\xac\xf3\xe2\x73\xff\x89\xe5\x24\x00\x0e\xeb\xe2\x7c" //mTj...s...$....| |
/* 0150 */ "\x6e\x60\x09\x40\x01\x24\x00\xde\xbe\x08\x00\x58\x31\x88\x85\x78" //n`.@.$.....X1..x |
/* 0160 */ "\x72\x29\x3c\x01\xeb\xd4\x91\xec\x5b\xc1\x28\xcc\x60\x6c\xe0\x1d" //r)<.....[.(.`l.. |
/* 0170 */ "\x04\x18\xc5\x29\x84\x9c\x9e\x10\x33\x81\xc2\x69\xf0\x34\xd8\x5d" //...)....3..i.4.] |
/* 0180 */ "\x2e\x34\x93\x22\xb3\xcc\x64\xd3\x98\x34\xe5\x16\x82\x49\x49\xa1" //.4."..d..4...II. |
/* 0190 */ "\x8a\x26\xd0\xae\x69\x94\x2a\x9a\x60\x00\x2c\x28\xbe\x09\x4b\x8e" //.&..i.*.`.,(..K. |
/* 01a0 */ "\x34\x4b\x41\xa8\xd0\x3a\xb5\x8a\xbc\x83\x6f\x80\x5d\xe4\x96\xe2" //4KA..:....o.]... |
/* 01b0 */ "\x0b\x83\x88\x05\xe0\x7a\x0e\x9d\x0c\x2a\x92\xd9\xa7\xb0\x3a\xb1" //.....z...*....:. |
/* 01c0 */ "\x0d\x9e\x01\x77\xa0\x5f\xda\x39\x4d\xa4\x56\xe7\x20\x0b\xc0\xc4" //...w._.9M.V. ... |
/* 01d0 */ "\x18\xbe\x08\x15\x25\xa3\x7d\x99\x90\x5d\xdb\xfe\xb8\x8a\xdc\x00" //....%.}..]...... |
/* 01e0 */ "\x0b\x05\x86\xc5\x24\xf3\x56\x4a\x70\x0f\xec\x3a\xfd\x34\x8a\xeb" //....$.VJp..:.4.. |
/* 01f0 */ "\x84\x11\x54\xb8\xe5\x9a\x7b\x62\xef\x22\xe8\xd8\x80\xa0\xdd\x47" //..T...{b.".....G |
/* 0200 */ "\x20\xb7\xeb\xfa\x7d\x38\x96\x8c\x03\x67\xa0\x5f\xfa\x2d\x06\x60" // ...}8...g._.-.` |
/* 0210 */ "\xa1\xf0\x42\xa9\x2b\x25\x3a\x05\xe1\xc7\x6f\x58\xeb\x88\x15\x82" //..B.+%:...oX.... |
/* 0220 */ "\x27\x43\x28\xd3\xd0\x2f\xc0\xed\xdd\x02\x7e\xc8\x84\xa0\x00\x3c" //'C(../....~....< |
/* 0230 */ "\x00\x36\xf6\x45\xdf\x40\x12\xf0\x4a\xe3\x15\x83\x81\x82\x40\x2c" //.6.E.@..J.....@, |
/* 0240 */ "\x18\xc4\x3a\xb9\x09\x4f\xae\x23\x85\xee\x1c\x1e\x23\x84\x0c\xfb" //..:..O.#....#... |
/* 0250 */ "\x79\x40\xc2\x72\xce\x18\x13\xc0\x92\x78\x44\xc7\xb4\x60\xc0\x0e" //y@.r.....xD..`.. |
/* 0260 */ "\x03\x43\xa4\x7a\xfa\x05\x13\x61\x64\x94\x14\x5a\x0b\x2c\xe6\x4d" //.C.z...ad..Z.,.M |
/* 0270 */ "\x31\xa3\xcc\x42\x25\x49\x32\x29\x54\x68\xb2\x17\x0e\x85\xd3\xe0" //1..B%I2)Th...... |
/* 0280 */ "\x78\x79\xcc\x01\x78\x85\xf0\xd7\x5c\x24\xb6\xc4\x1b\x7d\x0a\xbc" //xy..x....$...}.. |
/* 0290 */ "\xc7\x4e\x05\xa3\x33\xcb\x69\x51\xc4\xe8\x67\x36\xe0\xab\x17\x61" //.N..3.iQ..g6...a |
/* 02a0 */ "\xf9\x67\xb0\x5f\xe1\x1d\xc4\x96\xf0\x1b\x3c\x0e\xac\x0b\x46\x83" //.g._......<...F. |
/* 02b0 */ "\xcd\x69\x71\xce\x70\x21\x8f\xac\x83\xf9\xf9\xfd\x04\x94\x64\x89" //.iq.p!........d. |
/* 02c0 */ "\x6b\x67\xf0\x81\x9c\x01\x06\x83\x48\x2e\xe4\x3f\xa1\x46\x9e\xda" //kg......H..?.F.. |
/* 02d0 */ "\xa3\x9d\x4d\x83\x5f\x49\xd7\xf9\x23\xab\x41\xe6\xbd\x6d\x1d\x96" //..M._I..#.A..m.. |
/* 02e0 */ "\xa3\xe7\x24\x5f\x78\x23\x46\xf9\xd5\x00\xbb\xa1\xdb\xe8\xdd\x9d" //..$_x#F......... |
/* 02f0 */ "\xb0\xea\xa8\xa1\x57\x9a\x25\xb5\x00\x33\xe0\x0c\x07\x35\x29\x60" //....W.%..3...5)` |
/* 0300 */ "\x00\x6b\x1e\xd2\xc5\x58\x01\x33\x05\x70\x38\xa0\xc5\x40\x04\xd2" //.k...X.3.p8..@.. |
/* 0310 */ "\x4a\x03\xed\x67\x13\x87\x13\xf7\x14\x49\xe2\x83\x3c\x4b\x22\xa1" //J..g.....I..<K". |
/* 0320 */ "\x83\x6b\xe7\x8a\x26\x7d\x83\xa6\x81\x8c\x2d\x31\xe2\x8b\xc4\xd1" //.k..&}....-1.... |
/* 0330 */ "\xc5\x16\x53\x00\x00\xf1\xe5\x8b\xc5\x76\x4c\x00\x9b\xc7\xad\xc7" //..S......vL..... |
/* 0340 */ "\xbd\x60\xe3\xd9\x30\x0f\x0d\xb8\x15\xb8\xa0\xc7\x4e\x57\x13\x5a" //.`..0.......NW.Z |
/* 0350 */ "\xe3\xcf\x2c\x1f\xee\x53\x40\x00\xd1\xff\xde\x28\xa0\x09\xc0\x04" //..,..S@....(.... |
/* 0360 */ "\xdc\x1e\x1c\xa0\x64\xe0\x02\xcd\xca\x06\x70\x11\x73\x93\x4b\x8b" //....d.....p.s.K. |
/* 0370 */ "\xde\x3e\x0c\x70\x83\xc4\x17\xe6\x39\xfe\x4f\x28\x1c\x79\x48\x47" //.>.p....9.O(.yHG |
/* 0380 */ "\x28\x2b\x4e\x00\x03\x44\xf0\xd6\x28\x40\x09\xe0\x18\x4a\xbd\xec" //(+N..D..(@...J.. |
/* 0390 */ "\x21\x80\x05\xbb\x99\x5c\x7a\xdd\x81\x20\x63\x18\x09\x79\x9e\x17" //!.....z.. c..y.. |
/* 03a0 */ "\xa4\xf0\x00\xd0\x95\xf0\x30\xfa\x40\x04\x44\x02\x90\x01\x7e\x00" //......0.@.D...~. |
/* 03b0 */ "\x01\xf8\xc9\x70\x27\xce\x80\xd0\x10\x0b\xc0\x03\x28\xa4\x47\xe7" //...p'.......(.G. |
/* 03c0 */ "\x99\x45\xa9\x3f\x19\x11\x00\x65\x1e\xfb\x31\xf3\x21\x14\x7a\xe1" //.E.?...e..1.!.z. |
/* 03d0 */ "\x49\x37\x36\xa5\x7e\xaa\xa4\x00\x32\x85\x7e\x79\x0e\x97\x02\x52" //I76.~...2.~y...R |
/* 03e0 */ "\x7e\x81\x94\x7b\xea\x47\xcc\x80\x33\x5c\x62\xe0\x03\xc1\xd4\x60" //~..{.G..3.b....` |
/* 03f0 */ "\x0b\xe5\xfe\xe1\x7f\x18\xc5\x89\x2f\x83\x8e\x0e\x3b\x7c\x30\x22" //......../...;|0" |
/* 0400 */ "\x20\x00\x24\x93\xe1\xc0\x14\xa3\xe1\xc0\x08\x87\xc3\x8f\x83\xa0" // .$............. |
/* 0410 */ "\x19\x2f\x80\xa8\x07\xc0\x52\x21\xe0\x29\x04\xf8\x56\x0b\x40\x06" //./....R!.)..V.@. |
/* 0420 */ "\x20\x04\xf8\x58\xfd\x97\xc1\x9f\xe5\x80\x17\xf2\xcf\xfa\x47\xc1" // ..X..........G. |
/* 0430 */ "\x8f\xed\x1f\x0b\x2f\x83\x87\xf2\xc0\x0f\xf9\x6b\xfb\xa8\x3f\xe2" //..../......k..?. |
/* 0440 */ "\x3e\x16\x18\x35\x31\xf0\x6f\xf9\x60\x09\xfc\xb7\xf8\x30\x07\x40" //>..51.o.`....0.@ |
/* 0450 */ "\x06\x60\x04\x20\x07\xb0\x02\x00\x03\x74\x1f\x7d\x17\xc1\x90\x3f" //.`. .....t.}...? |
/* 0460 */ "\xfb\x00\xe0\x00\x21\x01\x10\x10\x89\x54\xa9\x57\xe1\x48\x48\xdd" //....!....T.W.HH. |
/* 0470 */ "\x6c\xfc\xf4\x1c\x0c\x07\xa9\x10\x7b\xe1\x10\x78\x49\x3e\x46\x10" //l.......{..xI>F. |
/* 0480 */ "\x1f\xbe\xf8\x50\x3d\xf3\x18\x36\x74\x02\x16\x04\x10\x06\x06\x08" //...P=..6t....... |
/* 0490 */ "\x0b\xe3\x8c\x01\x7c\x71\xb8\x3d\x03\xd8\x51\xe3\x65\x00\x11\x03" //....|q.=..Q.e... |
/* 04a0 */ "\xe4\x40\x27\xd9\xc3\xcd\x58\x83\x7f\x2d\x08\x3e\x15\x01\xd8\x3c" //.@'...X..-.>...< |
/* 04b0 */ "\xd5\x1f\x0a\x01\x98\x00\xec\x1f\x61\xf0\x67\xf9\x60\x0f\xfc\xb4" //........a.g.`... |
/* 04c0 */ "\xf8\x3d\xa0\x31\x03\xb3\x0e\x1d\x83\x20\x03\xe0\x04\x2f\x8d\x68" //.=.1..... .../.h |
/* 04d0 */ "\x40\x01\x40\xa0\x00\x81\x00\xc1\x00\xe1\x00\xe0\x9c\x31\xb0\x08" //@.@..........1.. |
/* 04e0 */ "\xf0\x94\xf0\xc6\xd4\x06\xe0\x00\x04\x11\xc0\x6d\xf0\xe4\xe3\xf9" //...........m.... |
/* 04f0 */ "\x3e\x1c\x7d\x27\xd8\x7c\x1d\x3c\xa9\x80\x4f\x80\xa7\xca\x9b\x02" //>.}'.|.<..O..... |
/* 0500 */ "\xc0\x01\xbf\xf3\xe1\x61\xf0\x70\xfe\x58\x05\x7f\x2e\x4e\x1f\x1c" //.....a.p.X...N.. |
/* 0510 */ "\xbe\x86\x8b\x00\x3f\x64\x22\x28\x02\xb7\xc6\xb5\x60\x00\xac\x37" //....?d"(....`..7 |
/* 0520 */ "\xca\x0a\x04\x05\x04\x08\x0a\x08\xf4\x36\x01\x7e\x12\x9f\x43\x6a" //.........6.~..Cj |
/* 0530 */ "\x0f\x60\x00\xe0\x64\xf8\x75\xf3\x37\xf8\x3a\x79\x53\x00\xcf\x01" //.`..d.u.7.:yS... |
/* 0540 */ "\x4f\x95\x2b\xce\x91\xf0\xb8\xbe\x0e\x1f\xcb\x00\xdf\xe5\x9f\xc1" //O.+............. |
/* 0550 */ "\xc3\xcf\x91\xf0\xad\x80\x0e\x41\xf4\x5f\x06\x7d\x0d\x03\xe0\x00" //.......A._.}.... |
/* 0560 */ "\x0b\x5c\x11\x8d\x5f\x1a\xda\x80\x02\x82\xc0\x03\x02\x43\x40\xc1" //...._........C@. |
/* 0570 */ "\x38\x27\x3c\x25\x80\x77\x84\xb8\xc0\xf8\x83\x38\x6b\x47\x84\xb6" //8'<%.w.....8kG.. |
/* 0580 */ "\xfe\x58\x07\xff\x2e\x48\x00\x36\x04\x0e\x7c\x28\x06\x40\x03\x80" //.X...H.6..|(.@.. |
/* 0590 */ "\x7d\x17\xc1\x9f\xe5\x80\x87\xf2\xcf\xf2\x47\xc2\x94\x00\x48\x00" //}.........G...H. |
/* 05a0 */ "\x2e\x41\x8d\x70\xde\x40\x01\x10\x3e\xcb\xe0\xc8\x0c\xc3\x88\x13" //.A.p.@..>....... |
/* 05b0 */ "\x1e\xc1\x4b\x76\x04\xe1\x3f\x7c\x6b\x7e\xf3\xcd\x07\x01\x87\x84" //..Kv..?|k~...... |
/* 05c0 */ "\x08\x03\x81\xd4\x70\x14\x3e\x34\xf0\x15\x0c\x3e\x32\x41\xa4\x50" //....p.>4...>2A.P |
/* 05d0 */ "\x00\xd8\x6d\x10\x12\x06\x00\xf8\x11\x02\xc2\x0d\x40\xbb\xe9\x23" //..m.........@..# |
/* 05e0 */ "\x02\x70\x12\x08\x43\x80\x00\x04\x08\x00\x41\x06\x60\x32\x42\x27" //.p..C.....A.`2B' |
/* 05f0 */ "\x01\x41\x98\x60\x55\xe8\x41\x90\x0c\x40\x40\x12\x3e\x72\xb0\x25" //.A.`U.A..@@.>r.% |
/* 0600 */ "\x02\x00\x08\x41\xa4\x0b\xc0\x40\x08\x1d\x3f\xd8\x00\x83\xef\x40" //...A...@..?....@ |
/* 0610 */ "\x01\x80\xbe\x0a\x7e\x20\xd8\x04\xff\x23\x0e\xd4\xbf\xa0\x5e\x02" //....~ ...#....^. |
/* 0620 */ "\x83\x38\xf0\x51\xc3\xe9\x10\x05\xe0\x24\x11\x01\x32\x11\x3f\xf1" //.8.Q.....$..2.?. |
/* 0630 */ "\x16\x01\x68\x09\x03\x47\x81\x94\x0c\x3b\x74\xfc\x41\x6f\xbc\x13" //..h..G...;t.Ao.. |
/* 0640 */ "\x81\x20\xda\x00\x02\x3f\x62\x01\xf3\x82\x1e\x14\x92\x8f\x9c\x70" //. ...?b........p |
/* 0650 */ "\x16\xf0\x94\x56\x3e\x51\x63\xe2\x18\x3c\x20\x94\x7c\x84\x03\xe2" //...V>Qc..< .|... |
/* 0660 */ "\x14\x3c\x20\xf9\x0f\xa1\x2b\x1f\x31\xf8\x08\x35\x0f\x98\x68\xf2" //.< ...+.1..5..h. |
/* 0670 */ "\xa0\xc3\xc0\x12\x20\x25\xe8\x87\x94\x81\x08\x36\x8f\x20\x0e\x04" //.... %.....6. .. |
/* 0680 */ "\x7e\x54\x56\x3e\x80\x01\x06\x91\xfa\x20\x47\xe2\x1a\x98\x6d\x01" //~TV>..... G...m. |
/* 0690 */ "\xfe\x95\x7c\xf4\x43\x1b\x18\x8e\x10\x6c\x03\x30\x12\x07\x8f\x04" //..|.C....l.0.... |
/* 06a0 */ "\xa7\x1e\xca\x34\x20\xd8\x1a\xfe\xfc\x10\x7a\xc2\x2c\x01\x78\xc3" //...4 .....z.,.x. |
/* 06b0 */ "\xd8\x00\x70\x40\xf0\x83\x50\x6d\xfa\xb1\xeb\x79\xa4\xa0\xd2\x20" //..p@..Pm...y...  |
/* 06c0 */ "\x48\x35\x04\x1b\x03\x8b\x0d\xa3\xdd\x6f\xca\x60\x0f\x61\xb4\x08" //H5.......o.`.a.. |
/* 06d0 */ "\x7f\xe6\x03\xf7\x10\x44\x3e\x2c\x34\x7c\x57\x04\x17\xc5\x61\x5e" //.....D>,4|W...a^ |
/* 06e0 */ "\x6a\x41\xac\x7c\xc2\x0c\xc3\xe2\x88\x21\x1f\x16\x18\x3e\x28\x00" //jA.|.....!...>(. |
/* 06f0 */ "\xc3\xe2\xc6\x11\xee\x7e\x08\xa4\x70\x4f\x9f\x3c\xa4\x00\x01\x30" //.....~..pO.<...0 |
/* 0700 */ "\x34\x1a\xc1\x04\x63\x80\x55\x10\x3c\x00\x02\x41\x50\x6d\x08\x34" //4...c.U.<..APm.4 |
/* 0710 */ "\x84\x19\x01\x03\x80\x01\xc6\x1e\xb4\x7b\x7d\x40\x0e\x41\x8c\x41" //.........{}@.A.A |
/* 0720 */ "\x88\x1a\x08\x18\x08\x16\x08\x12\x30\x7b\xb0\x82\xe7\x09\xc2\x55" //........0{.....U |
/* 0730 */ "\x81\x90\xd1\x60\x1b\x10\xa9\x02\x80\x00\x1c\xeb\x04\x80\xe3\x38" //...`...........8 |
/* 0740 */ "\x02\x51\x00\xa3\x7b\x4c\x83\x48\xa8\x01\x80\x60\x10\x01\xb0\x4d" //.Q..{L.H...`...M |
/* 0750 */ "\x00\x1a\x3a\x13\x01\x7b\x09\x1d\x67\x61\xb4\x54\x00\xc8\x1e\x00" //..:..{..ga.T.... |
/* 0760 */ "\x60\x37\x10\x07\x7f\xa4\x23\xe2\x66\xf4\x43\x01\x49\x30\x67\xca" //`7....#.f.C.I0g. |
/* 0770 */ "\x59\x85\x03\x8d\x3f\x46\x3f\x20\x08\x81\xa0\x06\x02\x8f\xf4\x54" //Y...?F? .......T |
/* 0780 */ "\xee\x28\x2c\x17\xf2\x90\x00\xe2\xeb\xe0\xbc\x04\x4c\x24\x00\x12" //.(,.........L$.. |
/* 0790 */ "\x04\xc3\x68\x00\x48\xc2\x95\xe8\x0a\xd0\x48\xb4\x05\x60\x24\x00" //..h.H.....H..`$. |
/* 07a0 */ "\x17\xf6\xaa\x00\x30\x11\x40\x91\x68\x14\x70\x27\xab\x83\xf7\xc2" //....0.@.h.p'.... |
/* 07b0 */ "\x40\x0c\x06\x9f\xe2\x19\xc0\xf0\x11\x04\xfe\x2e\xda\xa8\x57\x80" //@.............W. |
/* 07c0 */ "\x10\xbe\x60\x6c\x0c\x30\x70\xc2\x5c\x6f\x9c\xa8\x7c\xe9\x00\xac" //..`l.0p..o..|... |
/* 07d0 */ "\x00\xfb\x29\x1c\xf0\x68\xd7\xc1\xb2\xc3\x83\x1d\xcb\x1f\x38\xf5" //..)..h........8. |
/* 07e0 */ "\xf3\xa4\x03\xb2\x48\x4c\x07\xc1\x70\x45\xe0\xb7\xd0\x79\x61\x2e" //....HL..pE...ya. |
/* 07f0 */ "\xff\x8c\x29\xa2\xf9\xd1\xe9\xd3\x4f\xce\x62\x71\x77\xce\x83\x80" //..).....O.bqw... |
/* 0800 */ "\x18\x09\x27\xce\x61\xf6\x00\x3f\x84\xcf\x7e\x05\x1b\xf9\xc4\x80" //..'.a..?..~..... |
/* 0810 */ "\x06\x03\xf2\x02\xd0\x17\xcf\x98\xf6\x85\x7a\xf6\x89\x3d\x58\xfa" //..........z..=X. |
/* 0820 */ "\x0f\x08\xbd\x75\x10\x9f\x9d\x3e\x3d\x86\x7f\x9c\xf5\xbb\xe7\x00" //...u...>=....... |
/* 0830 */ "\x2d\x00\x3e\x37\x0b\xfa\x71\xf3\xa0\x81\x23\xcd\x30\xf5\xe0\xa2" //-.>7..q...#.0... |
/* 0840 */ "\xc8\x4e\x0a\x94\x7b\xdc\x08\x00\x60\x03\x01\x15\x09\xf0\x6c\x7a" //.N..{...`.....lz |
/* 0850 */ "\xe4\x03\x01\x20\x8c\x7d\x37\x88\x15\x0f\xe8\x46\xf8\x88\x50\x00" //... .}7....F..P. |
/* 0860 */ "\x4d\x40\x14\x82\xb0\x03\xeb\x87\xaf\x20\x56\x04\xd2\x00\xb0\x13" //M@....... V..... |
/* 0870 */ "\x50\x08\x17\xf7\x3b\x87\xb2\xa0\x0a\x00\x81\xec\x9a\xb8\x00\x09" //P...;........... |
/* 0880 */ "\x40\xd1\x71\x65\x79\x68\x25\x8e\x3a\x12\x11\x50\x06\x41\x80\x70" //@.qeyh%.:..P.A.p |
/* 0890 */ "\x2f\xa3\x60\x90\xc8\x30\x0e\x25\x83\x34\x00\x4b\x80\x04\xc2\x5e" ///.`..0.%.4.K...^ |
/* 08a0 */ "\x82\x59\xa3\xa1\x20\xfc\x42\x98\x56\xe5\xe4\x37\xa4\x62\x92\xb0" //.Y.. .B.V..7.b.. |
/* 08b0 */ "\x13\x03\x59\xfb\x27\xc0\x96\xc8\xe8\x48\x37\x85\x20\x20\x82\x01" //..Y.'....H7.  .. |
/* 08c0 */ "\xe1\xd0\x98\x4b\x00\x7c\x74\x24\x16\x05\xc4\x80\x31\x50\x08\xf2" //...K.|t$....1P.. |
/* 08d0 */ "\x98\x03\xd2\x51\xc8\xd1\x40\x00\x18\x6a\xff\xcf\x56\x84\xa0\x5c" //...Q..@..j..V... |
/* 08e0 */ "\x4b\x41\x18\x38\xc1\xec\x88\xc6\x7c\x2a\x81\x02\x86\x21\xea\x78" //KA.8....|*...!.x |
/* 08f0 */ "\x51\x87\x1a\x1c\x62\x18\x76\xc6\x3d\xec\x00\x0e\xc4\x17\x0e\x02" //Q...b.v.=....... |
/* 0900 */ "\x30\x3f\xc0\x92\x07\xe0\x3e\x3b\x0f\xfb\x4d\xc5\xf6\x5e\xc2\x7e" //0?....>;..M..^.~ |
/* 0910 */ "\x60\xdb\xe2\xc7\x53\x30\xff\xa8\x1f\x15\x81\x10\xc3\xfe\x7b\x7e" //`...S0........{~ |
/* 0920 */ "\x09\xf7\x30\x7f\xdf\x71\x89\x16\x40\x03\xd0\xe7\xc0\x97\xbf\x34" //..0..q..@......4 |
/* 0930 */ "\x96\x42\x56\x02\x4d\x82\x45\x42\x44\xc0\x6e\x3e\xb0\x4b\x00\x1a" //.BV.M.EBD.n>.K.. |
/* 0940 */ "\x7a\x78\x08\x80\x39\x6b\xa8\x01\x71\xc0\x9b\x9d\x47\x0a\xd1\x04" //zx..9k..q...G... |
/* 0950 */ "\x70\x00\xd1\x47\x07\xe9\x38\xc6\xe3\x74\x02\x52\xc1\x88\x07\xc0" //p..G..8..t.R.... |
/* 0960 */ "\x9a\x00\x07\x02\x74\x90\x7a\x80\x2a\xc2\x3f\xc6\x2e\x04\x20\x32" //....t.z.*.?... 2 |
/* 0970 */ "\x91\xd0\x00\xee\x28\xa1\x08\x0c\x38\x53\xe4\x96\x7e\x4a\x26\x27" //....(...8S..~J&' |
/* 0980 */ "\xf2\x4b\x1f\x24\xf0\x15\xf2\x5f\x49\xc5\x48\x82\x3e\x35\x01\xa0" //.K.$..._I.H.>5.. |
/* 0990 */ "\x05\x52\x60\x22\xc1\x3c\x44\x8e\x3e\x03\x23\x0f\x0d\x40\x50\x02" //.R`".<D.>.#..@P. |
/* 09a0 */ "\xb9\xc0\x44\x70\x24\x46\x02\x96\x2a\x35\x01\x80\x05\x88\xfa\x50" //..Dp$F..*5.....P |
/* 09b0 */ "\xe5\xf7\x0a\x02\x83\x50\x1b\x00\xa0\x8f\x05\x56\x35\x03\x09\x8d" //.....P.....V5... |
/* 09c0 */ "\x40\x48\x07\x84\x7d\x30\x12\x80\x9d\x05\x0d\x40\x90\x07\xc8\x30" //@H..}0.....@...0 |
/* 09d0 */ "\x18\x5a\xe0\x00\xe7\xfa\x00\x43\xef\x4c\x23\xf0\x14\x00\x11\x0f" //.Z.....C.L#..... |
/* 09e0 */ "\x71\x84\xe0\x20\xfe\x12\x48\x02\x83\xfc\x08\x74\xfb\x80\xcf\x09" //q.. ..H....t.... |
/* 09f0 */ "\x2c\x02\x61\xff\xe0\x29\x01\xb8\xf8\x10\x80\x2a\x1f\xf9\x82\x90" //,.a..).....*.... |
/* 0a00 */ "\x1b\xb0\x97\x8a\x83\xc1\xde\xe3\x3a\xfe\x65\xec\x25\xef\xc0\x46" //........:.e.%..F |
/* 0a10 */ "\x0c\x26\x74\x15\x09\x76\xfa\x0a\x27\x81\x20\x04\x33\x81\x0b\x88" //.&t..v..'. .3... |
/* 0a20 */ "\x74\x4a\x47\x46\xa0\x20\x01\x61\x70\x20\x2e\x1a\x61\x78\x0c\xbb" //tJGF. .ap ..ax.. |
/* 0a30 */ "\xca\xc4\x01\x40\xbe\x01\x87\x8f\xb8\x99\xf8\x51\x20\x15\x0a\xe3" //...@.......Q ... |
/* 0a40 */ "\x0a\x79\xf9\xc8\xc0\x57\x8b\x83\xc1\x3f\x38\xf5\x1d\x09\x81\x4f" //.y...W...?8....O |
/* 0a50 */ "\x3d\x5e\xf2\xd2\xa0\x0d\x40\xc1\x2f\x62\x20\x01\x50\x97\xcf\xc6" //=^....@./b .P... |
/* 0a60 */ "\x65\x1e\x1a\xfa\x20\xa0\x1f\x7a\x9f\x7e\x79\xa8\x57\xe7\x98\x00" //e... ..z.~y.W... |
/* 0a70 */ "\x40\x5f\xbe\x62\xd1\xe0\xc8\xc6\x53\x6f\x74\x10\xbc\x12\xc0\x00" //@_.b....Sot..... |
/* 0a80 */ "\x60\x38\x15\xc8\x02\xc1\x85\xc6\x18\xaa\x08\x05\x04\x8e\x04\xe0" //`8.............. |
/* 0a90 */ "\x1f\x87\x08\xd7\x84\xd0\x12\x00\x1e\x34\x2b\x80\x07\x2b\x98\xa0" //.........4+..+.. |
/* 0aa0 */ "\x7a\x88\xc1\x80\x07\x1c\x04\x55\x00\x2b\x8f\xd7\x30\x79\x85\x40" //z......U.+..0y.@ |
/* 0ab0 */ "\xc2\xdf\x6f\x5a\x2a\x00\x5f\xa2\x08\x02\xfd\xf8\x15\x1f\xe2\x90" //..oZ*._......... |
/* 0ac0 */ "\xa8\x01\xc2\xfe\x82\x80\x03\xfc\x83\xc3\xdc\x14\xd5\x00\x41\x20" //..............A  |
/* 0ad0 */ "\x90\x18\x0c\x00\x00\x6a\x04\xc0\x3e\x81\x83\xb0\x8f\x81\x28\xa0" //.....j..>.....(. |
/* 0ae0 */ "\x18\x62\xa9\x01\x90\x0f\x78\x96\x20\x00\x23\x7e\x4b\x34\x80\x54" //.b....x. .#~K4.T |
/* 0af0 */ "\x01\x91\x7f\xb1\x83\xc5\x0a\x00\x50\xdf\x20\x75\x45\xa7\xe7\x80" //........P. uE... |
/* 0b00 */ "\x3c\x4c\x20\xf8\x8f\x12\x82\x21\x58\x08\xbd\xf7\x1e\x7e\x0a\xd1" //<L ....!X....~.. |
/* 0b10 */ "\x08\x00\x45\x20\x4f\x61\x2d\x1d\x9b\x80\x10\x16\x00\x17\xf6\x11" //..E Oa-......... |
/* 0b20 */ "\xf1\x38\x12\xc0\x49\x20\x07\x19\xb6\x35\x85\x30\x0f\xf1\x9d\x5c" //.8..I ...5.0.... |
/* 0b30 */ "\xbf\x73\x12\x6d\x88\x78\xcd\x96\x02\x59\xc0\x38\xcd\xc6\x48\xfc" //.s.m.x...Y.8..H. |
/* 0b40 */ "\x06\x40\x9e\x82\x65\xa0\x1c\x68\x22\x40\x16\x00\x15\xc8\x21\x2c" //.@..e..h"@....!, |
/* 0b50 */ "\xfc\xee\x1b\x10\x80\x14\x00\x0f\x03\x7b\x09\xb9\x72\x38\x60\x7e" //.........{..r8`~ |
/* 0b60 */ "\xe4\x07\xeb\x01\x3e\xf2\xe3\xf3\xd8\x77\x8d\x01\x20\x1f\x1d\x83" //....>....w.. ... |
/* 0b70 */ "\x3e\x71\x87\x09\x84\xd3\xb8\xbf\x88\x80\x1f\x3e\xa2\xc0\x2a\x3b" //>q.........>..*; |
/* 0b80 */ "\x86\x19\xa2\x03\x00\xc9\x27\x88\x0e\x3b\x06\x0f\xc0\x6c\x1c\x6e" //......'..;...l.n |
/* 0b90 */ "\x23\xb6\x53\x8e\x4c\x50\x07\xbe\x62\x00\x2b\xf1\x44\x1f\x82\x2c" //#.S.LP..b.+.D.., |
/* 0ba0 */ "\x01\x5f\x15\xc9\x98\x3f\x02\x40\xf2\xfc\x61\x1c\xe8\x60\xfc\x50" //._...?.@..a..`.P |
/* 0bb0 */ "\x41\xf8\xa2\x00\x15\x40\x0b\x90\x85\xb0\x77\xbf\x71\xc9\x85\x20" //A....@....w.q..  |
/* 0bc0 */ "\x00\x38\xef\x1c\x9c\x63\x60\x4a\x60\x14\x01\xb6\x11\xf8\x11\x00" //.8...c`J`....... |
/* 0bd0 */ "\xf9\x00\x08\x9b\x00\x67\xbc\x16\x0f\xe6\x81\x30\x8f\xc0\x3c\x64" //.....g.....0..<d |
/* 0be0 */ "\xfa\xcc\x03\xe6\x1e\xf9\xc9\x00\xa0\x8d\x16\x02\x2d\x07\x00\x0a" //............-... |
/* 0bf0 */ "\x40\x0a\x90\x05\x30\x05\xa0\x0a\x48\x3f\x33\x0a\x6a\xb8\x18\x80" //@...0...H?3.j... |
/* 0c00 */ "\x7c\xda\x81\xc8\x00\x40\x05\x28\x03\xcc\x00\x1e\x00\x20\x0a\xe0" //|....@.(..... .. |
/* 0c10 */ "\x96\x06\x37\x24\x48\x02\xa0\x03\xc3\xdf\x84\x40\x02\xff\xf4\x26" //..7$H......@...& |
/* 0c20 */ "\xc0\x16\xc0\x3e\xfb\xf8\x0c\x02\x98\x65\x16\x20\x40\x1e\x72\x0e" //...>.....e. @.r. |
/* 0c30 */ "\x78\x84\xc0\x01\xf3\xe7\xe2\x87\xdf\x3e\xf1\x00\x2f\xd6\xb1\xf7" //x........>../... |
/* 0c40 */ "\x8c\x03\xe3\x7c\x35\x4c\x1c\x9f\x90\x1b\x00\x58\xdf\x82\x1e\x4a" //...|5L.....X...J |
/* 0c50 */ "\x01\x70\x3d\x40\x07\xb8\xd2\xc7\xe7\x7d\xa8\x90\x70\x8a\x79\x28" //.p=@.....}..p.y( |
/* 0c60 */ "\x07\xf6\x14\x01\xee\x41\xf1\xeb\xe7\xd8\x80\x01\x4f\xcb\x14\x0c" //.....A......O... |
/* 0c70 */ "\x89\xb8\xf8\xa1\xbe\x49\xac\x33\x33\x1c\x09\x9c\x2b\x42\xb8\x40" //.....I.33...+B.@ |
/* 0c80 */ "\x2d\xc4\xb6\x20\x03\xdf\x25\xc4\x95\xe1\xa6\x01\x35\x01\xf0\x70" //-.. ..%.....5..p |
/* 0c90 */ "\x7f\x38\xe0\xb0\x03\xe0\x1a\x1b\x80\xf9\x7c\x07\xef\x90\xdf\xcd" //.8........|..... |
/* 0ca0 */ "\x7c\x92\x07\xc9\x70\x43\x7e\x67\xc9\x70\x9a\x01\xf1\x4e\x7d\xb7" //|...pC~g.p...N}. |
/* 0cb0 */ "\xc9\x8a\x07\xc9\x20\x71\xaf\x7a\xe0\x37\xc1\x32\x00\xe0\x20\x1e" //.... q.z.7.2.. . |
/* 0cc0 */ "\x7a\x7d\xb0\x17\x81\x1e\x92\x8f\xde\x02\xf3\x28\x73\xc0\x31\xc6" //z}.........(s.1. |
/* 0cd0 */ "\x18\x15\xec\x49\xe3\x54\x81\x00\x00\xd8\x07\x87\x78\x8f\x07\xe0" //...I.T......x... |
/* 0ce0 */ "\x56\x00\x04\x01\xe7\xef\xaf\x1c\x0b\x10\x0f\x6c\x3e\x64\x48\x07" //V..........l>dH. |
/* 0cf0 */ "\xb8\x0c\x47\xac\x01\x5e\xc6\x70\x18\x8b\x00\xa0\x65\x2c\x80\x06" //..G..^.p....e,.. |
/* 0d00 */ "\x0d\xc2\x94\x15\xf5\x01\x6c\x1c\x10\x4c\x00\x5f\x60\x80\x16\x18" //......l..L._`... |
/* 0d10 */ "\xc0\x27\xf2\x23\x25\xe3\x39\x8e\xb8\xaa\x39\x8e\xbc\x6a\xe0\x79" //.'.#%.9...9..j.y |
/* 0d20 */ "\x1c\x2c\xc3\xdf\xe5\xd1\x15\x89\xdb\xdc\x08\xc3\xd0\x1a\x00\xa7" //.,.............. |
/* 0d30 */ "\x16\xa6\x33\xa7\x00\x0f\xf1\x09\x0f\x72\xd1\x77\x36\x73\x98\x50" //..3......r.w6s.P |
/* 0d40 */ "\x05\x38\xba\x00\x03\x60\x40\x03\xd8\x03\x18\x3b\x02\x60\x1e\xa0" //.8...`@....;.`.. |
/* 0d50 */ "\x71\x76\x8c\x72\xc8\xf9\x51\xdb\xf8\x00\x15\xf6\xd4\x0e\x82\xb0" //qv.r..Q......... |
/* 0d60 */ "\x00\x13\x02\xa0\x0a\x20\x71\x85\x8e\xba\x30\x3f\x60\x0a\xc1\xe4" //..... q...0?`... |
/* 0d70 */ "\xd0\x0d\x02\x00\x15\x90\x00\x3c\x7c\xa0\x24\x09\x80\x56\x40\x02" //.......<|.$..V@. |
/* 0d80 */ "\xf5\xf2\x80\x30\x3e\x1f\xc1\x49\x07\x0d\xf8\x1e\x82\x10\x0f\xa0" //...0>..I........ |
/* 0d90 */ "\x0a\xd8\x02\x98\x30\x08\x28\x00\x6a\x0b\x00\x1e\x20\x15\xa0\x05" //....0.(.j... ... |
/* 0da0 */ "\xd8\x30\x6e\x63\xca\x60\x0a\x00\x0c\x84\x2e\x13\x20\x14\x80\x0a" //.0nc.`...... ... |
/* 0db0 */ "\x70\x06\x0a\x48\x02\xec\x01\x44\x01\x4c\x00\x2c\x00\x17\x32\x6f" //p..H...D.L.,..2o |
/* 0dc0 */ "\xb8\x11\x02\x01\x40\x53\x00\x03\x8f\x64\xca\x97\x17\x08\x3d\x10" //....@S...d....=. |
/* 0dd0 */ "\x8e\x60\x9b\x78\x20\xb8\x5e\x00\x60\xf0\x2c\x47\xd2\xb5\xad\x16" //.`.x .^.`.,G.... |
/* 0de0 */ "\x31\xd2\xd6\xad\x07\x38\x6c\xe4\x2f\x8e\x50\x51\x69\xab\x3d\x7f" //1....8l./.PQi.=. |
/* 0df0 */ "\x09\xfa\xc7\x5d\x51\x42\x14\x7a\xee\xd3\x5f\xbf\xf0\x68\x44\x30" //...]QB.z.._..hD0 |
/* 0e00 */ "\x0a\xc5\x46\x07\x31\xd2\xa1\xf6\xee\xd5\xf5\x27\x00\x7c\x48\xe6" //..F.1......'.|H. |
/* 0e10 */ "\x0f\xe2\x54\x00\xe4\x41\x00\x30\x6b\xdd\xfa\xd7\x6a\x24\xb4\xd8" //..T..A.0k...j$.. |
/* 0e20 */ "\xed\x10\x6b\x4d\xae\xd5\x50\x03\xa5\xf3\x96\x60\x10\xc7\x64\xc0" //..kM..P....`..d. |
/* 0e30 */ "\xe5\x40\x11\x27\x1f\x51\xa0\xd6\x96\x66\x1b\x47\x09\x70\xc6\x00" //.@.'.Q...f.G.p.. |
/* 0e40 */ "\x6f\x77\xf3\x0d\xd4\x00\xc9\xf0\xa5\x3a\x35\x7e\xef\x70\xc3\xa8" //ow.......:5~.p.. |
/* 0e50 */ "\x8c\x03\x10\xc0\x17\x65\x0f\xac\x90\xf6\x66\x3d\x5d\xaa\x01\x49" //.....e....f=]..I |
/* 0e60 */ "\x00\x4f\xff\x59\x30\xfa\xc7\x8f\xad\x00\x00\x2d\x15\xfb\xf8\x26" //.O.Y0......-...& |
/* 0e70 */ "\x01\x29\xa9\x00\xd4\x08\x00\xc1\xe6\xed\x80\x34\x58\x04\xae\x46" //.).........4X..F |
/* 0e80 */ "\xa0\x44\x0e\x11\x2b\xa0\x7e\x00\x7f\x78\x6e\x42\x06\x7f\xba\xa0" //.D..+.~..xnB.... |
/* 0e90 */ "\x09\x1a\x1e\xc5\x72\x8b\xa5\x16\x7c\x2f\x50\x04\x51\x0c\xf2\x60" //....r...|/P.Q..` |
/* 0ea0 */ "\x20\x53\x00\x9d\x06\x20\x60\x18\x06\xd1\x80\x01\x80\x76\x7e\x75" // S... `......v~u |
/* 0eb0 */ "\x89\xf5\xfe\xa0\x08\x08\x02\x03\x50\x04\x8f\x03\xf7\x00\x8c\x02" //........P....... |
/* 0ec0 */ "\x9b\x2c\x18\x08\x5c\x80\xc5\xe0\x85\x9e\xd3\x82\xff\x9d\x1b\x15" //.,.............. |
/* 0ed0 */ "\xfb\xfc\xea\xfd\x50\x02\xb3\x80\x80\xc0\x41\x2d\x35\x00\x5b\x67" //....P.....A-5.[g |
/* 0ee0 */ "\xa5\xd8\x05\x9a\x61\x18\xb0\x60\x41\x87\x04\x5e\x0c\x81\x10\xc0" //....a..`A..^.... |
/* 0ef0 */ "\x16\x2a\xfd\xfe\x06\x73\x76\xab\xdd\x5f\x6f\xf5\x00\x52\x3f\xd8" //.*...sv.._o..R?. |
/* 0f00 */ "\xc0\x2f\x93\x30\x18\xee\xee\xee\x12\xfc\x80\x43\x00\xc0\x16\x4a" //./.0.......C...J |
/* 0f10 */ "\xfd\xfc\x17\xd4\x8f\xb2\xac\x60\x1d\xa0\xc7\xb3\xd0\x24\x9b\x54" //.......`.....$.T |
/* 0f20 */ "\x6a\xfc\x17\xec\xe7\x9e\x64\x60\x5a\xa0\x0c\x40\x80\x03\xe3\xd4" //j.....d`Z..@.... |
/* 0f30 */ "\x14\xa6\xd4\xe3\x74\x9b\x5d\x4e\x6e\xd7\x7b\xad\x40\x14\x30\x2f" //....t.]Nn.{.@.0/ |
/* 0f40 */ "\xcb\xb3\x80\xa3\x21\x00\x5a\xa0\x10\xc3\xc9\x41\xfe\xdd\xc4\xd4" //....!.Z....A.... |
/* 0f50 */ "\x0f\x23\x9c\x05\xbc\x1c\x14\xe0\xc0\xf9\x88\xc0\xaf\xb0\xcd\xff" //.#.............. |
/* 0f60 */ "\x6e\x38\x2d\xd0\xaf\xdf\x7d\x37\x04\x59\x15\x21\xf9\x4e\x07\x77" //n8-...}7.Y.!.N.w |
/* 0f70 */ "\x80\xae\x68\x57\xc2\xb1\xc0\xd6\x7c\x33\x0e\x0c\x36\x80\x7f\x38" //..hW....|3..6..8 |
/* 0f80 */ "\xbe\x7b\xcf\x22\x08\x46\x02\x86\x42\x3f\xbb\x01\x49\x30\x18\xd4" //.{.".F..B?..I0.. |
/* 0f90 */ "\x10\x30\x7c\x85\x40\x4b\xc3\xcc\x04\x40\x07\xe0\x0e\xf0\x44\x88" //.0|.@K...@....D. |
/* 0fa0 */ "\x60\x27\x74\x23\xdd\xce\x03\x33\x80\x9d\x18\x0a\x4c\x31\xe1\x48" //`'t#...3....L1.H |
/* 0fb0 */ "\xc0\x57\x70\xc0\x21\xe4\x43\x93\x0d\xa0\x23\xdd\xb7\xc7\xa8\x20" //.Wp.!.C...#....  |
/* 0fc0 */ "\x83\x58\xe5\xf0\xf0\xf1\x06\x61\x80\xaa\x51\x87\x3f\xbb\x1e\x3c" //.X.....a..Q.?..< |
/* 0fd0 */ "\x84\x1a\xc0\x77\xd7\x78\x48\x11\xe5\xb7\xc7\x79\x9c\x17\xfb\xa2" //...w.xH....y.... |
/* 0fe0 */ "\x40\xea\xc3\x68\xc0\x36\x8f\x3e\x04\xfb\xf4\x14\x00\x40\x30\x52" //@..h.6.>.....@0R |
/* 0ff0 */ "\x90\x81\x20\x82\x20\x83\x60\x76\x61\xb4\x04\xfa\x7d\xfe\xe2\x8c" //.. . .`va...}... |
/* 1000 */ "\x83\x50\x81\x20\xd6\x10\x6d\x0f\x3f\xe0\x83\x50\x09\xf6\x21\x80" //.P. ..m.?..P..!. |
/* 1010 */ "\x57\xe5\x88\x16\x3e\x59\xc3\xb5\x31\x06\x51\xd4\xee\x30\x66\xd3" //W...>Y..1.Q..0f. |
/* 1020 */ "\x00\x10\xaa\x00\x03\x01\x58\x0b\xb9\x80\x07\xc8\x36\x00\x03\x0c" //......X.....6... |
/* 1030 */ "\x56\xc8\xf0\x03\xb7\xb9\x06\xa0\x43\xca\xb5\x80\x18\x40\x01\xd0" //V.......C....@.. |
/* 1040 */ "\x34\x1b\x46\x00\x3a\x0c\x72\xa8\x79\xcc\x00\x19\x80\xad\xa2\x00" //4.F.:.r.y....... |
/* 1050 */ "\x00\x51\xca\x6d\xe9\x13\x00\x6b\x0d\xa2\x09\xe2\xb4\xc3\x01\x60" //.Q.m...k.......` |
/* 1060 */ "\x90\x03\xbd\x64\x60\x11\x61\xb4\x7e\x28\x84\x00\x10\x70\x82\x08" //...d`.a.~(...p.. |
/* 1070 */ "\x72\xfd\x5a\x3c\x32\x20\x88\x75\x9d\x78\x50\xc0\x28\xc3\x68\x0b" //r.Z<2 .u.xP.(.h. |
/* 1080 */ "\xf1\xfb\xa7\xc3\xc0\x8f\x8a\x58\xf8\xa0\x02\x8f\x29\x45\x20\x81" //.......X....)E . |
/* 1090 */ "\x22\x08\x83\x28\x41\x9c\x60\x30\x35\xa3\xa9\x13\x01\xea\x5f\xf7" //"..(A.`05....._. |
/* 10a0 */ "\x63\x60\xc6\x09\xc7\x60\x39\x80\x86\x0d\xfb\x58\x80\xa4\xa0\x47" //c`...`9....X...G |
/* 10b0 */ "\xa0\x0a\x80\x00\x62\x78\x14\x59\xc9\xda\x84\x41\x72\x7d\xd8\x82" //....bx.Y...Ar}.. |
/* 10c0 */ "\xe1\x01\xc0\x57\xf1\x40\x0a\x8e\x84\xd0\xa3\x81\x34\x27\xc0\x9e" //...W.@......4'.. |
/* 10d0 */ "\xc4\x27\xa0\x9a\x12\xc8\x4c\x37\x00\x3f\xdf\x5d\xd5\xc0\x01\x3c" //.'....L7.?.]...< |
/* 10e0 */ "\x46\x62\x94\xa4\xc6\xe2\xc7\x6e\x5a\x09\x3a\xc3\x8b\x6c\x01\x70" //Fb.....nZ.:..l.p |
/* 10f0 */ "\x60\xc0\x64\xe8\x14\x74\xca\xfd\xca\x76\x20\xf7\xea\xf0\xdc\x76" //`.d..t...v ....v |
/* 1100 */ "\x26\x85\x0b\xc0\x14\x40\x1c\x52\x7c\xa2\x57\xf1\xa3\x40\x52\x4a" //&....@.R|.W..@RJ |
/* 1110 */ "\x03\x03\x99\x61\x9e\x52\xd6\x02\x4b\x40\x97\x52\xe3\xf7\x76\xbe" //...a.R..K@.R..v. |
/* 1120 */ "\x58\xa9\xe9\xd3\x17\x74\xb7\x6d\xb0\x1c\x04\x60\x06\xa0\x06\xdc" //X....t.m...`.... |
/* 1130 */ "\xd7\xc6\xa2\x20\x2a\xc6\x5d\x3a\xc3\xa4\x4f\xf6\xf6\xcf\xf2\x02" //... *.]:..O..... |
/* 1140 */ "\x63\x59\x01\x80\xc7\x7f\x11\x87\xe4\x27\xf7\x7a\x80\x24\x1f\xa2" //cY.......'.z.$.. |
/* 1150 */ "\xfa\x2d\x23\xe9\x02\xaf\x43\x2a\x00\x72\x01\x00\x18\x48\xfd\x7e" //.-#...C*.r...H.~ |
/* 1160 */ "\x7c\xb6\x7a\x44\x3a\x35\x1d\x32\x9f\x2d\xbf\x49\x63\xf6\xff\xea" //|.zD:5.2.-.Ic... |
/* 1170 */ "\x03\x20\xb7\xf8\x3c\x42\x56\x03\x07\xaf\xdf\xe7\xa6\x4f\x41\x34" //. ..<BV......OA4 |
/* 1180 */ "\xf0\x07\xeb\xfd\x7e\xff\x36\xa6\x60\x28\xc0\x23\x0e\x0b\x03\x47" //....~.6.`(.#...G |
/* 1190 */ "\x4c\xb0\x10\x82\x05\x5d\x11\x1e\x6c\xf0\x18\x47\x93\x0a\xc1\x76" //L....]..l..G...v |
/* 11a0 */ "\x01\x80\xd7\x98\xfc\x9a\x81\xc8\xad\x0d\xb7\x3b\xad\x93\xf4\x80" //...........;.... |
/* 11b0 */ "\xc8\x39\x7f\xa7\xf5\x66\xdf\x1e\x01\x08\xd6\x40\x70\x19\x22\xdf" //.9...f.....@p.". |
/* 11c0 */ "\xc7\xee\x20\x4c\x56\x08\xe0\x4e\x10\x6f\x01\xa0\x60\x3f\x84\xfe" //.. LV..N.o..`?.. |
/* 11d0 */ "\x9d\x4a\x80\x25\x54\x00\x90\x0c\xfc\x06\x00\x2b\xfc\x82\x03\x90" //.J.%T......+.... |
/* 11e0 */ "\x16\x32\x9b\x54\x06\x74\xb5\xf8\xaf\xd4\xaa\x80\x12\x15\x37\xc6" //.2.T.t........7. |
/* 11f0 */ "\x4f\x41\x3c\x22\xa8\x01\x58\x5a\x80\x68\x77\x61\x58\x0c\x7e\xef" //OA<"..XZ.hwaX.~. |
/* 1200 */ "\xd4\xbd\x3f\xa7\x37\xfe\x08\xa0\x11\xf9\xb6\x1a\xf0\xcb\x9c\x54" //..?.7..........T |
/* 1210 */ "\x01\x24\x05\x7c\xb4\xe1\x74\xab\xd4\xc1\xe8\xa8\x7f\x87\x0d\x3e" //.$.|..t........> |
/* 1220 */ "\x1c\x30\x18\x80\xd4\x01\x6c\x68\x20\x30\x11\xd4\x30\x0f\x9e\xb1" //.0....lh 0..0... |
/* 1230 */ "\xd5\xc0\x60\x2f\xd1\xda\x2a\x15\xda\xa0\x09\x61\x85\xf8\x50\x40" //..`/..*....a..P@ |
/* 1240 */ "\x11\xf5\x40\x15\xff\xc8\x80\xc9\x40\x60\x27\xa9\x6c\x00\x5e\x2f" //..@.....@`'.l.^/ |
/* 1250 */ "\x89\x01\xcf\x98\x5e\xf1\x58\x3c\x3e\x04\x80\x5c\x3f\x42\x40\xf3" //....^.X<>...?B@. |
/* 1260 */ "\xf2\x84\x6d\x3e\x29\xa2\x3e\x7e\x88\xe1\xd7\x91\x95\xd3\xaa\x80" //..m>).>~........ |
/* 1270 */ "\x1b\x00\x40\x4c\x06\xbf\x56\xa8\x01\x58\x0e\xfc\x00\x22\xe0\x9e" //..@L..V..X...".. |
/* 1280 */ "\x5b\x01\x80\xbf\xdb\xf0\x1e\x38\xef\xd7\xc6\x0f\x0f\x80\xc0\x65" //[......8.......e |
/* 1290 */ "\x80\x30\x27\xdb\xfa\xb9\xcb\x00\x1f\x87\x22\xc3\x03\xe0\xd5\x10" //.0'......."..... |
/* 12a0 */ "\x37\xfa\x30\xf4\xfb\xf3\xc6\x44\x36\x70\x19\x3d\x7c\x50\x01\xcf" //7.0....D6p.=|P.. |
/* 12b0 */ "\xec\x7f\x14\x0a\x25\x74\x12\x61\x60\x04\xfd\x80\x44\x18\x18\x0b" //....%t.a`...D... |
/* 12c0 */ "\x37\x09\x65\xfe\xbd\xe7\x94\x33\xf1\xbd\x54\xa8\x01\x48\x56\x80" //7.e....3..T..HV. |
/* 12d0 */ "\x60\x75\xbb\xf5\x0e\xd1\x6d\x84\x30\x02\xf0\xc3\x1e\xa5\xd4\x40" //`u....m.0......@ |
/* 12e0 */ "\x0a\x40\x60\x51\xfa\x77\x8f\x40\x8c\x02\x1f\x99\x60\xac\x80\xd4" //.@`Q.w.@....`... |
/* 12f0 */ "\x00\x48\xee\xc0\x30\x45\x9b\xb1\x25\xa5\x86\xd1\x28\x98\x30\x03" //.H..0E..%...(.0. |
/* 1300 */ "\xc7\x42\x60\xe3\xa8\x77\xe8\x60\xf4\xd9\xfe\x8c\x7f\x3e\x5e\x8e" //.B`..w.`.....>^. |
/* 1310 */ "\xbe\xa9\xb3\xff\xa2\xe9\x55\x12\x83\x13\xe2\x72\x95\x5a\x98\xf9" //......U....r.Z.. |
/* 1320 */ "\x2c\x98\xea\x0c\x03\x71\xd0\x96\x5f\x94\x7a\x11\x16\xc0\x54\xca" //,....q.._.z...T. |
/* 1330 */ "\x45\x1c\x02\x06\xf5\x80\xcf\xcf\x81\x23\xb0\x13\x51\x9c\x0a\x35" //E........#..Q..5 |
/* 1340 */ "\x1c\x96\x10\x60\x2c\x12\xd2\x69\x00\x96\x80\x5c\x70\x22\xa0\x57" //...`,..i....p".W |
/* 1350 */ "\x03\xa7\x2a\x01\x18\x08\xea\x08\xb0\x06\x8c\x41\x00\x70\x05\x62" //..*........A.p.b |
/* 1360 */ "\xe1\x81\x2a\xc2\x30\x24\x19\xc6\x04\x7c\x8c\x01\x93\x30\x36\x70" //..*.0$...|...06p |
/* 1370 */ "\x0b\x90\x03\xe0\x4e\x03\xdf\x3c\x60\x49\xd8\x96\x80\x76\x0e\x40" //....N..<`I...v.@ |
/* 1380 */ "\xc8\xb0\x1c\x39\xec\x24\x33\x03\x86\xb0\x23\x8c\x5c\xb0\x07\x14" //...9.$3...#..... |
/* 1390 */ "\x60\x23\x95\xcb\x0c\x7d\x10\x6c\x18\x07\xb2\x30\x23\x44\x1b\x84" //`#...}.l...0#D.. |
/* 13a0 */ "\xe3\xaf\xd8\x46\xc0\x2e\xea\x2f\x1c\xb7\x10\x42\x46\x05\x94\x7d" //...F.../...BF..} |
/* 13b0 */ "\x83\xe0\x4f\x63\x07\x1f\x65\x45\xbc\x27\xd9\xae\x86\x6c\x08\xca" //..Oc..eE.'...l.. |
/* 13c0 */ "\xf2\x24\x12\x00\x61\xac\x46\x01\xa7\x80\x5b\x0f\x0a\x90\x17\x46" //.$..a.F...[....F |
/* 13d0 */ "\xe6\x04\x71\x60\x56\x28\xec\xa4\xf2\x5b\x81\xcb\x86\xc1\x06\xa1" //..q`V(...[...... |
/* 13e0 */ "\xe0\x91\x46\x05\xb1\xaa\x78\x06\x00\x77\x60\x57\x85\x40\x0a\xcd" //..F...x..w`W.@.. |
/* 13f0 */ "\xfc\x40\x6c\x2b\x1d\x23\x21\xf3\xc4\x41\x40\x9c\x51\x17\x15\x00" //.@l+.#!..A@.Q... |
/* 1400 */ "\x21\xcf\x10\x81\x08\x29\x18\x17\x22\x96\x42\x7b\x45\xc6\x06\x87" //!....)..".B{E... |
/* 1410 */ "\x20\xd6\x38\x35\xbc\x98\xb9\x78\xda\x7e\xa3\xa8\x92\x21\xf4\xdd" // .85...x.~...!.. |
/* 1420 */ "\x77\x12\xdf\x0b\x30\x24\x63\x88\x68\x82\x50\x03\xe3\x46\x05\x7f" //w...0$c.h.P..F.. |
/* 1430 */ "\x8f\x11\xb0\x38\x88\x17\xd5\x4f\x73\x05\xf0\x19\x93\xc3\x40\x66" //...8...Os.....@f |
/* 1440 */ "\x18\x87\x8b\x23\x03\x15\xc1\xb0\x4e\x23\x24\x8f\x8e\x16\xc0\x65" //...#....N#$....e |
/* 1450 */ "\x20\x00\x8c\x05\xd8\xf0\xb4\x60\x72\x20\x3f\xfc\x70\x03\x8f\x92" // ......`r ?.p... |
/* 1460 */ "\xf7\xc5\x7c\x97\xbb\x62\xe6\x01\xfc\x67\xdc\x51\x81\xc1\xc1\x1e" //..|..b...g.Q.... |
/* 1470 */ "\x6d\x70\x6d\x8f\x92\xc0\xdf\x78\x83\xe0\x2b\xb6\x06\x06\x8f\x92" //mpm....x..+..... |
/* 1480 */ "\xc0\xbc\xf1\x81\x41\xdd\x19\x42\x00\x38\x01\x3d\x81\xbc\x24\x50" //....A..B.8.=..$P |
/* 1490 */ "\x24\x7a\x03\xe8\x40\x41\x07\x47\x87\x80\xc2\xd8\x13\x34\x3d\xc9" //$z..@A.G.....4=. |
/* 14a0 */ "\x75\x51\x20\x84\x7a\x53\x0c\xaa\x52\xeb\xb1\x00\xc8\x31\x8f\x49" //uQ .zS..R....1.I |
/* 14b0 */ "\xd6\x06\xf6\x00\x7d\x18\x05\x20\xfa\x2f\x4b\x44\xb3\xd4\x8f\xa3" //....}.. ./KD.... |
/* 14c0 */ "\x00\x98\x1d\x52\x08\xfe\x8b\xd2\xcc\x0d\x36\xec\x5e\xf5\x3b\xc4" //...R......6.^.;. |
/* 14d0 */ "\xef\x4c\xd0\xef\x79\xc8\x55\x00\x03\x67\xe7\xcd\x8b\x28\x2a\xb7" //.L..y.U..g...(*. |
/* 14e0 */ "\x3e\x66\x40\x25\x40\x07\xa8\x01\x58\x02\x45\x80\xf7\x9d\xbd\xd8" //>f@%@...X.E..... |
/* 14f0 */ "\x05\xae\x02\xbe\x4a\x81\x80\xef\xd8\x0f\x40\x09\xf3\x83\x5c\x61" //....J.....@....a |
/* 1500 */ "\x43\xc0\x30\x39\x31\x8b\xac\xb5\x76\xdf\xcf\x12\x31\x5d\x98\x07" //C.091...v...1].. |
/* 1510 */ "\x13\xb9\xbe\x5c\x00\xf3\xcf\x62\x5d\xd7\xcb\x8c\x73\x05\x18\xa0" //.......b]...s... |
/* 1520 */ "\x6d\xf2\xd0\x23\x59\x29\x00\x08\xd7\x2f\x87\x65\x14\x98\xc6\x7d" //m..#Y).../.e...} |
/* 1530 */ "\x77\xe3\xec\xed\xb2\xe5\x0c\xe0\x5c\x03\xc1\xea\xea\x80\x0f\x3e" //w..............> |
/* 1540 */ "\x15\x43\x0e\x83\x38\x54\xb0\xf4\xe4\x7d\x78\x81\x08\x00\xe1\x47" //.C..8T...}x....G |
/* 1550 */ "\xc0\x00\x02\x00\xf8\xcc\x05\x80\x7f\x97\x7b\xd2\xa7\x59\x6f\x05" //..........{..Yo. |
/* 1560 */ "\xd7\x27\x48\x85\x29\x7c\x55\xad\x6b\xfc\xa8\xbc\x95\xc0\x99\x31" //.'H.)|U.k......1 |
/* 1570 */ "\x1d\x50\x04\x9d\x55\xbe\xa3\xe5\x20\x73\xfc\x3d\x1e\x00\x1e\xc0" //.P..U... s.=.... |
/* 1580 */ "\x87\x6e\x68\x2e\xec\x91\x87\x1a\x98\x81\xb1\x78\x9e\xb1\x5d\xf2" //.nh........x..]. |
/* 1590 */ "\x67\x9b\xff\xce\x10\x21\x73\x4b\x12\x00\x54\x03\x59\x4a\x48\x41" //g....!sK..T.YJHA |
/* 15a0 */ "\x32\x5b\xc1\x94\xa6\x80\x29\x81\x40\x9e\x60\x89\xfd\xde\x44\x21" //2[....).@.`...D! |
/* 15b0 */ "\x81\x1f\x55\x00\x47\xdb\x78\x0c\x0a\x3b\xd3\x61\x50\x03\xa1\x79" //..U.G.x..;.aP..y |
/* 15c0 */ "\x89\xe0\x68\xa8\x02\x4f\xee\xe0\x30\x2a\x5a\xd3\x00\x58\xd7\x31" //..h..O..0*Z..X.1 |
/* 15d0 */ "\x10\xf7\x82\x44\x78\xd5\xf5\x1e\xbb\xd4\x01\xa4\x5c\xa6\x58\x41" //...Dx.........XA |
/* 15e0 */ "\xbd\xcf\x3a\x80\xd4\x79\xa4\x48\x1a\x0f\x8a\xc0\x96\xa5\xa0\x18" //..:..y.H........ |
/* 15f0 */ "\x11\x8c\x10\x83\xde\x80\x24\x0f\x26\x80\xd2\xbd\xa8\xfb\xac\x21" //......$.&......! |
/* 1600 */ "\x12\x01\xa8\x03\x7b\x0d\xc0\x2c\x75\xf5\xdf\x2a\x7c\xaf\x13\x69" //....{..,u..*|..i |
/* 1610 */ "\xad\x00\x03\xf6\x28\x01\x7f\x41\xed\x08\x06\xf6\x85\x82\xe1\xe9" //....(..A........ |
/* 1620 */ "\xb7\x09\x25\xbc\x06\x03\x16\x63\xc0\xaa\x15\xf5\x40\x64\x64\x38" //..%....c....@dd8 |
/* 1630 */ "\x6e\x2a\x38\x60\xb0\x2c\x7c\x54\x00\x60\x1c\x67\x40\x9e\x47\x54" //n*8`.,|T.`.g@.GT |
/* 1640 */ "\x7d\x99\x00\xbe\x72\x96\x35\x10\x18\x12\xf4\x77\xaa\x48\x09\x9f" //}...r.5....w.H.. |
/* 1650 */ "\xee\x80\xda\x9a\xc0\x7d\x5e\x61\x08\x90\x60\x5f\x16\x51\x7c\x00" //.....}^a..`_.Q|. |
/* 1660 */ "\xd4\x03\x3d\x84\xf0\xf2\x0b\x56\x01\x80\xcf\x44\x79\xd0\xcc\x06" //..=....V...Dy... |
/* 1670 */ "\xff\x56\xe9\xde\xa8\x00\xd3\xff\x83\xca\x73\x89\x80\xde\xdd\x00" //.V........s..... |
/* 1680 */ "\xd8\x30\x04\x0b\xc6\x91\x43\x18\x01\xa8\xf8\x6e\x01\x56\xbd\x16" //.0....C....n.V.. |
/* 1690 */ "\xfd\x20\x34\x0e\x74\x0a\xfd\xfe\xfb\xd6\xca\x4d\xb9\xeb\x8f\x6b" //. 4.t......M...k |
/* 16a0 */ "\xe0\x30\x2c\x48\x34\xf7\x2c\x20\x0f\xf8\xd1\xe5\x17\x8d\x18\x98" //.0,H4., ........ |
/* 16b0 */ "\x30\x01\x42\x20\x00\x81\xe2\x5d\xd8\xb1\x8c\x54\x01\xb8\x50\x3d" //0.B ...]...T..P= |
/* 16c0 */ "\x78\x01\x01\x81\x3f\xe1\x2b\x27\xfb\x54\x01\x2a\x3c\x57\x82\x10" //x...?.+'.T.*<W.. |
/* 16d0 */ "\x4c\x0c\x2e\x81\x50\x05\x4a\x10\xb8\x08\x54\x39\xcd\x8b\x13\x80" //L...P.J...T9.... |
/* 16e0 */ "\xa9\x6b\x44\x2c\x48\xa5\x30\x1c\x3f\x5f\x40\x30\x38\xf3\xe4\xf4" //.kD,H.0.?_@08... |
/* 16f0 */ "\xea\x3c\xf9\xc4\x15\x03\x3f\x9f\x1e\xa5\x8e\xa1\xd4\x01\x31\xa2" //.<....?.......1. |
/* 1700 */ "\x3b\x0e\xbe\x18\x30\x32\x10\x00\x7e\x2d\x35\xea\xb8\xe2\xba\xc0" //;...02..~-5..... |
/* 1710 */ "\x02\x20\x8a\x80\x15\x1c\xe4\x07\xb3\xc2\x84\xc0\x11\x80\x71\x50" //. ............qP |
/* 1720 */ "\x36\xee\xdd\x88\x07\x0d\xa8\x80\xc4\x20\x81\x75\x00\x1f\x10\xb8" //6........ .u.... |
/* 1730 */ "\xe7\xae\x91\xc0\x98\x0c\x7a\x4b\x80\x37\xd1\x4e\x71\x00\x16\xac" //......zK.7.Nq... |
/* 1740 */ "\x42\x92\x40\xe3\x68\x2b\x5c\x0b\xa8\x01\x3f\x76\x2d\x01\x3d\x70" //B.@.h+....?v-.=p |
/* 1750 */ "\x34\x3f\x3e\x74\x7d\x04\x00\xc1\x07\xeb\xe7\xa4\x51\xd0\x98\xea" //4?>t}.......Q... |
/* 1760 */ "\x9d\x40\x17\xb3\x02\x08\xfc\xf9\xca\x1c\x01\x60\xc0\xe6\x0b\x19" //.@.........`.... |
/* 1770 */ "\x1d\x0a\xe7\x1c\xa8\x01\xe2\x99\x58\x9f\x5d\xe8\x9f\x2c\x04\xb5" //........X.]..,.. |
/* 1780 */ "\x4e\xc0\x36\xe2\x1f\xac\x83\xc7\x42\x5f\x4a\xaf\xd0\xc3\xe9\xf0" //N.6.....B_J..... |
/* 1790 */ "\xf4\xda\x7d\x04\xbe\x8c\xe0\x9c\xd4\x2a\xfa\x32\xbc\xe9\x5e\x9a" //..}......*.2..^. |
/* 17a0 */ "\xbf\xd1\xa3\xd2\x53\xd4\xb8\xe8\x4c\x70\x25\xa3\x02\xb6\x39\x0e" //....S...Lp%...9. |
/* 17b0 */ "\x85\x28\x45\x8c\x75\x00\x3a\x7b\xdf\x00\xe8\x91\x62\x0e\xc6\x08" //.(E.u.:{....b... |
/* 17c0 */ "\x7e\x8c\x0e\x18\x82\xf1\xfa\x01\x60\x7a\xb5\x40\x12\xf7\x5e\x01" //~.......`z.@..^. |
/* 17d0 */ "\xa8\xf9\xd2\x1b\x5a\xe9\xef\x78\x21\xc9\x02\x8c\xe0\x7d\xa0\xcf" //....Z..x!....}.. |
/* 17e0 */ "\x02\x57\x08\x77\x06\x98\xa8\xb3\x70\x49\x0c\x65\x21\x56\x08\x72" //.W.w....pI.e!V.r |
/* 17f0 */ "\x86\x4f\x53\xc6\x0b\x57\x7d\xc8\xca\x5b\x2a\x8c\xf6\xee\x03\x8f" //.OS..W}..[*..... |
/* 1800 */ "\xf2\x02\xb8\x1f\xaa\x72\x56\xa3\x84\x5c\x51\x27\xd7\x7a\x8c\x4a" //.....rV...Q'.z.J |
/* 1810 */ "\x28\x03\x89\xbc\x42\xb0\x14\xfe\x14\x40\x29\x00\x08\x0f\x50\x54" //(...B....@)...PT |
/* 1820 */ "\x0b\x12\x5f\xc0\xa4\x05\x53\xd2\xe0\x01\x57\x00\x0c\x01\x98\x59" //.._...S...W....Y |
/* 1830 */ "\xf2\xf3\x03\x47\x01\x38\xe7\x88\x87\xc6\x70\x24\xbb\x02\x32\x01" //...G.8....p$..2. |
/* 1840 */ "\x20\x24\xfe\x22\x8c\x09\x2b\x00\x98\x23\x7e\x68\x1c\xb9\x60\x47" // $."..+..#~h..`G |
/* 1850 */ "\x03\xc5\xb2\x23\xd0\x12\x02\x2d\xff\x8c\x18\x23\x95\x1c\xc1\x1c" //...#...-...#.... |
/* 1860 */ "\xab\xd8\x4e\x07\x93\x0f\x60\x8f\xd8\xc1\x1c\xb1\x63\xfe\xf3\x00" //..N...`.....c... |
/* 1870 */ "\xc0\xee\x33\x4f\xf0\xea\x91\x82\x35\xd4\x00\x1f\x14\x3a\x44\x04" //..3O....5....:D. |
/* 1880 */ "\xe0\x34\xdf\x00\x88\x02\xc4\x20\x48\x9d\x2a\xc2\x50\x19\x97\x81" //.4..... H.*.P... |
/* 1890 */ "\xa3\x02\x6d\x47\x3a\x90\x94\x06\x6f\xe4\x74\x80\x57\x02\x6b\xba" //..mG:...o.t.W.k. |
/* 18a0 */ "\x0f\x84\x60\x33\xcf\x0b\x00\x07\xc4\xba\x05\x3d\xce\x80\xc1\x7c" //..`3.......=...| |
/* 18b0 */ "\x2d\x24\x02\x94\xdb\x00\x9e\x2a\x3f\x73\xf9\x08\xc0\x61\x43\xac" //-$.....*?s...aC. |
/* 18c0 */ "\x01\x0c\x23\xe0\xbe\x37\x08\xc0\x5b\xc5\x30\x28\xe0\x07\x9d\xd0" //..#..7..[.0(.... |
/* 18d0 */ "\x42\x02\xe8\x27\xf2\xd8\x5e\xb5\x80\x60\x4f\x38\x28\x4a\x11\x40" //B..'..^..`O8(J.@ |
/* 18e0 */ "\x30\x50\x0c\x11\x0f\xfd\x69\x7a\xfc\x24\x6c\x7d\x6b\x17\x1f\xbe" //0P....iz.$l}k... |
/* 18f0 */ "\x2b\x15\xf9\x26\x9f\x92\x30\xc1\x49\x90\x05\xf2\x42\xb8\x29\x31" //+..&..0.I...B.)1 |
/* 1900 */ "\xf9\x8b\x21\x21\xf0\x11\xa0\xa0\x00\xfe\x8d\xa7\x0f\xf8\xb1\xff" //..!!............ |
/* 1910 */ "\x18\xa7\x7d\xf5\xa3\x05\x3e\xc4\x2b\xe4\xad\xbf\xc6\x11\xfe\x4a" //..}...>.+......J |
/* 1920 */ "\xa5\xfc\x5d\x10\xc2\x10\x1a\xbf\x8a\xe3\x05\xaa\x20\x35\xb0\x90" //..]......... 5.. |
/* 1930 */ "\x0c\x6e\xc3\x2a\xc0\x00"                                         //.n.*.. |
// Sent dumped on RDP Client (5) 6454 bytes |
// send_server_update done |
// GraphicsUpdatePDU::init::Initializing orders batch mcs_userid=0 shareid=65538 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[1](30) used=1080 free=15198 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(930,677,64,61) rop=cc srcx=0 srcy=0 cache_idx=87) |
// front::draw:draw_tile((0, 704, 32, 32) (32, 32, 32, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[2](2064) used=1086 free=15192 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[3](30) used=1108 free=15170 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(0,704,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// Widget_load: image file [./tests/fixtures/ad8b.png] is PNG file |
// front::draw:draw_tile((100, 100, 26, 32) (80, 50, 26, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[4](1808) used=1123 free=15155 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[5](30) used=1438 free=14840 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(100,100,26,32) rop=cc srcx=0 srcy=0 cache_idx=1) |
// Front::end_update |
// GraphicsUpdatePDU::flush_orders: order_count=6 |
// send_server_update: fastpath_support=yes compression_support=yes shareId=65538 encryptionLevel=0 initiator=0 type=0 data_extra=6 |
// Sending on RDP Client (5) 1100 bytes |
/* 0000 */ "\x00\x84\x4c\x80\x21\x45\x04\xc9\xdd\xa2\xbc\x1f\x84\xe8\x79\x45" //..L.!E........yE |
/* 0010 */ "\x20\x8a\xe0\x18\x05\x88\x90\x03\x06\x81\x08\x24\xa5\x06\x31\x96" // ..........$..1. |
/* 0020 */ "\x31\xa4\xe2\x6d\xa6\x20\xf2\x02\x10\xaa\xcb\x78\xd5\x31\x33\x9a" //1..m. .....x.13. |
/* 0030 */ "\xa0\x0c\xd3\xc5\x71\xd0\x9a\x81\x7a\x13\x44\xfa\xef\x43\xfb\x2a" //....q...z.D..C.* |
/* 0040 */ "\xbc\xf4\x7f\xa0\xa1\x76\xdf\x81\x4b\x99\x23\x89\xdd\x55\xca\x09" //.....v..K.#..U.. |
/* 0050 */ "\xc9\x9e\x3a\x0b\xc0\x2f\x41\x78\x03\xe3\x86\x80\x9e\xd9\xfa\x1f" //..:../Ax........ |
/* 0060 */ "\xb0\xe5\xe4\x30\x84\x0e\x59\xf8\xf0\x3f\xb9\x00\x13\xee\x3a\x02" //...0..Y..?....:. |
/* 0070 */ "\x30\x00\x5c\xfd\xb5\x40\x0f\x87\x43\x89\xdc\x8c\xf4\x7b\xa9\x73" //0....@..C....{.s |
/* 0080 */ "\x9f\x2a\xd9\x82\x3f\x5b\xed\xef\x74\xbc\x60\x6c\xc0\xa1\x8c\x01" //.*..?[..t.`l.... |
/* 0090 */ "\xb4\xf1\xf0\x13\xa5\x7b\xa0\x2c\xfc\xd9\xfd\x6e\x87\xb3\x8a\x7d" //.....{.,...n...} |
/* 00a0 */ "\x9f\xe5\xcb\x6a\x01\x26\xd3\x5c\x73\x79\x18\xa0\x75\xc9\xce\x00" //...j.&..sy..u... |
/* 00b0 */ "\xe4\x5f\xda\x80\xf0\xb1\x23\xa0\xb4\xf4\x0c\xbe\x76\x68\x0c\x39" //._....#.....vh.9 |
/* 00c0 */ "\xcf\xf9\xb8\xcb\xaa\x00\x98\x68\x39\xe4\xea\xf3\xe0\x11\xc7\xc4" //.......h9....... |
/* 00d0 */ "\x11\x89\xd5\x5c\xe3\x67\x8d\xc4\xfc\xa9\xc4\xed\xcf\xde\xf1\xc3" //.....g.......... |
/* 00e0 */ "\xc2\x94\xbb\x6f\xe8\xd1\x00\x76\x2c\x1c\x1c\xf4\x1b\x12\x74\x47" //...o...v,.....tG |
/* 00f0 */ "\x85\x67\x8e\x64\xf8\x1e\x3f\x2c\xac\xd3\xbf\x9b\xd3\x2e\x89\xf4" //.g.d..?,........ |
/* 0100 */ "\x0d\xed\x68\x03\x29\xc0\x3d\x2f\x28\x80\xbc\x5f\x60\x7d\x06\xe0" //..h.).=/(.._`}.. |
/* 0110 */ "\x17\xf2\x4f\x91\x9d\x24\x03\x3f\x7a\x58\x7f\x80\x0e\xc9\xe6\x57" //..O..$.?zX.....W |
/* 0120 */ "\xcd\x02\xfb\x21\xea\x59\xf1\xfd\xfa\x05\xf5\xe0\x13\xc4\x4e\xa9" //...!.Y........N. |
/* 0130 */ "\xb3\xef\x71\x21\x0f\x24\xb7\xdd\x80\x4a\x47\x55\x80\x2b\x42\x57" //..q!.$...JGU.+BW |
/* 0140 */ "\xca\x7d\xf9\x9f\x5d\xfe\x53\xa1\x02\xa0\x09\xc2\x01\x50\x04\xaf" //.}..].S......P.. |
/* 0150 */ "\xdf\xc0\x71\x5b\x20\x67\x16\x4f\xa8\x5c\xd8\x1e\x9c\x90\x57\xcf" //..q[ g.O......W. |
/* 0160 */ "\x6a\xc7\x98\x20\x0f\xc6\xb0\x08\xf5\xec\xc0\x21\x9d\x9f\xec\xba" //j.. .......!.... |
/* 0170 */ "\x2e\x86\x7d\x6c\xa1\x63\xb4\x03\x08\x45\x40\x0e\x4b\xe9\x97\x8d" //..}l.c...E@.K... |
/* 0180 */ "\xe3\x0d\xfb\xaa\x00\x8b\x34\xef\xe0\x3d\xfc\xce\x01\xf1\xf5\xe0" //......4..=...... |
/* 0190 */ "\x0c\xb1\x8f\xe6\xbd\x44\xc5\x94\x61\x9c\x4f\xf0\xc0\x04\xce\x12" //.....D..a.O..... |
/* 01a0 */ "\xc0\x43\x38\x20\x0c\x12\x7d\x8a\x11\x8c\x3d\x10\x55\x00\x5a\x5a" //.C8 ..}...=.U.ZZ |
/* 01b0 */ "\xd6\x02\xc1\x04\xfb\x05\xf3\x5e\x33\x88\x26\x27\x6e\x89\x31\x8e" //.......^3.&'n.1. |
/* 01c0 */ "\x23\x16\x70\x96\x88\x0d\xd2\x38\x33\x89\xfe\x10\x21\x06\x3f\x17" //#.p....83...!.?. |
/* 01d0 */ "\xc4\x39\xcc\x05\x8f\x87\x5b\x80\xe8\x13\x89\x84\x4d\xa6\xb5\x00" //.9....[.....M... |
/* 01e0 */ "\x4e\x74\x4a\x80\x2f\x11\x17\x40\xe6\x26\x35\x7b\xa6\x28\x8c\x38" //NtJ./..@.&5{.(.8 |
/* 01f0 */ "\x14\xfd\xe1\x96\x17\x10\x73\xe4\x9e\x25\x6a\x80\x2e\x3b\xbd\x54" //......s..%j..;.T |
/* 0200 */ "\x01\x25\xbc\x1f\x01\x1c\x05\xc0\x18\x97\x9d\xdb\xcd\xc2\x05\xc7" //.%.............. |
/* 0210 */ "\xf1\x4b\x5a\xa8\xf0\x0c\x65\x35\x50\x21\x0e\x02\xb9\x70\x16\x8e" //.KZ...e5P!...p.. |
/* 0220 */ "\x84\x08\x11\x4e\x08\x0b\x54\x01\x7d\xde\xed\xf6\xc0\x25\xce\x7e" //...N..T.}....%.~ |
/* 0230 */ "\xec\x0c\xfc\x63\x01\xc1\xd2\x62\x00\x06\xa4\xa8\x96\x55\xb8\x1c" //...c...b.....U.. |
/* 0240 */ "\x51\xa6\xd3\x57\x1c\x4e\x82\xc0\x06\x6e\x83\x0a\x25\xce\x39\x08" //Q..W.N...n..%.9. |
/* 0250 */ "\xc5\x8a\x2c\x08\x86\x97\x63\x56\xc0\xc5\x19\x56\x5b\x87\x22\x99" //..,...cV...V[.". |
/* 0260 */ "\x15\xd9\x42\xa7\xbd\xe2\xcd\x00\x00\xed\x08\x42\x09\xc2\x21\x82" //..B........B..!. |
/* 0270 */ "\xc0\x58\x1e\xe9\x89\x94\x10\x16\xa4\xac\x93\x3a\x83\x18\xc0\x6d" //.X.........:...m |
/* 0280 */ "\x8b\x57\x39\xe6\x0a\x60\x51\x2c\x02\x70\xab\x00\x5d\x00\x2e\x40" //.W9..`Q,.p..]..@ |
/* 0290 */ "\x17\x00\x0b\x20\x0b\xa0\x05\x30\x05\x50\x02\x48\x30\x45\xa0\x00" //... ...0.P.H0E.. |
/* 02a0 */ "\x49\xb9\x84\x7e\x0c\x00\x3e\x41\x82\x38\x58\x2c\x40\x15\x82\x64" //I..~..>A.8X,@..d |
/* 02b0 */ "\xc3\x4c\xcc\x23\xf8\x23\xf9\x00\x15\x90\x05\xe0\xb2\x40\xe3\x23" //.L.#.#.......@.# |
/* 02c0 */ "\x08\xfc\x0f\x05\x94\x05\x30\x4e\x30\x75\x8f\xe6\x2c\x02\xbf\x99" //......0N0u..,... |
/* 02d0 */ "\x82\x5c\x8c\x5f\x32\x07\x98\x02\xfb\x9d\x3e\x64\x40\xa0\x03\xd8" //..._2.....>d@... |
/* 02e0 */ "\x28\x8d\x80\x1e\x43\xf0\x2c\x15\xbe\xc1\x37\x08\x1b\x08\xf8\x00" //(...C.,...7..... |
/* 02f0 */ "\xfe\x00\x08\x7d\x39\xc1\x25\x6e\xec\x2f\xa6\xb0\x0a\xe1\x3e\x67" //...}9.%n./....>g |
/* 0300 */ "\xc9\x82\x01\xec\x13\x9d\x1f\x24\xc0\x3d\x82\x7c\xe3\xe4\xa0\x03" //.......$.=.|.... |
/* 0310 */ "\xdf\x6e\xe4\x02\xbe\x14\x60\xbc\x3a\xf4\x83\xf5\x82\xc5\x39\x80" //.n....`.:.....9. |
/* 0320 */ "\x42\xb8\x00\x1e\x2d\xb6\x02\x76\xca\x10\x10\x08\x61\x70\xe8\xa0" //B...-..v....ap.. |
/* 0330 */ "\x08\x63\xe5\x45\x80\x0c\x04\x7c\x07\x00\xda\xb1\x0d\xf3\x0a\x3e" //.c.E...|.......> |
/* 0340 */ "\x3e\x43\x0f\xd7\xd7\x9e\x58\x72\x9d\xf6\x54\x41\x8a\x34\x0a\xe7" //>C....Xr..TA.4.. |
/* 0350 */ "\x35\xb6\x49\x68\xb6\x78\xac\x06\xd7\x1d\xa2\x8c\x51\xa9\x37\xfb" //5.Ih.x......Q.7. |
/* 0360 */ "\xf8\x10\x17\xd3\x5a\xe3\xac\xb6\x78\xac\x0a\xd1\x15\xb7\xcb\x59" //....Z...x......Y |
/* 0370 */ "\x86\x28\xc0\x9d\xd6\xa0\x7f\xeb\x63\x6a\x8e\xdc\x65\xad\x03\x14" //.(......cj..e... |
/* 0380 */ "\x60\xbf\x99\x16\xd9\x2d\xce\x6a\xd4\x31\x46\x0f\x6d\xfa\x0a\x80" //`....-.j.1F.m... |
/* 0390 */ "\x18\x7f\x74\x75\xc6\x5a\x2f\xcd\x74\x80\x04\x03\x01\xb6\x49\x70" //..tu.Z/.t.....Ip |
/* 03a0 */ "\x44\xd0\x17\x08\x7f\x2c\xf1\x5c\x10\xfe\x1c\xe1\x96\xb9\xcd\x45" //D....,.........E |
/* 03b0 */ "\xf9\x01\x84\xc0\x60\x36\xff\xcb\x23\xff\x8c\x1f\x44\xc3\xc2\x8a" //....`6..#...D... |
/* 03c0 */ "\xdd\x78\x5d\x96\xff\x90\x42\x5f\x5e\x05\x26\xc2\x64\x9c\xd6\xe7" //.x]...B_^.&.d... |
/* 03d0 */ "\xf5\x06\xff\x9c\x07\xb2\x8c\x51\xb8\xb2\x3c\x00\x6c\xf1\x2b\xc4" //.......Q..<.l.+. |
/* 03e0 */ "\x0a\x80\x18\x59\x80\x4f\x82\x42\xd8\x09\x95\x73\x9a\x8d\x79\x99" //...Y.O.B...s..y. |
/* 03f0 */ "\x71\x96\x8d\x31\x79\x35\x69\xf9\x21\x52\x1a\x80\x18\x60\x40\x0f" //q..1y5i.!R...`@. |
/* 0400 */ "\xcf\x8f\x3f\x40\x9b\x10\xc5\x1b\xf2\xb0\x8e\xf9\x8f\xd4\x6b\x6c" //..?@..........kl |
/* 0410 */ "\x97\xe8\x5d\x40\x14\x28\xc0\x87\xd6\x4c\x0c\xa6\xa7\x05\x31\x8c" //..]@.(...L....1. |
/* 0420 */ "\x51\x88\x43\x7c\x80\x09\xfb\x88\x15\xba\x4b\x68\x8a\x8d\x7c\x95" //Q.C|......Kh..|. |
/* 0430 */ "\x40\x14\x64\xa0\x06\x06\x79\x67\x7d\xc0\xc4\xc1\x36\xfc\x51\xde" //@.d...yg}...6.Q. |
/* 0440 */ "\x52\x01\x0e\x01\x64\x00\x64\xc8\xf8\x00\x10\x00"                 //R...d.d..... |
// Sent dumped on RDP Client (5) 1100 bytes |
// send_server_update done |
// GraphicsUpdatePDU::init::Initializing orders batch mcs_userid=0 shareid=65538 |
// Listener closed |
// Incoming socket 5 (ip=10.10.47.175) |
// Socket RDP Client (5) : closing connection |
// RDP Client (0): total_received=1716, total_sent=19491 |
} /* end outdata */;

const char indata[] =
{
// Listen: binding socket 4 on 0.0.0.0:3389 |
// Listen: listening on socket 4 |
// Incoming socket to 5 (ip=10.10.47.175) |
// Reading font file ./tests/fixtures/sans-10.fv1 |
// font name <Bitstream Vera Sans> size <10> |
// Font file ./tests/fixtures/sans-10.fv1 defines glyphs up to 256 |
// Front::incoming |
// Front::incoming:CONNECTION_INITIATION |
// Front::incoming::receiving x224 request PDU |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x13"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 15 bytes |
// Recv done on RDP Client (5) 15 bytes |
/* 0000 */ "\x0e\xe0\x00\x00\x00\x00\x00\x01\x00\x08\x00\x01\x00\x00\x00"     //............... |
// Dump done on RDP Client (5) 15 bytes |
// CR Recv: PROTOCOL TLS 1.0 |
// Front::incoming::sending x224 connection confirm PDU |
// -----------------> Front::TLS Support Enabled |
// CC Send: PROTOCOL TLS 1.0 |
// Sending on RDP Client (5) 19 bytes |
// /* 0000 */ "\x03\x00\x00\x13\x0e\xd0\x00\x00\x00\x00\x00\x02\x01\x08\x00\x01" //................ |
// /* 0010 */ "\x00\x00\x00"                                                     //... |
// Sent dumped on RDP Client (5) 19 bytes |
// RIO *::enable_server_tls() start |
// RIO *::SSL_CTX_set_options() |
// RIO *::enable_server_tls() done |
// Front::incoming::Basic Settings Exchange |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x01\xac"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 424 bytes |
// Recv done on RDP Client (5) 424 bytes |
/* 0000 */ "\x02\xf0\x80\x7f\x65\x82\x01\xa0\x04\x01\x01\x04\x01\x01\x01\x01" //....e........... |
/* 0010 */ "\xff\x30\x19\x02\x01\x22\x02\x01\x02\x02\x01\x00\x02\x01\x01\x02" //.0...".......... |
/* 0020 */ "\x01\x00\x02\x01\x01\x02\x02\xff\xff\x02\x01\x02\x30\x19\x02\x01" //............0... |
/* 0030 */ "\x01\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02\x01\x00\x02\x01\x01" //................ |
/* 0040 */ "\x02\x02\x04\x20\x02\x01\x02\x30\x1c\x02\x02\xff\xff\x02\x02\xfc" //... ...0........ |
/* 0050 */ "\x17\x02\x02\xff\xff\x02\x01\x01\x02\x01\x00\x02\x01\x01\x02\x02" //................ |
/* 0060 */ "\xff\xff\x02\x01\x02\x04\x82\x01\x3f\x00\x05\x00\x14\x7c\x00\x01" //........?....|.. |
/* 0070 */ "\x81\x36\x00\x08\x00\x10\x00\x01\xc0\x00\x44\x75\x63\x61\x81\x28" //.6........Duca.( |
/* 0080 */ "\x01\xc0\xd8\x00\x04\x00\x08\x00\x00\x04\x00\x03\x01\xca\x03\xaa" //................ |
/* 0090 */ "\x0c\x04\x00\x00\xb0\x1d\x00\x00\x52\x00\x44\x00\x50\x00\x2d\x00" //........R.D.P.-. |
/* 00a0 */ "\x54\x00\x45\x00\x53\x00\x54\x00\x00\x00\x00\x00\x00\x00\x00\x00" //T.E.S.T......... |
/* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00c0 */ "\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0100 */ "\x00\x00\x00\x00\x01\xca\x01\x00\x00\x00\x00\x00\x10\x00\x0f\x00" //................ |
/* 0110 */ "\x2d\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //-............... |
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0150 */ "\x00\x00\x02\x00\x01\x00\x00\x00\x04\xc0\x0c\x00\x11\x00\x00\x00" //................ |
/* 0160 */ "\x00\x00\x00\x00\x02\xc0\x0c\x00\x1b\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0170 */ "\x03\xc0\x38\x00\x04\x00\x00\x00\x72\x64\x70\x64\x72\x00\x00\x00" //..8.....rdpdr... |
/* 0180 */ "\x00\x00\x80\x80\x72\x64\x70\x73\x6e\x64\x00\x00\x00\x00\x00\xc0" //....rdpsnd...... |
/* 0190 */ "\x64\x72\x64\x79\x6e\x76\x63\x00\x00\x00\x80\xc0\x63\x6c\x69\x70" //drdynvc.....clip |
/* 01a0 */ "\x72\x64\x72\x00\x00\x00\xa0\xc0"                                 //rdr..... |
// Dump done on RDP Client (5) 424 bytes |
// GCC::UserData tag=c001 length=216 |
// Received from Client GCC User Data CS_CORE (216 bytes) |
// cs_core::version [80004] RDP 5.0, 5.1, 5.2, and 6.0 clients) |
// cs_core::desktopWidth  = 1024 |
// cs_core::desktopHeight = 768 |
// cs_core::colorDepth    = [ca01] [RNS_UD_COLOR_8BPP] superseded by postBeta2ColorDepth |
// cs_core::SASSequence   = [aa03] [Unknown] |
// cs_core::keyboardLayout= 040c |
// cs_core::clientBuild   = 7600 |
// cs_core::clientName    = RDP-TEST |
// cs_core::keyboardType  = [0004] IBM enhanced (101-key or 102-key) keyboard |
// cs_core::keyboardSubType      = [0000] OEM code |
// cs_core::keyboardFunctionKey  = 12 function keys |
// cs_core::imeFileName    =  |
// cs_core::postBeta2ColorDepth  = [ca01] [8 bpp] |
// cs_core::clientProductId = 1 |
// cs_core::serialNumber = 0 |
// cs_core::highColorDepth  = [0010] [16-bit 565 RGB mask] |
// cs_core::supportedColorDepths  = [000f] [24/16/15/32] |
// cs_core::earlyCapabilityFlags  = [002d] |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_ERRINFO_PDU |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_STATUSINFO_PDU |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_STRONG_ASYMMETRIC_KEYS |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_VALID_CONNECTION_TYPE |
// cs_core::clientDigProductId=[00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 |
// cs_core::connectionType  = 2 |
// cs_core::pad1octet  = 0 |
// cs_core::serverSelectedProtocol = 1 |
// GCC::UserData tag=c004 length=12 |
// Receiving from Client GCC User Data CS_CLUSTER (12 bytes) |
// cs_cluster::flags [0011] |
// cs_cluster::redirectedSessionID = 0 |
// GCC::UserData tag=c002 length=12 |
// Received from Client GCC User Data CS_SECURITY (12 bytes) |
// CSSecGccUserData::encryptionMethods 27 |
// CSSecGccUserData::extEncryptionMethods 0 |
// GCC::UserData tag=c003 length=56 |
// Received from Client GCC User Data CS_NET (56 bytes) |
// cs_net::channelCount   = 4 |
// cs_net::channel '   rdpdr' [1004] INITIALIZED COMPRESS_RDP |
// cs_net::channel '  rdpsnd' [1005] INITIALIZED |
// cs_net::channel ' drdynvc' [1006] INITIALIZED COMPRESS_RDP |
// cs_net::channel ' cliprdr' [1007] INITIALIZED COMPRESS_RDP SHOW_PROTOCOL |
// Sending to client GCC User Data SC_CORE (12 bytes) |
// sc_core::version [80004] RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 servers) |
// sc_core::clientRequestedProtocols  = 1 |
// Sending to client GCC User Data SC_NET (16 bytes) |
// sc_net::MCSChannelId   = 1003 |
// sc_net::channelCount   = 4 |
// sc_net::channel[1004]::id = 1004 |
// sc_net::channel[1005]::id = 1005 |
// sc_net::channel[1006]::id = 1006 |
// sc_net::channel[1007]::id = 1007 |
// Sending to client GCC User Data SC_SECURITY (12 bytes) |
// sc_security::encryptionMethod = 0 |
// sc_security::encryptionLevel  = 0 |
// Sending on RDP Client (5) 109 bytes |
// /* 0000 */ "\x03\x00\x00\x6d\x02\xf0\x80\x7f\x66\x63\x0a\x01\x00\x02\x01\x00" //...m....fc...... |
// /* 0010 */ "\x30\x1a\x02\x01\x22\x02\x01\x03\x02\x01\x00\x02\x01\x01\x02\x01" //0..."........... |
// /* 0020 */ "\x00\x02\x01\x01\x02\x03\x00\xff\xf8\x02\x01\x02\x04\x3f\x00\x05" //.............?.. |
// /* 0030 */ "\x00\x14\x7c\x00\x01\x2a\x14\x76\x0a\x01\x01\x00\x01\xc0\x00\x4d" //..|..*.v.......M |
// /* 0040 */ "\x63\x44\x6e\x80\x28\x01\x0c\x0c\x00\x04\x00\x08\x00\x01\x00\x00" //cDn.(........... |
// /* 0050 */ "\x00\x03\x0c\x10\x00\xeb\x03\x04\x00\xec\x03\xed\x03\xee\x03\xef" //................ |
// /* 0060 */ "\x03\x02\x0c\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00"             //............. |
// Sent dumped on RDP Client (5) 109 bytes |
// Front::incoming::Channel Connection |
// Front::incoming::Recv MCS::ErectDomainRequest |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x0c"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 8 bytes |
// Recv done on RDP Client (5) 8 bytes |
/* 0000 */ "\x02\xf0\x80\x04\x01\x00\x01\x00"                                 //........ |
// Dump done on RDP Client (5) 8 bytes |
// Front::incoming::Recv MCS::AttachUserRequest |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x08"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 4 bytes |
// Recv done on RDP Client (5) 4 bytes |
/* 0000 */ "\x02\xf0\x80\x28"                                                 //...( |
// Dump done on RDP Client (5) 4 bytes |
// Front::incoming::Send MCS::AttachUserConfirm |
// Sending on RDP Client (5) 11 bytes |
// /* 0000 */ "\x03\x00\x00\x0b\x02\xf0\x80\x2e\x00\x00\x00"                     //........... |
// Sent dumped on RDP Client (5) 11 bytes |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x0c"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 8 bytes |
// Recv done on RDP Client (5) 8 bytes |
/* 0000 */ "\x02\xf0\x80\x38\x00\x00\x03\xe9"                                 //...8.... |
// Dump done on RDP Client (5) 8 bytes |
// Sending on RDP Client (5) 15 bytes |
// /* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xe9\x03\xe9"     //.......>....... |
// Sent dumped on RDP Client (5) 15 bytes |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x0c"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 8 bytes |
// Recv done on RDP Client (5) 8 bytes |
/* 0000 */ "\x02\xf0\x80\x38\x00\x00\x03\xeb"                                 //...8.... |
// Dump done on RDP Client (5) 8 bytes |
// Sending on RDP Client (5) 15 bytes |
// /* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xeb\x03\xeb"     //.......>....... |
// Sent dumped on RDP Client (5) 15 bytes |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x0c"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 8 bytes |
// Recv done on RDP Client (5) 8 bytes |
/* 0000 */ "\x02\xf0\x80\x38\x00\x00\x03\xec"                                 //...8.... |
// Dump done on RDP Client (5) 8 bytes |
// cjrq[0] = 1004 -> cjcf |
// Sending on RDP Client (5) 15 bytes |
// /* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xec\x03\xec"     //.......>....... |
// Sent dumped on RDP Client (5) 15 bytes |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x0c"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 8 bytes |
// Recv done on RDP Client (5) 8 bytes |
/* 0000 */ "\x02\xf0\x80\x38\x00\x00\x03\xed"                                 //...8.... |
// Dump done on RDP Client (5) 8 bytes |
// cjrq[1] = 1005 -> cjcf |
// Sending on RDP Client (5) 15 bytes |
// /* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xed\x03\xed"     //.......>....... |
// Sent dumped on RDP Client (5) 15 bytes |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x0c"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 8 bytes |
// Recv done on RDP Client (5) 8 bytes |
/* 0000 */ "\x02\xf0\x80\x38\x00\x00\x03\xee"                                 //...8.... |
// Dump done on RDP Client (5) 8 bytes |
// cjrq[2] = 1006 -> cjcf |
// Sending on RDP Client (5) 15 bytes |
// /* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xee\x03\xee"     //.......>....... |
// Sent dumped on RDP Client (5) 15 bytes |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x0c"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 8 bytes |
// Recv done on RDP Client (5) 8 bytes |
/* 0000 */ "\x02\xf0\x80\x38\x00\x00\x03\xef"                                 //...8.... |
// Dump done on RDP Client (5) 8 bytes |
// cjrq[3] = 1007 -> cjcf |
// Sending on RDP Client (5) 15 bytes |
// /* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xef\x03\xef"     //.......>....... |
// Sent dumped on RDP Client (5) 15 bytes |
// Front::incoming::RDP Security Commencement |
// TLS mode: exchange packet disabled |
// Front::incoming |
// Front::incoming::Secure Settings Exchange |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x01\x4b"                                                     //..K |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 327 bytes |
// Recv done on RDP Client (5) 327 bytes |
/* 0000 */ "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\x3c\x40\x00\x00\x00\x00" //...d....p.<@.... |
/* 0010 */ "\x00\x00\x00\xb3\x47\x0b\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00" //....G........... |
/* 0020 */ "\x00\x00\x00\x78\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x1a" //...x............ |
/* 0030 */ "\x00\x31\x00\x30\x00\x2e\x00\x31\x00\x30\x00\x2e\x00\x34\x00\x37" //.1.0...1.0...4.7 |
/* 0040 */ "\x00\x2e\x00\x31\x00\x37\x00\x35\x00\x00\x00\x40\x00\x43\x00\x3a" //...1.7.5...@.C.: |
/* 0050 */ "\x00\x5c\x00\x57\x00\x49\x00\x4e\x00\x44\x00\x4f\x00\x57\x00\x53" //...W.I.N.D.O.W.S |
/* 0060 */ "\x00\x5c\x00\x73\x00\x79\x00\x73\x00\x74\x00\x65\x00\x6d\x00\x33" //...s.y.s.t.e.m.3 |
/* 0070 */ "\x00\x32\x00\x5c\x00\x6d\x00\x73\x00\x74\x00\x73\x00\x63\x00\x61" //.2...m.s.t.s.c.a |
/* 0080 */ "\x00\x78\x00\x2e\x00\x64\x00\x6c\x00\x6c\x00\x00\x00\xc4\xff\xff" //.x...d.l.l...... |
/* 0090 */ "\xff\x50\x00\x61\x00\x72\x00\x69\x00\x73\x00\x2c\x00\x20\x00\x4d" //.P.a.r.i.s.,. .M |
/* 00a0 */ "\x00\x61\x00\x64\x00\x72\x00\x69\x00\x64\x00\x00\x00\x00\x00\x00" //.a.d.r.i.d...... |
/* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00d0 */ "\x00\x00\x00\x0a\x00\x00\x00\x05\x00\x03\x00\x00\x00\x00\x00\x00" //................ |
/* 00e0 */ "\x00\x00\x00\x00\x00\x50\x00\x61\x00\x72\x00\x69\x00\x73\x00\x2c" //.....P.a.r.i.s., |
/* 00f0 */ "\x00\x20\x00\x4d\x00\x61\x00\x64\x00\x72\x00\x69\x00\x64\x00\x00" //. .M.a.d.r.i.d.. |
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x05\x00\x02\x00\x00" //................ |
/* 0130 */ "\x00\x00\x00\x00\x00\xc4\xff\xff\xff\x00\x00\x00\x00\x07\x00\x00" //................ |
/* 0140 */ "\x00\x00\x00\x64\x00\x00\x00"                                     //...d... |
// Dump done on RDP Client (5) 327 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x00, 0x00, 0x00, 0x00, 0xb3, 0x47, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,  // .....G.......... |
// /* 0010 */ 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,  // ....x........... |
// /* 0020 */ 0x1a, 0x00, 0x31, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x34, 0x00,  // ..1.0...1.0...4. |
// /* 0030 */ 0x37, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x37, 0x00, 0x35, 0x00, 0x00, 0x00, 0x40, 0x00, 0x43, 0x00,  // 7...1.7.5...@.C. |
// /* 0040 */ 0x3a, 0x00, 0x5c, 0x00, 0x57, 0x00, 0x49, 0x00, 0x4e, 0x00, 0x44, 0x00, 0x4f, 0x00, 0x57, 0x00,  // :...W.I.N.D.O.W. |
// /* 0050 */ 0x53, 0x00, 0x5c, 0x00, 0x73, 0x00, 0x79, 0x00, 0x73, 0x00, 0x74, 0x00, 0x65, 0x00, 0x6d, 0x00,  // S...s.y.s.t.e.m. |
// /* 0060 */ 0x33, 0x00, 0x32, 0x00, 0x5c, 0x00, 0x6d, 0x00, 0x73, 0x00, 0x74, 0x00, 0x73, 0x00, 0x63, 0x00,  // 3.2...m.s.t.s.c. |
// /* 0070 */ 0x61, 0x00, 0x78, 0x00, 0x2e, 0x00, 0x64, 0x00, 0x6c, 0x00, 0x6c, 0x00, 0x00, 0x00, 0xc4, 0xff,  // a.x...d.l.l..... |
// /* 0080 */ 0xff, 0xff, 0x50, 0x00, 0x61, 0x00, 0x72, 0x00, 0x69, 0x00, 0x73, 0x00, 0x2c, 0x00, 0x20, 0x00,  // ..P.a.r.i.s.,. . |
// /* 0090 */ 0x4d, 0x00, 0x61, 0x00, 0x64, 0x00, 0x72, 0x00, 0x69, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00,  // M.a.d.r.i.d..... |
// /* 00a0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00b0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00c0 */ 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00d0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x00, 0x61, 0x00, 0x72, 0x00, 0x69, 0x00, 0x73, 0x00,  // ......P.a.r.i.s. |
// /* 00e0 */ 0x2c, 0x00, 0x20, 0x00, 0x4d, 0x00, 0x61, 0x00, 0x64, 0x00, 0x72, 0x00, 0x69, 0x00, 0x64, 0x00,  // ,. .M.a.d.r.i.d. |
// /* 00f0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0100 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0110 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 0x00, 0x02, 0x00,  // ................ |
// /* 0120 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc4, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00,  // ................ |
// /* 0130 */ 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,                          // ....d... |
// RDP-5 Style logon |
// Receiving from client InfoPacket |
// InfoPacket::CodePage 0 |
// InfoPacket::flags 0xb47b3 |
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
// InfoPacket::flags:FORCE_ENCRYPTED_CS_PDU yes |
// InfoPacket::flags:INFO_RAIL no |
// InfoPacket::flags:INFO_LOGONERRORS yes |
// InfoPacket::flags:INFO_MOUSE_HAS_WHEEL yes |
// InfoPacket::flags:INFO_PASSWORD_IS_SC_PIN no |
// InfoPacket::flags:INFO_NOAUDIOPLAYBACK yes |
// InfoPacket::flags:INFO_USING_SAVED_CREDS no |
// InfoPacket::flags:RNS_INFO_AUDIOCAPTURE no |
// InfoPacket::flags:RNS_INFO_VIDEO_DISABLE no |
// InfoPacket::cbDomain 2 |
// InfoPacket::cbUserName 4 |
// InfoPacket::cbPassword 2 |
// InfoPacket::cbAlternateShell 2 |
// InfoPacket::cbWorkingDir 2 |
// InfoPacket::Domain  |
// InfoPacket::UserName x |
// InfoPacket::Password <hidden> |
// InfoPacket::AlternateShell  |
// InfoPacket::WorkingDir  |
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2 |
// InfoPacket::ExtendedInfoPacket::cbClientAddress 26 |
// InfoPacket::ExtendedInfoPacket::clientAddress 10.10.47.175 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 64 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\WINDOWS\system32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 0 |
// InfoPacket::ExtendedInfoPacket::performanceFlags 7 |
// InfoPacket::ExtendedInfoPacket::cbAutoReconnectLen 0 |
// InfoPacket::ExtendedInfoPacket::autoReconnectCookie  |
// InfoPacket::ExtendedInfoPacket::reserved1 100 |
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
// client info: performance flags before=0x00000007 after=0x00000007 default=0x00000000 present=0x00000000 not-present=0x00000000 |
// Front Keyboard Layout = 0x40c |
// Front::incoming::licencing not client_info.is_mce |
// Front::incoming::licencing send_lic_initial |
// Sec clear payload to send: |
// /* 0000 */ 0x01, 0x02, 0x3e, 0x01, 0x7b, 0x3c, 0x31, 0xa6, 0xae, 0xe8, 0x74, 0xf6, 0xb4, 0xa5, 0x03, 0x90,  // ..>.{<1...t..... |
// /* 0010 */ 0xe7, 0xc2, 0xc7, 0x39, 0xba, 0x53, 0x1c, 0x30, 0x54, 0x6e, 0x90, 0x05, 0xd0, 0x05, 0xce, 0x44,  // ...9.S.0Tn.....D |
// /* 0020 */ 0x18, 0x91, 0x83, 0x81, 0x00, 0x00, 0x04, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x4d, 0x00, 0x69, 0x00,  // ........,...M.i. |
// /* 0030 */ 0x63, 0x00, 0x72, 0x00, 0x6f, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x66, 0x00, 0x74, 0x00, 0x20, 0x00,  // c.r.o.s.o.f.t. . |
// /* 0040 */ 0x43, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x70, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x61, 0x00, 0x74, 0x00,  // C.o.r.p.o.r.a.t. |
// /* 0050 */ 0x69, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x32, 0x00, 0x33, 0x00,  // i.o.n.......2.3. |
// /* 0060 */ 0x36, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0xb8, 0x00,  // 6............... |
// /* 0070 */ 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x5c, 0x00,  // ................ |
// /* 0080 */ 0x52, 0x53, 0x41, 0x31, 0x48, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00,  // RSA1H.......?... |
// /* 0090 */ 0x01, 0x00, 0x01, 0x00, 0x01, 0xc7, 0xc9, 0xf7, 0x8e, 0x5a, 0x38, 0xe4, 0x29, 0xc3, 0x00, 0x95,  // .........Z8.)... |
// /* 00a0 */ 0x2d, 0xdd, 0x4c, 0x3e, 0x50, 0x45, 0x0b, 0x0d, 0x9e, 0x2a, 0x5d, 0x18, 0x63, 0x64, 0xc4, 0x2c,  // -.L>PE...*].cd., |
// /* 00b0 */ 0xf7, 0x8f, 0x29, 0xd5, 0x3f, 0xc5, 0x35, 0x22, 0x34, 0xff, 0xad, 0x3a, 0xe6, 0xe3, 0x95, 0x06,  // ..).?.5"4..:.... |
// /* 00c0 */ 0xae, 0x55, 0x82, 0xe3, 0xc8, 0xc7, 0xb4, 0xa8, 0x47, 0xc8, 0x50, 0x71, 0x74, 0x29, 0x53, 0x89,  // .U......G.Pqt)S. |
// /* 00d0 */ 0x6d, 0x9c, 0xed, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x48, 0x00,  // m..p..........H. |
// /* 00e0 */ 0xa8, 0xf4, 0x31, 0xb9, 0xab, 0x4b, 0xe6, 0xb4, 0xf4, 0x39, 0x89, 0xd6, 0xb1, 0xda, 0xf6, 0x1e,  // ..1..K...9...... |
// /* 00f0 */ 0xec, 0xb1, 0xf0, 0x54, 0x3b, 0x5e, 0x3e, 0x6a, 0x71, 0xb4, 0xf7, 0x75, 0xc8, 0x16, 0x2f, 0x24,  // ...T;^>jq..u../$ |
// /* 0100 */ 0x00, 0xde, 0xe9, 0x82, 0x99, 0x5f, 0x33, 0x0b, 0xa9, 0xa6, 0x94, 0xaf, 0xcb, 0x11, 0xc3, 0xf2,  // ....._3......... |
// /* 0110 */ 0xdb, 0x09, 0x42, 0x68, 0x29, 0x56, 0x58, 0x01, 0x56, 0xdb, 0x59, 0x03, 0x69, 0xdb, 0x7d, 0x37,  // ..Bh)VX.V.Y.i.}7 |
// /* 0120 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x0e, 0x00,  // ................ |
// /* 0130 */ 0x6d, 0x69, 0x63, 0x72, 0x6f, 0x73, 0x6f, 0x66, 0x74, 0x2e, 0x63, 0x6f, 0x6d, 0x00,        // microsoft.com. |
// Sending on RDP Client (5) 337 bytes |
// /* 0000 */ "\x03\x00\x01\x51\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x81\x42\x80" //...Q...h....p.B. |
// /* 0010 */ "\x00\x00\x00\x01\x02\x3e\x01\x7b\x3c\x31\xa6\xae\xe8\x74\xf6\xb4" //.....>.{<1...t.. |
// /* 0020 */ "\xa5\x03\x90\xe7\xc2\xc7\x39\xba\x53\x1c\x30\x54\x6e\x90\x05\xd0" //......9.S.0Tn... |
// /* 0030 */ "\x05\xce\x44\x18\x91\x83\x81\x00\x00\x04\x00\x2c\x00\x00\x00\x4d" //..D........,...M |
// /* 0040 */ "\x00\x69\x00\x63\x00\x72\x00\x6f\x00\x73\x00\x6f\x00\x66\x00\x74" //.i.c.r.o.s.o.f.t |
// /* 0050 */ "\x00\x20\x00\x43\x00\x6f\x00\x72\x00\x70\x00\x6f\x00\x72\x00\x61" //. .C.o.r.p.o.r.a |
// /* 0060 */ "\x00\x74\x00\x69\x00\x6f\x00\x6e\x00\x00\x00\x08\x00\x00\x00\x32" //.t.i.o.n.......2 |
// /* 0070 */ "\x00\x33\x00\x36\x00\x00\x00\x0d\x00\x04\x00\x01\x00\x00\x00\x03" //.3.6............ |
// /* 0080 */ "\x00\xb8\x00\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x06" //................ |
// /* 0090 */ "\x00\x5c\x00\x52\x53\x41\x31\x48\x00\x00\x00\x00\x02\x00\x00\x3f" //...RSA1H.......? |
// /* 00a0 */ "\x00\x00\x00\x01\x00\x01\x00\x01\xc7\xc9\xf7\x8e\x5a\x38\xe4\x29" //............Z8.) |
// /* 00b0 */ "\xc3\x00\x95\x2d\xdd\x4c\x3e\x50\x45\x0b\x0d\x9e\x2a\x5d\x18\x63" //...-.L>PE...*].c |
// /* 00c0 */ "\x64\xc4\x2c\xf7\x8f\x29\xd5\x3f\xc5\x35\x22\x34\xff\xad\x3a\xe6" //d.,..).?.5"4..:. |
// /* 00d0 */ "\xe3\x95\x06\xae\x55\x82\xe3\xc8\xc7\xb4\xa8\x47\xc8\x50\x71\x74" //....U......G.Pqt |
// /* 00e0 */ "\x29\x53\x89\x6d\x9c\xed\x70\x00\x00\x00\x00\x00\x00\x00\x00\x08" //)S.m..p......... |
// /* 00f0 */ "\x00\x48\x00\xa8\xf4\x31\xb9\xab\x4b\xe6\xb4\xf4\x39\x89\xd6\xb1" //.H...1..K...9... |
// /* 0100 */ "\xda\xf6\x1e\xec\xb1\xf0\x54\x3b\x5e\x3e\x6a\x71\xb4\xf7\x75\xc8" //......T;^>jq..u. |
// /* 0110 */ "\x16\x2f\x24\x00\xde\xe9\x82\x99\x5f\x33\x0b\xa9\xa6\x94\xaf\xcb" //./$....._3...... |
// /* 0120 */ "\x11\xc3\xf2\xdb\x09\x42\x68\x29\x56\x58\x01\x56\xdb\x59\x03\x69" //.....Bh)VX.V.Y.i |
// /* 0130 */ "\xdb\x7d\x37\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x0e" //.}7............. |
// /* 0140 */ "\x00\x0e\x00\x6d\x69\x63\x72\x6f\x73\x6f\x66\x74\x2e\x63\x6f\x6d" //...microsoft.com |
// /* 0150 */ "\x00"                                                             //. |
// Sent dumped on RDP Client (5) 337 bytes |
// Front::incoming::waiting for answer to lic_initial |
// Front::incoming |
// Front::incoming::WAITING_FOR_ANSWER_TO_LICENCE |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\xab"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 167 bytes |
// Recv done on RDP Client (5) 167 bytes |
/* 0000 */ "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x9c\x80\x00\xf9\x1e\x13" //...d....p....... |
/* 0010 */ "\x83\x98\x00\x01\x00\x00\x00\x00\x00\x01\x03\xb6\xbf\xf9\xc7\x44" //...............D |
/* 0020 */ "\x71\x16\xea\x72\x2a\xeb\xc0\xb0\x5f\x60\x31\x04\x6c\x1c\x95\xae" //q..r*..._`1.l... |
/* 0030 */ "\x1e\xda\xd1\xba\x94\x1a\xaa\xc3\xb2\x36\xac\x00\x00\x48\x00\x9f" //.........6...H.. |
/* 0040 */ "\x09\xee\xb6\x56\x80\xb2\x43\xb2\xdd\x34\xed\x89\x53\x65\x55\x72" //...V..C..4..SeUr |
/* 0050 */ "\x77\x3b\xc2\x79\xf5\x71\x9e\x2f\x6a\x56\x09\x85\x5b\xff\xd0\xb4" //w;.y.q./jV..[... |
/* 0060 */ "\x0f\x85\x0c\x68\x30\x05\x34\x4c\xd5\x1f\x58\x29\x26\xae\xd2\xfe" //...h0.4L..X)&... |
/* 0070 */ "\xa8\x83\x9d\x72\xe5\x5f\x0d\x49\x51\x74\x5b\x21\x78\x22\x58\x00" //...r._.IQt[!x"X. |
/* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x0f\x00\x0f\x00\x41\x64\x6d\x69\x6e" //...........Admin |
/* 0090 */ "\x69\x73\x74\x72\x61\x74\x65\x75\x72\x00\x10\x00\x09\x00\x52\x44" //istrateur.....RD |
/* 00a0 */ "\x50\x2d\x54\x45\x53\x54\x00"                                     //P-TEST. |
// Dump done on RDP Client (5) 167 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x13, 0x83, 0x98, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0xb6, 0xbf, 0xf9, 0xc7,  // ................ |
// /* 0010 */ 0x44, 0x71, 0x16, 0xea, 0x72, 0x2a, 0xeb, 0xc0, 0xb0, 0x5f, 0x60, 0x31, 0x04, 0x6c, 0x1c, 0x95,  // Dq..r*..._`1.l.. |
// /* 0020 */ 0xae, 0x1e, 0xda, 0xd1, 0xba, 0x94, 0x1a, 0xaa, 0xc3, 0xb2, 0x36, 0xac, 0x00, 0x00, 0x48, 0x00,  // ..........6...H. |
// /* 0030 */ 0x9f, 0x09, 0xee, 0xb6, 0x56, 0x80, 0xb2, 0x43, 0xb2, 0xdd, 0x34, 0xed, 0x89, 0x53, 0x65, 0x55,  // ....V..C..4..SeU |
// /* 0040 */ 0x72, 0x77, 0x3b, 0xc2, 0x79, 0xf5, 0x71, 0x9e, 0x2f, 0x6a, 0x56, 0x09, 0x85, 0x5b, 0xff, 0xd0,  // rw;.y.q./jV..[.. |
// /* 0050 */ 0xb4, 0x0f, 0x85, 0x0c, 0x68, 0x30, 0x05, 0x34, 0x4c, 0xd5, 0x1f, 0x58, 0x29, 0x26, 0xae, 0xd2,  // ....h0.4L..X)&.. |
// /* 0060 */ 0xfe, 0xa8, 0x83, 0x9d, 0x72, 0xe5, 0x5f, 0x0d, 0x49, 0x51, 0x74, 0x5b, 0x21, 0x78, 0x22, 0x58,  // ....r._.IQt[!x"X |
// /* 0070 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x41, 0x64, 0x6d, 0x69,  // ............Admi |
// /* 0080 */ 0x6e, 0x69, 0x73, 0x74, 0x72, 0x61, 0x74, 0x65, 0x75, 0x72, 0x00, 0x10, 0x00, 0x09, 0x00, 0x52,  // nistrateur.....R |
// /* 0090 */ 0x44, 0x50, 0x2d, 0x54, 0x45, 0x53, 0x54, 0x00,                          // DP-TEST. |
// Front::NEW_LICENSE_REQUEST |
// Sec clear payload to send: |
// /* 0000 */ 0xff, 0x02, 0x10, 0x00, 0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x28, 0x14, 0x00, 0x00,  // ............(... |
// Sending on RDP Client (5) 34 bytes |
// /* 0000 */ "\x03\x00\x00\x22\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x14\x80\x00" //..."...h....p... |
// /* 0010 */ "\x10\x00\xff\x02\x10\x00\x07\x00\x00\x00\x02\x00\x00\x00\x28\x14" //..............(. |
// /* 0020 */ "\x00\x00"                                                         //.. |
// Sent dumped on RDP Client (5) 34 bytes |
// Front::incoming::send_demand_active |
// Front::send_demand_active |
// Sending to client General caps (24 bytes) |
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
// Sending to client Bitmap caps (28 bytes) |
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
// Sending to client Font caps (8 bytes) |
// Font caps::fontSupportFlags 1 |
// Font caps::pad2octets 0 |
// Sending to client Order caps (88 bytes) |
// Order caps::terminalDescriptor 0 |
// Order caps::pad4octetsA 1078071040 |
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
// Order caps::orderSupport[TS_NEG_MEM3BLT_INDEX] 0 |
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
// Sending to client ColorCache caps (8 bytes) |
// ColorCache caps::colorTableCacheSize 6 |
// ColorCache caps::pad2octets 0 |
// Sending to client Pointer caps (10 bytes) |
// Pointer caps::colorPointerFlag 1 |
// Pointer caps::colorPointerCacheSize 25 |
// Pointer caps::pointerCacheSize 25 |
// Sending to client Share caps (8 bytes) |
// Share caps::nodeId 1001 |
// Share caps::pad2octets 46562 |
// Sending to client Input caps (88 bytes) |
// Input caps::inputFlags 41 |
// Input caps::pad2octetsA 0 |
// Input caps::keyboardLayout 0 |
// Input caps::keyboardType 0 |
// Input caps::keyboardSubType 0 |
// Input caps::keyboardFunctionKey 0 |
// Input caps::imeFileName 3952044240 |
// Sec clear payload to send: |
// /* 0000 */ 0x20, 0x01, 0x11, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x04, 0x00, 0x0a, 0x01, 0x52, 0x44,  //  .............RD |
// /* 0010 */ 0x50, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x01, 0x00, 0x03, 0x00, 0x00, 0x02,  // P............... |
// /* 0020 */ 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,  // ................ |
// /* 0030 */ 0x1c, 0x00, 0x10, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x04, 0x00, 0x03, 0x00, 0x00,  // ................ |
// /* 0040 */ 0x01, 0x00, 0x01, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x08, 0x00, 0x01, 0x00,  // ................ |
// /* 0050 */ 0x00, 0x00, 0x03, 0x00, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ....X........... |
// /* 0060 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x42, 0x0f, 0x00, 0x01, 0x00, 0x14, 0x00, 0x00, 0x00,  // ......@B........ |
// /* 0070 */ 0x01, 0x00, 0x2f, 0x00, 0x22, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,  // ../."........... |
// /* 0080 */ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01,  // ................ |
// /* 0090 */ 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0xa1, 0x06, 0x00, 0x00, 0x40, 0x42, 0x0f, 0x00, 0x40, 0x42,  // ..........@B..@B |
// /* 00a0 */ 0x0f, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x08, 0x00, 0x06, 0x00,  // ................ |
// /* 00b0 */ 0x00, 0x00, 0x08, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x19, 0x00, 0x19, 0x00, 0x09, 0x00, 0x08, 0x00,  // ................ |
// /* 00c0 */ 0xe9, 0x03, 0xe2, 0xb5, 0x0d, 0x00, 0x58, 0x00, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ......X.)....... |
// /* 00d0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00e0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00f0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0100 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0110 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// Sending on RDP Client (5) 303 bytes |
// /* 0000 */ "\x03\x00\x01\x2f\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x81\x20\x20" //.../...h....p.   |
// /* 0010 */ "\x01\x11\x00\xe9\x03\x02\x00\x01\x00\x04\x00\x0a\x01\x52\x44\x50" //.............RDP |
// /* 0020 */ "\x00\x08\x00\x00\x00\x01\x00\x18\x00\x01\x00\x03\x00\x00\x02\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x1c" //................ |
// /* 0040 */ "\x00\x10\x00\x01\x00\x01\x00\x01\x00\x00\x04\x00\x03\x00\x00\x01" //................ |
// /* 0050 */ "\x00\x01\x00\x00\x08\x01\x00\x00\x00\x0e\x00\x08\x00\x01\x00\x00" //................ |
// /* 0060 */ "\x00\x03\x00\x58\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //...X............ |
// /* 0070 */ "\x00\x00\x00\x00\x00\x40\x42\x0f\x00\x01\x00\x14\x00\x00\x00\x01" //.....@B......... |
// /* 0080 */ "\x00\x2f\x00\x22\x00\x01\x01\x01\x01\x00\x00\x00\x00\x01\x01\x01" //./."............ |
// /* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x01\x00" //................ |
// /* 00a0 */ "\x01\x00\x00\x00\x00\xa1\x06\x00\x00\x40\x42\x0f\x00\x40\x42\x0f" //.........@B..@B. |
// /* 00b0 */ "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x08\x00\x06\x00\x00" //................ |
// /* 00c0 */ "\x00\x08\x00\x0a\x00\x01\x00\x19\x00\x19\x00\x09\x00\x08\x00\xe9" //................ |
// /* 00d0 */ "\x03\xe2\xb5\x0d\x00\x58\x00\x29\x00\x00\x00\x00\x00\x00\x00\x00" //.....X.)........ |
// /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"     //............... |
// Sent dumped on RDP Client (5) 303 bytes |
// Front::incoming::ACTIVATED (new license request) |
// Front::incoming |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x02\x07"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 515 bytes |
// Recv done on RDP Client (5) 515 bytes |
/* 0000 */ "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\xf8\xf8\x01\x13\x00\xe9" //...d....p....... |
/* 0010 */ "\x03\x02\x00\x01\x00\xe9\x03\x06\x00\xe2\x01\x4d\x53\x54\x53\x43" //...........MSTSC |
/* 0020 */ "\x00\x13\x00\x00\x00\x01\x00\x18\x00\x01\x00\x03\x00\x00\x02\x00" //................ |
/* 0030 */ "\x00\x00\x00\x0d\x04\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x1c" //................ |
/* 0040 */ "\x00\x10\x00\x01\x00\x01\x00\x01\x00\x00\x04\x00\x03\x00\x00\x01" //................ |
/* 0050 */ "\x00\x01\x00\x00\x08\x01\x00\x00\x00\x03\x00\x58\x00\x00\x00\x00" //...........X.... |
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0070 */ "\x00\x01\x00\x14\x00\x00\x00\x01\x00\x00\x00\xaa\x00\x01\x01\x01" //................ |
/* 0080 */ "\x01\x01\x00\x00\x01\x01\x01\x00\x01\x00\x00\x00\x01\x01\x01\x01" //................ |
/* 0090 */ "\x01\x01\x01\x01\x00\x01\x01\x01\x00\x00\x00\x00\x00\xa1\x06\x06" //................ |
/* 00a0 */ "\x00\x00\x00\x00\x00\x00\x84\x03\x00\x00\x00\x00\x00\xe4\x04\x00" //................ |
/* 00b0 */ "\x00\x04\x00\x28\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //...(............ |
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x78\x00\x00" //.............x.. |
/* 00d0 */ "\x02\x78\x00\x00\x08\x51\x01\x00\x20\x0a\x00\x08\x00\x06\x00\x00" //.x...Q.. ....... |
/* 00e0 */ "\x00\x07\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\x00\x0c" //................ |
/* 00f0 */ "\x00\x00\x00\x00\x00\x02\x00\x02\x00\x08\x00\x0a\x00\x01\x00\x14" //................ |
/* 0100 */ "\x00\x15\x00\x09\x00\x08\x00\x00\x00\x00\x00\x0d\x00\x58\x00\x91" //.............X.. |
/* 0110 */ "\x00\x20\x00\x0c\x04\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x0c" //. .............. |
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0160 */ "\x00\x00\x00\x0c\x00\x08\x00\x01\x00\x00\x00\x0e\x00\x08\x00\x01" //................ |
/* 0170 */ "\x00\x00\x00\x10\x00\x34\x00\xfe\x00\x04\x00\xfe\x00\x04\x00\xfe" //.....4.......... |
/* 0180 */ "\x00\x08\x00\xfe\x00\x08\x00\xfe\x00\x10\x00\xfe\x00\x20\x00\xfe" //............. .. |
/* 0190 */ "\x00\x40\x00\xfe\x00\x80\x00\xfe\x00\x00\x01\x40\x00\x00\x08\x00" //.@.........@.... |
/* 01a0 */ "\x01\x00\x01\x03\x00\x00\x00\x0f\x00\x08\x00\x01\x00\x00\x00\x11" //................ |
/* 01b0 */ "\x00\x0c\x00\x01\x00\x00\x00\x00\x14\x64\x00\x14\x00\x0c\x00\x01" //.........d...... |
/* 01c0 */ "\x00\x00\x00\x00\x00\x00\x00\x15\x00\x0c\x00\x02\x00\x00\x00\x00" //................ |
/* 01d0 */ "\x0a\x00\x01\x16\x00\x28\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //.....(.......... |
/* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1a\x00\x08\x00\x00" //................ |
/* 0200 */ "\x00\x00\x00"                                                     //... |
// Dump done on RDP Client (5) 515 bytes |
// sec decrypted payload: |
// /* 0000 */ 0xf8, 0x01, 0x13, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0xe9, 0x03, 0x06, 0x00, 0xe2, 0x01,  // ................ |
// /* 0010 */ 0x4d, 0x53, 0x54, 0x53, 0x43, 0x00, 0x13, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x01, 0x00,  // MSTSC........... |
// /* 0020 */ 0x03, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0030 */ 0x00, 0x00, 0x02, 0x00, 0x1c, 0x00, 0x10, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x04,  // ................ |
// /* 0040 */ 0x00, 0x03, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00,  // ................ |
// /* 0050 */ 0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // X............... |
// /* 0060 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,  // ................ |
// /* 0070 */ 0xaa, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00,  // ................ |
// /* 0080 */ 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,  // ................ |
// /* 0090 */ 0x00, 0x00, 0xa1, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x03, 0x00, 0x00, 0x00,  // ................ |
// /* 00a0 */ 0x00, 0x00, 0xe4, 0x04, 0x00, 0x00, 0x04, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........(....... |
// /* 00b0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00c0 */ 0x00, 0x00, 0x78, 0x00, 0x00, 0x02, 0x78, 0x00, 0x00, 0x08, 0x51, 0x01, 0x00, 0x20, 0x0a, 0x00,  // ..x...x...Q.. .. |
// /* 00d0 */ 0x08, 0x00, 0x06, 0x00, 0x00, 0x00, 0x07, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00e0 */ 0x00, 0x00, 0x05, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x08, 0x00,  // ................ |
// /* 00f0 */ 0x0a, 0x00, 0x01, 0x00, 0x14, 0x00, 0x15, 0x00, 0x09, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0100 */ 0x0d, 0x00, 0x58, 0x00, 0x91, 0x00, 0x20, 0x00, 0x0c, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,  // ..X... ......... |
// /* 0110 */ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0120 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0130 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0140 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0150 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00,  // ................ |
// /* 0160 */ 0x0e, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x34, 0x00, 0xfe, 0x00, 0x04, 0x00,  // ..........4..... |
// /* 0170 */ 0xfe, 0x00, 0x04, 0x00, 0xfe, 0x00, 0x08, 0x00, 0xfe, 0x00, 0x08, 0x00, 0xfe, 0x00, 0x10, 0x00,  // ................ |
// /* 0180 */ 0xfe, 0x00, 0x20, 0x00, 0xfe, 0x00, 0x40, 0x00, 0xfe, 0x00, 0x80, 0x00, 0xfe, 0x00, 0x00, 0x01,  // .. ...@......... |
// /* 0190 */ 0x40, 0x00, 0x00, 0x08, 0x00, 0x01, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x08, 0x00,  // @............... |
// /* 01a0 */ 0x01, 0x00, 0x00, 0x00, 0x11, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x14, 0x64, 0x00,  // ..............d. |
// /* 01b0 */ 0x14, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x0c, 0x00,  // ................ |
// /* 01c0 */ 0x02, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x16, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00,  // ..........(..... |
// /* 01d0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 01e0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 01f0 */ 0x1a, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,                          // ........ |
// Front::incoming::sec_flags=0 |
// Front received CONFIRMACTIVEPDU |
// process_confirm_active |
// lengthSourceDescriptor = 6 |
// lengthCombinedCapabilities = 482 |
// Front::capability 0 / 19 |
// Receiving from client General caps (24 bytes) |
// General caps::major 1 |
// General caps::minor 3 |
// General caps::protocol 512 |
// General caps::pad2octetA 0 |
// General caps::compression type 0 |
// General caps::extra flags 40d |
// General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED yes |
// General caps::extraflags:LONG_CREDENTIALS_SUPPORTED yes |
// General caps::extraflags:AUTORECONNECT_SUPPORTED yes |
// General caps::extraflags:ENC_SALTED_CHECKSUM no |
// General caps::extraflags:NO_BITMAP_COMPRESSION_HDR yes |
// General caps::updateCapability 0 |
// General caps::remoteUnshare 0 |
// General caps::compressionLevel 0 |
// General caps::refreshRectSupport 0 |
// General caps::suppressOutputSupport 0 |
// Front::capability 1 / 19 |
// Receiving from client Bitmap caps (28 bytes) |
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
// Front::capability 2 / 19 |
// Receiving from client Order caps (88 bytes) |
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
// Order caps::orderSupport[TS_NEG_MEM3BLT_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex1] 0 |
// Order caps::orderSupport[UnusedIndex2] 0 |
// Order caps::orderSupport[TS_NEG_DRAWNINEGRID_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_LINETO_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex3] 0 |
// Order caps::orderSupport[TS_NEG_SAVEBITMAP_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex4] 0 |
// Order caps::orderSupport[UnusedIndex5] 0 |
// Order caps::orderSupport[UnusedIndex6] 0 |
// Order caps::orderSupport[TS_NEG_MULTIDSTBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MULTIPATBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MULTISCRBLT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_FAST_INDEX_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_POLYGON_SC_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_POLYGON_CB_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_POLYLINE_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex7] 0 |
// Order caps::orderSupport[TS_NEG_FAST_GLYPH_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_ELLIPSE_SC_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_ELLIPSE_CB_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_INDEX_INDEX] 0 |
// Order caps::orderSupport[UnusedIndex8] 0 |
// Order caps::orderSupport[UnusedIndex9] 0 |
// Order caps::orderSupport[UnusedIndex10] 0 |
// Order caps::orderSupport[UnusedIndex11] 0 |
// Order caps::textFlags 1697 |
// Order caps::orderSupportExFlags 6 |
// Order caps::pad4octetsB 0 |
// Order caps::desktopSaveSize 230400 |
// Order caps::pad2octetsC 0 |
// Order caps::pad2octetsD 0 |
// Order caps::textANSICodePage 1252 |
// Order caps::pad2octetsE 0 |
// Front::capability 3 / 19 |
// Receiving from client BitmapCache caps (40 bytes) |
// BitmapCache caps::pad1 0 |
// BitmapCache caps::pad2 0 |
// BitmapCache caps::pad3 0 |
// BitmapCache caps::pad4 0 |
// BitmapCache caps::pad5 0 |
// BitmapCache caps::pad6 0 |
// BitmapCache caps::cache0Entries 120 |
// BitmapCache caps::cache0MaximumCellSize 512 |
// BitmapCache caps::cache1Entries 120 |
// BitmapCache caps::cache1MaximumCellSize 2048 |
// BitmapCache caps::cache2Entries 337 |
// BitmapCache caps::cache2MaximumCellSize 8192 |
// Front::capability 4 / 19 |
// Receiving from client CAPSTYPE_COLORCACHE |
// Front::capability 5 / 19 |
// Receiving from client CAPSTYPE_ACTIVATION |
// Front::capability 6 / 19 |
// Receiving from client CAPSTYPE_CONTROL |
// Front::capability 7 / 19 |
// Receiving from client Pointer caps (10 bytes) |
// Pointer caps::colorPointerFlag 1 |
// Pointer caps::colorPointerCacheSize 20 |
// Pointer caps::pointerCacheSize 21 |
// Front::capability 8 / 19 |
// Receiving from client CAPSTYPE_SHARE |
// Front::capability 9 / 19 |
// Receiving from client CAPSTYPE_INPUT |
// Front::capability 10 / 19 |
// Receiving from client CAPSTYPE_SOUND |
// Front::capability 11 / 19 |
// Front::capability 12 / 19 |
// Receiving from client CAPSTYPE_GLYPHCACHE |
// Front::capability 13 / 19 |
// Receiving from client CAPSTYPE_BRUSH |
// Receiving from client BrushCache caps (8 bytes) |
// BrushCacheCaps caps::brushSupportLevel 1 |
// Front::capability 14 / 19 |
// Receiving from client CAPSTYPE_OFFSCREENCACHE |
// Receiving from client OffScreenCache caps (12 bytes) |
// OffScreenCache caps::offscreenSupportLevel 1 |
// OffScreenCache caps::offscreenCacheSize 5120 |
// OffScreenCache caps::offscreenCacheEntries 100 |
// Front::capability 15 / 19 |
// Receiving from client CAPSTYPE_VIRTUALCHANNEL |
// Front::capability 16 / 19 |
// Receiving from client CAPSTYPE_DRAWNINEGRIDCACHE |
// Front::capability 17 / 19 |
// Receiving from client CAPSTYPE_DRAWGDIPLUS |
// Front::capability 18 / 19 |
// Receiving from client MultifragmentUpdate caps (8 bytes) |
// MultifragmentUpdate caps::MaxRequestSize 0 |
// process_confirm_active done p=0x7fffeb977b57 end=0x7fffeb977b57 |
// Front::reset::use_bitmap_comp=1 |
// Front::reset::use_compact_packets=1 |
// Front::reset::bitmap_cache_version=0 |
// Front: Use RDP 5.0 Bulk compression |
// GraphicsUpdatePDU::init::Initializing orders batch mcs_userid=0 shareid=65538 |
// GraphicsUpdatePDU::init::Initializing bitmaps batch mcs_userid=0 shareid=65538 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[0](2000) used=0 free=16278 |
// Front received CONFIRMACTIVEPDU done |
// Front::incoming |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x24"                                                     //..$ |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 32 bytes |
// Recv done on RDP Client (5) 32 bytes |
/* 0000 */ "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x16\x16\x00\x17\x00\xe9\x03" //...d....p....... |
/* 0010 */ "\x02\x00\x01\x00\x00\x01\x08\x00\x1f\x00\x00\x00\x01\x00\xe9\x03" //................ |
// Dump done on RDP Client (5) 32 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x16, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x08, 0x00, 0x1f, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x01, 0x00, 0xe9, 0x03,                                // ...... |
// Front::incoming::sec_flags=0 |
// Front received DATAPDU |
// Front::process_data(...) |
// sdata_in.pdutype2=31 sdata_in.len=8 sdata_in.compressedLen=0 remains=0 payload_len=4 |
// PDUTYPE2_SYNCHRONIZE |
// PDUTYPE2_SYNCHRONIZE messageType=1 controlId=1001 |
// send_synchronize |
// Sec clear payload to send: |
// /* 0000 */ 0x16, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x16, 0x00, 0x1f, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x01, 0x00, 0xea, 0x03,                                // ...... |
// Sending on RDP Client (5) 36 bytes |
// /* 0000 */ "\x03\x00\x00\x24\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x16\x16\x00" //...$...h....p... |
// /* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x16\x00\x1f\x00\x00\x00" //................ |
// /* 0020 */ "\x01\x00\xea\x03"                                                 //.... |
// Sent dumped on RDP Client (5) 36 bytes |
// send_synchronize done |
// process_data done |
// Front received DATAPDU done |
// Front::incoming |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x28"                                                     //..( |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 36 bytes |
// Recv done on RDP Client (5) 36 bytes |
/* 0000 */ "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00\x17\x00\xe9\x03" //...d....p....... |
/* 0010 */ "\x02\x00\x01\x00\x00\x01\x0c\x00\x14\x00\x00\x00\x04\x00\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00"                                                 //.... |
// Dump done on RDP Client (5) 36 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x0c, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                    // .......... |
// Front::incoming::sec_flags=0 |
// Front received DATAPDU |
// Front::process_data(...) |
// sdata_in.pdutype2=20 sdata_in.len=12 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// PDUTYPE2_CONTROL |
// send_control action=4 |
// Sec clear payload to send: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x1a, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00,                    // .......... |
// Sending on RDP Client (5) 40 bytes |
// /* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x1a\x1a\x00" //...(...h....p... |
// /* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00" //................ |
// /* 0020 */ "\x04\x00\x00\x00\xea\x03\x00\x00"                                 //........ |
// Sent dumped on RDP Client (5) 40 bytes |
// send_control done. action=4 |
// process_data done |
// Front received DATAPDU done |
// Front::incoming |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x28"                                                     //..( |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 36 bytes |
// Recv done on RDP Client (5) 36 bytes |
/* 0000 */ "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00\x17\x00\xe9\x03" //...d....p....... |
/* 0010 */ "\x02\x00\x01\x00\x00\x01\x0c\x00\x14\x00\x00\x00\x01\x00\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00"                                                 //.... |
// Dump done on RDP Client (5) 36 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x0c, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                    // .......... |
// Front::incoming::sec_flags=0 |
// Front received DATAPDU |
// Front::process_data(...) |
// sdata_in.pdutype2=20 sdata_in.len=12 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// PDUTYPE2_CONTROL |
// send_control action=2 |
// Sec clear payload to send: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x1a, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00,                    // .......... |
// Sending on RDP Client (5) 40 bytes |
// /* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x1a\x1a\x00" //...(...h....p... |
// /* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00" //................ |
// /* 0020 */ "\x02\x00\x00\x00\xea\x03\x00\x00"                                 //........ |
// Sent dumped on RDP Client (5) 40 bytes |
// send_control done. action=2 |
// process_data done |
// Front received DATAPDU done |
// Front::incoming |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x28"                                                     //..( |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 36 bytes |
// Recv done on RDP Client (5) 36 bytes |
/* 0000 */ "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00\x17\x00\xe9\x03" //...d....p....... |
/* 0010 */ "\x02\x00\x01\x00\x00\x01\x00\x00\x27\x00\x00\x00\x00\x00\x00\x00" //........'....... |
/* 0020 */ "\x03\x00\x32\x00"                                                 //..2. |
// Dump done on RDP Client (5) 36 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x27, 0x00,  // ..............'. |
// /* 0010 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x32, 0x00,                    // ........2. |
// Front::incoming::sec_flags=0 |
// Front received DATAPDU |
// Front::process_data(...) |
// sdata_in.pdutype2=39 sdata_in.len=0 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// PDUTYPE2_FONTLIST |
// send_fontmap |
// Sec clear payload to send: |
// /* 0000 */ 0x9a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0xbe, 0x00, 0x28, 0x61,  // ..............(a |
// /* 0010 */ 0x9a, 0x00, 0xbf, 0x81, 0x4d, 0x80, 0x0a, 0x00, 0x00, 0x00, 0x09, 0xfe, 0x10, 0x0c, 0x00, 0x13,  // ....M........... |
// /* 0020 */ 0xe5, 0x09, 0x80, 0x00, 0x40, 0x07, 0x80, 0x00, 0x80, 0x07, 0xfe, 0x80, 0x77, 0xe9, 0x09, 0xc0,  // ....@.......w... |
// /* 0030 */ 0x01, 0x40, 0x02, 0xc0, 0x01, 0xbf, 0x30, 0x20, 0x00, 0x84, 0x00, 0x24, 0x00, 0x80, 0x00, 0x28,  // .@....0 ...$...( |
// /* 0040 */ 0x00, 0x8b, 0xe9, 0x09, 0x40, 0x03, 0x00, 0x09, 0x00, 0x03, 0x40, 0x08, 0xc0, 0x03, 0x80, 0x06,  // ....@.....@..... |
// /* 0050 */ 0x40, 0x03, 0xc0, 0x05, 0x80, 0x04, 0x00, 0x05, 0x40, 0x04, 0x40, 0x07, 0x00, 0x04, 0x80, 0x06,  // @.......@.@..... |
// /* 0060 */ 0xc0, 0x04, 0xc0, 0x06, 0x80, 0x05, 0x00, 0x05, 0xfe, 0x90, 0x63, 0xed, 0x03, 0xbe, 0x30, 0x33,  // ..........c...03 |
// /* 0070 */ 0xf3, 0x03, 0x7e, 0xb3, 0xef, 0x40, 0xa7, 0x87, 0x82, 0x7e, 0x0a, 0x00, 0x78, 0x49, 0xe0, 0xe8,  // ..~..@...~..xI.. |
// /* 0080 */ 0x06, 0xf0, 0x30, 0x1f, 0xc1, 0xc0, 0x8f, 0x0b, 0x02, 0x7c, 0x84, 0xf1, 0xb4, 0x01, 0xf8, 0xc8,  // ..0......|...... |
// /* 0090 */ 0x04, 0x00, 0x53, 0xe4, 0xe1, 0x57, 0x90, 0x85, 0x7f, 0x18,                    // ..S..W.... |
// Sending on RDP Client (5) 169 bytes |
// /* 0000 */ "\x03\x00\x00\xa9\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x80\x9a\x9a" //.......h....p... |
// /* 0010 */ "\x00\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\xbe\x00\x28\x61\x9a" //.............(a. |
// /* 0020 */ "\x00\xbf\x81\x4d\x80\x0a\x00\x00\x00\x09\xfe\x10\x0c\x00\x13\xe5" //...M............ |
// /* 0030 */ "\x09\x80\x00\x40\x07\x80\x00\x80\x07\xfe\x80\x77\xe9\x09\xc0\x01" //...@.......w.... |
// /* 0040 */ "\x40\x02\xc0\x01\xbf\x30\x20\x00\x84\x00\x24\x00\x80\x00\x28\x00" //@....0 ...$...(. |
// /* 0050 */ "\x8b\xe9\x09\x40\x03\x00\x09\x00\x03\x40\x08\xc0\x03\x80\x06\x40" //...@.....@.....@ |
// /* 0060 */ "\x03\xc0\x05\x80\x04\x00\x05\x40\x04\x40\x07\x00\x04\x80\x06\xc0" //.......@.@...... |
// /* 0070 */ "\x04\xc0\x06\x80\x05\x00\x05\xfe\x90\x63\xed\x03\xbe\x30\x33\xf3" //.........c...03. |
// /* 0080 */ "\x03\x7e\xb3\xef\x40\xa7\x87\x82\x7e\x0a\x00\x78\x49\xe0\xe8\x06" //.~..@...~..xI... |
// /* 0090 */ "\xf0\x30\x1f\xc1\xc0\x8f\x0b\x02\x7c\x84\xf1\xb4\x01\xf8\xc8\x04" //.0......|....... |
// /* 00a0 */ "\x00\x53\xe4\xe1\x57\x90\x85\x7f\x18"                             //.S..W.... |
// Sent dumped on RDP Client (5) 169 bytes |
// send_fontmap done |
// Front::send_data_update_sync |
// send_server_update: fastpath_support=yes compression_support=yes shareId=65538 encryptionLevel=0 initiator=0 type=3 data_extra=0 |
// Sending on RDP Client (5) 5 bytes |
// /* 0000 */ "\x00\x05\x03\x00\x00"                                             //..... |
// Sent dumped on RDP Client (5) 5 bytes |
// send_server_update done |
// --------------> UP AND RUNNING <---------------- |
// asking for selector |
// process_data done |
// Front received DATAPDU done |
// Front::begin_update |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[1](23) used=1033 free=15245 |
// order(10 clip(0,0,1,1)):opaquerect(rect(0,0,1024,768) color=0x00ffff) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[2](23) used=1042 free=15236 |
// order(10 clip(0,0,1,1)):opaquerect(rect(5,5,1014,758) color=0x00f800) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[3](23) used=1050 free=15228 |
// order(10 clip(0,0,1,1)):opaquerect(rect(10,10,1004,748) color=0x0007e0) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[4](23) used=1058 free=15220 |
// order(10 clip(0,0,1,1)):opaquerect(rect(15,15,994,738) color=0x00001f) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[5](23) used=1066 free=15212 |
// order(10 clip(0,0,1,1)):opaquerect(rect(20,20,984,728) color=0x000000) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[6](23) used=1073 free=15205 |
// order(10 clip(0,0,1,1)):opaquerect(rect(30,30,964,708) color=0x000273) |
// Widget_load: image file [./tests/fixtures/Philips_PM5544_640.png] is PNG file |
// front::draw:draw_tile((192, 144, 64, 64) (0, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[7](8208) used=1081 free=15197 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[8](30) used=1209 free=15069 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 144, 64, 64) (64, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[9](8208) used=1223 free=15055 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[10](30) used=1374 free=14904 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=1) |
// front::draw:draw_tile((320, 144, 64, 64) (128, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[11](8208) used=1380 free=14898 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[12](30) used=1487 free=14791 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=2) |
// front::draw:draw_tile((384, 144, 64, 64) (192, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[13](8208) used=1493 free=14785 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[14](30) used=1935 free=14343 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=3) |
// front::draw:draw_tile((448, 144, 64, 64) (256, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[15](8208) used=1941 free=14337 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[16](30) used=2193 free=14085 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=4) |
// front::draw:draw_tile((512, 144, 64, 64) (320, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[17](8208) used=2199 free=14079 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[18](30) used=2462 free=13816 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=5) |
// front::draw:draw_tile((576, 144, 64, 64) (384, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[19](8208) used=2468 free=13810 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[20](30) used=2922 free=13356 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=6) |
// front::draw:draw_tile((640, 144, 64, 64) (448, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[21](8208) used=2928 free=13350 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[22](30) used=3021 free=13257 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=7) |
// front::draw:draw_tile((704, 144, 64, 64) (512, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[23](8208) used=3027 free=13251 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[24](30) used=3181 free=13097 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=8) |
// front::draw:draw_tile((768, 144, 64, 64) (576, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[25](8208) used=3187 free=13091 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[26](30) used=3332 free=12946 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,144,64,64) rop=cc srcx=0 srcy=0 cache_idx=9) |
// front::draw:draw_tile((192, 208, 64, 64) (0, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[27](8208) used=3338 free=12940 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[28](30) used=3441 free=12837 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=10) |
// front::draw:draw_tile((256, 208, 64, 64) (64, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[29](8208) used=3450 free=12828 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[30](30) used=3583 free=12695 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=11) |
// front::draw:draw_tile((320, 208, 64, 64) (128, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[31](8208) used=3589 free=12689 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[32](30) used=4141 free=12137 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=12) |
// front::draw:draw_tile((384, 208, 64, 64) (192, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[33](8208) used=4147 free=12131 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[34](30) used=4260 free=12018 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=13) |
// front::draw:draw_tile((448, 208, 64, 64) (256, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[35](8208) used=4266 free=12012 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[36](30) used=4306 free=11972 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=14) |
// front::draw:draw_tile((512, 208, 64, 64) (320, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[37](8208) used=4312 free=11966 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[38](30) used=4353 free=11925 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=15) |
// front::draw:draw_tile((576, 208, 64, 64) (384, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[39](8208) used=4359 free=11919 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[40](30) used=4450 free=11828 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=16) |
// front::draw:draw_tile((640, 208, 64, 64) (448, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[41](8208) used=4456 free=11822 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[42](30) used=5036 free=11242 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=17) |
// front::draw:draw_tile((704, 208, 64, 64) (512, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[43](8208) used=5042 free=11236 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[44](30) used=5175 free=11103 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=18) |
// front::draw:draw_tile((768, 208, 64, 64) (576, 64, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[45](8208) used=5181 free=11097 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[46](30) used=5287 free=10991 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,208,64,64) rop=cc srcx=0 srcy=0 cache_idx=19) |
// front::draw:draw_tile((192, 272, 64, 64) (0, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[47](8208) used=5293 free=10985 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[48](30) used=5398 free=10880 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=20) |
// front::draw:draw_tile((256, 272, 64, 64) (64, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[49](8208) used=5407 free=10871 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[50](30) used=5805 free=10473 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=21) |
// front::draw:draw_tile((320, 272, 64, 64) (128, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[51](8208) used=5811 free=10467 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[52](30) used=5957 free=10321 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=22) |
// front::draw:draw_tile((384, 272, 64, 64) (192, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[53](8208) used=5963 free=10315 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[54](30) used=6036 free=10242 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=23) |
// front::draw:draw_tile((448, 272, 64, 64) (256, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[55](8208) used=6042 free=10236 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[56](30) used=6116 free=10162 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=24) |
// front::draw:draw_tile((512, 272, 64, 64) (320, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[57](8208) used=6122 free=10156 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[58](30) used=6191 free=10087 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=25) |
// front::draw:draw_tile((576, 272, 64, 64) (384, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[59](8208) used=6197 free=10081 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[60](30) used=6268 free=10010 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=26) |
// front::draw:draw_tile((640, 272, 64, 64) (448, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[61](8208) used=6274 free=10004 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[62](30) used=6417 free=9861 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=27) |
// front::draw:draw_tile((704, 272, 64, 64) (512, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[63](8208) used=6423 free=9855 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[64](30) used=6842 free=9436 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=28) |
// front::draw:draw_tile((768, 272, 64, 64) (576, 128, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[65](8208) used=6848 free=9430 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[66](30) used=6956 free=9322 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,272,64,64) rop=cc srcx=0 srcy=0 cache_idx=29) |
// front::draw:draw_tile((192, 336, 64, 64) (0, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[67](8208) used=6962 free=9316 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[68](30) used=7047 free=9231 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=30) |
// front::draw:draw_tile((256, 336, 64, 64) (64, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[69](8208) used=7056 free=9222 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[70](30) used=7496 free=8782 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=31) |
// front::draw:draw_tile((320, 336, 64, 64) (128, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[71](8208) used=7502 free=8776 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[72](30) used=7616 free=8662 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=32) |
// front::draw:draw_tile((384, 336, 64, 64) (192, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[73](8208) used=7622 free=8656 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[74](30) used=7773 free=8505 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=33) |
// front::draw:draw_tile((448, 336, 64, 64) (256, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[75](8208) used=7779 free=8499 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[76](30) used=7953 free=8325 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=34) |
// front::draw:draw_tile((512, 336, 64, 64) (320, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[77](8208) used=7959 free=8319 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[78](30) used=8128 free=8150 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=35) |
// front::draw:draw_tile((576, 336, 64, 64) (384, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[79](8208) used=8134 free=8144 |
// GraphicsUpdatePDU::flush_orders: order_count=79 |
// send_server_update: fastpath_support=yes compression_support=yes shareId=65538 encryptionLevel=0 initiator=0 type=0 data_extra=79 |
// Sending on RDP Client (5) 5514 bytes |
// /* 0000 */ "\x00\x95\x8a\x80\x21\x83\x15\x4f\x00\x03\xbe\x01\x80\x7d\x17\xc3" //....!..O.....}.. |
// /* 0010 */ "\x80\x00\x05\x5f\x88\x95\x7c\x45\xff\xe2\x3f\xe0\x01\x57\xe2\x25" //..._..|E..?..W.% |
// /* 0020 */ "\x5f\x11\x7f\xf8\x80\x04\x9f\xa0\xf8\x89\x57\xc4\x5f\xfe\x20\x01" //_.........W._. . |
// /* 0030 */ "\xb7\xe8\x3e\x22\x55\xf1\x17\xff\x88\x00\x89\x7d\x07\xc4\x4a\xbe" //..>"U......}..J. |
// /* 0040 */ "\x22\xff\xf1\x00\x13\x6f\xa0\xf8\x89\x57\xc4\x5f\xfe\x20\x02\xb7" //"....o...W._. .. |
// /* 0050 */ "\xf4\x1f\x11\x2a\xf8\x8b\xff\xc4\x78\x96\x15\x7e\x22\x55\xf1\x17" //...*....x..~"U.. |
// /* 0060 */ "\xff\x88\xf1\x8c\x2a\xfc\x44\xab\xe2\x2f\xff\x10\x00\x49\xf4\x1f" //....*.D../...I.. |
// /* 0070 */ "\x11\x2a\xf8\x8b\xff\xc4\x00\x24\xfd\x07\xc4\x4a\xbe\x22\xff\xf1" //.*.....$...J.".. |
// /* 0080 */ "\x00\x0d\xbf\x41\xf1\x12\xaf\x88\xbf\xfc\x40\x04\x4b\xe8\x3e\x22" //...A......@.K.>" |
// /* 0090 */ "\x55\xf1\x17\xff\x88\x00\x9b\x7d\x07\xc4\x4a\xbe\x22\xff\xf1\x00" //U......}..J."... |
// /* 00a0 */ "\x15\xbf\xa0\xf8\x89\x57\xc4\x5f\xfe\x23\xc2\xaf\x20\x3e\x83\xe2" //.....W._.#.. >.. |
// /* 00b0 */ "\x2f\xff\x11\xe2\x17\x90\x09\x3e\x40\x24\xf9\x01\xea\x11\x27\xd0" ///......>@$....'. |
// /* 00c0 */ "\x7c\x44\xab\xe2\x2f\xff\x10\x00\x93\xf4\x1f\x11\x2a\xf8\x8b\xff" //|D../.......*... |
// /* 00d0 */ "\xc4\x00\x36\xfd\x07\xc4\x4a\xbe\x22\xff\xf1\x00\x11\x2f\xa0\xf8" //..6...J."..../.. |
// /* 00e0 */ "\x89\x57\xc4\x5f\xfe\x20\x02\x6d\xf4\x1f\x11\x2a\xf8\x8b\xff\xc4" //.W._. .m...*.... |
// /* 00f0 */ "\x00\x56\xfe\x83\xe2\x25\x5f\x11\x7f\xf8\x8f\x02\xbc\x80\xfa\x0f" //.V...%_......... |
// /* 0100 */ "\x88\xbf\xfc\x47\x84\x5e\x40\x36\xf9\x00\xdb\xe4\x07\x01\x11\x27" //...G.^@6.......' |
// /* 0110 */ "\xd0\x7c\x44\xab\xe2\x2f\xff\x10\x00\x93\xf4\x1f\x11\x2a\xf8\x8b" //.|D../.......*.. |
// /* 0120 */ "\xff\xc4\x00\x36\xfd\x07\xc4\x4a\xbe\x22\xff\xf1\x00\x11\x2f\xa0" //...6...J."..../. |
// /* 0130 */ "\xf8\x89\x57\xc4\x5f\xfe\x20\x02\x6d\xf4\x1f\x11\x2a\xf8\x8b\xff" //..W._. .m...*... |
// /* 0140 */ "\xc4\x00\x56\xfe\x83\xe2\x25\x5f\x11\x7f\xf8\x8f\xea\xf2\x03\xe8" //..V...%_........ |
// /* 0150 */ "\x3e\x22\xff\xf1\x1e\x01\x79\x01\x12\xf2\x02\x25\xe4\x07\x08\x11" //>"....y....%.... |
// /* 0160 */ "\x27\xd0\x7c\x44\xab\xe2\x2f\xff\x10\x00\x93\xf4\x1f\x11\x2a\xf8" //'.|D../.......*. |
// /* 0170 */ "\x8b\xff\xc4\x00\x36\xfd\x07\xc4\x4a\xbe\x22\xff\xf1\x00\x11\x2f" //....6...J."..../ |
// /* 0180 */ "\xa0\xf8\x89\x57\xc4\x5f\xfe\x20\x02\x6d\xf4\x1f\x11\x2a\xf8\x8b" //...W._. .m...*.. |
// /* 0190 */ "\xff\xc4\x00\x56\xfe\x83\xe2\x25\x5f\x11\x7f\xf8\x8f\xca\xf2\x03" //...V...%_....... |
// /* 01a0 */ "\xe8\x3e\x22\xff\xf1\x1f\xc5\xe4\x04\xdb\xc8\x09\xb7\x90\x1c\x3c" //.>"............< |
// /* 01b0 */ "\x44\x9f\x41\xf1\x12\xaf\x88\xbf\xfc\x40\x02\x4f\xd0\x7c\x44\xab" //D.A......@.O.|D. |
// /* 01c0 */ "\xe2\x2f\xff\x10\x00\xdb\xf4\x1f\x11\x2a\xf8\x8b\xff\xc4\x00\x44" //./.......*.....D |
// /* 01d0 */ "\xbe\x83\xe2\x25\x5f\x11\x7f\xf8\x80\x09\xb7\xd0\x7c\x44\xab\xe2" //...%_.......|D.. |
// /* 01e0 */ "\x2f\xff\x10\x01\x5b\xfa\x0f\x88\x95\x7c\x45\xff\xe2\x3e\xab\xc8" ///...[....|E..>.. |
// /* 01f0 */ "\x0f\xa0\xf8\x8b\xff\xc4\x7e\x17\x90\x15\xbf\x20\x2b\x7e\x40\x71" //......~.... +~@q |
// /* 0200 */ "\x61\x12\x7d\x07\xc4\x4a\xbe\x22\xff\xf1\x00\x09\x3f\x41\xf1\x12" //a.}..J."....?A.. |
// /* 0210 */ "\xaf\x88\xbf\xfc\x40\x03\x6f\xd0\x7c\x44\xab\xe2\x2f\xff\x10\x01" //....@.o.|D../... |
// /* 0220 */ "\x12\xfa\x0f\x88\x95\x7c\x45\xff\xe2\x00\x26\xdf\x41\xf1\x12\xaf" //.....|E...&.A... |
// /* 0230 */ "\x88\xbf\xfc\x40\x05\x6f\xe8\x3e\x22\x55\xf1\x17\xff\x88\xf8\xaf" //...@.o.>"U...... |
// /* 0240 */ "\x20\x3e\x83\xe2\x2f\xff\x11\xf4\x5e\x40\x5f\xfc\x80\xbf\xfc\x29" // >../...^@_....) |
// /* 0250 */ "\x00\x24\xfa\x0f\x88\x95\x7c\x45\xff\xe2\x00\x12\x7e\x83\xe2\x25" //.$....|E....~..% |
// /* 0260 */ "\x5f\x11\x7f\xf8\x80\x06\xdf\xa0\xf8\x89\x57\xc4\x5f\xfe\x20\x02" //_.........W._. . |
// /* 0270 */ "\x25\xf4\x1f\x11\x2a\xf8\x8b\xff\xc4\x00\x4d\xbe\x83\xe2\x25\x5f" //%...*.....M...%_ |
// /* 0280 */ "\x11\x7f\xf8\x80\x0a\xdf\xd0\x7c\x44\xab\xe2\x2f\xff\x11\xe3\x57" //.......|D../...W |
// /* 0290 */ "\x90\x1f\x41\xf1\x17\xff\x88\x09\x0a\x3c\xe5\xca\x03\xbf\xdf\xc4" //..A......<...... |
// /* 02a0 */ "\x4f\xc1\x41\x6e\xd7\x60\x0b\xc7\xc8\xab\x00\x3f\xc8\xa1\xf0\x01" //O.An.`.....?.... |
// /* 02b0 */ "\x11\xff\x91\x00\x02\x27\xe1\x41\x56\xcb\x63\x98\x10\x1b\x98\x00" //.....'.AV.c..... |
// /* 02c0 */ "\x00\x20\x10\x10\x02\x02\x00\x83\x8f\x9f\x46\x10\x18\xd5\x5a\x80" //. ........F...Z. |
// /* 02d0 */ "\x22\x21\x04\xa1\x5f\xef\xf0\x17\x78\xf5\x2e\xde\x69\x70\x88\x02" //"!.._...x...ip.. |
// /* 02e0 */ "\x40\x6b\x75\xea\x00\x8c\x08\x40\x0a\x03\x38\x97\x52\xe3\xcc\x2c" //@ku....@..8.R.., |
// /* 02f0 */ "\x0c\x17\xfb\xfd\x39\x56\x5b\x50\x01\xa1\x8c\x00\xc0\x47\x2c\xf4" //....9V[P.....G,. |
// /* 0300 */ "\x01\x19\x52\x90\x14\x06\x75\x56\x39\xa8\x01\x41\x8c\x41\x68\x02" //..R...uV9..A.Ah. |
// /* 0310 */ "\x28\xa5\x20\x50\x1b\xbc\xf4\xae\xe1\x40\x07\xa7\x4a\x94\xbe\xa6" //(. P.....@..J... |
// /* 0320 */ "\x73\xaa\xb1\x5b\x03\x87\x85\x53\x69\xad\x00\x46\xb7\x5e\x1e\x80" //s..[...Si..F.^.. |
// /* 0330 */ "\x9a\x49\x68\x02\x24\x84\x20\x16\xe4\x90\xd3\xf0\x20\x0a\x00\x04" //.Ih.$. ..... ... |
// /* 0340 */ "\x40\x01\x03\xe1\x29\x80\x70\xaf\x27\x63\x08\xe7\x18\xa0\x02\xdb" //@...).p.'c...... |
// /* 0350 */ "\xcd\x20\x26\x6a\xb4\x01\x3a\xea\x9c\x04\xfa\x1b\xe5\x48\x89\x45" //. &j..:......H.E |
// /* 0360 */ "\x28\x62\x10\x4b\x84\x00\x12\x80\x0b\x1e\x61\x80\xc5\x5b\x68\x02" //(b.K......a..[h. |
// /* 0370 */ "\x72\x03\xfa\x80\x14\x00\x40\x07\xc2\x0a\x1c\x08\x43\xcb\x91\x40" //r.....@.....C..@ |
// /* 0380 */ "\x05\xf1\xc4\x14\x21\x00\xc0\x56\x69\xd4\x01\x0f\x99\x12\x63\x10" //....!..Vi.....c. |
// /* 0390 */ "\x18\x0d\x5e\x7b\x43\x95\x29\x50\x01\xbc\x95\x0a\xb2\xd2\x00\x0a" //..^{C.)P........ |
// /* 03a0 */ "\x80\x10\x63\x18\x0d\x11\x14\xa7\xcc\xe3\x60\x2f\xd2\xfa\x59\x30" //..c.......`/..Y0 |
// /* 03b0 */ "\x3c\x78\x99\x0e\x73\xa0\x08\x56\xeb\xdf\x1c\x3d\x40\x12\xf3\x2a" //<x..s..V...=@..* |
// /* 03c0 */ "\xcc\x22\x04\x02\x80\x02\x00\x06\xbd\xe5\xdc\x57\x3a\x20\xa8\x01" //.".........W: .. |
// /* 03d0 */ "\xa2\x10\x4f\x55\xbc\xac\xa0\x08\x79\x52\x83\x60\x42\x15\x00\x29" //..OU....yR.`B..) |
// /* 03e0 */ "\x73\x98\x0d\x00\x43\xc9\x76\x1b\xe3\xf8\x45\x29\xfa\x2f\x1e\x83" //s...C.v...E)./.. |
// /* 03f0 */ "\x51\x4a\x6a\x00\x49\xad\x60\x2c\x03\xbf\xb9\x23\x07\xfa\x18\xd0" //QJj.I.`,...#.... |
// /* 0400 */ "\x04\x3c\x6c\x80\x7e\xe7\x8c\xc0\x00\x14\x01\x0f\x18\xac\x1e\x31" //.<l.~..........1 |
// /* 0410 */ "\x80\x20\x00\x39\x68\x0f\x8c\x70\x95\x80\x81\x80\x40\x84\x20\x91" //. .9h..p....@. . |
// /* 0420 */ "\xaa\xb1\x00\xb5\x45\xb5\xac\x04\x0e\x04\xfb\x6f\x5d\xb7\x80\x0f" //....E......o]... |
// /* 0430 */ "\x40\x57\x6d\xe4\x3c\x1c\x80\x03\xcc\x42\x10\x48\x14\x5a\x6b\x5f" //@Wm.<....B.H.Zk_ |
// /* 0440 */ "\xbf\x80\x0f\x40\xc4\x20\x8e\x30\xcf\x05\x3e\x9c\xba\xcb\x67\xd7" //...@. .0..>...g. |
// /* 0450 */ "\x7f\xa7\x2a\x30\xcf\xcd\x80\xa4\x96\x3d\xf4\xe8\xc2\x08\xb3\x4e" //..*0.....=.....N |
// /* 0460 */ "\xfa\x75\x53\x8e\x1e\x59\xfe\x55\x3a\xcb\x7e\xee\x23\x14\x6f\xa7" //.uS..Y.U:.~.#.o. |
// /* 0470 */ "\x24\x3c\x1e\x9f\x26\x7f\xcb\xc6\x51\xf2\xa2\x2d\x35\xfc\xd1\x04" //$<..&...Q..-5... |
// /* 0480 */ "\xfc\xd1\x57\x9e\xf8\x84\x3b\xf5\x50\x3f\x99\xb5\xdf\xe6\x62\xa3" //..W...;.P?....b. |
// /* 0490 */ "\x0c\x18\xa3\x57\xef\xe3\xd0\x48\x94\x52\xb7\x5e\xbf\xdf\xdf\x6d" //...W...H.R.^...m |
// /* 04a0 */ "\xef\xb1\x28\xa4\x0e\xa7\x1c\x59\xa7\x4f\xae\xe3\x8c\x03\x1c\x5c" //..(....Y.O...... |
// /* 04b0 */ "\x44\x1c\x34\xd4\x7b\x7b\xdc\x11\x56\x5a\x39\x67\xbf\xdf\xe3\xd7" //D.4.{{..VZ9g.... |
// /* 04c0 */ "\x7e\x22\x31\x50\x01\x61\x08\x01\x81\x8e\x59\xee\xf3\xd1\xcb\x3b" //~"1P.a....Y....; |
// /* 04d0 */ "\xb5\xfe\xff\xe2\xa4\x6a\xad\x7f\xbf\xae\xdb\xf8\x85\xc7\x13\x9a" //.....j.......... |
// /* 04e0 */ "\x8d\x55\xbc\x39\x0e\x30\xc3\x8a\x37\x8a\x30\x30\x82\x3a\xf8\x08" //.U.9.0..7.00.:.. |
// /* 04f0 */ "\x00\x3a\xc7\xf3\x11\x28\xa7\xd8\xaf\x98\x81\x2a\xcb\x6f\x7e\xcd" //.:...(.....*.o~. |
// /* 0500 */ "\xaf\xac\x18\xa3\x3e\xdb\xfe\xb4\xf9\x08\x71\x46\xf4\x2b\xe6\x4d" //....>.....qF.+.M |
// /* 0510 */ "\xcf\xe6\x6a\xb3\x4e\xf9\x90\xc8\x21\x04\xb8\x4f\xc1\x60\x7e\x55" //..j.N...!..O.`~U |
// /* 0520 */ "\xa2\x1f\x1c\x65\xc6\xc4\x11\xc1\xbd\xc2\x75\x02\x24\x21\x9b\x4d" //...e......u.$!.M |
// /* 0530 */ "\x40\x01\x7c\xfc\xa0\x04\x56\xeb\xd0\x20\x00\x03\x49\x2f\xce\x1a" //@.|...V.. ..I/.. |
// /* 0540 */ "\x49\x5e\x79\x03\x2b\xfc\x20\x20\x0c\x00\x0e\xdf\xc3\xc7\x1b\x6f" //I^y.+.  .......o |
// /* 0550 */ "\x25\x50\x01\x05\xc0\x00\x16\x80\x61\x6f\x7b\xb9\xa0\x01\x5f\xef" //%P......ao{..._. |
// /* 0560 */ "\xfe\x43\x7a\x7e\x3c\x6f\x3c\x17\xca\xb2\xd7\x9f\x1c\xc0\xe7\x8e" //.Cz~<o<......... |
// /* 0570 */ "\x50\x29\x94\x52\xaf\x3d\xf3\x9c\x81\x41\x1e\x69\xd5\x79\xec\xea" //P).R.=...A.i.y.. |
// /* 0580 */ "\xad\xe8\xc0\xe7\x12\xb7\xea\x88\x94\x51\xd7\xf5\x38\x25\x3f\x54" //.........Q..8%?T |
// /* 0590 */ "\x41\x54\x61\x82\x92\x5a\xbc\xf6\x35\x56\xfa\x01\xb6\x6f\xa0\x99" //ATa..Z..5V...o.. |
// /* 05a0 */ "\x7d\xe0\xc5\x1a\xb7\x5e\xf0\x50\x6d\x94\x42\x09\x07\x51\x86\x5c" //}....^.Pm.B..Q.. |
// /* 05b0 */ "\xe3\x8e\xb2\xdb\xbc\xf5\xe6\x9d\x7b\xaf\x7a\xec\x32\xd1\xf5\x11" //........{.z.2... |
// /* 05c0 */ "\x28\xa0\xa4\x96\x2d\x35\x18\xa3\x3c\xd3\xa7\x55\x6e\x17\x01\x9f" //(...-5..<..Un... |
// /* 05d0 */ "\xf0\xe2\xed\x30\x85\x59\x69\x87\x87\xd5\x38\xe0\xa4\x95\xd6\x5b" //...0.Yi...8....[ |
// /* 05e0 */ "\x16\x9a\xcd\xa6\xb4\x22\x84\x28\x1d\xde\x7a\x31\x46\x38\xa3\x50" //.....".(..z1F8.P |
// /* 05f0 */ "\xa4\xc6\x30\x27\x9a\x77\x10\x20\x83\xcf\xb6\x06\xe0\x74\x9e\x41" //..0'.w. .....t.A |
// /* 0600 */ "\x11\x3a\xab\x10\x00\xf1\xfc\x83\x0f\x67\x45\x27\x89\x90\x1b\x8f" //.:.......gE'.... |
// /* 0610 */ "\x88\xc2\x93\xc5\xfc\x0c\x63\xc6\x0a\xb9\xa4\x2f\x5e\xc2\xc6\xf1" //......c..../^... |
// /* 0620 */ "\xdd\xb1\xec\x2c\x9f\x7d\x85\x1c\x28\x62\xa7\x1c\x00\x19\x82\x47" //...,.}..(b.....G |
// /* 0630 */ "\xae\xef\x3c\xc4\xca\x30\xcf\x50\x80\x05\xe0\x55\x79\xec\xca\x28" //..<..0.P...Uy..( |
// /* 0640 */ "\xcf\xf2\x82\x30\x39\xf5\xde\x75\x56\xab\xcf\x79\x83\x40\x0b\x4f" //...09..uV..y.@.O |
// /* 0650 */ "\xd1\x06\xf1\xaf\x8e\x2c\x88\x94\x53\xf4\xbd\xa0\x00\x13\x82\xae" //.....,..S....... |
// /* 0660 */ "\xdb\xe3\x5f\x88\x29\x25\x51\x86\x7f\x28\x0e\x82\xc7\xae\xf5\xba" //.._.)%Q..(...... |
// /* 0670 */ "\xf0\xc5\x19\xd6\x5b\x32\x8a\x36\xfd\x00\x3f\x1e\x75\xee\xbc\x43" //....[2.6..?.u..C |
// /* 0680 */ "\xc5\x38\xeb\x2d\xb9\xc7\x14\x61\x8e\x42\x10\x41\xf8\x11\xe5\x9e" //.8.-...a.B.A.... |
// /* 0690 */ "\x75\x56\x21\xe3\x3e\x8b\x4d\x45\x24\xb1\x28\xa3\xd0\x84\x10\x33" //uV!.>.ME$.(....3 |
// /* 06a0 */ "\x88\x70\x87\xd4\x68\x19\xc5\x18\x62\x8d\xca\x44\x50\x85\x07\x9b" //.p..h...b..DP... |
// /* 06b0 */ "\x4d\x62\xd3\x57\x59\x68\xa4\x96\xa7\x1c\x89\x45\x0c\x3d\xbe\x95" //Mb.WYh.....E.=.. |
// /* 06c0 */ "\x29\x00\xf3\x3b\x30\x00\x7e\xd3\x87\xb0\x8e\x00\x74\x00\x0e\x1a" //)..;0.~.....t... |
// /* 06d0 */ "\x4e\x5e\x71\xc3\x6e\x79\x43\xc7\xb5\x1e\xc9\x8a\x00\x07\x2f\x99" //N^q.nyC......./. |
// /* 06e0 */ "\x00\x06\x29\xa0\x00\xfb\x2e\x38\x67\xc5\x43\xd9\xb0\x0a\x00\x07" //..)....8g.C..... |
// /* 06f0 */ "\x39\xe5\x1f\x8c\xdc\x04\x18\x00\x1e\x02\xda\xd6\x40\x2d\x02\x8d" //9...........@-.. |
// /* 0700 */ "\x55\xbc\xd2\x1b\x81\x2e\xdb\xf8\x30\x07\x60\x71\xaa\xb1\x84\x13" //U.......0.`q.... |
// /* 0710 */ "\x84\x6c\x40\xab\xf7\xf8\xb4\xd7\x86\x13\xe5\x2b\x75\xe7\x1f\x6c" //.l@........+u..l |
// /* 0720 */ "\xc8\xcc\x0a\x7d\x77\x75\x96\xf0\x96\x00\x0c\x7f\x52\x8c\x33\x83" //...}wu......R.3. |
// /* 0730 */ "\xc0\x00\x5a\x05\x1e\xbb\x8a\x49\x78\x4b\x00\x05\x60\x5c\x12\x90" //..Z....IxK..`... |
// /* 0740 */ "\x4f\x73\x40\x9f\xa8\x95\x38\xe3\x5f\xca\x08\xfd\xe3\xac\xb7\xd9" //Os@...8._....... |
// /* 0750 */ "\x30\x21\x02\x18\xa3\x18\x41\x1b\xbe\x50\x3a\x05\x3a\xab\x44\xa2" //0!....A..P:.:.D. |
// /* 0760 */ "\x9e\x85\x00\x06\x78\xd0\x29\x25\xf6\x98\x00\x2b\xf4\x22\xd3\x57" //....x.)%...+.".W |
// /* 0770 */ "\x4f\x94\x08\xe0\x66\xab\xcf\x79\x05\xbe\x60\x1b\xe6\x5b\xbf\xcc" //O...f..y..`..[.. |
// /* 0780 */ "\x02\x7c\xc5\xdf\xe6\x38\x5f\x80\x00\xa0\x75\xfb\xf8\xc5\x19\x46" //.|...8_...u....F |
// /* 0790 */ "\x18\xf7\xf3\x00\x1f\x60\xb3\x4e\xa9\xc7\x39\x4a\xaa\x00\x24\x00" //.....`.N..9J..$. |
// /* 07a0 */ "\x00\x3e\x6d\x44\xa2\x83\x8c\x00\x97\xfb\xff\x29\x37\x1d\xb3\x01" //.>mD.......)7... |
// /* 07b0 */ "\x75\xfe\xff\x05\xe1\x4f\x1c\x81\xe3\x94\xe6\x70\x21\xcb\x3a\xac" //u....O.....p!.:. |
// /* 07c0 */ "\xb4\x4f\xfe\x8e\x28\xce\x30\xcf\x53\xaa\x00\x3c\x28\xdc\x27\x94" //.O..(.0.S..<(.'. |
// /* 07d0 */ "\x0e\x01\xd1\xc8\xa6\x3d\x44\x10\x82\x08\xf0\xe2\x3a\xf1\x55\x71" //.....=D.....:.Uq |
// /* 07e0 */ "\xaa\x00\x0b\x7e\xd9\x89\x70\xd9\x91\x48\x15\xee\xbc\xab\x2d\x6a" //...~..p..H....-j |
// /* 07f0 */ "\xf9\x41\x18\x1b\xed\xbc\x63\xeb\x1b\x3e\x60\x3a\x07\x1e\xfc\x4a" //.A....c..>`:...J |
// /* 0800 */ "\x9c\x71\xbf\xe6\x02\xfe\x64\xdc\xe3\x9e\x9e\x80\x8f\x98\x59\xa7" //.q....d.......Y. |
// /* 0810 */ "\x71\xc8\x7c\xc8\x78\xe0\xfc\xb4\x02\xd0\x02\x3c\x4f\x1e\x33\x8f" //q.|.x......<O.3. |
// /* 0820 */ "\x2a\x47\xcc\x8f\x19\xc7\x95\x23\x84\x1c\xf2\x90\x79\xf1\xca\x39" //*G.....#....y..9 |
// /* 0830 */ "\x36\x9a\x90\xd0\x02\x09\x08\x79\x4b\x3e\x6b\x94\xdd\x52\x38\x46" //6......yK>k..R8F |
// /* 0840 */ "\x40\x30\x00\x1a\x80\x07\x08\xcc\x13\xb0\x13\x0b\xa6\x14\x74\xc9" //@0............t. |
// /* 0850 */ "\x71\x54\x74\xd6\x8e\x99\x91\xd3\x0d\xc7\x76\x0f\x7c\x23\x3d\x2f" //qTt.......v.|#=/ |
// /* 0860 */ "\xdc\x7c\x20\xf7\xa5\x5c\x75\xdf\x8e\x98\x73\x20\x07\x20\x00\xfa" //.| ...u...s . .. |
// /* 0870 */ "\xd3\xf8\xeb\x90\x0b\xc0\xe2\x42\x1b\x81\x00\x0f\x88\xe0\x1c\x00" //.......B........ |
// /* 0880 */ "\x0e\x1b\xe2\x3c\x61\x78\x0b\x01\xd6\xe1\xc5\x0d\xe2\x58\x3e\x86" //...<ax.......X>. |
// /* 0890 */ "\xf5\xd1\x9b\x44\x12\x80\x0b\x00\x7c\xf1\x63\xad\xcb\xad\x2d\x4f" //...D....|.c...-O |
// /* 08a0 */ "\xa3\xbd\x74\x22\x02\x97\x15\xa0\x02\xc1\x6e\x3e\x16\x8e\xb6\x8e" //..t"......n>.... |
// /* 08b0 */ "\xb9\x7c\x9d\x78\xe4\x15\x14\xa4\x3f\xae\x66\x5d\x76\xe3\xae\x69" //.|.x....?.f]v..i |
// /* 08c0 */ "\x00\x14\x3f\xf6\x3e\xa6\x6c\x0c\xde\x31\x75\xcf\xde\xf1\xe6\x08" //..?.>.l..1u..... |
// /* 08d0 */ "\x08\x00\x03\xe1\x7e\xf3\x05\xc0\xb0\x1a\xe2\x97\x00\x7c\x80\xa8" //....~........|.. |
// /* 08e0 */ "\x4f\xbc\xc0\xed\xff\x1c\x84\x94\x0e\xe9\xef\x2e\xa9\x70\x5b\x8d" //O............p[. |
// /* 08f0 */ "\x40\x0a\x84\xf1\xc7\xa1\x02\x10\xee\x12\xed\x89\x14\xbf\x1f\x3a" //@..............: |
// /* 0900 */ "\xeb\x4f\x0c\xe9\xb1\xf4\x16\x1e\x59\xf9\x5f\xa9\x7c\x12\x1c\x82" //.O......Y._.|... |
// /* 0910 */ "\x63\x82\x8c\xf2\xb3\x02\xf7\x01\x79\x4d\xf5\xf8\x68\x01\x32\xa5" //c.......yM..h.2. |
// /* 0920 */ "\x26\x06\x3f\x14\x47\x95\x13\x69\xa8\xf7\x70\x88\xe0\xba\x00\xb8" //&.?.G..i..p..... |
// /* 0930 */ "\x79\x5e\x02\x40\x00\xd6\xbc\xaf\x8a\xc0\x00\x57\x79\x1e\x3a\x80" //y^.@.......Wy.:. |
// /* 0940 */ "\x14\x06\xbf\xff\x49\xad\x63\xdc\x5a\x20\x32\x9d\x4b\x5a\xa8\x01" //....I.c.Z 2.KZ.. |
// /* 0950 */ "\x30\x84\x77\x67\x32\x94\x87\x53\x8d\x24\xb5\x0e\x4e\xee\xa4\x11" //0.wg2..S.$..N... |
// /* 0960 */ "\x40\x11\x85\x29\x5c\x03\x82\x18\x05\x3a\xb2\x0f\x38\xad\xc8\x42" //@..).....:..8..B |
// /* 0970 */ "\x10\xf7\xd8\x9b\x39\xcd\xc0\x38\x0c\x04\x18\x06\x80\x01\x50\xf1" //....9..8......P. |
// /* 0980 */ "\x08\x0d\xe3\xc6\x18\xbb\x60\x4b\xd7\x77\x39\x71\x55\xf1\x7c\x2a" //......`K.w9qU.|* |
// /* 0990 */ "\xcb\x78\x6b\xc7\xc7\x7c\xd8\xee\xf3\x55\x80\x2e\x26\x50\x20\x9a" //.xk..|...U..&P . |
// /* 09a0 */ "\xd7\x50\x02\xa5\xad\x00\xc0\x5f\x6d\xf4\x34\xd6\xb1\x86\x00\x8b" //.P....._m.4..... |
// /* 09b0 */ "\xfd\xfc\x56\x02\xa5\xde\x9c\xc3\x80\x01\x96\x02\x00\x1f\x74\x28" //..V...........t( |
// /* 09c0 */ "\x6c\xa5\x22\xbd\xee\xa5\x0e\x2b\x50\x02\x21\xe6\x7d\x34\x3f\x28" //l."....+P.!.}4?( |
// /* 09d0 */ "\x0f\x02\x19\x3c\x9e\x5c\xe0\xc8\xa1\x84\x21\x74\xd4\x7d\x67\x93" //...<......!t.}g. |
// /* 09e0 */ "\x80\x00\x62\x3e\xbb\x05\x80\x01\x8d\x81\x79\x2f\x01\x90\x20\xc0" //..b>......y/.. . |
// /* 09f0 */ "\x0a\x37\x8f\x45\x8c\x61\x90\x16\xe6\xba\x00\x35\xe2\x7a\x2d\xc7" //.7.E.a.....5.z-. |
// /* 0a00 */ "\xe9\x01\x93\x18\x80\x0f\xc0\xa9\xef\x70\x21\x00\x01\xee\xe1\xcb" //.........p!..... |
// /* 0a10 */ "\x5a\xd7\xce\x08\x41\x19\x4a\x7e\x3a\x5c\xe6\x82\x11\xf5\x66\x54" //Z...A.J~:.....fT |
// /* 0a20 */ "\xa5\xf3\x82\x10\x4e\x15\x01\xf8\x14\x39\xcf\x02\xa5\x3e\xac\x42" //....N....9...>.B |
// /* 0a30 */ "\xbe\xad\x41\xbf\xfd\x48\x84\x00\x04\x5b\x97\x6d\xfc\xc5\x1f\xd0" //..A..H...[.m.... |
// /* 0a40 */ "\x20\xdf\x7f\xbf\xd4\x00\xe6\x51\x40\x16\x00\xd0\x00\x07\x08\xd5" // ......Q@....... |
// /* 0a50 */ "\x5a\x02\xd2\x94\x00\x1f\x40\x00\x85\x29\x7e\x9a\xee\x1b\x09\xad" //Z.....@..)~..... |
// /* 0a60 */ "\x7e\x07\xbf\xb6\xf0\x3c\x78\xc6\x7c\x8d\x5f\x7b\x8d\x17\xef\x08" //~....<x.|._{.... |
// /* 0a70 */ "\x60\x3f\x76\xcb\x18\xfe\xed\xa4\xfd\xda\xa3\x9c\xfc\x75\xbd\xef" //`?v..........u.. |
// /* 0a80 */ "\xc0\x41\xe8\x0f\x75\xd8\x04\x21\xfc\xf3\x4f\x7b\xa1\x8c\x7c\x99" //.A..u..!..O{..|. |
// /* 0a90 */ "\x97\x39\xd8\xfe\x4d\x60\x0d\x07\xc9\x8b\xbf\x5d\xd3\x0f\xa6\x50" //.9..M`.....]...P |
// /* 0aa0 */ "\x3b\xff\x37\x47\x82\xe2\x03\xda\x3d\xe0\x37\x8b\xb1\xe0\x0e\xf8" //;.7G....=.7..... |
// /* 0ab0 */ "\xf1\xed\x99\xb7\xbd\xf8\xf1\xff\x62\xde\xf7\xe1\x8e\x2f\x93\x74" //........b..../.t |
// /* 0ac0 */ "\xff\xb7\x87\xed\xcf\xfe\xbb\x37\x44\x07\xc1\x97\xe1\xa8\xfb\x09" //.......7D....... |
// /* 0ad0 */ "\x52\x91\x00\x00\x8b\x00\x77\x6e\xa4\x6a\xad\x80\x2f\xc0\x09\x29" //R.....wn.j../..) |
// /* 0ae0 */ "\x4c\x04\x1c\x5d\x81\xa1\x08\x55\x00\x20\x42\x1e\x1f\xc0\x84\x28" //L..]...U. B....( |
// /* 0af0 */ "\x17\x76\x78\x0e\xa0\x06\xd9\xf8\x5c\x39\x18\x9c\x7b\x5e\x14\x61" //.vx......9..{^.a |
// /* 0b00 */ "\x9d\x95\x1e\x9c\xae\xef\x17\xc4\xd1\xe1\x79\xc0\x0d\xc1\xfa\x17" //..........y..... |
// /* 0b10 */ "\xb1\xe3\xc3\x8e\x86\x4c\x00\x98\xf9\x23\x08\x26\x00\x5f\xb2\x1e" //.....L...#.&._.. |
// /* 0b20 */ "\x32\xaf\xa5\x3f\x6f\x3f\xf2\xfb\x71\x80\x1c\x1f\x72\x3c\x06\x99" //2..?o?..q...r<.. |
// /* 0b30 */ "\x45\x00\x07\xfc\x36\xe1\xdd\x01\x80\x00\x6c\x9c\x5e\x71\x62\xc0" //E...6.....l.^qb. |
// /* 0b40 */ "\xe8\x00\x5e\x1d\x16\x37\xf2\x05\x17\x00\x50\x3c\x0a\x16\x80\xd8" //..^..7....P<.... |
// /* 0b50 */ "\xc6\x1c\x78\x4e\xb9\x81\x4d\xad\x6b\x00\xec\x80\x0c\xb8\x18\xc0" //..xN..M.k....... |
// /* 0b60 */ "\xfc\x07\x9b\x32\xda\x0f\x40\x02\x49\x4a\xe2\x40\x11\x80\xcc\xa2" //...2..@.IJ.@.... |
// /* 0b70 */ "\x94\x24\xd6\xb8\x08\xa4\x94\x43\x8f\x32\x8f\xcb\xcd\x80\x25\x9d" //.$.....C.2....%. |
// /* 0b80 */ "\x66\xd4\x79\xd5\x5a\xe0\x00\x07\xc6\x8d\xe7\xd1\x84\x13\xe3\x87" //f.y.Z........... |
// /* 0b90 */ "\xbc\x6f\x01\xbc\x59\x5e\x37\xc4\x67\x03\xdf\xa9\xde\x32\xa3\x6f" //.o..Y^7.g....2.o |
// /* 0ba0 */ "\xd8\xb9\xd0\x02\x3d\xc0\x30\x45\x29\xb9\xd0\x30\x3c\x6b\xf7\x40" //....=.0E)..0<k.@ |
// /* 0bb0 */ "\x70\x00\x18\xe7\xee\xc4\x6b\x04\x05\xbf\x5c\x79\x96\x03\xbf\x7f" //p.....k....y.... |
// /* 0bc0 */ "\x28\x78\x00\x1a\x77\xef\xc5\x33\x04\x1b\x03\x7f\x7d\xff\x78\xfe" //(x..w..3....}.x. |
// /* 0bd0 */ "\x46\x00\xdf\x2d\x11\x4a\x5c\xba\x43\x93\x60\x3c\x40\xc3\xf0\x14" //F..-.J..C.`<@... |
// /* 0be0 */ "\x94\xab\x85\x48\x1f\xce\x8f\x3d\x3d\x82\xa7\xbd\xfa\x52\x3c\xf2" //...H...==....R<. |
// /* 0bf0 */ "\xf3\xfb\xd0\xd1\x70\x3f\xf4\x04\x8f\x77\x06\x18\x7b\xd6\x44\x00" //....p?...w..{.D. |
// /* 0c00 */ "\x00\xcd\xf9\x11\xe2\x6a\x04\x22\x01\xe1\xdc\x09\xe0\x2e\x30\xce" //.....j."......0. |
// /* 0c10 */ "\x45\xe0\x01\x8f\x63\xb9\x14\x00\x05\xe0\x33\x79\xaf\x65\x68\x17" //E...c.....3y.eh. |
// /* 0c20 */ "\xe4\xc4\xe1\x1c\x35\x92\xf6\x59\x81\x5e\xf3\x43\x08\x27\xcf\x4c" //....5..Y.^.C.'.L |
// /* 0c30 */ "\xa2\x9d\x5f\x20\x52\x02\xeb\x2d\xec\xcd\x02\x7f\x5f\x09\x9f\x58" //.._ R..-...._..X |
// /* 0c40 */ "\x79\x12\xfd\xce\xce\x70\x23\xd0\xc1\xd9\xc6\x04\x20\x48\x21\x13" //y....p#..... H!. |
// /* 0c50 */ "\xaa\xb5\x39\x35\xad\x80\x43\x82\x92\xe7\x1c\xe4\x5e\x00\x0f\xa1" //..95..C.....^... |
// /* 0c60 */ "\x34\xa5\x53\xfd\x02\x0e\x81\x48\x84\x3c\xea\xe4\x5a\x00\x0d\x81" //4.S....H.<..Z... |
// /* 0c70 */ "\x10\x00\x26\xb5\xf1\x02\x7a\xd4\xd2\x95\xd6\xe2\x03\x3d\x46\x73" //..&...z......=Fs |
// /* 0c80 */ "\xf9\x00\xb8\x14\x0b\xf2\x80\x07\x7f\x0c\xf6\xa5\x80\xaf\xc2\xeb" //................ |
// /* 0c90 */ "\x9b\x01\x3e\xcc\x3b\x7c\x80\x47\x22\xd1\x0f\xe0\x92\xc6\x3e\xd5" //..>.;|.G".....>. |
// /* 0ca0 */ "\xb0\x0f\xed\x77\x5e\x86\x00\x90\x51\xce\x7b\x58\xc0\x2c\x09\x94" //...w^...Q.{X.,.. |
// /* 0cb0 */ "\xa6\xdf\xe0\x17\x0b\xc1\x0f\x7e\xf0\x00\x1c\x0a\x86\x31\x4f\x7b" //.......~.....1O{ |
// /* 0cc0 */ "\xed\x7f\x00\x50\x2b\x18\xc4\xb9\xcd\xf3\xe5\x00\x7e\xa0\xd2\x4a" //...P+.......~..J |
// /* 0cd0 */ "\xfb\xf2\x80\x10\x36\x10\x88\xd5\x5b\x00\x2e\x87\x20\x64\x00\x03" //....6...[... d.. |
// /* 0ce0 */ "\x14\x6f\x37\x07\x48\x58\xbe\x30\x72\x04\xd2\x95\xf7\xc3\xbc\x79" //.o7.HX.0r......y |
// /* 0cf0 */ "\x9c\x55\x1c\xdf\x12\x63\x16\x3f\x90\x1b\x81\x22\xf8\x76\x60\x40" //.U...c.?...".v`@ |
// /* 0d00 */ "\x84\x36\xf7\xbe\xd5\x30\x33\xe0\xa5\x3d\xef\xbd\x3d\x50\xfc\x3c" //.6...03..=..=P.< |
// /* 0d10 */ "\xef\x4f\x02\xff\x79\x4f\x7b\xe0\x33\x02\xdf\xbe\x78\x0d\xc0\xac" //.O..yO{.3...x... |
// /* 0d20 */ "\x0a\x4c\x63\x6f\x7b\xe0\x31\x02\x9e\x13\x70\x09\x9f\x4c\x38\x01" //.Lco{.1...p..L8. |
// /* 0d30 */ "\x40\x8f\xa2\x92\xa5\x2e\x00\x91\x81\x46\x49\x29\x50\x29\x52\x94" //@........FI)P)R. |
// /* 0d40 */ "\x6a\xac\xc0\xb9\xc9\x21\xdc\x0a\x39\x1b\xc0\x17\x21\xb1\xe8\x6f" //j....!..9...!..o |
// /* 0d50 */ "\x7e\x72\x8c\x33\xde\x2e\xca\x31\xc9\x0e\x03\x3b\x8b\x0c\x20\x9e" //~r.3...1...;.. . |
// /* 0d60 */ "\xfa\x80\xbe\x46\x6f\x74\xd2\xcd\x3b\xdd\x33\xc8\xde\x07\x38\x63" //...Fot..;.3...8c |
// /* 0d70 */ "\x2a\x71\xcf\x70\x40\x36\x00\x03\x18\x41\x3d\xc5\x00\x9c\x8d\x5e" //*q.p@6...A=....^ |
// /* 0d80 */ "\xd3\xbf\xbb\x1e\xd3\xac\x01\x07\x1e\xd3\x80\x0c\x01\x09\x2f\xdf" //............../. |
// /* 0d90 */ "\x2a\x38\x59\x1f\xfe\x44\x60\x10\x18\xe2\xd7\x42\x3d\x3e\x36\x12" //*8Y..D`....B=>6. |
// /* 0da0 */ "\xed\x3f\x9f\x4f\x7d\x41\x61\xbd\xa9\x66\x80\x05\xd3\xa5\x50\x02" //.?.O}Aa..f....P. |
// /* 0db0 */ "\xb7\xbd\xf7\x72\x9e\xa0\x0f\x8e\x18\x60\x0a\xbf\xdf\xc5\xba\x74" //...r.....`.....t |
// /* 0dc0 */ "\xaa\x00\x5d\x5c\xa0\x7e\xbf\xd4\x02\x20\x20\xd0\xd1\x5e\x9d\x2a" //..]..~...  ..^.* |
// /* 0dd0 */ "\x87\x18\x80\x0f\x6e\x99\x43\x8a\xf6\xe9\x5c\x05\x42\x3b\x44\x00" //....n.C.....B;D. |
// /* 0de0 */ "\x16\xba\x7a\xbd\x26\x00\x1c\xe9\xea\xf1\x6a\xe0\x79\xd3\xe2\x44" //..z.&.....j.y..D |
// /* 0df0 */ "\x80\x00\xd7\x7c\x97\x8a\xde\x0b\x3e\x76\x8a\xe5\xa7\x01\x75\xf3" //...|....>v....u. |
// /* 0e00 */ "\x8f\x58\x07\x98\x04\xaf\xf7\xf0\xee\xbd\xd1\xd7\xde\x7a\xff\xbe" //.X...........z.. |
// /* 0e10 */ "\x6b\xc6\x40\x1e\x72\x8e\xf1\xce\xc0\x4e\xbe\xd1\xf9\x00\x05\xdd" //k.@.r....N...... |
// /* 0e20 */ "\x7c\xe3\x90\xd3\xb3\x13\xc6\x08\xd5\x5a\xe0\x21\xe3\x09\x13\x00" //|........Z.!.... |
// /* 0e30 */ "\x03\x5c\xf1\x86\x2b\x58\x2e\xc9\xd9\x8f\x84\x02\x61\xfd\x9f\xd6" //....+X......a... |
// /* 0e40 */ "\x3b\x31\x48\x01\x65\x3b\x9a\x28\x07\xb7\x8b\xf4\x4d\xbd\xef\xc4" //;1H.e;.(....M... |
// /* 0e50 */ "\x0f\x13\x3b\x34\x57\xb3\x9e\x3d\x07\x76\x66\x81\xcf\x2e\xa8\x02" //..;4W..=.vf..... |
// /* 0e60 */ "\x32\x21\x0e\xcc\xb4\x9a\xd7\x70\x0e\x05\xec\xcb\x22\x00\x22\x9c" //2!.....p....".". |
// /* 0e70 */ "\xd2\x18\x08\xef\x17\xf0\x11\x5e\xcc\xbb\x81\xfa\x99\x4b\x8e\x50" //.......^.....K.P |
// /* 0e80 */ "\x00\xec\x17\xfe\x3e\x8e\x0c\x22\x08\x0c\x02\xef\xcb\x71\x43\xaf" //....>..".....qC. |
// /* 0e90 */ "\x70\xc1\x4e\xaa\xd4\x00\x4f\x3e\xa1\x95\x60\x00\xa7\x5f\xdc\xbe" //p.N...O>..`.._.. |
// /* 0ea0 */ "\xdf\xf9\xc7\x3d\xb2\xc1\x05\x0f\xf5\x00\xb8\x06\x95\x63\xaf\x7e" //...=.........c.~ |
// /* 0eb0 */ "\xc1\x54\x97\xe2\xbf\xdc\xb0\xf3\xb0\x40\x41\xa8\xdf\x14\x5d\x04" //.T.......@A...]. |
// /* 0ec0 */ "\xfd\x81\x08\x24\x83\xc1\x96\x3f\x01\xb5\x47\x3e\x28\x3f\xeb\x51" //...$...?..G>(?.Q |
// /* 0ed0 */ "\x0d\x9f\xe6\x25\x13\x5f\x8a\xfa\x37\xfe\x26\x8f\x3d\xf8\xaf\xa3" //...%._..7.&.=... |
// /* 0ee0 */ "\x7f\x8a\x4a\x7c\x30\xf9\x8f\x18\x3c\x09\x72\xce\x90\x4f\xa2\xf0" //..J|0...<.r..O.. |
// /* 0ef0 */ "\xac\x92\xf8\x9a\x3f\x75\x81\x7e\xe8\xa5\xf8\x85\xf3\x1f\x76\x7c" //....?u.~......v| |
// /* 0f00 */ "\x87\x3f\x77\x4a\x67\xdd\xa9\x0f\xdd\x86\xaa\x70\xf6\x05\x8a\x81" //.?wJg......p.... |
// /* 0f10 */ "\x08\x52\x97\xc7\x0f\x30\x60\x44\xc4\x06\x65\x56\xa2\x91\x31\x8c" //.R...0`D..eV..1. |
// /* 0f20 */ "\x70\x10\x81\x09\x10\x86\x01\xf2\x60\xc0\xc1\x0d\x12\x85\x1c\x18" //p.......`....... |
// /* 0f30 */ "\xf5\x3c\x71\x30\x54\x00\x7b\xb8\x47\x50\x08\xe4\x0c\xec\x23\x1e" //.<q0T.{.GP....#. |
// /* 0f40 */ "\xc6\x71\xd5\x0f\xf5\x70\x76\x16\x88\x14\x4a\x29\x67\xec\xb5\x14" //.q...pv...J)g... |
// /* 0f50 */ "\xf7\xbe\xda\x4e\xa5\x51\xd7\x2e\x7e\x76\x25\x1f\x39\xd0\x79\xf1" //...N.Q..~v%.9.y. |
// /* 0f60 */ "\x5d\xf1\x10\x00\x2e\x18\xd0\x2a\x00\x07\x05\xe1\x8d\x8c\x0f\x06" //]......*........ |
// /* 0f70 */ "\xe5\x28\x04\x16\x0b\xfc\x06\x89\x5f\xa7\xdf\xc1\xee\x00\x01\x54" //.(......_......T |
// /* 0f80 */ "\x00\x32\x26\x38\x08\x3d\x1a\x80\x0d\x6b\x18\xa3\x7c\xa5\x11\xce" //.2&8.=...k..|... |
// /* 0f90 */ "\x9e\xea\x4b\x40\x03\xa1\x47\x3a\x15\xa8\x00\xd7\x35\xa6\x00\x90" //..K@..G:....5... |
// /* 0fa0 */ "\x00\x0b\x81\x68\x14\x07\xc3\xe1\xf8\x0f\x87\x63\xb3\xdb\xca\xb3" //...h.......c.... |
// /* 0fb0 */ "\xd8\xf6\xbe\x73\xb9\xd4\x7c\xe7\x7a\x2b\xec\xd4\x1f\x81\x60\x42" //...s..|.z+....`B |
// /* 0fc0 */ "\xe7\x9a\xa0\x42\x09\xe6\xab\xfe\x37\xcf\xf1\xfc\x62\x86\x30\x00" //...B....7...b.0. |
// /* 0fd0 */ "\x33\xe5\x88\x16\x00\x03\x3c\xf2\xc6\x27\x58\x39\xa1\x40\x28\xbf" //3.....<..'X9.@(. |
// /* 0fe0 */ "\xe0\x46\xa0\xbd\x1e\xc1\xe5\x88\xa2\xba\x92\xc0\x66\x4d\x94\x00" //.F..........fM.. |
// /* 0ff0 */ "\x69\x34\x66\x80\x16\xea\x4b\x02\x99\x36\x2b\x49\xa8\xf3\x48\xcd" //i4f...K..6+I..H. |
// /* 1000 */ "\x16\x14\x45\xe5\x99\x90\x91\xd0\x02\xfe\x5c\xb4\x5e\x27\xb7\x04" //..E.........^'.. |
// /* 1010 */ "\x00\x5f\x07\xc0\xb8\x00\x19\xef\x83\xf1\x3b\xc1\xd5\xaa\x00\xf5" //._........;..... |
// /* 1020 */ "\x80\x1e\x03\x40\x00\x0f\x81\x04\x21\x72\x16\x4b\x80\x5c\x7e\x7a" //...@....!r.K..~z |
// /* 1030 */ "\x00\x0f\x70\x40\x08\xa7\x4d\x3c\x28\x64\x25\x80\xcb\x5b\x28\x00" //..p@..M<(d%..[(. |
// /* 1040 */ "\xb3\x48\xcc\x05\x52\x4d\x4e\x96\x78\x3d\x04\xf0\x7c\x30\x02\x78" //.H..RMN.x=..|0.x |
// /* 1050 */ "\x40\x65\xe1\x09\x18\x00\x03\x38\xf0\x86\x26\xd8\x3b\xe4\x43\xce" //@e.....8..&.;.C. |
// /* 1060 */ "\xf4\x1f\x01\x02\x10\xa0\x0f\x0f\xde\x2e\x10\x00\x1b\x9c\x00\x20" //...............  |
// /* 1070 */ "\x7e\xf1\x01\x29\x49\xa0\x05\x1a\xca\x60\x2a\x51\x3a\x00\x35\x98" //~..)I....`*Q:.5. |
// /* 1080 */ "\xd2\x9c\x6b\x29\x7b\xe6\x86\xf3\x0c\x00\xb8\xfb\xea\x49\x7c\x17" //..k){........I|. |
// /* 1090 */ "\x23\x20\x00\x67\x5e\x0b\xc4\xe3\x07\xa0\x28\xff\xe9\x37\xb4\x50" //# .g^.....(..7.P |
// /* 10a0 */ "\x0a\x00\x2f\x1e\xaa\x51\xfc\x00\x28\x49\xa2\x92\x4a\x60\x29\xbe" //../..Q..(I..J`). |
// /* 10b0 */ "\x06\x5d\x0d\x28\x01\x7f\x2e\x7b\x6b\x47\x82\xa0\x31\xc7\xe8\x94" //.].(...{kG..1... |
// /* 10c0 */ "\x7e\x5b\xd5\x16\x35\xcf\x0d\xe0\xdc\x60\x18\x3e\x25\x4f\x00\xbc" //~[..5....`.>%O.. |
// /* 10d0 */ "\x40\x62\xe8\x14\x02\x01\xf0\x0f\x06\xa9\xf7\x43\x48\x08\xbc\xb6" //@b.........CH... |
// /* 10e0 */ "\x8d\x66\x8e\x50\x02\xcd\x48\x60\x2a\x4d\x74\xeb\x34\x71\xda\xf1" //.f.P..H`*Mt.4q.. |
// /* 10f0 */ "\x3d\xa0\x05\x95\x10\xf8\x09\xe9\xe9\x69\x02\xc0\x6b\x0e\x11\xac" //=........i..k... |
// /* 1100 */ "\x0d\x9e\x1e\xeb\xa0\xe4\x16\x1e\xf9\x93\x01\xc0\xb8\xd4\x3b\xcd" //..............;. |
// /* 1110 */ "\xbd\xf3\x4a\xb2\xde\x9e\x75\x77\x9e\xa8\xc3\x3e\x78\x52\x4b\xd3" //..J...uw...>xRK. |
// /* 1120 */ "\xc6\x06\xbf\xd3\xa7\x7c\x0d\x76\x01\x7d\x54\x5a\x6b\x50\x05\x91" //.....|.v.}TZkP.. |
// /* 1130 */ "\x88\x60\x40\x49\x1b\x00\x03\x8b\x72\xb8\xc6\x28\x02\x38\x01\x40" //.`@I....r..(.8.@ |
// /* 1140 */ "\x03\x1f\x00\x80\xa8\xb5\xf2\xb2\x9e\x84\x1a\x09\x40\x47\x80\x3a" //............@G.: |
// /* 1150 */ "\x58\xc0\x5c\x06\x36\x21\xe4\x52\xc7\x0e\xf4\x06\xf2\xf7\xca\xea" //X...6!.R........ |
// /* 1160 */ "\x70\x00\xf8\x2b\x98\xc9\xdf\x9a\x40\x1e\x03\x5d\x18\xef\x8b\x00" //p..+....@..].... |
// /* 1170 */ "\xf0\x17\x72\x9e\x57\x72\xab\x2d\xe5\x8e\x12\x63\xf1\xa8\x74\x03" //..r.Wr.-...c..t. |
// /* 1180 */ "\xb0\x4c\x04\xc0\x6b\x43\x1f\xe8\x6b\xc5\x45\x01\xa8\x3d\xfd\x72" //.L..kC..k.E..=.r |
// /* 1190 */ "\xe0\x47\x4f\x08\x08\xf0\x8d\x5c\x96\xbd\x36\x1e\x00\xb8\x7d\x98" //.GO.......6...}. |
// /* 11a0 */ "\xbc\x8c\x74\xff\x80\x78\x0c\xcd\x4b\xea\x07\x7e\xa5\x0c\x90\xf6" //..t..x..K..~.... |
// /* 11b0 */ "\x2d\x80\x68\x0d\x4f\xf4\x41\xef\xd4\xa3\x8e\xa3\xd2\x0b\xff\x26" //-.h.O.A........& |
// /* 11c0 */ "\x02\x7a\x01\xd4\x86\x01\x3c\x1d\x4a\x2d\x7f\x48\xf8\x11\xd4\xa6" //.z....<.J-.H.... |
// /* 11d0 */ "\x00\xe0\x22\xa8\x5f\xcd\xe1\xbb\x19\x21\xec\x8d\x00\x50\x13\x2f" //.."._....!...P./ |
// /* 11e0 */ "\x2c\x43\xd8\x47\x54\x08\x01\xfc\x3a\x9c\x10\xf5\x4c\x98\x0b\x8f" //,C.GT...:...L... |
// /* 11f0 */ "\x80\xa2\xb8\x31\xd5\x52\x07\xe0\x33\x77\x3f\xc2\x9f\x08\x72\xd5" //...1.R..3w?...r. |
// /* 1200 */ "\xc4\x62\x8b\x66\x8e\x40\x47\x6c\xfd\x10\x20\x5a\x88\x02\x43\x03" //.b.f.@Gl.. Z..C. |
// /* 1210 */ "\x59\xe8\x88\xe5\xb9\x5c\xb8\xc3\x95\x00\x16\x5e\x89\xce\x5b\x55" //Y..........^..[U |
// /* 1220 */ "\xf1\xc3\x7c\x10\x9d\xa8\xe0\x77\xe6\x3c\x0e\xab\xe0\x88\x07\xc7" //..|....w.<...... |
// /* 1230 */ "\x60\x81\xe8\xed\x30\xef\x3b\x50\x2b\xa4\x73\xc6\xa1\xdf\xd3\x1f" //`...0.;P+.s..... |
// /* 1240 */ "\x22\x6e\x7c\xd3\x9c\x23\x17\x0b\xef\x2c\xd9\x1c\x00\x03\x5f\xe1" //"n|..#...,...._. |
// /* 1250 */ "\xfd\x8a\xee\x12\x1d\x75\x43\xe4\x00\x97\x50\x3a\x69\xba\x6d\x3a" //.....uC...P:i.m: |
// /* 1260 */ "\xa1\x4f\x51\xe8\xb7\xbd\xf9\x26\xba\x9b\x39\x06\x03\x82\x3a\x65" //.OQ....&..9...:e |
// /* 1270 */ "\xc7\x54\x8a\x8f\x5d\xfa\x71\x03\xbf\x44\x75\x5d\x79\xd1\xd5\xdd" //.T..].q..Du]y... |
// /* 1280 */ "\xd3\x4e\x3c\x14\xb8\x08\x71\x70\x91\xd0\x00\x34\x8f\x19\x62\x8d" //.N<...qp...4..b. |
// /* 1290 */ "\x84\xa5\xc1\xde\x06\x60\xc0\xb8\xf1\x8c\x78\xe1\x01\x57\xae\xd2" //.....`....x..W.. |
// /* 12a0 */ "\xe9\xf3\xab\x85\xd0\x0f\xaa\x94\x10\x43\xae\x14\x00\x3c\x0a\x9c" //.........C...<.. |
// /* 12b0 */ "\xc0\xa7\xf5\x81\x52\x1d\x4c\x27\x02\x6b\x30\x6e\xcc\x36\xb5\xf5" //....R.L'.k0n.6.. |
// /* 12c0 */ "\x31\xa8\x10\x85\xc2\x9c\x0c\x03\xd4\xc6\x14\x00\x47\x80\x00\xe5" //1...........G... |
// /* 12d0 */ "\x7c\x4e\x71\x94\x80\x8f\xf5\x31\x53\xe7\x50\x18\x8d\x2f\xc3\x30" //|Nq....1S.P../.0 |
// /* 12e0 */ "\xab\x2d\xa7\x7a\x4d\x4f\x7b\x11\x80\xd2\xd6\xbe\x82\xa0\x07\xd6" //.-.zMO{......... |
// /* 12f0 */ "\x65\xc3\x29\x82\x3c\xc3\xf8\x08\x39\xf1\x5c\xdd\x22\x03\xcd\x5a" //e.).<...9..."..Z |
// /* 1300 */ "\x3b\xc7\xbe\x2b\xa1\x34\x73\xa4\x85\xfe\x55\x2f\x94\xe6\xfb\x18" //;..+.4s...U/.... |
// /* 1310 */ "\x01\x61\xf1\x58\x0c\xd8\x3f\xf5\x14\x00\x1a\x18\xc7\x96\x60\x07" //.a.X..?.......`. |
// /* 1320 */ "\xed\xa8\x7c\xe1\x80\xf0\x17\x9a\x77\x16\x6a\x8d\x55\x86\x28\x00" //..|.....w.j.U.(. |
// /* 1330 */ "\xa4\x10\xee\xb5\x2c\x02\x19\x14\x00\x19\x94\x53\x01\x93\x4f\x44" //....,......S..OD |
// /* 1340 */ "\x45\x0f\x83\x08\x1e\x03\x06\x31\xa0\x00\xf4\x9a\x01\x1c\x51\x80" //E......1......Q. |
// /* 1350 */ "\x70\x06\x53\xb9\x0b\xea\x3c\x2a\x5f\xf0\xfd\x78\xa6\x7c\x67\x3e" //p.S...<*_..x.|g> |
// /* 1360 */ "\x3d\x9f\x23\xcf\x94\x67\xcb\x73\xe6\x59\xf3\x7c\xf9\xe6\x7a\x4e" //=.#..g.s.Y.|..zN |
// /* 1370 */ "\x04\x3b\x8d\x1b\xf8\x60\x04\x30\x35\x12\x03\x10\x8a\x50\xe0\x55" //.;...`.05....P.U |
// /* 1380 */ "\x6a\x6c\x8a\xad\xd7\x79\x42\x9d\x08\x40\x68\x55\xea\x1c\x8a\xad" //jl...yB..@hU.... |
// /* 1390 */ "\xc7\x21\x7c\xbf\xbb\xd8\x2f\xe3\x1c\x31\x54\xf6\xd6\xb8\x09\xd5" //.!|.../..1T..... |
// /* 13a0 */ "\x9f\x02\x74\x46\x06\xd6\x56\x7a\xcc\xc4\x63\x01\xd2\xa0\x26\xfd" //..tF..Vz..c...&. |
// /* 13b0 */ "\x5d\x62\x2b\xd7\x5a\x05\x3a\xdc\xb9\x5a\x40\xf9\x0f\x61\x47\x5b" //]b+.Z.:..Z@..aG[ |
// /* 13c0 */ "\xf9\xeb\x2d\x3d\x79\xa7\xac\x7c\xc1\x3f\x84\x49\xff\x5c\x36\xc3" //..-=y..|.?.I..6. |
// /* 13d0 */ "\x3e\x2b\x88\xab\xf4\x5f\xe3\x99\x27\xde\x88\x4f\xde\x82\xfe\xb8" //>+..._..'..O.... |
// /* 13e0 */ "\x75\xf6\xf2\xec\x0f\x3f\xe6\x7b\x30\xdf\xdb\x94\x1f\xb7\x08\xb4" //u....?.{0....... |
// /* 13f0 */ "\xe1\x3e\x1b\x91\x85\x28\x0c\xbc\x37\x31\x63\xc2\x89\xa1\xde\x15" //.>...(..71c..... |
// /* 1400 */ "\x22\x1e\x0f\x2e\x7f\x60\x00\x3a\x00\x6f\x03\x06\x09\xac\x8a\x00" //"....`.:.o...... |
// /* 1410 */ "\x8d\x0c\x63\x04\x12\xc5\x00\x37\x80\xc8\x06\x45\x83\xd1\x18\x20" //..c....7...E...  |
// /* 1420 */ "\x9a\x26\x51\x4a\x00\x6f\x01\x56\x09\x1d\x9c\x12\x1e\x1d\xfe\x89" //.&QJ.o.V........ |
// /* 1430 */ "\x1f\xf4\x48\xaa\xd0\x32\x5b\x3a\x35\xbc\x12\xcf\xeb\x25\x5a\x03" //..H..2[:5....%Z. |
// /* 1440 */ "\x27\x92\xf5\x90\x30\x08\x75\xb1\xa8\x51\x15\xc1\x00\x1f\xc7\x10" //'...0.u..Q...... |
// /* 1450 */ "\x40\x01\x83\xef\xf5\x21\x00\xe1\xc4\x22\xe0\xb4\xe4\xd8\xf2\x05" //@....!..."...... |
// /* 1460 */ "\x0c\xf8\xfd\xc2\xaf\x0a\x00\x87\x91\xf0\x0a\xf2\x3d\x70\x9c\xc3" //............=p.. |
// /* 1470 */ "\xc8\x71\xe4\xfc\x42\x7e\x63\x85\x05\xf2\x6c\x8f\x2b\xe1\x40\x09" //.q..B~c...l.+.@. |
// /* 1480 */ "\xef\xbe\xa0\x01\xc0\x6e\xe6\xb4\x24\x59\x0c\x06\x6c\x5b\x40\x10" //.....n..$Y..l[@. |
// /* 1490 */ "\x75\x25\x81\xc6\x95\xa7\x4b\xcb\x28\xc6\x30\x1b\x59\xa5\x1d\x90" //u%....K.(.0.Y... |
// /* 14a0 */ "\x97\x01\xdf\xba\xdb\x4c\x13\xcb\xb8\x10\x80\x01\xc8\x7c\xbb\x8c" //.....L.......|.. |
// /* 14b0 */ "\x7f\x0a\xad\x01\xfc\xbd\xc0\x21\xc3\xb8\x43\xe6\x25\x1f\xcb\x2f" //.......!..C.%../ |
// /* 14c0 */ "\x32\xf0\xf1\x65\x80\x18\x10\x80\x0f\x9a\xa2\x8f\xe5\x58\xf3\x6f" //2..e.........X.o |
// /* 14d0 */ "\x18\x07\xcc\x88\x73\x98\x0d\x1f\xcd\x2f\x3a\xf2\x60\x37\xf7\x88" //....s..../:.`7.. |
// /* 14e0 */ "\xd7\x0a\x08\x1f\x98\x10\x1b\x31\xad\x09\x52\x43\x01\x92\xf9\xec" //.......1..RC.... |
// /* 14f0 */ "\xc8\x4b\x04\x85\x2b\x4a\xd7\x92\x11\x98\x30\x31\x7c\x76\x0f\x50" //.K..+J....01|v.P |
// /* 1500 */ "\x00\x2e\x0a\x3e\x74\x92\x20\x00\x38\xe7\x9d\x31\x8d\x00\x11\xf5" //...>t. .8..1.... |
// /* 1510 */ "\xed\x91\x0f\x05\x88\x7c\xe6\xf3\xc1\x14\xa4\x8a\xa3\xde\x5d\x70" //.....|........]p |
// /* 1520 */ "\x01\x03\xe3\x2a\xf3\x1c\x00\xe0\x2d\x96\x09\xbd\x8f\x28\xc2\x6b" //...*....-....(.k |
// /* 1530 */ "\x5d\x46\x86\x30\xff\xe7\x08\xc0\x93\x8f\x2e\x56\x09\xfe\x8c\x02" //]F.0.......V.... |
// /* 1540 */ "\xb2\xf3\xa4\x60\x24\xb9\xb9\xc7\x2a\x38\x0a\x65\xe1\x47\x96\xe2" //...`$...*8.e.G.. |
// /* 1550 */ "\xa0\x03\xe9\x38\xe5\xa4\xf3\x9c\x01\x2a\xcb\x40\x7d\xc0\xf4\x52" //...8.....*.@}..R |
// /* 1560 */ "\xe1\x51\x03\x82\x10\x84\x1c\xd2\xab\x55\x28\xd5\x4a\x80\x20\x2b" //.Q.......U(.J. + |
// /* 1570 */ "\x29\x80\xbb\x4c\x68\x44\xa5\xb0\x19\xbc\xe6\x8e\x2b\x29\x11\xec" //)..LhD......+).. |
// /* 1580 */ "\xf0\x60\x0e\xec\xf0\xe0\x47\x44\x60\x00"                         //.`....GD`. |
// Sent dumped on RDP Client (5) 5514 bytes |
// send_server_update done |
// GraphicsUpdatePDU::init::Initializing orders batch mcs_userid=0 shareid=65538 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[1](30) used=149 free=16129 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=36) |
// front::draw:draw_tile((640, 336, 64, 64) (448, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[2](8208) used=155 free=16123 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[3](30) used=265 free=16013 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=37) |
// front::draw:draw_tile((704, 336, 64, 64) (512, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[4](8208) used=271 free=16007 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[5](30) used=721 free=15557 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=38) |
// front::draw:draw_tile((768, 336, 64, 64) (576, 192, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[6](8208) used=727 free=15551 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[7](30) used=812 free=15466 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,336,64,64) rop=cc srcx=0 srcy=0 cache_idx=39) |
// front::draw:draw_tile((192, 400, 64, 64) (0, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[8](8208) used=818 free=15460 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[9](30) used=911 free=15367 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=40) |
// front::draw:draw_tile((256, 400, 64, 64) (64, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[10](8208) used=920 free=15358 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[11](30) used=1370 free=14908 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=41) |
// front::draw:draw_tile((320, 400, 64, 64) (128, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[12](8208) used=1376 free=14902 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[13](30) used=1631 free=14647 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=42) |
// front::draw:draw_tile((384, 400, 64, 64) (192, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[14](8208) used=1637 free=14641 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[15](30) used=1936 free=14342 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=43) |
// front::draw:draw_tile((448, 400, 64, 64) (256, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[16](8208) used=1942 free=14336 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[17](30) used=2265 free=14013 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=44) |
// front::draw:draw_tile((512, 400, 64, 64) (320, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[18](8208) used=2271 free=14007 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[19](30) used=2592 free=13686 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=45) |
// front::draw:draw_tile((576, 400, 64, 64) (384, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[20](8208) used=2598 free=13680 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[21](30) used=2897 free=13381 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=46) |
// front::draw:draw_tile((640, 400, 64, 64) (448, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[22](8208) used=2903 free=13375 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[23](30) used=3141 free=13137 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=47) |
// front::draw:draw_tile((704, 400, 64, 64) (512, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[24](8208) used=3147 free=13131 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[25](30) used=3582 free=12696 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=48) |
// front::draw:draw_tile((768, 400, 64, 64) (576, 256, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[26](8208) used=3588 free=12690 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[27](30) used=3680 free=12598 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,400,64,64) rop=cc srcx=0 srcy=0 cache_idx=49) |
// front::draw:draw_tile((192, 464, 64, 64) (0, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[28](8208) used=3686 free=12592 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[29](30) used=3806 free=12472 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=50) |
// front::draw:draw_tile((256, 464, 64, 64) (64, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[30](8208) used=3815 free=12463 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[31](30) used=4091 free=12187 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=51) |
// front::draw:draw_tile((320, 464, 64, 64) (128, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[32](8208) used=4097 free=12181 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[33](30) used=4603 free=11675 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=52) |
// front::draw:draw_tile((384, 464, 64, 64) (192, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[34](8208) used=4609 free=11669 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[35](30) used=4815 free=11463 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=53) |
// front::draw:draw_tile((448, 464, 64, 64) (256, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[36](8208) used=4821 free=11457 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[37](30) used=4983 free=11295 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=54) |
// front::draw:draw_tile((512, 464, 64, 64) (320, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[38](8208) used=4989 free=11289 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[39](30) used=5149 free=11129 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=55) |
// front::draw:draw_tile((576, 464, 64, 64) (384, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[40](8208) used=5155 free=11123 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[41](30) used=5346 free=10932 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=56) |
// front::draw:draw_tile((640, 464, 64, 64) (448, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[42](8208) used=5352 free=10926 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[43](30) used=5804 free=10474 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=57) |
// front::draw:draw_tile((704, 464, 64, 64) (512, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[44](8208) used=5810 free=10468 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[45](30) used=6069 free=10209 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=58) |
// front::draw:draw_tile((768, 464, 64, 64) (576, 320, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[46](8208) used=6075 free=10203 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[47](30) used=6195 free=10083 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,464,64,64) rop=cc srcx=0 srcy=0 cache_idx=59) |
// front::draw:draw_tile((192, 528, 64, 64) (0, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[48](8208) used=6201 free=10077 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[49](30) used=6325 free=9953 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=60) |
// front::draw:draw_tile((256, 528, 64, 64) (64, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[50](8208) used=6334 free=9944 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[51](30) used=6472 free=9806 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=61) |
// front::draw:draw_tile((320, 528, 64, 64) (128, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[52](8208) used=6478 free=9800 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[53](30) used=6832 free=9446 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=62) |
// front::draw:draw_tile((384, 528, 64, 64) (192, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[54](8208) used=6838 free=9440 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[55](30) used=7365 free=8913 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=63) |
// front::draw:draw_tile((448, 528, 64, 64) (256, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[56](8208) used=7371 free=8907 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[57](30) used=7453 free=8825 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=64) |
// front::draw:draw_tile((512, 528, 64, 64) (320, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[58](8208) used=7459 free=8819 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[59](30) used=7518 free=8760 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=65) |
// front::draw:draw_tile((576, 528, 64, 64) (384, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[60](8208) used=7524 free=8754 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[61](30) used=8003 free=8275 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=66) |
// front::draw:draw_tile((640, 528, 64, 64) (448, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[62](8208) used=8009 free=8269 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[63](30) used=8324 free=7954 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=67) |
// front::draw:draw_tile((704, 528, 64, 64) (512, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[64](8208) used=8330 free=7948 |
// GraphicsUpdatePDU::flush_orders: order_count=64 |
// send_server_update: fastpath_support=yes compression_support=yes shareId=65538 encryptionLevel=0 initiator=0 type=0 data_extra=64 |
// Sending on RDP Client (5) 5230 bytes |
// /* 0000 */ "\x00\x94\x6e\x80\x21\x67\x14\x40\x00\x03\x84\x79\xc7\x18\x36\x15" //..n.!g.@...y..6. |
// /* 0010 */ "\xfe\x0c\xf9\xb7\x5f\x09\x38\x10\x38\x58\xc7\x9a\x76\xf3\x2c\x70" //...._.8.8X..v.,p |
// /* 0020 */ "\xab\x9f\x31\x71\xf0\xab\x8f\x2d\xf2\x70\xbf\x79\x09\x01\x87\xc6" //..1q...-.p.y.... |
// /* 0030 */ "\x68\x00\x20\x92\x98\x19\x45\x2a\x97\x52\x85\x54\x88\x00\x20\x2c" //h. ...E*.R.T.. , |
// /* 0040 */ "\xb3\x1a\x11\xa1\x6c\x06\x4d\x39\xa3\x85\x28\xf6\xcc\x30\x0c\x76" //....l.M9..(..0.v |
// /* 0050 */ "\xcc\x79\x76\x89\x00\x00\xd8\x7c\xb7\x8a\xfe\x17\x4e\x79\x4d\xe5" //.yv....|....NyM. |
// /* 0060 */ "\xca\x1d\xf2\xa4\x71\xcc\x79\x52\x30\x5c\x65\xf8\xfd\xe4\xbc\x38" //....q.yR0.e....8 |
// /* 0070 */ "\xee\xbc\x6f\x3e\x43\xbe\x3a\x33\x20\x03\xa7\x82\x4a\x60\x2d\xea" //..o>C.:3 ...J`-. |
// /* 0080 */ "\x34\x00\x01\x52\x18\x19\xca\x15\x69\x6a\x34\xa4\x61\x4d\x35\x21" //4..R....ij4.aM5! |
// /* 0090 */ "\xa7\x82\x28\xf6\xf3\x30\x08\x76\xf3\x78\xd2\x89\x40\x00\xe6\xbd" //..(..0.v.x..@... |
// /* 00a0 */ "\x01\xf1\x99\x80\x93\x75\xaa\x7b\x1a\x84\x79\x98\xba\x03\x47\x68" //.....u.{..y...Gh |
// /* 00b0 */ "\x4a\xaf\x42\xe8\x00\xa3\xcf\x63\x1c\xd2\xab\x9a\xa4\xf3\x5e\x9e" //J.B....c......^. |
// /* 00c0 */ "\x6d\x13\xcd\xca\x79\xbe\x4f\x38\x69\xe7\x21\x3c\xe6\xa7\x9d\x24" //m...y.O8i.!<...$ |
// /* 00d0 */ "\xf3\xae\x03\xf3\xf4\x70\x08\xc3\x86\x1c\x7e\x38\x07\xe8\x00\xe8" //.....p....~8.... |
// /* 00e0 */ "\x0c\xfc\x2c\x10\x04\x07\xe7\xda\xe0\x1b\x8f\x40\x47\x89\xc2\x3d" //..,........@G..= |
// /* 00f0 */ "\xbd\xe6\x3c\x0f\xcf\xcd\xc0\x3f\x0e\x81\x51\x01\xc1\x3b\xf1\xe1" //..<....?..Q..;.. |
// /* 0100 */ "\x48\xe8\x05\xeb\x8f\xf3\xff\x71\xc8\x0c\x50\xfb\xf2\x1d\x8a\x68" //H......q..P....h |
// /* 0110 */ "\x40\xb2\x58\x0d\x5c\xe6\x98\x6a\x43\x05\x54\x29\x73\x29\x88\x98" //@.X....jC.T)s).. |
// /* 0120 */ "\xcc\x0c\x54\x12\x9c\xbb\x27\x1d\xfb\xc1\x97\xb0\x0d\xc4\x3e\x04" //..T...'.......>. |
// /* 0130 */ "\x74\x06\x8e\xfe\xc0\xfe\xee\xce\x62\xe1\x0f\x92\xbc\xf7\x40\x8e" //t.......b.....@. |
// /* 0140 */ "\xdc\x13\x76\xf1\xec\x44\xfc\x53\x1d\xaa\x03\xf0\x1e\xd7\x52\x3c" //..v..D.S......R< |
// /* 0150 */ "\x01\x83\xae\x20\x3f\xba\xbb\xe9\x27\x5d\xea\xb1\xed\xba\xee\xbd" //... ?...']...... |
// /* 0160 */ "\x23\xf0\x19\x87\x6c\x67\x72\x76\x05\xd5\x75\xd0\x77\xcb\x91\x9c" //#...lgrv..u.w... |
// /* 0170 */ "\x11\x5f\x1e\x0c\x9e\xf5\x0f\x03\xd9\xe8\x06\x05\xe5\x80\xd8\x0c" //._.............. |
// /* 0180 */ "\xef\xf0\xc3\xbf\x80\xa3\xc5\x13\xde\xca\x4f\x00\x30\x39\x9c\x06" //..........O.09.. |
// /* 0190 */ "\x77\xc9\xaf\xf0\x0c\xe1\x11\x1c\x1e\x5d\x8b\x74\x27\x00\xa5\x8f" //w........].t'... |
// /* 01a0 */ "\x3d\xc0\x63\xf8\x4f\xea\x7b\x17\x30\x7d\xa2\xff\x7f\x9d\x55\x83" //=.c.O.{.0}....U. |
// /* 01b0 */ "\xb8\xa9\x29\x71\xe0\x7e\xd6\xeb\x80\x10\x27\x62\x58\xe6\x0b\x18" //..)q.~....'bX... |
// /* 01c0 */ "\x33\x12\x3b\x43\xcf\x6a\xf8\xf5\x73\x0a\x7d\x3b\x15\x47\x05\x85" //3.;C.j..s.};.G.. |
// /* 01d0 */ "\xc2\xa0\x0d\xe1\xb8\xc3\x39\xcc\x1c\x95\xa1\x47\x9e\x1d\xa2\x39" //......9....G...9 |
// /* 01e0 */ "\x00\x01\xbb\x39\x5f\x15\x26\x05\x55\x76\x72\xac\x17\xf4\xaf\x03" //...9_.&.Uvr..... |
// /* 01f0 */ "\xf8\x4d\xe1\x28\xc0\xa5\x58\x61\x59\x9d\x83\xbb\x48\x4c\x40\x04" //.M.(..XaY...HL@. |
// /* 0200 */ "\xa3\x8a\x4e\xc5\x3d\xa4\x36\xc1\xd6\x71\xc2\xc8\x13\x81\xd4\xd6" //..N.=.6..q...... |
// /* 0210 */ "\xfc\x07\x65\x30\x19\x52\x93\x57\x1e\x00\xae\x07\xf1\x7c\xd3\x6b" //..e0.R.W.....|.k |
// /* 0220 */ "\x5b\x5f\xcd\x0a\x70\xcf\xae\x0c\xc1\xee\x16\xc6\xcf\xa4\x13\xfe" //[_..p........... |
// /* 0230 */ "\x8e\xfb\x2c\x06\xf5\x50\x10\xc6\x3c\x04\x36\xc1\x17\xaf\xa9\xb3" //..,..P..<.6..... |
// /* 0240 */ "\x77\xd4\x08\xf1\x13\x8f\xa9\x89\x63\x1b\x7f\xd4\x74\x2c\x8f\x07" //w.......c...t,.. |
// /* 0250 */ "\x2c\x78\x2d\x51\xc3\xe8\xe1\xf4\x81\xfe\x35\x7d\x27\x14\x39\xfd" //,x-Q......5}'.9. |
// /* 0260 */ "\x33\x82\x33\x5f\x5c\x1d\xc4\xf2\x02\x70\xa0\x23\xf8\xc0\xf6\x07" //3.3_.....p.#.... |
// /* 0270 */ "\x52\x79\x80\x13\xee\x35\x00\x13\x03\x2d\xa6\x51\x4c\x13\x96\xc0" //Ry...5...-.QL... |
// /* 0280 */ "\x94\x53\x80\x6a\xa3\xf4\xba\xe1\xb8\x47\xc7\x80\x25\xb5\xaf\x0b" //.S.j.....G..%... |
// /* 0290 */ "\x6d\x00\x03\x38\x60\x47\x85\x67\x02\x09\x47\x8b\x0e\x9f\x54\x17" //m..8`G.g..G...T. |
// /* 02a0 */ "\xc5\xf6\xbf\x8c\xe0\x94\x67\x61\x47\x92\x22\xde\xf6\xeb\xf7\x01" //......gaG."..... |
// /* 02b0 */ "\x5e\x39\x7d\x27\x1a\x39\xf5\x94\xe0\xd0\x98\xf0\x95\x4b\x9c\xdd" //^9}'.9.......K.. |
// /* 02c0 */ "\xbf\x08\x27\xd0\x6f\xa4\xfc\x33\xe5\x47\x87\xaf\xdd\x4f\x6e\xdd" //..'.o..3.G...On. |
// /* 02d0 */ "\xbd\xed\xdf\xf3\x02\x38\xeb\x57\x8a\x27\x8f\xa4\xf8\x32\x7f\x5c" //.....8.W.'...2.. |
// /* 02e0 */ "\xf9\x8d\xdb\xe2\xbf\xa0\x3f\xb9\x6c\x22\x94\x88\x7b\x29\x42\xa5" //......?.l"..{)B. |
// /* 02f0 */ "\xad\x78\x9e\x02\xfc\xd2\x10\xe4\xa2\xa1\x1a\x49\x7e\xc8\x6f\x90" //.x.........I~.o. |
// /* 0300 */ "\x82\x9f\x2a\x52\xf6\x53\x98\x8c\x94\x80\x01\xdc\xb8\x77\x63\x70" //..*R.S.......wcp |
// /* 0310 */ "\xc3\x7f\x20\xa8\x43\x08\x43\x29\x4b\x4a\x52\x6b\x5a\x8c\x30\xd2" //.. .C.C)KJRkZ.0. |
// /* 0320 */ "\x4a\x71\x46\x1c\xb3\x90\xc0\x03\x6f\x78\x4c\x08\x29\x25\x30\xc1" //JqF.....oxL.)%0. |
// /* 0330 */ "\xa7\x82\x84\x24\x52\x94\x10\x88\x10\x84\x18\xc6\x14\xa4\xda\xd6" //...$R........... |
// /* 0340 */ "\xe3\x85\x19\x4b\x34\xe3\xcb\x3d\x7e\xff\x5b\xaf\x5d\xe7\xb7\x3c" //...K4..=~.[.]..< |
// /* 0350 */ "\x03\x22\x0c\x63\x08\x42\x24\x21\x20\x00\xa1\x8c\x49\x8c\x65\x4a" //.".c.B$! ...I.eJ |
// /* 0360 */ "\x50\xe7\x38\x96\x14\x3f\x61\x45\xd8\xc2\x6f\x15\x53\x8e\x53\xde" //P.8..?aE..o.S.S. |
// /* 0370 */ "\xed\x6b\x56\x73\x9f\x34\x00\x2c\x63\x14\x73\x9e\x86\xa0\xa0\xf8" //.kVs.4.,c.s..... |
// /* 0380 */ "\x00\x6a\x08\x45\x8c\x62\x0c\x62\x50\x85\x4b\x5a\x6d\xc1\x2e\xc9" //.j.E.b.bP.KZm... |
// /* 0390 */ "\x94\x50\xd3\x00\xf4\x2d\xef\x69\xad\x72\xa5\x20\x84\x26\x52\x9a" //.P...-.i.r. .&R. |
// /* 03a0 */ "\x18\xc1\x00\x03\x08\x40\x21\x0b\x6b\x59\x86\x15\xb7\x43\x9c\xcc" //.....@!.kY...C.. |
// /* 03b0 */ "\x3f\x10\x52\x4a\xe3\x0c\xa7\xbd\x96\x31\xfe\x2c\xa0\x42\x00\x84" //?.RJ.....1.,.B.. |
// /* 03c0 */ "\x32\x63\x14\x94\xab\x5a\xd7\xe2\x8a\x9e\x14\x73\x7e\x28\x21\x84" //2c...Z.....s~(!. |
// /* 03d0 */ "\x42\x94\xac\xe7\x28\xa5\x3c\x1a\xdf\x88\x48\x43\x47\x39\x7d\xc1" //B...(.<...HCG9}. |
// /* 03e0 */ "\x49\xec\x2c\x6f\x1d\xd2\xab\x07\x74\x51\x50\x00\x18\xf7\x18\xdc" //I.,o....tQP..... |
// /* 03f0 */ "\x47\x00\x4a\xc0\x30\x6c\xea\x65\x14\x8b\x4d\x63\x55\x6a\x14\x7a" //G.J.0l.e..McUj.z |
// /* 0400 */ "\xef\x00\x13\x8d\x55\xa2\xd3\x59\x94\x52\x5c\xe7\xd5\xf8\x73\xc4" //....U..Y.R....s. |
// /* 0410 */ "\x80\x00\x08\x43\x67\x39\xb5\xad\x53\xde\xea\x71\xc9\xb4\xd6\x75" //...Cg9..S..q...u |
// /* 0420 */ "\x85\x52\x8f\xb6\xf7\x9a\x76\x14\x43\x88\x73\x9c\xa9\x4b\xde\xf1" //.R....v.C.s..K.. |
// /* 0430 */ "\x3a\xab\x5e\xf0\x95\xa0\xa1\x08\x80\x02\x14\xa5\x73\x8e\x2e\xdb" //:.^.........s... |
// /* 0440 */ "\xd6\x69\xd6\xf7\xb8\x31\x8d\x0c\x60\xb1\x8e\x2d\x35\xc3\x4a\x29" //.i...1..`..-5.J) |
// /* 0450 */ "\xaa\x6b\x5d\x8c\x61\x94\xa6\x9e\xf6\x71\x85\x8f\x02\x92\x59\x52" //.k].a....q....YR |
// /* 0460 */ "\x92\x08\x40\x42\x13\x8c\x32\x85\x5b\xaf\x41\x5c\x61\x81\x08\x48" //..@B..2.[.A.a..H |
// /* 0470 */ "\x21\x12\xa5\x21\x49\x2d\xc0\x00\x3f\x80\x62\xa7\x1c\x34\xf4\x3a" //!..!I-..?.b..4.: |
// /* 0480 */ "\x84\xda\xd7\x15\x51\x86\x4b\x9c\xc3\x18\xda\x52\xa8\xe7\x24\x84" //....Q.K....R..$. |
// /* 0490 */ "\x3c\x35\x40\x12\x2c\x23\xf0\xb3\x9c\xc2\x94\x8b\x18\xe9\xef\x6e" //<5@.,#.........n |
// /* 04a0 */ "\x38\x56\xcd\x53\x8e\x18\x7b\x6d\x6b\x5a\x49\x4a\x21\x84\xf0\x09" //8V.S..{mkZIJ!... |
// /* 04b0 */ "\x87\x18\x64\x4a\x29\x6b\x5a\x65\x29\x20\x00\x93\x18\xe0\xd4\xe4" //..dJ)kZe) ...... |
// /* 04c0 */ "\xa9\x4a\x25\x0a\x52\x04\xff\x40\x84\x29\x73\x99\xa4\x96\x5c\xe6" //.J%.R..@.)s..... |
// /* 04d0 */ "\x50\x85\x02\x10\xb1\x8c\x42\x94\xb0\x6e\xf4\x26\xb5\xd1\xce\x48" //P.....B..n.&...H |
// /* 04e0 */ "\x30\xa3\xf2\xd6\xb4\x45\x06\xb5\xad\x22\xff\x45\x1c\xe5\x35\xae" //0....E...".E..5. |
// /* 04f0 */ "\x96\xb5\x80\xe0\x5c\x3c\x4c\x1e\xe6\x7b\xca\xf0\x95\x74\x10\x7b" //.....<L..{...t.{ |
// /* 0500 */ "\xc6\x0a\xc0\x00\xcd\xbd\xe3\x89\xa0\x09\x60\x04\x1d\xf6\xde\x71" //..........`....q |
// /* 0510 */ "\x46\xc1\x6c\x82\x9b\x18\xc2\x6b\x5e\x12\xe4\x40\x01\x22\xd3\x52" //F.l....k^..@.".R |
// /* 0520 */ "\xc6\x3a\x1f\x09\x65\xbd\xed\x67\x85\xfa\xe7\x1c\x85\x60\xeb\xc1" //.:..e..g.....`.. |
// /* 0530 */ "\x42\x14\x4a\x29\x7b\xaf\x4e\xb0\xa3\x52\xce\x72\xc2\x11\x4b\x5a" //B.J){.N..R.r..KZ |
// /* 0540 */ "\x34\xc3\x07\x21\x8a\x36\x10\x28\x8b\x5a\xd1\xc5\x1a\x35\x56\x6d" //4..!.6.(.Z...5Vm |
// /* 0550 */ "\xc0\xea\x00\xc6\x3f\x95\x1c\x7c\x9f\x28\xb3\x4e\x79\xa7\x54\x00" //....?..|.(.Ny.T. |
// /* 0560 */ "\x8b\x18\xc1\x70\xb1\x59\x66\x9c\x80\x02\x03\x0d\x4d\x6b\xf9\x50" //...p.Yf.....Mk.P |
// /* 0570 */ "\x19\x83\xf0\xc9\xad\x71\xaa\xb1\xc5\x1a\xd6\xb4\x40\x00\xda\xd7" //.....q......@... |
// /* 0580 */ "\x3a\xab\x0c\x51\xae\x16\xe0\x5e\x46\xa6\x2b\x1e\x00\x15\x61\x1d" //:..Q...^F.+...a. |
// /* 0590 */ "\xb4\x37\x86\x7c\x58\xc6\x38\xdf\x38\x5b\xbe\x56\x4d\x29\x58\x3c" //.7.|X.8.8[.VM)X< |
// /* 05a0 */ "\x08\x1d\x81\xf9\x5c\xbc\x98\x1c\xbe\xa8\x01\x11\x2a\x9f\x17\x73" //............*..s |
// /* 05b0 */ "\x4a\x55\x8c\x62\x44\x20\x51\xed\xd4\x5a\x6a\x69\xc6\xa5\x63\x18" //JU.bD Q..Zji..c. |
// /* 05c0 */ "\x04\x21\x85\x29\x71\x08\x82\xc6\x39\x31\x89\x00\x01\x0c\x6e\xd6" //.!.)q...91....n. |
// /* 05d0 */ "\xb5\x12\x8a\x38\xc3\x29\x7e\xdf\x20\x84\x51\xce\x5b\x5a\xee\x71" //...8.)~. .Q.[Z.q |
// /* 05e0 */ "\xcb\x7b\xdf\xf6\x80\xc6\x37\x18\x64\xca\x28\x9a\xd7\x82\xfb\x9f" //.{....7.d.(..... |
// /* 05f0 */ "\x29\x12\x8a\x16\x31\xd1\xce\x4b\x18\xe2\x5c\x3f\x9f\xf2\x04\x99" //)...1..K...?.... |
// /* 0600 */ "\xfb\x6a\xb8\x05\x68\x43\x82\x39\x8a\xb2\xde\x6b\xf0\x45\xc0\x26" //.j..hC.9...k.E.& |
// /* 0610 */ "\x0b\x00\x00\xcd\x38\x04\xe2\x64\x02\x5a\x01\x00\x11\xb5\xad\x20" //....8..d.Z.....  |
// /* 0620 */ "\x84\x43\x9c\xfd\x27\x14\xb5\xa6\x1f\x94\xe4\x35\x12\xa5\x26\x10" //.C..'......5..&. |
// /* 0630 */ "\x89\x73\x9d\xee\xbd\x57\xf9\x4c\x1a\xf7\x1f\x2a\x21\x4a\x58\x36" //.s...W.L...*!JX6 |
// /* 0640 */ "\xaa\x07\x2c\xf3\x69\xaf\xca\x8b\x7b\xd9\xe5\x9e\x2d\x35\x69\x4a" //..,.i...{...-5iJ |
// /* 0650 */ "\xa1\x8c\x08\x41\x3e\x54\x24\xf9\x6c\x21\x85\x7e\x0e\xb2\xd4\x94" //...A>T$.l!.~.... |
// /* 0660 */ "\xa8\x31\x8b\xcd\x3b\x01\xcf\x80\x2e\x78\x62\x29\x06\x31\x4d\x6b" //.1..;....xb).1Mk |
// /* 0670 */ "\x21\x81\xfa\x56\x38\x0d\xc5\x4e\x38\x0b\x07\xfa\x90\x59\x84\xbf" //!..V8..N8....Y.. |
// /* 0680 */ "\x45\x8c\x7f\xa2\x08\x6e\x14\x00\x58\x95\x6e\xbd\x47\x39\xf7\xbe" //E....n..X.n.G9.. |
// /* 0690 */ "\x9f\x2b\x8b\x8e\x38\x3b\xd1\x40\x84\x2e\x01\x2e\x01\xeb\x80\x00" //.+..8;.@........ |
// /* 06a0 */ "\x4b\x14\x5a\x6b\x00\x02\x51\xe3\x00\x6d\x6b\xa9\xe1\x71\x85\xad" //K.Zk..Q..mk..q.. |
// /* 06b0 */ "\x6b\xe5\x51\x84\x12\x97\xf2\x90\xe7\x3b\x9f\xca\x92\x8f\x95\x8a" //k.Q......;...... |
// /* 06c0 */ "\x29\x4a\x8c\x30\x31\x8c\x03\x07\x10\x0c\x20\x80\x84\x25\x08\x46" //)J.01..... ..%.F |
// /* 06d0 */ "\x92\x50\x84\x20\x42\x13\x08\x23\x6f\x70\x05\x2d\x68\x17\x06\xcf" //.P. B..#op.-h... |
// /* 06e0 */ "\x30\x0f\xdb\x04\xfa\x3e\xa4\x61\x01\x26\x06\x5e\x2a\xcb\x70\xe8" //0....>.a.&.^*.p. |
// /* 06f0 */ "\x6a\xbd\xc0\x12\x38\x05\xb8\x08\xc5\x2d\x6b\x80\xeb\x8a\x85\x16" //j...8....-k..... |
// /* 0700 */ "\xf2\x81\xd8\xb8\x00\x07\x83\xd1\xe6\x9d\x26\xf4\xc8\x86\x12\x9a" //..........&..... |
// /* 0710 */ "\xd7\x06\x31\xc2\xca\x50\x01\x80\x5c\x9f\x52\x03\x18\xc8\x60\x18" //..1..P....R...`. |
// /* 0720 */ "\x01\x00\xca\x53\x85\x70\xe0\x83\xea\xa3\x55\x6a\x3f\x07\xd5\x7e" //...S.p....Uj?..~ |
// /* 0730 */ "\xff\xf5\x23\x0a\xf1\xc1\x00\x04\x4b\xea\x4d\xad\x6a\xbf\x7f\x6d" //..#.....K.M.j..m |
// /* 0740 */ "\xfa\x92\x29\x25\xfa\x91\x3a\xfa\x93\x3e\xbb\xb6\xb5\xa2\x94\xf3" //..)%..:..>...... |
// /* 0750 */ "\xd0\x84\x10\x8f\xa9\x1c\xb3\x22\x7d\x77\xf4\xc4\x00\x00\x46\xaa" //......."}w....F. |
// /* 0760 */ "\xdd\x5d\x4d\x78\x4b\x16\x2e\x00\x21\xe4\x30\x7f\x4d\xc0\x47\xa6" //.]MxK...!.0.M.G. |
// /* 0770 */ "\xc5\x1e\x87\x9e\x30\x08\x00\xa8\x63\x1a\x8c\x33\xea\xe1\xc1\x8a" //....0...c..3.... |
// /* 0780 */ "\x3a\x5d\x38\xd4\x94\x61\x84\x9f\x51\x81\xb6\x22\xd6\xb4\x21\x84" //:]8..a..Q.."..!. |
// /* 0790 */ "\xaf\xe7\xea\x55\x9c\xe4\x53\xf4\x5f\xd4\x8b\x5a\xd7\xd4\xc3\x84" //...U..S._..Z.... |
// /* 07a0 */ "\xf5\x7d\x4a\x71\x86\x72\xca\x8f\xa9\x1e\x0c\x85\x18\x62\x4a\x56" //.}Jq.r.......bJV |
// /* 07b0 */ "\x17\x6d\x4b\x49\x08\x79\x36\xb0\xd3\x6b\xde\x9c\x3e\xf1\xa2\xe0" //.mKI.y6..k..>... |
// /* 07c0 */ "\x00\x3b\x0f\x38\x97\xbc\x6a\xbe\x00\x5e\x01\x19\xad\xd7\xa9\x6b" //.;.8..j..^.....k |
// /* 07d0 */ "\x52\xe7\x3a\xbc\xf6\x05\x81\xec\x90\xa5\x2b\x7b\xdc\x5a\x6b\x43" //R.:.......+{.ZkC |
// /* 07e0 */ "\x18\x5d\xb7\xf8\x36\x15\x65\xa0\x84\x31\xeb\xbf\x36\x38\x14\x92" //.]..6.e..1..68.. |
// /* 07f0 */ "\xa2\x94\xd7\xef\xf4\x73\x99\x94\x52\xa7\x1c\x83\x18\x90\xc6\x19" //.....s..R....... |
// /* 0800 */ "\x4a\x6e\x78\x63\x39\x67\x39\x9f\x5d\xc9\x08\x4d\x30\xc9\x88\xa1" //Jnxc9g9.]..M0... |
// /* 0810 */ "\x09\xf6\xdf\x63\x18\x75\x96\xf2\x10\x83\xcb\x3c\x88\x42\x6d\x35" //...c.u.....<.Bm5 |
// /* 0820 */ "\xa7\xbd\xca\x94\xaf\x75\xe6\x10\x8b\xbc\xf6\x1c\xe7\x02\x10\xc2" //.....u.......... |
// /* 0830 */ "\x2c\x80\xf8\xbb\x8c\x32\x8e\x73\x84\x1b\x1d\x9f\x11\x28\xa4\x9b" //,....2.s.....(.. |
// /* 0840 */ "\xa8\x32\x5d\xf6\x55\x38\xe3\x29\x4f\x4b\xd8\x91\x08\x5c\xe3\x9c" //.2].U8.)OK...... |
// /* 0850 */ "\xd2\x23\x9f\x14\x0a\x49\x49\x08\x78\x35\xa3\x40\x9e\x1b\x58\xf9" //.#...II.x5.@..X. |
// /* 0860 */ "\xfe\x03\x18\xc9\x08\x45\x30\x9c\x1b\x91\x2c\x26\xf1\x1a\x48\x84" //.....E0...,&..H. |
// /* 0870 */ "\x3a\x18\x44\x1b\x9c\xf1\x84\x22\x9e\xf7\x0a\x52\x93\x18\xc4\xa2" //:.D...."...R.... |
// /* 0880 */ "\x88\x21\x16\xf7\xb4\x94\xa6\x10\x86\x00\xef\x5e\xe5\xd2\x9a\x30" //.!.........^...0 |
// /* 0890 */ "\x79\x0c\x97\x80\x01\xc9\xb8\x72\xe3\x24\x01\x30\x00\x09\xee\x36" //y......r.$.0...6 |
// /* 08a0 */ "\xee\xd5\x38\x07\xd7\x40\x9c\x22\x3a\xc2\x07\xc1\xec\x14\xf8\xed" //..8..@.":....... |
// /* 08b0 */ "\xff\x3d\x5a\xc3\xff\x30\xdb\xda\x0e\x3b\x19\x4f\x43\xe7\xd1\x8e" //.=Z..0...;.OC... |
// /* 08c0 */ "\x05\xac\x03\xe7\x50\x26\x1f\xe1\x87\x19\x07\xd3\xe5\x93\x76\x64" //....P&........vd |
// /* 08d0 */ "\x7d\x3c\xbf\xcc\xf6\xa7\x9e\x40\xaf\x05\x46\x01\xa8\x8e\xed\x7b" //}<.....@..F....{ |
// /* 08e0 */ "\x7d\x68\xe7\x84\xee\x21\x61\x08\x58\x08\xf4\x00\x0f\xe5\x86\x13" //}h...!a.X....... |
// /* 08f0 */ "\x74\x9e\x30\xf1\xdd\xb2\x70\x52\x44\x94\x21\x77\x6a\x9c\x0c\x94" //t.0...pRD.!wj... |
// /* 0900 */ "\x0a\xe1\x46\xfd\xcf\xff\x7e\x70\x8f\x3d\x3c\x27\x5d\x6f\x9f\x52" //..F...~p.=<']o.R |
// /* 0910 */ "\x3c\x53\x3d\xc0\x27\xc3\x13\xdd\xa3\x0f\xb7\x1e\x5c\x0f\xf7\x67" //<S=.'..........g |
// /* 0920 */ "\xe8\xfc\x11\x2e\xee\x77\x1e\x7a\x9f\xc1\x3e\x7f\x9f\x0d\xcf\xb7" //.....w.z..>..... |
// /* 0930 */ "\xe3\xda\x78\x78\x16\x7b\xb2\xf6\x7e\xf8\xee\xd1\x03\x7e\x17\x77" //..xx.{..~....~.w |
// /* 0940 */ "\x5d\x8e\x30\xaf\xae\xc0\x50\x61\x9f\x8e\xee\xbf\x14\xb5\xa1\x0f" //].0...Pa........ |
// /* 0950 */ "\x36\xcc\x07\xcd\xa9\x52\x97\x76\xa0\xc7\x07\x98\x30\x00\x03\x4f" //6....R.v....0..O |
// /* 0960 */ "\xe2\xdb\x8a\x68\x01\x8e\x01\xed\xe7\x0e\x06\xc0\x3a\xf8\x7b\x75" //...h........:.{u |
// /* 0970 */ "\x82\x75\x18\x06\x3e\x30\x0c\xb3\x00\xc9\x30\x0c\x7c\x60\x19\x66" //.u..>0....0.|`.f |
// /* 0980 */ "\x09\xe6\xc7\x04\xbc\x30\xc1\x2e\x5e\x22\x83\x10\x00\x3e\xa6\x2f" //.....0..^"...>./ |
// /* 0990 */ "\x11\x5b\x48\x01\x97\x4e\x46\x01\xf4\xe5\x04\xc1\x40\x8c\x20\x4d" //.[H..NF.....@. M |
// /* 09a0 */ "\xe1\x0f\x04\x7d\x21\x00\x0e\x02\x01\x9c\x30\x1b\xdd\x4e\x01\xc0" //...}!.....0..N.. |
// /* 09b0 */ "\x41\xf3\x50\x01\x32\x00\x03\x07\xe2\xb9\x88\x28\x09\x9e\x02\x0f" //A.P.2......(.... |
// /* 09c0 */ "\xa8\x7d\x0e\x80\x27\x41\x91\xc0\x4a\xe8\xbe\x43\x88\x96\x17\xef" //.}..'A..J..C.... |
// /* 09d0 */ "\x91\x50\x48\x0c\x43\xeb\x15\x11\x6a\x00\x46\x45\x58\x13\xfe\x30" //.PH.C...j.FEX..0 |
// /* 09e0 */ "\x8c\xd8\x57\x01\x1c\x30\x8d\xd4\x60\xf6\xcc\x3c\x61\x18\x60\x2a" //..W..0..`..<a.`* |
// /* 09f0 */ "\x64\x42\x18\x06\x2e\x79\xd6\xba\x0a\xc0\x0f\x80\xe3\xcb\xe9\xfe" //dB...y.......... |
// /* 0a00 */ "\x14\x97\x8e\x04\x8e\x18\xa1\xec\x1d\x6a\xc0\x37\xe1\xee\xb9\x8e" //.........j.7.... |
// /* 0a10 */ "\x3c\xa1\xdf\x9d\x7c\x1d\x4c\x5f\x3c\x39\x83\xe2\xaf\x02\x33\xb9" //<...|.L_<9....3. |
// /* 0a20 */ "\x8c\xb0\x69\x23\x07\xd3\xd6\x01\xe4\x61\x5e\x77\x8b\x20\x83\xcf" //..i#.....a^w. .. |
// /* 0a30 */ "\x2f\x0b\xc1\x21\x08\xe6\x0f\xf1\x98\xb4\xd7\x04\x72\x86\x30\x17" ///..!........r.0. |
// /* 0a40 */ "\x4c\x08\x32\x94\xe1\xaf\xc5\x9c\xe7\x05\xdf\xc6\x0b\xff\x9c\x38" //L.2............8 |
// /* 0a50 */ "\xb8\xf0\x18\x67\x8e\xf4\x0f\x70\xe3\x74\x79\x81\x8f\x9b\x38\x0a" //...g...p.ty...8. |
// /* 0a60 */ "\x15\xe3\x39\x85\x29\x34\x7d\x40\xbf\x0e\x17\xcd\x74\xc5\x60\x0b" //..9.)4}@....t.`. |
// /* 0a70 */ "\x00\x0f\x27\x69\x33\x00\x03\xb6\xfb\x6b\x1b\x58\x09\xa6\x21\x25" //..'i3....k.X..!% |
// /* 0a80 */ "\xa3\x8a\x35\x03\x84\x63\xcc\x51\x87\xb2\x43\xb8\x58\x38\x79\x67" //..5..c.Q..C.X8yg |
// /* 0a90 */ "\x60\xc4\x54\x41\xbc\x0c\x62\xe0\x6d\x03\x5c\x08\x4c\x00\xbc\x46" //`.TA..b.m...L..F |
// /* 0aa0 */ "\x21\x66\x9c\xc0\x0d\xed\xa6\x1d\x8c\x60\x97\x45\x4e\x39\x86\x46" //!f.......`.EN9.F |
// /* 0ab0 */ "\x60\x5b\xf7\xb0\xac\x24\x70\xa3\x30\x04\xe1\x92\x81\x0f\xbf\x20" //`[...$p.0......  |
// /* 0ac0 */ "\x52\x05\x53\x8e\x72\x1f\x78\x2e\x1f\x6d\xec\x01\x77\xfb\xf8\x9f" //R.S.r.x..m..w... |
// /* 0ad0 */ "\x80\xe0\x32\xff\x7f\x12\xf4\xbb\x0a\xf3\x40\x8e\x1f\x44\x7e\x1c" //..2.......@..D~. |
// /* 0ae0 */ "\xb0\xbc\x22\x80\x01\xe9\x87\xcb\x0f\x7a\xe9\x77\x9e\xb0\x72\xee" //.."......z.w..r. |
// /* 0af0 */ "\x60\x28\xf5\x3c\xd2\x4b\x4e\x91\x61\xd1\xb8\x41\x08\xc3\x63\xf1" //`(.<.KN.a..A..c. |
// /* 0b00 */ "\xc0\x29\x40\x0c\x4d\x6b\xe0\x30\x91\x08\x30\x62\x74\x90\xef\x1c" //.)@.Mk.0..0bt... |
// /* 0b10 */ "\xb3\x00\x17\x01\xb7\xac\x0c\x18\x59\x8c\x37\x86\x44\x2f\x80\x19" //........Y.7.D/.. |
// /* 0b20 */ "\x83\x0f\xe7\x06\xb9\x63\x98\x30\xf6\xa0\x67\x0c\x9e\x5f\x26\x33" //.....c.0..g.._&3 |
// /* 0b30 */ "\x00\x37\xba\xa2\xd6\xb4\xc0\x0f\xed\xe7\x82\x1c\x8a\xc3\xdc\xc4" //.7.............. |
// /* 0b40 */ "\x4c\x1d\x91\x02\xdc\x94\x4c\x18\x3d\xf5\x81\x09\x23\x94\x3b\x80" //L.....L.=...#.;. |
// /* 0b50 */ "\x24\x30\x05\x00\x61\xcc\xb7\x41\xc8\xf9\xeb\x39\xcb\x00\x58\x06" //$0..a..A...9..X. |
// /* 0b60 */ "\x16\x0c\x60\x01\x68\xf9\xee\xe8\x00\xcc\x08\x1f\x1e\x0c\x61\x95" //..`.h.........a. |
// /* 0b70 */ "\x70\x8f\xf5\x72\x2a\x0f\x72\x66\x73\xed\x81\x0f\x9e\xfe\xb2\x50" //p..r*.rfs......P |
// /* 0b80 */ "\x84\xc0\x1f\xf3\xdd\xa5\x00\x03\xf8\x58\x7e\x2e\x14\x0c\x12\x6b" //.........X~....k |
// /* 0b90 */ "\x1c\x3b\x9c\xb3\x87\x38\x56\x21\x80\xb6\xe0\x86\x00\x22\xa3\x80" //.;...8V!.....".. |
// /* 0ba0 */ "\xb7\x2e\xff\x7f\xc0\x5b\x7f\x30\x43\x70\x40\xc1\xb9\x32\x68\x00" //.....[.0Cp@..2h. |
// /* 0bb0 */ "\x07\x41\xe4\xb7\x8c\xfc\x00\xd7\x0c\x18\xab\xfc\x15\x55\xba\xf1" //.A...........U.. |
// /* 0bc0 */ "\x14\x04\x42\x08\x2f\x70\x12\x04\x80\xcf\xae\xe2\x58\x5b\xe1\x42" //..B./p......X[.B |
// /* 0bd0 */ "\x65\x29\xe7\x02\x17\xa0\x01\x9a\x49\x58\x30\xe4\x4a\x05\xd9\x5e" //e)......IX0.J..^ |
// /* 0be0 */ "\x28\x01\x18\x23\x28\x0d\xd5\xa8\xce\x58\xc7\x70\x80\x04\x60\x29" //(..#(....X.p..`) |
// /* 0bf0 */ "\x6c\x86\x28\xd8\x0a\x5b\xea\x6e\xf3\xdc\x05\x2d\xa7\xa5\x28\xd4" //l.(..[.n...-..(. |
// /* 0c00 */ "\x00\x0e\x2b\xe3\x17\xa5\x2a\x26\x00\x6d\xde\x19\x70\x40\x04\xa0" //..+...*&.m..p@.. |
// /* 0c10 */ "\x18\x26\xb5\xe1\x88\x21\x63\x1c\x63\x01\x06\xed\x7f\xc2\xbe\x8c" //.&...!c.c....... |
// /* 0c20 */ "\x04\x19\xdc\x43\x00\x07\xc0\x41\xbe\x37\x48\x27\x54\x06\x02\x03" //...C...A.7H'T... |
// /* 0c30 */ "\x8f\x95\x18\x08\x37\xaf\xcd\x14\x6c\x00\x07\x13\xf3\x46\x31\x10" //....7...l....F1. |
// /* 0c40 */ "\x03\x7f\x34\x60\xe3\x0c\xf3\x42\xa7\x1c\x8a\xe0\x16\xbf\x28\x87" //..4`...B......(. |
// /* 0c50 */ "\x0d\x73\x30\x0b\x5d\x60\x2d\xb6\x01\x6c\xf3\x30\x0a\xc4\x60\x16" //.s0.]`-..l.0..`. |
// /* 0c60 */ "\xc8\xc0\x0f\xf1\x88\xf9\x23\x00\xb6\x69\x70\x41\xf3\x34\x9b\x80" //......#..ipA.4.. |
// /* 0c70 */ "\x01\xcc\xbc\xcd\x8c\xc0\x03\x09\x66\x83\x79\x4d\x8c\xbc\xd4\xa3" //........f.yM.... |
// /* 0c80 */ "\xb8\xc3\x30\xee\x3a\x80\x2a\xeb\x2d\x01\xa0\x61\xb2\x15\x1f\x07" //..0.:.*.-..a.... |
// /* 0c90 */ "\xdb\x86\x28\xcc\x03\x1f\xe8\x8e\x11\x82\x5f\x00\x18\x0b\xb1\x53" //..(......._....S |
// /* 0ca0 */ "\x8e\x00\xc0\x07\xb0\x06\xb4\x78\x5e\x30\x07\x57\x17\xd4\x8c\x01" //.......x^0.W.... |
// /* 0cb0 */ "\xd3\xf3\xfc\x0a\xe8\xe0\x00\x0e\x6f\xce\x9f\x19\xa8\x09\xc8\x01" //........o....... |
// /* 0cc0 */ "\x00\x17\x3a\x58\xe7\x58\xc3\x00\x09\xf5\xdf\x09\x4e\xc6\x1c\x71" //..:X.X......N..q |
// /* 0cd0 */ "\x1e\x59\xf0\x87\xd4\xf2\x36\x8e\xc6\x4b\xfd\xff\x05\x26\xbe\x3c" //.Y....6..K...&.< |
// /* 0ce0 */ "\x2c\xe0\xa5\x97\x3a\x28\xf9\x91\xe7\xe0\xfe\x18\x3b\x7e\x8b\xed" //,...:(......;~.. |
// /* 0cf0 */ "\xc8\x1f\xc3\x94\x17\x5f\xfb\x71\xe0\x58\xfb\xb8\xff\xeb\x8b\x60" //....._.q.X.....` |
// /* 0d00 */ "\xeb\xa7\xde\x70\x36\x7d\xed\x5f\x6d\xfc\xe8\xa2\x9e\x09\x87\x01" //...p6}._m....... |
// /* 0d10 */ "\xe1\x80\x42\xff\x7f\xa7\xf3\x3b\x01\x10\xf3\x9e\x72\xfc\x2e\xfe" //..B....;....r... |
// /* 0d20 */ "\x55\x37\x88\xb8\x47\xf4\x3c\x0d\xb3\x8f\x9a\x5f\x39\x83\x18\x1f" //U7..G.<...._9... |
// /* 0d30 */ "\x56\x1d\x65\xbf\x1d\xe7\xea\xf9\xcf\xd5\x78\x3e\x26\x31\x10\xfb" //V.e.......x>&1.. |
// /* 0d40 */ "\xc4\xf7\xe7\xc8\xf8\xef\x2e\x07\xfb\x28\x3e\x64\xff\x69\xfc\x38" //.........(>d.i.8 |
// /* 0d50 */ "\x8c\x60\xfc\x27\xc7\x60\xfb\xa8\xe1\x2e\xf9\x93\xe0\x28\xed\x44" //.`.'.`.......(.D |
// /* 0d60 */ "\xf9\x93\xe3\x89\xfd\xb8\xe0\xab\xc0\xff\x2d\x17\xc5\x60\x49\xc0" //..........-..`I. |
// /* 0d70 */ "\x78\x17\xcb\x81\x08\xb6\x01\x9b\xf3\x08\x63\x78\x06\x6e\x2f\xf7" //x.........cx.n/. |
// /* 0d80 */ "\xfc\x03\x37\xe8\x61\x22\xf8\xe1\x15\x47\x20\x00\x77\x6e\x29\xf8" //..7.a"...G .wn). |
// /* 0d90 */ "\xdd\x00\x0e\xb0\xcc\x46\x65\x74\x5e\xd2\x13\x5a\x89\xe0\x12\x98" //.....Fet^..Z.... |
// /* 0da0 */ "\xee\x1c\xec\xf8\xc3\x2a\xf4\x33\x6f\xed\x27\x10\x1a\xec\x2c\x57" //.....*.3o.'...,W |
// /* 0db0 */ "\xaa\x64\x60\x14\xa0\xaf\x67\xc5\x40\x0b\x07\x6f\x3b\x13\xbb\x49" //.d`...g.@..o;..I |
// /* 0dc0 */ "\xc7\xc7\x0c\x76\x7c\x60\xc3\x6a\xec\x29\x18\x30\x70\xab\x2d\xb8" //...v|`.j.).0p.-. |
// /* 0dd0 */ "\x46\x01\x60\x3e\xb9\x0f\xe0\xbb\x38\xe0\x51\x1f\x31\xec\x58\xbb" //F.`>....8.Q.1.X. |
// /* 0de0 */ "\xfc\xea\xe0\x6d\x7c\x00\x27\x85\xed\x7c\xe7\x02\xf8\xf6\xb4\x0f" //...m|.'..|...... |
// /* 0df0 */ "\xf1\x9b\x61\x0e\x07\x68\x58\xf8\xa1\x8a\x2f\x8d\x9d\x6a\x1d\x7e" //..a..hX.../..j.~ |
// /* 0e00 */ "\xc3\x7c\xfe\x9d\xa0\xab\xb4\x48\x77\x00\x4b\x14\x61\x9d\xa0\xa5" //.|.....Hw.K.a... |
// /* 0e10 */ "\x56\x5a\x3d\xd9\xc6\x30\x05\x44\x60\x0a\x39\xf9\x93\x80\x7a\xf1" //VZ=..0.D`.9...z. |
// /* 0e20 */ "\xc0\x1a\xd3\xd1\xed\xf7\xe3\x00\x9d\x85\xbd\x93\x07\x58\x00\x57" //.............X.W |
// /* 0e30 */ "\x63\xbc\x00\xef\xd1\x82\xe6\xde\xc8\x61\x36\x5f\x00\x78\x23\x10" //c........a6_.x#. |
// /* 0e40 */ "\x63\x3e\x83\xee\xde\x1a\x2c\xeb\xa0\x61\x5a\xa9\x3b\x00\x03\xc0" //c>....,..aZ.;... |
// /* 0e50 */ "\xab\x8f\x1f\x5b\x68\x01\xe6\x00\x93\xe1\xbb\xcf\x69\x62\x10\x4b" //...[h.......ib.K |
// /* 0e60 */ "\x80\x00\x16\x0c\x10\x60\xc1\x89\xc1\x84\x31\x88\xcf\x63\x15\x2a" //.....`....1..c.* |
// /* 0e70 */ "\xd5\x6f\x05\xe2\x1b\x5a\xc0\xa0\x35\x8b\x2d\x2d\xb8\xd0\x43\xfb" //.o...Z..5.--..C. |
// /* 0e80 */ "\xce\x23\x7f\x8c\x01\x90\x5e\x71\xb8\x54\x01\xf8\x9f\x30\x08\xfc" //.#....^q.T...0.. |
// /* 0e90 */ "\x60\x0a\x39\xc1\x7c\x8c\x22\x58\xc3\x49\xda\x53\xb1\x0e\x65\x08" //`.9.|."X.I.S..e. |
// /* 0ea0 */ "\x01\x1e\x00\x01\xe0\x93\x07\x91\x6d\xec\x00\xf7\x00\x4b\xb8\x66" //........m....K.f |
// /* 0eb0 */ "\xe3\xc2\x1a\x3e\x6b\x01\x9e\x56\x00\x5a\x79\x36\x68\x18\x8c\x85" //...>k..V.Zy6h... |
// /* 0ec0 */ "\x88\xf9\x60\x83\x07\xc5\x30\x76\xfb\x15\x1f\x46\x0e\xff\x80\x56" //..`...0v...F...V |
// /* 0ed0 */ "\xe3\x48\x09\x8d\x32\x80\x27\x19\x4b\xa8\x01\x28\x19\x62\xa9\xb9" //.H..2.'.K..(.b.. |
// /* 0ee0 */ "\x6d\x6b\xa0\x02\xaf\x38\xe0\x0e\x39\x93\x8b\xd4\x00\xa0\x98\x02" //mk...8..9....... |
// /* 0ef0 */ "\xc9\xe0\x56\x14\x33\x5f\x01\xfc\x00\xc3\x71\xe0\x01\xcc\x43\xb3" //..V.3_....q...C. |
// /* 0f00 */ "\x5f\x59\xe5\x35\x00\x0f\xda\xb0\x7a\x00\x06\xab\xcc\x9b\x15\x30" //_Y.5....z......0 |
// /* 0f10 */ "\x13\xec\x00\x1b\x58\x9a\x85\xe2\x47\x93\x5e\x41\x8c\x54\x89\xaf" //....X...G.^A.T.. |
// /* 0f20 */ "\x8f\x20\x37\x00\x27\x00\xb0\x7b\xda\x80\x21\xe3\x36\x2a\x90\xc3" //. 7.'..{..!.6*.. |
// /* 0f30 */ "\x12\x37\xcd\x88\x13\x0d\x2f\xc0\xec\x0d\x3a\x18\x07\x67\xc3\x5d" //.7..../...:..g.] |
// /* 0f40 */ "\x41\xc8\x11\x5c\x96\x41\x77\x62\xf9\x41\xb8\x0d\x9e\x7a\xc9\xf2" //A....Awb.A...z.. |
// /* 0f50 */ "\x03\x30\x20\xf8\x25\x0e\xbc\xcb\xf2\x83\x10\x26\xd8\x63\x0d\xbd" //.0 .%......&.c.. |
// /* 0f60 */ "\x9b\xe5\x05\xe0\x41\x5f\x98\xcf\xf2\x82\xd0\x1b\x34\xf5\xa7\xe4" //....A_......4... |
// /* 0f70 */ "\x05\x3f\x32\x47\x56\x6a\xf9\x41\x38\x12\x74\x30\x86\xce\xd7\xf2" //.?2GVj.A8.t0.... |
// /* 0f80 */ "\x82\x50\x29\x6c\x52\x01\x77\x6c\xf9\x41\x18\x11\x54\x96\x81\x7f" //.P)lR.wl.A..T... |
// /* 0f90 */ "\x6d\xb0\x79\x25\x88\x35\x2e\x80\x01\x80\xa6\x12\xca\xb3\x7d\x82" //m.y%.5........}. |
// /* 0fa0 */ "\xfe\xc0\x1f\x87\x60\xd5\xdb\xfc\x3e\xef\x4f\x6e\x08\xc4\x21\x55" //....`...>.On..!U |
// /* 0fb0 */ "\x85\x70\x00\x41\x08\x40\x34\xdb\x7e\x17\x4b\x9c\x2f\x77\x0e\x7e" //.p.A.@4.~.K./w.~ |
// /* 0fc0 */ "\xc5\x05\xfa\x8c\xe9\xf2\x18\x75\xfa\x7e\x43\x06\x57\xc6\x0c\xc1" //.......u.~C.W... |
// /* 0fd0 */ "\x00\x03\xe0\x08\xd5\xee\xbd\x85\xe2\x60\x1b\x00\x85\x17\x6d\xf8" //.........`....m. |
// /* 0fe0 */ "\x10\xa2\x01\x7e\xa1\x76\xde\xf3\xf2\xa3\x00\x86\x9e\xbe\x40\x13" //...~.v........@. |
// /* 0ff0 */ "\xf7\x1e\xfe\x40\x0f\xff\x1f\x3e\x43\x0e\xef\x43\xef\xc8\x00\xf8" //...@...>C..C.... |
// /* 1000 */ "\x78\xfd\xf2\x18\x42\xda\x1f\xfe\x44\x30\x00\x7c\x66\x06\x1f\x93" //x...B...D0.|f... |
// /* 1010 */ "\xe0\x00\x3e\x04\x8e\x05\x16\x00\x04\x7f\x80\xed\x18\x84\xec\x43" //..>............C |
// /* 1020 */ "\xc3\xda\x78\xdd\x97\x6d\xfc\xf3\x60\xdf\x0d\xa6\xc0\xe4\x10\x3e" //..x..m..`......> |
// /* 1030 */ "\x08\x53\x25\x81\xd5\xb0\xe1\x00\x67\xe5\x9a\x25\xb4\x2a\xf4\x82" //.S%.....g..%.*.. |
// /* 1040 */ "\xee\xc7\xeb\x80\xbc\x0c\x2a\x4b\x43\xaf\x50\x2f\xfe\xf4\x81\x48" //......*KC.P/...H |
// /* 1050 */ "\x1c\x32\x28\x89\x4e\x80\x5d\xfd\xdd\x02\x30\x30\xf8\x24\x9e\x6a" //.2(.N.]...00.$.j |
// /* 1060 */ "\x43\x67\xf6\xe4\x07\xc0\xe9\x71\xc9\x1d\x5a\x81\x7f\x6e\xf9\x80" //Cg.....q..Z..n.. |
// /* 1070 */ "\xce\x08\xeb\x34\xf5\x82\xde\xe3\xf3\x01\x50\x39\x64\x53\xec\x43" //...4......P9dS.C |
// /* 1080 */ "\x9d\x82\xff\xce\x8d\x80\x19\xa3\x98\x33\x8c\xfb\x07\x44\xce\x27" //.........3...D.' |
// /* 1090 */ "\x02\x5b\xa7\xa4\x56\x77\x6f\x7f\xb0\x0e\xd2\x05\x10\xbb\xc0\xae" //.[..Vwo......... |
// /* 10a0 */ "\xef\x1f\xd0\x07\x03\xbf\x5f\xda\xed\xf2\x0b\xbb\xd7\xf4\x44\xb1" //......_.......D. |
// /* 10b0 */ "\x5a\xcd\xf0\xbc\x88\x1d\xbe\x39\x04\xf0\x59\xf3\xeb\xbd\x09\x18" //Z......9..Y..... |
// /* 10c0 */ "\xac\xd2\x42\xe0\x42\x10\x28\x6f\x88\x8f\xf6\x0b\xf8\xef\x07\x34" //..B.B.(o.......4 |
// /* 10d0 */ "\x0e\xac\xc0\x03\x60\xbf\x8d\xc0\xcb\xa0\x88\x94\xea\x05\xfd\x80" //....`........... |
// /* 10e0 */ "\x0a\xc1\x7f\x19\xf3\x28\x16\x8d\x20\xbb\xb0\x02\x58\x2f\xe2\xff" //.....(.. ...X/.. |
// /* 10f0 */ "\x31\x46\x9e\xc0\x2e\xec\x00\xd6\x0c\x04\x76\x49\x96\xb0\x5b\xd8" //1F........vI..[. |
// /* 1100 */ "\x02\x2c\x18\x08\xc1\x6b\x8e\x10\xd9\xd8\x02\xac\x17\xf1\x2f\x01" //.,...k......../. |
// /* 1110 */ "\x28\x75\xe6\x00\xcb\x05\xfc\x43\xf5\x2c\xd3\xdf\x01\x03\x6c\x17" //(u.....C.,....l. |
// /* 1120 */ "\xf0\xf8\x15\xaa\x38\x81\x6f\x60\x0f\xb0\x5f\xc3\x60\x45\xd0\x4f" //....8.o`.._.`E.O |
// /* 1130 */ "\x11\x04\x6c\x17\xf0\xc8\x13\x44\xb7\xc2\x81\x2b\x05\xfc\x2b\x88" //..l....D...+..+. |
// /* 1140 */ "\x7a\xc5\x5e\x60\x14\xb0\x5f\xc2\x3f\xa6\x02\xb6\x0b\xf8\x3c\x0a" //z.^`.._.?.....<. |
// /* 1150 */ "\x99\x1c\x61\xb7\xb0\x0b\xd8\x2f\xe0\xd0\x10\x6a\x33\x00\xcd\x82" //..a..../...j3... |
// /* 1160 */ "\xfe\x09\x02\x6e\x86\x00\xd9\xd8\x06\xac\x17\xf0\x38\x12\x97\xa7" //...n........8... |
// /* 1170 */ "\x4c\x03\x76\x0b\xfc\x0c\x42\x09\x39\xab\x4a\x6e\xec\x03\xb6\x0b" //L.v...B.9.Jn.... |
// /* 1180 */ "\xfc\x0a\x59\x14\x90\xdd\xee\x6f\xd8\x76\x9d\x44\x6e\x08\xa4\x36" //..Y....o.v.Dn..6 |
// /* 1190 */ "\x84\x51\x9b\x82\x20\x45\x96\x7a\x03\xdd\x11\x02\x19\x43\x60\x86" //.Q.. E.z.....C`. |
// /* 11a0 */ "\x50\xe0\x11\x4a\x24\x82\x29\x43\xa0\x47\x20\x26\xb6\x7a\x10\xc8" //P..J$.)C.G &.z.. |
// /* 11b0 */ "\x04\x06\x1b\x35\xa1\x50\x23\x9f\x4a\x20\x11\x4f\xbc\xfc\x90\x41" //...5.P#.J .O...A |
// /* 11c0 */ "\x04\xa1\x80\x41\x20\x20\x10\x4c\x0b\x04\x80\xc6\xee\xf8\x3b\xe9" //...A  .L......;. |
// /* 11d0 */ "\xcb\xdd\x42\x06\x00\xc0\x2b\x2c\x1c\xe3\x13\x28\x0c\xf7\xa1\x43" //..B...+,...(...C |
// /* 11e0 */ "\x9b\xa2\xe1\x6d\xe9\xb3\x27\xe0\x00\x68\xbc\xb3\x71\x43\xc5\xd3" //...m..'..h..qC.. |
// /* 11f0 */ "\x8e\x0f\xc4\x4a\x72\x0d\xbe\x81\xe5\xf4\x04\x86\xeb\x40\x02\x00" //...Jr........@.. |
// /* 1200 */ "\xbc\x7a\xea\x00\x2c\x10\x10\x1b\xad\xc1\xfc\x06\x03\x61\xba\x5c" //.z..,........a.. |
// /* 1210 */ "\x27\xe0\x30\x1a\x0d\xd2\xe1\x52\x06\xa1\x58\x1e\xe8\x94\x07\xba" //'.0....R..X..... |
// /* 1220 */ "\x1c\x81\xce\x80\x29\x00\x73\x80\x80\xe2\x25\x50\x04\x62\xe7\x8a" //....).s...%P.b.. |
// /* 1230 */ "\xc3\x58\x0f\x8a\x70\x9f\x18\xbb\xc3\x18\x96\x00\x20\x80\x7d\xe5" //.X..p....... .}. |
// /* 1240 */ "\xe2\x02\x45\x6c\xa0\x7b\x05\x85\xa2\xc0\x36\x7b\x2c\xf6\x19\x14" //..El.{....6{,... |
// /* 1250 */ "\xe5\x9c\xb0\x5f\xee\x00\x01\x7f\x71\x18\x80\xd8\x98\x68\x03\xb4" //..._....q....h.. |
// /* 1260 */ "\x08\xe7\x80\x71\x41\x00\x03\xa9\x72\xd6\xc6\xa0\x02\x84\x01\xc5" //...qA...r....... |
// /* 1270 */ "\x65\x40\x07\xe2\xd0\xe1\x5f\x44\x00\x3c\x5c\x6c\x86\x08\x81\x6f" //e@...._D.<.l...o |
// /* 1280 */ "\x24\xa3\x4a\xe2\x9c\x5d\xe3\x8b\x1a\x08\xc1\x6f\x44\xa7\x52\xe3" //$.J..].....oD.R. |
// /* 1290 */ "\x9f\x3a\xe0\x70\x0b\xbb\x1f\xce\x62\xc7\xd7\xcc\x20\x5b\xec\xb3" //.:.p....b... [.. |
// /* 12a0 */ "\xd4\xd8\x66\x2d\x46\x7e\x65\x02\xff\x42\xaf\x14\x49\x7f\x36\x0c" //..f-F~e..B..I.6. |
// /* 12b0 */ "\x1e\x02\x36\x7d\xa7\xcc\x80\x1b\x3b\x44\xb7\xec\x3f\x34\x40\xe4" //..6}....;D..?4@. |
// /* 12c0 */ "\x17\x74\x4a\x76\x17\x77\x7c\xc8\x83\x50\x2f\xf6\x3a\xf4\xbe\x6a" //.tJv.w|..P/.:..j |
// /* 12d0 */ "\x39\xc5\xdf\x6c\x0a\x20\x55\x96\xf1\x69\x40\x98\xed\xee\x56\xfe" //9..l. U..i@...V. |
// /* 12e0 */ "\x2f\x51\xc5\xa5\x03\x61\xb7\xa7\x5b\xef\xd8\x11\x55\xfd\x9e\x1a" ///Q...a..[...U... |
// /* 12f0 */ "\x03\x51\xa6\x18\x5c\xad\xcd\xbd\x85\x85\xe3\x15\x1c\x4b\xf7\xd8" //.Q...........K.. |
// /* 1300 */ "\x48\x2e\xe4\x94\x62\xe8\x21\xdd\x43\x0b\x1a\xc6\x21\x4a\x08\x41" //H...b.!.C...!J.A |
// /* 1310 */ "\x18\x00\x3e\xb2\x81\x7f\x25\xa3\x61\x7c\x97\xfa\x99\x05\xde\xcb" //..>...%.a|...... |
// /* 1320 */ "\x3d\xf2\x46\x10\x0b\xbc\x9a\x6b\xf3\x30\x40\xb7\xa3\xf9\x83\x08" //=.F....k.0@..... |
// /* 1330 */ "\x11\x0d\x9f\x0f\xb6\x07\xbe\x54\x50\xab\xdc\x03\x83\xd0\x4e\x6a" //.......TP.....Nj |
// /* 1340 */ "\x2b\xfb\x25\x3b\x0b\xf8\xbe\xfc\x78\x0d\x07\xfe\xf8\x40\x80\x6c" //+.%;....x....@.l |
// /* 1350 */ "\xf2\xbf\xc3\x81\x25\x1b\xea\xe0\xc1\x6f\x09\x92\xf8\x02\xa4\x55" //....%....o.....U |
// /* 1360 */ "\x64\xee\xcb\x28\x1d\x82\xff\xfd\x23\xe9\xd0\x0d\x9e\x1b\xfc\x9f" //d..(....#....... |
// /* 1370 */ "\x4d\x40\xbf\xfd\x3c\x24\x55\x6f\x0d\xbe\x9c\x40\x2e\xe6\x98\x96" //M@..<$Uo...@.... |
// /* 1380 */ "\xf9\xf4\xd0\x18\x5f\x10\xe7\xd1\x31\xdb\xdb\xa1\x8c\x03\xff\x41" //...._...1......A |
// /* 1390 */ "\x20\xbb\xdc\xc0\x38\xbd\xb8\x45\xc7\x3c\xf5\x00\x0a\x22\x21\x40" // ...8..E.<..."!@ |
// /* 13a0 */ "\x1d\x20\x63\x80\xbb\x53\xa8\x00\x50\xe1\x00\x0a\x6c\x06\x73\xdf" //. c..S..P...l.s. |
// /* 13b0 */ "\xb5\xcd\xed\x70\xa9\x71\x3e\x94\x2e\x1c\xce\x14\xb8\x96\x00\xa1" //...p.q>......... |
// /* 13c0 */ "\xe2\xa1\xa7\x5c\x8c\x3a\x62\xc5\xc3\xa5\xdb\xa5\x93\xae\x0c\x61" //.....:b........a |
// /* 13d0 */ "\x57\x2b\xeb\x7d\x18\xa9\xb9\x8a\x9d\xeb\x02\xd0\xba\x59\x44\x80" //W+.}.........YD. |
// /* 13e0 */ "\x12\x02\x93\x80\x17\x5e\x46\x32\x53\xbd\x34\xf3\x04\x30\xdb\xe5" //.....^F2S.4..0.. |
// /* 13f0 */ "\xbe\x7c\xc3\xed\xbc\xd3\x38\x81\x1a\x3e\x8a\xc5\x5e\xf4\x1d\xb0" //.|....8..>..^... |
// /* 1400 */ "\x5b\xfd\xa3\x1f\x25\x20\xbb\xa3\xfe\x3a\x81\x02\x7b\x66\x3e\x9c" //[...% ...:..{f>. |
// /* 1410 */ "\x40\xea\xdf\x26\xc0\x6c\xff\xbb\x1f\x66\xe2\xe1\x85\xf0\x96\x73" //@..&.l...f.....s |
// /* 1420 */ "\x9c\x46\xb9\x2d\xee\x36\xf8\x13\x9d\xbe\xbb\x7f\x60\x61\xf3\x99" //.F.-.6......`a.. |
// /* 1430 */ "\x56\x5b\xd2\x85\xe5\x58\x3d\xe9\x3c\xe8\x8c\xee\x6e\xef\x6d\x7d" //V[...X=.<...n.m} |
// /* 1440 */ "\xe8\x6b\xe3\xb0\x07\x54\x60\x18\xe6\x23\x80\x8f\x06\xb8\xc3\xc7" //.k...T`..#...... |
// /* 1450 */ "\x7c\xf8\xf9\x33\xf4\xa3\xf1\x30\x10\x1d\xfe\x98\xf8\xef\xef\x7e" //|..3...0.......~ |
// /* 1460 */ "\x05\xbf\x09\xdf\x50\x37\x7e\x89\x3e\xe0\x03\x08\x60\x00"         //....P7~.>...`. |
// Sent dumped on RDP Client (5) 5230 bytes |
// send_server_update done |
// GraphicsUpdatePDU::init::Initializing orders batch mcs_userid=0 shareid=65538 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[1](30) used=139 free=16139 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=68) |
// front::draw:draw_tile((768, 528, 64, 64) (576, 384, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[2](8208) used=145 free=16133 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[3](30) used=275 free=16003 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,528,64,64) rop=cc srcx=0 srcy=0 cache_idx=69) |
// front::draw:draw_tile((192, 592, 64, 32) (0, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[4](4112) used=281 free=15997 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[5](30) used=353 free=15925 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(192,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=70) |
// front::draw:draw_tile((256, 592, 64, 32) (64, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[6](4112) used=364 free=15914 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[7](30) used=447 free=15831 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(256,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=71) |
// front::draw:draw_tile((320, 592, 64, 32) (128, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[8](4112) used=453 free=15825 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[9](30) used=512 free=15766 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(320,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=72) |
// front::draw:draw_tile((384, 592, 64, 32) (192, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[10](4112) used=518 free=15760 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[11](30) used=615 free=15663 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(384,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=73) |
// front::draw:draw_tile((448, 592, 64, 32) (256, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[12](4112) used=621 free=15657 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[13](30) used=879 free=15399 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(448,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=74) |
// front::draw:draw_tile((512, 592, 64, 32) (320, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[14](4112) used=885 free=15393 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[15](30) used=1153 free=15125 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(512,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=75) |
// front::draw:draw_tile((576, 592, 64, 32) (384, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[16](4112) used=1159 free=15119 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[17](30) used=1260 free=15018 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(576,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=76) |
// front::draw:draw_tile((640, 592, 64, 32) (448, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[18](4112) used=1266 free=15012 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[19](30) used=1319 free=14959 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(640,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=77) |
// front::draw:draw_tile((704, 592, 64, 32) (512, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[20](4112) used=1325 free=14953 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[21](30) used=1409 free=14869 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(704,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=78) |
// front::draw:draw_tile((768, 592, 64, 32) (576, 448, 64, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[22](4112) used=1415 free=14863 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[23](30) used=1496 free=14782 |
// order(13 clip(0,0,1,1)):memblt(cache_id=2 rect(768,592,64,32) rop=cc srcx=0 srcy=0 cache_idx=79) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[24](32) used=1502 free=14776 |
// order(9 clip(200,145,1,110)):lineto(back_mode=01 startx=200 starty=1198 endx=200 endy=145 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[25](32) used=1530 free=14748 |
// order(9 clip(200,145,1,110)):lineto(back_mode=01 startx=200 starty=145 endx=200 endy=1198 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[26](32) used=1536 free=14742 |
// order(9 clip(200,145,1,110)):lineto(back_mode=01 startx=201 starty=1198 endx=200 endy=145 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[27](32) used=1544 free=14734 |
// order(9 clip(200,145,1,110)):lineto(back_mode=01 startx=200 starty=145 endx=201 endy=1198 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[28](32) used=1554 free=14724 |
// order(9 clip(145,200,110,1)):lineto(back_mode=01 startx=1198 starty=200 endx=145 endy=200 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[29](32) used=1569 free=14709 |
// order(9 clip(145,200,110,1)):lineto(back_mode=01 startx=145 starty=200 endx=1198 endy=200 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[30](32) used=1575 free=14703 |
// order(9 clip(145,200,110,1)):lineto(back_mode=01 startx=1198 starty=201 endx=145 endy=200 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[31](32) used=1583 free=14695 |
// order(9 clip(145,200,110,1)):lineto(back_mode=01 startx=145 starty=200 endx=1198 endy=201 rop2=13 back_color=000000pen.style=0 pen.width=1 pen.color=00f800  |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[32](82) used=1593 free=14685 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[33](66) used=1659 free=14619 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[34](66) used=1709 free=14569 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[35](66) used=1759 free=14519 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[36](66) used=1809 free=14469 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[37](20) used=1859 free=14419 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[38](297) used=1879 free=14399 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[39](66) used=1929 free=14349 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[40](66) used=1979 free=14299 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[41](66) used=2029 free=14249 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[42](297) used=2079 free=14199 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[43](82) used=2111 free=14167 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[44](66) used=2177 free=14101 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[45](66) used=2227 free=14051 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[46](297) used=2277 free=14001 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[47](82) used=2309 free=13969 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[48](66) used=2375 free=13903 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[49](66) used=2425 free=13853 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[50](297) used=2475 free=13803 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[51](66) used=2507 free=13771 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[52](66) used=2557 free=13721 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[53](66) used=2607 free=13671 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[54](297) used=2657 free=13621 |
// Widget_load: image file [./tests/fixtures/xrdp24b-redemption.png] is PNG file |
// front::draw:draw_tile((738, 613, 64, 64) (0, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[55](8208) used=2692 free=13586 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[56](30) used=3170 free=13108 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(738,613,64,64) rop=cc srcx=0 srcy=0 cache_idx=80) |
// front::draw:draw_tile((802, 613, 64, 64) (64, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[57](8208) used=3179 free=13099 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[58](30) used=3890 free=12388 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(802,613,64,64) rop=cc srcx=0 srcy=0 cache_idx=81) |
// front::draw:draw_tile((866, 613, 64, 64) (128, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[59](8208) used=3896 free=12382 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[60](30) used=4667 free=11611 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(866,613,64,64) rop=cc srcx=0 srcy=0 cache_idx=82) |
// front::draw:draw_tile((930, 613, 64, 64) (192, 0, 64, 64)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[61](8208) used=4673 free=11605 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[62](30) used=5355 free=10923 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(930,613,64,64) rop=cc srcx=0 srcy=0 cache_idx=83) |
// front::draw:draw_tile((738, 677, 64, 61) (0, 64, 64, 61)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[63](7824) used=5361 free=10917 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[64](30) used=6300 free=9978 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(738,677,64,61) rop=cc srcx=0 srcy=0 cache_idx=84) |
// front::draw:draw_tile((802, 677, 64, 61) (64, 64, 64, 61)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[65](7824) used=6311 free=9967 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[66](30) used=7623 free=8655 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(802,677,64,61) rop=cc srcx=0 srcy=0 cache_idx=85) |
// front::draw:draw_tile((866, 677, 64, 61) (128, 64, 64, 61)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[67](7824) used=7629 free=8649 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[68](30) used=9025 free=7253 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(866,677,64,61) rop=cc srcx=0 srcy=0 cache_idx=86) |
// front::draw:draw_tile((930, 677, 64, 61) (192, 64, 64, 61)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[69](7824) used=9031 free=7247 |
// GraphicsUpdatePDU::flush_orders: order_count=69 |
// send_server_update: fastpath_support=yes compression_support=yes shareId=65538 encryptionLevel=0 initiator=0 type=0 data_extra=69 |
// Sending on RDP Client (5) 6454 bytes |
// /* 0000 */ "\x00\x99\x36\x80\x21\x2f\x19\x45\x00\x03\x7e\xe4\x53\x8b\xe0\x02" //..6.!/.E..~.S... |
// /* 0010 */ "\x26\x00\x9a\xcf\xb4\x6e\xfd\xa9\xe3\x80\x92\xa1\x81\x52\x58\xab" //&....n.......RX. |
// /* 0020 */ "\xfb\x83\x80\xe2\x87\xc5\x8d\x82\x14\x26\x5f\x4d\xed\x27\x10\x17" //.........&_M.'.. |
// /* 0030 */ "\xb9\x25\x00\x14\x85\xd3\xb3\xe3\xb1\x44\xe0\x0e\x69\x93\x8b\xe0" //.%.......D..i... |
// /* 0040 */ "\x0e\x69\xa0\x5c\x31\x55\x7b\xb4\x63\xc0\x80\x01\xcc\x46\xe1\x5d" //.i..1U{.c....F.] |
// /* 0050 */ "\x66\x91\x28\xa5\xc2\xf9\xda\x3e\x51\x00\x00\xf0\x99\x0b\xca\x36" //f.(....>Q......6 |
// /* 0060 */ "\xe6\x00\x8a\x01\xde\x49\xed\x67\x91\x1d\x49\xe7\x93\xfc\x60\x02" //.....I.g..I...`. |
// /* 0070 */ "\x28\x00\x02\xc5\x5e\x5c\xd8\xcf\x95\x4b\x21\x74\xa8\x01\x00\x1d" //(...^....K!t.... |
// /* 0080 */ "\x7d\x96\x15\x0b\x91\x0a\x5d\x03\x15\x66\x98\x31\x5a\xe5\xe0\x03" //}.....]..f.1Z... |
// /* 0090 */ "\x66\x09\xfc\x8f\x1f\xb0\x50\x73\x00\x1c\x46\x09\xfc\x8e\xc4\x4c" //f.....Ps..F....L |
// /* 00a0 */ "\x3a\x45\x9f\x71\x7a\xf6\xc8\xef\xc9\x16\x40\x20\x72\x00\x8d\x88" //:E.qz.....@ r... |
// /* 00b0 */ "\xe0\x18\xac\xf9\x83\x91\xd7\x0c\x56\x9c\xc5\x69\xb0\xc0\xdd\x8c" //........V..i.... |
// /* 00c0 */ "\xf8\xbc\x58\x00\x28\x08\xb7\x69\xf2\x80\x11\x07\x81\x00\x68\xde" //..X.(..i......h. |
// /* 00d0 */ "\x13\xc5\x10\x01\x1c\x00\xe3\x03\xee\x3a\x10\xa5\x4a\x44\xc0\x17" //.........:..JD.. |
// /* 00e0 */ "\x8c\x58\x69\x18\xb1\x1e\x31\x63\xb4\xe2\x84\xc9\xc3\x3d\x8c\x10" //.Xi...1c.....=.. |
// /* 00f0 */ "\x5e\xe0\x01\x58\xce\x67\xa2\x01\x87\x69\xe5\x1f\x8e\x43\xc3\x38" //^..X.g...i...C.8 |
// /* 0100 */ "\x96\x00\x24\x61\xb8\xd0\x00\x06\x2d\x22\x62\xd2\x98\xe0\xbb\x18" //..$a....-"b..... |
// /* 0110 */ "\xcd\x8b\xad\x8f\xc0\x32\x90\x00\x06\xa9\xe0\x1c\x54\x80\x12\x7b" //.....2......T..{ |
// /* 0120 */ "\x64\x0d\x24\xa4\x00\xbf\x14\x30\x01\x60\xc6\x38\x7a\x78\x42\xbb" //d.$....0.`.8zxB. |
// /* 0130 */ "\x20\xe7\xe1\x96\x2e\xaa\x8f\x60\xc6\x06\xce\x76\x0f\xcb\xd8\x0f" // ......`...v.... |
// /* 0140 */ "\x6d\x54\x6a\xac\xf3\xe2\x73\xff\x89\xe5\x24\x00\x0e\xeb\xe2\x7c" //mTj...s...$....| |
// /* 0150 */ "\x6e\x60\x09\x40\x01\x24\x00\xde\xbe\x08\x00\x58\x31\x88\x85\x78" //n`.@.$.....X1..x |
// /* 0160 */ "\x72\x29\x3c\x01\xeb\xd4\x91\xec\x5b\xc1\x28\xcc\x60\x6c\xe0\x1d" //r)<.....[.(.`l.. |
// /* 0170 */ "\x04\x18\xc5\x29\x84\x9c\x9e\x10\x33\x81\xc2\x69\xf0\x34\xd8\x5d" //...)....3..i.4.] |
// /* 0180 */ "\x2e\x34\x93\x22\xb3\xcc\x64\xd3\x98\x34\xe5\x16\x82\x49\x49\xa1" //.4."..d..4...II. |
// /* 0190 */ "\x8a\x26\xd0\xae\x69\x94\x2a\x9a\x60\x00\x2c\x28\xbe\x09\x4b\x8e" //.&..i.*.`.,(..K. |
// /* 01a0 */ "\x34\x4b\x41\xa8\xd0\x3a\xb5\x8a\xbc\x83\x6f\x80\x5d\xe4\x96\xe2" //4KA..:....o.]... |
// /* 01b0 */ "\x0b\x83\x88\x05\xe0\x7a\x0e\x9d\x0c\x2a\x92\xd9\xa7\xb0\x3a\xb1" //.....z...*....:. |
// /* 01c0 */ "\x0d\x9e\x01\x77\xa0\x5f\xda\x39\x4d\xa4\x56\xe7\x20\x0b\xc0\xc4" //...w._.9M.V. ... |
// /* 01d0 */ "\x18\xbe\x08\x15\x25\xa3\x7d\x99\x90\x5d\xdb\xfe\xb8\x8a\xdc\x00" //....%.}..]...... |
// /* 01e0 */ "\x0b\x05\x86\xc5\x24\xf3\x56\x4a\x70\x0f\xec\x3a\xfd\x34\x8a\xeb" //....$.VJp..:.4.. |
// /* 01f0 */ "\x84\x11\x54\xb8\xe5\x9a\x7b\x62\xef\x22\xe8\xd8\x80\xa0\xdd\x47" //..T...{b.".....G |
// /* 0200 */ "\x20\xb7\xeb\xfa\x7d\x38\x96\x8c\x03\x67\xa0\x5f\xfa\x2d\x06\x60" // ...}8...g._.-.` |
// /* 0210 */ "\xa1\xf0\x42\xa9\x2b\x25\x3a\x05\xe1\xc7\x6f\x58\xeb\x88\x15\x82" //..B.+%:...oX.... |
// /* 0220 */ "\x27\x43\x28\xd3\xd0\x2f\xc0\xed\xdd\x02\x7e\xc8\x84\xa0\x00\x3c" //'C(../....~....< |
// /* 0230 */ "\x00\x36\xf6\x45\xdf\x40\x12\xf0\x4a\xe3\x15\x83\x81\x82\x40\x2c" //.6.E.@..J.....@, |
// /* 0240 */ "\x18\xc4\x3a\xb9\x09\x4f\xae\x23\x85\xee\x1c\x1e\x23\x84\x0c\xfb" //..:..O.#....#... |
// /* 0250 */ "\x79\x40\xc2\x72\xce\x18\x13\xc0\x92\x78\x44\xc7\xb4\x60\xc0\x0e" //y@.r.....xD..`.. |
// /* 0260 */ "\x03\x43\xa4\x7a\xfa\x05\x13\x61\x64\x94\x14\x5a\x0b\x2c\xe6\x4d" //.C.z...ad..Z.,.M |
// /* 0270 */ "\x31\xa3\xcc\x42\x25\x49\x32\x29\x54\x68\xb2\x17\x0e\x85\xd3\xe0" //1..B%I2)Th...... |
// /* 0280 */ "\x78\x79\xcc\x01\x78\x85\xf0\xd7\x5c\x24\xb6\xc4\x1b\x7d\x0a\xbc" //xy..x....$...}.. |
// /* 0290 */ "\xc7\x4e\x05\xa3\x33\xcb\x69\x51\xc4\xe8\x67\x36\xe0\xab\x17\x61" //.N..3.iQ..g6...a |
// /* 02a0 */ "\xf9\x67\xb0\x5f\xe1\x1d\xc4\x96\xf0\x1b\x3c\x0e\xac\x0b\x46\x83" //.g._......<...F. |
// /* 02b0 */ "\xcd\x69\x71\xce\x70\x21\x8f\xac\x83\xf9\xf9\xfd\x04\x94\x64\x89" //.iq.p!........d. |
// /* 02c0 */ "\x6b\x67\xf0\x81\x9c\x01\x06\x83\x48\x2e\xe4\x3f\xa1\x46\x9e\xda" //kg......H..?.F.. |
// /* 02d0 */ "\xa3\x9d\x4d\x83\x5f\x49\xd7\xf9\x23\xab\x41\xe6\xbd\x6d\x1d\x96" //..M._I..#.A..m.. |
// /* 02e0 */ "\xa3\xe7\x24\x5f\x78\x23\x46\xf9\xd5\x00\xbb\xa1\xdb\xe8\xdd\x9d" //..$_x#F......... |
// /* 02f0 */ "\xb0\xea\xa8\xa1\x57\x9a\x25\xb5\x00\x33\xe0\x0c\x07\x35\x29\x60" //....W.%..3...5)` |
// /* 0300 */ "\x00\x6b\x1e\xd2\xc5\x58\x01\x33\x05\x70\x38\xa0\xc5\x40\x04\xd2" //.k...X.3.p8..@.. |
// /* 0310 */ "\x4a\x03\xed\x67\x13\x87\x13\xf7\x14\x49\xe2\x83\x3c\x4b\x22\xa1" //J..g.....I..<K". |
// /* 0320 */ "\x83\x6b\xe7\x8a\x26\x7d\x83\xa6\x81\x8c\x2d\x31\xe2\x8b\xc4\xd1" //.k..&}....-1.... |
// /* 0330 */ "\xc5\x16\x53\x00\x00\xf1\xe5\x8b\xc5\x76\x4c\x00\x9b\xc7\xad\xc7" //..S......vL..... |
// /* 0340 */ "\xbd\x60\xe3\xd9\x30\x0f\x0d\xb8\x15\xb8\xa0\xc7\x4e\x57\x13\x5a" //.`..0.......NW.Z |
// /* 0350 */ "\xe3\xcf\x2c\x1f\xee\x53\x40\x00\xd1\xff\xde\x28\xa0\x09\xc0\x04" //..,..S@....(.... |
// /* 0360 */ "\xdc\x1e\x1c\xa0\x64\xe0\x02\xcd\xca\x06\x70\x11\x73\x93\x4b\x8b" //....d.....p.s.K. |
// /* 0370 */ "\xde\x3e\x0c\x70\x83\xc4\x17\xe6\x39\xfe\x4f\x28\x1c\x79\x48\x47" //.>.p....9.O(.yHG |
// /* 0380 */ "\x28\x2b\x4e\x00\x03\x44\xf0\xd6\x28\x40\x09\xe0\x18\x4a\xbd\xec" //(+N..D..(@...J.. |
// /* 0390 */ "\x21\x80\x05\xbb\x99\x5c\x7a\xdd\x81\x20\x63\x18\x09\x79\x9e\x17" //!.....z.. c..y.. |
// /* 03a0 */ "\xa4\xf0\x00\xd0\x95\xf0\x30\xfa\x40\x04\x44\x02\x90\x01\x7e\x00" //......0.@.D...~. |
// /* 03b0 */ "\x01\xf8\xc9\x70\x27\xce\x80\xd0\x10\x0b\xc0\x03\x28\xa4\x47\xe7" //...p'.......(.G. |
// /* 03c0 */ "\x99\x45\xa9\x3f\x19\x11\x00\x65\x1e\xfb\x31\xf3\x21\x14\x7a\xe1" //.E.?...e..1.!.z. |
// /* 03d0 */ "\x49\x37\x36\xa5\x7e\xaa\xa4\x00\x32\x85\x7e\x79\x0e\x97\x02\x52" //I76.~...2.~y...R |
// /* 03e0 */ "\x7e\x81\x94\x7b\xea\x47\xcc\x80\x33\x5c\x62\xe0\x03\xc1\xd4\x60" //~..{.G..3.b....` |
// /* 03f0 */ "\x0b\xe5\xfe\xe1\x7f\x18\xc5\x89\x2f\x83\x8e\x0e\x3b\x7c\x30\x22" //......../...;|0" |
// /* 0400 */ "\x20\x00\x24\x93\xe1\xc0\x14\xa3\xe1\xc0\x08\x87\xc3\x8f\x83\xa0" // .$............. |
// /* 0410 */ "\x19\x2f\x80\xa8\x07\xc0\x52\x21\xe0\x29\x04\xf8\x56\x0b\x40\x06" //./....R!.)..V.@. |
// /* 0420 */ "\x20\x04\xf8\x58\xfd\x97\xc1\x9f\xe5\x80\x17\xf2\xcf\xfa\x47\xc1" // ..X..........G. |
// /* 0430 */ "\x8f\xed\x1f\x0b\x2f\x83\x87\xf2\xc0\x0f\xf9\x6b\xfb\xa8\x3f\xe2" //..../......k..?. |
// /* 0440 */ "\x3e\x16\x18\x35\x31\xf0\x6f\xf9\x60\x09\xfc\xb7\xf8\x30\x07\x40" //>..51.o.`....0.@ |
// /* 0450 */ "\x06\x60\x04\x20\x07\xb0\x02\x00\x03\x74\x1f\x7d\x17\xc1\x90\x3f" //.`. .....t.}...? |
// /* 0460 */ "\xfb\x00\xe0\x00\x21\x01\x10\x10\x89\x54\xa9\x57\xe1\x48\x48\xdd" //....!....T.W.HH. |
// /* 0470 */ "\x6c\xfc\xf4\x1c\x0c\x07\xa9\x10\x7b\xe1\x10\x78\x49\x3e\x46\x10" //l.......{..xI>F. |
// /* 0480 */ "\x1f\xbe\xf8\x50\x3d\xf3\x18\x36\x74\x02\x16\x04\x10\x06\x06\x08" //...P=..6t....... |
// /* 0490 */ "\x0b\xe3\x8c\x01\x7c\x71\xb8\x3d\x03\xd8\x51\xe3\x65\x00\x11\x03" //....|q.=..Q.e... |
// /* 04a0 */ "\xe4\x40\x27\xd9\xc3\xcd\x58\x83\x7f\x2d\x08\x3e\x15\x01\xd8\x3c" //.@'...X..-.>...< |
// /* 04b0 */ "\xd5\x1f\x0a\x01\x98\x00\xec\x1f\x61\xf0\x67\xf9\x60\x0f\xfc\xb4" //........a.g.`... |
// /* 04c0 */ "\xf8\x3d\xa0\x31\x03\xb3\x0e\x1d\x83\x20\x03\xe0\x04\x2f\x8d\x68" //.=.1..... .../.h |
// /* 04d0 */ "\x40\x01\x40\xa0\x00\x81\x00\xc1\x00\xe1\x00\xe0\x9c\x31\xb0\x08" //@.@..........1.. |
// /* 04e0 */ "\xf0\x94\xf0\xc6\xd4\x06\xe0\x00\x04\x11\xc0\x6d\xf0\xe4\xe3\xf9" //...........m.... |
// /* 04f0 */ "\x3e\x1c\x7d\x27\xd8\x7c\x1d\x3c\xa9\x80\x4f\x80\xa7\xca\x9b\x02" //>.}'.|.<..O..... |
// /* 0500 */ "\xc0\x01\xbf\xf3\xe1\x61\xf0\x70\xfe\x58\x05\x7f\x2e\x4e\x1f\x1c" //.....a.p.X...N.. |
// /* 0510 */ "\xbe\x86\x8b\x00\x3f\x64\x22\x28\x02\xb7\xc6\xb5\x60\x00\xac\x37" //....?d"(....`..7 |
// /* 0520 */ "\xca\x0a\x04\x05\x04\x08\x0a\x08\xf4\x36\x01\x7e\x12\x9f\x43\x6a" //.........6.~..Cj |
// /* 0530 */ "\x0f\x60\x00\xe0\x64\xf8\x75\xf3\x37\xf8\x3a\x79\x53\x00\xcf\x01" //.`..d.u.7.:yS... |
// /* 0540 */ "\x4f\x95\x2b\xce\x91\xf0\xb8\xbe\x0e\x1f\xcb\x00\xdf\xe5\x9f\xc1" //O.+............. |
// /* 0550 */ "\xc3\xcf\x91\xf0\xad\x80\x0e\x41\xf4\x5f\x06\x7d\x0d\x03\xe0\x00" //.......A._.}.... |
// /* 0560 */ "\x0b\x5c\x11\x8d\x5f\x1a\xda\x80\x02\x82\xc0\x03\x02\x43\x40\xc1" //...._........C@. |
// /* 0570 */ "\x38\x27\x3c\x25\x80\x77\x84\xb8\xc0\xf8\x83\x38\x6b\x47\x84\xb6" //8'<%.w.....8kG.. |
// /* 0580 */ "\xfe\x58\x07\xff\x2e\x48\x00\x36\x04\x0e\x7c\x28\x06\x40\x03\x80" //.X...H.6..|(.@.. |
// /* 0590 */ "\x7d\x17\xc1\x9f\xe5\x80\x87\xf2\xcf\xf2\x47\xc2\x94\x00\x48\x00" //}.........G...H. |
// /* 05a0 */ "\x2e\x41\x8d\x70\xde\x40\x01\x10\x3e\xcb\xe0\xc8\x0c\xc3\x88\x13" //.A.p.@..>....... |
// /* 05b0 */ "\x1e\xc1\x4b\x76\x04\xe1\x3f\x7c\x6b\x7e\xf3\xcd\x07\x01\x87\x84" //..Kv..?|k~...... |
// /* 05c0 */ "\x08\x03\x81\xd4\x70\x14\x3e\x34\xf0\x15\x0c\x3e\x32\x41\xa4\x50" //....p.>4...>2A.P |
// /* 05d0 */ "\x00\xd8\x6d\x10\x12\x06\x00\xf8\x11\x02\xc2\x0d\x40\xbb\xe9\x23" //..m.........@..# |
// /* 05e0 */ "\x02\x70\x12\x08\x43\x80\x00\x04\x08\x00\x41\x06\x60\x32\x42\x27" //.p..C.....A.`2B' |
// /* 05f0 */ "\x01\x41\x98\x60\x55\xe8\x41\x90\x0c\x40\x40\x12\x3e\x72\xb0\x25" //.A.`U.A..@@.>r.% |
// /* 0600 */ "\x02\x00\x08\x41\xa4\x0b\xc0\x40\x08\x1d\x3f\xd8\x00\x83\xef\x40" //...A...@..?....@ |
// /* 0610 */ "\x01\x80\xbe\x0a\x7e\x20\xd8\x04\xff\x23\x0e\xd4\xbf\xa0\x5e\x02" //....~ ...#....^. |
// /* 0620 */ "\x83\x38\xf0\x51\xc3\xe9\x10\x05\xe0\x24\x11\x01\x32\x11\x3f\xf1" //.8.Q.....$..2.?. |
// /* 0630 */ "\x16\x01\x68\x09\x03\x47\x81\x94\x0c\x3b\x74\xfc\x41\x6f\xbc\x13" //..h..G...;t.Ao.. |
// /* 0640 */ "\x81\x20\xda\x00\x02\x3f\x62\x01\xf3\x82\x1e\x14\x92\x8f\x9c\x70" //. ...?b........p |
// /* 0650 */ "\x16\xf0\x94\x56\x3e\x51\x63\xe2\x18\x3c\x20\x94\x7c\x84\x03\xe2" //...V>Qc..< .|... |
// /* 0660 */ "\x14\x3c\x20\xf9\x0f\xa1\x2b\x1f\x31\xf8\x08\x35\x0f\x98\x68\xf2" //.< ...+.1..5..h. |
// /* 0670 */ "\xa0\xc3\xc0\x12\x20\x25\xe8\x87\x94\x81\x08\x36\x8f\x20\x0e\x04" //.... %.....6. .. |
// /* 0680 */ "\x7e\x54\x56\x3e\x80\x01\x06\x91\xfa\x20\x47\xe2\x1a\x98\x6d\x01" //~TV>..... G...m. |
// /* 0690 */ "\xfe\x95\x7c\xf4\x43\x1b\x18\x8e\x10\x6c\x03\x30\x12\x07\x8f\x04" //..|.C....l.0.... |
// /* 06a0 */ "\xa7\x1e\xca\x34\x20\xd8\x1a\xfe\xfc\x10\x7a\xc2\x2c\x01\x78\xc3" //...4 .....z.,.x. |
// /* 06b0 */ "\xd8\x00\x70\x40\xf0\x83\x50\x6d\xfa\xb1\xeb\x79\xa4\xa0\xd2\x20" //..p@..Pm...y...  |
// /* 06c0 */ "\x48\x35\x04\x1b\x03\x8b\x0d\xa3\xdd\x6f\xca\x60\x0f\x61\xb4\x08" //H5.......o.`.a.. |
// /* 06d0 */ "\x7f\xe6\x03\xf7\x10\x44\x3e\x2c\x34\x7c\x57\x04\x17\xc5\x61\x5e" //.....D>,4|W...a^ |
// /* 06e0 */ "\x6a\x41\xac\x7c\xc2\x0c\xc3\xe2\x88\x21\x1f\x16\x18\x3e\x28\x00" //jA.|.....!...>(. |
// /* 06f0 */ "\xc3\xe2\xc6\x11\xee\x7e\x08\xa4\x70\x4f\x9f\x3c\xa4\x00\x01\x30" //.....~..pO.<...0 |
// /* 0700 */ "\x34\x1a\xc1\x04\x63\x80\x55\x10\x3c\x00\x02\x41\x50\x6d\x08\x34" //4...c.U.<..APm.4 |
// /* 0710 */ "\x84\x19\x01\x03\x80\x01\xc6\x1e\xb4\x7b\x7d\x40\x0e\x41\x8c\x41" //.........{}@.A.A |
// /* 0720 */ "\x88\x1a\x08\x18\x08\x16\x08\x12\x30\x7b\xb0\x82\xe7\x09\xc2\x55" //........0{.....U |
// /* 0730 */ "\x81\x90\xd1\x60\x1b\x10\xa9\x02\x80\x00\x1c\xeb\x04\x80\xe3\x38" //...`...........8 |
// /* 0740 */ "\x02\x51\x00\xa3\x7b\x4c\x83\x48\xa8\x01\x80\x60\x10\x01\xb0\x4d" //.Q..{L.H...`...M |
// /* 0750 */ "\x00\x1a\x3a\x13\x01\x7b\x09\x1d\x67\x61\xb4\x54\x00\xc8\x1e\x00" //..:..{..ga.T.... |
// /* 0760 */ "\x60\x37\x10\x07\x7f\xa4\x23\xe2\x66\xf4\x43\x01\x49\x30\x67\xca" //`7....#.f.C.I0g. |
// /* 0770 */ "\x59\x85\x03\x8d\x3f\x46\x3f\x20\x08\x81\xa0\x06\x02\x8f\xf4\x54" //Y...?F? .......T |
// /* 0780 */ "\xee\x28\x2c\x17\xf2\x90\x00\xe2\xeb\xe0\xbc\x04\x4c\x24\x00\x12" //.(,.........L$.. |
// /* 0790 */ "\x04\xc3\x68\x00\x48\xc2\x95\xe8\x0a\xd0\x48\xb4\x05\x60\x24\x00" //..h.H.....H..`$. |
// /* 07a0 */ "\x17\xf6\xaa\x00\x30\x11\x40\x91\x68\x14\x70\x27\xab\x83\xf7\xc2" //....0.@.h.p'.... |
// /* 07b0 */ "\x40\x0c\x06\x9f\xe2\x19\xc0\xf0\x11\x04\xfe\x2e\xda\xa8\x57\x80" //@.............W. |
// /* 07c0 */ "\x10\xbe\x60\x6c\x0c\x30\x70\xc2\x5c\x6f\x9c\xa8\x7c\xe9\x00\xac" //..`l.0p..o..|... |
// /* 07d0 */ "\x00\xfb\x29\x1c\xf0\x68\xd7\xc1\xb2\xc3\x83\x1d\xcb\x1f\x38\xf5" //..)..h........8. |
// /* 07e0 */ "\xf3\xa4\x03\xb2\x48\x4c\x07\xc1\x70\x45\xe0\xb7\xd0\x79\x61\x2e" //....HL..pE...ya. |
// /* 07f0 */ "\xff\x8c\x29\xa2\xf9\xd1\xe9\xd3\x4f\xce\x62\x71\x77\xce\x83\x80" //..).....O.bqw... |
// /* 0800 */ "\x18\x09\x27\xce\x61\xf6\x00\x3f\x84\xcf\x7e\x05\x1b\xf9\xc4\x80" //..'.a..?..~..... |
// /* 0810 */ "\x06\x03\xf2\x02\xd0\x17\xcf\x98\xf6\x85\x7a\xf6\x89\x3d\x58\xfa" //..........z..=X. |
// /* 0820 */ "\x0f\x08\xbd\x75\x10\x9f\x9d\x3e\x3d\x86\x7f\x9c\xf5\xbb\xe7\x00" //...u...>=....... |
// /* 0830 */ "\x2d\x00\x3e\x37\x0b\xfa\x71\xf3\xa0\x81\x23\xcd\x30\xf5\xe0\xa2" //-.>7..q...#.0... |
// /* 0840 */ "\xc8\x4e\x0a\x94\x7b\xdc\x08\x00\x60\x03\x01\x15\x09\xf0\x6c\x7a" //.N..{...`.....lz |
// /* 0850 */ "\xe4\x03\x01\x20\x8c\x7d\x37\x88\x15\x0f\xe8\x46\xf8\x88\x50\x00" //... .}7....F..P. |
// /* 0860 */ "\x4d\x40\x14\x82\xb0\x03\xeb\x87\xaf\x20\x56\x04\xd2\x00\xb0\x13" //M@....... V..... |
// /* 0870 */ "\x50\x08\x17\xf7\x3b\x87\xb2\xa0\x0a\x00\x81\xec\x9a\xb8\x00\x09" //P...;........... |
// /* 0880 */ "\x40\xd1\x71\x65\x79\x68\x25\x8e\x3a\x12\x11\x50\x06\x41\x80\x70" //@.qeyh%.:..P.A.p |
// /* 0890 */ "\x2f\xa3\x60\x90\xc8\x30\x0e\x25\x83\x34\x00\x4b\x80\x04\xc2\x5e" ///.`..0.%.4.K...^ |
// /* 08a0 */ "\x82\x59\xa3\xa1\x20\xfc\x42\x98\x56\xe5\xe4\x37\xa4\x62\x92\xb0" //.Y.. .B.V..7.b.. |
// /* 08b0 */ "\x13\x03\x59\xfb\x27\xc0\x96\xc8\xe8\x48\x37\x85\x20\x20\x82\x01" //..Y.'....H7.  .. |
// /* 08c0 */ "\xe1\xd0\x98\x4b\x00\x7c\x74\x24\x16\x05\xc4\x80\x31\x50\x08\xf2" //...K.|t$....1P.. |
// /* 08d0 */ "\x98\x03\xd2\x51\xc8\xd1\x40\x00\x18\x6a\xff\xcf\x56\x84\xa0\x5c" //...Q..@..j..V... |
// /* 08e0 */ "\x4b\x41\x18\x38\xc1\xec\x88\xc6\x7c\x2a\x81\x02\x86\x21\xea\x78" //KA.8....|*...!.x |
// /* 08f0 */ "\x51\x87\x1a\x1c\x62\x18\x76\xc6\x3d\xec\x00\x0e\xc4\x17\x0e\x02" //Q...b.v.=....... |
// /* 0900 */ "\x30\x3f\xc0\x92\x07\xe0\x3e\x3b\x0f\xfb\x4d\xc5\xf6\x5e\xc2\x7e" //0?....>;..M..^.~ |
// /* 0910 */ "\x60\xdb\xe2\xc7\x53\x30\xff\xa8\x1f\x15\x81\x10\xc3\xfe\x7b\x7e" //`...S0........{~ |
// /* 0920 */ "\x09\xf7\x30\x7f\xdf\x71\x89\x16\x40\x03\xd0\xe7\xc0\x97\xbf\x34" //..0..q..@......4 |
// /* 0930 */ "\x96\x42\x56\x02\x4d\x82\x45\x42\x44\xc0\x6e\x3e\xb0\x4b\x00\x1a" //.BV.M.EBD.n>.K.. |
// /* 0940 */ "\x7a\x78\x08\x80\x39\x6b\xa8\x01\x71\xc0\x9b\x9d\x47\x0a\xd1\x04" //zx..9k..q...G... |
// /* 0950 */ "\x70\x00\xd1\x47\x07\xe9\x38\xc6\xe3\x74\x02\x52\xc1\x88\x07\xc0" //p..G..8..t.R.... |
// /* 0960 */ "\x9a\x00\x07\x02\x74\x90\x7a\x80\x2a\xc2\x3f\xc6\x2e\x04\x20\x32" //....t.z.*.?... 2 |
// /* 0970 */ "\x91\xd0\x00\xee\x28\xa1\x08\x0c\x38\x53\xe4\x96\x7e\x4a\x26\x27" //....(...8S..~J&' |
// /* 0980 */ "\xf2\x4b\x1f\x24\xf0\x15\xf2\x5f\x49\xc5\x48\x82\x3e\x35\x01\xa0" //.K.$..._I.H.>5.. |
// /* 0990 */ "\x05\x52\x60\x22\xc1\x3c\x44\x8e\x3e\x03\x23\x0f\x0d\x40\x50\x02" //.R`".<D.>.#..@P. |
// /* 09a0 */ "\xb9\xc0\x44\x70\x24\x46\x02\x96\x2a\x35\x01\x80\x05\x88\xfa\x50" //..Dp$F..*5.....P |
// /* 09b0 */ "\xe5\xf7\x0a\x02\x83\x50\x1b\x00\xa0\x8f\x05\x56\x35\x03\x09\x8d" //.....P.....V5... |
// /* 09c0 */ "\x40\x48\x07\x84\x7d\x30\x12\x80\x9d\x05\x0d\x40\x90\x07\xc8\x30" //@H..}0.....@...0 |
// /* 09d0 */ "\x18\x5a\xe0\x00\xe7\xfa\x00\x43\xef\x4c\x23\xf0\x14\x00\x11\x0f" //.Z.....C.L#..... |
// /* 09e0 */ "\x71\x84\xe0\x20\xfe\x12\x48\x02\x83\xfc\x08\x74\xfb\x80\xcf\x09" //q.. ..H....t.... |
// /* 09f0 */ "\x2c\x02\x61\xff\xe0\x29\x01\xb8\xf8\x10\x80\x2a\x1f\xf9\x82\x90" //,.a..).....*.... |
// /* 0a00 */ "\x1b\xb0\x97\x8a\x83\xc1\xde\xe3\x3a\xfe\x65\xec\x25\xef\xc0\x46" //........:.e.%..F |
// /* 0a10 */ "\x0c\x26\x74\x15\x09\x76\xfa\x0a\x27\x81\x20\x04\x33\x81\x0b\x88" //.&t..v..'. .3... |
// /* 0a20 */ "\x74\x4a\x47\x46\xa0\x20\x01\x61\x70\x20\x2e\x1a\x61\x78\x0c\xbb" //tJGF. .ap ..ax.. |
// /* 0a30 */ "\xca\xc4\x01\x40\xbe\x01\x87\x8f\xb8\x99\xf8\x51\x20\x15\x0a\xe3" //...@.......Q ... |
// /* 0a40 */ "\x0a\x79\xf9\xc8\xc0\x57\x8b\x83\xc1\x3f\x38\xf5\x1d\x09\x81\x4f" //.y...W...?8....O |
// /* 0a50 */ "\x3d\x5e\xf2\xd2\xa0\x0d\x40\xc1\x2f\x62\x20\x01\x50\x97\xcf\xc6" //=^....@./b .P... |
// /* 0a60 */ "\x65\x1e\x1a\xfa\x20\xa0\x1f\x7a\x9f\x7e\x79\xa8\x57\xe7\x98\x00" //e... ..z.~y.W... |
// /* 0a70 */ "\x40\x5f\xbe\x62\xd1\xe0\xc8\xc6\x53\x6f\x74\x10\xbc\x12\xc0\x00" //@_.b....Sot..... |
// /* 0a80 */ "\x60\x38\x15\xc8\x02\xc1\x85\xc6\x18\xaa\x08\x05\x04\x8e\x04\xe0" //`8.............. |
// /* 0a90 */ "\x1f\x87\x08\xd7\x84\xd0\x12\x00\x1e\x34\x2b\x80\x07\x2b\x98\xa0" //.........4+..+.. |
// /* 0aa0 */ "\x7a\x88\xc1\x80\x07\x1c\x04\x55\x00\x2b\x8f\xd7\x30\x79\x85\x40" //z......U.+..0y.@ |
// /* 0ab0 */ "\xc2\xdf\x6f\x5a\x2a\x00\x5f\xa2\x08\x02\xfd\xf8\x15\x1f\xe2\x90" //..oZ*._......... |
// /* 0ac0 */ "\xa8\x01\xc2\xfe\x82\x80\x03\xfc\x83\xc3\xdc\x14\xd5\x00\x41\x20" //..............A  |
// /* 0ad0 */ "\x90\x18\x0c\x00\x00\x6a\x04\xc0\x3e\x81\x83\xb0\x8f\x81\x28\xa0" //.....j..>.....(. |
// /* 0ae0 */ "\x18\x62\xa9\x01\x90\x0f\x78\x96\x20\x00\x23\x7e\x4b\x34\x80\x54" //.b....x. .#~K4.T |
// /* 0af0 */ "\x01\x91\x7f\xb1\x83\xc5\x0a\x00\x50\xdf\x20\x75\x45\xa7\xe7\x80" //........P. uE... |
// /* 0b00 */ "\x3c\x4c\x20\xf8\x8f\x12\x82\x21\x58\x08\xbd\xf7\x1e\x7e\x0a\xd1" //<L ....!X....~.. |
// /* 0b10 */ "\x08\x00\x45\x20\x4f\x61\x2d\x1d\x9b\x80\x10\x16\x00\x17\xf6\x11" //..E Oa-......... |
// /* 0b20 */ "\xf1\x38\x12\xc0\x49\x20\x07\x19\xb6\x35\x85\x30\x0f\xf1\x9d\x5c" //.8..I ...5.0.... |
// /* 0b30 */ "\xbf\x73\x12\x6d\x88\x78\xcd\x96\x02\x59\xc0\x38\xcd\xc6\x48\xfc" //.s.m.x...Y.8..H. |
// /* 0b40 */ "\x06\x40\x9e\x82\x65\xa0\x1c\x68\x22\x40\x16\x00\x15\xc8\x21\x2c" //.@..e..h"@....!, |
// /* 0b50 */ "\xfc\xee\x1b\x10\x80\x14\x00\x0f\x03\x7b\x09\xb9\x72\x38\x60\x7e" //.........{..r8`~ |
// /* 0b60 */ "\xe4\x07\xeb\x01\x3e\xf2\xe3\xf3\xd8\x77\x8d\x01\x20\x1f\x1d\x83" //....>....w.. ... |
// /* 0b70 */ "\x3e\x71\x87\x09\x84\xd3\xb8\xbf\x88\x80\x1f\x3e\xa2\xc0\x2a\x3b" //>q.........>..*; |
// /* 0b80 */ "\x86\x19\xa2\x03\x00\xc9\x27\x88\x0e\x3b\x06\x0f\xc0\x6c\x1c\x6e" //......'..;...l.n |
// /* 0b90 */ "\x23\xb6\x53\x8e\x4c\x50\x07\xbe\x62\x00\x2b\xf1\x44\x1f\x82\x2c" //#.S.LP..b.+.D.., |
// /* 0ba0 */ "\x01\x5f\x15\xc9\x98\x3f\x02\x40\xf2\xfc\x61\x1c\xe8\x60\xfc\x50" //._...?.@..a..`.P |
// /* 0bb0 */ "\x41\xf8\xa2\x00\x15\x40\x0b\x90\x85\xb0\x77\xbf\x71\xc9\x85\x20" //A....@....w.q..  |
// /* 0bc0 */ "\x00\x38\xef\x1c\x9c\x63\x60\x4a\x60\x14\x01\xb6\x11\xf8\x11\x00" //.8...c`J`....... |
// /* 0bd0 */ "\xf9\x00\x08\x9b\x00\x67\xbc\x16\x0f\xe6\x81\x30\x8f\xc0\x3c\x64" //.....g.....0..<d |
// /* 0be0 */ "\xfa\xcc\x03\xe6\x1e\xf9\xc9\x00\xa0\x8d\x16\x02\x2d\x07\x00\x0a" //............-... |
// /* 0bf0 */ "\x40\x0a\x90\x05\x30\x05\xa0\x0a\x48\x3f\x33\x0a\x6a\xb8\x18\x80" //@...0...H?3.j... |
// /* 0c00 */ "\x7c\xda\x81\xc8\x00\x40\x05\x28\x03\xcc\x00\x1e\x00\x20\x0a\xe0" //|....@.(..... .. |
// /* 0c10 */ "\x96\x06\x37\x24\x48\x02\xa0\x03\xc3\xdf\x84\x40\x02\xff\xf4\x26" //..7$H......@...& |
// /* 0c20 */ "\xc0\x16\xc0\x3e\xfb\xf8\x0c\x02\x98\x65\x16\x20\x40\x1e\x72\x0e" //...>.....e. @.r. |
// /* 0c30 */ "\x78\x84\xc0\x01\xf3\xe7\xe2\x87\xdf\x3e\xf1\x00\x2f\xd6\xb1\xf7" //x........>../... |
// /* 0c40 */ "\x8c\x03\xe3\x7c\x35\x4c\x1c\x9f\x90\x1b\x00\x58\xdf\x82\x1e\x4a" //...|5L.....X...J |
// /* 0c50 */ "\x01\x70\x3d\x40\x07\xb8\xd2\xc7\xe7\x7d\xa8\x90\x70\x8a\x79\x28" //.p=@.....}..p.y( |
// /* 0c60 */ "\x07\xf6\x14\x01\xee\x41\xf1\xeb\xe7\xd8\x80\x01\x4f\xcb\x14\x0c" //.....A......O... |
// /* 0c70 */ "\x89\xb8\xf8\xa1\xbe\x49\xac\x33\x33\x1c\x09\x9c\x2b\x42\xb8\x40" //.....I.33...+B.@ |
// /* 0c80 */ "\x2d\xc4\xb6\x20\x03\xdf\x25\xc4\x95\xe1\xa6\x01\x35\x01\xf0\x70" //-.. ..%.....5..p |
// /* 0c90 */ "\x7f\x38\xe0\xb0\x03\xe0\x1a\x1b\x80\xf9\x7c\x07\xef\x90\xdf\xcd" //.8........|..... |
// /* 0ca0 */ "\x7c\x92\x07\xc9\x70\x43\x7e\x67\xc9\x70\x9a\x01\xf1\x4e\x7d\xb7" //|...pC~g.p...N}. |
// /* 0cb0 */ "\xc9\x8a\x07\xc9\x20\x71\xaf\x7a\xe0\x37\xc1\x32\x00\xe0\x20\x1e" //.... q.z.7.2.. . |
// /* 0cc0 */ "\x7a\x7d\xb0\x17\x81\x1e\x92\x8f\xde\x02\xf3\x28\x73\xc0\x31\xc6" //z}.........(s.1. |
// /* 0cd0 */ "\x18\x15\xec\x49\xe3\x54\x81\x00\x00\xd8\x07\x87\x78\x8f\x07\xe0" //...I.T......x... |
// /* 0ce0 */ "\x56\x00\x04\x01\xe7\xef\xaf\x1c\x0b\x10\x0f\x6c\x3e\x64\x48\x07" //V..........l>dH. |
// /* 0cf0 */ "\xb8\x0c\x47\xac\x01\x5e\xc6\x70\x18\x8b\x00\xa0\x65\x2c\x80\x06" //..G..^.p....e,.. |
// /* 0d00 */ "\x0d\xc2\x94\x15\xf5\x01\x6c\x1c\x10\x4c\x00\x5f\x60\x80\x16\x18" //......l..L._`... |
// /* 0d10 */ "\xc0\x27\xf2\x23\x25\xe3\x39\x8e\xb8\xaa\x39\x8e\xbc\x6a\xe0\x79" //.'.#%.9...9..j.y |
// /* 0d20 */ "\x1c\x2c\xc3\xdf\xe5\xd1\x15\x89\xdb\xdc\x08\xc3\xd0\x1a\x00\xa7" //.,.............. |
// /* 0d30 */ "\x16\xa6\x33\xa7\x00\x0f\xf1\x09\x0f\x72\xd1\x77\x36\x73\x98\x50" //..3......r.w6s.P |
// /* 0d40 */ "\x05\x38\xba\x00\x03\x60\x40\x03\xd8\x03\x18\x3b\x02\x60\x1e\xa0" //.8...`@....;.`.. |
// /* 0d50 */ "\x71\x76\x8c\x72\xc8\xf9\x51\xdb\xf8\x00\x15\xf6\xd4\x0e\x82\xb0" //qv.r..Q......... |
// /* 0d60 */ "\x00\x13\x02\xa0\x0a\x20\x71\x85\x8e\xba\x30\x3f\x60\x0a\xc1\xe4" //..... q...0?`... |
// /* 0d70 */ "\xd0\x0d\x02\x00\x15\x90\x00\x3c\x7c\xa0\x24\x09\x80\x56\x40\x02" //.......<|.$..V@. |
// /* 0d80 */ "\xf5\xf2\x80\x30\x3e\x1f\xc1\x49\x07\x0d\xf8\x1e\x82\x10\x0f\xa0" //...0>..I........ |
// /* 0d90 */ "\x0a\xd8\x02\x98\x30\x08\x28\x00\x6a\x0b\x00\x1e\x20\x15\xa0\x05" //....0.(.j... ... |
// /* 0da0 */ "\xd8\x30\x6e\x63\xca\x60\x0a\x00\x0c\x84\x2e\x13\x20\x14\x80\x0a" //.0nc.`...... ... |
// /* 0db0 */ "\x70\x06\x0a\x48\x02\xec\x01\x44\x01\x4c\x00\x2c\x00\x17\x32\x6f" //p..H...D.L.,..2o |
// /* 0dc0 */ "\xb8\x11\x02\x01\x40\x53\x00\x03\x8f\x64\xca\x97\x17\x08\x3d\x10" //....@S...d....=. |
// /* 0dd0 */ "\x8e\x60\x9b\x78\x20\xb8\x5e\x00\x60\xf0\x2c\x47\xd2\xb5\xad\x16" //.`.x .^.`.,G.... |
// /* 0de0 */ "\x31\xd2\xd6\xad\x07\x38\x6c\xe4\x2f\x8e\x50\x51\x69\xab\x3d\x7f" //1....8l./.PQi.=. |
// /* 0df0 */ "\x09\xfa\xc7\x5d\x51\x42\x14\x7a\xee\xd3\x5f\xbf\xf0\x68\x44\x30" //...]QB.z.._..hD0 |
// /* 0e00 */ "\x0a\xc5\x46\x07\x31\xd2\xa1\xf6\xee\xd5\xf5\x27\x00\x7c\x48\xe6" //..F.1......'.|H. |
// /* 0e10 */ "\x0f\xe2\x54\x00\xe4\x41\x00\x30\x6b\xdd\xfa\xd7\x6a\x24\xb4\xd8" //..T..A.0k...j$.. |
// /* 0e20 */ "\xed\x10\x6b\x4d\xae\xd5\x50\x03\xa5\xf3\x96\x60\x10\xc7\x64\xc0" //..kM..P....`..d. |
// /* 0e30 */ "\xe5\x40\x11\x27\x1f\x51\xa0\xd6\x96\x66\x1b\x47\x09\x70\xc6\x00" //.@.'.Q...f.G.p.. |
// /* 0e40 */ "\x6f\x77\xf3\x0d\xd4\x00\xc9\xf0\xa5\x3a\x35\x7e\xef\x70\xc3\xa8" //ow.......:5~.p.. |
// /* 0e50 */ "\x8c\x03\x10\xc0\x17\x65\x0f\xac\x90\xf6\x66\x3d\x5d\xaa\x01\x49" //.....e....f=]..I |
// /* 0e60 */ "\x00\x4f\xff\x59\x30\xfa\xc7\x8f\xad\x00\x00\x2d\x15\xfb\xf8\x26" //.O.Y0......-...& |
// /* 0e70 */ "\x01\x29\xa9\x00\xd4\x08\x00\xc1\xe6\xed\x80\x34\x58\x04\xae\x46" //.).........4X..F |
// /* 0e80 */ "\xa0\x44\x0e\x11\x2b\xa0\x7e\x00\x7f\x78\x6e\x42\x06\x7f\xba\xa0" //.D..+.~..xnB.... |
// /* 0e90 */ "\x09\x1a\x1e\xc5\x72\x8b\xa5\x16\x7c\x2f\x50\x04\x51\x0c\xf2\x60" //....r...|/P.Q..` |
// /* 0ea0 */ "\x20\x53\x00\x9d\x06\x20\x60\x18\x06\xd1\x80\x01\x80\x76\x7e\x75" // S... `......v~u |
// /* 0eb0 */ "\x89\xf5\xfe\xa0\x08\x08\x02\x03\x50\x04\x8f\x03\xf7\x00\x8c\x02" //........P....... |
// /* 0ec0 */ "\x9b\x2c\x18\x08\x5c\x80\xc5\xe0\x85\x9e\xd3\x82\xff\x9d\x1b\x15" //.,.............. |
// /* 0ed0 */ "\xfb\xfc\xea\xfd\x50\x02\xb3\x80\x80\xc0\x41\x2d\x35\x00\x5b\x67" //....P.....A-5.[g |
// /* 0ee0 */ "\xa5\xd8\x05\x9a\x61\x18\xb0\x60\x41\x87\x04\x5e\x0c\x81\x10\xc0" //....a..`A..^.... |
// /* 0ef0 */ "\x16\x2a\xfd\xfe\x06\x73\x76\xab\xdd\x5f\x6f\xf5\x00\x52\x3f\xd8" //.*...sv.._o..R?. |
// /* 0f00 */ "\xc0\x2f\x93\x30\x18\xee\xee\xee\x12\xfc\x80\x43\x00\xc0\x16\x4a" //./.0.......C...J |
// /* 0f10 */ "\xfd\xfc\x17\xd4\x8f\xb2\xac\x60\x1d\xa0\xc7\xb3\xd0\x24\x9b\x54" //.......`.....$.T |
// /* 0f20 */ "\x6a\xfc\x17\xec\xe7\x9e\x64\x60\x5a\xa0\x0c\x40\x80\x03\xe3\xd4" //j.....d`Z..@.... |
// /* 0f30 */ "\x14\xa6\xd4\xe3\x74\x9b\x5d\x4e\x6e\xd7\x7b\xad\x40\x14\x30\x2f" //....t.]Nn.{.@.0/ |
// /* 0f40 */ "\xcb\xb3\x80\xa3\x21\x00\x5a\xa0\x10\xc3\xc9\x41\xfe\xdd\xc4\xd4" //....!.Z....A.... |
// /* 0f50 */ "\x0f\x23\x9c\x05\xbc\x1c\x14\xe0\xc0\xf9\x88\xc0\xaf\xb0\xcd\xff" //.#.............. |
// /* 0f60 */ "\x6e\x38\x2d\xd0\xaf\xdf\x7d\x37\x04\x59\x15\x21\xf9\x4e\x07\x77" //n8-...}7.Y.!.N.w |
// /* 0f70 */ "\x80\xae\x68\x57\xc2\xb1\xc0\xd6\x7c\x33\x0e\x0c\x36\x80\x7f\x38" //..hW....|3..6..8 |
// /* 0f80 */ "\xbe\x7b\xcf\x22\x08\x46\x02\x86\x42\x3f\xbb\x01\x49\x30\x18\xd4" //.{.".F..B?..I0.. |
// /* 0f90 */ "\x10\x30\x7c\x85\x40\x4b\xc3\xcc\x04\x40\x07\xe0\x0e\xf0\x44\x88" //.0|.@K...@....D. |
// /* 0fa0 */ "\x60\x27\x74\x23\xdd\xce\x03\x33\x80\x9d\x18\x0a\x4c\x31\xe1\x48" //`'t#...3....L1.H |
// /* 0fb0 */ "\xc0\x57\x70\xc0\x21\xe4\x43\x93\x0d\xa0\x23\xdd\xb7\xc7\xa8\x20" //.Wp.!.C...#....  |
// /* 0fc0 */ "\x83\x58\xe5\xf0\xf0\xf1\x06\x61\x80\xaa\x51\x87\x3f\xbb\x1e\x3c" //.X.....a..Q.?..< |
// /* 0fd0 */ "\x84\x1a\xc0\x77\xd7\x78\x48\x11\xe5\xb7\xc7\x79\x9c\x17\xfb\xa2" //...w.xH....y.... |
// /* 0fe0 */ "\x40\xea\xc3\x68\xc0\x36\x8f\x3e\x04\xfb\xf4\x14\x00\x40\x30\x52" //@..h.6.>.....@0R |
// /* 0ff0 */ "\x90\x81\x20\x82\x20\x83\x60\x76\x61\xb4\x04\xfa\x7d\xfe\xe2\x8c" //.. . .`va...}... |
// /* 1000 */ "\x83\x50\x81\x20\xd6\x10\x6d\x0f\x3f\xe0\x83\x50\x09\xf6\x21\x80" //.P. ..m.?..P..!. |
// /* 1010 */ "\x57\xe5\x88\x16\x3e\x59\xc3\xb5\x31\x06\x51\xd4\xee\x30\x66\xd3" //W...>Y..1.Q..0f. |
// /* 1020 */ "\x00\x10\xaa\x00\x03\x01\x58\x0b\xb9\x80\x07\xc8\x36\x00\x03\x0c" //......X.....6... |
// /* 1030 */ "\x56\xc8\xf0\x03\xb7\xb9\x06\xa0\x43\xca\xb5\x80\x18\x40\x01\xd0" //V.......C....@.. |
// /* 1040 */ "\x34\x1b\x46\x00\x3a\x0c\x72\xa8\x79\xcc\x00\x19\x80\xad\xa2\x00" //4.F.:.r.y....... |
// /* 1050 */ "\x00\x51\xca\x6d\xe9\x13\x00\x6b\x0d\xa2\x09\xe2\xb4\xc3\x01\x60" //.Q.m...k.......` |
// /* 1060 */ "\x90\x03\xbd\x64\x60\x11\x61\xb4\x7e\x28\x84\x00\x10\x70\x82\x08" //...d`.a.~(...p.. |
// /* 1070 */ "\x72\xfd\x5a\x3c\x32\x20\x88\x75\x9d\x78\x50\xc0\x28\xc3\x68\x0b" //r.Z<2 .u.xP.(.h. |
// /* 1080 */ "\xf1\xfb\xa7\xc3\xc0\x8f\x8a\x58\xf8\xa0\x02\x8f\x29\x45\x20\x81" //.......X....)E . |
// /* 1090 */ "\x22\x08\x83\x28\x41\x9c\x60\x30\x35\xa3\xa9\x13\x01\xea\x5f\xf7" //"..(A.`05....._. |
// /* 10a0 */ "\x63\x60\xc6\x09\xc7\x60\x39\x80\x86\x0d\xfb\x58\x80\xa4\xa0\x47" //c`...`9....X...G |
// /* 10b0 */ "\xa0\x0a\x80\x00\x62\x78\x14\x59\xc9\xda\x84\x41\x72\x7d\xd8\x82" //....bx.Y...Ar}.. |
// /* 10c0 */ "\xe1\x01\xc0\x57\xf1\x40\x0a\x8e\x84\xd0\xa3\x81\x34\x27\xc0\x9e" //...W.@......4'.. |
// /* 10d0 */ "\xc4\x27\xa0\x9a\x12\xc8\x4c\x37\x00\x3f\xdf\x5d\xd5\xc0\x01\x3c" //.'....L7.?.]...< |
// /* 10e0 */ "\x46\x62\x94\xa4\xc6\xe2\xc7\x6e\x5a\x09\x3a\xc3\x8b\x6c\x01\x70" //Fb.....nZ.:..l.p |
// /* 10f0 */ "\x60\xc0\x64\xe8\x14\x74\xca\xfd\xca\x76\x20\xf7\xea\xf0\xdc\x76" //`.d..t...v ....v |
// /* 1100 */ "\x26\x85\x0b\xc0\x14\x40\x1c\x52\x7c\xa2\x57\xf1\xa3\x40\x52\x4a" //&....@.R|.W..@RJ |
// /* 1110 */ "\x03\x03\x99\x61\x9e\x52\xd6\x02\x4b\x40\x97\x52\xe3\xf7\x76\xbe" //...a.R..K@.R..v. |
// /* 1120 */ "\x58\xa9\xe9\xd3\x17\x74\xb7\x6d\xb0\x1c\x04\x60\x06\xa0\x06\xdc" //X....t.m...`.... |
// /* 1130 */ "\xd7\xc6\xa2\x20\x2a\xc6\x5d\x3a\xc3\xa4\x4f\xf6\xf6\xcf\xf2\x02" //... *.]:..O..... |
// /* 1140 */ "\x63\x59\x01\x80\xc7\x7f\x11\x87\xe4\x27\xf7\x7a\x80\x24\x1f\xa2" //cY.......'.z.$.. |
// /* 1150 */ "\xfa\x2d\x23\xe9\x02\xaf\x43\x2a\x00\x72\x01\x00\x18\x48\xfd\x7e" //.-#...C*.r...H.~ |
// /* 1160 */ "\x7c\xb6\x7a\x44\x3a\x35\x1d\x32\x9f\x2d\xbf\x49\x63\xf6\xff\xea" //|.zD:5.2.-.Ic... |
// /* 1170 */ "\x03\x20\xb7\xf8\x3c\x42\x56\x03\x07\xaf\xdf\xe7\xa6\x4f\x41\x34" //. ..<BV......OA4 |
// /* 1180 */ "\xf0\x07\xeb\xfd\x7e\xff\x36\xa6\x60\x28\xc0\x23\x0e\x0b\x03\x47" //....~.6.`(.#...G |
// /* 1190 */ "\x4c\xb0\x10\x82\x05\x5d\x11\x1e\x6c\xf0\x18\x47\x93\x0a\xc1\x76" //L....]..l..G...v |
// /* 11a0 */ "\x01\x80\xd7\x98\xfc\x9a\x81\xc8\xad\x0d\xb7\x3b\xad\x93\xf4\x80" //...........;.... |
// /* 11b0 */ "\xc8\x39\x7f\xa7\xf5\x66\xdf\x1e\x01\x08\xd6\x40\x70\x19\x22\xdf" //.9...f.....@p.". |
// /* 11c0 */ "\xc7\xee\x20\x4c\x56\x08\xe0\x4e\x10\x6f\x01\xa0\x60\x3f\x84\xfe" //.. LV..N.o..`?.. |
// /* 11d0 */ "\x9d\x4a\x80\x25\x54\x00\x90\x0c\xfc\x06\x00\x2b\xfc\x82\x03\x90" //.J.%T......+.... |
// /* 11e0 */ "\x16\x32\x9b\x54\x06\x74\xb5\xf8\xaf\xd4\xaa\x80\x12\x15\x37\xc6" //.2.T.t........7. |
// /* 11f0 */ "\x4f\x41\x3c\x22\xa8\x01\x58\x5a\x80\x68\x77\x61\x58\x0c\x7e\xef" //OA<"..XZ.hwaX.~. |
// /* 1200 */ "\xd4\xbd\x3f\xa7\x37\xfe\x08\xa0\x11\xf9\xb6\x1a\xf0\xcb\x9c\x54" //..?.7..........T |
// /* 1210 */ "\x01\x24\x05\x7c\xb4\xe1\x74\xab\xd4\xc1\xe8\xa8\x7f\x87\x0d\x3e" //.$.|..t........> |
// /* 1220 */ "\x1c\x30\x18\x80\xd4\x01\x6c\x68\x20\x30\x11\xd4\x30\x0f\x9e\xb1" //.0....lh 0..0... |
// /* 1230 */ "\xd5\xc0\x60\x2f\xd1\xda\x2a\x15\xda\xa0\x09\x61\x85\xf8\x50\x40" //..`/..*....a..P@ |
// /* 1240 */ "\x11\xf5\x40\x15\xff\xc8\x80\xc9\x40\x60\x27\xa9\x6c\x00\x5e\x2f" //..@.....@`'.l.^/ |
// /* 1250 */ "\x89\x01\xcf\x98\x5e\xf1\x58\x3c\x3e\x04\x80\x5c\x3f\x42\x40\xf3" //....^.X<>...?B@. |
// /* 1260 */ "\xf2\x84\x6d\x3e\x29\xa2\x3e\x7e\x88\xe1\xd7\x91\x95\xd3\xaa\x80" //..m>).>~........ |
// /* 1270 */ "\x1b\x00\x40\x4c\x06\xbf\x56\xa8\x01\x58\x0e\xfc\x00\x22\xe0\x9e" //..@L..V..X...".. |
// /* 1280 */ "\x5b\x01\x80\xbf\xdb\xf0\x1e\x38\xef\xd7\xc6\x0f\x0f\x80\xc0\x65" //[......8.......e |
// /* 1290 */ "\x80\x30\x27\xdb\xfa\xb9\xcb\x00\x1f\x87\x22\xc3\x03\xe0\xd5\x10" //.0'......."..... |
// /* 12a0 */ "\x37\xfa\x30\xf4\xfb\xf3\xc6\x44\x36\x70\x19\x3d\x7c\x50\x01\xcf" //7.0....D6p.=|P.. |
// /* 12b0 */ "\xec\x7f\x14\x0a\x25\x74\x12\x61\x60\x04\xfd\x80\x44\x18\x18\x0b" //....%t.a`...D... |
// /* 12c0 */ "\x37\x09\x65\xfe\xbd\xe7\x94\x33\xf1\xbd\x54\xa8\x01\x48\x56\x80" //7.e....3..T..HV. |
// /* 12d0 */ "\x60\x75\xbb\xf5\x0e\xd1\x6d\x84\x30\x02\xf0\xc3\x1e\xa5\xd4\x40" //`u....m.0......@ |
// /* 12e0 */ "\x0a\x40\x60\x51\xfa\x77\x8f\x40\x8c\x02\x1f\x99\x60\xac\x80\xd4" //.@`Q.w.@....`... |
// /* 12f0 */ "\x00\x48\xee\xc0\x30\x45\x9b\xb1\x25\xa5\x86\xd1\x28\x98\x30\x03" //.H..0E..%...(.0. |
// /* 1300 */ "\xc7\x42\x60\xe3\xa8\x77\xe8\x60\xf4\xd9\xfe\x8c\x7f\x3e\x5e\x8e" //.B`..w.`.....>^. |
// /* 1310 */ "\xbe\xa9\xb3\xff\xa2\xe9\x55\x12\x83\x13\xe2\x72\x95\x5a\x98\xf9" //......U....r.Z.. |
// /* 1320 */ "\x2c\x98\xea\x0c\x03\x71\xd0\x96\x5f\x94\x7a\x11\x16\xc0\x54\xca" //,....q.._.z...T. |
// /* 1330 */ "\x45\x1c\x02\x06\xf5\x80\xcf\xcf\x81\x23\xb0\x13\x51\x9c\x0a\x35" //E........#..Q..5 |
// /* 1340 */ "\x1c\x96\x10\x60\x2c\x12\xd2\x69\x00\x96\x80\x5c\x70\x22\xa0\x57" //...`,..i....p".W |
// /* 1350 */ "\x03\xa7\x2a\x01\x18\x08\xea\x08\xb0\x06\x8c\x41\x00\x70\x05\x62" //..*........A.p.b |
// /* 1360 */ "\xe1\x81\x2a\xc2\x30\x24\x19\xc6\x04\x7c\x8c\x01\x93\x30\x36\x70" //..*.0$...|...06p |
// /* 1370 */ "\x0b\x90\x03\xe0\x4e\x03\xdf\x3c\x60\x49\xd8\x96\x80\x76\x0e\x40" //....N..<`I...v.@ |
// /* 1380 */ "\xc8\xb0\x1c\x39\xec\x24\x33\x03\x86\xb0\x23\x8c\x5c\xb0\x07\x14" //...9.$3...#..... |
// /* 1390 */ "\x60\x23\x95\xcb\x0c\x7d\x10\x6c\x18\x07\xb2\x30\x23\x44\x1b\x84" //`#...}.l...0#D.. |
// /* 13a0 */ "\xe3\xaf\xd8\x46\xc0\x2e\xea\x2f\x1c\xb7\x10\x42\x46\x05\x94\x7d" //...F.../...BF..} |
// /* 13b0 */ "\x83\xe0\x4f\x63\x07\x1f\x65\x45\xbc\x27\xd9\xae\x86\x6c\x08\xca" //..Oc..eE.'...l.. |
// /* 13c0 */ "\xf2\x24\x12\x00\x61\xac\x46\x01\xa7\x80\x5b\x0f\x0a\x90\x17\x46" //.$..a.F...[....F |
// /* 13d0 */ "\xe6\x04\x71\x60\x56\x28\xec\xa4\xf2\x5b\x81\xcb\x86\xc1\x06\xa1" //..q`V(...[...... |
// /* 13e0 */ "\xe0\x91\x46\x05\xb1\xaa\x78\x06\x00\x77\x60\x57\x85\x40\x0a\xcd" //..F...x..w`W.@.. |
// /* 13f0 */ "\xfc\x40\x6c\x2b\x1d\x23\x21\xf3\xc4\x41\x40\x9c\x51\x17\x15\x00" //.@l+.#!..A@.Q... |
// /* 1400 */ "\x21\xcf\x10\x81\x08\x29\x18\x17\x22\x96\x42\x7b\x45\xc6\x06\x87" //!....)..".B{E... |
// /* 1410 */ "\x20\xd6\x38\x35\xbc\x98\xb9\x78\xda\x7e\xa3\xa8\x92\x21\xf4\xdd" // .85...x.~...!.. |
// /* 1420 */ "\x77\x12\xdf\x0b\x30\x24\x63\x88\x68\x82\x50\x03\xe3\x46\x05\x7f" //w...0$c.h.P..F.. |
// /* 1430 */ "\x8f\x11\xb0\x38\x88\x17\xd5\x4f\x73\x05\xf0\x19\x93\xc3\x40\x66" //...8...Os.....@f |
// /* 1440 */ "\x18\x87\x8b\x23\x03\x15\xc1\xb0\x4e\x23\x24\x8f\x8e\x16\xc0\x65" //...#....N#$....e |
// /* 1450 */ "\x20\x00\x8c\x05\xd8\xf0\xb4\x60\x72\x20\x3f\xfc\x70\x03\x8f\x92" // ......`r ?.p... |
// /* 1460 */ "\xf7\xc5\x7c\x97\xbb\x62\xe6\x01\xfc\x67\xdc\x51\x81\xc1\xc1\x1e" //..|..b...g.Q.... |
// /* 1470 */ "\x6d\x70\x6d\x8f\x92\xc0\xdf\x78\x83\xe0\x2b\xb6\x06\x06\x8f\x92" //mpm....x..+..... |
// /* 1480 */ "\xc0\xbc\xf1\x81\x41\xdd\x19\x42\x00\x38\x01\x3d\x81\xbc\x24\x50" //....A..B.8.=..$P |
// /* 1490 */ "\x24\x7a\x03\xe8\x40\x41\x07\x47\x87\x80\xc2\xd8\x13\x34\x3d\xc9" //$z..@A.G.....4=. |
// /* 14a0 */ "\x75\x51\x20\x84\x7a\x53\x0c\xaa\x52\xeb\xb1\x00\xc8\x31\x8f\x49" //uQ .zS..R....1.I |
// /* 14b0 */ "\xd6\x06\xf6\x00\x7d\x18\x05\x20\xfa\x2f\x4b\x44\xb3\xd4\x8f\xa3" //....}.. ./KD.... |
// /* 14c0 */ "\x00\x98\x1d\x52\x08\xfe\x8b\xd2\xcc\x0d\x36\xec\x5e\xf5\x3b\xc4" //...R......6.^.;. |
// /* 14d0 */ "\xef\x4c\xd0\xef\x79\xc8\x55\x00\x03\x67\xe7\xcd\x8b\x28\x2a\xb7" //.L..y.U..g...(*. |
// /* 14e0 */ "\x3e\x66\x40\x25\x40\x07\xa8\x01\x58\x02\x45\x80\xf7\x9d\xbd\xd8" //>f@%@...X.E..... |
// /* 14f0 */ "\x05\xae\x02\xbe\x4a\x81\x80\xef\xd8\x0f\x40\x09\xf3\x83\x5c\x61" //....J.....@....a |
// /* 1500 */ "\x43\xc0\x30\x39\x31\x8b\xac\xb5\x76\xdf\xcf\x12\x31\x5d\x98\x07" //C.091...v...1].. |
// /* 1510 */ "\x13\xb9\xbe\x5c\x00\xf3\xcf\x62\x5d\xd7\xcb\x8c\x73\x05\x18\xa0" //.......b]...s... |
// /* 1520 */ "\x6d\xf2\xd0\x23\x59\x29\x00\x08\xd7\x2f\x87\x65\x14\x98\xc6\x7d" //m..#Y).../.e...} |
// /* 1530 */ "\x77\xe3\xec\xed\xb2\xe5\x0c\xe0\x5c\x03\xc1\xea\xea\x80\x0f\x3e" //w..............> |
// /* 1540 */ "\x15\x43\x0e\x83\x38\x54\xb0\xf4\xe4\x7d\x78\x81\x08\x00\xe1\x47" //.C..8T...}x....G |
// /* 1550 */ "\xc0\x00\x02\x00\xf8\xcc\x05\x80\x7f\x97\x7b\xd2\xa7\x59\x6f\x05" //..........{..Yo. |
// /* 1560 */ "\xd7\x27\x48\x85\x29\x7c\x55\xad\x6b\xfc\xa8\xbc\x95\xc0\x99\x31" //.'H.)|U.k......1 |
// /* 1570 */ "\x1d\x50\x04\x9d\x55\xbe\xa3\xe5\x20\x73\xfc\x3d\x1e\x00\x1e\xc0" //.P..U... s.=.... |
// /* 1580 */ "\x87\x6e\x68\x2e\xec\x91\x87\x1a\x98\x81\xb1\x78\x9e\xb1\x5d\xf2" //.nh........x..]. |
// /* 1590 */ "\x67\x9b\xff\xce\x10\x21\x73\x4b\x12\x00\x54\x03\x59\x4a\x48\x41" //g....!sK..T.YJHA |
// /* 15a0 */ "\x32\x5b\xc1\x94\xa6\x80\x29\x81\x40\x9e\x60\x89\xfd\xde\x44\x21" //2[....).@.`...D! |
// /* 15b0 */ "\x81\x1f\x55\x00\x47\xdb\x78\x0c\x0a\x3b\xd3\x61\x50\x03\xa1\x79" //..U.G.x..;.aP..y |
// /* 15c0 */ "\x89\xe0\x68\xa8\x02\x4f\xee\xe0\x30\x2a\x5a\xd3\x00\x58\xd7\x31" //..h..O..0*Z..X.1 |
// /* 15d0 */ "\x10\xf7\x82\x44\x78\xd5\xf5\x1e\xbb\xd4\x01\xa4\x5c\xa6\x58\x41" //...Dx.........XA |
// /* 15e0 */ "\xbd\xcf\x3a\x80\xd4\x79\xa4\x48\x1a\x0f\x8a\xc0\x96\xa5\xa0\x18" //..:..y.H........ |
// /* 15f0 */ "\x11\x8c\x10\x83\xde\x80\x24\x0f\x26\x80\xd2\xbd\xa8\xfb\xac\x21" //......$.&......! |
// /* 1600 */ "\x12\x01\xa8\x03\x7b\x0d\xc0\x2c\x75\xf5\xdf\x2a\x7c\xaf\x13\x69" //....{..,u..*|..i |
// /* 1610 */ "\xad\x00\x03\xf6\x28\x01\x7f\x41\xed\x08\x06\xf6\x85\x82\xe1\xe9" //....(..A........ |
// /* 1620 */ "\xb7\x09\x25\xbc\x06\x03\x16\x63\xc0\xaa\x15\xf5\x40\x64\x64\x38" //..%....c....@dd8 |
// /* 1630 */ "\x6e\x2a\x38\x60\xb0\x2c\x7c\x54\x00\x60\x1c\x67\x40\x9e\x47\x54" //n*8`.,|T.`.g@.GT |
// /* 1640 */ "\x7d\x99\x00\xbe\x72\x96\x35\x10\x18\x12\xf4\x77\xaa\x48\x09\x9f" //}...r.5....w.H.. |
// /* 1650 */ "\xee\x80\xda\x9a\xc0\x7d\x5e\x61\x08\x90\x60\x5f\x16\x51\x7c\x00" //.....}^a..`_.Q|. |
// /* 1660 */ "\xd4\x03\x3d\x84\xf0\xf2\x0b\x56\x01\x80\xcf\x44\x79\xd0\xcc\x06" //..=....V...Dy... |
// /* 1670 */ "\xff\x56\xe9\xde\xa8\x00\xd3\xff\x83\xca\x73\x89\x80\xde\xdd\x00" //.V........s..... |
// /* 1680 */ "\xd8\x30\x04\x0b\xc6\x91\x43\x18\x01\xa8\xf8\x6e\x01\x56\xbd\x16" //.0....C....n.V.. |
// /* 1690 */ "\xfd\x20\x34\x0e\x74\x0a\xfd\xfe\xfb\xd6\xca\x4d\xb9\xeb\x8f\x6b" //. 4.t......M...k |
// /* 16a0 */ "\xe0\x30\x2c\x48\x34\xf7\x2c\x20\x0f\xf8\xd1\xe5\x17\x8d\x18\x98" //.0,H4., ........ |
// /* 16b0 */ "\x30\x01\x42\x20\x00\x81\xe2\x5d\xd8\xb1\x8c\x54\x01\xb8\x50\x3d" //0.B ...]...T..P= |
// /* 16c0 */ "\x78\x01\x01\x81\x3f\xe1\x2b\x27\xfb\x54\x01\x2a\x3c\x57\x82\x10" //x...?.+'.T.*<W.. |
// /* 16d0 */ "\x4c\x0c\x2e\x81\x50\x05\x4a\x10\xb8\x08\x54\x39\xcd\x8b\x13\x80" //L...P.J...T9.... |
// /* 16e0 */ "\xa9\x6b\x44\x2c\x48\xa5\x30\x1c\x3f\x5f\x40\x30\x38\xf3\xe4\xf4" //.kD,H.0.?_@08... |
// /* 16f0 */ "\xea\x3c\xf9\xc4\x15\x03\x3f\x9f\x1e\xa5\x8e\xa1\xd4\x01\x31\xa2" //.<....?.......1. |
// /* 1700 */ "\x3b\x0e\xbe\x18\x30\x32\x10\x00\x7e\x2d\x35\xea\xb8\xe2\xba\xc0" //;...02..~-5..... |
// /* 1710 */ "\x02\x20\x8a\x80\x15\x1c\xe4\x07\xb3\xc2\x84\xc0\x11\x80\x71\x50" //. ............qP |
// /* 1720 */ "\x36\xee\xdd\x88\x07\x0d\xa8\x80\xc4\x20\x81\x75\x00\x1f\x10\xb8" //6........ .u.... |
// /* 1730 */ "\xe7\xae\x91\xc0\x98\x0c\x7a\x4b\x80\x37\xd1\x4e\x71\x00\x16\xac" //......zK.7.Nq... |
// /* 1740 */ "\x42\x92\x40\xe3\x68\x2b\x5c\x0b\xa8\x01\x3f\x76\x2d\x01\x3d\x70" //B.@.h+....?v-.=p |
// /* 1750 */ "\x34\x3f\x3e\x74\x7d\x04\x00\xc1\x07\xeb\xe7\xa4\x51\xd0\x98\xea" //4?>t}.......Q... |
// /* 1760 */ "\x9d\x40\x17\xb3\x02\x08\xfc\xf9\xca\x1c\x01\x60\xc0\xe6\x0b\x19" //.@.........`.... |
// /* 1770 */ "\x1d\x0a\xe7\x1c\xa8\x01\xe2\x99\x58\x9f\x5d\xe8\x9f\x2c\x04\xb5" //........X.]..,.. |
// /* 1780 */ "\x4e\xc0\x36\xe2\x1f\xac\x83\xc7\x42\x5f\x4a\xaf\xd0\xc3\xe9\xf0" //N.6.....B_J..... |
// /* 1790 */ "\xf4\xda\x7d\x04\xbe\x8c\xe0\x9c\xd4\x2a\xfa\x32\xbc\xe9\x5e\x9a" //..}......*.2..^. |
// /* 17a0 */ "\xbf\xd1\xa3\xd2\x53\xd4\xb8\xe8\x4c\x70\x25\xa3\x02\xb6\x39\x0e" //....S...Lp%...9. |
// /* 17b0 */ "\x85\x28\x45\x8c\x75\x00\x3a\x7b\xdf\x00\xe8\x91\x62\x0e\xc6\x08" //.(E.u.:{....b... |
// /* 17c0 */ "\x7e\x8c\x0e\x18\x82\xf1\xfa\x01\x60\x7a\xb5\x40\x12\xf7\x5e\x01" //~.......`z.@..^. |
// /* 17d0 */ "\xa8\xf9\xd2\x1b\x5a\xe9\xef\x78\x21\xc9\x02\x8c\xe0\x7d\xa0\xcf" //....Z..x!....}.. |
// /* 17e0 */ "\x02\x57\x08\x77\x06\x98\xa8\xb3\x70\x49\x0c\x65\x21\x56\x08\x72" //.W.w....pI.e!V.r |
// /* 17f0 */ "\x86\x4f\x53\xc6\x0b\x57\x7d\xc8\xca\x5b\x2a\x8c\xf6\xee\x03\x8f" //.OS..W}..[*..... |
// /* 1800 */ "\xf2\x02\xb8\x1f\xaa\x72\x56\xa3\x84\x5c\x51\x27\xd7\x7a\x8c\x4a" //.....rV...Q'.z.J |
// /* 1810 */ "\x28\x03\x89\xbc\x42\xb0\x14\xfe\x14\x40\x29\x00\x08\x0f\x50\x54" //(...B....@)...PT |
// /* 1820 */ "\x0b\x12\x5f\xc0\xa4\x05\x53\xd2\xe0\x01\x57\x00\x0c\x01\x98\x59" //.._...S...W....Y |
// /* 1830 */ "\xf2\xf3\x03\x47\x01\x38\xe7\x88\x87\xc6\x70\x24\xbb\x02\x32\x01" //...G.8....p$..2. |
// /* 1840 */ "\x20\x24\xfe\x22\x8c\x09\x2b\x00\x98\x23\x7e\x68\x1c\xb9\x60\x47" // $."..+..#~h..`G |
// /* 1850 */ "\x03\xc5\xb2\x23\xd0\x12\x02\x2d\xff\x8c\x18\x23\x95\x1c\xc1\x1c" //...#...-...#.... |
// /* 1860 */ "\xab\xd8\x4e\x07\x93\x0f\x60\x8f\xd8\xc1\x1c\xb1\x63\xfe\xf3\x00" //..N...`.....c... |
// /* 1870 */ "\xc0\xee\x33\x4f\xf0\xea\x91\x82\x35\xd4\x00\x1f\x14\x3a\x44\x04" //..3O....5....:D. |
// /* 1880 */ "\xe0\x34\xdf\x00\x88\x02\xc4\x20\x48\x9d\x2a\xc2\x50\x19\x97\x81" //.4..... H.*.P... |
// /* 1890 */ "\xa3\x02\x6d\x47\x3a\x90\x94\x06\x6f\xe4\x74\x80\x57\x02\x6b\xba" //..mG:...o.t.W.k. |
// /* 18a0 */ "\x0f\x84\x60\x33\xcf\x0b\x00\x07\xc4\xba\x05\x3d\xce\x80\xc1\x7c" //..`3.......=...| |
// /* 18b0 */ "\x2d\x24\x02\x94\xdb\x00\x9e\x2a\x3f\x73\xf9\x08\xc0\x61\x43\xac" //-$.....*?s...aC. |
// /* 18c0 */ "\x01\x0c\x23\xe0\xbe\x37\x08\xc0\x5b\xc5\x30\x28\xe0\x07\x9d\xd0" //..#..7..[.0(.... |
// /* 18d0 */ "\x42\x02\xe8\x27\xf2\xd8\x5e\xb5\x80\x60\x4f\x38\x28\x4a\x11\x40" //B..'..^..`O8(J.@ |
// /* 18e0 */ "\x30\x50\x0c\x11\x0f\xfd\x69\x7a\xfc\x24\x6c\x7d\x6b\x17\x1f\xbe" //0P....iz.$l}k... |
// /* 18f0 */ "\x2b\x15\xf9\x26\x9f\x92\x30\xc1\x49\x90\x05\xf2\x42\xb8\x29\x31" //+..&..0.I...B.)1 |
// /* 1900 */ "\xf9\x8b\x21\x21\xf0\x11\xa0\xa0\x00\xfe\x8d\xa7\x0f\xf8\xb1\xff" //..!!............ |
// /* 1910 */ "\x18\xa7\x7d\xf5\xa3\x05\x3e\xc4\x2b\xe4\xad\xbf\xc6\x11\xfe\x4a" //..}...>.+......J |
// /* 1920 */ "\xa5\xfc\x5d\x10\xc2\x10\x1a\xbf\x8a\xe3\x05\xaa\x20\x35\xb0\x90" //..]......... 5.. |
// /* 1930 */ "\x0c\x6e\xc3\x2a\xc0\x00"                                         //.n.*.. |
// Sent dumped on RDP Client (5) 6454 bytes |
// send_server_update done |
// GraphicsUpdatePDU::init::Initializing orders batch mcs_userid=0 shareid=65538 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[1](30) used=1080 free=15198 |
// order(13 clip(145,200,110,1)):memblt(cache_id=2 rect(930,677,64,61) rop=cc srcx=0 srcy=0 cache_idx=87) |
// front::draw:draw_tile((0, 704, 32, 32) (32, 32, 32, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[2](2064) used=1086 free=15192 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[3](30) used=1108 free=15170 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(0,704,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// Widget_load: image file [./tests/fixtures/ad8b.png] is PNG file |
// front::draw:draw_tile((100, 100, 26, 32) (80, 50, 26, 32)) |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[4](1808) used=1123 free=15155 |
// <Serializer 0x7f0e407b6010> RDPSerializer::reserve_order[5](30) used=1438 free=14840 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(100,100,26,32) rop=cc srcx=0 srcy=0 cache_idx=1) |
// Front::end_update |
// GraphicsUpdatePDU::flush_orders: order_count=6 |
// send_server_update: fastpath_support=yes compression_support=yes shareId=65538 encryptionLevel=0 initiator=0 type=0 data_extra=6 |
// Sending on RDP Client (5) 1100 bytes |
// /* 0000 */ "\x00\x84\x4c\x80\x21\x45\x04\xc9\xdd\xa2\xbc\x1f\x84\xe8\x79\x45" //..L.!E........yE |
// /* 0010 */ "\x20\x8a\xe0\x18\x05\x88\x90\x03\x06\x81\x08\x24\xa5\x06\x31\x96" // ..........$..1. |
// /* 0020 */ "\x31\xa4\xe2\x6d\xa6\x20\xf2\x02\x10\xaa\xcb\x78\xd5\x31\x33\x9a" //1..m. .....x.13. |
// /* 0030 */ "\xa0\x0c\xd3\xc5\x71\xd0\x9a\x81\x7a\x13\x44\xfa\xef\x43\xfb\x2a" //....q...z.D..C.* |
// /* 0040 */ "\xbc\xf4\x7f\xa0\xa1\x76\xdf\x81\x4b\x99\x23\x89\xdd\x55\xca\x09" //.....v..K.#..U.. |
// /* 0050 */ "\xc9\x9e\x3a\x0b\xc0\x2f\x41\x78\x03\xe3\x86\x80\x9e\xd9\xfa\x1f" //..:../Ax........ |
// /* 0060 */ "\xb0\xe5\xe4\x30\x84\x0e\x59\xf8\xf0\x3f\xb9\x00\x13\xee\x3a\x02" //...0..Y..?....:. |
// /* 0070 */ "\x30\x00\x5c\xfd\xb5\x40\x0f\x87\x43\x89\xdc\x8c\xf4\x7b\xa9\x73" //0....@..C....{.s |
// /* 0080 */ "\x9f\x2a\xd9\x82\x3f\x5b\xed\xef\x74\xbc\x60\x6c\xc0\xa1\x8c\x01" //.*..?[..t.`l.... |
// /* 0090 */ "\xb4\xf1\xf0\x13\xa5\x7b\xa0\x2c\xfc\xd9\xfd\x6e\x87\xb3\x8a\x7d" //.....{.,...n...} |
// /* 00a0 */ "\x9f\xe5\xcb\x6a\x01\x26\xd3\x5c\x73\x79\x18\xa0\x75\xc9\xce\x00" //...j.&..sy..u... |
// /* 00b0 */ "\xe4\x5f\xda\x80\xf0\xb1\x23\xa0\xb4\xf4\x0c\xbe\x76\x68\x0c\x39" //._....#.....vh.9 |
// /* 00c0 */ "\xcf\xf9\xb8\xcb\xaa\x00\x98\x68\x39\xe4\xea\xf3\xe0\x11\xc7\xc4" //.......h9....... |
// /* 00d0 */ "\x11\x89\xd5\x5c\xe3\x67\x8d\xc4\xfc\xa9\xc4\xed\xcf\xde\xf1\xc3" //.....g.......... |
// /* 00e0 */ "\xc2\x94\xbb\x6f\xe8\xd1\x00\x76\x2c\x1c\x1c\xf4\x1b\x12\x74\x47" //...o...v,.....tG |
// /* 00f0 */ "\x85\x67\x8e\x64\xf8\x1e\x3f\x2c\xac\xd3\xbf\x9b\xd3\x2e\x89\xf4" //.g.d..?,........ |
// /* 0100 */ "\x0d\xed\x68\x03\x29\xc0\x3d\x2f\x28\x80\xbc\x5f\x60\x7d\x06\xe0" //..h.).=/(.._`}.. |
// /* 0110 */ "\x17\xf2\x4f\x91\x9d\x24\x03\x3f\x7a\x58\x7f\x80\x0e\xc9\xe6\x57" //..O..$.?zX.....W |
// /* 0120 */ "\xcd\x02\xfb\x21\xea\x59\xf1\xfd\xfa\x05\xf5\xe0\x13\xc4\x4e\xa9" //...!.Y........N. |
// /* 0130 */ "\xb3\xef\x71\x21\x0f\x24\xb7\xdd\x80\x4a\x47\x55\x80\x2b\x42\x57" //..q!.$...JGU.+BW |
// /* 0140 */ "\xca\x7d\xf9\x9f\x5d\xfe\x53\xa1\x02\xa0\x09\xc2\x01\x50\x04\xaf" //.}..].S......P.. |
// /* 0150 */ "\xdf\xc0\x71\x5b\x20\x67\x16\x4f\xa8\x5c\xd8\x1e\x9c\x90\x57\xcf" //..q[ g.O......W. |
// /* 0160 */ "\x6a\xc7\x98\x20\x0f\xc6\xb0\x08\xf5\xec\xc0\x21\x9d\x9f\xec\xba" //j.. .......!.... |
// /* 0170 */ "\x2e\x86\x7d\x6c\xa1\x63\xb4\x03\x08\x45\x40\x0e\x4b\xe9\x97\x8d" //..}l.c...E@.K... |
// /* 0180 */ "\xe3\x0d\xfb\xaa\x00\x8b\x34\xef\xe0\x3d\xfc\xce\x01\xf1\xf5\xe0" //......4..=...... |
// /* 0190 */ "\x0c\xb1\x8f\xe6\xbd\x44\xc5\x94\x61\x9c\x4f\xf0\xc0\x04\xce\x12" //.....D..a.O..... |
// /* 01a0 */ "\xc0\x43\x38\x20\x0c\x12\x7d\x8a\x11\x8c\x3d\x10\x55\x00\x5a\x5a" //.C8 ..}...=.U.ZZ |
// /* 01b0 */ "\xd6\x02\xc1\x04\xfb\x05\xf3\x5e\x33\x88\x26\x27\x6e\x89\x31\x8e" //.......^3.&'n.1. |
// /* 01c0 */ "\x23\x16\x70\x96\x88\x0d\xd2\x38\x33\x89\xfe\x10\x21\x06\x3f\x17" //#.p....83...!.?. |
// /* 01d0 */ "\xc4\x39\xcc\x05\x8f\x87\x5b\x80\xe8\x13\x89\x84\x4d\xa6\xb5\x00" //.9....[.....M... |
// /* 01e0 */ "\x4e\x74\x4a\x80\x2f\x11\x17\x40\xe6\x26\x35\x7b\xa6\x28\x8c\x38" //NtJ./..@.&5{.(.8 |
// /* 01f0 */ "\x14\xfd\xe1\x96\x17\x10\x73\xe4\x9e\x25\x6a\x80\x2e\x3b\xbd\x54" //......s..%j..;.T |
// /* 0200 */ "\x01\x25\xbc\x1f\x01\x1c\x05\xc0\x18\x97\x9d\xdb\xcd\xc2\x05\xc7" //.%.............. |
// /* 0210 */ "\xf1\x4b\x5a\xa8\xf0\x0c\x65\x35\x50\x21\x0e\x02\xb9\x70\x16\x8e" //.KZ...e5P!...p.. |
// /* 0220 */ "\x84\x08\x11\x4e\x08\x0b\x54\x01\x7d\xde\xed\xf6\xc0\x25\xce\x7e" //...N..T.}....%.~ |
// /* 0230 */ "\xec\x0c\xfc\x63\x01\xc1\xd2\x62\x00\x06\xa4\xa8\x96\x55\xb8\x1c" //...c...b.....U.. |
// /* 0240 */ "\x51\xa6\xd3\x57\x1c\x4e\x82\xc0\x06\x6e\x83\x0a\x25\xce\x39\x08" //Q..W.N...n..%.9. |
// /* 0250 */ "\xc5\x8a\x2c\x08\x86\x97\x63\x56\xc0\xc5\x19\x56\x5b\x87\x22\x99" //..,...cV...V[.". |
// /* 0260 */ "\x15\xd9\x42\xa7\xbd\xe2\xcd\x00\x00\xed\x08\x42\x09\xc2\x21\x82" //..B........B..!. |
// /* 0270 */ "\xc0\x58\x1e\xe9\x89\x94\x10\x16\xa4\xac\x93\x3a\x83\x18\xc0\x6d" //.X.........:...m |
// /* 0280 */ "\x8b\x57\x39\xe6\x0a\x60\x51\x2c\x02\x70\xab\x00\x5d\x00\x2e\x40" //.W9..`Q,.p..]..@ |
// /* 0290 */ "\x17\x00\x0b\x20\x0b\xa0\x05\x30\x05\x50\x02\x48\x30\x45\xa0\x00" //... ...0.P.H0E.. |
// /* 02a0 */ "\x49\xb9\x84\x7e\x0c\x00\x3e\x41\x82\x38\x58\x2c\x40\x15\x82\x64" //I..~..>A.8X,@..d |
// /* 02b0 */ "\xc3\x4c\xcc\x23\xf8\x23\xf9\x00\x15\x90\x05\xe0\xb2\x40\xe3\x23" //.L.#.#.......@.# |
// /* 02c0 */ "\x08\xfc\x0f\x05\x94\x05\x30\x4e\x30\x75\x8f\xe6\x2c\x02\xbf\x99" //......0N0u..,... |
// /* 02d0 */ "\x82\x5c\x8c\x5f\x32\x07\x98\x02\xfb\x9d\x3e\x64\x40\xa0\x03\xd8" //..._2.....>d@... |
// /* 02e0 */ "\x28\x8d\x80\x1e\x43\xf0\x2c\x15\xbe\xc1\x37\x08\x1b\x08\xf8\x00" //(...C.,...7..... |
// /* 02f0 */ "\xfe\x00\x08\x7d\x39\xc1\x25\x6e\xec\x2f\xa6\xb0\x0a\xe1\x3e\x67" //...}9.%n./....>g |
// /* 0300 */ "\xc9\x82\x01\xec\x13\x9d\x1f\x24\xc0\x3d\x82\x7c\xe3\xe4\xa0\x03" //.......$.=.|.... |
// /* 0310 */ "\xdf\x6e\xe4\x02\xbe\x14\x60\xbc\x3a\xf4\x83\xf5\x82\xc5\x39\x80" //.n....`.:.....9. |
// /* 0320 */ "\x42\xb8\x00\x1e\x2d\xb6\x02\x76\xca\x10\x10\x08\x61\x70\xe8\xa0" //B...-..v....ap.. |
// /* 0330 */ "\x08\x63\xe5\x45\x80\x0c\x04\x7c\x07\x00\xda\xb1\x0d\xf3\x0a\x3e" //.c.E...|.......> |
// /* 0340 */ "\x3e\x43\x0f\xd7\xd7\x9e\x58\x72\x9d\xf6\x54\x41\x8a\x34\x0a\xe7" //>C....Xr..TA.4.. |
// /* 0350 */ "\x35\xb6\x49\x68\xb6\x78\xac\x06\xd7\x1d\xa2\x8c\x51\xa9\x37\xfb" //5.Ih.x......Q.7. |
// /* 0360 */ "\xf8\x10\x17\xd3\x5a\xe3\xac\xb6\x78\xac\x0a\xd1\x15\xb7\xcb\x59" //....Z...x......Y |
// /* 0370 */ "\x86\x28\xc0\x9d\xd6\xa0\x7f\xeb\x63\x6a\x8e\xdc\x65\xad\x03\x14" //.(......cj..e... |
// /* 0380 */ "\x60\xbf\x99\x16\xd9\x2d\xce\x6a\xd4\x31\x46\x0f\x6d\xfa\x0a\x80" //`....-.j.1F.m... |
// /* 0390 */ "\x18\x7f\x74\x75\xc6\x5a\x2f\xcd\x74\x80\x04\x03\x01\xb6\x49\x70" //..tu.Z/.t.....Ip |
// /* 03a0 */ "\x44\xd0\x17\x08\x7f\x2c\xf1\x5c\x10\xfe\x1c\xe1\x96\xb9\xcd\x45" //D....,.........E |
// /* 03b0 */ "\xf9\x01\x84\xc0\x60\x36\xff\xcb\x23\xff\x8c\x1f\x44\xc3\xc2\x8a" //....`6..#...D... |
// /* 03c0 */ "\xdd\x78\x5d\x96\xff\x90\x42\x5f\x5e\x05\x26\xc2\x64\x9c\xd6\xe7" //.x]...B_^.&.d... |
// /* 03d0 */ "\xf5\x06\xff\x9c\x07\xb2\x8c\x51\xb8\xb2\x3c\x00\x6c\xf1\x2b\xc4" //.......Q..<.l.+. |
// /* 03e0 */ "\x0a\x80\x18\x59\x80\x4f\x82\x42\xd8\x09\x95\x73\x9a\x8d\x79\x99" //...Y.O.B...s..y. |
// /* 03f0 */ "\x71\x96\x8d\x31\x79\x35\x69\xf9\x21\x52\x1a\x80\x18\x60\x40\x0f" //q..1y5i.!R...`@. |
// /* 0400 */ "\xcf\x8f\x3f\x40\x9b\x10\xc5\x1b\xf2\xb0\x8e\xf9\x8f\xd4\x6b\x6c" //..?@..........kl |
// /* 0410 */ "\x97\xe8\x5d\x40\x14\x28\xc0\x87\xd6\x4c\x0c\xa6\xa7\x05\x31\x8c" //..]@.(...L....1. |
// /* 0420 */ "\x51\x88\x43\x7c\x80\x09\xfb\x88\x15\xba\x4b\x68\x8a\x8d\x7c\x95" //Q.C|......Kh..|. |
// /* 0430 */ "\x40\x14\x64\xa0\x06\x06\x79\x67\x7d\xc0\xc4\xc1\x36\xfc\x51\xde" //@.d...yg}...6.Q. |
// /* 0440 */ "\x52\x01\x0e\x01\x64\x00\x64\xc8\xf8\x00\x10\x00"                 //R...d.d..... |
// Sent dumped on RDP Client (5) 1100 bytes |
// send_server_update done |
// GraphicsUpdatePDU::init::Initializing orders batch mcs_userid=0 shareid=65538 |
// Listener closed |
// Incoming socket 5 (ip=10.10.47.175) |
// Socket RDP Client (5) : closing connection |
// RDP Client (0): total_received=1716, total_sent=19491 |
} /* end indata */;

