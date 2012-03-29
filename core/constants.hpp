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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   constants definition

*/

#if !defined(CONSTANTS_H)
#define CONSTANTS_H

#include "log.hpp"

#if !defined(SHARE_PATH)
#define SHARE_PATH "/usr/local/share/rdpproxy"
#endif

#if !defined(CFG_PATH)
#define CFG_PATH "/etc/rdpproxy"
#endif

#if !defined(PERSIST_PATH)
#define PERSIST_PATH "/tmp/rdpproxy"
#endif

#if !defined(PID_PATH)
#define PID_PATH "/var/run"
#endif


#if !defined(LICENCE_PATH)
#define LICENCE_PATH "/var/certs/redemption"
#endif

#if !defined(SBIN_PATH)
#define SBIN_PATH "/usr/local/sbin"
#endif

#define LOGIN_LOGO24 "ad24b.bmp"
#define CURSOR0 "cursor0.cur"
#define CURSOR1 "cursor1.cur"
#define FONT1 "sans-10.fv1"
#define REDEMPTION_LOGO24 "xrdp24b-redemption.bmp"
#define CAPTUREFONT "FreeSans.ttf"
#define LOCKFILE "rdpproxy.pid"

#define RSAKEYS_INI "rsakeys.ini"
#define RDPPROXY_INI "rdpproxy.ini"

/* TCP port for Remote Desktop Protocol */
#define TCP_PORT_RDP                   3389

enum Capture_flags {
    DONT_CAPTURE = 0,
    VIDEO_RECORDING = 1,
    VISU_REAL_TIME = 2
};

enum {
    POINTER_TO_SEND = 0,
    POINTER_ALLREADY_SENT
};

enum {
    BRUSH_TO_SEND = 0,
    BRUSH_ALLREADY_SENT
};

enum {
    ISO_PDU_CR = 0xE0, /* Connection Request */
    ISO_PDU_CC = 0xD0, /* Connection Confirm */
    ISO_PDU_DR = 0x80, /* Disconnect Request */
    ISO_PDU_DT = 0xF0, /* Data */
    ISO_PDU_ER = 0x70, /* Error */
};

/* MCS PDU codes */
TODO("MCS PDU codes definitions should move to core/RDP/mcs.hpp")
enum {
    MCS_EDRQ =  1, /* Erect Domain Request */
    MCS_DPUM =  8, /* Disconnect Provider Ultimatum */
    MCS_AURQ = 10, /* Attach User Request */
    MCS_AUCF = 11, /* Attach User Confirm */
    MCS_CJRQ = 14, /* Channel Join Request */
    MCS_CJCF = 15, /* Channel Join Confirm */
    MCS_SDRQ = 25, /* Send Data Request */
    MCS_SDIN = 26, /* Send Data Indication */
};

enum {
    BER_TAG_MCS_CONNECT_INITIAL  = 0x7f65,
    BER_TAG_MCS_CONNECT_RESPONSE = 0x7f66,
};

enum {
    BER_TAG_BOOLEAN      =    1,
    BER_TAG_INTEGER      =    2,
    BER_TAG_OCTET_STRING =    4,
    BER_TAG_RESULT       =   10,
};

enum {
    BER_TAG_MCS_DOMAIN_PARAMS = 0x30
};

enum {
    MCS_GLOBAL_CHANNEL   = 1003,
    MCS_USERCHANNEL_BASE = 1001,
};

/* RDP secure transport constants */
enum {
    SEC_RANDOM_SIZE   = 32,
    SEC_MODULUS_SIZE  = 64,
    SEC_MAX_MODULUS_SIZE  = 256,
    SEC_PADDING_SIZE  =  8,
    SEC_EXPONENT_SIZE =  4
};

enum {
    SEC_CLIENT_RANDOM = 0x0001,
    SEC_ENCRYPT       = 0x0008,
    SEC_LOGON_INFO    = 0x0040,
    SEC_LICENCE_NEG   = 0x0080,

    SEC_TAG_PUBKEY    = 0x0006,
    SEC_TAG_KEYSIG    = 0x0008,

