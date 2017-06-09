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

   Vnc module
*/

#pragma once

#include "main/version.hpp"

#include "utils/log.hpp"
#include "mod/internal/widget2/flat_vnc_authentification.hpp"
#include "mod/internal/widget2/notify_api.hpp"
#include "mod/internal/internal_mod.hpp"
#include "keyboard/keymapSym.hpp"
#include "utils/diffiehellman.hpp"
#include "utils/d3des.hpp"
#include "core/channel_list.hpp"
#include "core/RDP/pointer.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "core/report_message_api.hpp"
#include "utils/sugar/update_lock.hpp"
#include "transport/socket_transport.hpp"
#include "core/channel_names.hpp"
#include "utils/sugar/strutils.hpp"
#include "utils/utf.hpp"

#include <cstdlib>

#include <zlib.h>


// got extracts of VNC documentation from
// http://tigervnc.sourceforge.net/cgi-bin/rfbproto

struct mod_vnc : public InternalMod, private NotifyApi {
    static const uint32_t MAX_CLIPBOARD_DATA_SIZE = 1024 * 64;

    FlatVNCAuthentification challenge;

    /* mod data */
    char mod_name[256];
    BGRPalette palette;
    int vnc_desktop;
    char username[256];
    char password[256];

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

public:
    uint16_t width;
    uint16_t height;
    uint8_t  bpp;
    uint8_t  depth;

private:
    uint8_t endianess;
    uint8_t true_color_flag;

    uint16_t red_max;
    uint16_t green_max;
    uint16_t blue_max;

    uint8_t red_shift;
    uint8_t green_shift;
    uint8_t blue_shift;

    uint32_t   verbose;
    KeymapSym  keymapSym;

    uint8_t to_rdp_clipboard_data_buffer[MAX_CLIPBOARD_DATA_SIZE];
    InStream to_rdp_clipboard_data;
    bool     to_rdp_clipboard_data_is_utf8_encoded;

    StaticOutStream<MAX_CLIPBOARD_DATA_SIZE> to_vnc_clipboard_data;
    uint32_t to_vnc_clipboard_data_size;
    uint32_t to_vnc_clipboard_data_remaining;

private:
    const bool enable_clipboard_up;   // true clipboard available, false clipboard unavailable
    const bool enable_clipboard_down; // true clipboard available, false clipboard unavailable

    bool client_use_long_format_names = false;
    bool server_use_long_format_names = true;

    z_stream zstrm;

    enum {
        ASK_PASSWORD,
        DO_INITIAL_CLEAR_SCREEN,
        RETRY_CONNECTION,
        UP_AND_RUNNING,
        WAIT_PASSWORD,
        WAIT_SECURITY_TYPES,
        WAIT_CLIENT_UP_AND_RUNNING
    };

public:
    enum class ClipboardEncodingType : uint8_t {
        UTF8   = 0,
        Latin1 = 1
    };

public:
    std::string encodings;

private:
    int state;

    bool allow_authentification_retries;

private:
    bool is_first_membelt = true;
    bool left_ctrl_pressed = false;

