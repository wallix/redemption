/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.
h
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Clément Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Vnc module
*/

#pragma once

#include <cstdlib>

#include <zlib.h>

#include "core/buf64k.hpp"
#include "core/channel_list.hpp"
#include "core/channel_names.hpp"
#include "core/front_api.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "core/RDP/pointer.hpp"
#include "core/report_message_api.hpp"
#include "keyboard/keymapSym.hpp"
#include "main/version.hpp"
#include "mod/internal/client_execute.hpp"
#include "mod/internal/internal_mod.hpp"
#include "mod/internal/widget/flat_vnc_authentification.hpp"
#include "mod/internal/widget/notify_api.hpp"
#include "utils/diffiehellman.hpp"
#include "utils/hexdump.hpp"
#include "utils/d3des.hpp"
#include "utils/key_qvalue_pairs.hpp"
#include "utils/log.hpp"
#include "utils/sugar/make_unique.hpp"
#include "utils/sugar/update_lock.hpp"
#include "utils/sugar/strutils.hpp"
#include "utils/utf.hpp"
#include "utils/verbose_flags.hpp"
#include "utils/zlib.hpp"
#include "mod/vnc/vnc_verbose.hpp"

#include "mod/vnc/encoder/encoder_api.hpp"
#include "mod/vnc/encoder/zrle.hpp"
#include "mod/vnc/encoder/raw.hpp"

// got extracts of VNC documentation from
// http://tigervnc.sourceforge.net/cgi-bin/rfbproto

class mod_vnc : public InternalMod, private NotifyApi
{
    static const uint32_t MAX_CLIPBOARD_DATA_SIZE = 1024 * 64;

    FlatVNCAuthentification challenge;

    /* mod data */
    char mod_name[256];
    char username[256];
    char password[256];

public:
    struct Mouse {
        void move(Transport & t, int x, int y) {
            this->x = x;
            this->y = y;
            this->send(t);
        }

        void click(Transport & t, int x, int y, int mask, bool set) {
            if (set) {
                this->mod_mouse_state |= mask;
            }
            else {
                this->mod_mouse_state &= ~mask;
            }
            this->x = x;
            this->y = y;
            this->send(t);
        }

        void scroll(Transport & t, int mask) const {
            StaticOutStream<12> stream;
            this->write(stream, this->mod_mouse_state | mask);
            this->write(stream, this->mod_mouse_state);
            t.send(stream.get_data(), stream.get_offset());
        }

    private:
        uint8_t mod_mouse_state = 0;
        int x = 0;
        int y = 0;

        void write(OutStream & stream, uint8_t state) const {
            stream.out_uint8(5);
            stream.out_uint8(state);
            stream.out_uint16_be(this->x);
            stream.out_uint16_be(this->y);
        }

        void send(Transport & t) const {
            StaticOutStream<6> stream;
            this->write(stream, this->mod_mouse_state);
            t.send(stream.get_data(), stream.get_offset());
        }
    } mouse;

private:
    Transport & t;

    uint16_t width;
    uint16_t height;
    uint8_t  bpp;
    uint8_t  depth;

    uint8_t endianess;
    uint8_t true_color_flag;

    uint16_t red_max;
    uint16_t green_max;
    uint16_t blue_max;

    uint8_t red_shift;
    uint8_t green_shift;
    uint8_t blue_shift;

public:
    VNCVerbose verbose;

private:
    KeymapSym  keymapSym;

    StaticOutStream<MAX_CLIPBOARD_DATA_SIZE> to_vnc_clipboard_data;
    uint32_t to_vnc_clipboard_data_size;
    uint32_t to_vnc_clipboard_data_remaining;

    const bool enable_clipboard_up;   // true clipboard available, false clipboard unavailable
    const bool enable_clipboard_down; // true clipboard available, false clipboard unavailable

    bool client_use_long_format_names = false;
    bool server_use_long_format_names = true;

    enum {
        ASK_PASSWORD,
        DO_INITIAL_CLEAR_SCREEN,
        RETRY_CONNECTION,
        UP_AND_RUNNING,
        WAIT_PASSWORD,
        WAIT_SECURITY_TYPES,
        WAIT_SECURITY_TYPES_LEVEL,
        WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM,
        WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM_RESPONSE,
        WAIT_SECURITY_TYPES_MS_LOGON,
        WAIT_SECURITY_TYPES_MS_LOGON_RESPONSE,
        WAIT_SECURITY_TYPES_INVALID_AUTH,
        SERVER_INIT,
        SERVER_INIT_RESPONSE,
        WAIT_CLIENT_UP_AND_RUNNING
    };

public:
    enum class ClipboardEncodingType : uint8_t {
        UTF8   = 0,
        Latin1 = 1
    };

private:
    std::string encodings;

    int state;

    bool allow_authentification_retries;

    bool left_ctrl_pressed = false;

    bool     clipboard_requesting_for_data_is_delayed = false;
    int      clipboard_requested_format_id            = 0;
    std::chrono::microseconds clipboard_last_client_data_timestamp = std::chrono::microseconds{};

    ClipboardEncodingType clipboard_server_encoding_type;

    bool clipboard_owned_by_client = true;

    VncBogusClipboardInfiniteLoop bogus_clipboard_infinite_loop = VncBogusClipboardInfiniteLoop::delayed;

    uint32_t clipboard_general_capability_flags = 0;

    ReportMessageApi & report_message;

    time_t beginning;

    bool server_is_apple;

    int keylayout;

    ClientExecute* client_execute = nullptr;

    Zdecompressor<> zd;

public:
    mod_vnc( Transport & t
           , const char * username
           , const char * password
           , FrontAPI & front
           , uint16_t front_width
           , uint16_t front_height
           , Font const & font
           , const char * label_text_message
           , const char * label_text_password
           , Theme const & theme
           , int keylayout
           , int key_flags
           , bool clipboard_up
           , bool clipboard_down
           , const char * encodings
           , bool allow_authentification_retries
           , bool /*TODO is_socket_transport*/
           , ClipboardEncodingType clipboard_server_encoding_type
           , VncBogusClipboardInfiniteLoop bogus_clipboard_infinite_loop
           , ReportMessageApi & report_message
           , bool server_is_apple
           , ClientExecute* client_execute
           , VNCVerbose verbose
           )
    : InternalMod(front, front_width, front_height, font, theme, false)
    , challenge(front, front_width, front_height, this->screen, static_cast<NotifyApi*>(this),
                "Redemption " VERSION, this->theme(), label_text_message, label_text_password,
                this->font())
    , mod_name{0}
    , username{0}
    , password{0}
    , t(t)
    , width(0)
    , height(0)
    , bpp(0)
    , depth(0)
    , verbose(verbose)
    , keymapSym(static_cast<uint32_t>(verbose))
    , to_vnc_clipboard_data_size(0)
    , enable_clipboard_up(clipboard_up)
    , enable_clipboard_down(clipboard_down)
    , encodings(encodings)
    , state(WAIT_SECURITY_TYPES)
    , allow_authentification_retries(allow_authentification_retries || !(*password))
    , clipboard_server_encoding_type(clipboard_server_encoding_type)
    , bogus_clipboard_infinite_loop(bogus_clipboard_infinite_loop)
    , report_message(report_message)
    , server_is_apple(server_is_apple)
    , keylayout(keylayout)
    , client_execute(client_execute)
    , frame_buffer_update_ctx(this->zd, verbose)
    , clipboard_data_ctx(verbose)
    {
        LOG(LOG_INFO, "Creation of new mod 'VNC'");

        // Clear client screen
        this->invoke_asynchronous_graphic_task(AsynchronousGraphicTask::clear_screen);

        ::time(&this->beginning);

        // TODO init layout sym with apple layout
        if (this->server_is_apple) {
            keymapSym.init_layout_sym(0x0409);
        } else {
            keymapSym.init_layout_sym(keylayout);
        }
        // Initial state of keys (at least lock keys) is copied from Keymap2
        keymapSym.key_flags = key_flags;

        std::snprintf(this->username, sizeof(this->username), "%s", username);
        std::snprintf(this->password, sizeof(this->password), "%s", password);

        LOG(LOG_INFO, "Creation of new mod 'VNC' done");
    } // Constructor

    ~mod_vnc() override {
        this->screen.clear();
    }

    int get_fd() const override { return this->t.get_fd(); }

    template<std::size_t MaxLen>
    class MessageCtx
    {
        static_assert(MaxLen < 32*1024, "inefficient");

        enum State
        {
            Size,
            Data,
            Strip,
        };

    public:
        void restart()
        {
            this->state = State::Size;
        }

        template<class F>
        bool run(Buf64k & buf, F && f)
        {
            switch (this->state)
            {
                case State::Size:
                    this->state = this->read_size(buf);
                    break;
                case State::Data:
                    this->state = this->read_data(buf, f);
                    if (this->state == State::Size) {
                        return true;
                    }
                    break;
                case State::Strip:
                    this->state = this->strip_data(buf);
                    if (this->state == State::Size) {
                        return true;
                    }
                    break;
            }
            return false;
        }

    private:
        State state = State::Size;
        uint32_t len;

        State read_size(Buf64k & buf)
        {
            const size_t sz = 4;

            if (buf.remaining() < sz)
            {
                return State::Size;
            }

            this->len = InStream(buf.av(sz)).in_uint32_be();

            buf.advance(sz);

            return State::Data;
        }

        template<class F>
        State read_data(Buf64k & buf, F & f)
        {
            const size_t sz = std::min<size_t>(MaxLen, this->len);

            if (buf.remaining() < sz)
            {
                return State::Data;
            }

            f(array_view_u8{buf.av().data(), sz});
            buf.advance(sz);

            if (sz == this->len) {
                return State::Size;
            }

            this->len -= MaxLen;
            return State::Strip;
        }

        State strip_data(Buf64k & buf)
        {
            const size_t sz = std::min<size_t>(buf.remaining(), this->len);
            this->len -= sz;
            buf.advance(sz);

            return this->len ? State::Strip : State::Size;
        }
    };

    template<class T>
    struct BasicResult
    {
        static BasicResult fail() noexcept
        {
            return BasicResult{};
        }

        static BasicResult ok(T value) noexcept
        {
            return BasicResult{value};
        }

        bool operator!() const noexcept
        {
            return !this->is_ok;
        }

        explicit operator bool () const noexcept
        {
            return this->is_ok;
        }

        operator T () const noexcept
        {
            return this->value;
        }

    private:
        BasicResult() noexcept
          : is_ok(false)
        {}

        BasicResult(T value) noexcept
          : is_ok(true)
          , value(value)
        {}

        bool is_ok;
        T value;
    };

    class AuthResponseCtx
    {
        enum class State
        {
            Header,
            ReasonFail,
            Finish,
        };

        using Result = BasicResult<State>;

    public:
        void restart()
        {
            this->state = State::Header;
        }

        template<class F> // f(bool status, array_view_u8 raison_fail)
        bool run(Buf64k & buf, F && f)
        {
            switch (this->state)
            {
                case State::Header:
                    if (auto r = this->read_header(buf)) {
                        if (r == State::Finish) {
                            f(true, array_view_u8{});
                            return true;
                        }
                        this->reason.restart();
                        this->state = r;
                    }
                    else {
                        return false;
                    }
                    REDEMPTION_CXX_FALLTHROUGH;
                case State::ReasonFail:
                    return this->reason.run(buf, [&f](array_view_u8 av){ f(false, av); });
                case State::Finish:
                    return true;
            }

            REDEMPTION_UNREACHABLE();
        }

    private:
        using ReasonCtx = MessageCtx<256>;

        State state = State::Header;
        ReasonCtx reason;

        Result read_header(Buf64k & buf)
        {
            const size_t sz = 4;

            if (buf.remaining() < sz)
            {
                return Result::fail();
            }

            uint32_t const i = InStream(buf.av(sz)).in_uint32_be();

            buf.advance(sz);

            return Result::ok(i ? State::ReasonFail : State::Finish);
        }
    };
    AuthResponseCtx auth_response_ctx;

    class MsLogonCtx
    {
        enum class State
        {
            Data,
            Finish,
        };

    public:
        void restart() noexcept
        {
            this->state = State::Data;
        }

        bool run(Buf64k & buf)
        {
            return State::Finish == this->read_data(buf);
        }

        uint64_t gen;
        uint64_t mod;
        uint64_t resp;

    private:
        State state = State::Data;

        State read_data(Buf64k & buf)
        {
            const size_t sz = 24;

            if (buf.remaining() < sz)
            {
                return State::Data;
            }

            InStream stream(buf.av(sz));
            this->gen = stream.in_uint64_be();
            this->mod = stream.in_uint64_be();
            this->resp = stream.in_uint64_be();

            buf.advance(sz);

            return State::Finish;
        }
    };
    MsLogonCtx ms_logon_ctx;

    bool ms_logon(Buf64k & buf)
    {
        if (!this->ms_logon_ctx.run(buf)) {
            return false;
        }

        if (bool(this->verbose & VNCVerbose::basic_trace)) {
            LOG(LOG_INFO, "MS-Logon with following values:");
            LOG(LOG_INFO, "Gen=%" PRIu64, this->ms_logon_ctx.gen);
            LOG(LOG_INFO, "Mod=%" PRIu64, this->ms_logon_ctx.mod);
            LOG(LOG_INFO, "Resp=%" PRIu64, this->ms_logon_ctx.resp);
        }

        DiffieHellman dh(this->ms_logon_ctx.gen, this->ms_logon_ctx.mod);
        uint64_t pub = dh.createInterKey();

        StaticOutStream<32768> out_stream;
        out_stream.out_uint64_be(pub);

        uint64_t key = dh.createEncryptionKey(this->ms_logon_ctx.resp);
        uint8_t keybuffer[8] = {};
        dh.uint64_to_uint8p(key, keybuffer);

        rfbDesKey(keybuffer, EN0); // 0, encrypt

        uint8_t ms_username[256] = {};
        uint8_t ms_password[64] = {};
        memcpy(ms_username, this->username, 256);
        memcpy(ms_password, this->password, 64);
        uint8_t cp_username[256] = {};
        uint8_t cp_password[64] = {};
        rfbDesText(ms_username, cp_username, sizeof(ms_username), keybuffer);
        rfbDesText(ms_password, cp_password, sizeof(ms_password), keybuffer);

        out_stream.out_copy_bytes(cp_username, 256);
        out_stream.out_copy_bytes(cp_password, 64);

        this->t.send(out_stream.get_data(), out_stream.get_offset());
        // sec result

        return true;
    }

    MessageCtx<8192> invalid_auth_ctx;

    // 7.3.2   ServerInit
    // ------------------
    // After receiving the ClientInit message, the server sends a
    // ServerInit message. This tells the client the width and
    // height of the server's framebuffer, its pixel format and the
    // name associated with the desktop:

    // framebuffer-width  : 2 bytes
    // framebuffer-height : 2 bytes