    SEC_RSA_MAGIC     = 0x31415352, /* RSA1 */
};

enum DATA_BLOCK_TYPE {
//  The data block that follows contains Client Core Data (section 2.2.1.3.2).
CS_CORE = 0xC001,
// The data block that follows contains Client Security Data (section 2.2.1.3.3).
CS_SECURITY = 0xC002,
// The data block that follows contains Client Network Data (section 2.2.1.3.4).
CS_NET = 0xC003,
// The data block that follows contains Client Cluster Data (section 2.2.1.3.5).
CS_CLUSTER = 0xC004,
// The data block that follows contains Client Monitor Data (section 2.2.1.3.6).
CS_MONITOR = 0xC005,
// The data block that follows contains Server Core Data (section 2.2.1.4.2).
SC_CORE = 0x0C01,
// The data block that follows contains Server Security Data (section 2.2.1.4.3).
SC_SECURITY = 0x0C02,
// The data block that follows contains Server Network Data (section 2.2.1.4.4).
SC_NET = 0x0C03
};



/* RDP licensing constants */
enum {
LICENCE_TOKEN_SIZE             = 10,
LICENCE_HWID_SIZE              = 20,
LICENCE_SIGNATURE_SIZE         = 16,
};

enum {
LICENCE_TAG_DEMAND             = 0x01, // LICENSE_REQUEST
LICENCE_TAG_AUTHREQ            = 0x02, // PLATFORM_CHALLENGE
LICENCE_TAG_ISSUE              = 0x03, // NEW_LICENSE
LICENCE_TAG_REISSUE            = 0x04, // UPGRADE_LICENSE

LICENCE_TAG_PRESENT            = 0x12, // LICENSE_INFO
LICENCE_TAG_REQUEST            = 0x13, // NEW_LICENSE_REQUEST
LICENCE_TAG_AUTHRESP           = 0x15, // PLATFORM_CHALLENGE_RESPONSE

LICENCE_TAG_RESULT             = 0xff, // ERROR_ALERT

LICENCE_TAG_USER               = 0x000f,
LICENCE_TAG_HOST               = 0x0010,
};

/* RDP PDU codes */
enum {
PDUTYPE_DEMANDACTIVEPDU        = 1,
PDUTYPE_CONFIRMACTIVEPDU       = 3,
RDP_PDU_REDIRECT               = 4, // This one is not documented...
PDUTYPE_DEACTIVATEALLPDU       = 6,
PDUTYPE_DATAPDU                = 7,
PDUTYPE_SERVER_REDIR_PKT       = 10,
};

