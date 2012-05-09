/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Javier Caverni

   Logon packet


*/

#if !defined(__CORE_RDP_LOGON_HPP__)
#define __CORE_RDP_LOGON_HPP__

#include "log.hpp"

// 2.2.1.11.1.1 Info Packet (TS_INFO_PACKET)
// =========================================
// The TS_INFO_PACKET structure contains sensitive information (such as
// autologon password data) not passed to the server during the Basic Settings
// Exchange phase of the RDP Connection Sequence (see section 1.3.1.1 for an
// overview of the RDP Connection Sequence phases). The Info Packet is embedded
// in a Client Info PDU Data structure (section 2.2.1.11.1) and SHOULD be
// encrypted (see sections 5.3 and 5.4 for an overview of RDP security
// mechanisms).

// CodePage (4 bytes): A 32-bit, unsigned integer. If the flags field does not
// contain the INFO_UNICODE flag (0x00000010), then this field MUST contain the
// ANSI code page descriptor being used by the client (for a list of code pages,
// see [MSDN-CP]) to encode the character fields in the Info Packet and Extended
// Info Packet (section 2.2.1.11.1.1.1).

// However, if the flags field contains the INFO_UNICODE flag, then the CodePage
// field contains the active input locale identifier (for a list of input locale
// identifiers, see [MSFT-DIL]).

// flags (4 bytes): A 32-bit, unsigned integer. Option flags.