    // PIXEL_FORMAT       : 16 bytes
    // VNC pixel_format capabilities
    // -----------------------------
    // Server-pixel-format specifies the server's natural pixel
    // format. This pixel format will be used unless the client
    // requests a different format using the SetPixelFormat message
    // (SetPixelFormat).

    // PIXEL_FORMAT::bits per pixel  : 1 byte
    // PIXEL_FORMAT::color depth     : 1 byte

    // Bits-per-pixel is the number of bits used for each pixel
    // value on the wire. This must be greater than or equal to the
    // depth which is the number of useful bits in the pixel value.
    // Currently bits-per-pixel must be 8, 16 or 32. Less than 8-bit
    // pixels are not yet supported.

    // PIXEL_FORMAT::endianess       : 1 byte (0 = LE, 1 = BE)

    // Big-endian-flag is non-zero (true) if multi-byte pixels are
    // interpreted as big endian. Of course this is meaningless
    // for 8 bits-per-pixel.

    // PIXEL_FORMAT::true color flag : 1 byte
    // PIXEL_FORMAT::red max         : 2 bytes
    // PIXEL_FORMAT::green max       : 2 bytes
    // PIXEL_FORMAT::blue max        : 2 bytes
    // PIXEL_FORMAT::red shift       : 1 bytes
    // PIXEL_FORMAT::green shift     : 1 bytes
    // PIXEL_FORMAT::blue shift      : 1 bytes

    // If true-colour-flag is non-zero (true) then the last six
    // items specify how to extract the red, green and blue
    // intensities from the pixel value. Red-max is the maximum
    // red value (= 2^n - 1 where n is the number of bits used
    // for red). Note this value is always in big endian order.
    // Red-shift is the number of shifts needed to get the red
    // value in a pixel to the least significant bit. Green-max,
    // green-shift and blue-max, blue-shift are similar for green
    // and blue. For example, to find the red value (between 0 and
    // red-max) from a given pixel, do the following:

    // * Swap the pixel value according to big-endian-flag (e.g.
    // if big-endian-flag is zero (false) and host byte order is
    // big endian, then swap).
    // * Shift right by red-shift.
    // * AND with red-max (in host byte order).

    // If true-colour-flag is zero (false) then the server uses
    // pixel values which are not directly composed from the red,
    // green and blue intensities, but which serve as indices into
    // a colour map. Entries in the colour map are set by the
    // server using the SetColourMapEntries message
    // (SetColourMapEntries).

    // PIXEL_FORMAT::padding         : 3 bytes

    // name-length        : 4 bytes
    // name-string        : variable

    // The text encoding used for name-string is historically undefined but it is strongly recommended to use UTF-8 (see String Encodings for more details).

    // TODO not yet supported
    // If the Tight Security Type is activated, the server init
    // message is extended with an interaction capabilities section.

    class ServerInitCtx
    {
        enum class State
        {
            PixelFormat,
            EncodingName,
        };

    public:
        void restart()
        {
            this->state = State::PixelFormat;
        }

        bool run(Buf64k & buf, mod_vnc & vnc)
        {
            switch (this->state)
            {
            case State::PixelFormat:
                this->state = this->read_pixel_format(buf, vnc);
                if (this->state != State::EncodingName) {
                    break;
                }
                REDEMPTION_CXX_FALLTHROUGH;
            case State::EncodingName:
                this->state = this->read_encoding_name(buf, vnc);
                if (this->state == State::PixelFormat) {
                    return true;
                }
            }
            return false;
        }

    private:
        State state = State::PixelFormat;
        uint32_t lg;

        State read_pixel_format(Buf64k & buf, mod_vnc & vnc)
        {
            const size_t sz = 24;

            if (buf.remaining() < sz)
            {
                return State::PixelFormat;
            }

            InStream stream(buf.av(sz));
            vnc.width = stream.in_uint16_be();
            vnc.height = stream.in_uint16_be();
            vnc.bpp    = stream.in_uint8();
            vnc.depth  = stream.in_uint8();
            vnc.endianess = stream.in_uint8();
            vnc.true_color_flag = stream.in_uint8();
            vnc.red_max = stream.in_uint16_be();
            vnc.green_max = stream.in_uint16_be();
            vnc.blue_max = stream.in_uint16_be();
            vnc.red_shift = stream.in_uint8();
            vnc.green_shift = stream.in_uint8();
            vnc.blue_shift = stream.in_uint8();
            stream.in_skip_bytes(3); // skip padding

            // LOG(LOG_INFO, "VNC received: width=%d height=%d bpp=%d depth=%d endianess=%d true_color=%d red_max=%d green_max=%d blue_max=%d red_shift=%d green_shift=%d blue_shift=%d", this->width, this->height, this->bpp, this->depth, this->endianess, this->true_color_flag, this->red_max, this->green_max, this->blue_max, this->red_shift, this->green_shift, this->blue_shift);

            this->lg = stream.in_uint32_be();

            if (this->lg > sizeof(vnc.mod_name)-1) {
                LOG(LOG_ERR, "VNC connection error");
                throw Error(ERR_VNC_CONNECTION_ERROR);
            }

            buf.advance(sz);
            return State::EncodingName;
        }

        State read_encoding_name(Buf64k & buf, mod_vnc & vnc)
        {
            if (buf.remaining() < this->lg)
            {
                return State::EncodingName;
            }

            memcpy(vnc.mod_name, buf.av().data(), this->lg);
            vnc.mod_name[this->lg] = 0;

            buf.advance(this->lg);
            return State::PixelFormat;
        }
    };
    ServerInitCtx server_init_ctx;

    void initial_clear_screen(gdi::GraphicApi & drawable)
    {
        if (bool(this->verbose & VNCVerbose::connection)) {
            LOG(LOG_INFO, "state=DO_INITIAL_CLEAR_SCREEN");
        }
        // set almost null cursor, this is the little dot cursor
        Pointer cursor;
        cursor.x = 3;
        cursor.y = 3;
        // cursor.bpp = 24;
        cursor.width = 32;
        cursor.height = 32;
        memset(cursor.data + 31 * (32 * 3), 0xff, 9);
        memset(cursor.data + 30 * (32 * 3), 0xff, 9);
        memset(cursor.data + 29 * (32 * 3), 0xff, 9);
        memset(cursor.mask, 0xff, 32 * (32 / 8));
        cursor.update_bw();
        this->front.set_pointer(cursor);

        this->report_message.log5("type=\"SESSION_ESTABLISHED_SUCCESSFULLY\"");

        LOG(LOG_INFO, "VNC connection complete, connected ok\n");

        Rect const screen_rect(0, 0, this->width, this->height);

        this->front.begin_update();
        RDPOpaqueRect orect(screen_rect, RDPColor{});
        drawable.draw(orect, screen_rect, gdi::ColorCtx::from_bpp(this->bpp, this->palette_update_ctx.get_palette()));
        this->front.end_update();

        this->state = UP_AND_RUNNING;
        this->front.can_be_start_capture();
        this->update_screen(screen_rect);
        this->lib_open_clip_channel();

        this->event.reset_trigger_time();
        if (bool(this->verbose & VNCVerbose::connection)) {
            LOG(LOG_INFO, "VNC screen cleaning ok\n");
        }

        RDPECLIP::GeneralCapabilitySet general_caps(
            RDPECLIP::CB_CAPS_VERSION_1,
            this->server_use_long_format_names?RDPECLIP::CB_USE_LONG_FORMAT_NAMES:0);

        if (bool(this->verbose & VNCVerbose::basic_trace)) {
            LOG(LOG_INFO, "Server use %s format name",
                (this->server_use_long_format_names ? "long" : "short"));
        }

        RDPECLIP::ClipboardCapabilitiesPDU clip_cap_pdu(
                1,
                RDPECLIP::GeneralCapabilitySet::size()
            );

        StaticOutStream<1024> out_s;

        clip_cap_pdu.emit(out_s);
        general_caps.emit(out_s);

        size_t length     = out_s.get_offset();
        size_t chunk_size = length;

        if (bool(this->verbose & VNCVerbose::basic_trace)) {
            LOG(LOG_INFO, "mod_vnc server clipboard PDU: msgType=%s(%d)",
                RDPECLIP::get_msgType_name(clip_cap_pdu.header.msgType()),
                clip_cap_pdu.header.msgType()
                );
        }

        this->send_to_front_channel( channel_names::cliprdr
                                   , out_s.get_data()
                                   , length
                                   , chunk_size
                                   ,   CHANNELS::CHANNEL_FLAG_FIRST
                                     | CHANNELS::CHANNEL_FLAG_LAST
                                   );

        RDPECLIP::ServerMonitorReadyPDU server_monitor_ready_pdu;

        out_s.rewind();

        server_monitor_ready_pdu.emit(out_s);

        length     = out_s.get_offset();
        chunk_size = length;

        if (bool(this->verbose & VNCVerbose::basic_trace)) {
            LOG(LOG_INFO, "mod_vnc server clipboard PDU: msgType=%s(%d)",
                RDPECLIP::get_msgType_name(server_monitor_ready_pdu.header.msgType()),
                server_monitor_ready_pdu.header.msgType()
                );
        }

        this->send_to_front_channel( channel_names::cliprdr
                                   , out_s.get_data()
                                   , length
                                   , chunk_size
                                   ,   CHANNELS::CHANNEL_FLAG_FIRST
                                     | CHANNELS::CHANNEL_FLAG_LAST
                                   );
    }

    // TODO It may be possible to change several mouse buttons at once ? Current code seems to perform several send if that occurs. Is it what we want ?
    void rdp_input_mouse( int device_flags, int x, int y, Keymap2 * keymap ) override {
        if (this->state == WAIT_PASSWORD) {
            this->screen.rdp_input_mouse(device_flags, x, y, keymap);
            return;
        }

        if (this->state != UP_AND_RUNNING) {
            return;
        }

        if (device_flags & MOUSE_FLAG_MOVE) {
            this->mouse.move(this->t, x, y);
        }
        else if (device_flags & MOUSE_FLAG_BUTTON1) {
            this->mouse.click(this->t, x, y, 1 << 0, device_flags & MOUSE_FLAG_DOWN);
        }
        else if (device_flags & MOUSE_FLAG_BUTTON2) {
            this->mouse.click(this->t, x, y, 1 << 2, device_flags & MOUSE_FLAG_DOWN);
        }
        else if (device_flags & MOUSE_FLAG_BUTTON3) {
            this->mouse.click(this->t, x, y, 1 << 1, device_flags & MOUSE_FLAG_DOWN);
        }
        else if (device_flags == MOUSE_FLAG_BUTTON4 || device_flags == 0x0278) {
            this->mouse.scroll(this->t, 1 << 3);
        }
        else if (device_flags == MOUSE_FLAG_BUTTON5 || device_flags == 0x0388) {
            this->mouse.scroll(this->t, 1 << 4);
        }
    } // rdp_input_mouse

    //==============================================================================================================
    void rdp_input_scancode( long param1
                                   , long param2
                                   , long device_flags
                                   , long param4
                                   , Keymap2 * keymap
                                   ) override {
    //==============================================================================================================
        if (this->state == WAIT_PASSWORD) {
            this->screen.rdp_input_scancode(param1, param2, device_flags, param4, keymap);
            return;
        }

        if (this->state != UP_AND_RUNNING) {
            return;
        }

        // AltGr or Alt are catched by Appel OS

        // TODO detect if target is a Apple server and set layout to US before to call keymapSym::event()

        // TODO As down/up state is not stored in keymapSym, code below is quite dangerous
        if (bool(this->verbose & VNCVerbose::basic_trace)) {
            LOG(LOG_INFO, "mod_vnc::rdp_input_scancode(device_flags=%ld, param1=%ld)", device_flags, param1);
        }

        uint8_t downflag = !(device_flags & KBD_FLAG_UP);

        if (this->server_is_apple) {
            this->apple_keyboard_translation(device_flags, param1, downflag);
        } else {
            this->keyMapSym_event(device_flags, param1, downflag);
        }
    } // rdp_input_scancode

    void rdp_input_unicode(uint16_t /*unicode*/, uint16_t /*flag*/) override {
        LOG(LOG_WARNING, "mod_vnc::rdp_input_unicode: Unicode Keyboard Event is not yet supported");
    }


    void keyMapSym_event(int device_flags, long param1, uint8_t downflag) {
        this->keymapSym.event(device_flags, param1);
        int key = this->keymapSym.get_sym();

        if (key > 0) {
            if (this->left_ctrl_pressed) {
                if (key == 0xfe03) {
                    // alt gr => left ctrl is ignored
                    this->send_keyevent(downflag, key);
                }
                else {
                    this->send_keyevent(1, 0xffe3);
                    this->send_keyevent(downflag, key);
                }
                this->left_ctrl_pressed = false;
            }
            else if (!((key == 0xffe3) && downflag)) {
                this->send_keyevent(downflag, key);
            }
            else {
                // left ctrl is down
                this->left_ctrl_pressed = true;
            }
        }
    }

    void send_keyevent(uint8_t down_flag, uint32_t key) {
        if (bool(this->verbose & VNCVerbose::basic_trace)) {
            LOG(LOG_INFO, "VNC Send KeyEvent Flag down: %d, key: 0x%x", down_flag, key);
        }
        StaticOutPerStream<8> stream;
        stream.out_uint8(4);
        stream.out_uint8(down_flag); /* down/up flag */
        stream.out_clear_bytes(2);
        stream.out_uint32_be(key);
        this->t.send(stream.get_data(), stream.get_offset());
        this->event.set_trigger_time(1000);
    }