// [MS-RDPBCGR] 2.2.8.1.1.1.2 Share Data Header (TS_SHAREDATAHEADER)
// =================================================================
enum {
PDUTYPE2_UPDATE                = 2,  // Update PDU (section 2.2.9.1.1.3)
PDUTYPE2_CONTROL               = 20, // Control PDU (section 2.2.1.15.1)
PDUTYPE2_POINTER               = 27, // Pointer Update PDU (section 2.2.9.1.1.4)
PDUTYPE2_INPUT                 = 28, // Input PDU (section 2.2.8.1.1.3)
PDUTYPE2_SYNCHRONIZE           = 31, // Synchronize PDU (section 2.2.1.14.1)
PDUTYPE2_REFRESH_RECT          = 33, // Refresh Rect PDU (section 2.2.11.2.1)
PDUTYPE2_PLAY_SOUND            = 34, // Play Sound PDU (section 2.2.9.1.1.5.1)
PDUTYPE2_SUPPRESS_OUTPUT       = 35, // Suppress Output PDU (section 2.2.11.3.1)
PDUTYPE2_SHUTDOWN_REQUEST      = 36, // Shutdown Request PDU (section 2.2.2.2.1)
PDUTYPE2_SHUTDOWN_DENIED       = 37, // Shutdown Request Denied PDU
                                     // (section 2.2.2.3.1)
PDUTYPE2_SAVE_SESSION_INFO     = 38, // Save Session Info PDU
                                     // (section 2.2.10.1.1)
PDUTYPE2_FONTLIST              = 39, // Font List PDU (section 2.2.1.18.1)
PDUTYPE2_FONTMAP               = 40, // Font Map PDU (section 2.2.1.22.1)
PDUTYPE2_SET_KEYBOARD_INDICATORS = 41,     // Set Keyboard Indicators PDU
                                           //   (section 2.2.8.2.1.1)
PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST = 43, // Persistent Key List PDU
                                           // (section 2.2.1.17.1)
PDUTYPE2_BITMAPCACHE_ERROR_PDU = 44,       // Bitmap Cache Error PDU (see
                                           // [MS-RDPEGDI] section 2.2.2.3.1)
PDUTYPE2_SET_KEYBOARD_IME_STATUS = 45,     // Set Keyboard IME Status PDU
                                           // (section 2.2.8.2.2.1)
PDUTYPE2_OFFSCRCACHE_ERROR_PDU = 46,   // Offscreen Bitmap Cache Error PDU
                                       // (see [MS-RDPEGDI] section 2.2.2.3.2)
PDUTYPE2_SET_ERROR_INFO_PDU    = 47,   // Set Error Info PDU (section 2.2.5.1.1)
PDUTYPE2_DRAWNINEGRID_ERROR_PDU = 48,  // DrawNineGrid Cache Error PDU
                                       // (see [MS-RDPEGDI] section 2.2.2.3.3)
PDUTYPE2_DRAWGDIPLUS_ERROR_PDU = 49,   // GDI+ Error PDU
                                       // (see [MS-RDPEGDI] section 2.2.2.3.4)
PDUTYPE2_ARC_STATUS_PDU        = 50,   // Auto-Reconnect Status PDU
                                       // (section 2.2.4.1.1)
};

enum {
RDP_CTL_REQUEST_CONTROL        = 1,
RDP_CTL_GRANT_CONTROL          = 2,
RDP_CTL_DETACH                 = 3,
RDP_CTL_COOPERATE              = 4,
};

enum {
RDP_UPDATE_ORDERS              = 0,
RDP_UPDATE_BITMAP              = 1,
RDP_UPDATE_PALETTE             = 2,
RDP_UPDATE_SYNCHRONIZE         = 3,
};

enum {
RDP_POINTER_SYSTEM             = 1,
RDP_POINTER_MOVE               = 3,
RDP_POINTER_COLOR              = 6,
RDP_POINTER_CACHED             = 7,
};

enum {
RDP_NULL_POINTER               = 0,
RDP_DEFAULT_POINTER            = 0x7F00,
};

enum {
RDP_INPUT_SYNCHRONIZE          = 0,
RDP_INPUT_CODEPOINT            = 1,
RDP_INPUT_VIRTKEY              = 2,
RDP_INPUT_SCANCODE             = 4,
RDP_INPUT_MOUSE                = 0x8001,
};

/* Device flags */
enum {
KBD_FLAG_RIGHT                 = 0x0001,
KBD_FLAG_EXT                   = 0x0100,
KBD_FLAG_QUIET                 = 0x1000,
KBD_FLAG_DOWN                  = 0x4000,
KBD_FLAG_UP                    = 0x8000,
};

/* These are for synchronization; not for keystrokes */
enum {
KBD_FLAG_SCROLL                = 0x0001,
KBD_FLAG_NUMLOCK               = 0x0002,
KBD_FLAG_CAPITAL               = 0x0004,
};

/* See T.128 */
enum {
RDP_KEYPRESS                   = 0,
RDP_KEYRELEASE                 = (KBD_FLAG_DOWN | KBD_FLAG_UP),
};

enum {
MOUSE_FLAG_MOVE                = 0x0800,
MOUSE_FLAG_BUTTON1             = 0x1000,
MOUSE_FLAG_BUTTON2             = 0x2000,
MOUSE_FLAG_BUTTON3             = 0x4000,
MOUSE_FLAG_BUTTON4             = 0x0280,
MOUSE_FLAG_BUTTON5             = 0x0380,
MOUSE_FLAG_DOWN                = 0x8000,
};

/* Raster operation masks */
#define ROP2_S(rop3)  (rop3 & 0xf),
#define ROP2_P(rop3)  ((rop3 & 0x3) | ((rop3 & 0x30) >> 2)),