    const bool is_socket_transport;

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

public:
    //==============================================================================================================
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
           , bool is_socket_transport
           , ClipboardEncodingType clipboard_server_encoding_type
           , VncBogusClipboardInfiniteLoop bogus_clipboard_infinite_loop
           , ReportMessageApi & report_message
           , bool server_is_apple
           , uint32_t verbose
           )
    //==============================================================================================================
    : InternalMod(front, front_width, front_height, font, theme, false)
    , challenge(front, front_width, front_height, this->screen, static_cast<NotifyApi*>(this),
                "Redemption " VERSION, this->theme(), label_text_message, label_text_password,
                this->font())
    , mod_name{0}
    , palette(BGRPalette::classic_332())
    , vnc_desktop(0)
    , username{0}
    , password{0}
    , t(t)
    , verbose(verbose)
    , keymapSym(verbose)
    , to_rdp_clipboard_data(this->to_rdp_clipboard_data_buffer)
    , to_rdp_clipboard_data_is_utf8_encoded(false)
    , to_vnc_clipboard_data_size(0)
    , enable_clipboard_up(clipboard_up)
    , enable_clipboard_down(clipboard_down)
    , encodings(encodings)
    , state(WAIT_SECURITY_TYPES)
    , allow_authentification_retries(allow_authentification_retries || !(*password))
    , is_socket_transport(is_socket_transport)
    , clipboard_server_encoding_type(clipboard_server_encoding_type)
    , bogus_clipboard_infinite_loop(bogus_clipboard_infinite_loop)
    , report_message(report_message)
    , server_is_apple(server_is_apple)
    , keylayout(keylayout)
    {
    //--------------------------------------------------------------------------------------------------------------
        LOG(LOG_INFO, "Creation of new mod 'VNC'");

        ::time(&this->beginning);

        memset(&zstrm, 0, sizeof(zstrm));
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
        if (inflateInit(&this->zstrm) != Z_OK)
        REDEMPTION_DIAGNOSTIC_POP
        {
            LOG(LOG_ERR, "vnc zlib initialization failed");

            throw Error(ERR_VNC_ZLIB_INITIALIZATION);
        }
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

    //==============================================================================================================
    ~mod_vnc() override {
        inflateEnd(&this->zstrm);

        // TODO mod_vnc isn't owner of sck
        if (this->is_socket_transport) {
            auto & st = static_cast<SocketTransport&>(this->t);
            if (st.sck > 0){
                close(st.sck);
            }
        }

        this->screen.clear();
    }

    int get_fd() const override { return this->t.get_fd(); }

    void ms_logon(uint64_t gen, uint64_t mod, uint64_t resp) {
        if (this->verbose) {
            LOG(LOG_INFO, "MS-Logon with following values:");
            LOG(LOG_INFO, "Gen=%" PRIu64, gen);
            LOG(LOG_INFO, "Mod=%" PRIu64, mod);
            LOG(LOG_INFO, "Resp=%" PRIu64, resp);
        }
        DiffieHellman dh(gen, mod);
        uint64_t pub = dh.createInterKey();

        StaticOutStream<32768> out_stream;
        out_stream.out_uint64_be(pub);

        uint64_t key = dh.createEncryptionKey(resp);
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
        if (this->verbose) {
            LOG(LOG_INFO, "Waiting for password ack");
        }

        auto in_uint32_be = [&]{
            uint8_t buf_stream[4];
            this->t.recv_boom(buf_stream, 4);
            return Parse(buf_stream).in_uint32_be();

        };
        uint32_t i = in_uint32_be();
        if (i != 0u) {
            // vnc password failed
            LOG(LOG_INFO, "MS LOGON password FAILED\n");
            // Optionnal
            try {
                uint32_t reason_length = in_uint32_be();

                char   reason[256];
                char * preason = reason;

                this->t.recv_boom(preason, std::min<size_t>(sizeof(reason) - 1, reason_length));
                preason += std::min<size_t>(sizeof(reason) - 1, reason_length);
                *preason = 0;

                LOG(LOG_INFO, "Reason for the connection failure: %s", preason);
            }
            catch (Error const &) {
            }
        } else {
            if (this->verbose) {
                LOG(LOG_INFO, "MS LOGON password ok\n");
            }
        }

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
        LOG(LOG_INFO, "mod_vnc::rdp_input_scancode(device_flags=%ld, param1=%ld)", device_flags, param1);


        uint8_t downflag = !(device_flags & KBD_FLAG_UP);

        if (this->server_is_apple) {
            this->apple_keyboard_translation(device_flags, param1, downflag);
        } else {
            this->keyMapSym_event(device_flags, param1, downflag);
        }

    } // rdp_input_scancode


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
        if (this->verbose) {
            LOG(LOG_INFO, "VNC Send KeyEvent Flag down: %d, key: 0x%x", down_flag, key);
        }
        StaticOutPerStream<8> stream;
        stream.out_uint8(4);
        stream.out_uint8(down_flag); /* down/up flag */
        stream.out_clear_bytes(2);
        stream.out_uint32_be(key);
        this->t.send(stream.get_data(), stream.get_offset());
        this->event.set(1000);
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

            this->event.set(1000);
        };

        if (this->state == UP_AND_RUNNING) {
            if (this->clipboard_requested_format_id == RDPECLIP::CF_UNICODETEXT) {
                if (this->clipboard_server_encoding_type == ClipboardEncodingType::UTF8) {
                    if (this->verbose & 0x80) {
                        LOG(LOG_INFO, "mod_vnc::rdp_input_clip_data: CF_UNICODETEXT -> UTF-8");
                    }

                    const size_t utf8_data_length =
                        data_length / sizeof(uint16_t) * maximum_length_of_utf8_character_in_bytes;
                    std::unique_ptr<uint8_t[]> utf8_data(new uint8_t[utf8_data_length]);

                    ::UTF16toUTF8(data, data_length, utf8_data.get(), utf8_data_length);

                    client_cut_text(::char_ptr_cast(utf8_data.get()));
                }
                else {
                    if (this->verbose & 0x80) {
                        LOG(LOG_INFO, "mod_vnc::rdp_input_clip_data: CF_UNICODETEXT -> Latin-1");
                    }

                    const size_t latin1_data_length = data_length / sizeof(uint16_t);
                    std::unique_ptr<uint8_t[]> latin1_data(new uint8_t[latin1_data_length]);

                    ::UTF16toLatin1(data, data_length, latin1_data.get(), latin1_data_length);

                    client_cut_text(::char_ptr_cast(latin1_data.get()));
                }
            }
            else {
                if (this->clipboard_server_encoding_type == ClipboardEncodingType::UTF8) {
                    if (this->verbose & 0x80) {
                        LOG(LOG_INFO, "mod_vnc::rdp_input_clip_data: CF_TEXT -> UTF-8");
                    }

                    const size_t utf16_data_length = data_length * sizeof(uint16_t);
                    std::unique_ptr<uint8_t[]> utf16_data(new uint8_t[utf16_data_length]);

                    const size_t result = ::Latin1toUTF16(data, data_length, utf16_data.get(),
                        utf16_data_length);

                    const size_t utf8_data_length = data_length * maximum_length_of_utf8_character_in_bytes;
                    std::unique_ptr<uint8_t[]> utf8_data(new uint8_t[utf8_data_length]);

                    ::UTF16toUTF8(utf16_data.get(), result, utf8_data.get(), utf8_data_length);

                    client_cut_text(::char_ptr_cast(utf8_data.get()));
                }
                else {
                    if (this->verbose & 0x80) {
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
        if (this->verbose) {
            LOG( LOG_INFO
               , "KeymapSym::synchronize(time=%u, device_flags=%08x, param1=%04x, param1=%04x"
               , time, device_flags, param1, param2);
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
    static void fill_encoding_types_buffer(const char * encodings, OutStream & stream, uint16_t & number_of_encodings, uint32_t verbose)
    {
        if (verbose) {
            LOG(LOG_INFO, "VNC Encodings=\"%s\"", encodings);
        }

        auto stream_offset = stream.get_offset();

        char * end;
        char const * p = encodings;
        for (int32_t encoding_type = std::strtol(p, &end, 0);
            p != end;
            encoding_type = std::strtol(p, &end, 10))
        {
            if (verbose) {
                LOG(LOG_INFO, "VNC Encoding type=0x%08X", encoding_type);
            }
            stream.out_uint32_be(encoding_type);

            p = end;
            while (*p && (*p == ' ' || *p == '\t' || *p == ',')) {
                ++p;
            }
        }
        number_of_encodings = (stream.get_offset() - stream_offset) / 4;
    }

public:
    void draw_event(time_t now, gdi::GraphicApi & drawable) override {
        (void)now;
        if (this->verbose & 2) {
            LOG(LOG_INFO, "vnc::draw_event");
        }
        switch (this->state)
        {
        case ASK_PASSWORD:
            if (this->verbose & 1) {
                LOG(LOG_INFO, "state=ASK_PASSWORD");
            }
            this->screen.add_widget(&this->challenge);

            this->screen.set_widget_focus(&this->challenge, Widget2::focus_reason_tabkey);

            this->challenge.password_edit.set_text("");

            this->challenge.set_widget_focus(&this->challenge.password_edit, Widget2::focus_reason_tabkey);

            this->screen.rdp_input_invalidate(this->screen.get_rect());

            this->state = WAIT_PASSWORD;
            break;
        case DO_INITIAL_CLEAR_SCREEN:
            {
                if (this->verbose & 1) {
                    LOG(LOG_INFO, "state=DO_INITIAL_CLEAR_SCREEN");
                }
                // set almost null cursor, this is the little dot cursor
                Pointer cursor;
                cursor.x = 3;
                cursor.y = 3;
//                cursor.bpp = 24;
                cursor.width = 32;
                cursor.height = 32;
                memset(cursor.data + 31 * (32 * 3), 0xff, 9);
                memset(cursor.data + 30 * (32 * 3), 0xff, 9);
                memset(cursor.data + 29 * (32 * 3), 0xff, 9);
                memset(cursor.mask, 0xff, 32 * (32 / 8));
                cursor.update_bw();
                this->front.set_pointer(cursor);

                this->report_message.log4(false, "SESSION_ESTABLISHED_SUCCESSFULLY");

                LOG(LOG_INFO, "VNC connection complete, connected ok\n");
                this->front.begin_update();
                RDPOpaqueRect orect(Rect(0, 0, this->width, this->height), RDPColor{});
                drawable.draw(orect, Rect(0, 0, this->width, this->height), gdi::ColorCtx::from_bpp(this->bpp, this->palette));
                this->front.end_update();

                this->state = UP_AND_RUNNING;
                this->front.can_be_start_capture();
                this->update_screen(Rect(0, 0, this->width, this->height));
                this->lib_open_clip_channel();

                this->event.object_and_time = false;
                if (this->verbose & 1) {
                    LOG(LOG_INFO, "VNC screen cleaning ok\n");
                }

                RDPECLIP::GeneralCapabilitySet general_caps(
                    RDPECLIP::CB_CAPS_VERSION_1,
                    this->server_use_long_format_names?RDPECLIP::CB_USE_LONG_FORMAT_NAMES:0);

                if (this->verbose) {
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

                if (this->verbose) {
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

                if (this->verbose) {
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
            break;
        case RETRY_CONNECTION:
            if (this->verbose & 1) {
                LOG(LOG_INFO, "state=RETRY_CONNECTION");
            }
            try
            {
                this->t.connect();
            }
            catch (Error const &)
            {
                throw Error(ERR_VNC_CONNECTION_ERROR);
            }

            this->state = WAIT_SECURITY_TYPES;
            this->event.set();
            break;
        case UP_AND_RUNNING:
            if (this->verbose & 2) {
                LOG(LOG_INFO, "state=UP_AND_RUNNING");
            }
            if (this->is_socket_transport && static_cast<SocketTransport&>(this->t).can_recv()) {
                try {
                    uint8_t type; /* message-type */
                    this->t.recv_boom(&type, 1);
                    switch (type) {
                        case 0: /* framebuffer update */
                            this->lib_framebuffer_update(drawable);
                        break;
                        case 1: /* palette */
                            this->lib_palette_update(drawable);
                        break;
                        case 3: /* clipboard */ /* ServerCutText */
                            this->lib_clip_data();
                        break;
                        default:
                            LOG(LOG_INFO, "unknown in vnc_lib_draw_event %d\n", type);
                        break;
                    }
                }
                catch (const Error & e) {
                    LOG(LOG_INFO, "VNC Stopped [reason id=%u]", e.id);
                    this->event.signal = BACK_EVENT_NEXT;
                    this->front.must_be_stop_capture();
                }
                catch (...) {
                    LOG(LOG_INFO, "unexpected exception raised in VNC");
                    this->event.signal = BACK_EVENT_NEXT;
                    this->front.must_be_stop_capture();
                }
                if (this->event.signal != BACK_EVENT_NEXT) {
                    this->event.set(1000);
                }
            }
            else {
                this->update_screen(Rect(0, 0, this->width, this->height));
            }
            break;
        case WAIT_PASSWORD:
            if (this->verbose & 1) {
                LOG(LOG_INFO, "state=WAIT_PASSWORD");
            }
            this->event.object_and_time = false;
            this->event.reset();
            break;
        case WAIT_SECURITY_TYPES:
            {
                if (this->verbose & 1) {
                    LOG(LOG_INFO, "state=WAIT_SECURITY_TYPES");
                }

                /* protocol version */
                uint8_t server_protoversion[12];
                this->t.recv_boom(server_protoversion, 12);
                server_protoversion[11] = 0;
                if (this->verbose) {
                    LOG(LOG_INFO, "Server Protocol Version=%s\n", server_protoversion);
                }
                this->t.send("RFB 003.003\n", 12);
                // sec type

                int32_t const security_level = [this](){
                    uint8_t buf[4];
                    this->t.recv_boom(buf, sizeof(buf));
                    return Parse(buf).in_sint32_be();
                }();

                if (this->verbose) {
                    LOG(LOG_INFO, "security level is %d "
                        "(1 = none, 2 = standard, -6 = mslogon)\n",
                        security_level);
                }

                switch (security_level){
                    case 1: // none
                        break;
                    case 2: // the password and the server random
                    {
                        if (this->verbose) {
                            LOG(LOG_INFO, "Receiving VNC Server Random");
                        }
                        uint8_t buf[16];
                        auto recv = [&](size_t len) {
                            this->t.recv_boom(buf, len);
                        };
                        recv(16);

                        // taken from vncauth.c
                        {
                            char key[12] = {};

                            // key is simply password padded with nulls
                            strncpy(key, this->password, 8);
                            rfbDesKey(reinterpret_cast<unsigned char*>(key), EN0); // 0, encrypt
                            rfbDes(buf, buf);
                            rfbDes(buf + 8, buf + 8);
                        }
                        if (this->verbose) {
                            LOG(LOG_INFO, "Sending Password");
                        }
                        this->t.send(buf, 16);

                        // sec result
                        if (this->verbose) {
                            LOG(LOG_INFO, "Waiting for password ack");
                        }
                        recv(4);
                        int i = Parse(buf).in_uint32_be();
                        if (i != 0) {
                            // vnc password failed
                            // Optionnal
                            try
                            {
                                recv(4);
                                uint32_t reason_length = Parse(buf).in_uint32_be();

                                char   reason[256];
                                char * preason = reason;

                                this->t.recv_boom(preason,
                                                std::min<size_t>(sizeof(reason) - 1, reason_length));
                                preason += std::min<size_t>(sizeof(reason) - 1, reason_length);
                                *preason = 0;

                                LOG(LOG_INFO, "Reason for the connection failure: %s", preason);
                            }
                            catch (const Error &)
                            {
                            }

                            if (this->allow_authentification_retries)
                            {
                                LOG(LOG_ERR, "vnc password failed");

                                this->t.disconnect();

                                this->state = ASK_PASSWORD;
                                this->event.object_and_time = true;
                                this->event.set();

                                return;
                            }
                            else
                            {
                                LOG(LOG_ERR, "vnc password failed");
                                throw Error(ERR_VNC_CONNECTION_ERROR);
                            }
                        } else {
                            if (this->verbose) {
                                LOG(LOG_INFO, "vnc password ok\n");
                            }
                        }
                    }
                    break;
                    case -6: // MS-LOGON
                    {
                        LOG(LOG_INFO, "VNC MS-LOGON Auth");
                        uint8_t buf[8+8+8];
                        this->t.recv_boom(buf, sizeof(buf));
                        InStream stream(buf);
                        uint64_t gen = stream.in_uint64_be();
                        uint64_t mod = stream.in_uint64_be();
                        uint64_t resp = stream.in_uint64_be();
                        this->ms_logon(gen, mod, resp);
                    }
                    break;
                    case 0:
                    {
                        LOG(LOG_INFO, "VNC INVALID Auth");
                        uint8_t buf[8192];
                        this->t.recv_boom(buf, 4);
                        size_t reason_length = Parse(buf).in_uint32_be();
                        this->t.recv_boom(buf, reason_length);
                        hexdump_c(buf, reason_length);
                        throw Error(ERR_VNC_CONNECTION_ERROR);

                    }
                    default:
                        LOG(LOG_ERR, "vnc unexpected security level");
                        throw Error(ERR_VNC_CONNECTION_ERROR);
                }
                this->t.send("\x01", 1); // share flag

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

                {
                    uint8_t buf[24];
                    this->t.recv_boom(buf, sizeof(buf));  // server init

                    InStream stream(buf);
                    this->width = stream.in_uint16_be();
                    this->height = stream.in_uint16_be();
                    this->bpp    = stream.in_uint8();
                    this->depth  = stream.in_uint8();
                    this->endianess = stream.in_uint8();
                    this->true_color_flag = stream.in_uint8();
                    this->red_max = stream.in_uint16_be();
                    this->green_max = stream.in_uint16_be();
                    this->blue_max = stream.in_uint16_be();
                    this->red_shift = stream.in_uint8();
                    this->green_shift = stream.in_uint8();
                    this->blue_shift = stream.in_uint8();
                    stream.in_skip_bytes(3); // skip padding

                    // LOG(LOG_INFO, "VNC received: width=%d height=%d bpp=%d depth=%d endianess=%d true_color=%d red_max=%d green_max=%d blue_max=%d red_shift=%d green_shift=%d blue_shift=%d", this->width, this->height, this->bpp, this->depth, this->endianess, this->true_color_flag, this->red_max, this->green_max, this->blue_max, this->red_shift, this->green_shift, this->blue_shift);

                    int lg = stream.in_uint32_be();

                    if (lg > 255 || lg < 0) {
                        LOG(LOG_ERR, "VNC connection error");
                        throw Error(ERR_VNC_CONNECTION_ERROR);
                    }
                    this->t.recv_boom(this->mod_name, lg);
                    this->mod_name[lg] = 0;
                    // LOG(LOG_INFO, "VNC received: mod_name='%s'", this->mod_name);
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
                        if (this->verbose) {
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
                    if (this->verbose) {
                        LOG(LOG_INFO, "no resizing needed");
                    }
                    // no resizing needed
                    this->state = DO_INITIAL_CLEAR_SCREEN;
                    this->event.object_and_time = true;
                    this->event.set();
                    break;
                case FrontAPI::ResizeResult::no_need:
                    if (this->verbose) {
                        LOG(LOG_INFO, "no resizing needed");
                    }
                    // no resizing needed
                    this->state = DO_INITIAL_CLEAR_SCREEN;
                    this->event.object_and_time = true;
                    this->event.set();
                    break;
                case FrontAPI::ResizeResult::done:
                    if (this->verbose) {
                        LOG(LOG_INFO, "resizing done");
                    }
                    // resizing done
                    this->front_width  = this->width;
                    this->front_height = this->height;

                    this->state = WAIT_CLIENT_UP_AND_RUNNING;
                    this->event.object_and_time = true;

                    this->is_first_membelt = true;
                    break;
                case FrontAPI::ResizeResult::fail:
                    // resizing failed
                    // thow an Error ?
                    LOG(LOG_WARNING, "Older RDP client can't resize to server asked resolution, disconnecting");
                    throw Error(ERR_VNC_OLDER_RDP_CLIENT_CANT_RESIZE);
                }
            }
            break;
        case WAIT_CLIENT_UP_AND_RUNNING:
                LOG(LOG_WARNING, "Waiting for client be come up and running");
            break;
        default:
            LOG(LOG_ERR, "Unknown state=%d", this->state);
            throw Error(ERR_VNC);
        }

        if (this->event.waked_up_by_time) {
            this->event.reset();

            if (this->clipboard_requesting_for_data_is_delayed) {
                //const uint64_t usnow = ustime();
                //const uint64_t timeval_diff = usnow - this->clipboard_last_client_data_timestamp;
                //LOG(LOG_INFO,
                //    "usnow=%llu clipboard_last_client_data_timestamp=%llu timeval_diff=%llu",
                //    usnow, this->clipboard_last_client_data_timestamp, timeval_diff);
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_vnc server clipboard PDU: msgType=CB_FORMAT_DATA_REQUEST(%d) (time)",
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
    } // draw_event

private:
    struct ZRLEUpdateContext
    {
        uint8_t Bpp;

        uint16_t x;
        uint16_t cx;

        uint16_t cx_remain;
        uint16_t cy_remain;

        uint16_t tile_x;
        uint16_t tile_y;

        StaticOutStream<16384> data_remain;

        ZRLEUpdateContext() {}
    };

    void lib_framebuffer_update_zrle(InStream & uncompressed_data_buffer, ZRLEUpdateContext & update_context, gdi::GraphicApi & drawable)
    {
        uint8_t         tile_data[16384];    // max size with 16 bpp

        uint8_t const * remaining_data        = nullptr;
        uint16_t        remaining_data_length = 0;

        try
        {
            while (uncompressed_data_buffer.in_remain())
            {
                uint16_t tile_cx = std::min<uint16_t>(update_context.cx_remain, 64);
                uint16_t tile_cy = std::min<uint16_t>(update_context.cy_remain, 64);

                const uint8_t * tile_data_p = tile_data;

                uint16_t tile_data_length = tile_cx * tile_cy * update_context.Bpp;
                if (tile_data_length > sizeof(tile_data))
                {
                    LOG(LOG_ERR,
                        "VNC Encoding: ZRLE, tile buffer too small (%zu < %" PRIu16 ")",
                        sizeof(tile_data), tile_data_length);
                    throw Error(ERR_BUFFER_TOO_SMALL);
                }

                remaining_data        = uncompressed_data_buffer.get_current();
                remaining_data_length = uncompressed_data_buffer.in_remain();

                uint8_t   subencoding = uncompressed_data_buffer.in_uint8();

                if (this->verbose) {
                    LOG(LOG_INFO, "VNC Encoding: ZRLE, subencoding = %d",
                        subencoding);
                }

                if (!subencoding)
                {
                    if (this->verbose) {
                        LOG(LOG_INFO, "VNC Encoding: ZRLE, Raw pixel data");
                    }

                    if (uncompressed_data_buffer.in_remain() < tile_data_length)
                    {
                        throw Error(ERR_VNC_NEED_MORE_DATA);
                    }

                    tile_data_p = uncompressed_data_buffer.in_uint8p(tile_data_length);
                }
                else if (subencoding == 1)
                {
                    if (this->verbose) {
                        LOG(LOG_INFO,
                            "VNC Encoding: ZRLE, Solid tile (single color)");
                    }

                    if (uncompressed_data_buffer.in_remain() < update_context.Bpp)
                    {
                        throw Error(ERR_VNC_NEED_MORE_DATA);
                    }

                    const uint8_t * cpixel_pattern = uncompressed_data_buffer.in_uint8p(update_context.Bpp);

                    uint8_t * tmp_tile_data = tile_data;

                    for (int i = 0; i < tile_cx; i++, tmp_tile_data += update_context.Bpp)
                        memcpy(tmp_tile_data, cpixel_pattern, update_context.Bpp);

                    uint16_t line_size = tile_cx * update_context.Bpp;

                    for (int i = 1; i < tile_cy; i++, tmp_tile_data += line_size)
                        memcpy(tmp_tile_data, tile_data, line_size);
                }
                else if ((subencoding >= 2) && (subencoding <= 16))
                {
                    if (this->verbose) {
                        LOG(LOG_INFO,
                            "VNC Encoding: ZRLE, Packed palette types, "
                                "palette size=%d",
                            subencoding);
                    }

                    const uint8_t  * palette;
                    const uint8_t    palette_count = subencoding;
                    const uint16_t   palette_size  = palette_count * update_context.Bpp;

                    if (uncompressed_data_buffer.in_remain() < palette_size)
                    {
                        throw Error(ERR_VNC_NEED_MORE_DATA);
                    }

                    palette = uncompressed_data_buffer.in_uint8p(palette_size);

                    uint16_t   packed_pixels_length;

                    if (palette_count == 2)
                    {
                        packed_pixels_length = (tile_cx + 7) / 8 * tile_cy;
                    }
                    else if ((palette_count == 3) || (palette_count == 4))
                    {
                        packed_pixels_length = (tile_cx + 3) / 4 * tile_cy;
                    }
                    else// if ((palette_count >= 5) && (palette_count <= 16))
                    {
                        packed_pixels_length = (tile_cx + 1) / 2 * tile_cy;
                    }

                    if (uncompressed_data_buffer.in_remain() < packed_pixels_length)
                    {
                        throw Error(ERR_VNC_NEED_MORE_DATA);
                    }

                    const uint8_t * packed_pixels = uncompressed_data_buffer.in_uint8p(packed_pixels_length);

                    uint8_t * tmp_tile_data = tile_data;

                    uint16_t  tile_data_length_remain = tile_data_length;

                    uint8_t         pixel_remain         = tile_cx;
                    const uint8_t * packed_pixels_remain = packed_pixels;
                    uint8_t         current              = 0;
                    uint8_t         index                = 0;

                    uint8_t palette_index;

                    while (tile_data_length_remain >= update_context.Bpp)
                    {
                        pixel_remain--;

                        if (!index)
                        {
                            current = *packed_pixels_remain;
                            packed_pixels_remain++;
                        }

                        if (palette_count == 2)
                        {
                            palette_index = (current & 0x80) >> 7;
                            current <<= 1;
                            index++;

                            if (!pixel_remain || (index > 7))
                            {
                                index = 0;
                            }
                        }
                        else if ((palette_count == 3) || (palette_count == 4))
                        {
                            palette_index = (current & 0xC0) >> 6;
                            current <<= 2;
                            index++;

                            if (!pixel_remain || (index > 3))
                            {
                                index = 0;
                            }
                        }
                        else// if ((palette_count >= 5) && (palette_count <= 16))
                        {
                            palette_index = (current & 0xF0) >> 4;
                            current <<= 4;
                            index++;

                            if (!pixel_remain || (index > 1))
                            {
                                index = 0;
                            }
                        }

                        if (!pixel_remain)
                        {
                            pixel_remain = tile_cx;
                        }

                        const uint8_t * cpixel_pattern = palette + palette_index * update_context.Bpp;

                        memcpy(tmp_tile_data, cpixel_pattern, update_context.Bpp);

                        tmp_tile_data           += update_context.Bpp;
                        tile_data_length_remain -= update_context.Bpp;
                    }
                }
                else if ((subencoding >= 17) && (subencoding <= 127))
                {
                    LOG(LOG_ERR, "VNC Encoding: ZRLE, unused");
                    throw Error(ERR_VNC_ZRLE_PROTOCOL);
                }
                else if (subencoding == 128)
                {
                    if (this->verbose) {
                        LOG(LOG_INFO, "VNC Encoding: ZRLE, Plain RLE");
                    }

                    uint16_t   tile_data_length_remain = tile_data_length;

                    uint16_t   run_length    = 0;
                    uint8_t  * tmp_tile_data = tile_data;

                    while (tile_data_length_remain >= update_context.Bpp)
                    {

                        if (uncompressed_data_buffer.in_remain() < update_context.Bpp)
                        {
                            throw Error(ERR_VNC_NEED_MORE_DATA);
                        }

                        const uint8_t * cpixel_pattern = uncompressed_data_buffer.in_uint8p(update_context.Bpp);

                        run_length = 1;

                        while (true)
                        {
                            if (uncompressed_data_buffer.in_remain() < 1)
                            {
                                throw Error(ERR_VNC_NEED_MORE_DATA);
                            }

                            uint8_t byte_value = uncompressed_data_buffer.in_uint8();
                            run_length += byte_value;

                            if (byte_value != 255)
                                break;
                        }

                        // LOG(LOG_INFO, "VNC Encoding: ZRLE, run length=%u", run_length);

                        while ((tile_data_length_remain >= update_context.Bpp) && run_length)
                        {
                            memcpy(tmp_tile_data, cpixel_pattern, update_context.Bpp);

                            tmp_tile_data           += update_context.Bpp;
                            tile_data_length_remain -= update_context.Bpp;

                            run_length--;
                        }
                    }

                    // LOG(LOG_INFO, "VNC Encoding: ZRLE, run_length=%u", run_length);

                    REDASSERT(!run_length);
                    REDASSERT(!tile_data_length_remain);
                }
                else if (subencoding == 129)
                {
                    LOG(LOG_ERR, "VNC Encoding: ZRLE, unused");
                    throw Error(ERR_VNC_ZRLE_PROTOCOL);
                }
                else
                {
                    if (this->verbose) {
                        LOG(LOG_INFO, "VNC Encoding: ZRLE, Palette RLE");
                    }

                    const uint8_t  * palette;
                    const uint8_t    palette_count = subencoding - 128;
                    const uint16_t   palette_size  = palette_count * update_context.Bpp;

                    if (uncompressed_data_buffer.in_remain() < palette_size)
                    {
                        throw Error(ERR_VNC_NEED_MORE_DATA);
                    }

                    palette = uncompressed_data_buffer.in_uint8p(palette_size);

                    uint16_t   tile_data_length_remain = tile_data_length;

                    uint16_t   run_length    = 0;
                    uint8_t  * tmp_tile_data = tile_data;

                    while (tile_data_length_remain >= update_context.Bpp)
                    {
                        if (uncompressed_data_buffer.in_remain() < 1)
                        {
                            throw Error(ERR_VNC_NEED_MORE_DATA);
                        }

                        uint8_t         palette_index  = uncompressed_data_buffer.in_uint8();
                        const uint8_t * cpixel_pattern = palette + (palette_index & 0x7F) * update_context.Bpp;

                        run_length = 1;

                        if (palette_index & 0x80)
                        {
                            while (true)
                            {
                                if (uncompressed_data_buffer.in_remain() < 1)
                                {
                                    throw Error(ERR_VNC_NEED_MORE_DATA);
                                }

                                uint8_t byte_value = uncompressed_data_buffer.in_uint8();
                                run_length += byte_value;

                                if (byte_value != 255)
                                    break;
                            }
                        }

                        // LOG(LOG_INFO, "VNC Encoding: ZRLE, run length=%u", run_length);

                        while ((tile_data_length_remain >= update_context.Bpp) && run_length)
                        {
                            memcpy(tmp_tile_data, cpixel_pattern, update_context.Bpp);

                            tmp_tile_data           += update_context.Bpp;
                            tile_data_length_remain -= update_context.Bpp;

                            run_length--;
                        }
                    }

                    // LOG(LOG_INFO, "VNC Encoding: ZRLE, run_length=%u", run_length);

                    REDASSERT(!run_length);
                    REDASSERT(!tile_data_length_remain);
                }

                this->front.begin_update();
                this->draw_tile(Rect(update_context.tile_x, update_context.tile_y,
                                     tile_cx, tile_cy),
                                tile_data_p, drawable);
                this->front.end_update();

                update_context.cx_remain -= tile_cx;
                update_context.tile_x    += tile_cx;

                if (!update_context.cx_remain)
                {
                    update_context.cx_remain =  update_context.cx;
                    update_context.cy_remain -= tile_cy;

                    update_context.tile_x =  update_context.x;
                    update_context.tile_y += tile_cy;
                }
            }
        }
        catch (Error & e)
        {
            if (e.id != ERR_VNC_NEED_MORE_DATA)
                throw;
            else
            {
                update_context.data_remain.out_copy_bytes(remaining_data,
                    remaining_data_length);
            }
        }
    }
    //==============================================================================================================
    void lib_framebuffer_update(gdi::GraphicApi & drawable) {
    //==============================================================================================================
        uint8_t data_rec[256];
        InStream stream_rec(data_rec);
        uint8_t * end = data_rec;
        this->t.recv_boom(end, 3);
        end += 3;
        stream_rec.in_skip_bytes(1);
        size_t num_recs = stream_rec.in_uint16_be();

        uint8_t Bpp = nbbytes(this->bpp);
        for (size_t i = 0; i < num_recs; i++) {
            stream_rec = InStream(data_rec);
            end = data_rec;
            this->t.recv_boom(end, 12);
            end += 12;
            const uint16_t x = stream_rec.in_uint16_be();
            const uint16_t y = stream_rec.in_uint16_be();
            const uint16_t cx = stream_rec.in_uint16_be();
            const uint16_t cy = stream_rec.in_uint16_be();
            const uint32_t encoding = stream_rec.in_uint32_be();

            switch (encoding) {
            case 0: /* raw */
            {
                std::unique_ptr<uint8_t[]> raw(new(std::nothrow) uint8_t[cx * 16 * Bpp]);
                if (!raw) {
                    LOG(LOG_ERR, "Memory allocation failed for raw buffer in VNC");
                    throw Error(ERR_VNC_MEMORY_ALLOCATION_FAILED);
                }

                update_lock<FrontAPI> lock(this->front);
                for (uint16_t yy = y ; yy < y + cy ; yy += 16) {
                    uint8_t * tmp = raw.get();
                    uint16_t cyy = std::min<uint16_t>(16, cy-(yy-y));
                    this->t.recv_boom(tmp, cyy*cx*Bpp);
                    //LOG(LOG_INFO, "draw vnc: x=%d y=%d cx=%d cy=%d", x, yy, cx, cyy);
                    this->draw_tile(Rect(x, yy, cx, cyy), raw.get(), drawable);
                }
            }
            break;
            case 1: /* copy rect */
            {
                uint8_t data_copy_rect[4];
                InStream stream_copy_rect(data_copy_rect);
                uint8_t * end = data_copy_rect;
                this->t.recv_boom(end, 4);
                const int srcx = stream_copy_rect.in_uint16_be();
                const int srcy = stream_copy_rect.in_uint16_be();
                //LOG(LOG_INFO, "copy rect: x=%d y=%d cx=%d cy=%d encoding=%d src_x=%d, src_y=%d", x, y, cx, cy, encoding, srcx, srcy);
                const RDPScrBlt scrblt(Rect(x, y, cx, cy), 0xCC, srcx, srcy);
                update_lock<FrontAPI> lock(this->front);
                drawable.draw(scrblt, Rect(0, 0, this->front_width, this->front_height));
            }
            break;
            case 2: /* RRE */
            {
                //LOG(LOG_INFO, "VNC Encoding: RRE, Bpp = %u, x=%u, y=%u, cx=%u, cy=%u", Bpp, x, y, cx, cy);
                std::unique_ptr<uint8_t[]> raw(new(std::nothrow) uint8_t[cx * cy * Bpp]);
                if (!raw) {
                    LOG(LOG_ERR, "Memory allocation failed for RRE buffer in VNC");
                    throw Error(ERR_VNC_MEMORY_ALLOCATION_FAILED);
                }

                uint8_t data_rre[256];
                InStream stream_rre(data_rre);

                uint8_t * end = data_rre;
                this->t.recv_boom(end,
                      4   /* number-of-subrectangles */
                    + Bpp /* background-pixel-value */
                    );
                end += 4 + Bpp;

                uint32_t number_of_subrectangles_remain = stream_rre.in_uint32_be();

                uint8_t const * bytes_per_pixel = stream_rre.get_current();

                for (uint8_t * point_cur = raw.get(), * point_end = point_cur + cx * cy * Bpp;
                     point_cur < point_end; point_cur += Bpp) {
                    memcpy(point_cur, bytes_per_pixel, Bpp);
                }

                uint8_t    subrectangles_buf[65535];
                uint16_t   subrec_x, subrec_y, subrec_width, subrec_height;
                uint8_t  * point_line_cur;
                uint8_t  * point_line_end;
                uint32_t   i;
                uint32_t   ling_boundary;

                while (number_of_subrectangles_remain > 0) {
                    auto number_of_subrectangles_read = std::min<uint32_t>(4096, number_of_subrectangles_remain);

                    InStream subrectangles(subrectangles_buf);
                    end = subrectangles_buf;
                    this->t.recv_boom(end, (Bpp + 8) * number_of_subrectangles_read);

                    number_of_subrectangles_remain -= number_of_subrectangles_read;

                    for (i = 0; i < number_of_subrectangles_read; i++) {
                        bytes_per_pixel = subrectangles.get_current();
                        subrectangles.in_skip_bytes(Bpp);
                        subrec_x        = subrectangles.in_uint16_be();
                        subrec_y        = subrectangles.in_uint16_be();
                        subrec_width    = subrectangles.in_uint16_be();
                        subrec_height   = subrectangles.in_uint16_be();

                        ling_boundary = cx * Bpp;
                        point_line_cur = raw.get() + subrec_y * ling_boundary;
                        point_line_end = point_line_cur + subrec_height * ling_boundary;
                        for (; point_line_cur < point_line_end; point_line_cur += ling_boundary) {
                            for (uint8_t * point_cur = point_line_cur + subrec_x * Bpp,
                                 * point_end = point_cur + subrec_width * Bpp;
                                 point_cur < point_end; point_cur += Bpp) {
                                memcpy(point_cur, bytes_per_pixel, Bpp);
                            }
                        }
                    }
                }

                update_lock<FrontAPI> lock(this->front);
                this->draw_tile(Rect(x, y, cx, cy), raw.get(), drawable);
            }
            break;
            case 5: /* Hextile */
                LOG(LOG_INFO, "VNC Encoding: Hextile, Bpp = %u, x=%u, y=%u, cx=%u, cy=%u", Bpp, x, y, cx, cy);
            break;
            case 16:    /* ZRLE */
            {
                uint8_t data_zrle[4];
                uint8_t * end = data_zrle;

                //LOG(LOG_INFO, "VNC Encoding: ZRLE, Bpp = %u, x=%u, y=%u, cx=%u, cy=%u", Bpp, x, y, cx, cy);
                this->t.recv_boom(end, 4);

                uint32_t zlib_compressed_data_length = Parse(data_zrle).in_uint32_be();

                if (this->verbose)
                {
                    LOG(LOG_INFO, "VNC Encoding: ZRLE, compressed length = %u",
                        zlib_compressed_data_length);
                }

                if (zlib_compressed_data_length > 65536)
                {
                    LOG(LOG_ERR,
                        "VNC Encoding: ZRLE, compressed data buffer too small "
                            "(65536 < %" PRIu32 ")",
                        zlib_compressed_data_length);
                    throw Error(ERR_BUFFER_TOO_SMALL);
                }

                uint8_t zlib_compressed_data[65536];
                end = zlib_compressed_data;
                this->t.recv_boom(end, zlib_compressed_data_length);
                REDASSERT(end - zlib_compressed_data == 0);

                ZRLEUpdateContext zrle_update_context;

                zrle_update_context.Bpp       = Bpp;
                zrle_update_context.x         = x;
                zrle_update_context.cx        = cx;
                zrle_update_context.cx_remain = cx;
                zrle_update_context.cy_remain = cy;
                zrle_update_context.tile_x    = x;
                zrle_update_context.tile_y    = y;

                zstrm.avail_in = zlib_compressed_data_length;
                zstrm.next_in  = zlib_compressed_data;

                while (zstrm.avail_in > 0)
                {
                    constexpr std::size_t reserved_leading_space = 16384;
                    constexpr std::size_t total_size = 49152;
                    constexpr std::size_t data_size = total_size - reserved_leading_space;

                    uint8_t zlib_uncompressed_data_buffer[total_size];

                    uint8_t * data = zlib_uncompressed_data_buffer + reserved_leading_space;

                    zstrm.avail_out = data_size;
                    zstrm.next_out  = data;

                    int zlib_result = inflate(&zstrm, Z_NO_FLUSH);

                    if (zlib_result != Z_OK)
                    {
                        LOG(LOG_ERR, "vnc zlib decompression failed (%d)", zlib_result);
                        throw Error(ERR_VNC_ZLIB_INFLATE);
                    }

                    InStream zlib_uncompressed_data_stream(data, data_size - zstrm.avail_out);

                    if (this->verbose) {
                        LOG(LOG_INFO,
                            "VNC Encoding: ZRLE, uncompressed length=%lu remaining data size=%lu",
                            zlib_uncompressed_data_stream.in_remain(),
                            zrle_update_context.data_remain.get_offset());
                    }

                    if (zrle_update_context.data_remain.get_offset())
                    {
                        auto sz = zrle_update_context.data_remain.get_offset();
                        data -= sz;
                        memcpy(data, zrle_update_context.data_remain.get_data(), sz);
                        zlib_uncompressed_data_stream = InStream(data, data_size - zstrm.avail_out + sz);

                        zrle_update_context.data_remain.rewind();
                    }

                    this->lib_framebuffer_update_zrle(zlib_uncompressed_data_stream, zrle_update_context, drawable);
                }
            }
            break;
            case 0xffffff11: /* (-239) cursor */
            // TODO see why we get these empty rects ?
            if (cx > 0 && cy > 0) {
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

                const int sz_pixel_array = cx * cy * Bpp;
                const int sz_bitmask = nbbytes(cx) * cy;
                StreamBufMaker<65536> cursor_buf_maker;
                auto cursor_buf = cursor_buf_maker.reserve(sz_pixel_array + sz_bitmask);
                const uint8_t *vnc_pointer_data = cursor_buf;
                const uint8_t *vnc_pointer_mask = cursor_buf + sz_pixel_array;
                {
                    auto end = cursor_buf;
                    this->t.recv_boom(end, sz_pixel_array + sz_bitmask);
                }

                Pointer cursor;
                //LOG(LOG_INFO, "Cursor x=%u y=%u", x, y);
                cursor.x = x;
                cursor.y = y;
//                cursor.bpp = 24;
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
                                    int red   = (pixel >> this->red_shift) & red_max;
                                    int green = (pixel >> this->green_shift) & green_max;
                                    int blue  = (pixel >> this->blue_shift) & blue_max;
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
                this->front.begin_update();
                this->front.set_pointer(cursor);
                this->front.end_update();
            }
            break;
            default:
                LOG(LOG_ERR, "unexpected encoding %8x in lib_frame_buffer", encoding);
                throw Error(ERR_VNC_UNEXPECTED_ENCODING_IN_LIB_FRAME_BUFFER);
            }
        }

        this->update_screen(Rect(0, 0, this->width, this->height));
    } // lib_framebuffer_update

    //==============================================================================================================
    void lib_palette_update(gdi::GraphicApi & drawable) {
    //==============================================================================================================
        uint8_t buf[5];
        InStream stream(buf);
        {
            auto end = buf;
            this->t.recv_boom(end, 5);
        }
        stream.in_skip_bytes(1);
        int first_color = stream.in_uint16_be();
        int num_colors = stream.in_uint16_be();

        uint8_t buf2[8192];
        InStream stream2(buf2);
        {
            auto end = buf2;
            this->t.recv_boom(end, num_colors * 6);
        }

        if (num_colors <= 256) {
            for (int i = 0; i < num_colors; i++) {
                const int b = stream2.in_uint16_be() >> 8;
                const int g = stream2.in_uint16_be() >> 8;
                const int r = stream2.in_uint16_be() >> 8;
                this->palette.set_color(first_color + i, BGRColor(b, g, r));
            }
        }
        else {
            LOG(LOG_ERR, "VNC: number of palette colors too large: %d\n", num_colors);
        }

        this->front.set_palette(this->palette);
        this->front.begin_update();
        RDPColCache cmd(0, this->palette);
        drawable.draw(cmd);
        this->front.end_update();
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
    const CHANNELS::ChannelDef * get_channel_by_name(const char * channel_name) const {
    //==============================================================================================================
        return this->front.get_channel_list().get_by_name(channel_name);
    } // get_channel_by_name

    //******************************************************************************
    // Entry point for VNC server clipboard content reception
    // Conversion to RDP behaviour :
    //  - store this content in a buffer, waiting for an explicit request from the front
    //  - send a notification to the front (Format List PDU) that the server clipboard
    //    status has changed
    //******************************************************************************
    //==============================================================================================================
    void lib_clip_data(void) {
        this->to_rdp_clipboard_data = InStream(this->to_rdp_clipboard_data_buffer);
        {
            auto end = this->to_rdp_clipboard_data_buffer;
            this->t.recv_boom(end, 7);
        }
        this->to_rdp_clipboard_data.in_skip_bytes(3);   // padding(3)
        const uint32_t clipboard_data_length =          // length(4)
            this->to_rdp_clipboard_data.in_uint32_be();
        if (this->verbose) {
            LOG(LOG_INFO, "mod_vnc::lib_clip_data: clipboard_data_length=%u", clipboard_data_length);
        }

        uint32_t remaining_clipboard_data_length = clipboard_data_length;

        const bool clipboard_down_is_really_enabled =
            (this->enable_clipboard_down && this->get_channel_by_name(channel_names::cliprdr));
        if (clipboard_down_is_really_enabled) {
            this->to_rdp_clipboard_data = InStream(this->to_rdp_clipboard_data_buffer);

            if (clipboard_data_length < this->to_rdp_clipboard_data.get_capacity()) {
                auto end = this->to_rdp_clipboard_data_buffer;
                this->t.recv_boom(end, clipboard_data_length);  // Clipboard data.
                end += clipboard_data_length;
                *end++ = '\0';  // Null character.
                this->to_rdp_clipboard_data.in_skip_bytes(end - this->to_rdp_clipboard_data.get_data());

                remaining_clipboard_data_length = 0;

                this->to_rdp_clipboard_data_is_utf8_encoded =
                    ::is_utf8_string(this->to_rdp_clipboard_data.get_data(), clipboard_data_length);
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_vnc::lib_clip_data: to_rdp_clipboard_data_is_utf8_encoded=%s",
                        (this->to_rdp_clipboard_data_is_utf8_encoded ? "yes" : "no"));
                    if (clipboard_data_length <= 64) {
                        hexdump_c(this->to_rdp_clipboard_data.get_data(), clipboard_data_length);
                    }
                }
            }
            else {
                this->to_rdp_clipboard_data.in_skip_bytes(
                    ::snprintf(::char_ptr_cast(this->to_rdp_clipboard_data_buffer),
                               this->to_rdp_clipboard_data.get_capacity(),
                               "The text was too long to fit in the clipboard buffer. "
                                   "The buffer size is limited to %u bytes.",
                               static_cast<uint32_t>(this->to_rdp_clipboard_data.get_capacity())) +
                    1   // Null character.
                );

                this->to_rdp_clipboard_data_is_utf8_encoded = true;
            }
        }

        while (remaining_clipboard_data_length) {
            char drop[4096];

            char * end = drop;

            const uint32_t number_of_bytes_to_read =
                std::min<uint32_t>(remaining_clipboard_data_length, sizeof(drop));

            this->t.recv_boom(end, sizeof(number_of_bytes_to_read));
            remaining_clipboard_data_length -= number_of_bytes_to_read;
        }

        if (clipboard_down_is_really_enabled) {
            if (this->verbose) {
                LOG(LOG_INFO,
                    "mod_vnc::lib_clip_data: Sending Format List PDU (%d) to client.",
                    RDPECLIP::CB_FORMAT_LIST);
            }

            RDPECLIP::FormatListPDU format_list_pdu;
            StaticOutStream<256>    out_s;

            const bool unicodetext = this->to_rdp_clipboard_data_is_utf8_encoded;

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

            this->clipboard_owned_by_client = false;

            // Can stop RDP to VNC clipboard infinite loop.
            this->clipboard_requesting_for_data_is_delayed = false;
        }
        else {
            LOG(LOG_INFO, "mod_vnc::lib_clip_data: Clipboard Channel Redirection unavailable");
        }
    } // lib_clip_data

    //==============================================================================================================
    void send_to_mod_channel( const char * const front_channel_name
                                    , InStream & chunk
                                    , size_t length
                                    , uint32_t flags) override {
    //==============================================================================================================
        if (this->verbose) {
            LOG(LOG_INFO, "mod_vnc::send_to_mod_channel");
        }

        if (this->state != UP_AND_RUNNING) {
            return;
        }

        if (!::strcasecmp(front_channel_name, channel_names::cliprdr)) {
            this->clipboard_send_to_vnc(chunk, length, flags);
        }
        if (this->verbose) {
            LOG(LOG_INFO, "mod_vnc::send_to_mod_channel done");
        }
    } // send_to_mod_channel

private:
    void clipboard_send_to_vnc(InStream & chunk, size_t length, uint32_t flags)
    {
        if (this->verbose) {
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

        if (this->verbose) {
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

                if (this->verbose) {
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
                        if (this->verbose) {
                            LOG(LOG_INFO,
                                "mod_vnc server clipboard PDU: msgType=CB_FORMAT_DATA_REQUEST(%d)",
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
                            if (this->verbose) {
                                LOG(LOG_INFO,
                                    "mod_vnc server clipboard PDU: msgType=CB_FORMAT_DATA_REQUEST(%d) (delayed)",
                                    RDPECLIP::CB_FORMAT_DATA_REQUEST);
                            }
                            this->event.object_and_time = true;
                            this->event.set(MINIMUM_TIMEVAL - timeval_diff);

                            this->clipboard_requesting_for_data_is_delayed = true;
                        }
                        else if (this->bogus_clipboard_infinite_loop
                            != VncBogusClipboardInfiniteLoop::duplicated
                        && (this->clipboard_general_capability_flags
                            & RDPECLIP::CB_ALL_GENERAL_CAPABILITY_FLAGS)) {
                            if (this->verbose) {
                                LOG( LOG_INFO
                                   , "mod_vnc::clipboard_send_to_vnc: "
                                     "duplicated clipboard update event "
                                     "from Windows client is ignored"
                                   );
                            }
                        }
                        else {
                            if (this->verbose) {
                                LOG(LOG_INFO,
                                    "mod_vnc server clipboard PDU: msgType=CB_FORMAT_LIST(%d) (preventive)",
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
                       , "mod_vnc::clipboard_send_to_vnc: truncated CB_FORMAT_DATA_REQUEST(%d) data, need=%u remains=%zu"
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

                if (this->verbose) {
                    LOG( LOG_INFO
                       , "mod_vnc::clipboard_send_to_vnc: CB_FORMAT_DATA_REQUEST(%d) msgFlags=0x%02x datalen=%u requestedFormatId=%s(%u)"
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
                        if (this->verbose) {
                            LOG(LOG_INFO,
                                "mod_vnc server clipboard PDU: msgType=CB_FORMAT_DATA_RESPONSE(%d) - chunk_size=%u",
                                RDPECLIP::CB_FORMAT_DATA_RESPONSE,
                                static_cast<uint32_t>(chunk_size));
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
                    !this->to_rdp_clipboard_data_is_utf8_encoded) {
                    StreamBufMaker<65536> buf_maker;
                    OutStream out_stream = buf_maker.reserve_out_stream(
                            8 +                                         // clipHeader(8)
                            this->to_rdp_clipboard_data.get_offset() * 2 +    // data
                            1                                           // Null character
                        );

                    OutStream out_data_stream(
                        out_stream.get_data() + 8 /* clipHeader(8) */,
                        out_stream.get_capacity() - 8 /* clipHeader(8) */
                    );

                    const size_t to_rdp_clipboard_data_length =
                        ::linux_to_windows_newline_convert(
                                ::char_ptr_cast(this->to_rdp_clipboard_data.get_data()),
                                this->to_rdp_clipboard_data.get_offset(),
                                ::char_ptr_cast(out_data_stream.get_data()),
                                out_data_stream.get_capacity()
                            );
                    out_data_stream.out_skip_bytes(to_rdp_clipboard_data_length);

                    const bool response_ok = true;
                    const RDPECLIP::FormatDataResponsePDU format_data_response_pdu(response_ok);

                    format_data_response_pdu.emit_ex(out_stream, out_data_stream.get_offset());
                    out_stream.out_skip_bytes(out_data_stream.get_offset());

                    send_format_data_response(out_stream);

                    if (this->verbose) {
                        LOG(LOG_INFO,
                            "mod_vnc::clipboard_send_to_vnc: "
                                "Sending Format Data Response PDU (CF_TEXT) done");
                    }
                }
                else if (format_data_request_pdu.requestedFormatId == RDPECLIP::CF_UNICODETEXT) {
                    StreamBufMaker<65536> buf_maker;
                    OutStream out_stream = buf_maker.reserve_out_stream(
                            8 +                                         // clipHeader(8)
                            this->to_rdp_clipboard_data.get_offset() * 4 +    // data
                            1                                           // Null character
                        );

                    OutStream out_data_stream(
                            out_stream.get_data() + 8 /* clipHeader(8) */,
                            out_stream.get_capacity() - 8 /* clipHeader(8) */
                        );

                    size_t utf16_data_length;
                    if (this->to_rdp_clipboard_data_is_utf8_encoded) {
                        utf16_data_length = UTF8toUTF16_CrLf(
                                this->to_rdp_clipboard_data.get_data(),
                                out_data_stream.get_data(),
                                out_data_stream.get_capacity()
                            );
                    }
                    else {
                        utf16_data_length = Latin1toUTF16(
                                this->to_rdp_clipboard_data.get_data(),
                                this->to_rdp_clipboard_data.get_offset(),
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

                    if (this->verbose) {
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
                               , "mod_vnc::clipboard_send_to_vnc: truncated CB_FORMAT_DATA_RESPONSE(%d), need=%" PRIu32 " remains=%zu"
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

                        if (this->verbose) {
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
                                               "The buffer size is limited to %u bytes. "
                                               "The length of data is %u bytes.",
                                           static_cast<uint32_t>(this->to_vnc_clipboard_data.get_capacity()),
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
                REDASSERT(this->to_vnc_clipboard_data.get_offset() != 0);
                REDASSERT(this->to_vnc_clipboard_data_size);

                // Virtual channel data span in multiple Virtual Channel PDUs.
                if (this->verbose) {
                    LOG(LOG_INFO, "mod_vnc::clipboard_send_to_vnc: an other trunk");
                }

                if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) != 0) {
                    LOG(LOG_ERR, "mod_vnc::clipboard_send_to_vnc: flag CHANNEL_FLAG_FIRST unexpected");
                    throw Error(ERR_VNC);
                }

                // if (this->verbose) {
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
                    REDASSERT((this->to_vnc_clipboard_data.get_capacity() < this->to_vnc_clipboard_data_size) ||
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
                    if (this->verbose) {
                        LOG(LOG_INFO, "Client use %s format name",
                            (this->client_use_long_format_names ? "long" : "short"));
                    }

                    if (this->verbose) {
                        general_caps.log(LOG_INFO);
                    }
                }
            }
            break;
        }
        if (this->verbose) {
            LOG(LOG_INFO, "mod_vnc::clipboard_send_to_vnc: done");
        }
    } // clipboard_send_to_vnc

    // Front calls this member function when it became up and running.
public:
    void rdp_input_up_and_running() override {
        if (this->state == WAIT_CLIENT_UP_AND_RUNNING) {
            if (this->verbose) {
                LOG(LOG_INFO, "Client up and running");
            }
            this->state = DO_INITIAL_CLEAR_SCREEN;
            this->event.set();
        }
    }

    void notify(Widget2* sender, notify_event_t event) override {
        (void)sender;
        switch (event) {
        case NOTIFY_SUBMIT:
            this->screen.clear();

            memset(this->password, 0, sizeof(this->password));
            strncpy(this->password, this->challenge.password_edit.get_text(),
                    sizeof(this->password) - 1);
            this->password[sizeof(this->password) - 1] = 0;

            this->state = RETRY_CONNECTION;
            this->event.set();
            break;
        case NOTIFY_CANCEL:
            this->event.signal = BACK_EVENT_NEXT;
            this->event.set();

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
                /// NOTE force resize cliping with rdesktop...
                if (this->is_first_membelt && dst_tile.cx != 1 && dst_tile.cy != 1) {
                    drawable.draw(cmd2, Rect(dst_tile.x,dst_tile.y,1,1), tiled_bmp);
                    this->is_first_membelt = false;
                }
                drawable.draw(cmd2, dst_tile, tiled_bmp);
            }
        }
    }

public:
    void disconnect(time_t now) override {

        double seconds = ::difftime(now, this->beginning);
        LOG(LOG_INFO, "Client disconnect");

        char extra[1024];
        snprintf(extra, sizeof(extra), "duration=\"%02d:%02d:%02d\"",
                        (int(seconds) / 3600),
                        ((int(seconds) % 3600) / 60),
                        (int(seconds) % 60));

        this->report_message.log4(false, "SESSION_DISCONNECTION", extra);
    }

    Dimension get_dim() const override
    { return Dimension(this->width, this->height); }
};

