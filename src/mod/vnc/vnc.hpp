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
   Author(s): Christophe Grosjean, Javier Caverni, Clément Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Vnc module
*/

#pragma once

#include <cstdlib>

#include "core/log_id.hpp"
#include "core/buf64k.hpp"
#include "core/channel_list.hpp"
#include "core/channel_names.hpp"
#include "transport/transport.hpp"
#include "core/front_api.hpp"
#include "core/server_notifier_api.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/RDP/clipboard/format_list_serialize.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "core/RDP/rdp_pointer.hpp"

#include "gdi/screen_functions.hpp"
#include "gdi/graphic_api.hpp"

#include "keyboard/keymap2.hpp"
#include "keyboard/keymapsym.hpp"
#include "main/version.hpp"
#include "RAIL/client_execute.hpp"
#include "mod/mod_api.hpp"
#include "utils/diffiehellman.hpp"
#include "utils/hexdump.hpp"
#include "utils/d3des.hpp"
#include "utils/log.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/utf.hpp"
#include "utils/verbose_flags.hpp"
#include "utils/zlib.hpp"

#include "utils/timebase.hpp"
#include "core/events.hpp"

#include "mod/vnc/vnc_metrics.hpp"

#include "mod/vnc/dsm.hpp"
#include "mod/vnc/encoder/copyrect.hpp"
#include "mod/vnc/encoder/cursor.hpp"
#include "mod/vnc/encoder/encoder_api.hpp"
#include "mod/vnc/encoder/hextile.hpp"
#include "mod/vnc/encoder/raw.hpp"
#include "mod/vnc/encoder/rre.hpp"
#include "mod/vnc/encoder/zrle.hpp"
#include "mod/vnc/newline_convert.hpp"
#include "mod/vnc/vnc_verbose.hpp"
#include "acl/auth_api.hpp"

class UltraDSM;
class mod_vnc;

// got extracts of VNC documentation from
// http://tigervnc.sourceforge.net/cgi-bin/rfbproto



class mod_vnc : public mod_api
{

    static const uint32_t MAX_CLIPBOARD_DATA_SIZE = 1024 * 64;
    enum {
        maxSpokenVncProcotol = 3 * 1000 + 8 // 3.8
    };

    /* mod data */
    char mod_name[256] {0};
    char username[256] {0};
    char password[256] {0};

    FrontAPI& front;

public:

    /** @brief transport for VNC */
    class VncTransport {
    public:
        VncTransport(mod_vnc & mod, Transport & t)
        : m_trans(t)
        , m_mod(mod)
        {
        }

        void send(byte_ptr buffer, size_t len) {
            send(bytes_view(buffer, len));
        }

        void send(bytes_view buffer) {
            if (m_mod.dsmEncryption) {
                BufMaker<0x10000> tmpBuf;
                writable_bytes_view tmp = tmpBuf.dyn_array(buffer.size());

                m_mod.dsm->encrypt(buffer.data(), buffer.size(), tmp);
                m_trans.send(tmp);
            } else {
                m_trans.send(buffer);
            }
        }

        Transport::TlsResult enable_client_tls(ServerNotifier & server_notifier, const TLSClientParams & tls_client_params) {
            return m_trans.enable_client_tls(server_notifier, tls_client_params);
        }

        int get_fd() const {
            return m_trans.get_fd();
        }

        Transport &getTransport() const { return m_trans; }

    private:
        Transport & m_trans;
        mod_vnc & m_mod;
    };

    /** @brief a custom Vnc Buf64k */
    struct VncBuf64k : Buf64k
    {
        VncBuf64k(mod_vnc &mod)
            : m_mod(mod)
        {
        }

        size_type read_from(VncTransport vncTrans)
        {
            Transport & trans = vncTrans.getTransport();

            const size_type read_len = Buf64k::read_from(trans);

            if (m_mod.dsmEncryption){
                writable_bytes_view buf = this->av();
                m_mod.dsm->decrypt(buf.data(), read_len, buf);
            }

            return read_len;
        }

    private:
        mod_vnc & m_mod;
    };

    /**
     *
     */
    struct Mouse {
        void move(OutStream & out_stream, int x, int y) {
            this->x = x;
            this->y = y;
            this->send(out_stream);
        }