enum {
ROP2_COPY                      = 0xc,
ROP2_XOR                       = 0x6,
ROP2_AND                       = 0x8,
ROP2_NXOR                      = 0x9,
ROP2_OR                        = 0xe,
};

enum {
MIX_TRANSPARENT                = 0,
MIX_OPAQUE                     = 1,
};

enum {
TEXT2_VERTICAL                 = 0x04,
TEXT2_IMPLICIT_X               = 0x20,
};

/* RDP bitmap cache (version 2) constants */
enum {
BMPCACHE2_C0_CELLS             = 0x78,
BMPCACHE2_C1_CELLS             = 0x78,
BMPCACHE2_C2_CELLS             = 0x150,
BMPCACHE2_NUM_PSTCELLS         = 0x9f6,
};

enum {
PDU_FLAG_FIRST                 = 0x01,
PDU_FLAG_LAST                  = 0x02,
};

/* Maps to generalCapabilitySet in T.128 page 138 */

// 2.2.1.13.1.1.1 Capability Set (TS_CAPS_SET)
// ===========================================

// The TS_CAPS_SET structure is used to describe the type and size of a
// capability set exchanged between clients and servers. All capability sets
// conform to this basic structure (see section 2.2.7).

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type identifier
// of the capability set (see below).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes
//   of the capability data, including the size of the capabilitySetType and
//   lengthCapability fields.

// capabilityData (variable): Capability set data which conforms to the
//   structure of the type given by the capabilitySetType field.

