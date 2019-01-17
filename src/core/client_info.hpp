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

   header file for use with libxrdp.so / xrdp.dll

*/


#pragma once

#include <array>
#include <cstring>

#include "core/misc.hpp"
#include "core/RDP/gcc.hpp"
#include "core/RDP/logon.hpp"
#include "core/RDP/capabilities/bmpcache2.hpp"
#include "core/RDP/capabilities/cap_bitmap.hpp"
#include "core/RDP/capabilities/cap_bmpcache.hpp"
#include "core/RDP/capabilities/cap_glyphcache.hpp"
#include "core/RDP/capabilities/general.hpp"
#include "core/RDP/capabilities/largepointer.hpp"
#include "core/RDP/capabilities/offscreencache.hpp"
#include "core/RDP/capabilities/order.hpp"
#include "core/RDP/capabilities/multifragmentupdate.hpp"
#include "core/RDP/capabilities/rail.hpp"
#include "core/RDP/capabilities/window.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "gdi/screen_info.hpp"
#include "utils/get_printable_password.hpp"
#include "utils/sugar/cast.hpp"

struct ClientInfo
{
    ScreenInfo screen_info;

    // TODO CacheInfo { entrie, is_persistent, size }; or Cache5Info { entries[5], ... };
    /* bitmap cache info */
    uint8_t number_of_cache = 0;
    uint32_t cache1_entries = 600;
    bool     cache1_persistent = false;
    uint32_t cache1_size = 256;
    uint32_t cache2_entries = 300;
    bool     cache2_persistent = false;
    uint32_t cache2_size = 1024;
    uint32_t cache3_entries = 262;
    bool     cache3_persistent = false;
    uint32_t cache3_size = 4096;
    uint32_t cache4_entries = 262;
    bool     cache4_persistent = false;
    uint32_t cache4_size = 4096;
    uint32_t cache5_entries = 262;
    bool     cache5_persistent = false;
    uint32_t cache5_size = 4096;
    int cache_flags = 0;
    int bitmap_cache_version = 0; /* 0 = original version, 2 = v2 */

    /* pointer info */
    int pointer_cache_entries = 0;
    bool supported_new_pointer_update = false;

    //uint32_t desktop_cache = 0;
    bool use_compact_packets = false; /* rdp5 smaller packets */
    char hostname[16] = {0};
    int build = 0;
    int keylayout = 0;
    char username[sizeof(InfoPacket::UserName)] = {0};
    char password[sizeof(InfoPacket::Password)] = {0};
    char domain[sizeof(InfoPacket::Domain)] = {0};

    int rdp_compression = 0;
    int rdp_compression_type = 0;
    int rdp_autologin = 0;
    int encryptionLevel; /* 1, 2, 3 = low, medium, high */
    bool has_sound_code = false; /* 1 = leave sound at server */
    int is_mce = 0;
    uint32_t rdp5_performanceflags = 0;
    int brush_cache_code = 0; /* 0 = no cache 1 = 8x8 standard cache
                           2 = arbitrary dimensions */
    bool console_session = false;

    bool remote_program          = false;
    bool remote_program_enhanced = false;

    char alternate_shell[512] = { 0 };
    char working_dir[512] = { 0 };

    GCC::UserData::CSMonitor cs_monitor;

    ClientTimeZone client_time_zone;

    uint16_t cbAutoReconnectCookie = 0;
    uint8_t  autoReconnectCookie[28] = { 0 };

    GlyphCache::number_of_entries_t number_of_entries_in_glyph_cache = { {
          NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES
        , NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES
        , NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES
        , NUMBER_OF_GLYPH_CACHE_ENTRIES
    } };

    LargePointerCaps        large_pointer_caps;
    MultiFragmentUpdateCaps multi_fragment_update_caps;

    GeneralCaps             general_caps;
    BitmapCaps              bitmap_caps;
    OrderCaps               order_caps;
    BmpCacheCaps            bmp_cache_caps;
    BmpCache2Caps           bmp_cache_2_caps;
    OffScreenCacheCaps      off_screen_cache_caps;
    GlyphCacheCaps          glyph_cache_caps;
    RailCaps                rail_caps;
    WindowListCaps          window_list_caps;