// +----------------------------------------+----------------------------------+
// | 0x00000001 INFO_MOUSE                  | Indicates that the client machine|
// |                                        | has a mouse attached.            |
// +----------------------------------------+----------------------------------+
// | 0x00000002 INFO_DISABLECTRLALTDEL      | Indicates that the CTRL+ALT+DEL  |
// |                                        | (or the equivalent) secure access|
// |                                        | keyboard sequence is not required|
// |                                        | at the logon prompt.             |
// +----------------------------------------+----------------------------------+
// | 0x00000008 INFO_AUTOLOGON              | The client requests auto logon   |
// |                                        | using the included user name,    |
// |                                        | password and domain.             |
// +----------------------------------------+----------------------------------+
// | 0x00000010 INFO_UNICODE                | Indicates that the character set |
// |                                        | for strings in the Info Packet   |
// |                                        | and Extended Info Packet (section|
// |                                        | 2.2.1.11.1.1.1) is Unicode. If   |
// |                                        | this flag is absent, then the    |
// |                                        | ANSI character set that is       |
// |                                        | specified by the ANSI code page  |
// |                                        | descriptor in the CodePage field |
// |                                        | is used for strings in the Info  |
// |                                        | Packet and Extended Info Packet. |
// +----------------------------------------+----------------------------------+
// | 0x00000020 INFO_MAXIMIZESHELL          | Indicates that the alternate     |
// |                                        | shell   (specified in the        |
// |                                        | AlternateShell field of the Info |
// |                                        | Packet structure) MUST be started|
// |                                        | in a maximized state.            |
// +----------------------------------------+----------------------------------+
// | 0x00000040 INFO_LOGONNOTIFY            | Indicates that the client wants  |
// |                                        | to be informed of the user name  |
// |                                        | and domain used to log on to the |
// |                                        | server, as well as the ID of the |
// |                                        | session to which the user        |
// |                                        | connected. The Save Session Info |
// |                                        | PDU (section 2.2.10.1) is sent   |
// |                                        | from the server to notify the    |
// |                                        | client of this information using |
// |                                        | a Logon Info Version 1 (section  |
// |                                        | 2.2.10.1.1.1) or Logon Info      |
// |                                        | Version 2 (section 2.2.10.1.1.2) |
// |                                        | structure.                       |
// +----------------------------------------+----------------------------------+
// | 0x00000080 INFO_COMPRESSION            | Indicates that the               |
// |                                        | CompressionTypeMask is valid and |
// |                                        | contains the highest compression |
// |                                        | package type supported by the    |
// |                                        | client.                          |
// +----------------------------------------+----------------------------------+
// | 0x00001E00 CompressionTypeMask         | Indicates the highest compression|
// |                                        | package type supported. See the  |
// |                                        | discussion which follows this    |
// |                                        | table for more information.      |
// +----------------------------------------+----------------------------------+
// | 0x00000100 INFO_ENABLEWINDOWSKEY       | Indicates that the client uses   |
// |                                        | the Windows key on               |
// |                                        | Windows-compatible eyboards.     |
// +----------------------------------------+----------------------------------+
// | 0x00002000 INFO_REMOTECONSOLEAUDIO     | Requests that audio played in a  |
// |                                        | session hosted on a remote server|
// |                                        | be played on the server using the|
// |                                        | protocol defined in [MS-RDPEA]   |
// |                                        | sections 2 and 3.                |
// +----------------------------------------+----------------------------------+
// | 0x00004000 INFO_FORCE_ENCRYPTED_CS_PDU | Indicates that all               |
// |                                        | client-to-server traffic is      |
// |                                        | encrypted when encryption is in  |
// |                                        | force. Setting this flag prevents|
// |                                        | the server from processing       |
// |                                        | unencrypted packets in           |
// |                                        | man-in-the-middle attack         |
// |                                        | scenarios. This flag is only     |
// |                                        | understood by RDP 5.2, 6.0, 6.1, |
// |                                        | and 7.0 servers.                 |
// +----------------------------------------+----------------------------------+
// | 0x00008000 INFO_RAIL                   | Indicates that the remote        |
// |                                        | connection being established is  |
// |                                        | for the purpose of launching     |
// |                                        | remote programs using the        |
// |                                        | protocol defined in [MS-RDPERP]  |
// |                                        | sections 2 and 3. This flag is   |
// |                                        | only understood by RDP 6.0, 6.1, |
// |                                        | and 7.0 servers.                 |
// +----------------------------------------+----------------------------------+
// | 0x00010000 INFO_LOGONERRORS            | Indicates a request for logon    |
// |                                        | error notifications using the    |
// |                                        | Save Session Info PDU. This flag |
// |                                        | is only understood by RDP 6.0,   |
// |                                        | 6.1, and 7.0 servers.            |
// +----------------------------------------+----------------------------------+
// | 0x00020000 INFO_MOUSE_HAS_WHEEL        | Indicates that the mouse which is|
// |                                        | connected to the client machine  |
// |                                        | has a scroll wheel. This flag is |
// |                                        | only understood by RDP 6.0, 6.1, |
// |                                        | and 7.0 servers.                 |
// +----------------------------------------+----------------------------------+
// | 0x00040000 INFO_PASSWORD_IS_SC_PIN     | Indicates that the Password field|
// |                                        | in the Info Packet contains a    |
// |                                        | smart card personal              |
// |                                        | identification number (PIN).     |
// |                                        | This flag is only understood by  |
// |                                        | RDP 6.0, 6.1, and 7.0 servers.   |
// +----------------------------------------+----------------------------------+
// | 0x00080000 INFO_NOAUDIOPLAYBACK        | Indicates that audio redirection |
// |                                        | or playback (using the protocol  |
// |                                        | defined in [MS-RDPEA] sections 2 |
// |                                        | and 3) MUST NOT take place. This |
// |                                        | flag is only understood by RDP   |
// |                                        | 6.0, 6.1, and 7.0 servers.       |
// +----------------------------------------+----------------------------------+
// | 0x00100000 INFO_USING_SAVED_CREDS      | Any user credentials sent on the |
// |                                        | wire during the RDP Connection   |
// |                                        | Sequence (see sections 1.3.1.1   |
// |                                        | and 1.3.1.2) have been retrieved |
// |                                        | from a credential store and were |
// |                                        | not obtained directly from the   |
// |                                        | user.                            |
// +----------------------------------------+----------------------------------+
// | 0x00200000 RNS_INFO_AUDIOCAPTURE       | Indicates that the redirection   |
// |                                        | of client-side audio input to a  |
// |                                        | session hosted on a remote server|
// |                                        | is supported using the protocol  |
// |                                        | defined in [MS-RDPEAI] sections  |
// |                                        | 2 and 3. This flag is only       |
// |                                        | understood by RDP 7.0 servers.   |
// +----------------------------------------+----------------------------------+
// | 0x00400000 RNS_INFO_VIDEO_DISABLE      | Indicates that video redirection |
// |                                        | or playback (using the protocol  |
// |                                        | defined in [MS-RDPEV] sections 2 |
// |                                        | and 3) MUST NOT take place. This |
// |                                        | flag is only understood by RDP   |
// |                                        | 7.0 servers.                     |
// +----------------------------------------+----------------------------------+