enum {
// 1 CAPSTYPE_GENERAL General Capability Set (section 2.2.7.1.1)
//
RDP_CAPSET_GENERAL             = 1,
RDP_CAPLEN_GENERAL             = 0x18,

// 2 CAPSTYPE_BITMAP Bitmap Capability Set (section 2.2.7.1.2)
RDP_CAPSET_BITMAP              = 2,
RDP_CAPLEN_BITMAP              = 0x1C,

// 3 CAPSTYPE_ORDER Order Capability Set (section 2.2.7.1.3)
RDP_CAPSET_ORDER               = 3,
RDP_CAPLEN_ORDER               = 0x58,

// 4 CAPSTYPE_BITMAPCACHE Revision 1 Bitmap Cache Capability Set (section 2.2.7.1.4.1)
RDP_CAPSET_BMPCACHE            = 4,
RDP_CAPLEN_BMPCACHE            = 0x28,

// 5 CAPSTYPE_CONTROL Control Capability Set (section 2.2.7.2.2)
RDP_CAPSET_CONTROL             = 5,
RDP_CAPLEN_CONTROL             = 0x0C,


// 7 CAPSTYPE_ACTIVATION Window Activation Capability Set (section 2.2.7.2.3)
RDP_CAPSET_ACTIVATE            = 7,
RDP_CAPLEN_ACTIVATE            = 0x0C,

// 8 CAPSTYPE_POINTER Pointer Capability Set (section 2.2.7.1.5)
RDP_CAPSET_POINTER             = 8,
RDP_CAPLEN_POINTER             = 0x0a,
RDP_CAPLEN_POINTER_MONO        = 0x08,

// 9 CAPSTYPE_SHARE Share Capability Set (section 2.2.7.2.4)
RDP_CAPSET_SHARE               = 9,
RDP_CAPLEN_SHARE               = 0x08,

// 10 CAPSTYPE_COLORCACHE Color Table Cache Capability Set (see [MS-RDPEGDI] section 2.2.1.1)
RDP_CAPSET_COLCACHE            = 10,
RDP_CAPLEN_COLCACHE            = 0x08,

// 12 CAPSTYPE_SOUND Sound Capability Set (section 2.2.7.1.11)
RDP_CAPSET_SOUND               = 12,

// 13 CAPSTYPE_INPUT Input Capability Set (section 2.2.7.1.6)
RDP_CAPSET_INPUT               = 13,
RDP_CAPLEN_INPUT               = 0x58,

// 14 CAPSTYPE_FONT Font Capability Set (section 2.2.7.2.5)
RDP_CAPSET_FONT                = 14,
RDP_CAPLEN_FONT                = 0x04,

// 15 CAPSTYPE_BRUSH Brush Capability Set (section 2.2.7.1.7)
RDP_CAPSET_BRUSHCACHE          = 15,
RDP_CAPLEN_BRUSHCACHE          = 0x08,

// 16 CAPSTYPE_GLYPHCACHE Glyph Cache Capability Set (section 2.2.7.1.8)
RDP_CAPSET_GLYPHCACHE          = 16,

// 17 CAPSTYPE_OFFSCREENCACHE Offscreen Bitmap Cache Capability Set (section 2.2.7.1.9)
RDP_CAPSET_OFFSCREENCACHE      = 17,

// 18 CAPSTYPE_BITMAPCACHE_HOSTSUPPORT Bitmap Cache Host Support Capability Set (section 2.2.7.2.1)
RDP_CAPSET_BITMAP_OFFSCREEN    = 18,
RDP_CAPLEN_BITMAP_OFFSCREEN    = 0x08,

// 19 CAPSTYPE_BITMAPCACHE_REV2 Revision 2 Bitmap Cache Capability Set (section 2.2.7.1.4.2)
RDP_CAPSET_BMPCACHE2           = 19,
RDP_CAPLEN_BMPCACHE2           = 0x28,
BMPCACHE2_FLAG_PERSIST         = ((long)1<<31),

// 20 CAPSTYPE_VIRTUALCHANNEL Virtual Channel Capability Set (section 2.2.7.1.10)
RDP_CAPSET_VIRCHAN             = 20,
RDP_CAPLEN_VIRCHAN             = 0x08,

OS_MAJOR_TYPE_UNIX             = 4,
OS_MINOR_TYPE_XSERVER          = 7,

ORDER_CAP_NEGOTIATE            = 2,
ORDER_CAP_NOSUPPORT            = 4,

// 21 CAPSTYPE_DRAWNINEGRIDCACHE DrawNineGrid Cache Capability Set ([MS-RDPEGDI] section 2.2.1.2)

// 22 CAPSTYPE_DRAWGDIPLUS Draw GDI+ Cache Capability Set ([MS-RDPEGDI] section 2.2.1.3)

// 23 CAPSTYPE_RAIL Remote Programs Capability Set ([MS-RDPERP] section 2.2.1.1.1)

// 24 CAPSTYPE_WINDOW Window List Capability Set ([MS-RDPERP] section 2.2.1.1.2)

// 25 CAPSETTYPE_COMPDESK Desktop Composition Extension Capability Set (section 2.2.7.2.8)

// 26 CAPSETTYPE_MULTIFRAGMENTUPDATE Multifragment Update Capability Set (section 2.2.7.2.6)

// 27 CAPSETTYPE_LARGE_POINTER Large Pointer Capability Set (section 2.2.7.2.7)

};

/* Logon flags */
enum {
RDP_LOGON_AUTO                 = 0x0008,
RDP_LOGON_NORMAL               = 0x0033,
RDP_COMPRESSION                = 0x0080,
RDP_LOGON_BLOB                 = 0x0100,
RDP_COMPRESSION2               = 0x0200,
RDP_LOGON_LEAVE_AUDIO          = 0x2000,
};

enum {
RDP5_DISABLE_NOTHING           = 0x00,
RDP5_NO_WALLPAPER              = 0x01,
RDP5_NO_FULLWINDOWDRAG         = 0x02,
RDP5_NO_MENUANIMATIONS         = 0x04,
RDP5_NO_THEMING                = 0x08,
RDP5_NO_CURSOR_SHADOW          = 0x20,
RDP5_NO_CURSORSETTINGS         = 0x40 /* disables cursor blinking */,
};

/* compression types */
enum {
RDP_MPPC_BIG                   = 0x01,
RDP_MPPC_COMPRESSED            = 0x20,
RDP_MPPC_RESET                 = 0x40,
RDP_MPPC_FLUSH                 = 0x80,
RDP_MPPC_DICT_SIZE             = 8192,

RDP5_COMPRESSED                = 0x80

};

