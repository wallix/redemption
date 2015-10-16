const char outdata[] =
{
// Front::Received fast-path PUD, remains=10 |
// asking for selector |
// Ask acl |
// sending login=RED\tartempion@win2k12r2:rdp:windows:x |
// sending target_device=ASK |
// sending target_login=ASK |
// sending proto_dest=ASK |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'target_login'='RED\tartempion' |
// receiving 'login'='x' |
// receiving 'module'='transitory' |
// receiving 'target_device'='win2k12r2' |
// ===========> MODULE_NEXT |
// ----------> ACL next_module <-------- |
// ===============> WAIT WITH CURRENT MODULE |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'forcemodule'='True' |
// ===========> MODULE_NEXT |
// ----------> ACL next_module <-------- |
// ===============> WAIT WITH CURRENT MODULE |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'mode_console'='allow' |
// receiving 'module'='RDP' |
// receiving 'proto_dest'='RDP' |
// receiving 'session_id'='SESSIONID-0000' |
// receiving 'target_host'='10.10.47.128' |
// receiving 'target_login'='RED\tartempion' |
// receiving 'target_port'='3389' |
// receiving 'timeclose'='2051218799' |
// receiving 'disable_tsk_switch_shortcuts'='False' |
// receiving 'timezone'='-7200' |
// receiving 'target_password'='<hidden>' |
// receiving 'password'='<hidden>' |
// receiving 'proxy_opt'='RDP_CLIPBOARD_FILE,RDP_COM_PORT,RDP_DRIVE,RDP_PRINTER,RDP_SMARTCARD' |
// ===========> MODULE_NEXT |
// ----------> ACL next_module <-------- |
// ===========> MODULE_RDP |
// target_module=5 |
// ModuleManager::Creation of new mod 'RDP' |
// connecting to 10.10.47.128:3389 |
// connection to 10.10.47.128:3389 succeeded : socket 6 |
// Creation of new mod 'RDP' |
// ModRDPParams target_user="RED\tartempion" |
// ModRDPParams target_password="<hidden>" |
// ModRDPParams target_host="10.10.47.128" |
// ModRDPParams client_address="0.0.0.0" |
// ModRDPParams client_name="rzh" |
// ModRDPParams enable_tls=yes |
// ModRDPParams enable_nla=yes |
// ModRDPParams enable_krb=no |
// ModRDPParams enable_fastpath=yes |
// ModRDPParams enable_mem3blt=yes |
// ModRDPParams enable_bitmap_update=yes |
// ModRDPParams enable_new_pointer=yes |
// ModRDPParams enable_glyph_cache=yes |
// ModRDPParams enable_session_probe=no |
// ModRDPParams session_probe_launch_timeout=20000 |
// ModRDPParams session_probe_on_launch_failure=0 |
// ModRDPParams session_probe_keepalive_timeout=0 |
// ModRDPParams dsiable_clipboard_log=0x0 |
// ModRDPParams enable_transparent_mode=no |
// ModRDPParams output_filename="" |
// ModRDPParams persistent_key_list_transport=<(nil)> |
// ModRDPParams transparent_recorder_transport=<(nil)> |
// ModRDPParams key_flags=0 |
// ModRDPParams acl=<0x61400000f570> |
// ModRDPParams auth_channel="" |
// ModRDPParams alternate_shell="" |
// ModRDPParams shell_working_directory="" |
// ModRDPParams target_application_account="" |
// ModRDPParams target_application_password="<hidden>" |
// ModRDPParams rdp_compression=4 |
// ModRDPParams error_message=<0x7fff5b45f478> |
// ModRDPParams disconnect_on_logon_user_change=no |
// ModRDPParams open_session_timeout=0 |
// ModRDPParams certificate_change_action=1 |
// ModRDPParams extra_orders=15,16,17,18,22 |
// ModRDPParams enable_persistent_disk_bitmap_cache=yes |
// ModRDPParams enable_cache_waiting_list=yes |
// ModRDPParams persist_bitmap_cache_on_disk=no |
// ModRDPParams password_printing_mode=0 |
// ModRDPParams allow_channels=<0x7fff5b45d4b0> |
// ModRDPParams deny_channels=<0x7fff5b45d4b8> |
// ModRDPParams remote_program=no |
// ModRDPParams server_redirection_support=no |
// ModRDPParams bogus_sc_net_size=yes |
// ModRDPParams client_device_announce_timeout=1000 |
// ModRDPParams proxy_managed_drives= |
// ModRDPParams verbose=0x44000001 |
// ModRDPParams cache_verbose=0x00000000 |
// RDP Extra orders="15,16,17,18,22" |
// RDP Extra orders number=15 |
// RDP Extra orders=MultiDstBlt |
// RDP Extra orders number=16 |
// RDP Extra orders=MultiPatBlt |
// RDP Extra orders number=17 |
// RDP Extra orders=MultiScrBlt |
// RDP Extra orders number=18 |
// RDP Extra orders=MultiOpaqueRect |
// RDP Extra orders number=22 |
// RDP Extra orders=Polyline |
// Remote RDP Server domain="RED" login="tartempion" host="rzh" |
// Server key layout is 40c |
// mod_rdp::Early TLS Security Exchange |
// RdpNego::NEGO_STATE_INITIAL |
// RdpNego::send_x224_connection_request_pdu |
// RdpNego::send_x224_connection_request_pdu done |
// mod_rdp::Early TLS Security Exchange |
// RdpNego::NEGO_STATE_NLA |
// RdpNego::recv_connection_confirm |
// NEG_RSP_TYPE=2 NEG_RSP_FLAGS=15 NEG_RSP_LENGTH=8 NEG_RSP_SELECTED_PROTOCOL=2 |
// CC Recv: PROTOCOL HYBRID |
// activating SSL |
// Client TLS start |
// SSL_connect() |
// SSL_get_peer_certificate() |
// nb1=1070 nb2=1070 |
// TLS::X509 existing::issuer=CN = testkrb.red.ifr.lan |
// TLS::X509 existing::subject=CN = testkrb.red.ifr.lan |
// TLS::X509 existing::fingerprint=3c:99:59:17:ae:5e:08:98:32:06:59:3e:1b:2c:97:b6:6e:26:d1:c8 |
// SocketTransport::X509_get_pubkey() |
// SocketTransport::i2d_PublicKey() |
// SocketTransport::i2d_PublicKey() |
// TLS::X509::issuer=CN = testkrb.red.ifr.lan |
// TLS::X509::subject=CN = testkrb.red.ifr.lan |
// TLS::X509::fingerprint=3c:99:59:17:ae:5e:08:98:32:06:59:3e:1b:2c:97:b6:6e:26:d1:c8 |
// SocketTransport::enable_tls() done |
// activating CREDSSP |
// Credssp: NTLM Authentication |
// Credssp: TSCredentials::emit() NegoToken |
// Credssp TSCredentials::recv() NEGOTOKENS |
// using /dev/urandom as random source |
// using /dev/urandom as random source |
// Credssp: TSCredentials::emit() NegoToken |
// Credssp: TSCredentials::emit() pubKeyAuth |
// Credssp TSCredentials::recv() PUBKEYAUTH |
// Credssp: TSCredentials::emit() AuthInfo |
// mod_rdp::Early TLS Security Exchange |
// Sending to Server GCC User Data CS_CORE (216 bytes) |
// cs_core::version [80004] RDP 5.0, 5.1, 5.2, and 6.0 clients) |
// cs_core::desktopWidth  = 1440 |
// cs_core::desktopHeight = 900 |
// cs_core::colorDepth    = [ca01] [RNS_UD_COLOR_8BPP] superseded by postBeta2ColorDepth |
// cs_core::SASSequence   = [aa03] [Unknown] |
// cs_core::keyboardLayout= 040c |
// cs_core::clientBuild   = 2600 |
// cs_core::clientName    = rzh |
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
// cs_core::serverSelectedProtocol = 2 |
// Sending to server GCC User Data CS_CLUSTER (12 bytes) |
// cs_cluster::flags [0000] |
// cs_cluster::redirectedSessionID = 0 |
// Sending to server GCC User Data CS_SECURITY (12 bytes) |
// CSSecGccUserData::encryptionMethods 3 |
// CSSecGccUserData::extEncryptionMethods 0 |
// Sending to server GCC User Data CS_NET (12 bytes) |
// cs_net::channelCount   = 1 |
// cs_net::channel ' cliprdr' [1004] INITIALIZED COMPRESS_RDP SHOW_PROTOCOL |
// mod_rdp::Basic Settings Exchange |
// GCC::UserData tag=0c01 length=16 |
// Received from server GCC User Data SC_CORE (16 bytes) |
// sc_core::version [80004] RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 servers) |
// sc_core::clientRequestedProtocols  = 3 |
// sc_core::earlyCapabilityFlags  = 1 |
// GCC::UserData tag=0c03 length=12 |
// Received from server GCC User Data SC_NET (12 bytes) |
// sc_net::MCSChannelId   = 1003 |
// sc_net::channelCount   = 1 |
// sc_net::channel[1004]::id = 1004 |
// sc_net::padding 2 bytes 0000 |
// GCC::UserData tag=0c02 length=12 |
// Received from server GCC User Data SC_SECURITY (12 bytes) |
// sc_security::encryptionMethod = 0 |
// sc_security::encryptionLevel  = 0 |
// No encryption |
// mod_rdp::Channel Connection |
// Send MCS::ErectDomainRequest |
// Send MCS::AttachUserRequest |
// mod_rdp::Channel Connection Attach User |
// mod_rdp::RDP Security Commencement |
// mod_rdp::Secure Settings Exchange |
// mod_rdp::send_client_info_pdu |
// send extended login info (RDP5-style) 907fb RED:tartempion |
// Send data request InfoPacket |
// InfoPacket::CodePage 0 |
// InfoPacket::flags 0x907fb |
// InfoPacket::flags:INFO_MOUSE yes |
// InfoPacket::flags:INFO_DISABLECTRLALTDEL yes |
// InfoPacket::flags:INFO_AUTOLOGON yes |
// InfoPacket::flags:INFO_UNICODE yes |
// InfoPacket::flags:INFO_MAXIMIZESHELL  yes |
// InfoPacket::flags:INFO_LOGONNOTIFY yes |
// InfoPacket::flags:INFO_COMPRESSION yes |
// InfoPacket::flags:CompressionTypeMask yes |
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
// InfoPacket::cbDomain 6 |
// InfoPacket::cbUserName 20 |
// InfoPacket::cbPassword 28 |
// InfoPacket::cbAlternateShell 0 |
// InfoPacket::cbWorkingDir 0 |
// InfoPacket::Domain RED |
// InfoPacket::UserName tartempion |
// InfoPacket::Password <hidden> |
// InfoPacket::AlternateShell  |
// InfoPacket::WorkingDir  |
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2 |
// InfoPacket::ExtendedInfoPacket::cbClientAddress 16 |
// InfoPacket::ExtendedInfoPacket::clientAddress 0.0.0.0 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 64 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\Windows\System32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 0 |
// InfoPacket::ExtendedInfoPacket::performanceFlags 8 |
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
// mod_rdp::send_client_info_pdu done |
// Rdp::Get license: username="tartempion@RED" |
// Rdp::Get license: username="tartempion@RED" |
// /* 0000 */ 0xff, 0x03, 0x10, 0x00, 0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,  // ................ |
// Unprocessed Capability Set is encountered. capabilitySetType=Share Capability Set(9) |
// Received from server General caps (24 bytes) |
// General caps::major 1 |
// General caps::minor 3 |
// General caps::protocol 512 |
// General caps::pad2octetA 0 |
// General caps::compression type 0 |
// General caps::extra flags 41d |
// General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED yes |
// General caps::extraflags:LONG_CREDENTIALS_SUPPORTED yes |
// General caps::extraflags:AUTORECONNECT_SUPPORTED yes |
// General caps::extraflags:ENC_SALTED_CHECKSUM yes |
// General caps::extraflags:NO_BITMAP_COMPRESSION_HDR yes |
// General caps::updateCapability 0 |
// General caps::remoteUnshare 0 |
// General caps::compressionLevel 0 |
// General caps::refreshRectSupport 1 |
// General caps::suppressOutputSupport 1 |
// Unprocessed Capability Set is encountered. capabilitySetType=Virtual Channel Capability Set(20) |
// Unprocessed Capability Set is encountered. capabilitySetType=Font Capability Set(14) |
// Received from server Bitmap caps (28 bytes) |
// Bitmap caps::preferredBitsPerPixel 16 |
// Bitmap caps::receive1BitPerPixel 1 |
// Bitmap caps::receive4BitsPerPixel 1 |
// Bitmap caps::receive8BitsPerPixel 1 |
// Bitmap caps::desktopWidth 1440 |
// Bitmap caps::desktopHeight 900 |
// Bitmap caps::pad2octets 0 |
// Bitmap caps::desktopResizeFlag 1 (yes) |
// Bitmap caps::bitmapCompressionFlag 1 yes |
// Bitmap caps::highColorFlags 0 |
// Bitmap caps::drawingFlags 30 |
// Bitmap caps::drawingFlags:DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY yes |
// Bitmap caps::drawingFlags:DRAW_ALLOW_COLOR_SUBSAMPLING yes |
// Bitmap caps::drawingFlags:DRAW_ALLOW_SKIP_ALPHA yes |
// Bitmap caps::multipleRectangleSupport 1 |
// Bitmap caps::pad2octetsB 0 |
// Unprocessed Capability Set is encountered. capabilitySetType=Bitmap Codecs Capability Set(29) |
// Received from server Order caps (88 bytes) |
// Order caps::terminalDescriptor 0 |
// Order caps::pad4octetsA 1000000 |
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
// Order caps::orderSupport[TS_NEG_DRAWNINEGRID_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_LINETO_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] 0 |
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
// Order caps::orderSupport[TS_NEG_INDEX_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex8] 0 |
// Order caps::orderSupport[UnusedIndex9] 0 |
// Order caps::orderSupport[UnusedIndex10] 0 |
// Order caps::orderSupport[UnusedIndex11] 0 |
// Order caps::textFlags 1697 |
// Order caps::orderSupportExFlags 6 |
// Order caps::pad4octetsB 1000000 |
// Order caps::desktopSaveSize 1000000 |
// Order caps::pad2octetsC 1 |
// Order caps::pad2octetsD 0 |
// Order caps::textANSICodePage 0 |
// Order caps::pad2octetsE 0 |
// Unprocessed Capability Set is encountered. capabilitySetType=Color Table Cache Capability Set(10) |
// Unprocessed Capability Set is encountered. capabilitySetType=Bitmap Cache Host Support Capability Set(18) |
// Unprocessed Capability Set is encountered. capabilitySetType=Pointer Capability Set(8) |
// Unprocessed Capability Set is encountered. capabilitySetType=Large Pointer Capability Set(27) |
// Received from server Input caps (88 bytes) |
// Input caps::inputFlags 0x175 |
// Input caps::pad2octetsA 0 |
// Input caps::keyboardLayout 0 |
// Input caps::keyboardType 0 |
// Input caps::keyboardSubType 0 |
// Input caps::keyboardFunctionKey 0 |
// Input caps::imeFileName 1530876720 |
// Unprocessed Capability Set is encountered. capabilitySetType=Remote Programs Capability Set(23) |
// Unprocessed Capability Set is encountered. capabilitySetType=Window List Capability Set(24) |
// Unprocessed Capability Set is encountered. capabilitySetType=Desktop Composition Extension Capability Set(25) |
// Unprocessed Capability Set is encountered. capabilitySetType=Multifragment Update Capability Set(26) |
// Unprocessed Capability Set is encountered. capabilitySetType=Surface Commands Capability Set(28) |
// Unprocessed Capability Set is encountered. capabilitySetType=Frame Acknowledge Capability Set(30) |
// mod_rdp::send_confirm_active |
// Sending to server General caps (24 bytes) |
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
// Sending to server Bitmap caps (28 bytes) |
// Bitmap caps::preferredBitsPerPixel 16 |
// Bitmap caps::receive1BitPerPixel 1 |
// Bitmap caps::receive4BitsPerPixel 1 |
// Bitmap caps::receive8BitsPerPixel 1 |
// Bitmap caps::desktopWidth 1440 |
// Bitmap caps::desktopHeight 900 |
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
// Order caps::orderSupport[TS_NEG_POLYLINE_INDEX] 1 |
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
// Order caps::orderSupportExFlags 0 |
// Order caps::pad4octetsB 0 |
// Order caps::desktopSaveSize 0 |
// Order caps::pad2octetsC 0 |
// Order caps::pad2octetsD 0 |
// Order caps::textANSICodePage 1252 |
// Order caps::pad2octetsE 0 |
// Sending to server BitmapCache2 caps (40 bytes) |
// BitmapCache2 caps::cacheFlags 3 |
// BitmapCache2 caps::pad1 0 |
// BitmapCache2 caps::numCellCache 3 |
// BitmapCache2 caps::bitampCache0CellInfo NumEntries=120 persistent=no |
// BitmapCache2 caps::bitampCache1CellInfo NumEntries=120 persistent=no |
// BitmapCache2 caps::bitampCache2CellInfo NumEntries=2553 persistent=yes |
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
// Sending to server Pointer caps (10 bytes) |
// Pointer caps::colorPointerFlag 1 |
// Pointer caps::colorPointerCacheSize 25 |
// Pointer caps::pointerCacheSize 25 |
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
// Input caps::imeFileName 1530679824 |
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
// GlyphCache caps::GlyphCache[9].CacheMaximumCellSize=256 |
// GlyphCache caps::FragCache 0 |
// GlyphCache caps::GlyphSupportLevel 0 |
// GlyphCache caps::pad2octets 0 |
// mod_rdp::send_confirm_active done |
// Waiting for answer to confirm active |
// mod_rdp::send_synchronise |
// mod_rdp::send_synchronise done |
// mod_rdp::send_control |
// mod_rdp::send_control done |
// mod_rdp::send_control |
// mod_rdp::send_control done |
// use rdp5 |
// mod_rdp::send_fonts |
// mod_rdp::send_fonts done |
// Resizing to 1440x900x16 |
// WAITING_SYNCHRONIZE |
// WAITING_CTL_COOPERATE |
// WAITING_GRANT_CONTROL_COOPERATE |
// PDUTYPE2_FONTMAP |
// enable_session_probe=no session_probe_launch_timeout=20000 session_probe_on_launch_failure=0 |
// Ask acl |
// sending reporting=CONNECTION_SUCCESSFUL:win2k12r2:Ok. |
// sending keepalive=ASK |
// ModuleManager::Creation of new mod 'RDP' suceeded |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'keepalive'='True' |
// process save session info : Logon extended info |
// process save session info : Logon Errors Info |
// ErrorNotificationData=LOGON_MSG_SESSION_CONTINUE(0xFFFFFFFE) "The logon process is continuing." ErrorNotificationType=LOGON_FAILED_OTHER(0x00000002) "The logon process failed. The reason for the failure can be deduced from the ErrorNotificationData field." |
// process save session info : Logon long |
// Logon Info Version 2 (data): Domain="RED" UserName="tartempion" SessionId=2 |
// Ask acl |
// sending reporting=OPEN_SESSION_SUCCESSFUL:win2k12r2:Ok. |
// sending keepalive=ASK |
// process save session info : Logon extended info |
// process save session info : Auto-reconnect cookie |
// LogonId=2 |
// 0000 24 cd ca 7c 84 1c 8d d1 82 5d 13 8f 92 8b ea 00 $..|.....]...... |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'keepalive'='True' |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x1c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x1c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" //................ |
// /* 0010 */ "\x02\x00\x00\x00\x1e\x00\x00\x00\x00\x00\x00\x00"                 //............ |
// Dump done on rdpdr (-1) n bytes |
// mod_rdp server clipboard PDU |
// mod_rdp server clipboard PDU: msgType=CB_CLIP_CAPS(7) |
// mod_rdp Server Clipboard Capabilities PDU |
// RDPECLIP::GeneralCapabilitySet: capabilitySetType=CB_CAPSTYPE_GENERAL(1) lengthCapability=12 version=CB_CAPS_VERSION_2(0x00000002) generalFlags=0x0000001E |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x1c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x1c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" //................ |
/* 0010 */ "\x02\x00\x00\x00\x1e\x00\x00\x00\x00\x00\x00\x00"                 //............ |
// Sent dumped on channel (-1) n bytes |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"                 //............ |
// Dump done on rdpdr (-1) n bytes |
// mod_rdp server clipboard PDU |
// mod_rdp server clipboard PDU: msgType=CB_MONITOR_READY(1) |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"                 //............ |
// Sent dumped on channel (-1) n bytes |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x13\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" //................ |
// /* 0010 */ "\x02\x00\x00\x00\x02\x00\x00\x00"                                 //........ |
// Dump done on rdpdr (-1) n bytes |
// mod_rdp client clipboard PDU |
// mod_rdp::send_to_mod_cliprdr_channel: client clipboard PDU: msgType=CB_CLIP_CAPS(7) |
// mod_rdp::send_to_mod_cliprdr_channel: Client Clipboard Capabilities PDU |
// RDPECLIP::GeneralCapabilitySet: capabilitySetType=CB_CAPSTYPE_GENERAL(1) lengthCapability=12 version=CB_CAPS_VERSION_2(0x00000002) generalFlags=0x00000002 |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x13\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" //................ |
/* 0010 */ "\x02\x00\x00\x00\x02\x00\x00\x00"                                 //........ |
// Sent dumped on channel (-1) n bytes |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x14\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x13\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x14\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x02\x00\x00\x00\x0c\x00\x00\x00\x01\x00\x00\x00\x00\x00\x0d\x00" //................ |
// /* 0010 */ "\x00\x00\x00\x00"                                                 //.... |
// Dump done on rdpdr (-1) n bytes |
// mod_rdp client clipboard PDU |
// mod_rdp::send_to_mod_cliprdr_channel: client clipboard PDU: msgType=CB_FORMAT_LIST(2) |
// mod_rdp::send_to_mod_cliprdr_channel: clipboard is fully disabled (c) |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x08\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x08\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x01\x00\x00\x00\x00\x00"                                 //........ |
// Sent dumped on channel (-1) n bytes |
// Front::Received fast-path PUD, remains=10 |
// Front::Received fast-path PUD, remains=10 |
// Ask acl |
// sending keepalive=ASK |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'keepalive'='True' |
// Front::Received fast-path PUD, remains=10 |
// process disconnect pdu : code =        c error=LOGOFF_BY_USER |
// mod::rdp::DisconnectProviderUltimatum received |
// mod::rdp::DisconnectProviderUltimatum: reason=RN_USER_REQUESTED [3] |
// Ask acl |
// sending reporting=SESSION_EXCEPTION:win2k12r2:Code=5003 |
// sending keepalive=ASK |
// Connection to server closed |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'keepalive'='True' |
// ===========> MODULE_NEXT |
// ----------> ACL next_module <-------- |
// ===========> MODULE_CLOSE |
// mod_rdp::disconnect() |
// SEND MCS DISCONNECT PROVIDER ULTIMATUM PDU |
// ~mod_rdp(): Recv bmp cache count  = 643 |
// ~mod_rdp(): Recv order count      = 3713 |
// ~mod_rdp(): Recv bmp update count = 0 |
// RDP Target (0): total_received=127155, total_sent=11573 |
// target_module=8 |
// ModuleManager::Creation of new mod 'INTERNAL::Close' |
// Widget_load: image file [/usr/local/share/rdpproxy/wablogoblue.png] is PNG file |
// WabCloseMod: Ending session in 600 seconds |
// ModuleManager::internal module Close ready |
// Socket Authentifier (4) : closing connection |
// Session::Client Session Disconnected |
} /* end outdata */;