// The CompressionTypeMask is a 4-bit enumerated value containing the highest
// compression package support available on the client. The packages codes are:

// +-----------------------------+--------------------------------------------+
// | 0x0 PACKET_COMPR_TYPE_8K    | RDP 4.0 bulk compression (see section      |
// |                             | 3.1.8.4.1).                                |
// +-----------------------------+--------------------------------------------+
// | 0x1 PACKET_COMPR_TYPE_64K   | RDP 5.0 bulk compression (see section      |
// |                             | 3.1.8.4.2).                                |
// +-----------------------------+--------------------------------------------+
// | 0x2 PACKET_COMPR_TYPE_RDP6  | RDP 6.0 bulk compression (see [MS-RDPEGDI] |
// |                             | section 3.1.8.1).                          |
// +-----------------------------+--------------------------------------------+
// | 0x3 PACKET_COMPR_TYPE_RDP61 | RDP 6.1 bulk compression (see [MS-RDPEGDI] |
// |                             | section 3.1.8.2).                          |
// +-----------------------------+--------------------------------------------+

// If a client supports compression package n then it MUST support packages
// 0...(n - 1).

enum {
    INFO_MOUSE              = 0x00000001,
    INFO_DISABLECTRLALTDEL  = 0x00000002,
    INFO_AUTOLOGON          = 0x00000008,
    INFO_UNICODE            = 0x00000010,
    INFO_MAXIMIZESHELL      = 0x00000020,
    INFO_LOGONNOTIFY        = 0x00000040,
    INFO_COMPRESSION        = 0x00000080,
    INFO_ENABLEWINDOWSKEY   = 0x00000100,

    CompressionTypeMask     = 0x00001E00,

    INFO_REMOTECONSOLEAUDIO = 0x00002000,
    FORCE_ENCRYPTED_CS_PDU  = 0x00004000,
    INFO_RAIL               = 0x00008000,
    INFO_LOGONERRORS        = 0x00010000,
    INFO_MOUSE_HAS_WHEEL    = 0x00020000,
    INFO_PASSWORD_IS_SC_PIN = 0x00040000,
    INFO_NOAUDIOPLAYBACK    = 0x00080000,
    INFO_USING_SAVED_CREDS  = 0x00100000,
    RNS_INFO_AUDIOCAPTURE   = 0x00200000,
    RNS_INFO_VIDEO_DISABLE  = 0x00400000,
};

// to get PACKET_COMPR_TYPE: (flags & CompressonTypeMask) >> 9
// to set PACKET_COMPR_TYPE: (flags | (CompressonTypeMask & (compr_type << 9))
enum {
    PACKET_COMPR_TYPE_8K    = 0x00,
    PACKET_COMPR_TYPE_64K   = 0x01,
    PACKET_COMPR_TYPE_RDP6  = 0x02,
    PACKET_COMPR_TYPE_RDP61 = 0x03,
};

// cbDomain (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
//  character data in the Domain field. This size excludes the length of the
//  mandatory null terminator.

// cbUserName (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
//  character data in the UserName field. This size excludes the length of the
//  mandatory null terminator.

// cbPassword (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
//  character data in the Password field. This size excludes the length of the
//  mandatory null terminator.

// cbAlternateShell (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//   the character data in the AlternateShell field. This size excludes the
//   length of the mandatory null terminator.

// cbWorkingDir (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
//   character data in the WorkingDir field. This size excludes the length of
//   the mandatory null terminator.

