const char outdata[] = /* NOLINT */
{
// Listen: binding socket 3 on 0.0.0.0:3389 |
// Listen: listening on socket 3 |
// SocketTransport: recv_timeout=1000 |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 43 bytes |
// /* 0000 */ "\x03\x00\x00\x2b\x26\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" // ...+&......Cooki |
// /* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x72\x7a\x68\x6f" // e: mstshash=rzho |
// /* 0020 */ "\x75\x0d\x0a\x01\x00\x08\x00\x03\x00\x00\x00"                     // u.......... |
// Dump done on RDP Client (4) 43 bytes |
// Front::incoming: CONNECTION_INITIATION |
// Front::incoming: receiving x224 request PDU |
// CR Recv: PROTOCOL TLS |
// CR Recv: PROTOCOL HYBRID |
// Front::incoming: Fallback to legacy security protocol |
// Front::incoming: sending x224 connection confirm PDU |
// -----------------> Front::incoming: TLS Support not Enabled |
// Sending on RDP Client (4) 11 bytes |
/* 0000 */ "\x03\x00\x00\x0b\x06\xd0\x00\x00\x00\x00\x00"                     // ........... |
// Sent dumped on RDP Client (4) 11 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 446 bytes |
// /* 0000 */ "\x03\x00\x01\xbe\x02\xf0\x80\x7f\x65\x82\x01\xb2\x04\x01\x01\x04" // ........e....... |
// /* 0010 */ "\x01\x01\x01\x01\xff\x30\x20\x02\x02\x00\x22\x02\x02\x00\x02\x02" // .....0 ..."..... |
// /* 0020 */ "\x02\x00\x00\x02\x02\x00\x01\x02\x02\x00\x00\x02\x02\x00\x01\x02" // ................ |
// /* 0030 */ "\x02\xff\xff\x02\x02\x00\x02\x30\x20\x02\x02\x00\x01\x02\x02\x00" // .......0 ....... |
// /* 0040 */ "\x01\x02\x02\x00\x01\x02\x02\x00\x01\x02\x02\x00\x00\x02\x02\x00" // ................ |
// /* 0050 */ "\x01\x02\x02\x04\x20\x02\x02\x00\x02\x30\x20\x02\x02\xff\xff\x02" // .... ....0 ..... |
// /* 0060 */ "\x02\xfc\x17\x02\x02\xff\xff\x02\x02\x00\x01\x02\x02\x00\x00\x02" // ................ |
// /* 0070 */ "\x02\x00\x01\x02\x02\xff\xff\x02\x02\x00\x02\x04\x82\x01\x3f\x00" // ..............?. |
// /* 0080 */ "\x05\x00\x14\x7c\x00\x01\x81\x36\x00\x08\x00\x10\x00\x01\xc0\x00" // ...|...6........ |
// /* 0090 */ "\x44\x75\x63\x61\x81\x28\x01\xc0\xd8\x00\x04\x00\x08\x00\x20\x03" // Duca.(........ . |
// /* 00a0 */ "\x58\x02\x01\xca\x03\xaa\x0c\x04\x00\x00\x28\x0a\x00\x00\x72\x00" // X.........(...r. |
// /* 00b0 */ "\x7a\x00\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // z.h............. |
// /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00" // ................ |
// /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\xca\x01\x00\x00\x00" // ................ |
// /* 0120 */ "\x00\x00\x18\x00\x07\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\xc0" // ................ |
// /* 0170 */ "\x0c\x00\x09\x00\x00\x00\x00\x00\x00\x00\x02\xc0\x0c\x00\x03\x00" // ................ |
// /* 0180 */ "\x00\x00\x00\x00\x00\x00\x03\xc0\x38\x00\x04\x00\x00\x00\x63\x6c" // ........8.....cl |
// /* 0190 */ "\x69\x70\x72\x64\x72\x00\xc0\xa0\x00\x00\x72\x64\x70\x73\x6e\x64" // iprdr.....rdpsnd |
// /* 01a0 */ "\x00\x00\xc0\x00\x00\x00\x73\x6e\x64\x64\x62\x67\x00\x00\xc0\x00" // ......snddbg.... |
// /* 01b0 */ "\x00\x00\x72\x64\x70\x64\x72\x00\x00\x00\x80\x80\x00\x00"         // ..rdpdr....... |
// Dump done on RDP Client (4) 446 bytes |
// Front::incoming: Basic Settings Exchange |
// GCC::UserData tag=c001 length=216 |
// Front::incoming: Received from Client GCC User Data CS_CORE (216 bytes) |
// cs_core::length [00d8] |
// cs_core::version [80004] RDP 5.0, 5.1, 5.2, and 6.0 clients) |
// cs_core::desktopWidth  = 800 |
// cs_core::desktopHeight = 600 |
// cs_core::colorDepth    = [ca01] [RNS_UD_COLOR_8BPP] superseded by postBeta2ColorDepth |
// cs_core::SASSequence   = [aa03] [RNS_UD_SAS_DEL] |
// cs_core::keyboardLayout= 040c |
// cs_core::clientBuild   = 2600 |
// cs_core::clientName    = rzh |
// cs_core::keyboardType  = [0004] IBM enhanced (101-key or 102-key) keyboard |
// cs_core::keyboardSubType      = [0000] OEM code |
// cs_core::keyboardFunctionKey  = 12 function keys |
// cs_core::imeFileName    = |
// cs_core::postBeta2ColorDepth  = [ca01] [8 bpp] |
// cs_core::clientProductId = 1 |
// cs_core::serialNumber = 0 |
// cs_core::highColorDepth  = [0018] [24-bit RGB mask] |
// cs_core::supportedColorDepths  = [0007] [24/16/15/] |
// cs_core::earlyCapabilityFlags  = [0001] |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_ERRINFO_PDU |
// cs_core::clientDigProductId=[00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000] |
// cs_core::connectionType = <unknown> |
// cs_core::pad1octet = 0 |
// cs_core::serverSelectedProtocol = 0 |
// GCC::UserData tag=c004 length=12 |
// Front::incoming: Receiving from Client GCC User Data CS_CLUSTER (12 bytes) |
// cs_cluster::flags [0009] |
// cs_cluster::flags::REDIRECTION_SUPPORTED |
// cs_cluster::flags::redirectionVersion = 3 |
// GCC::UserData tag=c002 length=12 |
// Front::incoming: Received from Client GCC User Data CS_SECURITY (12 bytes) |
// CSSecGccUserData::encryptionMethods 3 |
// CSSecGccUserData::extEncryptionMethods 0 |
// GCC::UserData tag=c003 length=56 |
// Front::incoming: Received from Client GCC User Data CS_NET (56 bytes) |
// cs_net::channelCount   = 4 |
// cs_net::channel 'cliprdr' [1004] |
// cs_net::channel 'rdpsnd' [1005] |
// cs_net::channel 'snddbg' [1006] |
// cs_net::channel 'rdpdr' [1007] |
// Front::incoming: Sending to client GCC User Data SC_CORE (8 bytes) |
// sc_core::version [80004] RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 servers) |
// Front::incoming: Sending to client GCC User Data SC_NET (16 bytes) |
// sc_net::MCSChannelId   = 1003 |
// sc_net::channelCount   = 4 |
// sc_net::channel[1004]::id = 1004 |
// sc_net::channel[1005]::id = 1005 |
// sc_net::channel[1006]::id = 1006 |
// sc_net::channel[1007]::id = 1007 |
// Front::incoming: Sending to client GCC User Data SC_SECURITY (236 bytes) |
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
// Sending on RDP Client (4) 333 bytes |
/* 0000 */ "\x03\x00\x01\x4d\x02\xf0\x80\x7f\x66\x82\x01\x41\x0a\x01\x00\x02" // ...M....f..A.... |
/* 0010 */ "\x01\x00\x30\x1a\x02\x01\x22\x02\x01\x03\x02\x01\x00\x02\x01\x01" // ..0..."......... |
/* 0020 */ "\x02\x01\x00\x02\x01\x01\x02\x03\x00\xff\xf8\x02\x01\x02\x04\x82" // ................ |
/* 0030 */ "\x01\x1b\x00\x05\x00\x14\x7c\x00\x01\x2a\x14\x76\x0a\x01\x01\x00" // ......|..*.v.... |
/* 0040 */ "\x01\xc0\x00\x4d\x63\x44\x6e\x81\x04\x01\x0c\x08\x00\x04\x00\x08" // ...McDn......... |
/* 0050 */ "\x00\x03\x0c\x10\x00\xeb\x03\x04\x00\xec\x03\xed\x03\xee\x03\xef" // ................ |
/* 0060 */ "\x03\x02\x0c\xec\x00\x01\x00\x00\x00\x01\x00\x00\x00\x20\x00\x00" // ............. .. |
/* 0070 */ "\x00\xb8\x00\x00\x00\xb8\x6c\xda\xa6\xf0\xf6\x30\x8d\xa8\x16\xa6" // ......l....0.... |
/* 0080 */ "\x6e\xe0\xc3\xe5\xcc\x98\x76\xdd\xf5\xd0\x26\x74\x5f\x88\x4c\xc2" // n.....v...&t_.L. |
/* 0090 */ "\x50\xc0\xdf\xc9\x50\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00" // P...P........... |
/* 00a0 */ "\x00\x06\x00\x5c\x00\x52\x53\x41\x31\x48\x00\x00\x00\x00\x02\x00" // .....RSA1H...... |
/* 00b0 */ "\x00\x3f\x00\x00\x00\x01\x00\x01\x00\x67\xab\x0e\x6a\x9f\xd6\x2b" // .?.......g..j..+ |
/* 00c0 */ "\xa3\x32\x2f\x41\xd1\xce\xee\x61\xc3\x76\x0b\x26\x11\x70\x48\x8a" // .2/A...a.v.&.pH. |
/* 00d0 */ "\x8d\x23\x81\x95\xa0\x39\xf7\x5b\xaa\x3e\xf1\xed\xb8\xc4\xee\xce" // .#...9.[.>...... |
/* 00e0 */ "\x5f\x6a\xf5\x43\xce\x5f\x60\xca\x6c\x06\x75\xae\xc0\xd6\xa4\x0c" // _j.C._`.l.u..... |
/* 00f0 */ "\x92\xa4\xc6\x75\xea\x64\xb2\x50\x5b\x00\x00\x00\x00\x00\x00\x00" // ...u.d.P[....... |
/* 0100 */ "\x00\x08\x00\x48\x00\x6a\x41\xb1\x43\xcf\x47\x6f\xf1\xe6\xcc\xa1" // ...H.jA.C.Go.... |
/* 0110 */ "\x72\x97\xd9\xe1\x85\x15\xb3\xc2\x39\xa0\xa6\x26\x1a\xb6\x49\x01" // r.......9..&..I. |
/* 0120 */ "\xfa\xa6\xda\x60\xd7\x45\xf7\x2c\xee\xe4\x8e\x64\x2e\x37\x49\xf0" // ...`.E.,...d.7I. |
/* 0130 */ "\x4c\x94\x6f\x08\xf5\x63\x4c\x56\x29\x55\x5a\x63\x41\x2c\x20\x65" // L.o..cLV)UZcA, e |
/* 0140 */ "\x95\x99\xb1\x15\x7c\x00\x00\x00\x00\x00\x00\x00\x00"             // ....|........ |
// Sent dumped on RDP Client (4) 333 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 20 bytes |
// /* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x04\x00\x01\x00\x01\x03\x00\x00\x08" // ................ |
// /* 0010 */ "\x02\xf0\x80\x28"                                                 // ...( |
// Dump done on RDP Client (4) 20 bytes |
// Front::incoming: Channel Connection |
// Front::incoming: Recv MCS::ErectDomainRequest |
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
// Recv done on RDP Client (4) 95 bytes |
// /* 0000 */ "\x03\x00\x00\x5f\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x50\x01" // ..._...d....p.P. |
// /* 0010 */ "\x00\x00\x00\x48\x00\x00\x00\x62\x2f\xfb\xc8\x0f\x98\x75\x6c\xf1" // ...H...b/....ul. |
// /* 0020 */ "\xf8\x75\x54\x91\xdf\x6f\xf1\x62\x3a\x7e\xbe\x0d\xac\xd4\xd0\xe6" // .uT..o.b:~...... |
// /* 0030 */ "\x2a\xd0\x91\x83\x51\xeb\x05\x9b\x60\x23\x1c\xe3\x38\x95\x92\xc4" // *...Q...`#..8... |
// /* 0040 */ "\x28\xea\x3f\xda\xea\xe1\x10\xb3\x35\xab\x75\x4b\x15\x00\x6c\xeb" // (.?.....5.uK..l. |
// /* 0050 */ "\x7b\xf9\xf1\x9a\xf9\x3f\x4e\x00\x00\x00\x00\x00\x00\x00\x00"     // {....?N........ |
// Dump done on RDP Client (4) 95 bytes |
// Front::incoming: RDP Security Commencement |
// Front::incoming: Legacy RDP mode: expecting exchange packet |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 337 bytes |
// /* 0000 */ "\x03\x00\x01\x51\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\x42\x48" // ...Q...d....p.BH |
// /* 0010 */ "\x00\x00\x00\x16\x58\xe5\x9d\x6c\x04\x56\xb9\xc2\x52\xab\xe4\xf2" // ....X..l.V..R... |
// /* 0020 */ "\xc8\xb9\x1b\xf5\x71\x62\x81\x26\xe1\xab\xc7\xb0\x4b\xdc\x1d\x37" // ....qb.&....K..7 |
// /* 0030 */ "\x1f\xfa\x48\xb0\xa6\x25\x56\x02\x68\x61\x75\x30\x1d\x05\xcd\xbd" // ..H..%V.hau0.... |
// /* 0040 */ "\x14\xdf\x6c\x7d\xef\x84\x77\xa2\x1f\xcb\xce\x0c\x2b\x9f\xec\x57" // ..l}..w.....+..W |
// /* 0050 */ "\xb2\x7a\xc6\xe0\x78\xdd\x06\xe8\x77\xec\x01\xc8\xe6\x6e\x73\x1a" // .z..x...w....ns. |
// /* 0060 */ "\x3e\x9b\xa1\xff\x6b\x40\xb5\x18\x88\x42\xf5\x66\x13\x38\x36\x6c" // >...k@...B.f.86l |
// /* 0070 */ "\x72\x41\xf3\x49\xe9\x46\xec\xbb\x3d\x0d\x1d\x06\xb6\x2a\xdd\x87" // rA.I.F..=....*.. |
// /* 0080 */ "\xb6\x7c\xe9\xea\x14\x91\xbd\x32\xdb\xdf\xdc\xa2\xa9\xd7\xb7\x11" // .|.....2........ |
// /* 0090 */ "\x50\x10\x92\x0a\x7d\x8b\x5a\x57\x49\xbd\x1b\xe3\xf3\x77\x98\xd1" // P...}.ZWI....w.. |
// /* 00a0 */ "\x1c\x0e\xb3\xf3\x1c\xea\x1d\x8d\x18\xe7\xd6\x4c\xd6\xa5\xc9\x53" // ...........L...S |
// /* 00b0 */ "\xd9\x8c\x38\xe0\xdb\x68\x36\xb9\x8a\x50\x78\x1f\x76\x39\x53\x51" // ..8..h6..Px.v9SQ |
// /* 00c0 */ "\xf4\xfb\xf3\x63\x5f\x89\x60\x91\x29\x34\x3f\x2b\x1f\x10\x91\x63" // ...c_.`.)4?+...c |
// /* 00d0 */ "\x39\xd8\x2c\x6f\x31\x74\x98\x1c\x73\xc0\x65\x01\x4c\xcf\x5b\xbe" // 9.,o1t..s.e.L.[. |
// /* 00e0 */ "\xbb\x2f\x80\xdf\x79\x7e\x18\x84\x9b\x18\x90\x0a\xee\x2e\x34\x4c" // ./..y~........4L |
// /* 00f0 */ "\xcc\x8f\x52\x35\xbb\x01\x7b\x34\xf3\xbc\x90\xaf\x00\x28\x66\x52" // ..R5..{4.....(fR |
// /* 0100 */ "\x88\xc5\xbb\x0d\x27\x89\x75\x77\xec\x3a\x3d\x5b\xc7\x09\x1f\xd3" // ....'.uw.:=[.... |
// /* 0110 */ "\x0a\x99\xe8\x25\x04\xbf\xfd\xff\x83\xb1\xc4\x6d\xd7\xa2\xca\x75" // ...%.......m...u |
// /* 0120 */ "\xc4\xcd\x4f\xe3\x28\xa7\x61\x53\x9a\x9b\xd9\xc7\x46\x37\x77\xb7" // ..O.(.aS....F7w. |
// /* 0130 */ "\x70\xd0\xdf\xf2\x1f\xfe\x8c\xcd\x03\x12\xd7\xa8\x77\x6a\x2a\xa3" // p...........wj*. |
// /* 0140 */ "\xa0\xa2\xb1\xeb\x0c\x72\x5a\x2e\xbd\x89\x69\x47\x50\x2c\xec\xde" // .....rZ...iGP,.. |
// /* 0150 */ "\x75"                                                             // u |
// Dump done on RDP Client (4) 337 bytes |
// Front::incoming: Secure Settings Exchange |
// RDP-5 Style logon |
// Receiving from client InfoPacket |
// InfoPacket::CodePage 0 |
// InfoPacket::flags 0x133 |
// InfoPacket::flags:INFO_MOUSE yes |
// InfoPacket::flags:INFO_DISABLECTRLALTDEL yes |
// InfoPacket::flags:INFO_AUTOLOGON no |
// InfoPacket::flags:INFO_UNICODE yes |
// InfoPacket::flags:INFO_MAXIMIZESHELL  yes |
// InfoPacket::flags:INFO_LOGONNOTIFY no |
// InfoPacket::flags:INFO_COMPRESSION no |
// InfoPacket::flags:CompressionTypeMask no |
// InfoPacket::flags:INFO_ENABLEWINDOWSKEY  yes |
// InfoPacket::flags:INFO_REMOTECONSOLEAUDIO no |
// InfoPacket::flags:INFO_FORCE_ENCRYPTED_CS_PDU no |
// InfoPacket::flags:INFO_RAIL no |
// InfoPacket::flags:INFO_LOGONERRORS no |
// InfoPacket::flags:INFO_MOUSE_HAS_WHEEL no |
// InfoPacket::flags:INFO_PASSWORD_IS_SC_PIN no |
// InfoPacket::flags:INFO_NOAUDIOPLAYBACK no |
// InfoPacket::flags:INFO_USING_SAVED_CREDS no |
// InfoPacket::flags:INFO_AUDIOCAPTURE no |
// InfoPacket::flags:INFO_VIDEO_DISABLE no |
// InfoPacket::flags:INFO_HIDEF_RAIL_SUPPORTED no |
// InfoPacket::cbDomain 2 |
// InfoPacket::cbUserName 12 |
// InfoPacket::cbPassword 2 |
// InfoPacket::cbAlternateShell 2 |
// InfoPacket::cbWorkingDir 2 |
// InfoPacket::Domain |
// InfoPacket::UserName rzhou |
// InfoPacket::Password <hidden> |
// InfoPacket::AlternateShell |
// InfoPacket::WorkingDir |
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2 |
// InfoPacket::ExtendedInfoPacket::cbClientAddress 24 |
// InfoPacket::ExtendedInfoPacket::clientAddress 10.10.43.33 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 60 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\WINNT\System32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 0 |
// InfoPacket::ExtendedInfoPacket::performanceFlags 0x27 |
// InfoPacket::ExtendedInfoPacket::PERF_DISABLE_WALLPAPER |
// InfoPacket::ExtendedInfoPacket::PERF_DISABLE_FULLWINDOWDRAG |
// InfoPacket::ExtendedInfoPacket::PERF_DISABLE_MENUANIMATIONS |
// InfoPacket::ExtendedInfoPacket::PERF_DISABLE_CURSOR_SHADOW |
// InfoPacket::ExtendedInfoPacket::cbAutoReconnectLen 0 |
// InfoPacket::ExtendedInfoPacket::reserved1 0 |
// InfoPacket::ExtendedInfoPacket::reserved2 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::Bias 4294967236 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardName GTB, normaltid |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMonth 10 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDay 5 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wHour 3 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardBias 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightName GTB, sommartid |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMonth 3 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDay 5 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wHour 2 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightBias 4294967236 |
// client info: performance flags before=0x00000027 after=0x0000002F default=0x00000080 present=0x00000028 not-present=0x00000000 |
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
// Recv done on RDP Client (4) 157 bytes |
// /* 0000 */ "\x03\x00\x00\x9d\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x8e\x80" // .......d....p... |
// /* 0010 */ "\x00\x00\x00\x13\x03\x8a\x00\x01\x00\x00\x00\x00\x00\x01\xff\x00" // ................ |
// /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02" // ................ |
// /* 0040 */ "\x00\x48\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // .H.............. |
// /* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
// /* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0f\x00\x06\x00\x72" // ...............r |
// /* 0090 */ "\x7a\x68\x6f\x75\x00\x10\x00\x04\x00\x72\x7a\x68\x00"             // zhou.....rzh. |
// Dump done on RDP Client (4) 157 bytes |
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
//      General caps::extra flags 0 |
//      General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED no |
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
//      Bitmap caps::desktopWidth 800 |
//      Bitmap caps::desktopHeight 600 |
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
//      Input caps::inputFlags 0x1 |
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
// Sending on RDP Client (4) 327 bytes |
/* 0000 */ "\x03\x00\x01\x47\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x81\x38\x08" // ...G...h....p.8. |
/* 0010 */ "\x00\x00\x00\x96\x8d\x36\x32\x4B\x58\xe7\x2e\xb5\xfe\x81\x03\xf6" // .....62KX....... |
/* 0020 */ "\x1d\xcc\x5f\x7f\x2c\x1a\xa9\x3b\x9f\xa7\xed\x51\x37\xfc\x0c\xe5" // .._.,..;...Q7... |
/* 0030 */ "\xa4\xfd\x15\x05\xd5\xa7\xd3\x68\xf1\xf1\xe7\xa8\x2a\x02\x50\x64" // .......h....*.Pd |
/* 0040 */ "\xfd\x71\xca\x6a\x0a\x75\x02\x12\x84\x97\x64\x45\xb4\xf1\x15\xb9" // .q.j.u....dE.... |
/* 0050 */ "\x44\x50\xe1\x73\x11\x95\x8a\xa3\xac\x30\x6c\x6f\x65\xbb\xf2\x02" // DP.s.....0loe... |
/* 0060 */ "\x97\x22\xea\x5c\x6c\x45\x6c\x62\xf2\x85\x98\xc3\xaf\xbc\x49\xd2" // ."..lElb......I. |
/* 0070 */ "\x33\xae\x52\x57\x81\x9f\x31\x10\x62\xc3\x70\x71\x01\x5a\xa1\xd2" // 3.RW..1.b.pq.Z.. |
/* 0080 */ "\x0e\x30\x99\x85\xf7\x22\x7a\xfd\xca\xaf\x53\xdc\xf9\xeb\x2a\xc9" // .0..."z...S...*. |
/* 0090 */ "\xb0\x72\xf8\xee\xe2\xe8\x15\x6d\x7d\x6e\xd4\x25\x55\x1c\xda\x97" // .r.....m}n.%U... |
/* 00a0 */ "\x57\xf3\xdb\x62\xa6\x33\x25\x75\xf9\xe8\x66\x0c\x91\xb1\x4d\x90" // W..b.3%u..f...M. |
/* 00b0 */ "\x64\xff\xfc\x07\xb4\x26\x64\x38\xfa\xe5\x99\x2c\xe3\x00\x14\x04" // d....&d8...,.... |
/* 00c0 */ "\x60\x8c\xd4\xc5\xb2\x9c\x87\xfe\x9b\xf9\xb6\xe3\x33\x89\x6d\xd9" // `...........3.m. |
/* 00d0 */ "\x6b\x8e\xe8\x8e\x7d\xb9\x2a\xa7\xec\xbb\xcf\xb2\x14\xe2\xe6\xe9" // k...}.*......... |
/* 00e0 */ "\x23\x99\xf2\x48\x6e\x1a\xff\x9f\x9a\xb0\x0d\xe3\x6e\xf4\xc2\x5d" // #..Hn.......n..] |
/* 00f0 */ "\x3a\xca\x37\x04\x2c\xac\x36\x65\xed\x9f\x90\xdc\xa9\xa7\xde\x5a" // :.7.,.6e.......Z |
/* 0100 */ "\x5a\xa3\x81\x24\xa6\xbb\x05\xc6\x6c\x51\xd5\x53\x9d\xf8\xd1\xc3" // Z..$....lQ.S.... |
/* 0110 */ "\xff\x29\xbf\xcf\x24\x28\x8f\x65\xfd\x98\x72\xb8\x90\xb2\x74\x77" // .)..$(.e..r...tw |
/* 0120 */ "\x8d\x2e\xc1\xb2\xec\x3d\x74\x55\xa6\x1a\x9e\x3b\x69\xb8\xba\x3a" // .....=tU...;i..: |
/* 0130 */ "\x71\xd1\xc9\x4c\x55\xef\x4d\x07\x8b\xbf\x6d\xb8\xb1\xc3\xd2\x55" // q..LU.M...m....U |
/* 0140 */ "\x09\xf5\x21\xac\x10\x9f\x5a"                                     // ..!...Z |
// Sent dumped on RDP Client (4) 327 bytes |
// Front::send_demand_active: done |
// Front::incoming: ACTIVATED (new license request) |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 445 bytes |
// /* 0000 */ "\x03\x00\x01\xbd\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\xae\x38" // .......d....p..8 |
// /* 0010 */ "\x00\x00\x00\x8e\x6d\x0e\xfa\xda\xf5\xa6\xa6\x64\xcc\x2a\xdf\xdc" // ....m......d.*.. |
// /* 0020 */ "\xf5\x26\x73\xd7\x63\x23\x11\xdd\x3c\x52\x09\x4b\x48\x68\xed\x5e" // .&s.c#..<R.KHh.^ |
// /* 0030 */ "\x8d\x24\x55\xcb\xda\x4c\xcb\x79\x68\x96\xc0\xac\x06\x2a\x9f\x51" // .$U..L.yh....*.Q |
// /* 0040 */ "\xda\xcb\x5d\xc8\x35\x52\x00\x8c\xe6\xc8\x8c\x60\xc2\xc6\x3a\x03" // ..].5R.....`..:. |
// /* 0050 */ "\x50\x9b\xcd\xfa\x7e\xa6\x31\xef\x90\xc8\x95\xed\x1f\x26\x34\x9e" // P...~.1......&4. |
// /* 0060 */ "\xc7\xa3\x20\xd2\x6d\xeb\x6d\x6c\x64\x3d\x79\x31\x1d\xaf\x93\xa3" // .. .m.mld=y1.... |
// /* 0070 */ "\x1c\xf1\x8b\x66\xc5\x2b\x63\xdf\x1c\xd1\x97\xc8\x8e\xdf\x0a\x58" // ...f.+c........X |
// /* 0080 */ "\xd0\x5c\x22\xef\xf4\x65\x2f\xde\xa9\x2b\x78\xc7\x82\x02\x47\x1b" // .."..e/..+x...G. |
// /* 0090 */ "\x29\x7d\xd0\x5a\x74\xc6\x37\xd9\x22\x02\x1a\xdd\xb8\x38\x9b\x64" // )}.Zt.7."....8.d |
// /* 00a0 */ "\x6b\x93\xae\x06\xc4\xcb\x92\x4f\xf6\xbd\x24\x2d\x03\x39\xef\x6a" // k......O..$-.9.j |
// /* 00b0 */ "\x32\xbc\x9e\x5e\x11\xa1\x9b\x01\xf5\x19\x01\x05\xc7\x4a\xec\x41" // 2..^.........J.A |
// /* 00c0 */ "\xd3\xbd\x58\x09\x0e\x46\xab\xac\xec\x84\x3c\x48\xf5\xb0\xdf\xe8" // ..X..F....<H.... |
// /* 00d0 */ "\x61\xea\xa0\xbb\x00\xa8\x2a\x62\xfa\xb7\xf3\xb9\xc9\xed\xa6\x04" // a.....*b........ |
// /* 00e0 */ "\x22\x29\xc9\x28\x22\x62\x8b\x89\xb0\xd0\xee\xe6\xe0\xb0\x93\x44" // ").("b.........D |
// /* 00f0 */ "\xd0\xae\x8d\x39\x45\x2b\x95\x1b\xab\x21\x3c\x8b\x8e\x4e\x0e\xf1" // ...9E+...!<..N.. |
// /* 0100 */ "\x0f\x7b\x4d\x73\x87\x03\x0f\xb1\x0d\x45\x62\x73\x96\x6f\x14\x4a" // .{Ms.....Ebs.o.J |
// /* 0110 */ "\x8f\x35\xf6\x04\x3a\x06\xce\x6b\x12\x6f\x1c\xa2\x7b\x2a\x0c\x60" // .5..:..k.o..{*.` |
// /* 0120 */ "\xaa\xfc\x7b\x0d\xde\xf6\x77\x3c\x53\x76\x8e\xc4\xd4\x12\xef\x1b" // ..{...w<Sv...... |
// /* 0130 */ "\x4c\x83\x2b\x5f\x3d\xaf\xd4\x57\x75\xd0\x6c\x2a\x29\xaa\x28\x1e" // L.+_=..Wu.l*).(. |
// /* 0140 */ "\xfa\xd0\x23\x22\x1b\x2c\xee\x4d\xfc\xcf\x46\x87\xb6\x8f\x79\x19" // ..#".,.M..F...y. |
// /* 0150 */ "\xff\x2a\x01\xef\xc0\x82\x74\x28\x8b\x31\x38\xa9\xfc\x6e\xb4\x4e" // .*....t(.18..n.N |
// /* 0160 */ "\x8a\xe1\xa6\x9c\xe3\xff\x2a\xf9\xd7\xa1\x74\xad\x99\xec\x2a\x64" // ......*...t...*d |
// /* 0170 */ "\x0a\x14\x72\x11\x4b\xc4\x9b\x8f\x72\xe5\x4c\x9c\xec\x07\x36\xca" // ..r.K...r.L...6. |
// /* 0180 */ "\xab\x57\xaf\xd3\x9f\xba\xda\xfb\x51\xda\xeb\x01\xa9\x87\x16\x6b" // .W......Q......k |
// /* 0190 */ "\xb5\xb0\xdf\xe2\xc8\x9f\x98\xcf\x7f\x69\x42\xbd\xd6\xfa\xa4\x13" // .........iB..... |
// /* 01a0 */ "\x04\xfb\x72\x53\x3a\x82\x52\x7e\x10\x97\x02\xa4\x5e\x5e\xff\xf1" // ..rS:.R~....^^.. |
// /* 01b0 */ "\x20\x75\x44\xff\x6d\x53\x5c\x48\x7a\xa9\xec\x11\xf8"             //  uD.mS.Hz.... |
// Dump done on RDP Client (4) 445 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=38 |
// Front::incoming: Received CONFIRMACTIVEPDU |
// Front::process_confirm_active |
// Front::process_confirm_active: lengthSourceDescriptor = 6 |
// Front::process_confirm_active: lengthCombinedCapabilities = 396 |
// Front::capability 0 / 14 |
// Front::process_confirm_active: Receiving from client General caps (24 bytes) |
//      General caps::major 1 |
//      General caps::minor 3 |
//      General caps::protocol 512 |
//      General caps::pad2octetA 0 |
//      General caps::compression type 0 |
//      General caps::extra flags 0 |
//      General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED no |
//      General caps::extraflags:LONG_CREDENTIALS_SUPPORTED no |
//      General caps::extraflags:AUTORECONNECT_SUPPORTED no |
//      General caps::extraflags:ENC_SALTED_CHECKSUM no |
//      General caps::extraflags:NO_BITMAP_COMPRESSION_HDR no |
//      General caps::updateCapability 0 |
//      General caps::remoteUnshare 0 |
//      General caps::compressionLevel 0 |
//      General caps::refreshRectSupport 0 |
//      General caps::suppressOutputSupport 0 |
// Front::capability 1 / 14 |
// Front::process_confirm_active: Receiving from client Bitmap caps (28 bytes) |
//      Bitmap caps::preferredBitsPerPixel 24 |
//      Bitmap caps::receive1BitPerPixel 1 |
//      Bitmap caps::receive4BitsPerPixel 1 |
//      Bitmap caps::receive8BitsPerPixel 1 |
//      Bitmap caps::desktopWidth 800 |
//      Bitmap caps::desktopHeight 600 |
//      Bitmap caps::pad2octets 0 |
//      Bitmap caps::desktopResizeFlag 1 (yes) |
//      Bitmap caps::bitmapCompressionFlag 1 yes |
//      Bitmap caps::highColorFlags 0 |
//      Bitmap caps::drawingFlags 0 |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY no |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_COLOR_SUBSAMPLING no |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_SKIP_ALPHA no |
//      Bitmap caps::multipleRectangleSupport 1 |
//      Bitmap caps::pad2octetsB 0 |
// Front::capability 2 / 14 |
// Front::process_confirm_active: Receiving from client Order caps (88 bytes) |
//      Order caps::terminalDescriptor 0 |
//      Order caps::pad4octetsA 0 |
//      Order caps::desktopSaveXGranularity 1 |
//      Order caps::desktopSaveYGranularity 20 |
//      Order caps::pad2octetsA 0 |
//      Order caps::maximumOrderLevel 1 |
//      Order caps::numberFonts 327 |
//      Order caps::orderFlags 0x2A |
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
//      Order caps::orderSupport[SAVEBITMAP] 1 |
//      Order caps::orderSupport[WTEXTOUT] 0 |
//      Order caps::orderSupport[MEMBLT_V2] 1 |
//      Order caps::orderSupport[MEM3BLT_V2] 1 |
//      Order caps::orderSupport[MULTIDSTBLT] 0 |
//      Order caps::orderSupport[MULTIPATBLT] 0 |
//      Order caps::orderSupport[MULTISCRBLT] 0 |
//      Order caps::orderSupport[MULTIOPAQUERECT] 0 |
//      Order caps::orderSupport[FAST_INDEX] 0 |
//      Order caps::orderSupport[POLYGON_SC] 1 |
//      Order caps::orderSupport[POLYGON_CB] 1 |
//      Order caps::orderSupport[POLYLINE] 1 |
//      Order caps::orderSupport[UnusedIndex7] 0 |
//      Order caps::orderSupport[FAST_GLYPH] 0 |
//      Order caps::orderSupport[ELLIPSE_SC] 1 |
//      Order caps::orderSupport[ELLIPSE_CB] 1 |
//      Order caps::orderSupport[GLYPH] 1 |
//      Order caps::orderSupport[GLYPH_WEXTTEXTOUT] 0 |
//      Order caps::orderSupport[GLYPH_WLONGTEXTOUT] 0 |
//      Order caps::orderSupport[GLYPH_WLONGEXTTEXTOUT] 0 |
//      Order caps::orderSupport[UnusedIndex11] 0 |
//      Order caps::textFlags 1697 |
//      Order caps::orderSupportExFlags 0x0 |
//      Order caps::pad4octetsB 0 |
//      Order caps::desktopSaveSize 230400 |
//      Order caps::pad2octetsC 0 |
//      Order caps::pad2octetsD 0 |
//      Order caps::textANSICodePage 1252 |
//      Order caps::pad2octetsE 0 |
// Front::capability 3 / 14 |
// Front::process_confirm_active: Receiving from client BitmapCache caps (40 bytes) |
//      BitmapCache caps::pad1 0 |
//      BitmapCache caps::pad2 0 |
//      BitmapCache caps::pad3 0 |
//      BitmapCache caps::pad4 0 |
//      BitmapCache caps::pad5 0 |
//      BitmapCache caps::pad6 0 |
//      BitmapCache caps::cache0Entries 600 |
//      BitmapCache caps::cache0MaximumCellSize 768 |
//      BitmapCache caps::cache1Entries 300 |
//      BitmapCache caps::cache1MaximumCellSize 3072 |
//      BitmapCache caps::cache2Entries 262 |
//      BitmapCache caps::cache2MaximumCellSize 12288 |
// Front::capability 4 / 14 |
// Front::process_confirm_active: Receiving from client Pointer caps (8 bytes) |
//      Pointer caps::colorPointerFlag 0 |
//      Pointer caps::colorPointerCacheSize 20 |
// Front::capability 5 / 14 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_COLORCACHE |
// Front::capability 6 / 14 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_ACTIVATION |
// Front::capability 7 / 14 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_CONTROL |
// Front::capability 8 / 14 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_SHARE |
// Front::capability 9 / 14 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_BRUSH |
// Front::process_confirm_active: Receiving from client BrushCache caps (8 bytes) |
//      BrushCacheCaps caps::brushSupportLevel 1 |
// Front::capability 10 / 14 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_INPUT |
// Front::capability 11 / 14 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_SOUND |
// Front::capability 12 / 14 |
// Front::capability 13 / 14 |
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
//      GlyphCache caps::GlyphSupportLevel 2 |
//      GlyphCache caps::pad2octets 0 |
// Front::process_confirm_active: done p=0x7f9039eaec63 end=0x7f9039eaec63 |
// Front::reset: use_bitmap_comp=1 |
// Front::reset: use_compact_packets=0 |
// Front::reset: bitmap_cache_version=0 |
// ◢ In src/core/RDP/mppc.cpp:121 |
// Front::reset: invalid RDP compression code 0xffffffff |
// Front::incoming: Received CONFIRMACTIVEPDU done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 155 bytes |
// /* 0000 */ "\x03\x00\x00\x31\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x22\x08" // ...1...d....p.". |
// /* 0010 */ "\x00\x00\x00\x10\xa6\xc2\xb2\x31\x3f\xdf\xcd\x82\xb6\x32\x47\xb9" // .......1?....2G. |
// /* 0020 */ "\x52\x8c\xa7\x2e\x3c\x5f\xcf\xc2\x54\x23\xc2\x2c\x30\x57\x29\x31" // R...<_..T#.,0W)1 |
// /* 0030 */ "\x11\x03\x00\x00\x35\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x26" // ....5...d....p.& |
// /* 0040 */ "\x08\x00\x00\x00\x40\x98\xcb\xc0\x5e\xbf\x5b\xa3\xc5\xa0\xcc\xfe" // ....@...^.[..... |
// /* 0050 */ "\x9e\x1f\xc3\x2a\x64\x16\x96\x4e\x32\x69\x01\xf1\x38\x7f\xfb\x40" // ...*d..N2i..8..@ |
// /* 0060 */ "\xc8\x2b\xd7\xc3\xfc\xed\x03\x00\x00\x35\x02\xf0\x80\x64\x00\x00" // .+.......5...d.. |
// /* 0070 */ "\x03\xeb\x70\x80\x26\x08\x00\x00\x00\xa9\x77\x29\x66\x1e\x8c\x8e" // ..p.&.....w)f... |
// /* 0080 */ "\x2f\x24\x01\x8b\x11\xc6\x97\xc4\xb3\x6b\x7e\xc5\xc0\x36\x0e\xfc" // /$.......k~..6.. |
// /* 0090 */ "\x9c\x02\x52\xd6\xc1\x8a\x70\x4b\x33\x4f\x47"                     // ..R...pK3OG |
// Dump done on RDP Client (4) 155 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=8 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=31 sdata_in.len=8 sdata_in.compressedLen=0 remains=0 payload_len=4 |
// Front::process_data: PDUTYPE2_SYNCHRONIZE |
// Front::process_data: PDUTYPE2_SYNCHRONIZE messageType=1 controlId=1002 |
// Front::send_synchronize |
// Sec clear payload to send: |
// /* 0000 */ 0x16, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x16, 0x00, 0x1f, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x01, 0x00, 0xea, 0x03,                                                              // ...... |
// Sending on RDP Client (4) 48 bytes |
/* 0000 */ "\x03\x00\x00\x30\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x22\x08\x00" // ...0...h....p".. |
/* 0010 */ "\x00\x00\x16\x44\x24\x4d\xc8\x31\x13\x0e\x3a\xff\x9c\xe0\x53\xe6" // ...D$M.1..:...S. |
/* 0020 */ "\x94\xb4\xdd\xec\x9f\x68\xa9\xcf\xc8\xba\x09\xc8\x62\x85\xed\xc4" // .....h......b... |
// Sent dumped on RDP Client (4) 48 bytes |
// Front::send_synchronize: done |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=8 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=20 sdata_in.len=12 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// Front::process_data: PDUTYPE2_CONTROL |
// Front::send_control: action=4 |
// Sec clear payload to send: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x1a, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00,                                      // .......... |
// Sending on RDP Client (4) 52 bytes |
/* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x26\x08\x00" // ...4...h....p&.. |
/* 0010 */ "\x00\x00\x03\x06\x28\xcd\x96\x27\x3c\x3a\xa6\xbf\x60\xbe\x98\x31" // ....(..'<:..`..1 |
/* 0020 */ "\x5d\x52\xb5\x97\xb8\xd8\xa4\xa8\x59\x26\x7f\x08\x93\xf0\xef\x01" // ]R......Y&...... |
/* 0030 */ "\xd0\x88\x1c\x47"                                                 // ...G |
// Sent dumped on RDP Client (4) 52 bytes |
// Front::send_control: done. action=4 |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=8 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=20 sdata_in.len=12 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// Front::process_data: PDUTYPE2_CONTROL |
// Front::send_control: action=2 |
// Sec clear payload to send: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x1a, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00,                                      // .......... |
// Sending on RDP Client (4) 52 bytes |
/* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x26\x08\x00" // ...4...h....p&.. |
/* 0010 */ "\x00\x00\x17\x13\x3a\xb7\xd6\x48\x28\xb8\x09\x96\x95\x99\x57\x52" // ....:..H(.....WR |
/* 0020 */ "\x3d\x59\x57\xec\xcd\x1e\x09\x02\x61\x7b\x45\x89\x29\xdb\x8c\x86" // =YW.....a{E.)... |
/* 0030 */ "\x85\xb2\x27\xeb"                                                 // ..'. |
// Sent dumped on RDP Client (4) 52 bytes |
// Front::send_control: done. action=2 |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 61 bytes |
// /* 0000 */ "\x03\x00\x00\x3d\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x2e\x08" // ...=...d....p... |
// /* 0010 */ "\x00\x00\x00\x4d\xeb\x8a\x6c\x88\x1f\xae\x51\x67\x85\x4e\xe5\x48" // ...M..l...Qg.N.H |
// /* 0020 */ "\x00\x15\xda\x85\x9b\x42\x6a\xc9\x41\xe7\xb4\x75\xb3\xc5\xa7\xbb" // .....Bj.A..u.... |
// /* 0030 */ "\x5c\xc9\x67\x44\xbe\x25\xe7\x9b\xcf\x02\x78\x45\xe9"             // ..gD.%....xE. |
// Dump done on RDP Client (4) 61 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=8 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=28 sdata_in.len=20 sdata_in.compressedLen=0 remains=0 payload_len=16 |
// Front::process_data: PDUTYPE2_INPUT num_events=1 |
// Front::process_data: Slow-Path INPUT_EVENT_SYNC eventTime=0 toggleFlags=0x0000 |
// Front::process_data: (Slow-Path) Synchronize Event toggleFlags=0x0 |
// Front::process_data: PDUTYPE2_INPUT done |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 106 bytes |
// /* 0000 */ "\x03\x00\x00\x35\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x26\x08" // ...5...d....p.&. |
// /* 0010 */ "\x00\x00\x00\xea\xf7\x1e\xf6\x2f\x95\x1f\xe9\x94\x47\xb0\x49\xc1" // ......./....G.I. |
// /* 0020 */ "\x04\x0c\x06\x16\x31\x6c\x77\x66\x8f\xdd\x05\x4f\x88\x90\x41\x27" // ....1lwf...O..A' |
// /* 0030 */ "\xe8\x89\x8c\x2d\x53\x03\x00\x00\x35\x02\xf0\x80\x64\x00\x00\x03" // ...-S...5...d... |
// /* 0040 */ "\xeb\x70\x80\x26\x08\x00\x00\x00\xa5\xf6\xf6\x21\x2d\x17\x20\x8a" // .p.&.......!-. . |
// /* 0050 */ "\x80\xdd\xf5\xaa\x91\xff\xeb\xb1\x94\x9f\x31\xbf\x02\xdc\x7e\x3f" // ..........1...~? |
// /* 0060 */ "\x4d\x7b\x11\x5b\x7b\xe8\x40\x1c\x94\x5e"                         // M{.[{.@..^ |
// Dump done on RDP Client (4) 106 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=8 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=39 sdata_in.len=12 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// Front::process_data: PDUTYPE2_FONTLIST |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=8 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=39 sdata_in.len=12 sdata_in.compressedLen=0 remains=0 payload_len=8 |
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
// Sending on RDP Client (4) 217 bytes |
/* 0000 */ "\x03\x00\x00\xd9\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x80\xca\x08" // .......h....p... |
/* 0010 */ "\x00\x00\x00\xd7\x0f\x00\x1a\x05\x36\x63\x2f\x69\xed\xdf\x60\xe2" // ........6c/i..`. |
/* 0020 */ "\xd3\x9b\x00\x27\x97\x6f\x80\xaf\xb9\x1a\x49\x1d\xe9\x36\x86\xd8" // ...'.o....I..6.. |
/* 0030 */ "\xab\xc6\x00\x30\xe2\xb6\xf0\xd5\x56\xea\x4f\x1f\xb7\xd3\x3e\xfc" // ...0....V.O...>. |
/* 0040 */ "\x1e\xca\xde\xbf\xaa\x0f\xf7\xe7\xb8\xaf\x9f\xd6\x66\x6b\x6c\x39" // ............fkl9 |
/* 0050 */ "\xb5\x9f\xe2\xed\x75\x0a\x41\x9c\xb9\xeb\xd6\x0c\xaa\x37\xcf\xcf" // ....u.A......7.. |
/* 0060 */ "\xde\xd4\x87\xaf\xf8\x7a\xee\x09\x8b\x03\x34\x5e\xf2\xcc\xb2\xb0" // .....z....4^.... |
/* 0070 */ "\x12\x53\x18\xae\x8c\x4e\x29\x45\x0b\xe7\xd0\x56\x98\x6f\xdf\xc9" // .S...N)E...V.o.. |
/* 0080 */ "\x8d\x03\x6e\xaa\xbd\x90\x7f\x15\xab\x1b\x22\x30\xc1\x19\xf1\x69" // ..n......."0...i |
/* 0090 */ "\xee\xca\x3c\x2c\xee\x58\xa4\x8d\xcf\xd4\xdd\x64\x0b\xba\x18\x81" // ..<,.X.....d.... |
/* 00a0 */ "\xd5\x90\xb0\x70\x13\x97\xe5\xe6\xbd\x2b\xc4\xa9\x9a\xdf\x01\xab" // ...p.....+...... |
/* 00b0 */ "\x53\x4e\x4f\xb9\x1f\x9d\x36\xd4\x52\xdc\xa1\xb0\x05\x94\xb9\xa0" // SNO...6.R....... |
/* 00c0 */ "\xe2\xcd\xe6\x97\xf8\xd2\x5b\x76\xf0\x0f\x03\x5b\xcf\x86\xbb\x0d" // ......[v...[.... |
/* 00d0 */ "\x0e\x77\xa5\x6a\x68\x07\xca\xe7\xe2"                             // .w.jh.... |
// Sent dumped on RDP Client (4) 217 bytes |
// Front::send_fontmap: done |
// Front::send_data_update_sync |
// send_server_update: fastpath_support=no compression_support=no shareId=65538 encryptionLevel=1 initiator=0 type=3 data_extra=0 |
// Sec clear payload to send: |
// /* 0000 */ 0x16, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x16, 0x00, 0x02, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,                                                              // ...... |
// Sending on RDP Client (4) 48 bytes |
/* 0000 */ "\x03\x00\x00\x30\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x22\x08\x00" // ...0...h....p".. |
/* 0010 */ "\x00\x00\x02\xb2\xe7\x21\x3a\x51\x7c\x94\xb2\x02\x5d\xee\xb3\xbe" // .....!:Q|...]... |
/* 0020 */ "\x5e\x31\xfa\x7e\x28\x70\xde\x67\x59\xbf\xb5\x5e\x37\x02\xdb\x08" // ^1.~(p.gY..^7... |
} /* end outdata */;