        void click(OutStream & out_stream, int x, int y, int mask, bool set) {
            if (set) {
                this->mod_mouse_state |= mask;
            }
            else {
                this->mod_mouse_state &= ~mask;
            }
            this->x = x;
            this->y = y;
            this->send(out_stream);
        }

        void scroll(OutStream & out_stream, int mask) const {
            this->write(out_stream, this->mod_mouse_state | mask);
            this->write(out_stream, this->mod_mouse_state);
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

        void send(OutStream & out_stream) const {
            this->write(out_stream, this->mod_mouse_state);
        }
    } mouse;



private:
    VncTransport t;
    UltraDSM *dsm;
    bool dsmEncryption;

    uint16_t width;
    uint16_t height;
    BitsPerPixel bpp {};
    // TODO BytesPerPixel ?
    uint8_t  depth = 0;

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
    uint32_t to_vnc_clipboard_data_size = 0;
    uint32_t to_vnc_clipboard_data_remaining;

    const bool enable_clipboard_up;   // true clipboard available, false clipboard unavailable
    const bool enable_clipboard_down; // true clipboard available, false clipboard unavailable

    bool client_use_long_format_names = false;
    bool server_use_long_format_names = true;

    /** @brief state of the VNC state machine */
    enum VncState {
        DO_INITIAL_CLEAR_SCREEN,
        UP_AND_RUNNING,
        WAIT_SECURITY_TYPES,
        WAIT_SECURITY_TYPES_LEVEL,
        WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM,
        WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM_RESPONSE,
        WAIT_SECURITY_TYPES_MS_LOGON,
        WAIT_SECURITY_TYPES_MS_LOGON_RESPONSE,
        WAIT_SECURITY_TYPES_INVALID_AUTH,
        WAIT_SECURITY_RESULT,
        WAIT_SECURITY_ULTRA_CHALLENGE,
        DO_VENCRYPT_HANDSHAKE,
        SERVER_INIT,
        SERVER_INIT_RESPONSE,
        WAIT_CLIENT_UP_AND_RUNNING
    };

    /** @brief state for the VeNCrypt state machine */
    enum VeNCryptState {
        WAIT_VENCRYPT_VERSION,
        WAIT_VENCRYPT_VERSION_RESPONSE,
        WAIT_VENCRYPT_SUBTYPES,
        WAIT_VENCRYPT_AUTH_ANSWER
    };

    /** @brief status returned in a security reason */
    enum SecurityReasonStatus {
        SECURITY_REASON_OK = 0,
        SECURITY_REASON_FAILED = 1,
        SECURITY_REASON_TOO_MANY_ATTEMPTS = 2,
        SECURITY_REASON_CONTINUE = 0xffffffff // returned by UltraVNC
    };

public:
    /** @brief VNC clipboard encoding */
    enum class ClipboardEncodingType : uint8_t {
        UTF8   = 0,
        Latin1 = 1
    };

private:
    std::string encodings;

    VncState state = WAIT_SECURITY_TYPES;
    VeNCryptState vencryptState = WAIT_VENCRYPT_VERSION;

    bool     clipboard_requesting_for_data_is_delayed = false;
    uint32_t clipboard_requested_format_id            = 0;
    std::chrono::microseconds clipboard_last_client_data_timestamp = std::chrono::microseconds{};
    ClipboardEncodingType clipboard_server_encoding_type;
    bool clipboard_owned_by_client = true;
    VncBogusClipboardInfiniteLoop bogus_clipboard_infinite_loop = VncBogusClipboardInfiniteLoop::delayed;
    uint32_t clipboard_general_capability_flags = 0;
    time_t beginning;
    ClientExecute* rail_client_execute = nullptr;
    Zdecompressor<> zd;

    gdi::GraphicApi & gd;
    EventsGuard events_guard;
    EventRef clipboard_timeout_timer;

