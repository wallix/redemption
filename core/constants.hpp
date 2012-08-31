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

#if !defined(__CONSTANTS_HPP__)
#define __CONSTANTS_HPP__

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

enum {
    POINTER_TO_SEND = 0,
    POINTER_ALLREADY_SENT
};

enum {
    BRUSH_TO_SEND = 0,
    BRUSH_ALLREADY_SENT
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

enum {
    MOUSE_FLAG_MOVE                = 0x0800,
    MOUSE_FLAG_BUTTON1             = 0x1000,
    MOUSE_FLAG_BUTTON2             = 0x2000,
    MOUSE_FLAG_BUTTON3             = 0x4000,
    MOUSE_FLAG_BUTTON4             = 0x0280,
    MOUSE_FLAG_BUTTON5             = 0x0380,
    MOUSE_FLAG_DOWN                = 0x8000,
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

///* Virtual channel options */
//enum {
//    CHANNEL_OPTION_INITIALIZED     = 0x80000000,
//    CHANNEL_OPTION_ENCRYPT_RDP     = 0x40000000,
//    CHANNEL_OPTION_COMPRESS_RDP    = 0x00800000,
//    CHANNEL_OPTION_SHOW_PROTOCOL   = 0x00200000,
//};

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

/* button states */
enum {
BUTTON_STATE_UP   = 0,
BUTTON_STATE_DOWN = 1,
};

enum {
CB_ITEMCHANGE  = 300,
};

#endif
