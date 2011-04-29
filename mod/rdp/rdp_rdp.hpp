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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   rdp layer at rdp module, top level of stack architecture

*/

#if !defined(__RDP_RDP_HPP__)
#define __RDP_RDP_HPP__

#include "rdp_sec.hpp"
#include "rdp_orders.hpp"
#include "client_mod.hpp"

/* rdp */
struct rdp_rdp {
    rdp_sec sec_layer;
    rdp_orders orders;
    int share_id;
    int use_rdp5;
    int bitmap_compression;
    int bitmap_cache;
    int desktop_save;
    int polygon_ellipse_orders;
    int chan_id;
    int version;

    char password[256];
    char domain[256];
    char program[256];
    char directory[256];
    int keylayout;
    bool console_session;
    int bpp;

    struct rdp_cursor cursors[32];
    rdp_rdp(struct mod_rdp* owner, Transport *t, const char * username, const char * password, const char * hostname, vector<mcs_channel_item*> channel_list, int rdp_performance_flags, int width, int height, int bpp, int keylayout, bool console_session)
        #warning initialize members through constructor
        : sec_layer(t, this->use_rdp5, hostname, username), bpp(bpp)
        {
            LOG(LOG_INFO, "rdp_rdp login:%s host=%s\n", username, hostname);
            this->share_id = 0;
            this->use_rdp5 = 0;
            this->bitmap_compression = 1;
            this->bitmap_cache = 1;
            this->desktop_save = 0;
            this->polygon_ellipse_orders = 0;
            this->console_session = console_session;

            memset(this->password, 0, 256);
            strcpy(this->password, password);

            memset(this->domain, 0, 256);
            memset(this->program, 0, 256);
            memset(this->directory, 0, 256);

            this->keylayout = keylayout;
            LOG(LOG_INFO, "Server key layout is %x\n", this->keylayout);

            #warning I should change that to RAII by merging instanciation of sec_layer and connection, it should also remove some unecessary parameters from rdp_rdp object
            this->sec_layer.rdp_sec_connect(channel_list, width, height, bpp, keylayout, console_session);
    }
    ~rdp_rdp(){
        LOG(LOG_INFO, "End of rdp connection\n");
    }

