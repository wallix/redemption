const char outdata[] =
{
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x6e\x49\x01\x00\x0c\x00\x03\x00\x00\x00"                 //rDnI........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Announce Request |
// ServerAnnounceRequest: VersionMajor=0x0001 VersionMinor=0x000C ClientId=3 |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x6e\x49\x01\x00\x0c\x00\x03\x00\x00\x00"                 //rDnI........ |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x03\x00\x00\x00"                 //rDCC........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Announce Reply |
// ClientAnnounceReply: VersionMajor=0x0001 VersionMinor=0x000C ClientId=3 |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x03\x00\x00\x00"                 //rDCC........ |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=26 flags=0x00000003 chunk_data_length=26 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00" //rDNC............ |
// /* 0010 */ "\x72\x00\x7a\x00\x68\x00\x00\x00\x00\x00"                         //r.z.h..... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Name Request |
// ClientNameRequest: UnicodeFlag=0x1 CodePage=0 ComputerName="rzh" |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00" //rDNC............ |
/* 0010 */ "\x72\x00\x7a\x00\x68\x00\x00\x00\x00\x00"                         //r.z.h..... |
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
// Sending on channel (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x72\x44\x50\x53\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPS......,..... |
/* 0010 */ "\x02\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0030 */ "\x02\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
/* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
/* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x03\x00\x00\x00"                 //rDCC........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Client ID Confirm |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x03\x00\x00\x00"                 //rDCC........ |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=84 flags=0x00000003 chunk_data_length=84 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,..... |
// /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0a\x00\xff\xff\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
// /* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
// /* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Core Capability Response |
// FileSystemVirtualChannel::process_client_core_capability_response: numCapabilities=5 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0001 CapabilityLength=44 Version=0x2 |
// FileSystemVirtualChannel::process_client_general_capability_set: |
// GeneralCapabilitySet: osType=0x0 osVersion=0x0 protocolMajorVersion=0x1 protocolMinorVersion=0xA ioCode1=0xFFFF ioCode2=0x0 extendedPDU=0x7 extraFlags1=0x1 extraFlags2=0x0 SpecialTypeDeviceCap=0 |
// FileSystemVirtualChannel::process_client_general_capability_set:Deny user to send multiple simultaneous read or write requests on the same file from a redirected file system. |
// GeneralCapabilitySet: osType=0x0 osVersion=0x0 protocolMajorVersion=0x1 protocolMinorVersion=0xA ioCode1=0xFFFF ioCode2=0x0 extendedPDU=0x7 extraFlags1=0x0 extraFlags2=0x0 SpecialTypeDeviceCap=0 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0002 CapabilityLength=8 Version=0x1 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0003 CapabilityLength=8 Version=0x1 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0004 CapabilityLength=8 Version=0x2 |
// FileSystemVirtualChannel::process_client_core_capability_response: Client supports DRIVE_CAPABILITY_VERSION_02. |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0005 CapabilityLength=8 Version=0x1 |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,..... |
/* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0a\x00\xff\xff\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
/* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
/* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=8 flags=0x00000003 chunk_data_length=8 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x08\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x08\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x41\x44\x00\x00\x00\x00"                                 //rDAD.... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Device List Announce Request |
// FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request: DeviceCount=0 |
// FileSystemVirtualChannel::process_client_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4d\x44\x01\x00\x00\x00\x01\x00\x00\x00"                 //rDMD........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Drive Device List Remove |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4d\x44\x01\x00\x00\x00\x01\x00\x00\x00"                 //rDMD........ |
// Sent dumped on channel (-1) n bytes |
// process disconnect pdu : code =        0 error=? |
// process save session info : Logon long |
// Logon Info Version 2 (data): Domain="RED" UserName="tartempion" SessionId=2 |
// Ask acl |
// sending reporting=OPEN_SESSION_SUCCESSFUL:win2k12r2:Ok. |
// sending keepalive=ASK |
// process save session info : Logon extended info |
// process save session info : Auto-reconnect cookie |
// LogonId=2 |
// 0000 ef 36 73 11 08 54 58 ee cb 07 2c 68 fa 2d cc a2 .6s..TX...,h.-.. |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'keepalive'='True' |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x6e\x49\x01\x00\x0c\x00\x02\x00\x00\x00"                 //rDnI........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Announce Request |
// ServerAnnounceRequest: VersionMajor=0x0001 VersionMinor=0x000C ClientId=2 |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x6e\x49\x01\x00\x0c\x00\x02\x00\x00\x00"                 //rDnI........ |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x02\x00\x00\x00"                 //rDCC........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Announce Reply |
// ClientAnnounceReply: VersionMajor=0x0001 VersionMinor=0x000C ClientId=2 |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x02\x00\x00\x00"                 //rDCC........ |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=26 flags=0x00000003 chunk_data_length=26 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00" //rDNC............ |
// /* 0010 */ "\x72\x00\x7a\x00\x68\x00\x00\x00\x00\x00"                         //r.z.h..... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Name Request |
// ClientNameRequest: UnicodeFlag=0x1 CodePage=0 ComputerName="rzh" |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00" //rDNC............ |
/* 0010 */ "\x72\x00\x7a\x00\x68\x00\x00\x00\x00\x00"                         //r.z.h..... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x02\x00\x00\x00"                 //rDCC........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Client ID Confirm |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x02\x00\x00\x00"                 //rDCC........ |
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
// Sending on channel (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x72\x44\x50\x53\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPS......,..... |
/* 0010 */ "\x02\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0030 */ "\x02\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
/* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
/* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=8 flags=0x00000003 chunk_data_length=8 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x08\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x08\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x41\x44\x00\x00\x00\x00"                                 //rDAD.... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Device List Announce Request |
// FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request: DeviceCount=0 |
// FileSystemVirtualChannel::process_client_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4d\x44\x01\x00\x00\x00\x01\x00\x00\x00"                 //rDMD........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Drive Device List Remove |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4d\x44\x01\x00\x00\x00\x01\x00\x00\x00"                 //rDMD........ |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=84 flags=0x00000003 chunk_data_length=84 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,..... |
// /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0a\x00\xff\xff\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
// /* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
// /* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Core Capability Response |
// FileSystemVirtualChannel::process_client_core_capability_response: numCapabilities=5 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0001 CapabilityLength=44 Version=0x2 |
// FileSystemVirtualChannel::process_client_general_capability_set: |
// GeneralCapabilitySet: osType=0x0 osVersion=0x0 protocolMajorVersion=0x1 protocolMinorVersion=0xA ioCode1=0xFFFF ioCode2=0x0 extendedPDU=0x7 extraFlags1=0x1 extraFlags2=0x0 SpecialTypeDeviceCap=0 |
// FileSystemVirtualChannel::process_client_general_capability_set:Deny user to send multiple simultaneous read or write requests on the same file from a redirected file system. |
// GeneralCapabilitySet: osType=0x0 osVersion=0x0 protocolMajorVersion=0x1 protocolMinorVersion=0xA ioCode1=0xFFFF ioCode2=0x0 extendedPDU=0x7 extraFlags1=0x0 extraFlags2=0x0 SpecialTypeDeviceCap=0 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0002 CapabilityLength=8 Version=0x1 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0003 CapabilityLength=8 Version=0x1 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0004 CapabilityLength=8 Version=0x2 |
// FileSystemVirtualChannel::process_client_core_capability_response: Client supports DRIVE_CAPABILITY_VERSION_02. |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0005 CapabilityLength=8 Version=0x1 |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,..... |
/* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0a\x00\xff\xff\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
/* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
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
// FileSystemDriveManager::announce_drive |
// DeviceAnnounceHeader: DeviceType=RDPDR_DTYP_FILESYSTEM(8) DeviceId=32767 PreferredDosName="EXPORT" |
// 0000 45 58 50 4f 52 54 00                            EXPORT. |
// FileSystemDriveManager::announce_drive |
// DeviceAnnounceHeader: DeviceType=RDPDR_DTYP_FILESYSTEM(8) DeviceId=32768 PreferredDosName="SHARE" |
// 0000 53 48 41 52 45 00                               SHARE. |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x23\x00\x00\x00"                                                 //#... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x23\x00\x00\x00"                                                 //#... |
/* 0000 */ "\x72\x44\x41\x44\x01\x00\x00\x00\x08\x00\x00\x00\xff\x7f\x00\x00" //rDAD............ |
/* 0010 */ "\x45\x58\x50\x4f\x52\x54\x00\x00\x07\x00\x00\x00\x45\x58\x50\x4f" //EXPORT......EXPO |
/* 0020 */ "\x52\x54\x00"                                                     //RT. |
// Sent dumped on channel (-1) n bytes |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4c\x55"                                                 //rDLU |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=33 flags=0x00000003 chunk_data_length=33 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x21\x00\x00\x00"                                                 //!... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x21\x00\x00\x00"                                                 //!... |
// /* 0000 */ "\x72\x44\x41\x44\x01\x00\x00\x00\x08\x00\x00\x00\x01\x00\x00\x00" //rDAD............ |
// /* 0010 */ "\x68\x6f\x6d\x65\x00\x00\x00\x00\x05\x00\x00\x00\x68\x6f\x6d\x65" //home........home |
// /* 0020 */ "\x00"                                                             //. |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Device List Announce Request |
// FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request: DeviceCount=1 |
// FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request: DeviceType=RDPDR_DTYP_FILESYSTEM(8) DeviceId=1 PreferredDosName="home" DeviceDataLength=5 |
// FileSystemVirtualChannel::process_client_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4d\x44\x01\x00\x00\x00\x01\x00\x00\x00"                 //rDMD........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Drive Device List Remove |
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
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x22\x00\x00\x00"                                                 //"... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x22\x00\x00\x00"                                                 //"... |
/* 0000 */ "\x72\x44\x41\x44\x01\x00\x00\x00\x08\x00\x00\x00\x00\x80\x00\x00" //rDAD............ |
/* 0010 */ "\x53\x48\x41\x52\x45\x00\x00\x00\x06\x00\x00\x00\x53\x48\x41\x52" //SHARE.......SHAR |
/* 0020 */ "\x45\x00"                                                         //E. |
// Sent dumped on channel (-1) n bytes |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x72\x64\xff\x7f\x00\x00\x00\x00\x00\x00"                 //rDrd........ |
// Sent dumped on channel (-1) n bytes |
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
// Sending on channel (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x72\x64\x00\x80\x00\x00\x00\x00\x00\x00"                 //rDrd........ |
// Sent dumped on channel (-1) n bytes |
} /* end outdata */;