    VNCMetrics * metrics;
    SessionLogApi& session_log;
    /** @brief type of VNC authentication */
    enum VncAuthType : int32_t {
        VNC_AUTH_INVALID     = 0,
        VNC_AUTH_NONE         = 1,
        VNC_AUTH_VNC         = 2,
        VNC_AUTH_TIGHT         = 16,
        VNC_AUTH_ULTRA        = 17,
        VNC_AUTH_TLS         = 18,
        VNC_AUTH_VENCRYPT    = 19,
        VNC_AUTH_ULTRA_MsLogonIAuth = 112,
        VNC_AUTH_ULTRA_MsLogonIIAuth = 113,
        VNC_AUTH_ULTRA_SecureVNCPluginAuth = 114,
        VNC_AUTH_ULTRA_SecureVNCPluginAuth_new = 115,
        VeNCRYPT_TLSNone     = 257,
        VeNCRYPT_TLSVnc     = 258,
        VeNCRYPT_TLSPlain     = 259,
        VeNCRYPT_X509None    = 260,
        VeNCRYPT_X509Vnc    = 261,
        VeNCRYPT_X509Plain    = 262,
        VNC_AUTH_ULTRA_MS_LOGON = -6,
    };

    VncAuthType choosenAuth;

    const bool cursor_pseudo_encoding_supported;

public:
    mod_vnc( Transport & t
           , gdi::GraphicApi & gd
           , EventContainer & events
           , const char * username
           , const char * password
           , FrontAPI & front
           // TODO: front width and front height should be provided through info
           , uint16_t front_width
           , uint16_t front_height
           , int keylayout
           , int key_flags
           , bool clipboard_up
           , bool clipboard_down
           , const char * encodings
           , ClipboardEncodingType clipboard_server_encoding_type
           , VncBogusClipboardInfiniteLoop bogus_clipboard_infinite_loop
           , bool server_is_macos
           , bool server_is_unix
           , bool cursor_pseudo_encoding_supported
           , ClientExecute* rail_client_execute
           , VNCVerbose verbose
           , [[maybe_unused]] VNCMetrics * metrics
           , SessionLogApi& session_log);

    ~mod_vnc();

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
                    if (this->state != State::Data) {
                        return false;
                    }
                    REDEMPTION_CXX_FALLTHROUGH;
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

            f(u8_array_view{buf.av().data(), sz});
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
        BasicResult() noexcept = default;

        BasicResult(T value) noexcept
          : is_ok(true)
          , value(value)
        {}

        bool is_ok = false;
        T value;
    };

    class SecurityResult
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