    void apple_keyboard_translation(int device_flags, long param1, uint8_t downflag) {

        switch (this->keylayout) {

            case 0x040c:                                    // French
                switch (param1) {

                    case 0x0b:
                        if (this->keymapSym.is_alt_pressed()) {
                            this->send_keyevent(0, 0xffe9);
                            this->send_keyevent(downflag, 0xa4); /* @ */
                            this->send_keyevent(1, 0xffe9);
                        } else {
                            this->keyMapSym_event(device_flags, param1, downflag);
                        }
                        break;

                    case 0x04:
                        if (this->keymapSym.is_alt_pressed()) {
                            this->send_keyevent(0, 0xffe9);
                            this->send_keyevent(1, 0xffe2);
                            this->send_keyevent(downflag, 0xa4); /* # */
                            this->send_keyevent(0, 0xffe2);
                            this->send_keyevent(1, 0xffe9);
                        } else {
                            this->keyMapSym_event(device_flags, param1, downflag);
                        }
                        break;

                    case 0x35:
                        if (this->keymapSym.is_shift_pressed()) {
                            this->send_keyevent(0, 0xffe2);
                            this->send_keyevent(downflag, 0x36); /* § */
                            this->send_keyevent(1, 0xffe2);
                        } else {
                            if (device_flags & KeymapSym::KBDFLAGS_EXTENDED) {
                                this->send_keyevent(1, 0xffe2);
                                this->send_keyevent(downflag, 0x3e); /* / */
                                this->send_keyevent(0, 0xffe2);
                            } else {
                                this->send_keyevent(downflag, 0x38); /* ! */
                            }
                        }
                        break;

                    case 0x07: /* - */
                        if (!this->keymapSym.is_shift_pressed()) {
                            this->send_keyevent(1, 0xffe2);
                            this->send_keyevent(downflag, 0x3d);
                            this->send_keyevent(0, 0xffe2);
                        } else {
                            this->keyMapSym_event(device_flags, param1, downflag);
                        }
                        break;

                    case 0x2b: /* * */
                        this->send_keyevent(1, 0xffe2);
                        this->send_keyevent(downflag, 0x2a);
                        this->send_keyevent(0, 0xffe2);
                        break;

                    case 0x1b: /* £ */
                        if (this->keymapSym.is_shift_pressed()) {
                            this->send_keyevent(downflag, 0x5c);
                        } else {
                            this->keyMapSym_event(device_flags, param1, downflag);
                        }
                        break;

                    case 0x09: /* _ */
                        if (!this->keymapSym.is_shift_pressed()) {
                            this->send_keyevent(1, 0xffe2);
                            this->send_keyevent(downflag, 0xad);
                            this->send_keyevent(0, 0xffe2);
                        } else {
                            this->send_keyevent(downflag, 0x38); /* 8 */
                        }
                        break;

                    case 0x56:
                        if (this->keymapSym.is_shift_pressed()) {
                            this->send_keyevent(downflag, 0x7e); /* > */
                        } else {
                            this->send_keyevent(1, 0xffe2);
                            this->send_keyevent(downflag, 0x60); /* < */
                            this->send_keyevent(0, 0xffe2);
                        }
                        break;

                    case 0x0d: /* = */
                        this->send_keyevent(downflag, 0x2f);
                        break;

                    default:
                        this->keyMapSym_event(device_flags, param1, downflag);
                        break;
                }
                break;

            case 0x0407: // GERMAN
                // TODO treat problematic case
                switch (param1) {
                    default:
                        this->keyMapSym_event(device_flags, param1, downflag);
                        break;
                }
                break;

            case 0x0409: // United States
                // TODO treat problematic case
                switch (param1) {
                    default:
                        this->keyMapSym_event(device_flags, param1, downflag);
                        break;
                }
                break;

           case 0x0410: // Italian
               // TODO treat problematic case
               switch (param1) {
                    default:
                        this->keyMapSym_event(device_flags, param1, downflag);
                        break;
               }
               break;

            case 0x0419: // Russian
               // TODO treat problematic case
               switch (param1) {
                    default:
                        this->keyMapSym_event(device_flags, param1, downflag);
                        break;
               }
               break;

            case 0x041d: // Swedish
               // TODO treat problematic case
               switch (param1) {
                    default:
                        this->keyMapSym_event(device_flags, param1, downflag);
                        break;
               }
               break;

            case 0x046e: // Luxemburgish
               // TODO treat problematic case
               switch (param1) {
                    default:
                        this->keyMapSym_event(device_flags, param1, downflag);
                        break;
               }
               break;

            case 0x0807: // German Swizerland
               // TODO treat problematic case
               switch (param1) {
                    default:
                        this->keyMapSym_event(device_flags, param1, downflag);
                        break;
               }
               break;

            case 0x0809: // English UK
               // TODO treat problematic case
               switch (param1) {
                    default:
                        this->keyMapSym_event(device_flags, param1, downflag);
                        break;
               }
               break;

            case 0x080c: // French Belgium
               // TODO treat problematic case
               switch (param1) {
                    default:
                        this->keyMapSym_event(device_flags, param1, downflag);
                        break;
               }
               break;

            case 0x0813: // Dutch Belgium
               // TODO treat problematic case
               switch (param1) {
                    default:
                        this->keyMapSym_event(device_flags, param1, downflag);
                        break;
               }
               break;

            case 0x100c: // French Swizerland
               // TODO treat problematic case
               switch (param1) {
                    default:
                        this->keyMapSym_event(device_flags, param1, downflag);
                        break;
               }
               break;

            default:
               this->keyMapSym_event(device_flags, param1, downflag);
               break;
        }
    }

protected:
/*
    //==============================================================================================================
    void rdp_input_clip_data(const uint8_t * data, uint32_t length) {
    //==============================================================================================================
        if (this->state != UP_AND_RUNNING) {
            return;
        }

        StreamBufMaker<65535> stream_maker;
        OutStream stream = stream_maker.reserve_out_stream((length + 8);

        stream.out_uint8(6);                      // message-type : ClientCutText
        stream.out_clear_bytes(3);                // padding
        stream.out_uint32_be(length);             // length
        stream.out_copy_bytes(data, length);      // text

        this->t.send(stream.get_data(), (length + 8));

        this->event.set(1000);
    } // rdp_input_clip_data
*/

    void rdp_input_clip_data(uint8_t * data, size_t data_length) {
        auto client_cut_text = [this](char * str) {
            ::in_place_windows_to_linux_newline_convert(str);
            size_t const str_len = ::strlen(str);

            StreamBufMaker<65536> buf_maker;
            OutStream stream = buf_maker.reserve_out_stream(str_len + 8);

            stream.out_uint8(6);                    // message-type : ClientCutText
            stream.out_clear_bytes(3);              // padding
            stream.out_uint32_be(str_len);          // length
            stream.out_copy_bytes(str, str_len);    // text

            this->t.send(stream.get_data(), stream.get_offset());

            this->event.set_trigger_time(1000);
        };

        if (this->state == UP_AND_RUNNING) {
            if (this->clipboard_requested_format_id == RDPECLIP::CF_UNICODETEXT) {
                if (this->clipboard_server_encoding_type == ClipboardEncodingType::UTF8) {
                    if (bool(this->verbose & VNCVerbose::clipboard)) {
                        LOG(LOG_INFO, "mod_vnc::rdp_input_clip_data: CF_UNICODETEXT -> UTF-8");
                    }

                    const size_t utf8_data_length =
                        data_length / sizeof(uint16_t) * maximum_length_of_utf8_character_in_bytes + 1;
                    std::unique_ptr<uint8_t[]> utf8_data(new uint8_t[utf8_data_length]);

                    const auto len = ::UTF16toUTF8(data, data_length, utf8_data.get(),  utf8_data_length);
                    utf8_data[len] = 0;

                    client_cut_text(::char_ptr_cast(utf8_data.get()));
                }
                else {
                    if (bool(this->verbose & VNCVerbose::clipboard)) {
                        LOG(LOG_INFO, "mod_vnc::rdp_input_clip_data: CF_UNICODETEXT -> Latin-1");
                    }

                    const size_t latin1_data_length = data_length / sizeof(uint16_t) + 1;
                    std::unique_ptr<uint8_t[]> latin1_data(new uint8_t[latin1_data_length]);

                    const auto len = ::UTF16toLatin1(data, data_length, latin1_data.get(), latin1_data_length);
                    latin1_data[len] = 0;

                    client_cut_text(::char_ptr_cast(latin1_data.get()));
                }
            }
            else {
                if (this->clipboard_server_encoding_type == ClipboardEncodingType::UTF8) {
                    if (bool(this->verbose & VNCVerbose::clipboard)) {
                        LOG(LOG_INFO, "mod_vnc::rdp_input_clip_data: CF_TEXT -> UTF-8");
                    }

                    const size_t utf8_data_length = data_length * 2 + 1;
                    auto utf8_data = std::make_unique<uint8_t[]>(utf8_data_length);

                    const size_t len = ::Latin1toUTF8(data, data_length, utf8_data.get(),
                        utf8_data_length);
                    utf8_data[len] = 0;

                    client_cut_text(::char_ptr_cast(utf8_data.get()));
                }
                else {
                    if (bool(this->verbose & VNCVerbose::clipboard)) {
                        LOG(LOG_INFO, "mod_vnc::rdp_input_clip_data: CF_TEXT -> Latin-1");
                    }

                    client_cut_text(::char_ptr_cast(data));
                }
            }
        }

        this->clipboard_owned_by_client = true;

        this->clipboard_last_client_data_timestamp = ustime();
    }

public:
    //==============================================================================================================
    void rdp_input_synchronize( uint32_t time
                                      , uint16_t device_flags
                                      , int16_t param1
                                      , int16_t param2
                                      ) override {
    //==============================================================================================================
        if (bool(this->verbose & VNCVerbose::basic_trace)) {
            LOG( LOG_INFO
               , "KeymapSym::synchronize(time=%u, device_flags=%08x, param1=%04x, param1=%04x"
               , time, device_flags, unsigned(param1), unsigned(param2));
        }
        this->keymapSym.synchronize(param1);
    } // rdp_input_synchronize

private:
    void update_screen(Rect r, uint8_t incr = 1) {
        StaticOutStream<10> stream;
        /* FramebufferUpdateRequest */
        stream.out_uint8(3);
        stream.out_uint8(incr);
        stream.out_uint16_be(r.x);
        stream.out_uint16_be(r.y);
        stream.out_uint16_be(r.cx);
        stream.out_uint16_be(r.cy);
        this->t.send(stream.get_data(), stream.get_offset());
    } // update_screen

public:
    void rdp_input_invalidate(Rect r) override {
        if (this->state == WAIT_PASSWORD) {
            this->screen.rdp_input_invalidate(r);
            return;
        }

        if (this->state != UP_AND_RUNNING) {
            return;
        }

        Rect r_ = r.intersect(Rect(0, 0, this->width, this->height));

        if (!r_.isempty()) {
            this->update_screen(r_, 0);
        }
    } // rdp_input_invalidate

    void refresh(Rect r) override {
        this->rdp_input_invalidate(r);
    }

protected:

//   Encoding value |   Mnemonic     | Encoding Description
// -------------------------------------------------------------------------------------
//    0             |                | Raw Encoding                                    |
//    1             |                | CopyRect Encoding                               |
//    2             |                | RRE Encoding                                    |
//    4             |                | CoRRE Encoding                                  |
//    5             |                | Hextile Encoding                                |
//    6             |                | zlib Encoding                                   |
//    7             |                | Tight Encoding                                  |
//    8             |                | zlibhex Encoding                                |
//    16            |                | ZRLE Encoding                                   |
//    -23 to -32    |                | JPEG Quality Level Pseudo-encoding              |
//    -223          |                | DesktopSize Pseudo-encoding                     |
//    -224          |                | LastRect Pseudo-encoding                        |
//    -239          |                | Cursor Pseudo-encoding                          |
//    -240          |                | X Cursor Pseudo-encoding                        |
//    -247 to -256  |                | Compression Level Pseudo-encoding               |
//    -257          |                | QEMU Pointer Motion Change Pseudo-encoding      |
//    -258          |                | QEMU Extended Key Event Pseudo-encoding         |
//    -259          |                | QEMU Audio Pseudo-encoding                      |
//    -261          |                | LED State Pseudo-encoding                       |
//    -305          |                | gii Pseudo-encoding                             |
//    -307          |                | DesktopName Pseudo-encoding                     |
//    -308          |                | ExtendedDesktopSize Pseudo-encoding             |
//    -309          |                | xvp Pseudo-encoding                             |
//    -312          |                | Fence Pseudo-encoding                           |
//    -313          |                | ContinuousUpdates Pseudo-encoding               |
//    -314          |                | Cursor With Alpha Pseudo-encoding               |
//    -412 to -512  |                | JPEG Fine-Grained Quality Level Pseudo-encoding |
//    -763 to -768  |                | JPEG Subsampling Level Pseudo-encoding          |
//    0xc0a1e5ce    |                | Extended Clipboard Pseudo-encoding              |

    enum rfb_encodings {
        RAW_ENCODING      = 0,
        COPYRECT_ENCODING = 1,
        RRE_ENCODING      = 2,
        CORRE_ENCODING    = 4,
        HEXTILE_ENCODING  = 5,
        ZLIB_ENCODING     = 6,
        TIGHT_ENCODING    = 7,
        ZLIBHEX_ENCODING  = 8,
        ZRLE_ENCODING     = 16,
        JPEGQL1_PSEUDO_ENCODING      = -23,
        JPEGQL2_PSEUDO_ENCODING      = -24,
        JPEGQL3_PSEUDO_ENCODING      = -25,
        JPEGQL4_PSEUDO_ENCODING      = -26,
        JPEGQL5_PSEUDO_ENCODING      = -27,
        JPEGQL6_PSEUDO_ENCODING      = -28,
        JPEGQL7_PSEUDO_ENCODING      = -29,
        JPEGQL8_PSEUDO_ENCODING      = -30,
        JPEGQL9_PSEUDO_ENCODING      = -21,
        JPEGQLA_PSEUDO_ENCODING      = -32,
        DESKTOPSIZE_PSEUDO_ENCODING  = -223,
        LASTRECT_PSEUDO_ENCODING     = -224,
        CURSOR_PSEUDO_ENCODING       = -239,
        XCURSOR_PSEUDO_ENCODING      = -240,
    };


    static void fill_encoding_types_buffer(const char * encodings, OutStream & stream, uint16_t & number_of_encodings, VNCVerbose verbose)
    {
        if (bool(verbose & VNCVerbose::basic_trace)) {
            LOG(LOG_INFO, "VNC Encodings=\"%s\"", encodings);
        }

        auto stream_offset = stream.get_offset();

        char * end;
        char const * p = encodings;
        for (int32_t encoding_type = std::strtol(p, &end, 0); p != end ; encoding_type = std::strtol(p, &end, 0))
        {
            if (bool(verbose & VNCVerbose::basic_trace)) {
                LOG(LOG_INFO, "VNC Encoding type=0x%08X", unsigned(encoding_type));
            }
            stream.out_uint32_be(encoding_type);

            p = end;
            while (*p && (*p == ' ' || *p == '\t' || *p == ',')) {
                ++p;
            }
        }
        number_of_encodings = (stream.get_offset() - stream_offset) / 4;
    }

    class PasswordCtx
    {
        enum class State
        {
            RandomKey,
            Finish,
        };

    public:
        array_view_u8 server_random;

        void restart() noexcept
        {
            this->state = State::RandomKey;
        }

        bool run(Buf64k & buf) noexcept
        {
            return State::Finish == this->read_random_number(buf);
        }

    private:
        State state = State::RandomKey;

        State read_random_number(Buf64k & buf) noexcept
        {
            const size_t sz = 16;

            if (buf.remaining() < sz)
            {
                return State::RandomKey;
            }

            this->server_random = buf.av(sz);

            buf.advance(sz);
            return State::Finish;
        }
    };

    PasswordCtx password_ctx;
    struct UpAndRunningCtx
    {
        enum class State
        {
            Header,
            FrameBufferupdate,
            Palette,
            ServerCutText,
        };

