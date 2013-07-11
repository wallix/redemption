/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou

    rdp transparent module main header file
*/

#ifndef _REDEMPTION_MOD_RDP_TRANSPARENT_HPP_
#define _REDEMPTION_MOD_RDP_TRANSPARENT_HPP_

#include "front_api.hpp"
#include "mod_api.hpp"
#include "RDP/nego.hpp"

struct mod_rdp_transparent : public mod_api {
    FrontAPI & front;

    enum {
          EARLY
        , WAITING_SYNCHRONIZE
        , WAITING_CTL_COOPERATE
        , WAITING_GRANT_CONTROL_COOPERATE
        , WAITING_FONT_MAP
        , UP_AND_RUNNING
    } connection_finalization_state;

    uint32_t verbose;

    RdpNego nego;

    mod_rdp_transparent( Transport & trans
                       , const char * target_user
                       , const char * target_password
                       , FrontAPI & front
                       , const bool tls
                       , uint32_t verbose = 0)
            : mod_api(0, 0)
            , front(front)
            , connection_finalization_state(EARLY)
            , verbose(verbose)
            , nego(tls, &trans, target_user) {
        if (this->verbose & 1) {
            LOG(LOG_INFO, "Creation of new mod 'RDP Transparent'");
        }
    }

    virtual ~mod_rdp_transparent() {}

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) {}

    virtual void rdp_input_scancode( long param1, long param2, long param3, long param4
                                   , Keymap2 * keymap) {}

    virtual void rdp_input_synchronize( uint32_t time, uint16_t device_flags, int16_t param1
                                      , int16_t param2) {}

    virtual void rdp_input_invalidate(const Rect & r) {}

    // management of module originated event ("data received from server")
    // return non zero if module is "finished", 0 if it's still running
    // the null module never finish and accept any incoming event
    virtual void draw_event(void) {
        
    }

    virtual void begin_update() {}
    virtual void end_update() {}

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) {}
    virtual void draw(const RDPScrBlt &     cmd, const Rect & clip) {}
    virtual void draw(const RDPDestBlt &    cmd, const Rect & clip) {}
    virtual void draw(const RDPPatBlt &     cmd, const Rect & clip) {}
    virtual void draw(const RDPMemBlt &     cmd, const Rect & clip, const Bitmap & bmp) {}
    virtual void draw(const RDPMem3Blt &    cmd, const Rect & clip, const Bitmap & bmp) {}
    virtual void draw(const RDPLineTo &     cmd, const Rect & clip) {}
    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip) {}

    virtual void server_draw_text( int16_t x, int16_t y, const char * text, uint32_t fgcolor
                                 , uint32_t bgcolor, const Rect & clip) {}

    virtual void text_metrics(const char * text, int & width, int & height) {}

    virtual void send_to_front_channel( const char * const mod_channel_name, uint8_t * data
                                      , size_t length, size_t chunk_size, int flags) {}

    virtual void send_to_mod_channel( const char * const front_channel_name
                                    , Stream & chunk
                                    , size_t length
                                    , uint32_t flags) {
        if (this->verbose & 16) {
            LOG(LOG_INFO, "mod_rdp_transparent::send_to_mod_channel");
            LOG(LOG_INFO, "sending to channel %s", front_channel_name);
        }
    }
};  // struct mod_rdp_transparent

#endif  // #ifndef _REDEMPTION_MOD_RDP_TRANSPARENT_HPP_
