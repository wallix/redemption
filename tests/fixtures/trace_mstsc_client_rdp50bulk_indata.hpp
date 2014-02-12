// Listen: binding socket 4 on 0.0.0.0:3389 |
// Listen: listening on socket 4 |
// Incoming socket to 5 (ip=10.10.47.228) |
// Reading font file ./tests/fixtures/sans-10.fv1 |
// font name <Bitstream Vera Sans> size <10> |
// Font file ./tests/fixtures/sans-10.fv1 defines glyphs up to 256 |
// Front::incoming() |
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
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x0e\xe0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 13 bytes |
// Recv done on RDP Client (5) 13 bytes |
/* 0000 */ "\x00\x00\x00\x00\x00\x01\x00\x08\x00\x03\x00\x00\x00"             //............. |
// Dump done on RDP Client (5) 13 bytes |
// CR Recv: PROTOCOL TLS 1.0 |
// CR Recv: PROTOCOL HYBRID |
// Front::incoming::sending x224 connection confirm PDU |
// -----------------> Front::TLS Support Enabled |
// CC Send: PROTOCOL TLS 1.0 |
// Sending on RDP Client (5) 19 bytes |
// /* 0000 */ "\x03\x00\x00\x13\x0e\xd0\x00\x00\x00\x00\x00\x02\x00\x08\x00\x01" //................ |
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
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 422 bytes |
// Recv done on RDP Client (5) 422 bytes |
/* 0000 */ "\x80\x7f\x65\x82\x01\xa0\x04\x01\x01\x04\x01\x01\x01\x01\xff\x30" //..e............0 |
/* 0010 */ "\x19\x02\x01\x22\x02\x01\x02\x02\x01\x00\x02\x01\x01\x02\x01\x00" //..."............ |
/* 0020 */ "\x02\x01\x01\x02\x02\xff\xff\x02\x01\x02\x30\x19\x02\x01\x01\x02" //..........0..... |
/* 0030 */ "\x01\x01\x02\x01\x01\x02\x01\x01\x02\x01\x00\x02\x01\x01\x02\x02" //................ |
/* 0040 */ "\x04\x20\x02\x01\x02\x30\x1c\x02\x02\xff\xff\x02\x02\xfc\x17\x02" //. ...0.......... |
/* 0050 */ "\x02\xff\xff\x02\x01\x01\x02\x01\x00\x02\x01\x01\x02\x02\xff\xff" //................ |
/* 0060 */ "\x02\x01\x02\x04\x82\x01\x3f\x00\x05\x00\x14\x7c\x00\x01\x81\x36" //......?....|...6 |
/* 0070 */ "\x00\x08\x00\x10\x00\x01\xc0\x00\x44\x75\x63\x61\x81\x28\x01\xc0" //........Duca.(.. |
/* 0080 */ "\xd8\x00\x04\x00\x08\x00\x00\x04\x00\x03\x01\xca\x03\xaa\x0c\x04" //................ |
/* 0090 */ "\x00\x00\xb1\x1d\x00\x00\x57\x00\x49\x00\x4e\x00\x2d\x00\x41\x00" //......W.I.N.-.A. |
/* 00a0 */ "\x55\x00\x34\x00\x51\x00\x55\x00\x4e\x00\x4c\x00\x35\x00\x34\x00" //U.4.Q.U.N.L.5.4. |
/* 00b0 */ "\x31\x00\x4a\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x0c\x00" //1.J............. |
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0100 */ "\x00\x00\x01\xca\x01\x00\x00\x00\x00\x00\x10\x00\x0f\x00\x2d\x00" //..............-. |
/* 0110 */ "\x65\x00\x65\x00\x36\x00\x37\x00\x30\x00\x62\x00\x36\x00\x32\x00" //e.e.6.7.0.b.6.2. |
/* 0120 */ "\x2d\x00\x36\x00\x37\x00\x34\x00\x63\x00\x2d\x00\x34\x00\x64\x00" //-.6.7.4.c.-.4.d. |
/* 0130 */ "\x66\x00\x65\x00\x2d\x00\x61\x00\x38\x00\x31\x00\x33\x00\x2d\x00" //f.e.-.a.8.1.3.-. |
/* 0140 */ "\x36\x00\x38\x00\x37\x00\x39\x00\x33\x00\x33\x00\x64\x00\x00\x00" //6.8.7.9.3.3.d... |
/* 0150 */ "\x02\x00\x01\x00\x00\x00\x04\xc0\x0c\x00\x11\x00\x00\x00\x00\x00" //................ |
/* 0160 */ "\x00\x00\x02\xc0\x0c\x00\x1b\x00\x00\x00\x00\x00\x00\x00\x03\xc0" //................ |
/* 0170 */ "\x38\x00\x04\x00\x00\x00\x72\x64\x70\x64\x72\x00\x00\x00\x00\x00" //8.....rdpdr..... |
/* 0180 */ "\x80\x80\x72\x64\x70\x73\x6e\x64\x00\x00\x00\x00\x00\xc0\x64\x72" //..rdpsnd......dr |
/* 0190 */ "\x64\x79\x6e\x76\x63\x00\x00\x00\x80\xc0\x63\x6c\x69\x70\x72\x64" //dynvc.....cliprd |
/* 01a0 */ "\x72\x00\x00\x00\xa0\xc0"                                         //r..... |
// Dump done on RDP Client (5) 422 bytes |
// GCC::UserData tag=c001 length=216 |
// Received from Client GCC User Data CS_CORE (216 bytes) |
// cs_core::version [80004] RDP 5.0, 5.1, 5.2, and 6.0 clients) |
// cs_core::desktopWidth  = 1024 |
// cs_core::desktopHeight = 768 |
// cs_core::colorDepth    = [ca01] [RNS_UD_COLOR_8BPP] superseded by postBeta2ColorDepth |
// cs_core::SASSequence   = [aa03] [Unknown] |
// cs_core::keyboardLayout= 040c |
// cs_core::clientBuild   = 7601 |
// cs_core::clientName    = WIN-AU4QUNL541J |
// cs_core::keyboardType  = [0007] Japanese keyboard |
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
// cs_core::clientDigProductId=[650065003600370030006200360032002d0036003700340063002d0034006400660065002d0061003800310033002d0036003800370039003300330064000000 |
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
// sc_core::clientRequestedProtocols  = 3 |
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
// /* 0030 */ "\x00\x14\x7c\x00\x01\x2a\x14\x0a\x76\x01\x01\x00\x01\xc0\x00\x4d" //..|..*..v......M |
// /* 0040 */ "\x63\x44\x6e\x80\x28\x01\x0c\x0c\x00\x04\x00\x08\x00\x03\x00\x00" //cDn.(........... |
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
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 6 bytes |
// Recv done on RDP Client (5) 6 bytes |
/* 0000 */ "\x80\x04\x01\x00\x01\x00"                                         //...... |
// Dump done on RDP Client (5) 6 bytes |
// Front::incoming::Recv MCS::AttachUserRequest |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x08"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x80\x28"                                                         //.( |
// Dump done on RDP Client (5) 2 bytes |
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
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 6 bytes |
// Recv done on RDP Client (5) 6 bytes |
/* 0000 */ "\x80\x38\x00\x00\x03\xe9"                                         //.8.... |
// Dump done on RDP Client (5) 6 bytes |
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
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 6 bytes |
// Recv done on RDP Client (5) 6 bytes |
/* 0000 */ "\x80\x38\x00\x00\x03\xeb"                                         //.8.... |
// Dump done on RDP Client (5) 6 bytes |
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
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 6 bytes |
// Recv done on RDP Client (5) 6 bytes |
/* 0000 */ "\x80\x38\x00\x00\x03\xec"                                         //.8.... |
// Dump done on RDP Client (5) 6 bytes |
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
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 6 bytes |
// Recv done on RDP Client (5) 6 bytes |
/* 0000 */ "\x80\x38\x00\x00\x03\xed"                                         //.8.... |
// Dump done on RDP Client (5) 6 bytes |
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
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 6 bytes |
// Recv done on RDP Client (5) 6 bytes |
/* 0000 */ "\x80\x38\x00\x00\x03\xee"                                         //.8.... |
// Dump done on RDP Client (5) 6 bytes |
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
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 6 bytes |
// Recv done on RDP Client (5) 6 bytes |
/* 0000 */ "\x80\x38\x00\x00\x03\xef"                                         //.8.... |
// Dump done on RDP Client (5) 6 bytes |
// cjrq[3] = 1007 -> cjcf |
// Sending on RDP Client (5) 15 bytes |
// /* 0000 */ "\x03\x00\x00\x0f\x02\xf0\x80\x3e\x00\x00\x00\x03\xef\x03\xef"     //.......>....... |
// Sent dumped on RDP Client (5) 15 bytes |
// Front::incoming::RDP Security Commencement |
// TLS mode: exchange packet disabled |
// Front::incoming() |
// Front::incoming::Secure Settings Exchange |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x01\x4b"                                                     //..K |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 325 bytes |
// Recv done on RDP Client (5) 325 bytes |
/* 0000 */ "\x80\x64\x00\x00\x03\xeb\x70\x81\x3c\x40\x00\x00\x00\x0c\x04\x0c" //.d....p.<@...... |
/* 0010 */ "\x04\xb3\x47\x03\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00" //..G............. |
/* 0020 */ "\x00\x78\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x1a\x00\x31" //.x.............1 |
/* 0030 */ "\x00\x30\x00\x2e\x00\x31\x00\x30\x00\x2e\x00\x34\x00\x37\x00\x2e" //.0...1.0...4.7.. |
/* 0040 */ "\x00\x32\x00\x32\x00\x38\x00\x00\x00\x40\x00\x43\x00\x3a\x00\x5c" //.2.2.8...@.C.:.. |
/* 0050 */ "\x00\x57\x00\x69\x00\x6e\x00\x64\x00\x6f\x00\x77\x00\x73\x00\x5c" //.W.i.n.d.o.w.s.. |
/* 0060 */ "\x00\x73\x00\x79\x00\x73\x00\x74\x00\x65\x00\x6d\x00\x33\x00\x32" //.s.y.s.t.e.m.3.2 |
/* 0070 */ "\x00\x5c\x00\x6d\x00\x73\x00\x74\x00\x73\x00\x63\x00\x61\x00\x78" //...m.s.t.s.c.a.x |
/* 0080 */ "\x00\x2e\x00\x64\x00\x6c\x00\x6c\x00\x00\x00\xc4\xff\xff\xff\x50" //...d.l.l.......P |
/* 0090 */ "\x00\x61\x00\x72\x00\x69\x00\x73\x00\x2c\x00\x20\x00\x4d\x00\x61" //.a.r.i.s.,. .M.a |
/* 00a0 */ "\x00\x64\x00\x72\x00\x69\x00\x64\x00\x00\x00\x00\x00\x00\x00\x00" //.d.r.i.d........ |
/* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00d0 */ "\x00\x0a\x00\x00\x00\x05\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00e0 */ "\x00\x00\x00\x50\x00\x61\x00\x72\x00\x69\x00\x73\x00\x2c\x00\x20" //...P.a.r.i.s.,.  |
/* 00f0 */ "\x00\x4d\x00\x61\x00\x64\x00\x72\x00\x69\x00\x64\x00\x20\x00\x28" //.M.a.d.r.i.d. .( |
/* 0100 */ "\x00\x68\x00\x65\x00\x75\x00\x72\x00\x65\x00\x20\x00\x64\x00\x19" //.h.e.u.r.e. .d.. |
/* 0110 */ "\x20\xe9\x00\x74\x00\xe9\x00\x29\x00\x00\x00\x00\x00\x00\x00\x00" // ..t...)........ |
/* 0120 */ "\x00\x00\x00\x00\x00\x03\x00\x00\x00\x05\x00\x02\x00\x00\x00\x00" //................ |
/* 0130 */ "\x00\x00\x00\xc4\xff\xff\xff\x01\x00\x00\x00\x07\x00\x00\x00\x00" //................ |
/* 0140 */ "\x00\x64\x00\x00\x00"                                             //.d... |
// Dump done on RDP Client (5) 325 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x0c, 0x04, 0x0c, 0x04, 0xb3, 0x47, 0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,  // .....G.......... |
// /* 0010 */ 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,  // ....x........... |
// /* 0020 */ 0x1a, 0x00, 0x31, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x34, 0x00,  // ..1.0...1.0...4. |
// /* 0030 */ 0x37, 0x00, 0x2e, 0x00, 0x32, 0x00, 0x32, 0x00, 0x38, 0x00, 0x00, 0x00, 0x40, 0x00, 0x43, 0x00,  // 7...2.2.8...@.C. |
// /* 0040 */ 0x3a, 0x00, 0x5c, 0x00, 0x57, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x64, 0x00, 0x6f, 0x00, 0x77, 0x00,  // :...W.i.n.d.o.w. |
// /* 0050 */ 0x73, 0x00, 0x5c, 0x00, 0x73, 0x00, 0x79, 0x00, 0x73, 0x00, 0x74, 0x00, 0x65, 0x00, 0x6d, 0x00,  // s...s.y.s.t.e.m. |
// /* 0060 */ 0x33, 0x00, 0x32, 0x00, 0x5c, 0x00, 0x6d, 0x00, 0x73, 0x00, 0x74, 0x00, 0x73, 0x00, 0x63, 0x00,  // 3.2...m.s.t.s.c. |
// /* 0070 */ 0x61, 0x00, 0x78, 0x00, 0x2e, 0x00, 0x64, 0x00, 0x6c, 0x00, 0x6c, 0x00, 0x00, 0x00, 0xc4, 0xff,  // a.x...d.l.l..... |
// /* 0080 */ 0xff, 0xff, 0x50, 0x00, 0x61, 0x00, 0x72, 0x00, 0x69, 0x00, 0x73, 0x00, 0x2c, 0x00, 0x20, 0x00,  // ..P.a.r.i.s.,. . |
// /* 0090 */ 0x4d, 0x00, 0x61, 0x00, 0x64, 0x00, 0x72, 0x00, 0x69, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00,  // M.a.d.r.i.d..... |
// /* 00a0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00b0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00c0 */ 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 00d0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x00, 0x61, 0x00, 0x72, 0x00, 0x69, 0x00, 0x73, 0x00,  // ......P.a.r.i.s. |
// /* 00e0 */ 0x2c, 0x00, 0x20, 0x00, 0x4d, 0x00, 0x61, 0x00, 0x64, 0x00, 0x72, 0x00, 0x69, 0x00, 0x64, 0x00,  // ,. .M.a.d.r.i.d. |
// /* 00f0 */ 0x20, 0x00, 0x28, 0x00, 0x68, 0x00, 0x65, 0x00, 0x75, 0x00, 0x72, 0x00, 0x65, 0x00, 0x20, 0x00,  //  .(.h.e.u.r.e. . |
// /* 0100 */ 0x64, 0x00, 0x19, 0x20, 0xe9, 0x00, 0x74, 0x00, 0xe9, 0x00, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00,  // d.. ..t...)..... |
// /* 0110 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 0x00, 0x02, 0x00,  // ................ |
// /* 0120 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc4, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00,  // ................ |
// /* 0130 */ 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,                          // ....d... |
// RDP-5 Style logon |
// Receiving from client InfoPacket |
// InfoPacket::CodePage 67896332 |
// InfoPacket::flags 0x347b3 |
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
// InfoPacket::flags:INFO_NOAUDIOPLAYBACK no |
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
// InfoPacket::ExtendedInfoPacket::clientAddress 10.10.47.228 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 64 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\Windows\system32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 1 |
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
// Front::incoming() |
// Front::incoming::WAITING_FOR_ANSWER_TO_LICENCE |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\xb2"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 172 bytes |
// Recv done on RDP Client (5) 172 bytes |
/* 0000 */ "\x80\x64\x00\x00\x03\xeb\x70\x80\xa3\x80\x00\x99\x50\x13\x83\x9f" //.d....p.....P... |
/* 0010 */ "\x00\x01\x00\x00\x00\x00\x00\x01\x04\x73\xb1\xe2\xe4\x2f\xa3\xb7" //.........s.../.. |
/* 0020 */ "\x51\xe4\x92\x21\xa9\x83\xfe\xd0\x92\xaa\x7b\xd2\x0c\xed\xcd\x35" //Q..!......{....5 |
/* 0030 */ "\xfe\xd4\xf7\x9b\x1b\x16\x84\x25\xfe\x00\x00\x48\x00\xcc\x40\xaa" //.......%...H..@. |
/* 0040 */ "\xbd\x3b\xde\x03\xad\x35\x55\xd5\x7b\x54\xbd\x73\x08\xa3\x74\x40" //.;...5U.{T.s..t@ |
/* 0050 */ "\x55\x8e\x18\xd8\xf8\x41\x14\x0d\xc8\xe6\x28\xa4\xec\x6a\xbb\x78" //U....A....(..j.x |
/* 0060 */ "\x9a\xae\x47\x9d\xc9\x0a\xe1\xb9\x46\x8d\x80\x64\x1a\x3c\xd6\x59" //..G.....F..d.<.Y |
/* 0070 */ "\x8b\x92\xa9\xd5\x1d\x8e\xb7\xc9\x38\xdb\x3a\xdc\x2b\x00\x00\x00" //........8.:.+... |
/* 0080 */ "\x00\x00\x00\x00\x00\x0f\x00\x0f\x00\x41\x64\x6d\x69\x6e\x69\x73" //.........Adminis |
/* 0090 */ "\x74\x72\x61\x74\x65\x75\x72\x00\x10\x00\x10\x00\x57\x49\x4e\x2d" //trateur.....WIN- |
/* 00a0 */ "\x41\x55\x34\x51\x55\x4e\x4c\x35\x34\x31\x4a\x00"                 //AU4QUNL541J. |
// Dump done on RDP Client (5) 172 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x13, 0x83, 0x9f, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x73, 0xb1, 0xe2, 0xe4,  // ............s... |
// /* 0010 */ 0x2f, 0xa3, 0xb7, 0x51, 0xe4, 0x92, 0x21, 0xa9, 0x83, 0xfe, 0xd0, 0x92, 0xaa, 0x7b, 0xd2, 0x0c,  // /..Q..!......{.. |
// /* 0020 */ 0xed, 0xcd, 0x35, 0xfe, 0xd4, 0xf7, 0x9b, 0x1b, 0x16, 0x84, 0x25, 0xfe, 0x00, 0x00, 0x48, 0x00,  // ..5.......%...H. |
// /* 0030 */ 0xcc, 0x40, 0xaa, 0xbd, 0x3b, 0xde, 0x03, 0xad, 0x35, 0x55, 0xd5, 0x7b, 0x54, 0xbd, 0x73, 0x08,  // .@..;...5U.{T.s. |
// /* 0040 */ 0xa3, 0x74, 0x40, 0x55, 0x8e, 0x18, 0xd8, 0xf8, 0x41, 0x14, 0x0d, 0xc8, 0xe6, 0x28, 0xa4, 0xec,  // .t@U....A....(.. |
// /* 0050 */ 0x6a, 0xbb, 0x78, 0x9a, 0xae, 0x47, 0x9d, 0xc9, 0x0a, 0xe1, 0xb9, 0x46, 0x8d, 0x80, 0x64, 0x1a,  // j.x..G.....F..d. |
// /* 0060 */ 0x3c, 0xd6, 0x59, 0x8b, 0x92, 0xa9, 0xd5, 0x1d, 0x8e, 0xb7, 0xc9, 0x38, 0xdb, 0x3a, 0xdc, 0x2b,  // <.Y........8.:.+ |
// /* 0070 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x41, 0x64, 0x6d, 0x69,  // ............Admi |
// /* 0080 */ 0x6e, 0x69, 0x73, 0x74, 0x72, 0x61, 0x74, 0x65, 0x75, 0x72, 0x00, 0x10, 0x00, 0x10, 0x00, 0x57,  // nistrateur.....W |
// /* 0090 */ 0x49, 0x4e, 0x2d, 0x41, 0x55, 0x34, 0x51, 0x55, 0x4e, 0x4c, 0x35, 0x34, 0x31, 0x4a, 0x00,     // IN-AU4QUNL541J. |
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
// General caps::compression type 0 |
// General caps::extra flags 0 |
// General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED no |
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
// Input caps::imeFileName 910222736 |
// Sec clear payload to send: |
// /* 0000 */ 0x20, 0x01, 0x11, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x04, 0x00, 0x0a, 0x01, 0x52, 0x44,  //  .............RD |
// /* 0010 */ 0x50, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x01, 0x00, 0x03, 0x00, 0x00, 0x02,  // P............... |
// /* 0020 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,  // ................ |
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
// /* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x1c" //................ |
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
// Front::incoming() |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x02\x07"                                                     //... |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 513 bytes |
// Recv done on RDP Client (5) 513 bytes |
/* 0000 */ "\x80\x64\x00\x00\x03\xeb\x70\x81\xf8\xf8\x01\x13\x00\xe9\x03\x02" //.d....p......... |
/* 0010 */ "\x00\x01\x00\xe9\x03\x06\x00\xe2\x01\x4d\x53\x54\x53\x43\x00\x13" //.........MSTSC.. |
/* 0020 */ "\x00\x00\x00\x01\x00\x18\x00\x01\x00\x03\x00\x00\x02\x00\x00\x00" //................ |
/* 0030 */ "\x00\x0d\x04\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x1c\x00\x10" //................ |
/* 0040 */ "\x00\x01\x00\x01\x00\x01\x00\x00\x04\x00\x03\x00\x00\x01\x00\x01" //................ |
/* 0050 */ "\x00\x00\x08\x01\x00\x00\x00\x03\x00\x58\x00\x00\x00\x00\x00\x00" //.........X...... |
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01" //................ |
/* 0070 */ "\x00\x14\x00\x00\x00\x01\x00\x00\x00\xaa\x00\x01\x01\x01\x01\x01" //................ |
/* 0080 */ "\x00\x00\x01\x01\x01\x00\x01\x00\x00\x00\x01\x01\x01\x01\x01\x01" //................ |
/* 0090 */ "\x01\x01\x00\x01\x01\x01\x00\x00\x00\x00\x00\xa1\x06\x06\x00\x00" //................ |
/* 00a0 */ "\x00\x00\x00\x00\x84\x03\x00\x00\x00\x00\x00\xe4\x04\x00\x00\x04" //................ |
/* 00b0 */ "\x00\x28\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //.(.............. |
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x78\x00\x00\x02\x78" //...........x...x |
/* 00d0 */ "\x00\x00\x08\x51\x01\x00\x20\x0a\x00\x08\x00\x06\x00\x00\x00\x07" //...Q.. ......... |
/* 00e0 */ "\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\x00\x0c\x00\x00" //................ |
/* 00f0 */ "\x00\x00\x00\x02\x00\x02\x00\x08\x00\x0a\x00\x01\x00\x14\x00\x15" //................ |
/* 0100 */ "\x00\x09\x00\x08\x00\x00\x00\x00\x00\x0d\x00\x58\x00\x91\x00\x20" //...........X...  |
/* 0110 */ "\x00\x0c\x04\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00" //................ |
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0160 */ "\x00\x0c\x00\x08\x00\x01\x00\x00\x00\x0e\x00\x08\x00\x01\x00\x00" //................ |
/* 0170 */ "\x00\x10\x00\x34\x00\xfe\x00\x04\x00\xfe\x00\x04\x00\xfe\x00\x08" //...4............ |
/* 0180 */ "\x00\xfe\x00\x08\x00\xfe\x00\x10\x00\xfe\x00\x20\x00\xfe\x00\x40" //........... ...@ |
/* 0190 */ "\x00\xfe\x00\x80\x00\xfe\x00\x00\x01\x40\x00\x00\x08\x00\x01\x00" //.........@...... |
/* 01a0 */ "\x01\x03\x00\x00\x00\x0f\x00\x08\x00\x01\x00\x00\x00\x11\x00\x0c" //................ |
/* 01b0 */ "\x00\x01\x00\x00\x00\x00\x28\x64\x00\x14\x00\x0c\x00\x01\x00\x00" //......(d........ |
/* 01c0 */ "\x00\x00\x00\x00\x00\x15\x00\x0c\x00\x02\x00\x00\x00\x00\x0a\x00" //................ |
/* 01d0 */ "\x01\x16\x00\x28\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //...(............ |
/* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1a\x00\x08\x00\x00\x00\x00" //................ |
/* 0200 */ "\x00"                                                             //. |
// Dump done on RDP Client (5) 513 bytes |
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
// /* 0100 */ 0x0d, 0x00, 0x58, 0x00, 0x91, 0x00, 0x20, 0x00, 0x0c, 0x04, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,  // ..X... ......... |
// /* 0110 */ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0120 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0130 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0140 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................ |
// /* 0150 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00,  // ................ |
// /* 0160 */ 0x0e, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x34, 0x00, 0xfe, 0x00, 0x04, 0x00,  // ..........4..... |
// /* 0170 */ 0xfe, 0x00, 0x04, 0x00, 0xfe, 0x00, 0x08, 0x00, 0xfe, 0x00, 0x08, 0x00, 0xfe, 0x00, 0x10, 0x00,  // ................ |
// /* 0180 */ 0xfe, 0x00, 0x20, 0x00, 0xfe, 0x00, 0x40, 0x00, 0xfe, 0x00, 0x80, 0x00, 0xfe, 0x00, 0x00, 0x01,  // .. ...@......... |
// /* 0190 */ 0x40, 0x00, 0x00, 0x08, 0x00, 0x01, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x08, 0x00,  // @............... |
// /* 01a0 */ 0x01, 0x00, 0x00, 0x00, 0x11, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x28, 0x64, 0x00,  // .............(d. |
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
// Receiving from client CAPSTYPE_POINTER |
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
// OffScreenCache caps::offscreenCacheSize 10240 |
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
// process_confirm_active done p=0x7fff3648162f end=0x7fff3648162f |
// Front::reset() |
// Front::reset::use_bitmap_comp=1 |
// Front::reset::use_compact_packets=1 |
// Front::reset::bitmap_cache_version=0 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[0](2000) used=2 free=16532 |
// Front::send_pointer(cache_idx=0 x=0 y=0) |
// Front::send_pointer: fast-path |
// Sending on RDP Client (5) 3220 bytes |
// /* 0000 */ "\x00\x8c\x94\x09\x8e\x0c\x00\x00\x00\x00\x00\x00\x20\x00\x20\x00" //............ . . |
// /* 0010 */ "\x80\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0040 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0170 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0180 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0190 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0200 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0210 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0220 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0230 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0240 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0250 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0260 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0270 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0280 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0290 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0300 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0310 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0320 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0330 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0340 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0350 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0360 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0370 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0380 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0390 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0400 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0410 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0420 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0430 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0440 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0450 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0460 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0470 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0480 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0490 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0500 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\x00" //................ |
// /* 0510 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0520 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0530 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0540 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0550 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0560 */ "\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff" //................ |
// /* 0570 */ "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0580 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0590 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05c0 */ "\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff" //................ |
// /* 05d0 */ "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0600 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0610 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0620 */ "\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00" //................ |
// /* 0630 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0640 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0650 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0660 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0670 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0680 */ "\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00" //................ |
// /* 0690 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06d0 */ "\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00" //................ |
// /* 06e0 */ "\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00" //................ |
// /* 06f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0700 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0710 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0720 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0730 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00" //................ |
// /* 0740 */ "\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00" //................ |
// /* 0750 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0760 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0770 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0780 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0790 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff\xff" //................ |
// /* 07a0 */ "\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07f0 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0800 */ "\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0810 */ "\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0820 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0830 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0840 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0850 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0860 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff" //................ |
// /* 0870 */ "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0880 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0890 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08b0 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00" //................ |
// /* 08d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0900 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0910 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0920 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00" //................ |
// /* 0930 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0940 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0950 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0960 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0970 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0980 */ "\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0990 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09d0 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09e0 */ "\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a00 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a10 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a20 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a30 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a40 */ "\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a50 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a60 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a70 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a80 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a90 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff\xff" //................ |
// /* 0aa0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ab0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ac0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ad0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ae0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0af0 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00" //................ |
// /* 0b00 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b10 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b20 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b30 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b40 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b50 */ "\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b60 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b70 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b80 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b90 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ba0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bb0 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bc0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bd0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0be0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bf0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0c00 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0c10 */ "\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0c20 */ "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0c30 */ "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0c40 */ "\xff\xff\xff\xff\xff\xff\xff\xff\xfe\x7f\xff\xff\xfc\x3f\xff\xff" //.............?.. |
// /* 0c50 */ "\xfc\x3f\xff\xff\xf8\x7f\xff\xff\x78\x7f\xff\xff\x30\xff\xff\xff" //.?......x...0... |
// /* 0c60 */ "\x10\xff\xff\xff\x01\xff\xff\xff\x00\x1f\xff\xff\x00\x3f\xff\xff" //.............?.. |
// /* 0c70 */ "\x00\x7f\xff\xff\x00\xff\xff\xff\x01\xff\xff\xff\x03\xff\xff\xff" //................ |
// /* 0c80 */ "\x07\xff\xff\xff\x0f\xff\xff\xff\x1f\xff\xff\xff\x3f\xff\xff\xff" //............?... |
// /* 0c90 */ "\x7f\xff\xff\xff"                                                 //.... |
// Sent dumped on RDP Client (5) 3220 bytes |
// Front::send_pointer done |
// Front::send_pointer(cache_idx=1 x=15 y=16) |
// Front::send_pointer: fast-path |
// Sending on RDP Client (5) 3220 bytes |
// /* 0000 */ "\x00\x8c\x94\x09\x8e\x0c\x01\x00\x0f\x00\x10\x00\x20\x00\x20\x00" //............ . . |
// /* 0010 */ "\x80\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0040 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0170 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0180 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0190 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0200 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0210 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0220 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0230 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0240 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0250 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0260 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0270 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0280 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0290 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02d0 */ "\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 02e0 */ "\xff\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 02f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0300 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0310 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0320 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0330 */ "\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0340 */ "\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff" //................ |
// /* 0350 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0360 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0370 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0380 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0390 */ "\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 03a0 */ "\xff\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 03b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0400 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0410 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0420 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0430 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0440 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0450 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0460 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0470 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0480 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0490 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 04c0 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0500 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0510 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0520 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0530 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0540 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0550 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0560 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0570 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0580 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0590 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 05e0 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0600 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0610 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0620 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0630 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0640 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0650 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0660 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0670 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0680 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0690 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 06a0 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0700 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0710 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0720 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0730 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0740 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0750 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0760 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0770 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0780 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0790 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 07c0 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0800 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0810 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0820 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0830 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0840 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0850 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0860 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0870 */ "\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0880 */ "\xff\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0890 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08d0 */ "\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08e0 */ "\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff" //................ |
// /* 08f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0900 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0910 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0920 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0930 */ "\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0940 */ "\xff\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0950 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0960 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0970 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0980 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0990 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a00 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a10 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a20 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a30 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a40 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a50 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a60 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a70 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a80 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a90 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0aa0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ab0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ac0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ad0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ae0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0af0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b00 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b10 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b20 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b30 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b40 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b50 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b60 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b70 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b80 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b90 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ba0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bb0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bc0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bd0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0be0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bf0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0c00 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0c10 */ "\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0c20 */ "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0c30 */ "\xff\xe1\x0f\xff\xff\xe0\x0f\xff\xff\xe0\x0f\xff\xff\xfc\x7f\xff" //................ |
// /* 0c40 */ "\xff\xfc\x7f\xff\xff\xfc\x7f\xff\xff\xfc\x7f\xff\xff\xfc\x7f\xff" //................ |
// /* 0c50 */ "\xff\xfc\x7f\xff\xff\xfc\x7f\xff\xff\xfc\x7f\xff\xff\xfc\x7f\xff" //................ |
// /* 0c60 */ "\xff\xfc\x7f\xff\xff\xfc\x7f\xff\xff\xfc\x7f\xff\xff\xe0\x0f\xff" //................ |
// /* 0c70 */ "\xff\xe0\x0f\xff\xff\xe1\x0f\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0c80 */ "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0c90 */ "\xff\xff\xff\xff"                                                 //.... |
// Sent dumped on RDP Client (5) 3220 bytes |
// Front::send_pointer done |
// Front received CONFIRMACTIVEPDU done |
// Front::incoming() |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x24"                                                     //..$ |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 30 bytes |
// Recv done on RDP Client (5) 30 bytes |
/* 0000 */ "\x80\x64\x00\x00\x03\xeb\x70\x16\x16\x00\x17\x00\xe9\x03\x02\x00" //.d....p......... |
/* 0010 */ "\x01\x00\x00\x01\x08\x00\x1f\x00\x00\x00\x01\x00\xe9\x03"         //.............. |
// Dump done on RDP Client (5) 30 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x16, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x08, 0x00, 0x1f, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x01, 0x00, 0xe9, 0x03,                                // ...... |
// Front::incoming::sec_flags=0 |
// Front received DATAPDU |
// Front::process_data(...) |
// sdata_in.pdutype2=31 sdata_in.len=8 sdata_in.compressedLen=0 remains=4 payload_len=4 |
// PDUTYPE2_SYNCHRONIZE |
// PDUTYPE2_SYNCHRONIZE messageType=1 controlId=1001 |
// send_synchronize |
// Sec clear payload to send: |
// /* 0000 */ 0x16, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x08, 0x00, 0x1f, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x01, 0x00, 0xea, 0x03,                                // ...... |
// Sending on RDP Client (5) 36 bytes |
// /* 0000 */ "\x03\x00\x00\x24\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x16\x16\x00" //...$...h....p... |
// /* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x08\x00\x1f\x00\x00\x00" //................ |
// /* 0020 */ "\x01\x00\xea\x03"                                                 //.... |
// Sent dumped on RDP Client (5) 36 bytes |
// send_synchronize done |
// process_data done |
// Front received DATAPDU done |
// Front::incoming() |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x28"                                                     //..( |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 34 bytes |
// Recv done on RDP Client (5) 34 bytes |
/* 0000 */ "\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00\x17\x00\xe9\x03\x02\x00" //.d....p......... |
/* 0010 */ "\x01\x00\x00\x01\x0c\x00\x14\x00\x00\x00\x04\x00\x00\x00\x00\x00" //................ |
/* 0020 */ "\x00\x00"                                                         //.. |
// Dump done on RDP Client (5) 34 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x0c, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                    // .......... |
// Front::incoming::sec_flags=0 |
// Front received DATAPDU |
// Front::process_data(...) |
// sdata_in.pdutype2=20 sdata_in.len=12 sdata_in.compressedLen=0 remains=8 payload_len=8 |
// PDUTYPE2_CONTROL |
// send_control action=4 |
// Sec clear payload to send: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x0c, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00,                    // .......... |
// Sending on RDP Client (5) 40 bytes |
// /* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x1a\x1a\x00" //...(...h....p... |
// /* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x0c\x00\x14\x00\x00\x00" //................ |
// /* 0020 */ "\x04\x00\x00\x00\xea\x03\x00\x00"                                 //........ |
// Sent dumped on RDP Client (5) 40 bytes |
// send_control action=4 |
// process_data done |
// Front received DATAPDU done |
// Front::incoming() |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x28"                                                     //..( |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 34 bytes |
// Recv done on RDP Client (5) 34 bytes |
/* 0000 */ "\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00\x17\x00\xe9\x03\x02\x00" //.d....p......... |
/* 0010 */ "\x01\x00\x00\x01\x0c\x00\x14\x00\x00\x00\x01\x00\x00\x00\x00\x00" //................ |
/* 0020 */ "\x00\x00"                                                         //.. |
// Dump done on RDP Client (5) 34 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x0c, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                    // .......... |
// Front::incoming::sec_flags=0 |
// Front received DATAPDU |
// Front::process_data(...) |
// sdata_in.pdutype2=20 sdata_in.len=12 sdata_in.compressedLen=0 remains=8 payload_len=8 |
// PDUTYPE2_CONTROL |
// send_control action=2 |
// Sec clear payload to send: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x0c, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00,                    // .......... |
// Sending on RDP Client (5) 40 bytes |
// /* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x1a\x1a\x00" //...(...h....p... |
// /* 0010 */ "\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\x0c\x00\x14\x00\x00\x00" //................ |
// /* 0020 */ "\x02\x00\x00\x00\xea\x03\x00\x00"                                 //........ |
// Sent dumped on RDP Client (5) 40 bytes |
// send_control action=2 |
// process_data done |
// Front received DATAPDU done |
// Front::incoming() |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x10"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x0e"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 12 bytes |
// Recv done on RDP Client (5) 12 bytes |
/* 0000 */ "\x01\x0f\x62\x01\x0f\x20\x00\x08\x99\x02\xc4\x01"                 //..b.. ...... |
// Dump done on RDP Client (5) 12 bytes |
// Front::Received fast-path PUD, scancode keyboardFlags=0xC000, keyCode=0xF |
// Front::Received fast-path PUD done |
// Front::Received fast-path PUD, sync eventFlags=0x2 |
// Front::Received fast-path PUD done |
// Front::Received fast-path PUD, scancode keyboardFlags=0xC000, keyCode=0xF |
// Front::Received fast-path PUD done |
// Front::Received fast-path PUD, mouse pointerFlags=0x800, xPos=0x299, yPos=0x1C4 |
// Front::Received fast-path PUD done |
// Front::incoming() |
// Front::incoming::ACTIVATE_AND_PROCESS_DATA |
// Socket RDP Client (5) receiving 1 bytes |
// Recv done on RDP Client (5) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Client (5) 1 bytes |
// Socket RDP Client (5) receiving 3 bytes |
// Recv done on RDP Client (5) 3 bytes |
/* 0000 */ "\x00\x00\x28"                                                     //..( |
// Dump done on RDP Client (5) 3 bytes |
// Socket RDP Client (5) receiving 2 bytes |
// Recv done on RDP Client (5) 2 bytes |
/* 0000 */ "\x02\xf0"                                                         //.. |
// Dump done on RDP Client (5) 2 bytes |
// Socket RDP Client (5) receiving 34 bytes |
// Recv done on RDP Client (5) 34 bytes |
/* 0000 */ "\x80\x64\x00\x00\x03\xeb\x70\x1a\x1a\x00\x17\x00\xe9\x03\x02\x00" //.d....p......... |
/* 0010 */ "\x01\x00\x00\x01\x00\x00\x27\x00\x00\x00\x00\x00\x00\x00\x03\x00" //......'......... |
/* 0020 */ "\x32\x00"                                                         //2. |
// Dump done on RDP Client (5) 34 bytes |
// sec decrypted payload: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x27, 0x00,  // ..............'. |
// /* 0010 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x32, 0x00,                    // ........2. |
// Front::incoming::sec_flags=0 |
// Front received DATAPDU |
// Front::process_data(...) |
// sdata_in.pdutype2=39 sdata_in.len=0 sdata_in.compressedLen=0 remains=8 payload_len=8 |
// PDUTYPE2_FONTLIST |
// send_fontmap |
// Sec clear payload to send: |
// /* 0000 */ 0xbe, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0xb0, 0x00, 0x28, 0x00,  // ..............(. |
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
// /* 00b0 */ 0x02, 0x00, 0x29, 0x00, 0x01, 0x00, 0x2a, 0x00, 0x05, 0x00, 0x2b, 0x00, 0x2a, 0x00,        // ..)...*...+.*. |
// Sending on RDP Client (5) 205 bytes |
// /* 0000 */ "\x03\x00\x00\xcd\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x80\xbe\xbe" //.......h....p... |
// /* 0010 */ "\x00\x17\x00\xe9\x03\x02\x00\x01\x00\x00\x02\xb0\x00\x28\x00\x00" //.............(.. |
// /* 0020 */ "\x00\xff\x02\xb6\x00\x28\x00\x00\x00\x27\x00\x27\x00\x03\x00\x04" //.....(...'.'.... |
// /* 0030 */ "\x00\x00\x00\x26\x00\x01\x00\x1e\x00\x02\x00\x1f\x00\x03\x00\x1d" //...&............ |
// /* 0040 */ "\x00\x04\x00\x27\x00\x05\x00\x0b\x00\x06\x00\x28\x00\x08\x00\x21" //...'.......(...! |
// /* 0050 */ "\x00\x09\x00\x20\x00\x0a\x00\x22\x00\x0b\x00\x25\x00\x0c\x00\x24" //... ..."...%...$ |
// /* 0060 */ "\x00\x0d\x00\x23\x00\x0e\x00\x19\x00\x0f\x00\x16\x00\x10\x00\x15" //...#............ |
// /* 0070 */ "\x00\x11\x00\x1c\x00\x12\x00\x1b\x00\x13\x00\x1a\x00\x14\x00\x17" //................ |
// /* 0080 */ "\x00\x15\x00\x18\x00\x16\x00\x0e\x00\x18\x00\x0c\x00\x19\x00\x0d" //................ |
// /* 0090 */ "\x00\x1a\x00\x12\x00\x1b\x00\x14\x00\x1f\x00\x13\x00\x20\x00\x00" //............. .. |
// /* 00a0 */ "\x00\x21\x00\x0a\x00\x22\x00\x06\x00\x23\x00\x07\x00\x24\x00\x08" //.!..."...#...$.. |
// /* 00b0 */ "\x00\x25\x00\x09\x00\x26\x00\x04\x00\x27\x00\x03\x00\x28\x00\x02" //.%...&...'...(.. |
// /* 00c0 */ "\x00\x29\x00\x01\x00\x2a\x00\x05\x00\x2b\x00\x2a\x00"             //.)...*...+.*. |
// Sent dumped on RDP Client (5) 205 bytes |
// send_fontmap |
// Front::send_data_update_sync |
// Front::send_data_update_sync: fast-path |
// Sending on RDP Client (5) 5 bytes |
// /* 0000 */ "\x00\x05\x03\x00\x00"                                             //..... |
// Sent dumped on RDP Client (5) 5 bytes |
// Front::send_pointer(cache_idx=0 x=0 y=0) |
// Front::send_pointer: fast-path |
// Sending on RDP Client (5) 3220 bytes |
// /* 0000 */ "\x00\x8c\x94\x09\x8e\x0c\x00\x00\x00\x00\x00\x00\x20\x00\x20\x00" //............ . . |
// /* 0010 */ "\x80\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0040 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0170 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0180 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0190 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0200 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0210 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0220 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0230 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0240 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0250 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0260 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0270 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0280 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0290 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0300 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0310 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0320 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0330 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0340 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0350 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0360 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0370 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0380 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0390 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0400 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0410 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0420 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0430 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0440 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0450 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0460 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0470 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0480 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0490 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0500 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\x00" //................ |
// /* 0510 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0520 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0530 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0540 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0550 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0560 */ "\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff" //................ |
// /* 0570 */ "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0580 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0590 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05c0 */ "\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff" //................ |
// /* 05d0 */ "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0600 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0610 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0620 */ "\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00" //................ |
// /* 0630 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0640 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0650 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0660 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0670 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0680 */ "\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00" //................ |
// /* 0690 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06d0 */ "\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00" //................ |
// /* 06e0 */ "\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00" //................ |
// /* 06f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0700 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0710 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0720 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0730 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00" //................ |
// /* 0740 */ "\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00" //................ |
// /* 0750 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0760 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0770 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0780 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0790 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff\xff" //................ |
// /* 07a0 */ "\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07f0 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0800 */ "\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0810 */ "\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0820 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0830 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0840 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0850 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0860 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff" //................ |
// /* 0870 */ "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0880 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0890 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08b0 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00" //................ |
// /* 08d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0900 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0910 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0920 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00" //................ |
// /* 0930 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0940 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0950 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0960 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0970 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0980 */ "\x00\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0990 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09d0 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09e0 */ "\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a00 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a10 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a20 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a30 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a40 */ "\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a50 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a60 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a70 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a80 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a90 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff\xff" //................ |
// /* 0aa0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ab0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ac0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ad0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ae0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0af0 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00" //................ |
// /* 0b00 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b10 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b20 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b30 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b40 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b50 */ "\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b60 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b70 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b80 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b90 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ba0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bb0 */ "\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bc0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bd0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0be0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bf0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0c00 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0c10 */ "\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0c20 */ "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0c30 */ "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0c40 */ "\xff\xff\xff\xff\xff\xff\xff\xff\xfe\x7f\xff\xff\xfc\x3f\xff\xff" //.............?.. |
// /* 0c50 */ "\xfc\x3f\xff\xff\xf8\x7f\xff\xff\x78\x7f\xff\xff\x30\xff\xff\xff" //.?......x...0... |
// /* 0c60 */ "\x10\xff\xff\xff\x01\xff\xff\xff\x00\x1f\xff\xff\x00\x3f\xff\xff" //.............?.. |
// /* 0c70 */ "\x00\x7f\xff\xff\x00\xff\xff\xff\x01\xff\xff\xff\x03\xff\xff\xff" //................ |
// /* 0c80 */ "\x07\xff\xff\xff\x0f\xff\xff\xff\x1f\xff\xff\xff\x3f\xff\xff\xff" //............?... |
// /* 0c90 */ "\x7f\xff\xff\xff"                                                 //.... |
// Sent dumped on RDP Client (5) 3220 bytes |
// Front::send_pointer done |
// Front::send_pointer(cache_idx=1 x=15 y=16) |
// Front::send_pointer: fast-path |
// Sending on RDP Client (5) 3220 bytes |
// /* 0000 */ "\x00\x8c\x94\x09\x8e\x0c\x01\x00\x0f\x00\x10\x00\x20\x00\x20\x00" //............ . . |
// /* 0010 */ "\x80\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0040 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0170 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0180 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0190 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0200 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0210 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0220 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0230 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0240 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0250 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0260 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0270 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0280 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0290 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 02d0 */ "\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 02e0 */ "\xff\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 02f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0300 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0310 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0320 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0330 */ "\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0340 */ "\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff" //................ |
// /* 0350 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0360 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0370 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0380 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0390 */ "\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 03a0 */ "\xff\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 03b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 03f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0400 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0410 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0420 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0430 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0440 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0450 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0460 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0470 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0480 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0490 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 04c0 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 04f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0500 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0510 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0520 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0530 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0540 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0550 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0560 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0570 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0580 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0590 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 05e0 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 05f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0600 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0610 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0620 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0630 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0640 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0650 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0660 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0670 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0680 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0690 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 06a0 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 06f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0700 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0710 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0720 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0730 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0740 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0750 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0760 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0770 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0780 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0790 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 07c0 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 07f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0800 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0810 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff" //................ |
// /* 0820 */ "\xff\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0830 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0840 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0850 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0860 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0870 */ "\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0880 */ "\xff\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0890 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08d0 */ "\x00\x00\x00\x00\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 08e0 */ "\x00\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff" //................ |
// /* 08f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0900 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0910 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0920 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0930 */ "\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0940 */ "\xff\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0950 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0960 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0970 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0980 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0990 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 09f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a00 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a10 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a20 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a30 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a40 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a50 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a60 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a70 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a80 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0a90 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0aa0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ab0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ac0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ad0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ae0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0af0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b00 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b10 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b20 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b30 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b40 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b50 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b60 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b70 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b80 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0b90 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0ba0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bb0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bc0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bd0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0be0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0bf0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0c00 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0c10 */ "\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0c20 */ "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0c30 */ "\xff\xe1\x0f\xff\xff\xe0\x0f\xff\xff\xe0\x0f\xff\xff\xfc\x7f\xff" //................ |
// /* 0c40 */ "\xff\xfc\x7f\xff\xff\xfc\x7f\xff\xff\xfc\x7f\xff\xff\xfc\x7f\xff" //................ |
// /* 0c50 */ "\xff\xfc\x7f\xff\xff\xfc\x7f\xff\xff\xfc\x7f\xff\xff\xfc\x7f\xff" //................ |
// /* 0c60 */ "\xff\xfc\x7f\xff\xff\xfc\x7f\xff\xff\xfc\x7f\xff\xff\xe0\x0f\xff" //................ |
// /* 0c70 */ "\xff\xe0\x0f\xff\xff\xe1\x0f\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0c80 */ "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" //................ |
// /* 0c90 */ "\xff\xff\xff\xff"                                                 //.... |
// Sent dumped on RDP Client (5) 3220 bytes |
// Front::send_pointer done |
// --------------> UP AND RUNNING <---------------- |
// asking for selector |
// process_data done |
// Front received DATAPDU done |
// Front::begin_update() |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[1](23) used=1035 free=15499 |
// order(10 clip(0,0,1,1)):opaquerect(rect(0,0,1024,768) color=0x00ffff) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[2](23) used=1044 free=15490 |
// order(10 clip(0,0,1,1)):opaquerect(rect(5,5,1014,758) color=0x00f800) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[3](23) used=1052 free=15482 |
// order(10 clip(0,0,1,1)):opaquerect(rect(10,10,1004,748) color=0x0007e0) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[4](23) used=1060 free=15474 |
// order(10 clip(0,0,1,1)):opaquerect(rect(15,15,994,738) color=0x00001f) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[5](23) used=1068 free=15466 |
// order(10 clip(0,0,1,1)):opaquerect(rect(20,20,984,728) color=0x000000) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[6](23) used=1075 free=15459 |
// order(10 clip(0,0,1,1)):opaquerect(rect(30,30,964,708) color=0x000273) |
// Widget_load: image file [./tests/fixtures/Philips_PM5544_640.png] is PNG file |
// front::draw:draw_tile((192, 144, 32, 32) (0, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[7](2064) used=1083 free=15451 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[8](30) used=1142 free=15392 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 144, 32, 32) (32, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[9](2064) used=1156 free=15378 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[10](30) used=1211 free=15323 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 144, 32, 32) (64, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[11](2064) used=1214 free=15320 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[12](30) used=1273 free=15261 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 144, 32, 32) (96, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[13](2064) used=1276 free=15258 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[14](30) used=1341 free=15193 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 144, 32, 32) (128, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[15](2064) used=1344 free=15190 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[16](30) used=1385 free=15149 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 144, 32, 32) (160, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[17](2064) used=1388 free=15146 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[18](30) used=1426 free=15108 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 144, 32, 32) (192, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[19](2064) used=1429 free=15105 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[20](30) used=1490 free=15044 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 144, 32, 32) (224, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[21](2064) used=1493 free=15041 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[22](30) used=1558 free=14976 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 144, 32, 32) (256, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[23](2064) used=1561 free=14973 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[24](30) used=1697 free=14837 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 144, 32, 32) (288, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[25](2064) used=1700 free=14834 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[26](30) used=1826 free=14708 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 144, 32, 32) (320, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[27](2064) used=1829 free=14705 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[28](30) used=1955 free=14579 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 144, 32, 32) (352, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[29](2064) used=1958 free=14576 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[30](30) used=2098 free=14436 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 144, 32, 32) (384, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[31](2064) used=2101 free=14433 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[32](30) used=2167 free=14367 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 144, 32, 32) (416, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[33](2064) used=2170 free=14364 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[34](30) used=2231 free=14303 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 144, 32, 32) (448, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[35](2064) used=2234 free=14300 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[36](30) used=2272 free=14262 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 144, 32, 32) (480, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[37](2064) used=2275 free=14259 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[38](30) used=2312 free=14222 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 144, 32, 32) (512, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[39](2064) used=2315 free=14219 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[40](30) used=2376 free=14158 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 144, 32, 32) (544, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[41](2064) used=2379 free=14155 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[42](30) used=2441 free=14093 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 144, 32, 32) (576, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[43](2064) used=2444 free=14090 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[44](30) used=2502 free=14032 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 144, 32, 32) (608, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[45](2064) used=2505 free=14029 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[46](30) used=2565 free=13969 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,144,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((192, 176, 32, 32) (0, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[47](2064) used=2568 free=13966 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[48](30) used=2628 free=13906 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 176, 32, 32) (32, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[49](2064) used=2634 free=13900 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[50](30) used=2702 free=13832 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 176, 32, 32) (64, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[51](2064) used=2705 free=13829 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[52](30) used=2775 free=13759 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 176, 32, 32) (96, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[53](2064) used=2778 free=13756 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[54](30) used=2860 free=13674 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 176, 32, 32) (128, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[55](2064) used=2863 free=13671 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[56](30) used=2919 free=13615 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 176, 32, 32) (160, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[57](2064) used=2922 free=13612 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[58](30) used=2961 free=13573 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 176, 32, 32) (192, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[59](2064) used=2964 free=13570 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[60](30) used=3184 free=13350 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 176, 32, 32) (224, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[61](2064) used=3187 free=13347 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[62](30) used=3355 free=13179 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 176, 32, 32) (256, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[63](2064) used=3358 free=13176 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[64](30) used=3395 free=13139 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 176, 32, 32) (288, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[65](2064) used=3398 free=13136 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[66](30) used=3427 free=13107 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 176, 32, 32) (320, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[67](30) used=3430 free=13104 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 176, 32, 32) (352, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[68](2064) used=3433 free=13101 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[69](30) used=3469 free=13065 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 176, 32, 32) (384, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[70](2064) used=3472 free=13062 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[71](30) used=3640 free=12894 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 176, 32, 32) (416, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[72](2064) used=3643 free=12891 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[73](30) used=3875 free=12659 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 176, 32, 32) (448, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[74](2064) used=3878 free=12656 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[75](30) used=3917 free=12617 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 176, 32, 32) (480, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[76](2064) used=3920 free=12614 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[77](30) used=3974 free=12560 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 176, 32, 32) (512, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[78](2064) used=3977 free=12557 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[79](30) used=4058 free=12476 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 176, 32, 32) (544, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[80](2064) used=4061 free=12473 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[81](30) used=4132 free=12402 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 176, 32, 32) (576, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[82](2064) used=4135 free=12399 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[83](30) used=4207 free=12327 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 176, 32, 32) (608, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[84](2064) used=4210 free=12324 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[85](30) used=4275 free=12259 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,176,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((192, 208, 32, 32) (0, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[86](2064) used=4278 free=12256 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[87](30) used=4327 free=12207 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 208, 32, 32) (32, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[88](2064) used=4333 free=12201 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[89](30) used=4381 free=12153 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 208, 32, 32) (64, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[90](2064) used=4384 free=12150 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[91](30) used=4412 free=12122 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 208, 32, 32) (96, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[92](2064) used=4415 free=12119 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[93](30) used=4470 free=12064 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 208, 32, 32) (128, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[94](2064) used=4473 free=12061 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[95](30) used=4535 free=11999 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 208, 32, 32) (160, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[96](2064) used=4538 free=11996 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[97](30) used=4771 free=11763 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 208, 32, 32) (192, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[98](2064) used=4774 free=11760 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[99](30) used=4819 free=11715 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 208, 32, 32) (224, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[100](2064) used=4822 free=11712 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[101](30) used=4884 free=11650 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 208, 32, 32) (256, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[102](2064) used=4887 free=11647 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[103](30) used=4916 free=11618 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 208, 32, 32) (288, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[104](30) used=4919 free=11615 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 208, 32, 32) (320, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[105](30) used=4922 free=11612 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 208, 32, 32) (352, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[106](30) used=4925 free=11609 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 208, 32, 32) (384, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[107](2064) used=4928 free=11606 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[108](30) used=4964 free=11570 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 208, 32, 32) (416, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[109](2064) used=4967 free=11567 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[110](30) used=5011 free=11523 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 208, 32, 32) (448, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[111](2064) used=5014 free=11520 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[112](30) used=5259 free=11275 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 208, 32, 32) (480, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[113](2064) used=5262 free=11272 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[114](30) used=5322 free=11212 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 208, 32, 32) (512, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[115](2064) used=5325 free=11209 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[116](30) used=5380 free=11154 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 208, 32, 32) (544, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[117](2064) used=5383 free=11151 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[118](30) used=5411 free=11123 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 208, 32, 32) (576, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[119](2064) used=5414 free=11120 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[120](30) used=5463 free=11071 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 208, 32, 32) (608, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[121](2064) used=5466 free=11068 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[122](30) used=5515 free=11019 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,208,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((192, 240, 32, 32) (0, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[123](2064) used=5518 free=11016 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[124](30) used=5581 free=10953 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 240, 32, 32) (32, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[125](2064) used=5587 free=10947 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[126](30) used=5639 free=10895 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 240, 32, 32) (64, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[127](2064) used=5642 free=10892 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[128](30) used=5703 free=10831 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 240, 32, 32) (96, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[129](2064) used=5706 free=10828 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[130](30) used=5792 free=10742 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 240, 32, 32) (128, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[131](2064) used=5795 free=10739 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[132](30) used=6037 free=10497 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 240, 32, 32) (160, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[133](2064) used=6040 free=10494 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[134](30) used=6067 free=10467 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 240, 32, 32) (192, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[135](2064) used=6070 free=10464 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[136](30) used=6118 free=10416 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 240, 32, 32) (224, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[137](2064) used=6121 free=10413 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[138](30) used=6163 free=10371 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 240, 32, 32) (256, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[139](2064) used=6166 free=10368 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[140](30) used=6193 free=10341 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 240, 32, 32) (288, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[141](2064) used=6196 free=10338 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[142](30) used=6227 free=10307 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 240, 32, 32) (320, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[143](2064) used=6230 free=10304 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[144](30) used=6260 free=10274 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 240, 32, 32) (352, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[145](2064) used=6263 free=10271 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[146](30) used=6291 free=10243 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 240, 32, 32) (384, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[147](2064) used=6294 free=10240 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[148](30) used=6325 free=10209 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 240, 32, 32) (416, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[149](2064) used=6328 free=10206 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[150](30) used=6374 free=10160 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 240, 32, 32) (448, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[151](2064) used=6377 free=10157 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[152](30) used=6405 free=10129 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 240, 32, 32) (480, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[153](2064) used=6408 free=10126 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[154](30) used=6663 free=9871 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 240, 32, 32) (512, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[155](2064) used=6666 free=9868 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[156](30) used=6752 free=9782 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 240, 32, 32) (544, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[157](2064) used=6755 free=9779 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[158](30) used=6816 free=9718 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 240, 32, 32) (576, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[159](2064) used=6819 free=9715 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[160](30) used=6872 free=9662 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 240, 32, 32) (608, 96, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[161](2064) used=6875 free=9659 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[162](30) used=6941 free=9593 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,240,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((192, 272, 32, 32) (0, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[163](2064) used=6944 free=9590 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[164](30) used=7006 free=9528 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 272, 32, 32) (32, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[165](2064) used=7012 free=9522 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[166](30) used=7063 free=9471 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 272, 32, 32) (64, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[167](2064) used=7066 free=9468 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[168](30) used=7126 free=9408 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 272, 32, 32) (96, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[169](2064) used=7129 free=9405 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[170](30) used=7293 free=9241 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 272, 32, 32) (128, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[171](2064) used=7296 free=9238 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[172](30) used=7408 free=9126 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 272, 32, 32) (160, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[173](2064) used=7411 free=9123 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[174](30) used=7459 free=9075 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 272, 32, 32) (192, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[175](2064) used=7462 free=9072 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[176](30) used=7512 free=9022 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 272, 32, 32) (224, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[177](2064) used=7515 free=9019 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[178](30) used=7567 free=8967 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 272, 32, 32) (256, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[179](2064) used=7570 free=8964 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[180](30) used=7608 free=8926 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 272, 32, 32) (288, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[181](2064) used=7611 free=8923 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[182](30) used=7660 free=8874 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 272, 32, 32) (320, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[183](2064) used=7663 free=8871 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[184](30) used=7709 free=8825 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 272, 32, 32) (352, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[185](2064) used=7712 free=8822 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[186](30) used=7750 free=8784 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 272, 32, 32) (384, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[187](2064) used=7753 free=8781 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[188](30) used=7809 free=8725 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 272, 32, 32) (416, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[189](2064) used=7812 free=8722 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[190](30) used=7856 free=8678 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 272, 32, 32) (448, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[191](2064) used=7859 free=8675 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[192](30) used=7907 free=8627 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 272, 32, 32) (480, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[193](2064) used=7910 free=8624 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[194](30) used=8022 free=8512 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 272, 32, 32) (512, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[195](2064) used=8025 free=8509 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[196](30) used=8188 free=8346 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 272, 32, 32) (544, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[197](2064) used=8191 free=8343 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[198](30) used=8251 free=8283 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 272, 32, 32) (576, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[199](2064) used=8254 free=8280 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[200](30) used=8306 free=8228 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 272, 32, 32) (608, 128, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[201](2064) used=8309 free=8225 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[202](30) used=8371 free=8163 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,272,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((192, 304, 32, 32) (0, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[203](2064) used=8374 free=8160 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[204](30) used=8425 free=8109 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 304, 32, 32) (32, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[205](2064) used=8431 free=8103 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[206](30) used=8478 free=8056 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 304, 32, 32) (64, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[207](2064) used=8481 free=8053 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[208](30) used=8535 free=7999 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 304, 32, 32) (96, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[209](2064) used=8538 free=7996 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[210](30) used=8714 free=7820 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 304, 32, 32) (128, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[211](2064) used=8717 free=7817 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[212](30) used=8739 free=7795 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 304, 32, 32) (160, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[213](2064) used=8742 free=7792 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[214](30) used=8770 free=7764 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 304, 32, 32) (192, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[215](2064) used=8773 free=7761 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[216](30) used=8795 free=7739 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 304, 32, 32) (224, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[217](2064) used=8798 free=7736 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[218](30) used=8826 free=7708 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 304, 32, 32) (256, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[219](2064) used=8829 free=7705 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[220](30) used=8851 free=7683 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 304, 32, 32) (288, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[221](2064) used=8854 free=7680 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[222](30) used=8892 free=7642 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 304, 32, 32) (320, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[223](2064) used=8895 free=7639 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[224](30) used=8928 free=7606 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 304, 32, 32) (352, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[225](2064) used=8931 free=7603 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[226](30) used=8953 free=7581 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 304, 32, 32) (384, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[227](2064) used=8956 free=7578 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[228](30) used=8984 free=7550 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 304, 32, 32) (416, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[229](2064) used=8987 free=7547 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[230](30) used=9009 free=7525 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 304, 32, 32) (448, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[231](2064) used=9012 free=7522 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[232](30) used=9040 free=7494 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 304, 32, 32) (480, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[233](2064) used=9043 free=7491 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[234](30) used=9065 free=7469 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 304, 32, 32) (512, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[235](2064) used=9068 free=7466 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[236](30) used=9264 free=7270 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 304, 32, 32) (544, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[237](2064) used=9267 free=7267 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[238](30) used=9321 free=7213 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 304, 32, 32) (576, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[239](2064) used=9324 free=7210 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[240](30) used=9372 free=7162 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 304, 32, 32) (608, 160, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[241](2064) used=9375 free=7159 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[242](30) used=9429 free=7105 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,304,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((192, 336, 32, 32) (0, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[243](2064) used=9432 free=7102 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[244](30) used=9480 free=7054 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 336, 32, 32) (32, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[245](2064) used=9486 free=7048 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[246](30) used=9531 free=7003 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 336, 32, 32) (64, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[247](2064) used=9534 free=7000 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[248](30) used=9584 free=6950 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 336, 32, 32) (96, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[249](2064) used=9587 free=6947 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[250](30) used=9776 free=6758 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 336, 32, 32) (128, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[251](2064) used=9779 free=6755 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[252](30) used=9818 free=6716 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 336, 32, 32) (160, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[253](2064) used=9821 free=6713 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[254](30) used=9861 free=6673 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 336, 32, 32) (192, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[255](2064) used=9864 free=6670 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[256](30) used=9905 free=6629 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 336, 32, 32) (224, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[257](2064) used=9908 free=6626 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[258](30) used=9961 free=6573 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 336, 32, 32) (256, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[259](2064) used=9964 free=6570 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[260](30) used=10009 free=6525 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 336, 32, 32) (288, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[261](2064) used=10012 free=6522 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[262](30) used=10062 free=6472 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 336, 32, 32) (320, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[263](2064) used=10065 free=6469 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[264](30) used=10116 free=6418 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 336, 32, 32) (352, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[265](2064) used=10119 free=6415 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[266](30) used=10164 free=6370 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 336, 32, 32) (384, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[267](2064) used=10167 free=6367 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[268](30) used=10222 free=6312 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 336, 32, 32) (416, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[269](2064) used=10225 free=6309 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[270](30) used=10266 free=6268 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 336, 32, 32) (448, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[271](2064) used=10269 free=6265 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[272](30) used=10309 free=6225 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 336, 32, 32) (480, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[273](2064) used=10312 free=6222 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[274](30) used=10354 free=6180 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 336, 32, 32) (512, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[275](2064) used=10357 free=6177 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[276](30) used=10554 free=5980 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 336, 32, 32) (544, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[277](2064) used=10557 free=5977 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[278](30) used=10606 free=5928 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 336, 32, 32) (576, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[279](2064) used=10609 free=5925 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[280](30) used=10656 free=5878 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 336, 32, 32) (608, 192, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[281](2064) used=10659 free=5875 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[282](30) used=10707 free=5827 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,336,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((192, 368, 32, 32) (0, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[283](2064) used=10710 free=5824 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[284](30) used=10739 free=5795 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 368, 32, 32) (32, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[285](2064) used=10745 free=5789 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[286](30) used=10785 free=5749 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 368, 32, 32) (64, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[287](2064) used=10788 free=5746 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[288](30) used=10832 free=5702 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 368, 32, 32) (96, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[289](2064) used=10835 free=5699 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[290](30) used=11038 free=5496 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 368, 32, 32) (128, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[291](2064) used=11041 free=5493 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[292](30) used=11110 free=5424 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 368, 32, 32) (160, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[293](2064) used=11113 free=5421 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[294](30) used=11150 free=5384 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 368, 32, 32) (192, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[295](2064) used=11153 free=5381 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[296](30) used=11222 free=5312 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 368, 32, 32) (224, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[297](2064) used=11225 free=5309 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[298](30) used=11300 free=5234 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 368, 32, 32) (256, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[299](2064) used=11303 free=5231 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[300](30) used=11381 free=5153 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 368, 32, 32) (288, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[301](2064) used=11384 free=5150 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[302](30) used=11477 free=5057 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 368, 32, 32) (320, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[303](2064) used=11480 free=5054 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[304](30) used=11567 free=4967 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 368, 32, 32) (352, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[305](2064) used=11570 free=4964 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[306](30) used=11649 free=4885 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 368, 32, 32) (384, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[307](2064) used=11652 free=4882 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[308](30) used=11726 free=4808 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 368, 32, 32) (416, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[309](2064) used=11729 free=4805 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[310](30) used=11798 free=4736 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 368, 32, 32) (448, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[311](2064) used=11801 free=4733 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[312](30) used=11838 free=4696 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 368, 32, 32) (480, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[313](2064) used=11841 free=4693 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[314](30) used=11907 free=4627 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 368, 32, 32) (512, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[315](2064) used=11910 free=4624 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[316](30) used=12104 free=4430 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 368, 32, 32) (544, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[317](2064) used=12107 free=4427 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[318](30) used=12151 free=4383 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 368, 32, 32) (576, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[319](2064) used=12154 free=4380 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[320](30) used=12194 free=4340 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 368, 32, 32) (608, 224, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[321](2064) used=12197 free=4337 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[322](30) used=12226 free=4308 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,368,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((192, 400, 32, 32) (0, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[323](2064) used=12229 free=4305 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[324](30) used=12278 free=4256 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 400, 32, 32) (32, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[325](2064) used=12284 free=4250 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[326](30) used=12330 free=4204 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 400, 32, 32) (64, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[327](2064) used=12333 free=4201 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[328](30) used=12382 free=4152 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 400, 32, 32) (96, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[329](2064) used=12385 free=4149 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[330](30) used=12574 free=3960 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 400, 32, 32) (128, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[331](2064) used=12577 free=3957 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[332](30) used=12698 free=3836 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 400, 32, 32) (160, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[333](2064) used=12701 free=3833 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[334](30) used=12854 free=3680 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 400, 32, 32) (192, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[335](2064) used=12857 free=3677 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[336](30) used=13018 free=3516 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 400, 32, 32) (224, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[337](2064) used=13021 free=3513 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[338](30) used=13181 free=3353 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 400, 32, 32) (256, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[339](2064) used=13184 free=3350 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[340](30) used=13349 free=3185 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 400, 32, 32) (288, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[341](2064) used=13352 free=3182 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[342](30) used=13451 free=3083 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 400, 32, 32) (320, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[343](2064) used=13454 free=3080 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[344](30) used=13551 free=2983 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 400, 32, 32) (352, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[345](2064) used=13554 free=2980 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[346](30) used=13717 free=2817 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 400, 32, 32) (384, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[347](2064) used=13720 free=2814 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[348](30) used=13878 free=2656 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 400, 32, 32) (416, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[349](2064) used=13881 free=2653 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[350](30) used=14045 free=2489 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 400, 32, 32) (448, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[351](2064) used=14048 free=2486 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[352](30) used=14200 free=2334 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 400, 32, 32) (480, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[353](2064) used=14203 free=2331 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[354](30) used=14309 free=2225 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 400, 32, 32) (512, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[355](2064) used=14312 free=2222 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[356](30) used=14495 free=2039 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 400, 32, 32) (544, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[357](2064) used=14498 free=2036 |
// GraphicsUpdatePDU::flush_orders: order_count=357 offset=0 |
// GraphicsUpdatePDU::flush_orders: fast-path |
// Sending on RDP Client (5) 9129 bytes |
// /* 0000 */ "\x00\xa3\xa9\x80\x61\xa2\x23\x65\x01\x03\xbe\x01\x80\x00\x00\xfc" //....a.#e........ |
// /* 0010 */ "\x39\x00\x00\x55\xf8\x89\x57\xc4\x5f\xfe\x20\x00\x93\xe8\x3e\x22" //9..U..W._. ...>" |
// /* 0020 */ "\x55\xf1\x17\xff\x88\x00\x49\xfa\x0f\x88\x95\x7c\x45\xff\xe2\x00" //U.....I....|E... |
// /* 0030 */ "\x1b\x7e\x83\xe2\x25\x5f\x11\x7f\xf8\x80\x08\x97\xd0\x7c\x44\xab" //.~..%_.......|D. |
// /* 0040 */ "\xe2\x2f\xff\x10\x01\x36\xfa\x0f\x88\x95\x7c\x45\xff\xe2\x00\x2b" //./...6....|E...+ |
// /* 0050 */ "\x7f\x41\xf1\x12\xaf\x88\xbf\xfc\x47\x89\x61\x57\xe2\x25\x5f\x11" //.A......G.aW.%_. |
// /* 0060 */ "\x7f\xf8\x8f\x18\xc2\xaf\xc4\x4a\xbe\x22\xff\xf1\x00\x04\x9f\x41" //.......J.".....A |
// /* 0070 */ "\xf1\x12\xaf\x88\xbf\xfc\x40\x02\x4f\xd0\x7c\x44\xab\xe2\x2f\xff" //......@.O.|D../. |
// /* 0080 */ "\x10\x00\xdb\xf4\x1f\x11\x2a\xf8\x8b\xff\xc4\x00\x44\xbe\x83\xe2" //......*.....D... |
// /* 0090 */ "\x25\x5f\x11\x7f\xf8\x80\x09\xb7\xd0\x7c\x44\xab\xe2\x2f\xff\x10" //%_.......|D../.. |
// /* 00a0 */ "\x01\x5b\xfa\x0f\x88\x95\x7c\x45\xff\xe2\x3c\x2a\xf2\x03\xe8\x3e" //.[....|E..<*...> |
// /* 00b0 */ "\x22\xff\xf1\x1e\x21\x79\x00\x93\xe4\x02\x4f\x90\x1e\xa1\x12\x7d" //"...!y....O....} |
// /* 00c0 */ "\x07\xc4\x4a\xbe\x22\xff\xf1\x00\x09\x3f\x41\xf1\x12\xaf\x88\xbf" //..J."....?A..... |
// /* 00d0 */ "\xfc\x40\x03\x6f\xd0\x7c\x44\xab\xe2\x2f\xff\x10\x01\x12\xfa\x0f" //.@.o.|D../...... |
// /* 00e0 */ "\x88\x95\x7c\x45\xff\xe2\x00\x26\xdf\x41\xf1\x12\xaf\x88\xbf\xfc" //..|E...&.A...... |
// /* 00f0 */ "\x40\x05\x6f\xe8\x3e\x22\x55\xf1\x17\xff\x88\xf0\x2b\xc8\x0f\xa0" //@.o.>"U.....+... |
// /* 0100 */ "\xf8\x8b\xff\xc4\x78\x45\xe4\x03\x6f\x90\x0d\xbe\x40\x70\x11\x12" //....xE..o...@p.. |
// /* 0110 */ "\x7d\x07\xc4\x4a\xbe\x22\xff\xf1\x00\x09\x3f\x41\xf1\x12\xaf\x88" //}..J."....?A.... |
// /* 0120 */ "\xbf\xfc\x40\x03\x6f\xd0\x7c\x44\xab\xe2\x2f\xff\x10\x01\x12\xfa" //..@.o.|D../..... |
// /* 0130 */ "\x0f\x88\x95\x7c\x45\xff\xe2\x00\x26\xdf\x41\xf1\x12\xaf\x88\xbf" //...|E...&.A..... |
// /* 0140 */ "\xfc\x40\x05\x6f\xe8\x3e\x22\x55\xf1\x17\xff\x88\xfe\xaf\x20\x3e" //.@.o.>"U...... > |
// /* 0150 */ "\x83\xe2\x2f\xff\x11\xe0\x17\x90\x11\x2f\x20\x22\x5e\x40\x70\x81" //../....../ "^@p. |
// /* 0160 */ "\x12\x7d\x07\xc4\x4a\xbe\x22\xff\xf1\x00\x09\x3f\x41\xf1\x12\xaf" //.}..J."....?A... |
// /* 0170 */ "\x88\xbf\xfc\x40\x03\x6f\xd0\x7c\x44\xab\xe2\x2f\xff\x10\x01\x12" //...@.o.|D../.... |
// /* 0180 */ "\xfa\x0f\x88\x95\x7c\x45\xff\xe2\x00\x26\xdf\x41\xf1\x12\xaf\x88" //....|E...&.A.... |
// /* 0190 */ "\xbf\xfc\x40\x05\x6f\xe8\x3e\x22\x55\xf1\x17\xff\x88\xfc\xaf\x20" //..@.o.>"U......  |
// /* 01a0 */ "\x3e\x83\xe2\x2f\xff\x11\xfc\x5e\x40\x4d\xbc\x80\x9b\x79\x01\xc3" //>../...^@M...y.. |
// /* 01b0 */ "\xc4\x49\xf4\x1f\x11\x2a\xf8\x8b\xff\xc4\x00\x24\xfd\x07\xc4\x4a" //.I...*.....$...J |
// /* 01c0 */ "\xbe\x22\xff\xf1\x00\x0d\xbf\x41\xf1\x12\xaf\x88\xbf\xfc\x40\x04" //.".....A......@. |
// /* 01d0 */ "\x4b\xe8\x3e\x22\x55\xf1\x17\xff\x88\x00\x9b\x7d\x07\xc4\x4a\xbe" //K.>"U......}..J. |
// /* 01e0 */ "\x22\xff\xf1\x00\x15\xbf\xa0\xf8\x89\x57\xc4\x5f\xfe\x23\xea\xbc" //"........W._.#.. |
// /* 01f0 */ "\x80\xfa\x0f\x88\xbf\xfc\x47\xe1\x79\x01\x5b\xf2\x02\xb7\xe4\x07" //......G.y.[..... |
// /* 0200 */ "\x16\x11\x27\xd0\x7c\x44\xab\xe2\x2f\xff\x10\x00\x93\xf4\x1f\x11" //..'.|D../....... |
// /* 0210 */ "\x2a\xf8\x8b\xff\xc4\x00\x36\xfd\x07\xc4\x4a\xbe\x22\xff\xf1\x00" //*.....6...J."... |
// /* 0220 */ "\x11\x2f\xa0\xf8\x89\x57\xc4\x5f\xfe\x20\x02\x6d\xf4\x1f\x11\x2a" //./...W._. .m...* |
// /* 0230 */ "\xf8\x8b\xff\xc4\x00\x56\xfe\x83\xe2\x25\x5f\x11\x7f\xf8\x8f\x8a" //.....V...%_..... |
// /* 0240 */ "\xf2\x03\xe8\x3e\x22\xff\xf1\x1f\x45\xe4\x05\xff\xc8\x0b\xff\xc2" //...>"...E....... |
// /* 0250 */ "\x90\x02\x4f\xa0\xf8\x89\x57\xc4\x5f\xfe\x20\x01\x27\xe8\x3e\x22" //..O...W._. .'.>" |
// /* 0260 */ "\x55\xf1\x17\xff\x88\x00\x6d\xfa\x0f\x88\x95\x7c\x45\xff\xe2\x00" //U.....m....|E... |
// /* 0270 */ "\x22\x5f\x41\xf1\x12\xaf\x88\xbf\xfc\x40\x04\xdb\xe8\x3e\x22\x55" //"_A......@...>"U |
// /* 0280 */ "\xf1\x17\xff\x88\x00\xad\xfd\x07\xc4\x4a\xbe\x22\xff\xf1\x1e\x35" //.........J."...5 |
// /* 0290 */ "\x79\x01\xf4\x1f\x11\x7f\xf8\x80\x90\xa3\xc0\x00\x40\x00\x3b\xfd" //y...........@.;. |
// /* 02a0 */ "\xfc\x44\xfc\x14\x16\xed\x76\x00\xbc\x7c\x8a\xb0\x03\xfc\x8a\x1f" //.D....v..|...... |
// /* 02b0 */ "\x00\x11\x1f\xf9\x10\x00\x22\x7e\x14\x15\x6c\xb6\x39\x81\x01\x97" //......"~..l.9... |
// /* 02c0 */ "\x00\x00\x02\x01\x00\x80\x10\x10\x08\x16\x79\xf4\x1d\x01\x9d\x55" //..........y....U |
// /* 02d0 */ "\x88\x60\x31\xaa\xb5\x3c\x42\x09\x70\xa0\x01\x53\xa5\x4a\x55\x00" //.`1..<B.p..S.JU. |
// /* 02e0 */ "\x28\x31\x88\x2d\x3d\x14\xa5\x80\xc2\xff\x7f\x80\xcd\xa6\xb4\xfa" //(1.-=........... |
// /* 02f0 */ "\xdd\x78\x42\x02\x69\x25\xa7\x92\x10\x80\x30\x24\x86\x9f\x80\x80" //.xB.i%....0$.... |
// /* 0300 */ "\x50\x00\x22\x00\x04\x1f\x09\x4c\x03\x2a\xf0\x4e\x25\x1e\x09\x50" //P."....L.*.N%..P |
// /* 0310 */ "\x00\x84\x20\x94\x2f\xe6\xbb\xcf\x69\x7e\x0b\x9a\x00\x12\x29\x48" //.. ./...i~....)H |
// /* 0320 */ "\x17\x87\x45\x6c\x04\x17\xfb\xfd\x00\x0a\xb7\x5e\x0e\xa0\x01\x78" //..El.......^...x |
// /* 0330 */ "\x14\xdc\xa2\x04\x41\xe4\x3d\xe8\x00\xa2\x10\x48\x0c\x4a\x29\xff" //....A.=....H.J). |
// /* 0340 */ "\x61\x56\x5b\x47\xff\xb4\x0d\xff\x61\x66\x9c\x4e\xc0\x71\xf5\x90" //aV[G....af.N.q.. |
// /* 0350 */ "\xe7\x3a\x3d\x6e\xbc\x31\x48\xff\xc6\x0f\xfd\x06\x9f\xfb\x13\x2f" //.:=n.1H......../ |
// /* 0360 */ "\xfd\x01\x3f\xf8\xab\x2d\xff\xd1\x12\x8a\x50\xff\xf5\x04\xa2\x94" //..?..-....P..... |
// /* 0370 */ "\xd4\x00\x83\x18\xc0\x68\x9e\x87\x78\xfa\x02\x6b\x75\xea\x80\x33" //.....h..x..ku..3 |
// /* 0380 */ "\x8c\x30\x06\x80\x15\xe0\x43\x37\x81\x20\x73\xc0\x98\x8d\x78\x12" //.0....C7. s...x. |
// /* 0390 */ "\x03\x7c\x08\xbb\xcf\x7c\x08\xff\xd0\x1b\xff\xa1\x35\xac\x05\x82" //.|...|......5... |
// /* 03a0 */ "\xc1\x7f\xbf\xfe\xc8\x19\xfd\x98\x8b\xfe\xc8\x10\xf1\x25\x08\x78" //.............%.x |
// /* 03b0 */ "\x72\xc4\x2b\x75\xe6\x08\x00\x00\x07\xe9\x83\x0f\xd3\x8f\xa8\x40" //r.+u...........@ |
// /* 03c0 */ "\x10\x21\x08\x24\x6b\x80\xd9\x76\xdf\x40\x0c\xfe\x53\x04\x55\x96" //.!.$k..v.@..S.U. |
// /* 03d0 */ "\x8e\x59\xef\xf7\xf8\xf5\xde\x80\x19\xfe\xea\x04\x48\x43\x36\x9a" //.Y..........HC6. |
// /* 03e0 */ "\x8f\x40\x80\x00\x0d\x24\xa0\x0f\xde\x00\x8f\x4c\xed\x45\x10\x82" //.@...$.....L.E.. |
// /* 03f0 */ "\x40\xd7\x6d\xf7\xfb\xfc\x6a\xac\x71\xff\x94\x5f\x2a\x80\x68\x63" //@.m...j.q.._*.hc |
// /* 0400 */ "\x00\x30\x11\xcb\x3d\x00\x23\xc0\x02\x7f\x58\x26\xd3\x5a\x00\x45" //.0..=.#...X&.Z.E |
// /* 0410 */ "\x6e\xbc\x0f\xc1\x7d\x40\x08\xf4\xcb\xdb\xb2\xf7\xe0\x4c\x5e\x7c" //n...}@.......L^| |
// /* 0420 */ "\x08\xf6\x82\x08\xa3\x0c\x55\x96\xbc\xd3\xbc\x6e\x09\xbf\xdf\xd9" //......U....n.... |
// /* 0430 */ "\xc4\x20\x90\x29\x94\x52\xaf\x3d\x1e\x62\x10\x82\x01\x41\x1e\x69" //. .).R.=.b...A.i |
// /* 0440 */ "\xd5\x79\xec\xea\xad\x3e\xbb\x8e\x7a\xf0\xc7\xfa\x91\x28\xa3\xaf" //.y...>..z....(.. |
// /* 0450 */ "\xea\x2e\xdb\xc7\x19\x04\x20\x90\x55\x18\x60\xa4\x96\xaf\x3d\x8d" //...... .U.`...=. |
// /* 0460 */ "\x55\x97\x6d\xe3\x6c\xdf\x3d\x32\xfb\x81\x8a\x35\x6e\xbd\x1e\xbb" //U.m.l.=2...5n... |
// /* 0470 */ "\x8d\xb2\xf8\x14\xa3\x0c\xb9\xc7\x07\x9a\xfd\x30\xab\x80\xd2\x78" //...........0...x |
// /* 0480 */ "\x35\xc4\xea\xac\x69\xe9\x38\xa4\xd6\xeb\xc2\xf4\x9f\x25\x9d\xfc" //5...i.8......%.. |
// /* 0490 */ "\x97\x0e\x3e\x4b\xc5\xbf\xc9\x62\x00\x00\x08\x04\x15\xd6\x5b\x77" //..>K...b......[w |
// /* 04a0 */ "\x9e\xbc\xd3\xaf\x75\xe7\xdf\x0a\xcc\x1e\x25\x14\x14\x92\xc5\xa6" //....u.....%..... |
// /* 04b0 */ "\xa3\x14\x67\x9a\x74\xea\xac\x79\x67\x19\xfa\x6a\x80\x14\xb9\xcc" //..g.t..yg..j.... |
// /* 04c0 */ "\x06\x10\xab\x2d\x30\xf2\x6a\xa7\x1c\x14\x92\xba\xcb\x62\xd3\x59" //...-0.j......b.Y |
// /* 04d0 */ "\xb4\xd6\x84\x50\x85\x03\xbb\xcf\x46\x28\xc7\x14\x6a\x14\x98\xc6" //...P....F(..j... |
// /* 04e0 */ "\x04\xf3\x4e\x8d\x55\x84\x1f\x3c\x78\x8a\x00\xd3\x7c\x79\x0b\x34" //..N.U..<x...|y.4 |
// /* 04f0 */ "\xe3\x06\x02\xff\x67\xa1\xce\x74\x00\x0a\xdd\x78\x3e\x80\x03\xe4" //....g..t...x>... |
// /* 0500 */ "\x17\xb3\x27\xc8\x3c\x31\xb8\x2b\xed\xbe\xf7\x5e\x79\xa7\x5d\xe7" //..'.<1.+...^y.]. |
// /* 0510 */ "\xae\xb2\xd1\x38\x39\xe5\x9e\x75\x56\x79\xa7\x0c\x51\xa2\xd3\x51" //...89..uVy..Q..Q |
// /* 0520 */ "\x49\x2c\x4a\x29\xe3\xd4\x0a\x35\xf4\x1e\x22\x40\xce\x28\xc3\x14" //I,J)...5.."@.(.. |
// /* 0530 */ "\x6e\x19\xe2\x84\x28\x3c\xda\x6b\x16\x9a\xba\xcb\x45\x24\xb5\x38" //n...(<.k....E$.8 |
// /* 0540 */ "\xe4\x4a\x28\x61\xe5\xe4\xa9\x48\x07\x89\x3b\xc7\xb1\x30\x01\xa0" //.J(a...H..;..0.. |
// /* 0550 */ "\x00\x70\x54\x71\x27\x8f\x23\x3d\x7b\x1e\x3e\x8f\x22\xbc\x83\x4d" //.pTq'.#={.>."..M |
// /* 0560 */ "\xfe\x41\x87\xff\x20\xe2\xfb\xe4\x11\xcc\x12\x3d\x77\x79\xa7\x2a" //.A.. ......=wy.* |
// /* 0570 */ "\xcb\x54\x61\x9c\x15\xc2\xf0\x2a\xbc\xf6\x65\x14\x67\x10\x82\x08" //.Ta....*..e.g... |
// /* 0580 */ "\xc0\xe7\xd7\x79\xd5\x5a\xaf\x3d\xe4\x71\x00\x2d\x22\x10\x40\xde" //...y.Z.=.q.-".@. |
// /* 0590 */ "\x1e\x42\x18\x13\xac\xb6\x25\x14\xe0\xa6\xe0\xb2\x13\x82\xae\xdb" //.B....%......... |
// /* 05a0 */ "\xe3\x5f\x80\x29\x25\x51\x86\x7e\xf0\x74\x16\x3d\x77\xe3\x9a\xa3" //._.)%Q.~.t.=w... |
// /* 05b0 */ "\x3a\xcb\x66\x51\x46\xd1\x08\x20\x64\x0a\xe7\x1c\x51\x86\x39\x7c" //:.fQF.. d...Q.9| |
// /* 05c0 */ "\x93\xa7\x95\x16\xc0\x1a\x00\x27\x95\x23\x8b\xeb\x82\xb1\x80\xdf" //.......'.#...... |
// /* 05d0 */ "\x82\x8e\x5e\xd0\x93\x50\x0f\x27\xe0\x67\xe4\xf5\x00\x23\x89\x0a" //..^..P.'.g...#.. |
// /* 05e0 */ "\x35\x56\xbf\xdf\xd7\x6d\xe4\xbc\x48\x5c\x45\xa2\xa0\x03\xe4\x44" //5V...m..H.E....D |
// /* 05f0 */ "\x06\x3d\x77\xe2\x5a\xf4\x8c\xf1\x1e\x54\x52\x52\x80\x78\x8f\x47" //.=w.Z....TRR.x.G |
// /* 0600 */ "\x12\x50\x03\x3f\x81\x63\x8d\x2e\xdc\x67\x23\x8b\x35\x90\xf8\x16" //.P.?.c...g#.5... |
// /* 0610 */ "\x38\xcd\xb8\xb3\xcc\x08\x72\xce\xab\x2d\x60\x3e\xe3\x4a\x9b\x4d" //8.....r..-`>.J.M |
// /* 0620 */ "\x49\x08\x47\xa0\x5f\xf8\x01\xe4\x5a\xe4\x1d\xe4\x8c\x7e\x9c\x45" //I.G._...Z....~.E |
// /* 0630 */ "\xbf\x48\x8b\xd9\x6e\x4c\xf1\x70\xa1\x71\x2d\x8e\x4c\x91\xc9\x7f" //.H..nL.p.q-.L... |
// /* 0640 */ "\x0f\x28\xed\xc0\x6d\xc1\x85\xca\x29\xcb\x51\xcb\x6e\x3c\x3a\xa8" //.(..m...).Q.n<:. |
// /* 0650 */ "\x02\xa2\x94\x87\xf0\xe6\x1c\xb0\x13\x7d\x77\x0e\x67\x2c\xf0\x37" //.........}w.g,.7 |
// /* 0660 */ "\x2c\xb9\xf2\x8c\x98\xf8\x03\x12\xff\x00\x39\xa0\x3f\xd3\xc0\x11" //,.........9.?... |
// /* 0670 */ "\x12\x8a\x73\x59\x8f\x00\x28\xfe\x94\x70\xa7\x8e\x6a\xb1\xe6\x58" //..sY..(..p..j..X |
// /* 0680 */ "\xe6\x9a\xf0\x33\x9a\x84\xbc\x78\x06\x0b\x7c\x03\x89\x5f\x80\x5c" //...3...x..|.._.. |
// /* 0690 */ "\xef\x9e\x6b\x78\x05\xcf\x46\x3f\xf2\x97\xff\xa3\x82\x8b\x9e\x84" //..kx..F?........ |
// /* 06a0 */ "\x30\x17\x7d\x78\xe7\x97\x87\x8f\xa7\xc6\x8f\xfb\x7e\x07\x00\x74" //0.}x........~..t |
// /* 06b0 */ "\x46\x70\x53\x99\xd5\x58\x3b\xfd\xba\x23\x38\x6b\x40\x2d\x00\x36" //FpS..X;..#8k@-.6 |
// /* 06c0 */ "\x54\xa4\xc0\x23\x7f\xbf\xf4\x4c\x72\xb4\x08\x74\x4c\x70\x4e\x87" //T..#...Lr..tLpN. |
// /* 06d0 */ "\x0f\xff\xc4\x61\xd2\x26\xd4\x00\xa0\x35\xba\xf5\x3e\x04\x21\xe6" //...a.&...5..>.!. |
// /* 06e0 */ "\xe8\xe2\x62\xe0\xa5\x3c\xbe\x63\xc1\x0f\x08\xcf\x86\x40\x16\x04" //..b..<.c.....@.. |
// /* 06f0 */ "\x10\x6a\x00\x04\xc0\x00\x0c\xdf\xc0\x58\xf0\xdf\x74\x86\xc5\xde" //.j.......X..t... |
// /* 0700 */ "\x3d\x4b\xb7\x9a\x5c\x10\x3a\x43\x20\x42\x00\x50\x19\xc4\xba\x97" //=K....:C B.P.... |
// /* 0710 */ "\x1e\x61\xf0\x6c\x50\x38\xe0\xb8\x20\x41\x5f\xa3\xa0\x0b\xd5\x07" //.a.lP8.. A_..... |
// /* 0720 */ "\xd1\x08\x20\x05\x3b\xc9\x30\x73\xe0\x7c\x7c\x22\xe9\x26\xb7\x9a" //.. .;.0s.||".&.. |
// /* 0730 */ "\x40\x4c\xd5\x69\x55\xd5\x3f\x02\x40\x2c\x79\x86\x03\x15\x6d\xa5" //@L.iU.?.@,y...m. |
// /* 0740 */ "\x48\x0f\xfc\x07\x20\xbe\x45\x82\x84\x20\x1e\x0a\x4a\x3c\xa9\x4b" //H... .E.. ..J<.K |
// /* 0750 */ "\x80\xac\x01\x70\x32\x72\x13\x00\x52\xfc\x13\x08\xbe\x09\xc5\x0f" //...p2r..R....... |
// /* 0760 */ "\xc1\x30\x13\x5d\x53\x80\x9f\x43\x7c\xa7\x1d\x27\xab\xc1\xe0\x99" //.0.]S..C|..'.... |
// /* 0770 */ "\x01\xfd\x40\x0a\x00\x20\x03\xec\x55\x0f\xd4\x95\x40\x09\xf0\xa4" //..@.. ..U...@... |
// /* 0780 */ "\x49\xb8\xbe\x2a\xf3\xda\x1f\x85\x1d\x30\x33\x00\xb0\x78\x56\x38" //I..*.....03..xV8 |
// /* 0790 */ "\x2c\xbc\x2b\x65\x3e\x15\x8e\x21\x0f\x3e\x3c\x2a\xf0\x7c\x06\xc0" //,.+e>..!.><*.|.. |
// /* 07a0 */ "\x84\x39\x9b\x4f\x82\x67\xdc\x52\x8a\x52\x02\x7b\xf8\x00\x16\x2f" //.9.O.g.R.R.{.../ |
// /* 07b0 */ "\xf7\x1d\x38\x1f\xee\xf8\xa8\xb8\xb8\x57\xf5\x08\x78\xe6\xc4\x25" //..8......W..x..% |
// /* 07c0 */ "\x4a\x57\x32\x00\x11\x08\x27\xea\x8a\x7f\xea\xc6\x9b\xfa\x9d\x2f" //JW2...'......../ |
// /* 07d0 */ "\xe4\xf8\x0e\x04\xfb\x6f\x5d\xb7\x8f\x71\xe3\x18\x7c\xf7\x43\x78" //.....o]..q..|.Cx |
// /* 07e0 */ "\x81\x45\xa6\xb5\xfb\xf8\xf4\x0c\x42\x08\xe3\x0c\xad\xd7\xbe\x8c" //.E......B....... |
// /* 07f0 */ "\xba\xcb\x67\xd7\x7f\xa3\x0a\x30\xcf\xc5\x62\x92\x58\xf7\xd1\x93" //..g....0..b.X... |
// /* 0800 */ "\x08\x22\xcd\x3b\xe8\xcd\x4e\x38\x79\x67\xf9\x32\xeb\x2d\xfb\x58" //.".;..N8yg.2.-.X |
// /* 0810 */ "\x0c\x51\xbe\x8c\xc4\xa2\x93\xaf\xe9\x87\x89\xdf\x26\x22\xd3\x5f" //.Q..........&"._ |
// /* 0820 */ "\xc6\x82\x7e\x38\xab\xcf\x67\xd7\x71\xdf\xa6\x81\xfc\xbd\x6e\xff" //..~8..g.q.....n. |
// /* 0830 */ "\x2e\xfa\x50\xa2\x51\x4a\xdd\x7a\xff\x7f\x7d\xb7\xbe\x44\xa2\x9e" //..P.QJ.z..}..D.. |
// /* 0840 */ "\x8d\x8e\xac\x39\xca\x94\xbc\x04\xe2\x93\x0b\xb6\xfe\x14\x51\x5f" //...9..........Q_ |
// /* 0850 */ "\xf4\xfc\x46\xfd\x3f\x8c\x67\xd3\xe3\x8a\x8b\x6b\x58\x0d\xc0\x80" //..F.?.g....kX... |
// /* 0860 */ "\x0a\x04\x31\x46\xf4\x7b\xa3\x74\xac\xd3\xba\x85\x9e\x3b\xa9\x80" //..1F.{.t.....;.. |
// /* 0870 */ "\x38\xb3\x28\xf6\xf7\xba\x2f\x00\xc0\x0b\x08\x40\x0c\x0c\x72\xcf" //8.(.../....@..r. |
// /* 0880 */ "\x77\x9e\x8e\x59\xdd\xaf\xf7\xfe\x63\x0e\x29\x95\x1a\xab\x79\x09" //w..Y....c.)...y. |
// /* 0890 */ "\x03\x71\x86\x1c\x51\xbc\x9b\x5e\x58\x3a\xf8\xf4\x3b\xce\xcd\x03" //.q..Q..^X:..;... |
// /* 08a0 */ "\x89\x45\x3e\xb4\xfc\xb4\x09\x56\x5b\x7b\xf7\x1c\xfd\x40\xc5\x19" //.E>....V[{...@.. |
// /* 08b0 */ "\xf6\xdf\xf5\x22\x07\x53\x8e\x1c\x51\xa3\xdf\x7e\x60\x77\x3f\x97" //...".S..Q..~`w?. |
// /* 08c0 */ "\xa2\xcf\x5a\x37\xcb\x8f\x35\xc7\x06\xa7\x9a\xef\x97\xdc\x00\x00" //..Z7..5......... |
// /* 08d0 */ "\x5c\x20\x1c\x1c\x96\xf7\xbe\x0d\x4b\xfd\xff\xc0\x2f\x27\x40\xef" //. ......K.../'@. |
// /* 08e0 */ "\xe8\x81\x0f\xd1\x88\x77\xe6\x7e\x8c\x90\x3f\x43\xf0\x3e\x18\x0f" //.....w.~..?C.>.. |
// /* 08f0 */ "\x9b\x2b\xf1\xd8\xaf\xe1\x7e\x3c\x8f\x7f\x1b\xd3\x33\x53\x8e\x7e" //.+....~<....3S.~ |
// /* 0900 */ "\x78\xf6\xee\xc0\xf0\x16\xd6\xb3\x8b\x81\xf0\x14\x0a\xbf\x7f\x18" //x............... |
// /* 0910 */ "\xa3\x0e\x41\x27\xd7\x75\x9a\x77\x50\x18\x1c\x64\xe5\xfa\xb7\xbd" //..A'.u.wP..d.... |
// /* 0920 */ "\xfb\x78\xc0\x1d\x7f\xbf\xc1\x7f\xf4\x70\x31\xaf\x75\x04\xfa\x63" //.x.......p1.u..c |
// /* 0930 */ "\x8e\x28\xce\x30\xcf\x57\x3a\x23\x52\xed\xbf\xdd\xc1\x1f\x11\x9d" //.(.0.W:#R....... |
// /* 0940 */ "\x7a\xe6\xbb\x28\x85\xbf\x0c\xc4\xba\x79\xc0\xa4\x0a\xf7\x5e\x55" //z..(.....y....^U |
// /* 0950 */ "\x96\xb5\x74\xfc\x87\xdb\x78\xc7\xd4\x36\x74\xd8\x40\xe3\xdf\x7d" //..t...x..6t.@..} |
// /* 0960 */ "\x53\x8e\x37\x88\x41\x02\xfe\x5c\xdc\xe3\x9d\x35\x81\x1f\x2c\xb3" //S.7.A......5..,. |
// /* 0970 */ "\x4e\xb9\xc7\x1d\x7e\x59\xe3\xe2\x3c\xd7\x18\x05\xcd\xe6\xbb\xac" //N...~Y..<....... |
// /* 0980 */ "\xf9\xac\x7d\xd6\xd4\x21\x04\x1b\x81\x70\xa9\x5b\xc7\x79\x9e\x0c" //..}..!...p.[.y.. |
// /* 0990 */ "\x20\x9d\xac\x5e\x78\x45\xa6\xbe\x4a\x0d\xc0\xab\x75\xe7\x18\x67" // ..^xE..J...u..g |
// /* 09a0 */ "\x01\x50\xcc\x0a\x7d\x77\x75\x96\xf6\xb9\x0c\x7e\xf2\x8c\x33\xaf" //.P..}wu....~..3. |
// /* 09b0 */ "\x60\x5a\x05\x1e\xbb\x8a\x49\x5a\x04\x20\x82\xb0\x2e\xc8\x08\x27" //`Z....IZ. .....' |
// /* 09c0 */ "\x57\x90\x9c\x08\xf2\xcf\x53\x8e\x76\xd4\x08\xfd\xb3\xac\xb7\xaa" //W.....S.v....... |
// /* 09d0 */ "\xe8\x43\x86\xe8\xc2\x08\xdd\xe5\xd4\x0a\x75\x56\x89\x45\x1c\x3a" //.C........uV.E.: |
// /* 09e0 */ "\xb3\x3c\x50\x14\x92\xf9\x64\x15\xf9\x11\x69\xaf\x57\x40\x47\xa5" //.<P...d...i.W@G. |
// /* 09f0 */ "\xd5\x79\xef\x0b\xf0\x84\x10\x1b\xe5\xdb\xb8\x84\x10\x13\xe5\xae" //.y.............. |
// /* 0a00 */ "\xff\x2c\xf3\xf2\xd0\x15\x18\x63\xdf\xc7\x71\x65\x54\x00\x48\x00" //.,.....c..qeT.H. |
// /* 0a10 */ "\x00\x7c\xa6\xe2\xdf\x5c\x59\xa3\x89\x04\xf5\x2c\x9e\xa7\x58\xf1" //.|....Y....,..X. |
// /* 0a20 */ "\x1c\x71\x73\x8a\x27\xaa\xe3\x93\xe7\x95\xcb\x5d\xfa\xac\x9f\xf5" //.qs.'......].... |
// /* 0a30 */ "\x3a\x96\xe2\x85\xdc\x7a\x39\x71\x43\x01\x17\x34\x27\x53\x88\xe7" //:....z9qC..4'S.. |
// /* 0a40 */ "\x81\x1f\x76\x39\x8d\x60\x89\xfe\x71\x42\xff\x2e\x6c\x4e\x6d\x0e" //..v9.`..qB..lNm. |
// /* 0a50 */ "\x63\x2e\x6e\x8a\x07\x37\xb7\x8b\xdc\xd8\xdc\xd6\xea\x7d\x1d\xe6" //c.n..7.......}.. |
// /* 0a60 */ "\xf5\x1e\x3a\x8e\x6c\x6e\x78\x58\x73\x7a\x9e\xac\x13\xcf\x0e\xab" //..:.lnxXsz...... |
// /* 0a70 */ "\xbe\x14\x83\xaf\x0a\x62\x71\xe1\x47\x40\x54\x06\xd1\x04\xa0\x02" //.....bq.G@T..... |
// /* 0a80 */ "\xc0\x1f\x3c\x1c\xe8\x0a\x80\xa5\xc5\x68\x00\xb0\x5b\x8f\x82\x63" //..<......h..[..c |
// /* 0a90 */ "\xa0\x03\xc3\x95\x57\x9e\xf4\x17\x8e\xad\x3a\x79\x7a\x8e\xfc\x15" //....W.....:yz... |
// /* 0aa0 */ "\x8f\x46\x00\x05\x2e\x00\xf9\x01\x50\x9f\x79\x79\xd2\x9a\x3c\x1a" //.F......P.yy..<. |
// /* 0ab0 */ "\xa5\xc1\x6e\x35\x00\x2a\x13\xc7\x55\xb7\xa8\x5e\x0f\x52\xfc\x1f" //..n5.*..U..^.R.. |
// /* 0ac0 */ "\x1c\xef\x5d\x29\xa3\xab\xbe\x1c\x03\x22\xab\xe0\xb8\xc0\x2b\xbb" //..])....."....+. |
// /* 0ad0 */ "\x75\x26\x70\x29\x50\x30\x1a\x13\xc1\x0e\xa5\x4e\xa6\x0c\x05\xe3" //u&p)P0.....N.... |
// /* 0ae0 */ "\x14\xff\x01\x00\x6c\x7c\x11\x47\x96\x7e\xa7\x0e\xa4\xee\xa5\xf3" //....l|.G.~...... |
// /* 0af0 */ "\x80\xc5\x9e\x4f\xac\x0c\xdc\xf0\x25\x91\x78\x11\x80\xce\xf4\xb0" //...O....%.x..... |
// /* 0b00 */ "\x00\xff\xf8\xe3\x6a\xa7\x84\x21\x08\x7d\x48\xb3\x9c\xdc\x20\x00" //....j..!.}H... . |
// /* 0b10 */ "\x3d\x58\xe2\xa0\x00\x06\x47\xfd\xf1\x21\xfe\xef\x3e\xc7\xa1\x1d" //=X....G..!..>... |
// /* 0b20 */ "\x58\xee\xc0\x02\xe9\xcc\xc8\x19\x40\x02\x08\x42\xec\x2c\xfe\xc8" //X.......@..B.,.. |
// /* 0b30 */ "\x70\x0f\x8a\xa0\x3f\xf9\xe2\x1b\xfc\xe3\xaa\x7e\x56\x00\x3f\xbf" //p...?......~V.?. |
// /* 0b40 */ "\x18\x0d\xef\x7e\x0d\x4e\x0e\x2f\x0a\xc7\x52\x5a\xbe\x00\xf2\x6a" //...~.N./..RZ...j |
// /* 0b50 */ "\x00\x0e\x70\x60\x50\xc2\x10\x87\x3e\xac\x78\x65\x00\xf0\xcc\xf6" //..p`P...>.xe.... |
// /* 0b60 */ "\xee\xdb\x87\x2c\x60\x15\xa4\x6a\xac\xc2\x10\x3d\x02\xbf\xf2\xad" //...,`..j...=.... |
// /* 0b70 */ "\x0f\xc0\x7d\xa4\x90\x75\x0c\xb0\x84\x2a\x80\x10\x21\x08\x0d\x00" //..}..u...*..!... |
// /* 0b80 */ "\x36\xce\x73\xc3\xd1\x70\x81\x78\x06\x6b\x9e\x01\xc6\xb5\xe0\x17" //6.s..p.x.k...... |
// /* 0b90 */ "\x3e\xc1\x00\x00\xfc\x06\x54\xa4\x3f\xd3\xc0\xd2\x95\xf1\xa0\x21" //>.....T.?......! |
// /* 0ba0 */ "\x0b\xe3\x44\x98\xc4\x7e\x05\x6f\x7b\x45\x29\x1e\x21\xf8\xe0\x42" //..D..~.o{E).!..B |
// /* 0bb0 */ "\x1f\x8e\xa7\xbd\xd0\xc6\x3a\x5e\x44\xb9\xcf\xe4\x55\x3f\xeb\xc7" //......:^D...U?.. |
// /* 0bc0 */ "\xc8\x7e\x6c\x64\xc6\x3c\xfb\x22\x03\xe0\x0a\xf0\x63\xee\x25\x4a" //.~ld.<."....c.%J |
// /* 0bd0 */ "\x44\x00\x02\x2d\xd8\x01\x19\x1a\xab\x75\xd7\x00\x24\xa5\x30\x70" //D..-.....u..$.0p |
// /* 0be0 */ "\x72\x50\x00\x02\x10\xa0\x5d\x09\x20\x39\xc7\xa1\x21\x46\x19\xd4" //rP....]. 9..!F.. |
// /* 0bf0 */ "\x71\xfa\xc6\x10\x4e\x85\x95\xc0\xf5\xe0\x08\x59\xa7\x0f\xd0\x92" //q...N......Y.... |
// /* 0c00 */ "\x7c\x09\x35\x38\xe3\xef\x58\x17\xe1\x8e\xa5\x74\x43\x01\xe9\x62" //|.58..X....tC..b |
// /* 0c10 */ "\xfa\x0f\xb1\x97\x57\x7b\xeb\x33\x0f\xfe\x01\x4c\xa2\x8c\x00\x1c" //....W{.3...L.... |
// /* 0c20 */ "\x85\xab\x01\x19\xbd\x59\x63\xde\xaa\x09\x40\x49\x08\x4b\x78\x38" //.....Yc...@I.Kx8 |
// /* 0c30 */ "\xa8\x00\x49\x29\x50\x12\xc6\x3b\x9c\xb7\xa0\x1a\x02\xfb\x6f\x1f" //..I)P..;......o. |
// /* 0c40 */ "\x81\x18\x41\x3c\x14\xfe\x11\xc0\x77\x62\x4e\x3c\xb5\x6b\x5a\xa0" //..A<....wbN<.kZ. |
// /* 0c50 */ "\x04\x70\x15\x59\x00\x7c\x32\xa1\x26\xb5\xc0\x45\x24\xa2\x1c\x3a" //.p.Y.|2.&..E$..: |
// /* 0c60 */ "\x15\x00\xa4\x52\x97\xf0\x0e\x17\x8e\x85\x6a\x3c\xea\xad\x70\xa0" //...R......j<..p. |
// /* 0c70 */ "\x70\x1d\x9e\x92\x3b\x10\x00\x78\xba\x30\x08\xda\xe7\x60\xc1\x3c" //p...;..x.0...`.< |
// /* 0c80 */ "\xef\xc0\xf8\x78\xf4\x97\xdf\xf4\xcf\x1f\xe7\x86\x03\x1e\x0d\x78" //...x...........x |
// /* 0c90 */ "\x4d\x70\xac\x78\x42\x47\xff\x3e\x23\xbf\x9c\x5b\x01\x49\x4a\x12" //Mp.xBG.>#..[.IJ. |
// /* 0ca0 */ "\xb8\x5b\x40\x60\x34\xf7\xbf\x42\x45\xce\x7f\xe8\x1a\x1e\xed\x48" //.[@`4..BE......H |
// /* 0cb0 */ "\x30\xf5\x84\xda\x3f\x7e\x36\x6f\xde\x00\xe1\x32\xe1\x3b\x43\xd0" //0...?~6o...2.;C. |
// /* 0cc0 */ "\x26\x94\xa1\x08\x20\xef\x05\xb7\xba\x03\x90\x19\x31\x8b\x1f\x41" //&... .......1..A |
// /* 0cd0 */ "\xc8\x45\x29\xb7\xbd\x8e\xc0\x81\x08\x6d\xef\x7d\x27\xc3\x30\x2a" //.E)......m.}'.0* |
// /* 0ce0 */ "\x18\xc5\x3d\xef\xa1\x14\xd0\xc6\x25\xce\x7d\x08\x82\xff\x6d\x4f" //..=.....%.}...mO |
// /* 0cf0 */ "\x7b\xe9\x5e\x16\xfd\x53\xd0\xae\x64\xc6\x36\xf7\xbe\x85\x61\x4f" //{.^..S..d.6...aO |
// /* 0d00 */ "\x05\x3a\x14\x84\xe0\x2d\x29\x5d\x2e\xc2\x30\x22\x00\x09\x52\x97" //.:...-)]..0"..R. |
// /* 0d10 */ "\x43\x07\x0a\x18\x81\x4a\x94\xa3\x55\x66\x01\xce\x16\x6e\x9d\xf1" //C....J..Uf...n.. |
// /* 0d20 */ "\xc2\xae\x03\x60\x47\x96\x75\x18\x67\x0a\xb0\x67\x4f\x71\x84\x13" //...`G.u.g..gOq.. |
// /* 0d30 */ "\xa1\xe8\x2f\x85\x23\xa1\xd8\x2b\x8d\x03\xa1\xb8\x27\x85\x40\x73" //../.#..+....'.@s |
// /* 0d40 */ "\xd0\x6a\x9c\x73\xa1\xc8\x1b\xa3\x50\xc2\x09\xd3\xac\x09\xc2\x98" //.j.s....P....... |
// /* 0d50 */ "\x42\xf4\x21\x04\xef\x03\x15\x38\xe7\x44\x10\x17\x10\x41\x84\x13" //B.!....8.D...A.. |
// /* 0d60 */ "\x9b\x23\xa6\xec\xbf\x70\xa8\xa1\xfe\x15\x15\xc1\xb9\xeb\x8b\xc0" //.#...p.......... |
// /* 0d70 */ "\x28\xd1\x03\x61\x08\x8d\x55\xb9\xf2\x47\xec\x0c\x51\xaf\xf7\xf1" //(..a..U..G..Q... |
// /* 0d80 */ "\xe6\x2e\x0f\xea\x07\x1d\x49\x80\xde\xeb\xdc\x75\xe3\xa0\x1d\x72" //......I....u...r |
// /* 0d90 */ "\x14\xc7\x92\x76\xdc\x90\x5c\x8c\x9c\x93\x63\xa0\x25\xd8\x00\x59" //...v......c.%..Y |
// /* 0da0 */ "\xb9\x27\x06\xb9\x25\xbc\xe2\x1c\xe4\x96\xfb\xeb\x84\x73\xfd\x4f" //.'..%........s.O |
// /* 0db0 */ "\x2a\x7d\xb9\x50\x39\xf5\x61\xca\x9e\xb9\x98\x6d\xce\xff\x6b\x77" //*}.P9.a....m..kw |
// /* 0dc0 */ "\x14\x85\xbd\xec\x03\x97\xab\x97\xd6\x1e\x4d\xaf\xe9\xb7\x35\xc7" //..........M...5. |
// /* 0dd0 */ "\x5d\x56\x02\x51\xae\x6c\x8f\x41\xb9\xb0\xb8\x73\x84\x39\xb0\xb8" //]V.Q.l.A...s.9.. |
// /* 0de0 */ "\x03\x2e\x11\x0f\xe9\x3c\xf4\x9f\xd2\xf0\x1a\x6b\x01\x23\x62\xe0" //.....<.....k.#b. |
// /* 0df0 */ "\x03\xc0\x11\x4f\x4d\x6b\xe4\x69\x14\xea\x5a\xd5\x40\x09\x84\x20" //...OMk.i..Z.@..  |
// /* 0e00 */ "\x16\x9e\xca\x53\x4e\x34\x92\xd4\x30\x27\x13\xab\x4c\x53\xe1\x4a" //...SN4..0'..LS.J |
// /* 0e10 */ "\x57\x00\x00\x2e\x74\xd1\x70\x00\x03\xea\x72\xf0\x2d\x92\xf8\x15" //W...t.p...r.-... |
// /* 0e20 */ "\xe6\x48\xf3\x5b\x9d\x35\xb0\x79\xb4\x9a\xd6\x1d\xc6\xbd\xce\xa8" //.H.[.5.y........ |
// /* 0e30 */ "\xca\x53\xe7\x07\xfa\x07\x01\x78\xf2\x3c\x64\xfe\xf7\x80\xef\xe3" //.S.....x.<d..... |
// /* 0e40 */ "\x9d\x54\xb8\xaa\xff\xa8\xc1\x01\x94\xf1\x81\x24\x22\x8e\x9a\xd6" //.T.........$"... |
// /* 0e50 */ "\x30\xd1\x7f\xbf\x8a\xc0\x54\x95\x69\x4c\x27\x22\xbc\xf2\xe1\x0e" //0.....T.iL'".... |
// /* 0e60 */ "\x2b\xe3\xa7\x80\x23\x05\xc1\x3c\x01\x68\xfe\x00\x8c\x07\xe6\xff" //+...#..<.h...... |
// /* 0e70 */ "\x4f\x00\x47\x3b\x67\x80\x1e\x3a\x04\xa6\xb5\xd4\x00\xa9\x6b\x40" //O.G;g..:......k@ |
// /* 0e80 */ "\x3c\x6a\xd4\x3f\x47\x55\x00\x25\x84\xe6\xa0\x05\x60\x20\x01\xe9" //<j.?GU.%....` .. |
// /* 0e90 */ "\x6e\xa1\xb2\x94\xd0\x02\x50\xe2\xb5\x00\x22\x1c\x0b\xb7\x34\x3f" //n.....P..."...4? |
// /* 0ea0 */ "\x2c\xbc\x33\x56\x5f\x0c\xe3\x63\xf0\xca\x8d\xe1\x81\x63\x18\x64" //,.3V_..c.....c.d |
// /* 0eb0 */ "\x05\xb9\xae\x99\x78\x9e\xf1\x95\x71\xc5\x0f\xf3\x93\x40\x84\x07" //....x...q....@.. |
// /* 0ec0 */ "\xb9\x96\xad\x6b\x5c\xcc\xe1\x94\xa4\x7e\x05\x2e\x73\x41\xe8\x14" //...k.....~..sA.. |
// /* 0ed0 */ "\xe6\xf3\x1f\x4a\x09\x08\x47\xe0\x50\xe7\x32\x00\x0f\xa5\x10\xaf" //...J..G.P.2..... |
// /* 0ee0 */ "\xa5\x30\x6f\xed\x32\x21\x01\x16\xe5\xdb\x7f\x11\xd2\x08\x40\x83" //.0o.2!........@. |
// /* 0ef0 */ "\x7d\xfe\xff\x50\x03\x99\x45\x00\x70\x5e\x51\xc2\x35\x56\xe3\xa4" //}..P..E.p^Q.5V.. |
// /* 0f00 */ "\x0f\xa0\x00\x42\x94\xbf\x2c\xf3\x5e\x84\xd6\xbf\xff\x5c\xf4\x1f" //...B..,.^....... |
// /* 0f10 */ "\x19\xe2\x38\xf1\xac\xfd\xae\x14\x52\x91\xf2\x18\x0f\xd9\xa2\xc6" //..8.....R....... |
// /* 0f20 */ "\x3f\xb3\x5c\xef\xdf\x19\x47\x39\xe8\x93\x0c\xa5\x23\xd0\x1e\x77" //?.....G9....#..w |
// /* 0f30 */ "\x71\xd0\xce\xba\x25\xe5\x63\x18\xc0\x45\x91\x80\xd2\x5d\xc8\x5e" //q...%.c..E...].^ |
// /* 0f40 */ "\xb5\x80\xef\x5a\xe2\x19\xeb\x55\x2a\xf1\x3d\x16\xe2\x2e\xb9\xb0" //...Z...U*.=..... |
// /* 0f50 */ "\x01\x7d\xea\xeb\x13\xb0\x2f\xb3\x54\x00\x6b\xc4\xf4\x12\xe1\x05" //.}..../.T.k..... |
// /* 0f60 */ "\xf3\xf4\x40\x6c\x63\x0d\xbc\x6a\x02\x5e\x24\x0f\xf3\xea\x73\x28" //..@lc..j.^$...s( |
// /* 0f70 */ "\x7f\x38\xe6\x70\xfe\x76\x37\xfc\xc2\xff\xcd\x48\xbc\x4f\x7c\x28" //.8.p.v7....H.O|( |
// /* 0f80 */ "\xa2\x7a\xdd\xcf\x7e\xdf\x39\x0f\x8f\x38\x19\xcf\x8f\xb6\xa7\x5e" //.z..~.9..8.....^ |
// /* 0f90 */ "\x27\xa2\x5f\xa9\xe3\xe6\x01\x8e\x90\x31\x2f\xbd\x8f\x3e\x70\x00" //'._......1/..>p. |
// /* 0fa0 */ "\x4f\x82\xde\x3e\x8f\xc5\xa0\x8f\xe2\xc7\x90\xc8\x00\x6f\xf1\x28" //O..>.........o.( |
// /* 0fb0 */ "\x9f\x87\x6f\xae\xfc\x37\xc8\xc2\x28\x00\x57\x89\xe8\x77\xdf\xd3" //..o..7..(.W..w.. |
// /* 0fc0 */ "\xc4\x5b\x06\xf8\x01\xf8\xe3\xf1\x69\xeb\x87\xe2\xbe\x83\x60\x0f" //.[......i.....`. |
// /* 0fd0 */ "\x0f\x3c\x4b\x1f\x85\xeb\xa4\x05\x14\xa5\xcb\xea\x03\x60\x3c\x19" //.<K..........`<. |
// /* 0fe0 */ "\x5d\x0a\x57\x0b\x97\xb0\x27\xc6\x5b\xf9\xc4\x15\xfb\xde\x10\x70" //].W...'.[......p |
// /* 0ff0 */ "\x12\xab\x97\xdf\xc6\xe1\xf7\xc2\x3e\x0d\x51\xef\x13\xd8\x0b\x8c" //........>.Q..... |
// /* 1000 */ "\x33\x89\x78\x63\xa7\x53\x89\x48\x5e\x03\x37\x9a\xf3\xfc\x0b\xf1" //3.xc.S.H^.7..... |
// /* 1010 */ "\x5c\x0f\x40\x4d\x64\xbc\xff\x82\xb8\x20\xec\x61\x04\x1e\xe7\x00" //..@Md.... .a.... |
// /* 1020 */ "\xc0\x40\x61\x48\x0b\xac\xb7\xa0\x5c\x7d\x4b\x99\xe0\x20\xc8\x12" //.@aH.....}K.. .. |
// /* 1030 */ "\xfd\x0e\x81\xe1\x1c\x0d\x4c\xc0\x2f\xa1\x08\x12\x08\x44\xea\xad" //......L./....D.. |
// /* 1040 */ "\x4e\x4d\x6b\x10\x80\xdc\xe3\x9c\x4b\x81\xf4\x26\x94\xae\x50\xe0" //NMk.....K..&..P. |
// /* 1050 */ "\xe8\x14\x88\x43\xa6\x43\x89\x60\x37\xa2\x01\x35\xaf\xa1\x08\x37" //...C.C.`7..5...7 |
// /* 1060 */ "\x89\xf3\xa0\xf8\x33\xd0\x47\x31\x08\x20\x5c\x0a\x05\xf8\x43\xbf" //....3.G1. ....C. |
// /* 1070 */ "\x76\x7a\x17\x4b\x4a\x57\x42\xe8\xf5\x39\xdb\xb5\x2b\x89\x60\x87" //vz.KJWB..9..+.`. |
// /* 1080 */ "\xee\x41\x63\x1f\x43\x60\x3f\xac\x1d\x0d\x00\xd0\x1a\x39\xce\x01" //.Ac.C`?......9.. |
// /* 1090 */ "\xb2\x02\xc0\x99\x7a\x58\x07\x78\x64\x3a\x20\xd7\x4d\x98\xe8\x83" //....zX.xd: .M... |
// /* 10a0 */ "\x36\x3e\x9b\x2e\x88\x28\x1f\x3e\x40\xc1\xa4\x97\x00\x2d\x3d\x85" //6>...(.>@....-=. |
// /* 10b0 */ "\x1c\xa0\x36\xe4\xe6\xf5\x4b\x93\xfc\x60\x1d\x37\x28\xc7\x28\x07" //..6...K..`.7(.(. |
// /* 10c0 */ "\x27\x45\x43\x9d\x53\x96\x44\x72\x8b\x8e\x50\x11\xc9\xd1\x50\xc0" //'EC.S.Dr..P...P. |
// /* 10d0 */ "\xe8\x40\x18\x08\xfd\x7f\x94\x5c\x72\x74\x54\x3a\xc8\x00\x7a\x18" //.@......rtT:..z. |
// /* 10e0 */ "\xcf\x2d\x9c\x79\x8c\xed\xcc\x31\xe1\x37\x86\x60\x16\x1b\xd0\x8a" //.-.y...1.7.`.... |
// /* 10f0 */ "\x58\x00\x79\x8b\x3c\x28\x18\x6a\xbf\xdf\xc5\xb9\x8b\x20\x20\xd0" //X.y.<(.j.....  . |
// /* 1100 */ "\xd1\x5e\x62\xce\x88\x2b\x80\xaf\x80\x27\xa4\x63\xc0\x16\x4d\xe0" //.^b..+...'.c..M. |
// /* 1110 */ "\x07\x43\xdd\x9b\xb9\xc8\x55\x00\x0a\x44\x21\xf1\x5c\x03\x5c\xe4" //.C....U..D!..... |
// /* 1120 */ "\x67\xc7\x15\x81\xd1\xdf\xe2\xf0\x4b\x34\x74\x09\x73\xe9\x60\x12" //g.......K4t.s.`. |
// /* 1130 */ "\x99\x60\x01\xe8\x1a\xe8\x26\x60\x10\xe8\x8c\xf4\x14\x74\x10\x74" //.`....&`.....t.t |
// /* 1140 */ "\x18\x74\x05\x54\x00\x70\x01\x3a\x03\xc1\x94\x69\x25\xf0\x2d\xf7" //.t.T.p.:...i%.-. |
// /* 1150 */ "\x0a\xdb\x00\x95\x66\x01\x40\xfc\xae\x88\x15\x1e\xbb\xff\xb7\x83" //....f.@......... |
// /* 1160 */ "\x63\xc2\x8a\x7c\x88\x43\x00\x9b\xca\x6f\x14\xd7\x45\x19\x10\x01" //c..|.C...o..E... |
// /* 1170 */ "\x03\xf3\xbb\xc0\x97\xd3\x51\xc6\xb5\xd1\x47\x30\x0a\x07\x9c\x63" //......Q...G0...c |
// /* 1180 */ "\xa2\x85\xfa\x12\x7f\xca\x1d\xe3\xa9\xf1\x83\xfb\x5f\x5c\xc8\xe8" //............_... |
// /* 1190 */ "\xa1\xc0\x01\x58\xc1\x09\x0a\x3c\x29\x4b\xa2\x7c\x0c\x79\x09\x47" //...X...<)K.|.y.G |
// /* 11a0 */ "\x91\x08\x0c\x7d\xa8\xff\xe8\x07\xff\x98\xbf\xff\xc6\x2b\xff\xe0" //...}.........+.. |
// /* 11b0 */ "\x1b\xd7\xea\x5c\x72\x93\x60\xbf\xf0\xe6\x40\x64\x17\x71\xf8\x08" //....r.`...@d.q.. |
// /* 11c0 */ "\x36\x01\x23\x07\xc1\x39\xac\xc0\x15\x9c\x7e\x03\x4c\x8e\x7c\xa8" //6.#..9....~.L.|. |
// /* 11d0 */ "\x90\xdd\xc7\xea\x00\xd4\xb3\x9e\x8d\x4f\x65\x81\x9d\x8a\x40\x5e" //.........Oe...@^ |
// /* 11e0 */ "\xbc\x5e\x99\x55\xa8\xa4\x4c\x74\x00\x99\x10\x86\x04\xc3\x61\x1e" //.^.U..Lt......a. |
// /* 11f0 */ "\x81\x0d\x12\x85\x1c\xe1\x7f\x15\x00\x1e\x91\x51\x32\x8a\x0f\xf6" //...........Q2... |
// /* 1200 */ "\xe0\x60\x17\x5a\x49\xe7\x4e\x51\x86\x73\xb3\x88\x94\x52\xcf\xce" //.`.ZI.NQ.s...R.. |
// /* 1210 */ "\xb5\x4f\x7b\xe8\x87\x58\x04\x22\x3e\x66\x05\x08\x43\xfd\xed\x43" //.O{..X.">f..C..C |
// /* 1220 */ "\xfc\x53\xd7\x02\x7e\x4a\xf2\xc7\xe6\x7c\x58\x7c\xcf\x02\x16\x0b" //.S..~J...|X|.... |
// /* 1230 */ "\xf8\x05\x82\x80\x06\x44\xc7\x01\x07\xa3\x53\x2d\x63\x14\x00\x3a" //.....D....S-c..: |
// /* 1240 */ "\x14\x73\x92\x5a\x99\x73\x5a\xb8\x74\xe4\x20\x84\x0f\xc0\x7c\x02" //.s.Z.sZ.t. ...|. |
// /* 1250 */ "\xe8\x0c\xf1\x8c\x74\x31\xab\x7f\x45\xf8\xf9\x8b\x5f\x4b\xa9\xe8" //....t1..E..._K.. |
// /* 1260 */ "\x7a\x81\x36\xf4\x4f\xbf\x21\xd7\xea\xae\x96\x91\x6f\xe8\x58\x19" //z.6.O.!.....o.X. |
// /* 1270 */ "\xf1\x9c\x73\xfd\xda\x80\x01\x60\xbf\xc0\x68\x95\xfa\x7d\xfc\x1c" //..s....`..h..}.. |
// /* 1280 */ "\x02\xc1\x4a\xb5\x8c\x51\xbc\x76\xa2\x39\xd3\xdd\x49\x69\x57\x35" //..J..Q.v.9..IiW5 |
// /* 1290 */ "\xa7\x6c\x0c\xcc\x90\x70\x0d\x9e\xc1\x2f\xe7\x7f\xf5\xe6\x67\xed" //.l...p.../....g. |
// /* 12a0 */ "\x8a\x2b\xa9\x2c\x06\x64\xd9\x40\x06\x93\x46\x68\x8e\xa4\xb4\x58" //.+.,.d.@..Fh...X |
// /* 12b0 */ "\x51\x17\x9a\x14\x00\x69\x09\x1b\x4f\xf5\x46\xff\xac\xf6\x48\xdc" //Q....i..O.F...H. |
// /* 12c0 */ "\x22\xff\x81\xa3\x82\xf4\x7b\x07\xfb\x50\x01\xdd\x49\x60\x53\x26" //".....{..P..I`S& |
// /* 12d0 */ "\xc5\x69\x35\x1e\x69\x19\xa0\x03\xff\xb9\x68\xe7\x3e\xf1\x65\x60" //.i5.i.....h.>.e` |
// /* 12e0 */ "\xb9\x8e\x0f\xe3\x14\xef\xd0\x02\x64\x25\xa7\x4b\x08\xf0\x75\xf8" //........d%.K..u. |
// /* 12f0 */ "\x8b\xf4\xdf\x10\xd7\xe9\xbe\xe4\x31\x40\x0f\xb0\x03\xc0\x60\x1f" //........1@....`. |
// /* 1300 */ "\xb1\x49\x64\x25\x80\xcb\x5b\x28\x00\xb3\x48\xcc\x05\x52\x4d\x49" //.Id%..[(..H..RMI |
// /* 1310 */ "\x82\x78\x76\x1d\xbc\x3b\x8f\xac\x9f\xd2\xf9\xfb\xc5\x00\x40\x7e" //.xv..;........@~ |
// /* 1320 */ "\xf0\x01\x60\x80\x94\xa4\xd0\x02\x8d\x65\x30\x15\x28\x9d\x12\xcc" //..`......e0.(... |
// /* 1330 */ "\x69\x83\xad\xdf\xc5\x44\x01\x71\xfe\x37\xe4\xee\x0f\xe3\x14\x00" //i....D.q.7...... |
// /* 1340 */ "\x2b\x31\xa5\x38\xd6\x53\x40\x02\x01\x71\xa7\x15\x28\xfe\x9b\xc1" //+1.8.S@..q..(... |
// /* 1350 */ "\x3b\x5b\x51\xc7\xef\x10\x19\xbd\xa2\x80\x10\x01\xfb\xd4\x7f\x0e" //;[Q............. |
// /* 1360 */ "\x0a\x12\x68\xa4\x92\x98\x0a\x6c\x4e\x80\x05\x74\x34\xea\xc7\x1e" //..h....lN..t4... |
// /* 1370 */ "\xec\x01\x06\x78\x93\x0f\x25\xbf\xdd\xf5\x65\x78\x91\xfd\x62\x89" //...x..%...ex..b. |
// /* 1380 */ "\x74\x34\xa0\x05\xff\x39\xa2\x06\x38\xfc\xa2\x8f\xf3\x7e\xfb\xe0" //t4...9..8....~.. |
// /* 1390 */ "\x86\xfd\xf7\xe8\xe0\x05\x3c\x02\xf1\x01\x8b\xa0\x50\x00\x3c\x0a" //......<.....P.<. |
// /* 13a0 */ "\xb0\x53\xee\x86\x90\x11\x79\x6d\x1a\xcd\x1c\xa5\x1a\x90\xfa\xd1" //.S....ym........ |
// /* 13b0 */ "\x4a\x2a\x21\xfe\x77\xc4\x9b\x83\xf6\xc5\x00\x04\xd4\x86\x02\xa4" //J*!.w........... |
// /* 13c0 */ "\xd7\x4e\xb3\x47\x28\x00\x25\x44\x3e\x1f\x53\x80\x48\x6e\x07\xf8" //.N.G(.%D>.S.Hn.. |
// /* 13d0 */ "\x17\x0e\x07\xe0\x35\x38\xe0\xf7\x48\x80\x87\x48\x87\xca\x40\x62" //....58..H..H..@b |
// /* 13e0 */ "\xd8\x1e\x0e\x07\x79\xe7\x87\xb9\x11\x30\x11\x94\xdd\xe7\xaa\x30" //....y....0.....0 |
// /* 13f0 */ "\xc1\xee\x9a\xec\x04\x60\x1a\xfe\x2c\x04\x5c\x1a\xe9\x64\xfd\x51" //.....`..,....d.Q |
// /* 1400 */ "\x50\x05\x91\x88\x71\x99\x98\xb7\x8c\xf1\x8a\x78\xcd\x49\x1f\x00" //P...q......x.I.. |
// /* 1410 */ "\x80\x99\x31\xf2\x46\x80\x88\x0b\xd8\x47\x24\x40\x44\x04\x55\x08" //..1.F....G$@D.U. |
// /* 1420 */ "\x7a\x04\x3c\x01\x70\x7b\x1e\x80\x9c\x10\xf4\x74\x03\xc0\x5c\x5a" //z.<.p{.....t...Z |
// /* 1430 */ "\xc7\xa0\x35\xc1\x8e\x8f\x00\x68\x0c\xdd\xcc\x7a\x05\x3e\x10\xc1" //..5....h...z.>.. |
// /* 1440 */ "\x85\x2e\x49\x2a\x2d\x9a\x39\x01\x1d\xb3\xf2\x4a\x51\x00\x48\x60" //..I*-.9....JQ.H` |
// /* 1450 */ "\x6b\x34\xec\x11\x4f\x1c\x93\xa0\x12\xa0\x02\xca\x52\x03\xe5\x17" //k4..O.......R... |
// /* 1460 */ "\x24\xd8\x03\xf2\x2b\x00\xee\x87\x7e\x7b\xcf\x31\x32\x8a\x0f\x60" //$...+...~{.12..` |
// /* 1470 */ "\xa8\x84\x4a\x28\x3b\xf3\x1d\x53\x50\x2b\xa4\x73\xd0\x11\xf8\xdf" //..J(;..SP+.s.... |
// /* 1480 */ "\x1e\x2e\x71\xc1\xff\x42\x2e\x0f\xbe\x66\x9e\x64\xbb\x73\x18\x78" //..q..B...f.d.s.x |
// /* 1490 */ "\x09\xcc\x92\x3a\x2e\x50\x05\x03\x99\x13\x99\x43\xa2\xdc\xf3\x22" //...:.P.....C..." |
// /* 14a0 */ "\x73\x28\x0c\x73\x22\x7e\x45\xc2\x31\xc0\x62\x78\xd1\xed\xd1\x6f" //s(.s"~E.1.bx...o |
// /* 14b0 */ "\x50\x0a\x77\x39\xa9\xe8\xab\x7c\xe6\xf0\xe6\xb9\x5d\x05\x76\xe8" //P.w9...|....].v. |
// /* 14c0 */ "\x1a\xc0\x3b\x78\x00\x50\x3a\x05\x3a\x22\xd4\x7a\xef\xd0\x81\xd4" //..;x.P:.:".z.... |
// /* 14d0 */ "\x92\x3a\x09\x3a\x0c\x04\x3a\x09\x3a\x0c\x2e\x11\x0f\x00\xcf\x41" //.:.:..:.:......A |
// /* 14e0 */ "\x2d\xb0\x0f\x45\x80\x28\x60\x21\x06\x02\x0f\x60\x21\xd8\xf7\x3a" //-..E.(`!...`!..: |
// /* 14f0 */ "\xa0\x03\x81\xd0\x9f\x6a\x2d\xef\x77\x00\x00\x3e\x89\x70\x60\x02" //.....j-.w..>.p`. |
// /* 1500 */ "\x07\xc7\xa9\xfe\x6f\x02\x89\xba\x25\xee\x81\x80\x9f\x63\xa2\x5c" //....o...%....c.. |
// /* 1510 */ "\x60\x6c\x68\xfe\xa0\xec\x05\x77\x3c\x63\x9f\xca\xfd\x5a\xe8\x8f" //`lh....w<c...Z.. |
// /* 1520 */ "\xc4\xb1\xd3\x49\x47\xb3\x9c\xf4\x47\x80\x0e\x88\x33\xfe\x40\x1f" //...IG...G...3.@. |
// /* 1530 */ "\xe6\x64\x7f\xe7\x19\x0f\xf9\x53\x38\x0d\xa9\x71\xca\x00\x1d\x82" //.d.....S8..q.... |
// /* 1540 */ "\xff\xdb\x45\xc7\x05\x00\xe8\x86\xa1\xd7\x84\xa9\xb8\x0a\xf9\x0c" //..E............. |
// /* 1550 */ "\xab\x09\x53\x70\x16\x62\xfb\x7f\x1f\xe8\x0a\xa0\x5f\xc4\x69\xd8" //..Sp.b......_.i. |
// /* 1560 */ "\x2a\x69\x63\xaf\x0f\xc0\x4a\xa4\xa3\xf8\x03\xad\xfb\x0c\x36\xfe" //*ic...J.......6. |
// /* 1570 */ "\x8d\xf4\x5d\xd1\xbe\x7a\x3e\xc5\x0f\xef\xad\x51\xce\x90\x38\x10" //..]..z>....Q..8. |
// /* 1580 */ "\xf4\x81\x4a\x26\xbf\x3b\x0f\xdd\x14\x7e\x30\x8f\x9d\x5f\x72\x0a" //..J&.;...~0.._r. |
// /* 1590 */ "\x64\xbf\x2a\x28\x17\xff\xf1\x52\xce\x8e\x6f\x03\x51\x2f\x85\x1f" //d.*(...R..o.Q/.. |
// /* 15a0 */ "\x9a\x80\xac\xe1\x3e\x73\x80\x9f\x39\xe2\x0f\xe7\x37\x33\xb5\xff" //....>s..9...73.. |
// /* 15b0 */ "\x01\xe0\x58\x7f\xd9\xc7\xbf\xec\xf3\xf3\xc7\xef\xdf\xf8\xe0\xfb" //..X............. |
// /* 15c0 */ "\x38\xff\x8d\xf9\xe1\xe3\xf7\xef\xfc\x70\x7d\x9c\x79\x9b\x6d\x4e" //8........p}.y.mN |
// /* 15d0 */ "\xe7\x34\xa0\x00\x07\xc0\x82\x10\xb8\x12\xa5\xce\x3f\x39\x00\x07" //.4..........?9.. |
// /* 15e0 */ "\xe0\x4b\x62\x9f\xa7\x1c\x01\xe6\x00\x21\xc0\xb1\xe0\xf8\x08\x10" //.Kb......!...... |
// /* 15f0 */ "\x85\x3b\x9b\x32\x41\x73\xe6\x32\xbc\x7e\x48\xf1\x37\x00\xfd\xe3" //.;.2As.2.~H.7... |
// /* 1600 */ "\xc4\xd7\xe5\xed\xb9\xb9\xe5\xf7\xef\xfc\x70\x7d\x9c\x7f\xc6\xdc" //..........p}.... |
// /* 1610 */ "\xd0\x72\xfb\xf7\xfe\x38\x3e\xce\x33\x7f\xb3\x8c\xd7\xec\xc7\x3b" //.r...8>.3......; |
// /* 1620 */ "\x84\x05\x45\xaf\x86\xc8\x4a\x02\x3c\x01\xc3\x58\x25\x01\x8d\x88" //..E...J.<..X%... |
// /* 1630 */ "\x0f\xc0\x56\x30\x0b\xaa\xf2\xf7\xc3\x60\x25\xc6\xd9\xc3\x60\x23" //..V0.....`%...`# |
// /* 1640 */ "\x01\xae\x8c\x60\x83\x90\x8c\x05\xdc\xa7\x86\xc8\x46\x02\x24\xc6" //...`........F.$. |
// /* 1650 */ "\x3d\x01\x1d\x00\xc1\x8b\xc1\x08\x0d\x68\x60\x7e\x02\x6b\xd8\x45" //=........h`~.k.E |
// /* 1660 */ "\x50\xe6\x08\x5c\x08\xc1\x7a\xc0\xff\xfd\x37\x25\xae\x63\x38\x5e" //P.....z...7%.c8^ |
// /* 1670 */ "\x46\x30\x08\x68\x3a\x03\x33\xc0\xdf\x01\xdf\xa3\x23\x24\x38\x04" //F0.h:.3.....#$8. |
// /* 1680 */ "\x5c\x1b\x01\xa9\xfe\x21\xef\xd1\x93\x82\x1c\x02\x42\x0c\xfe\x71" //.....!......B..q |
// /* 1690 */ "\x01\x3d\x00\xc0\x23\xa0\xbf\x01\x4a\x8f\x34\xa3\xe0\x46\x01\x25" //.=..#...J.4..F.% |
// /* 16a0 */ "\x05\x73\x66\xcc\x64\x87\x00\x97\xbd\x0f\x1c\x74\xb6\xe3\x8a\xe3" //.sf.d......t.... |
// /* 16b0 */ "\x6e\xe3\xa3\x1c\xbd\x2d\x03\x8e\x7b\x8e\xab\x97\xa4\x00\x0b\x71" //n....-..{......q |
// /* 16c0 */ "\xcf\x7d\xe5\xc0\x63\xd8\x13\xd1\xe7\x6e\x5d\xdb\x27\x72\x3d\x9c" //.}..c....n].'r=. |
// /* 16d0 */ "\x0c\x09\x18\x18\x2a\x39\x02\xe3\xe2\xcd\xb9\x52\xb9\x48\xb0\x04" //....*9.....R.H.. |
// /* 16e0 */ "\x24\x00\x20\x72\xa9\x72\xaf\x60\xaa\xd8\xe5\x42\xe7\x99\x58\x01" //$. r.r.`...B..X. |
// /* 16f0 */ "\xe1\x80\x27\x9f\xe6\xbc\x59\xb6\x0a\xf9\x40\xcf\x30\x6f\x10\x72" //..'...Y...@.0o.r |
// /* 1700 */ "\xcf\x4f\xc2\x61\xcf\xe8\xc0\xa6\x0a\x7b\x6e\x02\x49\x72\xdf\x40" //.O.a.....{n.Ir.@ |
// /* 1710 */ "\x84\x2e\x0c\x1c\xb7\xe5\x40\x07\x04\x94\x6f\x40\x02\xf4\xcd\x72" //......@...o@...r |
// /* 1720 */ "\xdf\x9e\x5b\x16\xdb\x80\x4b\x63\xf9\x9b\xbc\x4d\x3c\xed\x9f\xc2" //..[...Kc...M<... |
// /* 1730 */ "\xfc\x60\xe5\xbb\xa6\x11\x53\x2a\xcb\x79\x6a\xdd\x5e\x7b\x47\x6d" //.`....S*.yj.^{Gm |
// /* 1740 */ "\x6b\x18\xa4\x7f\x53\x07\xf5\x99\x87\xf5\xc6\x5d\xfd\x54\x85\x59" //k...S......].T.Y |
// /* 1750 */ "\x68\x8c\x06\x96\xb5\xfd\x55\x00\x3e\x6e\x2a\x44\xa9\x4a\x03\x10" //h.....U.>n*D.J.. |
// /* 1760 */ "\x8a\x50\xe0\x55\x6a\x6c\x8a\xad\xcb\x19\x42\x9d\x08\x40\x68\x55" //.P.Ujl....B..@hU |
// /* 1770 */ "\xea\x80\x54\x8a\xac\x0e\x01\x01\xbe\x5f\xdd\xec\x17\xfa\x43\x6b" //..T......_....Ck |
// /* 1780 */ "\x5c\x04\xea\xce\x2f\xe0\xf4\x06\xb3\xca\x39\x18\xab\x0f\xc0\x4d" //..../.....9....M |
// /* 1790 */ "\xf9\x44\x22\xbc\xb0\x02\x9c\xbc\x51\x28\xa0\xff\xac\x50\x1e\x55" //.D".....Q(...P.U |
// /* 17a0 */ "\xb1\xca\x8e\x39\x66\xc6\x00\xa0\x88\x27\xe7\x49\xff\x19\xb6\x18" //...9f....'.I.... |
// /* 17b0 */ "\x3f\xcc\x52\xbf\x0c\x7d\xd8\x92\x7d\x94\x27\xec\x8b\xf9\x64\x4f" //?.R..}..}.'...dO |
// /* 17c0 */ "\xd6\xc3\x97\x44\x7e\xf8\xe6\x65\x5f\x5a\x10\x7e\xb1\x16\x9c\x27" //...D~..e_Z.~...' |
// /* 17d0 */ "\xe8\x08\xc1\xb9\xbb\x0e\xf0\x0b\x10\xf0\xa4\x50\x03\x64\x55\x68" //...........P.dUh |
// /* 17e0 */ "\x19\x2d\x9d\x1a\xde\x09\x67\xb9\xf3\xa7\x8f\xd5\x18\x14\xa9\xfa" //.-....g......... |
// /* 17f0 */ "\xaf\x94\x8b\xe0\xa8\x00\x10\x2a\xb4\x06\x4f\x25\xc0\x03\xca\x00" //.......*..O%.... |
// /* 1800 */ "\x03\x11\x4f\x30\x38\x85\x77\x8d\x7b\x56\x70\xc0\x46\xde\xd1\x11" //..O08.w.{Vp.F... |
// /* 1810 */ "\x60\x3a\x06\x17\x10\x1a\xd0\x91\x64\x30\x19\xb1\x6d\x00\x31\xd4" //`:......d0..m.1. |
// /* 1820 */ "\x97\xc2\xeb\xf8\x3f\xec\x8c\x05\xa3\xb0\x27\x90\x6c\x67\x00\x23" //....?.....'.lg.# |
// /* 1830 */ "\xa0\x71\xa5\x69\xd2\xf2\xca\x31\x8c\x06\xd6\x69\x47\x64\x25\x78" //.q.i...1...iGd%x |
// /* 1840 */ "\xe6\x5f\x50\x4f\x12\x61\xc2\x7b\x60\x7f\x12\xd8\xaf\xea\x40\x6c" //._PO.a.{`.....@l |
// /* 1850 */ "\xc6\xb4\x25\x49\x0c\x06\x4a\x5b\x40\x05\x64\xf1\x44\x58\x3c\x51" //..%I..J[@.d.DX<Q |
// /* 1860 */ "\x3c\x27\xb6\x0b\xf1\x45\x88\x73\x25\x10\xd2\xd9\x09\x60\x90\xa5" //<'...E.s%....`.. |
// /* 1870 */ "\x69\x5a\xf2\x42\x33\x06\x0b\xd8\x73\x52\x60\x83\xa4\xc7\xc4\xb3" //iZ.B3...sR`..... |
// /* 1880 */ "\xe5\x37\xf5\xbe\xab\x71\xcd\x4a\x04\x3f\xa8\x87\xfe\x82\xc4\x60" //.7...q.J.?.....` |
// /* 1890 */ "\x84\x21\x07\x34\xaa\xd5\x4a\x35\x52\xa5\x8a\xca\x44\x79\xb4\x1b" //.!.4..J5R...Dy.. |
// /* 18a0 */ "\xb9\xa5\xbc\xb7\xc8\x2b\xe5\x71\x15\x47\x01\x10\x28\x00\xa2\xb2" //.....+.q.G..(... |
// /* 18b0 */ "\x98\x0b\xb4\xc6\x84\x4a\x5b\x01\x9b\xce\x68\xe2\xb2\x9f\x2d\xb9" //.....J[...h...-. |
// /* 18c0 */ "\xca\x56\x09\xaa\xd8\x67\xc4\x51\x50\x04\xad\xd7\x80\x7f\x22\x03" //.V...g.QP.....". |
// /* 18d0 */ "\x0f\x8c\xd0\x00\x41\x25\x30\x32\x8a\x55\x2e\xa5\x0a\xa9\x12\x51" //....A%02.U.....Q |
// /* 18e0 */ "\xc2\x94\x79\xcb\x9e\x39\xbe\xbc\x55\x5c\x0a\x76\x1c\xf1\x54\x7b" //..y..9..U..v..T{ |
// /* 18f0 */ "\x25\x40\x0c\x04\x94\xc0\x59\x66\x34\x23\x42\xd8\x0c\x9a\x73\xe2" //%@....Yf4#B...s. |
// /* 1900 */ "\x60\x07\x89\xa7\x84\x07\x05\x3f\xf2\x9b\xd4\x68\x00\x02\xa4\x34" //`......?...h...4 |
// /* 1910 */ "\xf0\x45\x1e\x76\xb2\x15\x80\x08\x9e\x17\x9c\x00\xf4\xfd\x77\xfd" //.E.v..........w. |
// /* 1920 */ "\x40\x1e\x50\x8f\x59\x07\x60\x67\x28\x55\xa5\xa8\xd6\x0c\xf1\x0a" //@.P.Y.`g(U...... |
// /* 1930 */ "\x90\xd3\x4d\x48\x7c\x30\x1f\xc0\xc5\xb3\x38\xfd\xb8\xe1\xc3\x78" //..MH|0....8....x |
// /* 1940 */ "\x03\xf2\x25\x88\xf1\xcb\x50\xf0\x02\xa1\xd8\xa6\x84\x0b\x25\x80" //..%...P.......%. |
// /* 1950 */ "\xd5\xce\x69\x86\xa4\x30\x55\x42\x97\x32\x98\x89\x8c\xc0\xc5\x41" //..i..0UB.2.....A |
// /* 1960 */ "\x2b\x8e\xc9\xdf\x9f\x89\x7b\x01\x62\x1f\x02\x38\xe2\x85\xf9\xf7" //+.....{.b..8.... |
// /* 1970 */ "\x38\xe2\x85\xf9\xd2\xb9\xfa\xd5\xd8\xe4\x7e\x03\x13\x52\x1f\xc0" //8.........~..R.. |
// /* 1980 */ "\x00\xe1\xf8\x0f\x37\x08\x1e\x00\xc0\xcc\x61\x6e\x7b\x21\xf8\x0c" //....7.....an{!.. |
// /* 1990 */ "\xeb\xa0\xc5\x8a\xa1\x73\xde\x81\xf8\x0c\xc1\xcf\x9d\x7c\x60\x67" //.....s.......|`g |
// /* 19a0 */ "\x50\xaf\x81\x43\xfc\xe6\x9f\xfa\x84\x98\xff\xd5\x1e\x80\x60\x68" //P..C..........`h |
// /* 19b0 */ "\x90\xa4\x06\x77\xf6\xce\xff\xd9\xe1\x58\xe8\x47\x07\x80\x18\x1a" //...w.....X.G.... |
// /* 19c0 */ "\x4c\x27\xd0\xda\x6e\xc7\x21\x9e\x88\x8e\x14\x4f\x44\x5f\x71\x80" //L'..n.!....OD_q. |
// /* 19d0 */ "\x07\x2b\x27\x2b\x87\x28\x97\x3c\xab\xe7\x83\x5c\x9b\xd5\x79\xef" //.+'+.(.<......y. |
// /* 19e0 */ "\x2b\xe9\x78\xf7\x4c\xfe\xa7\xf4\xbe\x16\x8e\x78\xba\x2a\xcb\x79" //+.x.L......x.*.y |
// /* 19f0 */ "\x8d\x25\x4a\x5c\xef\xe3\x99\x2e\x1c\xbc\x2b\x89\xeb\xf9\x5f\x29" //.%J.......+..._) |
// /* 1a00 */ "\xa0\x07\x38\x16\x12\x8c\x5f\xef\xf8\x3f\x65\x85\xa3\x5d\xea\xdc" //..8..._..?e..].. |
// /* 1a10 */ "\xe2\x1c\x0a\xc2\x1c\xe2\x1c\xe3\x97\x07\x0f\xe7\x38\x1f\xd9\xfc" //............8... |
// /* 1a20 */ "\xef\x04\x45\x30\x2b\xb6\x9d\x52\x9c\xea\xe1\xc0\x07\x84\xe6\xfd" //..E0+..R........ |
// /* 1a30 */ "\x11\x23\xc8\xc8\x0a\xbd\x76\x97\x4f\x9d\x5c\x2e\x80\x3c\xed\x20" //.#....v.O....<.  |
// /* 1a40 */ "\x82\x1d\x82\xda\x9e\x0e\x8c\x03\xdc\xfd\x77\xc2\x99\xce\x99\x4f" //..........w....O |
// /* 1a50 */ "\x9d\x40\x62\x34\xbf\xdd\x1d\xeb\xde\x13\x8e\x2c\xe2\x08\x77\x3b" //.@b4.......,..w; |
// /* 1a60 */ "\x95\xc2\xd5\xfb\xe6\x7d\xfb\xf1\x9e\x7e\xf7\xe2\x60\x4b\x27\x3a" //.....}...~..`K': |
// /* 1a70 */ "\x3d\x01\xf0\x3e\x78\x7f\x05\x42\x07\xf0\x49\x98\x7e\x03\x81\x16" //=..>x..B..I.~... |
// /* 1a80 */ "\xd8\x1e\x45\x82\x1a\x63\x02\x4e\xc6\x0d\xa6\x04\x70\x65\x60\x47" //..E..c.N....pe`G |
// /* 1a90 */ "\xc6\x07\x83\x0f\xe0\xa9\xc8\xfa\x0a\x00\x0d\x0c\x63\x08\x60\x40" //............c.`@ |
// /* 1aa0 */ "\x0f\xd8\x54\x02\x68\x63\x1c\xf2\xcf\x34\xec\x29\xc0\x07\xf4\xa0" //..T.hc...4.).... |
// /* 1ab0 */ "\x7f\xc1\xaa\x00\x0c\xca\x29\x84\x5c\x27\x9c\xda\xa0\x13\x32\x8a" //......)..'....2. |
// /* 1ac0 */ "\x7e\x04\x18\xc6\x80\x03\x81\x59\x00\x47\x14\x60\x1e\x23\xca\x86" //~......Y.G.`.#.. |
// /* 1ad0 */ "\x01\xe3\x7d\x54\x98\xc7\xc3\xef\x0b\xc7\x87\xe3\xc4\xf1\xe3\x09" //..}T............ |
// /* 1ae0 */ "\xf1\xcc\x79\x06\x3c\x93\x1e\x51\x8f\x2c\xc7\x98\x62\x9e\xf6\x31" //..y.<..Q.,..b..1 |
// /* 1af0 */ "\xe8\xe8\xc2\x3f\x4f\x47\x59\xb7\xa3\x9c\xeb\xb0\xc1\x1f\x6a\x00" //...?OGY.......j. |
// /* 1b00 */ "\x6f\x95\x98\x4c\x26\x30\x15\xb3\x82\x44\x50\x03\x7c\x8c\x01\x91" //o..L&0...DP.|... |
// /* 1b10 */ "\x61\x70\xf6\x12\x4e\x06\xf8\xf4\x03\xe7\x28\xfd\x6a\x07\xb5\xa7" //ap..N.....(.j... |
// /* 1b20 */ "\x9f\x63\xc1\x17\x85\x42\xf0\xc1\xe2\x4a\x18\xc7\xfa\x91\x09\x94" //.c...B...J...... |
// /* 1b30 */ "\x53\x07\x8d\x61\xe3\x0d\xfd\xee\xe6\xc9\x03\x9f\xb9\xec\x2c\x28" //S..a..........,( |
// /* 1b40 */ "\x01\x9f\xda\xbd\x65\x85\x00\x33\xc7\x0f\x54\xdd\x00\x33\x01\x12" //....e..3..T..3.. |
// /* 1b50 */ "\x3c\x61\x47\xde\x08\xc4\xf3\xc1\x0e\x7c\xb8\xf8\x25\x44\xf3\xc5" //<aG......|..%D.. |
// /* 1b60 */ "\xd0\x02\xbc\x13\x54\x4f\x00\x3c\xf1\x87\xbf\x94\x4f\x3d\xe5\x86" //....TO.<....O=.. |
// /* 1b70 */ "\xcb\x57\x09\xe7\x83\xa5\x07\xc0\x6f\x07\x51\xff\x83\x9c\xfe\xf1" //.W......o.Q..... |
// /* 1b80 */ "\xf0\x72\x8f\xe0\xe7\x96\xb0\x05\xf0\x8d\x51\xfc\x22\xf2\xee\x00" //.r........Q."... |
// /* 1b90 */ "\xb3\x28\xa6\x02\xf2\x79\x83\x07\xcf\x08\xa7\x3e\x11\x95\x0f\x08" //.(...y.....>.... |
// /* 1ba0 */ "\xe2\x9d\xe1\x17\x41\xcd\x7c\x2a\xa5\xf8\x55\xe6\x39\xe1\x75\x00" //....A.|*..U.9.u. |
// /* 1bb0 */ "\x30\x21\x0c\x1b\xe3\x34\xbf\x0b\xbc\xd1\x80\x07\xcc\x88\x73\x98" //0!...4........s. |
// /* 1bc0 */ "\x0d\x2f\xc3\x8f\x36\xdd\x03\x04\xef\xbf\xfa\xe1\x41\xf1\x04\xa5" //./..6.......A... |
// /* 1bd0 */ "\x78\x83\x14\x8f\x10\x3a\x23\xeb\x70\x80\x80\xf0\xad\x78\x6e\x00" //x....:#.p....xn. |
// /* 1be0 */ "\x70\x78\x38\xad\x80\x87\x8c\x51\xc6\x19\xe1\x3a\xc2\x78\x90\xbb" //px8....Q...:.x.. |
// /* 1bf0 */ "\x6f\xf1\x50\x07\xf9\x3c\x25\x15\x00\x1f\x44\xc5\x6e\xbd\xe2\x97" //o.P..<%...D.n... |
// /* 1c00 */ "\x01\x9b\xc2\x8b\xef\x0d\x5a\x07\x86\x9d\x1a\xf1\xf3\xb3\xd8\x11" //......Z......... |
// /* 1c10 */ "\x50\xc0\x75\x60\x18\x0a\x6b\x5f\xb5\xde\x74\x8f\x60\x46\x09\xf5" //P.u`..k_..t.`F.. |
// /* 1c20 */ "\xab\x9c\x73\xda\xea\x00\x2f\x84\x9c\xaa\x80\x3c\x00\x5e\xcc\x89" //..s.../....<.^.. |
// /* 1c30 */ "\x5f\x84\xa4\xf7\xc2\x58\xe1\x42\x77\x48\xdc\xfc\x27\x1c\x19\x1e" //_....X.BwH..'... |
// /* 1c40 */ "\x66\x82\x4f\x06\xcf\x05\xe7\x98\xf0\xe0\xea\xc0\x97\x31\xe5\xfc" //f.O..........1.. |
// /* 1c50 */ "\x38\x35\x44\x9f\xc1\xb3\xc4\x03\x6e\x94\x18\x78\x2e\x38\x7c\x3c" //85D.....n..x.8|< |
// /* 1c60 */ "\xbd\x87\x83\xa7\x87\xc4\xf9\x6e\xb8\x7c\x47\x96\x30\xf5\x3d\xf1" //.......n.|G.0.=. |
// /* 1c70 */ "\x70\xf2\x30\x51\x36\xde\x7c\x8f\x27\x87\x7f\x73\x0b\x02\xd5\xee" //p.0Q6.|.'..s.... |
// /* 1c80 */ "\xbc\x7b\xfc\xe3\xce\xf8\x71\xe5\xaf\x3a\x57\x1e\x58\xf1\x6d\xe0" //.{....q..:W.X.m. |
// /* 1c90 */ "\x1d\x07\x81\x6e\x67\xe0\x57\x4d\x1c\x79\x22\x39\x7f\x86\x39\x27" //...ng.WM.y"9..9' |
// /* 1ca0 */ "\xcf\x25\x78\xe4\xbf\x1c\x99\xe3\x93\x7c\x72\x77\x8e\x50\x13\xca" //.%x......|rw.P.. |
// /* 1cb0 */ "\x26\x39\x48\xc7\x29\x95\x47\x0b\x42\xa8\x7e\xa6\xcb\x85\x1c\x54" //&9H.).G.B.~....T |
// /* 1cc0 */ "\x65\xce\x60\x3c\xba\xdc\xbc\x43\xf5\x1b\x18\xc0\x0d\x0f\xc7\xef" //e.`<...C........ |
// /* 1cd0 */ "\x5a\x61\xcb\xd9\x51\x14\x92\xe0\xb9\x7a\xf5\xdd\x72\xf4\x8e\x5f" //Za..Q....z..r.._ |
// /* 1ce0 */ "\x6e\x67\x11\xe1\x97\x84\x5e\x19\x9f\x10\xc7\x8a\xa7\xc6\x71\xe3" //ng....^.......q. |
// /* 1cf0 */ "\xb8\xf2\x1c\x79\x2e\x3c\xa7\x1e\x5b\x54\x50\x42\x3d\x0b\x3c\xf9" //...y.<..[TPB=.<. |
// /* 1d00 */ "\xf6\xf3\xcb\xd9\xae\x76\x88\x0d\x7a\x17\x40\x05\x1e\x7b\xe7\xba" //.....v..z.@..{.. |
// /* 1d10 */ "\xe9\xb1\x31\xec\x07\x48\xfd\xf3\xd1\x07\x6a\x58\xf3\xdc\x06\x3f" //..1..H....jX...? |
// /* 1d20 */ "\x86\xc2\xa3\xa2\x54\x7e\x25\x2e\x3c\x0f\xd3\x65\xd1\x29\xfa\xe3" //....T~%.<..e.).. |
// /* 1d30 */ "\xa4\x7e\xdd\x21\xd8\x17\xc3\x0e\x91\xef\xc1\x1d\x52\x36\xc0\xb9" //.~.!........R6.. |
// /* 1d40 */ "\xf7\xd3\x2d\xc0\x95\xd3\x5d\xc1\x02\x30\xb0\x2b\x82\x85\x60\x1c" //..-...]..0.+..`. |
// /* 1d50 */ "\x24\xa9\x4b\xa6\x1c\x44\x00\x70\x46\x6f\x07\x8e\x5e\x40\x74\xc3" //$.K..D.pFo..^@t. |
// /* 1d60 */ "\xb6\x00\x3e\xa8\x51\x81\x73\xb0\x95\x29\x07\x78\xa3\x8b\xa6\x4f" //..>.Q.s..).x...O |
// /* 1d70 */ "\x0b\xc0\x1f\x5a\x76\x01\xb5\xbc\x03\x82\x95\x29\x7f\x48\xc0\x35" //...Zv......).H.5 |
// /* 1d80 */ "\xdb\x60\x3a\xe7\xbf\xc2\x61\x9c\x09\xa7\x05\x3d\x38\x51\x1d\x39" //.`:...a....=8Q.9 |
// /* 1d90 */ "\x5f\xc7\x00\xf8\xc1\x53\x6c\x1b\x62\xc0\x27\xa1\x81\xe8\x0d\xbd" //_....Sl.b.'..... |
// /* 1da0 */ "\xed\xd4\x00\x00\x2b\x84\x23\x81\x75\x70\x47\x8c\x21\x08\x80\xe0" //....+.#.upG.!... |
// /* 1db0 */ "\xa8\xe4\x06\x5c\xe7\x82\x96\x01\x3c\x0f\x67\xed\xc7\x05\xe8\xf0" //........<.g..... |
// /* 1dc0 */ "\x6c\x60\xfa\xd8\xc2\x06\xef\xf4\x77\x00\x00\x08\xe0\xb1\x1e\x15" //l`......w....... |
// /* 1dd0 */ "\x9e\x0e\x11\xff\xa3\xf1\xc2\x29\x4e\x01\xf7\x90\x42\x11\x7e\xe0" //.......)N...B.~. |
// /* 1de0 */ "\x0f\x9c\x10\xc1\xfd\x4a\x15\x2d\x6b\xc2\xc0\xbf\x26\x04\x38\x60" //.....J.-k...&.8` |
// /* 1df0 */ "\xa8\x46\x92\x5a\x80\x1e\x0c\x93\x77\x2d\x62\x0b\x46\x00\xc5\x96" //.F.Z....w-b.F... |
// /* 1e00 */ "\xcf\x40\x62\xd5\xe8\x00\x54\x21\x84\x21\x94\xa5\xa5\x29\x35\xad" //.@b...T!.!...)5. |
// /* 1e10 */ "\x46\x18\x69\x25\x38\xa3\x0e\x59\xc8\x61\xb7\xba\xf0\xc5\x18\x52" //F.i%8..Y.a.....R |
// /* 1e20 */ "\x4a\x61\x84\x2b\xc5\x08\x48\xa5\x28\x21\x10\x21\x08\x31\x8c\x29" //Ja.+..H.(!.!.1.) |
// /* 1e30 */ "\x49\xb5\xad\xc6\x19\x70\x85\x01\xc5\x90\x42\x2c\x63\x10\x63\x12" //I....p....B,c.c. |
// /* 1e40 */ "\x84\x2a\x5a\xd6\x03\x30\x41\x26\x51\x43\x4c\x10\xb0\xb7\xbd\xa6" //.*Z..0A&QCL..... |
// /* 1e50 */ "\xb5\xca\x94\x82\x10\x99\x4a\x68\x63\x04\x00\x0c\x21\x00\x84\x2d" //......Jhc...!..- |
// /* 1e60 */ "\xad\x66\x18\x42\xfd\xca\x19\x03\xc3\x81\x50\xab\x2d\xe4\x39\x50" //.f.B......P.-.9P |
// /* 1e70 */ "\xcf\x1e\x63\x0a\xf1\xe2\x00\x00\xab\x2d\x59\xa7\x1e\x59\xeb\xf7" //..c......-Y..Y.. |
// /* 1e80 */ "\xfa\xdd\x7a\xef\x3d\xb9\xe0\x89\x40\x63\x18\x42\x11\x21\x09\x06" //..z.=...@c.B.!.. |
// /* 1e90 */ "\x0e\x86\x49\x8c\x65\x4a\x50\xe7\x38\x96\x22\x1c\x62\x23\x88\xc4" //..I.eJP.8.".b#.. |
// /* 1ea0 */ "\x19\x35\x53\x8e\x53\xde\xed\x6b\x56\x73\x90\x4e\x15\x0b\x18\xc5" //.5S.S..kVs.N.... |
// /* 1eb0 */ "\x1c\xe7\x94\xda\x00\x07\x17\x87\x39\x98\x79\x18\x29\x25\x71\x86" //........9.y.)%q. |
// /* 1ec0 */ "\x53\xde\xcb\x18\xfc\x8d\x94\x08\x40\x10\x86\x4c\x62\x92\x95\x6b" //S.......@..Lb..k |
// /* 1ed0 */ "\x5a\xfc\x51\x53\xc4\x3e\x6f\xc5\x04\x30\x88\x52\x95\x9c\xe5\x14" //Z.QS.>o..0.R.... |
// /* 1ee0 */ "\xa6\x45\xc5\x0d\x02\x10\x24\x21\xa3\xe0\x81\xe7\x97\x2b\x04\xcb" //.E....$!.....+.. |
// /* 1ef0 */ "\x3c\xb9\x78\x99\xee\x30\x8c\x12\x65\x14\x8b\x4d\x63\x55\x6a\x14" //<.x..0..e..McUj. |
// /* 1f00 */ "\x7a\xef\x1a\x8d\x55\xa2\xd3\x59\x94\x52\x5c\xe7\x4b\x5a\xa3\x9c" //z...U..Y.R..KZ.. |
// /* 1f10 */ "\xd8\xf9\x84\x41\x0d\x9c\xe6\xd6\xb5\x4f\x7b\xa9\xc7\x26\xd3\x59" //...A.....O{..&.Y |
// /* 1f20 */ "\xd6\x22\xe7\x3e\xdb\xde\x69\xd8\x85\x62\x21\xce\x72\xa5\x2e\x01" //.".>..i..b!.r... |
// /* 1f30 */ "\x31\x3a\xab\x5e\xeb\xde\x5b\x7a\x3d\x53\x8e\x1a\x78\xdd\x42\x6d" //1:.^..[z=S..x.Bm |
// /* 1f40 */ "\x6b\x8a\xa8\xc3\x25\xce\x61\x8c\x6d\x29\x54\x73\x92\x42\x18\x17" //k...%.a.m)Ts.B.. |
// /* 1f50 */ "\x8f\x52\x2c\x2c\x7c\xb3\x9c\xc2\x94\x8b\x18\xe9\xef\x6e\x38\x8c" //.R,,|........n8. |
// /* 1f60 */ "\x49\x53\x8e\x18\x41\x21\xce\x76\xb5\xa4\x94\xa2\x18\x2f\x01\xd8" //IS..A!.v...../.. |
// /* 1f70 */ "\x71\xc3\xa8\xaa\x5a\xd6\x09\xc4\x70\x98\x74\x1a\x79\x92\xa2\x7e" //q...Z...p.t.y..~ |
// /* 1f80 */ "\x3c\x79\x92\xa2\x3f\x49\xf8\x85\x08\x44\x00\x10\xa5\x2b\x9c\x71" //<y..?I...D...+.q |
// /* 1f90 */ "\x76\xde\xb3\x81\x83\x50\x63\x1a\x18\xc1\x63\x1c\x5a\x6a\x43\x0c" //v....Pc...c.ZjC. |
// /* 1fa0 */ "\x9b\x4d\x53\x5a\xec\x63\x0c\xa5\x34\xf7\xb3\x8c\x39\xbe\x14\x92" //.MSZ.c..4...9... |
// /* 1fb0 */ "\xf1\xe3\x04\x04\x21\x79\x29\x5b\xaf\x41\x5c\x61\x81\x08\x48\x21" //....!y)[.A.a..H! |
// /* 1fc0 */ "\x12\xa5\x21\x49\x2f\x99\x10\x5b\x5a\xd3\x29\x49\x00\x04\x98\xc7" //..!I/..[Z.)I.... |
// /* 1fd0 */ "\x0b\x81\xa5\x4a\x51\x28\x52\x90\x27\x97\x81\x08\x52\xe7\x33\x49" //...JQ(R.'...R.3I |
// /* 1fe0 */ "\x2c\xb9\xcc\xa1\x0a\x04\x21\x63\x18\x85\x29\x61\x79\x38\xe9\x84" //,.....!c..)ay8.. |
// /* 1ff0 */ "\x39\x20\xc4\x53\x0b\x5a\xd1\x17\xf4\x22\xff\x45\x1c\xe5\x35\xad" //9 .S.Z...".E..5. |
// /* 2000 */ "\x93\x85\x0e\x3c\x2a\xc3\x9e\x63\x88\xc7\x98\xf1\xc1\x5e\x20\x08" //...<*..c.....^ . |
// /* 2010 */ "\x3b\xed\xbc\xe2\x8d\x85\x9f\x85\x36\x31\x84\xd6\xb9\xb4\xd4\x86" //;.......61...... |
// /* 2020 */ "\x31\x16\x9a\x96\x31\xd0\xf8\x80\x2d\xef\x6b\x3c\x26\x17\x38\xe4" //1...1...-.k<&.8. |
// /* 2030 */ "\x2b\x0c\x24\x0a\x10\xa2\x51\x4b\xdd\x7a\x75\x86\xff\xd6\x73\x96" //+.$...QK.zu...s. |
// /* 2040 */ "\x10\x8a\x5a\xd1\xa6\x1e\xa6\x0c\x51\xb0\x9d\x8c\x5a\xf8\x51\x44" //..Z.....Q...Z.QD |
// /* 2050 */ "\x00\x02\xa7\xc3\x6c\xd2\x95\x63\x18\x91\x08\x14\x79\x69\x16\x9a" //....l..c....yi.. |
// /* 2060 */ "\x9a\x71\x5d\x58\xc6\x01\x08\x61\x4a\x5c\x26\x60\xb1\x8e\x4c\x62" //.q]X...aJ.&`..Lb |
// /* 2070 */ "\x40\x00\xcb\xe5\xa4\x4a\x28\xe3\x0c\xa5\xf9\x70\x82\x11\x47\x39" //@....J(....p..G9 |
// /* 2080 */ "\x6d\x6b\xb9\xc7\x2d\xef\x7f\xda\x03\x18\xd9\xfa\x44\x38\x06\x67" //mk..-.......D8.g |
// /* 2090 */ "\x83\x7d\x65\x5b\xcd\x18\xaa\x79\xa1\x0f\x38\xa3\x46\xaa\xcd\xb8" //.}e[...y..8.F... |
// /* 20a0 */ "\x38\xa4\x18\xc7\xf2\xa1\x35\xac\x03\xe5\x16\x69\xcf\x34\xe2\xcc" //8.....5....i.4.. |
// /* 20b0 */ "\x18\x54\x41\x08\x0f\xe4\xac\xed\x0e\x03\x86\x38\xc3\x26\x51\x44" //.TA........8.&QD |
// /* 20c0 */ "\xd6\xbb\x39\xc8\x63\x1f\x06\x08\x2c\x63\xa3\x9c\x96\x31\xc4\xb8" //..9.c...,c...1.. |
// /* 20d0 */ "\x74\x08\x79\x3f\x84\xbf\xab\xd9\xb6\xe1\xe1\x36\x0a\x98\x9f\x0e" //t.y?.......6.... |
// /* 20e0 */ "\x3c\x4d\x6a\x5f\x4d\xd9\xda\x03\xe1\xe8\x21\x17\xba\xf5\x5e\x7b" //<Mj_M.....!...^{ |
// /* 20f0 */ "\x0a\x52\xc2\x3c\xd0\x39\x67\x9b\x4d\x7e\x54\x5b\xde\xcf\x2c\xf1" //.R.<.9g.M~T[..,. |
// /* 2100 */ "\x69\xab\x4a\x55\x0c\x62\xe1\x12\x03\x62\x8b\x4d\x61\x8a\x3c\x19" //i.JU.b...b.Ma.<. |
// /* 2110 */ "\x4d\xad\x78\x78\x0d\x0c\xb5\xad\x7c\xaa\x30\x82\x52\xfe\x50\x47" //M.xx....|.0.R.PG |
// /* 2120 */ "\x02\xd6\x15\x65\xb8\xa0\x7d\x5e\xf1\x73\x86\xad\x70\x28\x78\x92" //...e..}^.s..p(x. |
// /* 2130 */ "\xf8\xc1\x80\x22\x22\x10\x4f\x05\xc2\x4f\xae\x42\x18\x75\xc8\xeb" //..."".O..O.B.u.. |
// /* 2140 */ "\x2d\x49\x4a\x83\x18\xbc\xd3\xa6\x5f\x71\x87\xb2\xe2\x0c\x62\x9a" //-IJ....._q....b. |
// /* 2150 */ "\xd6\x43\x03\xf7\x9c\x3c\xd3\xaa\x00\x55\x38\xe0\x2c\x1f\xea\x41" //.C...<...U8.,..A |
// /* 2160 */ "\x63\x1d\xfe\xfe\x58\xc7\xfa\x23\xd4\xd1\x16\x87\x39\xdc\xfc\x5f" //c...X..#....9.._ |
// /* 2170 */ "\x25\x08\x5f\x2b\x04\x52\x95\x18\x60\x67\xb2\xc5\x8c\x66\x10\x40" //%._+.R..`g...f.@ |
// /* 2180 */ "\x42\x12\x84\x23\x49\x28\x42\x10\x21\x09\x84\x11\xb7\xa9\x5a\x96" //B..#I(B.!.....Z. |
// /* 2190 */ "\xb4\x0b\x86\x14\x5e\xe1\x60\xfd\x97\xd4\x82\xc6\x3e\x75\x6e\x05" //....^.`.....>un. |
// /* 21a0 */ "\x4e\x06\x11\x4b\xed\xad\x01\x7e\x66\x8e\x2e\x2f\x33\x5c\x3f\xcc" //N..K...~f../3.?. |
// /* 21b0 */ "\xc0\x1e\x0f\x07\x9a\x74\x9b\xc7\x22\x18\x4a\x6b\x5c\x18\xc7\x0f" //.....t..".Jk.... |
// /* 21c0 */ "\x63\xc0\x07\x77\x17\xd4\x80\xc6\x32\x18\x82\x6b\x5b\x29\x4e\x1e" //c..w....2..k[)N. |
// /* 21d0 */ "\x5f\x82\x0f\xa8\x8d\x55\xa8\xfc\x0c\x15\xfb\xff\xd4\x8c\x3c\xfd" //_....U........<. |
// /* 21e0 */ "\x04\x00\x18\x5e\x99\x01\x03\x21\x83\xf9\x2e\x02\x3c\x96\x28\xf1" //...^...!....<.(. |
// /* 21f0 */ "\xbc\xa0\x00\x80\xc5\x43\x18\xd4\x61\x9f\x53\x0f\x60\x47\x07\xe8" //.....C..a.S.`G.. |
// /* 2200 */ "\xfa\x90\x49\xf5\x18\x33\xea\x2d\x6b\x42\x18\x56\x7a\x43\x18\xf1" //..I..3.-kB.VzC.. |
// /* 2210 */ "\x36\x22\x07\xc0\x22\xb8\xbc\xb9\xec\x4e\x11\x20\xda\x00\x01\xe3" //6".."....N. .... |
// /* 2220 */ "\x78\xb5\xad\x57\xef\xed\xbe\x37\x91\x49\x2f\x8d\xe3\x05\xed\xbe" //x..W...7.I/..... |
// /* 2230 */ "\xa2\x7d\x77\x6d\x6b\x45\x29\xe7\xc9\x08\x21\x1f\x52\x39\x66\xc4" //.}wmkE)...!.R9f. |
// /* 2240 */ "\xfa\xef\xee\xb8\x00\xfe\xc0\xc6\x3c\x0f\xba\xc2\xc8\x0e\x01\x28" //........<......( |
// /* 2250 */ "\x00\x07\x8e\xe2\xce\x72\x29\xe4\xfb\xf1\xdc\x5a\xd6\xbe\xa6\x1c" //.....r)....Z.... |
// /* 2260 */ "\x18\xeb\xea\x53\x8c\x33\x96\x74\x7d\x48\xf3\x9c\x28\xc3\x12\x52" //...S.3.t}H..(..R |
// /* 2270 */ "\xb0\xf5\xca\x5a\x48\x43\xef\xe7\x00\xac\x7e\xc3\xcc\xe8\x5f\x99" //...ZHC....~..._. |
// /* 2280 */ "\xf1\x84\xf9\x9d\x19\xc0\x11\x25\xa9\x73\x9d\x5e\x7b\x02\xc1\xc3" //.......%.s.^{... |
// /* 2290 */ "\xa8\x52\x95\xbd\xee\x2d\x35\xa1\x8c\x2e\xdb\xff\xfc\x2a\xcb\x41" //.R...-5......*.A |
// /* 22a0 */ "\x08\x63\xd7\x7e\x56\xb0\x29\x25\x45\x29\xaf\xdf\xe8\xe7\x33\x2c" //.c.~V.)%E)....3, |
// /* 22b0 */ "\x42\x96\x83\x18\x90\xc1\x99\x4a\x6e\x78\x80\xd1\x67\x39\x9f\x5d" //B......Jnx..g9.] |
// /* 22c0 */ "\xf0\xb0\x6b\xa0\x11\xc6\x19\x47\x39\xc2\x8f\x4c\x22\x36\x89\x45" //..k....G9..L"6.E |
// /* 22d0 */ "\x24\xdd\x2a\x92\xe7\x38\x10\x85\x4e\x38\xca\x53\xd2\x8a\x24\x42" //$.*..8..N8.S..$B |
// /* 22e0 */ "\x17\x38\xe7\x2b\x58\xe6\x1f\x02\x92\x52\x42\x1e\x0b\xa8\x40\x21" //.8.+X....RB...@! |
// /* 22f0 */ "\x09\xa4\x96\xc7\xcc\xa4\x18\xc6\x48\x42\x29\x85\xa8\x5d\x00\xb0" //........HB)..].. |
// /* 2300 */ "\x5d\xf2\xde\x2b\x7e\x5b\x44\x8d\x31\x32\xc0\xa1\x09\xf6\xdf\x63" //]..+~[D.12.....c |
// /* 2310 */ "\x18\x75\x96\xf2\x80\x83\xcb\x3c\x88\x43\x14\xfb\x9e\xe5\x4a\x57" //.u.....<.C....JW |
// /* 2320 */ "\xba\xf3\x08\x45\xde\x7b\x0e\xf1\x38\x3b\xc8\x08\x97\x20\xb8\x4d" //...E.{..8;... .M |
// /* 2330 */ "\xe3\x44\x91\x08\x74\x01\x88\x37\x34\xb3\x08\x45\x3d\xee\x14\xa5" //.D..t..74..E=... |
// /* 2340 */ "\x26\x31\x89\x45\x10\x42\x2d\xef\x69\x29\x4c\x21\x01\xdc\x1b\xb2" //&1.E.B-.i)L!.... |
// /* 2350 */ "\xf1\x6d\x62\xe9\x67\x8b\x6e\x51\xf4\x82\x7d\x7e\xdd\x92\xc2\x70" //.mb.g.nQ..}~...p |
// /* 2360 */ "\x1e\xba\xf1\x22\x10\x1f\xc5\x1f\x8c\x24\xac\xeb\xe3\x3d\x91\xb2" //...".....$...=.. |
// /* 2370 */ "\xfa\xd1\x6f\xfa\xce\xc8\xbc\x5f\x6a\x25\xdd\x88\x5d\x91\x39\xbe" //..o...._j%..].9. |
// /* 2380 */ "\xf4\x7f\x88\x87\x7e\x27\x84\xa3\xb2\x1e\xdf\x9a\x3b\x23\x00\xfc" //....~'......;#.. |
// /* 2390 */ "\x0e\xec\x79\x1c\x25\x1f\x55\xd9\x41\x80\x2d\x63\xb1\xfb\x83\xc8" //..y.%.U.A.-c.... |
// /* 23a0 */ "\x43\xca\x51\x01\xc1\x3b\x4e\xe0\x32"                             //C.Q..;N.2 |
// Sent dumped on RDP Client (5) 9129 bytes |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[1](30) used=52 free=16482 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 400, 32, 32) (576, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[2](2064) used=55 free=16479 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[3](30) used=101 free=16433 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 400, 32, 32) (608, 256, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[4](2064) used=104 free=16430 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[5](30) used=156 free=16378 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,400,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((192, 432, 32, 32) (0, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[6](2064) used=159 free=16375 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[7](30) used=207 free=16327 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 432, 32, 32) (32, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[8](2064) used=213 free=16321 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[9](30) used=260 free=16274 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 432, 32, 32) (64, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[10](2064) used=263 free=16271 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[11](30) used=317 free=16217 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 432, 32, 32) (96, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[12](2064) used=320 free=16214 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[13](30) used=515 free=16019 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 432, 32, 32) (128, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[14](2064) used=518 free=16016 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[15](30) used=582 free=15952 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 432, 32, 32) (160, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[16](2064) used=585 free=15949 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[17](30) used=669 free=15865 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 432, 32, 32) (192, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[18](2064) used=672 free=15862 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[19](30) used=755 free=15779 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 432, 32, 32) (224, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[20](2064) used=758 free=15776 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[21](30) used=841 free=15693 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 432, 32, 32) (256, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[22](2064) used=844 free=15690 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[23](30) used=927 free=15607 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 432, 32, 32) (288, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[24](2064) used=930 free=15604 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[25](30) used=1065 free=15469 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 432, 32, 32) (320, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[26](2064) used=1068 free=15466 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[27](30) used=1203 free=15331 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 432, 32, 32) (352, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[28](2064) used=1206 free=15328 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[29](30) used=1288 free=15246 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 432, 32, 32) (384, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[30](2064) used=1291 free=15243 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[31](30) used=1373 free=15161 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 432, 32, 32) (416, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[32](2064) used=1376 free=15158 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[33](30) used=1460 free=15074 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 432, 32, 32) (448, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[34](2064) used=1463 free=15071 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[35](30) used=1546 free=14988 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 432, 32, 32) (480, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[36](2064) used=1549 free=14985 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[37](30) used=1605 free=14929 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 432, 32, 32) (512, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[38](2064) used=1608 free=14926 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[39](30) used=1795 free=14739 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 432, 32, 32) (544, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[40](2064) used=1798 free=14736 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[41](30) used=1852 free=14682 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 432, 32, 32) (576, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[42](2064) used=1855 free=14679 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[43](30) used=1903 free=14631 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 432, 32, 32) (608, 288, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[44](2064) used=1906 free=14628 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[45](30) used=1954 free=14580 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,432,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((192, 464, 32, 32) (0, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[46](2064) used=1957 free=14577 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[47](30) used=2019 free=14515 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 464, 32, 32) (32, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[48](2064) used=2025 free=14509 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[49](30) used=2076 free=14458 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 464, 32, 32) (64, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[50](2064) used=2079 free=14455 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[51](30) used=2139 free=14395 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 464, 32, 32) (96, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[52](2064) used=2142 free=14392 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[53](30) used=2325 free=14209 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 464, 32, 32) (128, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[54](2064) used=2328 free=14206 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[55](30) used=2474 free=14060 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 464, 32, 32) (160, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[56](2064) used=2477 free=14057 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[57](30) used=2579 free=13955 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 464, 32, 32) (192, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[58](2064) used=2582 free=13952 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[59](30) used=2672 free=13862 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 464, 32, 32) (224, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[60](2064) used=2675 free=13859 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[61](30) used=2771 free=13763 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 464, 32, 32) (256, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[62](2064) used=2774 free=13760 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[63](30) used=2865 free=13669 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 464, 32, 32) (288, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[64](2064) used=2868 free=13666 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[65](30) used=2958 free=13576 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 464, 32, 32) (320, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[66](2064) used=2961 free=13573 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[67](30) used=3052 free=13482 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 464, 32, 32) (352, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[68](2064) used=3055 free=13479 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[69](30) used=3143 free=13391 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 464, 32, 32) (384, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[70](2064) used=3146 free=13388 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[71](30) used=3247 free=13287 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 464, 32, 32) (416, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[72](2064) used=3250 free=13284 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[73](30) used=3340 free=13194 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 464, 32, 32) (448, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[74](2064) used=3343 free=13191 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[75](30) used=3444 free=13090 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 464, 32, 32) (480, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[76](2064) used=3447 free=13087 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[77](30) used=3577 free=12957 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 464, 32, 32) (512, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[78](2064) used=3580 free=12954 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[79](30) used=3740 free=12794 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 464, 32, 32) (544, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[80](2064) used=3743 free=12791 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[81](30) used=3803 free=12731 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 464, 32, 32) (576, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[82](2064) used=3806 free=12728 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[83](30) used=3858 free=12676 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 464, 32, 32) (608, 320, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[84](2064) used=3861 free=12673 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[85](30) used=3926 free=12608 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,464,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((192, 496, 32, 32) (0, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[86](2064) used=3929 free=12605 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[87](30) used=3992 free=12542 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 496, 32, 32) (32, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[88](2064) used=3998 free=12536 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[89](30) used=4050 free=12484 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 496, 32, 32) (64, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[90](2064) used=4053 free=12481 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[91](30) used=4111 free=12423 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 496, 32, 32) (96, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[92](2064) used=4114 free=12420 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[93](30) used=4200 free=12334 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 496, 32, 32) (128, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[94](2064) used=4203 free=12331 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[95](30) used=4450 free=12084 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 496, 32, 32) (160, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[96](2064) used=4453 free=12081 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[97](30) used=4496 free=12038 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 496, 32, 32) (192, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[98](2064) used=4499 free=12035 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[99](30) used=4538 free=11996 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 496, 32, 32) (224, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[100](2064) used=4541 free=11993 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[101](30) used=4602 free=11932 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 496, 32, 32) (256, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[102](2064) used=4605 free=11929 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[103](30) used=4634 free=11900 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 496, 32, 32) (288, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[104](30) used=4637 free=11897 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 496, 32, 32) (320, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[105](2064) used=4640 free=11894 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[106](30) used=4669 free=11865 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 496, 32, 32) (352, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[107](30) used=4672 free=11862 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 496, 32, 32) (384, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[108](2064) used=4675 free=11859 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[109](30) used=4718 free=11816 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 496, 32, 32) (416, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[110](2064) used=4721 free=11813 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[111](30) used=4757 free=11777 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 496, 32, 32) (448, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[112](2064) used=4760 free=11774 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[113](30) used=4793 free=11741 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 496, 32, 32) (480, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[114](2064) used=4796 free=11738 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[115](30) used=5016 free=11518 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 496, 32, 32) (512, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[116](2064) used=5019 free=11515 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[117](30) used=5104 free=11430 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 496, 32, 32) (544, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[118](2064) used=5107 free=11427 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[119](30) used=5165 free=11369 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 496, 32, 32) (576, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[120](2064) used=5168 free=11366 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[121](30) used=5221 free=11313 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 496, 32, 32) (608, 352, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[122](2064) used=5224 free=11310 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[123](30) used=5287 free=11247 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,496,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((192, 528, 32, 32) (0, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[124](2064) used=5290 free=11244 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[125](30) used=5342 free=11192 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 528, 32, 32) (32, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[126](2064) used=5348 free=11186 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[127](30) used=5396 free=11138 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 528, 32, 32) (64, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[128](2064) used=5399 free=11135 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[129](30) used=5427 free=11107 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 528, 32, 32) (96, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[130](2064) used=5430 free=11104 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[131](30) used=5485 free=11049 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 528, 32, 32) (128, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[132](2064) used=5488 free=11046 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[133](30) used=5539 free=10995 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 528, 32, 32) (160, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[134](2064) used=5542 free=10992 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[135](30) used=5788 free=10746 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 528, 32, 32) (192, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[136](2064) used=5791 free=10743 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[137](30) used=5867 free=10667 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 528, 32, 32) (224, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[138](2064) used=5870 free=10664 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[139](30) used=5949 free=10585 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 528, 32, 32) (256, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[140](2064) used=5952 free=10582 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[141](30) used=5992 free=10542 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 528, 32, 32) (288, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[142](2064) used=5995 free=10539 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[143](30) used=6038 free=10496 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 528, 32, 32) (320, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[144](2064) used=6041 free=10493 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[145](30) used=6084 free=10450 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 528, 32, 32) (352, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[146](2064) used=6087 free=10447 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[147](30) used=6118 free=10416 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 528, 32, 32) (384, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[148](30) used=6121 free=10413 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 528, 32, 32) (416, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[149](2064) used=6124 free=10410 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[150](30) used=6191 free=10343 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 528, 32, 32) (448, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[151](2064) used=6194 free=10340 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[152](30) used=6422 free=10112 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 528, 32, 32) (480, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[153](2064) used=6425 free=10109 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[154](30) used=6472 free=10062 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 528, 32, 32) (512, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[155](2064) used=6475 free=10059 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[156](30) used=6530 free=10004 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 528, 32, 32) (544, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[157](2064) used=6533 free=10001 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[158](30) used=6561 free=9973 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 528, 32, 32) (576, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[159](2064) used=6564 free=9970 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[160](30) used=6613 free=9921 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 528, 32, 32) (608, 384, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[161](2064) used=6616 free=9918 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[162](30) used=6671 free=9863 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,528,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((192, 560, 32, 32) (0, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[163](2064) used=6674 free=9860 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[164](30) used=6733 free=9801 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 560, 32, 32) (32, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[165](2064) used=6739 free=9795 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[166](30) used=6807 free=9727 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 560, 32, 32) (64, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[167](2064) used=6810 free=9724 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[168](30) used=6879 free=9655 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 560, 32, 32) (96, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[169](2064) used=6882 free=9652 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[170](30) used=6964 free=9570 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 560, 32, 32) (128, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[171](2064) used=6967 free=9567 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[172](30) used=7026 free=9508 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 560, 32, 32) (160, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[173](2064) used=7029 free=9505 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[174](30) used=7067 free=9467 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 560, 32, 32) (192, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[175](2064) used=7070 free=9464 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[176](30) used=7316 free=9218 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 560, 32, 32) (224, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[177](2064) used=7319 free=9215 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[178](30) used=7497 free=9037 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 560, 32, 32) (256, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[179](2064) used=7500 free=9034 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[180](30) used=7531 free=9003 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 560, 32, 32) (288, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[181](2064) used=7534 free=9000 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[182](30) used=7577 free=8957 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 560, 32, 32) (320, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[183](2064) used=7580 free=8954 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[184](30) used=7608 free=8926 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 560, 32, 32) (352, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[185](2064) used=7611 free=8923 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[186](30) used=7644 free=8890 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 560, 32, 32) (384, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[187](2064) used=7647 free=8887 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[188](30) used=7834 free=8700 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 560, 32, 32) (416, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[189](2064) used=7837 free=8697 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[190](30) used=8077 free=8457 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 560, 32, 32) (448, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[191](2064) used=8080 free=8454 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[192](30) used=8118 free=8416 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 560, 32, 32) (480, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[193](2064) used=8121 free=8413 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[194](30) used=8175 free=8359 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 560, 32, 32) (512, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[195](2064) used=8178 free=8356 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[196](30) used=8257 free=8277 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 560, 32, 32) (544, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[197](2064) used=8260 free=8274 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[198](30) used=8332 free=8202 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 560, 32, 32) (576, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[199](2064) used=8335 free=8199 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[200](30) used=8408 free=8126 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 560, 32, 32) (608, 416, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[201](2064) used=8411 free=8123 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[202](30) used=8471 free=8063 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,560,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((192, 592, 32, 32) (0, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[203](2064) used=8474 free=8060 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[204](30) used=8532 free=8002 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(192,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((224, 592, 32, 32) (32, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[205](2064) used=8538 free=7996 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[206](30) used=8588 free=7946 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(224,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((256, 592, 32, 32) (64, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[207](2064) used=8591 free=7943 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[208](30) used=8649 free=7885 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(256,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((288, 592, 32, 32) (96, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[209](2064) used=8652 free=7882 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[210](30) used=8712 free=7822 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(288,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((320, 592, 32, 32) (128, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[211](2064) used=8715 free=7819 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[212](30) used=8750 free=7784 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(320,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((352, 592, 32, 32) (160, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[213](2064) used=8753 free=7781 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[214](30) used=8789 free=7745 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(352,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((384, 592, 32, 32) (192, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[215](2064) used=8792 free=7742 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[216](30) used=8851 free=7683 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(384,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((416, 592, 32, 32) (224, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[217](2064) used=8854 free=7680 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[218](30) used=8919 free=7615 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(416,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((448, 592, 32, 32) (256, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[219](2064) used=8922 free=7612 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[220](30) used=9062 free=7472 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(448,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((480, 592, 32, 32) (288, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[221](2064) used=9065 free=7469 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[222](30) used=9218 free=7316 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(480,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((512, 592, 32, 32) (320, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[223](2064) used=9221 free=7313 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[224](30) used=9369 free=7165 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(512,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((544, 592, 32, 32) (352, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[225](2064) used=9372 free=7162 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[226](30) used=9520 free=7014 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(544,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((576, 592, 32, 32) (384, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[227](2064) used=9523 free=7011 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[228](30) used=9590 free=6944 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(576,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((608, 592, 32, 32) (416, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[229](2064) used=9593 free=6941 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[230](30) used=9657 free=6877 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(608,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((640, 592, 32, 32) (448, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[231](2064) used=9660 free=6874 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[232](30) used=9696 free=6838 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(640,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((672, 592, 32, 32) (480, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[233](2064) used=9699 free=6835 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[234](30) used=9736 free=6798 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(672,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((704, 592, 32, 32) (512, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[235](2064) used=9739 free=6795 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[236](30) used=9798 free=6736 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(704,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((736, 592, 32, 32) (544, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[237](2064) used=9801 free=6733 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[238](30) used=9860 free=6674 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(736,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((768, 592, 32, 32) (576, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[239](2064) used=9863 free=6671 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[240](30) used=9916 free=6618 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(768,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((800, 592, 32, 32) (608, 448, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[241](2064) used=9919 free=6615 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[242](30) used=9979 free=6555 |
// order(13 clip(0,0,1,1)):memblt(cache_id=1 rect(800,592,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[243](32) used=9982 free=6552 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[244](32) used=10010 free=6524 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[245](32) used=10016 free=6518 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[246](32) used=10024 free=6510 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[247](32) used=10034 free=6500 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[248](32) used=10049 free=6485 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[249](32) used=10055 free=6479 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[250](32) used=10063 free=6471 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[251](82) used=10073 free=6461 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[252](66) used=10139 free=6395 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[253](66) used=10189 free=6345 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[254](66) used=10239 free=6295 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[255](66) used=10289 free=6245 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[256](20) used=10339 free=6195 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[257](297) used=10359 free=6175 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[258](66) used=10409 free=6125 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[259](66) used=10459 free=6075 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[260](66) used=10509 free=6025 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[261](297) used=10559 free=5975 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[262](82) used=10591 free=5943 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[263](66) used=10657 free=5877 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[264](66) used=10707 free=5827 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[265](297) used=10757 free=5777 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[266](82) used=10789 free=5745 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[267](66) used=10855 free=5679 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[268](66) used=10905 free=5629 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[269](297) used=10955 free=5579 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[270](66) used=10987 free=5547 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[271](66) used=11037 free=5497 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[272](66) used=11087 free=5447 |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[273](297) used=11137 free=5397 |
// Widget_load: image file [./tests/fixtures/xrdp24b-redemption.png] is PNG file |
// front::draw:draw_tile((738, 613, 32, 32) (0, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[274](2064) used=11172 free=5362 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[275](30) used=11224 free=5310 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(738,613,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((770, 613, 32, 32) (32, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[276](2064) used=11229 free=5305 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[277](30) used=11300 free=5234 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(770,613,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((802, 613, 32, 32) (64, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[278](2064) used=11303 free=5231 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[279](30) used=11428 free=5106 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(802,613,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((834, 613, 32, 32) (96, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[280](2064) used=11431 free=5103 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[281](30) used=11490 free=5044 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(834,613,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((866, 613, 32, 32) (128, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[282](2064) used=11493 free=5041 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[283](30) used=11660 free=4874 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(866,613,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((898, 613, 32, 32) (160, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[284](2064) used=11663 free=4871 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[285](30) used=11843 free=4691 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(898,613,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((930, 613, 32, 32) (192, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[286](2064) used=11846 free=4688 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[287](30) used=11970 free=4564 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(930,613,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((962, 613, 32, 32) (224, 0, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[288](2064) used=11973 free=4561 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[289](30) used=12168 free=4366 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(962,613,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((738, 645, 32, 32) (0, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[290](2064) used=12171 free=4363 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[291](30) used=12329 free=4205 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(738,645,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((770, 645, 32, 32) (32, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[292](2064) used=12335 free=4199 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[293](30) used=12575 free=3959 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(770,645,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((802, 645, 32, 32) (64, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[294](2064) used=12578 free=3956 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[295](30) used=12925 free=3609 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(802,645,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((834, 645, 32, 32) (96, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[296](2064) used=12928 free=3606 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[297](30) used=13127 free=3407 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(834,645,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((866, 645, 32, 32) (128, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[298](2064) used=13130 free=3404 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[299](30) used=13452 free=3082 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(866,645,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((898, 645, 32, 32) (160, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[300](2064) used=13455 free=3079 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[301](30) used=13601 free=2933 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(898,645,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((930, 645, 32, 32) (192, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[302](2064) used=13604 free=2930 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[303](30) used=13799 free=2735 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(930,645,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((962, 645, 32, 32) (224, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[304](2064) used=13802 free=2732 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[305](30) used=13983 free=2551 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(962,645,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((738, 677, 32, 32) (0, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[306](2064) used=13986 free=2548 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[307](30) used=14223 free=2311 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(738,677,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((770, 677, 32, 32) (32, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[308](2064) used=14229 free=2305 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[309](30) used=14480 free=2054 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(770,677,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((802, 677, 32, 32) (64, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[310](2064) used=14483 free=2051 |
// GraphicsUpdatePDU::flush_orders: order_count=310 offset=0 |
// GraphicsUpdatePDU::flush_orders: fast-path |
// Sending on RDP Client (5) 8230 bytes |
// /* 0000 */ "\x00\xa0\x26\x80\x21\x1f\x20\x36\x01\xc1\x09\x1b\xf7\x2a\x77\x30" //..&.!. 6.....*w0 |
// /* 0010 */ "\x77\x36\x8c\x00\x33\x6c\x1d\xcd\xe7\x03\x17\x56\x09\xfe\x2c\xd3" //w6..3l.....V..,. |
// /* 0020 */ "\xb0\x23\xc4\xf5\xb5\x7f\x5b\xef\x3b\x18\x00\x7f\x66\xee\xf3\xb5" //.#....[.;...f... |
// /* 0030 */ "\x81\x8a\x63\x03\x17\xc7\x8a\x27\x07\xc6\x6f\x80\x00\x58\x00\x1d" //..c....'..o..X.. |
// /* 0040 */ "\x82\xca\xe3\xc4\x0c\x00\x0a\xc0\x01\x6c\x15\xf1\x1d\xff\x55\x0c" //.........l....U. |
// /* 0050 */ "\x3e\xb1\x97\xf7\x8f\x29\xbf\xbd\xf8\xc9\x81\x8a\xf5\x00\x03\xfe" //>....).......... |
// /* 0060 */ "\x51\x80\x6d\x58\x6a\xa9\x81\x97\x23\x05\xb9\x00\x10\x30\x01\xdc" //Q.mXj...#....0.. |
// /* 0070 */ "\x4c\x00\x70\x36\x36\xfe\x55\x8f\x2e\x30\x01\xdd\x80\x01\xe0\x03" //L.p66.U..0...... |
// /* 0080 */ "\x6b\xc3\x62\x16\x00\x9d\xe2\x88\xc1\x79\x5b\x60\x03\xce\x11\x80" //k.b......y[`.... |
// /* 0090 */ "\x3c\x00\x71\x5c\x8c\xc1\x08\x58\x3d\x26\x7f\xc9\x03\xfc\xc6\x08" //<.q....X=&...... |
// /* 00a0 */ "\x23\x7f\x9b\x9a\x7f\x98\xc2\x28\x6c\x03\x34\x09\x15\xc2\x7a\x0c" //#......(l.4...z. |
// /* 00b0 */ "\x28\xa6\xc3\xa2\x8e\xf2\xe1\x5e\x0a\x81\xec\x3a\xda\xe7\xc8\x14" //(......^...:.... |
// /* 00c0 */ "\xe0\xd7\x38\x00\x26\x0b\x18\xfb\xdd\x84\xff\x60\x7b\xf5\xb0\x01" //..8.&......`{... |
// /* 00d0 */ "\xcc\x72\x6a\x61\xe3\xc8\xfa\x3a\x37\x61\x7e\xb8\xc3\xdd\xef\xa3" //.rja...:7a~..... |
// /* 00e0 */ "\x65\x8c\x78\xa7\x81\x81\x21\x20\x7f\x84\xc3\x00\x4e\x47\x49\x90" //e.x...! ....NGI. |
// /* 00f0 */ "\x7f\x88\xdc\x2f\x2b\xf1\x46\x17\x9f\x7b\x96\x0a\x10\xb0\x7d\x34" //.../+.F..{....}4 |
// /* 0100 */ "\x84\x60\xd1\xd5\x7e\xfe\x82\x10\x23\x83\x6a\x18\x54\x2d\x86\x1d" //.`..~...#.j.T-.. |
// /* 0110 */ "\x98\x36\xe1\xf7\x5c\x3a\x2b\xf8\x43\x6b\x5b\x9f\x18\x0a\xc0\x63" //.6...:+.Ck[....c |
// /* 0120 */ "\xaf\x43\x5e\x10\xfd\x00\x60\x04\x7f\x3f\x00\xf3\x56\x17\x97\x78" //.C^...`..?..V..x |
// /* 0130 */ "\x0e\xf1\x36\xdb\x9b\x91\x57\x00\x07\x7c\x07\x85\xe9\x1b\xc2\xf8" //..6...W..|...... |
// /* 0140 */ "\xe2\x28\x37\x7c\x9f\x83\xf0\xb7\x27\x2b\x27\x77\xbf\x7f\xc2\xdc" //.(7|....'+'w.... |
// /* 0150 */ "\x9d\xcb\xbe\x0b\xc2\xd4\xf5\xe1\x6c\x5e\x3c\x2c\xc4\xc9\x8e\xdb" //........l^<,.... |
// /* 0160 */ "\x9c\xd5\x00\x22\xc6\x30\x5c\x58\x6a\x59\xa7\x75\x3c\x43\x53\x5a" //...".0.XjY.u<CSZ |
// /* 0170 */ "\xfe\x54\x06\x61\xbe\x32\x6b\x5c\x6a\xac\x71\x46\xb5\xad\x10\x00" //.T.a.2k.j.qF.... |
// /* 0180 */ "\x36\xb5\xe0\xef\x85\x1a\xe1\x6e\x02\xe6\x7c\x62\xb1\xe0\x6b\xa6" //6......n..|b..k. |
// /* 0190 */ "\x22\xf8\x43\x70\x0a\xcc\x58\xc6\x18\x42\x30\x0a\x0c\x7c\xac\x9a" //".Cp..X..B0..|.. |
// /* 01a0 */ "\x52\xaf\x75\xe0\xec\x2d\xa4\xe2\x62\xc2\x1b\x15\x20\xf2\x57\x14" //R.u..-..b... .W. |
// /* 01b0 */ "\x03\xa7\x51\x04\x22\x1c\xe7\xe1\x58\xa5\xad\x30\xfc\xa7\x0a\xb8" //..Q."...X..0.... |
// /* 01c0 */ "\x95\x29\x30\x84\x4b\x9c\xef\x7d\xc5\x12\xa5\x2c\x34\x33\x1f\x2a" //.)0.K..}...,43.* |
// /* 01d0 */ "\x3b\x8a\x47\x71\xee\x5b\x00\x14\x36\xb7\x5e\xa3\x9c\xf8\xeb\x4f" //;.Gq.[..6.^....O |
// /* 01e0 */ "\x95\x91\x0e\x00\x16\x7c\xa8\xb1\x8c\x07\x61\x27\xa0\x42\x17\x93" //.....|....a'.B.. |
// /* 01f0 */ "\x1e\x52\x00\x4b\xbc\x95\x1c\x48\xde\x4a\xbc\x52\x7d\x1d\xc9\xbe" //.R.K...H.J.R}... |
// /* 0200 */ "\x07\x81\x4d\x78\x57\x6e\xda\xbf\x03\xe1\x5a\xe3\xaf\xc1\xfa\xe3" //..MxWn....Z..... |
// /* 0210 */ "\xb4\x4f\xbb\xe1\x7a\xe2\x57\xb7\x63\x9f\x82\xf0\xb5\x61\x72\x6d" //.O..z.W.c....arm |
// /* 0220 */ "\xba\xfa\x78\xff\xee\xb0\x29\xfb\xfd\xdc\xb3\xfd\x82\x79\xc4\x39" //..x...)......y.9 |
// /* 0230 */ "\xe6\x40\x4e\x1c\x55\x5b\xde\xf1\x7f\xec\x73\x67\x8e\xb0\x2f\x94" //.@N.U[....sg../. |
// /* 0240 */ "\x6d\xeb\x62\xe7\x8d\x5f\x3a\x3a\xd2\xc6\x06\xcb\x40\x98\x7f\x45" //m.b.._::....@..E |
// /* 0250 */ "\x86\x7d\x0e\x3c\x16\xd0\xfa\x18\xff\x08\xe7\x85\x5f\xfa\x3b\x0b" //.}.<........_.;. |
// /* 0260 */ "\xc7\x86\xa3\x9e\x0d\x7d\x28\xec\x1f\x1c\xe8\xe0\x21\x0b\x94\x40" //.....}(.....!..@ |
// /* 0270 */ "\x47\xc7\xec\x47\x8c\xfa\xe9\x94\x53\x9e\x28\x4b\xed\x39\xe1\x84" //G..G....S.(K.9.. |
// /* 0280 */ "\xb9\xe0\x5f\xef\x86\xd6\xbc\x1a\x57\x3e\xaf\x79\xfa\xfa\x5f\xc7" //.._.....W>.y.._. |
// /* 0290 */ "\x4b\x27\x4a\x37\x4b\xe8\xeb\x89\x60\x00\x7d\x70\x2b\xa0\x43\x8d" //K'J7K...`.}p+.C. |
// /* 02a0 */ "\x73\xa5\xe3\xef\x80\x16\x3f\xcc\x75\x60\xdb\xae\x46\xc1\xdd\x72" //s.....?.u`..F..r |
// /* 02b0 */ "\x18\x85\xef\x5e\x9e\x55\xfc\xf6\xeb\x16\xc0\xe5\xf8\xf5\x93\x60" //...^.U.........` |
// /* 02c0 */ "\x70\xd7\x59\x07\x59\x66\x21\x7a\xb0\x05\x0f\xe7\x1d\x15\x5f\xce" //p.Y.Yf!z......_. |
// /* 02d0 */ "\xf0\xb4\x94\x18\x7d\x87\xf7\x61\x66\xd6\x13\x84\x61\x67\x96\x16" //....}..af...ag.. |
// /* 02e0 */ "\x87\x8c\x38\x42\xc3\x85\x33\x87\x0c\x58\x5a\x65\x86\xfe\x6b\xf3" //..8B..3..XZe..k. |
// /* 02f0 */ "\x21\x4a\x40\x14\x0e\xba\x91\x50\xc3\x7c\xfd\x1d\x75\x2a\x01\x8b" //!J@....P.|..u*.. |
// /* 0300 */ "\x40\x58\x72\xa1\xd7\x65\xd7\x61\x85\xc4\xa7\xfc\xb1\x67\x84\xf8" //@Xr..e.a.....g.. |
// /* 0310 */ "\xc1\xfd\xaf\x0b\xc1\x47\x5d\x8e\x18\xc0\x16\x17\x0a\x60\xb4\x26" //.....G]......`.& |
// /* 0320 */ "\x07\x57\x81\x8c\x0f\x7c\xa3\xb2\x94\xf5\xde\x9f\xfe\xa0\x7f\xf8" //.W...|.......... |
// /* 0330 */ "\xef\xb1\xb6\x1d\xaa\x8f\xbc\xff\xf1\x12\xc4\x2e\xd2\xbf\x89\x19" //................ |
// /* 0340 */ "\x23\x83\x84\x0f\x61\xb0\xc7\x5f\x07\x32\xc6\x0c\xc5\x7c\xca\xe1" //#...a.._.2...|.. |
// /* 0350 */ "\xc8\x62\xc0\x90\xcc\x37\x69\x58\x23\x75\x80\x52\x83\x78\x6f\x36" //.b...7iX#u.R.xo6 |
// /* 0360 */ "\xba\xc6\x2a\x00\x6c\x21\x18\x70\x20\x9b\x82\x98\x98\x2b\x75\x50" //..*.l!.p ....+uP |
// /* 0370 */ "\x04\x8b\x4d\x70\xe0\x6c\xc5\xa6\xb5\x00\x4f\x5d\x28\x01\x2c\xa5" //..Mp.l....O](.,. |
// /* 0380 */ "\x3e\x4d\x54\x01\x71\x98\x64\x61\x8a\x46\x29\x5b\x33\x60\x42\x97" //>MT.q.da.F)[3`B. |
// /* 0390 */ "\x11\x80\xf7\x7f\x76\x04\x40\x0c\x7c\xb8\xec\x2c\xf1\x3c\x42\x94" //....v.@.|..,.<B. |
// /* 03a0 */ "\x9a\x00\x31\x03\x8b\xb3\x1d\x54\x30\x37\x68\x07\xae\x94\xc1\x7c" //..1....T07h....| |
// /* 03b0 */ "\xf5\x8e\x1e\x88\x00\x63\x2a\x57\xef\x61\xba\x20\x2f\xf0\x73\xc7" //.....c*W.a. /.s. |
// /* 03c0 */ "\x8f\x32\x3c\x95\x16\x73\x9c\x35\x24\x03\xc3\x7a\xb5\xc4\x59\x80" //.2<..s.5$..z..Y. |
// /* 03d0 */ "\x78\x40\x58\x01\x55\x80\x35\xd0\x31\x0c\x4d\x62\xc1\x08\x50\xd2" //x@X.U.5.1.Mb..P. |
// /* 03e0 */ "\x94\xfd\xce\x35\xda\x06\x8a\x10\xb1\x89\x53\x9c\x22\xe6\xd3\x82" //...5......S."... |
// /* 03f0 */ "\xa7\x91\x66\x1e\xc4\x9b\xdf\xef\xfd\x89\x3a\xc8\x30\x4b\x54\xac" //..f.......:.0KT. |
// /* 0400 */ "\xf8\xe9\xe5\x5a\x57\xf2\xa8\x43\xc7\xea\x7c\x18\xc7\x17\x83\xc7" //...ZW..C..|..... |
// /* 0410 */ "\x19\x18\xc5\xed\x60\xf8\xe7\x63\x6f\x60\x86\x07\xd8\xda\xfc\x53" //....`..co`.....S |
// /* 0420 */ "\x49\xbe\x29\xc5\x2f\xc5\x2a\x07\x92\x18\x6a\xad\xd8\xf7\x21\x8a" //I.)./.*...j...!. |
// /* 0430 */ "\x37\x63\xdf\x5b\xc3\xb4\xa7\xfb\x9e\x1d\xa5\x1f\x0e\xc0\x44\x18" //7c.[..........D. |
// /* 0440 */ "\xc7\x94\x22\x8e\x58\xc7\xe2\x34\x57\xb2\x57\x45\xfe\xff\xd9\x2b" //..".X..4W.WE...+ |
// /* 0450 */ "\x80\x86\x07\xd9\x2a\xee\xf3\xde\xc9\x5a\x78\x8e\xb0\x6d\xb3\xc4" //....*....Zx..m.. |
// /* 0460 */ "\x76\x99\xe2\x38\x10\xf0\xec\xc0\x00\x3b\x29\x6d\x7f\xc4\xbc\x2e" //v..8.....;)m.... |
// /* 0470 */ "\xca\x5e\x7f\x87\xab\xdb\xdc\x1e\x1e\x98\x47\x64\xf8\x6d\xe1\x92" //.^........Gd.m.. |
// /* 0480 */ "\xec\xb3\x81\x0f\x03\xd7\x65\x98\x2c\x63\xec\xa2\x81\xf6\x4f\x8e" //......e.,c....O. |
// /* 0490 */ "\xa1\x6e\xcb\x3d\x9e\x1d\xaf\x3d\xf0\x54\xe3\x9e\x1d\x76\x2c\xf8" //.n.=...=.T...v,. |
// /* 04a0 */ "\x3f\x0f\x57\x9c\x5e\x1e\xbc\x42\xa2\xf0\xf6\x10\xce\xc0\x1d\x04" //?.W.^..B........ |
// /* 04b0 */ "\x30\x9e\xc0\x18\x75\xec\x8e\xc0\x11\x53\x8e\x7a\x35\xd8\x03\x7b" //0...u....S.z5..{ |
// /* 04c0 */ "\x81\x0f\x86\xe5\x63\xc3\x78\xe0\x78\xe0\x05\x1e\xa7\x1c\x80\x8c" //....c.x.x....... |
// /* 04d0 */ "\x51\xa8\x01\x1f\x7d\x07\x02\xc1\x9a\x0e\xe5\xea\x9c\x70\x06\x3d" //Q...}........p.= |
// /* 04e0 */ "\xd7\xfa\x3f\xd4\x76\x12\xe2\xfa\x91\xd8\x4b\xbf\xd0\x75\xc0\xfb" //..?.v.....K..u.. |
// /* 04f0 */ "\x83\xc4\x49\x00\x17\x15\x07\x60\x48\xec\x31\xe7\x10\xc1\x7b\x0c" //..I....`H.1...{. |
// /* 0500 */ "\x67\xe1\xda\xf4\x2e\xd4\xf7\x9a\x77\x42\x36\x03\x67\x98\x78\x14" //g.......wB6.g.x. |
// /* 0510 */ "\x06\x7d\x77\x10\x80\x63\x4f\xc7\x62\xaf\x42\xff\x7f\xec\x55\xaf" //.}w..cO.b.B...U. |
// /* 0520 */ "\x8a\x2b\x03\xaa\xbc\x51\x78\x09\x08\x00\x73\x80\xec\x58\x8f\x2d" //.+...Qx...s..X.- |
// /* 0530 */ "\xce\x0e\x1d\x65\xa3\xf8\xb8\xd1\x87\x45\xe2\xff\x8a\xda\x23\xe3" //...e.....E....#. |
// /* 0540 */ "\x30\xb9\xa3\xa9\xb3\xe5\x46\x17\x42\x85\xdc\x54\x92\xe2\xf3\x1f" //0.....F.B..T.... |
// /* 0550 */ "\x9a\x9f\xff\x93\x0f\x09\x44\xfa\xef\x8b\xf3\x67\xb3\x97\x8b\x86" //......D....g.... |
// /* 0560 */ "\x21\xa3\xc8\xb5\x82\xa0\x76\xe8\xa2\xe8\xae\xe8\xbb\x18\x42\xb9" //!.....v.......B. |
// /* 0570 */ "\xd1\xa9\x70\x40\x01\x80\x98\x41\x07\xfc\x41\x1d\x0e\x1e\x52\xab" //..p@...A..A...R. |
// /* 0580 */ "\xbf\xcc\x9c\x3c\xbc\xf9\x8f\x2f\x45\x7e\xff\x87\xa0\x9f\x30\xb3" //...<.../E~....0. |
// /* 0590 */ "\x4e\xf2\x24\x0f\xf0\x85\x73\xa1\x61\xe0\x28\x4d\x6b\xa2\xf8\x25" //N.$...s.a.(Mk..% |
// /* 05a0 */ "\x16\x9a\xfa\x4c\x0b\x02\xb3\x9c\xe1\xe2\xc8\x65\x29\xe8\x8a\xe8" //...L.......e)... |
// /* 05b0 */ "\x96\xe8\xa5\x18\x90\x0e\x3b\x1f\xc6\x05\x85\x8f\x95\x1d\xb9\xb0" //......;......... |
// /* 05c0 */ "\xc0\xb5\xf1\xdb\x9b\x01\x6f\x31\xc7\x54\x0d\xba\x99\xbc\xba\xf3" //......o1.T...... |
// /* 05d0 */ "\x1c\x76\x59\xb8\x00\x17\x53\xd7\x8c\x58\x15\x82\x05\xba\x9e\xba" //.vY...S..X...... |
// /* 05e0 */ "\x5b\x06\x3a\x9e\xbf\x28\x02\xb1\xff\xe3\xb4\x73\xff\xde\x05\x36" //[.:..(.....s...6 |
// /* 05f0 */ "\x8e\xcb\xfb\x43\xba\xda\xd5\x03\x07\xc7\xbe\x2b\xad\xb6\x1d\x5d" //...C.......+...] |
// /* 0600 */ "\xe7\x17\x02\xf4\xc0\x00\xbb\x18\x38\x69\xb1\x1a\x7c\x76\x2b\x76" //........8i..|v+v |
// /* 0610 */ "\x33\x9e\xc7\x6e\xc5\x0c\x3c\x2f\x58\x67\x91\x8d\x2c\xde\x04\x9f" //3..n..</Xg..,... |
// /* 0620 */ "\xf4\xf0\x25\xe2\x46\x26\x04\x55\x60\x57\xac\xc0\xbf\x2b\xb3\x1d" //..%.F&.U`W...+.. |
// /* 0630 */ "\x47\xae\xff\xed\xe0\xd8\xf0\xa3\x0f\x25\x04\x3f\x14\x61\x07\x73" //G........%.?.a.s |
// /* 0640 */ "\x06\x2e\x13\x9b\x80\x0f\xa0\x37\xf3\xbc\x79\xf9\xd9\xd6\xc0\x3d" //.......7..y....= |
// /* 0650 */ "\x00\xc3\xd8\x6e\xce\xb5\xe8\x49\xff\x46\xee\x13\x33\xee\xc7\xf7" //...n...I.F..3... |
// /* 0660 */ "\xbc\x01\x24\xec\xed\x50\xfa\x1d\x2a\x83\x23\xc4\x96\x68\x0c\x42" //..$..P..*.#..h.B |
// /* 0670 */ "\x87\x4a\x54\x45\x15\xed\x0c\xa5\x35\x0c\xe2\x39\xbd\xd8\x3f\xec" //.JTE....5..9..?. |
// /* 0680 */ "\xf2\xd9\x7f\xdb\xc0\x2f\x8e\xce\x8a\x0c\x8e\x02\x57\x45\xf1\xbc" //...../......WE.. |
// /* 0690 */ "\x76\x74\xaf\x1b\x82\x55\x11\x6a\x00\x46\x45\x58\x44\x9a\x31\x2a" //vt...U.j.FEXD.1* |
// /* 06a0 */ "\xa3\xb9\xfc\x61\xff\x38\xf6\x6c\x62\x93\xd9\x91\x08\x70\xc0\x8c" //...a.8.lb....p.. |
// /* 06b0 */ "\x14\x97\x18\xa4\xaa\x31\x2e\x52\xe0\x8f\xe1\x99\xb5\x78\x67\x1c" //.....1.R.....xg. |
// /* 06c0 */ "\x58\x82\xc5\xca\x93\x01\xc7\x1d\x4c\x35\xee\xed\x48\x9f\x5d\xc7" //X.......L5..H.]. |
// /* 06d0 */ "\x71\x69\x00\xf2\xcf\xc4\x08\x37\x84\x65\x98\xba\x60\x35\xec\xc3" //qi.....7.e..`5.. |
// /* 06e0 */ "\x2d\xfe\xfe\x33\x81\xcc\x98\xac\x60\x31\x8e\x2c\x03\xb8\x83\xbc" //-..3....`1.,.... |
// /* 06f0 */ "\xd1\xc2\xa1\xfa\x58\xb2\x39\xc0\x44\xd3\x8a\x7b\x04\x3c\x2e\x82" //....X.9.D..{.<.. |
// /* 0700 */ "\x98\xd0\xe9\x5f\xbf\xf8\x06\x1f\x6d\xfc\xda\x82\x7f\xe8\xd9\x7f" //..._....m....... |
// /* 0710 */ "\xbf\x89\x71\x3f\x62\x77\x00\x8e\x3a\x0a\x7d\xdc\x30\xe5\x52\x81" //..q?bw..:.}.0.R. |
// /* 0720 */ "\xc4\x91\xe6\x53\xf7\xa4\x17\x79\xeb\x84\xa9\x4a\x9d\x22\xc6\x37" //...S...y...J.".7 |
// /* 0730 */ "\xf0\x82\x11\x40\xc0\x2c\x4a\x6a\x6b\x5f\x69\x84\x88\x43\x03\x02" //...@.,Jjk_i..C.. |
// /* 0740 */ "\x83\x60\x30\x63\x1c\x0b\xe2\x0c\xed\x5e\xe4\xc2\x0b\xc5\x33\xf5" //.`0c.....^....3. |
// /* 0750 */ "\xd8\x8d\x84\x03\xc1\x5d\x5b\x9c\x9f\xc1\x38\xa7\x16\xb0\x44\x73" //.....][...8...Ds |
// /* 0760 */ "\x07\xf8\x82\x3b\x5b\x30\x7e\xc8\x1f\xff\xb0\x34\xec\x49\x8c\x70" //...;[0~....4.I.p |
// /* 0770 */ "\x1f\x08\x1b\x03\x38\xb0\x54\xcb\xd7\x51\x86\x4d\x9e\xba\xbc\x31" //....8.T..Q.M...1 |
// /* 0780 */ "\xc0\xc5\xb0\xf0\x40\x05\x40\xc0\xbc\x2c\x20\x5a\xa7\xe3\xad\x57" //....@.@.., Z...W |
// /* 0790 */ "\x8e\x9d\xa2\xa3\x82\x0b\x15\xcd\xd6\x18\xfa\x7e\xeb\xec\x30\x00" //...........~..0. |
// /* 07a0 */ "\x32\xe1\xb6\x0d\xb8\x09\x73\x3f\x80\xda\x00\x19\xa4\x97\x1c\xc3" //2.....s?........ |
// /* 07b0 */ "\x2e\x72\x0c\x28\xaa\xfd\x51\xc4\x69\xfa\xaf\x13\x5e\x40\x2f\x05" //.r.(..Q.i...^@/. |
// /* 07c0 */ "\x01\x16\x14\xd6\x0b\xdc\x01\x40\xb9\x84\x28\x5c\x73\xc9\xad\x62" //.......@..(.s..b |
// /* 07d0 */ "\xef\x08\xa5\x3c\x90\x56\xb5\xaa\x32\x6b\x5d\x00\x22\x86\x31\xe7" //...<.V..2k].".1. |
// /* 07e0 */ "\xd3\x3f\x57\x07\x8a\xab\x07\x5d\x3c\x01\x78\x46\xea\x30\x9a\x33" //.?W....]<.xF.0.3 |
// /* 07f0 */ "\xcb\x01\x04\xd6\xbf\x10\x0b\x18\xff\x03\xe1\xbf\x1f\x33\x8c\x33" //.............3.3 |
// /* 0800 */ "\xf1\xb0\x66\x7b\xf1\xd7\xa7\xb6\xfc\x73\x47\x71\x86\x63\x6b\x75" //..f{.....sGq.cku |
// /* 0810 */ "\x00\x21\xd6\x5b\xc4\x7d\xd3\x22\x43\x3c\x69\x86\x1f\x33\x7e\xeb" //.!.[.}."C<i..3~. |
// /* 0820 */ "\xc5\xd2\x70\x1b\xc2\x1c\x65\xfc\x54\xd7\x59\x6e\x27\x2c\x02\x10" //..p...e.T.Yn',.. |
// /* 0830 */ "\xf8\x60\xf3\x80\x4e\x67\x0f\x9d\xed\xef\x39\xa7\x82\x62\xf9\x48" //.`..Ng....9..b.H |
// /* 0840 */ "\xc3\x09\x71\x70\x88\xf8\x2e\x71\xb4\x56\xc4\x5c\x43\x9c\x45\x60" //..qp...q.V..C.E` |
// /* 0850 */ "\x0b\x9b\x15\x40\xf0\xc4\x2c\x73\x76\xab\xdd\x7b\x9f\xa4\x61\x78" //...@..,sv..{..ax |
// /* 0860 */ "\xe7\xc6\x73\x6a\x71\x0b\x7c\xa8\xa9\xe2\x65\x53\xee\x2e\xd2\x7e" //..sjq.|...eS...~ |
// /* 0870 */ "\x28\xe8\x40\xc6\x1a\x4e\x67\x2e\x72\x33\xfe\xdf\x67\x0f\xeb\x5d" //(.@..Ng.r3..g..] |
// /* 0880 */ "\x0b\xe2\xbf\x7f\x99\x45\x3e\xd6\x8f\xb6\xfe\x21\x71\x8e\x2c\x23" //.....E>....!q.,# |
// /* 0890 */ "\x88\x7b\x88\x30\x47\x19\xfa\x0c\x20\x9c\x40\xce\x5f\x49\xfd\xf4" //.{.0G... .@._I.. |
// /* 08a0 */ "\x71\xe7\xdd\x35\xbf\x22\x4f\x49\xe8\xf2\xf4\xfc\xc7\xe4\xae\x92" //q..5."OI........ |
// /* 08b0 */ "\x8e\x91\x53\xf4\xa3\xcb\x8f\x23\x2a\x00\xd4\x31\x8f\x4f\xce\x16" //..S....#*..1.O.. |
// /* 08c0 */ "\x1b\x7a\x7e\xf0\x25\x73\xa2\xfe\x65\x74\x5a\x07\x25\x55\xc0\x00" //.z~.%s..etZ.%U.. |
// /* 08d0 */ "\x0b\x91\xeb\x14\x68\xd0\xcd\xbf\x92\xa4\x72\x49\x8e\x47\xaa\x86" //....h.....rI.G.. |
// /* 08e0 */ "\x0a\x3d\x73\x1a\x8e\x4a\x91\x85\x0f\xa7\x0a\x28\xc2\xae\x33\x77" //.=s..J.....(..3w |
// /* 08f0 */ "\x8b\x82\xef\x86\x27\x96\xf6\xdc\xb5\xd0\x1a\xd4\x4f\xa0\x74\x80" //....'.......O.t. |
// /* 0900 */ "\x01\x72\xdb\x40\x6b\xb0\xb1\x5e\x5b\x6e\x84\x41\x5e\x0f\x91\xd0" //.r.@k..^[n.A^... |
// /* 0910 */ "\x71\xcb\x6d\xe0\x06\x2d\x63\x7f\xd8\xc3\x7a\x2f\xfb\x78\x0c\x45" //q.m..-c...z/.x.E |
// /* 0920 */ "\xd0\x4b\x70\xee\x6a\x55\xd0\x36\xf9\xa9\xe1\xcc\x5a\x8c\xbf\xc6" //.Kp.jU.6....Z... |
// /* 0930 */ "\x3c\x5d\x5d\x04\xe3\x01\xdb\x64\x05\xcf\x11\xcf\xfe\xa3\xd8\x8d" //<]]....d........ |
// /* 0940 */ "\x5b\x99\xc3\xc0\x53\xcf\x33\x63\x0f\xb2\xf0\x33\x17\x4d\x3a\xe9" //[...S.3c...3.M:. |
// /* 0950 */ "\xb0\x1f\x63\x01\xfd\x47\x59\xa6\x23\x57\xd2\xc1\xd0\x1a\x44\x00" //..c..GY.#W....D. |
// /* 0960 */ "\xb0\x21\x96\xfe\x6b\x8f\x39\x3a\x03\x5e\x16\x38\x60\x45\xa7\x40" //.!..k.9:.^.8`E.@ |
// /* 0970 */ "\x68\xc1\x5a\x91\xfd\x96\xef\x16\xc8\x7f\xf3\xc7\x10\xa2\xc7\x40" //h.Z............@ |
// /* 0980 */ "\x2c\xf0\x2d\x75\x61\xce\x1d\xb8\x27\xc7\x05\x67\x85\x63\x9f\x25" //,.-ua...'..g.c.% |
// /* 0990 */ "\x47\x30\xc0\x34\x62\xfc\x26\x37\x0a\x8c\x2d\xca\xfa\xaf\x0c\xb0" //G0.4b.&7..-..... |
// /* 09a0 */ "\xaa\xc5\x7b\xd9\xfe\xaf\x02\x23\xcf\x17\x07\x34\x87\xfa\x79\x26" //..{....#...4..y& |
// /* 09b0 */ "\x03\x70\xd2\x0b\x0c\x4d\x47\xa3\x22\x80\x1b\x6f\x7b\xfb\x4b\x85" //.p...MG."..o{.K. |
// /* 09c0 */ "\x8b\xfb\x4d\xa7\xfb\x63\x67\xfe\xd1\x8d\x78\x9d\x7e\x13\xd1\xd8" //..M..cg...x.~... |
// /* 09d0 */ "\x1a\x74\x30\x0e\xcf\x60\xe8\x02\x2b\x92\xc8\x2e\xec\x56\x0e\x7f" //.t0..`..+....V.. |
// /* 09e0 */ "\x6b\x3c\xf5\x92\xc1\x7f\x19\x81\x07\xc1\x28\x75\xe6\x5b\x05\xfc" //k<........(u.[.. |
// /* 09f0 */ "\x62\x04\xdb\x0c\x61\xb7\xb3\x58\x2f\xe2\xf0\x20\xaf\xc4\x67\xb0" //b...a..X/.. ..g. |
// /* 0a00 */ "\x5f\xc5\xa0\x36\x69\xeb\x4d\x83\x9f\x90\x3e\x09\x23\xab\x35\x58" //_..6i.M...>.#.5X |
// /* 0a10 */ "\x2f\xe2\x70\x24\xe8\x61\x0d\x9d\xae\xc1\x7f\x12\x81\x4b\x62\x90" ///.p$.a.......Kb. |
// /* 0a20 */ "\x0b\xbb\x65\x83\x9f\x80\xaa\x4b\x40\xbf\xb6\xd8\x39\xf5\x30\x2a" //..e....K@...9.0* |
// /* 0a30 */ "\x12\x59\x56\x6f\xb0\x60\x55\x28\x81\x57\x6f\xf0\xfb\xbd\x3d\xb8" //.YVo.`U(.Wo...=. |
// /* 0a40 */ "\x23\x82\xec\x53\x7c\xb4\xc4\xb9\x4e\x83\xe7\x3b\xfd\xfc\x2f\xcc" //#..S|...N..;../. |
// /* 0a50 */ "\xaf\x7e\xb1\x8b\x2c\xdf\xef\xe1\x3c\xa3\xd8\xe4\x04\x0f\xd0\xb9" //.~..,...<....... |
// /* 0a60 */ "\x7b\xaf\x3b\xdf\xef\xe0\xdd\x26\xcb\xb6\xfc\x2d\xc4\x05\xfa\x45" //{.;....&...-...E |
// /* 0a70 */ "\xdb\x7b\xcf\xc9\x74\x9e\x76\xd5\x02\x7e\x93\xdd\xfe\xfe\x07\xfd" //.{..t.v..~...... |
// /* 0a80 */ "\xd8\xb7\x61\x8c\x4a\x61\xf7\x18\xe6\x04\x20\x98\xc5\xcd\xc0\x19" //..a.Ja.... ..... |
// /* 0a90 */ "\x88\x41\x87\xf8\xb5\x26\xb0\xce\x83\xd7\x37\x86\x21\xc0\x10\x29" //.A...&....7.!..) |
// /* 0aa0 */ "\x64\x52\x43\x77\xa0\x07\x58\x2f\xfc\xb9\xd6\x0b\xfd\x11\xbb\x0e" //dRCw..X/........ |
// /* 0ab0 */ "\xb5\x02\x28\xcd\xc1\x10\x22\xcb\x3d\x01\xee\x88\x81\x0c\xa1\xb0" //..(...".=....... |
// /* 0ac0 */ "\x43\x28\x70\x08\xa5\x0a\x41\x14\xe4\x87\x11\xbb\xbe\x0c\x10\x00" //C(p...A......... |
// /* 0ad0 */ "\x29\x40\x67\xb8\x81\xa1\x5f\xbf\xdc\x1b\x78\x04\x4e\x05\x1f\x78" //)@g..._...x.N..x |
// /* 0ae0 */ "\x3e\xf0\xc8\xf4\x62\x3d\x35\xff\x1a\x63\x3f\x8a\x87\x40\x8e\x40" //>...b=5..c?..@.@ |
// /* 0af0 */ "\x4d\x6c\xf4\x21\x90\x08\x0c\x36\x6b\x42\xa0\x47\x28\x9f\x49\x00" //Ml.!...6kB.G(.I. |
// /* 0b00 */ "\x8a\x78\x3d\xe1\x58\x20\x82\x50\xc0\x20\x90\x10\x08\x23\x2f\x77" //.x=.X .P. ...#/w |
// /* 0b10 */ "\xe5\x6e\xbd\x42\x06\x00\xe8\x46\xc0\x95\x2b\x85\xef\xc2\x53\x85" //.n.B...F..+...S. |
// /* 0b20 */ "\xca\x36\xf0\x96\x16\x07\xba\x25\x01\xee\x87\x20\x73\xa0\x03\xc0" //.6.....%... s... |
// /* 0b30 */ "\x1c\xee\x64\x18\xb7\x16\xf9\xf4\x6d\x4d\xc4\x94\xca\x0d\xd2\x80" //..d.....mM...... |
// /* 0b40 */ "\x05\x87\xbe\xe0\x06\x9b\x00\x73\x80\x80\xe3\x36\xe0\x04\x63\xf7" //.......s...6..c. |
// /* 0b50 */ "\x73\xfd\xb5\xe2\x02\x45\x6c\xfe\xaf\xdc\xf9\xa8\xc4\x06\xc4\xc3" //s....El......... |
// /* 0b60 */ "\x4d\xa0\x47\x3f\x76\x3c\xf9\x5f\xba\xc4\x3f\x74\x79\x1e\xa8\x02" //M.G?v<._..?ty... |
// /* 0b70 */ "\x1f\x8b\x2f\x86\x03\xc3\x2e\x6f\xcb\x7d\xe8\x40\x1c\x02\x22\x04" //../....o.}.@..". |
// /* 0b80 */ "\xc7\x6f\x6e\x86\x3f\xf9\x29\x20\xbb\xdc\xe0\x1c\x04\xde\x13\x70" //.on.?.) .......p |
// /* 0b90 */ "\xb3\x50\x00\xa2\x22\x14\xd2\x06\x38\x0b\xb6\x27\x25\x43\x84\x00" //.P.."...8..'%C.. |
// /* 0ba0 */ "\x13\x60\x33\x9a\xb6\x01\x43\x8b\x83\x97\xac\xa6\xaf\xe0\x6c\x6a" //.`3...C.......lj |
// /* 0bb0 */ "\xbe\x06\x8e\x29\x88\x0a\x4c\xb4\x7e\x07\x60\xbf\xb2\x53\x83\xe0" //...)..L.~.`..S.. |
// /* 0bc0 */ "\x83\xdf\x20\xc3\x6f\x96\xf3\xea\x71\x01\x5f\xd2\x65\xa3\xdc\x55" //.. .o...q._.e..U |
// /* 0bd0 */ "\x98\x97\x2a\x3e\x7a\xc5\x5e\x2e\xfb\xf2\xc1\x6f\x95\xfd\xf9\x90" //..*>z.^....o.... |
// /* 0be0 */ "\x5d\xd1\xfe\xfc\xc0\x81\x28\xdf\xe2\xa0\x75\x6f\x91\x40\x36\x7f" //].....(...uo.@6. |
// /* 0bf0 */ "\xd3\x84\x37\xf4\xcd\x02\xfe\xe1\x86\xd4\xd6\x73\x96\xf7\x1b\x7c" //..7........s...| |
// /* 0c00 */ "\x09\xce\xdf\x5d\xbf\xe1\xb1\x39\x55\x96\xf1\x47\x71\x40\x71\x46" //...]...9U..Gq@qF |
// /* 0c10 */ "\x74\x07\x8c\x02\xb3\x1d\x00\x67\xe3\x27\x55\x61\xfe\x8d\x0c\x68" //t......g.'Ua...h |
// /* 0c20 */ "\x0b\x1d\x1c\x21\x76\xdf\xd0\xbd\xf2\x27\xe8\x3e\xf3\xa6\x6d\x7e" //...!v....'.>..m~ |
// /* 0c30 */ "\x47\xc6\x7f\x0a\xff\x95\xe0\x8a\xe2\x74\xe4\x7a\xf5\x3c\x11\x7a" //G........t.z.<.z |
// /* 0c40 */ "\x9f\x12\x0f\x53\xb1\xc9\x5e\x30\x2c\x34\x6b\x1c\x9a\xa3\x1c\x95" //...S..^0,4k..... |
// /* 0c50 */ "\xcf\xd5\x8e\x53\x78\xf2\xd5\xdb\x96\x43\x95\xcb\x94\xd3\x96\xd3" //...Sx....C...... |
// /* 0c60 */ "\x97\x0c\x72\xa1\x72\xd2\x71\xb0\x0e\x77\x8f\x63\x3e\x77\x20\x56" //..r.r.q..w.c>w V |
// /* 0c70 */ "\x03\x93\x5f\x3f\xd4\x87\xff\xac\x79\x97\x47\x32\x77\x45\x2c\x39" //.._?....y.G2wE,9 |
// /* 0c80 */ "\x97\x57\x2e\xa7\xdf\xbf\x2a\xba\x1a\x6f\x0e\xe1\x9c\xe7\x00\xc1" //.W....*..o...... |
// /* 0c90 */ "\x24\x91\x82\x4a\xa3\xc9\xbc\xfc\xfa\x9c\xf5\x58\x06\x0e\x80\x79" //$..J.......X...y |
// /* 0ca0 */ "\xf7\x39\xfa\x30\x7f\xf4\x73\xe6\x62\x49\xcd\x3f\xd5\x77\x0b\x7f" //.9.0..s.bI.?.w.. |
// /* 0cb0 */ "\xab\xc8\x96\xd1\x80\x5e\x73\xc5\x7d\x2c\x20\xc0\xc3\xa9\xcc\x7e" //.....^s.}, ....~ |
// /* 0cc0 */ "\xb3\x1f\xfe\x38\xe7\xb0\xfe\xfb\x00\xba\xcf\x65\x42\x06\x2e\x51" //...8.......eB..Q |
// /* 0cd0 */ "\x93\x00\x2c\x7e\xd7\xcf\x91\x4c\x67\x15\x6c\x31\xcb\xd3\x1f\xad" //..,~...Lg.l1.... |
// /* 0ce0 */ "\xcf\x02\x88\xc2\x1c\x70\x05\x01\xac\x59\x69\x6d\xc6\x98\x43\xce" //.....p...Yim..C. |
// /* 0cf0 */ "\x00\xba\x2a\x42\xf3\x8f\x82\x9e\x48\x8c\x38\x57\xb6\x40\xa5\x8f" //..*B....H.8W.@.. |
// /* 0d00 */ "\x78\xbc\x15\x18\x05\xb5\xe0\xa9\xc7\x3f\xec\x75\x71\x04\xfe\x0a" //x........?.uq... |
// /* 0d10 */ "\x90\x56\xe3\x48\x09\x8d\x32\x95\x19\x4b\xa0\x02\xaf\x38\xf4\x6c" //.V.H..2..K...8.l |
// /* 0d20 */ "\x89\x38\xbf\x82\x2f\x06\x1c\x3c\x11\x78\x32\x5e\x30\x1c\xd4\x78" //.8../..<.x2^0..x |
// /* 0d30 */ "\x09\xe0\x88\xe8\xae\xf0\x44\xe3\xf4\x7c\xfc\x17\x21\x31\x94\xba" //......D..|..!1.. |
// /* 0d40 */ "\x80\x12\x81\x90\x0f\x78\x25\x0d\xb5\xae\x80\x13\x27\x17\xa8\x01" //.....x%.....'... |
// /* 0d50 */ "\x41\x30\x0c\xdb\xf8\x3d\x4f\x3e\x15\x8f\x72\xed\xc3\x6a\x3d\x42" //A0...=O>..r..j=B |
// /* 0d60 */ "\xf0\xab\xc1\xec\x76\x68\xc7\x4a\x73\xe0\xc9\x0d\xc1\x22\x8c\x75" //....vh.Js....".u |
// /* 0d70 */ "\x77\x18\xed\x76\x3f\x13\x1d\xb5\x3f\xf4\x72\xeb\x7f\xeb\xef\x9c" //w..v?...?.r..... |
// /* 0d80 */ "\xe4\x07\x3f\xe1\x86\x10\xdf\xce\x44\x30\xb7\x69\x83\xde\xaf\xd3" //..?.....D0.i.... |
// /* 0d90 */ "\x1d\x2e\x36\xe7\xc0\x8d\x70\x4d\x61\xca\x86\x42\x8f\x00\x58\x30" //..6...pMa..B..X0 |
// /* 0da0 */ "\x29\x93\x04\x7b\x47\x6a\xa6\x3c\x15\x7f\xb8\xc7\xd3\xea\xdd\x7b" //)..{Gj.<.......{ |
// /* 0db0 */ "\xba\xd4\x86\x36\xb5\xe1\x9b\x17\x62\x28\xfd\xa0\xbf\x1d\xa1\xb1" //...6....b(...... |
// /* 0dc0 */ "\x4e\x9d\xb1\x03\xab\x74\xe4\x40\xcb\xa0\x88\x94\xea\x05\xff\x91" //N....t.@........ |
// /* 0dd0 */ "\xd0\x66\x06\x1f\x04\x05\xa3\x48\x2e\xfd\x3c\xc2\xff\x2d\x46\x9e" //.f.....H..<..-F. |
// /* 0de0 */ "\xc0\x2e\xfd\x3b\x02\xd0\x24\x99\x6b\x05\xbd\xa3\xa7\x2c\x5a\xe3" //...;..$.k....,Z. |
// /* 0df0 */ "\x84\x36\x7e\x9c\xa1\x2e\x9c\xaa\x1d\x7b\xa6\xe0\x43\xf3\x2c\xd3" //.6~......{..C.,. |
// /* 0e00 */ "\xde\x9b\xd2\x1f\x02\xb5\x47\x10\x2d\xfd\x37\x41\xb0\x22\xe8\x24" //......G.-.7A.".$ |
// /* 0e10 */ "\x0e\xac\xe3\x60\xbf\x86\x40\x9a\x25\xb2\x0b\xbb\x95\x82\xfe\x15" //...`..@.%....... |
// /* 0e20 */ "\xd4\xd9\x62\xaf\x3a\x58\x2f\xe1\x1f\xbe\xdd\x6c\x1d\x34\x94\xc8" //..b.:X/....l.4.. |
// /* 0e30 */ "\xe3\x0d\xbd\xde\xc1\x7f\x06\x80\x83\x51\x9e\x6c\x17\xf0\x48\x13" //.........Q.l..H. |
// /* 0e40 */ "\x74\x30\x06\xce\xf5\x60\xbf\x81\xc0\x94\xad\xf2\x0b\xbb\xdf\x2b" //t0...`.........+ |
// /* 0e50 */ "\xe0\x84\x12\x73\x56\x94\xdd\xdf\x71\x2c\xe5\x64\x34\x87\xae\x6f" //...sV...q,.d4..o |
// /* 0e60 */ "\x15\x54\xb0\x51\xfc\x64\x05\xb7\x7f\x38\x37\xc5\x42\x61\xc8\x20" //.T.Q.d...87.Ba.  |
// /* 0e70 */ "\x7c\x10\xa6\x4b\x03\xab\x40\x2e\xe3\x3f\x28\xd1\x2d\xa1\x57\xa4" //|..K..@..?(.-.W. |
// /* 0e80 */ "\x17\x76\x3f\x41\xc0\x54\x96\x87\x5e\xa0\x5f\xfd\x06\x14\x81\xc3" //.v?A.T..^._..... |
// /* 0e90 */ "\x22\x88\x94\xe8\x05\xdf\xad\x50\x47\xc8\x89\x3c\xd4\x86\xcf\xe7" //"......PG..<.... |
// /* 0ea0 */ "\xe0\x7c\x0e\x97\x1c\x91\xd5\xa8\x17\xf6\xef\x36\x3d\x9a\xb3\x4f" //.|.........6=..O |
// /* 0eb0 */ "\x58\x2d\xfe\x77\x05\x40\xe5\x91\x4f\xad\x0e\x76\x0b\xfe\x3e\x8a" //X-.w.@..O..v..>. |
// /* 0ec0 */ "\x61\xc8\x79\xc4\x58\x4b\x74\xf4\x8a\xce\xed\x60\xbf\xf2\x6b\x40" //a.y.XKt....`..k@ |
// /* 0ed0 */ "\xa2\x17\x78\x15\xdd\xe3\xcd\x28\x1d\xfa\xfe\xd7\xe6\xb7\x9d\x1c" //..x....(........ |
// /* 0ee0 */ "\x0a\x16\xf8\xe4\x13\xfa\x1f\x39\xce\xa4\x75\x6b\x9a\xf8\x17\x9d" //.......9..uk.... |
// /* 0ef0 */ "\x6b\xa2\x2e\xde\x2e\x22\x53\x90\x70\xf2\xa3\xab\x7a\xc1\x89\x9c" //k...."S.p...z... |
// /* 0f00 */ "\xef\xa4\xae\xdd\x23\x90\x12\x1b\xaf\x48\xe5\xe0\x01\xb6\x02\x03" //....#....H...... |
// /* 0f10 */ "\x75\x00\x8f\xc0\x6c\x37\x40\x09\xff\x4d\x65\xc2\x65\xc9\x2a\x79" //u...l7@..Me.e.*y |
// /* 0f20 */ "\x6b\xed\xfa\x23\xa6\x1d\xf2\xd7\xac\x03\xc7\xb5\x00\x33\xcb\x2a" //k..#.........3.* |
// /* 0f30 */ "\x07\x3f\xd5\x96\x7b\x0c\xc3\xe0\x0f\x18\xde\x0e\x57\xdb\xdf\xaa" //.?..{.......W... |
// /* 0f40 */ "\x00\x3f\x00\xc7\x0d\x9a\x25\x82\x20\x5b\xc9\x28\xd2\xb8\xa7\x01" //.?....%. [.(.... |
// /* 0f50 */ "\x27\xbf\xd0\x46\x0b\x7a\x25\x3a\x97\x1c\xf9\xb5\x03\x80\x5d\xd8" //'..F.z%:......]. |
// /* 0f60 */ "\xfe\x68\x77\xe5\xc2\x05\xbe\xcb\x3d\x4d\x86\x0e\xf0\xf0\x78\x7d" //.hw.....=M....x} |
// /* 0f70 */ "\x42\xaf\x14\x49\x7f\x1d\xb0\x7f\xd3\x64\x33\xf1\xc8\x0d\x9d\xa2" //B..I.....d3..... |
// /* 0f80 */ "\x5a\x1f\x04\xfc\x71\x03\x90\x5d\xd1\x29\xcd\xbf\xba\x60\xd4\x0b" //Z...q..].)...`.. |
// /* 0f90 */ "\xfd\x8e\xbd\x2f\x9a\xf1\x63\xae\x03\x6e\x00\x8e\xa5\xe7\x2b\x7f" //.../..c..n....+. |
// /* 0fa0 */ "\x01\x47\x00\x44\x0d\x86\xde\x9d\x6f\xbf\x5f\xff\x4c\x78\xe8\x0d" //.G.D....o._.Lx.. |
// /* 0fb0 */ "\x46\x98\x45\x22\x51\x47\xef\xc4\x90\x5d\xc9\x28\xd7\x34\x0e\x02" //F.E"QG...].(.4.. |
// /* 0fc0 */ "\x37\x8d\xd4\x9e\x7e\xbb\x0f\x9f\x9c\x6d\xa3\x86\x3d\x90\xf1\xf0" //7...~....m..=... |
// /* 0fd0 */ "\x71\xb5\x70\x6e\x9c\x7e\x31\x3c\x6f\xe3\x8d\xb4\x62\x18\xf9\xc3" //q.pn.~1<o...b... |
// /* 0fe0 */ "\x37\x08\x94\x50\xba\x08\x77\x50\xc4\x34\xb1\x82\x29\x18\x87\xa5" //7..P..wP.4..)... |
// /* 0ff0 */ "\x7f\x25\xa3\x17\xf9\x87\x0b\x2c\xf7\xcd\x97\x00\xbb\xc9\xa6\xbf" //.%.....,........ |
// /* 1000 */ "\x2e\xd0\x2d\xe8\xfe\x82\xb8\x11\x0d\x9e\xd5\x1c\xea\xb1\x34\x2a" //..-...........4* |
// /* 1010 */ "\xf4\x32\x28\x3d\x04\xea\x81\x0c\x94\xe2\xef\xb9\x3f\xf5\x07\xfe" //.2(=........?... |
// /* 1020 */ "\x57\x02\x01\xb3\xf5\x87\x7d\xe9\x24\xeb\x00\xfa\x72\xc1\x6f\x09" //W.....}.$...r.o. |
// /* 1030 */ "\x92\xff\xa9\x91\x55\x93\xba\x9a\x3a\xd1\xc4\x1e\x6b\xf4\x64\x06" //....U...:...k.d. |
// /* 1040 */ "\xcf\xd6\x2f\x0f\xa3\x72\x2a\xb7\x5a\x4a\x80\x5d\xcd\x32\x17\x9a" //../..r*.ZJ.].2.. |
// /* 1050 */ "\xc1\x88\xcb\xce\x98\x7e\x9e\xb9\x6d\xf9\x38\x3d\xcf\x6e\xac\x18" //.....~..m.8=.n.. |
// /* 1060 */ "\x00\x58\xba\x0e\x39\xa1\xc7\x55\xf9\xc4\x51\x6e\x68\x2c\x00\x3d" //.X..9..U..Qnh,.= |
// /* 1070 */ "\x1c\xd0\x23\x9b\xdd\xf7\xc4\xdb\x9e\x1b\x9c\xcb\x9b\xd3\x9e\xab" //..#............. |
// /* 1080 */ "\x9e\xe4\x00\x58\x2a\x18\xfe\xa2\x56\x72\x32\x43\xc6\x6a\x1f\x01" //...X*...Vr2C.j.. |
// /* 1090 */ "\x36\x06\x89\x99\x7d\x37\x9e\xc4\x73\xc1\x73\x8f\x9e\xb2\x93\xd0" //6...}7..s.s..... |
// /* 10a0 */ "\x8b\x1c\x44\x30\xf0\x95\xe3\xf7\xf8\xe8\xba\xf0\x63\xc2\x51\x91" //..D0........c.Q. |
// /* 10b0 */ "\x96\xeb\x18\xb1\xb8\x35\x3a\x10\xc7\x47\x28\xf0\xac\x74\x66\x40" //.....5:..G(..tf@ |
// /* 10c0 */ "\x5e\xe4\x94\x00\x52\x17\x4e\x8c\xce\xb5\x73\x40\xb8\x78\x2e\xf1" //^...R.N...s@.x.. |
// /* 10d0 */ "\x20\x73\xc1\x77\x90\x07\x99\x19\x97\x60\x0b\x06\x3f\xad\xc3\xa8" // s.w.....`..?... |
// /* 10e0 */ "\x24\x78\x22\x29\x64\x2e\x95\x00\x20\x03\xaf\xa0\x6d\x6b\xa5\xd0" //$x")d... ...mk.. |
// /* 10f0 */ "\x2e\x15\x00\x26\x0c\x7f\xcb\x5e\x0c\xbc\x11\xf9\xb7\x58\x17\x1b" //...&...^.....X.. |
// /* 1100 */ "\x89\xc0\x66\x6c\x63\x16\xf0\x54\x75\x89\x50\x03\x71\x35\x33\x02" //..flc..Tu.P.q53. |
// /* 1110 */ "\x41\xba\xc3\xba\xc6\xb0\x28\xdc\x75\x8d\x60\xe1\x17\xff\x8f\xea" //A.....(.u.`..... |
// /* 1120 */ "\xff\xf7\x80\xf9\xe0\xc3\xa6\x70\x3c\x01\xc3\x21\x6a\xb2\x34\xc3" //.......p<..!j.4. |
// /* 1130 */ "\xc7\xb1\x85\xd8\x5c\x4f\xe7\x04\x99\xbc\x0d\x1e\x0f\x9f\x0b\xb2" //.....O.......... |
// /* 1140 */ "\x36\x83\xae\x3c\x28\x01\x61\x81\x8d\xc4\x8b\x80\x18\x32\x38\x7c" //6..<(.a......28| |
// /* 1150 */ "\x64\x71\xca\xc8\x2b\xf1\xd7\x11\xe8\x98\xe0\xbd\x18\xdb\x91\x70" //dq..+..........p |
// /* 1160 */ "\x97\x78\xd7\x8c\xe2\xa3\x92\x62\x40\x0f\x19\x1e\x0d\xe3\x28\xc8" //.x.....b@.....(. |
// /* 1170 */ "\x1f\x91\x50\xc7\x21\xb3\x88\x2f\xe7\x80\x74\x64\x65\x3b\xb8\x4a" //..P.!../..tde;.J |
// /* 1180 */ "\xff\xec\xfa\xc7\x61\x9c\x0d\x56\x26\xf1\xa8\x2b\xff\x32\x40\x92" //....a..V&..+.2@. |
// /* 1190 */ "\x81\x90\x60\xe3\x83\xe4\xe4\x63\xde\x1f\x3a\x3f\xfc\x62\x34\xa7" //..`....c..:?.b4. |
// /* 11a0 */ "\xff\xbe\x63\x4c\x0c\x66\xc6\xbf\xcf\x4c\xce\x04\x12\x64\x62\x88" //..cL.f...L...db. |
// /* 11b0 */ "\xc9\x12\x6f\xcd\x1c\x55\xdf\x9a\xf0\x34\x04\x7c\x13\x05\x03\x0f" //..o..U...4.|.... |
// /* 11c0 */ "\xbd\x3c\xc1\x39\x09\xaa\x2c\x1c\x1a\x9f\x9e\x3a\xf0\xed\xd8\x32" //.<.9..,....:...2 |
// /* 11d0 */ "\x0f\x80\x18\x32\x60\x5c\xb0\x64\x92\xf7\x57\xbc\xe5\x59\x16\x13" //...2`..d..W..Y.. |
// /* 11e0 */ "\x5b\x84\xff\xc4\xb3\x80\x58\x1f\xfa\xf0\x27\xc0\x00\x9f\xff\x06" //[.....X...'..... |
// /* 11f0 */ "\x00\x19\xf8\x1e\x32\x4d\x1c\x78\xae\x32\x22\x53\x21\x8a\x4f\x03" //....2M.x.2"S!.O. |
// /* 1200 */ "\xd8\xc1\xd7\x0e\x63\xce\x09\x95\x1a\xc8\x62\xb1\x70\x9d\xf8\x12" //....c.....b.p... |
// /* 1210 */ "\x70\x81\x63\xc0\x97\x84\x14\x00\x01\x26\xe0\x0c\x19\x28\x89\x92" //p.c......&...(.. |
// /* 1220 */ "\x8b\x67\x81\xf5\x1a\x64\xa3\xc7\x03\xdc\x2e\xa0\x3d\x68\xc5\x52" //.g...d......=h.R |
// /* 1230 */ "\x51\x88\x31\x7c\x10\x2a\x4b\x46\x9e\xc0\xea\xc4\x36\x7c\x08\x23" //Q.1|.*KF....6|.# |
// /* 1240 */ "\x22\x70\x58\x6c\x52\x4f\x35\x64\xa7\x00\xd9\xe0\x17\x7e\xba\x40" //"pXlRO5d.....~.@ |
// /* 1250 */ "\xf8\x25\x2e\x39\x66\x9e\xd8\xba\x98\x3a\xe1\x02\xa0\xb7\xeb\xfa" //.%.9f....:...... |
// /* 1260 */ "\x7d\x38\x96\x8c\x03\x67\xa0\x5f\xfa\x84\x01\xa0\xa1\xf0\x42\xa9" //}8...g._......B. |
// /* 1270 */ "\x2b\x25\x3a\x05\x77\xeb\x63\x20\x70\x44\xe8\x65\x1a\x7a\x05\xf7" //+%:.w.c pD.e.z.. |
// /* 1280 */ "\x1d\x6b\x9e\x4f\x8c\x42\x5d\xb1\x7f\x05\xd0\x80\x03\x7e\x4b\x44" //.k.O.B]......~KD |
// /* 1290 */ "\x8f\x48\x45\x33\xd2\x0e\x13\x57\x82\x33\xe3\x04\x52\x38\x4d\x61" //.HE3...W.3..R8Ma |
// /* 12a0 */ "\x24\x01\x84\xd3\xe0\x69\xb0\xba\x5c\x69\x26\x45\x67\x98\xc9\xa7" //$....i...i&Eg... |
// /* 12b0 */ "\x30\x69\xca\x2d\x04\x92\x93\x43\x14\x4d\xa1\x5c\xd3\x2a\x00\xd5" //0i.-...C.M...*.. |
// /* 12c0 */ "\x34\xc0\x38\x51\x7f\x95\x0d\x12\xd0\x6a\x34\x0e\xad\x62\xaf\x20" //4.8Q.....j4..b.  |
// /* 12d0 */ "\xdb\xe0\x17\x79\x25\xb8\x82\xe0\xe2\x01\x78\x82\xd9\xbd\x0d\x30" //...y%.....x....0 |
// /* 12e0 */ "\x0f\x27\x3c\xf2\x80\xc8\xad\xce\x40\x17\x8f\x3a\xa0\x24\x56\xec" //.'<.....@..:.$V. |
// /* 12f0 */ "\x36\xa6\xba\xe2\xd3\x25\xd7\x12\x19\x88\x6e\xa0\x02\x5e\x5c\x88" //6....%....n..^.. |
// /* 1300 */ "\x3f\x97\x31\xc2\x36\xec\x22\x30\x95\x29\x17\xf0\x97\x79\x72\x0b" //?.1.6."0.)...yr. |
// /* 1310 */ "\xf8\x4b\xbc\xa2\xe1\xf3\x87\x9a\x19\x30\x3f\x4c\x06\x48\x80\xd0" //.K.......0?L.H.. |
// /* 1320 */ "\xe9\x1e\x4b\x50\x85\x13\x61\x64\x94\x14\x5a\x0b\x2c\xe6\x4d\x31" //..KP..ad..Z.,.M1 |
// /* 1330 */ "\xa3\xcc\x42\x25\x49\x32\x29\x54\x68\xb2\x17\x0e\x85\xd3\xe0\x62" //..B%I2)Th......b |
// /* 1340 */ "\x0e\x00\x17\x88\x5e\x08\x7e\x5c\x24\xb6\xc4\x1b\x7d\x0a\xbc\xc7" //....^.~.$...}... |
// /* 1350 */ "\x4e\x05\xa3\x33\xcb\x69\x51\xc4\xe8\x60\x7c\x10\x55\x8b\xac\x8c" //N..3.iQ..`|.U... |
// /* 1360 */ "\xb3\xf4\xeb\x48\x2e\xec\x16\xf0\x1b\x3c\x0e\xac\x0b\x46\x83\xcd" //...H.....<...F.. |
// /* 1370 */ "\x46\xbb\x40\x80\x20\x1e\x57\xe2\x37\x09\x00\x77\xe1\x05\x1e\xef" //F.@. .W.7..w.... |
// /* 1380 */ "\x50\x01\x3c\xa7\x1c\x56\x1c\x22\xd4\x0c\x98\x17\x87\x08\x88\xa6" //P.<..V."........ |
// /* 1390 */ "\x00\xc0\xa9\x71\xce\x83\x48\x47\x42\xb8\x60\xf1\x80\x92\x8c\x91" //...q..HGB.`..... |
// /* 13a0 */ "\x2d\x6c\xf1\x60\x67\xc2\xaf\x13\x08\x7c\x48\xa3\x4f\x6d\x51\xc2" //-l.`g....|H.OmQ. |
// /* 13b0 */ "\xf8\x27\x89\xa3\xa9\xca\x47\x56\x83\xcd\x5b\x61\x83\x9d\xf0\x9f" //.'....GV..[a.... |
// /* 13c0 */ "\x35\x22\xfb\x81\x1a\x37\xcd\x98\x05\xdd\x0e\xdf\x46\xec\x12\xf9" //5"...7......F... |
// /* 13d0 */ "\xb1\xd3\xa1\x42\xaf\x34\x4b\x6a\x18\xa2\xd5\xd1\x8e\x70\x0a\x7b" //...B.4Kj.....p.{ |
// /* 13e0 */ "\xca\xf7\xc5\x9d\x84\xc3\x06\x92\x52\x7f\x77\x38\xad\x2a\x3d\x4f" //........R.w8.*=O |
// /* 13f0 */ "\x1e\x5f\x9e\x2a\x6a\x86\x1b\x88\x9e\xb7\x2f\x31\x32\x63\x9e\x3c" //._.*j...../12c.< |
// /* 1400 */ "\x68\xea\x51\xc5\x7e\x70\xd1\x9e\xc7\xd5\x00\x13\x49\x28\x08\x03" //h.Q.~p......I(.. |
// /* 1410 */ "\xf6\x4c\x48\x9f\x02\x4f\x1d\x97\x1a\x39\xeb\x3f\x3d\x6b\x71\xe3" //.LH..O...9.?=kq. |
// /* 1420 */ "\x45\x72\x4f\x01\xcf\x24\xbf\x1f\x05\x83\x3f\x3d\xf2\xa2\x72\x98" //ErO..$....?=..r. |
// /* 1430 */ "\x8a\x17\x1b\xe7\x29\xb8\xe3\x7d\x1d\x3b\x23\x96\x05\xf2\x73\xd8" //....)..}.;#...s. |
// /* 1440 */ "\x9b\x86\x30\x64\x01\x9b\x97\xfc\xf8\x49\xcb\xcd\xc5\x8b\x1e\x61" //..0d.....I.....a |
// /* 1450 */ "\x51\xd3\x96\x78\xb1\xd4\xf7\xff\x71\x83\xff\xe1\xc0\x03\x07\x37" //Q..x....q......7 |
// /* 1460 */ "\xa8\xca\x1b\x6e\x6d\xce\x20\x58\x73\x85\x8e\x9a\x1a\x78\xf2\xf0" //...nm. Xs....x.. |
// /* 1470 */ "\x1b\x2b\xff\x5e\x07\xe1\x00\x17\x09\x00\x37\x73\xed\x9f\x1c\xb2" //.+.^......7s.... |
// /* 1480 */ "\x2d\x77\x3d\xc2\x6b\xfe\x27\x9d\x9e\xc1\xfc\x13\x06\x00\x0c\x1d" //-w=.k.'......... |
// /* 1490 */ "\x18\x9c\xbb\x18\x0d\x8a\x29\xff\xf5\xc1\x26\x70\xe8\x8c\xf4\x5f" //......)...&p..._ |
// /* 14a0 */ "\x74\x67\x9c\x07\x0e\xf1\xbd\x06\x84\xaf\x81\x87\xd2\x00\x22\x20" //tg............"  |
// /* 14b0 */ "\x14\x80\x0b\xf0\x00\x0f\xc6\x4b\x81\x3e\x74\x06\x80\xe0\x05\x23" //.......K.>t....# |
// /* 14c0 */ "\x28\xa4\x47\xe7\x99\x45\xa9\x3f\x19\x11\x00\x65\x1e\xfb\x31\xf3" //(.G..E.?...e..1. |
// /* 14d0 */ "\x21\x14\x7a\xe1\x49\x37\x36\xa5\x7e\xaa\xa4\x00\x32\x85\x7e\x79" //!.z.I76.~...2.~y |
// /* 14e0 */ "\x0e\x97\x02\x52\x7e\x81\x94\x7b\xea\x47\xcc\x8c\x50\xf4\x08\x00" //...R~..{.G..P... |
// /* 14f0 */ "\x03\x07\xc2\x95\x2b\xe5\xfe\xe1\x7f\x18\x00\xc0\x1a\x51\xf0\x6f" //....+........Q.o |
// /* 1500 */ "\x02\x35\x3e\x18\x11\x10\x00\x12\x49\xf0\xe0\x0a\x51\xf0\xe0\x04" //.5>.....I...Q... |
// /* 1510 */ "\x43\xe1\xc7\xc1\xd3\xa8\x0b\xc0\x52\x03\xe0\x29\x10\xf0\x14\x82" //C.......R..).... |
// /* 1520 */ "\x7c\x2b\x05\xa0\x03\x10\x02\x7c\x2c\x7e\xcb\xe0\xcf\xf2\xc0\x0b" //|+.....|,~...... |
// /* 1530 */ "\xf9\x67\xfd\x23\xe0\xc7\xf6\x8f\x85\x97\xc1\xc3\xf9\x60\x07\xfc" //.g.#.........`.. |
// /* 1540 */ "\xb5\xfd\xd4\x1f\xf1\x1f\x0b\x00\x7f\xa8\xf8\x35\xfc\xb0\x04\xfe" //...........5.... |
// /* 1550 */ "\x5b\xfc\x18\x03\xa0\x03\x30\x02\x10\x03\xd8\x01\x00\x01\x88\x32" //[.....0........2 |
// /* 1560 */ "\x81\xbb\xe0\xd8\x1f\xfd\x80\x70\x00\x10\x80\x88\x08\x44\xaa\x54" //.......p.....D.T |
// /* 1570 */ "\xab\xf0\xa4\x24\x6e\xb6\x7e\x7a\x0e\x07\x84\x5c\xc0\x03\xdf\x08" //...$n.~z........ |
// /* 1580 */ "\x82\x00\x5d\xf1\x9f\x22\xa0\x3f\x7d\xf0\xa0\x7b\xe6\x02\xba\x7d" //..]..".?}..{...} |
// /* 1590 */ "\x02\xc0\x82\x00\xc0\xc1\x01\x7c\x71\x80\x2f\x8e\x37\x07\xa0\x7b" //.......|q./.7..{ |
// /* 15a0 */ "\x0a\x3c\x6c\xa0\x02\x20\x7c\x88\x04\xfb\x38\x79\xab\x10\x6f\xe5" //.<l.. |...8y..o. |
// /* 15b0 */ "\xa6\x53\xd3\x8f\x84\x03\xb0\x79\xaa\x3e\x14\x03\x30\x01\xd8\x3e" //.S.....y.>..0..> |
// /* 15c0 */ "\xc3\xe0\xcf\xf2\xc0\x1f\xf9\x69\xf0\x7b\x40\x62\x07\x66\x1c\x3b" //.......i.{@b.f.; |
// /* 15d0 */ "\x06\x40\x07\xc0\x08\x5f\x1a\xd0\x80\x02\x81\x40\x01\x02\x01\x82" //.@..._.....@.... |
// /* 15e0 */ "\x01\xc2\x01\xc1\x38\x63\x60\x11\xe1\x29\xe1\x8d\xa8\x0d\xc0\x00" //....8c`..)...... |
// /* 15f0 */ "\x08\x23\x80\xdb\xe1\xc9\xc7\xf2\x7c\x38\xfa\x4f\xb0\xf8\x3a\x79" //.#......|8.O..:y |
// /* 1600 */ "\x53\x00\x9f\x01\x4f\x95\x36\x05\x80\x03\x7f\xe7\xc2\xc3\xe0\xe1" //S...O.6......... |
// /* 1610 */ "\xfc\xb0\x0a\xfe\x5c\x9c\x3e\x39\x7d\x0d\x16\x00\x70\x04\x60\x04" //......>9}...p.`. |
// /* 1620 */ "\x50\x05\x6f\x8d\x6a\xc0\x01\x41\x19\x55\xd0\x08\x0a\x08\x10\x14" //P.o.j..A.U...... |
// /* 1630 */ "\x11\xe8\x6c\x02\xfc\x25\x3e\x86\xd4\x1e\xc0\x01\xc0\xc9\xf0\xeb" //..l..%>......... |
// /* 1640 */ "\xe6\x6f\xf0\x74\xf2\xa6\x01\x9e\x02\x9f\x2a\x57\x9d\x23\xe1\x71" //.o.t......*W.#.q |
// /* 1650 */ "\x7c\x1c\x3f\x96\x01\xbf\xcb\x3f\x83\x87\x9f\x23\xe1\x5b\x00\x1c" //|.?....?...#.[.. |
// /* 1660 */ "\x83\xe8\xbe\x0c\xfa\x1a\x31\x0a\x69\x6b\x82\x31\xab\xe3\x5b\x50" //......1.ik.1..[P |
// /* 1670 */ "\x00\x50\x58\x00\x60\x48\x68\x18\x27\x04\xe7\x84\xb0\x0e\xf0\x97" //.PX.`Hh.'....... |
// /* 1680 */ "\x18\x1f\x10\x67\x0d\x68\xf0\x96\xdf\xcb\x00\xff\xe5\xc9\x00\x06" //...g.h.......... |
// /* 1690 */ "\x40\xf3\x84\x71\xac\x20\x62\x41\xb7\xc1\xbf\xe5\x80\x87\xf2\xcf" //@..q. bA........ |
// /* 16a0 */ "\xf2\x47\xc2\x94\x00\x48\x00\x28\x00\x30\x07\x0d\xe4\x00\x11\x03" //.G...H.(.0...... |
// /* 16b0 */ "\xec\xbe\x0c\x80\xcc\x38\x81\x32\xb7\x74\xb7\x00\x20\x00\x3f\x7c" //.....8.2.t.. .?| |
// /* 16c0 */ "\x6b\x7e\xf3\xcd\x07\x01\x87\x84\x08\x03\xe0\xdb\xad\xc4\x60\x20" //k~............`  |
// /* 16d0 */ "\x06\x14\xe6\x1b\x47\xc7\x61\xb8\x29\x41\xa4\x76\x8d\x48\x32\x3c" //....G.a.)A.v.H2< |
// /* 16e0 */ "\x19\xe3\x11\x4b\x0a\x29\xa8\x31\x8b\x9a\x06\x5b\x33\x2c\x86\x83" //...K.).1...[3,.. |
// /* 16f0 */ "\x50\x85\x72\x5b\x16\xf4\x01\x06\x1b\x47\x6b\x52\x0d\x20\x72\x08" //P.r[.....GkR. r. |
// /* 1700 */ "\x83\x58\x20\x8c\x00\x60\xf2\x00\xbc\x15\x06\xd0\x83\x48\x41\x90" //.X ..`.......HA. |
// /* 1710 */ "\x10\x38\x00\x1c\x62\x7f\xe5\x00\x05\x06\x31\x06\x20\x68\x20\x60" //.8..b.....1. h ` |
// /* 1720 */ "\x20\x58\x20\x48\x00\x38\x00\x20\xb9\xa4\xf8\x31\xcf\xa4\x06\xe1" // X H.8. ...1.... |
// /* 1730 */ "\xe0\xac\x71\x0a\xa8\x02\x86\xc3\x68\x81\x78\x05\x8a\xa6\x44\x80" //..q.....h.x...D. |
// /* 1740 */ "\x14\xf8\xe8\x48\x0c\x09\x06\xa0\x40\xa1\xf6\x50\x17\xc0\x90\xe8" //...H....@..P.... |
// /* 1750 */ "\x11\x03\x40\x06\x1e\xf8\x88\x09\xb8\x48\x76\x12\x09\x40\x6e\x5e" //..@......Hv..@n^ |
// /* 1760 */ "\x33\x4b\x7e\x21\xa3\xe2\x08\x80\x3c\xe5\x0f\x61\x23\xf0\x13\x6f" //3K~!....<..a#..o |
// /* 1770 */ "\x88\xb8\x7d\x24\x2f\xe2\x11\xbe\x20\x07\xed\x4a\xcf\xd6\x0a\x7c" //..}$/... ..J...| |
// /* 1780 */ "\x45\x3f\xe9\x00\x00\x29\x91\xc0\x9c\x5b\x91\x8f\x51\xed\xa8\x02" //E?...)...[..Q... |
// /* 1790 */ "\x11\xd0\x9b\x85\x48\x06\x1c\xf8\x12\xf6\x12\xf4\x12\xb2\x12\xb0" //....H........... |
// /* 17a0 */ "\x12\x6c\x12\x2a\x12\x26\x03\x71\xc7\x1d\xe9\x60\x03\x4f\x18\x41" //.l.*.&.q...`.O.A |
// /* 17b0 */ "\x10\x05\x3f\xff\x2e\x3f\xfa\xe3\x5f\xfb\x1c\x51\xa2\x00\xa0\x05" //..?..?.._..Q.... |
// /* 17c0 */ "\x47\x42\x6a\x00\x82\x61\x20\x30\x1a\x00\x0e\x13\x1f\x7c\xc0\x97" //GBj..a 0.....|.. |
// /* 17d0 */ "\xcc\x40\x00\x06\x20\x4f\x81\x28\xa0\x18\x55\x94\x2f\x02\x8e\x04" //.@.. O.(..U./... |
// /* 17e0 */ "\xb4\x80\x61\xae\x80\xbc\x06\x14\x01\x86\xb7\x82\xde\x67\x63\xf3" //..a..........gc. |
// /* 17f0 */ "\x90\xa4\x09\xec\x25\xa3\x88\xa0\x00\x4e\x04\xb0\x12\x48\x01\x88" //....%....N...H.. |
// /* 1800 */ "\x0e\x82\x50\x2b\x97\x4f\x76\x22\x89\x08\x7d\x23\x3f\x07\xc0\x00" //..P+.Ov"..}#?... |
// /* 1810 */ "\x74\x09\xe8\x26\x5a\x01\xee\xe0\x64\x0d\x64\x26\x59\xf7\x5e\xc3" //t..&Z...d.d&Y.^. |
// /* 1820 */ "\x04\xc0\xde\xc2\x6e\x5e\x08\x61\x2f\xc0\x37\xd6\x5c\x7e\x5b\x09" //....n^.a/.7..~[. |
// /* 1830 */ "\x76\x20\xcf\x9e\x77\x09\x84\xd3\xbc\x34\x22\x00\xb9\x9d\x60\xc6" //v ..w....4"...`. |
// /* 1840 */ "\x6a\x03\x93\xf9\xab\x1c\x0f\x84\xf3\x51\x84\x7e\xa0\x0a\xc0\x3c" //j........Q.~...< |
// /* 1850 */ "\x02\x35\x01\x60\x1f\xa8\x70\xf5\x00\xc0\x64\x03\xcf\x5f\x50\x09" //.5.`..p...d.._P. |
// /* 1860 */ "\x40\x01\x01\x80\xb0\x0a\x8d\x7d\x40\x58\x0c\x04\x00\x53\xe4\xac" //@......}@X...S.. |
// /* 1870 */ "\x03\xde\x90\x05\x0d\x40\x64\x02\xb4\x4c\x81\x1a\x60\x01\x7f\x61" //.....@d..L..`..a |
// /* 1880 */ "\x1f\xc8\x6f\x06\x01\xf1\xee\x01\x4c\x49\xf2\x00\x15\xc6\xee\x60" //..o.....LI.....` |
// /* 1890 */ "\xb1\xf8\x0a\x00\x57\xcc\x00\xf0\x2d\x82\x01\xf1\xf8\x0d\x83\xeb" //....W...-....... |
// /* 18a0 */ "\x20\x02\x9f\x59\xf6\x96\x0e\x07\x29\x07\x04\x79\xa0\x0f\x7d\x67" // ..Y....)..y..}g |
// /* 18b0 */ "\xda\x10\x7d\xa5\x83\xf8\x20\x1f\xc0\x81\xe0\xb8\xfb\x86\x0f\xb8" //..}... ......... |
// /* 18c0 */ "\x20\xfb\x88\x00\x55\x00\x2e\x42\x16\xc0\xc1\xe7\x7a\xe1\x60\xf3" // ...U..B....z.`. |
// /* 18d0 */ "\xbd\xe0\xe3\x58\x16\x80\x90\x05\x52\x58\x47\xc5\x60\x40\x00\x36" //...X....RXG.`@.6 |
// /* 18e0 */ "\x01\xe1\xd8\x15\x80\x01\x00\x79\xa9\x84\x7c\x4e\x05\x00\x0a\x00" //.......y..|N.... |
// /* 18f0 */ "\x1f\x6b\x61\x1f\x10\x81\x90\x00\x7f\xf0\xab\x6b\x08\xfd\x2c\x80" //.ka........k..,. |
// /* 1900 */ "\x06\x0d\x00\x01\x00\x0a\xf9\xe0\xb6\x01\x50\x01\xfb\x9c\x34\x0b" //..........P...4. |
// /* 1910 */ "\xce\xf0\x3e\x80\x2a\x35\x06\x80\x0f\x10\x0a\xd0\x02\xec\x18\x1a" //..>.*5.......... |
// /* 1920 */ "\x11\xe5\x20\x0c\x84\x2e\x13\x20\x14\x80\x0a\x20\x0a\x00\x0a\x48" //.. .... ... ...H |
// /* 1930 */ "\x02\xec\x01\x44\x01\x4c\x00\x2c\x00\x0c\x1c\x1b\x5e\x3f\x8c\x48" //...D.L.,....^?.H |
// /* 1940 */ "\x8b\xb8\x23\x08\xfc\x05\x80\x28\x2c\x53\xd8\xcf\x92\xc2\x8f\x09" //..#....(,S...... |
// /* 1950 */ "\x00\xf7\x06\x40\x9c\x06\xc1\xe9\x07\xaa\xa1\x00\x04\x7a\x02\x40" //...@.........z.@ |
// /* 1960 */ "\x3d\xe6\xa0\x97\xb4\xc3\xd0\x26\x01\xfb\x07\xe4\x58\x05\x38\x3f" //=......&....X.8? |
// /* 1970 */ "\x84\x7f\x98\x7a\x03\x40\x14\xf4\x80\x3e\x04\x00\x3f\x20\xfb\x00" //...z.@...>..? .. |
// /* 1980 */ "\x00\xbe\x11\xc0\xef\xa8\xe1\x14\x0d\x81\x00\x0f\x20\x61\xd6\x13" //............ a.. |
// /* 1990 */ "\x00\xf5\x00\x03\x0a\x6f\x02\xfe\x4c\xe1\x4e\x00\x57\xd8\x50\x38" //.....o..L.N.W.P8 |
// /* 19a0 */ "\x50\x40\x4c\x0a\x80\x28\x80\x01\x85\x37\x80\xff\xf9\x60\x0a\xe1" //P@L..(...7...`.. |
// /* 19b0 */ "\x4e\x06\x81\x00\x0a\xc8\x00\x30\xa6\x78\x12\x04\xc0\x2b\x20\x01" //N......0.x...+ . |
// /* 19c0 */ "\xc3\x9f\x88\x23\x08\xf9\x00\xfd\x00\x52\x40\x03\x83\xc1\x50\x2b" //...#.....R@...P+ |
// /* 19d0 */ "\x00\x57\xa1\x98\xd2\x4d\xe8\x68\xc3\x08\xaf\x43\x5e\x29\x55\x80" //.W...M.h...C^)U. |
// /* 19e0 */ "\xce\x4d\x2a\x00\x1b\x0d\xa3\xe4\x23\x03\xfc\x20\x61\x23\xe2\x08" //.M*.....#.. a#.. |
// /* 19f0 */ "\x38\x2f\xc2\x0d\x83\x94\x2a\x71\xc1\x1d\x10\xf8\x86\x8f\xac\x40" //8/....*q.......@ |
// /* 1a00 */ "\xf8\x8c\x40\x1b\x28\x35\x8f\xb4\x94\x7d\x64\x03\xe2\x16\x3e\xb0" //..@.(5...}d...>. |
// /* 1a10 */ "\xc3\xeb\x02\x10\x6d\x1c\x1e\x63\x97\x46\x84\x1e\x0c\xee\xc4\x03" //....m..c.F...... |
// /* 1a20 */ "\x73\xe0\xc9\x98\x7d\x64\x23\xe2\x18\x03\x90\x41\xac\x40\x50\x50" //s...}d#....A.@PP |
// /* 1a30 */ "\x2c\x10\xa5\xbf\xd9\x07\xc0\x88\x16\x10\x6a\x1c\xd4\x1f\xa5\xee" //,.........j..... |
// /* 1a40 */ "\x94\x08\x00\x41\x06\x60\x1b\xfe\x5e\xdf\x78\xbd\xe7\xd5\x60\x4b" //...A.`..^.x...`K |
// /* 1a50 */ "\xc8\x71\x48\x6c\xfc\x40\x02\x0f\xad\x00\x06\x01\x9e\x8d\x7b\xd9" //.qHl.@........{. |
// /* 1a60 */ "\xf9\x81\x70\x12\x07\x01\x3f\x1a\x82\xc8\x44\xff\x34\x58\x02\xa0" //..p...?...D.4X.. |
// /* 1a70 */ "\x28\x34\x0e\x00\xef\x29\x82\xa0\x48\x36\x80\x00\x81\x3f\x45\x9a" //(4...)..H6...?E. |
// /* 1a80 */ "\xff\xd4\x14\x7c\xa3\x87\xee\x2c\x7c\x83\x07\xc8\x40\x3e\x41\x40" //...|...,|...@>A@ |
// /* 1a90 */ "\x2f\xd5\xaf\x90\x58\xf9\x0a\x87\xc8\x34\x78\x92\x44\x04\xbd\xe2" ///...X....4x.D... |
// /* 1aa0 */ "\xf1\x10\xe0\x47\xc2\x51\x58\x0c\xfb\x01\x1f\x88\x6d\xfb\x82\x08" //...G.QX.....m... |
// /* 1ab0 */ "\x80\x84\x08\x00\xe1\x06\xc1\xe6\x04\xe3\xcf\x06\x84\x1b\x03\x77" //...............w |
// /* 1ac0 */ "\xd6\x20\xd8\x02\xe0\x9d\x1a\x47\x04\x0f\x08\x35\x07\x16\x1b\x45" //. .....G...5...E |
// /* 1ad0 */ "\x25\x06\x91\xce\x3c\x83\x60\xb9\xa8\x80\xb0\xda\x38\x2b\xc2\x70" //%...<.`.....8+.p |
// /* 1ae0 */ "\x0f\xac\xf0\x4c\xe6\x1e\xa3\x7f\x28\xa4\x54\x00\xc0\x30\x29\xe8" //...L....(.T..0). |
// /* 1af0 */ "\xd8\x26\x80\x09\x1d\x09\x67\x61\xb4\x54\x00\xc8\x32\x54\x0a\xe3" //.&....ga.T..2T.. |
// /* 1b00 */ "\x92\x12\xc8\x36\x0c\x0e\xa4\xc2\x87\xe9\x67\xcc\x7d\x80\x11\x03" //...6......g.}... |
// /* 1b10 */ "\x40\x0c\x05\x1c\x00\x73\x0b\xee\x30\x1f\x58\x07\x0f\xc7\xba\x21" //@....s..0.X....! |
// /* 1b20 */ "\x92\x30\xfc\xc2\x02\xb4\x13\xf5\x44\x15\x00\x1e\x76\x0f\x92\x61" //.0......D...v..a |
// /* 1b30 */ "\x48\xc3\xf9\x40\x01\xc8\x27\xde\x60\x1c\xa4\x28\x00\x77\x04\xbd" //H..@..'.`..(.w.. |
// /* 1b40 */ "\xb2\x5a\xc1\x17\xe4\xc0\x05\x60\x07\x82\x7a\x39\xf9\xe7\x94\xd1" //.Z.....`..z9.... |
// /* 1b50 */ "\x63\xe4\xc0\x06\x01\x93\x09\x80\xfc\x98\xf1\x02\x5d\xe2\x08\xf2" //c...........]... |
// /* 1b60 */ "\x84\x34\xfc\x99\x38\x01\xe8\xac\xe1\x05\x0f\x61\x23\x68\x00\x60" //.4..8......a#h.` |
// /* 1b70 */ "\x3d\x12\x03\x7e\x6f\xaf\x30\x3e\x4b\xf8\xbc\xba\xc4\x00\x2f\x06" //=..~o.0>K...../. |
// /* 1b80 */ "\x19\xfe\x4c\x00\x5a\x00\x7a\x2c\xbe\x48\x81\x23\xc5\x30\xf4\x12" //..L.Z.z,.H.#.0.. |
// /* 1b90 */ "\x33\xe8\x68\x04\xc8\x12\x08\x08\xaf\xcc\x10\x46\x3e\x63\xff\x2a" //3.h........F>c.* |
// /* 1ba0 */ "\x00\x03\x35\x00\x52\x0a\xc0\x0f\x3c\xc0\xc7\x00\x05\x40\x14\x01" //..5.R...<....@.. |
// /* 1bb0 */ "\x03\xcc\x30\x31\x50\x06\x41\x80\x7a\x6e\x05\xf8\x60\xb9\xf6\x3a" //..01P.A.zn..`..: |
// /* 1bc0 */ "\x08\x05\x7e\xfc\x04\x10\x72\x98\x40\x44\xc2\x45\x20\x48\x34\x8f" //..~...r.@D.E H4. |
// /* 1bd0 */ "\xca\x08\xf0\x18\x02\x03\x4e\xe7\xf6\x41\xa0\x00\x0c\x74\x04\x40" //......N..A...t.@ |
// /* 1be0 */ "\x61\x20\x01\xfc\x07\xa2\x75\xc4\x47\x1c\x72\xee\x00\xa1\x47\x42" //a ....u.G.r...GB |
// /* 1bf0 */ "\x60\x2f\x61\x23\xac\x7f\x1c\x47\xd3\x10\xd3\xf1\x14\xef\x88\x13" //`/a#...G........ |
// /* 1c00 */ "\xe6\xb6\x58\x09\x1f\xe8\xae\x01\xc8\xa7\xa8\xef\x46\x67\x4f\xaa" //..X.........FgO. |
// /* 1c10 */ "\xa8\x7c\x41\xaf\xdc\x3d\x7d\xc4\x74\x27\xa8\xf3\xd2\x4e\xa5\xd5" //.|A..=}.t'...N.. |
// /* 1c20 */ "\xd4\xda\x0d\xfe\xd1\xf3\xed\x4f\xb4\x1e\x30\x78\x32\xba\x90\x47" //.......O..0x2..G |
// /* 1c30 */ "\x52\x87\xd6\x75\x34\x75\x3c\x0b\xd4\x61\x40\x1e\x95\x81\x58\x13" //R..u4u<..a@...X. |
// /* 1c40 */ "\x48\x02\xc0\x4d\x44\x5f\xa6\x5a\xe0\x00\x25\x03\x45\xea\x16\x5a" //H..MD_.Z..%.E..Z |
// /* 1c50 */ "\x09\x6a\xf2\x48\xd8\x24\x32\x0c\x03\x98\x7c\x8d\x00\x12\xe0\x01" //.j.H.$2...|..... |
// /* 1c60 */ "\x3e\xa5\xe6\xdf\xa8\x7c\x09\xa4\xac\x04\xf4\x8c\x3d\x04\xbe\x04" //>....|......=... |
// /* 1c70 */ "\xb0\x26\x47\x72\x04\xfc\x9a\xf0\xe8\xc4\xcf\x87\x43\xf2\x45\x00" //.&Gr........C.E. |
// /* 1c80 */ "\x03\xf5\xa4\x83\xd4\x56\x11\xff\x4b\x40\x84\x06\x52\x3a\x1d\xc2" //.....V..K@..R:.. |
// /* 1c90 */ "\xe0\x21\x01\x87\x0a\x07\x79\x0c\x21\x01\x89\x89\x87\x7a\x5c\x21" //.!....y.!....z.! |
// /* 1ca0 */ "\x01\x78\x0a\xfa\x13\xe9\xf2\x08\xf8\x6f\xa3\x4e\x3f\xdc\x8c\x3c" //.x.......o.N?..< |
// /* 1cb0 */ "\x1b\x01\x8e\x04\x88\xc0\x52\xc5\x43\x38\x2f\x1c\xbe\x91\x40\x50" //......R.C8/...@P |
// /* 1cc0 */ "\x33\x81\x80\x4a\x02\x6b\x8c\xa8\x10\xbf\x90\x3a\x0a\x0c\xf3\x51" //3..J.k.....:...Q |
// /* 1cd0 */ "\xcf\xed\x00\x87\xc2\xfd\x5a\x13\x80\x83\x8f\x05\x7e\xc3\xa7\xd2" //......Z.....~... |
// /* 1ce0 */ "\x06\x0a\x85\x7e\x40\xa4\x06\xe2\x26\x15\xf6\x82\x90\x1b\xb0\x97" //...~@...&....... |
// /* 1cf0 */ "\x84\x80\x0e\xbf\x69\x7b\x09\x09\xe1\x2e\x76\x8e\x84\xd4\x64\xbd" //....i{....v...d. |
// /* 1d00 */ "\x40\xdd\x52\x9d\x73\x00\xcd\xa0\x60\x30\x20\x2f\x57\x85\xea\x00" //@.R.s...`0 /W... |
// /* 1d10 */ "\xc0\xfe\xc0\x0a\x04\xbc\x7d\x60\x28\xe8\x3f\x0f\x48\x79\xf9\x60" //......}`(.?.Hy.` |
// /* 1d20 */ "\x42\xb1\x9f\x97\x0f\x51\xd0\x98\x2c\xf3\xcc\x86\x11\xf2\x20\x01" //B....Q..,..... . |
// /* 1d30 */ "\x50\x97\xcf\xb2\x01\x24\xf0\x29\xec\x25\xf7\xe5\xc0\x78\x58\x0b" //P....$.).%...xX. |
// /* 1d40 */ "\xf7\x88\x56\x81\x31\xef\x44\x90\x82\x87\xba\xee\x2c\x00\x03\xb0" //..V.1.D.....,... |
// /* 1d50 */ "\x48\xe0\x4e\x25\x62\x80\x01\xf1\xcf\x36\xb8\xdc\xc5\x00\x54\x6f" //H.N%b....6....To |
// /* 1d60 */ "\xd4\x6a\x80\x15\xc7\x81\xb1\x80\x50\x6f\xcc\xea\x87\xbd\x80\x30" //.j......Po.....0 |
// /* 1d70 */ "\x12\x0c\x5c\xb3\x38\x1a\xa5\xc0\x16\xf1\x11\xbc\x77\x72\x0f\x80" //....8.......wr.. |
// /* 1d80 */ "\x3b\x75\x97\x52\x60\x22\xc2\xdd\x5e\xd4\x9b\x00\x48\xb4\x06\x01" //;u.R`"..^...H... |
// /* 1d90 */ "\xd2\x09\xd2\x2a\x08\x07\xba\xab\x07\xf0\xd2\xba\x04\xc2\x3e\x81" //...*..........>. |
// /* 1da0 */ "\xd2\xa5\x20\xea\xd4\xb0\x09\xf4\x9e\x75\x5e\x8f\xb8\x61\x1f\x60" //.. ......u^..a.` |
// /* 1db0 */ "\xe1\x42\x90\x7d\xa0\x07\xf2\x20\x7f\x24\x83\xed\x2c\x1f\x6a\x72" //.B.}... .$..,.jr |
// /* 1dc0 */ "\x08\x82\x02\x01\xfc\x0c\x1f\x69\x20\xfb\x40\x0e\xb2\xc4\x01\xff" //.......i .@..... |
// /* 1dd0 */ "\xb5\x04\x1f\xc0\xc1\xf6\x94\x00\xbe\xc7\x4e\xc2\x8f\x2a\xc7\x51" //..........N..*.Q |
// /* 1de0 */ "\xfd\xfc\xab\x0c\x14\x58\x08\xb4\x1c\x00\x29\x00\x2a\x40\x14\xc0" //.....X....).*@.. |
// /* 1df0 */ "\x16\x80\x29\x20\xea\xf8\x14\xea\x48\x81\xc8\x00\x40\x05\x28\x03" //..) ....H...@.(. |
// /* 1e00 */ "\xc3\xbe\xa9\x75\xa5\x48\x02\xa0\x1f\x2f\x0b\x00\x5b\x00\xff\xcb" //...u.H.../..[... |
// /* 1e10 */ "\x88\x16\x03\x94\x0f\x8f\x7a\x70\xc1\xc4\xf9\xf2\x20\x00\x2b\xb0" //......zp.... .+. |
// /* 1e20 */ "\xc5\x58\x02\xfa\xdc\x00\x20\x03\xdf\x32\x3d\x14\xf9\x8a\x07\xcc" //.X.... ..2=..... |
// /* 1e30 */ "\x90\x0f\x2f\x28\x1d\x90\x1d\x9c\x2a\x81\xe8\x23\x07\xd2\x76\x8a" //../(....*..#..v. |
// /* 1e40 */ "\x8e\xcb\x53\xf1\x1d\xa4\xa3\xf9\x38\x00\xbf\x03\xe2\x3d\x81\x1e" //..S.....8....=.. |
// /* 1e50 */ "\x1c\x7a\x80\xd0\xc2\x3e\x2f\x50\x08\x90\x0f\xe2\x39\x2a\xc4\x88" //.z...>/P....9*.. |
// /* 1e60 */ "\x4f\x43\x5c\x9b\xc2\xea\x5f\x62\x89\x20\x01\x13\x6c\xeb\xda\xed" //OC...._b. ..l... |
// /* 1e70 */ "\x8e\xf5\xdb\x80\xcb\xe9\xa8\x1c\x04\x67\xda\xae\xbb\x0e\xbd\x2f" //.........g...../ |
// /* 1e80 */ "\x98\xf0\x8b\xe6\x08\x3e\x64\xf8\xf5\xf9\x1d\x85\x1f\x31\xd8\x2e" //.....>d......1.. |
// /* 1e90 */ "\x3c\xa2\xfb\x8f\xe9\x5f\x42\x38\x11\xb8\x2a\x90\x3e\x84\xf9\xdd" //<...._B8..*.>... |
// /* 1ea0 */ "\xc1\xb6\x7b\x77\xcf\x03\x77\x81\x1f\x62\x7b\x9f\xbc\xba\xf1\xb3" //..{w..w..b{..... |
// /* 1eb0 */ "\xe2\x3f\xa4\xfc\x47\xaf\xdc\x18\xe7\xb9\x9c\xfc\xc7\xd4\x84\x00" //.?..G........... |
// /* 1ec0 */ "\xbe\x1a\x73\x80\xc2\x9e\x78\xba\xf7\x9e\x00\xdd\x23\x1d\x75\xe2" //..s...x.....#.u. |
// /* 1ed0 */ "\xa0\x0a\x83\x40\x01\x06\xe9\xf6\x13\xe6\xdc\x0b\xea\xe8\x74\xe8" //...@..........t. |
// /* 1ee0 */ "\xb2\x00\x28\xc0\x0a\xae\xa6\x6e\x99\xae\x9b\xb1\x30\xf9\x07\x8e" //..(....n....0... |
// /* 1ef0 */ "\x9d\xa8\x47\xe4\x4a\x3e\x41\x83\xe4\x2a\x1f\xb0\x71\xfb\x24\x40" //..G.J>A..*..q.$@ |
// /* 1f00 */ "\x2b\x01\x01\x05\x3e\x73\xfa\x4f\x50\x18\xea\x32\x41\x90\x7c\x84" //+...>s.OP..2A.|. |
// /* 1f10 */ "\x23\xf6\x1e\x3f\x63\xaa\xfc\x28\x7c\x85\x63\xfa\x0a\x3f\xa4\x8c" //#..?c..(|.c..?.. |
// /* 1f20 */ "\x7e\xc9\xb0\x75\x73\x24\x7e\xc5\x03\xee\x45\x09\x06\xa0\x13\xcf" //~..us$~...E..... |
// /* 1f30 */ "\x94\xec\xc3\x68\xf8\xe7\x0f\x4d\x3b\x0e\x65\xd8\x0a\x3b\x0e\x47" //...h...M;.e..;.G |
// /* 1f40 */ "\x60\x29\xfa\xc3\x8f\x88\xec\xd1\x55\x0f\xb4\xec\xd1\xa1\x47\xd6" //`)......U.....G. |
// /* 1f50 */ "\x76\x68\x09\xf6\x26\x89\x28\x10\x1d\xbb\xd6\x78\xdb\x3d\x66\x10" //vh..&.(....x.=f. |
// /* 1f60 */ "\xf0\xc2\x9e\xc3\x68\x10\xf2\x8b\x86\x93\xaf\x33\x84\xe1\x01\xc1" //....h......3.... |
// /* 1f70 */ "\x86\xd0\x1f\xe1\xa9\x41\xa4\x7c\xf1\x87\x57\x90\x77\xcf\x7e\x09" //.....A.|..W.w.~. |
// /* 1f80 */ "\xac\x77\x9e\x34\x39\x76\xfa\x76\x4d\x8f\x9a\x41\x8c\x39\xb0\xda" //.w.49v.vM..A.9.. |
// /* 1f90 */ "\x3d\x16\x78\xf4\x18\x27\xea\x44\x81\xd5\x86\xd0\x0d\x04\x00\x18" //=.x..'.D........ |
// /* 1fa0 */ "\x20\x48\x20\x88\x20\xd8\x3c\xf4\xa2\x20\xd4\x20\x48\x35\x84\x1b" // H . .<.. . H5.. |
// /* 1fb0 */ "\x43\xcb\x0d\xa2\x8a\x40\x70\xf7\xea\xa1\x02\xc0\xf4\x09\x07\x87" //C....@p......... |
// /* 1fc0 */ "\x90\x74\x87\xd6\xa1\xd8\x12\x0d\x80\x00\xc3\x8b\xf0\x6f\xe7\x0c" //.t...........o.. |
// /* 1fd0 */ "\x31\x32\x48\x66\x06\x83\x68\xc0\x58\x01\x8c\x05\xe6\x17\x81\xe0" //12Hf..h.X....... |
// /* 1fe0 */ "\x30\xc4\x00\x00\xa3\x00\x25\x1e\x18\x6d\x18\x0c\x64\x83\x30\x20" //0.....%..m..d.0  |
// /* 1ff0 */ "\x78\x00\x40\x1c\x64\x61\xb4\x7d\xa8\x84\x00\x10\x0e\xb1\x60\x2e" //x.@.da.}......`. |
// /* 2000 */ "\xe2\x0d\x80\x82\x20\xc0\xc1\xe1\x92\x0c\xa2\x90\x40\x91\x04\x41" //.... .......@..A |
// /* 2010 */ "\x94\x20\xce\x30\x19\xc2\xd1\xc0\x25\xc0\xfe\x20\x98\x0f\xd8\x81" //. .0....%.. .... |
// /* 2020 */ "\xd9\x02\x04\x38\x58\x80"                                         //...8X. |
// Sent dumped on RDP Client (5) 8230 bytes |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[1](30) used=378 free=16156 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(802,677,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((834, 677, 32, 32) (96, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[2](2064) used=381 free=16153 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[3](30) used=546 free=15988 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(834,677,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((866, 677, 32, 32) (128, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[4](2064) used=549 free=15985 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[5](30) used=826 free=15708 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(866,677,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((898, 677, 32, 32) (160, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[6](2064) used=829 free=15705 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[7](30) used=935 free=15599 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(898,677,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((930, 677, 32, 32) (192, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[8](2064) used=938 free=15596 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[9](30) used=1063 free=15471 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(930,677,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((962, 677, 32, 32) (224, 64, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[10](2064) used=1066 free=15468 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[11](30) used=1167 free=15367 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(962,677,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((738, 709, 32, 29) (0, 96, 32, 29) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[12](1872) used=1170 free=15364 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[13](30) used=1350 free=15184 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(738,709,32,29) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((770, 709, 32, 29) (32, 96, 32, 29) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[14](1872) used=1358 free=15176 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[15](30) used=1673 free=14861 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(770,709,32,29) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((802, 709, 32, 29) (64, 96, 32, 29) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[16](1872) used=1676 free=14858 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[17](30) used=1872 free=14662 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(802,709,32,29) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((834, 709, 32, 29) (96, 96, 32, 29) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[18](1872) used=1875 free=14659 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[19](30) used=2495 free=14039 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(834,709,32,29) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((866, 709, 32, 29) (128, 96, 32, 29) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[20](1872) used=2498 free=14036 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[21](30) used=3002 free=13532 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(866,709,32,29) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((898, 709, 32, 29) (160, 96, 32, 29) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[22](1872) used=3005 free=13529 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[23](30) used=3535 free=12999 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(898,709,32,29) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((930, 709, 32, 29) (192, 96, 32, 29) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[24](1872) used=3538 free=12996 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[25](30) used=4247 free=12287 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(930,709,32,29) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((962, 709, 32, 29) (224, 96, 32, 29) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[26](1872) used=4250 free=12284 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[27](30) used=4434 free=12100 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(962,709,32,29) rop=cc srcx=0 srcy=0 cache_idx=0) |
// front::draw:draw_tile((0, 704, 32, 32) (32, 32, 32, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[28](2064) used=4437 free=12097 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[29](30) used=4459 free=12075 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(0,704,32,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// Widget_load: image file [./tests/fixtures/ad8b.png] is PNG file |
// front::draw:draw_tile((100, 100, 26, 32) (80, 50, 26, 32) |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[30](1808) used=4467 free=12067 |
// /* BMP Cache compressed V1*/ |
// <Serializer 0x7f9f3998c010> RDPSerializer::reserve_order[31](30) used=4782 free=11752 |
// order(13 clip(145,200,110,1)):memblt(cache_id=1 rect(100,100,26,32) rop=cc srcx=0 srcy=0 cache_idx=0) |
// Front::end_update() |
// GraphicsUpdatePDU::flush_orders: order_count=32 offset=0 |
// GraphicsUpdatePDU::flush_orders: fast-path |
// Sending on RDP Client (5) 3648 bytes |
// /* 0000 */ "\x00\x8e\x40\x80\x21\x39\x0e\x20\x00\x03\x6b\xe8\x17\x8e\x25\xea" //..@.!9. ..k...%. |
// /* 0010 */ "\x8a\x27\x80\x22\x71\x0b\x00\xa0\x03\xf3\x27\x01\x34\x00\x00\x82" //.'."q.....'.4... |
// /* 0020 */ "\x40\x88\x1e\x3a\x3c\xaa\x00\xd4\x50\x0e\xae\x43\x8e\xa7\x8a\x80" //@..:<...P..C.... |
// /* 0030 */ "\x2a\xff\x30\x32\x0d\x83\xcc\x8e\x81\x41\x6a\x80\x2d\x14\x27\x1e" //*.02.....Aj.-.'. |
// /* 0040 */ "\x49\x00\x3b\x0b\x04\xfe\x74\x25\x80\x7c\xe3\x0d\x00\x07\xa6\x0b" //I.;...t%.|...... |
// /* 0050 */ "\x9d\x84\x02\x12\x7d\x08\x20\xeb\x00\x84\xff\xb0\x1c\xb7\xfd\xc5" //....}. ......... |
// /* 0060 */ "\x43\xf7\x8a\x3a\xfd\x47\xd1\x23\x7d\xf0\x1b\x0a\xc7\x87\x00\x0f" //C..:.G.#}....... |
// /* 0070 */ "\x9f\xfc\x07\x5b\xf0\x68\x10\x82\x91\xd8\x2e\x05\xb0\x4a\x03\xf7" //...[.h.......J.. |
// /* 0080 */ "\xc5\x63\xae\xab\xc3\xcb\x97\x84\x27\x04\x31\xb9\xc0\x22\x1f\x4d" //.c......'.1..".M |
// /* 0090 */ "\xd7\xa1\x2d\xf0\x10\x05\x1f\xc0\x40\x52\x09\x47\x42\x18\x17\x81" //..-.....@R.GB... |
// /* 00a0 */ "\x42\xf0\x01\xe3\xaf\x7b\xb2\xc0\x8b\xaf\x32\xe2\x01\x50\x06\x61" //B....{....2..P.a |
// /* 00b0 */ "\x88\x78\x72\x1e\xf9\xc6\x8d\x82\x6a\x24\x0b\x1d\xc2\xa0\x5a\xa0" //.xr.....j$....Z. |
// /* 00c0 */ "\x11\xe1\x90\x07\x3e\x57\x84\x81\x1c\xf5\x9d\x57\x23\x01\xec\x67" //....>W.....W#..g |
// /* 00d0 */ "\x80\x30\x98\x0f\x5a\x21\x5f\x10\x8a\x01\xfb\x5d\xc9\x5c\xc3\x5d" //.0..Z!_....]...] |
// /* 00e0 */ "\xb5\xdd\x8b\x95\x8e\xc5\xc5\x0f\xf8\x58\x09\x15\xe0\x58\xa8\x90" //.........X...X.. |
// /* 00f0 */ "\x42\x3c\xb3\x02\xfd\xd0\x20\x19\x06\x31\xe5\x28\x18\xf9\x90\x0a" //B<.... ..1.(.... |
// /* 0100 */ "\x41\xf3\x0d\x18\x3b\x5d\xf3\x00\x9d\xc7\xa0\x8c\x00\x6a\x02\x83" //A...;].......j.. |
// /* 0110 */ "\xdd\xf1\xe5\xa7\x82\x8f\x61\x22\x9c\x0f\x40\x78\x0f\x85\xc0\xee" //......a"..@x.... |
// /* 0120 */ "\xf1\xa4\x50\x00\x4e\x06\xce\x01\x72\xf6\x2a\x95\x1d\x09\xc0\x32" //..P.N...r.*....2 |
// /* 0130 */ "\xa8\x1e\x04\x16\x7e\x87\x38\x89\x22\xfb\x01\xef\x93\x08\xdd\x7d" //....~.8."......} |
// /* 0140 */ "\x9c\xe2\x60\x07\xe8\xe1\x0b\xfe\x0f\xa3\x0d\x1f\x22\xd9\xff\x9b" //..`........."... |
// /* 0150 */ "\x07\x26\x3b\x2d\x98\x30\x46\xa3\xc6\xcc\x09\x99\x5d\x99\xa2\xe7" //.&;-.0F.....]... |
// /* 0160 */ "\xf1\x1d\xa6\x1d\x6a\x5d\x94\x23\xb4\xb4\x7a\x82\x3d\x51\x1d\x99" //....j].#..z.=Q.. |
// /* 0170 */ "\xa1\x6b\xeb\x48\x17\xce\x76\x96\x89\x67\xc4\x5c\x7e\x20\xd8\x27" //.k.H..v..g..~ .' |
// /* 0180 */ "\x2e\x6a\x4f\x70\xf7\x9a\x2c\x1b\x87\x1b\x00\x24\x44\x05\x3c\x53" //.jOp..,....$D.<S |
// /* 0190 */ "\x9c\xa8\x52\x02\xa9\xd3\x10\xb3\xd0\x09\xdb\x45\xcf\xc8\x4f\xd2" //..R........E..O. |
// /* 01a0 */ "\x68\x19\xcd\x04\x23\xcc\x64\x2d\xd7\x39\x1c\xeb\x9c\xbd\x84\xf5" //h...#.d-.9...... |
// /* 01b0 */ "\xde\x8e\xb9\xd1\x63\x1e\xe3\xa2\xf4\xa8\x52\x03\x48\x1d\xc0\x50" //....c.....R.H..P |
// /* 01c0 */ "\xac\x1a\x48\x13\x80\xd3\x45\x02\xe0\x48\x9e\x7c\x09\x40\x66\x42" //..H...E..H.|.@fB |
// /* 01d0 */ "\x61\x7f\x51\x2d\x00\x12\x80\xcd\xc2\xb9\x06\xb8\x23\x84\x60\x33" //a.Q-........#.`3 |
// /* 01e0 */ "\xc0\x90\xdf\x32\xbb\x28\x44\x14\x7b\xea\x42\xc7\xd4\x42\x87\x43" //...2.(D.{.B..B.C |
// /* 01f0 */ "\x7c\x20\x11\x80\xb7\x8a\x06\xc0\x80\x7d\x5c\x10\x80\xba\x09\xe0" //| .......}...... |
// /* 0200 */ "\x2e\xb6\x54\xee\x08\x40\x5e\x7b\x4e\x11\x40\x3b\x49\x41\x10\xff" //..T..@^{N.@;IA.. |
// /* 0210 */ "\xd4\x87\xaf\xbc\x46\x1e\x0e\xf4\x58\x42\x02\x56\x2a\x1d\xde\x8c" //....F...XB.V*... |
// /* 0220 */ "\x21\x01\x30\xed\xa0\xc0\x1e\xc8\x08\xa8\x98\x77\xe2\x2c\x84\x87" //!.0........w.,.. |
// /* 0230 */ "\xc0\x46\x82\x83\xfd\x86\x70\xfe\xcb\x1f\xec\x53\xbe\xe2\xd1\xdb" //.F....p....S.... |
// /* 0240 */ "\xf1\x0a\xf9\x0b\x6f\xec\x23\xfc\x85\x4b\xb7\xe0\x8f\xe4\x2a\xfe" //....o.#..K....*. |
// /* 0250 */ "\x20\x72\xe3\x54\x01\x83\x00\xfa\xb6\x4e\x33\xb7\xda\x9f\xd2\x75" // r.T.....N3....u |
// /* 0260 */ "\x40\x0b\x0e\x81\x62\x62\xd8\x0f\xc0\x98\x8c\x04\xc4\x0f\x80\xe0" //@...bb.......... |
// /* 0270 */ "\x48\x37\x44\xc7\x46\x4a\xc0\x7c\x51\x40\xe8\x90\xea\x4f\x0c\x1e" //H7D.FJ.|Q@...O.. |
// /* 0280 */ "\xe7\x48\x3e\xe3\xa6\x53\x81\x43\xa3\xf4\x75\x13\x8e\x96\x4e\x92" //.H>..S.C..u...N. |
// /* 0290 */ "\xd1\xd4\x4d\xf6\xa0\x80\x52\x9b\x60\x13\xb9\x80\x60\x46\x67\x8b" //..M...R.`...`Fg. |
// /* 02a0 */ "\x63\xc1\xef\x16\xde\x03\xfc\x00\x50\xd6\x69\xd0\x1b\x62\xd0\xe5" //c.......P.i..b.. |
// /* 02b0 */ "\xce\x81\xc1\x45\x44\xb0\x09\xc2\xac\x01\x74\x00\xb9\x00\x5c\x00" //...ED.....t..... |
// /* 02c0 */ "\x2c\x80\x2e\x80\x14\xc0\x15\x40\x09\x20\xc0\x8e\x41\x26\xe6\x11" //,......@. ..A&.. |
// /* 02d0 */ "\xf8\x30\x00\xf9\x06\x04\x9c\x75\xe9\x05\x20\x00\x8d\x33\x30\x8f" //.0.....u.. ..30. |
// /* 02e0 */ "\xc1\x08\x07\xe0\x02\xb2\x00\xb9\x00\x0f\x2f\x60\xfc\x0e\x80\x3c" //........../`...< |
// /* 02f0 */ "\xc0\x29\x00\x01\x1d\x63\xf9\x6b\x00\xaf\xe3\x00\x73\x01\x7c\xb0" //.)...c.k....s.|. |
// /* 0300 */ "\xf3\x00\x59\x00\x06\x05\x95\x75\x0b\x62\x5a\xc7\x90\x2b\x13\x37" //..Y....u.bZ..+.7 |
// /* 0310 */ "\x6d\xc0\xc5\x00\x02\xec\xfb\x10\x3e\xb0\x01\x50\x00\x11\xef\x90" //m.......>..P.... |
// /* 0320 */ "\x00\x1f\x80\x05\x0f\x78\x01\x02\xc0\x9f\x60\x0f\xa3\x16\x01\x58" //.....x....`....X |
// /* 0330 */ "\x07\xc8\x92\x01\xec\x09\x3a\xf9\x06\x01\xec\x09\x7a\xf9\x08\x00" //......:.....z... |
// /* 0340 */ "\xf7\xd8\xa9\x00\xaf\x56\x5a\xeb\x83\x1f\x98\x17\xc7\x56\x70\x32" //.....VZ......Vp2 |
// /* 0350 */ "\xb6\x03\xb8\x19\x5a\x7c\x50\x43\xe5\x61\x1d\x51\xe3\xa5\xd3\xa0" //....Z|PC.a.Q.... |
// /* 0360 */ "\xd1\xa3\xa6\x4a\x81\xd3\x95\xd5\x37\xc4\xc0\x6f\x44\x19\x07\xc8" //...J....7..oD... |
// /* 0370 */ "\x60\x38\xa7\x54\xd9\x30\xfd\x92\x11\xd5\x37\x5a\x47\xec\x75\x4d" //`8.T.0....7ZG.uM |
// /* 0380 */ "\xf3\x10\x3f\x63\xaa\x6b\x01\xf1\x4c\x40\x4f\x99\x62\xd0\x02\x82" //..?c.k..L@O.b... |
// /* 0390 */ "\x2d\x62\x02\xa2\x81\x0e\x80\x01\x97\x71\xe9\xb1\xd1\x02\xce\x0b" //-b.......q...... |
// /* 03a0 */ "\x0f\x82\xac\x41\xe0\x58\xe5\xe9\x6b\x5a\x2c\x63\xa5\xad\x5a\x0e" //...A.X..kZ,c..Z. |
// /* 03b0 */ "\x68\x63\x02\xf9\x65\xe1\x16\x9a\xb3\xd7\xf0\xd4\x7c\x0a\xc0\x8a" //hc..e.......|... |
// /* 03c0 */ "\x10\xa3\xd7\x76\x9a\xfe\x18\xbc\x48\x84\x1f\x6e\xed\x5f\x25\x87" //...v....H..n._%. |
// /* 03d0 */ "\x6b\x95\x00\x39\x10\x40\x0c\x1e\xf7\x7e\xb5\xda\x89\x2d\x36\x3b" //k..9.@...~...-6; |
// /* 03e0 */ "\x44\x1a\xd3\x6b\xb5\x0a\x5d\x45\x2a\x00\x89\x39\x5c\x29\x06\xb4" //D..k..]E*..9.).. |
// /* 03f0 */ "\xb3\x30\xda\x05\x2a\x00\x64\xf8\x81\xf9\x1a\xbf\x60\x3c\xb7\xcc" //.0..*.d.....`<.. |
// /* 0400 */ "\x48\x79\xe3\x1e\xae\xd4\x74\x80\x2f\x99\x00\x00\x2d\x15\xfc\x3e" //Hy....t./...-..> |
// /* 0410 */ "\x4c\x00\x30\x79\xbb\x0f\x40\x88\x1c\x22\x57\x4e\xaa\x09\x85\x40" //L.0y..@.."WN...@ |
// /* 0420 */ "\x12\x35\x81\xdb\x97\x3b\xa5\x16\x7c\x2f\x01\x87\x5c\xc4\xa0\x7d" //.5...;..|/.....} |
// /* 0430 */ "\x5d\x56\x0e\x6a\xc0\x3b\x3e\x01\x3d\x4f\xaf\xf5\x00\x40\x4c\x02" //]V.j.;>.=O...@L. |
// /* 0440 */ "\x2c\x10\xba\x09\x7a\x65\xce\x41\x01\xb3\xda\x70\xa4\xd3\xa5\x52" //,...ze.A...p...R |
// /* 0450 */ "\xbf\x7f\x9d\x5f\xaa\x00\x56\x70\x10\x18\x10\x25\xa6\xf9\x7e\x12" //..._..Vp...%..~. |
// /* 0460 */ "\xe9\x04\xe9\x24\xfe\xf0\x22\x18\x02\xc5\x5f\xbf\xc1\x0e\x6e\xd5" //...$.."..._...n. |
// /* 0470 */ "\x7b\xab\xed\xfe\xbf\x7f\x12\xe8\xfa\xe9\x38\xfb\x40\x43\x00\xc0" //{.........8.@C.. |
// /* 0480 */ "\x16\x4a\xfe\x4a\x9e\x00\x0c\x79\x9d\x02\x49\xb5\x46\xaf\xdd\x3d" //.J.J...y..I.F..= |
// /* 0490 */ "\x9d\x2e\x5e\x17\x41\x8a\x6d\x4e\x37\x49\xb5\xd4\xe6\xed\x77\xba" //..^.A.mN7I....w. |
// /* 04a0 */ "\xd4\xee\x82\x35\x00\x8e\x9b\xe0\x0f\xca\x11\xe0\x44\xec\x23\x0d" //...5........D.#. |
// /* 04b0 */ "\xf8\x12\xf6\xb2\x58\xae\x03\xb9\x2c\x56\x7e\x0a\x43\xc8\xf9\x82" //....X...,V~.C... |
// /* 04c0 */ "\x40\x62\x85\x1c\xea\x98\x7c\x09\x03\xe8\xc4\xc0\x36\xc8\x1c\xcb" //@b....|.....6... |
// /* 04d0 */ "\x14\x4f\x96\xb9\xed\x38\x03\x15\x8c\x87\xe0\x31\x69\x1e\x04\x79" //.O...8.....1i..y |
// /* 04e0 */ "\x7a\xf1\x01\x9b\x53\x07\x60\x72\x2b\x43\x6d\xce\xeb\x64\xe3\x0c" //z...S.`r+Cm..d.. |
// /* 04f0 */ "\xb4\x40\x98\xac\x18\x12\x20\xc6\x53\x6a\x80\xce\x96\x87\x28\x77" //.@.... .Sj....(w |
// /* 0500 */ "\x61\x58\x0c\x7e\xef\x88\xb8\xa1\x74\xa8\x7e\x02\x3a\x86\x3f\x01" //aX.~....t.~.:.?. |
// /* 0510 */ "\x3d\x4b\xe7\x96\x3a\xcb\x6d\x0b\xfa\xbe\x2d\x5f\x44\xae\x82\x4c" //=K..:.m...-_D..L |
// /* 0520 */ "\x23\xb0\x3a\xdd\xfa\x87\x68\xb6\xc2\x07\x20\x8b\x37\x62\x4b\x4b" //#.:...h... .7bKK |
// /* 0530 */ "\x0d\xa2\x51\x30\x1b\x80\xca\xad\x4c\x7e\xd5\x31\xd4\x06\xd9\x7e" //..Q0....L~.1...~ |
// /* 0540 */ "\x39\xe8\x44\x5b\x9c\x9a\xa3\x47\xc2\x06\x23\x51\x91\x6c\x04\x33" //9.D[...G..#Q.l.3 |
// /* 0550 */ "\x00\xcb\x41\x20\xf8\x17\xf1\x50\x08\xea\x10\xa8\x01\x11\x00\x72" //..A ...P.......r |
// /* 0560 */ "\xd6\x95\xf0\x2f\x43\xe0\xbb\x82\x4a\x5f\x05\x20\xc1\x31\x87\xa0" //.../C...J_. .1.. |
// /* 0570 */ "\x9a\x12\xc8\x4c\x37\x04\xfe\x5e\x09\x62\x5c\x00\x13\xc4\x66\x29" //...L7..^.b....f) |
// /* 0580 */ "\x4a\x4c\x6e\x2c\x76\xe5\xa0\x93\xaa\x18\xc0\x7b\x44\x74\x26\x05" //JLn,v......{Dt&. |
// /* 0590 */ "\x1d\x32\xbf\x70\xdc\x88\x3d\xfa\xbc\x37\x1d\x89\xa1\x42\xf0\x50" //.2.p..=..7...B.P |
// /* 05a0 */ "\xd8\x04\x02\x44\x21\x44\xf5\x40\x52\x4a\x15\x02\x5d\x4b\x8f\xdd" //...D!D.@RJ..]K.. |
// /* 05b0 */ "\xda\xfe\x62\x7a\x74\xc5\xcd\xb1\x82\x82\xc0\x70\x1c\x0a\xb4\x7a" //..bzt......p...z |
// /* 05c0 */ "\xa7\x04\xc0\x87\x48\x9f\xed\xed\x95\xfc\x2f\x30\x63\x59\x01\x80" //....H...../0cY.. |
// /* 05d0 */ "\xf9\x28\x06\xc3\xf2\x13\xfb\xbd\x40\x22\x0f\xe0\x14\x06\xbd\x0c" //.(......@"...... |
// /* 05e0 */ "\xa8\x01\xc8\x04\x00\x61\x23\xf5\xf9\xf2\xd9\xe9\x10\xe8\xd4\x74" //.....a#........t |
// /* 05f0 */ "\xca\x7c\xb6\xfd\x25\x8f\xdb\xff\x88\x0c\x82\xdf\xd8\xf8\x42\x56" //.|..%.........BV |
// /* 0600 */ "\x03\x03\xaf\xdf\xe7\xa6\x5e\x91\x8a\x7f\x60\xd0\x23\x0e\x11\x3b" //......^...`.#..; |
// /* 0610 */ "\x47\x4c\x99\xbc\xa4\xae\x88\x8f\x36\x7f\xe0\x8f\xd5\x58\x2e\xc0" //GL......6....X.. |
// /* 0620 */ "\x30\x1a\xf3\x1f\x99\xdf\xd9\xc5\x05\x3f\xab\x60\x45\xc5\x40\x10" //0........?.`E.@. |
// /* 0630 */ "\x8d\x64\x07\xaf\x02\xde\x02\x01\x50\x03\x40\xc0\xc9\x69\xfd\x3a" //.d......P.@..i.: |
// /* 0640 */ "\x95\x00\x4a\xa8\x01\x20\x19\xff\x10\x01\x5f\xdb\x10\x1c\xfd\xbb" //..J.. ...._..... |
// /* 0650 */ "\xf3\xd2\xa0\x04\x85\x4d\xf0\xcb\xc0\x35\x50\x08\xb0\xb5\x79\x75" //.....M...5P...yu |
// /* 0660 */ "\x3f\xa7\x37\xfd\x98\xa0\x11\xf9\x22\x1a\xf2\xd6\xf4\x54\x64\x05" //?.7....."....Td. |
// /* 0670 */ "\x60\x30\x11\xe8\xa8\x7f\x81\x8d\x3e\x06\x30\x18\x80\xd4\x6c\x68" //`0......>.0...lh |
// /* 0680 */ "\x3f\x25\x63\xab\x80\xc0\x5f\xa3\xb4\x54\x2b\xb5\x40\x12\xc3\x0b" //?%c..._..T+.@... |
// /* 0690 */ "\xee\x27\x01\x1f\x54\x01\x5f\x01\xa8\xc0\x64\xbe\x15\xa9\x01\xcf" //.'..T._...d..... |
// /* 06a0 */ "\x90\x17\x7f\x34\xbc\x18\x04\x80\x5c\x3f\x1a\x40\xf3\xf2\x84\x6d" //...4.....?.@...m |
// /* 06b0 */ "\x3e\x16\x92\x3e\x40\x2a\xe9\xd5\x40\x0d\x80\x20\x26\x03\x5f\xab" //>..>@*..@.. &._. |
// /* 06c0 */ "\x65\xa6\xa1\xd8\x0c\x04\xf4\x8b\x80\x21\x6c\x06\x02\xff\x6f\xec" //e........!l...o. |
// /* 06d0 */ "\xb2\x3b\xf4\x71\x80\x20\x06\xa2\x03\x2c\xf0\x7d\x58\x60\x7c\x6a" //.;.q. ...,.}X`|j |
// /* 06e0 */ "\x62\x06\xff\x46\x1e\x9f\x7e\x08\xcb\x53\x4f\x6c\x9e\xbe\x28\x00" //b..F..~..SOl..(. |
// /* 06f0 */ "\xe7\xcf\xaf\xd4\x00\xfc\xf0\x08\x83\x03\x01\x66\xe0\xaa\xbf\xd7" //...........f.... |
// /* 0700 */ "\xbc\xc4\x86\x7d\xe7\xa1\x61\xfa\x4b\x81\x63\x20\xf5\x2e\xa2\x00" //...}..a.K.c .... |
// /* 0710 */ "\x52\x03\x02\x8f\xd3\xbc\x4e\x04\x60\x10\xfb\x8b\x05\x67\x09\xab" //R.....N.`....g.. |
// /* 0720 */ "\x94\xdb\x23\xa1\x30\x71\xd4\x3b\xf4\x30\x7a\x6c\xff\x46\x3f\x9f" //..#.0q.;.0zl.F?. |
// /* 0730 */ "\x2f\x47\x5f\x54\xd9\xff\x71\x74\xaa\x89\x41\x89\xf0\x36\x30\x60" ///G_T..qt..A..60` |
// /* 0740 */ "\x58\x85\x16\x8e\x01\x03\x7a\xc1\x85\xe7\xc0\x91\x8c\x17\x96\x83" //X.....z......... |
// /* 0750 */ "\x01\x60\xee\x63\x4f\x5d\xb5\xc7\xae\x08\x6b\x00\x5e\x20\x80\x3d" //.`.cO]....k.^ .= |
// /* 0760 */ "\xac\x17\x0e\xb3\xc0\xde\x25\xf1\x6b\xe6\x69\x8d\xa7\x99\xa5\x28" //......%.k.i....( |
// /* 0770 */ "\x74\x76\x75\x4c\x60\x18\xa7\x50\xa6\x03\x65\x88\x00\x50\x86\x30" //tvuL`..P..e..P.0 |
// /* 0780 */ "\x87\x71\x07\x54\x32\x3b\xa4\x63\x55\x17\xcb\x40\x8d\x64\xb9\x8a" //.q.T2;.cU..@.d.. |
// /* 0790 */ "\xa1\x50\x04\xbf\xdf\xf8\x1c\xb1\x2b\xac\x00\x70\x67\x40\xb0\xd4" //.P......+..pg@.. |
// /* 07a0 */ "\x7b\x18\xe3\xb0\x4a\xc0\x3e\xa1\x85\x2d\x9e\x72\x4e\xaa\xdf\x7d" //{...J.>..-.rN..} |
// /* 07b0 */ "\xf2\x90\x39\xfd\xdf\x05\x6d\xba\xef\x89\x00\x2a\x01\xac\xa5\x24" //..9...m....*...$ |
// /* 07c0 */ "\x20\x9c\x3c\x59\x6e\x94\x28\x1d\xda\xe8\x2b\xee\xd5\x08\x63\xf6" // .<Yn.(...+...c. |
// /* 07d0 */ "\xb2\x1b\x02\xa0\x09\x3f\xbb\xe1\x4d\x72\xd3\x00\x58\x7f\xb7\xa0" //.....?..Mr..X... |
// /* 07e0 */ "\x0c\x0d\x47\x81\xec\x54\x00\xf2\x18\xc2\xa0\x09\x6a\x5a\x01\x81" //..G..T......jZ.. |
// /* 07f0 */ "\x18\xc1\x08\x02\xbc\x6e\xb0\x1d\x78\x61\x67\xe0\x12\x04\xc2\x6d" //.....n..xag....m |
// /* 0800 */ "\x4c\x28\x90\xc4\x9f\x6e\xfd\x0b\x78\x0c\x06\x2c\xc7\x81\x58\xab" //L(...n..x..,..X. |
// /* 0810 */ "\xc9\xf0\x46\x43\x87\x37\xa3\xd2\x4f\x27\xc3\x1a\x88\x0c\x09\x7a" //..FC.7..O'.....z |
// /* 0820 */ "\x3b\xc6\xd8\x04\xcf\xc5\x8c\x06\xd4\xd7\xc5\xd2\x18\x42\x24\x02" //;............B$. |
// /* 0830 */ "\xa1\xf0\x19\xe8\x8f\x09\x78\x01\x80\xdf\xea\xdc\x76\x55\x00\x1e" //......x.....vU.. |
// /* 0840 */ "\x30\x73\x26\xf0\xbb\x10\x7c\x05\x3c\x8f\x48\x1c\x12\x15\xfb\xfd" //0s&...|.<.H..... |
// /* 0850 */ "\xf7\x8f\xcc\x9b\x70\x75\x1e\xd7\xc0\x60\x58\xf0\xbc\x17\xe3\x55" //....pu...`X....U |
// /* 0860 */ "\x83\x9d\x19\xff\x0a\xa5\xbe\xda\xa0\x09\x51\xc0\x0b\xcc\x37\x02" //..........Q...7. |
// /* 0870 */ "\x01\x0d\x81\xc7\x9f\x27\xa7\x51\xe7\xce\x13\x48\x19\xfc\xf8\xf5" //.....'.Q...H.... |
// /* 0880 */ "\x2c\x75\x0e\xa0\x08\x07\x35\x01\xc4\x84\x26\x00\x00\xde\x4c\x4a" //,u....5...&...LJ |
// /* 0890 */ "\x13\x00\x47\x42\xa4\x0d\xbb\xa6\xaa\x00\x3c\x27\x01\x8f\x49\x79" //..GB......<'..Iy |
// /* 08a0 */ "\xc9\xa2\x9c\x6a\x40\x2d\x58\x9b\x84\x81\x81\xc1\x8f\xcf\x9c\x2c" //...j@-X........, |
// /* 08b0 */ "\xc1\x00\x30\x41\xfa\xf9\xe9\x14\x74\x26\x3a\xa7\x50\x05\xe4\x84" //..0A....t&:.P... |
// /* 08c0 */ "\x82\x3f\x3e\x61\x74\xf8\xb0\x09\x86\x41\xe3\xa1\x2f\xa5\x57\xe8" //.?>at....A../.W. |
// /* 08d0 */ "\x61\xf4\xf8\x7a\x6d\x3e\x82\x5f\x46\x58\xe3\xa1\x30\xab\xe8\xca" //a..zm>._FX..0... |
// /* 08e0 */ "\xf3\xa5\x7a\x6a\xff\x46\x8f\x49\x4f\x52\xe3\xa1\x31\xcc\x1b\x7a" //..zj.F.IOR..1..z |
// /* 08f0 */ "\x90\x05\x06\xf7\x8b\x60\xde\x84\x17\xb0\xf2\x3a\x13\x19\xc0\xd9" //.....`.....:.... |
// /* 0900 */ "\xac\x10\xa1\x0c\x98\x00\x94\xd4\x2b\x81\x1b\xa4\x17\x99\x56\x20" //........+.....V  |
// /* 0910 */ "\x7c\xca\xa9\x03\x00\x2a\xa0\x38\x18\x5a\x30\x2b\x64\x70\xbb\x7a" //|....*.8.Z0+dp.z |
// /* 0920 */ "\x7c\x3c\x03\x03\x93\x18\xba\xcb\x72\x31\x3b\x82\x40\x09\x46\x18" //|<......r1;.@.F. |
// /* 0930 */ "\x03\x8e\xd4\xdf\x2d\xcc\x48\x17\xed\x80\x58\x0e\x60\x00\x4f\xae" //....-.H...X.`.O. |
// /* 0940 */ "\xfe\xa8\x74\xad\x72\x00\x70\x37\x8c\x29\x07\x0f\x54\xe0\xbc\x1b" //..t.r.p7.)..T... |
// /* 0950 */ "\x8d\x00\x58\x37\x1a\x1d\x65\xb8\x08\x15\xc2\x00\x07\x0a\xe1\xa4" //..X7..e......... |
// /* 0960 */ "\x03\xb5\xe1\xb6\x04\xfe\xef\x81\x62\x5c\x9d\x40\x3c\x0a\x5c\x2d" //........b..@<..- |
// /* 0970 */ "\x58\xa1\x35\x78\x9e\xb1\x5d\xf3\x63\x7b\xf3\xcf\x03\xb7\x0b\x40" //X.5x..].c{.....@ |
// /* 0980 */ "\x65\x40\x1b\xc3\xd1\x3f\xbb\xc8\x84\x38\xb9\xb1\xd1\x0b\x0d\xc6" //e@...?...8...... |
// /* 0990 */ "\xbc\x9a\xe1\x2c\xc8\xbe\x48\xf5\xde\xa0\x0d\x22\xe5\xde\x93\x18" //...,..H....".... |
// /* 09a0 */ "\xfd\x62\x07\x81\xc0\x69\x5c\x56\x5e\x45\x61\x56\x56\x02\xb1\x04" //.b...i.V^EaVV... |
// /* 09b0 */ "\x7d\x65\x00\x2f\xc3\xae\x2b\xd0\x0d\xc5\x79\x80\x1a\x95\xba\xf0" //}e./..+...y..... |
// /* 09c0 */ "\x7f\x6f\x26\x10\xbd\x8e\x2b\x73\x51\xe2\xb4\xf2\xd0\x42\xa0\x03" //.o&...+sQ....B.. |
// /* 09d0 */ "\xff\xe0\x33\x8a\x9b\x01\x47\x9e\xfe\x7a\xc0\x15\xe8\x6a\xa1\x8c" //..3...G..z...j.. |
// /* 09e0 */ "\x00\xe4\x5f\x18\xc2\xbb\x6f\x06\x90\x07\xfc\x46\xfe\xce\x1e\x6c" //.._...o....F...l |
// /* 09f0 */ "\x6f\x2c\x00\xa1\x10\x00\x41\x31\x95\x94\x58\xc6\x0a\x10\x84\x20" //o,....A1..X....  |
// /* 0a00 */ "\x54\x01\x52\x84\x2a\x86\x15\xea\x00\xc0\x61\xce\x6c\x58\xab\x7d" //T.R.*.....a.lX.} |
// /* 0a10 */ "\x4b\x5a\x21\x62\x45\x29\x81\x32\x94\xcf\xae\xf8\x35\x62\x22\xd3" //KZ!bE).2....5b". |
// /* 0a20 */ "\x5e\x6f\x68\x24\x98\xc7\x02\x85\x31\x81\x08\x3f\xbb\x4a\x21\x04" //^oh$....1..?.J!. |
// /* 0a30 */ "\x0b\xf1\xab\xca\x8c\xaa\x9a\x80\x95\xae\x05\xd4\x00\x9f\xb8\xbd" //................ |
// /* 0a40 */ "\x21\xce\x62\x10\x1b\x19\x1d\x0a\xe7\x1c\xc8\x86\xf3\x8a\xc0\x94" //!.b............. |
// /* 0a50 */ "\x4f\x96\x02\x5a\xa7\x00\xe2\xe2\x57\xd6\x60\xd9\x68\xa1\x16\x31" //O..Z....W.`.h..1 |
// /* 0a60 */ "\xd4\x00\xe9\xfe\x8f\x57\xef\xf2\x21\x0e\x53\x40\xef\x53\x82\xbd" //.....W..!.S@.S.. |
// /* 0a70 */ "\x4e\xbd\xd7\x80\x70\xd9\xec\x63\x39\xb5\x3d\xef\xea\x3c\x3a\xe3" //N...p..c9.=..<:. |
// /* 0a80 */ "\x03\x35\x16\x6e\x2e\xe1\x8c\xd2\x18\x6f\xe0\x5c\x01\x20\xfa\xd0" //.5.n.....o... .. |
// /* 0a90 */ "\xda\xd6\x03\x51\x9e\xdd\xc0\x72\xb3\x13\xd5\x64\x0c\x1e\x9f\x00" //...Q...r...d.... |
// /* 0aa0 */ "\x14\x49\xf5\xde\xa3\x12\x8a\x00\xd4\x00\x6f\xd7\xfe\xa7\xf3\x38" //.I........o....8 |
// /* 0ab0 */ "\xe1\xab\x8c\xdb\x86\xad\x42\x20\xd0\x21\x04\x94\xa0\xc6\x32\xc6" //......B .!....2. |
// /* 0ac0 */ "\x34\x94\xaa\x18\xc0\xc6\x40\x72\x2a\xcb\x78\x58\xb1\x57\x52\xa0" //4.....@r*.xX.WR. |
// /* 0ad0 */ "\x0c\xd3\xc5\xc1\xc3\x43\x00\x9e\x19\x65\xdb\x7f\x75\x06\x5c\xab" //.....C...e..u... |
// /* 0ae0 */ "\x73\x98\x72\x11\x70\x08\x8e\x19\xca\x80\x23\x73\x5f\x00\x06\x1e" //s.r.p.....#s_... |
// /* 0af0 */ "\xa0\x63\x6d\xa0\xe5\x9f\xd9\xec\x2e\x38\x40\x04\xfa\x6e\x19\xb5" //.cm......8@..n.. |
// /* 0b00 */ "\xf6\xe7\x98\xe5\x4b\x9c\xf8\x1d\x4c\x11\xfa\xdf\x6f\x7b\xa5\xad" //....K...L...o{.. |
// /* 0b10 */ "\x28\xf5\x3a\x48\x78\x53\xb8\x4c\x81\x38\x9d\x38\x6d\xe1\xfa\xe6" //(.:HxS.L.8.8m... |
// /* 0b20 */ "\xa0\x07\xc1\x79\x50\x04\x9b\x4d\x72\x3b\x74\x60\x5a\x76\x2c\x9c" //...yP..Mr;t`Zv,. |
// /* 0b30 */ "\x77\xbb\x7e\xda\x03\xc3\x10\x0a\x80\x23\xef\x55\xc8\xfc\xb0\x07" //w.~......#.U.... |
// /* 0b40 */ "\xee\x05\x40\x13\x11\xfd\xbc\x63\x5e\x56\x02\x63\x52\xd5\x40\x0f" //..@....c^V.cR.@. |
// /* 0b50 */ "\x18\xd2\x38\x66\x4f\xe1\xaf\x0e\xc7\x80\xa9\x76\xdf\xc4\xc9\x50" //..8fO......v...P |
// /* 0b60 */ "\x03\xb1\x71\x88\x79\x75\x87\x59\x1f\x9d\xc6\x75\xf8\xaf\x80\x1b" //..q.yu.Y...u.... |
// /* 0b70 */ "\x9a\x85\x03\x7a\x6e\x00\xca\x7a\xd6\x3c\x66\x02\xf0\xd5\x81\xe2" //...zn..z.<f..... |
// /* 0b80 */ "\x9d\xb7\xe4\x9f\xab\x3c\x64\x26\xc9\xc7\x61\xf3\x59\x27\x49\xe0" //.....<d&..a.Y'I. |
// /* 0b90 */ "\x87\x13\x77\x82\x2f\xcb\x4f\xaf\x1e\x32\x8c\x30\x61\x35\x00\x4a" //..w./.O..2.0a5.J |
// /* 0ba0 */ "\x46\x35\xde\x02\xb4\x25\x7c\xa7\xd4\x80\x39\xf5\xdf\xe5\x20\xbf" //F5...%|...9... . |
// /* 0bb0 */ "\xc2\x38\x13\x43\x6b\x5d\x09\x04\x23\x85\x5c\xd4\x01\x5f\x3d\x34" //.8.Ck]..#...._=4 |
// /* 0bc0 */ "\xe0\x43\x1d\x9a\x5f\x14\x08\xf5\xe9\xa8\x33\xb3\xe0\x06\x54\x72" //.C.._.....3...Tr |
// /* 0bd0 */ "\x85\x8b\x3d\x4c\x21\x15\x00\x39\x2f\xba\x68\x12\x97\xe1\x30\x22" //..=L!..9/.h...0" |
// /* 0be0 */ "\xcd\x3b\xf0\x0f\x6c\xff\x29\xe3\x4b\xc7\x67\xc0\x1c\x58\x82\xeb" //.;..l.).K.g..X.. |
// /* 0bf0 */ "\x88\xe0\x70\x04\x33\x82\x00\xe6\xa0\xf3\x6b\x5b\x0f\x19\xa5\x40" //..p.3.....k[...@ |
// /* 0c00 */ "\x16\x96\xb4\x07\x50\x03\x09\xf4\xab\xe6\xbc\x49\x10\x5c\x57\xab" //....P......I..W. |
// /* 0c10 */ "\x15\xfb\xfb\xed\xbe\x04\x20\xc7\xe1\xa8\x87\x39\x80\xb1\xf0\x81" //...... ....9.... |
// /* 0c20 */ "\x7b\x64\x09\x92\x3c\x98\x61\xed\x81\x98\x9e\xb3\x02\xd1\xce\x7e" //{d..<.a........~ |
// /* 0c30 */ "\xc4\x51\x18\x42\x3f\xcc\x64\xa9\xc7\x86\x5c\xe0\x50\x13\xdc\x6d" //.Q.B?.d.....P..m |
// /* 0c40 */ "\x34\x55\x96\xfc\xd4\xb7\x81\x44\x7b\x61\xc0\xca\x78\xde\x29\x6b" //4U.....D{a..x.)k |
// /* 0c50 */ "\x55\x1f\x68\xb3\x4d\x48\x10\x86\xa0\x0b\xc0\x2a\x01\x1e\xad\xe0" //U.h.MH.....*.... |
// /* 0c60 */ "\x30\x22\x9c\x03\x58\xe3\x5a\xa8\x02\xe1\x93\x26\x41\x66\x10\x00" //0"..X.Z....&Af.. |
// /* 0c70 */ "\x35\x25\x44\xb3\x90\xe0\xe2\x8d\x36\x9a\xb8\xe3\x1e\xd6\x0d\x74" //5%D.....6......t |
// /* 0c80 */ "\x80\x42\x1e\x3b\x2c\x6c\x0a\xc1\x4a\xa0\x0a\x15\x3d\xef\x17\x3c" //.B.;,l..J...=..< |
// /* 0c90 */ "\x87\x68\x58\x7c\xed\x1e\xc0\x0b\xd3\x8c\x54\x10\x16\xa4\xa1\xce" //.hX|......T..... |
// /* 0ca0 */ "\x31\x11\x2b\xc0\x19\xd8\xca\x78\xc2\xbe\x0a\x43\xc8\x51\x43\xf3" //1.+....x...C.QC. |
// /* 0cb0 */ "\xda\xaf\x3d\xc1\xc0\x0f\x37\xbc\xb8\xf9\x2f\x29\xbf\xc2\x44\x21" //..=...7.../)..D! |
// /* 0cc0 */ "\x84\x95\x9c\x17\x10\x12\x84\x2f\x92\xe1\x5a\x80\xc3\x9c\xfe\x4b" //......./..Z....K |
// /* 0cd0 */ "\xcd\x6e\x02\xac\x6d\xee\xe0\xcf\x38\x68\x3d\xfc\x5c\x42\x1e\x2d" //.n..m...8h=..B.- |
// /* 0ce0 */ "\x7c\x97\x38\xf7\xfa\x12\x10\xfe\x85\x7e\xfe\x03\x8b\xcf\x43\x55" //|.8......~....CU |
// /* 0cf0 */ "\x00\x5e\x61\x0f\x9e\xf4\x7b\x17\xdf\x86\x38\xe8\x31\x57\x2b\x8b" //.^a...{...8.1W+. |
// /* 0d00 */ "\xd4\x40\x6e\x91\xc1\x8e\x96\xec\x64\xc7\x34\x73\x91\x8a\x80\x2f" //.@n.....d.4s.../ |
// /* 0d10 */ "\x83\xa2\x65\x14\xc5\xe6\x48\x34\x88\x42\x5d\x91\x56\x03\x14\x65" //..e...H4.B].V..e |
// /* 0d20 */ "\x59\x6e\x1f\x9e\x61\x3b\xc7\x9e\xe3\x2a\xfb\xf0\x62\x93\x76\x4f" //Yn..a;...*..b.vO |
// /* 0d30 */ "\x6d\x61\xad\x15\x03\x83\x93\x01\x72\xe0\x39\x85\x07\xde\xc3\x51" //ma......r.9....Q |
// /* 0d40 */ "\x06\x28\xd0\x2b\x9c\xd6\xd9\x25\xa2\xd9\xe2\xb0\x1b\x5c\x76\x8a" //.(.+...%......v. |
// /* 0d50 */ "\x31\x46\xa4\xe2\xa6\x30\x17\xd3\x5a\xe3\xac\xb6\x78\xac\x0a\xd1" //1F...0..Z...x... |
// /* 0d60 */ "\x15\xb7\xcb\x59\x86\x28\xc0\x95\x00\x6b\x39\xc8\xff\xd6\xc6\xd5" //...Y.(...k9..... |
// /* 0d70 */ "\x1d\xb8\xcb\x5a\x06\x28\xc1\x7f\x32\x2d\xb2\x5b\x9c\xd5\xa8\x62" //...Z.(..2-.[...b |
// /* 0d80 */ "\x8c\x1e\xdb\xf4\x15\x0c\x47\x0e\x01\x80\xdc\x65\xa2\xfc\xa7\x48" //......G....e...H |
// /* 0d90 */ "\x00\x40\x30\x1b\x64\x97\x25\x81\x01\x70\x78\xea\xcf\x15\x18\xa8" //.@0.d.%..px..... |
// /* 0da0 */ "\x01\x8e\x70\x90\x5c\xe6\xb8\x20\xd1\x64\x13\x01\x80\xdb\xff\x2c" //..p.... .d....., |
// /* 0db0 */ "\x8f\xfe\x30\x7d\x12\x04\x21\x80\xd3\xc2\xf8\xc3\x04\x0f\xf5\xe4" //..0}..!......... |
// /* 0dc0 */ "\x52\x6c\x26\x49\xca\x52\xec\x83\x7f\xce\x03\xd9\x46\x28\xdc\x45" //Rl&I.R......F(.E |
// /* 0dd0 */ "\x9e\x00\x36\x78\x95\xe2\x05\x40\x0c\x2c\xc3\x06\xc0\x80\xc5\xb0" //..6x...@.,...... |
// /* 0de0 */ "\x41\x42\xe7\x35\x1a\xf3\x32\xe3\x2d\x1a\x62\xf2\x6a\xd3\xf2\x42" //AB.5..2.-.b.j..B |
// /* 0df0 */ "\xa4\x35\x00\x30\xc0\x80\x1f\x9f\x1e\x7e\x81\x36\x21\x8a\x37\xe5" //.5.0.....~.6!.7. |
// /* 0e00 */ "\x60\xfd\xe3\x1f\xa8\xd6\xd9\x2f\xd0\xba\x80\x28\x51\x81\x0f\xac" //`....../...(Q... |
// /* 0e10 */ "\x98\x06\x29\x4e\x0a\x63\x18\xa3\x10\x86\xf9\x00\x13\xf7\x10\x2b" //..)N.c.........+ |
// /* 0e20 */ "\x74\x96\xd1\x15\x1a\xf9\x2a\x80\x28\xc9\x40\x0e\x55\xac\x3e\xe0" //t.....*.(.@.U.>. |
// /* 0e30 */ "\x16\xa0\x9b\x78\xc5\x18\x01\x49\x04\x39\x90\x01\x90\x00\x68\x00" //...x...I.9....h. |
// Sent dumped on RDP Client (5) 3648 bytes |
// Listener closed |
// Incoming socket 5 (ip=10.10.47.228) |
// Socket RDP Client (5) : closing connection |
// RDP Client (0): total_received=1737, total_sent=35116 |