        void restart() noexcept
        {
            this->state = State::Header;
        }

        bool run(Buf64k & buf, gdi::GraphicApi & drawable, mod_vnc & vnc)
        {
            switch (this->state)
            {
            case State::Header:
                if (buf.remaining() < 1) {
                    return false;
                }

                this->message_type = buf.av()[0];

                buf.advance(1);

                switch (this->message_type)
                {
                    case 0: /* framebuffer update */
                        vnc.frame_buffer_update_ctx.start(vnc.bpp, nbbytes(vnc.bpp));
                        this->state = State::FrameBufferupdate;
                        return vnc.lib_frame_buffer_update(drawable, buf);
                    case 1: /* palette */
                        vnc.palette_update_ctx.start();
                        this->state = State::Palette;
                        return vnc.lib_palette_update(drawable, buf);
                    case 2: /* bell */
                        // TODO bell
                        return true;
                    case 3: /* clipboard */ /* ServerCutText */
                        vnc.clipboard_data_ctx.start(
                            vnc.enable_clipboard_down
                         && vnc.get_channel_by_name(channel_names::cliprdr));
                        this->state = State::ServerCutText;
                        return vnc.lib_clip_data(buf);
                    default:
                        LOG(LOG_INFO, "unknown in vnc_lib_draw_event %d\n", message_type);
                        throw Error(ERR_VNC);
                }
                break;

            case State::FrameBufferupdate: return vnc.lib_frame_buffer_update(drawable, buf);
            case State::Palette:           return vnc.lib_palette_update(drawable, buf);
            case State::ServerCutText:     return vnc.lib_clip_data(buf);
            }

            return false;
        }

    private:
        State state = State::Header;
        uint8_t message_type;
    };

    UpAndRunningCtx up_and_running_ctx;

    Buf64k buf;

public:
    void draw_event(time_t /*now*/, gdi::GraphicApi & drawable) override
    {
        if (bool(this->verbose & VNCVerbose::draw_event)) {
            LOG(LOG_INFO, "vnc::draw_event");
        }

        if (AsynchronousGraphicTask::none != this->asynchronous_graphic_task) {
            this->perform_asynchronous_graphic_task(drawable);
            return;
        }

        if (!this->event.is_waked_up_by_time()) {
            this->buf.read_from(this->t);
        }

        while (this->draw_event_impl(drawable)) {

        }

        this->check_timeout();
    }

private:
    bool draw_event_impl(gdi::GraphicApi & drawable)
    {
        switch (this->state)
        {
        case ASK_PASSWORD:
            if (bool(this->verbose & VNCVerbose::connection)) {
                LOG(LOG_INFO, "state=ASK_PASSWORD");
            }
            this->screen.add_widget(&this->challenge);

            this->screen.set_widget_focus(&this->challenge, Widget::focus_reason_tabkey);

            this->challenge.password_edit.set_text("");

            this->challenge.set_widget_focus(&this->challenge.password_edit, Widget::focus_reason_tabkey);

            this->screen.rdp_input_invalidate(this->screen.get_rect());

            this->state = WAIT_PASSWORD;
            return true;

        case DO_INITIAL_CLEAR_SCREEN:
            this->initial_clear_screen(drawable);
            return false;

        case RETRY_CONNECTION:
            if (bool(this->verbose & VNCVerbose::connection)) {
                LOG(LOG_INFO, "state=RETRY_CONNECTION");
            }
            try
            {
                if (!this->t.connect()) {
                    LOG(LOG_ERR, "VNC connection error");
                    throw Error(ERR_VNC_CONNECTION_ERROR);
                }
            }
            catch (Error const &)
            {
                throw Error(ERR_VNC_CONNECTION_ERROR);
            }

            this->state = WAIT_SECURITY_TYPES;
            return true;

        case UP_AND_RUNNING:
            if (bool(this->verbose & VNCVerbose::draw_event)) {
                LOG(LOG_INFO, "state=UP_AND_RUNNING");
            }

            if (!this->event.is_waked_up_by_time()) {
                try {
                    while (this->up_and_running_ctx.run(buf, drawable, *this)) {
                        this->up_and_running_ctx.restart();
                    }
                    return false;
                }
                catch (const Error & e) {
                    LOG(LOG_ERR, "VNC Stopped [reason id=%u]", e.id);
                    this->event.signal = BACK_EVENT_NEXT;
                    this->front.must_be_stop_capture();
                }
                catch (...) {
                    LOG(LOG_ERR, "unexpected exception raised in VNC");
                    this->event.signal = BACK_EVENT_NEXT;
                    this->front.must_be_stop_capture();
                }

                if (this->event.signal != BACK_EVENT_NEXT) {
                    this->event.set_trigger_time(1000);
                }
            }
            else {
                this->update_screen(Rect(0, 0, this->width, this->height));
            }
            return false;

        case WAIT_PASSWORD:
            if (bool(this->verbose & VNCVerbose::connection)) {
                LOG(LOG_INFO, "state=WAIT_PASSWORD");
            }
            this->event.reset_trigger_time();
            return false;

        case WAIT_SECURITY_TYPES:
            {
                if (bool(this->verbose & VNCVerbose::connection)) {
                    LOG(LOG_INFO, "state=WAIT_SECURITY_TYPES");
                }

                size_t const protocol_version_len = 12;

                if (buf.remaining() < protocol_version_len) {
                    return false;
                }

                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    // protocol_version_len - zero terminal
                    LOG(LOG_INFO, "Server Protocol Version=%.*s\n",
                        int(protocol_version_len-1), buf.av().data());
                }

                buf.advance(protocol_version_len);

                this->t.send("RFB 003.003\n", 12);
                // sec type

                this->state = WAIT_SECURITY_TYPES_LEVEL;
            }
            REDEMPTION_CXX_FALLTHROUGH;

        case WAIT_SECURITY_TYPES_LEVEL:
            {
                if (buf.remaining() < 4) {
                    return false;
                }
                int32_t const security_level = InStream(buf.av(4)).in_sint32_be();
                buf.advance(4);

                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG(LOG_INFO, "security level is %d "
                        "(1 = none, 2 = standard, -6 = mslogon)\n",
                        security_level);
                }

                switch (security_level){
                    case 1: // none
                        this->state = SERVER_INIT;
                        break;
                    case 2: // the password and the server random
                        this->state = WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM;
                        break;
                    case -6: // MS-LOGON
                        this->state = WAIT_SECURITY_TYPES_MS_LOGON;
                        break;
                    case 0:
                        this->state = WAIT_SECURITY_TYPES_INVALID_AUTH;
                        break;
                    default:
                        LOG(LOG_ERR, "vnc unexpected security level");
                        throw Error(ERR_VNC_CONNECTION_ERROR);
                }
            }
            return true;

        case WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM:
            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                LOG(LOG_INFO, "Receiving VNC Server Random");
            }

            {
                if (!this->password_ctx.run(this->buf)) {
                    return false;
                }
                this->password_ctx.restart();

                char key[12] = {};

                // key is simply password padded with nulls
                strncpy(key, this->password, 8);
                rfbDesKey(reinterpret_cast<unsigned char*>(key), EN0); // 0, encrypt
                auto const random_buf = this->password_ctx.server_random.data();
                rfbDes(random_buf, random_buf);
                rfbDes(random_buf + 8, random_buf + 8);

                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG(LOG_INFO, "Sending Password");
                }
                this->t.send(random_buf, 16);
            }
            this->state = WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM_RESPONSE;
            REDEMPTION_CXX_FALLTHROUGH;

        case WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM_RESPONSE:
            {
                // sec result
                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG(LOG_INFO, "Waiting for password ack");
                }

                if (!this->auth_response_ctx.run(buf, [this](bool status, byte_array bytes){
                    if (status) {
                        if (bool(this->verbose & VNCVerbose::basic_trace)) {
                            LOG(LOG_INFO, "vnc password ok\n");
                        }
                    }
                    else {
                        LOG(LOG_INFO, "vnc password failed. Reason: %.*s",
                            int(bytes.size()), bytes.to_charp());

                        if (this->allow_authentification_retries)
                        {
                            this->t.disconnect();

                            this->state = ASK_PASSWORD;
                            this->event.set_trigger_time(wait_obj::NOW);
                        }
                        else
                        {
                            throw Error(ERR_VNC_CONNECTION_ERROR);
                        }
                    }
                })) {
                    return false;
                }
                this->auth_response_ctx.restart();

                this->state = SERVER_INIT;
            }
            return true;

        case WAIT_SECURITY_TYPES_MS_LOGON:
            {
                LOG(LOG_INFO, "VNC MS-LOGON Auth");

                if (!this->ms_logon(buf)) {
                    return false;
                }
                this->ms_logon_ctx.restart();
            }
            this->state = WAIT_SECURITY_TYPES_MS_LOGON_RESPONSE;
            REDEMPTION_CXX_FALLTHROUGH;

        case WAIT_SECURITY_TYPES_MS_LOGON_RESPONSE:
            {
                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG(LOG_INFO, "Waiting for password ack");
                }

                if (!this->auth_response_ctx.run(buf, [this](bool status, byte_array bytes){
                    if (status) {
                        if (bool(this->verbose & VNCVerbose::basic_trace)) {
                            LOG(LOG_INFO, "MS LOGON password ok\n");
                        }
                    }
                    else {
                        LOG(LOG_INFO, "MS LOGON password FAILED. Reason: %.*s",
                            int(bytes.size()), bytes.to_charp());
                    }
                })) {
                    return false;
                }
                this->auth_response_ctx.restart();
            }
            this->state = SERVER_INIT;
            return true;

        case WAIT_SECURITY_TYPES_INVALID_AUTH:
            {
                LOG(LOG_INFO, "VNC INVALID Auth");

                if (!this->invalid_auth_ctx.run(buf, [](array_view_u8 av){
                    hexdump_c(av.data(), av.size());
                })) {
                    return false;
                }
                this->invalid_auth_ctx.restart();

                throw Error(ERR_VNC_CONNECTION_ERROR);
                // return true;
            }

        case SERVER_INIT:
            this->t.send("\x01", 1); // share flag
            this->state = SERVER_INIT_RESPONSE;
            REDEMPTION_CXX_FALLTHROUGH;

        case SERVER_INIT_RESPONSE:
            {
                if (!this->server_init_ctx.run(buf, *this)) {
                    return false;
                }
                this->server_init_ctx.restart();
            }

            // should be connected

            {
            // 7.4.1   SetPixelFormat
            // ----------------------
            // Sets the format in which pixel values should be sent in
            // FramebufferUpdate messages. If the client does not send
            // a SetPixelFormat message then the server sends pixel values
            // in its natural format as specified in the ServerInit message
            // (ServerInit).

            // If true-colour-flag is zero (false) then this indicates that
            // a "colour map" is to be used. The server can set any of the
            // entries in the colour map using the SetColourMapEntries
            // message (SetColourMapEntries). Immediately after the client
            // has sent this message the colour map is empty, even if
            // entries had previously been set by the server.

            // Note that a client must not have an outstanding
            // FramebufferUpdateRequest when it sends SetPixelFormat
            // as it would be impossible to determine if the next *
            // FramebufferUpdate is using the new or the previous pixel
            // format.

                StaticOutStream<20> stream;
                // Set Pixel format
                stream.out_uint8(0);

                // Padding 3 bytes
                stream.out_uint8(0);
                stream.out_uint8(0);
                stream.out_uint8(0);

                // VNC pixel_format capabilities
                // -----------------------------
                // bits per pixel  : 1 byte
                // color depth     : 1 byte
                // endianess       : 1 byte (0 = LE, 1 = BE)
                // true color flag : 1 byte
                // red max         : 2 bytes
                // green max       : 2 bytes
                // blue max        : 2 bytes
                // red shift       : 1 bytes
                // green shift     : 1 bytes
                // blue shift      : 1 bytes
                // padding         : 3 bytes

                // 8 bpp
                // -----
                // "\x08\x08\x00"
                // "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                // "\0\0\0"

                // 15 bpp
                // ------
                // "\x10\x0F\x00"
                // "\x01\x00\x1F\x00\x1F\x00\x1F\x0A\x05\x00"
                // "\0\0\0"

                // 24 bpp
                // ------
                // "\x20\x18\x00"
                // "\x01\x00\xFF\x00\xFF\x00\xFF\x10\x08\x00"
                // "\0\0\0"

                // 16 bpp
                // ------
                // "\x10\x10\x00"
                // "\x01\x00\x1F\x00\x2F\x00\x1F\x0B\x05\x00"
                // "\0\0\0"

                const char * pixel_format =
                    "\x10" // bits per pixel  : 1 byte =  16
                    "\x10" // color depth     : 1 byte =  16
                    "\x00" // endianess       : 1 byte =  LE
                    "\x01" // true color flag : 1 byte = yes
                    "\x00\x1F" // red max     : 2 bytes = 31
                    "\x00\x3F" // green max   : 2 bytes = 63
                    "\x00\x1F" // blue max    : 2 bytes = 31
                    "\x0B" // red shift       : 1 bytes = 11
                    "\x05" // green shift     : 1 bytes =  5
                    "\x00" // blue shift      : 1 bytes =  0
                    "\0\0\0"; // padding      : 3 bytes
                stream.out_copy_bytes(pixel_format, 16);
                this->t.send(stream.get_data(), stream.get_offset());

                this->bpp = 16;
                this->depth  = 16;
                this->endianess = 0;
                this->true_color_flag = 1;
                this->red_max       = 0x1F;
                this->green_max     = 0x3F;
                this->blue_max      = 0x1F;
                this->red_shift     = 0x0B;
                this->green_shift   = 0x05;
                this->blue_shift    = 0;
            }

            // 7.4.2   SetEncodings
            // --------------------

            // Sets the encoding types in which pixel data can be sent by
            // the server. The order of the encoding types given in this
            // message is a hint by the client as to its preference (the
            // first encoding specified being most preferred). The server
            // may or may not choose to make use of this hint. Pixel data
            // may always be sent in raw encoding even if not specified
            // explicitly here.

            // In addition to genuine encodings, a client can request
            // "pseudo-encodings" to declare to the server that it supports
            // certain extensions to the protocol. A server which does not
            // support the extension will simply ignore the pseudo-encoding.
            // Note that this means the client must assume that the server
            // does not support the extension until it gets some extension-
            // -specific confirmation from the server.
            {
                const char * encodings           = this->encodings.c_str();
                uint16_t     number_of_encodings = 0;

                // SetEncodings
                StaticOutStream<32768> stream;
                stream.out_uint8(2);
                stream.out_uint8(0);

                uint32_t number_of_encodings_offset = stream.get_offset();
                stream.out_clear_bytes(2);

                this->fill_encoding_types_buffer(encodings, stream, number_of_encodings, this->verbose);

                if (!number_of_encodings)
                {
                    if (bool(this->verbose & VNCVerbose::basic_trace)) {
                        LOG(LOG_WARNING, "mdo_vnc: using default encoding types - RRE(2),Raw(0),CopyRect(1),Cursor pseudo-encoding(-239)");
                    }

                    stream.out_uint32_be(0);            // raw
                    stream.out_uint32_be(1);            // copy rect
                    stream.out_uint32_be(2);            // RRE
                    stream.out_uint32_be(0xffffff11);   // (-239) cursor
                    number_of_encodings = 4;
                }

                stream.set_out_uint16_be(number_of_encodings, number_of_encodings_offset);
                this->t.send(stream.get_data(), 4 + number_of_encodings * 4);
            }

            switch (this->front.server_resize(this->width, this->height, this->bpp)){
            case FrontAPI::ResizeResult::instant_done:
                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG(LOG_INFO, "no resizing needed");
                }
                // no resizing needed
                this->state = DO_INITIAL_CLEAR_SCREEN;
                this->event.set_trigger_time(wait_obj::NOW);
                break;
            case FrontAPI::ResizeResult::remoteapp:
                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG(LOG_INFO, "resizing remoteapp");
                }
                if (this->client_execute) {
                    this->client_execute->adjust_window_to_mod();
                }
                // RZ: Continue with FrontAPI::ResizeResult::no_need
                REDEMPTION_CXX_FALLTHROUGH;
            case FrontAPI::ResizeResult::no_need:
                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG(LOG_INFO, "no resizing needed");
                }
                // no resizing needed
                this->state = DO_INITIAL_CLEAR_SCREEN;
                this->event.set_trigger_time(wait_obj::NOW);
                break;
            case FrontAPI::ResizeResult::done:
                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG(LOG_INFO, "resizing done");
                }
                // resizing done
                this->front_width  = this->width;
                this->front_height = this->height;

                this->state = WAIT_CLIENT_UP_AND_RUNNING;
                this->event.set_trigger_time(wait_obj::NOW);
                break;
            case FrontAPI::ResizeResult::fail:
                // resizing failed
                // thow an Error ?
                LOG(LOG_WARNING, "Older RDP client can't resize to server asked resolution, disconnecting");
                throw Error(ERR_VNC_OLDER_RDP_CLIENT_CANT_RESIZE);
            }
            return true;

        case WAIT_CLIENT_UP_AND_RUNNING:
            LOG(LOG_WARNING, "Waiting for client be come up and running");
            break;

        default:
            LOG(LOG_ERR, "Unknown state=%d", this->state);
            throw Error(ERR_VNC);
        }

        return false;
    } // draw_event_impl