// Domain (variable): Variable-length logon domain of the user (the length in
//   bytes is given by the cbDomain field). The maximum length allowed by
//   RDP 4.0 and RDP 5.0 servers is 52 bytes (including the mandatory null
//   terminator). RDP 5.1, 5.2, 6.0, 6.1, and 7.0 allow a maximum length of
//   512 bytes (including the mandatory null terminator). The field MUST contain
//   at least a null terminator character in Windows-1252 or Unicode format
//   (depending on the presence of the INFO_UNICODE flag).

// UserName (variable): Variable-length logon user name of the user (the length
//  in bytes is given by the cbUserName field). The maximum length allowed by
//  RDP 4.0 servers is 44 bytes (including the mandatory null terminator).
//  RDP 5.0, 5.1, 5.2, 6.0, 6.1, and 7.0 allow a maximum length of 512 bytes
//  (including the mandatory null terminator). The field MUST contain at least
//  a null terminator character in Windows-1252 or Unicode format (depending on
//  the presence of the INFO_UNICODE flag).

// Password (variable): Variable-length logon password of the user (the length
//  in bytes is given by the cbPassword field). The maximum length allowed by
//  RDP 4.0 and RDP 5.0 servers is 32 bytes (including the mandatory null
//  terminator). RDP 5.1, 5.2, 6.0, 6.1, and 7.0 allow a maximum length of
//  512 bytes (including the mandatory null terminator). The field MUST contain
//  at least a null terminator character in Windows-1252 or Unicode format
//  (depending on the presence of the INFO_UNICODE flag).

// AlternateShell (variable): Variable-length path to the executable file of an
//   alternate shell, e.g. "c:\dir\prog.exe" (the length in bytes is given by
//   the cbAlternateShell field). The maximum allowed length is 512 bytes
//   (including the mandatory null terminator). This field MUST only be
//   initialized if the client is requesting a shell other than the default.
//   The field MUST contain at  least a null terminator character in
//   Windows-1252 or Unicode format (depending on the presence of the
//   INFO_UNICODE flag).

// WorkingDir (variable): Variable-length directory that contains the executable
// file specified in the AlternateShell field or any related files (the length
// in bytes is given by the cbWorkingDir field). The maximum allowed length is
// 512 bytes (including the mandatory null terminator). This field MAY be
// initialized if the client is requesting a shell other than the default. The
// field MUST contain at least a null terminator character in Windows-1252 or
// Unicode format (depending on the presence of the INFO_UNICODE flag).

// extraInfo (variable): Optional and variable-length extended information
// used in RDP 5.0, 5.1, 5.2, 6.0, 6.1, and 7.0, and specified in section
// 2.2.1.11.1.1.1.

// 2.2.1.11.1.1.1 Extended Info Packet (TS_EXTENDED_INFO_PACKET)
// =============================================================
// The TS_EXTENDED_INFO_PACKET structure contains user information specific to
// RDP 5.0, 5.1, 5.2, 6.0, 6.1, and 7.0.

// clientAddressFamily (2 bytes): A 16-bit, unsigned integer. The numeric socket
// descriptor for the client address type.

// 0x00002 AF_INET The clientAddress field contains an IPv4 address.
// 0x0017 AF_INET6 The clientAddress field contains an IPv6 address.

// cbClientAddress (2 bytes): A 16-bit, unsigned integer. The size in bytes of
// the character data in the clientAddress field. This size includes the length
// of the mandatory null terminator.

// clientAddress (variable): Variable-length textual representation of the
// client IPv4 or IPv6 address. The maximum allowed length (including the
// mandatory null terminator) is 64 bytes for RDP 5.0, 5.1, 5.2, and 6.0, and
// 80 bytes for RDP 6.1 and 7.0.

// cbClientDir (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
// character data in the clientDir field. This size includes the length of the
// mandatory null terminator.

// clientDir (variable): Variable-length directory that contains either (a) the
// folder path on the client machine from which the client software is being run
//, or (b) the full path of the software module implementing the client (see
// section 4.1.10 for an example). The maximum allowed length is 512 bytes
// (including the mandatory null terminator).

