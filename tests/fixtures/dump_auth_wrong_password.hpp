// ReDemPtion 0.9.283-future starting
// Listen: binding socket 4 on 0.0.0.0:3389
// Listen: listening on socket 4
// Setting new session socket to 5
// src=127.0.0.1 sport=45965 dst=127.0.0.1 dport=3389
// New session on 5 (pid=8902) from 127.0.0.1 to 127.0.0.1
// connecting to 127.0.0.1:3450
// connection to 127.0.0.1:3450 succeeded : socket 4
// auth::SessionManager
// Reading font file /usr/local/share/rdpproxy/dejavu_11.fv1
// font name <DejaVu Sans> size <11>
// Session::session_main_loop() starting
// Session::-------------- Initializing client session -------------------
// Front::incoming:CONNECTION_INITIATION
// Fallback to legacy security protocol
// -----------------> Front::TLS Support not Enabled
// GCC::UserData tag=c001 length=212
// GCC::UserData tag=c004 length=12
// GCC::UserData tag=c002 length=12
// GCC::UserData tag=c003 length=32
// using /dev/urandom as random source
// Front::incoming::Secure Settings Exchange
// RDP-5 Style logon
// Receiving from client InfoPacket
// InfoPacket::CodePage 0
// InfoPacket::flags 0x133
// InfoPacket::flags:INFO_MOUSE yes
// InfoPacket::flags:INFO_DISABLECTRLALTDEL yes
// InfoPacket::flags:INFO_AUTOLOGON no
// InfoPacket::flags:INFO_UNICODE yes
// InfoPacket::flags:INFO_MAXIMIZESHELL  yes
// InfoPacket::flags:INFO_LOGONNOTIFY no
// InfoPacket::flags:INFO_COMPRESSION no
// InfoPacket::flags:CompressionTypeMask no
// InfoPacket::flags:INFO_ENABLEWINDOWSKEY  yes
// InfoPacket::flags:INFO_REMOTECONSOLEAUDIO no
// InfoPacket::flags:FORCE_ENCRYPTED_CS_PDU no
// InfoPacket::flags:INFO_RAIL no
// InfoPacket::flags:INFO_LOGONERRORS no
// InfoPacket::flags:INFO_MOUSE_HAS_WHEEL no
// InfoPacket::flags:INFO_PASSWORD_IS_SC_PIN no
// InfoPacket::flags:INFO_NOAUDIOPLAYBACK no
// InfoPacket::flags:INFO_USING_SAVED_CREDS no
// InfoPacket::flags:RNS_INFO_AUDIOCAPTURE no
// InfoPacket::flags:RNS_INFO_VIDEO_DISABLE no
// InfoPacket::cbDomain 2
// InfoPacket::cbUserName 10
// InfoPacket::cbPassword 2
// InfoPacket::cbAlternateShell 2
// InfoPacket::cbWorkingDir 2
// InfoPacket::Domain 
// InfoPacket::UserName mtan
// InfoPacket::Password <hidden>
// InfoPacket::AlternateShell 
// InfoPacket::WorkingDir 
// InfoPacket::ExtendedInfoPacket::clientAddressFamily 2
// InfoPacket::ExtendedInfoPacket::cbClientAddress 20
// InfoPacket::ExtendedInfoPacket::clientAddress 127.0.0.1
// InfoPacket::ExtendedInfoPacket::cbClientDir 60
// InfoPacket::ExtendedInfoPacket::clientDir C:\WINNT\System32\mstscax.dll
// InfoPacket::ExtendedInfoPacket::clientSessionId 4294967294
// InfoPacket::ExtendedInfoPacket::performanceFlags 7
// InfoPacket::ExtendedInfoPacket::cbAutoReconnectLen 0
// InfoPacket::ExtendedInfoPacket::autoReconnectCookie 
// InfoPacket::ExtendedInfoPacket::reserved1 0
// InfoPacket::ExtendedInfoPacket::reserved2 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::Bias 120
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardName GMT Standard Time
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wYear 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMonth 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDayOfWeek 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wDay 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wHour 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMinute 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wSecond 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardDate.wMilliseconds 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::StandardBias 60
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightName GMT Daylight Time
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wYear 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMonth 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDayOfWeek 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wDay 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wHour 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMinute 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wSecond 0
// InfoPacket::ExtendedInfoPacket::ClientTimeZone::DaylightDate.wMilliseconds 0
// Front::incoming::ACTIVATED (new license request)
// Session::session_setup_mod(target_module=8, submodule=0)
// Session::Creation of new mod 'CLI parse'
// asking for selector
// Session::Creation of new mod 'CLI parse' suceeded