private:
    void check_timeout()
    {
        if (this->event.is_waked_up_by_time()) {
            this->event.reset_trigger_time();

            if (this->clipboard_requesting_for_data_is_delayed) {
                //const uint64_t usnow = ustime();
                //const uint64_t timeval_diff = usnow - this->clipboard_last_client_data_timestamp;
                //LOG(LOG_INFO,
                //    "usnow=%llu clipboard_last_client_data_timestamp=%llu timeval_diff=%llu",
                //    usnow, this->clipboard_last_client_data_timestamp, timeval_diff);
                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG(LOG_INFO,
                        "mod_vnc server clipboard PDU: msgType=CB_FORMAT_DATA_REQUEST(%u) (time)",
                        RDPECLIP::CB_FORMAT_DATA_REQUEST);
                }

                // Build and send a CB_FORMAT_DATA_REQUEST to front (for format CF_TEXT or CF_UNICODETEXT)
                // 04 00 00 00 04 00 00 00 0? 00 00 00
                // 00 00 00 00
                RDPECLIP::FormatDataRequestPDU format_data_request_pdu(this->clipboard_requested_format_id);
                StaticOutStream<256>           out_s;

                format_data_request_pdu.emit(out_s);

                size_t length     = out_s.get_offset();
                size_t chunk_size = length;

                this->clipboard_requesting_for_data_is_delayed = false;
                this->send_to_front_channel( channel_names::cliprdr
                                           , out_s.get_data()
                                           , length
                                           , chunk_size
                                           , CHANNELS::CHANNEL_FLAG_FIRST
                                           | CHANNELS::CHANNEL_FLAG_LAST
                                           );
            }
        }
    }

