/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   xup module main header file
*/

#include "mod/xup/xup.hpp"

#include "core/RDP/rdp_pointer.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/front_api.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/screen_functions.hpp"
#include "transport/transport.hpp"
#include "utils/stream.hpp"
#include "utils/bitmap.hpp"
#include "utils/sugar/buf_maker.hpp"


namespace
{
    enum {
        XUPWM_PAINT        = 3,
        XUPWM_KEYDOWN      = 15,
        XUPWM_KEYUP        = 16,
        XUPWM_SYNCHRONIZE  = 17,
        XUPWM_MOUSEMOVE    = 100,
        XUPWM_LBUTTONUP    = 101,
        XUPWM_LBUTTONDOWN  = 102,
        XUPWM_RBUTTONUP    = 103,
        XUPWM_RBUTTONDOWN  = 104,
        XUPWM_BUTTON3UP    = 105,
        XUPWM_BUTTON3DOWN  = 106,
        XUPWM_BUTTON4UP    = 107,
        XUPWM_BUTTON4DOWN  = 108,
        XUPWM_BUTTON5UP    = 109,
        XUPWM_BUTTON5DOWN  = 110,
        XUPWM_BUTTON_OK    = 300,
        XUPWM_SCREENUPDATE = 0x4444,
        XUPWM_CHANNELDATA  = 0x5555
    };

    enum {
        XUPWM_INVALIDATE = 200
    };

    void x_input_event(
        Transport& t, const int msg,
        const long param1, const long param2, const long param3, const long param4)
    {
        StaticOutStream<256> stream;
        stream.out_skip_bytes(4); // skip yet unknown len
        stream.out_uint16_le(103);
        stream.out_uint32_le(msg);
        stream.out_uint32_le(param1);
        stream.out_uint32_le(param2);
        stream.out_uint32_le(param3);
        stream.out_uint32_le(param4);
        uint32_t len = stream.get_offset();
        stream.stream_at(0).out_uint32_le(len);
        t.send(stream.get_data(), len);
    }
} // anonymous namespace


xup_mod::xup_mod(
    Transport & t, SessionReactor& session_reactor, FrontAPI & front,
    uint16_t front_width, uint16_t front_height, BitsPerPixel context_bpp)
: front(front)
, bpp(context_bpp)
, t(t)
, rop(0xCC)
, session_reactor(session_reactor)
{
    StaticOutStream<256> stream;
    stream.out_skip_bytes(4);
    stream.out_uint16_le(103);
    stream.out_uint32_le(200);
    /* x and y */
    stream.out_uint32_le(0);
    /* width and height */
    stream.out_uint32_le((uint32_t(front_width) << 16) | front_height);
    stream.out_uint32_le(0);
    stream.out_uint32_le(0);
    stream.stream_at(0).out_uint32_le(stream.get_offset());
    this->t.send(stream.get_bytes());

    this->fd_event = session_reactor
    .create_graphic_fd_event(this->t.get_fd())
    .set_timeout(std::chrono::milliseconds(0))
    .on_exit(jln::propagate_exit())
    .on_action([this](JLN_TOP_CTX ctx, gdi::GraphicApi& gd){
        this->draw_event(gd);
        return ctx.need_more_data();
    })
    .on_timeout([](JLN_TOP_TIMER_CTX ctx, gdi::GraphicApi& gd){
        gdi_clear_screen(gd, Dimension{0xffffu, 0xffffu});
        // rearmed by clipboard
        return ctx.disable_timeout().ready();
    });
}

void xup_mod::rdp_input_mouse(int device_flags, int x, int y, Keymap2 * /*keymap*/)
{
    LOG(LOG_INFO, "input mouse");

    if (device_flags & MOUSE_FLAG_MOVE) { /* 0x0800 */
        x_input_event(this->t, XUPWM_MOUSEMOVE, x, y, 0, 0);
    }
    if (device_flags & MOUSE_FLAG_BUTTON1) { /* 0x1000 */
        x_input_event(this->t,
            XUPWM_LBUTTONUP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
            x, y, 0, 0);
    }
    if (device_flags & MOUSE_FLAG_BUTTON2) { /* 0x2000 */
        x_input_event(this->t,
            XUPWM_RBUTTONUP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
            x, y, 0, 0);
    }
    if (device_flags & MOUSE_FLAG_BUTTON3) { /* 0x4000 */
        x_input_event(this->t,
            XUPWM_BUTTON3UP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
            x, y, 0, 0);
    }
    if (device_flags == MOUSE_FLAG_BUTTON4 || /* 0x0280 */ device_flags == 0x0278) {
        x_input_event(this->t, XUPWM_BUTTON4DOWN, x, y, 0, 0);
        x_input_event(this->t, XUPWM_BUTTON4UP, x, y, 0, 0);
    }
    if (device_flags == MOUSE_FLAG_BUTTON5 || /* 0x0380 */ device_flags == 0x0388) {
        x_input_event(this->t, XUPWM_BUTTON5DOWN, x, y, 0, 0);
        x_input_event(this->t, XUPWM_BUTTON5UP, x, y, 0, 0);
    }
}

void xup_mod::rdp_input_scancode(long param1, long param2, long device_flags, long param4, Keymap2 * keymap)
{
    // TODO xup_mod::rdp_input_scancode: unimplemented
    (void)param1;
    (void)param2;
    (void)device_flags;
    (void)param4;
    (void)keymap;
    LOG(LOG_INFO, "scan code");
    /*
    if (ki != 0) {
        int msg = (device_flags & KBD_FLAG_UP)?XUPWM_KEYUP:XUPWM_KEYDOWN;
        x_input_event(this->t, msg, ki->chr, ki->sym, param1, device_flags);
    }
    */
}