/* Keymap flags */
enum {
    MapRightShiftMask          = (1 << 0),
    MapLeftShiftMask               = (1 << 1),
    MapShiftMask                   = (MapRightShiftMask | MapLeftShiftMask),

    MapRightAltMask                = (1 << 2),
    MapLeftAltMask                 = (1 << 3),
    MapAltGrMask                   = MapRightAltMask,

    MapRightCtrlMask               = (1 << 4),
    MapLeftCtrlMask                = (1 << 5),
    MapCtrlMask                    = (MapRightCtrlMask | MapLeftCtrlMask),

    MapRightWinMask                = (1 << 6),
    MapLeftWinMask                 = (1 << 7),
    MapWinMask                     = (MapRightWinMask | MapLeftWinMask),

    MapNumLockMask                 = (1 << 8),
    MapCapsLockMask                = (1 << 9),

    MapLocalStateMask              = (1 << 10),

    MapInhibitMask                 = (1 << 11),
};

#define MASK_ADD_BITS(var, mask)       (var |= mask)
#define MASK_REMOVE_BITS(var, mask)    (var &= ~mask)
#define MASK_HAS_BITS(var, mask)       ((var & mask)>0)
#define MASK_CHANGE_BIT(var, mask, active) (var = ((var & ~mask) | (active ? mask : 0)))

/* Clipboard constants, "borrowed" from GCC system headers in the w32 cross compiler */
enum {
    CF_TEXT                        = 1,
    CF_BITMAP                      = 2,
    CF_METAFILEPICT                = 3,
    CF_SYLK                        = 4,
    CF_DIF                         = 5,
    CF_TIFF                        = 6,
    CF_OEMTEXT                     = 7,
    CF_DIB                         = 8,
    CF_PALETTE                     = 9,
    CF_PENDATA                     = 10,
    CF_RIFF                        = 11,
    CF_WAVE                        = 12,
    CF_UNICODETEXT                 = 13,
    CF_ENHMETAFILE                 = 14,
    CF_HDROP                       = 15,
    CF_LOCALE                      = 16,
    CF_MAX                         = 17,
    CF_OWNERDISPLAY                = 128,
    CF_DSPTEXT                     = 129,
    CF_DSPBITMAP                   = 130,
    CF_DSPMETAFILEPICT             = 131,
    CF_DSPENHMETAFILE              = 142,
    CF_PRIVATEFIRST                = 512,
    CF_PRIVATELAST                 = 767,
    CF_GDIOBJFIRST                 = 768,
    CF_GDIOBJLAST                  = 1023,
};

/* Sound format constants */
enum {
    WAVE_FORMAT_PCM	               = 1,
    WAVE_FORMAT_ADPCM              = 2,
    WAVE_FORMAT_ALAW               = 6,
    WAVE_FORMAT_MULAW              = 7,
};

/* Virtual channel options */
enum {
    CHANNEL_OPTION_INITIALIZED     = 0x80000000,
    CHANNEL_OPTION_ENCRYPT_RDP     = 0x40000000,
    CHANNEL_OPTION_COMPRESS_RDP    = 0x00800000,
    CHANNEL_OPTION_SHOW_PROTOCOL   = 0x00200000,
};

/* NT status codes for RDPDR */
enum {
    STATUS_SUCCESS                 = 0x00000000,
    STATUS_PENDING                 = 0x00000103,

    STATUS_NO_MORE_FILES           = 0x80000006,
    STATUS_DEVICE_PAPER_EMPTY      = 0x8000000e,
    STATUS_DEVICE_POWERED_OFF      = 0x8000000f,
    STATUS_DEVICE_OFF_LINE         = 0x80000010,
    STATUS_DEVICE_BUSY             = 0x80000011,

