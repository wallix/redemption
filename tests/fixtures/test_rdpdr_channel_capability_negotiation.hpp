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
// FileSystemVirtualChannel::process_client_message: total_length=36 flags=0x00000003 chunk_data_length=36 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x24\x00\x00\x00"                                                 //$... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x24\x00\x00\x00"                                                 //$... |
// /* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00" //rDNC............ |
// /* 0010 */ "\x57\x00\x49\x00\x4e\x00\x32\x00\x4b\x00\x38\x00\x2d\x00\x36\x00" //W.I.N.2.K.8.-.6. |
// /* 0020 */ "\x34\x00\x00\x00"                                                 //4... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Name Request |
// ClientNameRequest: UnicodeFlag=0x1 CodePage=0 ComputerName="WIN2K8-64" |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x03\x00\x00\x00"                 //rDCC........ |
// Sent dumped on channel (-1) n bytes |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x24\x00\x00\x00"                                                 //$... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x24\x00\x00\x00"                                                 //$... |
/* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00" //rDNC............ |
/* 0010 */ "\x57\x00\x49\x00\x4e\x00\x32\x00\x4b\x00\x38\x00\x2d\x00\x36\x00" //W.I.N.2.K.8.-.6. |
/* 0020 */ "\x34\x00\x00\x00"                                                 //4... |
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
// /* 0010 */ "\x02\x00\x00\x00\x00\x00\x06\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
// /* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x01\x00\x00\x00\x05\x00\x08\x00" //................ |
// /* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Core Capability Response |
// FileSystemVirtualChannel::process_client_core_capability_response: numCapabilities=5 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0001 CapabilityLength=44 Version=0x2 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0002 CapabilityLength=8 Version=0x1 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0003 CapabilityLength=8 Version=0x1 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0004 CapabilityLength=8 Version=0x1 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0005 CapabilityLength=8 Version=0x1 |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
/* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,..... |
/* 0010 */ "\x02\x00\x00\x00\x00\x00\x06\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
/* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x01\x00\x00\x00\x05\x00\x08\x00" //................ |
/* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Sent dumped on channel (-1) n bytes |
// process save session info : Logon extended info |
// process save session info : Logon Errors Info |
// ErrorNotificationType=LOGON_MSG_SESSION_CONTINUE(0xFFFFFFFE) "The logon process is continuing." ErrorNotificationData=LOGON_WARNING(0x00000003) "The logon process has displayed a warning." |
// process save session info : Logon long |
// Logon Info Version 2 (data): Domain="QA" UserName="administrateur" SessionId=3 |
// Ask acl |
// sending reporting=OPEN_SESSION_SUCCESSFUL:win2k8:OK. |
// sending keepalive=ASK |
// process save session info : Logon extended info |
// process save session info : Auto-reconnect cookie |
// LogonId=3 |
// 0000 1b 00 88 9b 55 13 18 33 88 89 58 60 0d 1c d8 49 ....U..3..X`...I |
// FileSystemVirtualChannel::process_server_message: total_length=4 flags=0x00000003 chunk_data_length=4 |
// Recv done on rdpdr (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4c\x55"                                                 //rDLU |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server User Logged On |
// Sending on channel (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4c\x55"                                                 //rDLU |
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
// FileSystemVirtualChannel::process_client_message: total_length=36 flags=0x00000003 chunk_data_length=36 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x24\x00\x00\x00"                                                 //$... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x24\x00\x00\x00"                                                 //$... |
/* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00" //rDNC............ |
/* 0010 */ "\x57\x00\x49\x00\x4e\x00\x32\x00\x4b\x00\x38\x00\x2d\x00\x36\x00" //W.I.N.2.K.8.-.6. |
/* 0020 */ "\x34\x00\x00\x00"                                                 //4... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Name Request |
// ClientNameRequest: UnicodeFlag=0x1 CodePage=0 ComputerName="WIN2K8-64" |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x0c\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x43\x43\x01\x00\x0c\x00\x03\x00\x00\x00"                 //rDCC........ |
// Sent dumped on channel (-1) n bytes |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x24\x00\x00\x00"                                                 //$... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x24\x00\x00\x00"                                                 //$... |
// /* 0000 */ "\x72\x44\x4e\x43\x01\x00\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00" //rDNC............ |
// /* 0010 */ "\x57\x00\x49\x00\x4e\x00\x32\x00\x4b\x00\x38\x00\x2d\x00\x36\x00" //W.I.N.2.K.8.-.6. |
// /* 0020 */ "\x34\x00\x00\x00"                                                 //4... |
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
/* 0010 */ "\x02\x00\x00\x00\x00\x00\x06\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
/* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
/* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x01\x00\x00\x00\x05\x00\x08\x00" //................ |
/* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_client_message: Client Core Capability Response |
// FileSystemVirtualChannel::process_client_core_capability_response: numCapabilities=5 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0001 CapabilityLength=44 Version=0x2 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0002 CapabilityLength=8 Version=0x1 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0003 CapabilityLength=8 Version=0x1 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0004 CapabilityLength=8 Version=0x1 |
// FileSystemVirtualChannel::process_client_core_capability_response: CapabilityType=0x0005 CapabilityLength=8 Version=0x1 |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x54\x00\x00\x00"                                                 //T... |
// /* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,..... |
// /* 0010 */ "\x02\x00\x00\x00\x00\x00\x06\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................ |
// /* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
// /* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................ |
// /* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x01\x00\x00\x00\x05\x00\x08\x00" //................ |
// /* 0050 */ "\x01\x00\x00\x00"                                                 //.... |
// Sent dumped on channel (-1) n bytes |
// process save session info : Logon extended info |
// process save session info : Logon Errors Info |
// ErrorNotificationType=LOGON_MSG_SESSION_CONTINUE(0xFFFFFFFE) "The logon process is continuing." ErrorNotificationData=LOGON_WARNING(0x00000003) "The logon process has displayed a warning." |
// process save session info : Logon long |
// Logon Info Version 2 (data): Domain="QA" UserName="administrateur" SessionId=3 |
// Ask acl |
// sending reporting=OPEN_SESSION_SUCCESSFUL:win2k8:OK. |
// sending keepalive=ASK |
// process save session info : Logon extended info |
// process save session info : Auto-reconnect cookie |
// LogonId=3 |
// 0000 1b 00 88 9b 55 13 18 33 88 89 58 60 0d 1c d8 49 ....U..3..X`...I |
// FileSystemVirtualChannel::process_server_message: total_length=4 flags=0x00000003 chunk_data_length=4 |
// Recv done on rdpdr (-1) n bytes |
/* 0000 */ "\x01\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
/* 0000 */ "\x72\x44\x4c\x55"                                                 //rDLU |
// Dump done on rdpdr (-1) n bytes |
// FileSystemVirtualChannel::process_server_message: Server User Logged On |
// Sending on channel (-1) n bytes |
// /* 0000 */ "\x00\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x03\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x04\x00\x00\x00"                                                 //.... |
// /* 0000 */ "\x72\x44\x4c\x55"                                                 //rDLU |
// Sent dumped on channel (-1) n bytes |
} /* end indata */;

