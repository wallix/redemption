const char outdata[] =
{
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x6e\x49\x01\x00\x0c\x00\x06\x00\x00\x00"                 //rDnI........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Announce Request |
// ServerAnnounceRequest: VersionMajor=0x0001 VersionMinor=0x000C ClientId=6 |
// FileSystemVirtualChannel::process_server_announce_request: |
// ClientAnnounceReply: VersionMajor=0x0001 VersionMinor=0x0006 ClientId=5245 |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x43\x43\x01\x00\x06\x00\x7d\x14\x00\x00"                 //rDCC....}... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_server_announce_request: |
// ClientNameRequest: UnicodeFlag=0x1 CodePage=0 ComputerName="rzh" |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00" //rDNC............ |
/* 0010 */ "\x72\x00\x7a\x00\x68\x00\x00\x00"                                 //r.z.h... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: total_length=84 flags=0x00000003 chunk_data_length=84 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x72\x44\x50\x53\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPS......,..... |
// /* 0010 */ "\x02\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x02\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
// /* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
// /* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Core Capability Request |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x7d\x14\x00\x00"                 //rDCC....}... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Client ID Confirm |
// FileSystemVirtualChannel::process_server_client_id_confirm: |
// GeneralCapabilitySet: osType=0x2 osVersion=0x50001 protocolMajorVersion=0x1 protocolMinorVersion=0xC ioCode1=0xFFFF ioCode2=0x0 extendedPDU=0x7 extraFlags1=0x0 extraFlags2=0x0 SpecialTypeDeviceCap=0 |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,..... |
/* 0010 */ "\x02\x00\x00\x00\x01\x00\x05\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
/* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x01\x00\x00\x00\x05\x00\x08\x00" //................ |
/* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Sent dumped on channel (-1) n bytes |
// process disconnect pdu : code =        0 error=? |
// process save session info : Logon long |
// Logon Info Version 2 (data): Domain="RED" UserName="tartempion" SessionId=4 |
// Ask acl |
// sending reporting=OPEN_SESSION_SUCCESSFUL:win2k12r2:Ok. |
// sending keepalive=ASK |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'keepalive'='True' |
// process save session info : Logon extended info |
// process save session info : Auto-reconnect cookie |
// LogonId=4 |
// 0000 14 b1 6b f7 86 0c 0a e6 c9 4b fa 2d 93 48 5b fb ..k......K.-.H[. |
// mod_rdp server clipboard PDU |
// mod_rdp server clipboard PDU: msgType=CB_CLIP_CAPS(7) |
// mod_rdp Server Clipboard Capabilities PDU |
// RDPECLIP::GeneralCapabilitySet: capabilitySetType=CB_CAPSTYPE_GENERAL(1) lengthCapability=12 version=CB_CAPS_VERSION_2(0x00000002) generalFlags=0x0000001E |
// mod_rdp server clipboard PDU |
// mod_rdp server clipboard PDU: msgType=CB_MONITOR_READY(1) |
// mod_rdp client clipboard PDU |
// mod_rdp::send_to_mod_cliprdr_channel: client clipboard PDU: msgType=CB_CLIP_CAPS(7) |
// mod_rdp::send_to_mod_cliprdr_channel: Client Clipboard Capabilities PDU |
// RDPECLIP::GeneralCapabilitySet: capabilitySetType=CB_CAPSTYPE_GENERAL(1) lengthCapability=12 version=CB_CAPS_VERSION_2(0x00000002) generalFlags=0x00000002 |
// mod_rdp client clipboard PDU |
// mod_rdp::send_to_mod_cliprdr_channel: client clipboard PDU: msgType=CB_FORMAT_LIST(2) |
// mod_rdp Clipboard: Long Format Name variant of Format List PDU is used for exchanging updated format names. |
// mod_rdp Clipboard: formatId=1 wszFormatName="" |
// mod_rdp Clipboard: formatId=13 wszFormatName="" |
// mod_rdp server clipboard PDU |
// mod_rdp server clipboard PDU: msgType=CB_FORMAT_LIST_RESPONSE(3) |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x6e\x49\x01\x00\x0c\x00\x04\x00\x00\x00"                 //rDnI........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Announce Request |
// ServerAnnounceRequest: VersionMajor=0x0001 VersionMinor=0x000C ClientId=4 |
// FileSystemVirtualChannel::process_server_announce_request: |
// ClientAnnounceReply: VersionMajor=0x0001 VersionMinor=0x0006 ClientId=5245 |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x43\x43\x01\x00\x06\x00\x7d\x14\x00\x00"                 //rDCC....}... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_server_announce_request: |
// ClientNameRequest: UnicodeFlag=0x1 CodePage=0 ComputerName="rzh" |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00" //rDNC............ |
/* 0010 */ "\x72\x00\x7a\x00\x68\x00\x00\x00"                                 //r.z.h... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: total_length=84 flags=0x00000003 chunk_data_length=84 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x72\x44\x50\x53\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPS......,..... |
// /* 0010 */ "\x02\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x02\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
// /* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
// /* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Core Capability Request |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x7d\x14\x00\x00"                 //rDCC....}... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Client ID Confirm |
// FileSystemVirtualChannel::process_server_client_id_confirm: |
// GeneralCapabilitySet: osType=0x2 osVersion=0x50001 protocolMajorVersion=0x1 protocolMinorVersion=0xC ioCode1=0xFFFF ioCode2=0x0 extendedPDU=0x7 extraFlags1=0x0 extraFlags2=0x0 SpecialTypeDeviceCap=0 |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,..... |
/* 0010 */ "\x02\x00\x00\x00\x01\x00\x05\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
/* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x01\x00\x00\x00\x05\x00\x08\x00" //................ |
/* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: total_length=4 flags=0x00000003 chunk_data_length=4 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4c\x55"                                                 //rDLU |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server User Logged On |
// FileSystemDriveManager::AnnounceDrivePartially |
// DeviceAnnounceHeader: DeviceType=RDPDR_DTYP_FILESYSTEM(8) DeviceId=32767 PreferredDosName="EXPORT" |
// 0000 45 58 50 4f 52 54 00                            EXPORT. |
// FileSystemDriveManager::AnnounceDrivePartially |
// DeviceAnnounceHeader: DeviceType=RDPDR_DTYP_FILESYSTEM(8) DeviceId=32768 PreferredDosName="SHARE" |
// 0000 53 48 41 52 45 00                               SHARE. |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x72\x64\xff\x7f\x00\x00\x00\x00\x00\x00"                 //rDrd........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Device Announce Response |
// ServerDeviceAnnounceResponse: DeviceId=32767 ResultCode=0x00000000 |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x72\x64\x00\x80\x00\x00\x00\x00\x00\x00"                 //rDrd........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Device Announce Response |
// ServerDeviceAnnounceResponse: DeviceId=32768 ResultCode=0x00000000 |
} /* end outdata */;

const char indata[] =
{
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x6e\x49\x01\x00\x0c\x00\x06\x00\x00\x00"                 //rDnI........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Announce Request |
// ServerAnnounceRequest: VersionMajor=0x0001 VersionMinor=0x000C ClientId=6 |
// FileSystemVirtualChannel::process_server_announce_request: |
// ClientAnnounceReply: VersionMajor=0x0001 VersionMinor=0x0006 ClientId=5245 |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x43\x43\x01\x00\x06\x00\x7d\x14\x00\x00"                 //rDCC....}... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_server_announce_request: |
// ClientNameRequest: UnicodeFlag=0x1 CodePage=0 ComputerName="rzh" |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00" //rDNC............ |
// /* 0010 */ "\x72\x00\x7a\x00\x68\x00\x00\x00"                                 //r.z.h... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: total_length=84 flags=0x00000003 chunk_data_length=84 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x72\x44\x50\x53\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPS......,..... |
/* 0010 */ "\x02\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0030 */ "\x02\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
/* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
/* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Core Capability Request |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x7d\x14\x00\x00"                 //rDCC....}... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Client ID Confirm |
// FileSystemVirtualChannel::process_server_client_id_confirm: |
// GeneralCapabilitySet: osType=0x2 osVersion=0x50001 protocolMajorVersion=0x1 protocolMinorVersion=0xC ioCode1=0xFFFF ioCode2=0x0 extendedPDU=0x7 extraFlags1=0x0 extraFlags2=0x0 SpecialTypeDeviceCap=0 |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,..... |
// /* 0010 */ "\x02\x00\x00\x00\x01\x00\x05\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
// /* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x01\x00\x00\x00\x05\x00\x08\x00" //................ |
// /* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Sent dumped on channel (-1) n bytes |
// process disconnect pdu : code =        0 error=? |
// process save session info : Logon long |
// Logon Info Version 2 (data): Domain="RED" UserName="tartempion" SessionId=4 |
// Ask acl |
// sending reporting=OPEN_SESSION_SUCCESSFUL:win2k12r2:Ok. |
// sending keepalive=ASK |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'keepalive'='True' |
// process save session info : Logon extended info |
// process save session info : Auto-reconnect cookie |
// LogonId=4 |
// 0000 14 b1 6b f7 86 0c 0a e6 c9 4b fa 2d 93 48 5b fb ..k......K.-.H[. |
// mod_rdp server clipboard PDU |
// mod_rdp server clipboard PDU: msgType=CB_CLIP_CAPS(7) |
// mod_rdp Server Clipboard Capabilities PDU |
// RDPECLIP::GeneralCapabilitySet: capabilitySetType=CB_CAPSTYPE_GENERAL(1) lengthCapability=12 version=CB_CAPS_VERSION_2(0x00000002) generalFlags=0x0000001E |
// mod_rdp server clipboard PDU |
// mod_rdp server clipboard PDU: msgType=CB_MONITOR_READY(1) |
// mod_rdp client clipboard PDU |
// mod_rdp::send_to_mod_cliprdr_channel: client clipboard PDU: msgType=CB_CLIP_CAPS(7) |
// mod_rdp::send_to_mod_cliprdr_channel: Client Clipboard Capabilities PDU |
// RDPECLIP::GeneralCapabilitySet: capabilitySetType=CB_CAPSTYPE_GENERAL(1) lengthCapability=12 version=CB_CAPS_VERSION_2(0x00000002) generalFlags=0x00000002 |
// mod_rdp client clipboard PDU |
// mod_rdp::send_to_mod_cliprdr_channel: client clipboard PDU: msgType=CB_FORMAT_LIST(2) |
// mod_rdp Clipboard: Long Format Name variant of Format List PDU is used for exchanging updated format names. |
// mod_rdp Clipboard: formatId=1 wszFormatName="" |
// mod_rdp Clipboard: formatId=13 wszFormatName="" |
// mod_rdp server clipboard PDU |
// mod_rdp server clipboard PDU: msgType=CB_FORMAT_LIST_RESPONSE(3) |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x6e\x49\x01\x00\x0c\x00\x04\x00\x00\x00"                 //rDnI........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Announce Request |
// ServerAnnounceRequest: VersionMajor=0x0001 VersionMinor=0x000C ClientId=4 |
// FileSystemVirtualChannel::process_server_announce_request: |
// ClientAnnounceReply: VersionMajor=0x0001 VersionMinor=0x0006 ClientId=5245 |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x43\x43\x01\x00\x06\x00\x7d\x14\x00\x00"                 //rDCC....}... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_server_announce_request: |
// ClientNameRequest: UnicodeFlag=0x1 CodePage=0 ComputerName="rzh" |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x18\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00" //rDNC............ |
// /* 0010 */ "\x72\x00\x7a\x00\x68\x00\x00\x00"                                 //r.z.h... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: total_length=84 flags=0x00000003 chunk_data_length=84 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x72\x44\x50\x53\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPS......,..... |
/* 0010 */ "\x02\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0030 */ "\x02\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
/* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
/* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Core Capability Request |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x7d\x14\x00\x00"                 //rDCC....}... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Client ID Confirm |
// FileSystemVirtualChannel::process_server_client_id_confirm: |
// GeneralCapabilitySet: osType=0x2 osVersion=0x50001 protocolMajorVersion=0x1 protocolMinorVersion=0xC ioCode1=0xFFFF ioCode2=0x0 extendedPDU=0x7 extraFlags1=0x0 extraFlags2=0x0 SpecialTypeDeviceCap=0 |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,..... |
// /* 0010 */ "\x02\x00\x00\x00\x01\x00\x05\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
// /* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x01\x00\x00\x00\x05\x00\x08\x00" //................ |
// /* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: total_length=4 flags=0x00000003 chunk_data_length=4 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4c\x55"                                                 //rDLU |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server User Logged On |
// FileSystemDriveManager::AnnounceDrivePartially |
// DeviceAnnounceHeader: DeviceType=RDPDR_DTYP_FILESYSTEM(8) DeviceId=32767 PreferredDosName="EXPORT" |
// 0000 45 58 50 4f 52 54 00                            EXPORT. |
// FileSystemDriveManager::AnnounceDrivePartially |
// DeviceAnnounceHeader: DeviceType=RDPDR_DTYP_FILESYSTEM(8) DeviceId=32768 PreferredDosName="SHARE" |
// 0000 53 48 41 52 45 00                               SHARE. |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x72\x64\xff\x7f\x00\x00\x00\x00\x00\x00"                 //rDrd........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Device Announce Response |
// ServerDeviceAnnounceResponse: DeviceId=32767 ResultCode=0x00000000 |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x72\x64\x00\x80\x00\x00\x00\x00\x00\x00"                 //rDrd........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Device Announce Response |
// ServerDeviceAnnounceResponse: DeviceId=32768 ResultCode=0x00000000 |
} /* end indata */;