private:
    struct FrameBufferUpdateCtx
    {
        enum class State
        {
            Header,
            Encoding,
            Data,
            RreData,
        };

        using Result = BasicResult<State>;

        FrameBufferUpdateCtx(Zdecompressor<> & zd, VNCVerbose verbose)
          : zd{zd}
          , verbose(verbose)
        {
//            REDEMPTION_DIAGNOSTIC_PUSH
//            REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
////            if (inflateInit(&this->zstrm) != Z_OK)
//            REDEMPTION_DIAGNOSTIC_POP
//            {
//                LOG(LOG_ERR, "vnc zlib initialization failed");
//                throw Error(ERR_VNC_ZLIB_INITIALIZATION);
//            }
        }

        ~FrameBufferUpdateCtx()
        {
//            inflateEnd(&this->zstrm);
        }

        void start(uint8_t bpp, uint8_t Bpp)
        {
            this->bpp = bpp;
            this->Bpp = Bpp;
            this->state = State::Header;
        }

        bool run(Buf64k & buf, mod_vnc & vnc, gdi::GraphicApi & drawable)
        {
            Result r = Result::fail();

            for (;;) {
                switch (this->state)
                {
                case State::Header:
                    r = this->read_header(buf); 
                break;
                case State::Encoding:
                    if (0 == this->num_recs) {
                        return true;
                    }
                    {
                        const size_t sz = 12;

                        if (buf.remaining() < sz)
                        {
                            r = Result::fail();
                        }
                        else {
                            InStream stream(buf.av(sz));
                            this->x = stream.in_uint16_be();
                            this->y = stream.in_uint16_be();
                            this->cx = stream.in_uint16_be();
                            this->cy = stream.in_uint16_be();
                            this->encoding = stream.in_sint32_be();

                            LOG(LOG_INFO, "Encoding: %u (%u, %u, %u, %u) : %d", this->num_recs, this->x, this->y, this->cx, this->cy, this->encoding);
//                            hexdump_d(buf.av(sz).data(), sz);

                            --this->num_recs;
                            switch (this->encoding){
                            default:
                            break;
                            case RAW_ENCODING:  /* raw */
                                this->encoder = new VNC::Encoder::Raw(this->bpp, this->Bpp, this->x, this->y, this->cx, this->cy, VNCVerbose::basic_trace);
                            break;
                            case ZRLE_ENCODING: /* ZRLE */
                                this->encoder = new VNC::Encoder::Zrle(this->bpp, this->Bpp, this->x, this->y, this->cx, this->cy, this->zd, VNCVerbose::basic_trace);
                            break;
                            }
                            buf.advance(sz);

                            // TODO see why we get these empty rects ?
                            // return State::Encoding;
                            r = Result::ok(State::Data);
                        }
                    }
                    break;
                case State::Data:

                    switch (this->encoding)
                    {
                    case COPYRECT_ENCODING:  /* copy rect */ 
                        LOG(LOG_INFO, "COPYRECT_ENCODING");
                        r = this->read_data_copy_rect(buf, vnc, drawable); 
                    break;
                    case RRE_ENCODING:  /* RRE */
                        LOG(LOG_INFO, "RRE_ENCODING");
                        r = this->read_data_rre(buf); 
                    break;
                    case RAW_ENCODING:  /* raw */
                    case ZRLE_ENCODING: /* ZRLE */
                    {
                        LOG(LOG_INFO, "%s", (this->encoding == RAW_ENCODING) ? "RAW_ENCODING" : "ZRLE_ENCODING");
                        // Pre Assertion: we have an encoder
                        if (encoder->consume(buf, drawable)){
                            // consume returns true if encoder is finished (ready to be resetted)
                            r = Result::ok(State::Encoding);
                            delete encoder;
                            encoder = nullptr;
                            break;
                        }
                        r = Result::fail();
                    }
                    break;
                    case CURSOR_PSEUDO_ENCODING: /* (-239) cursor */
                        LOG(LOG_INFO, "CURSOR_PSEUDO_ENCODING");
                        r = this->read_data_cursor(buf, vnc, drawable); 
                    break;
                    case HEXTILE_ENCODING: /* Hextile */ // TODO unimplemented
                        LOG(LOG_INFO, "HEXTILE_ENCODING");
                        LOG(LOG_INFO,
                            "VNC Encoding: Hextile, Bpp = %u, x=%u, y=%u, cx=%u, cy=%u",
                            this->Bpp, this->x, this->y, this->cx, this->cy);
                        r = Result::ok(State::Encoding);
                        break;
                    default:
                        LOG(LOG_ERR, "unexpected encoding %d in lib_frame_buffer", encoding);
                        throw Error(ERR_VNC_UNEXPECTED_ENCODING_IN_LIB_FRAME_BUFFER);
                    }
                    break;
                break;
                case State::RreData: r = this->read_data_rre_data(buf, vnc, drawable); break;
                }

                if (!r) {
                    return false;
                }
                this->state = r;
            }
        }

    private:
        uint8_t bpp;
        uint8_t Bpp;

        State state;

        uint16_t num_recs;

        uint16_t x;
        uint16_t y;
        uint16_t cx;
        uint16_t cy;
        int32_t encoding;

        std::vector<uint8_t> accumulator;
        std::vector<uint8_t> accumulator_uncompressed;
        uint32_t zlib_compressed_data_length;
        
        VNC::Encoder::EncoderApi * encoder = nullptr;


        uint32_t number_of_subrectangles_remain;
        std::unique_ptr<uint8_t[]> rre_raw;

        Zdecompressor<> & zd;

        VNCVerbose verbose;

        Result read_header(Buf64k & buf) noexcept
        {
            const size_t sz = 3;

            if (buf.remaining() < sz)
            {
                return Result::fail();
            }

            InStream stream(buf.av(sz));
            stream.in_skip_bytes(1);
            this->num_recs = stream.in_uint16_be();

            LOG(LOG_INFO, "FrameBufferUpdate: HEADER (%u)", this->num_recs);
//            hexdump_d(buf.av(sz).data(), sz);

            buf.advance(sz);
            return Result::ok(State::Encoding);
        }


//  7.5.1   FramebufferUpdate (part 2 : rectangles)
// ----------------------------------

//  FrameBufferUpdate message is followed by number-of-rectangles 
// of pixel data. Each rectangle consists of:

//     No. of bytes | Type   |  Description
// ---------------------------------------------
//         2        |  U16   |  x-position
//         2        |  U16   |  y-position
//         2        |  U16   |  width
//         2        |  U16   |  height
//         4        |  S32   |  encoding-type

//  followed by the pixel data in the specified encoding. See Encodings for the format of the data for each encoding
// and Pseudo-encodings for the meaning of pseudo-encodings.

// Note that a framebuffer update marks a transition from one valid framebuffer state to another. That
// means that a single update handles all received FramebufferUpdateRequest up to the point where th
// e update is sent out.

// However, because there is no strong connection between a FramebufferUpdateRequest and a subsequent 
// FramebufferUpdate, a client that has more than one FramebufferUpdateRequest pending at any given 
// time cannot be sure that it has received all framebuffer updates.


        Result read_encoding(Buf64k & buf) noexcept
        {

            const size_t sz = 12;

            if (buf.remaining() < sz)
            {
                return Result::fail();
            }

            InStream stream(buf.av(sz));
            this->x = stream.in_uint16_be();
            this->y = stream.in_uint16_be();
            this->cx = stream.in_uint16_be();
            this->cy = stream.in_uint16_be();
            this->encoding = stream.in_sint32_be();

            LOG(LOG_INFO, "Encoding: %u (%u, %u, %u, %u) : %d", this->num_recs, this->x, this->y, this->cx, this->cy, this->encoding);
//            hexdump_d(buf.av(sz).data(), sz);

            --this->num_recs;
            buf.advance(sz);

            // TODO see why we get these empty rects ?
            // return State::Encoding;
            return Result::ok(State::Data);
        }

        Result read_data_copy_rect(Buf64k & buf, mod_vnc & vnc, gdi::GraphicApi & drawable)
        {
            const size_t sz = 4;

            if (buf.remaining() < sz)
            {
                return Result::fail();
            }

            InStream stream_copy_rect(buf.av(sz));
            uint16_t const srcx = stream_copy_rect.in_uint16_be();
            uint16_t const srcy = stream_copy_rect.in_uint16_be();

            buf.advance(sz);

            update_lock<gdi::GraphicApi> lock(drawable);
            drawable.draw(
                RDPScrBlt(Rect(this->x, this->y, this->cx, this->cy), 0xCC, srcx, srcy),
                Rect(0, 0, vnc.front_width, vnc.front_height)
            );

            return Result::ok(State::Encoding);
        }

        Result read_data_rre(Buf64k & buf)
        {
            const size_t sz = 4 + this->Bpp;

            if (buf.remaining() < sz)
            {
                return Result::fail();
            }

            InStream stream_rre(buf.av(sz));
            this->number_of_subrectangles_remain = stream_rre.in_uint32_be();

            //LOG(LOG_INFO, "VNC Encoding: RRE, Bpp = %u, x=%u, y=%u, cx=%u, cy=%u", Bpp, x, y, cx, cy);
            this->rre_raw.reset(new(std::nothrow) uint8_t[cx * cy * Bpp]);
            if (!this->rre_raw) {
                LOG(LOG_ERR, "Memory allocation failed for RRE buffer in VNC");
                throw Error(ERR_VNC_MEMORY_ALLOCATION_FAILED);
            }

            for (uint8_t * point_cur = this->rre_raw.get(), * point_end = point_cur + cx * cy * Bpp;
                    point_cur < point_end; point_cur += Bpp) {
                memcpy(point_cur, stream_rre.get_current(), Bpp);
            }

            buf.advance(sz);

            return Result::ok(State::RreData);
        }

        Result read_data_rre_data(Buf64k & buf, mod_vnc & vnc, gdi::GraphicApi & drawable)
        {
            if (!this->number_of_subrectangles_remain) {
                // TODO use MultiRect
                update_lock<gdi::GraphicApi> lock(drawable);
                vnc.draw_tile(Rect(this->x, this->y, this->cx, this->cy), this->rre_raw.get(), drawable);
                this->rre_raw.reset();
                return Result::ok(State::Encoding);
            }

            const size_t sz = 8 + this->Bpp;

            if (buf.remaining() < sz)
            {
                return Result::fail();
            }

            --this->number_of_subrectangles_remain;

            InStream subrectangles(buf.av(sz));
            auto bytes_per_pixel = subrectangles.get_current();
            subrectangles.in_skip_bytes(Bpp);
            auto subrec_x        = subrectangles.in_uint16_be();
            auto subrec_y        = subrectangles.in_uint16_be();
            auto subrec_width    = subrectangles.in_uint16_be();
            auto subrec_height   = subrectangles.in_uint16_be();

            auto ling_boundary = cx * Bpp;
            auto point_line_cur = this->rre_raw.get() + subrec_y * ling_boundary;
            auto point_line_end = point_line_cur + subrec_height * ling_boundary;
            for (; point_line_cur < point_line_end; point_line_cur += ling_boundary) {
                for (uint8_t * point_cur = point_line_cur + subrec_x * Bpp,
                        * point_end = point_cur + subrec_width * Bpp;
                        point_cur < point_end; point_cur += Bpp) {
                    memcpy(point_cur, bytes_per_pixel, Bpp);
                }
            }

            buf.advance(sz);

            return Result::ok(State::RreData);
        }

        Result read_data_cursor(Buf64k & buf, mod_vnc & vnc, gdi::GraphicApi & drawable)
        {
            // TODO see why we get these empty rects ?
            if (this->cx <= 0 && this->cy <= 0) {
                return Result::ok(State::Encoding);
            }

            // 7.7.2   Cursor Pseudo-encoding
            // ------------------------------

            // A client which requests the Cursor pseudo-encoding is
            // declaring that it is capable of drawing a mouse cursor
            // locally. This can significantly improve perceived performance
            // over slow links.

            // The server sets the cursor shape by sending a pseudo-rectangle
            // with the Cursor pseudo-encoding as part of an update.

            // x, y : The pseudo-rectangle's x-position and y-position
            // indicate the hotspot of the cursor,

            // cx, cy : width and height indicate the width and height of
            // the cursor in pixels.

            // The data consists of width * height pixel values followed by
            // a bitmask.

            // PIXEL array : width * height * bytesPerPixel
            // bitmask     : floor((width + 7) / 8) * height

            // The bitmask consists of left-to-right, top-to-bottom
            // scanlines, where each scanline is padded to a whole number of
            // bytes. Within each byte the most significant bit represents
            // the leftmost pixel, with a 1-bit meaning the corresponding
            // pixel in the cursor is valid.

            const int sz_pixel_array = this->cx * this->cy * this->Bpp;
            const int sz_bitmask = nbbytes(this->cx) * this->cy;

            if (sz_pixel_array + sz_bitmask > 65536)
            {
                LOG(LOG_ERR,
                    "VNC Encoding: Cursor, data buffer too small (65536 < %d)",
                    sz_pixel_array + sz_bitmask);
                throw Error(ERR_BUFFER_TOO_SMALL);
            }

            if (buf.remaining() < sz_pixel_array + sz_bitmask)
            {
                return Result::fail();
            }

            auto cursor_buf = buf.av(sz_pixel_array + sz_bitmask).data();
            const uint8_t * vnc_pointer_data = cursor_buf;
            const uint8_t * vnc_pointer_mask = cursor_buf + sz_pixel_array;

            Pointer cursor;
            //LOG(LOG_INFO, "Cursor x=%u y=%u", x, y);
            cursor.x = x;
            cursor.y = y;
            // cursor.bpp = 24;
            cursor.width = 32;
            cursor.height = 32;
            // a VNC pointer of 1x1 size is not visible, so a default minimal pointer (dot pointer) is provided instead
            if (cx == 1 && cy == 1) {
                // TODO Appearence of this 1x1 cursor looks broken, check what we actually get
                memset(cursor.data, 0, sizeof(cursor.data));
                cursor.data[2883] = 0xFF;
                cursor.data[2884] = 0xFF;
                cursor.data[2885] = 0xFF;
                memset(cursor.mask, 0xFF, sizeof(cursor.mask));
                cursor.mask[116] = 0x1F;
                cursor.mask[120] = 0x1F;
                cursor.mask[124] = 0x1F;
            }
            else {
                // clear target cursor mask
                for (size_t tmpy = 0; tmpy < 32; tmpy++) {
                    for (size_t mask_x = 0; mask_x < nbbytes(32); mask_x++) {
                        cursor.mask[tmpy*nbbytes(32) + mask_x] = 0xFF;
                    }
                }
                // TODO The code below is likely to explain the yellow pointer: we ask for 16 bits for VNC, but we work with cursor as if it were 24 bits. We should use decode primitives and reencode it appropriately. Cursor has the right shape because the mask used is 1 bit per pixel arrays
                // copy vnc pointer and mask to rdp pointer and mask

                for (int yy = 0; yy < cy; yy++) {
                    for (int xx = 0 ; xx < cx ; xx++){
                        if (vnc_pointer_mask[yy * nbbytes(cx) + xx / 8 ] & (0x80 >> (xx&7))){
                            if ((yy < 32) && (xx < 32)){
                                cursor.mask[(31-yy) * nbbytes(32) + (xx / 8)] &= ~(0x80 >> (xx&7));
                                int pixel = 0;
                                for (int tt = 0 ; tt < Bpp; tt++){
                                    pixel += vnc_pointer_data[(yy * cx + xx) * Bpp + tt] << (8 * tt);
                                }
                                // TODO temporary: force black cursor
                                int red   = (pixel >> vnc.red_shift) & vnc.red_max;
                                int green = (pixel >> vnc.green_shift) & vnc.green_max;
                                int blue  = (pixel >> vnc.blue_shift) & vnc.blue_max;
                                cursor.data[((31-yy) * 32 + xx) * 3 + 0] = (red << 3) | (red >> 2);
                                cursor.data[((31-yy) * 32 + xx) * 3 + 1] = (green << 2) | (green >> 4);
                                cursor.data[((31-yy) * 32 + xx) * 3 + 2] = (blue << 3) | (blue >> 2);
                            }
                        }
                    }
                }
                /* keep these in 32x32, vnc cursor can be alot bigger */
                /* (anyway hotspot is usually 0, 0)                   */
                //if (x > 31) { x = 31; }
                //if (y > 31) { y = 31; }
            }
            cursor.update_bw();
            // TODO we should manage cursors bigger then 32 x 32  this is not an RDP protocol limitation
            drawable.begin_update();
            drawable.set_pointer(cursor);
            drawable.end_update();

            buf.advance(sz_pixel_array + sz_bitmask);
            return Result::ok(State::Encoding);
        }
    };
    FrameBufferUpdateCtx frame_buffer_update_ctx;

    bool lib_frame_buffer_update(gdi::GraphicApi & drawable, Buf64k & buf)
    {
        if (!this->frame_buffer_update_ctx.run(buf, *this, drawable)) {
            return false;
        }

        this->update_screen(Rect(0, 0, this->width, this->height));
        return true;
    } // lib_frame_buffer_update

    class PaletteUpdateCtx
    {
        enum class State
        {
            Header,
            Data,
            SkipData,
        };

        using Result = BasicResult<State>;

    public:
        void start()
        {
            this->state = State::Header;
            this->num_colors = 1; // sentinel
        }

        bool run(Buf64k & buf) noexcept
        {
            for (;;) {
                Result r = [this, &buf]{
                    switch (this->state) {
                        case State::Header:   return this->read_header(buf);
                        case State::Data:     return this->read_data(buf);
                        case State::SkipData: return this->skip_data(buf);
                    }
                    REDEMPTION_UNREACHABLE();
                }();

                if (!r) {
                    return false;
                }
                if (0 == this->num_colors) {
                    return true;
                }
                this->state = r;
            }
        }

        BGRPalette const & get_palette() const noexcept
        {
            return this->palette;
        }

    private:
        State state;

        uint16_t first_color;
        uint16_t num_colors;

        BGRPalette palette = BGRPalette::classic_332();

        Result read_header(Buf64k & buf) noexcept
        {
            if (buf.remaining() < 4)
            {
                return Result::fail();
            }

            InStream stream(buf.av(4));
            stream.in_skip_bytes(1);
            this->first_color = stream.in_uint16_be();
            this->num_colors = stream.in_uint16_be();

            buf.advance(4);

            if (this->first_color + this->num_colors > 256) {
                LOG(LOG_ERR, "VNC: number of palette colors too large: %d\n",
                    this->num_colors);
                return Result::ok(State::SkipData);
            }

            return Result::ok(State::Data);
        }

        Result read_data(Buf64k & buf) noexcept
        {
            if (buf.remaining() < 6)
            {
                return Result::fail();
            }

            InStream stream(buf.av());
            uint16_t const n = std::min<uint16_t>(
                stream.get_capacity() / 6,
                this->num_colors
            );
            this->num_colors -= n;

            uint16_t const max = n + this->first_color;
            for (; this->first_color < max; ++this->first_color) {
                const int b = stream.in_uint16_be() >> 8;
                const int g = stream.in_uint16_be() >> 8;
                const int r = stream.in_uint16_be() >> 8;
                this->palette.set_color(this->first_color, BGRColor(b, g, r));
            }

            buf.advance(n * 6);

            return Result::ok(State::Data);
        }

        Result skip_data(Buf64k & buf) noexcept
        {
            auto const n = std::min(buf.remaining(), uint16_t(this->num_colors * 6));
            this->num_colors -= n / 6;
            buf.advance(n);
            return Result::ok(State::SkipData);
        }
    };
    PaletteUpdateCtx palette_update_ctx;

    bool lib_palette_update(gdi::GraphicApi & drawable, Buf64k & buf)
    {
        if (!this->palette_update_ctx.run(buf)) {
            return false;
        }

        this->front.set_palette(this->palette_update_ctx.get_palette());
        this->front.begin_update();
        drawable.draw(RDPColCache(0, this->palette_update_ctx.get_palette()));
        this->front.end_update();

        return true;
    } // lib_palette_update

    /******************************************************************************/
    void lib_open_clip_channel(void) {
        const CHANNELS::ChannelDef * channel = this->get_channel_by_name(channel_names::cliprdr);

        if (channel) {
            // Monitor ready PDU send to front
            RDPECLIP::ServerMonitorReadyPDU server_monitor_ready_pdu;
            StaticOutStream<64>             out_s;

/*
            //- Beginning of clipboard PDU Header ----------------------------
            out_s.out_uint16_le(1); // MSG Type 2 bytes
            out_s.out_uint16_le(0); // MSG flags 2 bytes
            out_s.out_uint32_le(0); // Datalen of the rest of the message
            //- End of clipboard PDU Header ----------------------------------
            //- Beginning of Monitor Ready PDU payload ----------------------------
            //- End of Monitor Ready PDU payload -------------------------------
            out_s.out_clear_bytes(4);
            out_s.mark_end();
*/

            server_monitor_ready_pdu.emit(out_s);

            size_t length     = out_s.get_offset();
            size_t chunk_size = length;

            this->send_to_front_channel( channel_names::cliprdr
                                       , out_s.get_data()
                                       , length
                                       , chunk_size
                                       ,   CHANNELS::CHANNEL_FLAG_FIRST
                                         | CHANNELS::CHANNEL_FLAG_LAST
                                       );
        }
        else {
            LOG(LOG_INFO, "Clipboard Channel Redirection unavailable");
        }
    } // lib_open_clip_channel

    //==============================================================================================================
    const CHANNELS::ChannelDef * get_channel_by_name(CHANNELS::ChannelNameId channel_name) const {
    //==============================================================================================================
        return this->front.get_channel_list().get_by_name(channel_name);
    } // get_channel_by_name

    class ClipboardDataCtx
    {
        enum class State
        {
            Header,
            Data,
            SkipData,
        };

        using Result = BasicResult<State>;

    public:
        explicit ClipboardDataCtx(VNCVerbose verbose)
          : verbose(verbose)
          , to_rdp_clipboard_data(this->to_rdp_clipboard_data_buffer)
          , to_rdp_clipboard_data_is_utf8_encoded(false)
        {}

        void start(bool clipboard_down_is_really_enabled) noexcept
        {
            this->clipboard_down_is_really_enabled = clipboard_down_is_really_enabled;
            this->remaining_data_length = 1; // sentinel
            this->state = State::Header;
        }

        bool run(Buf64k & buf) noexcept
        {
            for (;;) {
                Result r = [this, &buf]{
                    switch (this->state) {
                        case State::Header:   return this->read_header(buf);
                        case State::Data:     return this->read_data(buf);
                        case State::SkipData: return this->skip_data(buf);
                    }
                    REDEMPTION_UNREACHABLE();
                }();

                if (!r) {
                    return false;
                }
                if (0 == this->remaining_data_length) {
                    return true;
                }
                this->state = r;
            }
        }

        bool clipboard_is_enabled() const noexcept
        {
            return this->clipboard_down_is_really_enabled;
        }

        array_view_const_u8 clipboard_data() const noexcept
        {
            return {
                this->to_rdp_clipboard_data.get_data(),
                this->to_rdp_clipboard_data.get_offset()
            };
        }

        bool clipboard_data_is_utf8_encoded() const noexcept
        {
            return this->to_rdp_clipboard_data_is_utf8_encoded;
        }

    private:
        State    state;
        VNCVerbose  verbose;

        bool     clipboard_down_is_really_enabled;

        uint32_t data_length;
        uint32_t remaining_data_length;

        uint8_t  to_rdp_clipboard_data_buffer[MAX_CLIPBOARD_DATA_SIZE];
        InStream to_rdp_clipboard_data; // NOTE can be array_view

        bool     to_rdp_clipboard_data_is_utf8_encoded;

        Result read_header(Buf64k & buf) noexcept
        {
            if (buf.remaining() < 7) {
                return Result::fail();
            }

            InStream stream(buf.av(7));
            stream.in_skip_bytes(3);                   // padding(3)
            this->data_length = stream.in_uint32_be(); // length(4)
            this->remaining_data_length = this->data_length;
            buf.advance(7);

            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                LOG(LOG_INFO, "mod_vnc::lib_clip_data: clipboard_data_length=%u", this->data_length);
            }

            this->to_rdp_clipboard_data = InStream(this->to_rdp_clipboard_data_buffer);

            if (!clipboard_down_is_really_enabled) {
                return Result::ok(State::SkipData);
            }

            if (this->data_length < this->to_rdp_clipboard_data.get_capacity()) {
                return Result::ok(State::Data);
            }

            this->to_rdp_clipboard_data.in_skip_bytes(::snprintf(
                ::char_ptr_cast(this->to_rdp_clipboard_data_buffer),
                this->to_rdp_clipboard_data.get_capacity(),
                "The text was too long to fit in the clipboard buffer. "
                "The buffer size is limited to %zu bytes.",
                this->to_rdp_clipboard_data.get_capacity()
            ) + 1 /* Null character */);

            this->to_rdp_clipboard_data_is_utf8_encoded = true;

            return Result::ok(State::SkipData);
        }

        Result read_data(Buf64k & buf) noexcept
        {
            if (!buf.remaining()) {
                return Result::fail();
            }

            auto const av = buf.av(std::min<uint32_t>(
                this->remaining_data_length, buf.remaining()));

            auto const buf_pos = this->data_length - this->remaining_data_length;
            memcpy(this->to_rdp_clipboard_data_buffer + buf_pos, av.data(), av.size());
            this->remaining_data_length -= av.size();
            buf.advance(av.size());

            if (this->remaining_data_length) {
                return Result::ok(State::Data);
            }

            this->to_rdp_clipboard_data_buffer[this->data_length] = '\0';
            this->to_rdp_clipboard_data.in_skip_bytes(this->data_length);

            this->to_rdp_clipboard_data_is_utf8_encoded =
                ::is_utf8_string(this->to_rdp_clipboard_data.get_data(), this->data_length);

            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                LOG(LOG_INFO,
                    "mod_vnc::lib_clip_data: to_rdp_clipboard_data_is_utf8_encoded=%s",
                    (this->to_rdp_clipboard_data_is_utf8_encoded ? "yes" : "no"));
                if (this->data_length <= 64) {
//                    hexdump_c(this->to_rdp_clipboard_data.get_data(), this->data_length);
                }
            }

            return Result::ok(State::Header);
        }

        Result skip_data(Buf64k & buf) noexcept
        {
            if (!buf.remaining()) {
                return Result::fail();
            }

            const auto number_of_bytes_to_read =
                std::min<size_t>(this->remaining_data_length, buf.remaining());
            buf.advance(number_of_bytes_to_read);
            this->remaining_data_length -= number_of_bytes_to_read;
            return Result::ok(State::SkipData);
        }
    };
    ClipboardDataCtx clipboard_data_ctx;

    //******************************************************************************
    // Entry point for VNC server clipboard content reception
    // Conversion to RDP behaviour :
    //  - store this content in a buffer, waiting for an explicit request from the front
    //  - send a notification to the front (Format List PDU) that the server clipboard
    //    status has changed
    //******************************************************************************
    bool lib_clip_data(Buf64k & buf)
    {
        if (!this->clipboard_data_ctx.run(buf)) {
            return false;
        }

        if (this->clipboard_data_ctx.clipboard_is_enabled()) {
            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                LOG(LOG_INFO,
                    "mod_vnc::lib_clip_data: Sending Format List PDU (%u) to client.",
                    RDPECLIP::CB_FORMAT_LIST);
            }

            RDPECLIP::FormatListPDU format_list_pdu;
            StaticOutStream<256>    out_s;

            format_list_pdu.emit_ex(
                out_s,
                this->clipboard_data_ctx.clipboard_data_is_utf8_encoded()
            );

            size_t length     = out_s.get_offset();
            size_t chunk_size = std::min<size_t>(length, CHANNELS::CHANNEL_CHUNK_LENGTH);

            this->send_to_front_channel(
                channel_names::cliprdr,
                out_s.get_data(),
                length,
                chunk_size,
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST
            );

            this->clipboard_owned_by_client = false;

            // Can stop RDP to VNC clipboard infinite loop.
            this->clipboard_requesting_for_data_is_delayed = false;
        }
        else {
            LOG(LOG_INFO, "mod_vnc::lib_clip_data: Clipboard Channel Redirection unavailable");
        }

        return true;
    } // lib_clip_data

    void send_to_mod_channel(
        CHANNELS::ChannelNameId front_channel_name,
        InStream & chunk,
        size_t length,
        uint32_t flags
    ) override {
        if (bool(this->verbose & VNCVerbose::basic_trace)) {
            LOG(LOG_INFO, "mod_vnc::send_to_mod_channel");
        }

        if (this->state != UP_AND_RUNNING) {
            return;
        }

        if (front_channel_name == channel_names::cliprdr) {
            this->clipboard_send_to_vnc(chunk, length, flags);
        }
        if (bool(this->verbose & VNCVerbose::basic_trace)) {
            LOG(LOG_INFO, "mod_vnc::send_to_mod_channel done");
        }
    } // send_to_mod_channel