// clientTimeZone (172 bytes): A TS_TIME_ZONE_INFORMATION structure (section
// 2.2.1.11.1.1.1.1) that contains time zone information for a client. This
// packet is used by RDP 5.2, 6.0, 6.1, and 7.0 servers.

// clientSessionId (4 bytes): A 32-bit, unsigned integer. This field was added
// in RDP 5.1 and is currently ignored by the server. It SHOULD be set to 0.

// performanceFlags (4 bytes): A 32-bit, unsigned integer. It specifies a list
// of server desktop shell features to enable or disable in the session (with
// the goal of optimizing bandwidth usage). It is used by RDP 5.1, 5.2, 6.0,
// 6.1, and 7.0 servers.

// +--------------------------------------------+------------------------------+
// | 0x00000001 PERF_DISABLE_WALLPAPER          | Disable desktop wallpaper.   |
// +--------------------------------------------+------------------------------+
// | 0x00000002 PERF_DISABLE_FULLWINDOWDRAG     | Disable full-window drag     |
// |                                            |(only the window outline is   |
// |                                            |displayed when the window is  |
// |                                            | moved).                      |
// +--------------------------------------------+------------------------------+
// | 0x00000004 PERF_DISABLE_MENUANIMATIONS     | Disable menu animations.     |
// +--------------------------------------------+------------------------------+
// | 0x00000008 PERF_DISABLE_THEMING            | Disable user interface theme.|
// +--------------------------------------------+------------------------------+
// | 0x00000010 PERF_RESERVED1                  | Reserved for future use.     |
// +--------------------------------------------+------------------------------+
// | 0x00000020 PERF_DISABLE_CURSOR_SHADOW      | Disable mouse cursor shadows.|
// +--------------------------------------------+------------------------------+
// | 0x00000040 PERF_DISABLE_CURSORSETTINGS     | Disable cursor blinking.     |
// +--------------------------------------------+------------------------------+
// | 0x00000080 PERF_ENABLE_FONT_SMOOTHING      | Enable font smoothing.       |
// +--------------------------------------------+------------------------------+
// | 0x00000100 PERF_ENABLE_DESKTOP_COMPOSITION | Enable Desktop Composition.  |
// +--------------------------------------------+------------------------------+
// | 0x80000000 PERF_RESERVED2                  | Reserved for future use.     |
// +--------------------------------------------+------------------------------+

enum {
    PERF_DISABLE_WALLPAPER          = 0x00000001,
    PERF_DISABLE_FULLWINDOWDRAG     = 0x00000002,
    PERF_DISABLE_MENUANIMATIONS     = 0x00000004,
    PERF_DISABLE_THEMING            = 0x00000008,
    PERF_RESERVED1                  = 0x00000010,
    PERF_DISABLE_CURSOR_SHADOW      = 0x00000020,
    PERF_DISABLE_CURSORSETTINGS     = 0x00000040,
    PERF_ENABLE_FONT_SMOOTHING      = 0x00000080,
    PERF_ENABLE_DESKTOP_COMPOSITION = 0x00000100,
    PERF_RESERVED2                  = 0x80000000,
};

// cbAutoReconnectLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
// of the cookie specified by the autoReconnectCookie field. This field is only
// read by RDP 5.2, 6.0, 6.1, and 7.0 servers.

// autoReconnectCookie (28 bytes): Buffer containing an ARC_CS_PRIVATE_PACKET
// structure (section 2.2.4.3). This buffer is a unique cookie that allows a
// disconnected client to seamlessly reconnect to a previously established
// session (see section 5.5 for more details). The autoReconnectCookie field is
// only read by RDP 5.2, 6.0, 6.1, and 7.0 servers and the maximum allowed
// length is 128 bytes.

// reserved1 (2 bytes): This field is reserved for future use and has no affect
// on RDP wire traffic. If this field is present, the reserved2 field MUST
// be present.

// reserved2 (2 bytes): This field is reserved for future use and has no affect
// on RDP wire traffic. This field MUST be present if the reserved1 field
// is present.

// 2.2.1.11.1.1.1.1 Time Zone Information (TS_TIME_ZONE_INFORMATION)
// =================================================================
// The TS_TIME_ZONE_INFORMATION structure contains client time zone information.