const char indata[] = /* NOLINT */
{
// Listen: binding socket 3 on 0.0.0.0:3389 |
// Listen: listening on socket 3 |
// SocketTransport: recv_timeout=1000 |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 43 bytes |
/* 0000 */ "\x03\x00\x00\x2b\x26\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" // ...+&......Cooki |
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x72\x7a\x68\x6f" // e: mstshash=rzho |
/* 0020 */ "\x75\x0d\x0a\x01\x00\x08\x00\x03\x00\x00\x00"                     // u.......... |
// Dump done on RDP Client (4) 43 bytes |
// Front::incoming: CONNECTION_INITIATION |
// Front::incoming: receiving x224 request PDU |
// CR Recv: PROTOCOL TLS |
// CR Recv: PROTOCOL HYBRID |
// Front::incoming: Fallback to legacy security protocol |
// Front::incoming: sending x224 connection confirm PDU |
// -----------------> Front::incoming: TLS Support not Enabled |
// Sending on RDP Client (4) 11 bytes |
// /* 0000 */ "\x03\x00\x00\x0b\x06\xd0\x00\x00\x00\x00\x00"                     // ........... |
// Sent dumped on RDP Client (4) 11 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 446 bytes |
/* 0000 */ "\x03\x00\x01\xbe\x02\xf0\x80\x7f\x65\x82\x01\xb2\x04\x01\x01\x04" // ........e....... |
/* 0010 */ "\x01\x01\x01\x01\xff\x30\x20\x02\x02\x00\x22\x02\x02\x00\x02\x02" // .....0 ..."..... |
/* 0020 */ "\x02\x00\x00\x02\x02\x00\x01\x02\x02\x00\x00\x02\x02\x00\x01\x02" // ................ |
/* 0030 */ "\x02\xff\xff\x02\x02\x00\x02\x30\x20\x02\x02\x00\x01\x02\x02\x00" // .......0 ....... |
/* 0040 */ "\x01\x02\x02\x00\x01\x02\x02\x00\x01\x02\x02\x00\x00\x02\x02\x00" // ................ |
/* 0050 */ "\x01\x02\x02\x04\x20\x02\x02\x00\x02\x30\x20\x02\x02\xff\xff\x02" // .... ....0 ..... |
/* 0060 */ "\x02\xfc\x17\x02\x02\xff\xff\x02\x02\x00\x01\x02\x02\x00\x00\x02" // ................ |
/* 0070 */ "\x02\x00\x01\x02\x02\xff\xff\x02\x02\x00\x02\x04\x82\x01\x3f\x00" // ..............?. |
/* 0080 */ "\x05\x00\x14\x7c\x00\x01\x81\x36\x00\x08\x00\x10\x00\x01\xc0\x00" // ...|...6........ |
/* 0090 */ "\x44\x75\x63\x61\x81\x28\x01\xc0\xd8\x00\x04\x00\x08\x00\x20\x03" // Duca.(........ . |
/* 00a0 */ "\x58\x02\x01\xca\x03\xaa\x0c\x04\x00\x00\x28\x0a\x00\x00\x72\x00" // X.........(...r. |
/* 00b0 */ "\x7a\x00\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // z.h............. |
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00" // ................ |
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\xca\x01\x00\x00\x00" // ................ |
/* 0120 */ "\x00\x00\x18\x00\x07\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\xc0" // ................ |
/* 0170 */ "\x0c\x00\x09\x00\x00\x00\x00\x00\x00\x00\x02\xc0\x0c\x00\x03\x00" // ................ |
/* 0180 */ "\x00\x00\x00\x00\x00\x00\x03\xc0\x38\x00\x04\x00\x00\x00\x63\x6c" // ........8.....cl |
/* 0190 */ "\x69\x70\x72\x64\x72\x00\xc0\xa0\x00\x00\x72\x64\x70\x73\x6e\x64" // iprdr.....rdpsnd |
/* 01a0 */ "\x00\x00\xc0\x00\x00\x00\x73\x6e\x64\x64\x62\x67\x00\x00\xc0\x00" // ......snddbg.... |
/* 01b0 */ "\x00\x00\x72\x64\x70\x64\x72\x00\x00\x00\x80\x80\x00\x00"         // ..rdpdr....... |
// Dump done on RDP Client (4) 446 bytes |
// Front::incoming: Basic Settings Exchange |
// GCC::UserData tag=c001 length=216 |
// Front::incoming: Received from Client GCC User Data CS_CORE (216 bytes) |
// cs_core::length [00d8] |
// cs_core::version [80004] RDP 5.0, 5.1, 5.2, and 6.0 clients) |
// cs_core::desktopWidth  = 800 |
// cs_core::desktopHeight = 600 |
// cs_core::colorDepth    = [ca01] [RNS_UD_COLOR_8BPP] superseded by postBeta2ColorDepth |
// cs_core::SASSequence   = [aa03] [RNS_UD_SAS_DEL] |
// cs_core::keyboardLayout= 040c |
// cs_core::clientBuild   = 2600 |
// cs_core::clientName    = rzh |
// cs_core::keyboardType  = [0004] IBM enhanced (101-key or 102-key) keyboard |
// cs_core::keyboardSubType      = [0000] OEM code |
// cs_core::keyboardFunctionKey  = 12 function keys |
// cs_core::imeFileName    = |
// cs_core::postBeta2ColorDepth  = [ca01] [8 bpp] |
// cs_core::clientProductId = 1 |
// cs_core::serialNumber = 0 |
// cs_core::highColorDepth  = [0018] [24-bit RGB mask] |
// cs_core::supportedColorDepths  = [0007] [24/16/15/] |
// cs_core::earlyCapabilityFlags  = [0001] |
// cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_ERRINFO_PDU |
// cs_core::clientDigProductId=[00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000] |
// cs_core::connectionType = <unknown> |
// cs_core::pad1octet = 0 |
// cs_core::serverSelectedProtocol = 0 |
// GCC::UserData tag=c004 length=12 |
// Front::incoming: Receiving from Client GCC User Data CS_CLUSTER (12 bytes) |
// cs_cluster::flags [0009] |
// cs_cluster::flags::REDIRECTION_SUPPORTED |
// cs_cluster::flags::redirectionVersion = 3 |
// GCC::UserData tag=c002 length=12 |
// Front::incoming: Received from Client GCC User Data CS_SECURITY (12 bytes) |
// CSSecGccUserData::encryptionMethods 3 |
// CSSecGccUserData::extEncryptionMethods 0 |
// GCC::UserData tag=c003 length=56 |
// Front::incoming: Received from Client GCC User Data CS_NET (56 bytes) |
// cs_net::channelCount   = 4 |
// cs_net::channel 'cliprdr' [1004] |
// cs_net::channel 'rdpsnd' [1005] |
// cs_net::channel 'snddbg' [1006] |
// cs_net::channel 'rdpdr' [1007] |
// Front::incoming: Sending to client GCC User Data SC_CORE (8 bytes) |
// sc_core::version [80004] RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 servers) |
// Front::incoming: Sending to client GCC User Data SC_NET (16 bytes) |
// sc_net::MCSChannelId   = 1003 |
// sc_net::channelCount   = 4 |
// sc_net::channel[1004]::id = 1004 |
// sc_net::channel[1005]::id = 1005 |
// sc_net::channel[1006]::id = 1006 |
// sc_net::channel[1007]::id = 1007 |
// Front::incoming: Sending to client GCC User Data SC_SECURITY (236 bytes) |
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
// Sending on RDP Client (4) 333 bytes |
// /* 0000 */ "\x03\x00\x01\x4d\x02\xf0\x80\x7f\x66\x82\x01\x41\x0a\x01\x00\x02" // ...M....f..A.... |
// /* 0010 */ "\x01\x00\x30\x1a\x02\x01\x22\x02\x01\x03\x02\x01\x00\x02\x01\x01" // ..0..."......... |
// /* 0020 */ "\x02\x01\x00\x02\x01\x01\x02\x03\x00\xff\xf8\x02\x01\x02\x04\x82" // ................ |
// /* 0030 */ "\x01\x1b\x00\x05\x00\x14\x7c\x00\x01\x2a\x14\x76\x0a\x01\x01\x00" // ......|..*.v.... |
// /* 0040 */ "\x01\xc0\x00\x4d\x63\x44\x6e\x81\x04\x01\x0c\x08\x00\x04\x00\x08" // ...McDn......... |
// /* 0050 */ "\x00\x03\x0c\x10\x00\xeb\x03\x04\x00\xec\x03\xed\x03\xee\x03\xef" // ................ |
// /* 0060 */ "\x03\x02\x0c\xec\x00\x01\x00\x00\x00\x01\x00\x00\x00\x20\x00\x00" // ............. .. |
// /* 0070 */ "\x00\xb8\x00\x00\x00\xb8\x6c\xda\xa6\xf0\xf6\x30\x8d\xa8\x16\xa6" // ......l....0.... |
// /* 0080 */ "\x6e\xe0\xc3\xe5\xcc\x98\x76\xdd\xf5\xd0\x26\x74\x5f\x88\x4c\xc2" // n.....v...&t_.L. |
// /* 0090 */ "\x50\xc0\xdf\xc9\x50\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00" // P...P........... |
// /* 00a0 */ "\x00\x06\x00\x5c\x00\x52\x53\x41\x31\x48\x00\x00\x00\x00\x02\x00" // .....RSA1H...... |
// /* 00b0 */ "\x00\x3f\x00\x00\x00\x01\x00\x01\x00\x67\xab\x0e\x6a\x9f\xd6\x2b" // .?.......g..j..+ |
// /* 00c0 */ "\xa3\x32\x2f\x41\xd1\xce\xee\x61\xc3\x76\x0b\x26\x11\x70\x48\x8a" // .2/A...a.v.&.pH. |
// /* 00d0 */ "\x8d\x23\x81\x95\xa0\x39\xf7\x5b\xaa\x3e\xf1\xed\xb8\xc4\xee\xce" // .#...9.[.>...... |
// /* 00e0 */ "\x5f\x6a\xf5\x43\xce\x5f\x60\xca\x6c\x06\x75\xae\xc0\xd6\xa4\x0c" // _j.C._`.l.u..... |
// /* 00f0 */ "\x92\xa4\xc6\x75\xea\x64\xb2\x50\x5b\x00\x00\x00\x00\x00\x00\x00" // ...u.d.P[....... |
// /* 0100 */ "\x00\x08\x00\x48\x00\x6a\x41\xb1\x43\xcf\x47\x6f\xf1\xe6\xcc\xa1" // ...H.jA.C.Go.... |
// /* 0110 */ "\x72\x97\xd9\xe1\x85\x15\xb3\xc2\x39\xa0\xa6\x26\x1a\xb6\x49\x01" // r.......9..&..I. |
// /* 0120 */ "\xfa\xa6\xda\x60\xd7\x45\xf7\x2c\xee\xe4\x8e\x64\x2e\x37\x49\xf0" // ...`.E.,...d.7I. |
// /* 0130 */ "\x4c\x94\x6f\x08\xf5\x63\x4c\x56\x29\x55\x5a\x63\x41\x2c\x20\x65" // L.o..cLV)UZcA, e |
// /* 0140 */ "\x95\x99\xb1\x15\x7c\x00\x00\x00\x00\x00\x00\x00\x00"             // ....|........ |
// Sent dumped on RDP Client (4) 333 bytes |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 20 bytes |
/* 0000 */ "\x03\x00\x00\x0c\x02\xf0\x80\x04\x00\x01\x00\x01\x03\x00\x00\x08" // ................ |
/* 0010 */ "\x02\xf0\x80\x28"                                                 // ...( |
// Dump done on RDP Client (4) 20 bytes |
// Front::incoming: Channel Connection |
// Front::incoming: Recv MCS::ErectDomainRequest |
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
// Recv done on RDP Client (4) 95 bytes |
/* 0000 */ "\x03\x00\x00\x5f\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x50\x01" // ..._...d....p.P. |
/* 0010 */ "\x00\x00\x00\x48\x00\x00\x00\x62\x2f\xfb\xc8\x0f\x98\x75\x6c\xf1" // ...H...b/....ul. |
/* 0020 */ "\xf8\x75\x54\x91\xdf\x6f\xf1\x62\x3a\x7e\xbe\x0d\xac\xd4\xd0\xe6" // .uT..o.b:~...... |
/* 0030 */ "\x2a\xd0\x91\x83\x51\xeb\x05\x9b\x60\x23\x1c\xe3\x38\x95\x92\xc4" // *...Q...`#..8... |
/* 0040 */ "\x28\xea\x3f\xda\xea\xe1\x10\xb3\x35\xab\x75\x4b\x15\x00\x6c\xeb" // (.?.....5.uK..l. |
/* 0050 */ "\x7b\xf9\xf1\x9a\xf9\x3f\x4e\x00\x00\x00\x00\x00\x00\x00\x00"     // {....?N........ |
// Dump done on RDP Client (4) 95 bytes |
// Front::incoming: RDP Security Commencement |
// Front::incoming: Legacy RDP mode: expecting exchange packet |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 337 bytes |
/* 0000 */ "\x03\x00\x01\x51\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\x42\x48" // ...Q...d....p.BH |
/* 0010 */ "\x00\x00\x00\x16\x58\xe5\x9d\x6c\x04\x56\xb9\xc2\x52\xab\xe4\xf2" // ....X..l.V..R... |
/* 0020 */ "\xc8\xb9\x1b\xf5\x71\x62\x81\x26\xe1\xab\xc7\xb0\x4b\xdc\x1d\x37" // ....qb.&....K..7 |
/* 0030 */ "\x1f\xfa\x48\xb0\xa6\x25\x56\x02\x68\x61\x75\x30\x1d\x05\xcd\xbd" // ..H..%V.hau0.... |
/* 0040 */ "\x14\xdf\x6c\x7d\xef\x84\x77\xa2\x1f\xcb\xce\x0c\x2b\x9f\xec\x57" // ..l}..w.....+..W |
/* 0050 */ "\xb2\x7a\xc6\xe0\x78\xdd\x06\xe8\x77\xec\x01\xc8\xe6\x6e\x73\x1a" // .z..x...w....ns. |
/* 0060 */ "\x3e\x9b\xa1\xff\x6b\x40\xb5\x18\x88\x42\xf5\x66\x13\x38\x36\x6c" // >...k@...B.f.86l |
/* 0070 */ "\x72\x41\xf3\x49\xe9\x46\xec\xbb\x3d\x0d\x1d\x06\xb6\x2a\xdd\x87" // rA.I.F..=....*.. |
/* 0080 */ "\xb6\x7c\xe9\xea\x14\x91\xbd\x32\xdb\xdf\xdc\xa2\xa9\xd7\xb7\x11" // .|.....2........ |
/* 0090 */ "\x50\x10\x92\x0a\x7d\x8b\x5a\x57\x49\xbd\x1b\xe3\xf3\x77\x98\xd1" // P...}.ZWI....w.. |
/* 00a0 */ "\x1c\x0e\xb3\xf3\x1c\xea\x1d\x8d\x18\xe7\xd6\x4c\xd6\xa5\xc9\x53" // ...........L...S |
/* 00b0 */ "\xd9\x8c\x38\xe0\xdb\x68\x36\xb9\x8a\x50\x78\x1f\x76\x39\x53\x51" // ..8..h6..Px.v9SQ |
/* 00c0 */ "\xf4\xfb\xf3\x63\x5f\x89\x60\x91\x29\x34\x3f\x2b\x1f\x10\x91\x63" // ...c_.`.)4?+...c |
/* 00d0 */ "\x39\xd8\x2c\x6f\x31\x74\x98\x1c\x73\xc0\x65\x01\x4c\xcf\x5b\xbe" // 9.,o1t..s.e.L.[. |
/* 00e0 */ "\xbb\x2f\x80\xdf\x79\x7e\x18\x84\x9b\x18\x90\x0a\xee\x2e\x34\x4c" // ./..y~........4L |
/* 00f0 */ "\xcc\x8f\x52\x35\xbb\x01\x7b\x34\xf3\xbc\x90\xaf\x00\x28\x66\x52" // ..R5..{4.....(fR |
/* 0100 */ "\x88\xc5\xbb\x0d\x27\x89\x75\x77\xec\x3a\x3d\x5b\xc7\x09\x1f\xd3" // ....'.uw.:=[.... |
/* 0110 */ "\x0a\x99\xe8\x25\x04\xbf\xfd\xff\x83\xb1\xc4\x6d\xd7\xa2\xca\x75" // ...%.......m...u |
/* 0120 */ "\xc4\xcd\x4f\xe3\x28\xa7\x61\x53\x9a\x9b\xd9\xc7\x46\x37\x77\xb7" // ..O.(.aS....F7w. |
/* 0130 */ "\x70\xd0\xdf\xf2\x1f\xfe\x8c\xcd\x03\x12\xd7\xa8\x77\x6a\x2a\xa3" // p...........wj*. |
/* 0140 */ "\xa0\xa2\xb1\xeb\x0c\x72\x5a\x2e\xbd\x89\x69\x47\x50\x2c\xec\xde" // .....rZ...iGP,.. |
/* 0150 */ "\x75"                                                             // u |
// Dump done on RDP Client (4) 337 bytes |
// Front::incoming: Secure Settings Exchange |
// RDP-5 Style logon |
// Receiving from client InfoPacket |
// InfoPacket::CodePage 0 |
// InfoPacket::flags 0x133 |
// InfoPacket::flags:INFO_MOUSE yes |
// InfoPacket::flags:INFO_DISABLECTRLALTDEL yes |
// InfoPacket::flags:INFO_AUTOLOGON no |
// InfoPacket::flags:INFO_UNICODE yes |
// InfoPacket::flags:INFO_MAXIMIZESHELL  yes |
// InfoPacket::flags:INFO_LOGONNOTIFY no |
// InfoPacket::flags:INFO_COMPRESSION no |
// InfoPacket::flags:CompressionTypeMask no |
// InfoPacket::flags:INFO_ENABLEWINDOWSKEY  yes |
// InfoPacket::flags:INFO_REMOTECONSOLEAUDIO no |
// InfoPacket::flags:INFO_FORCE_ENCRYPTED_CS_PDU no |
// InfoPacket::flags:INFO_RAIL no |
// InfoPacket::flags:INFO_LOGONERRORS no |
// InfoPacket::flags:INFO_MOUSE_HAS_WHEEL no |
// InfoPacket::flags:INFO_PASSWORD_IS_SC_PIN no |
// InfoPacket::flags:INFO_NOAUDIOPLAYBACK no |
// InfoPacket::flags:INFO_USING_SAVED_CREDS no |
// InfoPacket::flags:INFO_AUDIOCAPTURE no |
// InfoPacket::flags:INFO_VIDEO_DISABLE no |
// InfoPacket::flags:INFO_HIDEF_RAIL_SUPPORTED no |
// InfoPacket::cbDomain 2 |
// InfoPacket::cbUserName 12 |
// InfoPacket::cbPassword 2 |
// InfoPacket::cbAlternateShell 2 |
// InfoPacket::cbWorkingDir 2 |
// InfoPacket::Domain |
// InfoPacket::UserName rzhou |
// InfoPacket::Password <hidden> |
// InfoPacket::AlternateShell |
// InfoPacket::WorkingDir |
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2 |
// InfoPacket::ExtendedInfoPacket::cbClientAddress 24 |
// InfoPacket::ExtendedInfoPacket::clientAddress 10.10.43.33 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 60 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\WINNT\System32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 0 |
// InfoPacket::ExtendedInfoPacket::performanceFlags 0x27 |
// InfoPacket::ExtendedInfoPacket::PERF_DISABLE_WALLPAPER |
// InfoPacket::ExtendedInfoPacket::PERF_DISABLE_FULLWINDOWDRAG |
// InfoPacket::ExtendedInfoPacket::PERF_DISABLE_MENUANIMATIONS |
// InfoPacket::ExtendedInfoPacket::PERF_DISABLE_CURSOR_SHADOW |
// InfoPacket::ExtendedInfoPacket::cbAutoReconnectLen 0 |
// InfoPacket::ExtendedInfoPacket::reserved1 0 |
// InfoPacket::ExtendedInfoPacket::reserved2 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::Bias 4294967236 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardName GTB, normaltid |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMonth 10 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDay 5 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wHour 3 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardBias 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightName GTB, sommartid |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMonth 3 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDay 5 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wHour 2 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightBias 4294967236 |
// client info: performance flags before=0x00000027 after=0x0000002F default=0x00000080 present=0x00000028 not-present=0x00000000 |
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
// Recv done on RDP Client (4) 157 bytes |
/* 0000 */ "\x03\x00\x00\x9d\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x8e\x80" // .......d....p... |
/* 0010 */ "\x00\x00\x00\x13\x03\x8a\x00\x01\x00\x00\x00\x00\x00\x01\xff\x00" // ................ |
/* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02" // ................ |
/* 0040 */ "\x00\x48\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // .H.............. |
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0f\x00\x06\x00\x72" // ...............r |
/* 0090 */ "\x7a\x68\x6f\x75\x00\x10\x00\x04\x00\x72\x7a\x68\x00"             // zhou.....rzh. |
// Dump done on RDP Client (4) 157 bytes |
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
//      General caps::extra flags 0 |
//      General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED no |
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
//      Bitmap caps::desktopWidth 800 |
//      Bitmap caps::desktopHeight 600 |
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
//      Input caps::inputFlags 0x1 |
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
// Sending on RDP Client (4) 327 bytes |
// /* 0000 */ "\x03\x00\x01\x47\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x81\x38\x08" // ...G...h....p.8. |
// /* 0010 */ "\x00\x00\x00\xc3\x2f\xf7\x30\xc9\xea\x10\x08\xb5\xfe\x81\x03\xf6" // ..../.0......... |
// /* 0020 */ "\x1d\xcc\x5f\x7f\x2c\x1a\xa9\x3b\x9f\xa7\xed\x51\x37\xfc\x0c\xe5" // .._.,..;...Q7... |
// /* 0030 */ "\xa4\xfd\x15\x05\xd5\xa7\xd3\x68\xf1\xf1\xe7\xa8\x2a\x02\x50\x64" // .......h....*.Pd |
// /* 0040 */ "\xfd\x71\xca\x6a\x0a\x75\x02\x12\x84\x97\x64\x45\xb4\xf1\x15\xb9" // .q.j.u....dE.... |
// /* 0050 */ "\x44\x50\xe1\x73\x11\x95\x8a\xa3\xac\x30\x6c\x6f\x65\xbb\xf2\x02" // DP.s.....0loe... |
// /* 0060 */ "\x97\x22\xea\x5c\x6c\x45\x6c\x62\xf2\x85\x98\xc3\xaf\xbc\x49\xd2" // ."..lElb......I. |
// /* 0070 */ "\x33\xae\x52\x57\x81\x9f\x31\x10\x62\xc3\x70\x71\x01\x5a\xa1\xd2" // 3.RW..1.b.pq.Z.. |
// /* 0080 */ "\x0e\x30\x99\x85\xf7\x22\x7a\xfd\xca\xaf\x53\xdc\xf9\xeb\x2a\xc9" // .0..."z...S...*. |
// /* 0090 */ "\xb0\x72\xf8\xee\xe2\xe8\x15\x6d\x7d\x6e\xd5\x25\x55\x1c\xda\x97" // .r.....m}n.%U... |
// /* 00a0 */ "\x57\xf3\xdb\x62\xa6\x33\x25\x75\xf9\xe8\x66\x0c\x91\xb1\x4d\x90" // W..b.3%u..f...M. |
// /* 00b0 */ "\x64\xff\xfc\x07\xb4\x26\x64\x38\xfa\xe5\x99\x2c\xe3\x00\x14\x04" // d....&d8...,.... |
// /* 00c0 */ "\x60\x8c\xd4\xc5\xb2\x9c\x87\xfe\x9b\xf9\xb6\xe3\x33\x89\x6d\xd9" // `...........3.m. |
// /* 00d0 */ "\x6b\x8e\xe8\x8e\x7d\xb9\x2a\xa7\xec\xbb\xcf\xb2\x14\xe2\xe6\xe9" // k...}.*......... |
// /* 00e0 */ "\x23\x99\xf2\x48\x6e\x1a\xff\x9f\x9a\xb0\x0d\xe3\x6e\xf4\xc2\x5d" // #..Hn.......n..] |
// /* 00f0 */ "\x3a\xca\x37\x04\x2c\xac\x36\x65\xed\x9f\x90\xdc\xa9\xa7\xde\x5a" // :.7.,.6e.......Z |
// /* 0100 */ "\x5a\xa3\x81\x24\xa6\xbb\x05\xc6\x6c\x51\xd5\x53\x9d\xf8\xd1\xc3" // Z..$....lQ.S.... |
// /* 0110 */ "\xff\x29\xbf\xcf\x24\x28\x8f\x65\xfd\x98\x72\xb8\x90\xb2\x74\x77" // .)..$(.e..r...tw |
// /* 0120 */ "\x8d\x2e\xc1\xb2\xec\x3d\x74\x55\xa6\x1a\x9e\x3b\x69\xb8\xba\x3a" // .....=tU...;i..: |
// /* 0130 */ "\x71\xd1\xc9\x4c\x55\xef\x4d\x07\x8b\xbf\x6d\xb8\xb1\xc3\xd2\x55" // q..LU.M...m....U |
// /* 0140 */ "\x09\xf5\x21\xac\x10\x9f\x5a"                                     // ..!...Z |
// Sent dumped on RDP Client (4) 327 bytes |
// Front::send_demand_active: done |
// Front::incoming: ACTIVATED (new license request) |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 445 bytes |
/* 0000 */ "\x03\x00\x01\xbd\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\xae\x38" // .......d....p..8 |
/* 0010 */ "\x00\x00\x00\x8e\x6d\x0e\xfa\xda\xf5\xa6\xa6\x64\xcc\x2a\xdf\xdc" // ....m......d.*.. |
/* 0020 */ "\xf5\x26\x73\xd7\x63\x23\x11\xdd\x3c\x52\x09\x4b\x48\x68\xed\x5e" // .&s.c#..<R.KHh.^ |
/* 0030 */ "\x8d\x24\x55\xcb\xda\x4c\xcb\x79\x68\x96\xc0\xac\x06\x2a\x9f\x51" // .$U..L.yh....*.Q |
/* 0040 */ "\xda\xcb\x5d\xc8\x35\x52\x00\x8c\xe6\xc8\x8c\x60\xc2\xc6\x3a\x03" // ..].5R.....`..:. |
/* 0050 */ "\x50\x9b\xcd\xfa\x7e\xa6\x31\xef\x90\xc8\x95\xed\x1f\x26\x34\x9e" // P...~.1......&4. |
/* 0060 */ "\xc7\xa3\x20\xd2\x6d\xeb\x6d\x6c\x64\x3d\x79\x31\x1d\xaf\x93\xa3" // .. .m.mld=y1.... |
/* 0070 */ "\x1c\xf1\x8b\x66\xc5\x2b\x63\xdf\x1c\xd1\x97\xc8\x8e\xdf\x0a\x58" // ...f.+c........X |
/* 0080 */ "\xd0\x5c\x22\xef\xf4\x65\x2f\xde\xa9\x2b\x78\xc7\x82\x02\x47\x1b" // .."..e/..+x...G. |
/* 0090 */ "\x29\x7d\xd0\x5a\x74\xc6\x37\xd9\x22\x02\x1a\xdd\xb8\x38\x9b\x64" // )}.Zt.7."....8.d |
/* 00a0 */ "\x6b\x93\xae\x06\xc4\xcb\x92\x4f\xf6\xbd\x24\x2d\x03\x39\xef\x6a" // k......O..$-.9.j |
/* 00b0 */ "\x32\xbc\x9e\x5e\x11\xa1\x9b\x01\xf5\x19\x01\x05\xc7\x4a\xec\x41" // 2..^.........J.A |
/* 00c0 */ "\xd3\xbd\x58\x09\x0e\x46\xab\xac\xec\x84\x3c\x48\xf5\xb0\xdf\xe8" // ..X..F....<H.... |
/* 00d0 */ "\x61\xea\xa0\xbb\x00\xa8\x2a\x62\xfa\xb7\xf3\xb9\xc9\xed\xa6\x04" // a.....*b........ |
/* 00e0 */ "\x22\x29\xc9\x28\x22\x62\x8b\x89\xb0\xd0\xee\xe6\xe0\xb0\x93\x44" // ").("b.........D |
/* 00f0 */ "\xd0\xae\x8d\x39\x45\x2b\x95\x1b\xab\x21\x3c\x8b\x8e\x4e\x0e\xf1" // ...9E+...!<..N.. |
/* 0100 */ "\x0f\x7b\x4d\x73\x87\x03\x0f\xb1\x0d\x45\x62\x73\x96\x6f\x14\x4a" // .{Ms.....Ebs.o.J |
/* 0110 */ "\x8f\x35\xf6\x04\x3a\x06\xce\x6b\x12\x6f\x1c\xa2\x7b\x2a\x0c\x60" // .5..:..k.o..{*.` |
/* 0120 */ "\xaa\xfc\x7b\x0d\xde\xf6\x77\x3c\x53\x76\x8e\xc4\xd4\x12\xef\x1b" // ..{...w<Sv...... |
/* 0130 */ "\x4c\x83\x2b\x5f\x3d\xaf\xd4\x57\x75\xd0\x6c\x2a\x29\xaa\x28\x1e" // L.+_=..Wu.l*).(. |
/* 0140 */ "\xfa\xd0\x23\x22\x1b\x2c\xee\x4d\xfc\xcf\x46\x87\xb6\x8f\x79\x19" // ..#".,.M..F...y. |
/* 0150 */ "\xff\x2a\x01\xef\xc0\x82\x74\x28\x8b\x31\x38\xa9\xfc\x6e\xb4\x4e" // .*....t(.18..n.N |
/* 0160 */ "\x8a\xe1\xa6\x9c\xe3\xff\x2a\xf9\xd7\xa1\x74\xad\x99\xec\x2a\x64" // ......*...t...*d |
/* 0170 */ "\x0a\x14\x72\x11\x4b\xc4\x9b\x8f\x72\xe5\x4c\x9c\xec\x07\x36\xca" // ..r.K...r.L...6. |
/* 0180 */ "\xab\x57\xaf\xd3\x9f\xba\xda\xfb\x51\xda\xeb\x01\xa9\x87\x16\x6b" // .W......Q......k |
/* 0190 */ "\xb5\xb0\xdf\xe2\xc8\x9f\x98\xcf\x7f\x69\x42\xbd\xd6\xfa\xa4\x13" // .........iB..... |
/* 01a0 */ "\x04\xfb\x72\x53\x3a\x82\x52\x7e\x10\x97\x02\xa4\x5e\x5e\xff\xf1" // ..rS:.R~....^^.. |
/* 01b0 */ "\x20\x75\x44\xff\x6d\x53\x5c\x48\x7a\xa9\xec\x11\xf8"             //  uD.mS.Hz.... |
// Dump done on RDP Client (4) 445 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=38 |
// Front::incoming: Received CONFIRMACTIVEPDU |
// Front::process_confirm_active |
// Front::process_confirm_active: lengthSourceDescriptor = 6 |
// Front::process_confirm_active: lengthCombinedCapabilities = 396 |
// Front::capability 0 / 14 |
// Front::process_confirm_active: Receiving from client General caps (24 bytes) |
//      General caps::major 1 |
//      General caps::minor 3 |
//      General caps::protocol 512 |
//      General caps::pad2octetA 0 |
//      General caps::compression type 0 |
//      General caps::extra flags 0 |
//      General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED no |
//      General caps::extraflags:LONG_CREDENTIALS_SUPPORTED no |
//      General caps::extraflags:AUTORECONNECT_SUPPORTED no |
//      General caps::extraflags:ENC_SALTED_CHECKSUM no |
//      General caps::extraflags:NO_BITMAP_COMPRESSION_HDR no |
//      General caps::updateCapability 0 |
//      General caps::remoteUnshare 0 |
//      General caps::compressionLevel 0 |
//      General caps::refreshRectSupport 0 |
//      General caps::suppressOutputSupport 0 |
// Front::capability 1 / 14 |
// Front::process_confirm_active: Receiving from client Bitmap caps (28 bytes) |
//      Bitmap caps::preferredBitsPerPixel 24 |
//      Bitmap caps::receive1BitPerPixel 1 |
//      Bitmap caps::receive4BitsPerPixel 1 |
//      Bitmap caps::receive8BitsPerPixel 1 |
//      Bitmap caps::desktopWidth 800 |
//      Bitmap caps::desktopHeight 600 |
//      Bitmap caps::pad2octets 0 |
//      Bitmap caps::desktopResizeFlag 1 (yes) |
//      Bitmap caps::bitmapCompressionFlag 1 yes |
//      Bitmap caps::highColorFlags 0 |
//      Bitmap caps::drawingFlags 0 |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY no |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_COLOR_SUBSAMPLING no |
//      Bitmap caps::drawingFlags:DRAW_ALLOW_SKIP_ALPHA no |
//      Bitmap caps::multipleRectangleSupport 1 |
//      Bitmap caps::pad2octetsB 0 |
// Front::capability 2 / 14 |
// Front::process_confirm_active: Receiving from client Order caps (88 bytes) |
//      Order caps::terminalDescriptor 0 |
//      Order caps::pad4octetsA 0 |
//      Order caps::desktopSaveXGranularity 1 |
//      Order caps::desktopSaveYGranularity 20 |
//      Order caps::pad2octetsA 0 |
//      Order caps::maximumOrderLevel 1 |
//      Order caps::numberFonts 327 |
//      Order caps::orderFlags 0x2A |
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
//      Order caps::orderSupport[SAVEBITMAP] 1 |
//      Order caps::orderSupport[WTEXTOUT] 0 |
//      Order caps::orderSupport[MEMBLT_V2] 1 |
//      Order caps::orderSupport[MEM3BLT_V2] 1 |
//      Order caps::orderSupport[MULTIDSTBLT] 0 |
//      Order caps::orderSupport[MULTIPATBLT] 0 |
//      Order caps::orderSupport[MULTISCRBLT] 0 |
//      Order caps::orderSupport[MULTIOPAQUERECT] 0 |
//      Order caps::orderSupport[FAST_INDEX] 0 |
//      Order caps::orderSupport[POLYGON_SC] 1 |
//      Order caps::orderSupport[POLYGON_CB] 1 |
//      Order caps::orderSupport[POLYLINE] 1 |
//      Order caps::orderSupport[UnusedIndex7] 0 |
//      Order caps::orderSupport[FAST_GLYPH] 0 |
//      Order caps::orderSupport[ELLIPSE_SC] 1 |
//      Order caps::orderSupport[ELLIPSE_CB] 1 |
//      Order caps::orderSupport[GLYPH] 1 |
//      Order caps::orderSupport[GLYPH_WEXTTEXTOUT] 0 |
//      Order caps::orderSupport[GLYPH_WLONGTEXTOUT] 0 |
//      Order caps::orderSupport[GLYPH_WLONGEXTTEXTOUT] 0 |
//      Order caps::orderSupport[UnusedIndex11] 0 |
//      Order caps::textFlags 1697 |
//      Order caps::orderSupportExFlags 0x0 |
//      Order caps::pad4octetsB 0 |
//      Order caps::desktopSaveSize 230400 |
//      Order caps::pad2octetsC 0 |
//      Order caps::pad2octetsD 0 |
//      Order caps::textANSICodePage 1252 |
//      Order caps::pad2octetsE 0 |
// Front::capability 3 / 14 |
// Front::process_confirm_active: Receiving from client BitmapCache caps (40 bytes) |
//      BitmapCache caps::pad1 0 |
//      BitmapCache caps::pad2 0 |
//      BitmapCache caps::pad3 0 |
//      BitmapCache caps::pad4 0 |
//      BitmapCache caps::pad5 0 |
//      BitmapCache caps::pad6 0 |
//      BitmapCache caps::cache0Entries 600 |
//      BitmapCache caps::cache0MaximumCellSize 768 |
//      BitmapCache caps::cache1Entries 300 |
//      BitmapCache caps::cache1MaximumCellSize 3072 |
//      BitmapCache caps::cache2Entries 262 |
//      BitmapCache caps::cache2MaximumCellSize 12288 |
// Front::capability 4 / 14 |
// Front::process_confirm_active: Receiving from client Pointer caps (8 bytes) |
//      Pointer caps::colorPointerFlag 0 |
//      Pointer caps::colorPointerCacheSize 20 |
// Front::capability 5 / 14 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_COLORCACHE |
// Front::capability 6 / 14 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_ACTIVATION |
// Front::capability 7 / 14 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_CONTROL |
// Front::capability 8 / 14 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_SHARE |
// Front::capability 9 / 14 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_BRUSH |
// Front::process_confirm_active: Receiving from client BrushCache caps (8 bytes) |
//      BrushCacheCaps caps::brushSupportLevel 1 |
// Front::capability 10 / 14 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_INPUT |
// Front::capability 11 / 14 |
// Front::process_confirm_active: Receiving from client CAPSTYPE_SOUND |
// Front::capability 12 / 14 |
// Front::capability 13 / 14 |
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
//      GlyphCache caps::GlyphSupportLevel 2 |
//      GlyphCache caps::pad2octets 0 |
// Front::process_confirm_active: done p=0x7f9039eaec63 end=0x7f9039eaec63 |
// Front::reset: use_bitmap_comp=1 |
// Front::reset: use_compact_packets=0 |
// Front::reset: bitmap_cache_version=0 |
// ◢ In src/core/RDP/mppc.cpp:121 |
// Front::reset: invalid RDP compression code 0xffffffff |
// Front::incoming: Received CONFIRMACTIVEPDU done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 155 bytes |
/* 0000 */ "\x03\x00\x00\x31\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x22\x08" // ...1...d....p.". |
/* 0010 */ "\x00\x00\x00\x10\xa6\xc2\xb2\x31\x3f\xdf\xcd\x82\xb6\x32\x47\xb9" // .......1?....2G. |
/* 0020 */ "\x52\x8c\xa7\x2e\x3c\x5f\xcf\xc2\x54\x23\xc2\x2c\x30\x57\x29\x31" // R...<_..T#.,0W)1 |
/* 0030 */ "\x11\x03\x00\x00\x35\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x26" // ....5...d....p.& |
/* 0040 */ "\x08\x00\x00\x00\x40\x98\xcb\xc0\x5e\xbf\x5b\xa3\xc5\xa0\xcc\xfe" // ....@...^.[..... |
/* 0050 */ "\x9e\x1f\xc3\x2a\x64\x16\x96\x4e\x32\x69\x01\xf1\x38\x7f\xfb\x40" // ...*d..N2i..8..@ |
/* 0060 */ "\xc8\x2b\xd7\xc3\xfc\xed\x03\x00\x00\x35\x02\xf0\x80\x64\x00\x00" // .+.......5...d.. |
/* 0070 */ "\x03\xeb\x70\x80\x26\x08\x00\x00\x00\xa9\x77\x29\x66\x1e\x8c\x8e" // ..p.&.....w)f... |
/* 0080 */ "\x2f\x24\x01\x8b\x11\xc6\x97\xc4\xb3\x6b\x7e\xc5\xc0\x36\x0e\xfc" // /$.......k~..6.. |
/* 0090 */ "\x9c\x02\x52\xd6\xc1\x8a\x70\x4b\x33\x4f\x47"                     // ..R...pK3OG |
// Dump done on RDP Client (4) 155 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=8 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=31 sdata_in.len=8 sdata_in.compressedLen=0 remains=0 payload_len=4 |
// Front::process_data: PDUTYPE2_SYNCHRONIZE |
// Front::process_data: PDUTYPE2_SYNCHRONIZE messageType=1 controlId=1002 |
// Front::send_synchronize |
// Sec clear payload to send: |
// /* 0000 */ 0x16, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x16, 0x00, 0x1f, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x01, 0x00, 0xea, 0x03,                                                              // ...... |
// Sending on RDP Client (4) 48 bytes |
// /* 0000 */ "\x03\x00\x00\x30\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x22\x08\x00" // ...0...h....p".. |
// /* 0010 */ "\x00\x00\x16\x44\x24\x4d\xc8\x31\x13\x0e\x3a\xff\x9c\xe0\x53\xe6" // ...D$M.1..:...S. |
// /* 0020 */ "\x94\xb4\xdd\xec\x9f\x68\xa9\xcf\xc8\xba\x09\xc8\x62\x85\xed\xc4" // .....h......b... |
// Sent dumped on RDP Client (4) 48 bytes |
// Front::send_synchronize: done |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=8 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=20 sdata_in.len=12 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// Front::process_data: PDUTYPE2_CONTROL |
// Front::send_control: action=4 |
// Sec clear payload to send: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x1a, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00,                                      // .......... |
// Sending on RDP Client (4) 52 bytes |
// /* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x26\x08\x00" // ...4...h....p&.. |
// /* 0010 */ "\x00\x00\x03\x06\x28\xcd\x96\x27\x3c\x3a\xa6\xbf\x60\xbe\x98\x31" // ....(..'<:..`..1 |
// /* 0020 */ "\x5d\x52\xb5\x97\xb8\xd8\xa4\xa8\x59\x26\x7f\x08\x93\xf0\xef\x01" // ]R......Y&...... |
// /* 0030 */ "\xd0\x88\x1c\x47"                                                 // ...G |
// Sent dumped on RDP Client (4) 52 bytes |
// Front::send_control: done. action=4 |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=8 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=20 sdata_in.len=12 sdata_in.compressedLen=0 remains=0 payload_len=8 |
// Front::process_data: PDUTYPE2_CONTROL |
// Front::send_control: action=2 |
// Sec clear payload to send: |
// /* 0000 */ 0x1a, 0x00, 0x17, 0x00, 0xe9, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x1a, 0x00, 0x14, 0x00,  // ................ |
// /* 0010 */ 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00,                                      // .......... |
// Sending on RDP Client (4) 52 bytes |
// /* 0000 */ "\x03\x00\x00\x34\x02\xf0\x80\x68\x00\x00\x03\xeb\x70\x26\x08\x00" // ...4...h....p&.. |
// /* 0010 */ "\x00\x00\x17\x13\x3a\xb7\xd6\x48\x28\xb8\x09\x96\x95\x99\x57\x52" // ....:..H(.....WR |
// /* 0020 */ "\x3d\x59\x57\xec\xcd\x1e\x09\x02\x61\x7b\x45\x89\x29\xdb\x8c\x86" // =YW.....a{E.)... |
// /* 0030 */ "\x85\xb2\x27\xeb"                                                 // ..'. |
// Sent dumped on RDP Client (4) 52 bytes |
// Front::send_control: done. action=2 |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 61 bytes |
/* 0000 */ "\x03\x00\x00\x3d\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x2e\x08" // ...=...d....p... |
/* 0010 */ "\x00\x00\x00\x4d\xeb\x8a\x6c\x88\x1f\xae\x51\x67\x85\x4e\xe5\x48" // ...M..l...Qg.N.H |
/* 0020 */ "\x00\x15\xda\x85\x9b\x42\x6a\xc9\x41\xe7\xb4\x75\xb3\xc5\xa7\xbb" // .....Bj.A..u.... |
/* 0030 */ "\x5c\xc9\x67\x44\xbe\x25\xe7\x9b\xcf\x02\x78\x45\xe9"             // ..gD.%....xE. |
// Dump done on RDP Client (4) 61 bytes |
// Front::incoming: ACTIVATE_AND_PROCESS_DATA |
// Front::incoming: sec_flags=8 |
// Front::incoming: Received DATAPDU |
// Front::process_data |
// Front::process_data: sdata_in.pdutype2=28 sdata_in.len=20 sdata_in.compressedLen=0 remains=0 payload_len=16 |
// Front::process_data: PDUTYPE2_INPUT num_events=1 |
// Front::process_data: Slow-Path INPUT_EVENT_SYNC eventTime=0 toggleFlags=0x0000 |
// Front::process_data: (Slow-Path) Synchronize Event toggleFlags=0x0 |
// Front::process_data: PDUTYPE2_INPUT done |
// Front::process_data: done |
// Front::incoming: Received DATAPDU done |
// Front::incoming |
// Socket RDP Client (4) receiving 65535 bytes |
// Recv done on RDP Client (4) 106 bytes |
/* 0000 */ "\x03\x00\x00\x35\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x26\x08" // ...5...d....p.&. |
/* 0010 */ "\x00\x00\x00\xea\xf7\x1e\xf6\x2f\x95\x1f\xe9\x94\x47\xb0\x49\xc1" // ......./....G.I. |
/* 0020 */ "\x04\x0c\x06\x16\x31\x6c\x77\x66\x8f\xdd\x05\x4f\x88\x90\x41\x27" // ....1lwf...O..A' |
/* 0030 */ "\xe8\x89\x8c\x2d\x53\x03\x00\x00\x35\x02\xf0\x80\x64\x00\x00\x03" // ...-S...5...d... |
/* 0040 */ "\xeb\x70\x80\x26\x08\x00\x00\x00\xa5\xf6\xf6\x21\x2d\x17\x20\x8a" // .p.&.......!-. . |
/* 0050 */ "\x80\xdd\xf5\xaa\x91\xff\xeb\xb1\x94\x9f\x31\xbf\x02\xdc\x7e\x3f" // ..........1...~? |
/* 0060 */ "\x4d\x7b\x11\x5b\x7b\xe8\x40\x1c\x94\x5e"                         // M{.[{.@..^ |
} /* end indata */;