private:
    void clipboard_send_to_vnc(InStream & chunk, size_t length, uint32_t flags)
    {
        if (bool(this->verbose & VNCVerbose::basic_trace)) {
            LOG( LOG_INFO, "mod_vnc::clipboard_send_to_vnc:"
                           " length=%zu chunk_size=%zu flags=0x%08X"
               , length, chunk.get_capacity(), flags);
        }

        // TODO Create a unit tested class for clipboard messages

        /*
         * rdp message :
         *
         *   -------------------------------------------------------------------------------------------
         *                    rdesktop                    |             freerdp / mstsc
         *   -------------------------------------------------------------------------------------------
         *                                          serveur paste
         *   -------------------------------------------------------------------------------------------
         *    mod   -> front : (4) format_data_resquest   |   mod   -> front : (4) format_data_resquest
         *  _ front -> mod   : (5) format_data_response   |   front -> mod   : (5) format_data_response
         * |  front -> mod   : (2) format_list            |
         * |_ mod   -> front : (3) format_list_response   |
         *   -------------------------------------------------------------------------------------------
         *                                          serveur copy
         *   -------------------------------------------------------------------------------------------
         *    mod   -> front : (2) format_list            |   mod   -> front : (2) format_list
         *    front -> mod   : (3) format_list_response   |   front -> mod   : (3) format_list_response
         *    front -> mod   : (4) format_data_resquest   |   front -> mod   : (4) format_data_resquest
         *    mod   -> front : (5) format_data_response   |   mod   -> front : (5) format_data_response
         *    front -> mod   : (4) format_data_resquest   |
         *    mod   -> front : (5) format_data_response   |
         *   -------------------------------------------------------------------------------------------
         *                                            host copy
         *  _-------------------------------------------------------------------------------------------
         * |  front -> mod   : (2) format_list            |   front -> mod   : (2) format_list
         * |_ mod   -> front : (3) format_list_response   |   mod   -> front : (3) format_list_response
         *   -------------------------------------------------------------------------------------------
         *                                            host paste
         *   -------------------------------------------------------------------------------------------
         *    front -> mod   : (4) format_data_resquest   |   front -> mod   : (4) format_data_resquest
         *    mod   -> front : (5) format_data_response   |   mod   -> front : (5) format_data_response
         *   -------------------------------------------------------------------------------------------
         *
         * rdesktop : paste serveur = paste serveur + host copy
         */

        // specific treatement depending on msgType
        RDPECLIP::RecvPredictor rp(chunk);
        uint16_t msgType = rp.msgType;

        if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) == 0) {
            msgType = RDPECLIP::CB_CHUNKED_FORMAT_DATA_RESPONSE;
            // msgType read is not a msgType, it's a part of data.
        }

        if (bool(this->verbose & VNCVerbose::basic_trace)) {
            LOG(LOG_INFO, "mod_vnc client clipboard PDU: msgType=%s(%d)",
                RDPECLIP::get_msgType_name(msgType), msgType);
        }

        switch (msgType) {
            case RDPECLIP::CB_FORMAT_LIST: {
                // Client notify that a copy operation have occured.
                // Two operations should be done :
                //  - Always: send a RDP acknowledge (CB_FORMAT_LIST_RESPONSE)
                //  - Only if clipboard content formats list include "UNICODETEXT:
                // send a request for it in that format
                RDPECLIP::FormatListPDU format_list_pdu;

                if (!this->client_use_long_format_names) {
                    format_list_pdu.recv(chunk);
                }
                else {
                    format_list_pdu.recv_long(chunk);
                }

                //---- Beginning of clipboard PDU Header ----------------------------

                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG(LOG_INFO, "mod_vnc server clipboard PDU: msgType=CB_FORMAT_LIST_RESPONSE(%u)",
                        RDPECLIP::CB_FORMAT_LIST_RESPONSE);
                }

                bool response_ok = true;

                // Build and send the CB_FORMAT_LIST_RESPONSE (with status = OK)
                // 03 00 01 00 00 00 00 00 00 00 00 00
                RDPECLIP::FormatListResponsePDU format_list_response_pdu(response_ok);
                StaticOutStream<256>            out_s;

                format_list_response_pdu.emit(out_s);

                size_t length     = out_s.get_offset();
                size_t chunk_size = length;

                this->send_to_front_channel( channel_names::cliprdr
                                           , out_s.get_data()
                                           , length
                                           , chunk_size
                                           ,   CHANNELS::CHANNEL_FLAG_FIRST
                                             | CHANNELS::CHANNEL_FLAG_LAST
                                           );

                using std::chrono::microseconds;
                constexpr microseconds MINIMUM_TIMEVAL(250000LL);

                if (this->enable_clipboard_up
                && (format_list_pdu.contains_data_in_text_format
                 || format_list_pdu.contains_data_in_unicodetext_format)) {
                    if (this->clipboard_server_encoding_type == ClipboardEncodingType::UTF8) {
                        this->clipboard_requested_format_id =
                            (format_list_pdu.contains_data_in_unicodetext_format ?
                             RDPECLIP::CF_UNICODETEXT : RDPECLIP::CF_TEXT);
                    }
                    else {
                        this->clipboard_requested_format_id =
                            (format_list_pdu.contains_data_in_text_format ?
                             RDPECLIP::CF_TEXT : RDPECLIP::CF_UNICODETEXT);
                    }

                    const microseconds usnow = ustime();
                    const microseconds timeval_diff = usnow - this->clipboard_last_client_data_timestamp;
                    //LOG(LOG_INFO,
                    //    "usnow=%llu clipboard_last_client_data_timestamp=%llu timeval_diff=%llu",
                    //    usnow, this->clipboard_last_client_data_timestamp, timeval_diff);
                    if ((timeval_diff > MINIMUM_TIMEVAL) || !this->clipboard_owned_by_client) {
                        if (bool(this->verbose & VNCVerbose::basic_trace)) {
                            LOG(LOG_INFO,
                                "mod_vnc server clipboard PDU: msgType=CB_FORMAT_DATA_REQUEST(%u)",
                                RDPECLIP::CB_FORMAT_DATA_REQUEST);
                        }

                        // Build and send a CB_FORMAT_DATA_REQUEST to front (for format CF_TEXT or CF_UNICODETEXT)
                        // 04 00 00 00 04 00 00 00 0? 00 00 00
                        // 00 00 00 00
                        RDPECLIP::FormatDataRequestPDU format_data_request_pdu(this->clipboard_requested_format_id);
                        out_s.rewind();

                        format_data_request_pdu.emit(out_s);

                        length     = out_s.get_offset();
                        chunk_size = length;

                        this->clipboard_requesting_for_data_is_delayed = false;

                        this->send_to_front_channel( channel_names::cliprdr
                                                   , out_s.get_data()
                                                   , length
                                                   , chunk_size
                                                   , CHANNELS::CHANNEL_FLAG_FIRST
                                                   | CHANNELS::CHANNEL_FLAG_LAST
                                                   );
                    }
                    else {
                        if (this->bogus_clipboard_infinite_loop == VncBogusClipboardInfiniteLoop::delayed) {
                            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                                LOG(LOG_INFO,
                                    "mod_vnc server clipboard PDU: msgType=CB_FORMAT_DATA_REQUEST(%u) (delayed)",
                                    RDPECLIP::CB_FORMAT_DATA_REQUEST);
                            }
                            this->event.set_trigger_time(MINIMUM_TIMEVAL - timeval_diff);

                            this->clipboard_requesting_for_data_is_delayed = true;
                        }
                        else if (this->bogus_clipboard_infinite_loop
                            != VncBogusClipboardInfiniteLoop::duplicated
                        && (this->clipboard_general_capability_flags
                            & RDPECLIP::CB_ALL_GENERAL_CAPABILITY_FLAGS)) {
                            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                                LOG( LOG_INFO
                                   , "mod_vnc::clipboard_send_to_vnc: "
                                     "duplicated clipboard update event "
                                     "from Windows client is ignored"
                                   );
                            }
                        }
                        else {
                            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                                LOG(LOG_INFO,
                                    "mod_vnc server clipboard PDU: msgType=CB_FORMAT_LIST(%u) (preventive)",
                                    RDPECLIP::CB_FORMAT_LIST);
                            }

                            RDPECLIP::FormatListPDU format_list_pdu;
                            StaticOutStream<256>    out_s;

                            const bool unicodetext = (this->clipboard_requested_format_id == RDPECLIP::CF_UNICODETEXT);

                            format_list_pdu.emit_ex(out_s, unicodetext);

                            size_t length     = out_s.get_offset();
                            size_t chunk_size = std::min<size_t>(length, CHANNELS::CHANNEL_CHUNK_LENGTH);

                            this->send_to_front_channel(channel_names::cliprdr,
                                                        out_s.get_data(),
                                                        length,
                                                        chunk_size,
                                                          CHANNELS::CHANNEL_FLAG_FIRST
                                                        | CHANNELS::CHANNEL_FLAG_LAST
                                                       );
                        }
                    }
                }
            }
            break;

            case RDPECLIP::CB_FORMAT_DATA_REQUEST: {
                const unsigned expected = 10; /* msgFlags(2) + datalen(4) + requestedFormatId(4) */
                if (!chunk.in_check_rem(expected)) {
                    LOG( LOG_ERR
                       , "mod_vnc::clipboard_send_to_vnc: truncated CB_FORMAT_DATA_REQUEST(%u) data, need=%u remains=%zu"
                       , RDPECLIP::CB_FORMAT_DATA_REQUEST, expected, chunk.in_remain());
                    throw Error(ERR_VNC);
                }

                // This is a fake treatment that pretends to send the Request
                //  to VNC server. Instead, the RDP PDU is handled localy and
                //  the clipboard PDU, if any, is likewise built localy and
                //  sent back to front.
                RDPECLIP::FormatDataRequestPDU format_data_request_pdu;

                // 04 00 00 00 04 00 00 00 0d 00 00 00 00 00 00 00
                format_data_request_pdu.recv(chunk);

                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG( LOG_INFO
                       , "mod_vnc::clipboard_send_to_vnc: CB_FORMAT_DATA_REQUEST(%u) msgFlags=0x%02x datalen=%u requestedFormatId=%s(%u)"
                       , RDPECLIP::CB_FORMAT_DATA_REQUEST
                       , format_data_request_pdu.header.msgFlags()
                       , format_data_request_pdu.header.dataLen()
                       , RDPECLIP::get_Format_name(format_data_request_pdu.requestedFormatId)
                       , format_data_request_pdu.requestedFormatId
                       );
                }

                auto send_format_data_response = [this] (OutStream & pdu_stream) {
                    size_t pdu_data_length           = pdu_stream.get_offset();
                    size_t remaining_pdu_data_length = pdu_data_length;

                    uint8_t * chunk_data = pdu_stream.get_data();

                    int send_flags =
                        (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);

                    do {
                        const size_t chunk_size = std::min<size_t>(
                                CHANNELS::CHANNEL_CHUNK_LENGTH,
                                remaining_pdu_data_length
                            );

                        remaining_pdu_data_length -= chunk_size;

                        if (!remaining_pdu_data_length) {
                            send_flags |= CHANNELS::CHANNEL_FLAG_LAST;
                        }

                        this->send_to_front_channel(channel_names::cliprdr,
                                                    chunk_data,
                                                    pdu_data_length,
                                                    chunk_size,
                                                    send_flags
                                                   );
                        if (bool(this->verbose & VNCVerbose::basic_trace)) {
                            LOG(LOG_INFO,
                                "mod_vnc server clipboard PDU: msgType=CB_FORMAT_DATA_RESPONSE(%u) - chunk_size=%zu",
                                RDPECLIP::CB_FORMAT_DATA_RESPONSE, chunk_size);
                        }

                        send_flags &= ~CHANNELS::CHANNEL_FLAG_FIRST;

                        chunk_data += chunk_size;
                    }
                    while (remaining_pdu_data_length);
                };

                if (this->bogus_clipboard_infinite_loop != VncBogusClipboardInfiniteLoop::delayed && this->clipboard_owned_by_client) {
                    StreamBufMaker<65536> buf_maker;
                    OutStream out_stream = buf_maker.reserve_out_stream(
                            8 +                                 // clipHeader(8)
                            this->to_vnc_clipboard_data.get_offset()  // data
                        );

                    const bool response_ok = true;
                    const RDPECLIP::FormatDataResponsePDU format_data_response_pdu(response_ok);

                    format_data_response_pdu.emit_ex(out_stream, this->to_vnc_clipboard_data.get_offset());
                    out_stream.out_copy_bytes(this->to_vnc_clipboard_data.get_data(), this->to_vnc_clipboard_data.get_offset());

                    send_format_data_response(out_stream);

                    break;
                }

                if ((format_data_request_pdu.requestedFormatId == RDPECLIP::CF_TEXT) &&
                    !this->clipboard_data_ctx.clipboard_data_is_utf8_encoded()) {
                    StreamBufMaker<65536> buf_maker;
                    OutStream out_stream = buf_maker.reserve_out_stream(
                            8 +                                         // clipHeader(8)
                            this->clipboard_data_ctx.clipboard_data().size() * 2 +    // data
                            1                                           // Null character
                        );

                    OutStream out_data_stream(
                        out_stream.get_data() + 8 /* clipHeader(8) */,
                        out_stream.get_capacity() - 8 /* clipHeader(8) */
                    );

                    const size_t to_rdp_clipboard_data_length =
                        ::linux_to_windows_newline_convert(
                            ::char_ptr_cast(this->clipboard_data_ctx.clipboard_data().data()),
                            this->clipboard_data_ctx.clipboard_data().size(),
                            ::char_ptr_cast(out_data_stream.get_data()),
                            out_data_stream.get_capacity()
                        );
                    out_data_stream.out_skip_bytes(to_rdp_clipboard_data_length);

                    const bool response_ok = true;
                    const RDPECLIP::FormatDataResponsePDU format_data_response_pdu(response_ok);

                    format_data_response_pdu.emit_ex(out_stream, out_data_stream.get_offset());
                    out_stream.out_skip_bytes(out_data_stream.get_offset());

                    send_format_data_response(out_stream);

                    if (bool(this->verbose & VNCVerbose::basic_trace)) {
                        LOG(LOG_INFO,
                            "mod_vnc::clipboard_send_to_vnc: "
                                "Sending Format Data Response PDU (CF_TEXT) done");
                    }
                }
                else if (format_data_request_pdu.requestedFormatId == RDPECLIP::CF_UNICODETEXT) {
                    StreamBufMaker<65536> buf_maker;
                    OutStream out_stream = buf_maker.reserve_out_stream(
                        8 +                                         // clipHeader(8)
                        this->clipboard_data_ctx.clipboard_data().size() * 4 +    // data
                        1                                           // Null character
                    );

                    OutStream out_data_stream(
                        out_stream.get_data() + 8 /* clipHeader(8) */,
                        out_stream.get_capacity() - 8 /* clipHeader(8) */
                    );

                    size_t utf16_data_length;
                    if (this->clipboard_data_ctx.clipboard_data_is_utf8_encoded()) {
                        utf16_data_length = UTF8toUTF16_CrLf(
                            this->clipboard_data_ctx.clipboard_data().data(),
                            out_data_stream.get_data(),
                            out_data_stream.get_capacity()
                        );
                    }
                    else {
                        utf16_data_length = Latin1toUTF16(
                            this->clipboard_data_ctx.clipboard_data().data(),
                            this->clipboard_data_ctx.clipboard_data().size(),
                            out_data_stream.get_data(),
                            out_data_stream.get_capacity()
                        );
                    }

                    out_data_stream.out_skip_bytes(utf16_data_length);
                    out_data_stream.out_uint16_le(0x0000);  // Null character

                    const bool response_ok = true;
                    const RDPECLIP::FormatDataResponsePDU format_data_response_pdu(response_ok);

                    format_data_response_pdu.emit_ex(out_stream, out_data_stream.get_offset());
                    out_stream.out_skip_bytes(out_data_stream.get_offset());

                    send_format_data_response(out_stream);

                    if (bool(this->verbose & VNCVerbose::basic_trace)) {
                        LOG(LOG_INFO,
                            "mod_vnc::clipboard_send_to_vnc: "
                                "Sending Format Data Response PDU (CF_UNICODETEXT) done");
                    }
                }
                else {
                    LOG( LOG_INFO
                       , "mod_vnc::clipboard_send_to_vnc: resquested clipboard format Id 0x%02x is not supported by VNC PROXY"
                       , format_data_request_pdu.requestedFormatId);
                }
            }
            break;

            case RDPECLIP::CB_FORMAT_DATA_RESPONSE: {
                RDPECLIP::FormatDataResponsePDU format_data_response_pdu;

                format_data_response_pdu.recv(chunk);

                if (format_data_response_pdu.header.msgFlags() == RDPECLIP::CB_RESPONSE_OK) {
                    if ((flags & CHANNELS::CHANNEL_FLAG_LAST) != 0) {
                        if (!chunk.in_check_rem(format_data_response_pdu.header.dataLen())) {
                            LOG( LOG_ERR
                               , "mod_vnc::clipboard_send_to_vnc: truncated CB_FORMAT_DATA_RESPONSE(%u), need=%u remains=%zu"
                               , RDPECLIP::CB_FORMAT_DATA_RESPONSE
                               , format_data_response_pdu.header.dataLen(), chunk.in_remain());
                            throw Error(ERR_VNC);
                        }

                        this->to_vnc_clipboard_data.rewind();

                        this->to_vnc_clipboard_data.out_copy_bytes(chunk.get_current(), format_data_response_pdu.header.dataLen());

                        this->rdp_input_clip_data(this->to_vnc_clipboard_data.get_data(),
                            this->to_vnc_clipboard_data.get_offset());
                    }
                    else {
                        // Virtual channel data span in multiple Virtual Channel PDUs.

                        if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) == 0) {
                            LOG(LOG_ERR, "mod_vnc::clipboard_send_to_vnc: flag CHANNEL_FLAG_FIRST expected");
                            throw Error(ERR_VNC);
                        }

                        this->to_vnc_clipboard_data_size      =
                        this->to_vnc_clipboard_data_remaining = format_data_response_pdu.header.dataLen();

                        if (bool(this->verbose & VNCVerbose::basic_trace)) {
                            LOG( LOG_INFO
                               , "mod_vnc::clipboard_send_to_vnc: virtual channel data span in multiple Virtual Channel PDUs: total=%u"
                               , this->to_vnc_clipboard_data_size);
                        }

                        this->to_vnc_clipboard_data.rewind();

                        if (this->to_vnc_clipboard_data.get_capacity() >= this->to_vnc_clipboard_data_size) {
                            uint32_t number_of_bytes_to_read = std::min<uint32_t>(
                                    this->to_vnc_clipboard_data_remaining,
                                    chunk.in_remain()
                                );
                            this->to_vnc_clipboard_data.out_copy_bytes(chunk.get_current(), number_of_bytes_to_read);

                            this->to_vnc_clipboard_data_remaining -= number_of_bytes_to_read;
                        }
                        else {
                                  char   * latin1_overflow_message_buffer        = ::char_ptr_cast(this->to_vnc_clipboard_data.get_data());
                            const size_t   latin1_overflow_message_buffer_length = this->to_vnc_clipboard_data.get_capacity();

                            const size_t latin1_overflow_message_length =
                                ::snprintf(latin1_overflow_message_buffer,
                                           latin1_overflow_message_buffer_length,
                                           "The data was too large to fit into the clipboard buffer. "
                                               "The buffer size is limited to %zu bytes. "
                                               "The length of data is %" PRIu32 " bytes.",
                                           this->to_vnc_clipboard_data.get_capacity(),
                                           this->to_vnc_clipboard_data_size);

                            this->to_vnc_clipboard_data.out_skip_bytes(latin1_overflow_message_length);
                            this->to_vnc_clipboard_data.out_clear_bytes(1); /* null-terminator */

                            this->clipboard_requested_format_id = RDPECLIP::CF_TEXT;
                        }
                    }
                }
            }
            break;

            case RDPECLIP::CB_CHUNKED_FORMAT_DATA_RESPONSE:
                assert(this->to_vnc_clipboard_data.get_offset() != 0);
                assert(this->to_vnc_clipboard_data_size);

                // Virtual channel data span in multiple Virtual Channel PDUs.
                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG(LOG_INFO, "mod_vnc::clipboard_send_to_vnc: an other trunk");
                }

                if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) != 0) {
                    LOG(LOG_ERR, "mod_vnc::clipboard_send_to_vnc: flag CHANNEL_FLAG_FIRST unexpected");
                    throw Error(ERR_VNC);
                }

                // if (bool(this->verbose & VNCVerbose::basic_trace)) {
                //     LOG( LOG_INFO, "mod_vnc::clipboard_send_to_vnc: trunk size=%u, capacity=%u"
                //        , chunk.in_remain(), static_cast<unsigned>(this->to_vnc_clipboard_data.tailroom()));
                // }

                if (this->to_vnc_clipboard_data.get_capacity() >= this->to_vnc_clipboard_data_size) {
                    uint32_t number_of_bytes_to_read = std::min<uint32_t>(
                            this->to_vnc_clipboard_data_remaining,
                            chunk.in_remain()
                        );

                    this->to_vnc_clipboard_data.out_copy_bytes(chunk.get_current(), number_of_bytes_to_read);

                    this->to_vnc_clipboard_data_remaining -= number_of_bytes_to_read;
                }

                if ((flags & CHANNELS::CHANNEL_FLAG_LAST) != 0) {
                    assert((this->to_vnc_clipboard_data.get_capacity() < this->to_vnc_clipboard_data_size) ||
                        !this->to_vnc_clipboard_data_remaining);

                    this->rdp_input_clip_data(this->to_vnc_clipboard_data.get_data(),
                        this->to_vnc_clipboard_data.get_offset());
                }
            break;

            case RDPECLIP::CB_CLIP_CAPS:
            {
                RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu;

                clipboard_caps_pdu.recv(chunk);

                RDPECLIP::CapabilitySetRecvFactory caps_recv_factory(chunk);

                if (caps_recv_factory.capabilitySetType == RDPECLIP::CB_CAPSTYPE_GENERAL) {
                    RDPECLIP::GeneralCapabilitySet general_caps;
                    general_caps.recv(chunk, caps_recv_factory);

                    this->clipboard_general_capability_flags = general_caps.generalFlags();

                    if (general_caps.generalFlags() & RDPECLIP::CB_USE_LONG_FORMAT_NAMES) {
                        this->client_use_long_format_names = true;
                    }
                    if (bool(this->verbose & VNCVerbose::basic_trace)) {
                        LOG(LOG_INFO, "Client use %s format name",
                            (this->client_use_long_format_names ? "long" : "short"));
                    }

                    if (bool(this->verbose & VNCVerbose::basic_trace)) {
                        general_caps.log(LOG_INFO);
                    }
                }
            }
            break;
        }
        if (bool(this->verbose & VNCVerbose::basic_trace)) {
            LOG(LOG_INFO, "mod_vnc::clipboard_send_to_vnc: done");
        }
    } // clipboard_send_to_vnc

    // Front calls this member function when it became up and running.