const char indata[] =
{
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x6e\x49\x01\x00\x0c\x00\x03\x00\x00\x00"                 //rDnI........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Announce Request |
// ServerAnnounceRequest: VersionMajor=0x0001 VersionMinor=0x000C ClientId=3 |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x6e\x49\x01\x00\x0c\x00\x03\x00\x00\x00"                 //rDnI........ |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x03\x00\x00\x00"                 //rDCC........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Announce Reply |
// ClientAnnounceReply: VersionMajor=0x0001 VersionMinor=0x000C ClientId=3 |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x03\x00\x00\x00"                 //rDCC........ |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=26 flags=0x00000003 chunk_data_length=26 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00" //rDNC............ |
/* 0010 */ "\x72\x00\x7a\x00\x68\x00\x00\x00\x00\x00"                         //r.z.h..... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Name Request |
// ClientNameRequest: UnicodeFlag=0x1 CodePage=0 ComputerName="rzh" |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00" //rDNC............ |
// /* 0010 */ "\x72\x00\x7a\x00\x68\x00\x00\x00\x00\x00"                         //r.z.h..... |
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
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x72\x44\x50\x53\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPS......,..... |
// /* 0010 */ "\x02\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x02\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
// /* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
// /* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x03\x00\x00\x00"                 //rDCC........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Client ID Confirm |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x03\x00\x00\x00"                 //rDCC........ |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=84 flags=0x00000003 chunk_data_length=84 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,..... |
/* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0a\x00\xff\xff\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
/* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
/* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Core Capability Response |
// FileSystemVirtualChannel::process_client_core_capability_response: numCapabilities=5 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0001 CapabilityLength=44 Version=0x2 |
// FileSystemVirtualChannel::process_client_general_capability_set: |
// GeneralCapabilitySet: osType=0x0 osVersion=0x0 protocolMajorVersion=0x1 protocolMinorVersion=0xA ioCode1=0xFFFF ioCode2=0x0 extendedPDU=0x7 extraFlags1=0x1 extraFlags2=0x0 SpecialTypeDeviceCap=0 |
// FileSystemVirtualChannel::process_client_general_capability_set:Deny user to send multiple simultaneous read or write requests on the same file from a redirected file system. |
// GeneralCapabilitySet: osType=0x0 osVersion=0x0 protocolMajorVersion=0x1 protocolMinorVersion=0xA ioCode1=0xFFFF ioCode2=0x0 extendedPDU=0x7 extraFlags1=0x0 extraFlags2=0x0 SpecialTypeDeviceCap=0 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0002 CapabilityLength=8 Version=0x1 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0003 CapabilityLength=8 Version=0x1 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0004 CapabilityLength=8 Version=0x2 |
// FileSystemVirtualChannel::process_client_core_capability_response: Client supports DRIVE_CAPABILITY_VERSION_02. |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0005 CapabilityLength=8 Version=0x1 |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,..... |
// /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0a\x00\xff\xff\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
// /* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
// /* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=8 flags=0x00000003 chunk_data_length=8 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x08\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x08\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x41\x44\x00\x00\x00\x00"                                 //rDAD.... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Device List Announce Request |
// FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request: DeviceCount=0 |
// FileSystemVirtualChannel::process_client_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4d\x44\x01\x00\x00\x00\x01\x00\x00\x00"                 //rDMD........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Drive Device List Remove |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4d\x44\x01\x00\x00\x00\x01\x00\x00\x00"                 //rDMD........ |
// Sent dumped on channel (-1) n bytes |
// process disconnect pdu : code =        0 error=? |
// process save session info : Logon long |
// Logon Info Version 2 (data): Domain="RED" UserName="tartempion" SessionId=2 |
// Ask acl |
// sending reporting=OPEN_SESSION_SUCCESSFUL:win2k12r2:Ok. |
// sending keepalive=ASK |
// process save session info : Logon extended info |
// process save session info : Auto-reconnect cookie |
// LogonId=2 |
// 0000 ef 36 73 11 08 54 58 ee cb 07 2c 68 fa 2d cc a2 .6s..TX...,h.-.. |
// +++++++++++> ACL receive <++++++++++++++++ |
// receiving 'keepalive'='True' |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x6e\x49\x01\x00\x0c\x00\x02\x00\x00\x00"                 //rDnI........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Announce Request |
// ServerAnnounceRequest: VersionMajor=0x0001 VersionMinor=0x000C ClientId=2 |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x6e\x49\x01\x00\x0c\x00\x02\x00\x00\x00"                 //rDnI........ |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x02\x00\x00\x00"                 //rDCC........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Announce Reply |
// ClientAnnounceReply: VersionMajor=0x0001 VersionMinor=0x000C ClientId=2 |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x02\x00\x00\x00"                 //rDCC........ |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=26 flags=0x00000003 chunk_data_length=26 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00" //rDNC............ |
/* 0010 */ "\x72\x00\x7a\x00\x68\x00\x00\x00\x00\x00"                         //r.z.h..... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Name Request |
// ClientNameRequest: UnicodeFlag=0x1 CodePage=0 ComputerName="rzh" |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x1a\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00" //rDNC............ |
// /* 0010 */ "\x72\x00\x7a\x00\x68\x00\x00\x00\x00\x00"                         //r.z.h..... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x02\x00\x00\x00"                 //rDCC........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server Client ID Confirm |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x02\x00\x00\x00"                 //rDCC........ |
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
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x72\x44\x50\x53\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPS......,..... |
// /* 0010 */ "\x02\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x02\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
// /* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
// /* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=8 flags=0x00000003 chunk_data_length=8 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x08\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x08\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x41\x44\x00\x00\x00\x00"                                 //rDAD.... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Device List Announce Request |
// FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request: DeviceCount=0 |
// FileSystemVirtualChannel::process_client_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4d\x44\x01\x00\x00\x00\x01\x00\x00\x00"                 //rDMD........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Drive Device List Remove |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4d\x44\x01\x00\x00\x00\x01\x00\x00\x00"                 //rDMD........ |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=84 flags=0x00000003 chunk_data_length=84 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,..... |
/* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0a\x00\xff\xff\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
/* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
/* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Core Capability Response |
// FileSystemVirtualChannel::process_client_core_capability_response: numCapabilities=5 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0001 CapabilityLength=44 Version=0x2 |
// FileSystemVirtualChannel::process_client_general_capability_set: |
// GeneralCapabilitySet: osType=0x0 osVersion=0x0 protocolMajorVersion=0x1 protocolMinorVersion=0xA ioCode1=0xFFFF ioCode2=0x0 extendedPDU=0x7 extraFlags1=0x1 extraFlags2=0x0 SpecialTypeDeviceCap=0 |
// FileSystemVirtualChannel::process_client_general_capability_set:Deny user to send multiple simultaneous read or write requests on the same file from a redirected file system. |
// GeneralCapabilitySet: osType=0x0 osVersion=0x0 protocolMajorVersion=0x1 protocolMinorVersion=0xA ioCode1=0xFFFF ioCode2=0x0 extendedPDU=0x7 extraFlags1=0x0 extraFlags2=0x0 SpecialTypeDeviceCap=0 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0002 CapabilityLength=8 Version=0x1 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0003 CapabilityLength=8 Version=0x1 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0004 CapabilityLength=8 Version=0x2 |
// FileSystemVirtualChannel::process_client_core_capability_response: Client supports DRIVE_CAPABILITY_VERSION_02. |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0005 CapabilityLength=8 Version=0x1 |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,..... |
// /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x0a\x00\xff\xff\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
// /* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x02\x00\x00\x00\x05\x00\x08\x00" //................ |
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
// FileSystemDriveManager::announce_drive |
// DeviceAnnounceHeader: DeviceType=RDPDR_DTYP_FILESYSTEM(8) DeviceId=32767 PreferredDosName="EXPORT" |
// 0000 45 58 50 4f 52 54 00                            EXPORT. |
// FileSystemDriveManager::announce_drive |
// DeviceAnnounceHeader: DeviceType=RDPDR_DTYP_FILESYSTEM(8) DeviceId=32768 PreferredDosName="SHARE" |
// 0000 53 48 41 52 45 00                               SHARE. |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x23\x00\x00\x00"                                                 //#... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x23\x00\x00\x00"                                                 //#... |
// /* 0000 */ "\x72\x44\x41\x44\x01\x00\x00\x00\x08\x00\x00\x00\xff\x7f\x00\x00" //rDAD............ |
// /* 0010 */ "\x45\x58\x50\x4f\x52\x54\x00\x00\x07\x00\x00\x00\x45\x58\x50\x4f" //EXPORT......EXPO |
// /* 0020 */ "\x52\x54\x00"                                                     //RT. |
// Sent dumped on channel (-1) n bytes |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4c\x55"                                                 //rDLU |
// Sent dumped on channel (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: total_length=33 flags=0x00000003 chunk_data_length=33 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x21\x00\x00\x00"                                                 //!... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x21\x00\x00\x00"                                                 //!... |
/* 0000 */ "\x72\x44\x41\x44\x01\x00\x00\x00\x08\x00\x00\x00\x01\x00\x00\x00" //rDAD............ |
/* 0010 */ "\x68\x6f\x6d\x65\x00\x00\x00\x00\x05\x00\x00\x00\x68\x6f\x6d\x65" //home........home |
/* 0020 */ "\x00"                                                             //. |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Device List Announce Request |
// FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request: DeviceCount=1 |
// FileSystemVirtualChannel::DeviceRedirectionManager::process_client_device_list_announce_request: DeviceType=RDPDR_DTYP_FILESYSTEM(8) DeviceId=1 PreferredDosName="home" DeviceDataLength=5 |
// FileSystemVirtualChannel::process_client_message: total_length=12 flags=0x00000003 chunk_data_length=12 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4d\x44\x01\x00\x00\x00\x01\x00\x00\x00"                 //rDMD........ |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Drive Device List Remove |
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
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x22\x00\x00\x00"                                                 //"... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x22\x00\x00\x00"                                                 //"... |
// /* 0000 */ "\x72\x44\x41\x44\x01\x00\x00\x00\x08\x00\x00\x00\x00\x80\x00\x00" //rDAD............ |
// /* 0010 */ "\x53\x48\x41\x52\x45\x00\x00\x00\x06\x00\x00\x00\x53\x48\x41\x52" //SHARE.......SHAR |
// /* 0020 */ "\x45\x00"                                                         //E. |
// Sent dumped on channel (-1) n bytes |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x72\x64\xff\x7f\x00\x00\x00\x00\x00\x00"                 //rDrd........ |
// Sent dumped on channel (-1) n bytes |
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
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x72\x64\x00\x80\x00\x00\x00\x00\x00\x00"                 //rDrd........ |
// Sent dumped on channel (-1) n bytes |
} /* end indata */;

