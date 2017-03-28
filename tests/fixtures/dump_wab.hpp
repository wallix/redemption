const char outdata[] =
{
// connecting to 10.10.47.154:3389 |
// connection to 10.10.47.154:3389 succeeded : socket 4 |
// --------- CREATION OF MOD ------------------------ |
// Creation of new mod 'RDP' |
// ModRDPParams target_user="x" |
// ModRDPParams target_password="<hidden>" |
// ModRDPParams target_host="10.10.47.154" |
// ModRDPParams client_address="192.168.1.100" |
// ModRDPParams auth_user="" |
// ModRDPParams target_application="" |
// ModRDPParams enable_tls=yes |
// ModRDPParams enable_nla=no |
// ModRDPParams enable_krb=no |
// ModRDPParams enable_fastpath=no |
// ModRDPParams enable_mem3blt=no |
// ModRDPParams enable_bitmap_update=no |
// ModRDPParams enable_new_pointer=no |
// ModRDPParams enable_glyph_cache=no |
// ModRDPParams enable_session_probe=no |
// ModRDPParams enable_session_probe_launch_mask=yes |
// ModRDPParams session_probe_use_clipboard_based_launcher=no |
// ModRDPParams session_probe_launch_timeout=0 |
// ModRDPParams session_probe_launch_fallback_timeout=0 |
// ModRDPParams session_probe_start_launch_timeout_timer_only_after_logon=yes |
// ModRDPParams session_probe_on_launch_failure=1 |
// ModRDPParams session_probe_keepalive_timeout=0 |
// ModRDPParams session_probe_on_keepalive_timeout_disconnect_user=yes |
// ModRDPParams session_probe_end_disconnected_session=no |
// ModRDPParams session_probe_customize_executable_name=no |
// ModRDPParams dsiable_clipboard_log_syslog=no |
// ModRDPParams dsiable_clipboard_log_wrm=no |
// ModRDPParams dsiable_file_system_log_syslog=no |
// ModRDPParams dsiable_file_system_log_wrm=no |
// ModRDPParams enable_transparent_mode=no |
// ModRDPParams output_filename="" |
// ModRDPParams persistent_key_list_transport=<(nil)> |
// ModRDPParams transparent_recorder_transport=<(nil)> |
// ModRDPParams key_flags=7 |
// ModRDPParams acl=<(nil)> |
// ModRDPParams outbound_connection_blocking_rules="" |
// ModRDPParams ignore_auth_channel=no |
// ModRDPParams auth_channel="" |
// ModRDPParams alternate_shell="" |
// ModRDPParams working_dir="" |
// ModRDPParams use_client_provided_alternate_shell=no |
// ModRDPParams target_application_account="" |
// ModRDPParams target_application_password="<hidden>" |
// ModRDPParams rdp_compression=0 |
// ModRDPParams error_message=<(nil)> |
// ModRDPParams disconnect_on_logon_user_change=no |
// ModRDPParams open_session_timeout=0 |
// ModRDPParams server_cert_store=yes |
// ModRDPParams server_cert_check=1 |
// ModRDPParams server_access_allowed_message=1 |
// ModRDPParams server_cert_create_message=1 |
// ModRDPParams server_cert_success_message=1 |
// ModRDPParams server_cert_failure_message=1 |
// ModRDPParams server_cert_error_message=1 |
// ModRDPParams hide_client_name=no |
// ModRDPParams extra_orders= |
// ModRDPParams enable_persistent_disk_bitmap_cache=no |
// ModRDPParams enable_cache_waiting_list=no |
// ModRDPParams persist_bitmap_cache_on_disk=no |
// ModRDPParams password_printing_mode=0 |
// ModRDPParams allow_channels=<none> |
// ModRDPParams deny_channels=<none> |
// ModRDPParams server_redirection_support=yes |
// ModRDPParams bogus_sc_net_size=yes |
// ModRDPParams proxy_managed_drives= |
// ModRDPParams lang=EN |
// ModRDPParams verbose=0x000001FF |
// ModRDPParams cache_verbose=0x00000000 |
// RDP Extra orders="" |
// Remote RDP Server domain="" login="x" host="192-168-1-100" |
// enable_session_probe=no |
// Server key layout is 40c |
// Init with Redir_info: RedirectionInfo(valid=false, session_id=0, host='', username='', password='<null>', domain='', LoadBalanceInfoLength=0, dont_store_username=false, server_tsv_capable=false, smart_card_logon=false) |
// ServerRedirectionSupport=true |
// mod_rdp::Early TLS Security Exchange |
// RdpNego::NEGO_STATE_INITIAL |
// RdpNego::send_x224_connection_request_pdu |
// Send cookie: |
// /* 0000 */ "\x43\x6f\x6f\x6b\x69\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68" // |Cookie: mstshash |
// /* 0010 */ "\x3d\x78\x0d\x0a"                                                 //=x.. |
// Sending on RDP Wab Target (4) 39 bytes |
/* 0000 */ "\x03\x00\x00\x27\x22\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" // |...'"......Cooki |
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x78\x0d\x0a\x01" // |e: mstshash=x... |
/* 0020 */ "\x00\x08\x00\x01\x00\x00\x00"                                     //....... |
// Sent dumped on RDP Wab Target (4) 39 bytes |
// RdpNego::send_x224_connection_request_pdu done |
// mod_rdp::Early TLS Security Exchange |
// RdpNego::NEGO_STATE_TLS |
// RdpNego::recv_connection_confirm |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x13"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 15 bytes |
// Recv done on RDP Wab Target (4) 15 bytes |
// /* 0000 */ "\x0e\xd0\x00\x00\x00\x00\x00\x02\x01\x08\x00\x01\x00\x00\x00"     //............... |
// Dump done on RDP Wab Target (4) 15 bytes |
// NEG_RSP_TYPE=2 NEG_RSP_FLAGS=1 NEG_RSP_LENGTH=8 NEG_RSP_SELECTED_PROTOCOL=1 |
// CC Recv: PROTOCOL TLS 1.0 |
// activating SSL |
// Client TLS start |
// SSL_connect() |
// SSL_get_peer_certificate() |
// failed to create directory /etc/rdpproxy/cert/rdp : Permission denied [13] |
// Failed to create certificate directory: /etc/rdpproxy/cert/rdp/device_id  |
// Dumping X509 peer certificate: "/etc/rdpproxy/cert/rdp/device_id/rdp,10.10.47.154,3389,X509.pem" |
// Failed to dump X509 peer certificate |
// SocketTransport::X509_get_pubkey() |
// SocketTransport::i2d_PublicKey() |
// SocketTransport::i2d_PublicKey() |
// TLS::X509::issuer=CN = wabinstall.ifr.lan |
// TLS::X509::subject=CN = wabinstall.ifr.lan |
// TLS::X509::fingerprint=b4:3f:9b:85:33:ce:d9:af:71:59:1c:b0:4e:5e:17:5c:09:bd:ad:06 |
// SocketTransport::enable_tls() done |
// SocketTransport::enable_tls() done |
// RdpNego::recv_connection_confirm done |
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
// cs_core::serverSelectedProtocol = 1 |
// CS_Cluster: Server Redirection Supported |
// Sending to server GCC User Data CS_CLUSTER (12 bytes) |
// cs_cluster::flags [000d] |
// cs_cluster::redirectedSessionID = 0 |
// Sending to server GCC User Data CS_SECURITY (12 bytes) |
// CSSecGccUserData::encryptionMethods 3 |
// CSSecGccUserData::extEncryptionMethods 0 |
// Sending on RDP Wab Target (4) 376 bytes |
/* 0000 */ "\x03\x00\x01\x78\x02\xf0\x80\x7f\x65\x82\x01\x6c\x04\x01\x01\x04" // |...x....e..l.... |
/* 0010 */ "\x01\x01\x01\x01\xff\x30\x1a\x02\x01\x22\x02\x01\x02\x02\x01\x00" // |.....0..."...... |
/* 0020 */ "\x02\x01\x01\x02\x01\x00\x02\x01\x01\x02\x03\x00\xff\xff\x02\x01" // |................ |
/* 0030 */ "\x02\x30\x19\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02" // |.0.............. |
/* 0040 */ "\x01\x00\x02\x01\x01\x02\x02\x04\x20\x02\x01\x02\x30\x1f\x02\x03" // |........ ...0... |
/* 0050 */ "\x00\xff\xff\x02\x02\xfc\x17\x02\x03\x00\xff\xff\x02\x01\x01\x02" // |................ |
/* 0060 */ "\x01\x00\x02\x01\x01\x02\x03\x00\xff\xff\x02\x01\x02\x04\x82\x01" // |................ |
/* 0070 */ "\x07\x00\x05\x00\x14\x7c\x00\x01\x80\xfe\x00\x08\x00\x10\x00\x01" // |.....|.......... |
/* 0080 */ "\xc0\x00\x44\x75\x63\x61\x80\xf0\x01\xc0\xd8\x00\x04\x00\x08\x00" // |..Duca.......... |
/* 0090 */ "\x00\x04\x00\x03\x01\xca\x03\xaa\x0c\x04\x00\x00\x28\x0a\x00\x00" // |............(... |
/* 00a0 */ "\x31\x00\x39\x00\x32\x00\x2d\x00\x31\x00\x36\x00\x38\x00\x2d\x00" // |1.9.2.-.1.6.8.-. |
/* 00b0 */ "\x31\x00\x2d\x00\x31\x00\x30\x00\x30\x00\x00\x00\x00\x00\x00\x00" // |1.-.1.0.0....... |
/* 00c0 */ "\x04\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\xca\x01\x00" // |................ |
/* 0110 */ "\x00\x00\x00\x00\x10\x00\x07\x00\x01\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00" // |................ |
/* 0160 */ "\x04\xc0\x0c\x00\x0d\x00\x00\x00\x00\x00\x00\x00\x02\xc0\x0c\x00" // |................ |
/* 0170 */ "\x03\x00\x00\x00\x00\x00\x00\x00"                                 //........ |
// Sent dumped on RDP Wab Target (4) 376 bytes |
// mod_rdp::Basic Settings Exchange |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x65"                                                     //..e |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 97 bytes |
// Recv done on RDP Wab Target (4) 97 bytes |
// /* 0000 */ "\x02\xf0\x80\x7f\x66\x5b\x0a\x01\x00\x02\x01\x00\x30\x1a\x02\x01" // |....f[......0... |
// /* 0010 */ "\x22\x02\x01\x03\x02\x01\x00\x02\x01\x01\x02\x01\x00\x02\x01\x01" // |"............... |
// /* 0020 */ "\x02\x03\x00\xff\xf8\x02\x01\x02\x04\x37\x00\x05\x00\x14\x7c\x00" // |.........7....|. |
// /* 0030 */ "\x01\x2a\x14\x76\x0a\x01\x01\x00\x01\xc0\x00\x4d\x63\x44\x6e\x80" // |.*.v.......McDn. |
// /* 0040 */ "\x20\x01\x0c\x0c\x00\x04\x00\x08\x00\x01\x00\x00\x00\x03\x0c\x08" // | ............... |
// /* 0050 */ "\x00\xeb\x03\x00\x00\x02\x0c\x0c\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0060 */ "\x00"                                                             //. |
// Dump done on RDP Wab Target (4) 97 bytes |
// GCC::UserData tag=0c01 length=12 |
// Received from server GCC User Data SC_CORE (12 bytes) |
// sc_core::version [80004] RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 servers) |
// sc_core::clientRequestedProtocols  = 1 |
// GCC::UserData tag=0c03 length=8 |
// server_channels_count=0 sent_channels_count=0 |
// Received from server GCC User Data SC_NET (8 bytes) |
// sc_net::MCSChannelId   = 1003 |
// sc_net::channelCount   = 0 |
// GCC::UserData tag=0c02 length=12 |
// Received from server GCC User Data SC_SECURITY (12 bytes) |
// sc_security::encryptionMethod = 0 |
// sc_security::encryptionLevel  = 0 |
// No encryption |
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
// mod_rdp::Secure Settings Exchange |
// mod_rdp::send_client_info_pdu |
// send data request |
// send extended login info (RDP5-style) 1017b :x |
// Sending on RDP Wab Target (4) 331 bytes |
/* 0000 */ "\x03\x00\x01\x4b\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x81\x3c\x40" // |...K...d. ..p.<@ |
/* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x7b\x01\x01\x00\x00\x00\x02\x00\x02" // |.......{........ |
/* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x78\x00\x00\x00\x78\x00\x00\x00\x00" // |.......x...x.... |
/* 0030 */ "\x00\x00\x00\x02\x00\x1c\x00\x31\x00\x39\x00\x32\x00\x2e\x00\x31" // |.......1.9.2...1 |
/* 0040 */ "\x00\x36\x00\x38\x00\x2e\x00\x31\x00\x2e\x00\x31\x00\x30\x00\x30" // |.6.8...1...1.0.0 |
/* 0050 */ "\x00\x00\x00\x40\x00\x43\x00\x3a\x00\x5c\x00\x57\x00\x69\x00\x6e" // |...@.C.:...W.i.n |
/* 0060 */ "\x00\x64\x00\x6f\x00\x77\x00\x73\x00\x5c\x00\x53\x00\x79\x00\x73" // |.d.o.w.s...S.y.s |
/* 0070 */ "\x00\x74\x00\x65\x00\x6d\x00\x33\x00\x32\x00\x5c\x00\x6d\x00\x73" // |.t.e.m.3.2...m.s |
/* 0080 */ "\x00\x74\x00\x73\x00\x63\x00\x61\x00\x78\x00\x2e\x00\x64\x00\x6c" // |.t.s.c.a.x...d.l |
/* 0090 */ "\x00\x6c\x00\x00\x00\x00\x00\x00\x00\x47\x00\x00\x00\x00\x00\x00" // |.l.......G...... |
/* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00\x05" // |................ |
/* 00e0 */ "\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x47\x00\x00" // |.............G.. |
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03" // |................ |
/* 0130 */ "\x00\x00\x00\x05\x00\x01\x00\x00\x00\x00\x00\x00\x00\xc4\xff\xff" // |................ |
/* 0140 */ "\xff\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00"                     //........... |
// Sent dumped on RDP Wab Target (4) 331 bytes |
// send data request done |
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
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::Bias 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardName GMT |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMonth 10 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDay 5 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wHour 2 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardBias 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightName GMT (heure d'été) |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMonth 3 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDay 5 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wHour 1 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightBias 4294967236 |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x81\x42\x80\x00\x00\x00\x01" // |...h. ..p.B..... |
// /* 0010 */ "\x02\x3e\x01\x7b\x3c\x31\xa6\xae\xe8\x74\xf6\xb4\xa5\x03\x90\xe7" // |.>.{<1...t...... |
// /* 0020 */ "\xc2\xc7\x39\xba\x53\x1c\x30\x54\x6e\x90\x05\xd0\x05\xce\x44\x18" // |..9.S.0Tn.....D. |
// /* 0030 */ "\x91\x83\x81\x00\x00\x04\x00\x2c\x00\x00\x00\x4d\x00\x69\x00\x63" // |.......,...M.i.c |
// /* 0040 */ "\x00\x72\x00\x6f\x00\x73\x00\x6f\x00\x66\x00\x74\x00\x20\x00\x43" // |.r.o.s.o.f.t. .C |
// /* 0050 */ "\x00\x6f\x00\x72\x00\x70\x00\x6f\x00\x72\x00\x61\x00\x74\x00\x69" // |.o.r.p.o.r.a.t.i |
// /* 0060 */ "\x00\x6f\x00\x6e\x00\x00\x00\x08\x00\x00\x00\x32\x00\x33\x00\x36" // |.o.n.......2.3.6 |
// /* 0070 */ "\x00\x00\x00\x0d\x00\x04\x00\x01\x00\x00\x00\x03\x00\xb8\x00\x01" // |................ |
// /* 0080 */ "\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x06\x00\x5c\x00\x52" // |...............R |
// /* 0090 */ "\x53\x41\x31\x48\x00\x00\x00\x00\x02\x00\x00\x3f\x00\x00\x00\x01" // |SA1H.......?.... |
// /* 00a0 */ "\x00\x01\x00\x01\xc7\xc9\xf7\x8e\x5a\x38\xe4\x29\xc3\x00\x95\x2d" // |........Z8.)...- |
// /* 00b0 */ "\xdd\x4c\x3e\x50\x45\x0b\x0d\x9e\x2a\x5d\x18\x63\x64\xc4\x2c\xf7" // |.L>PE...*].cd.,. |
// /* 00c0 */ "\x8f\x29\xd5\x3f\xc5\x35\x22\x34\xff\xad\x3a\xe6\xe3\x95\x06\xae" // |.).?.5"4..:..... |
// /* 00d0 */ "\x55\x82\xe3\xc8\xc7\xb4\xa8\x47\xc8\x50\x71\x74\x29\x53\x89\x6d" // |U......G.Pqt)S.m |
// /* 00e0 */ "\x9c\xed\x70\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x48\x00\xa8" // |..p..........H.. |
// /* 00f0 */ "\xf4\x31\xb9\xab\x4b\xe6\xb4\xf4\x39\x89\xd6\xb1\xda\xf6\x1e\xec" // |.1..K...9....... |
// /* 0100 */ "\xb1\xf0\x54\x3b\x5e\x3e\x6a\x71\xb4\xf7\x75\xc8\x16\x2f\x24\x00" // |..T;^>jq..u../$. |
// /* 0110 */ "\xde\xe9\x82\x99\x5f\x33\x0b\xa9\xa6\x94\xaf\xcb\x11\xc3\xf2\xdb" // |...._3.......... |
// /* 0120 */ "\x09\x42\x68\x29\x56\x58\x01\x56\xdb\x59\x03\x69\xdb\x7d\x37\x00" // |.Bh)VX.V.Y.i.}7. |
// /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x0e\x00\x0e\x00\x6d" // |...............m |
// /* 0140 */ "\x69\x63\x72\x6f\x73\x6f\x66\x74\x2e\x63\x6f\x6d\x00"             //icrosoft.com. |
// Dump done on RDP Wab Target (4) 333 bytes |
// Rdp::License Request |
// send data request |
// Sending on RDP Wab Target (4) 163 bytes |
/* 0000 */ "\x03\x00\x00\xa3\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x80\x94\x80" // |.......d. ..p... |
/* 0010 */ "\x00\x00\x00\x13\x03\x90\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02" // |................ |
/* 0040 */ "\x00\x48\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |.H.............. |
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0f\x00\x02\x00\x78" // |...............x |
/* 0090 */ "\x00\x10\x00\x0e\x00\x31\x39\x32\x2d\x31\x36\x38\x2d\x31\x2d\x31" // |.....192-168-1-1 |
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
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x14\x80\x00\x10\x00\xff\x02" // |...h. ..p....... |
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
// /* 0000 */ "\x00\x01\x37"                                                     //..7 |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 307 bytes |
// Recv done on RDP Wab Target (4) 307 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x81\x28\x28\x01\x11\x00\x09" // |...h. ..p.((.... |
// /* 0010 */ "\x04\x02\x00\x01\x00\x04\x00\x12\x01\x52\x44\x50\x00\x09\x00\x00" // |.........RDP.... |
// /* 0020 */ "\x00\x01\x00\x18\x00\x01\x00\x03\x00\x00\x02\x00\x00\x00\x00\x01" // |................ |
// /* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x1c\x00\x10\x00\x01" // |................ |
// /* 0040 */ "\x00\x01\x00\x01\x00\x00\x04\x00\x03\x00\x00\x01\x00\x01\x00\x00" // |................ |
// /* 0050 */ "\x08\x01\x00\x00\x00\x0e\x00\x08\x00\x01\x00\x00\x00\x03\x00\x58" // |...............X |
// /* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0070 */ "\x00\x40\x42\x0f\x00\x01\x00\x14\x00\x00\x00\x01\x00\x2f\x00\x22" // |.@B........../." |
// /* 0080 */ "\x00\x01\x01\x01\x01\x01\x00\x00\x00\x01\x01\x01\x00\x00\x00\x00" // |................ |
// /* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x01\x00\x01\x00\x00\x00" // |................ |
// /* 00a0 */ "\x00\xa1\x06\x00\x00\x40\x42\x0f\x00\x40\x42\x0f\x00\x01\x00\x00" // |.....@B..@B..... |
// /* 00b0 */ "\x00\x00\x00\x00\x00\x12\x00\x08\x00\x01\x00\x00\x00\x0a\x00\x08" // |................ |
// /* 00c0 */ "\x00\x06\x00\x00\x00\x08\x00\x0a\x00\x01\x00\x19\x00\x19\x00\x09" // |................ |
// /* 00d0 */ "\x00\x08\x00\x09\x04\xe2\xb5\x0d\x00\x58\x00\x29\x00\x00\x00\x00" // |.........X.).... |
// /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0130 */ "\x00\x00\x00"                                                     //... |
// Dump done on RDP Wab Target (4) 307 bytes |
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
// Input caps::imeFileName  |
// mod_rdp::process_server_caps done |
// mod_rdp::send_confirm_active |
// send data request |
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
// Input caps::imeFileName  |
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
// Sending on RDP Wab Target (4) 424 bytes |
/* 0000 */ "\x03\x00\x01\xa8\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x81\x99\x99" // |.......d. ..p... |
/* 0010 */ "\x01\x13\x00\x09\x04\x02\x00\x01\x00\xea\x03\x05\x00\x84\x01\x4d" // |...............M |
/* 0020 */ "\x53\x54\x53\x43\x0d\x00\x00\x00\x01\x00\x18\x00\x01\x00\x03\x00" // |STSC............ |
/* 0030 */ "\x00\x02\x00\x00\x00\x00\x0c\x04\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0040 */ "\x02\x00\x1c\x00\x10\x00\x01\x00\x01\x00\x01\x00\x00\x04\x00\x03" // |................ |
/* 0050 */ "\x00\x00\x01\x00\x01\x00\x00\x08\x01\x00\x00\x00\x03\x00\x58\x00" // |..............X. |
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0070 */ "\x00\x00\x00\x00\x01\x00\x14\x00\x00\x00\x01\x00\x00\x00\xaa\x00" // |................ |
/* 0080 */ "\x01\x01\x01\x01\x00\x00\x00\x00\x01\x00\x01\x00\x00\x01\x00\x00" // |................ |
/* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00" // |................ |
/* 00a0 */ "\xa1\x06\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 00b0 */ "\xe4\x04\x00\x00\x04\x00\x28\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |......(......... |
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 00d0 */ "\x58\x02\x00\x02\x2c\x01\x00\x08\x06\x01\x00\x20\x0a\x00\x08\x00" // |X...,...... .... |
/* 00e0 */ "\x06\x00\x00\x00\x07\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 00f0 */ "\x05\x00\x0c\x00\x00\x00\x00\x00\x02\x00\x02\x00\x08\x00\x08\x00" // |................ |
/* 0100 */ "\x01\x00\x14\x00\x09\x00\x08\x00\x00\x00\x00\x00\x0d\x00\x58\x00" // |..............X. |
/* 0110 */ "\x01\x00\x00\x00\x09\x04\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0120 */ "\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0160 */ "\x00\x00\x00\x00\x0c\x00\x08\x00\x01\x00\x00\x00\x0e\x00\x08\x00" // |................ |
/* 0170 */ "\x01\x00\x00\x00\x10\x00\x34\x00\xfe\x00\x04\x00\xfe\x00\x04\x00" // |......4......... |
/* 0180 */ "\xfe\x00\x08\x00\xfe\x00\x08\x00\xfe\x00\x10\x00\xfe\x00\x20\x00" // |.............. . |
/* 0190 */ "\xfe\x00\x40\x00\xfe\x00\x80\x00\xfe\x00\x00\x01\x40\x00\x00\x08" // |..@.........@... |
/* 01a0 */ "\x00\x01\x00\x01\x00\x00\x00\x00"                                 //........ |
// Sent dumped on RDP Wab Target (4) 424 bytes |
// send data request done |
// mod_rdp::send_confirm_active done |
// Waiting for answer to confirm active |
// mod_rdp::send_synchronise |
// send data request |
// Sending on RDP Wab Target (4) 36 bytes |
/* 0000 */ "\x03\x00\x00\x24\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x16\x16\x00" // |...$...d. ..p... |
/* 0010 */ "\x17\x00\x09\x04\x02\x00\x01\x00\x00\x02\x16\x00\x1f\x00\x00\x00" // |................ |
/* 0020 */ "\x01\x00\xea\x03"                                                 //.... |
// Sent dumped on RDP Wab Target (4) 36 bytes |
// send data request done |
// mod_rdp::send_synchronise done |
// mod_rdp::send_control |
// send data request |
// Sending on RDP Wab Target (4) 40 bytes |
/* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x1a\x1a\x00" // |...(...d. ..p... |
/* 0010 */ "\x17\x00\x09\x04\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00" // |................ |
/* 0020 */ "\x04\x00\x00\x00\x00\x00\x00\x00"                                 //........ |
// Sent dumped on RDP Wab Target (4) 40 bytes |
// send data request done |
// mod_rdp::send_control done |
// mod_rdp::send_control |
// send data request |
// Sending on RDP Wab Target (4) 40 bytes |
/* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x1a\x1a\x00" // |...(...d. ..p... |
/* 0010 */ "\x17\x00\x09\x04\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00" // |................ |
/* 0020 */ "\x01\x00\x00\x00\x00\x00\x00\x00"                                 //........ |
// Sent dumped on RDP Wab Target (4) 40 bytes |
// send data request done |
// mod_rdp::send_control done |
// use rdp5 |
// mod_rdp::send_fonts |
// send data request |
// Sending on RDP Wab Target (4) 40 bytes |
/* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x1a\x1a\x00" // |...(...d. ..p... |
/* 0010 */ "\x17\x00\x09\x04\x02\x00\x01\x00\x00\x02\x1a\x00\x27\x00\x00\x00" // |............'... |
/* 0020 */ "\x00\x00\x00\x00\x03\x00\x32\x00"                                 //......2. |
// Sent dumped on RDP Wab Target (4) 40 bytes |
// send data request done |
// mod_rdp::send_fonts done |
// mod_rdp::send_input_slowpath |
// send data request |
// Sending on RDP Wab Target (4) 48 bytes |
/* 0000 */ "\x03\x00\x00\x30\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x22\x22\x00" // |...0...d. ..p"". |
/* 0010 */ "\x17\x00\x09\x04\x02\x00\x01\x00\x00\x04\x22\x00\x1c\x00\x00\x00" // |.........."..... |
/* 0020 */ "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// Sent dumped on RDP Wab Target (4) 48 bytes |
// send data request done |
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
// /* 0000 */ "\x00\x00\x24"                                                     //..$ |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 32 bytes |
// Recv done on RDP Wab Target (4) 32 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x16\x16\x00\x17\x00\x09\x04" // |...h. ..p....... |
// /* 0010 */ "\x02\x00\x01\x00\x00\x02\x16\x00\x1f\x00\x00\x00\x01\x00\xea\x03" // |................ |
// Dump done on RDP Wab Target (4) 32 bytes |
// LOOPING on PDUs: 22 |
// PDUTYPE_DATAPDU |
// WAITING_SYNCHRONIZE |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x28"                                                     //..( |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 36 bytes |
// Recv done on RDP Wab Target (4) 36 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x1a\x1a\x00\x17\x00\x09\x04" // |...h. ..p....... |
// /* 0010 */ "\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00\x04\x00\x00\x00" // |................ |
// /* 0020 */ "\xea\x03\x00\x00"                                                 //.... |
// Dump done on RDP Wab Target (4) 36 bytes |
// LOOPING on PDUs: 26 |
// PDUTYPE_DATAPDU |
// WAITING_CTL_COOPERATE |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x28"                                                     //..( |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 36 bytes |
// Recv done on RDP Wab Target (4) 36 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x1a\x1a\x00\x17\x00\x09\x04" // |...h. ..p....... |
// /* 0010 */ "\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00\x02\x00\x00\x00" // |................ |
// /* 0020 */ "\xea\x03\x00\x00"                                                 //.... |
// Dump done on RDP Wab Target (4) 36 bytes |
// LOOPING on PDUs: 26 |
// PDUTYPE_DATAPDU |
// WAITING_GRANT_CONTROL_COOPERATE |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\xcd"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 201 bytes |
// Recv done on RDP Wab Target (4) 201 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x80\xbe\xbe\x00\x17\x00\x09" // |...h. ..p....... |
// /* 0010 */ "\x04\x02\x00\x01\x00\x00\x02\xbe\x00\x28\x00\x00\x00\xff\x02\xb6" // |.........(...... |
// /* 0020 */ "\x00\x28\x00\x00\x00\x27\x00\x27\x00\x03\x00\x04\x00\x00\x00\x26" // |.(...'.'.......& |
// /* 0030 */ "\x00\x01\x00\x1e\x00\x02\x00\x1f\x00\x03\x00\x1d\x00\x04\x00\x27" // |...............' |
// /* 0040 */ "\x00\x05\x00\x0b\x00\x06\x00\x28\x00\x08\x00\x21\x00\x09\x00\x20" // |.......(...!...  |
// /* 0050 */ "\x00\x0a\x00\x22\x00\x0b\x00\x25\x00\x0c\x00\x24\x00\x0d\x00\x23" // |..."...%...$...# |
// /* 0060 */ "\x00\x0e\x00\x19\x00\x0f\x00\x16\x00\x10\x00\x15\x00\x11\x00\x1c" // |................ |
// /* 0070 */ "\x00\x12\x00\x1b\x00\x13\x00\x1a\x00\x14\x00\x17\x00\x15\x00\x18" // |................ |
// /* 0080 */ "\x00\x16\x00\x0e\x00\x18\x00\x0c\x00\x19\x00\x0d\x00\x1a\x00\x12" // |................ |
// /* 0090 */ "\x00\x1b\x00\x14\x00\x1f\x00\x13\x00\x20\x00\x00\x00\x21\x00\x0a" // |......... ...!.. |
// /* 00a0 */ "\x00\x22\x00\x06\x00\x23\x00\x07\x00\x24\x00\x08\x00\x25\x00\x09" // |."...#...$...%.. |
// /* 00b0 */ "\x00\x26\x00\x04\x00\x27\x00\x03\x00\x28\x00\x02\x00\x29\x00\x01" // |.&...'...(...).. |
// /* 00c0 */ "\x00\x2a\x00\x05\x00\x2b\x00\x2a\x00"                             //.*...+.*. |
// Dump done on RDP Wab Target (4) 201 bytes |
// LOOPING on PDUs: 190 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_FONTMAP |
// mod_rdp::send_input_slowpath |
// send data request |
// Sending on RDP Wab Target (4) 48 bytes |
/* 0000 */ "\x03\x00\x00\x30\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x22\x22\x00" // |...0...d. ..p"". |
/* 0010 */ "\x17\x00\x09\x04\x02\x00\x01\x00\x00\x04\x22\x00\x1c\x00\x00\x00" // |.........."..... |
/* 0020 */ "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00" // |................ |
// Sent dumped on RDP Wab Target (4) 48 bytes |
// send data request done |
// mod_rdp::send_input_slowpath done |
// ========= CREATION OF MOD DONE ==================== |
// ===================> count = 0 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x24"                                                     //..$ |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 32 bytes |
// Recv done on RDP Wab Target (4) 32 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x16\x16\x00\x17\x00\x09\x04" // |...h. ..p....... |
// /* 0010 */ "\x02\x00\x01\x00\x00\x02\x16\x00\x02\x00\x00\x00\x03\x00\x00\x00" // |................ |
// Dump done on RDP Wab Target (4) 32 bytes |
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
// /* 0000 */ "\x00\x04\x71"                                                     //..q |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 1133 bytes |
// Recv done on RDP Wab Target (4) 1133 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x84\x62\x62\x04\x17\x00\x09" // |...h. ..p.bb.... |
// /* 0010 */ "\x04\x02\x00\x01\x00\x00\x02\x62\x04\x02\x00\x00\x00\x00\x00\x00" // |.......b........ |
// /* 0020 */ "\x00\x07\x00\x00\x00\x03\xfc\x03\x00\x00\x01\x00\x00\x01\x00\x00" // |................ |
// /* 0030 */ "\x00\x00\x55\x00\x00\x00\xaa\x00\x00\x00\xff\x00\x00\x00\x00\x24" // |..U............$ |
// /* 0040 */ "\x00\x00\x55\x24\x00\x00\xaa\x24\x00\x00\xff\x24\x00\x00\x00\x49" // |..U$...$...$...I |
// /* 0050 */ "\x00\x00\x55\x49\x00\x00\xaa\x49\x00\x00\xff\x49\x00\x00\x00\x6d" // |..UI...I...I...m |
// /* 0060 */ "\x00\x00\x55\x6d\x00\x00\xaa\x6d\x00\x00\xff\x6d\x00\x00\x00\x92" // |..Um...m...m.... |
// /* 0070 */ "\x00\x00\x55\x92\x00\x00\xaa\x92\x00\x00\xff\x92\x00\x00\x00\xb6" // |..U............. |
// /* 0080 */ "\x00\x00\x55\xb6\x00\x00\xaa\xb6\x00\x00\xff\xb6\x00\x00\x00\xdb" // |..U............. |
// /* 0090 */ "\x00\x00\x55\xdb\x00\x00\xaa\xdb\x00\x00\xff\xdb\x00\x00\x00\xff" // |..U............. |
// /* 00a0 */ "\x00\x00\x55\xff\x00\x00\xaa\xff\x00\x00\xff\xff\x00\x00\x00\x00" // |..U............. |
// /* 00b0 */ "\x24\x00\x55\x00\x24\x00\xaa\x00\x24\x00\xff\x00\x24\x00\x00\x24" // |$.U.$...$...$..$ |
// /* 00c0 */ "\x24\x00\x55\x24\x24\x00\xaa\x24\x24\x00\xff\x24\x24\x00\x00\x49" // |$.U$$..$$..$$..I |
// /* 00d0 */ "\x24\x00\x55\x49\x24\x00\xaa\x49\x24\x00\xff\x49\x24\x00\x00\x6d" // |$.UI$..I$..I$..m |
// /* 00e0 */ "\x24\x00\x55\x6d\x24\x00\xaa\x6d\x24\x00\xff\x6d\x24\x00\x00\x92" // |$.Um$..m$..m$... |
// /* 00f0 */ "\x24\x00\x55\x92\x24\x00\xaa\x92\x24\x00\xff\x92\x24\x00\x00\xb6" // |$.U.$...$...$... |
// /* 0100 */ "\x24\x00\x55\xb6\x24\x00\xaa\xb6\x24\x00\xff\xb6\x24\x00\x00\xdb" // |$.U.$...$...$... |
// /* 0110 */ "\x24\x00\x55\xdb\x24\x00\xaa\xdb\x24\x00\xff\xdb\x24\x00\x00\xff" // |$.U.$...$...$... |
// /* 0120 */ "\x24\x00\x55\xff\x24\x00\xaa\xff\x24\x00\xff\xff\x24\x00\x00\x00" // |$.U.$...$...$... |
// /* 0130 */ "\x49\x00\x55\x00\x49\x00\xaa\x00\x49\x00\xff\x00\x49\x00\x00\x24" // |I.U.I...I...I..$ |
// /* 0140 */ "\x49\x00\x55\x24\x49\x00\xaa\x24\x49\x00\xff\x24\x49\x00\x00\x49" // |I.U$I..$I..$I..I |
// /* 0150 */ "\x49\x00\x55\x49\x49\x00\xaa\x49\x49\x00\xff\x49\x49\x00\x00\x6d" // |I.UII..II..II..m |
// /* 0160 */ "\x49\x00\x55\x6d\x49\x00\xaa\x6d\x49\x00\xff\x6d\x49\x00\x00\x92" // |I.UmI..mI..mI... |
// /* 0170 */ "\x49\x00\x55\x92\x49\x00\xaa\x92\x49\x00\xff\x92\x49\x00\x00\xb6" // |I.U.I...I...I... |
// /* 0180 */ "\x49\x00\x55\xb6\x49\x00\xaa\xb6\x49\x00\xff\xb6\x49\x00\x00\xdb" // |I.U.I...I...I... |
// /* 0190 */ "\x49\x00\x55\xdb\x49\x00\xaa\xdb\x49\x00\xff\xdb\x49\x00\x00\xff" // |I.U.I...I...I... |
// /* 01a0 */ "\x49\x00\x55\xff\x49\x00\xaa\xff\x49\x00\xff\xff\x49\x00\x00\x00" // |I.U.I...I...I... |
// /* 01b0 */ "\x6d\x00\x55\x00\x6d\x00\xaa\x00\x6d\x00\xff\x00\x6d\x00\x00\x24" // |m.U.m...m...m..$ |
// /* 01c0 */ "\x6d\x00\x55\x24\x6d\x00\xaa\x24\x6d\x00\xff\x24\x6d\x00\x00\x49" // |m.U$m..$m..$m..I |
// /* 01d0 */ "\x6d\x00\x55\x49\x6d\x00\xaa\x49\x6d\x00\xff\x49\x6d\x00\x00\x6d" // |m.UIm..Im..Im..m |
// /* 01e0 */ "\x6d\x00\x55\x6d\x6d\x00\xaa\x6d\x6d\x00\xff\x6d\x6d\x00\x00\x92" // |m.Umm..mm..mm... |
// /* 01f0 */ "\x6d\x00\x55\x92\x6d\x00\xaa\x92\x6d\x00\xff\x92\x6d\x00\x00\xb6" // |m.U.m...m...m... |
// /* 0200 */ "\x6d\x00\x55\xb6\x6d\x00\xaa\xb6\x6d\x00\xff\xb6\x6d\x00\x00\xdb" // |m.U.m...m...m... |
// /* 0210 */ "\x6d\x00\x55\xdb\x6d\x00\xaa\xdb\x6d\x00\xff\xdb\x6d\x00\x00\xff" // |m.U.m...m...m... |
// /* 0220 */ "\x6d\x00\x55\xff\x6d\x00\xaa\xff\x6d\x00\xff\xff\x6d\x00\x00\x00" // |m.U.m...m...m... |
// /* 0230 */ "\x92\x00\x55\x00\x92\x00\xaa\x00\x92\x00\xff\x00\x92\x00\x00\x24" // |..U............$ |
// /* 0240 */ "\x92\x00\x55\x24\x92\x00\xaa\x24\x92\x00\xff\x24\x92\x00\x00\x49" // |..U$...$...$...I |
// /* 0250 */ "\x92\x00\x55\x49\x92\x00\xaa\x49\x92\x00\xff\x49\x92\x00\x00\x6d" // |..UI...I...I...m |
// /* 0260 */ "\x92\x00\x55\x6d\x92\x00\xaa\x6d\x92\x00\xff\x6d\x92\x00\x00\x92" // |..Um...m...m.... |
// /* 0270 */ "\x92\x00\x55\x92\x92\x00\xaa\x92\x92\x00\xff\x92\x92\x00\x00\xb6" // |..U............. |
// /* 0280 */ "\x92\x00\x55\xb6\x92\x00\xaa\xb6\x92\x00\xff\xb6\x92\x00\x00\xdb" // |..U............. |
// /* 0290 */ "\x92\x00\x55\xdb\x92\x00\xaa\xdb\x92\x00\xff\xdb\x92\x00\x00\xff" // |..U............. |
// /* 02a0 */ "\x92\x00\x55\xff\x92\x00\xaa\xff\x92\x00\xff\xff\x92\x00\x00\x00" // |..U............. |
// /* 02b0 */ "\xb6\x00\x55\x00\xb6\x00\xaa\x00\xb6\x00\xff\x00\xb6\x00\x00\x24" // |..U............$ |
// /* 02c0 */ "\xb6\x00\x55\x24\xb6\x00\xaa\x24\xb6\x00\xff\x24\xb6\x00\x00\x49" // |..U$...$...$...I |
// /* 02d0 */ "\xb6\x00\x55\x49\xb6\x00\xaa\x49\xb6\x00\xff\x49\xb6\x00\x00\x6d" // |..UI...I...I...m |
// /* 02e0 */ "\xb6\x00\x55\x6d\xb6\x00\xaa\x6d\xb6\x00\xff\x6d\xb6\x00\x00\x92" // |..Um...m...m.... |
// /* 02f0 */ "\xb6\x00\x55\x92\xb6\x00\xaa\x92\xb6\x00\xff\x92\xb6\x00\x00\xb6" // |..U............. |
// /* 0300 */ "\xb6\x00\x55\xb6\xb6\x00\xaa\xb6\xb6\x00\xff\xb6\xb6\x00\x00\xdb" // |..U............. |
// /* 0310 */ "\xb6\x00\x55\xdb\xb6\x00\xaa\xdb\xb6\x00\xff\xdb\xb6\x00\x00\xff" // |..U............. |
// /* 0320 */ "\xb6\x00\x55\xff\xb6\x00\xaa\xff\xb6\x00\xff\xff\xb6\x00\x00\x00" // |..U............. |
// /* 0330 */ "\xdb\x00\x55\x00\xdb\x00\xaa\x00\xdb\x00\xff\x00\xdb\x00\x00\x24" // |..U............$ |
// /* 0340 */ "\xdb\x00\x55\x24\xdb\x00\xaa\x24\xdb\x00\xff\x24\xdb\x00\x00\x49" // |..U$...$...$...I |
// /* 0350 */ "\xdb\x00\x55\x49\xdb\x00\xaa\x49\xdb\x00\xff\x49\xdb\x00\x00\x6d" // |..UI...I...I...m |
// /* 0360 */ "\xdb\x00\x55\x6d\xdb\x00\xaa\x6d\xdb\x00\xff\x6d\xdb\x00\x00\x92" // |..Um...m...m.... |
// /* 0370 */ "\xdb\x00\x55\x92\xdb\x00\xaa\x92\xdb\x00\xff\x92\xdb\x00\x00\xb6" // |..U............. |
// /* 0380 */ "\xdb\x00\x55\xb6\xdb\x00\xaa\xb6\xdb\x00\xff\xb6\xdb\x00\x00\xdb" // |..U............. |
// /* 0390 */ "\xdb\x00\x55\xdb\xdb\x00\xaa\xdb\xdb\x00\xff\xdb\xdb\x00\x00\xff" // |..U............. |
// /* 03a0 */ "\xdb\x00\x55\xff\xdb\x00\xaa\xff\xdb\x00\xff\xff\xdb\x00\x00\x00" // |..U............. |
// /* 03b0 */ "\xff\x00\x55\x00\xff\x00\xaa\x00\xff\x00\xff\x00\xff\x00\x00\x24" // |..U............$ |
// /* 03c0 */ "\xff\x00\x55\x24\xff\x00\xaa\x24\xff\x00\xff\x24\xff\x00\x00\x49" // |..U$...$...$...I |
// /* 03d0 */ "\xff\x00\x55\x49\xff\x00\xaa\x49\xff\x00\xff\x49\xff\x00\x00\x6d" // |..UI...I...I...m |
// /* 03e0 */ "\xff\x00\x55\x6d\xff\x00\xaa\x6d\xff\x00\xff\x6d\xff\x00\x00\x92" // |..Um...m...m.... |
// /* 03f0 */ "\xff\x00\x55\x92\xff\x00\xaa\x92\xff\x00\xff\x92\xff\x00\x00\xb6" // |..U............. |
// /* 0400 */ "\xff\x00\x55\xb6\xff\x00\xaa\xb6\xff\x00\xff\xb6\xff\x00\x00\xdb" // |..U............. |
// /* 0410 */ "\xff\x00\x55\xdb\xff\x00\xaa\xdb\xff\x00\xff\xdb\xff\x00\x00\xff" // |..U............. |
// /* 0420 */ "\xff\x00\x55\xff\xff\x00\xaa\xff\xff\x00\xff\xff\xff\x00\x09\x0a" // |..U............. |
// /* 0430 */ "\x3f\x10\x00\x3f\x00\xc5\x00\x12\x00\xff\xff\x01\x3f\x12\x00\x40" // |?..?........?..@ |
// /* 0440 */ "\x00\x01\x00\x0f\x00\x51\x8c\x01\x3f\x0f\x00\x3e\x00\xc6\x00\x01" // |.....Q..?..>.... |
// /* 0450 */ "\x00\x73\x02\x01\x0e\x3f\x00\x01\x00\x12\x00\x01\x0b\xd5\x00\x3e" // |.s...?.........> |
// /* 0460 */ "\x00\x14\x00\x01\x0f\x0f\x00\x51\x00\xc7\x00\x01\x00"             //.......Q..... |
// Dump done on RDP Wab Target (4) 1133 bytes |
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
// /* 0000 */ "\x00\x0a\x8c"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 2696 bytes |
// Recv done on RDP Wab Target (4) 2696 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x8a\x7d\x7d\x0a\x17\x00\x09" // |...h. ..p.}}.... |
// /* 0010 */ "\x04\x02\x00\x01\x00\x00\x02\x7d\x0a\x02\x00\x00\x00\x00\x00\x00" // |.......}........ |
// /* 0020 */ "\x00\x95\x00\x00\x00\x01\x3f\x00\x00\x00\x00\x00\x04\x0a\x00\xec" // |......?......... |
// /* 0030 */ "\x08\x01\x0e\x0a\x00\x14\x00\x10\x00\x01\x05\x7f\x00\x3d\x03\x01" // |.............=.. |
// /* 0040 */ "\x05\xec\x03\x14\x00\x01\x0f\x00\x00\x1a\x00\xbc\x03\x07\x00\x01" // |................ |
// /* 0050 */ "\x05\xec\x03\x14\x00\x01\x0f\x00\x00\x21\x00\x00\x04\x08\x00\x01" // |.........!...... |
// /* 0060 */ "\x0e\x29\x00\x0f\x00\x10\x00\x01\x01\xf1\x03\x01\x0f\x00\x00\x39" // |.).............9 |
// /* 0070 */ "\x00\x00\x04\x05\x00\x01\x0e\x3e\x00\x0f\x00\x14\x00\x01\x05\xd6" // |.......>........ |
// /* 0080 */ "\x00\x05\x00\x01\x01\x98\x03\x11\x05\x59\x0a\x01\x0f\x00\x00\x52" // |.........Y.....R |
// /* 0090 */ "\x00\x00\x04\x6c\x02\x01\x0e\xbe\x02\x13\x03\x06\x00\x01\x0e\xc4" // |...l............ |
// /* 00a0 */ "\x02\x3c\x00\x11\x00\x01\x05\x6a\x00\xa9\x02\x01\x0f\x34\x03\xbe" // |.<.....j.....4.. |
// /* 00b0 */ "\x02\x0f\x00\x17\x00\x11\x0d\x29\x36\xea\x11\x05\x5f\xca\x11\x05" // |.......)6..._... |
// /* 00c0 */ "\x30\x05\x01\x0f\x5d\x03\xbf\x02\x0f\x00\x14\x00\x11\x0d\x28\x0e" // |0...].........(. |
// /* 00d0 */ "\xee\x11\x05\x37\xf2\x11\x05\x30\x05\x11\x0f\xa7\x02\xfb\x0e\x11" // |...7...0........ |
// /* 00e0 */ "\x01\x29\x11\x05\x30\x05\x11\x0f\x99\x10\x09\xf2\x11\x05\x37\xf2" // |.)..0.........7. |
// /* 00f0 */ "\x11\x05\x30\x05\x01\x07\x5d\x03\xd3\x02\x45\x00\x11\x05\x5f\xca" // |..0...]...E..._. |
// /* 0100 */ "\x11\x05\x30\x05\x01\x0f\x00\x00\xd5\x02\x3c\x00\x0a\x00\x01\x05" // |..0.......<..... |
// /* 0110 */ "\x6a\x00\x96\x03\x01\x0f\x00\x00\xdf\x02\x3c\x00\x03\x00\x01\x05" // |j.........<..... |
// /* 0120 */ "\x6a\x00\xc0\x02\x11\x0f\x96\x03\x6a\x11\x01\x0f\x69\x03\xdf\x02" // |j.......j...i... |
// /* 0130 */ "\x28\x00\x17\x00\x11\x05\x72\xfd\x01\x0f\x00\x00\xf6\x02\x00\x04" // |(.....r......... |
// /* 0140 */ "\x0a\x00\x01\x0f\x14\x00\x0a\x00\x6b\x00\x10\x00\x03\x25\x00\x08" // |........k....%.. |
// /* 0150 */ "\x00\x03\x07\x01\x00\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00" // |................ |
// /* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x41\x00\x22\x00\x14\x00\x08\x00" // |........A."..... |
// /* 0170 */ "\x08\x00\x14\x00\x22\x00\x41\x00\x00\x00\x00\x00\x00\x00\x03\x25" // |....".A........% |
// /* 0180 */ "\x00\x08\x00\x03\x07\x01\x01\x00\x01\x00\xf0\xff\x10\x00\x10\x00" // |................ |
// /* 0190 */ "\x00\x00\x00\x00\x00\x00\x07\xc0\x18\x20\x20\x10\x27\x48\x4c\xc8" // |.........  .'HL. |
// /* 01a0 */ "\x48\x48\x48\x48\x4c\xd0\x27\x60\x20\x00\x18\x20\x07\xc0\x00\x00" // |HHHHL.'` .. .... |
// /* 01b0 */ "\x03\x25\x00\x08\x00\x03\x07\x01\x02\x00\x02\x00\xf0\xff\x10\x00" // |.%.............. |
// /* 01c0 */ "\x10\x00\x00\x00\x00\x00\x00\x00\x18\x00\x28\x00\x08\x00\x08\x00" // |..........(..... |
// /* 01d0 */ "\x08\x00\x08\x00\x08\x00\x08\x00\x08\x00\x3e\x00\x00\x00\x00\x00" // |..........>..... |
// /* 01e0 */ "\x00\x00\x03\x25\x00\x08\x00\x03\x07\x01\x03\x00\x01\x00\xf0\xff" // |...%............ |
// /* 01f0 */ "\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x1c\x00\x22\x00\x41\x00" // |............".A. |
// /* 0200 */ "\x41\x00\x41\x00\x41\x00\x41\x00\x41\x00\x22\x00\x1c\x00\x00\x00" // |A.A.A.A.A."..... |
// /* 0210 */ "\x00\x00\x00\x00\x03\x15\x00\x08\x00\x03\x07\x01\x04\x00\x02\x00" // |................ |
// /* 0220 */ "\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0230 */ "\x00\x20\x20\x00\x00\x00\x03\x25\x00\x08\x00\x03\x07\x01\x05\x00" // |.  ....%........ |
// /* 0240 */ "\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x06\x00" // |................ |
// /* 0250 */ "\x0a\x00\x12\x00\x12\x00\x22\x00\x42\x00\x7f\x00\x02\x00\x02\x00" // |......".B....... |
// /* 0260 */ "\x02\x00\x00\x00\x00\x00\x00\x00\x03\x25\x00\x08\x00\x03\x07\x01" // |.........%...... |
// /* 0270 */ "\x06\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0280 */ "\x3e\x00\x43\x00\x01\x00\x01\x00\x1e\x00\x03\x00\x01\x00\x01\x00" // |>.C............. |
// /* 0290 */ "\x43\x00\x3c\x00\x00\x00\x00\x00\x00\x00\x0d\x1b\xfb\x3f\x3f\xf0" // |C.<..........??. |
// /* 02a0 */ "\x14\x0a\x7e\x19\x07\x03\x01\xff\xff\x00\xec\x08\x00\x14\x00\x0a" // |..~............. |
// /* 02b0 */ "\x00\x7f\x00\x1a\x00\x14\x00\x0a\x00\x7f\x00\x1a\x00\x03\xaa\xaa" // |................ |
// /* 02c0 */ "\x55\xaa\x55\xaa\x55\xaa\x13\x00\x1a\x00\x1a\x00\x00\x01\x09\x02" // |U.U.U........... |
// /* 02d0 */ "\x0e\x03\x09\x04\x09\x02\x04\x03\x09\x04\x09\x05\x04\x06\x09\x04" // |................ |
// /* 02e0 */ "\x09\x06\x04\x06\x09\x19\x0a\x37\xfb\x1f\x61\x98\x43\x03\x25\x00" // |.......7..a.C.%. |
// /* 02f0 */ "\x08\x00\x03\x07\x01\x07\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00" // |................ |
// /* 0300 */ "\x00\x00\x00\x00\x00\xff\x80\x08\x00\x08\x00\x08\x00\x08\x00\x08" // |................ |
// /* 0310 */ "\x00\x08\x00\x08\x00\x08\x00\x08\x00\x00\x00\x00\x00\x00\x00\x03" // |................ |
// /* 0320 */ "\x15\x00\x08\x00\x03\x07\x01\x08\x00\x01\x00\xf0\xff\x08\x00\x10" // |................ |
// /* 0330 */ "\x00\x00\x00\x00\x00\x00\x3c\x42\x02\x3e\x42\x42\x46\x3a\x00\x00" // |......<B.>BBF:.. |
// /* 0340 */ "\x00\x03\x15\x00\x08\x00\x03\x07\x01\x09\x00\x01\x00\xf0\xff\x08" // |................ |
// /* 0350 */ "\x00\x10\x00\x00\x00\x00\x00\x00\x58\x60\x40\x40\x40\x40\x40\x40" // |........X`@@@@@@ |
// /* 0360 */ "\x00\x00\x00\x03\x25\x00\x08\x00\x03\x07\x01\x0a\x00\x01\x00\xf0" // |....%........... |
// /* 0370 */ "\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1d" // |................ |
// /* 0380 */ "\x00\x23\x00\x41\x00\x41\x00\x41\x00\x41\x00\x23\x00\x1d\x00\x01" // |.#.A.A.A.A.#.... |
// /* 0390 */ "\x00\x23\x00\x1e\x00\x03\x25\x00\x08\x00\x03\x07\x01\x0b\x00\x01" // |.#....%......... |
// /* 03a0 */ "\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 03b0 */ "\x00\x1e\x00\x23\x00\x41\x00\x7f\x00\x40\x00\x40\x00\x21\x00\x1e" // |...#.A...@.@.!.. |
// /* 03c0 */ "\x00\x00\x00\x00\x00\x00\x00\x03\x15\x00\x08\x00\x03\x07\x01\x0c" // |................ |
// /* 03d0 */ "\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x40\x40\xf8\x40" // |............@@.@ |
// /* 03e0 */ "\x40\x40\x40\x40\x40\x38\x00\x00\x00\x03\x15\x00\x08\x00\x03\x07" // |@@@@@8.......... |
// /* 03f0 */ "\x01\x0d\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0400 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x25\x00\x08\x00" // |............%... |
// /* 0410 */ "\x03\x07\x01\x0e\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00" // |................ |
// /* 0420 */ "\x00\x00\x00\x0f\x80\x30\x40\x20\x00\x40\x00\x40\x00\x41\xc0\x40" // |.....0@ .@.@.A.@ |
// /* 0430 */ "\x40\x20\x40\x30\x40\x0f\x80\x00\x00\x00\x00\x00\x00\x03\x25\x00" // |@ @0@.........%. |
// /* 0440 */ "\x08\x00\x03\x07\x01\x0f\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00" // |................ |
// /* 0450 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1c\x00\x22\x00\x41\x00\x41" // |...........".A.A |
// /* 0460 */ "\x00\x41\x00\x41\x00\x22\x00\x1c\x00\x00\x00\x00\x00\x00\x00\x03" // |.A.A.".......... |
// /* 0470 */ "\x25\x00\x08\x00\x03\x07\x01\x10\x00\x01\x00\xf0\xff\x10\x00\x10" // |%............... |
// /* 0480 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x41\x00\x41\x00\x41" // |...........A.A.A |
// /* 0490 */ "\x00\x41\x00\x41\x00\x41\x00\x63\x00\x3d\x00\x00\x00\x00\x00\x00" // |.A.A.A.c.=...... |
// /* 04a0 */ "\x00\x03\x25\x00\x08\x00\x03\x07\x01\x11\x00\x01\x00\xf0\xff\x10" // |..%............. |
// /* 04b0 */ "\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x5c\x00\x62" // |...............b |
// /* 04c0 */ "\x00\x41\x00\x41\x00\x41\x00\x41\x00\x62\x00\x5c\x00\x40\x00\x40" // |.A.A.A.A.b...@.@ |
// /* 04d0 */ "\x00\x40\x00\x0d\x1b\xa0\x3b\x30\xf0\xfb\x1f\x5c\x1f\x98\x43\x00" // |.@....;0......C. |
// /* 04e0 */ "\x29\x00\x70\x00\x39\x00\x29\x00\x70\x00\x39\x00\x39\x00\x18\x07" // |).p.9.).p.9.9... |
// /* 04f0 */ "\x00\x08\x09\x09\x08\x0a\x05\x0b\x09\x0c\x09\x0d\x05\x0e\x04\x09" // |................ |
// /* 0500 */ "\x0b\x0f\x05\x10\x09\x11\x09\x09\x0a\x05\xdb\x00\xc2\x02\x0d\x1b" // |................ |
// /* 0510 */ "\x40\x15\x28\x05\xdb\x00\x9c\x03\xe0\x00\x0d\x01\xe0\x00\x0d\x01" // |@.(............. |
// /* 0520 */ "\xdf\x00\x0c\x07\x00\x08\x09\x09\x08\x0a\x05\x0b\x09\x0c\x09\x09" // |................ |
// /* 0530 */ "\x0a\x05\x9d\x03\x54\x00\x03\x25\x00\x08\x00\x03\x07\x01\x12\x00" // |....T..%........ |
// /* 0540 */ "\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x7e\x00" // |..............~. |
// /* 0550 */ "\x43\x00\x41\x00\x41\x00\x43\x00\x7e\x00\x40\x00\x40\x00\x40\x00" // |C.A.A.C.~.@.@.@. |
// /* 0560 */ "\x40\x00\x00\x00\x00\x00\x00\x00\x03\x15\x00\x08\x00\x03\x07\x01" // |@............... |
// /* 0570 */ "\x13\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x00\x00\x1c" // |................ |
// /* 0580 */ "\x22\x40\x40\x40\x40\x22\x1c\x00\x00\x00\x03\x15\x00\x08\x00\x03" // |"@@@@".......... |
// /* 0590 */ "\x07\x01\x14\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x40\x40" // |..............@@ |
// /* 05a0 */ "\x40\x40\x40\x40\x40\x40\x40\x40\x40\x00\x00\x00\x0d\x1b\x40\x15" // |@@@@@@@@@.....@. |
// /* 05b0 */ "\x28\x41\x9d\x03\x54\xa2\x03\xdb\x03\xa2\x03\xdb\x03\xa1\x03\x10" // |(A..T........... |
// /* 05c0 */ "\x12\x00\x09\x09\x0f\x05\x0c\x09\x0f\x05\x13\x09\x0f\x08\x14\x09" // |................ |
// /* 05d0 */ "\x09\x0a\x3f\x10\x00\x3f\x00\xc5\x00\x12\x00\xff\xff\x01\x3f\x12" // |..?..?........?. |
// /* 05e0 */ "\x00\x40\x00\x01\x00\x0f\x00\x51\x8c\x01\x3f\x0f\x00\x3e\x00\xc6" // |.@.....Q..?..>.. |
// /* 05f0 */ "\x00\x01\x00\x73\x02\x01\x0e\x3f\x00\x01\x00\x12\x00\x01\x0b\xd5" // |...s...?........ |
// /* 0600 */ "\x00\x3e\x00\x14\x00\x01\x0f\x0f\x00\x51\x00\xc7\x00\x01\x00\x01" // |.>.......Q...... |
// /* 0610 */ "\x3f\xdc\x00\x3f\x00\xbb\x02\x12\x00\xff\xff\x11\x0f\xff\xff\x01" // |?..?............ |
// /* 0620 */ "\xef\x01\x0e\x3f\x00\x01\x00\x12\x00\x01\x0b\x97\x03\x3e\x00\x14" // |...?.........>.. |
// /* 0630 */ "\x00\x01\x0f\xdb\x00\x51\x00\xbd\x02\x01\x00\x01\x0f\x9e\x03\x3f" // |.....Q.........? |
// /* 0640 */ "\x00\x52\x00\x12\x00\x11\x0f\xff\xff\x01\xef\x11\x0e\x01\xae\x11" // |.R.............. |
// /* 0650 */ "\x11\x0b\x53\xff\x02\x11\x0f\xad\x13\x53\xed\x11\x3f\x1f\xb9\xdc" // |..S......S..?... |
// /* 0660 */ "\x16\xec\x08\x11\x0f\x01\x01\xfd\xfd\x03\x15\x00\x08\x00\x03\x07" // |................ |
// /* 0670 */ "\x01\x15\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x7e\x40" // |..............~@ |
// /* 0680 */ "\x40\x40\x7e\x40\x40\x40\x40\x40\x00\x00\x00\x03\x15\x00\x08\x00" // |@@~@@@@@........ |
// /* 0690 */ "\x03\x07\x01\x16\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x40" // |...............@ |
// /* 06a0 */ "\x40\x00\x40\x40\x40\x40\x40\x40\x40\x40\x00\x00\x00\x09\x1b\xe0" // |@.@@@@@@@@...... |
// /* 06b0 */ "\x3f\x38\xec\x08\x00\xc3\x03\x0d\x00\xe4\x03\x1d\x00\xc3\x03\x0d" // |?8.............. |
// /* 06c0 */ "\x00\xe4\x03\x1d\x00\xc2\x03\x1d\x00\x0c\x15\x00\x16\x08\x14\x03" // |................ |
// /* 06d0 */ "\x0c\x03\x0b\x05\x09\x09\x19\x0a\x3f\xff\xff\x01\xee\xff\xff\x11" // |........?....... |
// /* 06e0 */ "\x0e\x02\xd4\x13\x11\x0b\x2e\xfe\x02\x11\x0f\xd2\x15\x2e\xeb\x01" // |................ |
// /* 06f0 */ "\x3f\x13\x03\xbe\x02\x21\x00\x17\x00\xec\x08\x11\x0f\x01\x01\xfd" // |?....!.......... |
// /* 0700 */ "\xfd\x03\x25\x00\x08\x00\x03\x07\x01\x17\x00\x01\x00\xf0\xff\x10" // |..%............. |
// /* 0710 */ "\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\x00\xe0\x03" // |............ ... |
// /* 0720 */ "\xe0\x0f\xe0\x3f\xe0\xff\xe0\x3f\xe0\x0f\xe0\x03\xe0\x00\xe0\x00" // |...?...?........ |
// /* 0730 */ "\x20\x00\x00\x03\x15\x00\x08\x00\x03\x07\x01\x18\x00\x01\x00\xf0" // | ............... |
// /* 0740 */ "\xff\x08\x00\x10\x00\x00\x00\x00\x00\x00\x00\x02\x0e\x3e\xfe\x3e" // |.............>.> |
// /* 0750 */ "\x0e\x02\x00\x00\x00\x09\x1b\xc0\x3f\x38\x1a\x03\xc1\x02\x2c\x03" // |........?8....,. |
// /* 0760 */ "\xd1\x02\x1a\x03\xc1\x02\x2c\x03\xd1\x02\x19\x03\xd1\x02\x04\x17" // |......,......... |
// /* 0770 */ "\x00\x18\x0b\x19\x0a\x0f\x2f\xff\xfc\x03\x11\x0f\x01\x01\xfd\xfd" // |....../......... |
// /* 0780 */ "\x09\x1b\x40\x15\x28\x4a\x03\x55\x03\x4a\x03\x55\x03\x49\x03\x02" // |..@.(J.U.J.U.I.. |
// /* 0790 */ "\x17\x00\x19\x0a\x3b\x29\x01\xfe\xff\xff\x09\x1b\x70\x15\x28\x00" // |....;)......p.(. |
// /* 07a0 */ "\x00\x00\xff\xff\x00\x6e\x03\x77\x03\x6e\x03\x77\x03\x6c\x03\x02" // |.....n.w.n.w.l.. |
// /* 07b0 */ "\x02\x00\x19\x0a\x0f\xff\xff\x01\xef\x11\x0e\x01\xe9\x11\x11\x0b" // |................ |
// /* 07c0 */ "\x18\xff\x02\x11\x0f\xe8\x13\x18\xed\x11\x3f\x19\xef\xf5\x0f\xec" // |..........?..... |
// /* 07d0 */ "\x08\x03\x15\x00\x08\x00\x03\x07\x01\x19\x00\x01\x00\xf0\xff\x08" // |................ |
// /* 07e0 */ "\x00\x10\x00\x00\x00\x00\x08\x18\x10\x10\x30\x20\x20\x60\x40\x40" // |..........0  `@@ |
// /* 07f0 */ "\xc0\x80\x00\x0d\x1b\x70\x15\x28\x5a\xe8\xc1\x02\xa2\xd0\x02\xff" // |.....p.(Z....... |
// /* 0800 */ "\xff\x00\xec\x08\x00\x85\x03\x93\x03\x85\x03\x93\x03\x84\x03\x04" // |................ |
// /* 0810 */ "\x19\x00\x03\x05\x19\x0a\x0f\x1d\xfd\x0c\x07\x11\x0f\x01\x01\xfd" // |................ |
// /* 0820 */ "\xfd\x03\x25\x00\x08\x00\x03\x07\x01\x1a\x00\x01\x00\xf0\xff\x10" // |..%............. |
// /* 0830 */ "\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\xe0\x00\xf8" // |................ |
// /* 0840 */ "\x00\xfe\x00\xff\x80\xff\xc0\xff\x00\xfc\x00\xf0\x00\xc0\x00\x00" // |................ |
// /* 0850 */ "\x00\x00\x00\x09\x1b\x40\x15\x28\xa9\x03\xb4\x03\xa9\x03\xb4\x03" // |.....@.(........ |
// /* 0860 */ "\xa8\x03\x02\x1a\x00\x19\x0a\x0f\x28\xff\x0a\x03\x11\x0f\x01\x01" // |........(....... |
// /* 0870 */ "\xfd\xfd\x03\x15\x00\x08\x00\x03\x07\x01\x1b\x00\x01\x00\xf0\xff" // |................ |
// /* 0880 */ "\x08\x00\x10\x00\x00\x00\x00\x00\x00\x00\x80\xe0\xf8\xfe\xf8\xe0" // |................ |
// /* 0890 */ "\x80\x00\x00\x00\x09\x1b\x40\x15\x28\xd2\x03\xe4\x03\xd2\x03\xe4" // |......@.(....... |
// /* 08a0 */ "\x03\xd1\x03\x04\x1b\x00\x1a\x07\x09\x0a\x0f\x2a\x03\xdf\x02\x3f" // |...........*...? |
// /* 08b0 */ "\x00\x17\x00\x11\x0f\x01\x01\xfd\xfd\x03\x15\x00\x08\x00\x03\x07" // |................ |
// /* 08c0 */ "\x01\x1c\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x40\x40" // |..............@@ |
// /* 08d0 */ "\x40\x40\x40\x40\x40\x40\x40\x7e\x00\x00\x00\x09\x1b\xc0\x3f\x38" // |@@@@@@@~......?8 |
// /* 08e0 */ "\x31\x03\xe2\x02\x61\x03\xf2\x02\x31\x03\xe2\x02\x61\x03\xf2\x02" // |1...a...1...a... |
// /* 08f0 */ "\x30\x03\xf2\x02\x0c\x1c\x00\x0f\x07\x0a\x09\x0f\x09\x10\x09\x0c" // |0............... |
// /* 0900 */ "\x09\x19\x0a\x3f\xff\xff\x01\xee\xff\xff\x11\x0e\x02\xc5\x13\x11" // |...?............ |
// /* 0910 */ "\x0b\x3d\xfe\x02\x11\x0f\xc3\x15\x3d\xeb\x11\x3f\x67\xeb\x0b\x15" // |.=......=..?g... |
// /* 0920 */ "\xec\x08\x11\x0f\x01\x01\xfd\xfd\x03\x25\x00\x08\x00\x03\x07\x01" // |.........%...... |
// /* 0930 */ "\x1d\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0940 */ "\x1f\x00\x30\x80\x60\x00\x40\x00\x40\x00\x40\x00\x40\x00\x60\x00" // |..0.`.@.@.@.@.`. |
// /* 0950 */ "\x30\x80\x1f\x00\x00\x00\x00\x00\x00\x00\x03\x25\x00\x08\x00\x03" // |0..........%.... |
// /* 0960 */ "\x07\x01\x1e\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00" // |................ |
// /* 0970 */ "\x00\x00\x00\x00\x00\x00\x5e\x00\x63\x00\x41\x00\x41\x00\x41\x00" // |......^.c.A.A.A. |
// /* 0980 */ "\x41\x00\x41\x00\x41\x00\x00\x00\x00\x00\x00\x00\x09\x1b\x40\x15" // |A.A.A.........@. |
// /* 0990 */ "\x28\x98\x03\xd3\x03\x98\x03\xd3\x03\x97\x03\x0e\x1d\x00\x0f\x0a" // |(............... |
// /* 09a0 */ "\x1e\x09\x1e\x09\x0b\x09\x13\x09\x0c\x08\x19\x0a\x3f\xff\xff\x01" // |............?... |
// /* 09b0 */ "\xee\xff\xff\x11\x0e\x02\xba\x13\x11\x0b\x48\xfe\x02\x11\x0f\xb8" // |..........H..... |
// /* 09c0 */ "\x15\x48\xeb\x01\x3f\x3c\x00\xc4\x02\x2e\x00\x1e\x00\xec\x08\x11" // |.H..?<.......... |
// /* 09d0 */ "\x0f\x01\x01\xfd\xfd\x03\x15\x00\x08\x00\x03\x07\x01\x1f\x00\x01" // |................ |
// /* 09e0 */ "\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x30\x40\x40\xf0\x40\x40\x40" // |.........0@@.@@@ |
// /* 09f0 */ "\x40\x40\x40\x40\x00\x00\x00\x03\x15\x00\x08\x00\x03\x07\x01\x20" // |@@@@...........  |
// /* 0a00 */ "\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0a10 */ "\x00\x00\x78\x00\x00\x00\x00\x00\x00\x03\x25\x00\x08\x00\x03\x07" // |..x.......%..... |
// /* 0a20 */ "\x01\x21\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00" // |.!.............. |
// /* 0a30 */ "\x00\x7e\x00\x43\x00\x41\x00\x41\x00\x43\x00\x7e\x00\x43\x00\x41" // |.~.C.A.A.C.~.C.A |
// /* 0a40 */ "\x00\x41\x00\x40\x80\x00\x00\x00\x00\x00\x00\x09\x1b\xc0\x3f\x38" // |.A.@..........?8 |
// /* 0a50 */ "\x44\x00\xcc\x02\x64\x00\xdc\x02\x44\x00\xcc\x02\x64\x00\xdc\x02" // |D...d...D...d... |
// /* 0a60 */ "\x43\x00\xdc\x02\x0a\x1f\x00\x09\x04\x20\x05\x15\x05\x21\x08\x19" // |C........ ...!.. |
// /* 0a70 */ "\x0a\x3f\xff\xff\x01\xe7\xff\xff\x11\x0e\x02\xd6\x1a\x11\x0b\x2c" // |.?............., |
// /* 0a80 */ "\xfe\x02\x11\x0f\xd4\x1c\x2c\xe4"                                 //......,. |
// Dump done on RDP Wab Target (4) 2696 bytes |
// LOOPING on PDUs: 2685 |
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
// order(10):opaquerect(rect(0,702,787,6) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,708,60,17) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,708,681,17) color=0x0008ec) |
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
// order(10):opaquerect(rect(0,725,60,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,725,918,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,735,60,3) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,735,704,3) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,738,810,20) color=0x0008ec) |
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
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,708,46,30) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(61,709,43,27) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(68,716,33,17) op=(68,716,33,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=67 glyph_y=732 data_len=10 [0x1f 0x00 0x09 0x04 0x20 0x05 0x15 0x05 0x21 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,708,44,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,710,2,28) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(104,708,2,30) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,736,46,2) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 3 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x04\xca"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 1222 bytes |
// Recv done on RDP Wab Target (4) 1222 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x84\xbb\xbb\x04\x17\x00\x09" // |...h. ..p....... |
// /* 0010 */ "\x04\x02\x00\x01\x00\x00\x02\xbb\x04\x02\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0020 */ "\x00\x73\x00\x00\x00\x01\x3f\x00\x00\x00\x00\x00\x04\x0a\x00\xec" // |.s....?......... |
// /* 0030 */ "\x08\x01\x0e\x0a\x00\x14\x00\x10\x00\x01\x05\x7f\x00\x3d\x03\x01" // |.............=.. |
// /* 0040 */ "\x05\xec\x03\x14\x00\x01\x0f\x00\x00\x1a\x00\xbc\x03\x07\x00\x01" // |................ |
// /* 0050 */ "\x05\xec\x03\x14\x00\x01\x0f\x00\x00\x21\x00\x00\x04\x08\x00\x01" // |.........!...... |
// /* 0060 */ "\x0e\x29\x00\x0f\x00\x10\x00\x01\x01\xf1\x03\x01\x0f\x00\x00\x39" // |.).............9 |
// /* 0070 */ "\x00\x00\x04\x05\x00\x01\x0e\x3e\x00\x0f\x00\x14\x00\x01\x05\xd6" // |.......>........ |
// /* 0080 */ "\x00\x05\x00\x01\x01\x98\x03\x11\x05\x59\x0a\x01\x0f\x00\x00\x52" // |.........Y.....R |
// /* 0090 */ "\x00\x00\x04\x6c\x02\x01\x0e\xbe\x02\x13\x03\x06\x00\x01\x0e\xc4" // |...l............ |
// /* 00a0 */ "\x02\x3c\x00\x11\x00\x01\x05\x6a\x00\xa9\x02\x01\x0f\x34\x03\xbe" // |.<.....j.....4.. |
// /* 00b0 */ "\x02\x0f\x00\x17\x00\x11\x0d\x29\x36\xea\x11\x05\x5f\xca\x11\x05" // |.......)6..._... |
// /* 00c0 */ "\x30\x05\x01\x0f\x5d\x03\xbf\x02\x0f\x00\x14\x00\x11\x0d\x28\x0e" // |0...].........(. |
// /* 00d0 */ "\xee\x11\x05\x37\xf2\x11\x05\x30\x05\x11\x0f\xa7\x02\xfb\x0e\x11" // |...7...0........ |
// /* 00e0 */ "\x01\x29\x11\x05\x30\x05\x11\x0f\x99\x10\x09\xf2\x11\x05\x37\xf2" // |.)..0.........7. |
// /* 00f0 */ "\x11\x05\x30\x05\x01\x07\x5d\x03\xd3\x02\x45\x00\x11\x05\x5f\xca" // |..0...]...E..._. |
// /* 0100 */ "\x11\x05\x30\x05\x01\x0f\x00\x00\xd5\x02\x3c\x00\x0a\x00\x01\x05" // |..0.......<..... |
// /* 0110 */ "\x6a\x00\x96\x03\x01\x0f\x00\x00\xdf\x02\x3c\x00\x03\x00\x01\x05" // |j.........<..... |
// /* 0120 */ "\x6a\x00\xc0\x02\x11\x0f\x96\x03\x6a\x11\x01\x0f\x69\x03\xdf\x02" // |j.......j...i... |
// /* 0130 */ "\x28\x00\x17\x00\x11\x05\x72\xfd\x01\x0f\x00\x00\xf6\x02\x00\x04" // |(.....r......... |
// /* 0140 */ "\x0a\x00\x01\x0f\x14\x00\x0a\x00\x6b\x00\x10\x00\x0d\x1b\xc0\x3f" // |........k......? |
// /* 0150 */ "\x38\x0f\x14\x00\x0a\x00\x7e\x00\x19\x00\x14\x00\x0a\x00\x7f\x00" // |8.....~......... |
// /* 0160 */ "\x1a\x00\x14\x00\x0a\x00\x7f\x00\x1a\x00\x13\x00\x1a\x00\x1a\x00" // |................ |
// /* 0170 */ "\x00\x01\x09\x02\x0e\x03\x09\x04\x09\x02\x04\x03\x09\x04\x09\x05" // |................ |
// /* 0180 */ "\x04\x06\x09\x04\x09\x06\x04\x06\x09\x19\x0a\x37\xfb\x1f\x61\x98" // |...........7..a. |
// /* 0190 */ "\x43\x0d\x1b\xa0\x3b\x30\xf0\xfb\x1f\x5c\x1f\x98\x43\x00\x29\x00" // |C...;0......C.). |
// /* 01a0 */ "\x70\x00\x39\x00\x29\x00\x70\x00\x39\x00\x39\x00\x18\x07\x00\x08" // |p.9.).p.9.9..... |
// /* 01b0 */ "\x09\x09\x08\x0a\x05\x0b\x09\x0c\x09\x0d\x05\x0e\x04\x09\x0b\x0f" // |................ |
// /* 01c0 */ "\x05\x10\x09\x11\x09\x09\x0a\x05\xdb\x00\xc2\x02\x0d\x1b\x40\x15" // |..............@. |
// /* 01d0 */ "\x28\x05\xdb\x00\x9c\x03\xe0\x00\x0d\x01\xe0\x00\x0d\x01\xdf\x00" // |(............... |
// /* 01e0 */ "\x0c\x07\x00\x08\x09\x09\x08\x0a\x05\x0b\x09\x0c\x09\x09\x0a\x05" // |................ |
// /* 01f0 */ "\x9d\x03\x54\x00\x0d\x1b\x40\x15\x28\x41\x9d\x03\x54\xa2\x03\xdb" // |..T...@.(A..T... |
// /* 0200 */ "\x03\xa2\x03\xdb\x03\xa1\x03\x10\x12\x00\x09\x09\x0f\x05\x0c\x09" // |................ |
// /* 0210 */ "\x0f\x05\x13\x09\x0f\x08\x14\x09\x09\x0a\x3f\x10\x00\x3f\x00\xc5" // |..........?..?.. |
// /* 0220 */ "\x00\x12\x00\xff\xff\x01\x3f\x12\x00\x40\x00\x01\x00\x0f\x00\x51" // |......?..@.....Q |
// /* 0230 */ "\x8c\x01\x3f\x0f\x00\x3e\x00\xc6\x00\x01\x00\x73\x02\x01\x0e\x3f" // |..?..>.....s...? |
// /* 0240 */ "\x00\x01\x00\x12\x00\x01\x0b\xd5\x00\x3e\x00\x14\x00\x01\x0f\x0f" // |.........>...... |
// /* 0250 */ "\x00\x51\x00\xc7\x00\x01\x00\x01\x3f\xdc\x00\x3f\x00\xbb\x02\x12" // |.Q......?..?.... |
// /* 0260 */ "\x00\xff\xff\x11\x0f\xff\xff\x01\xef\x01\x0e\x3f\x00\x01\x00\x12" // |...........?.... |
// /* 0270 */ "\x00\x01\x0b\x97\x03\x3e\x00\x14\x00\x01\x0f\xdb\x00\x51\x00\xbd" // |.....>.......Q.. |
// /* 0280 */ "\x02\x01\x00\x01\x0f\x9e\x03\x3f\x00\x52\x00\x12\x00\x11\x0f\xff" // |.......?.R...... |
// /* 0290 */ "\xff\x01\xef\x11\x0e\x01\xae\x11\x11\x0b\x53\xff\x02\x11\x0f\xad" // |..........S..... |
// /* 02a0 */ "\x13\x53\xed\x11\x3f\x1f\xb9\xdc\x16\xec\x08\x11\x0f\x01\x01\xfd" // |.S..?........... |
// /* 02b0 */ "\xfd\x09\x1b\xe0\x3f\x38\xec\x08\x00\xc3\x03\x0d\x00\xe4\x03\x1d" // |....?8.......... |
// /* 02c0 */ "\x00\xc3\x03\x0d\x00\xe4\x03\x1d\x00\xc2\x03\x1d\x00\x0c\x15\x00" // |................ |
// /* 02d0 */ "\x16\x08\x14\x03\x0c\x03\x0b\x05\x09\x09\x19\x0a\x3f\xff\xff\x01" // |............?... |
// /* 02e0 */ "\xee\xff\xff\x11\x0e\x02\xd4\x13\x11\x0b\x2e\xfe\x02\x11\x0f\xd2" // |................ |
// /* 02f0 */ "\x15\x2e\xeb\x01\x3f\x13\x03\xbe\x02\x21\x00\x17\x00\xec\x08\x11" // |....?....!...... |
// /* 0300 */ "\x0f\x01\x01\xfd\xfd\x09\x1b\xc0\x3f\x38\x1a\x03\xc1\x02\x2c\x03" // |........?8....,. |
// /* 0310 */ "\xd1\x02\x1a\x03\xc1\x02\x2c\x03\xd1\x02\x19\x03\xd1\x02\x04\x17" // |......,......... |
// /* 0320 */ "\x00\x18\x0b\x19\x0a\x0f\x2f\xff\xfc\x03\x11\x0f\x01\x01\xfd\xfd" // |....../......... |
// /* 0330 */ "\x09\x1b\x40\x15\x28\x4a\x03\x55\x03\x4a\x03\x55\x03\x49\x03\x02" // |..@.(J.U.J.U.I.. |
// /* 0340 */ "\x17\x00\x19\x0a\x3b\x29\x01\xfe\xff\xff\x09\x1b\x70\x15\x28\x00" // |....;)......p.(. |
// /* 0350 */ "\x00\x00\xff\xff\x00\x6e\x03\x77\x03\x6e\x03\x77\x03\x6c\x03\x02" // |.....n.w.n.w.l.. |
// /* 0360 */ "\x02\x00\x19\x0a\x0f\xff\xff\x01\xef\x11\x0e\x01\xe9\x11\x11\x0b" // |................ |
// /* 0370 */ "\x18\xff\x02\x11\x0f\xe8\x13\x18\xed\x11\x3f\x19\xef\xf5\x0f\xec" // |..........?..... |
// /* 0380 */ "\x08\x0d\x1b\x70\x15\x28\x5a\xe8\xc1\x02\xa2\xd0\x02\xff\xff\x00" // |...p.(Z......... |
// /* 0390 */ "\xec\x08\x00\x85\x03\x93\x03\x85\x03\x93\x03\x84\x03\x04\x19\x00" // |................ |
// /* 03a0 */ "\x03\x05\x19\x0a\x0f\x1d\xfd\x0c\x07\x11\x0f\x01\x01\xfd\xfd\x09" // |................ |
// /* 03b0 */ "\x1b\x40\x15\x28\xa9\x03\xb4\x03\xa9\x03\xb4\x03\xa8\x03\x02\x1a" // |.@.(............ |
// /* 03c0 */ "\x00\x19\x0a\x0f\x28\xff\x0a\x03\x11\x0f\x01\x01\xfd\xfd\x09\x1b" // |....(........... |
// /* 03d0 */ "\x40\x15\x28\xd2\x03\xe4\x03\xd2\x03\xe4\x03\xd1\x03\x04\x1b\x00" // |@.(............. |
// /* 03e0 */ "\x1a\x07\x09\x0a\x0f\x2a\x03\xdf\x02\x3f\x00\x17\x00\x11\x0f\x01" // |.....*...?...... |
// /* 03f0 */ "\x01\xfd\xfd\x09\x1b\xc0\x3f\x38\x31\x03\xe2\x02\x61\x03\xf2\x02" // |......?81...a... |
// /* 0400 */ "\x31\x03\xe2\x02\x61\x03\xf2\x02\x30\x03\xf2\x02\x0c\x1c\x00\x0f" // |1...a...0....... |
// /* 0410 */ "\x07\x0a\x09\x0f\x09\x10\x09\x0c\x09\x19\x0a\x3f\xff\xff\x01\xee" // |...........?.... |
// /* 0420 */ "\xff\xff\x11\x0e\x02\xc5\x13\x11\x0b\x3d\xfe\x02\x11\x0f\xc3\x15" // |.........=...... |
// /* 0430 */ "\x3d\xeb\x11\x3f\x67\xeb\x0b\x15\xec\x08\x11\x0f\x01\x01\xfd\xfd" // |=..?g........... |
// /* 0440 */ "\x09\x1b\x40\x15\x28\x98\x03\xd3\x03\x98\x03\xd3\x03\x97\x03\x0e" // |..@.(........... |
// /* 0450 */ "\x1d\x00\x0f\x0a\x1e\x09\x1e\x09\x0b\x09\x13\x09\x0c\x08\x19\x0a" // |................ |
// /* 0460 */ "\x3f\xff\xff\x01\xee\xff\xff\x11\x0e\x02\xba\x13\x11\x0b\x48\xfe" // |?.............H. |
// /* 0470 */ "\x02\x11\x0f\xb8\x15\x48\xeb\x01\x3f\x3c\x00\xc4\x02\x2e\x00\x1e" // |.....H..?<...... |
// /* 0480 */ "\x00\xec\x08\x11\x0f\x01\x01\xfd\xfd\x09\x1b\xc0\x3f\x38\x44\x00" // |............?8D. |
// /* 0490 */ "\xcc\x02\x64\x00\xdc\x02\x44\x00\xcc\x02\x64\x00\xdc\x02\x43\x00" // |..d...D...d...C. |
// /* 04a0 */ "\xdc\x02\x0a\x1f\x00\x09\x04\x20\x05\x15\x05\x21\x08\x19\x0a\x3f" // |....... ...!...? |
// /* 04b0 */ "\xff\xff\x01\xe7\xff\xff\x11\x0e\x02\xd6\x1a\x11\x0b\x2c\xfe\x02" // |.............,.. |
// /* 04c0 */ "\x11\x0f\xd4\x1c\x2c\xe4"                                         //....,. |
// Dump done on RDP Wab Target (4) 1222 bytes |
// LOOPING on PDUs: 1211 |
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
// order(10):opaquerect(rect(0,702,787,6) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,708,60,17) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,708,681,17) color=0x0008ec) |
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
// order(10):opaquerect(rect(0,725,60,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,725,918,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,735,60,3) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,735,704,3) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,738,810,20) color=0x0008ec) |
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
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,708,46,30) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(61,709,43,27) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(68,716,33,17) op=(68,716,33,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=67 glyph_y=732 data_len=10 [0x1f 0x00 0x09 0x04 0x20 0x05 0x15 0x05 0x21 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,708,44,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,710,2,28) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(104,708,2,30) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,736,46,2) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 4 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x52"                                                     //..R |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 78 bytes |
// Recv done on RDP Wab Target (4) 78 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x44\x44\x00\x17\x00\x09\x04" // |...h. ..pDD..... |
// /* 0010 */ "\x02\x00\x01\x00\x00\x02\x44\x00\x02\x00\x00\x00\x00\x00\x00\x00" // |......D......... |
// /* 0020 */ "\x05\x00\x00\x00\x01\x0f\x10\x00\x3f\x00\xc5\x00\x12\x00\x11\x0f" // |........?....... |
// /* 0030 */ "\xff\xff\x01\xef\x01\x0e\x3f\x00\x01\x00\x12\x00\x01\x0b\xd5\x00" // |......?......... |
// /* 0040 */ "\x3e\x00\x14\x00\x01\x0f\x0f\x00\x51\x00\xc7\x00\x01\x00"         //>.......Q..... |
// Dump done on RDP Wab Target (4) 78 bytes |
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
// /* 0000 */ "\x00\x0d\x0f"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 3339 bytes |
// Recv done on RDP Wab Target (4) 3339 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x8d\x00\x00\x0d\x17\x00\x09" // |...h. ..p....... |
// /* 0010 */ "\x04\x02\x00\x01\x00\x00\x02\x00\x0d\x02\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0020 */ "\x00\xae\x00\x00\x00\x01\x3f\x00\x00\x00\x00\x00\x04\x0a\x00\xec" // |......?......... |
// /* 0030 */ "\x08\x01\x0e\x0a\x00\x14\x00\x10\x00\x01\x05\x7f\x00\x3d\x03\x01" // |.............=.. |
// /* 0040 */ "\x05\xec\x03\x14\x00\x01\x0f\x00\x00\x1a\x00\xbc\x03\x07\x00\x01" // |................ |
// /* 0050 */ "\x05\xec\x03\x14\x00\x01\x0f\x00\x00\x21\x00\x00\x04\x08\x00\x01" // |.........!...... |
// /* 0060 */ "\x0e\x29\x00\x0f\x00\x10\x00\x01\x01\xf1\x03\x01\x0f\x00\x00\x39" // |.).............9 |
// /* 0070 */ "\x00\x00\x04\x05\x00\x01\x0e\x3e\x00\x0f\x00\x14\x00\x01\x05\xd6" // |.......>........ |
// /* 0080 */ "\x00\x05\x00\x01\x01\x98\x03\x11\x05\x59\x0a\x01\x0f\x00\x00\x52" // |.........Y.....R |
// /* 0090 */ "\x00\x00\x04\x05\x00\x01\x0e\x57\x00\x0f\x00\x84\x00\x01\x01\xf1" // |.......W........ |
// /* 00a0 */ "\x03\x01\x0f\x00\x00\xdb\x00\x00\x04\xe3\x01\x01\x0e\xbe\x02\x13" // |................ |
// /* 00b0 */ "\x03\x06\x00\x01\x0e\xc4\x02\x3c\x00\x11\x00\x01\x05\x6a\x00\xa9" // |.......<.....j.. |
// /* 00c0 */ "\x02\x01\x0f\x34\x03\xbe\x02\x0f\x00\x17\x00\x11\x0d\x29\x36\xea" // |...4.........)6. |
// /* 00d0 */ "\x11\x05\x5f\xca\x11\x05\x30\x05\x01\x0f\x5d\x03\xbf\x02\x0f\x00" // |.._...0...]..... |
// /* 00e0 */ "\x14\x00\x11\x0d\x28\x0e\xee\x11\x05\x37\xf2\x11\x05\x30\x05\x11" // |....(....7...0.. |
// /* 00f0 */ "\x0f\xa7\x02\xfb\x0e\x11\x01\x29\x11\x05\x30\x05\x11\x0f\x99\x10" // |.......)..0..... |
// /* 0100 */ "\x09\xf2\x11\x05\x37\xf2\x11\x05\x30\x05\x01\x07\x5d\x03\xd3\x02" // |....7...0...]... |
// /* 0110 */ "\x45\x00\x11\x05\x5f\xca\x11\x05\x30\x05\x01\x0f\x00\x00\xd5\x02" // |E..._...0....... |
// /* 0120 */ "\x3c\x00\x0a\x00\x01\x05\x6a\x00\x96\x03\x01\x0f\x00\x00\xdf\x02" // |<.....j......... |
// /* 0130 */ "\x3c\x00\x03\x00\x01\x05\x6a\x00\xc0\x02\x11\x0f\x96\x03\x6a\x11" // |<.....j.......j. |
// /* 0140 */ "\x01\x0f\x69\x03\xdf\x02\x28\x00\x17\x00\x11\x05\x72\xfd\x01\x0f" // |..i...(.....r... |
// /* 0150 */ "\x00\x00\xf6\x02\x00\x04\x0a\x00\x01\x0f\x14\x00\x0a\x00\x6b\x00" // |..............k. |
// /* 0160 */ "\x10\x00\x0d\x1b\xc0\x3f\x38\x0f\x14\x00\x0a\x00\x7e\x00\x19\x00" // |.....?8.....~... |
// /* 0170 */ "\x14\x00\x0a\x00\x7f\x00\x1a\x00\x14\x00\x0a\x00\x7f\x00\x1a\x00" // |................ |
// /* 0180 */ "\x13\x00\x1a\x00\x1a\x00\x00\x01\x09\x02\x0e\x03\x09\x04\x09\x02" // |................ |
// /* 0190 */ "\x04\x03\x09\x04\x09\x05\x04\x06\x09\x04\x09\x06\x04\x06\x09\x19" // |................ |
// /* 01a0 */ "\x0a\x37\xfb\x1f\x61\x98\x43\x0d\x1b\xa0\x3b\x30\xf0\xfb\x1f\x5c" // |.7..a.C...;0.... |
// /* 01b0 */ "\x1f\x98\x43\x00\x29\x00\x70\x00\x39\x00\x29\x00\x70\x00\x39\x00" // |..C.).p.9.).p.9. |
// /* 01c0 */ "\x39\x00\x18\x07\x00\x08\x09\x09\x08\x0a\x05\x0b\x09\x0c\x09\x0d" // |9............... |
// /* 01d0 */ "\x05\x0e\x04\x09\x0b\x0f\x05\x10\x09\x11\x09\x09\x0a\x05\xdb\x00" // |................ |
// /* 01e0 */ "\xc2\x02\x0d\x1b\x40\x15\x28\x05\xdb\x00\x9c\x03\xe0\x00\x0d\x01" // |....@.(......... |
// /* 01f0 */ "\xe0\x00\x0d\x01\xdf\x00\x0c\x07\x00\x08\x09\x09\x08\x0a\x05\x0b" // |................ |
// /* 0200 */ "\x09\x0c\x09\x09\x0a\x05\x9d\x03\x54\x00\x0d\x1b\x40\x15\x28\x41" // |........T...@.(A |
// /* 0210 */ "\x9d\x03\x54\xa2\x03\xdb\x03\xa2\x03\xdb\x03\xa1\x03\x10\x12\x00" // |..T............. |
// /* 0220 */ "\x09\x09\x0f\x05\x0c\x09\x0f\x05\x13\x09\x0f\x08\x14\x09\x09\x0a" // |................ |
// /* 0230 */ "\x3f\x10\x00\x3f\x00\xc5\x00\x12\x00\xff\xff\x11\x0f\xff\xff\x01" // |?..?............ |
// /* 0240 */ "\xef\x01\x0e\x3f\x00\x01\x00\x12\x00\x01\x0b\xd5\x00\x3e\x00\x14" // |...?.........>.. |
// /* 0250 */ "\x00\x01\x0f\x0f\x00\x51\x00\xc7\x00\x01\x00\x01\x0f\xdc\x00\x3f" // |.....Q.........? |
// /* 0260 */ "\x00\xbb\x02\x12\x00\x11\x0f\xff\xff\x01\xef\x01\x0e\x3f\x00\x01" // |.............?.. |
// /* 0270 */ "\x00\x12\x00\x01\x0b\x97\x03\x3e\x00\x14\x00\x01\x0f\xdb\x00\x51" // |.......>.......Q |
// /* 0280 */ "\x00\xbd\x02\x01\x00\x01\x0f\x9e\x03\x3f\x00\x52\x00\x12\x00\x11" // |.........?.R.... |
// /* 0290 */ "\x0f\xff\xff\x01\xef\x11\x0e\x01\xae\x11\x11\x0b\x53\xff\x02\x11" // |............S... |
// /* 02a0 */ "\x0f\xad\x13\x53\xed\x11\x3f\x1f\xb9\xdc\x16\xec\x08\x11\x0f\x01" // |...S..?......... |
// /* 02b0 */ "\x01\xfd\xfd\x09\x1b\xe0\x3f\x38\xec\x08\x00\xc3\x03\x0d\x00\xe4" // |......?8........ |
// /* 02c0 */ "\x03\x1d\x00\xc3\x03\x0d\x00\xe4\x03\x1d\x00\xc2\x03\x1d\x00\x0c" // |................ |
// /* 02d0 */ "\x15\x00\x16\x08\x14\x03\x0c\x03\x0b\x05\x09\x09\x19\x0a\x3f\xff" // |..............?. |
// /* 02e0 */ "\xff\x01\xee\xff\xff\x11\x0e\x02\xd4\x13\x11\x0b\x2e\xfe\x02\x11" // |................ |
// /* 02f0 */ "\x0f\xd2\x15\x2e\xeb\x01\x3f\x0f\x00\x57\x00\xe2\x03\x16\x00\x73" // |......?..W.....s |
// /* 0300 */ "\x02\x01\x0f\x11\x00\x59\x00\xc8\x00\x12\x00\x03\x15\x00\x08\x00" // |.....Y.......... |
// /* 0310 */ "\x03\x07\x01\x22\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00" // |..."............ |
// /* 0320 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xfe\x09\x1b\xe0" // |................ |
// /* 0330 */ "\x3f\x38\x73\x02\x00\x14\x00\x5a\x00\x5e\x00\x6a\x00\x14\x00\x5a" // |?8s....Z.^.j...Z |
// /* 0340 */ "\x00\x5e\x00\x6a\x00\x13\x00\x6a\x00\x12\x0a\x00\x09\x09\x0f\x05" // |.^.j...j........ |
// /* 0350 */ "\x10\x09\x11\x09\x22\x09\x08\x07\x11\x08\x11\x09\x09\x0a\x05\xdd" // |...."........... |
// /* 0360 */ "\x00\xbe\x02\x03\x25\x00\x08\x00\x03\x07\x01\x23\x00\x01\x00\xf0" // |....%......#.... |
// /* 0370 */ "\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x44" // |...............D |
// /* 0380 */ "\x40\x44\x40\x44\x40\x2a\x80\x2a\x80\x2a\x80\x11\x00\x11\x00\x00" // |@D@D@*.*.*...... |
// /* 0390 */ "\x00\x00\x00\x00\x00\x03\x25\x00\x08\x00\x03\x07\x01\x24\x00\x01" // |......%......$.. |
// /* 03a0 */ "\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x01\x00\x01\x00\x01" // |................ |
// /* 03b0 */ "\x00\x1d\x00\x23\x00\x41\x00\x41\x00\x41\x00\x41\x00\x23\x00\x1d" // |...#.A.A.A.A.#.. |
// /* 03c0 */ "\x00\x00\x00\x00\x00\x00\x00\x03\x15\x00\x08\x00\x03\x07\x01\x25" // |...............% |
// /* 03d0 */ "\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x00\x00\x3c\x42" // |..............<B |
// /* 03e0 */ "\x40\x70\x0e\x02\x42\x3c\x00\x00\x00\x03\x15\x00\x08\x00\x03\x07" // |@p..B<.......... |
// /* 03f0 */ "\x01\x26\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x40\x40\x40" // |.&...........@@@ |
// /* 0400 */ "\x42\x44\x48\x70\x50\x48\x44\x42\x00\x00\x00\x03\x15\x00\x08\x00" // |BDHpPHDB........ |
// /* 0410 */ "\x03\x07\x01\x27\x00\x02\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00" // |...'............ |
// /* 0420 */ "\x00\x00\x00\x20\x20\x00\x00\x00\x20\x20\x00\x00\x00\x03\x25\x00" // |...  ...  ....%. |
// /* 0430 */ "\x08\x00\x03\x07\x01\x28\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00" // |.....(.......... |
// /* 0440 */ "\x00\x00\x00\x00\x00\x08\x00\x14\x00\x14\x00\x22\x00\x22\x00\x22" // |..........."."." |
// /* 0450 */ "\x00\x7f\x00\x41\x00\x41\x00\x80\x80\x00\x00\x00\x00\x00\x00\x09" // |...A.A.......... |
// /* 0460 */ "\x1b\x40\x15\x28\xe0\x00\xb8\x02\xe0\x00\xb8\x02\xdf\x00\x7a\x08" // |.@.(..........z. |
// /* 0470 */ "\x00\x11\x08\x11\x09\x22\x09\x23\x07\x16\x0b\x1e\x03\x24\x09\x0f" // |.....".#.....$.. |
// /* 0480 */ "\x09\x23\x09\x25\x0b\x20\x08\x0b\x05\x00\x09\x11\x09\x14\x09\x0f" // |.#.%. .......... |
// /* 0490 */ "\x03\x09\x09\x0b\x05\x09\x09\x22\x05\x1f\x07\x08\x04\x26\x08\x0b" // |.......".....&.. |
// /* 04a0 */ "\x08\x14\x09\x0f\x03\x0a\x09\x16\x09\x1e\x03\x01\x09\x14\x0e\x0f" // |................ |
// /* 04b0 */ "\x03\x13\x09\x08\x08\x14\x08\x01\x03\x08\x0e\x11\x08\x11\x09\x22" // |..............." |
// /* 04c0 */ "\x09\x23\x07\x16\x0b\x1e\x03\x24\x09\x0f\x09\x23\x09\x25\x0b\x20" // |.#.....$...#.%.  |
// /* 04d0 */ "\x08\x0b\x05\x00\x09\x11\x09\x14\x09\x0f\x03\x09\x09\x0b\x05\x09" // |................ |
// /* 04e0 */ "\x09\x27\x05\x28\x05\x12\x09\x12\x09\x09\x0a\x05\x9f\x03\x50\x00" // |.'.(..........P. |
// /* 04f0 */ "\x09\x1b\x40\x15\x28\xa2\x03\xbd\x03\xa2\x03\xbd\x03\xa1\x03\x06" // |..@.(........... |
// /* 0500 */ "\x28\x00\x12\x09\x12\x09\x09\x0a\x3f\x0f\x00\x6d\x00\xe2\x03\x16" // |(.......?..m.... |
// /* 0510 */ "\x00\x7e\xef\x01\x0f\x11\x00\x6f\x00\xc8\x00\x12\x00\x03\x25\x00" // |.~.....o......%. |
// /* 0520 */ "\x08\x00\x03\x07\x01\x29\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00" // |.....).......... |
// /* 0530 */ "\x00\x00\x00\x00\x00\x3c\x00\x46\x00\x02\x00\x02\x00\x06\x00\x04" // |.....<.F........ |
// /* 0540 */ "\x00\x08\x00\x10\x00\x20\x00\x7e\x00\x00\x00\x00\x00\x00\x00\x09" // |..... .~........ |
// /* 0550 */ "\x1b\xf0\x3f\x38\x00\x00\x00\x7e\xef\x00\x14\x00\x70\x00\x72\x00" // |..?8...~....p.r. |
// /* 0560 */ "\x80\x00\x14\x00\x70\x00\x72\x00\x80\x00\x13\x00\x80\x00\x16\x0a" // |....p.r......... |
// /* 0570 */ "\x00\x09\x09\x0f\x05\x10\x09\x11\x09\x22\x09\x23\x07\x29\x0b\x26" // |.........".#.).& |
// /* 0580 */ "\x09\x02\x08\x29\x09\x09\x0a\x05\xdd\x00\xbe\x02\x03\x25\x00\x08" // |...).........%.. |
// /* 0590 */ "\x00\x03\x07\x01\x2a\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00" // |....*........... |
// /* 05a0 */ "\x00\x00\x40\x00\x40\x00\x40\x00\x5c\x00\x62\x00\x41\x00\x41\x00" // |..@.@.@...b.A.A. |
// /* 05b0 */ "\x41\x00\x41\x00\x62\x00\x5c\x00\x00\x00\x00\x00\x00\x00\x03\x25" // |A.A.b..........% |
// /* 05c0 */ "\x00\x08\x00\x03\x07\x01\x2b\x00\x01\x00\xf0\xff\x10\x00\x10\x00" // |......+......... |
// /* 05d0 */ "\x00\x00\x00\x00\x00\x00\x7f\x00\x02\x00\x02\x00\x04\x00\x04\x00" // |................ |
// /* 05e0 */ "\x08\x00\x08\x00\x10\x00\x10\x00\x20\x00\x00\x00\x00\x00\x00\x00" // |........ ....... |
// /* 05f0 */ "\x03\x25\x00\x08\x00\x03\x07\x01\x2c\x00\x01\x00\xf0\xff\x10\x00" // |.%......,....... |
// /* 0600 */ "\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x5c\xe0\x63\x10" // |..............c. |
// /* 0610 */ "\x42\x10\x42\x10\x42\x10\x42\x10\x42\x10\x42\x10\x00\x00\x00\x00" // |B.B.B.B.B.B..... |
// /* 0620 */ "\x00\x00\x03\x25\x00\x08\x00\x03\x07\x01\x2d\x00\x01\x00\xf0\xff" // |...%......-..... |
// /* 0630 */ "\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x7e\x00\x41\x80\x40\xc0" // |..........~.A.@. |
// /* 0640 */ "\x40\x40\x40\x40\x40\x40\x40\x40\x40\xc0\x41\x80\x7e\x00\x00\x00" // |@@@@@@@@@.A.~... |
// /* 0650 */ "\x00\x00\x00\x00\x09\x1b\x40\x15\x28\xe0\x00\xb1\x02\xe0\x00\xb1" // |......@.(....... |
// /* 0660 */ "\x02\xdf\x00\x76\x23\x00\x29\x0b\x26\x09\x02\x08\x29\x09\x20\x09" // |...v#.).&...). . |
// /* 0670 */ "\x11\x05\x10\x09\x2a\x09\x20\x09\x05\x05\x05\x09\x04\x09\x29\x04" // |....*. .......). |
// /* 0680 */ "\x2b\x09\x22\x09\x09\x07\x0b\x05\x24\x09\x20\x09\x08\x05\x24\x08" // |+.".....$. ...$. |
// /* 0690 */ "\x2c\x09\x16\x0d\x1e\x03\x16\x09\x25\x03\x0c\x08\x09\x05\x08\x05" // |,.......%....... |
// /* 06a0 */ "\x0c\x08\x0f\x05\x09\x09\x01\x05\x14\x0e\x0f\x03\x13\x09\x08\x08" // |................ |
// /* 06b0 */ "\x14\x08\x01\x03\x23\x0e\x29\x0b\x26\x09\x02\x08\x29\x09\x20\x09" // |....#.).&...). . |
// /* 06c0 */ "\x11\x05\x10\x09\x2a\x09\x20\x09\x05\x05\x05\x09\x04\x09\x29\x04" // |....*. .......). |
// /* 06d0 */ "\x2b\x09\x27\x09\x21\x05\x2d\x0a\x12\x0b\x09\x0a\x05\x9f\x03\x50" // |+.'.!.-........P |
// /* 06e0 */ "\x00\x09\x1b\x40\x15\x28\xa2\x03\xc0\x03\xa2\x03\xc0\x03\xa1\x03" // |...@.(.......... |
// /* 06f0 */ "\x06\x21\x00\x2d\x0a\x12\x0b\x09\x0a\x3f\x0f\x00\x83\x00\xe2\x03" // |.!.-.....?...... |
// /* 0700 */ "\x16\x00\xbd\xce\x01\x0f\x11\x00\x85\x00\xc8\x00\x12\x00\x09\x1b" // |................ |
// /* 0710 */ "\xe0\x3f\x38\xbd\xce\x00\x14\x00\x86\x00\x69\x00\x96\x00\x14\x00" // |.?8.......i..... |
// /* 0720 */ "\x86\x00\x69\x00\x96\x00\x13\x00\x96\x00\x14\x0a\x00\x09\x09\x0f" // |..i............. |
// /* 0730 */ "\x05\x10\x09\x11\x09\x22\x09\x23\x07\x29\x0b\x26\x09\x06\x08\x09" // |.....".#.).&.... |
// /* 0740 */ "\x0a\x05\xdd\x00\xbe\x02\x03\x25\x00\x08\x00\x03\x07\x01\x2e\x00" // |.......%........ |
// /* 0750 */ "\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x1e\x00" // |................ |
// /* 0760 */ "\x31\x00\x60\x00\x40\x00\x5e\x00\x63\x00\x41\x00\x41\x00\x23\x00" // |1.`.@.^.c.A.A.#. |
// /* 0770 */ "\x1e\x00\x00\x00\x00\x00\x00\x00\x03\x25\x00\x08\x00\x03\x07\x01" // |.........%...... |
// /* 0780 */ "\x2f\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00" // |/............... |
// /* 0790 */ "\x00\x00\x00\x00\x1d\x00\x23\x00\x41\x00\x41\x00\x41\x00\x41\x00" // |......#.A.A.A.A. |
// /* 07a0 */ "\x23\x00\x1d\x00\x01\x00\x01\x00\x01\x00\x09\x1b\x40\x15\x28\xe0" // |#...........@.(. |
// /* 07b0 */ "\x00\xa2\x02\xe0\x00\xa2\x02\xdf\x00\x72\x23\x00\x29\x0b\x26\x09" // |.........r#.).&. |
// /* 07c0 */ "\x06\x08\x20\x09\x11\x05\x10\x09\x2a\x09\x20\x09\x05\x05\x2e\x09" // |.. .....*. ..... |
// /* 07d0 */ "\x04\x09\x2b\x04\x06\x09\x22\x09\x2f\x07\x08\x09\x20\x08\x08\x05" // |..+..."./... ... |
// /* 07e0 */ "\x24\x08\x2c\x09\x16\x0d\x1e\x03\x16\x09\x25\x03\x0c\x08\x09\x05" // |$.,.......%..... |
// /* 07f0 */ "\x08\x05\x0c\x08\x0b\x05\x10\x09\x09\x09\x01\x05\x14\x0e\x0f\x03" // |................ |
// /* 0800 */ "\x13\x09\x08\x08\x14\x08\x01\x03\x23\x0e\x29\x0b\x26\x09\x06\x08" // |........#.).&... |
// /* 0810 */ "\x20\x09\x11\x05\x10\x09\x2a\x09\x20\x09\x05\x05\x2e\x09\x04\x09" // | .....*. ....... |
// /* 0820 */ "\x2b\x04\x06\x09\x27\x09\x21\x05\x2d\x0a\x12\x0b\x09\x0a\x05\x9f" // |+...'.!.-....... |
// /* 0830 */ "\x03\x50\x00\x09\x1b\x40\x15\x28\xa2\x03\xc0\x03\xa2\x03\xc0\x03" // |.P...@.(........ |
// /* 0840 */ "\xa1\x03\x06\x21\x00\x2d\x0a\x12\x0b\x09\x0a\x3f\x0f\x00\x99\x00" // |...!.-.....?.... |
// /* 0850 */ "\xe2\x03\x16\x00\x7e\xef\x01\x0f\x11\x00\x9b\x00\xc8\x00\x12\x00" // |....~........... |
// /* 0860 */ "\x03\x25\x00\x08\x00\x03\x07\x01\x30\x00\x01\x00\xf0\xff\x10\x00" // |.%......0....... |
// /* 0870 */ "\x10\x00\x00\x00\x00\x00\x00\x00\x3e\x00\x63\x00\x41\x00\x63\x00" // |........>.c.A.c. |
// /* 0880 */ "\x1c\x00\x63\x00\x41\x00\x41\x00\x63\x00\x3e\x00\x00\x00\x00\x00" // |..c.A.A.c.>..... |
// /* 0890 */ "\x00\x00\x09\x1b\xe0\x3f\x38\x7e\xef\x00\x14\x00\x9c\x00\x69\x00" // |.....?8~......i. |
// /* 08a0 */ "\xac\x00\x14\x00\x9c\x00\x69\x00\xac\x00\x13\x00\xac\x00\x14\x0a" // |......i......... |
// /* 08b0 */ "\x00\x09\x09\x0f\x05\x10\x09\x11\x09\x22\x09\x23\x07\x29\x0b\x26" // |.........".#.).& |
// /* 08c0 */ "\x09\x30\x08\x09\x0a\x05\xdd\x00\xbe\x02\x09\x1b\x40\x15\x28\xe0" // |.0..........@.(. |
// /* 08d0 */ "\x00\x78\x02\xe0\x00\x78\x02\xdf\x00\x6c\x23\x00\x29\x0b\x26\x09" // |.x...x...l#.).&. |
// /* 08e0 */ "\x30\x08\x20\x09\x11\x05\x09\x09\x16\x05\x20\x03\x05\x05\x2b\x09" // |0. ....... ...+. |
// /* 08f0 */ "\x04\x09\x02\x04\x2e\x09\x22\x09\x08\x07\x24\x08\x2c\x09\x16\x0d" // |......"...$.,... |
// /* 0900 */ "\x1e\x03\x16\x09\x25\x03\x0c\x08\x09\x05\x08\x05\x0c\x08\x0b\x05" // |....%........... |
// /* 0910 */ "\x10\x09\x09\x09\x01\x05\x14\x0e\x0f\x03\x13\x09\x08\x08\x14\x08" // |................ |
// /* 0920 */ "\x01\x03\x23\x0e\x29\x0b\x26\x09\x30\x08\x20\x09\x11\x05\x09\x09" // |..#.).&.0. ..... |
// /* 0930 */ "\x16\x05\x20\x03\x05\x05\x2b\x09\x04\x09\x02\x04\x2e\x09\x27\x09" // |.. ...+.......'. |
// /* 0940 */ "\x21\x05\x2d\x0a\x12\x0b\x09\x0a\x05\x9f\x03\x50\x00\x09\x1b\x40" // |!.-........P...@ |
// /* 0950 */ "\x15\x28\xa2\x03\xc0\x03\xa2\x03\xc0\x03\xa1\x03\x06\x21\x00\x2d" // |.(...........!.- |
// /* 0960 */ "\x0a\x12\x0b\x09\x0a\x3f\x0f\x00\xaf\x00\xe2\x03\x16\x00\xbd\xce" // |.....?.......... |
// /* 0970 */ "\x01\x0f\x11\x00\xb1\x00\xc8\x00\x12\x00\x09\x1b\xe0\x3f\x38\xbd" // |.............?8. |
// /* 0980 */ "\xce\x00\x14\x00\xb2\x00\x69\x00\xc2\x00\x14\x00\xb2\x00\x69\x00" // |......i.......i. |
// /* 0990 */ "\xc2\x00\x13\x00\xc2\x00\x14\x0a\x00\x09\x09\x0f\x05\x10\x09\x11" // |................ |
// /* 09a0 */ "\x09\x22\x09\x23\x07\x29\x0b\x26\x09\x30\x08\x09\x0a\x05\xdd\x00" // |.".#.).&.0...... |
// /* 09b0 */ "\xbe\x02\x09\x1b\x40\x15\x28\xe0\x00\x65\x02\xe0\x00\x65\x02\xdf" // |....@.(..e...e.. |
// /* 09c0 */ "\x00\x64\x23\x00\x29\x0b\x26\x09\x30\x08\x20\x09\x11\x05\x09\x09" // |.d#.).&.0. ..... |
// /* 09d0 */ "\x16\x05\x20\x03\x05\x05\x2b\x09\x04\x09\x02\x04\x2e\x09\x22\x09" // |.. ...+.......". |
// /* 09e0 */ "\x09\x07\x0b\x05\x24\x09\x0b\x09\x2c\x09\x11\x0d\x0c\x09\x16\x05" // |....$...,....... |
// /* 09f0 */ "\x0f\x03\x1e\x09\x01\x09\x14\x0e\x0f\x03\x13\x09\x08\x08\x14\x08" // |................ |
// /* 0a00 */ "\x01\x03\x23\x0e\x29\x0b\x26\x09\x30\x08\x20\x09\x11\x05\x09\x09" // |..#.).&.0. ..... |
// /* 0a10 */ "\x16\x05\x20\x03\x05\x05\x2b\x09\x04\x09\x02\x04\x2e\x09\x27\x09" // |.. ...+.......'. |
// /* 0a20 */ "\x21\x05\x2d\x0a\x12\x0b\x09\x0a\x05\x9f\x03\x50\x00\x09\x1b\x40" // |!.-........P...@ |
// /* 0a30 */ "\x15\x28\xa2\x03\xc0\x03\xa2\x03\xc0\x03\xa1\x03\x06\x21\x00\x2d" // |.(...........!.- |
// /* 0a40 */ "\x0a\x12\x0b\x09\x0a\x3f\x0f\x00\xc5\x00\xe2\x03\x16\x00\x7e\xef" // |.....?........~. |
// /* 0a50 */ "\x01\x0f\x11\x00\xc7\x00\xc8\x00\x12\x00\x09\x1b\xe0\x3f\x38\x7e" // |.............?8~ |
// /* 0a60 */ "\xef\x00\x14\x00\xc8\x00\x69\x00\xd8\x00\x14\x00\xc8\x00\x69\x00" // |......i.......i. |
// /* 0a70 */ "\xd8\x00\x13\x00\xd8\x00\x14\x0a\x00\x09\x09\x0f\x05\x10\x09\x11" // |................ |
// /* 0a80 */ "\x09\x22\x09\x23\x07\x29\x0b\x26\x09\x30\x08\x09\x0a\x05\xdd\x00" // |.".#.).&.0...... |
// /* 0a90 */ "\xbe\x02\x09\x1b\x40\x15\x28\xe0\x00\xa2\x02\xe0\x00\xa2\x02\xdf" // |....@.(......... |
// /* 0aa0 */ "\x00\x72\x23\x00\x29\x0b\x26\x09\x30\x08\x20\x09\x11\x05\x10\x09" // |.r#.).&.0. ..... |
// /* 0ab0 */ "\x2a\x09\x20\x09\x05\x05\x2e\x09\x04\x09\x30\x04\x30\x09\x22\x09" // |*. .......0.0.". |
// /* 0ac0 */ "\x2f\x07\x08\x09\x20\x08\x08\x05\x24\x08\x2c\x09\x16\x0d\x1e\x03" // |/... ...$.,..... |
// /* 0ad0 */ "\x16\x09\x25\x03\x0c\x08\x09\x05\x08\x05\x0c\x08\x0b\x05\x10\x09" // |..%............. |
// /* 0ae0 */ "\x09\x09\x01\x05\x14\x0e\x0f\x03\x13\x09\x08\x08\x14\x08\x01\x03" // |................ |
// /* 0af0 */ "\x23\x0e\x29\x0b\x26\x09\x30\x08\x20\x09\x11\x05\x10\x09\x2a\x09" // |#.).&.0. .....*. |
// /* 0b00 */ "\x20\x09\x05\x05\x2e\x09\x04\x09\x30\x04\x30\x09\x27\x09\x21\x05" // | .......0.0.'.!. |
// /* 0b10 */ "\x2d\x0a\x12\x0b\x09\x0a\x05\x9f\x03\x50\x00\x09\x1b\x40\x15\x28" // |-........P...@.( |
// /* 0b20 */ "\xa2\x03\xc0\x03\xa2\x03\xc0\x03\xa1\x03\x06\x21\x00\x2d\x0a\x12" // |...........!.-.. |
// /* 0b30 */ "\x0b\x09\x0a\x3f\x13\x03\xbe\x02\x21\x00\x17\x00\xec\x08\x11\x0f" // |...?....!....... |
// /* 0b40 */ "\x01\x01\xfd\xfd\x09\x1b\xf0\x3f\x38\xff\xff\x00\xec\x08\x00\x1a" // |.......?8....... |
// /* 0b50 */ "\x03\xc1\x02\x2c\x03\xd1\x02\x1a\x03\xc1\x02\x2c\x03\xd1\x02\x19" // |...,.......,.... |
// /* 0b60 */ "\x03\xd1\x02\x04\x17\x00\x18\x0b\x19\x0a\x0f\x2f\xff\xfc\x03\x11" // |.........../.... |
// /* 0b70 */ "\x0f\x01\x01\xfd\xfd\x09\x1b\x40\x15\x28\x4a\x03\x55\x03\x4a\x03" // |.......@.(J.U.J. |
// /* 0b80 */ "\x55\x03\x49\x03\x02\x17\x00\x19\x0a\x3b\x29\x01\xfe\xff\xff\x09" // |U.I......;)..... |
// /* 0b90 */ "\x1b\x70\x15\x28\x00\x00\x00\xff\xff\x00\x6e\x03\x77\x03\x6e\x03" // |.p.(......n.w.n. |
// /* 0ba0 */ "\x77\x03\x6c\x03\x02\x02\x00\x19\x0a\x0f\xff\xff\x01\xef\x11\x0e" // |w.l............. |
// /* 0bb0 */ "\x01\xe9\x11\x11\x0b\x18\xff\x02\x11\x0f\xe8\x13\x18\xed\x11\x3f" // |...............? |
// /* 0bc0 */ "\x19\xef\xf5\x0f\xec\x08\x0d\x1b\x70\x15\x28\x5a\xe8\xc1\x02\xa2" // |........p.(Z.... |
// /* 0bd0 */ "\xd0\x02\xff\xff\x00\xec\x08\x00\x85\x03\x93\x03\x85\x03\x93\x03" // |................ |
// /* 0be0 */ "\x84\x03\x04\x19\x00\x02\x05\x19\x0a\x0f\x1d\xfd\x0c\x07\x11\x0f" // |................ |
// /* 0bf0 */ "\x01\x01\xfd\xfd\x09\x1b\x40\x15\x28\xa9\x03\xb4\x03\xa9\x03\xb4" // |......@.(....... |
// /* 0c00 */ "\x03\xa8\x03\x02\x1a\x00\x19\x0a\x0f\x28\xff\x0a\x03\x11\x0f\x01" // |.........(...... |
// /* 0c10 */ "\x01\xfd\xfd\x09\x1b\x40\x15\x28\xd2\x03\xe4\x03\xd2\x03\xe4\x03" // |.....@.(........ |
// /* 0c20 */ "\xd1\x03\x04\x1b\x00\x1a\x07\x09\x0a\x0f\x2a\x03\xdf\x02\x3f\x00" // |..........*...?. |
// /* 0c30 */ "\x17\x00\x11\x0f\x01\x01\xfd\xfd\x09\x1b\xc0\x3f\x38\x31\x03\xe2" // |...........?81.. |
// /* 0c40 */ "\x02\x61\x03\xf2\x02\x31\x03\xe2\x02\x61\x03\xf2\x02\x30\x03\xf2" // |.a...1...a...0.. |
// /* 0c50 */ "\x02\x0c\x1c\x00\x0f\x07\x0a\x09\x0f\x09\x10\x09\x0c\x09\x19\x0a" // |................ |
// /* 0c60 */ "\x3f\xff\xff\x01\xee\xff\xff\x11\x0e\x02\xc5\x13\x11\x0b\x3d\xfe" // |?.............=. |
// /* 0c70 */ "\x02\x11\x0f\xc3\x15\x3d\xeb\x11\x3f\x67\xeb\x0b\x15\xec\x08\x11" // |.....=..?g...... |
// /* 0c80 */ "\x0f\x01\x01\xfd\xfd\x09\x1b\x40\x15\x28\x98\x03\xd3\x03\x98\x03" // |.......@.(...... |
// /* 0c90 */ "\xd3\x03\x97\x03\x0e\x1d\x00\x0f\x0a\x1e\x09\x1e\x09\x0b\x09\x13" // |................ |
// /* 0ca0 */ "\x09\x0c\x08\x19\x0a\x3f\xff\xff\x01\xee\xff\xff\x11\x0e\x02\xba" // |.....?.......... |
// /* 0cb0 */ "\x13\x11\x0b\x48\xfe\x02\x11\x0f\xb8\x15\x48\xeb\x01\x3f\x3c\x00" // |...H......H..?<. |
// /* 0cc0 */ "\xc4\x02\x2e\x00\x1e\x00\xec\x08\x11\x0f\x01\x01\xfd\xfd\x09\x1b" // |................ |
// /* 0cd0 */ "\xc0\x3f\x38\x44\x00\xcc\x02\x64\x00\xdc\x02\x44\x00\xcc\x02\x64" // |.?8D...d...D...d |
// /* 0ce0 */ "\x00\xdc\x02\x43\x00\xdc\x02\x0a\x1f\x00\x09\x04\x20\x05\x15\x05" // |...C........ ... |
// /* 0cf0 */ "\x21\x08\x19\x0a\x3f\xff\xff\x01\xe7\xff\xff\x11\x0e\x02\xd6\x1a" // |!...?........... |
// /* 0d00 */ "\x11\x0b\x2c\xfe\x02\x11\x0f\xd4\x1c\x2c\xe4"                     //..,......,. |
// Dump done on RDP Wab Target (4) 3339 bytes |
// LOOPING on PDUs: 3328 |
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
// order(10):opaquerect(rect(0,87,15,132) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,87,15,132) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,219,1024,483) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,702,787,6) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,708,60,17) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,708,681,17) color=0x0008ec) |
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
// order(10):opaquerect(rect(0,725,60,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,725,918,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,735,60,3) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,735,704,3) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,738,810,20) color=0x0008ec) |
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
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(20,90,75,17) op=(20,90,75,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=106 data_len=18 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x22 0x09 0x08 0x07 0x11 0x08 0x11 0x09] |
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
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(224,90,473,17) op=(224,90,473,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=106 data_len=122 [0x08 0x00 0x11 0x08 0x11 0x09 0x22 0x09 0x23 0x07 0x16 0x0b 0x1e 0x03 0x24 0x09 0x0f 0x09 0x23 0x09 0x25 0x0b 0x20 0x08 0x0b 0x05 0x00 0x09 0x11 0x09 0x14 0x09 0x0f 0x03 0x09 0x09 0x0b 0x05 0x09 0x09 0x22 0x05 0x1f 0x07 0x08 0x04 0x26 0x08 0x0b 0x08 0x14 0x09 0x0f 0x03 0x0a 0x09 0x16 0x09 0x1e 0x03 0x01 0x09 0x14 0x0e 0x0f 0x03 0x13 0x09 0x08 0x08 0x14 0x08 0x01 0x03 0x08 0x0e 0x11 0x08 0x11 0x09 0x22 0x09 0x23 0x07 0x16 0x0b 0x1e 0x03 0x24 0x09 0x0f 0x09 0x23 0x09 0x25 0x0b 0x20 0x08 0x0b 0x05 0x00 0x09 0x11 0x09 0x14 0x09 0x0f 0x03 0x09 0x09 0x0b 0x05 0x09 0x09 0x27 0x05 0x28 0x05 0x12 0x09 0x12 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,89,80,18) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(930,90,28,17) op=(930,90,28,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=106 data_len=6 [0x28 0x00 0x12 0x09 0x12 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,109,994,22) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,111,200,18) color=0x00ef7e) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(20,112,95,17) op=(20,112,95,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=128 data_len=22 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x22 0x09 0x23 0x07 0x29 0x0b 0x26 0x09 0x02 0x08 0x29 0x09] |
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
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(224,112,466,17) op=(224,112,466,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=128 data_len=118 [0x23 0x00 0x29 0x0b 0x26 0x09 0x02 0x08 0x29 0x09 0x20 0x09 0x11 0x05 0x10 0x09 0x2a 0x09 0x20 0x09 0x05 0x05 0x05 0x09 0x04 0x09 0x29 0x04 0x2b 0x09 0x22 0x09 0x09 0x07 0x0b 0x05 0x24 0x09 0x20 0x09 0x08 0x05 0x24 0x08 0x2c 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x25 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0f 0x05 0x09 0x09 0x01 0x05 0x14 0x0e 0x0f 0x03 0x13 0x09 0x08 0x08 0x14 0x08 0x01 0x03 0x23 0x0e 0x29 0x0b 0x26 0x09 0x02 0x08 0x29 0x09 0x20 0x09 0x11 0x05 0x10 0x09 0x2a 0x09 0x20 0x09 0x05 0x05 0x05 0x09 0x04 0x09 0x29 0x04 0x2b 0x09 0x27 0x09 0x21 0x05 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,111,80,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(930,112,31,17) op=(930,112,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=128 data_len=6 [0x21 0x00 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,131,994,22) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,133,200,18) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(20,134,86,17) op=(20,134,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=150 data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x22 0x09 0x23 0x07 0x29 0x0b 0x26 0x09 0x06 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,133,702,18) color=0x00cebd) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(224,134,451,17) op=(224,134,451,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=150 data_len=114 [0x23 0x00 0x29 0x0b 0x26 0x09 0x06 0x08 0x20 0x09 0x11 0x05 0x10 0x09 0x2a 0x09 0x20 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x2b 0x04 0x06 0x09 0x22 0x09 0x2f 0x07 0x08 0x09 0x20 0x08 0x08 0x05 0x24 0x08 0x2c 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x25 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x14 0x0e 0x0f 0x03 0x13 0x09 0x08 0x08 0x14 0x08 0x01 0x03 0x23 0x0e 0x29 0x0b 0x26 0x09 0x06 0x08 0x20 0x09 0x11 0x05 0x10 0x09 0x2a 0x09 0x20 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x2b 0x04 0x06 0x09 0x27 0x09 0x21 0x05 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,133,80,18) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(930,134,31,17) op=(930,134,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=150 data_len=6 [0x21 0x00 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,153,994,22) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,155,200,18) color=0x00ef7e) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(20,156,86,17) op=(20,156,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=172 data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x22 0x09 0x23 0x07 0x29 0x0b 0x26 0x09 0x30 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,155,702,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(224,156,409,17) op=(224,156,409,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=172 data_len=108 [0x23 0x00 0x29 0x0b 0x26 0x09 0x30 0x08 0x20 0x09 0x11 0x05 0x09 0x09 0x16 0x05 0x20 0x03 0x05 0x05 0x2b 0x09 0x04 0x09 0x02 0x04 0x2e 0x09 0x22 0x09 0x08 0x07 0x24 0x08 0x2c 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x25 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x14 0x0e 0x0f 0x03 0x13 0x09 0x08 0x08 0x14 0x08 0x01 0x03 0x23 0x0e 0x29 0x0b 0x26 0x09 0x30 0x08 0x20 0x09 0x11 0x05 0x09 0x09 0x16 0x05 0x20 0x03 0x05 0x05 0x2b 0x09 0x04 0x09 0x02 0x04 0x2e 0x09 0x27 0x09 0x21 0x05 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,155,80,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(930,156,31,17) op=(930,156,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=172 data_len=6 [0x21 0x00 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,175,994,22) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,177,200,18) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(20,178,86,17) op=(20,178,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=194 data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x22 0x09 0x23 0x07 0x29 0x0b 0x26 0x09 0x30 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,177,702,18) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(224,178,390,17) op=(224,178,390,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=194 data_len=100 [0x23 0x00 0x29 0x0b 0x26 0x09 0x30 0x08 0x20 0x09 0x11 0x05 0x09 0x09 0x16 0x05 0x20 0x03 0x05 0x05 0x2b 0x09 0x04 0x09 0x02 0x04 0x2e 0x09 0x22 0x09 0x09 0x07 0x0b 0x05 0x24 0x09 0x0b 0x09 0x2c 0x09 0x11 0x0d 0x0c 0x09 0x16 0x05 0x0f 0x03 0x1e 0x09 0x01 0x09 0x14 0x0e 0x0f 0x03 0x13 0x09 0x08 0x08 0x14 0x08 0x01 0x03 0x23 0x0e 0x29 0x0b 0x26 0x09 0x30 0x08 0x20 0x09 0x11 0x05 0x09 0x09 0x16 0x05 0x20 0x03 0x05 0x05 0x2b 0x09 0x04 0x09 0x02 0x04 0x2e 0x09 0x27 0x09 0x21 0x05 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,177,80,18) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(930,178,31,17) op=(930,178,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=194 data_len=6 [0x21 0x00 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,197,994,22) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,199,200,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(20,200,86,17) op=(20,200,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=216 data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x22 0x09 0x23 0x07 0x29 0x0b 0x26 0x09 0x30 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,199,702,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(224,200,451,17) op=(224,200,451,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=216 data_len=114 [0x23 0x00 0x29 0x0b 0x26 0x09 0x30 0x08 0x20 0x09 0x11 0x05 0x10 0x09 0x2a 0x09 0x20 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x30 0x04 0x30 0x09 0x22 0x09 0x2f 0x07 0x08 0x09 0x20 0x08 0x08 0x05 0x24 0x08 0x2c 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x25 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x14 0x0e 0x0f 0x03 0x13 0x09 0x08 0x08 0x14 0x08 0x01 0x03 0x23 0x0e 0x29 0x0b 0x26 0x09 0x30 0x08 0x20 0x09 0x11 0x05 0x10 0x09 0x2a 0x09 0x20 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x30 0x04 0x30 0x09 0x27 0x09 0x21 0x05 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,199,80,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(930,200,31,17) op=(930,200,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=216 data_len=6 [0x21 0x00 0x2d 0x0a 0x12 0x0b] |
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
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,708,46,30) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(61,709,43,27) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(68,716,33,17) op=(68,716,33,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=67 glyph_y=732 data_len=10 [0x1f 0x00 0x09 0x04 0x20 0x05 0x15 0x05 0x21 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,708,44,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,710,2,28) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(104,708,2,30) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,736,46,2) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 6 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
// /* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
// /* 0000 */ "\x00\x00\x6b"                                                     //..k |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 103 bytes |
// Recv done on RDP Wab Target (4) 103 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x5d\x5d\x00\x17\x00\x09\x04" // |...h. ..p]]..... |
// /* 0010 */ "\x02\x00\x01\x00\x00\x02\x5d\x00\x02\x00\x00\x00\x00\x00\x00\x00" // |......]......... |
// /* 0020 */ "\x06\x00\x00\x00\x01\x0f\x6d\x03\xc0\x02\x17\x00\x12\x00\x09\x1b" // |......m......... |
// /* 0030 */ "\xf0\x3f\x38\x00\x00\x00\xff\xff\x00\x6e\x03\xc1\x02\x77\x03\xd1" // |.?8......n...w.. |
// /* 0040 */ "\x02\x6e\x03\xc1\x02\x77\x03\xd1\x02\x6c\x03\xd1\x02\x02\x02\x00" // |.n...w...l...... |
// /* 0050 */ "\x19\x0a\x0f\xff\xff\x01\xef\x11\x0e\x01\xe9\x11\x11\x0b\x18\xff" // |................ |
// /* 0060 */ "\x02\x11\x0f\xe8\x13\x18\xed"                                     //....... |
// Dump done on RDP Wab Target (4) 103 bytes |
// LOOPING on PDUs: 93 |
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
// /* 0000 */ "\x00\x00\x4a"                                                     //..J |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 70 bytes |
// Recv done on RDP Wab Target (4) 70 bytes |
// /* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x3c\x3c\x00\x17\x00\x09\x04" // |...h. ..p<<..... |
// /* 0010 */ "\x02\x00\x01\x00\x00\x02\x3c\x00\x02\x00\x00\x00\x00\x00\x00\x00" // |......<......... |
// /* 0020 */ "\x02\x00\x00\x00\x11\x3f\x19\xef\xf5\x0f\xec\x08\x2d\x1b\x70\x15" // |.....?......-.p. |
// /* 0030 */ "\x28\xff\xff\x00\xec\x08\x00\x85\x03\x93\x03\x85\x03\x93\x03\x84" // |(............... |
// /* 0040 */ "\x03\x04\x19\x00\x02\x05"                                         //...... |
// Dump done on RDP Wab Target (4) 70 bytes |
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
// ~mod_rdp(): Recv order count      = 458 |
// ~mod_rdp(): Recv bmp update count = 0 |
// RDP Wab Target (0): total_received=9869, total_sent=1629 |
} /* end outdata */;

const char indata[] =
{
// connecting to 10.10.47.154:3389 |
// connection to 10.10.47.154:3389 succeeded : socket 4 |
// --------- CREATION OF MOD ------------------------ |
// Creation of new mod 'RDP' |
// ModRDPParams target_user="x" |
// ModRDPParams target_password="<hidden>" |
// ModRDPParams target_host="10.10.47.154" |
// ModRDPParams client_address="192.168.1.100" |
// ModRDPParams auth_user="" |
// ModRDPParams target_application="" |
// ModRDPParams enable_tls=yes |
// ModRDPParams enable_nla=no |
// ModRDPParams enable_krb=no |
// ModRDPParams enable_fastpath=no |
// ModRDPParams enable_mem3blt=no |
// ModRDPParams enable_bitmap_update=no |
// ModRDPParams enable_new_pointer=no |
// ModRDPParams enable_glyph_cache=no |
// ModRDPParams enable_session_probe=no |
// ModRDPParams enable_session_probe_launch_mask=yes |
// ModRDPParams session_probe_use_clipboard_based_launcher=no |
// ModRDPParams session_probe_launch_timeout=0 |
// ModRDPParams session_probe_launch_fallback_timeout=0 |
// ModRDPParams session_probe_start_launch_timeout_timer_only_after_logon=yes |
// ModRDPParams session_probe_on_launch_failure=1 |
// ModRDPParams session_probe_keepalive_timeout=0 |
// ModRDPParams session_probe_on_keepalive_timeout_disconnect_user=yes |
// ModRDPParams session_probe_end_disconnected_session=no |
// ModRDPParams session_probe_customize_executable_name=no |
// ModRDPParams dsiable_clipboard_log_syslog=no |
// ModRDPParams dsiable_clipboard_log_wrm=no |
// ModRDPParams dsiable_file_system_log_syslog=no |
// ModRDPParams dsiable_file_system_log_wrm=no |
// ModRDPParams enable_transparent_mode=no |
// ModRDPParams output_filename="" |
// ModRDPParams persistent_key_list_transport=<(nil)> |
// ModRDPParams transparent_recorder_transport=<(nil)> |
// ModRDPParams key_flags=7 |
// ModRDPParams acl=<(nil)> |
// ModRDPParams outbound_connection_blocking_rules="" |
// ModRDPParams ignore_auth_channel=no |
// ModRDPParams auth_channel="" |
// ModRDPParams alternate_shell="" |
// ModRDPParams working_dir="" |
// ModRDPParams use_client_provided_alternate_shell=no |
// ModRDPParams target_application_account="" |
// ModRDPParams target_application_password="<hidden>" |
// ModRDPParams rdp_compression=0 |
// ModRDPParams error_message=<(nil)> |
// ModRDPParams disconnect_on_logon_user_change=no |
// ModRDPParams open_session_timeout=0 |
// ModRDPParams server_cert_store=yes |
// ModRDPParams server_cert_check=1 |
// ModRDPParams server_access_allowed_message=1 |
// ModRDPParams server_cert_create_message=1 |
// ModRDPParams server_cert_success_message=1 |
// ModRDPParams server_cert_failure_message=1 |
// ModRDPParams server_cert_error_message=1 |
// ModRDPParams hide_client_name=no |
// ModRDPParams extra_orders= |
// ModRDPParams enable_persistent_disk_bitmap_cache=no |
// ModRDPParams enable_cache_waiting_list=no |
// ModRDPParams persist_bitmap_cache_on_disk=no |
// ModRDPParams password_printing_mode=0 |
// ModRDPParams allow_channels=<none> |
// ModRDPParams deny_channels=<none> |
// ModRDPParams server_redirection_support=yes |
// ModRDPParams bogus_sc_net_size=yes |
// ModRDPParams proxy_managed_drives= |
// ModRDPParams lang=EN |
// ModRDPParams verbose=0x000001FF |
// ModRDPParams cache_verbose=0x00000000 |
// RDP Extra orders="" |
// Remote RDP Server domain="" login="x" host="192-168-1-100" |
// enable_session_probe=no |
// Server key layout is 40c |
// Init with Redir_info: RedirectionInfo(valid=false, session_id=0, host='', username='', password='<null>', domain='', LoadBalanceInfoLength=0, dont_store_username=false, server_tsv_capable=false, smart_card_logon=false) |
// ServerRedirectionSupport=true |
// mod_rdp::Early TLS Security Exchange |
// RdpNego::NEGO_STATE_INITIAL |
// RdpNego::send_x224_connection_request_pdu |
// Send cookie: |
// /* 0000 */ "\x43\x6f\x6f\x6b\x69\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68" // |Cookie: mstshash |
// /* 0010 */ "\x3d\x78\x0d\x0a"                                                 //=x.. |
// Sending on RDP Wab Target (4) 39 bytes |
// /* 0000 */ "\x03\x00\x00\x27\x22\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" // |...'"......Cooki |
// /* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x78\x0d\x0a\x01" // |e: mstshash=x... |
// /* 0020 */ "\x00\x08\x00\x01\x00\x00\x00"                                     //....... |
// Sent dumped on RDP Wab Target (4) 39 bytes |
// RdpNego::send_x224_connection_request_pdu done |
// mod_rdp::Early TLS Security Exchange |
// RdpNego::NEGO_STATE_TLS |
// RdpNego::recv_connection_confirm |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x13"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 15 bytes |
// Recv done on RDP Wab Target (4) 15 bytes |
/* 0000 */ "\x0e\xd0\x00\x00\x00\x00\x00\x02\x01\x08\x00\x01\x00\x00\x00"     //............... |
// Dump done on RDP Wab Target (4) 15 bytes |
// NEG_RSP_TYPE=2 NEG_RSP_FLAGS=1 NEG_RSP_LENGTH=8 NEG_RSP_SELECTED_PROTOCOL=1 |
// CC Recv: PROTOCOL TLS 1.0 |
// activating SSL |
// Client TLS start |
// SSL_connect() |
// SSL_get_peer_certificate() |
// failed to create directory /etc/rdpproxy/cert/rdp : Permission denied [13] |
// Failed to create certificate directory: /etc/rdpproxy/cert/rdp/device_id  |
// Dumping X509 peer certificate: "/etc/rdpproxy/cert/rdp/device_id/rdp,10.10.47.154,3389,X509.pem" |
// Failed to dump X509 peer certificate |
// SocketTransport::X509_get_pubkey() |
// SocketTransport::i2d_PublicKey() |
// SocketTransport::i2d_PublicKey() |
// TLS::X509::issuer=CN = wabinstall.ifr.lan |
// TLS::X509::subject=CN = wabinstall.ifr.lan |
// TLS::X509::fingerprint=b4:3f:9b:85:33:ce:d9:af:71:59:1c:b0:4e:5e:17:5c:09:bd:ad:06 |
// SocketTransport::enable_tls() done |
// SocketTransport::enable_tls() done |
// RdpNego::recv_connection_confirm done |
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
// cs_core::serverSelectedProtocol = 1 |
// CS_Cluster: Server Redirection Supported |
// Sending to server GCC User Data CS_CLUSTER (12 bytes) |
// cs_cluster::flags [000d] |
// cs_cluster::redirectedSessionID = 0 |
// Sending to server GCC User Data CS_SECURITY (12 bytes) |
// CSSecGccUserData::encryptionMethods 3 |
// CSSecGccUserData::extEncryptionMethods 0 |
// Sending on RDP Wab Target (4) 376 bytes |
// /* 0000 */ "\x03\x00\x01\x78\x02\xf0\x80\x7f\x65\x82\x01\x6c\x04\x01\x01\x04" // |...x....e..l.... |
// /* 0010 */ "\x01\x01\x01\x01\xff\x30\x1a\x02\x01\x22\x02\x01\x02\x02\x01\x00" // |.....0..."...... |
// /* 0020 */ "\x02\x01\x01\x02\x01\x00\x02\x01\x01\x02\x03\x00\xff\xff\x02\x01" // |................ |
// /* 0030 */ "\x02\x30\x19\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02" // |.0.............. |
// /* 0040 */ "\x01\x00\x02\x01\x01\x02\x02\x04\x20\x02\x01\x02\x30\x1f\x02\x03" // |........ ...0... |
// /* 0050 */ "\x00\xff\xff\x02\x02\xfc\x17\x02\x03\x00\xff\xff\x02\x01\x01\x02" // |................ |
// /* 0060 */ "\x01\x00\x02\x01\x01\x02\x03\x00\xff\xff\x02\x01\x02\x04\x82\x01" // |................ |
// /* 0070 */ "\x07\x00\x05\x00\x14\x7c\x00\x01\x80\xfe\x00\x08\x00\x10\x00\x01" // |.....|.......... |
// /* 0080 */ "\xc0\x00\x44\x75\x63\x61\x80\xf0\x01\xc0\xd8\x00\x04\x00\x08\x00" // |..Duca.......... |
// /* 0090 */ "\x00\x04\x00\x03\x01\xca\x03\xaa\x0c\x04\x00\x00\x28\x0a\x00\x00" // |............(... |
// /* 00a0 */ "\x31\x00\x39\x00\x32\x00\x2d\x00\x31\x00\x36\x00\x38\x00\x2d\x00" // |1.9.2.-.1.6.8.-. |
// /* 00b0 */ "\x31\x00\x2d\x00\x31\x00\x30\x00\x30\x00\x00\x00\x00\x00\x00\x00" // |1.-.1.0.0....... |
// /* 00c0 */ "\x04\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\xca\x01\x00" // |................ |
// /* 0110 */ "\x00\x00\x00\x00\x10\x00\x07\x00\x01\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00" // |................ |
// /* 0160 */ "\x04\xc0\x0c\x00\x0d\x00\x00\x00\x00\x00\x00\x00\x02\xc0\x0c\x00" // |................ |
// /* 0170 */ "\x03\x00\x00\x00\x00\x00\x00\x00"                                 //........ |
// Sent dumped on RDP Wab Target (4) 376 bytes |
// mod_rdp::Basic Settings Exchange |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x65"                                                     //..e |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 97 bytes |
// Recv done on RDP Wab Target (4) 97 bytes |
/* 0000 */ "\x02\xf0\x80\x7f\x66\x5b\x0a\x01\x00\x02\x01\x00\x30\x1a\x02\x01" // |....f[......0... |
/* 0010 */ "\x22\x02\x01\x03\x02\x01\x00\x02\x01\x01\x02\x01\x00\x02\x01\x01" // |"............... |
/* 0020 */ "\x02\x03\x00\xff\xf8\x02\x01\x02\x04\x37\x00\x05\x00\x14\x7c\x00" // |.........7....|. |
/* 0030 */ "\x01\x2a\x14\x76\x0a\x01\x01\x00\x01\xc0\x00\x4d\x63\x44\x6e\x80" // |.*.v.......McDn. |
/* 0040 */ "\x20\x01\x0c\x0c\x00\x04\x00\x08\x00\x01\x00\x00\x00\x03\x0c\x08" // | ............... |
/* 0050 */ "\x00\xeb\x03\x00\x00\x02\x0c\x0c\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0060 */ "\x00"                                                             //. |
// Dump done on RDP Wab Target (4) 97 bytes |
// GCC::UserData tag=0c01 length=12 |
// Received from server GCC User Data SC_CORE (12 bytes) |
// sc_core::version [80004] RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 servers) |
// sc_core::clientRequestedProtocols  = 1 |
// GCC::UserData tag=0c03 length=8 |
// server_channels_count=0 sent_channels_count=0 |
// Received from server GCC User Data SC_NET (8 bytes) |
// sc_net::MCSChannelId   = 1003 |
// sc_net::channelCount   = 0 |
// GCC::UserData tag=0c02 length=12 |
// Received from server GCC User Data SC_SECURITY (12 bytes) |
// sc_security::encryptionMethod = 0 |
// sc_security::encryptionLevel  = 0 |
// No encryption |
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
// mod_rdp::Secure Settings Exchange |
// mod_rdp::send_client_info_pdu |
// send data request |
// send extended login info (RDP5-style) 1017b :x |
// Sending on RDP Wab Target (4) 331 bytes |
// /* 0000 */ "\x03\x00\x01\x4b\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x81\x3c\x40" // |...K...d. ..p.<@ |
// /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x7b\x01\x01\x00\x00\x00\x02\x00\x02" // |.......{........ |
// /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x78\x00\x00\x00\x78\x00\x00\x00\x00" // |.......x...x.... |
// /* 0030 */ "\x00\x00\x00\x02\x00\x1c\x00\x31\x00\x39\x00\x32\x00\x2e\x00\x31" // |.......1.9.2...1 |
// /* 0040 */ "\x00\x36\x00\x38\x00\x2e\x00\x31\x00\x2e\x00\x31\x00\x30\x00\x30" // |.6.8...1...1.0.0 |
// /* 0050 */ "\x00\x00\x00\x40\x00\x43\x00\x3a\x00\x5c\x00\x57\x00\x69\x00\x6e" // |...@.C.:...W.i.n |
// /* 0060 */ "\x00\x64\x00\x6f\x00\x77\x00\x73\x00\x5c\x00\x53\x00\x79\x00\x73" // |.d.o.w.s...S.y.s |
// /* 0070 */ "\x00\x74\x00\x65\x00\x6d\x00\x33\x00\x32\x00\x5c\x00\x6d\x00\x73" // |.t.e.m.3.2...m.s |
// /* 0080 */ "\x00\x74\x00\x73\x00\x63\x00\x61\x00\x78\x00\x2e\x00\x64\x00\x6c" // |.t.s.c.a.x...d.l |
// /* 0090 */ "\x00\x6c\x00\x00\x00\x00\x00\x00\x00\x47\x00\x00\x00\x00\x00\x00" // |.l.......G...... |
// /* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00\x05" // |................ |
// /* 00e0 */ "\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x47\x00\x00" // |.............G.. |
// /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03" // |................ |
// /* 0130 */ "\x00\x00\x00\x05\x00\x01\x00\x00\x00\x00\x00\x00\x00\xc4\xff\xff" // |................ |
// /* 0140 */ "\xff\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00"                     //........... |
// Sent dumped on RDP Wab Target (4) 331 bytes |
// send data request done |
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
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::Bias 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardName GMT |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMonth 10 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDay 5 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wHour 2 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardBias 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightName GMT (heure d'été) |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wYear 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMonth 3 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDayOfWeek 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDay 5 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wHour 1 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMinute 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wSecond 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMilliseconds 0 |
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightBias 4294967236 |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x81\x42\x80\x00\x00\x00\x01" // |...h. ..p.B..... |
/* 0010 */ "\x02\x3e\x01\x7b\x3c\x31\xa6\xae\xe8\x74\xf6\xb4\xa5\x03\x90\xe7" // |.>.{<1...t...... |
/* 0020 */ "\xc2\xc7\x39\xba\x53\x1c\x30\x54\x6e\x90\x05\xd0\x05\xce\x44\x18" // |..9.S.0Tn.....D. |
/* 0030 */ "\x91\x83\x81\x00\x00\x04\x00\x2c\x00\x00\x00\x4d\x00\x69\x00\x63" // |.......,...M.i.c |
/* 0040 */ "\x00\x72\x00\x6f\x00\x73\x00\x6f\x00\x66\x00\x74\x00\x20\x00\x43" // |.r.o.s.o.f.t. .C |
/* 0050 */ "\x00\x6f\x00\x72\x00\x70\x00\x6f\x00\x72\x00\x61\x00\x74\x00\x69" // |.o.r.p.o.r.a.t.i |
/* 0060 */ "\x00\x6f\x00\x6e\x00\x00\x00\x08\x00\x00\x00\x32\x00\x33\x00\x36" // |.o.n.......2.3.6 |
/* 0070 */ "\x00\x00\x00\x0d\x00\x04\x00\x01\x00\x00\x00\x03\x00\xb8\x00\x01" // |................ |
/* 0080 */ "\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x06\x00\x5c\x00\x52" // |...............R |
/* 0090 */ "\x53\x41\x31\x48\x00\x00\x00\x00\x02\x00\x00\x3f\x00\x00\x00\x01" // |SA1H.......?.... |
/* 00a0 */ "\x00\x01\x00\x01\xc7\xc9\xf7\x8e\x5a\x38\xe4\x29\xc3\x00\x95\x2d" // |........Z8.)...- |
/* 00b0 */ "\xdd\x4c\x3e\x50\x45\x0b\x0d\x9e\x2a\x5d\x18\x63\x64\xc4\x2c\xf7" // |.L>PE...*].cd.,. |
/* 00c0 */ "\x8f\x29\xd5\x3f\xc5\x35\x22\x34\xff\xad\x3a\xe6\xe3\x95\x06\xae" // |.).?.5"4..:..... |
/* 00d0 */ "\x55\x82\xe3\xc8\xc7\xb4\xa8\x47\xc8\x50\x71\x74\x29\x53\x89\x6d" // |U......G.Pqt)S.m |
/* 00e0 */ "\x9c\xed\x70\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x48\x00\xa8" // |..p..........H.. |
/* 00f0 */ "\xf4\x31\xb9\xab\x4b\xe6\xb4\xf4\x39\x89\xd6\xb1\xda\xf6\x1e\xec" // |.1..K...9....... |
/* 0100 */ "\xb1\xf0\x54\x3b\x5e\x3e\x6a\x71\xb4\xf7\x75\xc8\x16\x2f\x24\x00" // |..T;^>jq..u../$. |
/* 0110 */ "\xde\xe9\x82\x99\x5f\x33\x0b\xa9\xa6\x94\xaf\xcb\x11\xc3\xf2\xdb" // |...._3.......... |
/* 0120 */ "\x09\x42\x68\x29\x56\x58\x01\x56\xdb\x59\x03\x69\xdb\x7d\x37\x00" // |.Bh)VX.V.Y.i.}7. |
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x0e\x00\x0e\x00\x6d" // |...............m |
/* 0140 */ "\x69\x63\x72\x6f\x73\x6f\x66\x74\x2e\x63\x6f\x6d\x00"             //icrosoft.com. |
// Dump done on RDP Wab Target (4) 333 bytes |
// Rdp::License Request |
// send data request |
// Sending on RDP Wab Target (4) 163 bytes |
// /* 0000 */ "\x03\x00\x00\xa3\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x80\x94\x80" // |.......d. ..p... |
// /* 0010 */ "\x00\x00\x00\x13\x03\x90\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02" // |................ |
// /* 0040 */ "\x00\x48\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |.H.............. |
// /* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0f\x00\x02\x00\x78" // |...............x |
// /* 0090 */ "\x00\x10\x00\x0e\x00\x31\x39\x32\x2d\x31\x36\x38\x2d\x31\x2d\x31" // |.....192-168-1-1 |
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
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x14\x80\x00\x10\x00\xff\x02" // |...h. ..p....... |
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
/* 0000 */ "\x00\x01\x37"                                                     //..7 |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 307 bytes |
// Recv done on RDP Wab Target (4) 307 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x81\x28\x28\x01\x11\x00\x09" // |...h. ..p.((.... |
/* 0010 */ "\x04\x02\x00\x01\x00\x04\x00\x12\x01\x52\x44\x50\x00\x09\x00\x00" // |.........RDP.... |
/* 0020 */ "\x00\x01\x00\x18\x00\x01\x00\x03\x00\x00\x02\x00\x00\x00\x00\x01" // |................ |
/* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x1c\x00\x10\x00\x01" // |................ |
/* 0040 */ "\x00\x01\x00\x01\x00\x00\x04\x00\x03\x00\x00\x01\x00\x01\x00\x00" // |................ |
/* 0050 */ "\x08\x01\x00\x00\x00\x0e\x00\x08\x00\x01\x00\x00\x00\x03\x00\x58" // |...............X |
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0070 */ "\x00\x40\x42\x0f\x00\x01\x00\x14\x00\x00\x00\x01\x00\x2f\x00\x22" // |.@B........../." |
/* 0080 */ "\x00\x01\x01\x01\x01\x01\x00\x00\x00\x01\x01\x01\x00\x00\x00\x00" // |................ |
/* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x01\x00\x01\x00\x00\x00" // |................ |
/* 00a0 */ "\x00\xa1\x06\x00\x00\x40\x42\x0f\x00\x40\x42\x0f\x00\x01\x00\x00" // |.....@B..@B..... |
/* 00b0 */ "\x00\x00\x00\x00\x00\x12\x00\x08\x00\x01\x00\x00\x00\x0a\x00\x08" // |................ |
/* 00c0 */ "\x00\x06\x00\x00\x00\x08\x00\x0a\x00\x01\x00\x19\x00\x19\x00\x09" // |................ |
/* 00d0 */ "\x00\x08\x00\x09\x04\xe2\xb5\x0d\x00\x58\x00\x29\x00\x00\x00\x00" // |.........X.).... |
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0130 */ "\x00\x00\x00"                                                     //... |
// Dump done on RDP Wab Target (4) 307 bytes |
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
// Input caps::imeFileName  |
// mod_rdp::process_server_caps done |
// mod_rdp::send_confirm_active |
// send data request |
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
// Input caps::imeFileName  |
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
// Sending on RDP Wab Target (4) 424 bytes |
// /* 0000 */ "\x03\x00\x01\xa8\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x81\x99\x99" // |.......d. ..p... |
// /* 0010 */ "\x01\x13\x00\x09\x04\x02\x00\x01\x00\xea\x03\x05\x00\x84\x01\x4d" // |...............M |
// /* 0020 */ "\x53\x54\x53\x43\x0d\x00\x00\x00\x01\x00\x18\x00\x01\x00\x03\x00" // |STSC............ |
// /* 0030 */ "\x00\x02\x00\x00\x00\x00\x0c\x04\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0040 */ "\x02\x00\x1c\x00\x10\x00\x01\x00\x01\x00\x01\x00\x00\x04\x00\x03" // |................ |
// /* 0050 */ "\x00\x00\x01\x00\x01\x00\x00\x08\x01\x00\x00\x00\x03\x00\x58\x00" // |..............X. |
// /* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0070 */ "\x00\x00\x00\x00\x01\x00\x14\x00\x00\x00\x01\x00\x00\x00\xaa\x00" // |................ |
// /* 0080 */ "\x01\x01\x01\x01\x00\x00\x00\x00\x01\x00\x01\x00\x00\x01\x00\x00" // |................ |
// /* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00" // |................ |
// /* 00a0 */ "\xa1\x06\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 00b0 */ "\xe4\x04\x00\x00\x04\x00\x28\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |......(......... |
// /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 00d0 */ "\x58\x02\x00\x02\x2c\x01\x00\x08\x06\x01\x00\x20\x0a\x00\x08\x00" // |X...,...... .... |
// /* 00e0 */ "\x06\x00\x00\x00\x07\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 00f0 */ "\x05\x00\x0c\x00\x00\x00\x00\x00\x02\x00\x02\x00\x08\x00\x08\x00" // |................ |
// /* 0100 */ "\x01\x00\x14\x00\x09\x00\x08\x00\x00\x00\x00\x00\x0d\x00\x58\x00" // |..............X. |
// /* 0110 */ "\x01\x00\x00\x00\x09\x04\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0120 */ "\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// /* 0160 */ "\x00\x00\x00\x00\x0c\x00\x08\x00\x01\x00\x00\x00\x0e\x00\x08\x00" // |................ |
// /* 0170 */ "\x01\x00\x00\x00\x10\x00\x34\x00\xfe\x00\x04\x00\xfe\x00\x04\x00" // |......4......... |
// /* 0180 */ "\xfe\x00\x08\x00\xfe\x00\x08\x00\xfe\x00\x10\x00\xfe\x00\x20\x00" // |.............. . |
// /* 0190 */ "\xfe\x00\x40\x00\xfe\x00\x80\x00\xfe\x00\x00\x01\x40\x00\x00\x08" // |..@.........@... |
// /* 01a0 */ "\x00\x01\x00\x01\x00\x00\x00\x00"                                 //........ |
// Sent dumped on RDP Wab Target (4) 424 bytes |
// send data request done |
// mod_rdp::send_confirm_active done |
// Waiting for answer to confirm active |
// mod_rdp::send_synchronise |
// send data request |
// Sending on RDP Wab Target (4) 36 bytes |
// /* 0000 */ "\x03\x00\x00\x24\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x16\x16\x00" // |...$...d. ..p... |
// /* 0010 */ "\x17\x00\x09\x04\x02\x00\x01\x00\x00\x02\x16\x00\x1f\x00\x00\x00" // |................ |
// /* 0020 */ "\x01\x00\xea\x03"                                                 //.... |
// Sent dumped on RDP Wab Target (4) 36 bytes |
// send data request done |
// mod_rdp::send_synchronise done |
// mod_rdp::send_control |
// send data request |
// Sending on RDP Wab Target (4) 40 bytes |
// /* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x1a\x1a\x00" // |...(...d. ..p... |
// /* 0010 */ "\x17\x00\x09\x04\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00" // |................ |
// /* 0020 */ "\x04\x00\x00\x00\x00\x00\x00\x00"                                 //........ |
// Sent dumped on RDP Wab Target (4) 40 bytes |
// send data request done |
// mod_rdp::send_control done |
// mod_rdp::send_control |
// send data request |
// Sending on RDP Wab Target (4) 40 bytes |
// /* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x1a\x1a\x00" // |...(...d. ..p... |
// /* 0010 */ "\x17\x00\x09\x04\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00" // |................ |
// /* 0020 */ "\x01\x00\x00\x00\x00\x00\x00\x00"                                 //........ |
// Sent dumped on RDP Wab Target (4) 40 bytes |
// send data request done |
// mod_rdp::send_control done |
// use rdp5 |
// mod_rdp::send_fonts |
// send data request |
// Sending on RDP Wab Target (4) 40 bytes |
// /* 0000 */ "\x03\x00\x00\x28\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x1a\x1a\x00" // |...(...d. ..p... |
// /* 0010 */ "\x17\x00\x09\x04\x02\x00\x01\x00\x00\x02\x1a\x00\x27\x00\x00\x00" // |............'... |
// /* 0020 */ "\x00\x00\x00\x00\x03\x00\x32\x00"                                 //......2. |
// Sent dumped on RDP Wab Target (4) 40 bytes |
// send data request done |
// mod_rdp::send_fonts done |
// mod_rdp::send_input_slowpath |
// send data request |
// Sending on RDP Wab Target (4) 48 bytes |
// /* 0000 */ "\x03\x00\x00\x30\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x22\x22\x00" // |...0...d. ..p"". |
// /* 0010 */ "\x17\x00\x09\x04\x02\x00\x01\x00\x00\x04\x22\x00\x1c\x00\x00\x00" // |.........."..... |
// /* 0020 */ "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
// Sent dumped on RDP Wab Target (4) 48 bytes |
// send data request done |
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
/* 0000 */ "\x00\x00\x24"                                                     //..$ |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 32 bytes |
// Recv done on RDP Wab Target (4) 32 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x16\x16\x00\x17\x00\x09\x04" // |...h. ..p....... |
/* 0010 */ "\x02\x00\x01\x00\x00\x02\x16\x00\x1f\x00\x00\x00\x01\x00\xea\x03" // |................ |
// Dump done on RDP Wab Target (4) 32 bytes |
// LOOPING on PDUs: 22 |
// PDUTYPE_DATAPDU |
// WAITING_SYNCHRONIZE |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x28"                                                     //..( |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 36 bytes |
// Recv done on RDP Wab Target (4) 36 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x1a\x1a\x00\x17\x00\x09\x04" // |...h. ..p....... |
/* 0010 */ "\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00\x04\x00\x00\x00" // |................ |
/* 0020 */ "\xea\x03\x00\x00"                                                 //.... |
// Dump done on RDP Wab Target (4) 36 bytes |
// LOOPING on PDUs: 26 |
// PDUTYPE_DATAPDU |
// WAITING_CTL_COOPERATE |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x28"                                                     //..( |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 36 bytes |
// Recv done on RDP Wab Target (4) 36 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x1a\x1a\x00\x17\x00\x09\x04" // |...h. ..p....... |
/* 0010 */ "\x02\x00\x01\x00\x00\x02\x1a\x00\x14\x00\x00\x00\x02\x00\x00\x00" // |................ |
/* 0020 */ "\xea\x03\x00\x00"                                                 //.... |
// Dump done on RDP Wab Target (4) 36 bytes |
// LOOPING on PDUs: 26 |
// PDUTYPE_DATAPDU |
// WAITING_GRANT_CONTROL_COOPERATE |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\xcd"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 201 bytes |
// Recv done on RDP Wab Target (4) 201 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x80\xbe\xbe\x00\x17\x00\x09" // |...h. ..p....... |
/* 0010 */ "\x04\x02\x00\x01\x00\x00\x02\xbe\x00\x28\x00\x00\x00\xff\x02\xb6" // |.........(...... |
/* 0020 */ "\x00\x28\x00\x00\x00\x27\x00\x27\x00\x03\x00\x04\x00\x00\x00\x26" // |.(...'.'.......& |
/* 0030 */ "\x00\x01\x00\x1e\x00\x02\x00\x1f\x00\x03\x00\x1d\x00\x04\x00\x27" // |...............' |
/* 0040 */ "\x00\x05\x00\x0b\x00\x06\x00\x28\x00\x08\x00\x21\x00\x09\x00\x20" // |.......(...!...  |
/* 0050 */ "\x00\x0a\x00\x22\x00\x0b\x00\x25\x00\x0c\x00\x24\x00\x0d\x00\x23" // |..."...%...$...# |
/* 0060 */ "\x00\x0e\x00\x19\x00\x0f\x00\x16\x00\x10\x00\x15\x00\x11\x00\x1c" // |................ |
/* 0070 */ "\x00\x12\x00\x1b\x00\x13\x00\x1a\x00\x14\x00\x17\x00\x15\x00\x18" // |................ |
/* 0080 */ "\x00\x16\x00\x0e\x00\x18\x00\x0c\x00\x19\x00\x0d\x00\x1a\x00\x12" // |................ |
/* 0090 */ "\x00\x1b\x00\x14\x00\x1f\x00\x13\x00\x20\x00\x00\x00\x21\x00\x0a" // |......... ...!.. |
/* 00a0 */ "\x00\x22\x00\x06\x00\x23\x00\x07\x00\x24\x00\x08\x00\x25\x00\x09" // |."...#...$...%.. |
/* 00b0 */ "\x00\x26\x00\x04\x00\x27\x00\x03\x00\x28\x00\x02\x00\x29\x00\x01" // |.&...'...(...).. |
/* 00c0 */ "\x00\x2a\x00\x05\x00\x2b\x00\x2a\x00"                             //.*...+.*. |
// Dump done on RDP Wab Target (4) 201 bytes |
// LOOPING on PDUs: 190 |
// PDUTYPE_DATAPDU |
// PDUTYPE2_FONTMAP |
// mod_rdp::send_input_slowpath |
// send data request |
// Sending on RDP Wab Target (4) 48 bytes |
// /* 0000 */ "\x03\x00\x00\x30\x02\xf0\x80\x64\x00\x20\x03\xeb\x70\x22\x22\x00" // |...0...d. ..p"". |
// /* 0010 */ "\x17\x00\x09\x04\x02\x00\x01\x00\x00\x04\x22\x00\x1c\x00\x00\x00" // |.........."..... |
// /* 0020 */ "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00" // |................ |
// Sent dumped on RDP Wab Target (4) 48 bytes |
// send data request done |
// mod_rdp::send_input_slowpath done |
// ========= CREATION OF MOD DONE ==================== |
// ===================> count = 0 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x24"                                                     //..$ |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 32 bytes |
// Recv done on RDP Wab Target (4) 32 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x16\x16\x00\x17\x00\x09\x04" // |...h. ..p....... |
/* 0010 */ "\x02\x00\x01\x00\x00\x02\x16\x00\x02\x00\x00\x00\x03\x00\x00\x00" // |................ |
// Dump done on RDP Wab Target (4) 32 bytes |
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
/* 0000 */ "\x00\x04\x71"                                                     //..q |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 1133 bytes |
// Recv done on RDP Wab Target (4) 1133 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x84\x62\x62\x04\x17\x00\x09" // |...h. ..p.bb.... |
/* 0010 */ "\x04\x02\x00\x01\x00\x00\x02\x62\x04\x02\x00\x00\x00\x00\x00\x00" // |.......b........ |
/* 0020 */ "\x00\x07\x00\x00\x00\x03\xfc\x03\x00\x00\x01\x00\x00\x01\x00\x00" // |................ |
/* 0030 */ "\x00\x00\x55\x00\x00\x00\xaa\x00\x00\x00\xff\x00\x00\x00\x00\x24" // |..U............$ |
/* 0040 */ "\x00\x00\x55\x24\x00\x00\xaa\x24\x00\x00\xff\x24\x00\x00\x00\x49" // |..U$...$...$...I |
/* 0050 */ "\x00\x00\x55\x49\x00\x00\xaa\x49\x00\x00\xff\x49\x00\x00\x00\x6d" // |..UI...I...I...m |
/* 0060 */ "\x00\x00\x55\x6d\x00\x00\xaa\x6d\x00\x00\xff\x6d\x00\x00\x00\x92" // |..Um...m...m.... |
/* 0070 */ "\x00\x00\x55\x92\x00\x00\xaa\x92\x00\x00\xff\x92\x00\x00\x00\xb6" // |..U............. |
/* 0080 */ "\x00\x00\x55\xb6\x00\x00\xaa\xb6\x00\x00\xff\xb6\x00\x00\x00\xdb" // |..U............. |
/* 0090 */ "\x00\x00\x55\xdb\x00\x00\xaa\xdb\x00\x00\xff\xdb\x00\x00\x00\xff" // |..U............. |
/* 00a0 */ "\x00\x00\x55\xff\x00\x00\xaa\xff\x00\x00\xff\xff\x00\x00\x00\x00" // |..U............. |
/* 00b0 */ "\x24\x00\x55\x00\x24\x00\xaa\x00\x24\x00\xff\x00\x24\x00\x00\x24" // |$.U.$...$...$..$ |
/* 00c0 */ "\x24\x00\x55\x24\x24\x00\xaa\x24\x24\x00\xff\x24\x24\x00\x00\x49" // |$.U$$..$$..$$..I |
/* 00d0 */ "\x24\x00\x55\x49\x24\x00\xaa\x49\x24\x00\xff\x49\x24\x00\x00\x6d" // |$.UI$..I$..I$..m |
/* 00e0 */ "\x24\x00\x55\x6d\x24\x00\xaa\x6d\x24\x00\xff\x6d\x24\x00\x00\x92" // |$.Um$..m$..m$... |
/* 00f0 */ "\x24\x00\x55\x92\x24\x00\xaa\x92\x24\x00\xff\x92\x24\x00\x00\xb6" // |$.U.$...$...$... |
/* 0100 */ "\x24\x00\x55\xb6\x24\x00\xaa\xb6\x24\x00\xff\xb6\x24\x00\x00\xdb" // |$.U.$...$...$... |
/* 0110 */ "\x24\x00\x55\xdb\x24\x00\xaa\xdb\x24\x00\xff\xdb\x24\x00\x00\xff" // |$.U.$...$...$... |
/* 0120 */ "\x24\x00\x55\xff\x24\x00\xaa\xff\x24\x00\xff\xff\x24\x00\x00\x00" // |$.U.$...$...$... |
/* 0130 */ "\x49\x00\x55\x00\x49\x00\xaa\x00\x49\x00\xff\x00\x49\x00\x00\x24" // |I.U.I...I...I..$ |
/* 0140 */ "\x49\x00\x55\x24\x49\x00\xaa\x24\x49\x00\xff\x24\x49\x00\x00\x49" // |I.U$I..$I..$I..I |
/* 0150 */ "\x49\x00\x55\x49\x49\x00\xaa\x49\x49\x00\xff\x49\x49\x00\x00\x6d" // |I.UII..II..II..m |
/* 0160 */ "\x49\x00\x55\x6d\x49\x00\xaa\x6d\x49\x00\xff\x6d\x49\x00\x00\x92" // |I.UmI..mI..mI... |
/* 0170 */ "\x49\x00\x55\x92\x49\x00\xaa\x92\x49\x00\xff\x92\x49\x00\x00\xb6" // |I.U.I...I...I... |
/* 0180 */ "\x49\x00\x55\xb6\x49\x00\xaa\xb6\x49\x00\xff\xb6\x49\x00\x00\xdb" // |I.U.I...I...I... |
/* 0190 */ "\x49\x00\x55\xdb\x49\x00\xaa\xdb\x49\x00\xff\xdb\x49\x00\x00\xff" // |I.U.I...I...I... |
/* 01a0 */ "\x49\x00\x55\xff\x49\x00\xaa\xff\x49\x00\xff\xff\x49\x00\x00\x00" // |I.U.I...I...I... |
/* 01b0 */ "\x6d\x00\x55\x00\x6d\x00\xaa\x00\x6d\x00\xff\x00\x6d\x00\x00\x24" // |m.U.m...m...m..$ |
/* 01c0 */ "\x6d\x00\x55\x24\x6d\x00\xaa\x24\x6d\x00\xff\x24\x6d\x00\x00\x49" // |m.U$m..$m..$m..I |
/* 01d0 */ "\x6d\x00\x55\x49\x6d\x00\xaa\x49\x6d\x00\xff\x49\x6d\x00\x00\x6d" // |m.UIm..Im..Im..m |
/* 01e0 */ "\x6d\x00\x55\x6d\x6d\x00\xaa\x6d\x6d\x00\xff\x6d\x6d\x00\x00\x92" // |m.Umm..mm..mm... |
/* 01f0 */ "\x6d\x00\x55\x92\x6d\x00\xaa\x92\x6d\x00\xff\x92\x6d\x00\x00\xb6" // |m.U.m...m...m... |
/* 0200 */ "\x6d\x00\x55\xb6\x6d\x00\xaa\xb6\x6d\x00\xff\xb6\x6d\x00\x00\xdb" // |m.U.m...m...m... |
/* 0210 */ "\x6d\x00\x55\xdb\x6d\x00\xaa\xdb\x6d\x00\xff\xdb\x6d\x00\x00\xff" // |m.U.m...m...m... |
/* 0220 */ "\x6d\x00\x55\xff\x6d\x00\xaa\xff\x6d\x00\xff\xff\x6d\x00\x00\x00" // |m.U.m...m...m... |
/* 0230 */ "\x92\x00\x55\x00\x92\x00\xaa\x00\x92\x00\xff\x00\x92\x00\x00\x24" // |..U............$ |
/* 0240 */ "\x92\x00\x55\x24\x92\x00\xaa\x24\x92\x00\xff\x24\x92\x00\x00\x49" // |..U$...$...$...I |
/* 0250 */ "\x92\x00\x55\x49\x92\x00\xaa\x49\x92\x00\xff\x49\x92\x00\x00\x6d" // |..UI...I...I...m |
/* 0260 */ "\x92\x00\x55\x6d\x92\x00\xaa\x6d\x92\x00\xff\x6d\x92\x00\x00\x92" // |..Um...m...m.... |
/* 0270 */ "\x92\x00\x55\x92\x92\x00\xaa\x92\x92\x00\xff\x92\x92\x00\x00\xb6" // |..U............. |
/* 0280 */ "\x92\x00\x55\xb6\x92\x00\xaa\xb6\x92\x00\xff\xb6\x92\x00\x00\xdb" // |..U............. |
/* 0290 */ "\x92\x00\x55\xdb\x92\x00\xaa\xdb\x92\x00\xff\xdb\x92\x00\x00\xff" // |..U............. |
/* 02a0 */ "\x92\x00\x55\xff\x92\x00\xaa\xff\x92\x00\xff\xff\x92\x00\x00\x00" // |..U............. |
/* 02b0 */ "\xb6\x00\x55\x00\xb6\x00\xaa\x00\xb6\x00\xff\x00\xb6\x00\x00\x24" // |..U............$ |
/* 02c0 */ "\xb6\x00\x55\x24\xb6\x00\xaa\x24\xb6\x00\xff\x24\xb6\x00\x00\x49" // |..U$...$...$...I |
/* 02d0 */ "\xb6\x00\x55\x49\xb6\x00\xaa\x49\xb6\x00\xff\x49\xb6\x00\x00\x6d" // |..UI...I...I...m |
/* 02e0 */ "\xb6\x00\x55\x6d\xb6\x00\xaa\x6d\xb6\x00\xff\x6d\xb6\x00\x00\x92" // |..Um...m...m.... |
/* 02f0 */ "\xb6\x00\x55\x92\xb6\x00\xaa\x92\xb6\x00\xff\x92\xb6\x00\x00\xb6" // |..U............. |
/* 0300 */ "\xb6\x00\x55\xb6\xb6\x00\xaa\xb6\xb6\x00\xff\xb6\xb6\x00\x00\xdb" // |..U............. |
/* 0310 */ "\xb6\x00\x55\xdb\xb6\x00\xaa\xdb\xb6\x00\xff\xdb\xb6\x00\x00\xff" // |..U............. |
/* 0320 */ "\xb6\x00\x55\xff\xb6\x00\xaa\xff\xb6\x00\xff\xff\xb6\x00\x00\x00" // |..U............. |
/* 0330 */ "\xdb\x00\x55\x00\xdb\x00\xaa\x00\xdb\x00\xff\x00\xdb\x00\x00\x24" // |..U............$ |
/* 0340 */ "\xdb\x00\x55\x24\xdb\x00\xaa\x24\xdb\x00\xff\x24\xdb\x00\x00\x49" // |..U$...$...$...I |
/* 0350 */ "\xdb\x00\x55\x49\xdb\x00\xaa\x49\xdb\x00\xff\x49\xdb\x00\x00\x6d" // |..UI...I...I...m |
/* 0360 */ "\xdb\x00\x55\x6d\xdb\x00\xaa\x6d\xdb\x00\xff\x6d\xdb\x00\x00\x92" // |..Um...m...m.... |
/* 0370 */ "\xdb\x00\x55\x92\xdb\x00\xaa\x92\xdb\x00\xff\x92\xdb\x00\x00\xb6" // |..U............. |
/* 0380 */ "\xdb\x00\x55\xb6\xdb\x00\xaa\xb6\xdb\x00\xff\xb6\xdb\x00\x00\xdb" // |..U............. |
/* 0390 */ "\xdb\x00\x55\xdb\xdb\x00\xaa\xdb\xdb\x00\xff\xdb\xdb\x00\x00\xff" // |..U............. |
/* 03a0 */ "\xdb\x00\x55\xff\xdb\x00\xaa\xff\xdb\x00\xff\xff\xdb\x00\x00\x00" // |..U............. |
/* 03b0 */ "\xff\x00\x55\x00\xff\x00\xaa\x00\xff\x00\xff\x00\xff\x00\x00\x24" // |..U............$ |
/* 03c0 */ "\xff\x00\x55\x24\xff\x00\xaa\x24\xff\x00\xff\x24\xff\x00\x00\x49" // |..U$...$...$...I |
/* 03d0 */ "\xff\x00\x55\x49\xff\x00\xaa\x49\xff\x00\xff\x49\xff\x00\x00\x6d" // |..UI...I...I...m |
/* 03e0 */ "\xff\x00\x55\x6d\xff\x00\xaa\x6d\xff\x00\xff\x6d\xff\x00\x00\x92" // |..Um...m...m.... |
/* 03f0 */ "\xff\x00\x55\x92\xff\x00\xaa\x92\xff\x00\xff\x92\xff\x00\x00\xb6" // |..U............. |
/* 0400 */ "\xff\x00\x55\xb6\xff\x00\xaa\xb6\xff\x00\xff\xb6\xff\x00\x00\xdb" // |..U............. |
/* 0410 */ "\xff\x00\x55\xdb\xff\x00\xaa\xdb\xff\x00\xff\xdb\xff\x00\x00\xff" // |..U............. |
/* 0420 */ "\xff\x00\x55\xff\xff\x00\xaa\xff\xff\x00\xff\xff\xff\x00\x09\x0a" // |..U............. |
/* 0430 */ "\x3f\x10\x00\x3f\x00\xc5\x00\x12\x00\xff\xff\x01\x3f\x12\x00\x40" // |?..?........?..@ |
/* 0440 */ "\x00\x01\x00\x0f\x00\x51\x8c\x01\x3f\x0f\x00\x3e\x00\xc6\x00\x01" // |.....Q..?..>.... |
/* 0450 */ "\x00\x73\x02\x01\x0e\x3f\x00\x01\x00\x12\x00\x01\x0b\xd5\x00\x3e" // |.s...?.........> |
/* 0460 */ "\x00\x14\x00\x01\x0f\x0f\x00\x51\x00\xc7\x00\x01\x00"             //.......Q..... |
// Dump done on RDP Wab Target (4) 1133 bytes |
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
/* 0000 */ "\x00\x0a\x8c"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 2696 bytes |
// Recv done on RDP Wab Target (4) 2696 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x8a\x7d\x7d\x0a\x17\x00\x09" // |...h. ..p.}}.... |
/* 0010 */ "\x04\x02\x00\x01\x00\x00\x02\x7d\x0a\x02\x00\x00\x00\x00\x00\x00" // |.......}........ |
/* 0020 */ "\x00\x95\x00\x00\x00\x01\x3f\x00\x00\x00\x00\x00\x04\x0a\x00\xec" // |......?......... |
/* 0030 */ "\x08\x01\x0e\x0a\x00\x14\x00\x10\x00\x01\x05\x7f\x00\x3d\x03\x01" // |.............=.. |
/* 0040 */ "\x05\xec\x03\x14\x00\x01\x0f\x00\x00\x1a\x00\xbc\x03\x07\x00\x01" // |................ |
/* 0050 */ "\x05\xec\x03\x14\x00\x01\x0f\x00\x00\x21\x00\x00\x04\x08\x00\x01" // |.........!...... |
/* 0060 */ "\x0e\x29\x00\x0f\x00\x10\x00\x01\x01\xf1\x03\x01\x0f\x00\x00\x39" // |.).............9 |
/* 0070 */ "\x00\x00\x04\x05\x00\x01\x0e\x3e\x00\x0f\x00\x14\x00\x01\x05\xd6" // |.......>........ |
/* 0080 */ "\x00\x05\x00\x01\x01\x98\x03\x11\x05\x59\x0a\x01\x0f\x00\x00\x52" // |.........Y.....R |
/* 0090 */ "\x00\x00\x04\x6c\x02\x01\x0e\xbe\x02\x13\x03\x06\x00\x01\x0e\xc4" // |...l............ |
/* 00a0 */ "\x02\x3c\x00\x11\x00\x01\x05\x6a\x00\xa9\x02\x01\x0f\x34\x03\xbe" // |.<.....j.....4.. |
/* 00b0 */ "\x02\x0f\x00\x17\x00\x11\x0d\x29\x36\xea\x11\x05\x5f\xca\x11\x05" // |.......)6..._... |
/* 00c0 */ "\x30\x05\x01\x0f\x5d\x03\xbf\x02\x0f\x00\x14\x00\x11\x0d\x28\x0e" // |0...].........(. |
/* 00d0 */ "\xee\x11\x05\x37\xf2\x11\x05\x30\x05\x11\x0f\xa7\x02\xfb\x0e\x11" // |...7...0........ |
/* 00e0 */ "\x01\x29\x11\x05\x30\x05\x11\x0f\x99\x10\x09\xf2\x11\x05\x37\xf2" // |.)..0.........7. |
/* 00f0 */ "\x11\x05\x30\x05\x01\x07\x5d\x03\xd3\x02\x45\x00\x11\x05\x5f\xca" // |..0...]...E..._. |
/* 0100 */ "\x11\x05\x30\x05\x01\x0f\x00\x00\xd5\x02\x3c\x00\x0a\x00\x01\x05" // |..0.......<..... |
/* 0110 */ "\x6a\x00\x96\x03\x01\x0f\x00\x00\xdf\x02\x3c\x00\x03\x00\x01\x05" // |j.........<..... |
/* 0120 */ "\x6a\x00\xc0\x02\x11\x0f\x96\x03\x6a\x11\x01\x0f\x69\x03\xdf\x02" // |j.......j...i... |
/* 0130 */ "\x28\x00\x17\x00\x11\x05\x72\xfd\x01\x0f\x00\x00\xf6\x02\x00\x04" // |(.....r......... |
/* 0140 */ "\x0a\x00\x01\x0f\x14\x00\x0a\x00\x6b\x00\x10\x00\x03\x25\x00\x08" // |........k....%.. |
/* 0150 */ "\x00\x03\x07\x01\x00\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00" // |................ |
/* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x41\x00\x22\x00\x14\x00\x08\x00" // |........A."..... |
/* 0170 */ "\x08\x00\x14\x00\x22\x00\x41\x00\x00\x00\x00\x00\x00\x00\x03\x25" // |....".A........% |
/* 0180 */ "\x00\x08\x00\x03\x07\x01\x01\x00\x01\x00\xf0\xff\x10\x00\x10\x00" // |................ |
/* 0190 */ "\x00\x00\x00\x00\x00\x00\x07\xc0\x18\x20\x20\x10\x27\x48\x4c\xc8" // |.........  .'HL. |
/* 01a0 */ "\x48\x48\x48\x48\x4c\xd0\x27\x60\x20\x00\x18\x20\x07\xc0\x00\x00" // |HHHHL.'` .. .... |
/* 01b0 */ "\x03\x25\x00\x08\x00\x03\x07\x01\x02\x00\x02\x00\xf0\xff\x10\x00" // |.%.............. |
/* 01c0 */ "\x10\x00\x00\x00\x00\x00\x00\x00\x18\x00\x28\x00\x08\x00\x08\x00" // |..........(..... |
/* 01d0 */ "\x08\x00\x08\x00\x08\x00\x08\x00\x08\x00\x3e\x00\x00\x00\x00\x00" // |..........>..... |
/* 01e0 */ "\x00\x00\x03\x25\x00\x08\x00\x03\x07\x01\x03\x00\x01\x00\xf0\xff" // |...%............ |
/* 01f0 */ "\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x1c\x00\x22\x00\x41\x00" // |............".A. |
/* 0200 */ "\x41\x00\x41\x00\x41\x00\x41\x00\x41\x00\x22\x00\x1c\x00\x00\x00" // |A.A.A.A.A."..... |
/* 0210 */ "\x00\x00\x00\x00\x03\x15\x00\x08\x00\x03\x07\x01\x04\x00\x02\x00" // |................ |
/* 0220 */ "\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0230 */ "\x00\x20\x20\x00\x00\x00\x03\x25\x00\x08\x00\x03\x07\x01\x05\x00" // |.  ....%........ |
/* 0240 */ "\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x06\x00" // |................ |
/* 0250 */ "\x0a\x00\x12\x00\x12\x00\x22\x00\x42\x00\x7f\x00\x02\x00\x02\x00" // |......".B....... |
/* 0260 */ "\x02\x00\x00\x00\x00\x00\x00\x00\x03\x25\x00\x08\x00\x03\x07\x01" // |.........%...... |
/* 0270 */ "\x06\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0280 */ "\x3e\x00\x43\x00\x01\x00\x01\x00\x1e\x00\x03\x00\x01\x00\x01\x00" // |>.C............. |
/* 0290 */ "\x43\x00\x3c\x00\x00\x00\x00\x00\x00\x00\x0d\x1b\xfb\x3f\x3f\xf0" // |C.<..........??. |
/* 02a0 */ "\x14\x0a\x7e\x19\x07\x03\x01\xff\xff\x00\xec\x08\x00\x14\x00\x0a" // |..~............. |
/* 02b0 */ "\x00\x7f\x00\x1a\x00\x14\x00\x0a\x00\x7f\x00\x1a\x00\x03\xaa\xaa" // |................ |
/* 02c0 */ "\x55\xaa\x55\xaa\x55\xaa\x13\x00\x1a\x00\x1a\x00\x00\x01\x09\x02" // |U.U.U........... |
/* 02d0 */ "\x0e\x03\x09\x04\x09\x02\x04\x03\x09\x04\x09\x05\x04\x06\x09\x04" // |................ |
/* 02e0 */ "\x09\x06\x04\x06\x09\x19\x0a\x37\xfb\x1f\x61\x98\x43\x03\x25\x00" // |.......7..a.C.%. |
/* 02f0 */ "\x08\x00\x03\x07\x01\x07\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00" // |................ |
/* 0300 */ "\x00\x00\x00\x00\x00\xff\x80\x08\x00\x08\x00\x08\x00\x08\x00\x08" // |................ |
/* 0310 */ "\x00\x08\x00\x08\x00\x08\x00\x08\x00\x00\x00\x00\x00\x00\x00\x03" // |................ |
/* 0320 */ "\x15\x00\x08\x00\x03\x07\x01\x08\x00\x01\x00\xf0\xff\x08\x00\x10" // |................ |
/* 0330 */ "\x00\x00\x00\x00\x00\x00\x3c\x42\x02\x3e\x42\x42\x46\x3a\x00\x00" // |......<B.>BBF:.. |
/* 0340 */ "\x00\x03\x15\x00\x08\x00\x03\x07\x01\x09\x00\x01\x00\xf0\xff\x08" // |................ |
/* 0350 */ "\x00\x10\x00\x00\x00\x00\x00\x00\x58\x60\x40\x40\x40\x40\x40\x40" // |........X`@@@@@@ |
/* 0360 */ "\x00\x00\x00\x03\x25\x00\x08\x00\x03\x07\x01\x0a\x00\x01\x00\xf0" // |....%........... |
/* 0370 */ "\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1d" // |................ |
/* 0380 */ "\x00\x23\x00\x41\x00\x41\x00\x41\x00\x41\x00\x23\x00\x1d\x00\x01" // |.#.A.A.A.A.#.... |
/* 0390 */ "\x00\x23\x00\x1e\x00\x03\x25\x00\x08\x00\x03\x07\x01\x0b\x00\x01" // |.#....%......... |
/* 03a0 */ "\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 03b0 */ "\x00\x1e\x00\x23\x00\x41\x00\x7f\x00\x40\x00\x40\x00\x21\x00\x1e" // |...#.A...@.@.!.. |
/* 03c0 */ "\x00\x00\x00\x00\x00\x00\x00\x03\x15\x00\x08\x00\x03\x07\x01\x0c" // |................ |
/* 03d0 */ "\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x40\x40\xf8\x40" // |............@@.@ |
/* 03e0 */ "\x40\x40\x40\x40\x40\x38\x00\x00\x00\x03\x15\x00\x08\x00\x03\x07" // |@@@@@8.......... |
/* 03f0 */ "\x01\x0d\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x00\x00" // |................ |
/* 0400 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x25\x00\x08\x00" // |............%... |
/* 0410 */ "\x03\x07\x01\x0e\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00" // |................ |
/* 0420 */ "\x00\x00\x00\x0f\x80\x30\x40\x20\x00\x40\x00\x40\x00\x41\xc0\x40" // |.....0@ .@.@.A.@ |
/* 0430 */ "\x40\x20\x40\x30\x40\x0f\x80\x00\x00\x00\x00\x00\x00\x03\x25\x00" // |@ @0@.........%. |
/* 0440 */ "\x08\x00\x03\x07\x01\x0f\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00" // |................ |
/* 0450 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1c\x00\x22\x00\x41\x00\x41" // |...........".A.A |
/* 0460 */ "\x00\x41\x00\x41\x00\x22\x00\x1c\x00\x00\x00\x00\x00\x00\x00\x03" // |.A.A.".......... |
/* 0470 */ "\x25\x00\x08\x00\x03\x07\x01\x10\x00\x01\x00\xf0\xff\x10\x00\x10" // |%............... |
/* 0480 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x41\x00\x41\x00\x41" // |...........A.A.A |
/* 0490 */ "\x00\x41\x00\x41\x00\x41\x00\x63\x00\x3d\x00\x00\x00\x00\x00\x00" // |.A.A.A.c.=...... |
/* 04a0 */ "\x00\x03\x25\x00\x08\x00\x03\x07\x01\x11\x00\x01\x00\xf0\xff\x10" // |..%............. |
/* 04b0 */ "\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x5c\x00\x62" // |...............b |
/* 04c0 */ "\x00\x41\x00\x41\x00\x41\x00\x41\x00\x62\x00\x5c\x00\x40\x00\x40" // |.A.A.A.A.b...@.@ |
/* 04d0 */ "\x00\x40\x00\x0d\x1b\xa0\x3b\x30\xf0\xfb\x1f\x5c\x1f\x98\x43\x00" // |.@....;0......C. |
/* 04e0 */ "\x29\x00\x70\x00\x39\x00\x29\x00\x70\x00\x39\x00\x39\x00\x18\x07" // |).p.9.).p.9.9... |
/* 04f0 */ "\x00\x08\x09\x09\x08\x0a\x05\x0b\x09\x0c\x09\x0d\x05\x0e\x04\x09" // |................ |
/* 0500 */ "\x0b\x0f\x05\x10\x09\x11\x09\x09\x0a\x05\xdb\x00\xc2\x02\x0d\x1b" // |................ |
/* 0510 */ "\x40\x15\x28\x05\xdb\x00\x9c\x03\xe0\x00\x0d\x01\xe0\x00\x0d\x01" // |@.(............. |
/* 0520 */ "\xdf\x00\x0c\x07\x00\x08\x09\x09\x08\x0a\x05\x0b\x09\x0c\x09\x09" // |................ |
/* 0530 */ "\x0a\x05\x9d\x03\x54\x00\x03\x25\x00\x08\x00\x03\x07\x01\x12\x00" // |....T..%........ |
/* 0540 */ "\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x7e\x00" // |..............~. |
/* 0550 */ "\x43\x00\x41\x00\x41\x00\x43\x00\x7e\x00\x40\x00\x40\x00\x40\x00" // |C.A.A.C.~.@.@.@. |
/* 0560 */ "\x40\x00\x00\x00\x00\x00\x00\x00\x03\x15\x00\x08\x00\x03\x07\x01" // |@............... |
/* 0570 */ "\x13\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x00\x00\x1c" // |................ |
/* 0580 */ "\x22\x40\x40\x40\x40\x22\x1c\x00\x00\x00\x03\x15\x00\x08\x00\x03" // |"@@@@".......... |
/* 0590 */ "\x07\x01\x14\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x40\x40" // |..............@@ |
/* 05a0 */ "\x40\x40\x40\x40\x40\x40\x40\x40\x40\x00\x00\x00\x0d\x1b\x40\x15" // |@@@@@@@@@.....@. |
/* 05b0 */ "\x28\x41\x9d\x03\x54\xa2\x03\xdb\x03\xa2\x03\xdb\x03\xa1\x03\x10" // |(A..T........... |
/* 05c0 */ "\x12\x00\x09\x09\x0f\x05\x0c\x09\x0f\x05\x13\x09\x0f\x08\x14\x09" // |................ |
/* 05d0 */ "\x09\x0a\x3f\x10\x00\x3f\x00\xc5\x00\x12\x00\xff\xff\x01\x3f\x12" // |..?..?........?. |
/* 05e0 */ "\x00\x40\x00\x01\x00\x0f\x00\x51\x8c\x01\x3f\x0f\x00\x3e\x00\xc6" // |.@.....Q..?..>.. |
/* 05f0 */ "\x00\x01\x00\x73\x02\x01\x0e\x3f\x00\x01\x00\x12\x00\x01\x0b\xd5" // |...s...?........ |
/* 0600 */ "\x00\x3e\x00\x14\x00\x01\x0f\x0f\x00\x51\x00\xc7\x00\x01\x00\x01" // |.>.......Q...... |
/* 0610 */ "\x3f\xdc\x00\x3f\x00\xbb\x02\x12\x00\xff\xff\x11\x0f\xff\xff\x01" // |?..?............ |
/* 0620 */ "\xef\x01\x0e\x3f\x00\x01\x00\x12\x00\x01\x0b\x97\x03\x3e\x00\x14" // |...?.........>.. |
/* 0630 */ "\x00\x01\x0f\xdb\x00\x51\x00\xbd\x02\x01\x00\x01\x0f\x9e\x03\x3f" // |.....Q.........? |
/* 0640 */ "\x00\x52\x00\x12\x00\x11\x0f\xff\xff\x01\xef\x11\x0e\x01\xae\x11" // |.R.............. |
/* 0650 */ "\x11\x0b\x53\xff\x02\x11\x0f\xad\x13\x53\xed\x11\x3f\x1f\xb9\xdc" // |..S......S..?... |
/* 0660 */ "\x16\xec\x08\x11\x0f\x01\x01\xfd\xfd\x03\x15\x00\x08\x00\x03\x07" // |................ |
/* 0670 */ "\x01\x15\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x7e\x40" // |..............~@ |
/* 0680 */ "\x40\x40\x7e\x40\x40\x40\x40\x40\x00\x00\x00\x03\x15\x00\x08\x00" // |@@~@@@@@........ |
/* 0690 */ "\x03\x07\x01\x16\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x40" // |...............@ |
/* 06a0 */ "\x40\x00\x40\x40\x40\x40\x40\x40\x40\x40\x00\x00\x00\x09\x1b\xe0" // |@.@@@@@@@@...... |
/* 06b0 */ "\x3f\x38\xec\x08\x00\xc3\x03\x0d\x00\xe4\x03\x1d\x00\xc3\x03\x0d" // |?8.............. |
/* 06c0 */ "\x00\xe4\x03\x1d\x00\xc2\x03\x1d\x00\x0c\x15\x00\x16\x08\x14\x03" // |................ |
/* 06d0 */ "\x0c\x03\x0b\x05\x09\x09\x19\x0a\x3f\xff\xff\x01\xee\xff\xff\x11" // |........?....... |
/* 06e0 */ "\x0e\x02\xd4\x13\x11\x0b\x2e\xfe\x02\x11\x0f\xd2\x15\x2e\xeb\x01" // |................ |
/* 06f0 */ "\x3f\x13\x03\xbe\x02\x21\x00\x17\x00\xec\x08\x11\x0f\x01\x01\xfd" // |?....!.......... |
/* 0700 */ "\xfd\x03\x25\x00\x08\x00\x03\x07\x01\x17\x00\x01\x00\xf0\xff\x10" // |..%............. |
/* 0710 */ "\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\x00\xe0\x03" // |............ ... |
/* 0720 */ "\xe0\x0f\xe0\x3f\xe0\xff\xe0\x3f\xe0\x0f\xe0\x03\xe0\x00\xe0\x00" // |...?...?........ |
/* 0730 */ "\x20\x00\x00\x03\x15\x00\x08\x00\x03\x07\x01\x18\x00\x01\x00\xf0" // | ............... |
/* 0740 */ "\xff\x08\x00\x10\x00\x00\x00\x00\x00\x00\x00\x02\x0e\x3e\xfe\x3e" // |.............>.> |
/* 0750 */ "\x0e\x02\x00\x00\x00\x09\x1b\xc0\x3f\x38\x1a\x03\xc1\x02\x2c\x03" // |........?8....,. |
/* 0760 */ "\xd1\x02\x1a\x03\xc1\x02\x2c\x03\xd1\x02\x19\x03\xd1\x02\x04\x17" // |......,......... |
/* 0770 */ "\x00\x18\x0b\x19\x0a\x0f\x2f\xff\xfc\x03\x11\x0f\x01\x01\xfd\xfd" // |....../......... |
/* 0780 */ "\x09\x1b\x40\x15\x28\x4a\x03\x55\x03\x4a\x03\x55\x03\x49\x03\x02" // |..@.(J.U.J.U.I.. |
/* 0790 */ "\x17\x00\x19\x0a\x3b\x29\x01\xfe\xff\xff\x09\x1b\x70\x15\x28\x00" // |....;)......p.(. |
/* 07a0 */ "\x00\x00\xff\xff\x00\x6e\x03\x77\x03\x6e\x03\x77\x03\x6c\x03\x02" // |.....n.w.n.w.l.. |
/* 07b0 */ "\x02\x00\x19\x0a\x0f\xff\xff\x01\xef\x11\x0e\x01\xe9\x11\x11\x0b" // |................ |
/* 07c0 */ "\x18\xff\x02\x11\x0f\xe8\x13\x18\xed\x11\x3f\x19\xef\xf5\x0f\xec" // |..........?..... |
/* 07d0 */ "\x08\x03\x15\x00\x08\x00\x03\x07\x01\x19\x00\x01\x00\xf0\xff\x08" // |................ |
/* 07e0 */ "\x00\x10\x00\x00\x00\x00\x08\x18\x10\x10\x30\x20\x20\x60\x40\x40" // |..........0  `@@ |
/* 07f0 */ "\xc0\x80\x00\x0d\x1b\x70\x15\x28\x5a\xe8\xc1\x02\xa2\xd0\x02\xff" // |.....p.(Z....... |
/* 0800 */ "\xff\x00\xec\x08\x00\x85\x03\x93\x03\x85\x03\x93\x03\x84\x03\x04" // |................ |
/* 0810 */ "\x19\x00\x03\x05\x19\x0a\x0f\x1d\xfd\x0c\x07\x11\x0f\x01\x01\xfd" // |................ |
/* 0820 */ "\xfd\x03\x25\x00\x08\x00\x03\x07\x01\x1a\x00\x01\x00\xf0\xff\x10" // |..%............. |
/* 0830 */ "\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\xe0\x00\xf8" // |................ |
/* 0840 */ "\x00\xfe\x00\xff\x80\xff\xc0\xff\x00\xfc\x00\xf0\x00\xc0\x00\x00" // |................ |
/* 0850 */ "\x00\x00\x00\x09\x1b\x40\x15\x28\xa9\x03\xb4\x03\xa9\x03\xb4\x03" // |.....@.(........ |
/* 0860 */ "\xa8\x03\x02\x1a\x00\x19\x0a\x0f\x28\xff\x0a\x03\x11\x0f\x01\x01" // |........(....... |
/* 0870 */ "\xfd\xfd\x03\x15\x00\x08\x00\x03\x07\x01\x1b\x00\x01\x00\xf0\xff" // |................ |
/* 0880 */ "\x08\x00\x10\x00\x00\x00\x00\x00\x00\x00\x80\xe0\xf8\xfe\xf8\xe0" // |................ |
/* 0890 */ "\x80\x00\x00\x00\x09\x1b\x40\x15\x28\xd2\x03\xe4\x03\xd2\x03\xe4" // |......@.(....... |
/* 08a0 */ "\x03\xd1\x03\x04\x1b\x00\x1a\x07\x09\x0a\x0f\x2a\x03\xdf\x02\x3f" // |...........*...? |
/* 08b0 */ "\x00\x17\x00\x11\x0f\x01\x01\xfd\xfd\x03\x15\x00\x08\x00\x03\x07" // |................ |
/* 08c0 */ "\x01\x1c\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x40\x40" // |..............@@ |
/* 08d0 */ "\x40\x40\x40\x40\x40\x40\x40\x7e\x00\x00\x00\x09\x1b\xc0\x3f\x38" // |@@@@@@@~......?8 |
/* 08e0 */ "\x31\x03\xe2\x02\x61\x03\xf2\x02\x31\x03\xe2\x02\x61\x03\xf2\x02" // |1...a...1...a... |
/* 08f0 */ "\x30\x03\xf2\x02\x0c\x1c\x00\x0f\x07\x0a\x09\x0f\x09\x10\x09\x0c" // |0............... |
/* 0900 */ "\x09\x19\x0a\x3f\xff\xff\x01\xee\xff\xff\x11\x0e\x02\xc5\x13\x11" // |...?............ |
/* 0910 */ "\x0b\x3d\xfe\x02\x11\x0f\xc3\x15\x3d\xeb\x11\x3f\x67\xeb\x0b\x15" // |.=......=..?g... |
/* 0920 */ "\xec\x08\x11\x0f\x01\x01\xfd\xfd\x03\x25\x00\x08\x00\x03\x07\x01" // |.........%...... |
/* 0930 */ "\x1d\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0940 */ "\x1f\x00\x30\x80\x60\x00\x40\x00\x40\x00\x40\x00\x40\x00\x60\x00" // |..0.`.@.@.@.@.`. |
/* 0950 */ "\x30\x80\x1f\x00\x00\x00\x00\x00\x00\x00\x03\x25\x00\x08\x00\x03" // |0..........%.... |
/* 0960 */ "\x07\x01\x1e\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00" // |................ |
/* 0970 */ "\x00\x00\x00\x00\x00\x00\x5e\x00\x63\x00\x41\x00\x41\x00\x41\x00" // |......^.c.A.A.A. |
/* 0980 */ "\x41\x00\x41\x00\x41\x00\x00\x00\x00\x00\x00\x00\x09\x1b\x40\x15" // |A.A.A.........@. |
/* 0990 */ "\x28\x98\x03\xd3\x03\x98\x03\xd3\x03\x97\x03\x0e\x1d\x00\x0f\x0a" // |(............... |
/* 09a0 */ "\x1e\x09\x1e\x09\x0b\x09\x13\x09\x0c\x08\x19\x0a\x3f\xff\xff\x01" // |............?... |
/* 09b0 */ "\xee\xff\xff\x11\x0e\x02\xba\x13\x11\x0b\x48\xfe\x02\x11\x0f\xb8" // |..........H..... |
/* 09c0 */ "\x15\x48\xeb\x01\x3f\x3c\x00\xc4\x02\x2e\x00\x1e\x00\xec\x08\x11" // |.H..?<.......... |
/* 09d0 */ "\x0f\x01\x01\xfd\xfd\x03\x15\x00\x08\x00\x03\x07\x01\x1f\x00\x01" // |................ |
/* 09e0 */ "\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x30\x40\x40\xf0\x40\x40\x40" // |.........0@@.@@@ |
/* 09f0 */ "\x40\x40\x40\x40\x00\x00\x00\x03\x15\x00\x08\x00\x03\x07\x01\x20" // |@@@@...........  |
/* 0a00 */ "\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00" // |................ |
/* 0a10 */ "\x00\x00\x78\x00\x00\x00\x00\x00\x00\x03\x25\x00\x08\x00\x03\x07" // |..x.......%..... |
/* 0a20 */ "\x01\x21\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00" // |.!.............. |
/* 0a30 */ "\x00\x7e\x00\x43\x00\x41\x00\x41\x00\x43\x00\x7e\x00\x43\x00\x41" // |.~.C.A.A.C.~.C.A |
/* 0a40 */ "\x00\x41\x00\x40\x80\x00\x00\x00\x00\x00\x00\x09\x1b\xc0\x3f\x38" // |.A.@..........?8 |
/* 0a50 */ "\x44\x00\xcc\x02\x64\x00\xdc\x02\x44\x00\xcc\x02\x64\x00\xdc\x02" // |D...d...D...d... |
/* 0a60 */ "\x43\x00\xdc\x02\x0a\x1f\x00\x09\x04\x20\x05\x15\x05\x21\x08\x19" // |C........ ...!.. |
/* 0a70 */ "\x0a\x3f\xff\xff\x01\xe7\xff\xff\x11\x0e\x02\xd6\x1a\x11\x0b\x2c" // |.?............., |
/* 0a80 */ "\xfe\x02\x11\x0f\xd4\x1c\x2c\xe4"                                 //......,. |
// Dump done on RDP Wab Target (4) 2696 bytes |
// LOOPING on PDUs: 2685 |
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
// order(10):opaquerect(rect(0,702,787,6) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,708,60,17) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,708,681,17) color=0x0008ec) |
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
// order(10):opaquerect(rect(0,725,60,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,725,918,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,735,60,3) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,735,704,3) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,738,810,20) color=0x0008ec) |
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
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,708,46,30) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(61,709,43,27) color=0x0008ec) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(68,716,33,17) op=(68,716,33,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=67 glyph_y=732 data_len=10 [0x1f 0x00 0x09 0x04 0x20 0x05 0x15 0x05 0x21 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,708,44,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,710,2,28) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(104,708,2,30) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,736,46,2) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 3 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x04\xca"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 1222 bytes |
// Recv done on RDP Wab Target (4) 1222 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x84\xbb\xbb\x04\x17\x00\x09" // |...h. ..p....... |
/* 0010 */ "\x04\x02\x00\x01\x00\x00\x02\xbb\x04\x02\x00\x00\x00\x00\x00\x00" // |................ |
/* 0020 */ "\x00\x73\x00\x00\x00\x01\x3f\x00\x00\x00\x00\x00\x04\x0a\x00\xec" // |.s....?......... |
/* 0030 */ "\x08\x01\x0e\x0a\x00\x14\x00\x10\x00\x01\x05\x7f\x00\x3d\x03\x01" // |.............=.. |
/* 0040 */ "\x05\xec\x03\x14\x00\x01\x0f\x00\x00\x1a\x00\xbc\x03\x07\x00\x01" // |................ |
/* 0050 */ "\x05\xec\x03\x14\x00\x01\x0f\x00\x00\x21\x00\x00\x04\x08\x00\x01" // |.........!...... |
/* 0060 */ "\x0e\x29\x00\x0f\x00\x10\x00\x01\x01\xf1\x03\x01\x0f\x00\x00\x39" // |.).............9 |
/* 0070 */ "\x00\x00\x04\x05\x00\x01\x0e\x3e\x00\x0f\x00\x14\x00\x01\x05\xd6" // |.......>........ |
/* 0080 */ "\x00\x05\x00\x01\x01\x98\x03\x11\x05\x59\x0a\x01\x0f\x00\x00\x52" // |.........Y.....R |
/* 0090 */ "\x00\x00\x04\x6c\x02\x01\x0e\xbe\x02\x13\x03\x06\x00\x01\x0e\xc4" // |...l............ |
/* 00a0 */ "\x02\x3c\x00\x11\x00\x01\x05\x6a\x00\xa9\x02\x01\x0f\x34\x03\xbe" // |.<.....j.....4.. |
/* 00b0 */ "\x02\x0f\x00\x17\x00\x11\x0d\x29\x36\xea\x11\x05\x5f\xca\x11\x05" // |.......)6..._... |
/* 00c0 */ "\x30\x05\x01\x0f\x5d\x03\xbf\x02\x0f\x00\x14\x00\x11\x0d\x28\x0e" // |0...].........(. |
/* 00d0 */ "\xee\x11\x05\x37\xf2\x11\x05\x30\x05\x11\x0f\xa7\x02\xfb\x0e\x11" // |...7...0........ |
/* 00e0 */ "\x01\x29\x11\x05\x30\x05\x11\x0f\x99\x10\x09\xf2\x11\x05\x37\xf2" // |.)..0.........7. |
/* 00f0 */ "\x11\x05\x30\x05\x01\x07\x5d\x03\xd3\x02\x45\x00\x11\x05\x5f\xca" // |..0...]...E..._. |
/* 0100 */ "\x11\x05\x30\x05\x01\x0f\x00\x00\xd5\x02\x3c\x00\x0a\x00\x01\x05" // |..0.......<..... |
/* 0110 */ "\x6a\x00\x96\x03\x01\x0f\x00\x00\xdf\x02\x3c\x00\x03\x00\x01\x05" // |j.........<..... |
/* 0120 */ "\x6a\x00\xc0\x02\x11\x0f\x96\x03\x6a\x11\x01\x0f\x69\x03\xdf\x02" // |j.......j...i... |
/* 0130 */ "\x28\x00\x17\x00\x11\x05\x72\xfd\x01\x0f\x00\x00\xf6\x02\x00\x04" // |(.....r......... |
/* 0140 */ "\x0a\x00\x01\x0f\x14\x00\x0a\x00\x6b\x00\x10\x00\x0d\x1b\xc0\x3f" // |........k......? |
/* 0150 */ "\x38\x0f\x14\x00\x0a\x00\x7e\x00\x19\x00\x14\x00\x0a\x00\x7f\x00" // |8.....~......... |
/* 0160 */ "\x1a\x00\x14\x00\x0a\x00\x7f\x00\x1a\x00\x13\x00\x1a\x00\x1a\x00" // |................ |
/* 0170 */ "\x00\x01\x09\x02\x0e\x03\x09\x04\x09\x02\x04\x03\x09\x04\x09\x05" // |................ |
/* 0180 */ "\x04\x06\x09\x04\x09\x06\x04\x06\x09\x19\x0a\x37\xfb\x1f\x61\x98" // |...........7..a. |
/* 0190 */ "\x43\x0d\x1b\xa0\x3b\x30\xf0\xfb\x1f\x5c\x1f\x98\x43\x00\x29\x00" // |C...;0......C.). |
/* 01a0 */ "\x70\x00\x39\x00\x29\x00\x70\x00\x39\x00\x39\x00\x18\x07\x00\x08" // |p.9.).p.9.9..... |
/* 01b0 */ "\x09\x09\x08\x0a\x05\x0b\x09\x0c\x09\x0d\x05\x0e\x04\x09\x0b\x0f" // |................ |
/* 01c0 */ "\x05\x10\x09\x11\x09\x09\x0a\x05\xdb\x00\xc2\x02\x0d\x1b\x40\x15" // |..............@. |
/* 01d0 */ "\x28\x05\xdb\x00\x9c\x03\xe0\x00\x0d\x01\xe0\x00\x0d\x01\xdf\x00" // |(............... |
/* 01e0 */ "\x0c\x07\x00\x08\x09\x09\x08\x0a\x05\x0b\x09\x0c\x09\x09\x0a\x05" // |................ |
/* 01f0 */ "\x9d\x03\x54\x00\x0d\x1b\x40\x15\x28\x41\x9d\x03\x54\xa2\x03\xdb" // |..T...@.(A..T... |
/* 0200 */ "\x03\xa2\x03\xdb\x03\xa1\x03\x10\x12\x00\x09\x09\x0f\x05\x0c\x09" // |................ |
/* 0210 */ "\x0f\x05\x13\x09\x0f\x08\x14\x09\x09\x0a\x3f\x10\x00\x3f\x00\xc5" // |..........?..?.. |
/* 0220 */ "\x00\x12\x00\xff\xff\x01\x3f\x12\x00\x40\x00\x01\x00\x0f\x00\x51" // |......?..@.....Q |
/* 0230 */ "\x8c\x01\x3f\x0f\x00\x3e\x00\xc6\x00\x01\x00\x73\x02\x01\x0e\x3f" // |..?..>.....s...? |
/* 0240 */ "\x00\x01\x00\x12\x00\x01\x0b\xd5\x00\x3e\x00\x14\x00\x01\x0f\x0f" // |.........>...... |
/* 0250 */ "\x00\x51\x00\xc7\x00\x01\x00\x01\x3f\xdc\x00\x3f\x00\xbb\x02\x12" // |.Q......?..?.... |
/* 0260 */ "\x00\xff\xff\x11\x0f\xff\xff\x01\xef\x01\x0e\x3f\x00\x01\x00\x12" // |...........?.... |
/* 0270 */ "\x00\x01\x0b\x97\x03\x3e\x00\x14\x00\x01\x0f\xdb\x00\x51\x00\xbd" // |.....>.......Q.. |
/* 0280 */ "\x02\x01\x00\x01\x0f\x9e\x03\x3f\x00\x52\x00\x12\x00\x11\x0f\xff" // |.......?.R...... |
/* 0290 */ "\xff\x01\xef\x11\x0e\x01\xae\x11\x11\x0b\x53\xff\x02\x11\x0f\xad" // |..........S..... |
/* 02a0 */ "\x13\x53\xed\x11\x3f\x1f\xb9\xdc\x16\xec\x08\x11\x0f\x01\x01\xfd" // |.S..?........... |
/* 02b0 */ "\xfd\x09\x1b\xe0\x3f\x38\xec\x08\x00\xc3\x03\x0d\x00\xe4\x03\x1d" // |....?8.......... |
/* 02c0 */ "\x00\xc3\x03\x0d\x00\xe4\x03\x1d\x00\xc2\x03\x1d\x00\x0c\x15\x00" // |................ |
/* 02d0 */ "\x16\x08\x14\x03\x0c\x03\x0b\x05\x09\x09\x19\x0a\x3f\xff\xff\x01" // |............?... |
/* 02e0 */ "\xee\xff\xff\x11\x0e\x02\xd4\x13\x11\x0b\x2e\xfe\x02\x11\x0f\xd2" // |................ |
/* 02f0 */ "\x15\x2e\xeb\x01\x3f\x13\x03\xbe\x02\x21\x00\x17\x00\xec\x08\x11" // |....?....!...... |
/* 0300 */ "\x0f\x01\x01\xfd\xfd\x09\x1b\xc0\x3f\x38\x1a\x03\xc1\x02\x2c\x03" // |........?8....,. |
/* 0310 */ "\xd1\x02\x1a\x03\xc1\x02\x2c\x03\xd1\x02\x19\x03\xd1\x02\x04\x17" // |......,......... |
/* 0320 */ "\x00\x18\x0b\x19\x0a\x0f\x2f\xff\xfc\x03\x11\x0f\x01\x01\xfd\xfd" // |....../......... |
/* 0330 */ "\x09\x1b\x40\x15\x28\x4a\x03\x55\x03\x4a\x03\x55\x03\x49\x03\x02" // |..@.(J.U.J.U.I.. |
/* 0340 */ "\x17\x00\x19\x0a\x3b\x29\x01\xfe\xff\xff\x09\x1b\x70\x15\x28\x00" // |....;)......p.(. |
/* 0350 */ "\x00\x00\xff\xff\x00\x6e\x03\x77\x03\x6e\x03\x77\x03\x6c\x03\x02" // |.....n.w.n.w.l.. |
/* 0360 */ "\x02\x00\x19\x0a\x0f\xff\xff\x01\xef\x11\x0e\x01\xe9\x11\x11\x0b" // |................ |
/* 0370 */ "\x18\xff\x02\x11\x0f\xe8\x13\x18\xed\x11\x3f\x19\xef\xf5\x0f\xec" // |..........?..... |
/* 0380 */ "\x08\x0d\x1b\x70\x15\x28\x5a\xe8\xc1\x02\xa2\xd0\x02\xff\xff\x00" // |...p.(Z......... |
/* 0390 */ "\xec\x08\x00\x85\x03\x93\x03\x85\x03\x93\x03\x84\x03\x04\x19\x00" // |................ |
/* 03a0 */ "\x03\x05\x19\x0a\x0f\x1d\xfd\x0c\x07\x11\x0f\x01\x01\xfd\xfd\x09" // |................ |
/* 03b0 */ "\x1b\x40\x15\x28\xa9\x03\xb4\x03\xa9\x03\xb4\x03\xa8\x03\x02\x1a" // |.@.(............ |
/* 03c0 */ "\x00\x19\x0a\x0f\x28\xff\x0a\x03\x11\x0f\x01\x01\xfd\xfd\x09\x1b" // |....(........... |
/* 03d0 */ "\x40\x15\x28\xd2\x03\xe4\x03\xd2\x03\xe4\x03\xd1\x03\x04\x1b\x00" // |@.(............. |
/* 03e0 */ "\x1a\x07\x09\x0a\x0f\x2a\x03\xdf\x02\x3f\x00\x17\x00\x11\x0f\x01" // |.....*...?...... |
/* 03f0 */ "\x01\xfd\xfd\x09\x1b\xc0\x3f\x38\x31\x03\xe2\x02\x61\x03\xf2\x02" // |......?81...a... |
/* 0400 */ "\x31\x03\xe2\x02\x61\x03\xf2\x02\x30\x03\xf2\x02\x0c\x1c\x00\x0f" // |1...a...0....... |
/* 0410 */ "\x07\x0a\x09\x0f\x09\x10\x09\x0c\x09\x19\x0a\x3f\xff\xff\x01\xee" // |...........?.... |
/* 0420 */ "\xff\xff\x11\x0e\x02\xc5\x13\x11\x0b\x3d\xfe\x02\x11\x0f\xc3\x15" // |.........=...... |
/* 0430 */ "\x3d\xeb\x11\x3f\x67\xeb\x0b\x15\xec\x08\x11\x0f\x01\x01\xfd\xfd" // |=..?g........... |
/* 0440 */ "\x09\x1b\x40\x15\x28\x98\x03\xd3\x03\x98\x03\xd3\x03\x97\x03\x0e" // |..@.(........... |
/* 0450 */ "\x1d\x00\x0f\x0a\x1e\x09\x1e\x09\x0b\x09\x13\x09\x0c\x08\x19\x0a" // |................ |
/* 0460 */ "\x3f\xff\xff\x01\xee\xff\xff\x11\x0e\x02\xba\x13\x11\x0b\x48\xfe" // |?.............H. |
/* 0470 */ "\x02\x11\x0f\xb8\x15\x48\xeb\x01\x3f\x3c\x00\xc4\x02\x2e\x00\x1e" // |.....H..?<...... |
/* 0480 */ "\x00\xec\x08\x11\x0f\x01\x01\xfd\xfd\x09\x1b\xc0\x3f\x38\x44\x00" // |............?8D. |
/* 0490 */ "\xcc\x02\x64\x00\xdc\x02\x44\x00\xcc\x02\x64\x00\xdc\x02\x43\x00" // |..d...D...d...C. |
/* 04a0 */ "\xdc\x02\x0a\x1f\x00\x09\x04\x20\x05\x15\x05\x21\x08\x19\x0a\x3f" // |....... ...!...? |
/* 04b0 */ "\xff\xff\x01\xe7\xff\xff\x11\x0e\x02\xd6\x1a\x11\x0b\x2c\xfe\x02" // |.............,.. |
/* 04c0 */ "\x11\x0f\xd4\x1c\x2c\xe4"                                         //....,. |
// Dump done on RDP Wab Target (4) 1222 bytes |
// LOOPING on PDUs: 1211 |
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
// order(10):opaquerect(rect(0,702,787,6) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,708,60,17) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,708,681,17) color=0x0008ec) |
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
// order(10):opaquerect(rect(0,725,60,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,725,918,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,735,60,3) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,735,704,3) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,738,810,20) color=0x0008ec) |
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
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,708,46,30) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(61,709,43,27) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(68,716,33,17) op=(68,716,33,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=67 glyph_y=732 data_len=10 [0x1f 0x00 0x09 0x04 0x20 0x05 0x15 0x05 0x21 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,708,44,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,710,2,28) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(104,708,2,30) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,736,46,2) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 4 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x52"                                                     //..R |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 78 bytes |
// Recv done on RDP Wab Target (4) 78 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x44\x44\x00\x17\x00\x09\x04" // |...h. ..pDD..... |
/* 0010 */ "\x02\x00\x01\x00\x00\x02\x44\x00\x02\x00\x00\x00\x00\x00\x00\x00" // |......D......... |
/* 0020 */ "\x05\x00\x00\x00\x01\x0f\x10\x00\x3f\x00\xc5\x00\x12\x00\x11\x0f" // |........?....... |
/* 0030 */ "\xff\xff\x01\xef\x01\x0e\x3f\x00\x01\x00\x12\x00\x01\x0b\xd5\x00" // |......?......... |
/* 0040 */ "\x3e\x00\x14\x00\x01\x0f\x0f\x00\x51\x00\xc7\x00\x01\x00"         //>.......Q..... |
// Dump done on RDP Wab Target (4) 78 bytes |
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
/* 0000 */ "\x00\x0d\x0f"                                                     //... |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 3339 bytes |
// Recv done on RDP Wab Target (4) 3339 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x8d\x00\x00\x0d\x17\x00\x09" // |...h. ..p....... |
/* 0010 */ "\x04\x02\x00\x01\x00\x00\x02\x00\x0d\x02\x00\x00\x00\x00\x00\x00" // |................ |
/* 0020 */ "\x00\xae\x00\x00\x00\x01\x3f\x00\x00\x00\x00\x00\x04\x0a\x00\xec" // |......?......... |
/* 0030 */ "\x08\x01\x0e\x0a\x00\x14\x00\x10\x00\x01\x05\x7f\x00\x3d\x03\x01" // |.............=.. |
/* 0040 */ "\x05\xec\x03\x14\x00\x01\x0f\x00\x00\x1a\x00\xbc\x03\x07\x00\x01" // |................ |
/* 0050 */ "\x05\xec\x03\x14\x00\x01\x0f\x00\x00\x21\x00\x00\x04\x08\x00\x01" // |.........!...... |
/* 0060 */ "\x0e\x29\x00\x0f\x00\x10\x00\x01\x01\xf1\x03\x01\x0f\x00\x00\x39" // |.).............9 |
/* 0070 */ "\x00\x00\x04\x05\x00\x01\x0e\x3e\x00\x0f\x00\x14\x00\x01\x05\xd6" // |.......>........ |
/* 0080 */ "\x00\x05\x00\x01\x01\x98\x03\x11\x05\x59\x0a\x01\x0f\x00\x00\x52" // |.........Y.....R |
/* 0090 */ "\x00\x00\x04\x05\x00\x01\x0e\x57\x00\x0f\x00\x84\x00\x01\x01\xf1" // |.......W........ |
/* 00a0 */ "\x03\x01\x0f\x00\x00\xdb\x00\x00\x04\xe3\x01\x01\x0e\xbe\x02\x13" // |................ |
/* 00b0 */ "\x03\x06\x00\x01\x0e\xc4\x02\x3c\x00\x11\x00\x01\x05\x6a\x00\xa9" // |.......<.....j.. |
/* 00c0 */ "\x02\x01\x0f\x34\x03\xbe\x02\x0f\x00\x17\x00\x11\x0d\x29\x36\xea" // |...4.........)6. |
/* 00d0 */ "\x11\x05\x5f\xca\x11\x05\x30\x05\x01\x0f\x5d\x03\xbf\x02\x0f\x00" // |.._...0...]..... |
/* 00e0 */ "\x14\x00\x11\x0d\x28\x0e\xee\x11\x05\x37\xf2\x11\x05\x30\x05\x11" // |....(....7...0.. |
/* 00f0 */ "\x0f\xa7\x02\xfb\x0e\x11\x01\x29\x11\x05\x30\x05\x11\x0f\x99\x10" // |.......)..0..... |
/* 0100 */ "\x09\xf2\x11\x05\x37\xf2\x11\x05\x30\x05\x01\x07\x5d\x03\xd3\x02" // |....7...0...]... |
/* 0110 */ "\x45\x00\x11\x05\x5f\xca\x11\x05\x30\x05\x01\x0f\x00\x00\xd5\x02" // |E..._...0....... |
/* 0120 */ "\x3c\x00\x0a\x00\x01\x05\x6a\x00\x96\x03\x01\x0f\x00\x00\xdf\x02" // |<.....j......... |
/* 0130 */ "\x3c\x00\x03\x00\x01\x05\x6a\x00\xc0\x02\x11\x0f\x96\x03\x6a\x11" // |<.....j.......j. |
/* 0140 */ "\x01\x0f\x69\x03\xdf\x02\x28\x00\x17\x00\x11\x05\x72\xfd\x01\x0f" // |..i...(.....r... |
/* 0150 */ "\x00\x00\xf6\x02\x00\x04\x0a\x00\x01\x0f\x14\x00\x0a\x00\x6b\x00" // |..............k. |
/* 0160 */ "\x10\x00\x0d\x1b\xc0\x3f\x38\x0f\x14\x00\x0a\x00\x7e\x00\x19\x00" // |.....?8.....~... |
/* 0170 */ "\x14\x00\x0a\x00\x7f\x00\x1a\x00\x14\x00\x0a\x00\x7f\x00\x1a\x00" // |................ |
/* 0180 */ "\x13\x00\x1a\x00\x1a\x00\x00\x01\x09\x02\x0e\x03\x09\x04\x09\x02" // |................ |
/* 0190 */ "\x04\x03\x09\x04\x09\x05\x04\x06\x09\x04\x09\x06\x04\x06\x09\x19" // |................ |
/* 01a0 */ "\x0a\x37\xfb\x1f\x61\x98\x43\x0d\x1b\xa0\x3b\x30\xf0\xfb\x1f\x5c" // |.7..a.C...;0.... |
/* 01b0 */ "\x1f\x98\x43\x00\x29\x00\x70\x00\x39\x00\x29\x00\x70\x00\x39\x00" // |..C.).p.9.).p.9. |
/* 01c0 */ "\x39\x00\x18\x07\x00\x08\x09\x09\x08\x0a\x05\x0b\x09\x0c\x09\x0d" // |9............... |
/* 01d0 */ "\x05\x0e\x04\x09\x0b\x0f\x05\x10\x09\x11\x09\x09\x0a\x05\xdb\x00" // |................ |
/* 01e0 */ "\xc2\x02\x0d\x1b\x40\x15\x28\x05\xdb\x00\x9c\x03\xe0\x00\x0d\x01" // |....@.(......... |
/* 01f0 */ "\xe0\x00\x0d\x01\xdf\x00\x0c\x07\x00\x08\x09\x09\x08\x0a\x05\x0b" // |................ |
/* 0200 */ "\x09\x0c\x09\x09\x0a\x05\x9d\x03\x54\x00\x0d\x1b\x40\x15\x28\x41" // |........T...@.(A |
/* 0210 */ "\x9d\x03\x54\xa2\x03\xdb\x03\xa2\x03\xdb\x03\xa1\x03\x10\x12\x00" // |..T............. |
/* 0220 */ "\x09\x09\x0f\x05\x0c\x09\x0f\x05\x13\x09\x0f\x08\x14\x09\x09\x0a" // |................ |
/* 0230 */ "\x3f\x10\x00\x3f\x00\xc5\x00\x12\x00\xff\xff\x11\x0f\xff\xff\x01" // |?..?............ |
/* 0240 */ "\xef\x01\x0e\x3f\x00\x01\x00\x12\x00\x01\x0b\xd5\x00\x3e\x00\x14" // |...?.........>.. |
/* 0250 */ "\x00\x01\x0f\x0f\x00\x51\x00\xc7\x00\x01\x00\x01\x0f\xdc\x00\x3f" // |.....Q.........? |
/* 0260 */ "\x00\xbb\x02\x12\x00\x11\x0f\xff\xff\x01\xef\x01\x0e\x3f\x00\x01" // |.............?.. |
/* 0270 */ "\x00\x12\x00\x01\x0b\x97\x03\x3e\x00\x14\x00\x01\x0f\xdb\x00\x51" // |.......>.......Q |
/* 0280 */ "\x00\xbd\x02\x01\x00\x01\x0f\x9e\x03\x3f\x00\x52\x00\x12\x00\x11" // |.........?.R.... |
/* 0290 */ "\x0f\xff\xff\x01\xef\x11\x0e\x01\xae\x11\x11\x0b\x53\xff\x02\x11" // |............S... |
/* 02a0 */ "\x0f\xad\x13\x53\xed\x11\x3f\x1f\xb9\xdc\x16\xec\x08\x11\x0f\x01" // |...S..?......... |
/* 02b0 */ "\x01\xfd\xfd\x09\x1b\xe0\x3f\x38\xec\x08\x00\xc3\x03\x0d\x00\xe4" // |......?8........ |
/* 02c0 */ "\x03\x1d\x00\xc3\x03\x0d\x00\xe4\x03\x1d\x00\xc2\x03\x1d\x00\x0c" // |................ |
/* 02d0 */ "\x15\x00\x16\x08\x14\x03\x0c\x03\x0b\x05\x09\x09\x19\x0a\x3f\xff" // |..............?. |
/* 02e0 */ "\xff\x01\xee\xff\xff\x11\x0e\x02\xd4\x13\x11\x0b\x2e\xfe\x02\x11" // |................ |
/* 02f0 */ "\x0f\xd2\x15\x2e\xeb\x01\x3f\x0f\x00\x57\x00\xe2\x03\x16\x00\x73" // |......?..W.....s |
/* 0300 */ "\x02\x01\x0f\x11\x00\x59\x00\xc8\x00\x12\x00\x03\x15\x00\x08\x00" // |.....Y.......... |
/* 0310 */ "\x03\x07\x01\x22\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00" // |..."............ |
/* 0320 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xfe\x09\x1b\xe0" // |................ |
/* 0330 */ "\x3f\x38\x73\x02\x00\x14\x00\x5a\x00\x5e\x00\x6a\x00\x14\x00\x5a" // |?8s....Z.^.j...Z |
/* 0340 */ "\x00\x5e\x00\x6a\x00\x13\x00\x6a\x00\x12\x0a\x00\x09\x09\x0f\x05" // |.^.j...j........ |
/* 0350 */ "\x10\x09\x11\x09\x22\x09\x08\x07\x11\x08\x11\x09\x09\x0a\x05\xdd" // |...."........... |
/* 0360 */ "\x00\xbe\x02\x03\x25\x00\x08\x00\x03\x07\x01\x23\x00\x01\x00\xf0" // |....%......#.... |
/* 0370 */ "\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x44" // |...............D |
/* 0380 */ "\x40\x44\x40\x44\x40\x2a\x80\x2a\x80\x2a\x80\x11\x00\x11\x00\x00" // |@D@D@*.*.*...... |
/* 0390 */ "\x00\x00\x00\x00\x00\x03\x25\x00\x08\x00\x03\x07\x01\x24\x00\x01" // |......%......$.. |
/* 03a0 */ "\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x01\x00\x01\x00\x01" // |................ |
/* 03b0 */ "\x00\x1d\x00\x23\x00\x41\x00\x41\x00\x41\x00\x41\x00\x23\x00\x1d" // |...#.A.A.A.A.#.. |
/* 03c0 */ "\x00\x00\x00\x00\x00\x00\x00\x03\x15\x00\x08\x00\x03\x07\x01\x25" // |...............% |
/* 03d0 */ "\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00\x00\x00\x3c\x42" // |..............<B |
/* 03e0 */ "\x40\x70\x0e\x02\x42\x3c\x00\x00\x00\x03\x15\x00\x08\x00\x03\x07" // |@p..B<.......... |
/* 03f0 */ "\x01\x26\x00\x01\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x40\x40\x40" // |.&...........@@@ |
/* 0400 */ "\x42\x44\x48\x70\x50\x48\x44\x42\x00\x00\x00\x03\x15\x00\x08\x00" // |BDHpPHDB........ |
/* 0410 */ "\x03\x07\x01\x27\x00\x02\x00\xf0\xff\x08\x00\x10\x00\x00\x00\x00" // |...'............ |
/* 0420 */ "\x00\x00\x00\x20\x20\x00\x00\x00\x20\x20\x00\x00\x00\x03\x25\x00" // |...  ...  ....%. |
/* 0430 */ "\x08\x00\x03\x07\x01\x28\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00" // |.....(.......... |
/* 0440 */ "\x00\x00\x00\x00\x00\x08\x00\x14\x00\x14\x00\x22\x00\x22\x00\x22" // |..........."."." |
/* 0450 */ "\x00\x7f\x00\x41\x00\x41\x00\x80\x80\x00\x00\x00\x00\x00\x00\x09" // |...A.A.......... |
/* 0460 */ "\x1b\x40\x15\x28\xe0\x00\xb8\x02\xe0\x00\xb8\x02\xdf\x00\x7a\x08" // |.@.(..........z. |
/* 0470 */ "\x00\x11\x08\x11\x09\x22\x09\x23\x07\x16\x0b\x1e\x03\x24\x09\x0f" // |.....".#.....$.. |
/* 0480 */ "\x09\x23\x09\x25\x0b\x20\x08\x0b\x05\x00\x09\x11\x09\x14\x09\x0f" // |.#.%. .......... |
/* 0490 */ "\x03\x09\x09\x0b\x05\x09\x09\x22\x05\x1f\x07\x08\x04\x26\x08\x0b" // |.......".....&.. |
/* 04a0 */ "\x08\x14\x09\x0f\x03\x0a\x09\x16\x09\x1e\x03\x01\x09\x14\x0e\x0f" // |................ |
/* 04b0 */ "\x03\x13\x09\x08\x08\x14\x08\x01\x03\x08\x0e\x11\x08\x11\x09\x22" // |..............." |
/* 04c0 */ "\x09\x23\x07\x16\x0b\x1e\x03\x24\x09\x0f\x09\x23\x09\x25\x0b\x20" // |.#.....$...#.%.  |
/* 04d0 */ "\x08\x0b\x05\x00\x09\x11\x09\x14\x09\x0f\x03\x09\x09\x0b\x05\x09" // |................ |
/* 04e0 */ "\x09\x27\x05\x28\x05\x12\x09\x12\x09\x09\x0a\x05\x9f\x03\x50\x00" // |.'.(..........P. |
/* 04f0 */ "\x09\x1b\x40\x15\x28\xa2\x03\xbd\x03\xa2\x03\xbd\x03\xa1\x03\x06" // |..@.(........... |
/* 0500 */ "\x28\x00\x12\x09\x12\x09\x09\x0a\x3f\x0f\x00\x6d\x00\xe2\x03\x16" // |(.......?..m.... |
/* 0510 */ "\x00\x7e\xef\x01\x0f\x11\x00\x6f\x00\xc8\x00\x12\x00\x03\x25\x00" // |.~.....o......%. |
/* 0520 */ "\x08\x00\x03\x07\x01\x29\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00" // |.....).......... |
/* 0530 */ "\x00\x00\x00\x00\x00\x3c\x00\x46\x00\x02\x00\x02\x00\x06\x00\x04" // |.....<.F........ |
/* 0540 */ "\x00\x08\x00\x10\x00\x20\x00\x7e\x00\x00\x00\x00\x00\x00\x00\x09" // |..... .~........ |
/* 0550 */ "\x1b\xf0\x3f\x38\x00\x00\x00\x7e\xef\x00\x14\x00\x70\x00\x72\x00" // |..?8...~....p.r. |
/* 0560 */ "\x80\x00\x14\x00\x70\x00\x72\x00\x80\x00\x13\x00\x80\x00\x16\x0a" // |....p.r......... |
/* 0570 */ "\x00\x09\x09\x0f\x05\x10\x09\x11\x09\x22\x09\x23\x07\x29\x0b\x26" // |.........".#.).& |
/* 0580 */ "\x09\x02\x08\x29\x09\x09\x0a\x05\xdd\x00\xbe\x02\x03\x25\x00\x08" // |...).........%.. |
/* 0590 */ "\x00\x03\x07\x01\x2a\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00" // |....*........... |
/* 05a0 */ "\x00\x00\x40\x00\x40\x00\x40\x00\x5c\x00\x62\x00\x41\x00\x41\x00" // |..@.@.@...b.A.A. |
/* 05b0 */ "\x41\x00\x41\x00\x62\x00\x5c\x00\x00\x00\x00\x00\x00\x00\x03\x25" // |A.A.b..........% |
/* 05c0 */ "\x00\x08\x00\x03\x07\x01\x2b\x00\x01\x00\xf0\xff\x10\x00\x10\x00" // |......+......... |
/* 05d0 */ "\x00\x00\x00\x00\x00\x00\x7f\x00\x02\x00\x02\x00\x04\x00\x04\x00" // |................ |
/* 05e0 */ "\x08\x00\x08\x00\x10\x00\x10\x00\x20\x00\x00\x00\x00\x00\x00\x00" // |........ ....... |
/* 05f0 */ "\x03\x25\x00\x08\x00\x03\x07\x01\x2c\x00\x01\x00\xf0\xff\x10\x00" // |.%......,....... |
/* 0600 */ "\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x5c\xe0\x63\x10" // |..............c. |
/* 0610 */ "\x42\x10\x42\x10\x42\x10\x42\x10\x42\x10\x42\x10\x00\x00\x00\x00" // |B.B.B.B.B.B..... |
/* 0620 */ "\x00\x00\x03\x25\x00\x08\x00\x03\x07\x01\x2d\x00\x01\x00\xf0\xff" // |...%......-..... |
/* 0630 */ "\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x7e\x00\x41\x80\x40\xc0" // |..........~.A.@. |
/* 0640 */ "\x40\x40\x40\x40\x40\x40\x40\x40\x40\xc0\x41\x80\x7e\x00\x00\x00" // |@@@@@@@@@.A.~... |
/* 0650 */ "\x00\x00\x00\x00\x09\x1b\x40\x15\x28\xe0\x00\xb1\x02\xe0\x00\xb1" // |......@.(....... |
/* 0660 */ "\x02\xdf\x00\x76\x23\x00\x29\x0b\x26\x09\x02\x08\x29\x09\x20\x09" // |...v#.).&...). . |
/* 0670 */ "\x11\x05\x10\x09\x2a\x09\x20\x09\x05\x05\x05\x09\x04\x09\x29\x04" // |....*. .......). |
/* 0680 */ "\x2b\x09\x22\x09\x09\x07\x0b\x05\x24\x09\x20\x09\x08\x05\x24\x08" // |+.".....$. ...$. |
/* 0690 */ "\x2c\x09\x16\x0d\x1e\x03\x16\x09\x25\x03\x0c\x08\x09\x05\x08\x05" // |,.......%....... |
/* 06a0 */ "\x0c\x08\x0f\x05\x09\x09\x01\x05\x14\x0e\x0f\x03\x13\x09\x08\x08" // |................ |
/* 06b0 */ "\x14\x08\x01\x03\x23\x0e\x29\x0b\x26\x09\x02\x08\x29\x09\x20\x09" // |....#.).&...). . |
/* 06c0 */ "\x11\x05\x10\x09\x2a\x09\x20\x09\x05\x05\x05\x09\x04\x09\x29\x04" // |....*. .......). |
/* 06d0 */ "\x2b\x09\x27\x09\x21\x05\x2d\x0a\x12\x0b\x09\x0a\x05\x9f\x03\x50" // |+.'.!.-........P |
/* 06e0 */ "\x00\x09\x1b\x40\x15\x28\xa2\x03\xc0\x03\xa2\x03\xc0\x03\xa1\x03" // |...@.(.......... |
/* 06f0 */ "\x06\x21\x00\x2d\x0a\x12\x0b\x09\x0a\x3f\x0f\x00\x83\x00\xe2\x03" // |.!.-.....?...... |
/* 0700 */ "\x16\x00\xbd\xce\x01\x0f\x11\x00\x85\x00\xc8\x00\x12\x00\x09\x1b" // |................ |
/* 0710 */ "\xe0\x3f\x38\xbd\xce\x00\x14\x00\x86\x00\x69\x00\x96\x00\x14\x00" // |.?8.......i..... |
/* 0720 */ "\x86\x00\x69\x00\x96\x00\x13\x00\x96\x00\x14\x0a\x00\x09\x09\x0f" // |..i............. |
/* 0730 */ "\x05\x10\x09\x11\x09\x22\x09\x23\x07\x29\x0b\x26\x09\x06\x08\x09" // |.....".#.).&.... |
/* 0740 */ "\x0a\x05\xdd\x00\xbe\x02\x03\x25\x00\x08\x00\x03\x07\x01\x2e\x00" // |.......%........ |
/* 0750 */ "\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x1e\x00" // |................ |
/* 0760 */ "\x31\x00\x60\x00\x40\x00\x5e\x00\x63\x00\x41\x00\x41\x00\x23\x00" // |1.`.@.^.c.A.A.#. |
/* 0770 */ "\x1e\x00\x00\x00\x00\x00\x00\x00\x03\x25\x00\x08\x00\x03\x07\x01" // |.........%...... |
/* 0780 */ "\x2f\x00\x01\x00\xf0\xff\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00" // |/............... |
/* 0790 */ "\x00\x00\x00\x00\x1d\x00\x23\x00\x41\x00\x41\x00\x41\x00\x41\x00" // |......#.A.A.A.A. |
/* 07a0 */ "\x23\x00\x1d\x00\x01\x00\x01\x00\x01\x00\x09\x1b\x40\x15\x28\xe0" // |#...........@.(. |
/* 07b0 */ "\x00\xa2\x02\xe0\x00\xa2\x02\xdf\x00\x72\x23\x00\x29\x0b\x26\x09" // |.........r#.).&. |
/* 07c0 */ "\x06\x08\x20\x09\x11\x05\x10\x09\x2a\x09\x20\x09\x05\x05\x2e\x09" // |.. .....*. ..... |
/* 07d0 */ "\x04\x09\x2b\x04\x06\x09\x22\x09\x2f\x07\x08\x09\x20\x08\x08\x05" // |..+..."./... ... |
/* 07e0 */ "\x24\x08\x2c\x09\x16\x0d\x1e\x03\x16\x09\x25\x03\x0c\x08\x09\x05" // |$.,.......%..... |
/* 07f0 */ "\x08\x05\x0c\x08\x0b\x05\x10\x09\x09\x09\x01\x05\x14\x0e\x0f\x03" // |................ |
/* 0800 */ "\x13\x09\x08\x08\x14\x08\x01\x03\x23\x0e\x29\x0b\x26\x09\x06\x08" // |........#.).&... |
/* 0810 */ "\x20\x09\x11\x05\x10\x09\x2a\x09\x20\x09\x05\x05\x2e\x09\x04\x09" // | .....*. ....... |
/* 0820 */ "\x2b\x04\x06\x09\x27\x09\x21\x05\x2d\x0a\x12\x0b\x09\x0a\x05\x9f" // |+...'.!.-....... |
/* 0830 */ "\x03\x50\x00\x09\x1b\x40\x15\x28\xa2\x03\xc0\x03\xa2\x03\xc0\x03" // |.P...@.(........ |
/* 0840 */ "\xa1\x03\x06\x21\x00\x2d\x0a\x12\x0b\x09\x0a\x3f\x0f\x00\x99\x00" // |...!.-.....?.... |
/* 0850 */ "\xe2\x03\x16\x00\x7e\xef\x01\x0f\x11\x00\x9b\x00\xc8\x00\x12\x00" // |....~........... |
/* 0860 */ "\x03\x25\x00\x08\x00\x03\x07\x01\x30\x00\x01\x00\xf0\xff\x10\x00" // |.%......0....... |
/* 0870 */ "\x10\x00\x00\x00\x00\x00\x00\x00\x3e\x00\x63\x00\x41\x00\x63\x00" // |........>.c.A.c. |
/* 0880 */ "\x1c\x00\x63\x00\x41\x00\x41\x00\x63\x00\x3e\x00\x00\x00\x00\x00" // |..c.A.A.c.>..... |
/* 0890 */ "\x00\x00\x09\x1b\xe0\x3f\x38\x7e\xef\x00\x14\x00\x9c\x00\x69\x00" // |.....?8~......i. |
/* 08a0 */ "\xac\x00\x14\x00\x9c\x00\x69\x00\xac\x00\x13\x00\xac\x00\x14\x0a" // |......i......... |
/* 08b0 */ "\x00\x09\x09\x0f\x05\x10\x09\x11\x09\x22\x09\x23\x07\x29\x0b\x26" // |.........".#.).& |
/* 08c0 */ "\x09\x30\x08\x09\x0a\x05\xdd\x00\xbe\x02\x09\x1b\x40\x15\x28\xe0" // |.0..........@.(. |
/* 08d0 */ "\x00\x78\x02\xe0\x00\x78\x02\xdf\x00\x6c\x23\x00\x29\x0b\x26\x09" // |.x...x...l#.).&. |
/* 08e0 */ "\x30\x08\x20\x09\x11\x05\x09\x09\x16\x05\x20\x03\x05\x05\x2b\x09" // |0. ....... ...+. |
/* 08f0 */ "\x04\x09\x02\x04\x2e\x09\x22\x09\x08\x07\x24\x08\x2c\x09\x16\x0d" // |......"...$.,... |
/* 0900 */ "\x1e\x03\x16\x09\x25\x03\x0c\x08\x09\x05\x08\x05\x0c\x08\x0b\x05" // |....%........... |
/* 0910 */ "\x10\x09\x09\x09\x01\x05\x14\x0e\x0f\x03\x13\x09\x08\x08\x14\x08" // |................ |
/* 0920 */ "\x01\x03\x23\x0e\x29\x0b\x26\x09\x30\x08\x20\x09\x11\x05\x09\x09" // |..#.).&.0. ..... |
/* 0930 */ "\x16\x05\x20\x03\x05\x05\x2b\x09\x04\x09\x02\x04\x2e\x09\x27\x09" // |.. ...+.......'. |
/* 0940 */ "\x21\x05\x2d\x0a\x12\x0b\x09\x0a\x05\x9f\x03\x50\x00\x09\x1b\x40" // |!.-........P...@ |
/* 0950 */ "\x15\x28\xa2\x03\xc0\x03\xa2\x03\xc0\x03\xa1\x03\x06\x21\x00\x2d" // |.(...........!.- |
/* 0960 */ "\x0a\x12\x0b\x09\x0a\x3f\x0f\x00\xaf\x00\xe2\x03\x16\x00\xbd\xce" // |.....?.......... |
/* 0970 */ "\x01\x0f\x11\x00\xb1\x00\xc8\x00\x12\x00\x09\x1b\xe0\x3f\x38\xbd" // |.............?8. |
/* 0980 */ "\xce\x00\x14\x00\xb2\x00\x69\x00\xc2\x00\x14\x00\xb2\x00\x69\x00" // |......i.......i. |
/* 0990 */ "\xc2\x00\x13\x00\xc2\x00\x14\x0a\x00\x09\x09\x0f\x05\x10\x09\x11" // |................ |
/* 09a0 */ "\x09\x22\x09\x23\x07\x29\x0b\x26\x09\x30\x08\x09\x0a\x05\xdd\x00" // |.".#.).&.0...... |
/* 09b0 */ "\xbe\x02\x09\x1b\x40\x15\x28\xe0\x00\x65\x02\xe0\x00\x65\x02\xdf" // |....@.(..e...e.. |
/* 09c0 */ "\x00\x64\x23\x00\x29\x0b\x26\x09\x30\x08\x20\x09\x11\x05\x09\x09" // |.d#.).&.0. ..... |
/* 09d0 */ "\x16\x05\x20\x03\x05\x05\x2b\x09\x04\x09\x02\x04\x2e\x09\x22\x09" // |.. ...+.......". |
/* 09e0 */ "\x09\x07\x0b\x05\x24\x09\x0b\x09\x2c\x09\x11\x0d\x0c\x09\x16\x05" // |....$...,....... |
/* 09f0 */ "\x0f\x03\x1e\x09\x01\x09\x14\x0e\x0f\x03\x13\x09\x08\x08\x14\x08" // |................ |
/* 0a00 */ "\x01\x03\x23\x0e\x29\x0b\x26\x09\x30\x08\x20\x09\x11\x05\x09\x09" // |..#.).&.0. ..... |
/* 0a10 */ "\x16\x05\x20\x03\x05\x05\x2b\x09\x04\x09\x02\x04\x2e\x09\x27\x09" // |.. ...+.......'. |
/* 0a20 */ "\x21\x05\x2d\x0a\x12\x0b\x09\x0a\x05\x9f\x03\x50\x00\x09\x1b\x40" // |!.-........P...@ |
/* 0a30 */ "\x15\x28\xa2\x03\xc0\x03\xa2\x03\xc0\x03\xa1\x03\x06\x21\x00\x2d" // |.(...........!.- |
/* 0a40 */ "\x0a\x12\x0b\x09\x0a\x3f\x0f\x00\xc5\x00\xe2\x03\x16\x00\x7e\xef" // |.....?........~. |
/* 0a50 */ "\x01\x0f\x11\x00\xc7\x00\xc8\x00\x12\x00\x09\x1b\xe0\x3f\x38\x7e" // |.............?8~ |
/* 0a60 */ "\xef\x00\x14\x00\xc8\x00\x69\x00\xd8\x00\x14\x00\xc8\x00\x69\x00" // |......i.......i. |
/* 0a70 */ "\xd8\x00\x13\x00\xd8\x00\x14\x0a\x00\x09\x09\x0f\x05\x10\x09\x11" // |................ |
/* 0a80 */ "\x09\x22\x09\x23\x07\x29\x0b\x26\x09\x30\x08\x09\x0a\x05\xdd\x00" // |.".#.).&.0...... |
/* 0a90 */ "\xbe\x02\x09\x1b\x40\x15\x28\xe0\x00\xa2\x02\xe0\x00\xa2\x02\xdf" // |....@.(......... |
/* 0aa0 */ "\x00\x72\x23\x00\x29\x0b\x26\x09\x30\x08\x20\x09\x11\x05\x10\x09" // |.r#.).&.0. ..... |
/* 0ab0 */ "\x2a\x09\x20\x09\x05\x05\x2e\x09\x04\x09\x30\x04\x30\x09\x22\x09" // |*. .......0.0.". |
/* 0ac0 */ "\x2f\x07\x08\x09\x20\x08\x08\x05\x24\x08\x2c\x09\x16\x0d\x1e\x03" // |/... ...$.,..... |
/* 0ad0 */ "\x16\x09\x25\x03\x0c\x08\x09\x05\x08\x05\x0c\x08\x0b\x05\x10\x09" // |..%............. |
/* 0ae0 */ "\x09\x09\x01\x05\x14\x0e\x0f\x03\x13\x09\x08\x08\x14\x08\x01\x03" // |................ |
/* 0af0 */ "\x23\x0e\x29\x0b\x26\x09\x30\x08\x20\x09\x11\x05\x10\x09\x2a\x09" // |#.).&.0. .....*. |
/* 0b00 */ "\x20\x09\x05\x05\x2e\x09\x04\x09\x30\x04\x30\x09\x27\x09\x21\x05" // | .......0.0.'.!. |
/* 0b10 */ "\x2d\x0a\x12\x0b\x09\x0a\x05\x9f\x03\x50\x00\x09\x1b\x40\x15\x28" // |-........P...@.( |
/* 0b20 */ "\xa2\x03\xc0\x03\xa2\x03\xc0\x03\xa1\x03\x06\x21\x00\x2d\x0a\x12" // |...........!.-.. |
/* 0b30 */ "\x0b\x09\x0a\x3f\x13\x03\xbe\x02\x21\x00\x17\x00\xec\x08\x11\x0f" // |...?....!....... |
/* 0b40 */ "\x01\x01\xfd\xfd\x09\x1b\xf0\x3f\x38\xff\xff\x00\xec\x08\x00\x1a" // |.......?8....... |
/* 0b50 */ "\x03\xc1\x02\x2c\x03\xd1\x02\x1a\x03\xc1\x02\x2c\x03\xd1\x02\x19" // |...,.......,.... |
/* 0b60 */ "\x03\xd1\x02\x04\x17\x00\x18\x0b\x19\x0a\x0f\x2f\xff\xfc\x03\x11" // |.........../.... |
/* 0b70 */ "\x0f\x01\x01\xfd\xfd\x09\x1b\x40\x15\x28\x4a\x03\x55\x03\x4a\x03" // |.......@.(J.U.J. |
/* 0b80 */ "\x55\x03\x49\x03\x02\x17\x00\x19\x0a\x3b\x29\x01\xfe\xff\xff\x09" // |U.I......;)..... |
/* 0b90 */ "\x1b\x70\x15\x28\x00\x00\x00\xff\xff\x00\x6e\x03\x77\x03\x6e\x03" // |.p.(......n.w.n. |
/* 0ba0 */ "\x77\x03\x6c\x03\x02\x02\x00\x19\x0a\x0f\xff\xff\x01\xef\x11\x0e" // |w.l............. |
/* 0bb0 */ "\x01\xe9\x11\x11\x0b\x18\xff\x02\x11\x0f\xe8\x13\x18\xed\x11\x3f" // |...............? |
/* 0bc0 */ "\x19\xef\xf5\x0f\xec\x08\x0d\x1b\x70\x15\x28\x5a\xe8\xc1\x02\xa2" // |........p.(Z.... |
/* 0bd0 */ "\xd0\x02\xff\xff\x00\xec\x08\x00\x85\x03\x93\x03\x85\x03\x93\x03" // |................ |
/* 0be0 */ "\x84\x03\x04\x19\x00\x02\x05\x19\x0a\x0f\x1d\xfd\x0c\x07\x11\x0f" // |................ |
/* 0bf0 */ "\x01\x01\xfd\xfd\x09\x1b\x40\x15\x28\xa9\x03\xb4\x03\xa9\x03\xb4" // |......@.(....... |
/* 0c00 */ "\x03\xa8\x03\x02\x1a\x00\x19\x0a\x0f\x28\xff\x0a\x03\x11\x0f\x01" // |.........(...... |
/* 0c10 */ "\x01\xfd\xfd\x09\x1b\x40\x15\x28\xd2\x03\xe4\x03\xd2\x03\xe4\x03" // |.....@.(........ |
/* 0c20 */ "\xd1\x03\x04\x1b\x00\x1a\x07\x09\x0a\x0f\x2a\x03\xdf\x02\x3f\x00" // |..........*...?. |
/* 0c30 */ "\x17\x00\x11\x0f\x01\x01\xfd\xfd\x09\x1b\xc0\x3f\x38\x31\x03\xe2" // |...........?81.. |
/* 0c40 */ "\x02\x61\x03\xf2\x02\x31\x03\xe2\x02\x61\x03\xf2\x02\x30\x03\xf2" // |.a...1...a...0.. |
/* 0c50 */ "\x02\x0c\x1c\x00\x0f\x07\x0a\x09\x0f\x09\x10\x09\x0c\x09\x19\x0a" // |................ |
/* 0c60 */ "\x3f\xff\xff\x01\xee\xff\xff\x11\x0e\x02\xc5\x13\x11\x0b\x3d\xfe" // |?.............=. |
/* 0c70 */ "\x02\x11\x0f\xc3\x15\x3d\xeb\x11\x3f\x67\xeb\x0b\x15\xec\x08\x11" // |.....=..?g...... |
/* 0c80 */ "\x0f\x01\x01\xfd\xfd\x09\x1b\x40\x15\x28\x98\x03\xd3\x03\x98\x03" // |.......@.(...... |
/* 0c90 */ "\xd3\x03\x97\x03\x0e\x1d\x00\x0f\x0a\x1e\x09\x1e\x09\x0b\x09\x13" // |................ |
/* 0ca0 */ "\x09\x0c\x08\x19\x0a\x3f\xff\xff\x01\xee\xff\xff\x11\x0e\x02\xba" // |.....?.......... |
/* 0cb0 */ "\x13\x11\x0b\x48\xfe\x02\x11\x0f\xb8\x15\x48\xeb\x01\x3f\x3c\x00" // |...H......H..?<. |
/* 0cc0 */ "\xc4\x02\x2e\x00\x1e\x00\xec\x08\x11\x0f\x01\x01\xfd\xfd\x09\x1b" // |................ |
/* 0cd0 */ "\xc0\x3f\x38\x44\x00\xcc\x02\x64\x00\xdc\x02\x44\x00\xcc\x02\x64" // |.?8D...d...D...d |
/* 0ce0 */ "\x00\xdc\x02\x43\x00\xdc\x02\x0a\x1f\x00\x09\x04\x20\x05\x15\x05" // |...C........ ... |
/* 0cf0 */ "\x21\x08\x19\x0a\x3f\xff\xff\x01\xe7\xff\xff\x11\x0e\x02\xd6\x1a" // |!...?........... |
/* 0d00 */ "\x11\x0b\x2c\xfe\x02\x11\x0f\xd4\x1c\x2c\xe4"                     //..,......,. |
// Dump done on RDP Wab Target (4) 3339 bytes |
// LOOPING on PDUs: 3328 |
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
// order(10):opaquerect(rect(0,87,15,132) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(1009,87,15,132) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,219,1024,483) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,702,787,6) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,708,60,17) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,708,681,17) color=0x0008ec) |
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
// order(10):opaquerect(rect(0,725,60,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,725,918,10) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,735,60,3) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(106,735,704,3) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(0,738,810,20) color=0x0008ec) |
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
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(20,90,75,17) op=(20,90,75,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=106 data_len=18 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x22 0x09 0x08 0x07 0x11 0x08 0x11 0x09] |
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
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(224,90,473,17) op=(224,90,473,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=106 data_len=122 [0x08 0x00 0x11 0x08 0x11 0x09 0x22 0x09 0x23 0x07 0x16 0x0b 0x1e 0x03 0x24 0x09 0x0f 0x09 0x23 0x09 0x25 0x0b 0x20 0x08 0x0b 0x05 0x00 0x09 0x11 0x09 0x14 0x09 0x0f 0x03 0x09 0x09 0x0b 0x05 0x09 0x09 0x22 0x05 0x1f 0x07 0x08 0x04 0x26 0x08 0x0b 0x08 0x14 0x09 0x0f 0x03 0x0a 0x09 0x16 0x09 0x1e 0x03 0x01 0x09 0x14 0x0e 0x0f 0x03 0x13 0x09 0x08 0x08 0x14 0x08 0x01 0x03 0x08 0x0e 0x11 0x08 0x11 0x09 0x22 0x09 0x23 0x07 0x16 0x0b 0x1e 0x03 0x24 0x09 0x0f 0x09 0x23 0x09 0x25 0x0b 0x20 0x08 0x0b 0x05 0x00 0x09 0x11 0x09 0x14 0x09 0x0f 0x03 0x09 0x09 0x0b 0x05 0x09 0x09 0x27 0x05 0x28 0x05 0x12 0x09 0x12 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,89,80,18) color=0x000273) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=000273 bk=(930,90,28,17) op=(930,90,28,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=106 data_len=6 [0x28 0x00 0x12 0x09 0x12 0x09] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,109,994,22) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,111,200,18) color=0x00ef7e) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(20,112,95,17) op=(20,112,95,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=128 data_len=22 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x22 0x09 0x23 0x07 0x29 0x0b 0x26 0x09 0x02 0x08 0x29 0x09] |
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
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(224,112,466,17) op=(224,112,466,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=128 data_len=118 [0x23 0x00 0x29 0x0b 0x26 0x09 0x02 0x08 0x29 0x09 0x20 0x09 0x11 0x05 0x10 0x09 0x2a 0x09 0x20 0x09 0x05 0x05 0x05 0x09 0x04 0x09 0x29 0x04 0x2b 0x09 0x22 0x09 0x09 0x07 0x0b 0x05 0x24 0x09 0x20 0x09 0x08 0x05 0x24 0x08 0x2c 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x25 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0f 0x05 0x09 0x09 0x01 0x05 0x14 0x0e 0x0f 0x03 0x13 0x09 0x08 0x08 0x14 0x08 0x01 0x03 0x23 0x0e 0x29 0x0b 0x26 0x09 0x02 0x08 0x29 0x09 0x20 0x09 0x11 0x05 0x10 0x09 0x2a 0x09 0x20 0x09 0x05 0x05 0x05 0x09 0x04 0x09 0x29 0x04 0x2b 0x09 0x27 0x09 0x21 0x05 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,111,80,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(930,112,31,17) op=(930,112,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=128 data_len=6 [0x21 0x00 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,131,994,22) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,133,200,18) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(20,134,86,17) op=(20,134,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=150 data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x22 0x09 0x23 0x07 0x29 0x0b 0x26 0x09 0x06 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,133,702,18) color=0x00cebd) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(224,134,451,17) op=(224,134,451,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=150 data_len=114 [0x23 0x00 0x29 0x0b 0x26 0x09 0x06 0x08 0x20 0x09 0x11 0x05 0x10 0x09 0x2a 0x09 0x20 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x2b 0x04 0x06 0x09 0x22 0x09 0x2f 0x07 0x08 0x09 0x20 0x08 0x08 0x05 0x24 0x08 0x2c 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x25 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x14 0x0e 0x0f 0x03 0x13 0x09 0x08 0x08 0x14 0x08 0x01 0x03 0x23 0x0e 0x29 0x0b 0x26 0x09 0x06 0x08 0x20 0x09 0x11 0x05 0x10 0x09 0x2a 0x09 0x20 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x2b 0x04 0x06 0x09 0x27 0x09 0x21 0x05 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,133,80,18) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(930,134,31,17) op=(930,134,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=150 data_len=6 [0x21 0x00 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,153,994,22) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,155,200,18) color=0x00ef7e) |
// ======================================== |
// rdp_orders_process_glyphcache |
// rdp_orders_process_glyphcache done |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(20,156,86,17) op=(20,156,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=172 data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x22 0x09 0x23 0x07 0x29 0x0b 0x26 0x09 0x30 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,155,702,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(224,156,409,17) op=(224,156,409,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=172 data_len=108 [0x23 0x00 0x29 0x0b 0x26 0x09 0x30 0x08 0x20 0x09 0x11 0x05 0x09 0x09 0x16 0x05 0x20 0x03 0x05 0x05 0x2b 0x09 0x04 0x09 0x02 0x04 0x2e 0x09 0x22 0x09 0x08 0x07 0x24 0x08 0x2c 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x25 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x14 0x0e 0x0f 0x03 0x13 0x09 0x08 0x08 0x14 0x08 0x01 0x03 0x23 0x0e 0x29 0x0b 0x26 0x09 0x30 0x08 0x20 0x09 0x11 0x05 0x09 0x09 0x16 0x05 0x20 0x03 0x05 0x05 0x2b 0x09 0x04 0x09 0x02 0x04 0x2e 0x09 0x27 0x09 0x21 0x05 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,155,80,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(930,156,31,17) op=(930,156,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=172 data_len=6 [0x21 0x00 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,175,994,22) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,177,200,18) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(20,178,86,17) op=(20,178,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=194 data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x22 0x09 0x23 0x07 0x29 0x0b 0x26 0x09 0x30 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,177,702,18) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(224,178,390,17) op=(224,178,390,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=194 data_len=100 [0x23 0x00 0x29 0x0b 0x26 0x09 0x30 0x08 0x20 0x09 0x11 0x05 0x09 0x09 0x16 0x05 0x20 0x03 0x05 0x05 0x2b 0x09 0x04 0x09 0x02 0x04 0x2e 0x09 0x22 0x09 0x09 0x07 0x0b 0x05 0x24 0x09 0x0b 0x09 0x2c 0x09 0x11 0x0d 0x0c 0x09 0x16 0x05 0x0f 0x03 0x1e 0x09 0x01 0x09 0x14 0x0e 0x0f 0x03 0x13 0x09 0x08 0x08 0x14 0x08 0x01 0x03 0x23 0x0e 0x29 0x0b 0x26 0x09 0x30 0x08 0x20 0x09 0x11 0x05 0x09 0x09 0x16 0x05 0x20 0x03 0x05 0x05 0x2b 0x09 0x04 0x09 0x02 0x04 0x2e 0x09 0x27 0x09 0x21 0x05 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,177,80,18) color=0x00cebd) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00cebd bk=(930,178,31,17) op=(930,178,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=194 data_len=6 [0x21 0x00 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(15,197,994,22) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(17,199,200,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(20,200,86,17) op=(20,200,86,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=19 glyph_y=216 data_len=20 [0x0a 0x00 0x09 0x09 0x0f 0x05 0x10 0x09 0x11 0x09 0x22 0x09 0x23 0x07 0x29 0x0b 0x26 0x09 0x30 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(221,199,702,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(224,200,451,17) op=(224,200,451,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=223 glyph_y=216 data_len=114 [0x23 0x00 0x29 0x0b 0x26 0x09 0x30 0x08 0x20 0x09 0x11 0x05 0x10 0x09 0x2a 0x09 0x20 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x30 0x04 0x30 0x09 0x22 0x09 0x2f 0x07 0x08 0x09 0x20 0x08 0x08 0x05 0x24 0x08 0x2c 0x09 0x16 0x0d 0x1e 0x03 0x16 0x09 0x25 0x03 0x0c 0x08 0x09 0x05 0x08 0x05 0x0c 0x08 0x0b 0x05 0x10 0x09 0x09 0x09 0x01 0x05 0x14 0x0e 0x0f 0x03 0x13 0x09 0x08 0x08 0x14 0x08 0x01 0x03 0x23 0x0e 0x29 0x0b 0x26 0x09 0x30 0x08 0x20 0x09 0x11 0x05 0x10 0x09 0x2a 0x09 0x20 0x09 0x05 0x05 0x2e 0x09 0x04 0x09 0x30 0x04 0x30 0x09 0x27 0x09 0x21 0x05 0x2d 0x0a 0x12 0x0b] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(927,199,80,18) color=0x00ef7e) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=000000 fore_color=00ef7e bk=(930,200,31,17) op=(930,200,31,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=929 glyph_y=216 data_len=6 [0x21 0x00 0x2d 0x0a 0x12 0x0b] |
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
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,708,46,30) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(61,709,43,27) color=0x0008ec) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(27 clip(0,0,1024,768)):glyphIndex(cache_id=07 fl_accel=3 ui_charinc=0 f_op_redundant=1 back_color=00ffff fore_color=0008ec bk=(68,716,33,17) op=(68,716,33,17) brush.(org_x=0, org_y=0, style=3 hatch=170 extra=[aa,55,aa,55,aa,55,aa]) glyph_x=67 glyph_y=732 data_len=10 [0x1f 0x00 0x09 0x04 0x20 0x05 0x15 0x05 0x21 0x08] |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,708,44,2) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,710,2,28) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(104,708,2,30) color=0x00ffff) |
// ======================================== |
// --------- FRONT ------------------------ |
// order(10):opaquerect(rect(60,736,46,2) color=0x00ffff) |
// ======================================== |
// process_orders done |
// ===================> count = 6 |
// Socket RDP Wab Target (4) receiving 1 bytes |
// Recv done on RDP Wab Target (4) 1 bytes |
/* 0000 */ "\x03"                                                             //. |
// Dump done on RDP Wab Target (4) 1 bytes |
// Socket RDP Wab Target (4) receiving 3 bytes |
// Recv done on RDP Wab Target (4) 3 bytes |
/* 0000 */ "\x00\x00\x6b"                                                     //..k |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 103 bytes |
// Recv done on RDP Wab Target (4) 103 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x5d\x5d\x00\x17\x00\x09\x04" // |...h. ..p]]..... |
/* 0010 */ "\x02\x00\x01\x00\x00\x02\x5d\x00\x02\x00\x00\x00\x00\x00\x00\x00" // |......]......... |
/* 0020 */ "\x06\x00\x00\x00\x01\x0f\x6d\x03\xc0\x02\x17\x00\x12\x00\x09\x1b" // |......m......... |
/* 0030 */ "\xf0\x3f\x38\x00\x00\x00\xff\xff\x00\x6e\x03\xc1\x02\x77\x03\xd1" // |.?8......n...w.. |
/* 0040 */ "\x02\x6e\x03\xc1\x02\x77\x03\xd1\x02\x6c\x03\xd1\x02\x02\x02\x00" // |.n...w...l...... |
/* 0050 */ "\x19\x0a\x0f\xff\xff\x01\xef\x11\x0e\x01\xe9\x11\x11\x0b\x18\xff" // |................ |
/* 0060 */ "\x02\x11\x0f\xe8\x13\x18\xed"                                     //....... |
// Dump done on RDP Wab Target (4) 103 bytes |
// LOOPING on PDUs: 93 |
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
/* 0000 */ "\x00\x00\x4a"                                                     //..J |
// Dump done on RDP Wab Target (4) 3 bytes |
// Socket RDP Wab Target (4) receiving 70 bytes |
// Recv done on RDP Wab Target (4) 70 bytes |
/* 0000 */ "\x02\xf0\x80\x68\x00\x20\x03\xeb\x70\x3c\x3c\x00\x17\x00\x09\x04" // |...h. ..p<<..... |
/* 0010 */ "\x02\x00\x01\x00\x00\x02\x3c\x00\x02\x00\x00\x00\x00\x00\x00\x00" // |......<......... |
/* 0020 */ "\x02\x00\x00\x00\x11\x3f\x19\xef\xf5\x0f\xec\x08\x2d\x1b\x70\x15" // |.....?......-.p. |
/* 0030 */ "\x28\xff\xff\x00\xec\x08\x00\x85\x03\x93\x03\x85\x03\x93\x03\x84" // |(............... |
/* 0040 */ "\x03\x04\x19\x00\x02\x05"                                         //...... |
// Dump done on RDP Wab Target (4) 70 bytes |
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
// ~mod_rdp(): Recv order count      = 458 |
// ~mod_rdp(): Recv bmp update count = 0 |
// RDP Wab Target (0): total_received=9869, total_sent=1629 |
} /* end indata */;