    private:
        void out_general_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending general caps to server\n");
            stream.out_uint16_le(RDP_CAPSET_GENERAL);
            stream.out_uint16_le(RDP_CAPLEN_GENERAL);
            stream.out_uint16_le(1); /* OS major type */
            stream.out_uint16_le(3); /* OS minor type */
            stream.out_uint16_le(0x200); /* Protocol version */
            stream.out_uint16_le(0); /* Pad */
            stream.out_uint16_le(0); /* Compression types */
            stream.out_uint16_le(this->use_rdp5 ? 0x40d : 0);
            stream.out_uint16_le(0); /* Update capability */
            stream.out_uint16_le(0); /* Remote unshare capability */
            stream.out_uint16_le(0); /* Compression level */
            stream.out_uint16_le(0); /* Pad */
        }

        void out_bitmap_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending bitmap caps to server\n");
            stream.out_uint16_le(RDP_CAPSET_BITMAP);
            stream.out_uint16_le(RDP_CAPLEN_BITMAP);
            stream.out_uint16_le(this->bpp); /* Preferred bpp */
            stream.out_uint16_le(1); /* Receive 1 BPP */
            stream.out_uint16_le(1); /* Receive 4 BPP */
            stream.out_uint16_le(1); /* Receive 8 BPP */
            stream.out_uint16_le(800); /* Desktop width */
            stream.out_uint16_le(600); /* Desktop height */
            stream.out_uint16_le(0); /* Pad */
            stream.out_uint16_le(1); /* Allow resize */
            stream.out_uint16_le(this->bitmap_compression); /* Support compression */
            stream.out_uint16_le(0); /* Unknown */
            stream.out_uint16_le(1); /* Unknown */
            stream.out_uint16_le(0); /* Pad */
        }



        void out_order_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending order caps to server\n");

            char order_caps[32];

            memset(order_caps, 0, 32);
            order_caps[0] = 1; /* dest blt */
            order_caps[1] = 1; /* pat blt */
            order_caps[2] = 1; /* screen blt */
            order_caps[3] = this->bitmap_cache; /* memblt */
            order_caps[4] = 0; /* triblt */
            order_caps[8] = 1; /* line */
            order_caps[9] = 1; /* line */
            order_caps[10] = 1; /* rect */
            order_caps[11] = this->desktop_save; /* desksave */
            order_caps[13] = 1; /* memblt another above */
            order_caps[14] = 1; /* triblt another above */
            order_caps[20] = this->polygon_ellipse_orders; /* polygon */
            order_caps[21] = this->polygon_ellipse_orders; /* polygon2 */
            order_caps[22] = 0; /* todo polyline */
            order_caps[25] = this->polygon_ellipse_orders; /* ellipse */
            order_caps[26] = this->polygon_ellipse_orders; /* ellipse2 */
            order_caps[27] = 1; /* text2 */
            stream.out_uint16_le(RDP_CAPSET_ORDER);
            stream.out_uint16_le(RDP_CAPLEN_ORDER);
            stream.out_clear_bytes(20); /* Terminal desc, pad */
            stream.out_uint16_le(1); /* Cache X granularity */
            stream.out_uint16_le(20); /* Cache Y granularity */
            stream.out_uint16_le(0); /* Pad */
            stream.out_uint16_le(1); /* Max order level */
            stream.out_uint16_le(0x147); /* Number of fonts */
            stream.out_uint16_le(0x2a); /* Capability flags */
            stream.out_copy_bytes(order_caps, 32); /* Orders supported */
            stream.out_uint16_le(0x6a1); /* Text capability flags */
            stream.out_clear_bytes(6); /* Pad */
            stream.out_uint32_le(this->desktop_save * 0x38400); /* Desktop cache size */
            stream.out_uint32_le(0); /* Unknown */
            stream.out_uint32_le(0x4e4); /* Unknown */
        }

        void out_bmpcache_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending bmpcache caps to server\n");
            #warning see details for bmpcache caps
            stream.out_uint16_le(RDP_CAPSET_BMPCACHE);
            stream.out_uint16_le(RDP_CAPLEN_BMPCACHE);
            int Bpp = nbbytes(this->bpp);
            stream.out_clear_bytes(24); /* unused */
            stream.out_uint16_le(0x258); /* entries */
            stream.out_uint16_le(0x100 * Bpp); /* max cell size */
            stream.out_uint16_le(0x12c); /* entries */
            stream.out_uint16_le(0x400 * Bpp); /* max cell size */
            stream.out_uint16_le(0x106); /* entries */
            stream.out_uint16_le(0x1000 * Bpp); /* max cell size */
        }


        /* Output control capability set */
        void out_control_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending control caps to server\n");
            stream.out_uint16_le(RDP_CAPSET_CONTROL);
            stream.out_uint16_le(RDP_CAPLEN_CONTROL);
            stream.out_uint16_le(0); /* Control capabilities */
            stream.out_uint16_le(0); /* Remote detach */
            stream.out_uint16_le(2); /* Control interest */
            stream.out_uint16_le(2); /* Detach interest */
        }


        void out_activate_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending Activate caps to server\n");

            stream.out_uint16_le(RDP_CAPSET_ACTIVATE);
            stream.out_uint16_le(RDP_CAPLEN_ACTIVATE);
            stream.out_uint16_le(0); /* Help key */
            stream.out_uint16_le(0); /* Help index key */
            stream.out_uint16_le(0); /* Extended help key */
            stream.out_uint16_le(0); /* Window activate */
        }

        void out_pointer_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending Pointer caps to server\n");

            stream.out_uint16_le(RDP_CAPSET_POINTER);
            stream.out_uint16_le(RDP_CAPLEN_POINTER_MONO);
            stream.out_uint16_le(0); /* Color pointer */
            stream.out_uint16_le(20); /* Cache size */
        }

        void out_share_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending share caps to server\n");

            stream.out_uint16_le(RDP_CAPSET_SHARE);
            stream.out_uint16_le(RDP_CAPLEN_SHARE);
            stream.out_uint16_le(0); /* userid */
            stream.out_uint16_le(0); /* pad */
        }

        void out_colcache_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending colcache caps to server\n");

            stream.out_uint16_le(RDP_CAPSET_COLCACHE);
            stream.out_uint16_le(RDP_CAPLEN_COLCACHE);
            stream.out_uint16_le(6); /* cache size */
            stream.out_uint16_le(0); /* pad */
        }


        void send_confirm_active(Stream & stream, client_mod * mod) throw(Error)
        {
            LOG(LOG_INFO, "Sending confirm active to server\n");

            char caps_0x0d[] = {
            0x01, 0x00, 0x00, 0x00, 0x09, 0x04, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
            };

            char caps_0x0c[] = { 0x01, 0x00, 0x00, 0x00 };

            char caps_0x0e[] = { 0x01, 0x00, 0x00, 0x00 };

            char caps_0x10[] = {
            0xFE, 0x00, 0x04, 0x00, 0xFE, 0x00, 0x04, 0x00,
            0xFE, 0x00, 0x08, 0x00, 0xFE, 0x00, 0x08, 0x00,
            0xFE, 0x00, 0x10, 0x00, 0xFE, 0x00, 0x20, 0x00,
            0xFE, 0x00, 0x40, 0x00, 0xFE, 0x00, 0x80, 0x00,
            0xFE, 0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 0x08,
            0x00, 0x01, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00
            };

            int sec_flags = SEC_ENCRYPT;
            //sec_flags = RDP5_FLAG | SEC_ENCRYPT;
            int caplen = RDP_CAPLEN_GENERAL + RDP_CAPLEN_BITMAP + RDP_CAPLEN_ORDER +
                    RDP_CAPLEN_BMPCACHE + RDP_CAPLEN_COLCACHE +
                    RDP_CAPLEN_ACTIVATE + RDP_CAPLEN_CONTROL +
                    RDP_CAPLEN_POINTER_MONO + RDP_CAPLEN_SHARE +
                    0x58 + 0x08 + 0x08 + 0x34 /* unknown caps */  +
                    4 /* w2k fix, why? */ ;
            this->sec_layer.rdp_sec_init(stream, sec_flags);
            stream.out_uint16_le(2 + 14 + caplen + sizeof(RDP_SOURCE));
            stream.out_uint16_le((RDP_PDU_CONFIRM_ACTIVE | 0x10)); /* Version 1 */
            stream.out_uint16_le((this->sec_layer.mcs_layer.userid + 1001));
            stream.out_uint32_le(this->share_id);
            stream.out_uint16_le(0x3ea); /* userid */
            stream.out_uint16_le(sizeof(RDP_SOURCE));
            stream.out_uint16_le(caplen);
            stream.out_copy_bytes(RDP_SOURCE, sizeof(RDP_SOURCE));
            stream.out_uint16_le( 0xd); /* num_caps */
            stream.out_clear_bytes( 2); /* pad */
            this->out_general_caps(stream);
            this->out_bitmap_caps(stream);
            this->out_order_caps(stream);

            #warning two identical calls in a row, this is strange, check documentation
            this->out_bmpcache_caps(stream);
            if(this->use_rdp5 == 0){
                this->out_bmpcache_caps(stream);
            }
            else {
                this->out_bmpcache2_caps(stream, mod);
            }
            this->out_colcache_caps(stream);
            this->out_activate_caps(stream);
            this->out_control_caps(stream);
            this->out_pointer_caps(stream);
            this->out_share_caps(stream);
            this->out_unknown_caps(stream, 0x0d, 0x58, caps_0x0d); /* international? */
            this->out_unknown_caps(stream, 0x0c, 0x08, caps_0x0c);
            this->out_unknown_caps(stream, 0x0e, 0x08, caps_0x0e);
            this->out_unknown_caps(stream, 0x10, 0x34, caps_0x10); /* glyph cache? */
            stream.mark_end();
            this->sec_layer.rdp_sec_send(stream, sec_flags);
            LOG(LOG_INFO, "Waiting for answer to confirm active\n");
        }


        void out_unknown_caps(Stream & stream, int id, int length, char* caps)
        {
            LOG(LOG_INFO, "Sending unknown caps to server\n");
            stream.out_uint16_le(id);
            stream.out_uint16_le(length);
            stream.out_copy_bytes(caps, length - 4);
        }


        void process_pointer_pdu(Stream & stream, client_mod * mod) throw(Error)
        {
            LOG(LOG_INFO, "Process pointer PDU\n");

            int message_type = stream.in_uint16_le();
            stream.skip_uint8(2); /* pad */
            switch (message_type) {
            case RDP_POINTER_MOVE:
            {
                int x = stream.in_uint16_le();
                int y = stream.in_uint16_le();
            }
            break;
            case RDP_POINTER_COLOR:
                this->process_color_pointer_pdu(stream, mod);
                break;
            case RDP_POINTER_CACHED:
                this->process_cached_pointer_pdu(stream, mod);
                break;
            case RDP_POINTER_SYSTEM:
                this->process_system_pointer_pdu(stream, mod);
                break;
            default:
                break;
            }
        }

        void process_palette(Stream & stream, client_mod * mod)
        {
            LOG(LOG_INFO, "Process palette\n");

            stream.skip_uint8(2); /* pad */
            uint16_t numberColors = stream.in_uint16_le();
            assert(numberColors == 256);
            stream.skip_uint8(2); /* pad */
            for (int i = 0; i < 256; i++) {
                uint8_t r = stream.in_uint8();
                uint8_t g = stream.in_uint8();
                uint8_t b = stream.in_uint8();
//                uint32_t color = stream.in_bytes_le(3);
                this->orders.cache_colormap.palette[0][i] = (r << 16)
                                                          | (g << 8)
                                                          |  b;
            }
            mod->set_mod_palette(this->orders.cache_colormap.palette[0]);
        }


        void process_disconnect_pdu(Stream & stream)
        {
            LOG(LOG_INFO, "process disconnect pdu\n");
        }

        void process_general_caps(Stream & stream)
        {
            LOG(LOG_INFO, "process general caps\n");
            stream.skip_uint8(10);
            /* Receiving rdp_5 extra flags supported for RDP 5.0 and later versions*/
            int extraflags = stream.in_uint16_le();
            if (extraflags == 0){
                this->use_rdp5 = 0;
            }
            LOG(LOG_INFO, "process general caps %d ok\n", extraflags);
        }