    STATUS_INVALID_HANDLE          = 0xc0000008,
    STATUS_INVALID_PARAMETER       = 0xc000000d,
    STATUS_NO_SUCH_FILE            = 0xc000000f,
    STATUS_INVALID_DEVICE_REQUEST  = 0xc0000010,
    STATUS_ACCESS_DENIED           = 0xc0000022,
    STATUS_OBJECT_NAME_COLLISION   = 0xc0000035,
    STATUS_DISK_FULL               = 0xc000007f,
    STATUS_FILE_IS_A_DIRECTORY     = 0xc00000ba,
    STATUS_NOT_SUPPORTED           = 0xc00000bb,
    STATUS_TIMEOUT                 = 0xc0000102,
    STATUS_CANCELLED               = 0xc0000120,
};

/* RDPDR constants */
enum {
RDPDR_MAX_DEVICES              = 0x10,
DEVICE_TYPE_SERIAL             = 0x01,
DEVICE_TYPE_PARALLEL           = 0x02,
DEVICE_TYPE_PRINTER            = 0x04,
DEVICE_TYPE_DISK               = 0x08,
DEVICE_TYPE_SCARD              = 0x20,
};

enum {
FILE_DIRECTORY_FILE            = 0x00000001,
FILE_NON_DIRECTORY_FILE        = 0x00000040,
FILE_OPEN_FOR_FREE_SPACE_QUERY = 0x00800000,
};

/* RDP5 disconnect PDU */
enum {
exDiscReasonNoInfo                            = 0x0000,
exDiscReasonAPIInitiatedDisconnect            = 0x0001,
exDiscReasonAPIInitiatedLogoff                = 0x0002,
exDiscReasonServerIdleTimeout                 = 0x0003,
exDiscReasonServerLogonTimeout                = 0x0004,
exDiscReasonReplacedByOtherConnection         = 0x0005,
exDiscReasonOutOfMemory                       = 0x0006,
exDiscReasonServerDeniedConnection            = 0x0007,
exDiscReasonServerDeniedConnectionFips        = 0x0008,
exDiscReasonLicenseInternal                   = 0x0100,
exDiscReasonLicenseNoLicenseServer            = 0x0101,
exDiscReasonLicenseNoLicense                  = 0x0102,
exDiscReasonLicenseErrClientMsg               = 0x0103,
exDiscReasonLicenseHwidDoesntMatchLicense     = 0x0104,
exDiscReasonLicenseErrClientLicense           = 0x0105,
exDiscReasonLicenseCantFinishProtocol         = 0x0106,
exDiscReasonLicenseClientEndedProtocol        = 0x0107,
exDiscReasonLicenseErrClientEncryption        = 0x0108,
exDiscReasonLicenseCantUpgradeLicense         = 0x0109,
exDiscReasonLicenseNoRemoteConnections        = 0x010a,
};


/* drawable types */
enum {
WND_TYPE_BITMAP  = 0,
WND_TYPE_WND     = 1,
WND_TYPE_SCREEN  = 2,
WND_TYPE_BUTTON  = 3,
WND_TYPE_IMAGE   = 4,
WND_TYPE_EDIT    = 5,
WND_TYPE_LABEL   = 6,
WND_TYPE_COMBO   = 7,
WND_TYPE_SPECIAL = 8,
WND_TYPE_LISTBOX = 9,
};

/* button states */
enum {
BUTTON_STATE_UP   = 0,
BUTTON_STATE_DOWN = 1,
};

/* messages */

TODO(" This messages have names of messages from winuser.h but values are completely different. See what it is about ? Looks like they are internal to proxy and value is irrelevant ?")

enum {
WM_PAINT       = 3,
WM_KEYDOWN     = 15,
WM_KEYUP       = 16,
WM_SYNCHRONIZE = 17,
WM_MOUSEMOVE   = 100,
WM_LBUTTONUP   = 101,
WM_LBUTTONDOWN = 102,
WM_RBUTTONUP   = 103,
WM_RBUTTONDOWN = 104,
WM_BUTTON3UP   = 105,
WM_BUTTON3DOWN = 106,
WM_BUTTON4UP   = 107,
WM_BUTTON4DOWN = 108,
WM_BUTTON5UP   = 109,
WM_BUTTON5DOWN = 110,
WM_BUTTON_OK = 300,
WM_SCREENUPDATE = 0x4444,
WM_CHANNELDATA = 0x5555,
};



enum {
CB_ITEMCHANGE  = 300,
};

#endif