        template<class F> // f(bool status, u8_array_view raison_fail)
        bool run(Buf64k & buf, F && f)
        {
            switch (this->state)
            {
                case State::Header:
                    if (auto r = this->read_header(buf)) {
                        if (r == State::Finish) {
                            f(true, nullptr);
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
                    return this->reason.run(buf, [&f](u8_array_view av){ f(false, av); });
                case State::Finish:
                    return true;
            }

            REDEMPTION_UNREACHABLE();
        }

    private:
        using ReasonCtx = MessageCtx<256>;

        State state = State::Header;
        ReasonCtx reason;

        static Result read_header(Buf64k & buf)
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
    SecurityResult auth_response_ctx;

    class MsLogonCtx {
        enum class State {
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

    bool ms_logon(Buf64k & buf);

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


//    7.4.7   EnableContinuousUpdates

//    This message informs the server to switch between only sending FramebufferUpdate messages as a result of a
//    FramebufferUpdateRequest message, or sending FramebufferUpdate messages continuously.

//    Note that there is currently no way to determine if the server supports this message except for using the
//       Tight Security Type authentication.

//    No. of bytes       Type     [Value]     Description
//            1           U8       150       message-type
//            1           U8                 enable-flag
//            2           U16                x-position
//            2           U16                y-position
//            2           U16                width
//            2           U16                height

//    If enable-flag is non-zero, then the server can start sending FramebufferUpdate messages as needed for the area
// specified by x-position, y-position, width, and height. If continuous updates are already active, then they must
// remain active active and the coordinates must be replaced with the last message seen.

//    If enable-flag is zero, then the server must only send FramebufferUpdate messages as a result of receiving
// FramebufferUpdateRequest messages. The server must also immediately send out a EndOfContinuousUpdates message.
// This message must be sent out even if continuous updates were already disabled.

//    The server must ignore all incremental update requests (FramebufferUpdateRequest with incremental set to
// non-zero) as long as continuous updates are active. Non-incremental update requests must however be honored,
// even if the area in such a request does not overlap the area specified for continuous updates.

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
            vnc.bpp    = safe_int(stream.in_uint8());
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

    void initial_clear_screen(gdi::GraphicApi & drawable);

    // TODO It may be possible to change several mouse buttons at once ? Current code seems to perform several send if that occurs. Is it what we want ?
    void rdp_input_mouse( int device_flags, int x, int y, Keymap2 * /*keymap*/ ) override;
    void rdp_input_scancode(long keycode, long /*param2*/, long device_flags, long /*param4*/, Keymap2 * /*keymap*/) override;
    void rdp_input_unicode(uint16_t /*unicode*/, uint16_t /*flag*/) override;

    void send_keyevent(uint8_t down_flag, uint32_t key);

private:
    void rdp_input_clip_data(bytes_view data);

public:
    void rdp_input_synchronize(
        uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2
    ) override;

private:
    void update_screen(Rect r, uint8_t incr);

public:
    void rdp_input_invalidate(Rect r) override;

    void refresh(Rect r) override {
        LOG(LOG_INFO, "Front::refresh");
        this->rdp_input_invalidate(r);
    }

private:

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

    // VNC Client to Server Messages
    enum VNC_client_to_server_messages {
        VNC_CS_MSG_SET_PIXEL_FORMAT              = 0,
        VNC_CS_MSG_SET_ENCODINGS                 = 2,
        VNC_CS_MSG_FRAME_BUFFER_UPDATE_REQUEST   = 3,
        VNC_CS_MSG_KEY_EVENT                     = 4,
        VNC_CS_MSG_POINTER_EVENT                 = 5,
        VNC_CS_MSG_CLIENT_CUT_TEXT               = 6,
        // Optional Messages
        VNC_CS_MSG_FILE_TRANSFER                  = 7,
        VNC_CS_MSG_SET_SCALE                      = 8,
        VNC_CS_MSG_SET_SERVER_INPUT               = 9,
        VNC_CS_MSG_SET_SW                         = 10,
        VNC_CS_MSG_TEXT_CHAT                      = 11,
        VNC_CS_MSG_KEY_FRAME_REQUEST              = 12,
        VNC_CS_MSG_KEEP_ALIVE                     = 13,
        VNC_CS_MSG_ULTRA_VNC_RESERVED1            = 14,
        VNC_CS_MSG_SET_SCALE_FACTOR               = 15,
        VNC_CS_MSG_ULTRA_VNC_RESERVED2            = 16,
        VNC_CS_MSG_ULTRA_VNC_RESERVED3            = 17,
        VNC_CS_MSG_ULTRA_VNC_RESERVED4            = 18,
        VNC_CS_MSG_ULTRA_VNC_RESERVED5            = 19,
        VNC_CS_MSG_REQUEST_SESSION                = 20,
        VNC_CS_MSG_SET_SESSION                    = 21,
        VNC_CS_MSG_NOTIFY_PLUGIN_STREAMING        = 80,
        VNC_CS_MSG_VMWARE1                        = 127,
        VNC_CS_MSG_CAR_CONNECTIVITY               = 128,
        VNC_CS_MSG_ENABLE_CONTINUOUS_UPDATE       = 150,
        VNC_CS_MSG_CLIENT_FENCE                   = 248,
        VNC_CS_MSG_OLIVE_CALL_CONTROL             = 249,
        VNC_CS_MSG_XVP_CLIENT_MESSAGE             = 250,
        VNC_CS_MSG_SET_DESKTOP_SIZE               = 251,
        VNC_CS_MSG_TIGHT                          = 252,
        VNC_CS_MSG_GII_CLIENT_MESSAGE             = 253,
        VNC_CS_MSG_VMWARE2                        = 254,
        VNC_CS_MSG_QEMU_CLIENT_MESSAGE            = 255,
    };

    // VNC Client to Server Messages
    enum VNC_server_to_client_messages {
        VNC_SC_MSG_FRAMEBUFFER_UPDATE             = 0,
        VNC_SC_MSG_SET_COLOUR_MAP_ENTRIES         = 1,
        VNC_SC_MSG_BELL                           = 2,
        VNC_SC_MSG_SERVER_CUT_TEXT                = 3,
        VNC_SC_MSG_END_OF_CONTINUOUS_UPDATE       = 150,
        VNC_SC_MSG_SERVER_STATE                   = 173,
        VNC_SC_MSG_SERVER_FENCE                   = 248,
        VNC_SC_MSG_XVP                            = 250,
        VNC_SC_MSG_TIGHT                          = 252,
        VNC_SC_MSG_GII                            = 253,
    };

    class PasswordCtx
    {
        enum class State
        {
            RandomKey,
            Finish,
        };

    public:
        writable_u8_array_view server_random;

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

                this->message_type = VNC_server_to_client_messages(buf.av()[0]);

                buf.advance(1);


                switch (this->message_type)
                {
                    case VNC_SC_MSG_FRAMEBUFFER_UPDATE: /* framebuffer update */
                        vnc.frame_buffer_update_ctx.start(vnc.bpp, to_bytes_per_pixel(vnc.bpp));
                        this->state = State::FrameBufferupdate;
                        return vnc.lib_frame_buffer_update(drawable, buf);
                    case VNC_SC_MSG_SET_COLOUR_MAP_ENTRIES: /* palette */
                        vnc.palette_update_ctx.start();
                        this->state = State::Palette;
                        return vnc.lib_palette_update(drawable, buf);
                    case VNC_SC_MSG_BELL: /* bell */
                        // TODO bell
                        return true;
                    case VNC_SC_MSG_SERVER_CUT_TEXT: /* clipboard */ /* ServerCutText */
                        vnc.clipboard_data_ctx.start(
                            vnc.enable_clipboard_down
                         && vnc.get_channel_by_name(channel_names::cliprdr));
                        this->state = State::ServerCutText;
                        return vnc.lib_clip_data(buf);
                    default:
                        LOG(LOG_ERR, "unknown message type in vnc %u", message_type);
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
        VNC_server_to_client_messages message_type;
    };

    bool doTlsSwitch();

    UpAndRunningCtx up_and_running_ctx;

    VncBuf64k server_data_buf;
    int spokenProtocol;
    bool tlsSwitch;

public:
    void draw_event(gdi::GraphicApi & gd);

private:
    static const char *securityTypeString(int32_t t);

    void updatePreferedAuth(int32_t authId, VncAuthType &preferedAuth, size_t &preferedAuthIndex);

    bool readSecurityResult(InStream &s, uint32_t &status, bool &haveReason, std::string &reason, size_t &skipLen) const;

    bool treatVeNCrypt();

    bool draw_event_impl(gdi::GraphicApi & gd);

private:
    void check_timeout();

private:
    struct FrameBufferUpdateCtx
    {
        enum class State
        {
            Header,
            Encoding,
            Data,
        };

        using Result = BasicResult<State>;

        VNC::Encoder::EncoderState last;

        FrameBufferUpdateCtx(Zdecompressor<> & zd, VNCVerbose verbose)
          : bpp(BitsPerPixel::BitsPP32)
          , state(State::Header)
          , num_recs(0)
          , x(0)
          , y(0)
          , cx(0)
          , cy(0)
          , encoding(0)
          , zd{zd}
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
            this->last = VNC::Encoder::EncoderState::Ready;
        }

//        ~FrameBufferUpdateCtx()
//        {
//            inflateEnd(&this->zstrm);
//        }

        void start(BitsPerPixel bpp, BytesPerPixel Bpp)
        {
            this->bpp = bpp;
            this->Bpp = Bpp;
            this->state = State::Header;
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

        size_t last_avail = 0;

        bool run(Buf64k & buf, mod_vnc & vnc, gdi::GraphicApi & drawable)
        {
            Result r = Result::fail();

            for (;;) {
                switch (this->state)
                {
                case State::Header:
                {
                    const size_t sz = 3;

                    if (buf.remaining() < sz)
                    {
                        r = Result::fail();
                        break;
                    }

                    InStream stream(buf.av(sz));
                    stream.in_skip_bytes(1);
                    this->num_recs = stream.in_uint16_be();

                    buf.advance(sz);
                    r = Result::ok(State::Encoding);
                }
                break;
                case State::Encoding:
                {
                    if (0 == this->num_recs) {
                        this->state = State::Header;
                        return true;
                    }
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

                        LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "Encoding: %u (%u, %u, %u, %u) : %d", this->num_recs, this->x, this->y, this->cx, this->cy, this->encoding);

                        --this->num_recs;

                        LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO,
                            "%s %d (%d, %d, %d, %d)",
                            (this->encoding == HEXTILE_ENCODING)
                            ? "HEXTILE_ENCODING"
                            : (this->encoding == CURSOR_PSEUDO_ENCODING)
                            ? "CURSOR_PSEUDO_ENCODING"
                            : (this->encoding == COPYRECT_ENCODING)
                            ? "COPYRECT_ENCODING"
                            : (this->encoding == RRE_ENCODING)
                            ? "RRE_ENCODING"
                            : (this->encoding == RAW_ENCODING)
                            ? "RAW_ENCODING"
                            : (this->encoding == ZRLE_ENCODING)
                            ? "ZRLE_ENCODING"
                            : "UNKNOWN_ENCODING",
                            this->encoding , this->x, this->y, this->cx, this->cy);

                        switch (this->encoding){
                        case COPYRECT_ENCODING:  /* raw */
                            this->encoder = VNC::Encoder::copy_rect_encoder(
                                Rect(this->x, this->y, this->cx, this->cy),
                                vnc.width, vnc.height);
                            break;
                        case HEXTILE_ENCODING:  /* hextile */
                            this->encoder = VNC::Encoder::hextile_encoder(
                                this->bpp, this->Bpp, Rect(this->x, this->y, this->cx, this->cy),
                                vnc.verbose);
                            break;
                        case CURSOR_PSEUDO_ENCODING:  /* cursor */
                            this->encoder = VNC::Encoder::cursor_encoder(
                                this->Bpp, Rect(this->x, this->y, this->cx, this->cy),
                                vnc.red_shift, vnc.red_max, vnc.green_shift, vnc.green_max,
                                vnc.blue_shift, vnc.blue_max, vnc.verbose);
                            break;
                        case RAW_ENCODING:  /* raw */
                            this->encoder = VNC::Encoder::raw_encoder(
                                this->bpp, this->Bpp, Rect(this->x, this->y, this->cx, this->cy));
                            break;
                        case ZRLE_ENCODING: /* ZRLE */
                            this->encoder = VNC::Encoder::zrle_encoder(
                                this->bpp, this->Bpp, Rect(this->x, this->y, this->cx, this->cy),
                                this->zd, vnc.verbose);
                            break;
                        case RRE_ENCODING: /* RRE */
                            this->encoder = VNC::Encoder::rre_encoder(
                                this->bpp, this->Bpp, Rect(this->x, this->y, this->cx, this->cy));
                            break;
                        default:
                            LOG(LOG_ERR, "unexpected VNC encoding %d", encoding);
                            throw Error(ERR_VNC_UNEXPECTED_ENCODING_IN_LIB_FRAME_BUFFER);
                        }
                        buf.advance(sz);
                        // Note: it is important to immediately call State::Data as in some cases there won't be
                        // any trailing data to expect.
                        this->last = VNC::Encoder::EncoderState::Ready;
                        r = Result::ok(State::Data);
                    }
                }
                break;
                case State::Data:
                    {
                        if (this->last == VNC::Encoder::EncoderState::NeedMoreData){
                            if (this->last_avail == buf.remaining()){
                                LOG(LOG_ERR, "new call to vnc::mod without new data");
                                throw Error(ERR_VNC);
                            }
                        }
                        if (!bool(this->encoder)){
                            LOG(LOG_ERR, "Call to vnc::mod with null encoder");
                            throw Error(ERR_VNC);
                        }

                        // Pre Assertion: we have an encoder
                        switch (encoder(buf, drawable)){
                            case VNC::Encoder::EncoderState::Ready:
                                r = Result::ok(State::Data);
                                this->last = VNC::Encoder::EncoderState::Ready;
                            break;
                            case VNC::Encoder::EncoderState::NeedMoreData:
                                r = Result::fail();
                                this->last_avail = buf.remaining();
                                this->last = VNC::Encoder::EncoderState::NeedMoreData;
                            break;
                            case VNC::Encoder::EncoderState::Exit:
                                // consume returns true if encoder is finished (ready to be resetted)
                                r = Result::ok(State::Encoding);
                                encoder = nullptr;
                                this->last = VNC::Encoder::EncoderState::NeedMoreData;
                                break;
                        }
                    }
                }

                if (!r) {
                    return false;
                }
                this->state = r;
            }

            return true;
        }

    private:
        BitsPerPixel bpp;
        BytesPerPixel Bpp;

        State state;

        uint16_t num_recs;

        uint16_t x;
        uint16_t y;
        uint16_t cx;
        uint16_t cy;
        int32_t encoding;

        VNC::Encoder::Encoder encoder;

        Zdecompressor<> & zd;

        VNCVerbose verbose;
    };
    FrameBufferUpdateCtx frame_buffer_update_ctx;

    bool lib_frame_buffer_update(gdi::GraphicApi & drawable, Buf64k & buf)
    {
        drawable.begin_update();
        const bool ok = this->frame_buffer_update_ctx.run(buf, *this, drawable);
        drawable.end_update();
        if (!ok) {
            return false;
        }

        this->update_screen(Rect(0, 0, this->width, this->height), 1);
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

        [[nodiscard]] BGRPalette const & get_palette() const noexcept
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
                LOG(LOG_ERR, "VNC: number of palette colors too large: %d",
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

        drawable.set_palette(this->palette_update_ctx.get_palette());
        drawable.begin_update();
        drawable.draw(RDPColCache(0, this->palette_update_ctx.get_palette()));
        drawable.end_update();

        return true;
    } // lib_palette_update

    /******************************************************************************/
    void lib_open_clip_channel() {
        const CHANNELS::ChannelDef * channel = this->get_channel_by_name(channel_names::cliprdr);

        if (channel) {
            // Monitor ready PDU send to front
            RDPECLIP::ServerMonitorReadyPDU server_monitor_ready_pdu;
            RDPECLIP::CliprdrHeader         header(RDPECLIP::CB_MONITOR_READY, RDPECLIP::CB_RESPONSE__NONE_, server_monitor_ready_pdu.size());

            StaticOutStream<64> out_s;

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

            header.emit(out_s);
            server_monitor_ready_pdu.emit(out_s);

            size_t chunk_size = out_s.get_offset();

            this->send_to_front_channel( channel_names::cliprdr
                                       , out_s.get_data()
                                       , chunk_size // total size is chunk size
                                       , chunk_size
                                       ,   CHANNELS::CHANNEL_FLAG_FIRST
                                         | CHANNELS::CHANNEL_FLAG_LAST
                                       );
        }
        else {
            LOG(LOG_ERR, "Clipboard Channel Redirection unavailable");
        }
    } // lib_open_clip_channel

    //==============================================================================================================
    [[nodiscard]] const CHANNELS::ChannelDef * get_channel_by_name(CHANNELS::ChannelNameId channel_name) const {
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
          : state(State::Header)
          , verbose(verbose)
          , clipboard_down_is_really_enabled(false)
          , data_length(0)
          , remaining_data_length(0)
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

        [[nodiscard]] bool clipboard_is_enabled() const noexcept
        {
            return this->clipboard_down_is_really_enabled;
        }

        [[nodiscard]] bytes_view clipboard_data() const noexcept
        {
            return this->to_rdp_clipboard_data.get_consumed_bytes();
        }

        [[nodiscard]] bool clipboard_data_is_utf8_encoded() const noexcept
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

            LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "mod_vnc::lib_clip_data: clipboard_data_length=%u", this->data_length);

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
                // if (this->data_length <= 64) {
                   // hexdump_c(this->to_rdp_clipboard_data.get_data(), this->data_length);
                // }
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
    bool lib_clip_data(Buf64k & buf);

private:
    void send_to_front_channel(CHANNELS::ChannelNameId mod_channel_name, uint8_t const * data,
            size_t length, size_t chunk_size, int flags);
public:
    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream & chunk, size_t length, uint32_t flags) override;

private:
    void send_clipboard_pdu_to_front(const OutStream & out_stream);

    void clipboard_send_to_vnc_server(InStream & chunk, size_t length, uint32_t flags);

public:
    // Front calls this member function when it became up and running.
    void rdp_gdi_up_and_running() override;
    void rdp_gdi_down() override {}

private:
    [[nodiscard]] bool is_up_and_running() const override {
        return (UP_AND_RUNNING == this->state);
    }

    void draw_tile(Rect rect, const uint8_t * raw, gdi::GraphicApi & drawable);

public:
    bool server_error_encountered() const override { return false; }

    void disconnect() override;

    [[nodiscard]] Dimension get_dim() const override
    { return Dimension(this->width, this->height); }
};