void xup_mod::rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
{
    // TODO xup_mod::rdp_input_scancode: unimplemented
    (void)time;
    (void)device_flags;
    (void)param1;
    (void)param2;
    LOG(LOG_INFO, "overloaded by subclasses");
}

void xup_mod::rdp_input_invalidate(Rect r)
{
    LOG(LOG_INFO, "rdp_input_invalidate");
    if (!r.isempty()) {
        x_input_event(this->t, XUPWM_INVALIDATE,
            ((r.x & 0xffff) << 16) | (r.y & 0xffff),
            ((r.cx & 0xffff) << 16) | (r.cy & 0xffff),
            0, 0);
    }
}

void xup_mod::refresh(Rect r)
{
    this->rdp_input_invalidate(r);
}

void xup_mod::draw_event(gdi::GraphicApi & gd)
{
    try{
        BufMaker<32768> buf_maker;

        InStream stream(this->t.recv_boom(buf_maker.static_array().data(), 8));
        unsigned type = stream.in_uint16_le();
        unsigned num_orders = stream.in_uint16_le();
        unsigned len = stream.in_uint32_le();
        if (type == 1) {
            stream = InStream(this->t.recv_boom(buf_maker.dyn_array(len)));

            for (unsigned index = 0; index < num_orders; index++) {
                type = stream.in_uint16_le();
                switch (type) {
                case 1:
                    gd.begin_update();
                    break;
                case 2:
                    gd.end_update();
                    break;
                case 3:
                {
                    const Rect r(
                        stream.in_sint16_le(),
                        stream.in_sint16_le(),
                        stream.in_uint16_le(),
                        stream.in_uint16_le());
                        gd.draw(RDPPatBlt(r, this->rop, color_encode(BLACK, this->bpp), color_encode(WHITE, this->bpp),
                        RDPBrush(r.x, r.y, 3, 0xaa,
                        byte_ptr_cast("\xaa\x55\xaa\x55\xaa\x55\xaa\x55"))
                        ), r, gdi::ColorCtx::from_bpp(this->bpp, BGRPalette::classic_332()));
                }
                break;
                case 4:
                {
                    const Rect r(
                        stream.in_sint16_le(),
                        stream.in_sint16_le(),
                        stream.in_uint16_le(),
                        stream.in_uint16_le());
                    const int srcx = stream.in_sint16_le();
                    const int srcy = stream.in_sint16_le();
                    const RDPScrBlt scrblt(r, 0xCC, srcx, srcy);
                    gd.draw(scrblt, r);
                }
                break;
                case 5:
                {
                    const Rect r(
                        stream.in_sint16_le(),
                        stream.in_sint16_le(),
                        stream.in_uint16_le(),
                        stream.in_uint16_le());
                    const uint32_t len_bmpdata = stream.in_uint32_le();
                    const uint8_t * bmpdata = stream.in_uint8p(len_bmpdata);
                    int width = stream.in_uint16_le();
                    int height = stream.in_uint16_le();
                    int srcx = stream.in_sint16_le();
                    int srcy = stream.in_sint16_le();
                    Bitmap bmp(this->bpp, this->bpp, &BGRPalette::classic_332(), width, height, bmpdata, len_bmpdata);
                    gd.draw(RDPMemBlt(0, r, 0xCC, srcx, srcy, 0), r, bmp);
                }
                break;
                case 10: /* server_set_clip */
                {
                    const Rect r(
                        stream.in_sint16_le(),
                        stream.in_sint16_le(),
                        stream.in_uint16_le(),
                        stream.in_uint16_le());
                        // TODO see clip management
//                        this->server_set_clip(r);
                }
                break;
                case 11: /* server_reset_clip */
                        // TODO see clip management
//                        this->server_reset_clip();
                break;
                case 12: /* server_set_fgcolor */
                {
                    this->fgcolor = BGRColor(stream.in_uint32_le()); // TODO RGB or BGR ?
                }
                break;
                case 14:
                    this->rop = stream.in_uint16_le();
                break;
                case 17:
                {
                    int style = stream.in_uint16_le();
                    int width = stream.in_uint16_le();
                    this->pen.style = style;
                    this->pen.width = width;
                }
                break;
                case 18:
                {
                    int x1 = stream.in_sint16_le();
                    int y1 = stream.in_sint16_le();
                    int x2 = stream.in_sint16_le();
                    int y2 = stream.in_sint16_le();
                    const RDPLineTo lineto(
                        1, x1, y1, x2, y2, color_encode(WHITE, this->bpp), this->rop,
                        RDPPen(this->pen.style, this->pen.width, encode_color24()(this->fgcolor)));
                    gd.draw(lineto, Rect(0,0,1,1), gdi::ColorCtx::from_bpp(this->bpp, BGRPalette::classic_332()));
                }
                break;
                case 19:
                {
//                        auto hotspot_x = stream.in_uint16_le();
                    stream.in_skip_bytes(2);
//                        auto hotspot_y = stream.in_uint16_le();
                    stream.in_skip_bytes(2);
//                      Hotspot hotspot(hotspot_x, hotspot_y);
                    stream.in_skip_bytes(32 * (32 * 3)); // data
                    stream.in_skip_bytes(32 * (32 / 8)); // mask
                    // TODO: we just ignore cursor data for now. Fix that later
                    gd.set_pointer(0, dot_pointer(), gdi::GraphicApi::SetPointerMode::Insert);
                }
                break;
                default:
                    throw 1;
                }
            }
        }
    }
    catch(...){
        this->session_reactor_signal = BACK_EVENT_NEXT;
        this->front.must_be_stop_capture();
    }
}