const char indata[] =
{
// Front::Received fast-path PUD, remains=10 |
// asking for selector |
// Ask acl |
// sending login=RED\tartempion@win2k12r2:rdp:windows:x |
// sending target_device=ASK |
// sending target_login=ASK |
// sending proto_dest=ASK |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'target_login'='RED\tartempion' |
// receiving 'login'='x' |
// receiving 'module'='transitory' |
// receiving 'target_device'='win2k12r2' |
// ===========> MODULE_NEXT |
// ----------> ACL next_module <-------- |
// ===============> WAIT WITH CURRENT MODULE |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'forcemodule'='True' |
// ===========> MODULE_NEXT |
// ----------> ACL next_module <-------- |
// ===============> WAIT WITH CURRENT MODULE |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'mode_console'='allow' |
// receiving 'module'='RDP' |
// receiving 'proto_dest'='RDP' |
// receiving 'session_id'='SESSIONID-0000' |
// receiving 'target_host'='10.10.47.128' |
// receiving 'target_login'='RED\tartempion' |
// receiving 'target_port'='3389' |
// receiving 'timeclose'='2051218799' |
// receiving 'disable_tsk_switch_shortcuts'='False' |
// receiving 'timezone'='-7200' |
// receiving 'target_password'='<hidden>' |
// receiving 'password'='<hidden>' |
// receiving 'proxy_opt'='RDP_CLIPBOARD_FILE,RDP_COM_PORT,RDP_DRIVE,RDP_PRINTER,RDP_SMARTCARD' |
// ===========> MODULE_NEXT |
// ----------> ACL next_module <-------- |
// ===========> MODULE_RDP |
// target_module=5 |
// ModuleManager::Creation of new mod 'RDP' |
// connecting to 10.10.47.128:3389 |
// connection to 10.10.47.128:3389 succeeded : socket 6 |
// Creation of new mod 'RDP' |
// ModRDPParams target_user="RED\tartempion" |
// ModRDPParams target_password="<hidden>" |
// ModRDPParams target_host="10.10.47.128" |
// ModRDPParams client_address="0.0.0.0" |
// ModRDPParams client_name="rzh" |
// ModRDPParams enable_tls=yes |
// ModRDPParams enable_nla=yes |
// ModRDPParams enable_krb=no |
// ModRDPParams enable_fastpath=yes |
// ModRDPParams enable_mem3blt=yes |
// ModRDPParams enable_bitmap_update=yes |
// ModRDPParams enable_new_pointer=yes |
// ModRDPParams enable_glyph_cache=yes |
// ModRDPParams enable_session_probe=no |
// ModRDPParams session_probe_launch_timeout=20000 |
// ModRDPParams session_probe_on_launch_failure=0 |
// ModRDPParams session_probe_keepalive_timeout=0 |
// ModRDPParams dsiable_clipboard_log=0x0 |
// ModRDPParams enable_transparent_mode=no |
// ModRDPParams output_filename="" |
// ModRDPParams persistent_key_list_transport=<(nil)> |
// ModRDPParams transparent_recorder_transport=<(nil)> |
// ModRDPParams key_flags=0 |
// ModRDPParams acl=<0x61400000f570> |
// ModRDPParams auth_channel="" |
// ModRDPParams alternate_shell="" |
// ModRDPParams shell_working_directory="" |
// ModRDPParams target_application_account="" |
// ModRDPParams target_application_password="<hidden>" |
// ModRDPParams rdp_compression=4 |
// ModRDPParams error_message=<0x7fff5b45f478> |
// ModRDPParams disconnect_on_logon_user_change=no |
// ModRDPParams open_session_timeout=0 |
// ModRDPParams certificate_change_action=1 |
// ModRDPParams extra_orders=15,16,17,18,22 |
// ModRDPParams enable_persistent_disk_bitmap_cache=yes |
// ModRDPParams enable_cache_waiting_list=yes |
// ModRDPParams persist_bitmap_cache_on_disk=no |
// ModRDPParams password_printing_mode=0 |
// ModRDPParams allow_channels=<0x7fff5b45d4b0> |
// ModRDPParams deny_channels=<0x7fff5b45d4b8> |
// ModRDPParams remote_program=no |
// ModRDPParams server_redirection_support=no |
// ModRDPParams bogus_sc_net_size=yes |
// ModRDPParams client_device_announce_timeout=1000 |
// ModRDPParams proxy_managed_drives= |
// ModRDPParams verbose=0x44000001 |
// ModRDPParams cache_verbose=0x00000000 |
// RDP Extra orders="15,16,17,18,22" |
// RDP Extra orders number=15 |
// RDP Extra orders=MultiDstBlt |
// RDP Extra orders number=16 |
// RDP Extra orders=MultiPatBlt |
// RDP Extra orders number=17 |
// RDP Extra orders=MultiScrBlt |
// RDP Extra orders number=18 |
// RDP Extra orders=MultiOpaqueRect |
// RDP Extra orders number=22 |
// RDP Extra orders=Polyline |
// Remote RDP Server domain="RED" login="tartempion" host="rzh" |
// Server key layout is 40c |
// mod_rdp::Early TLS Security Exchange |
// RdpNego::NEGO_STATE_INITIAL |
// RdpNego::send_x224_connection_request_pdu |
// RdpNego::send_x224_connection_request_pdu done |
// mod_rdp::Early TLS Security Exchange |
// RdpNego::NEGO_STATE_NLA |
// RdpNego::recv_connection_confirm |
// NEG_RSP_TYPE=2 NEG_RSP_FLAGS=15 NEG_RSP_LENGTH=8 NEG_RSP_SELECTED_PROTOCOL=2 |
// CC Recv: PROTOCOL HYBRID |
// activating SSL |
// Client TLS start |
// SSL_connect() |
// SSL_get_peer_certificate() |
// nb1=1070 nb2=1070 |
// TLS::X509 existing::issuer=CN = testkrb.red.ifr.lan |
// TLS::X509 existing::subject=CN = testkrb.red.ifr.lan |
// TLS::X509 existing::fingerprint=3c:99:59:17:ae:5e:08:98:32:06:59:3e:1b:2c:97:b6:6e:26:d1:c8 |
// SocketTransport::X509_get_pubkey() |
// SocketTransport::i2d_PublicKey() |
// SocketTransport::i2d_PublicKey() |
// TLS::X509::issuer=CN = testkrb.red.ifr.lan |
// TLS::X509::subject=CN = testkrb.red.ifr.lan |
// TLS::X509::fingerprint=3c:99:59:17:ae:5e:08:98:32:06:59:3e:1b:2c:97:b6:6e:26:d1:c8 |
// SocketTransport::enable_tls() done |
// activating CREDSSP |
// Credssp: NTLM Authentication |
// Credssp: TSCredentials::emit() NegoToken |
// Credssp TSCredentials::recv() NEGOTOKENS |
// using /dev/urandom as random source |
// using /dev/urandom as random source |
// Credssp: TSCredentials::emit() NegoToken |
// Credssp: TSCredentials::emit() pubKeyAuth |
// Credssp TSCredentials::recv() PUBKEYAUTH |
// Credssp: TSCredentials::emit() AuthInfo |
// mod_rdp::Early TLS Security Exchange |
// Sending to Server GCC User Data CS_CORE (216 bytes) |
// cs_core::version [80004] RDP 5.0, 5.1, 5.2, and 6.0 clients) |
// cs_core::desktopWidth  = 1440 |
// cs_core::desktopHeight = 900 |
// cs_core::colorDepth    = [ca01] [RNS_UD_COLOR_8BPP] superseded by postBeta2ColorDepth |
// cs_core::SASSequence   = [aa03] [Unknown] |
// cs_core::keyboardLayout= 040c |
// cs_core::clientBuild   = 2600 |
// cs_core::clientName    = rzh |
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
// cs_core::serverSelectedProtocol = 2 |
// Sending to server GCC User Data CS_CLUSTER (12 bytes) |
// cs_cluster::flags [0000] |
// cs_cluster::redirectedSessionID = 0 |
// Sending to server GCC User Data CS_SECURITY (12 bytes) |
// CSSecGccUserData::encryptionMethods 3 |
// CSSecGccUserData::extEncryptionMethods 0 |
// Sending to server GCC User Data CS_NET (12 bytes) |
// cs_net::channelCount   = 1 |
// cs_net::channel ' cliprdr' [1004] INITIALIZED COMPRESS_RDP SHOW_PROTOCOL |
// mod_rdp::Basic Settings Exchange |
// GCC::UserData tag=0c01 length=16 |
// Received from server GCC User Data SC_CORE (16 bytes) |
// sc_core::version [80004] RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 servers) |
// sc_core::clientRequestedProtocols  = 3 |
// sc_core::earlyCapabilityFlags  = 1 |
// GCC::UserData tag=0c03 length=12 |
// Received from server GCC User Data SC_NET (12 bytes) |
// sc_net::MCSChannelId   = 1003 |
// sc_net::channelCount   = 1 |
// sc_net::channel[1004]::id = 1004 |
// sc_net::padding 2 bytes 0000 |
// GCC::UserData tag=0c02 length=12 |
// Received from server GCC User Data SC_SECURITY (12 bytes) |
// sc_security::encryptionMethod = 0 |
// sc_security::encryptionLevel  = 0 |
// No encryption |
// mod_rdp::Channel Connection |
// Send MCS::ErectDomainRequest |
// Send MCS::AttachUserRequest |
// mod_rdp::Channel Connection Attach User |
// mod_rdp::RDP Security Commencement |
// mod_rdp::Secure Settings Exchange |
// mod_rdp::send_client_info_pdu |
// send extended login info (RDP5-style) 907fb RED:tartempion |
// Send data request InfoPacket |
// InfoPacket::CodePage 0 |
// InfoPacket::flags 0x907fb |
// InfoPacket::flags:INFO_MOUSE yes |
// InfoPacket::flags:INFO_DISABLECTRLALTDEL yes |
// InfoPacket::flags:INFO_AUTOLOGON yes |
// InfoPacket::flags:INFO_UNICODE yes |
// InfoPacket::flags:INFO_MAXIMIZESHELL  yes |
// InfoPacket::flags:INFO_LOGONNOTIFY yes |
// InfoPacket::flags:INFO_COMPRESSION yes |
// InfoPacket::flags:CompressionTypeMask yes |
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
// InfoPacket::cbDomain 6 |
// InfoPacket::cbUserName 20 |
// InfoPacket::cbPassword 28 |
// InfoPacket::cbAlternateShell 0 |
// InfoPacket::cbWorkingDir 0 |
// InfoPacket::Domain RED |
// InfoPacket::UserName tartempion |
// InfoPacket::Password <hidden> |
// InfoPacket::AlternateShell  |
// InfoPacket::WorkingDir  |
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2 |
// InfoPacket::ExtendedInfoPacket::cbClientAddress 16 |
// InfoPacket::ExtendedInfoPacket::clientAddress 0.0.0.0 |
// InfoPacket::ExtendedInfoPacket::cbClientDir 64 |
// InfoPacket::ExtendedInfoPacket::clientDir C:\Windows\System32\mstscax.dll |
// InfoPacket::ExtendedInfoPacket::clientSessionId 0 |
// InfoPacket::ExtendedInfoPacket::performanceFlags 8 |
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
// mod_rdp::send_client_info_pdu done |
// Rdp::Get license: username="tartempion@RED" |
// Rdp::Get license: username="tartempion@RED" |
// /* 0000 */ 0xff, 0x03, 0x10, 0x00, 0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,  // ................ |
// Unprocessed Capability Set is encountered. capabilitySetType=Share Capability Set(9) |
// Received from server General caps (24 bytes) |
// General caps::major 1 |
// General caps::minor 3 |
// General caps::protocol 512 |
// General caps::pad2octetA 0 |
// General caps::compression type 0 |
// General caps::extra flags 41d |
// General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED yes |
// General caps::extraflags:LONG_CREDENTIALS_SUPPORTED yes |
// General caps::extraflags:AUTORECONNECT_SUPPORTED yes |
// General caps::extraflags:ENC_SALTED_CHECKSUM yes |
// General caps::extraflags:NO_BITMAP_COMPRESSION_HDR yes |
// General caps::updateCapability 0 |
// General caps::remoteUnshare 0 |
// General caps::compressionLevel 0 |
// General caps::refreshRectSupport 1 |
// General caps::suppressOutputSupport 1 |
// Unprocessed Capability Set is encountered. capabilitySetType=Virtual Channel Capability Set(20) |
// Unprocessed Capability Set is encountered. capabilitySetType=Font Capability Set(14) |
// Received from server Bitmap caps (28 bytes) |
// Bitmap caps::preferredBitsPerPixel 16 |
// Bitmap caps::receive1BitPerPixel 1 |
// Bitmap caps::receive4BitsPerPixel 1 |
// Bitmap caps::receive8BitsPerPixel 1 |
// Bitmap caps::desktopWidth 1440 |
// Bitmap caps::desktopHeight 900 |
// Bitmap caps::pad2octets 0 |
// Bitmap caps::desktopResizeFlag 1 (yes) |
// Bitmap caps::bitmapCompressionFlag 1 yes |
// Bitmap caps::highColorFlags 0 |
// Bitmap caps::drawingFlags 30 |
// Bitmap caps::drawingFlags:DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY yes |
// Bitmap caps::drawingFlags:DRAW_ALLOW_COLOR_SUBSAMPLING yes |
// Bitmap caps::drawingFlags:DRAW_ALLOW_SKIP_ALPHA yes |
// Bitmap caps::multipleRectangleSupport 1 |
// Bitmap caps::pad2octetsB 0 |
// Unprocessed Capability Set is encountered. capabilitySetType=Bitmap Codecs Capability Set(29) |
// Received from server Order caps (88 bytes) |
// Order caps::terminalDescriptor 0 |
// Order caps::pad4octetsA 1000000 |
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
// Order caps::orderSupport[TS_NEG_DRAWNINEGRID_INDEX] 0 |
// Order caps::orderSupport[TS_NEG_LINETO_INDEX] 1 |
// Order caps::orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] 0 |
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
// Order caps::orderSupport[TS_NEG_INDEX_INDEX] 1 |
// Order caps::orderSupport[UnusedIndex8] 0 |
// Order caps::orderSupport[UnusedIndex9] 0 |
// Order caps::orderSupport[UnusedIndex10] 0 |
// Order caps::orderSupport[UnusedIndex11] 0 |
// Order caps::textFlags 1697 |
// Order caps::orderSupportExFlags 6 |
// Order caps::pad4octetsB 1000000 |
// Order caps::desktopSaveSize 1000000 |
// Order caps::pad2octetsC 1 |
// Order caps::pad2octetsD 0 |
// Order caps::textANSICodePage 0 |
// Order caps::pad2octetsE 0 |
// Unprocessed Capability Set is encountered. capabilitySetType=Color Table Cache Capability Set(10) |
// Unprocessed Capability Set is encountered. capabilitySetType=Bitmap Cache Host Support Capability Set(18) |
// Unprocessed Capability Set is encountered. capabilitySetType=Pointer Capability Set(8) |
// Unprocessed Capability Set is encountered. capabilitySetType=Large Pointer Capability Set(27) |
// Received from server Input caps (88 bytes) |
// Input caps::inputFlags 0x175 |
// Input caps::pad2octetsA 0 |
// Input caps::keyboardLayout 0 |
// Input caps::keyboardType 0 |
// Input caps::keyboardSubType 0 |
// Input caps::keyboardFunctionKey 0 |
// Input caps::imeFileName 1530876720 |
// Unprocessed Capability Set is encountered. capabilitySetType=Remote Programs Capability Set(23) |
// Unprocessed Capability Set is encountered. capabilitySetType=Window List Capability Set(24) |
// Unprocessed Capability Set is encountered. capabilitySetType=Desktop Composition Extension Capability Set(25) |
// Unprocessed Capability Set is encountered. capabilitySetType=Multifragment Update Capability Set(26) |
// Unprocessed Capability Set is encountered. capabilitySetType=Surface Commands Capability Set(28) |
// Unprocessed Capability Set is encountered. capabilitySetType=Frame Acknowledge Capability Set(30) |
// mod_rdp::send_confirm_active |
// Sending to server General caps (24 bytes) |
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
// Sending to server Bitmap caps (28 bytes) |
// Bitmap caps::preferredBitsPerPixel 16 |
// Bitmap caps::receive1BitPerPixel 1 |
// Bitmap caps::receive4BitsPerPixel 1 |
// Bitmap caps::receive8BitsPerPixel 1 |
// Bitmap caps::desktopWidth 1440 |
// Bitmap caps::desktopHeight 900 |
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
// Order caps::orderSupport[TS_NEG_POLYLINE_INDEX] 1 |
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
// Order caps::orderSupportExFlags 0 |
// Order caps::pad4octetsB 0 |
// Order caps::desktopSaveSize 0 |
// Order caps::pad2octetsC 0 |
// Order caps::pad2octetsD 0 |
// Order caps::textANSICodePage 1252 |
// Order caps::pad2octetsE 0 |
// Sending to server BitmapCache2 caps (40 bytes) |
// BitmapCache2 caps::cacheFlags 3 |
// BitmapCache2 caps::pad1 0 |
// BitmapCache2 caps::numCellCache 3 |
// BitmapCache2 caps::bitampCache0CellInfo NumEntries=120 persistent=no |
// BitmapCache2 caps::bitampCache1CellInfo NumEntries=120 persistent=no |
// BitmapCache2 caps::bitampCache2CellInfo NumEntries=2553 persistent=yes |
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
// Sending to server Pointer caps (10 bytes) |
// Pointer caps::colorPointerFlag 1 |
// Pointer caps::colorPointerCacheSize 25 |
// Pointer caps::pointerCacheSize 25 |
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
// Input caps::imeFileName 1530679824 |
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
// GlyphCache caps::GlyphCache[9].CacheMaximumCellSize=256 |
// GlyphCache caps::FragCache 0 |
// GlyphCache caps::GlyphSupportLevel 0 |
// GlyphCache caps::pad2octets 0 |
// mod_rdp::send_confirm_active done |
// Waiting for answer to confirm active |
// mod_rdp::send_synchronise |
// mod_rdp::send_synchronise done |
// mod_rdp::send_control |
// mod_rdp::send_control done |
// mod_rdp::send_control |
// mod_rdp::send_control done |
// use rdp5 |
// mod_rdp::send_fonts |
// mod_rdp::send_fonts done |
// Resizing to 1440x900x16 |
// WAITING_SYNCHRONIZE |
// WAITING_CTL_COOPERATE |
// WAITING_GRANT_CONTROL_COOPERATE |
// PDUTYPE2_FONTMAP |
// enable_session_probe=no session_probe_launch_timeout=20000 session_probe_on_launch_failure=0 |
// Ask acl |
// sending reporting=CONNECTION_SUCCESSFUL:win2k12r2:Ok. |
// sending keepalive=ASK |
// ModuleManager::Creation of new mod 'RDP' suceeded |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'keepalive'='True' |
// process save session info : Logon extended info |
// process save session info : Logon Errors Info |
// ErrorNotificationData=LOGON_MSG_SESSION_CONTINUE(0xFFFFFFFE) "The logon process is continuing." ErrorNotificationType=LOGON_FAILED_OTHER(0x00000002) "The logon process failed. The reason for the failure can be deduced from the ErrorNotificationData field." |
// process save session info : Logon long |
// Logon Info Version 2 (data): Domain="RED" UserName="tartempion" SessionId=2 |
// Ask acl |
// sending reporting=OPEN_SESSION_SUCCESSFUL:win2k12r2:Ok. |
// sending keepalive=ASK |
// process save session info : Logon extended info |
// process save session info : Auto-reconnect cookie |
// LogonId=2 |
// 0000 24 cd ca 7c 84 1c 8d d1 82 5d 13 8f 92 8b ea 00 $..|.....]...... |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'keepalive'='True' |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x1c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x1c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" //................ |
/* 0010 */ "\x02\x00\x00\x00\x1e\x00\x00\x00\x00\x00\x00\x00"                 //............ |
// Dump done on rdpdr (-1) n bytes |
// mod_rdp server clipboard PDU |
// mod_rdp server clipboard PDU: msgType=CB_CLIP_CAPS(7) |
// mod_rdp Server Clipboard Capabilities PDU |
// RDPECLIP::GeneralCapabilitySet: capabilitySetType=CB_CAPSTYPE_GENERAL(1) lengthCapability=12 version=CB_CAPS_VERSION_2(0x00000002) generalFlags=0x0000001E |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x1c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x1c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" //................ |
// /* 0010 */ "\x02\x00\x00\x00\x1e\x00\x00\x00\x00\x00\x00\x00"                 //............ |
// Sent dumped on channel (-1) n bytes |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"                 //............ |
// Dump done on rdpdr (-1) n bytes |
// mod_rdp server clipboard PDU |
// mod_rdp server clipboard PDU: msgType=CB_MONITOR_READY(1) |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"                 //............ |
// Sent dumped on channel (-1) n bytes |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x13\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" //................ |
/* 0010 */ "\x02\x00\x00\x00\x02\x00\x00\x00"                                 //........ |
// Dump done on rdpdr (-1) n bytes |
// mod_rdp client clipboard PDU |
// mod_rdp::send_to_mod_cliprdr_channel: client clipboard PDU: msgType=CB_CLIP_CAPS(7) |
// mod_rdp::send_to_mod_cliprdr_channel: Client Clipboard Capabilities PDU |
// RDPECLIP::GeneralCapabilitySet: capabilitySetType=CB_CAPSTYPE_GENERAL(1) lengthCapability=12 version=CB_CAPS_VERSION_2(0x00000002) generalFlags=0x00000002 |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x13\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" //................ |
// /* 0010 */ "\x02\x00\x00\x00\x02\x00\x00\x00"                                 //........ |
// Sent dumped on channel (-1) n bytes |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x14\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x13\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x14\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x02\x00\x00\x00\x0c\x00\x00\x00\x01\x00\x00\x00\x00\x00\x0d\x00" //................ |
/* 0010 */ "\x00\x00\x00\x00"                                                 //.... |
// Dump done on rdpdr (-1) n bytes |
// mod_rdp client clipboard PDU |
// mod_rdp::send_to_mod_cliprdr_channel: client clipboard PDU: msgType=CB_FORMAT_LIST(2) |
// mod_rdp::send_to_mod_cliprdr_channel: clipboard is fully disabled (c) |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x08\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x08\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x01\x00\x00\x00\x00\x00"                                 //........ |
// Sent dumped on channel (-1) n bytes |
// Front::Received fast-path PUD, remains=10 |
// Front::Received fast-path PUD, remains=10 |
// Ask acl |
// sending keepalive=ASK |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'keepalive'='True' |
// Front::Received fast-path PUD, remains=10 |
// process disconnect pdu : code =        c error=LOGOFF_BY_USER |
// mod::rdp::DisconnectProviderUltimatum received |
// mod::rdp::DisconnectProviderUltimatum: reason=RN_USER_REQUESTED [3] |
// Ask acl |
// sending reporting=SESSION_EXCEPTION:win2k12r2:Code=5003 |
// sending keepalive=ASK |
// Connection to server closed |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'keepalive'='True' |
// ===========> MODULE_NEXT |
// ----------> ACL next_module <-------- |
// ===========> MODULE_CLOSE |
// mod_rdp::disconnect() |
// SEND MCS DISCONNECT PROVIDER ULTIMATUM PDU |
// ~mod_rdp(): Recv bmp cache count  = 643 |
// ~mod_rdp(): Recv order count      = 3713 |
// ~mod_rdp(): Recv bmp update count = 0 |
// RDP Target (0): total_received=127155, total_sent=11573 |
// target_module=8 |
// ModuleManager::Creation of new mod 'INTERNAL::Close' |
// Widget_load: image file [/usr/local/share/rdpproxy/wablogoblue.png] is PNG file |
// WabCloseMod: Ending session in 600 seconds |
// ModuleManager::internal module Close ready |
// Socket Authentifier (4) : closing connection |
// Session::Client Session Disconnected |
} /* end indata */;