// Bias (4 bytes): A 32-bit, unsigned integer that contains the current bias for
// local time translation on the client. The bias is the difference, in minutes,
// between Coordinated Universal Time (UTC) and local time. All translations
// between UTC and local time are based on the following formula:
// UTC = local time + bias

    // Note: really this is a signed integer as Bias can be negative!

// StandardName (64 bytes): An array of 32 Unicode characters. The descriptive
// name for standard time on the client.

// StandardDate (16 bytes): A TS_SYSTEMTIME (section 2.2.1.11.1.1.1.1.1)
// structure that contains the date and local time when the transition from
// daylight saving time to standard time occurs on the client. If this field
// contains a valid date and time, then the DaylightDate field MUST also contain
// a valid date and time. If the wYear, wMonth, wDayOfWeek, wDay, wHour,
// wMinute, wSecond, and wMilliseconds fields are all set to zero, then the
// client does not support daylight saving time.

// StandardBias (4 bytes): A 32-bit, unsigned integer that contains the bias
// value to be used during local time translations that occur during standard
// time. This value is added to the value of the Bias field to form the bias
// used during standard time. This field MUST be ignored if a valid date and
// time is not specified in the StandardDate field or the wYear, wMonth,
// wDayOfWeek, wDay, wHour, wMinute, wSecond, and wMilliseconds fields of the
// StandardDate field are all set to zero.

// DaylightName (64 bytes): An array of 32 Unicode characters. The descriptive
// name for daylight saving time on the client.

// DaylightDate (16 bytes): A TS_SYSTEMTIME (section 2.2.1.11.1.1.1.1.1)
// structure that contains a date and local time when the transition from
// standard time to daylight saving time occurs on the client. If this field
// contains a valid date and time, then the StandardDate field MUST also contain
// a valid date and time. If the wYear, wMonth, wDayOfWeek, wDay, wHour,
// wMinute, wSecond, and wMilliseconds fields are all set to zero, then the
// client does not support daylight saving time.

// DaylightBias (4 bytes): A 32-bit, unsigned integer that contains the bias
// value to be used during local time translations that occur during daylight
// saving time. This value is added to the value of the Bias field to form the
// bias used during daylight saving time. This field MUST be ignored if a valid
// date and time is not specified in the DaylightDate field or the wYear,
// wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, and wMilliseconds fields
// of the DaylightDate field are all set to zero.

// 2.2.1.11.1.1.1.1.1 System Time (TS_SYSTEMTIME)
// ==============================================
// The TS_SYSTEMTIME structure contains a date and local time when the
// transition occurs between daylight saving time to standard time occurs or
// standard time to daylight saving time.

// wYear (2 bytes): A 16-bit, unsigned integer. This field MUST be set to zero.

// wMonth (2 bytes): A 16-bit, unsigned integer. The month when transition
// occurs.

// 1=January, 2=February, 3=March, 4=April, 5=May, 6=June, 7=July, 8=August
// 9=September, 10=October, 11=November, 12=December

// wDayOfWeek (2 bytes): A 16-bit, unsigned integer. The day of the week when
// transition occurs.

// 0=Sunday, 1=Monday, 2=Tuesday, 3=Wednesday, 4=Thursday, 5=Friday, 6=Saturday

// wDay (2 bytes): A 16-bit, unsigned integer. The occurrence of wDayOfWeek
// within the month when the transition takes place.

// 1 First occurrence of wDayOfWeek
// 2 Second occurrence of wDayOfWeek
// 3 Third occurrence of wDayOfWeek
// 4 Fourth occurrence of wDayOfWeek
// 5 Last occurrence of wDayOfWeek

// wHour (2 bytes): A 16-bit, unsigned integer. The hour when transition occurs
// (0 to 23).

// wMinute (2 bytes): A 16-bit, unsigned integer. The minute when transition
// occurs (0 to 59).

// wSecond (2 bytes): A 16-bit, unsigned integer. The second when transition
// occurs (0 to 59).

// wMilliseconds (2 bytes): A 16-bit, unsigned integer. The millisecond when
// transition occurs (0 to 999).