// 2.2.7.1.2    Bitmap Capability Set (TS_BITMAP_CAPABILITYSET)
//  The TS_BITMAP_CAPABILITYSET structure is used to advertise bitmap-orientated characteristics and
//  is based on the capability set specified in [T128] section 8.2.4. This capability is sent by both client
//  and server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the capability set. This
//   field MUST be set to CAPSTYPE_BITMAP (2).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes of the capability
//   data, including the size of the capabilitySetType and lengthCapability fields.

// preferredBitsPerPixel (2 bytes): A 16-bit, unsigned integer. Color depth of the remote
//   session. In RDP 4.0 and 5.0, this field MUST be set to 8 (even for a 16-color session).

// receive1BitPerPixel (2 bytes): A 16-bit, unsigned integer. Indicates whether the client can
//   receive 1 bpp. This field is ignored and SHOULD be set to TRUE (0x0001).

// receive4BitsPerPixel (2 bytes): A 16-bit, unsigned integer. Indicates whether the client can
//   receive 4 bpp. This field is ignored and SHOULD be set to TRUE (0x0001).

// receive8BitsPerPixel (2 bytes): A 16-bit, unsigned integer. Indicates whether the client can
//   receive 8 bpp. This field is ignored and SHOULD be set to TRUE (0x0001).

// desktopWidth (2 bytes): A 16-bit, unsigned integer. The width of the desktop in the remote
//   session.

// desktopHeight (2 bytes): A 16-bit, unsigned integer. The height of the desktop in the remote
//   session.

// pad2octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field are ignored.

// desktopResizeFlag (2 bytes): A 16-bit, unsigned integer. Indicates whether desktop resizing
//   is supported.
//   0x0000 FALSE  Desktop resizing is not supported.
//   0x0001 TRUE   Desktop resizing is supported.
//   If a desktop resize occurs, the server will deactivate the session (see section 1.3.1.3), and on
//   session reactivation will specify the new desktop size in the desktopWidth and
//   desktopHeight fields in the Bitmap Capability Set, along with a value of TRUE for the
//   desktopResizeFlag field. The client should check these sizes and, if different from the
//   previous desktop size, resize any windows to support this size.

// bitmapCompressionFlag (2 bytes): A 16-bit, unsigned integer. Indicates whether the client
//   supports bitmap compression. RDP requires bitmap compression and hence this field MUST be
//   set to TRUE (0x0001). If it is not set to TRUE, the server MUST NOT continue with the
//   connection.

// highColorFlags (1 byte): An 8-bit, unsigned integer. Client support for 16 bpp color modes.
//   This field is ignored and SHOULD be set to 0.