    ClientInfo() = default;

    void process_logon_info( InStream & stream
                           , bool ignore_logon_password
                           , uint32_t performance_flags_default
                           , uint32_t performance_flags_force_present
                           , uint32_t performance_flags_force_not_present
                           , uint32_t password_printing_mode
                           , bool verbose
                           )
    {
        InfoPacket infoPacket;

        infoPacket.recv(stream);
        if (verbose) {
            infoPacket.log("Receiving from client", password_printing_mode);
        }

        memcpy(this->domain, infoPacket.Domain, sizeof(infoPacket.Domain));
        memcpy(this->username, infoPacket.UserName, sizeof(infoPacket.UserName));
        if (!ignore_logon_password){
            memcpy(this->password, infoPacket.Password, sizeof(infoPacket.Password));
        }
        else{
            if (verbose){
                array_view_const_char const av = ::get_printable_password(
                    {this->password, strlen(this->password)}, password_printing_mode);
                LOG(LOG_INFO, "client info: logon password %.*s ignored",
                    int(av.size()), av.data());
            }
        }

        this->rdp5_performanceflags = infoPacket.extendedInfoPacket.performanceFlags;

        if (this->rdp5_performanceflags == 0){
            this->rdp5_performanceflags = performance_flags_default;
        }
        this->rdp5_performanceflags |= performance_flags_force_present;
        this->rdp5_performanceflags &= ~performance_flags_force_not_present;

        if (verbose){
            LOG(LOG_INFO,
                "client info: performance flags before=0x%08X after=0x%08X default=0x%08X present=0x%08X not-present=0x%08X",
                infoPacket.extendedInfoPacket.performanceFlags, this->rdp5_performanceflags, performance_flags_default,
                performance_flags_force_present, performance_flags_force_not_present);
        }

        const uint32_t mandatory_flags = INFO_MOUSE
                                       | INFO_DISABLECTRLALTDEL
                                       | INFO_UNICODE
                                       // | INFO_MAXIMIZESHELL // unnecessary. Following by "'RDP' failed at RDP_GET_LICENSE state" if absent.
                                       ;

        if ((infoPacket.flags & mandatory_flags) != mandatory_flags){
            throw Error(ERR_SEC_PROCESS_LOGON_UNKNOWN_FLAGS);
        }
        if (infoPacket.flags & INFO_REMOTECONSOLEAUDIO) {
            this->has_sound_code = true;
        }
        // TODO for now not allowing both autologon and mce
        if ((infoPacket.flags & INFO_AUTOLOGON) && (!this->is_mce)){
            this->rdp_autologin = 1;
        }
        if (infoPacket.flags & INFO_COMPRESSION){
            this->rdp_compression      = 1;
            this->rdp_compression_type = ((infoPacket.flags & CompressionTypeMask) >> 9);
        }

        this->remote_program          = (infoPacket.flags & INFO_RAIL);
        this->remote_program_enhanced = (infoPacket.flags & INFO_HIDEF_RAIL_SUPPORTED);

        if (0 != ::strcasecmp(::char_ptr_cast(infoPacket.AlternateShell), DUMMY_REMOTEAPP) &&
            (::strcasestr(::char_ptr_cast(infoPacket.AlternateShell), DUMMY_REMOTEAPP ":") != ::char_ptr_cast(infoPacket.AlternateShell))) {
            snprintf(this->alternate_shell, sizeof(this->alternate_shell), "%s", infoPacket.AlternateShell);
            snprintf(this->working_dir,     sizeof(this->working_dir),     "%s", infoPacket.WorkingDir    );
        }

        this->client_time_zone = infoPacket.extendedInfoPacket.clientTimeZone;

        if (infoPacket.extendedInfoPacket.cbAutoReconnectLen) {
            this->cbAutoReconnectCookie = infoPacket.extendedInfoPacket.cbAutoReconnectLen;

            ::memcpy(this->autoReconnectCookie, infoPacket.extendedInfoPacket.autoReconnectCookie, sizeof(this->autoReconnectCookie));
        }
    }
};