const char outdata[] =
{
// Session::-------------- Running -------------------
// Session::back_event fired
// signal in transitory mode
// Session::back event end module
// Session::starting next module
// auth::ask_next_module
// auth::ask_next_module default state
// sending proxy_type=RDP
// sending display_message=
// sending accept_message=
// sending ip_client=127.0.0.1
// sending ip_target=127.0.0.1
// sending login=mtan
// sending password=ASK
// sending target_login=ASK
// sending target_device=ASK
// sending proto_dest=ASK
// sending selector=ASK
// sending selector_group_filter=
// sending selector_device_filter=
// sending selector_lines_per_page=20
// sending selector_current_page=1
// sending target_password=ASK
// sending width=800
// sending height=600
// sending bpp=24
// sending real_target_device=
// Data size sending without header 355
// Sending on Authentifier (4) 359 bytes
/* 0000 */ "\x00\x00\x01\x63\x70\x72\x6f\x78\x79\x5f\x74\x79\x70\x65\x0a\x21" //...cproxy_type.!
/* 0010 */ "\x52\x44\x50\x0a\x64\x69\x73\x70\x6c\x61\x79\x5f\x6d\x65\x73\x73" //RDP.display_mess
/* 0020 */ "\x61\x67\x65\x0a\x21\x0a\x61\x63\x63\x65\x70\x74\x5f\x6d\x65\x73" //age.!.accept_mes
/* 0030 */ "\x73\x61\x67\x65\x0a\x21\x0a\x69\x70\x5f\x63\x6c\x69\x65\x6e\x74" //sage.!.ip_client
/* 0040 */ "\x0a\x21\x31\x32\x37\x2e\x30\x2e\x30\x2e\x31\x0a\x69\x70\x5f\x74" //.!127.0.0.1.ip_t
/* 0050 */ "\x61\x72\x67\x65\x74\x0a\x21\x31\x32\x37\x2e\x30\x2e\x30\x2e\x31" //arget.!127.0.0.1
/* 0060 */ "\x0a\x6c\x6f\x67\x69\x6e\x0a\x21\x6d\x74\x61\x6e\x0a\x70\x61\x73" //.login.!mtan.pas
/* 0070 */ "\x73\x77\x6f\x72\x64\x0a\x41\x53\x4b\x0a\x74\x61\x72\x67\x65\x74" //sword.ASK.target
/* 0080 */ "\x5f\x6c\x6f\x67\x69\x6e\x0a\x41\x53\x4b\x0a\x74\x61\x72\x67\x65" //_login.ASK.targe
/* 0090 */ "\x74\x5f\x64\x65\x76\x69\x63\x65\x0a\x41\x53\x4b\x0a\x70\x72\x6f" //t_device.ASK.pro
/* 00a0 */ "\x74\x6f\x5f\x64\x65\x73\x74\x0a\x41\x53\x4b\x0a\x73\x65\x6c\x65" //to_dest.ASK.sele
/* 00b0 */ "\x63\x74\x6f\x72\x0a\x41\x53\x4b\x0a\x73\x65\x6c\x65\x63\x74\x6f" //ctor.ASK.selecto
/* 00c0 */ "\x72\x5f\x67\x72\x6f\x75\x70\x5f\x66\x69\x6c\x74\x65\x72\x0a\x21" //r_group_filter.!
/* 00d0 */ "\x0a\x73\x65\x6c\x65\x63\x74\x6f\x72\x5f\x64\x65\x76\x69\x63\x65" //.selector_device
/* 00e0 */ "\x5f\x66\x69\x6c\x74\x65\x72\x0a\x21\x0a\x73\x65\x6c\x65\x63\x74" //_filter.!.select
/* 00f0 */ "\x6f\x72\x5f\x6c\x69\x6e\x65\x73\x5f\x70\x65\x72\x5f\x70\x61\x67" //or_lines_per_pag
/* 0100 */ "\x65\x0a\x21\x32\x30\x0a\x73\x65\x6c\x65\x63\x74\x6f\x72\x5f\x63" //e.!20.selector_c
/* 0110 */ "\x75\x72\x72\x65\x6e\x74\x5f\x70\x61\x67\x65\x0a\x21\x31\x0a\x74" //urrent_page.!1.t
/* 0120 */ "\x61\x72\x67\x65\x74\x5f\x70\x61\x73\x73\x77\x6f\x72\x64\x0a\x41" //arget_password.A
/* 0130 */ "\x53\x4b\x0a\x77\x69\x64\x74\x68\x0a\x21\x38\x30\x30\x0a\x68\x65" //SK.width.!800.he
/* 0140 */ "\x69\x67\x68\x74\x0a\x21\x36\x30\x30\x0a\x62\x70\x70\x0a\x21\x32" //ight.!600.bpp.!2
/* 0150 */ "\x34\x0a\x72\x65\x61\x6c\x5f\x74\x61\x72\x67\x65\x74\x5f\x64\x65" //4.real_target_de
/* 0160 */ "\x76\x69\x63\x65\x0a\x21\x0a"                                     //vice.!.
// Sent dumped on Authentifier (4) 359 bytes
// session::next_state 1








// Session::-------------- Running -------------------
// Session::back_event fired
// signal in transitory mode
// Session::back event end module
// Session::starting next module
// auth::ask_next_module
// auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS state
// auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_AUTH_USER=False
// auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_PASSWORD=True
// auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_PASSWORD is asked
// session::next_state 5
// Session::session_setup_mod(target_module=5, submodule=11)
// Session::Creation of internal module 'Login'
// loading bitmap /usr/local/share/rdpproxy/ad24b.bmp
// loading file 140 x 140 x 8
// loading bitmap /usr/local/share/rdpproxy/xrdp24b-redemption.bmp
// loading file 256 x 125 x 24
// Session::internal module Login ready
// Session::back_event fired




// 268

// 8
// asking for selector
// Session::back_event fired
// Session::back event end module
// Session::starting next module
// auth::ask_next_module
// auth::ask_next_module MOD_STATE_DONE_LOGIN state
// sending proxy_type=RDP
// sending display_message=
// sending accept_message=
// sending ip_client=127.0.0.1
// sending ip_target=127.0.0.1
// sending login=x
// sending password=<hidden>
// sending target_login=ASK
// sending target_device=ASK
// sending proto_dest=ASK
// sending selector=ASK
// sending selector_group_filter=
// sending selector_device_filter=
// sending selector_lines_per_page=20
// sending selector_current_page=1
// sending target_password=ASK
// sending width=800
// sending height=600
// sending bpp=24
// sending real_target_device=
// Data size sending without header 355
// Sending on Authentifier (4) 359 bytes
/* 0000 */ "\x00\x00\x01\x63\x70\x72\x6f\x78\x79\x5f\x74\x79\x70\x65\x0a\x21" //...cproxy_type.!
/* 0010 */ "\x52\x44\x50\x0a\x64\x69\x73\x70\x6c\x61\x79\x5f\x6d\x65\x73\x73" //RDP.display_mess
/* 0020 */ "\x61\x67\x65\x0a\x21\x0a\x61\x63\x63\x65\x70\x74\x5f\x6d\x65\x73" //age.!.accept_mes
/* 0030 */ "\x73\x61\x67\x65\x0a\x21\x0a\x69\x70\x5f\x63\x6c\x69\x65\x6e\x74" //sage.!.ip_client
/* 0040 */ "\x0a\x21\x31\x32\x37\x2e\x30\x2e\x30\x2e\x31\x0a\x69\x70\x5f\x74" //.!127.0.0.1.ip_t
/* 0050 */ "\x61\x72\x67\x65\x74\x0a\x21\x31\x32\x37\x2e\x30\x2e\x30\x2e\x31" //arget.!127.0.0.1
/* 0060 */ "\x0a\x6c\x6f\x67\x69\x6e\x0a\x21\x78\x0a\x70\x61\x73\x73\x77\x6f" //.login.!x.passwo
/* 0070 */ "\x72\x64\x0a\x21\x77\x72\x6f\x6e\x67\x0a\x74\x61\x72\x67\x65\x74" //rd.!wrong.target
/* 0080 */ "\x5f\x6c\x6f\x67\x69\x6e\x0a\x41\x53\x4b\x0a\x74\x61\x72\x67\x65" //_login.ASK.targe
/* 0090 */ "\x74\x5f\x64\x65\x76\x69\x63\x65\x0a\x41\x53\x4b\x0a\x70\x72\x6f" //t_device.ASK.pro
/* 00a0 */ "\x74\x6f\x5f\x64\x65\x73\x74\x0a\x41\x53\x4b\x0a\x73\x65\x6c\x65" //to_dest.ASK.sele
/* 00b0 */ "\x63\x74\x6f\x72\x0a\x41\x53\x4b\x0a\x73\x65\x6c\x65\x63\x74\x6f" //ctor.ASK.selecto
/* 00c0 */ "\x72\x5f\x67\x72\x6f\x75\x70\x5f\x66\x69\x6c\x74\x65\x72\x0a\x21" //r_group_filter.!
/* 00d0 */ "\x0a\x73\x65\x6c\x65\x63\x74\x6f\x72\x5f\x64\x65\x76\x69\x63\x65" //.selector_device
/* 00e0 */ "\x5f\x66\x69\x6c\x74\x65\x72\x0a\x21\x0a\x73\x65\x6c\x65\x63\x74" //_filter.!.select
/* 00f0 */ "\x6f\x72\x5f\x6c\x69\x6e\x65\x73\x5f\x70\x65\x72\x5f\x70\x61\x67" //or_lines_per_pag
/* 0100 */ "\x65\x0a\x21\x32\x30\x0a\x73\x65\x6c\x65\x63\x74\x6f\x72\x5f\x63" //e.!20.selector_c
/* 0110 */ "\x75\x72\x72\x65\x6e\x74\x5f\x70\x61\x67\x65\x0a\x21\x31\x0a\x74" //urrent_page.!1.t
/* 0120 */ "\x61\x72\x67\x65\x74\x5f\x70\x61\x73\x73\x77\x6f\x72\x64\x0a\x41" //arget_password.A
/* 0130 */ "\x53\x4b\x0a\x77\x69\x64\x74\x68\x0a\x21\x38\x30\x30\x0a\x68\x65" //SK.width.!800.he
/* 0140 */ "\x69\x67\x68\x74\x0a\x21\x36\x30\x30\x0a\x62\x70\x70\x0a\x21\x32" //ight.!600.bpp.!2
/* 0150 */ "\x34\x0a\x72\x65\x61\x6c\x5f\x74\x61\x72\x67\x65\x74\x5f\x64\x65" //4.real_target_de
/* 0160 */ "\x76\x69\x63\x65\x0a\x21\x0a"                                     //vice.!.
// Sent dumped on Authentifier (4) 359 bytes
// session::next_state 1



};