// drawingFlags (1 byte): An 8-bit, unsigned integer. Flags describing support for 32 bpp
//   bitmaps.
// 0x02 DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY Indicates support for lossy compression of 32 bpp bitmaps by reducing color-fidelity on a per-pixel basis.
// 0x04 DRAW_ALLOW_COLOR_SUBSAMPLING      Indicates support for chroma subsampling when compressing 32 bpp bitmaps.
// 0x08 DRAW_ALLOW_SKIP_ALPHA             Indicates that the client supports the removal of the alpha-channel when compressing 32 bpp bitmaps. In this case the alpha is assumed to be 0xFF, meaning the bitmap is opaque.
// Compression of 32 bpp bitmaps is specified in [MS-RDPEGDI] section 3.1.9.

// multipleRectangleSupport (2 bytes): A 16-bit, unsigned integer. Indicates whether the client
//  supports the use of multiple bitmap rectangles. RDP requires the use of multiple bitmap
//  rectangles and hence this field MUST be set to TRUE (0x0001). If it is not set to TRUE, the
//  server MUST NOT continue with the connection.

// pad2octetsB (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field are ignored.

        /* Process a bitmap capability set */
        void process_bitmap_caps(Stream & stream)
        {
            LOG(LOG_INFO, "process bitmap caps\n");
            this->bpp = stream.in_uint16_le();
            stream.skip_uint8(6);
            int width = stream.in_uint16_le();
            int height = stream.in_uint16_le();
            /* todo, call reset if needed and use width and height */
            LOG(LOG_INFO, "process bitmap caps (%dx%dx%d) [bpp=%d] ok\n", width, height, bpp, this->bpp);
        }


        void process_server_caps(Stream & stream, int len)
        {
            LOG(LOG_INFO, "process server caps\n");
            int n;
            int ncapsets;
            int capset_type;
            int capset_length;
            uint8_t* next;
            uint8_t* start;

            start = stream.p;
            ncapsets = stream.in_uint16_le();
            stream.skip_uint8(2); /* pad */
            for (n = 0; n < ncapsets; n++) {
                if (stream.p > start + len) {
                    return;
                }
                capset_type = stream.in_uint16_le();
                capset_length = stream.in_uint16_le();
                next = (stream.p + capset_length) - 4;
                switch (capset_type) {
                case RDP_CAPSET_GENERAL:
                    this->process_general_caps(stream);
                    break;
                case RDP_CAPSET_BITMAP:
                    this->process_bitmap_caps(stream);
                    break;
                default:
                    break;
                }
                stream.p = next;
            }
            LOG(LOG_INFO, "process server ok\n");
        }


        void send_control(Stream & stream, int action) throw (Error)
        {
            this->init_data(stream);
            stream.out_uint16_le(action);
            stream.out_uint16_le(0); /* userid */
            stream.out_uint32_le(0); /* control id */
            stream.mark_end();
            this->send_data(stream, RDP_DATA_PDU_CONTROL, MCS_GLOBAL_CHANNEL);
        }


        void send_synchronise(Stream & stream) throw (Error)
        {
            this->init_data(stream);
            stream.out_uint16_le(1); /* type */
            stream.out_uint16_le(1002);
            stream.mark_end();
            this->send_data(stream, RDP_DATA_PDU_SYNCHRONISE, MCS_GLOBAL_CHANNEL);
        }

        void send_fonts(Stream & stream, int seq) throw(Error)
        {
            this->init_data(stream);
            stream.out_uint16_le(0); /* number of fonts */
            stream.out_uint16_le(0); /* pad? */
            stream.out_uint16_le(seq); /* unknown */
            stream.out_uint16_le(0x32); /* entry size */
            stream.mark_end();
            this->send_data(stream, RDP_DATA_PDU_FONT2, MCS_GLOBAL_CHANNEL);
        }

    #define RDP5_FLAG 0x0030

    public:

        void init(Stream & stream) throw(Error)
        {
            this->sec_layer.rdp_sec_init(stream, SEC_ENCRYPT);
            stream.rdp_hdr = stream.p;
            stream.p += 6;
        }

        void rdp_channel_init(Stream & stream)
        {
            this->sec_layer.rdp_sec_init(stream, SEC_ENCRYPT);
            stream.channel_hdr = stream.p;
            stream.p += 8;
        }

        /******************************************************************************/

        /* Send persistent bitmap cache enumeration PDU's
        Not implemented yet because it should be implemented
        before in server_rdp_process_data case. The problem is that
        we don't save the bitmap key list attached with rdp_bmpcache2 capability
        message so we can't develop this function yet */

        void enum_bmpcache2()
        {

        }

        void send_login_info(int flags, int rdp5_performanceflags) throw(Error)
        {
            LOG(LOG_INFO, "send login info to server\n");
            time_t t = time(NULL);
            time_t tzone;

            rdp5_performanceflags = RDP5_NO_WALLPAPER;

            Stream stream(8192);
            // The WAB does not send it's IP to server. Is it what we want ?
            const char * ip_source = "\0\0\0\0";

            int sec_flags = SEC_LOGON_INFO | SEC_ENCRYPT;
            this->sec_layer.rdp_sec_init(stream, sec_flags);
            if(!this->use_rdp5){
                LOG(LOG_INFO, "send login info (RDP4-style) %s:%s\n",this->domain, this->sec_layer.username);

                stream.out_uint32_le(0);
                stream.out_uint32_le(flags);
                stream.out_uint16_le(2 * strlen(this->domain));
                stream.out_uint16_le(2 * strlen(this->sec_layer.username));
                stream.out_uint16_le(2 * strlen(this->password));
                stream.out_uint16_le(2 * strlen(this->program));
                stream.out_uint16_le(2 * strlen(this->directory));
                stream.out_unistr(this->domain);
                stream.out_unistr(this->sec_layer.username);
                stream.out_unistr(this->password);
                stream.out_unistr(this->program);
                stream.out_unistr(this->directory);
            }
            else {
                LOG(LOG_INFO, "send login info (RDP5-style) %x %s:%s\n",flags,
                    this->domain,
                    this->sec_layer.username);

                flags |= RDP_LOGON_BLOB;
                stream.out_uint32_le(0);
                stream.out_uint32_le(flags);
                stream.out_uint16_le(2 * strlen(this->domain));
                stream.out_uint16_le(2 * strlen(this->sec_layer.username));
                if (flags & RDP_LOGON_AUTO){
                    stream.out_uint16_le(2 * strlen(this->password));
                }
                if (flags & RDP_LOGON_BLOB && ! (flags & RDP_LOGON_AUTO)){
                    stream.out_uint16_le(0);
                }
                stream.out_uint16_le(2 * strlen(this->program));
                stream.out_uint16_le(2 * strlen(this->directory));
                if ( 0 < (2 * strlen(this->domain))){
                    stream.out_unistr(this->domain);
                }
                else {
                    stream.out_uint16_le(0);
                }
                stream.out_unistr(this->sec_layer.username);
                if (flags & RDP_LOGON_AUTO){
                    stream.out_unistr(this->password);
                }
                else{
                    stream.out_uint16_le(0);
                }
                if (0 < 2 * strlen(this->program)){
                    stream.out_unistr(this->program);
                }
                else {
                    stream.out_uint16_le(0);
                }
                if (2 * strlen(this->directory) < 0){
                    stream.out_unistr(this->directory);
                }
                else{
                    stream.out_uint16_le(0);
                }
                stream.out_uint16_le(2);
                stream.out_uint16_le(2 * strlen(ip_source) + 2);
                stream.out_unistr(ip_source);
                stream.out_uint16_le(2 * strlen("C:\\WINNT\\System32\\mstscax.dll") + 2);
                stream.out_unistr("C:\\WINNT\\System32\\mstscax.dll");

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
                stream.out_uint32_le(0xfffffffe);
                stream.out_uint32_le(rdp5_performanceflags);
                stream.out_uint16_le(0);
                this->use_rdp5 = 0;
            }
            stream.mark_end();
            this->sec_layer.rdp_sec_send(stream, sec_flags);
            LOG(LOG_INFO, "send login info ok\n");
        }


        void send(Stream & stream, int pdu_type) throw(Error)
        {
            stream.p = stream.rdp_hdr;
            int len = stream.end - stream.p;
            stream.out_uint16_le(len);

            /* Added in order to adapt to version 5 packet */
            if (this->use_rdp5)
            {
                stream.out_uint16_le(RDP_PDU_DATA | 0x10);
                stream.out_uint16_le(this->sec_layer.mcs_layer.userid);
                stream.out_uint32_le(this->share_id);
                stream.out_uint8(0);  /* pad */
                stream.out_uint8(1);  /* stream id*/
                stream.out_uint16_le(len - 14);
                stream.out_uint8(pdu_type);
                stream.out_uint8(0);  /* compress type */
                stream.out_uint16_le(0);  /* compress length */
            }
            else {
                stream. out_uint16_le(pdu_type | 0x10);
                stream.out_uint16_le(this->sec_layer.mcs_layer.userid);
            }
            int sec_flags = SEC_ENCRYPT;
            this->sec_layer.rdp_sec_send(stream, sec_flags);
        }

        /* Initialise an RDP data packet */
        int init_data(Stream & stream)
        {
            this->sec_layer.rdp_sec_init(stream, SEC_ENCRYPT);
            stream.rdp_hdr = stream.p;
            stream.p += 18;
            return 0;
        }

        /* Send an RDP data packet */
        void send_data(Stream & stream, int pdu_data_type, int chan_id) throw(Error)
        {
            stream.p = stream.rdp_hdr;
            int len = stream.end - stream.p;
            stream.out_uint16_le(len);
            stream.out_uint16_le(RDP_PDU_DATA | 0x10);
            stream.out_uint16_le(this->sec_layer.mcs_layer.userid);
            stream.out_uint32_le(this->share_id);
            stream.out_uint8(0); /* pad */
            stream.out_uint8(1); /* stream id */
            stream.out_uint16_le( len - 14);
            stream.out_uint8(pdu_data_type);
            stream.out_uint8(0); /* compress type */
            stream.out_uint16_le(0); /* compress len */
            int sec_flags = SEC_ENCRYPT;
            this->sec_layer.rdp_sec_send_to_channel(stream, sec_flags, chan_id);
        }


        void send_input(Stream & stream, int time, int message_type,
                        int device_flags, int param1, int param2) throw(Error)
        {
            if (this->init_data(stream) != 0) {
                throw Error(ERR_RDP_SEND_INPUT_INIT_DATA_NOK);
            }
            stream.out_uint16_le(1); /* number of events */
            stream.out_uint16_le(0);
            stream.out_uint32_le(time);
            stream.out_uint16_le(message_type);
            stream.out_uint16_le(device_flags);
            stream.out_uint16_le(param1);
            stream.out_uint16_le(param2);
            stream.mark_end();
            this->send_data(stream, RDP_DATA_PDU_INPUT, MCS_GLOBAL_CHANNEL);
        }

        void send_invalidate(Stream & stream,int left, int top, int width, int height) throw(Error)
        {
            if (this->init_data(stream) != 0) {
                throw Error(ERR_RDP_SEND_INVALIDATE_INIT_DATA_NOK);
            }
            stream.out_uint32_le(1);
            stream.out_uint16_le(left);
            stream.out_uint16_le(top);
            stream.out_uint16_le((left + width) - 1);
            stream.out_uint16_le((top + height) - 1);
            stream.mark_end();
            this->send_data(stream, 33, MCS_GLOBAL_CHANNEL);
        }


        void recv(Stream & stream, int* type, client_mod * mod) throw(Error)
        {
            int len;
            int pdu_type;
            int chan;
            int version;
            //int pdu_data_type;

            chan = 0;
            if (stream.next_packet >= stream.end || stream.next_packet == 0) {
                this->sec_layer.rdp_sec_recv(stream, chan, version, mod);
                if (version == 0xff){
                    stream.next_packet = stream.end;
                    *type = 0;
                    return;
                }
                else if (version != 3){
                    /* We must verify this condition because I'm not pretty sure that
                    it's good. I think we need to recover protocol version and not
                    this one. By the moment, I put the same condition that appears
                    in rdesktop */
                    //rdp5_process_data(self, stream, pdu_data_type);
                    // packet setup Added by kriss
                    stream.next_packet = stream.p;
                    *type = 0;
                    return;
                }
                stream.next_packet = stream.p;
            }
            else {
                stream.p = stream.next_packet;
            }
            len = stream.in_uint16_le();
            if (len == 0x8000) {
                stream.next_packet += 8;
                *type = 0;
                return;
            }
            pdu_type = stream.in_uint16_le();
            stream.skip_uint8(2);
            *type = pdu_type & 0xf;
            stream.next_packet += len;
            this->chan_id = chan;
        }


        void process_data_pdu(Stream & stream, client_mod * mod)
        {
            int data_pdu_type;
            int ctype;
            int clen;
            int len;

            stream.skip_uint8(6); /* shareid, pad, Streamid */
            len = stream.in_uint16_le();
            data_pdu_type = stream.in_uint8();
            ctype = stream.in_uint8();
            clen = stream.in_uint16_le();
            clen -= 18;
            switch (data_pdu_type) {
            case RDP_DATA_PDU_UPDATE:
                this->process_update_pdu(stream, mod);
                break;
            case RDP_DATA_PDU_CONTROL:
                break;
            case RDP_DATA_PDU_SYNCHRONISE:
                break;
            case RDP_DATA_PDU_POINTER:
                this->process_pointer_pdu(stream, mod);
                break;
            case RDP_DATA_PDU_BELL:
                break;
            case RDP_DATA_PDU_LOGON:
//                LOG(LOG_INFO, "DATA PDU LOGON\n");
                break;
            case RDP_DATA_PDU_DISCONNECT:
//                LOG(LOG_INFO, "DATA PDU DISCONNECT\n");
                this->process_disconnect_pdu(stream);
                break;
            default:
                break;
            }
        }


        void process_demand_active(Stream & stream, client_mod * mod) throw(Error)
        {
//            LOG(LOG_INFO, "process demand active\n");

            int type;
            int len_src_descriptor;
            int len_combined_caps;

            this->share_id = stream.in_uint32_le();
            len_src_descriptor = stream.in_uint16_le();
            len_combined_caps = stream.in_uint16_le();
            stream.skip_uint8(len_src_descriptor);
            this->process_server_caps(stream, len_combined_caps);
            this->send_confirm_active(stream, mod);
            this->send_synchronise(stream);
            this->send_control(stream, RDP_CTL_COOPERATE);
            this->send_control(stream, RDP_CTL_REQUEST_CONTROL);
            this->recv(stream, &type, mod); /* RDP_PDU_SYNCHRONIZE */
            this->recv(stream, &type, mod); /* RDP_CTL_COOPERATE */
            this->recv(stream, &type, mod); /* RDP_CTL_GRANT_CONTROL */
            this->send_input(stream, 0, RDP_INPUT_SYNCHRONIZE, 0, 0, 0);
            /* Including RDP 5.0 capabilities */
            if (this->use_rdp5 != 0){
                this->enum_bmpcache2();
                this->send_fonts(stream, 3);
            }
            else{
                this->send_fonts(stream, 1);
                this->send_fonts(stream, 2);
            }
            this->recv(stream, &type, mod); /* RDP_PDU_UNKNOWN 0x28 (Fonts?) */
            this->orders.rdp_orders_reset_state();
            LOG(LOG_INFO, "process demand active ok, reset state [bpp=%d]\n", this->bpp);
        }

        void send_redirect_pdu(long param1, long param2, long param3, int param4,
                                      vector<mcs_channel_item*> channel_list) throw(Error)
        {
            char* data;
            char* name;
            int chan_id;
            int total_data_length;
            int size;
            int flags;
            int index;
            int channel_id;
            int num_channels_src;
            int num_channels_dst;
            int sec_flags;
            struct mcs_channel_item* channel_item;
            /* We need to verify this in order to right process the stream passed */
            chan_id = (int)(param1 & 0xffff);
            chan_id = chan_id + MCS_GLOBAL_CHANNEL + 1;
            flags = (int)((param1 >> 16) & 0xffff);
            size = param2;
            data = (char*)param3;
            total_data_length = param4;
            /* We need to recover the name of the channel linked with this
            channel_id in order to match it with the same channel on the
            first channel_list created by the RDP client at initialization
            process*/

            num_channels_src = (int) channel_list.size();
            for (index = 0; index < num_channels_src; index++){
                channel_item = channel_list[index];
                if (chan_id == channel_item->chanid){
                    name = channel_item->name;
                }
            }
            /* Here, we're going to search the correct channel in order to send
            information throughout this channel to RDP server */
            num_channels_dst = (int) this->sec_layer.mcs_layer.channel_list.size();
            for (index = 0; index < num_channels_dst; index++){
                channel_item = this->sec_layer.mcs_layer.channel_list[index];
                if (strcmp(name, channel_item->name) == 0){
                    channel_id = channel_item->chanid;
                }
            }
            /*Copy data from s to data and after that close stream and send
            it to send_data with channel_id so we need to pass chan_id to
            send_data also in order to be able to redirect data in the correct
            way*/
            Stream stream(8192);
            this->rdp_channel_init(stream);
            stream.p = stream.channel_hdr;

            stream.out_uint32_le(total_data_length);
            stream.out_uint32_le(flags);
            memcpy(stream.p, data, size);
            stream.p+= size;
            stream.mark_end();

            /* in send_redirect_pdu, sending data from stream.p throughout channel channel_item->name */
            //g_hexdump(stream.p, size + 8);
            sec_flags = SEC_ENCRYPT;
            /* We need to call send_data but with another code because we need to build an
            virtual_channel packet and not an MCS_GLOBAL_CHANNEL packet */
            this->sec_layer.rdp_sec_send_to_channel(stream, sec_flags, channel_id);
        }

    void process_color_pointer_pdu(Stream & stream, client_mod * mod) throw(Error)
    {
        unsigned cache_idx;
        unsigned dlen;
        unsigned mlen;
        struct rdp_cursor* cursor;

        cache_idx = stream.in_uint16_le();
        if (cache_idx >= (sizeof(this->cursors) / sizeof(cursor))) {
            throw Error(ERR_RDP_PROCESS_COLOR_POINTER_CACHE_NOT_OK);
        }
        cursor = this->cursors + cache_idx;
        cursor->x = stream.in_uint16_le();
        cursor->y = stream.in_uint16_le();
        cursor->width = stream.in_uint16_le();
        cursor->height = stream.in_uint16_le();
        mlen = stream.in_uint16_le(); /* mask length */
        dlen = stream.in_uint16_le(); /* data length */
        if ((mlen > sizeof(cursor->mask)) || (dlen > sizeof(cursor->data))) {
            throw Error(ERR_RDP_PROCESS_COLOR_POINTER_LEN_NOT_OK);
        }
        memcpy( cursor->data, stream.in_uint8p( dlen),  dlen);
        memcpy( cursor->mask, stream.in_uint8p( mlen),  mlen);
        mod->server_set_pointer(cursor->x, cursor->y, cursor->data, cursor->mask);
    }

    void process_cached_pointer_pdu(Stream & stream, client_mod * mod)
    {
        int cache_idx;

        cache_idx = stream.in_uint16_le();
        if (cache_idx < 0){
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_LESS_0);
        }
        if (cache_idx >= (int)(sizeof(this->cursors) / sizeof(rdp_cursor))) {
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }
        struct rdp_cursor* cursor = this->cursors + cache_idx;
        mod->server_set_pointer(cursor->x, cursor->y, cursor->data, cursor->mask);
    }

    void process_system_pointer_pdu(Stream & stream, client_mod * mod)
    {
        int system_pointer_type;

        system_pointer_type = stream.in_uint16_le();
        switch (system_pointer_type) {
        case RDP_NULL_POINTER:
            {
                struct rdp_cursor cursor;
                memset(cursor.mask, 0xff, sizeof(cursor.mask));
                #warning we should pass in a cursor to set_pointer instead of individual fields
                mod->server_set_pointer(cursor.x, cursor.y, cursor.data, cursor.mask);
                mod->set_pointer_display();
            }
            break;
        default:
            break;
        }
    }

    void process_bitmap_updates(Stream & stream, client_mod * mod)
    {
        // RDP-BCGR: 2.2.9.1.1.3.1.2 Bitmap Update (TS_UPDATE_BITMAP)
        // ----------------------------------------------------------
        // The TS_UPDATE_BITMAP structure contains one or more rectangular
        // clippings taken from the server-side screen frame buffer (see [T128]
        // section 8.17).

        // shareDataHeader (18 bytes): Share Data Header (section 2.2.8.1.1.1.2)
        // containing information about the packet. The type subfield of the
        // pduType field of the Share Control Header (section 2.2.8.1.1.1.1)
        // MUST be set to PDUTYPE_DATAPDU (7). The pduType2 field of the Share
        // Data Header MUST be set to PDUTYPE2_UPDATE (2).

        // bitmapData (variable): The actual bitmap update data, as specified in
        // section 2.2.9.1.1.3.1.2.1.

        // 2.2.9.1.1.3.1.2.1 Bitmap Update Data (TS_UPDATE_BITMAP_DATA)
        // ------------------------------------------------------------
        // The TS_UPDATE_BITMAP_DATA structure encapsulates the bitmap data that
        // defines a Bitmap Update (section 2.2.9.1.1.3.1.2).

        // updateType (2 bytes): A 16-bit, unsigned integer. The graphics update
        // type. This field MUST be set to UPDATETYPE_BITMAP (0x0001).

        // numberRectangles (2 bytes): A 16-bit, unsigned integer.
        // The number of screen rectangles present in the rectangles field.
        size_t numberRectangles = stream.in_uint16_le();
        for (size_t i = 0; i < numberRectangles; i++) {
            // rectangles (variable): Variable-length array of TS_BITMAP_DATA
            // (section 2.2.9.1.1.3.1.2.2) structures, each of which contains a
            // rectangular clipping taken from the server-side screen frame buffer.
            // The number of screen clippings in the array is specified by the
            // numberRectangles field.

            // 2.2.9.1.1.3.1.2.2 Bitmap Data (TS_BITMAP_DATA)
            // ----------------------------------------------

            // The TS_BITMAP_DATA structure wraps the bitmap data bytestream
            // for a screen area rectangle containing a clipping taken from
            // the server-side screen frame buffer.

            // A 16-bit, unsigned integer. Left bound of the rectangle.
            const uint16_t destLeft = stream.in_uint16_le();

            // A 16-bit, unsigned integer. Top bound of the rectangle.
            const uint16_t destTop = stream.in_uint16_le();

            // A 16-bit, unsigned integer. Right bound of the rectangle.
            const uint16_t destRight = stream.in_uint16_le();

            // A 16-bit, unsigned integer. Bottom bound of the rectangle.
            const uint16_t destBottom = stream.in_uint16_le();

            // A 16-bit, unsigned integer. The width of the rectangle.
            const uint16_t width = stream.in_uint16_le();

            // A 16-bit, unsigned integer. The height of the rectangle.
            const uint16_t height = stream.in_uint16_le();

            // A 16-bit, unsigned integer. The color depth of the rectangle
            // data in bits-per-pixel.
            const uint8_t bpp = stream.in_uint16_le();

            // A 16-bit, unsigned integer. The flags describing the format
            // of the bitmap data in the bitmapDataStream field.

            // +-----------------------------------+---------------------------+
            // | 0x0001 BITMAP_COMPRESSION         | Indicates that the bitmap |
            // |                                   | data is compressed. This  |
            // |                                   | implies that the          |
            // |                                   | bitmapComprHdr field is   |
            // |                                   | present if the NO_BITMAP_C|
            // |                                   |OMPRESSION_HDR (0x0400)    |
            // |                                   | flag is not set.          |
            // +-----------------------------------+---------------------------+
            // | 0x0400 NO_BITMAP_COMPRESSION_HDR  | Indicates that the        |
            // |                                   | bitmapComprHdr field is   |
            // |                                   | not present(removed for   |
            // |                                   | bandwidth efficiency to   |
            // |                                   | save 8 bytes).            |
            // +-----------------------------------+---------------------------+

            int flags = stream.in_uint16_le();
            uint16_t bitmapLength = stream.in_uint16_le();

            Rect boundary(destLeft, destTop,
                          destRight - destLeft + 1,
                          destBottom - destTop + 1);

            // BITMAP_COMPRESSION 0x0001
            // Indicates that the bitmap data is compressed. This implies
            // that the bitmapComprHdr field is present if the
            // NO_BITMAP_COMPRESSION_HDR (0x0400) flag is not set.

            Bitmap bitmap(bpp, width, height);

            if (flags & 0x0001){
                uint16_t size = 0;
                if (flags & 0x400) {
                    size = bitmapLength;
                }
                else {
                // bitmapComprHdr (8 bytes): Optional Compressed Data Header
                // structure (see Compressed Data Header (TS_CD_HEADER)
                // (section 2.2.9.1.1.3.1.2.3)) specifying the bitmap data
                // in the bitmapDataStream. This field MUST be present if
                // the BITMAP_COMPRESSION (0x0001) flag is present in the
                // Flags field, but the NO_BITMAP_COMPRESSION_HDR (0x0400)
                // flag is not.
                    // bitmapComprHdr
                    stream.skip_uint8(2); /* pad */
                    size = stream.in_uint16_le();
                    uint16_t line_size = stream.in_uint16_le();
                    uint16_t final_size = stream.in_uint16_le();

                    assert(line_size == bitmap.line_size);
                    assert(final_size == bitmap.bmp_size);
                }

                const uint8_t * data = stream.in_uint8p(size);

                bitmap.decompress(data, size);
            }
            else {
                const uint8_t * data = stream.in_uint8p(bitmapLength);
                assert(bitmapLength == bitmap.bmp_size);
                bitmap.copy(data);
            }
            mod->server_paint_rect(bitmap, boundary, 0, 0, this->orders.cache_colormap.palette[0]);
        }
    }

    void process_update_pdu(Stream & stream, client_mod * mod)
    {
        int update_type;
        int count;

    // MS-RDPBCGR: 1.3.6
    // -----------------
    // The most fundamental output that a server can send to a connected client
    // is bitmap images of the remote session using the Update Bitmap PDU. This
    // allows the client to render the working space and enables a user to
    // interact with the session running on the server. The global palette
    // information for a session is sent to the client in the Update Palette PDU.


        update_type = stream.in_uint16_le();
        mod->server_begin_update();
        switch (update_type) {
        case RDP_UPDATE_ORDERS:
            stream.skip_uint8(2); /* pad */
            count = stream.in_uint16_le();
            stream.skip_uint8(2); /* pad */
            this->orders.rdp_orders_process_orders(stream, count, mod);
            break;
        case RDP_UPDATE_BITMAP:
            this->process_bitmap_updates(stream, mod);
            break;
        case RDP_UPDATE_PALETTE:
            this->process_palette(stream, mod);
            break;
        case RDP_UPDATE_SYNCHRONIZE:
            break;
        default:
            break;
        }
        mod->server_end_update();
    }

    void out_bmpcache2_caps(Stream & stream, client_mod * mod)
    {
        int i;

        stream.out_uint16_le(RDP_CAPSET_BMPCACHE2);
        stream.out_uint16_le(RDP_CAPLEN_BMPCACHE2);

        /* version */
        stream.out_uint16_le(
            mod->front->orders->rdp_layer->client_info.bitmap_cache_persist_enable ? 2 : 0);

        stream.out_uint16_be(3);	/* number of caches in this set */


        /* Sending bitmap capabilities version 2 */
        #warning no need any more to set a limit at 2000, use real figures
        i = mod->front->orders->rdp_layer->client_info.cache1_entries;
        i = std::min(i, 2000);
        stream.out_uint32_le(i);
        i = mod->front->orders->rdp_layer->client_info.cache2_entries;
        i = std::min(i, 2000);
        stream.out_uint32_le(i);
        i = mod->front->orders->rdp_layer->client_info.cache3_entries;
        i = std::min(i, 2000);
        stream.out_uint32_le(i);

        stream.out_clear_bytes(20);	/* other bitmap caches not used */
    }
};




#endif