// 5.5 Automatic Reconnection
// ==========================

// The Automatic Reconnection feature allows a client to reconnect to an
// existing session (after a short-term network failure has occurred) without
// having to resend the user's credentials to the server. A connection which
// employs Automatic Reconnection proceeds as follows:

// 1. The user logs in to a new or existing session. As soon as the user has
// been authenticated, a Server Auto-Reconnect Packet (section 2.2.4.2) is
// generated by the server and sent to the client in the Save Session Info PDU
// (section 2.2.10.1). The Auto-Reconnect Packet (also called the auto-reconnect
// cookie) contains a 16-byte cryptographically secure random number (called the
// auto-reconnect random) and the ID of the session to which the user has
// connected.

// 2. The client receives the cookie and stores it in memory, never allowing
// programmatic access to it.

// 3. In the case of a disconnection due to a network error, the client attempts
// to reconnect to the server by trying to reconnect continuously or for a
// predetermined number of times. Once it has connected, the client and server
// may exchange large random numbers (the client and server random specified in
// section 5.3.4). If Enhanced RDP Security (section 5.4) is in effect, no
// client random is sent to the server (see section 5.3.2).

// 4. The client derives a 16-byte security verifier from the random number
// contained in the auto-reconnect cookie received in Step 2. This security
// verifier is wrapped in a Client Auto-Reconnect Packet (section 2.2.4.3) and
// sent to the server as part of the extended information (see section
// 2.2.1.11.1.1.1) of the Client Info PDU (section 2.2.1.11).

// The auto-reconnect random is used to key the HMAC function (see [RFC2104]),
// which uses MD5 as the iterative hash function. The security verifier is
// derived by applying the HMAC to the client random received in Step 3.

// SecurityVerifier = HMAC(AutoReconnectRandom, ClientRandom)

// The one-way HMAC transformation prevents an unauthenticated server from
// obtaining the original auto-reconnect random and replaying it for the purpose
// of connecting to the user's existing session.

// When Enhanced RDP Security is in effect the client random value is not
// generated (see section 5.3.2). In this case, for the purpose of generating
// the security verifier, the client random is assumed to be an array of 16 zero
//  bytes, that is, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }.

// This effectively means that the derived security verifier will always have
// the same value when carrying out auto-reconnect under the Enhanced RDP
// Security. Hence, care must be taken to authenticate the identity of the
// server to which the client is reconnecting, ensuring that the identity has
// not changed in the period between connections.

// 5. When the server receives the Client Auto-Reconnect Packet, it looks up the
// auto-reconnect random for the session and computes the security verifier
// using the client random (the same calculation executed by the client). If the
// security verifier value which the client transmitted matches the one computed
// by the server, the client is granted access. At this point, the server has
// confirmed that the client requesting auto-reconnection was the last one
// connected to the session in question.

// 6. If the check in Step 5 passes, then the client is automatically
// reconnected to the desired session; otherwise the client must obtain the
// user's credentials to regain access to the session on the remote server.

// The auto-reconnect cookie associated with a given session is flushed and
// regenerated whenever a client connects to the session or the session is
// reset. This ensures that if a different client connects to the session, then
// any previous clients which were connected can no longer use the
// auto-reconnect mechanism to connect. Furthermore, the server invalidates and
// updates the cookie at hourly intervals, sending the new cookie to the client
// in the Save Session Info PDU.