public:
    void rdp_input_up_and_running() override {
        if (this->state == WAIT_CLIENT_UP_AND_RUNNING) {
            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                LOG(LOG_INFO, "Client up and running");
            }
            this->state = DO_INITIAL_CLEAR_SCREEN;
            this->event.set_trigger_time(wait_obj::NOW);
        }
    }

    void notify(Widget* sender, notify_event_t event) override {
        (void)sender;
        switch (event) {
        case NOTIFY_SUBMIT:
            this->screen.clear();

            memset(this->password, 0, sizeof(this->password));
            strncpy(this->password, this->challenge.password_edit.get_text(),
                    sizeof(this->password) - 1);
            this->password[sizeof(this->password) - 1] = 0;

            this->state = RETRY_CONNECTION;
            this->event.set_trigger_time(wait_obj::NOW);
            break;
        case NOTIFY_CANCEL:
            this->event.signal = BACK_EVENT_NEXT;
            this->event.set_trigger_time(wait_obj::NOW);

            this->screen.clear();
            break;
        default:
            break;
        }
    }

private:
    bool is_up_and_running() override {
        return (UP_AND_RUNNING == this->state);
    }

    void draw_tile(Rect rect, const uint8_t * raw, gdi::GraphicApi & drawable)
    {
        const uint16_t TILE_CX = 32;
        const uint16_t TILE_CY = 32;

        for (int y = 0; y < rect.cy ; y += TILE_CY) {
            uint16_t cy = std::min(TILE_CY, uint16_t(rect.cy - y));

            for (int x = 0; x < rect.cx ; x += TILE_CX) {
                uint16_t cx = std::min(TILE_CX, uint16_t(rect.cx - x));

                const Rect src_tile(x, y, cx, cy);
                const Bitmap tiled_bmp(raw, rect.cx, rect.cy, this->bpp, src_tile);
                const Rect dst_tile(rect.x + x, rect.y + y, cx, cy);
                const RDPMemBlt cmd2(0, dst_tile, 0xCC, 0, 0, 0);
                drawable.draw(cmd2, dst_tile, tiled_bmp);
            }
        }
    }

public:
    void disconnect(time_t now) override {

        double seconds = ::difftime(now, this->beginning);
        LOG(LOG_INFO, "Client disconnect");

        char extra[1024];
        snprintf(extra, sizeof(extra), "%02d:%02d:%02d",
                        (int(seconds) / 3600),
                        ((int(seconds) % 3600) / 60),
                        (int(seconds) % 60));

        auto info = key_qvalue_pairs({
            {"type", "SESSION_DISCONNECTION"},
            {"duration", extra},
            });

        this->report_message.log5(info);
    }

    Dimension get_dim() const override
    { return Dimension(this->width, this->height); }

    void get_event_handlers(std::vector<EventHandler>& out_event_handlers) override {
        mod_api::get_event_handlers(out_event_handlers);
    }
};