const char indata[] =
{
// Session::-------------- Waiting for authentifier -------------------
// Socket Authentifier (4) receiving 4 bytes
// Recv done on Authentifier (4) 4 bytes
/* 0000 */ "\x00\x00\x01\xd3"                                                 //....
// Dump done on Authentifier (4) 4 bytes
// Socket Authentifier (4) receiving 467 bytes
// Recv done on Authentifier (4) 467 bytes
/* 0000 */ "\x69\x70\x5f\x74\x61\x72\x67\x65\x74\x0a\x21\x31\x32\x37\x2e\x30" //ip_target.!127.0
/* 0010 */ "\x2e\x30\x2e\x31\x0a\x64\x69\x73\x70\x6c\x61\x79\x5f\x6d\x65\x73" //.0.1.display_mes
/* 0020 */ "\x73\x61\x67\x65\x0a\x21\x0a\x73\x65\x6c\x65\x63\x74\x6f\x72\x5f" //sage.!.selector_
/* 0030 */ "\x67\x72\x6f\x75\x70\x5f\x66\x69\x6c\x74\x65\x72\x0a\x21\x0a\x69" //group_filter.!.i
/* 0040 */ "\x70\x5f\x63\x6c\x69\x65\x6e\x74\x0a\x21\x31\x32\x37\x2e\x30\x2e" //p_client.!127.0.
/* 0050 */ "\x30\x2e\x31\x0a\x66\x69\x6c\x65\x5f\x65\x6e\x63\x72\x79\x70\x74" //0.1.file_encrypt
/* 0060 */ "\x69\x6f\x6e\x0a\x21\x74\x72\x75\x65\x0a\x73\x65\x6c\x65\x63\x74" //ion.!true.select
/* 0070 */ "\x6f\x72\x0a\x41\x53\x4b\x0a\x74\x61\x72\x67\x65\x74\x5f\x64\x65" //or.ASK.target_de
/* 0080 */ "\x76\x69\x63\x65\x0a\x41\x53\x4b\x0a\x73\x65\x6c\x65\x63\x74\x6f" //vice.ASK.selecto
/* 0090 */ "\x72\x5f\x64\x65\x76\x69\x63\x65\x5f\x66\x69\x6c\x74\x65\x72\x0a" //r_device_filter.
/* 00a0 */ "\x21\x0a\x70\x61\x73\x73\x77\x6f\x72\x64\x0a\x41\x53\x4b\x0a\x74" //!.password.ASK.t
/* 00b0 */ "\x61\x72\x67\x65\x74\x5f\x70\x61\x73\x73\x77\x6f\x72\x64\x0a\x41" //arget_password.A
/* 00c0 */ "\x53\x4b\x0a\x6b\x65\x65\x70\x61\x6c\x69\x76\x65\x0a\x21\x74\x72" //SK.keepalive.!tr
/* 00d0 */ "\x75\x65\x0a\x61\x63\x63\x65\x70\x74\x5f\x6d\x65\x73\x73\x61\x67" //ue.accept_messag
/* 00e0 */ "\x65\x0a\x21\x0a\x74\x72\x61\x6e\x73\x5f\x6f\x6b\x0a\x21\x4f\x75" //e.!.trans_ok.!Ou
/* 00f0 */ "\x69\x0a\x61\x75\x74\x68\x65\x6e\x74\x69\x63\x61\x74\x65\x64\x0a" //i.authenticated.
/* 0100 */ "\x21\x66\x61\x6c\x73\x65\x0a\x74\x61\x72\x67\x65\x74\x5f\x6c\x6f" //!false.target_lo
/* 0110 */ "\x67\x69\x6e\x0a\x41\x53\x4b\x0a\x68\x65\x69\x67\x68\x74\x0a\x21" //gin.ASK.height.!
/* 0120 */ "\x31\x30\x32\x34\x0a\x77\x69\x64\x74\x68\x0a\x21\x31\x32\x38\x30" //1024.width.!1280
/* 0130 */ "\x0a\x74\x72\x61\x6e\x73\x5f\x63\x61\x6e\x63\x65\x6c\x0a\x21\x41" //.trans_cancel.!A
/* 0140 */ "\x6e\x6e\x75\x6c\x65\x72\x0a\x63\x6c\x69\x70\x62\x6f\x61\x72\x64" //nnuler.clipboard
/* 0150 */ "\x0a\x21\x74\x72\x75\x65\x0a\x73\x65\x6c\x65\x63\x74\x6f\x72\x5f" //.!true.selector_
/* 0160 */ "\x63\x75\x72\x72\x65\x6e\x74\x5f\x70\x61\x67\x65\x0a\x21\x31\x0a" //current_page.!1.
/* 0170 */ "\x62\x70\x70\x0a\x21\x38\x0a\x73\x65\x6c\x65\x63\x74\x6f\x72\x5f" //bpp.!8.selector_
/* 0180 */ "\x6c\x69\x6e\x65\x73\x5f\x70\x65\x72\x5f\x70\x61\x67\x65\x0a\x21" //lines_per_page.!
/* 0190 */ "\x32\x30\x0a\x6c\x6f\x67\x69\x6e\x0a\x21\x6d\x74\x61\x6e\x0a\x70" //20.login.!mtan.p
/* 01a0 */ "\x72\x6f\x74\x6f\x5f\x64\x65\x73\x74\x0a\x41\x53\x4b\x0a\x70\x72" //roto_dest.ASK.pr
/* 01b0 */ "\x6f\x78\x79\x5f\x74\x79\x70\x65\x0a\x21\x52\x44\x50\x0a\x72\x65" //oxy_type.!RDP.re
/* 01c0 */ "\x61\x6c\x5f\x74\x61\x72\x67\x65\x74\x5f\x64\x65\x76\x69\x63\x65" //al_target_device
/* 01d0 */ "\x0a\x21\x0a"                                                     //.!.
// Dump done on Authentifier (4) 467 bytes
// Data size received without header 467
// auth::in_items
// receiving 'ip_target'='127.0.0.1'
// receiving 'display_message'=''
// receiving 'selector_group_filter'=''
// receiving 'ip_client'='127.0.0.1'
// receiving 'file_encryption'='True'
// receiving ASK 'selector'
// receiving ASK 'target_device'
// receiving 'selector_device_filter'=''
// receiving ASK 'password'
// receiving ASK 'target_password'
// receiving 'keepalive'='True'
// receiving 'accept_message'=''
// receiving 'trans_ok'='Oui'
// receiving 'authenticated'='False'
// receiving ASK 'target_login'
// receiving 'height'='1024'
// receiving 'width'='1280'
// receiving 'trans_cancel'='Annuler'
// receiving 'clipboard'='True'
// receiving 'selector_current_page'='1'
// receiving 'bpp'='8'
// receiving 'selector_lines_per_page'='20'
// receiving 'login'='mtan'
// receiving ASK 'proto_dest'
// receiving 'proxy_type'='RDP'
// receiving 'real_target_device'=''
// SESSION_ID = 


// Session::-------------- Waiting for authentifier -------------------
// Socket Authentifier (4) receiving 4 bytes
// Recv done on Authentifier (4) 4 bytes
/* 0000 */ "\x00\x00\x01\xd0"                                                 //....
// Dump done on Authentifier (4) 4 bytes
// Socket Authentifier (4) receiving 464 bytes
// Recv done on Authentifier (4) 464 bytes
/* 0000 */ "\x69\x70\x5f\x74\x61\x72\x67\x65\x74\x0a\x21\x31\x32\x37\x2e\x30" //ip_target.!127.0
/* 0010 */ "\x2e\x30\x2e\x31\x0a\x64\x69\x73\x70\x6c\x61\x79\x5f\x6d\x65\x73" //.0.1.display_mes
/* 0020 */ "\x73\x61\x67\x65\x0a\x21\x0a\x68\x65\x69\x67\x68\x74\x0a\x21\x31" //sage.!.height.!1
/* 0030 */ "\x30\x32\x34\x0a\x74\x61\x72\x67\x65\x74\x5f\x6c\x6f\x67\x69\x6e" //024.target_login
/* 0040 */ "\x0a\x41\x53\x4b\x0a\x6b\x65\x65\x70\x61\x6c\x69\x76\x65\x0a\x21" //.ASK.keepalive.!
/* 0050 */ "\x74\x72\x75\x65\x0a\x61\x63\x63\x65\x70\x74\x5f\x6d\x65\x73\x73" //true.accept_mess
/* 0060 */ "\x61\x67\x65\x0a\x21\x0a\x74\x72\x61\x6e\x73\x5f\x6f\x6b\x0a\x21" //age.!.trans_ok.!
/* 0070 */ "\x4f\x75\x69\x0a\x61\x75\x74\x68\x65\x6e\x74\x69\x63\x61\x74\x65" //Oui.authenticate
/* 0080 */ "\x64\x0a\x21\x66\x61\x6c\x73\x65\x0a\x77\x69\x64\x74\x68\x0a\x21" //d.!false.width.!
/* 0090 */ "\x31\x32\x38\x30\x0a\x63\x6c\x69\x70\x62\x6f\x61\x72\x64\x0a\x21" //1280.clipboard.!
/* 00a0 */ "\x74\x72\x75\x65\x0a\x73\x65\x6c\x65\x63\x74\x6f\x72\x5f\x63\x75" //true.selector_cu
/* 00b0 */ "\x72\x72\x65\x6e\x74\x5f\x70\x61\x67\x65\x0a\x21\x31\x0a\x74\x72" //rrent_page.!1.tr
/* 00c0 */ "\x61\x6e\x73\x5f\x63\x61\x6e\x63\x65\x6c\x0a\x21\x41\x6e\x6e\x75" //ans_cancel.!Annu
/* 00d0 */ "\x6c\x65\x72\x0a\x70\x72\x6f\x78\x79\x5f\x74\x79\x70\x65\x0a\x21" //ler.proxy_type.!
/* 00e0 */ "\x52\x44\x50\x0a\x73\x65\x6c\x65\x63\x74\x6f\x72\x5f\x67\x72\x6f" //RDP.selector_gro
/* 00f0 */ "\x75\x70\x5f\x66\x69\x6c\x74\x65\x72\x0a\x21\x0a\x69\x70\x5f\x63" //up_filter.!.ip_c
/* 0100 */ "\x6c\x69\x65\x6e\x74\x0a\x21\x31\x32\x37\x2e\x30\x2e\x30\x2e\x31" //lient.!127.0.0.1
/* 0110 */ "\x0a\x66\x69\x6c\x65\x5f\x65\x6e\x63\x72\x79\x70\x74\x69\x6f\x6e" //.file_encryption
/* 0120 */ "\x0a\x21\x74\x72\x75\x65\x0a\x73\x65\x6c\x65\x63\x74\x6f\x72\x0a" //.!true.selector.
/* 0130 */ "\x41\x53\x4b\x0a\x74\x61\x72\x67\x65\x74\x5f\x64\x65\x76\x69\x63" //ASK.target_devic
/* 0140 */ "\x65\x0a\x41\x53\x4b\x0a\x73\x65\x6c\x65\x63\x74\x6f\x72\x5f\x64" //e.ASK.selector_d
/* 0150 */ "\x65\x76\x69\x63\x65\x5f\x66\x69\x6c\x74\x65\x72\x0a\x21\x0a\x70" //evice_filter.!.p
/* 0160 */ "\x61\x73\x73\x77\x6f\x72\x64\x0a\x41\x53\x4b\x0a\x6c\x6f\x67\x69" //assword.ASK.logi
/* 0170 */ "\x6e\x0a\x21\x78\x0a\x62\x70\x70\x0a\x21\x38\x0a\x73\x65\x6c\x65" //n.!x.bpp.!8.sele
/* 0180 */ "\x63\x74\x6f\x72\x5f\x6c\x69\x6e\x65\x73\x5f\x70\x65\x72\x5f\x70" //ctor_lines_per_p
/* 0190 */ "\x61\x67\x65\x0a\x21\x32\x30\x0a\x74\x61\x72\x67\x65\x74\x5f\x70" //age.!20.target_p
/* 01a0 */ "\x61\x73\x73\x77\x6f\x72\x64\x0a\x41\x53\x4b\x0a\x72\x65\x61\x6c" //assword.ASK.real
/* 01b0 */ "\x5f\x74\x61\x72\x67\x65\x74\x5f\x64\x65\x76\x69\x63\x65\x0a\x21" //_target_device.!
/* 01c0 */ "\x0a\x70\x72\x6f\x74\x6f\x5f\x64\x65\x73\x74\x0a\x41\x53\x4b\x0a" //.proto_dest.ASK.
// Dump done on Authentifier (4) 464 bytes
// Data size received without header 464
// auth::in_items
// receiving 'ip_target'='127.0.0.1'
// receiving 'display_message'=''
// receiving 'height'='1024'
// receiving ASK 'target_login'
// receiving 'keepalive'='True'
// receiving 'accept_message'=''
// receiving 'trans_ok'='Oui'
// receiving 'authenticated'='False'
// receiving 'width'='1280'
// receiving 'clipboard'='True'
// receiving 'selector_current_page'='1'
// receiving 'trans_cancel'='Annuler'
// receiving 'proxy_type'='RDP'
// receiving 'selector_group_filter'=''
// receiving 'ip_client'='127.0.0.1'
// receiving 'file_encryption'='True'
// receiving ASK 'selector'
// receiving ASK 'target_device'
// receiving 'selector_device_filter'=''
// receiving ASK 'password'
// receiving 'login'='x'
// receiving 'bpp'='8'
// receiving 'selector_lines_per_page'='20'
// receiving ASK 'target_password'
// receiving 'real_target_device'=''
// receiving ASK 'proto_dest'
// SESSION_ID = 
};











// Session::-------------- Running -------------------
// Session::back_event fired
// signal in transitory mode
// Session::back event end module
// Session::starting next module
// auth::ask_next_module
// auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS state
// auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_AUTH_USER=False
// auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_PASSWORD=True
// auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_PASSWORD is asked
// session::next_state 5
// Session::session_setup_mod(target_module=5, submodule=11)
// Session::Creation of internal module 'Login'
// loading bitmap /usr/local/share/rdpproxy/ad24b.bmp
// loading file 140 x 140 x 8
// loading bitmap /usr/local/share/rdpproxy/xrdp24b-redemption.bmp
// loading file 256 x 125 x 24
// Session::internal module Login ready
// Session::back_event fired