static inline void send_logon_info_packet(Stream & stream, const char * domain, const char * username, const char * password, const char * program, const char * directory, int use_rdp5, int rdp5_performanceflags)
{
    uint32_t flags = INFO_MOUSE
                   | INFO_DISABLECTRLALTDEL
                   | ((strlen(password) > 0) * INFO_AUTOLOGON)
                   | INFO_UNICODE
                   | INFO_MAXIMIZESHELL
                   ;

    time_t t = time(NULL);
    time_t tzone;

    stream.out_uint32_le(0);
    stream.out_uint32_le(flags);

    stream.out_uint16_le(2 * strlen(domain));
    stream.out_uint16_le(2 * strlen(username));
    stream.out_uint16_le(2 * strlen(password));
    stream.out_uint16_le(2 * strlen(program));
    stream.out_uint16_le(2 * strlen(directory));
    stream.out_unistr(domain);
    stream.out_unistr(username);
    if (flags & RDP_LOGON_AUTO){
        stream.out_unistr(password);
    }
    else{
        stream.out_uint16_le(0);
    }
    stream.out_unistr(program);
    stream.out_unistr(directory);

    if(!use_rdp5){
        LOG(LOG_INFO, "send login info (RDP4-style) %s:%s", domain, username);
    }
    else {
        LOG(LOG_INFO, "send extended login info (RDP5-style) %x %s:%s",flags,
            domain,
            username);
    }
    if (use_rdp5){
        // The WAB does not send it's IP to server. Is it what we want ?
        // rdesktop sends the faked IP below
        const char * ip_source = "10.10.9.161";

        stream.out_uint16_le(0x00002);
        stream.out_uint16_le(2 * strlen(ip_source) + 2);
        stream.out_unistr(ip_source);
        const char path[] = "C:\\WINNT\\System32\\mstscax.dll";
        stream.out_uint16_le(2 * sizeof(path));
        stream.out_unistr(path);

        tzone = (mktime(gmtime(&t)) - mktime(localtime(&t))) / 60;
        stream.out_uint32_le(tzone);

        stream.out_unistr("GTB, normaltid");
        stream.out_clear_bytes(62 - 2 * strlen("GTB, normaltid"));

        stream.out_uint32_le(0x0a0000);
        stream.out_uint32_le(0x050000);
        stream.out_uint32_le(3);
        stream.out_uint32_le(0);
        stream.out_uint32_le(0);

        stream.out_unistr("GTB, sommartid");
        stream.out_clear_bytes(62 - 2 * strlen("GTB, sommartid"));

        stream.out_uint32_le(0x30000);
        stream.out_uint32_le(0x050000);
        stream.out_uint32_le(2);
        stream.out_uint32_le(0);
        stream.out_uint32_le(0xffffffc4);

        stream.out_uint32_le(0xfffffffe); // session id

        stream.out_uint32_le(rdp5_performanceflags);

        stream.out_uint16_le(0);

        stream.out_uint16_le(0);
        stream.out_uint16_le(0);
    }
}

static inline void recv_logon_info(Stream & stream, uint32_t & flags, uint32_t & rdp5_performanceflags, char * domain, char * username, char * password, char * program, char * directory)
{
    uint32_t codepage = stream.in_uint32_le();
    flags = stream.in_uint32_le();

    unsigned len_domain = stream.in_uint16_le() + 2;
    unsigned len_user = stream.in_uint16_le() + 2;
    unsigned len_password = stream.in_uint16_le() + 2;
    unsigned len_program = stream.in_uint16_le() + 2;
    unsigned len_directory = stream.in_uint16_le() + 2;
    stream.in_uni_to_ascii_str(domain, len_domain);
    stream.in_uni_to_ascii_str(username, len_user);

    // We have a password available
    if (flags & RDP_LOGON_AUTO) {
        stream.in_uni_to_ascii_str(password, len_password);
    } else {
        stream.in_skip_bytes(len_password);
    }
    stream.in_uni_to_ascii_str(program, len_program);
    stream.in_uni_to_ascii_str(directory, len_directory);

    TODO("we should rely on available data size instead of INFO_ENABLEWINDOWSKEY flag")
    if (flags & INFO_ENABLEWINDOWSKEY) {
        LOG(LOG_DEBUG, "RDP-5 Style logon");
        stream.in_skip_bytes(2);
        unsigned len_ip = stream.in_uint16_le();
        char tmpdata[256];
        stream.in_uni_to_ascii_str(tmpdata, len_ip);
        unsigned len_dll = stream.in_uint16_le();
        stream.in_uni_to_ascii_str(tmpdata, len_dll);
        stream.in_skip_bytes(172); /* skip time data */
        rdp5_performanceflags = stream.in_uint32_le();
        // more data here
        TODO("We should take care of remaining data if any")
    }
}


#endif
