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
   Author(s): Christophe Grosjean, Javier Caverni, Clément Moroldo, David Fort
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Vnc module
*/

#include "mod/vnc/vnc.hpp"
#include <openssl/tls1.h>
#include "acl/gd_provider.hpp"
#include <openssl/tls1.h>



#ifndef __EMSCRIPTEN__
# define IF_ENABLE_METRICS(m) do { if (this->metrics) this->metrics->m; } while (0)
#else
# define IF_ENABLE_METRICS(m) do {} while(0)
#endif

mod_vnc::mod_vnc( Transport & t
           , TimeBase& time_base
           , GdProvider & gd_provider
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
           , AuthApi & sesman
           )
    : front(front)
    , t(VncTransport(*this, t))
    , dsm(nullptr)
    , dsmEncryption(false)
    , width(front_width)
    , height(front_height)
    , verbose(verbose /*| VNCVerbose::basic_trace | VNCVerbose::connection*/)
    , keymapSym(keylayout, key_flags, server_is_unix, server_is_macos, static_cast<uint32_t>(verbose & VNCVerbose::keymap))
    , enable_clipboard_up(clipboard_up)
    , enable_clipboard_down(clipboard_down)
    , encodings(encodings)
    , clipboard_server_encoding_type(clipboard_server_encoding_type)
    , bogus_clipboard_infinite_loop(bogus_clipboard_infinite_loop)
    , rail_client_execute(rail_client_execute)
    , time_base(time_base)
    , gd_provider(gd_provider)
    , events(events)
#ifndef __EMSCRIPTEN__
    , metrics(metrics)
    , sesman(sesman)
#endif
    , choosenAuth(VNC_AUTH_INVALID)
    , cursor_pseudo_encoding_supported(cursor_pseudo_encoding_supported)
    , server_data_buf(*this)
    , tlsSwitch(false)
    , frame_buffer_update_ctx(this->zd, verbose)
    , clipboard_data_ctx(verbose)
{
    LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "Creation of new mod 'VNC'");

    ::time(&this->beginning);

    // TODO init layout sym with macos layout

    std::snprintf(this->username, sizeof(this->username), "%s", username);
    std::snprintf(this->password, sizeof(this->password), "%s", password);

    this->events.create_event_timeout(
        "VNC Init Event", this,
        this->time_base.get_current_time(),
        [this](Event&event)
        {
            // First Timeout Clear Screen
            gdi_clear_screen(this->gd_provider.get_graphics(), this->get_dim());
            // Following fd timeouts
            event.rename("VNC Fd Event");
            event.alarm.set_fd(this->t.get_fd(), std::chrono::seconds{300});
            event.alarm.set_timeout(this->time_base.get_current_time()+std::chrono::seconds{300});
            event.actions.set_timeout_function([](Event&/*event*/){});
            event.actions.set_action_function([this](Event&/*ebent*/)
            {
                this->draw_event(this->gd_provider.get_graphics());
            });
        });
}

bool mod_vnc::ms_logon(Buf64k & buf)
{
    if (!this->ms_logon_ctx.run(buf)) {
        return false;
    }

    if (bool(this->verbose & VNCVerbose::basic_trace)) {
        LOG(LOG_INFO, "MS-Logon with following values:");
        LOG(LOG_INFO, "Gen=0x%" PRIx64, this->ms_logon_ctx.gen);
        LOG(LOG_INFO, "Mod=0x%" PRIx64, this->ms_logon_ctx.mod);
        LOG(LOG_INFO, "Resp=0x%" PRIx64, this->ms_logon_ctx.resp);
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

    this->t.send(out_stream.get_produced_bytes());
    IF_ENABLE_METRICS(data_from_client(out_stream.get_offset()));
    // sec result

    return true;
}

void mod_vnc::initial_clear_screen(gdi::GraphicApi & drawable)
{
    LOG_IF(bool(this->verbose & VNCVerbose::connection), LOG_INFO, "state=DO_INITIAL_CLEAR_SCREEN");

    // set almost null cursor, this is the little dot cursor
    drawable.set_pointer(0, dot_pointer(), gdi::GraphicApi::SetPointerMode::Insert);

    this->sesman.log6(LogId::SESSION_ESTABLISHED_SUCCESSFULLY, {});

    Rect const screen_rect(0, 0, this->width, this->height);

    drawable.begin_update();
    RDPOpaqueRect orect(screen_rect, RDPColor{});
    drawable.draw(orect, screen_rect, gdi::ColorCtx::from_bpp(this->bpp, this->palette_update_ctx.get_palette()));
    drawable.end_update();

    this->state = UP_AND_RUNNING;
    this->front.can_be_start_capture(false);

    this->update_screen(screen_rect, 1);
    this->lib_open_clip_channel();

    LOG_IF(bool(this->verbose & VNCVerbose::connection), LOG_INFO, "VNC screen cleaning ok");

    RDPECLIP::GeneralCapabilitySet general_caps(
        RDPECLIP::CB_CAPS_VERSION_1,
        (this->server_use_long_format_names ? RDPECLIP::CB_USE_LONG_FORMAT_NAMES : 0));

    LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
        "Server use %s format name", this->server_use_long_format_names ? "long":"short");

    RDPECLIP::ClipboardCapabilitiesPDU clip_cap_pdu(1);

    RDPECLIP::CliprdrHeader clip_header(RDPECLIP::CB_CLIP_CAPS, 0,
        clip_cap_pdu.size() + general_caps.size());

    StaticOutStream<1024> out_s;

    clip_header.emit(out_s);
    clip_cap_pdu.emit(out_s);
    general_caps.emit(out_s);

    size_t length     = out_s.get_offset();
    size_t chunk_size = length;

    LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
        "mod_vnc server clipboard PDU: msgType=%s(%d)",
        RDPECLIP::get_msgType_name(clip_header.msgType()),
        clip_header.msgType());

    this->send_to_front_channel( channel_names::cliprdr
                               , out_s.get_data()
                               , length
                               , chunk_size
                               ,   CHANNELS::CHANNEL_FLAG_FIRST
                                 | CHANNELS::CHANNEL_FLAG_LAST
                               );

    RDPECLIP::ServerMonitorReadyPDU server_monitor_ready_pdu;
    RDPECLIP::CliprdrHeader         header(RDPECLIP::CB_MONITOR_READY, RDPECLIP::CB_RESPONSE__NONE_, server_monitor_ready_pdu.size());

    out_s.rewind();

    header.emit(out_s);
    server_monitor_ready_pdu.emit(out_s);

    length     = out_s.get_offset();
    chunk_size = length;

    LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
        "mod_vnc server clipboard PDU: msgType=%s(%d)",
        RDPECLIP::get_msgType_name(header.msgType()),
        header.msgType());

    this->send_to_front_channel( channel_names::cliprdr
                               , out_s.get_data()
                               , length
                               , chunk_size
                               ,   CHANNELS::CHANNEL_FLAG_FIRST
                                 | CHANNELS::CHANNEL_FLAG_LAST
                               );
}

void mod_vnc::rdp_input_mouse( int device_flags, int x, int y, Keymap2 * /*keymap*/ )
{
    if (this->state != UP_AND_RUNNING) {
        return;
    }

    StaticOutStream<32> out_stream;

    if (device_flags & MOUSE_FLAG_MOVE) {
        this->mouse.move(out_stream, x, y);
        IF_ENABLE_METRICS(mouse_move(x, y));
    }
    else if (device_flags & MOUSE_FLAG_BUTTON1) {
        this->mouse.click(out_stream, x, y, 1 << 0, device_flags & MOUSE_FLAG_DOWN);
        IF_ENABLE_METRICS(right_click());
    }
    else if (device_flags & MOUSE_FLAG_BUTTON2) {
        this->mouse.click(out_stream, x, y, 1 << 2, device_flags & MOUSE_FLAG_DOWN);
        IF_ENABLE_METRICS(left_click());
    }
    else if (device_flags & MOUSE_FLAG_BUTTON3) {
        this->mouse.click(out_stream, x, y, 1 << 1, device_flags & MOUSE_FLAG_DOWN);
    }
    else if (device_flags == MOUSE_FLAG_BUTTON4 || device_flags == 0x0278) {
        this->mouse.scroll(out_stream, 1 << 3);
    }
    else if (device_flags == MOUSE_FLAG_BUTTON5 || device_flags == 0x0388) {
        this->mouse.scroll(out_stream, 1 << 4);
    }
    else {
        return ;
    }

    this->t.send(out_stream.get_produced_bytes());
}

void mod_vnc::rdp_input_scancode(long keycode, long /*param2*/, long device_flags, long /*param4*/,
        Keymap2 * /*keymap*/)
{
    if (this->state != UP_AND_RUNNING) {
        return;
    }

    // AltGr or Alt are catched by Mac OS

    LOG_IF(bool(this->verbose & VNCVerbose::keymap_stack), LOG_INFO, "mod_vnc::rdp_input_scancode(device_flags=%ld, keycode=%ld)", device_flags, keycode);

    IF_ENABLE_METRICS(key_pressed());

    keymapSym.event(device_flags, keycode);

    uint8_t downflag = 0;
    while (uint32_t key = this->keymapSym.get_sym(downflag)){
        if (bool(this->verbose & VNCVerbose::keymap_stack)) {
            LOG(LOG_INFO, "keyloop::key=%u (%x) %s param1=%u nbsym=%u",
                key, key, downflag?"DOWN":"UP",
                static_cast<unsigned>(keycode),
                this->keymapSym.nb_sym_available());
        }
        this->send_keyevent(downflag, key);
    }
}

void mod_vnc::rdp_input_unicode(uint16_t /*unicode*/, uint16_t /*flag*/)  {
    LOG(LOG_WARNING, "mod_vnc::rdp_input_unicode: Unicode Keyboard Event is not yet supported");
}

void mod_vnc::send_keyevent(uint8_t down_flag, uint32_t key) {
    LOG_IF(bool(this->verbose & VNCVerbose::keymap_stack), LOG_INFO, "VNC Send KeyEvent Flag down: %d, key: 0x%x", down_flag, key);
    StaticOutStream<8> stream;
    stream.out_uint8(4);
    stream.out_uint8(down_flag); /* down/up flag */
    stream.out_clear_bytes(2);
    stream.out_uint32_be(key);
    this->t.send(stream.get_produced_bytes());
    IF_ENABLE_METRICS(data_from_client(stream.get_offset()));

}

void mod_vnc::rdp_input_clip_data(bytes_view data)
{
    if (this->state == UP_AND_RUNNING) {
        auto client_cut_text = [this](std::size_t n, auto f){
            StreamBufMaker<65536> buf_maker;
            OutStream stream = buf_maker.reserve_out_stream(n + 8u);
            auto str = f(stream.get_tail().from_offset(8));

            stream.out_uint8(6);                // message-type : ClientCutText
            stream.out_clear_bytes(3);          // padding
            stream.out_uint32_be(str.size());   // length
            stream.out_skip_bytes(str.size());  // text

            this->t.send(stream.get_produced_bytes());
            IF_ENABLE_METRICS(data_from_client(stream.get_offset()));
            IF_ENABLE_METRICS(clipboard_data_from_client(this->to_vnc_clipboard_data.get_offset()));
        };

        if (this->clipboard_requested_format_id == RDPECLIP::CF_UNICODETEXT) {
            if (this->clipboard_server_encoding_type == ClipboardEncodingType::UTF8) {
                LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
                    "mod_vnc::rdp_input_clip_data: CF_UNICODETEXT -> UTF-8");

                const size_t utf8_data_length =
                    data.size() / sizeof(uint16_t) * maximum_length_of_utf8_character_in_bytes + 1;

                client_cut_text(utf8_data_length, [&](writable_bytes_view utf8_data){
                    const auto len = ::UTF16toUTF8(
                        data.data(), data.size(), utf8_data.data(), utf8_data_length);
                    return utf8_data.first(len);
                });
            }
            else {
                LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
                    "mod_vnc::rdp_input_clip_data: CF_UNICODETEXT -> Latin-1");

                const size_t latin1_data_length = data.size() / sizeof(uint16_t) + 1;
                client_cut_text(latin1_data_length, [&](writable_bytes_view latin1_data){
                    const auto len = ::UTF16toLatin1(
                        data.data(), data.size(), latin1_data.data(), latin1_data_length);
                    return latin1_data.first(len);
                });
            }
        }
        else {
            if (this->clipboard_server_encoding_type == ClipboardEncodingType::UTF8) {
                LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
                    "mod_vnc::rdp_input_clip_data: CF_TEXT -> UTF-8");

                const size_t utf8_data_length = data.size() * 2 + 1;
                client_cut_text(utf8_data_length, [&](writable_bytes_view utf8_data){
                    const size_t len = ::Latin1toUTF8(
                        data.data(), data.size(), utf8_data.data(), utf8_data_length);
                    return utf8_data.first(len);
                });
            }
            else {
                LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
                    "mod_vnc::rdp_input_clip_data: CF_TEXT -> Latin-1");

                client_cut_text(data.size(), [&](writable_bytes_view utf8_data){
                    memcpy(utf8_data.data(), data.data(), data.size());
                    return utf8_data;
                });
            }
        }
    }

    this->clipboard_owned_by_client = true;

    this->clipboard_last_client_data_timestamp = ustime();
}


void mod_vnc::rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
{
    LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO,
        "KeymapSym::synchronize(time=%u, device_flags=%08x, param1=%04x, param1=%04x",
        time, device_flags, unsigned(param1), unsigned(param2));
    this->keymapSym.synchronize(param1);
}

void mod_vnc::update_screen(Rect r, uint8_t incr) {
    StaticOutStream<10> stream;
    /* FramebufferUpdateRequest */
    stream.out_uint8(VNC_CS_MSG_FRAME_BUFFER_UPDATE_REQUEST);
    stream.out_uint8(incr);
    stream.out_uint16_be(r.x);
    stream.out_uint16_be(r.y);
    stream.out_uint16_be(r.cx);
    stream.out_uint16_be(r.cy);
    this->t.send(stream.get_produced_bytes());
    IF_ENABLE_METRICS(data_from_client(stream.get_offset()));
}

void mod_vnc::rdp_input_invalidate(Rect r) {
    LOG(LOG_INFO, "mod_vnc::rdp_input_invalidate");
    if (this->state != UP_AND_RUNNING) {
        LOG(LOG_INFO, "mod_vnc::rdp_input_invalidate not up and running");
        return;
    }

    Rect r_ = r.intersect(Rect(0, 0, this->width, this->height));

    if (!r_.isempty()) {
        this->update_screen(r_, 0);
    }
    else {
        LOG(LOG_INFO, "mod_vnc::rdp_input_invalidate empty rect");
    }
}


bool mod_vnc::doTlsSwitch()
{
    TLSClientParams tlsParams;

    switch(this->choosenAuth) {
    case VeNCRYPT_TLSNone:
    case VeNCRYPT_TLSPlain:
    case VeNCRYPT_TLSVnc:
        /* needed params for anonymous TLS */
        tlsParams.security_level = 0;
        tlsParams.tls_max_level = TLS1_2_VERSION;
        tlsParams.cipher_string = "ADH";
        tlsParams.anonymous_tls = true;
        break;
    default:
        break;
    }
    NullServerNotifier notifier;

    switch (this->t.enable_client_tls(notifier, tlsParams)) {
        case Transport::TlsResult::WaitExternalEvent:
        case Transport::TlsResult::Want:
            return false;
        case Transport::TlsResult::Fail:
            LOG(LOG_ERR, "mod_vnc::enable_client_tls fail");
            throw Error(ERR_VNC_CONNECTION_ERROR);
        case Transport::TlsResult::Ok:
            return true;
        default:
            LOG(LOG_ERR, "unexpected result");
            throw Error(ERR_VNC_CONNECTION_ERROR);
    }
}


void mod_vnc::draw_event(gdi::GraphicApi & gd)
{
    bool can_read = true;

    LOG_IF(bool(this->verbose & VNCVerbose::draw_event), LOG_INFO, "vnc::draw_event");

    if (this->tlsSwitch) {
        if (this->doTlsSwitch()) {
            this->tlsSwitch = false;
            can_read = true;

            switch(this->choosenAuth) {
            case VeNCRYPT_TLSNone:
            case VeNCRYPT_X509None:
                this->state = WAIT_SECURITY_RESULT;
                break;
            case VeNCRYPT_TLSPlain:
            case VeNCRYPT_X509Plain: {
                StaticOutStream<4 + 4 + 256 + 256> ostream;

                ostream.out_uint32_be(strlen(this->username));
                ostream.out_uint32_be(strlen(this->password));
                ostream.out_copy_bytes(this->username, strlen(this->username));
                ostream.out_copy_bytes(this->password, strlen(this->password));

                this->t.send(ostream.get_data(), ostream.get_offset());
                this->state = WAIT_SECURITY_RESULT;
                break;
            }
            case VeNCRYPT_TLSVnc:
            case VeNCRYPT_X509Vnc:
                this->state = WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM;
                break;
            default:
                LOG(LOG_ERR, "auth %d not handled yet", this->choosenAuth);
                break;
            }
        } else {
            can_read = false;
        }

    }

    if (can_read) {
        this->server_data_buf.read_from(this->t);
    }

    [[maybe_unused]]
    uint64_t const data_server_before = this->server_data_buf.remaining();

    while (this->draw_event_impl(gd) && !this->tlsSwitch) {
    }

    uint64_t const data_server_after = this->server_data_buf.remaining();
    IF_ENABLE_METRICS(data_from_server(data_server_before - data_server_after));

    LOG_IF(bool(this->verbose & VNCVerbose::draw_event), LOG_INFO, "Remaining in buffer : %" PRIu64, data_server_after);

    this->check_timeout();

}

const char *mod_vnc::securityTypeString(uint32_t t) {
    static char format[] = "<unknown 0xXXXXXXXX>";

    switch(t) {
    case VNC_AUTH_INVALID: return "invalid";
    case VNC_AUTH_NONE: return "None";
    case VNC_AUTH_VNC: return "VNC";
    case VNC_AUTH_ULTRA: return "Ultra";
    case VNC_AUTH_TIGHT: return "TightVNC";
    case VNC_AUTH_ULTRA_MsLogonIAuth: return "Ultra MsLogonIAuth";
    case VNC_AUTH_ULTRA_MsLogonIIAuth: return "Ultra MsLogon2Auth";
    case VNC_AUTH_ULTRA_SecureVNCPluginAuth: return "UtraVNC DSM old";
    case VNC_AUTH_ULTRA_SecureVNCPluginAuth_new: return "UtraVNC DSM new";
    case VNC_AUTH_TLS: return "TLS";
    case VNC_AUTH_ULTRA_MS_LOGON: return "Ultra MS-logon";
    case VNC_AUTH_VENCRYPT: return "VeNCrypt";
    case VeNCRYPT_TLSNone: return "TLS none";
    case VeNCRYPT_TLSVnc: return "TLS VNC";
    case VeNCRYPT_TLSPlain: return "TLS plain";
    case VeNCRYPT_X509None: return "X509 none";
    case VeNCRYPT_X509Vnc: return "X509 VNC";
    case VeNCRYPT_X509Plain: return "X509 plain";
    default:
        snprintf(format, sizeof(format), "<unknown 0x%x>", t);
        return format;
    }
}

void mod_vnc::updatePreferedAuth (uint32_t authId, VncAuthType &preferedAuth, size_t &preferedAuthIndex) {
    static VncAuthType preferedAuthTypes[] = {
        VeNCRYPT_X509Plain, VeNCRYPT_X509Vnc, VeNCRYPT_X509None,
        VeNCRYPT_TLSPlain, VeNCRYPT_TLSVnc, VeNCRYPT_TLSNone,
        VNC_AUTH_ULTRA_SecureVNCPluginAuth_new, VNC_AUTH_ULTRA_SecureVNCPluginAuth,
        VNC_AUTH_VENCRYPT, VNC_AUTH_ULTRA_MsLogonIIAuth,
        VNC_AUTH_ULTRA_MS_LOGON, VNC_AUTH_VNC, VNC_AUTH_NONE
    };

    const size_t nauths = sizeof(preferedAuthTypes) / sizeof(preferedAuthTypes[0]);
    for (size_t i = 0; i < std::min(nauths, preferedAuthIndex); i++) {
        if (preferedAuthTypes[i] == authId) {
            preferedAuth = static_cast<VncAuthType>(authId);
            preferedAuthIndex = i;
            return;
        }
    }
}

bool mod_vnc::readSecurityResult(InStream &s, uint32_t &status, bool &haveReason, std::string &reason, size_t &skipLen) const {
    if (s.in_remain() < 4){
        return false;
    }

    skipLen = 4;
    status = s.in_uint32_be();
    switch(status) {
    case 0: // SUCCESS
        return true;
    case 1:        // Failed
    case 2: {    // too many attempts
        /*   Version 3.8 onwards
         * If unsuccessful, the server sends a string describing the reason for the failure, and then closes the connection:
         * No. of bytes     Type     Description
         *                4     U32     reason-length
         *    reason-length     U8 array     reason-string
         *
         */
        if (this->spokenProtocol >= 3008) {
            haveReason = true;
            if (s.in_remain() < 4)
                return false;

            uint32_t reasonLen = s.in_uint32_be();
            if (s.in_remain() < reasonLen)
                return false;

            reason = "";
            reason.append(char_ptr_cast(s.get_current()), reasonLen);
            skipLen = 4 + 4 + reasonLen;
        } else {
            haveReason = false;
        }
        return true;
    }
    case 0xffffffff: // UltraVNC continue code
        return true;
    default:
        LOG(LOG_ERR, "unknown auth failed reason 0x%x", status);
        return true;
    }
}


bool mod_vnc::treatVeNCrypt() {
    InStream s(this->server_data_buf.av());

    switch(this->vencryptState) {
    case WAIT_VENCRYPT_VERSION: {
        if (s.in_remain() < 2){
            return false;
        }

        uint8_t major, minor;
        major = s.in_uint8();
        minor = s.in_uint8();

        if (major != 0 && minor != 2) {
            LOG(LOG_ERR, "unsupported VeNCrypt version %d.%d", major, minor);
            throw Error(ERR_VNC_CONNECTION_ERROR);
        }

        uint8_t clientVersion[2] = {0, 2};
        this->t.send(clientVersion, 2);

        this->server_data_buf.advance(2);
        this->vencryptState = WAIT_VENCRYPT_VERSION_RESPONSE;
        break;
    }

    case WAIT_VENCRYPT_VERSION_RESPONSE: {
        if (s.in_remain() < 1){
            return false;
        }

        uint8_t ack = s.in_uint8();
        if (ack != 0) {
            LOG(LOG_ERR, "server discarded our version");
            throw Error(ERR_VNC_CONNECTION_ERROR);
        }
        this->vencryptState = WAIT_VENCRYPT_SUBTYPES;
        this->server_data_buf.advance(1);
        REDEMPTION_CXX_FALLTHROUGH;
    }

    case WAIT_VENCRYPT_SUBTYPES: {
        if (s.in_remain() < 1){
            return false;
        }

        uint8_t nSubTypes = s.in_uint8();
        if (nSubTypes == 0) {
            LOG(LOG_ERR, "no VeNCrypt subtypes");
            throw Error(ERR_VNC_CONNECTION_ERROR);
        }

        if (s.in_remain() / 4 < nSubTypes){
            return false;
        }

        LOG(LOG_DEBUG, "VeNCrypt subtypes:");

        VncAuthType preferedAuth = VNC_AUTH_INVALID;
        size_t preferedAuthIndex = 255;

        for (uint8_t i = 0; i < nSubTypes; i++) {
            uint32_t subtype = s.in_uint32_be();
            LOG(LOG_DEBUG, " * %s", securityTypeString(subtype));

            if (subtype == VNC_AUTH_VENCRYPT) {
                LOG(LOG_ERR, "VeNCrypt auth type not allowed in VeNCrypt subtypes");
                throw Error(ERR_VNC_CONNECTION_ERROR);
            }

            this->updatePreferedAuth(subtype, preferedAuth, preferedAuthIndex);
        }
        this->server_data_buf.advance(1 + nSubTypes * 4);

        LOG(LOG_DEBUG, "selected VeNCrypt security is %s", securityTypeString(preferedAuth));
        if (preferedAuth == VNC_AUTH_INVALID) {
            throw Error(ERR_VNC_CONNECTION_ERROR);
        }

        StaticOutStream<4> outStream;
        outStream.out_uint32_be(static_cast<uint32_t>(preferedAuth));
        this->t.send(outStream.get_produced_bytes());

        this->choosenAuth = preferedAuth;
        this->vencryptState = WAIT_VENCRYPT_AUTH_ANSWER;
        break;
    }
    case WAIT_VENCRYPT_AUTH_ANSWER: {
        switch(this->choosenAuth ) {
        case VNC_AUTH_NONE:
            this->state = WAIT_SECURITY_RESULT;
            break;
        case VNC_AUTH_VNC:
            this->state = WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM;
            break;

        case VeNCRYPT_TLSNone:
        case VeNCRYPT_TLSPlain:
        case VeNCRYPT_TLSVnc:

        case VeNCRYPT_X509None:
        case VeNCRYPT_X509Plain:
        case VeNCRYPT_X509Vnc: {
            /* only TLS and X509 subtypes have an answer packet */
            if (s.in_remain() < 1){
                return false;
            }
            uint8_t ack = s.in_uint8();
            if (ack != 1) {
                LOG(LOG_ERR, "server not ok with our authType");
                throw Error(ERR_VNC_CONNECTION_ERROR);
            }
            this->server_data_buf.advance(1);
            this->tlsSwitch = !this->doTlsSwitch();
            break;
        }
        default:
            LOG(LOG_ERR, "unknown state");
            throw Error(ERR_VNC_CONNECTION_ERROR);
        }

        break;
    }
    default:
        break;
    }
    return true;
}


bool mod_vnc::draw_event_impl(gdi::GraphicApi & gd)
{
    switch (this->state)
    {
    case DO_INITIAL_CLEAR_SCREEN:
        this->initial_clear_screen(gd);
        return false;

    case UP_AND_RUNNING:
        LOG_IF(bool(this->verbose & VNCVerbose::draw_event), LOG_INFO, "state=UP_AND_RUNNING");

        try {
            while (this->up_and_running_ctx.run(this->server_data_buf, gd, *this)) {
                this->up_and_running_ctx.restart();
            }
            this->update_screen(Rect(0, 0, this->width, this->height), 1);
        }
        catch (const Error & e) {
            LOG(LOG_ERR, "VNC Stopped: %s", e.errmsg());
            this->set_mod_signal(BACK_EVENT_NEXT);
            this->front.must_be_stop_capture();
//            throw Error(ERR_BACK_EVENT_NEXT);
        }
        catch (...) {
            LOG(LOG_ERR, "unexpected exception raised in VNC");
            this->set_mod_signal(BACK_EVENT_NEXT);
            this->front.must_be_stop_capture();
//            throw Error(ERR_BACK_EVENT_NEXT);
        }

        return false;

    case WAIT_SECURITY_TYPES:
        {
            LOG_IF(bool(this->verbose & VNCVerbose::connection), LOG_INFO, "state=WAIT_SECURITY_TYPES");

            size_t const protocol_version_len = 12;

            if (this->server_data_buf.remaining() < protocol_version_len) {
                return false;
            }

            //
            // the buffer is supposed to be
            //      RFB XXX.YYY\n
            // with XXX and YYY being major and minor version
            //
            const uint8_t *rfbString = this->server_data_buf.av().data();

            if (memcmp(rfbString, "RFB ", 4) != 0) {
                LOG(LOG_INFO, "Invalid server handshake");
                throw Error(ERR_VNC_CONNECTION_ERROR);
            }

            if (rfbString[7] != '.') {
                LOG(LOG_INFO, "Invalid server handshake");
                throw Error(ERR_VNC_CONNECTION_ERROR);
            }

            // TODO std::from_chars
            auto versionParser = [](const uint8_t *str, int &v) {
                v = 0;
                for (int i = 0; i < 3; i++) {
                    if (str[i] < '0' || str[i] > '9'){
                        return false;
                    }
                    v = v * 10 + (str[i] - '0');
                }
                return true;
            };

            int major, minor;
            if (!versionParser(&rfbString[4], major) || !versionParser(&rfbString[8], minor)) {
                LOG(LOG_INFO, "Invalid server handshake");
                throw Error(ERR_VNC_CONNECTION_ERROR);
            }
            LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO,
                    "Server Protocol Version=%d.%d", major, minor);

            int serverProtocol = major * 1000 + minor;
            this->spokenProtocol = std::min(static_cast<int>(maxSpokenVncProcotol), serverProtocol);

            char handshakeAnswer[13];
            snprintf(handshakeAnswer, sizeof(handshakeAnswer), "RFB %.3d.%.3d\n",
                    this->spokenProtocol / 1000, this->spokenProtocol % 1000);
            this->t.send(handshakeAnswer, 12);

            this->server_data_buf.advance(protocol_version_len);
            IF_ENABLE_METRICS(data_from_client(12));

            this->state = WAIT_SECURITY_TYPES_LEVEL;
        }
        REDEMPTION_CXX_FALLTHROUGH;

    case WAIT_SECURITY_TYPES_LEVEL:
        {
            InStream s(this->server_data_buf.av());

            if (this->spokenProtocol >= 3007) {
                //
                // in version 3.7 or greater the packet has format
                // uint8         number of security types
                // variable     array of security types
                //
                if (s.in_remain() < 1) {
                    return false;
                }

                uint8_t nAuthTypes = s.in_uint8();
                if (s.in_remain() < nAuthTypes) {
                    return false;
                }

                if (!nAuthTypes) {
                    // 0 authTypes means an error occurred and it's followed by a reason packet:
                    // u32 - reasonLength
                    // u8 array - reason string
                    if (s.in_remain() < 4){
                        return false;
                    }

                    uint32_t reasonLen = s.in_uint32_be();
                    if (s.in_remain() < reasonLen){
                        return false;
                    }

                    std::string reason;
                    reason.append(char_ptr_cast(s.get_current()), reasonLen);

                    LOG(LOG_INFO, "connection failed, reason=%s", reason.c_str());
                    throw Error(ERR_VNC_CONNECTION_ERROR);
                }

                VncAuthType preferedAuth = VNC_AUTH_INVALID;
                size_t preferedAuthIndex = 255;

                LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO,
                    "got %d security types:", nAuthTypes);

                for (size_t i = 0; i < nAuthTypes; i++) {
                    VncAuthType authType = static_cast<VncAuthType>(s.in_uint8());
                    LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO,
                               "* %s", securityTypeString(authType));
                    this->updatePreferedAuth(authType, preferedAuth, preferedAuthIndex);
                }
                LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO,
                       "%s security choosen", securityTypeString(preferedAuth));

                this->server_data_buf.advance(1 + nAuthTypes);

                this->choosenAuth = preferedAuth;
                uint8_t authAnswer = static_cast<uint8_t>(preferedAuth);
                this->t.send(bytes_view{&authAnswer, 1});

                switch (preferedAuth){
                    case VNC_AUTH_INVALID:
                        this->state = WAIT_SECURITY_TYPES_INVALID_AUTH;
                        break;
                    case VNC_AUTH_NONE:
                        this->state = WAIT_SECURITY_RESULT;
                        break;
                    case VNC_AUTH_VNC:
                        this->state = WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM;
                        break;
                    case VNC_AUTH_ULTRA_MS_LOGON:
                        this->state = WAIT_SECURITY_TYPES_MS_LOGON;
                        break;
                    case VNC_AUTH_ULTRA_MsLogonIAuth:
                        LOG(LOG_ERR, "MsLogonIAuth not supported");
                        throw Error(ERR_VNC_CONNECTION_ERROR);
                    case VNC_AUTH_ULTRA_MsLogonIIAuth:
                        this->state = WAIT_SECURITY_TYPES_MS_LOGON;
                        break;
                    case VNC_AUTH_VENCRYPT:
                        this->state = DO_VENCRYPT_HANDSHAKE;
                        break;
                    case VNC_AUTH_ULTRA_SecureVNCPluginAuth:
                    case VNC_AUTH_ULTRA_SecureVNCPluginAuth_new:
                        this->state = WAIT_SECURITY_ULTRA_CHALLENGE;
                        break;
                    default:
                        LOG(LOG_ERR, "internal bug when computing prefered VNC auth");
                        throw Error(ERR_VNC_CONNECTION_ERROR);
                }
            } else {
                // version 3.3 or less, the server decides the security type that
                // is used
                if (s.in_remain() < 4) {
                    return false;
                }
                int32_t security_type = s.in_sint32_be();
                this->server_data_buf.advance(4);

                LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO,
                    "security level is %d (1 = none, 2 = standard, -6 = mslogon)",
                    security_type);

                switch (security_type) {
                    case VNC_AUTH_INVALID:
                        this->state = WAIT_SECURITY_TYPES_INVALID_AUTH;
                        break;
                    case VNC_AUTH_NONE:
                        this->state = SERVER_INIT;
                        break;
                    case VNC_AUTH_VNC:
                        this->state = WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM;
                        break;
                    case VNC_AUTH_ULTRA_MS_LOGON:
                        this->state = WAIT_SECURITY_TYPES_MS_LOGON;
                        break;
                    case VNC_AUTH_VENCRYPT:
                        this->state = DO_VENCRYPT_HANDSHAKE;
                        break;
                    case VNC_AUTH_ULTRA_SecureVNCPluginAuth_new:
                        this->state = WAIT_SECURITY_ULTRA_CHALLENGE;
                        break;
                    default:
                        LOG(LOG_ERR, "vnc unexpected security level");
                        throw Error(ERR_VNC_CONNECTION_ERROR);
                }

            }
        }
        return true;

    case WAIT_SECURITY_RESULT: {
        uint32_t status;
        bool haveReason = false;
        std::string reason;
        size_t skipLen;
        InStream s(this->server_data_buf.av());

        if (!this->readSecurityResult(s, status, haveReason, reason, skipLen)){
            return false;
        }

        VncState nextState;
        switch(status) {
        case SECURITY_REASON_OK:
            nextState = SERVER_INIT;
            break;
        case SECURITY_REASON_FAILED:
        case SECURITY_REASON_TOO_MANY_ATTEMPTS: {
            const char *authErrorStr = (status == SECURITY_REASON_FAILED) ? "failed" : "failed (too many attempts)";
            if (!haveReason){
                reason = "<no reason>";
            }

            LOG(LOG_ERR, "vnc auth %s, reason=%s", authErrorStr, reason.c_str());
            throw Error(ERR_VNC_CONNECTION_ERROR);
        }
        case SECURITY_REASON_CONTINUE:
            nextState = WAIT_SECURITY_TYPES_LEVEL;
            break;

        default:
            LOG(LOG_ERR, "unknown auth failed reason 0x%x", status);
            throw Error(ERR_VNC_CONNECTION_ERROR);
        }

        this->state = nextState;
        this->server_data_buf.advance(skipLen);
        return true;
    }

    case DO_VENCRYPT_HANDSHAKE:
        return this->treatVeNCrypt();

    case WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM:
        LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "Receiving VNC Server Random");

        {
            if (!this->password_ctx.run(this->server_data_buf)) {
                return false;
            }
            this->password_ctx.restart();

            char key[12] = {};

            // key is simply password padded with nulls
            strncpy(key, char_ptr_cast(byte_ptr_cast(this->password)), 8);
            rfbDesKey(byte_ptr_cast(key), EN0); // 0, encrypt
            auto const random_buf = this->password_ctx.server_random.data();
            rfbDes(random_buf, random_buf);
            rfbDes(random_buf + 8, random_buf + 8);

            LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "Sending Password");
            this->t.send(random_buf, 16);
            IF_ENABLE_METRICS(data_from_client(16));
        }
        this->state = WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM_RESPONSE;
        REDEMPTION_CXX_FALLTHROUGH;

    case WAIT_SECURITY_TYPES_PASSWORD_AND_SERVER_RANDOM_RESPONSE:
        {
            // sec result
            LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "Waiting for password ack");

            if (!this->auth_response_ctx.run(this->server_data_buf, [this](bool status, bytes_view bytes){
                if (status) {
                    LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "vnc password ok");
                }
                else {
                    LOG(LOG_INFO, "vnc password failed. Reason: %.*s",
                        int(bytes.size()), bytes.as_charp());
                    throw Error(ERR_VNC_CONNECTION_ERROR);
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

            if (!this->ms_logon(this->server_data_buf)) {
                return false;
            }
            this->ms_logon_ctx.restart();
        }
        this->state = WAIT_SECURITY_TYPES_MS_LOGON_RESPONSE;
        REDEMPTION_CXX_FALLTHROUGH;

    case WAIT_SECURITY_TYPES_MS_LOGON_RESPONSE:
        {
            LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "Waiting for password ack");

            if (!this->auth_response_ctx.run(this->server_data_buf, [this](bool status, bytes_view bytes){
                if (status) {
                    LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "MS LOGON password ok");
                }
                else {
                    LOG(LOG_INFO, "MS LOGON password FAILED. Reason: %.*s",
                        int(bytes.size()), bytes.as_charp());
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
            LOG(LOG_ERR, "VNC INVALID Auth");

            if (!this->invalid_auth_ctx.run(this->server_data_buf, [](bytes_view av){
                hexdump_c(av);
            })) {
                return false;
            }
            this->invalid_auth_ctx.restart();

            throw Error(ERR_VNC_CONNECTION_ERROR);
            // return true;
        }

    case WAIT_SECURITY_ULTRA_CHALLENGE: {
        uint8_t passPhraseUsed;
        if (!this->dsm){
            dsm = new UltraDSM(this->password);
        }

        InStream challenge(this->server_data_buf.av());
        uint16_t challengeLen;
        if (!dsm->handleChallenge(challenge, challengeLen, passPhraseUsed)){
            return false;
        }

        this->server_data_buf.advance(challengeLen + 1 + 2);

        StaticOutStream<2> lenStream;
        StaticOutReservedStreamHelper<2, 65535> out;
        OutStream &outPacket = out.get_data_stream();
        dsm->getResponse(outPacket);

        lenStream.out_uint16_le(outPacket.get_offset());
        out.copy_to_head(lenStream.get_produced_bytes());
        writable_bytes_view packet = out.get_packet();
        this->t.send(packet.begin(), packet.size());

        this->dsmEncryption = true;

        if (passPhraseUsed != 2) {
            lenStream.rewind(0);
            uint16_t passLen = strlen(this->password);
            lenStream.out_uint16_le(passLen);

            this->t.send(lenStream.get_data(), 2);
            this->t.send(this->password, passLen);
        }
        this->state = WAIT_SECURITY_RESULT;
        break;
    }

    case SERVER_INIT:
        this->t.send("\x01", 1); // share flag
        IF_ENABLE_METRICS(data_from_client(1));
        this->state = SERVER_INIT_RESPONSE;
        REDEMPTION_CXX_FALLTHROUGH;

    case SERVER_INIT_RESPONSE:
        {
            if (!this->server_init_ctx.run(this->server_data_buf, *this)) {
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

//                const char * pixel_format =  "\x20\x18\x00"
//                                             "\x01\x00\xFF\x00\xFF\x00\xFF\x10\x08\x00"
//                                             "\0\0\0" ;


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
            this->t.send(stream.get_produced_bytes());
            IF_ENABLE_METRICS(data_from_client(stream.get_offset()));

            this->bpp = BitsPerPixel{16};
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

        // See Encodings for a description of each encoding and Pseudo-encodings for the meaning of pseudo-encodings.

        // No. of bytes     Type     [Value]     Description
        // -------------+---------+-----------+---------------------
        //         1    |    U8   |     2     |  message-type
        //         1    |         |           |    padding
        //         2    |    U16  |           | number-of-encodings
        //
        // followed by number-of-encodings repetitions of the following:
        //
        // No. of bytes     Type     Description
        // ----------------------------------------
        //         4         S32     encoding-type

        {
            // SetEncodings
            StaticOutStream<32768> stream;

            bool support_zrle_encoding          = true;
            bool support_hextile_encoding       = false;
            bool support_rre_encoding           = false;
            bool support_raw_encoding           = true;
            bool support_copyrect_encoding      = true;
            bool support_cursor_pseudo_encoding = this->cursor_pseudo_encoding_supported;

            char const * p = this->encodings.c_str();
            if (p && *p){
                support_zrle_encoding          = false;
                for (;;){
                    while (*p == ','){++p;}
                    char * end;
                    int32_t encoding_type = std::strtol(p, &end, 0);
                    if (p == end) { break; }
                    p = end;
                    switch (encoding_type){
                    case HEXTILE_ENCODING:
                        support_hextile_encoding = true;
                    break;
                    case ZRLE_ENCODING:
                        support_zrle_encoding = true;
                    break;
                    case RRE_ENCODING:
                        support_rre_encoding = true;
                    break;
                    default:
                    break;
                    }
                }
            }
            else {
                support_zrle_encoding          = true;
                support_hextile_encoding       = true;
                support_rre_encoding           = true;
            }

            uint16_t number_of_encodings =  support_zrle_encoding
                                         +  support_hextile_encoding
                                         +  support_raw_encoding
                                         +  support_copyrect_encoding
                                         +  support_rre_encoding
                                         +  support_cursor_pseudo_encoding;

//                LOG(LOG_INFO, "number of encodings=%d", number_of_encodings);

            stream.out_uint8(VNC_CS_MSG_SET_ENCODINGS);
            stream.out_uint8(0);
            stream.out_uint16_be(number_of_encodings);
            if (support_zrle_encoding)          {
                LOG(LOG_INFO, "enable ZRLE encoding");
                stream.out_uint32_be(ZRLE_ENCODING);
            }            // (16) Zrle
            if (support_hextile_encoding)       {
                LOG(LOG_INFO, "enable hextile encoding");
                stream.out_uint32_be(HEXTILE_ENCODING);
            }         // (5) Hextile
            if (support_raw_encoding)           {
                LOG(LOG_INFO, "enable RAW encoding");
                stream.out_uint32_be(RAW_ENCODING);
            }             // (0) raw
            if (support_copyrect_encoding)      {
                LOG(LOG_INFO, "enable copyrect encoding");
                stream.out_uint32_be(COPYRECT_ENCODING);
            }        // (1) copy rect
            if (support_rre_encoding)           {
                LOG(LOG_INFO, "enable rre encoding");
                stream.out_uint32_be(RRE_ENCODING);
            }             // (2) RRE
            if (support_cursor_pseudo_encoding) {
                LOG(LOG_INFO, "enable cursor pseudo encoding");
                stream.out_uint32_be(CURSOR_PSEUDO_ENCODING);
            }   // (-239) cursor

            assert(4u + number_of_encodings * 4u == stream.get_offset());

            this->t.send(stream.get_data(), 4u + number_of_encodings * 4u);
            IF_ENABLE_METRICS(data_from_client(stream.get_offset()));
        }


        switch (this->front.server_resize({align4(this->width), this->height, this->bpp})){
        case FrontAPI::ResizeResult::instant_done:
            LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "no resizing needed");
            // no resizing needed
            this->state = DO_INITIAL_CLEAR_SCREEN;
            break;
        case FrontAPI::ResizeResult::remoteapp:
            LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "resizing remoteapp");
            if (this->rail_client_execute) {
                this->rail_client_execute->adjust_window_to_mod();
            }
            // RZ: Continue with FrontAPI::ResizeResult::no_need
            REDEMPTION_CXX_FALLTHROUGH;
        case FrontAPI::ResizeResult::no_need:
            LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "no resizing needed");
            // no resizing needed
            this->state = DO_INITIAL_CLEAR_SCREEN;
            break;
        case FrontAPI::ResizeResult::done:
            LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "resizing done");
            // resizing done
            this->state = WAIT_CLIENT_UP_AND_RUNNING;
            break;
        case FrontAPI::ResizeResult::fail:
            // resizing failed
            // thow an Error ?
            LOG(LOG_ERR, "Older RDP client can't resize resolution from server, disconnecting");
            throw Error(ERR_VNC_OLDER_RDP_CLIENT_CANT_RESIZE);
        }
        return true;

    case WAIT_CLIENT_UP_AND_RUNNING:
        LOG(LOG_INFO, "Waiting for client become up and running");
        break;

    default:
        LOG(LOG_ERR, "Unknown state=%d", static_cast<int>(this->state));
        throw Error(ERR_VNC);
    }

    return false;
}

void mod_vnc::check_timeout()
{
    if (this->clipboard_requesting_for_data_is_delayed) {
        //const uint64_t usnow = ustime();
        //const uint64_t timeval_diff = usnow - this->clipboard_last_client_data_timestamp;
        //LOG(LOG_INFO,
        //    "usnow=%llu clipboard_last_client_data_timestamp=%llu timeval_diff=%llu",
        //    usnow, this->clipboard_last_client_data_timestamp, timeval_diff);
        LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
            "mod_vnc server clipboard PDU: msgType=CB_FORMAT_DATA_REQUEST(%u) (time)",
            RDPECLIP::CB_FORMAT_DATA_REQUEST);

        // Build and send a CB_FORMAT_DATA_REQUEST to front (for format CF_TEXT or CF_UNICODETEXT)
        // 04 00 00 00 04 00 00 00 0? 00 00 00
        // 00 00 00 00
        RDPECLIP::FormatDataRequestPDU format_data_request_pdu(this->clipboard_requested_format_id);
        RDPECLIP::CliprdrHeader format_data_request_header(RDPECLIP::CB_FORMAT_DATA_REQUEST, RDPECLIP::CB_RESPONSE__NONE_, format_data_request_pdu.size());
        StaticOutStream<256>           out_s;

        format_data_request_header.emit(out_s);
        format_data_request_pdu.emit(out_s);

        size_t length     = out_s.get_offset();
        size_t chunk_size = length;

        this->clipboard_requesting_for_data_is_delayed = false;
        this->clipboard_timeout_timer = this->events.erase_event(this->clipboard_timeout_timer);
        this->send_to_front_channel( channel_names::cliprdr
                                   , out_s.get_data()
                                   , length
                                   , chunk_size
                                   , CHANNELS::CHANNEL_FLAG_FIRST
                                   | CHANNELS::CHANNEL_FLAG_LAST
                                   );
    }
}


bool mod_vnc::lib_clip_data(Buf64k & buf)
{
    if (!this->clipboard_data_ctx.run(buf)) {
        return false;
    }

    if (this->clipboard_data_ctx.clipboard_is_enabled()) {
        LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
            "mod_vnc::lib_clip_data: Sending Format List PDU (%u) to client.",
            RDPECLIP::CB_FORMAT_LIST);

        StaticOutStream<256> out_s;
        Cliprdr::format_list_serialize_with_header(
            out_s, Cliprdr::IsLongFormat(false),
            std::array{Cliprdr::FormatNameRef{RDPECLIP::CF_UNICODETEXT, {}}});

        const size_t totalLength = out_s.get_offset();
        this->send_to_front_channel(channel_names::cliprdr,
                out_s.get_data(),
                totalLength,
                totalLength,
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST
            );

        this->clipboard_owned_by_client = false;

        // Can stop RDP to VNC clipboard infinite loop.
        this->clipboard_requesting_for_data_is_delayed = false;
        this->clipboard_timeout_timer = this->events.erase_event(this->clipboard_timeout_timer);
    }
    else {
        LOG(LOG_WARNING, "mod_vnc::lib_clip_data: Clipboard Channel Redirection unavailable");
    }

    return true;
}


void mod_vnc::send_to_front_channel(CHANNELS::ChannelNameId mod_channel_name, uint8_t const * data,
        size_t length, size_t chunk_size, int flags)
{
    LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "mod_vnc::send_to_front_channel");

    const CHANNELS::ChannelDef * front_channel = this->front.get_channel_list().get_by_name(mod_channel_name);
    if (front_channel) {
        this->front.send_to_channel(*front_channel, {data, chunk_size}, length, flags);
        IF_ENABLE_METRICS(clipboard_data_from_client(chunk_size));
    }
}

void mod_vnc::send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream & chunk,
        size_t length, uint32_t flags)
{
    LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO, "mod_vnc::send_to_mod_channel");

    if (this->state != UP_AND_RUNNING) {
        return;
    }

    if (front_channel_name == channel_names::cliprdr) {
        this->clipboard_send_to_vnc_server(chunk, length, flags);
    }
}

void mod_vnc::send_clipboard_pdu_to_front(const OutStream & out_stream)
{
    // Send clipboard as a train of consecutive PDU
    size_t pdu_data_length           = out_stream.get_offset();
    size_t remaining_pdu_data_length = pdu_data_length;
    uint8_t * chunk_data = out_stream.get_data();

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
        LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
            "mod_vnc server clipboard PDU: msgType=CB_FORMAT_DATA_RESPONSE(%u) - chunk_size=%zu",
            RDPECLIP::CB_FORMAT_DATA_RESPONSE, chunk_size);

        send_flags &= ~CHANNELS::CHANNEL_FLAG_FIRST;

        chunk_data += chunk_size;
    }
    while (remaining_pdu_data_length);
}


void mod_vnc::clipboard_send_to_vnc_server(InStream & chunk, size_t length, uint32_t flags)
{
    LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO,
        "mod_vnc::clipboard_send_to_vnc_server: length=%zu chunk_size=%zu flags=0x%08X",
        length, chunk.get_capacity(), flags);

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
    uint16_t msgType = rp.msgType();

    if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) == 0) {
        msgType = RDPECLIP::CB_CHUNKED_FORMAT_DATA_RESPONSE;
        // msgType read is not a msgType, it's a part of data.
    }

    LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
        "mod_vnc client clipboard PDU: msgType=%s(%d)",
        RDPECLIP::get_msgType_name(msgType), msgType);

    switch (msgType) {
        case RDPECLIP::CB_FORMAT_LIST: {
            RDPECLIP::CliprdrHeader incoming_header;
            incoming_header.recv(chunk);
            if (bool(this->verbose & VNCVerbose::clipboard)) {
                incoming_header.log(LOG_INFO);
            }

            // Client notify that a copy operation have occured.
            // Two operations should be done :
            //  - Always: send a RDP acknowledge (CB_FORMAT_LIST_RESPONSE)
            //  - Only if clipboard content formats list include "UNICODETEXT:
            // send a request for it in that format
            bool contains_data_in_text_format = false;
            bool contains_data_in_unicodetext_format = false;
            Cliprdr::format_list_extract(
                chunk,
                Cliprdr::IsLongFormat(this->client_use_long_format_names),
                Cliprdr::IsAscii(bool(incoming_header.msgFlags() & RDPECLIP::CB_ASCII_NAMES)),
                [&](uint32_t format_id, auto const& name) {
                    contains_data_in_text_format |= (format_id == RDPECLIP::CF_TEXT);
                    contains_data_in_unicodetext_format |= (format_id == RDPECLIP::CF_UNICODETEXT);
                    if (bool(this->verbose & VNCVerbose::clipboard)) {
                        Cliprdr::log_format_name("", format_id, name);
                    }
                }
            );


            //---- Beginning of clipboard PDU Header ----------------------------

            LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
                "mod_vnc server clipboard PDU: msgType=CB_FORMAT_LIST_RESPONSE(%u)",
                RDPECLIP::CB_FORMAT_LIST_RESPONSE);

            // Build and send the CB_FORMAT_LIST_RESPONSE (with status = OK)
            // 03 00 01 00 00 00 00 00 00 00 00 00
            RDPECLIP::CliprdrHeader clipboard_header(RDPECLIP::CB_FORMAT_LIST_RESPONSE, RDPECLIP::CB_RESPONSE_OK, 0);
            StaticOutStream<256>    out_s;

            clipboard_header.emit(out_s);

            size_t chunk_size = out_s.get_offset();

            this->send_to_front_channel( channel_names::cliprdr
                                       , out_s.get_data()
                                       , chunk_size // total length is chunk size
                                       , chunk_size
                                       ,   CHANNELS::CHANNEL_FLAG_FIRST
                                         | CHANNELS::CHANNEL_FLAG_LAST
                                       );

            using std::chrono::microseconds;
            constexpr microseconds MINIMUM_TIMEVAL(250000LL);

            if (this->enable_clipboard_up
             && (contains_data_in_text_format || contains_data_in_unicodetext_format)
            ) {
                this->clipboard_requested_format_id = contains_data_in_unicodetext_format
                    ? RDPECLIP::CF_UNICODETEXT
                    : RDPECLIP::CF_TEXT;

                const microseconds usnow        = ustime(this->time_base.get_current_time());
                const microseconds timeval_diff = usnow - this->clipboard_last_client_data_timestamp;
                if ((timeval_diff > MINIMUM_TIMEVAL) || !this->clipboard_owned_by_client) {
                    LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
                        "mod_vnc server clipboard PDU: msgType=CB_FORMAT_DATA_REQUEST(%u)",
                        RDPECLIP::CB_FORMAT_DATA_REQUEST);

                    // Build and send a CB_FORMAT_DATA_REQUEST to front (for format CF_TEXT or CF_UNICODETEXT)
                    // 04 00 00 00 04 00 00 00 0? 00 00 00
                    // 00 00 00 00

                    RDPECLIP::FormatDataRequestPDU format_data_request_pdu(this->clipboard_requested_format_id);
                    RDPECLIP::CliprdrHeader format_data_request_header(RDPECLIP::CB_FORMAT_DATA_REQUEST, RDPECLIP::CB_RESPONSE__NONE_, format_data_request_pdu.size());
                    out_s.rewind();

                    format_data_request_header.emit(out_s);
                    format_data_request_pdu.emit(out_s);

                    chunk_size = out_s.get_offset();

                    this->clipboard_requesting_for_data_is_delayed = false;
                    this->clipboard_timeout_timer = this->events.erase_event(this->clipboard_timeout_timer);
                    this->send_to_front_channel( channel_names::cliprdr
                                               , out_s.get_data()
                                               , chunk_size // total length is chunk_size
                                               , chunk_size
                                               , CHANNELS::CHANNEL_FLAG_FIRST
                                               | CHANNELS::CHANNEL_FLAG_LAST
                                               );
                }
                else {
                    if (this->bogus_clipboard_infinite_loop == VncBogusClipboardInfiniteLoop::delayed) {
                        LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
                            "mod_vnc server clipboard PDU: msgType=CB_FORMAT_DATA_REQUEST(%u) (delayed)",
                            RDPECLIP::CB_FORMAT_DATA_REQUEST);
                        this->clipboard_requesting_for_data_is_delayed = true;
                        // arms timeout
                        this->clipboard_timeout_timer = this->events.create_event_timeout(
                            "VNC Clipboard Timeout Event", this,
                            this->time_base.get_current_time()+(MINIMUM_TIMEVAL - timeval_diff),
                            [this](Event&){this->check_timeout();});
                    }
                    else if ((this->bogus_clipboard_infinite_loop != VncBogusClipboardInfiniteLoop::duplicated)
                        &&  ((this->clipboard_general_capability_flags & RDPECLIP::CB__MINIMUM_WINDOWS_CLIENT_GENERAL_CAPABILITY_FLAGS_)
                          == RDPECLIP::CB__MINIMUM_WINDOWS_CLIENT_GENERAL_CAPABILITY_FLAGS_)
                    ) {
                        LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
                            "mod_vnc::clipboard_send_to_vnc_server: "
                            "duplicated clipboard update event "
                            "from Windows client is ignored");
                    }
                    else {
                        LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
                            "mod_vnc server clipboard PDU: msgType=CB_FORMAT_LIST(%u) (preventive)",
                            RDPECLIP::CB_FORMAT_LIST);

                        StaticOutStream<256> out_s;
                        Cliprdr::format_list_serialize_with_header(
                            out_s, Cliprdr::IsLongFormat(false),
                            std::array{Cliprdr::FormatNameRef{
                                this->clipboard_requested_format_id, {}}});

                        const size_t totalLength = out_s.get_offset();
                        this->send_to_front_channel(channel_names::cliprdr,
                                out_s.get_data(),
                                totalLength,
                                totalLength,
                                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST
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
                   , "mod_vnc::clipboard_send_to_vnc_server: truncated CB_FORMAT_DATA_REQUEST(%u) data, need=%u remains=%zu"
                   , RDPECLIP::CB_FORMAT_DATA_REQUEST, expected, chunk.in_remain());
                throw Error(ERR_VNC);
            }

            // This is a fake treatment that pretends to send the Request
            //  to VNC server. Instead, the RDP PDU is handled localy and
            //  the clipboard PDU, if any, is likewise built localy and
            //  sent back to front.
            RDPECLIP::CliprdrHeader format_data_request_header;
            RDPECLIP::FormatDataRequestPDU format_data_request_pdu;

            // 04 00 00 00 04 00 00 00 0d 00 00 00 00 00 00 00
            format_data_request_header.recv(chunk);
            format_data_request_pdu.recv(chunk);

            LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
                "mod_vnc::clipboard_send_to_vnc_server: CB_FORMAT_DATA_REQUEST(%u) msgFlags=0x%02x datalen=%u requestedFormatId=%s(%u)",
                RDPECLIP::CB_FORMAT_DATA_REQUEST,
                format_data_request_header.msgFlags(),
                format_data_request_header.dataLen(),
                RDPECLIP::get_FormatId_name(format_data_request_pdu.requestedFormatId),
                format_data_request_pdu.requestedFormatId);

            if (this->bogus_clipboard_infinite_loop != VncBogusClipboardInfiniteLoop::delayed
            && this->clipboard_owned_by_client) {
                StreamBufMaker<65536> buf_maker;
                OutStream out_stream = buf_maker.reserve_out_stream(
                        8 +                                       // clipHeader(8)
                        this->to_vnc_clipboard_data.get_offset()  // data
                    );

                RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, this->to_vnc_clipboard_data.get_offset());
                const RDPECLIP::FormatDataResponsePDU format_data_response_pdu;
                header.emit(out_stream);
                format_data_response_pdu.emit(out_stream, this->to_vnc_clipboard_data.get_data(), this->to_vnc_clipboard_data.get_offset());

                this->send_clipboard_pdu_to_front(out_stream);
                break;
            }

            if ((format_data_request_pdu.requestedFormatId == RDPECLIP::CF_TEXT)
            && !this->clipboard_data_ctx.clipboard_data_is_utf8_encoded()) {
                StreamBufMaker<65536> buf_maker;
                OutStream out_stream = buf_maker.reserve_out_stream(
                    RDPECLIP::CliprdrHeader::size() +
                    this->clipboard_data_ctx.clipboard_data().size() * 2 +    // data
                    1                                           // Null character
                );

                auto to_rdp_clipboard_data =
                    ::linux_to_windows_newline_convert(
                        this->clipboard_data_ctx.clipboard_data().as_chars(),
                        out_stream.get_tail().as_chars()
                            .drop_front(RDPECLIP::CliprdrHeader::size())
                            // Null character
                            .drop_back(1)
                    );
                const auto to_rdp_clipboard_data_length = to_rdp_clipboard_data.size()
                    + 1; // Null character
                // Null character
                *to_rdp_clipboard_data.end() = '\x0';

                RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, to_rdp_clipboard_data_length);
                const RDPECLIP::FormatDataResponsePDU format_data_response_pdu;
                header.emit(out_stream);
                format_data_response_pdu.emit(out_stream, out_stream.get_data() + RDPECLIP::CliprdrHeader::size(), to_rdp_clipboard_data_length);
                this->send_clipboard_pdu_to_front(out_stream);

                LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
                    "mod_vnc::clipboard_send_to_vnc_server: "
                    "Sending Format Data Response PDU (CF_TEXT) done");
            }
            else if (format_data_request_pdu.requestedFormatId == RDPECLIP::CF_UNICODETEXT) {
                BufMaker<65536> buf_maker;
                auto uninit_buf_av = buf_maker.dyn_array(
                    8 +                                         // clipHeader(8)
                    this->clipboard_data_ctx.clipboard_data().size() * 4 +    // data
                    2                                           // Null character
                );
                auto const header_size = RDPECLIP::CliprdrHeader::size();

                // [ .................... out_stream .................... ]
                // [ [ ... header_stream(8) ... ] [ ... data_stream ... ] ]
                OutStream out_stream(uninit_buf_av);
                OutStream data_stream(uninit_buf_av.from_offset(header_size));

                if (this->clipboard_data_ctx.clipboard_data_is_utf8_encoded()) {
                    size_t utf16_data_length = UTF8toUTF16_CrLf(
                        this->clipboard_data_ctx.clipboard_data(),
                        data_stream.get_data(),
                        data_stream.get_capacity()-2
                    );
                    data_stream.out_skip_bytes(utf16_data_length);
                }
                else {
                    size_t utf16_data_length = Latin1toUTF16(
                        this->clipboard_data_ctx.clipboard_data(),
                        data_stream.get_data(),
                        data_stream.get_capacity()-2
                    );
                    data_stream.out_skip_bytes(utf16_data_length);
                }
                data_stream.out_uint16_le(0x0000);  // Null character

                RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, data_stream.get_offset());
                //const RDPECLIP::FormatDataResponsePDU format_data_response_pdu;
                header.emit(out_stream);
                assert(out_stream.get_current() == data_stream.get_data());
                out_stream.out_skip_bytes(data_stream.get_offset());
                //format_data_response_pdu.emit(out_stream, data_stream.get_data(), out_stream.get_offset());

                this->send_clipboard_pdu_to_front(out_stream);

                LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO,
                    "mod_vnc::clipboard_send_to_vnc_server: "
                    "Sending Format Data Response PDU (CF_UNICODETEXT) done");
            }
            else {
                LOG( LOG_INFO
                   , "mod_vnc::clipboard_send_to_vnc_server: resquested clipboard format Id 0x%02x is not supported by VNC PROXY"
                   , format_data_request_pdu.requestedFormatId);
            }
        }
        break;

        case RDPECLIP::CB_FORMAT_DATA_RESPONSE: {
            RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_FAIL, 0);
            // TODO: really we are not using format data response but reading underlying data directly
            // we should not do that! We should reassemble consecutive data packets until data_response
            // is reassembled, then check if it is full and should be send or not.
//                RDPECLIP::FormatDataResponsePDU format_data_response_pdu;

            header.recv(chunk);
            if (header.msgFlags() == RDPECLIP::CB_RESPONSE_OK) {
                // TODO: we should be able to unify both sides of the condition
                // the case where we have only one PDU is the extreme case of
                // when we have a train of consecutive PDU

                // Here is where we receive the first packet, the followup will be treated
                // in state RDPECLIP::CB_CHUNKED_FORMAT_DATA_RESPONSE (pseudo state
                // when we do not have a first packet)

                if ((flags & CHANNELS::CHANNEL_FLAG_LAST) != 0) {
                    if (!chunk.in_check_rem(header.dataLen())) {
                        LOG( LOG_ERR
                           , "mod_vnc::clipboard_send_to_vnc_server: truncated CB_FORMAT_DATA_RESPONSE(%u), need=%u remains=%zu"
                           , RDPECLIP::CB_FORMAT_DATA_RESPONSE
                           , header.dataLen(), chunk.in_remain());
                        throw Error(ERR_VNC);
                    }

//                        format_data_response_pdu.recv(chunk);

                    this->to_vnc_clipboard_data.rewind();

                    this->to_vnc_clipboard_data.out_copy_bytes(
                        chunk.get_current(), header.dataLen());

                    this->rdp_input_clip_data(this->to_vnc_clipboard_data.get_produced_bytes());
                } // CHANNELS::CHANNEL_FLAG_LAST
                else {
                    // Virtual channel data span in multiple Virtual Channel PDUs.

                    if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) == 0) {
                        LOG(LOG_ERR, "mod_vnc::clipboard_send_to_vnc_server: flag CHANNEL_FLAG_FIRST expected");
                        throw Error(ERR_VNC);
                    }

                    this->to_vnc_clipboard_data_size      =
                    this->to_vnc_clipboard_data_remaining = header.dataLen();

                    LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO,
                        "mod_vnc::clipboard_send_to_vnc_server: virtual channel data span in multiple Virtual Channel PDUs: total=%u",
                        this->to_vnc_clipboard_data_size);

                    this->to_vnc_clipboard_data.rewind();

                    if (this->to_vnc_clipboard_data.get_capacity() >= this->to_vnc_clipboard_data_size) {
                        uint32_t number_of_bytes_to_read = std::min<uint32_t>(
                            this->to_vnc_clipboard_data_remaining, chunk.in_remain());
                        this->to_vnc_clipboard_data.out_copy_bytes(
                            chunk.view_bytes(number_of_bytes_to_read));

                        this->to_vnc_clipboard_data_remaining -= number_of_bytes_to_read;
                    }
                    else {
                        char * latin1_overflow_message_buffer = ::char_ptr_cast(this->to_vnc_clipboard_data.get_data());
                        const size_t latin1_overflow_message_buffer_length = this->to_vnc_clipboard_data.get_capacity();

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
                    }
                } // else CHANNELS::CHANNEL_FLAG_LAST
            } // RDPECLIP::CB_RESPONSE_OK
        }
        break;

        case RDPECLIP::CB_CHUNKED_FORMAT_DATA_RESPONSE:
            assert(this->to_vnc_clipboard_data.get_offset() != 0);
            assert(this->to_vnc_clipboard_data_size);

            // Virtual channel data span in multiple Virtual Channel PDUs.
            LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO, "mod_vnc::clipboard_send_to_vnc_server: an other trunk");

            if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) != 0) {
                LOG(LOG_ERR, "mod_vnc::clipboard_send_to_vnc_server: flag CHANNEL_FLAG_FIRST unexpected");
                throw Error(ERR_VNC);
            }

            // if (bool(this->verbose & VNCVerbose::basic_trace)) {
            //     LOG( LOG_INFO, "mod_vnc::clipboard_send_to_vnc_server: trunk size=%u, capacity=%u"
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

                this->rdp_input_clip_data(this->to_vnc_clipboard_data.get_produced_bytes());
            }
        break;

        case RDPECLIP::CB_CLIP_CAPS:
        {
            RDPECLIP::CliprdrHeader clipboard_header;
            clipboard_header.recv(chunk);

            RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu;
            clipboard_caps_pdu.recv(chunk);
            assert(1 == clipboard_caps_pdu.cCapabilitiesSets());

            RDPECLIP::CapabilitySetRecvFactory caps_recv_factory(chunk);
            if (caps_recv_factory.capabilitySetType() == RDPECLIP::CB_CAPSTYPE_GENERAL) {
                RDPECLIP::GeneralCapabilitySet general_caps;
                general_caps.recv(chunk, caps_recv_factory);

                this->clipboard_general_capability_flags = general_caps.generalFlags();

                if (general_caps.generalFlags() & RDPECLIP::CB_USE_LONG_FORMAT_NAMES) {
                    this->client_use_long_format_names = true;
                }

                LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO,
                    "Client use %s format name",
                    (this->client_use_long_format_names ? "long" : "short"));

                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    general_caps.log(LOG_INFO);
                }
            }
        }
        break;
    }
    LOG_IF(bool(this->verbose & VNCVerbose::clipboard), LOG_INFO, "mod_vnc::clipboard_send_to_vnc_server: done");
}


void mod_vnc::init()
{
    if (this->gd_provider.is_ready_to_draw()
    && this->state == WAIT_CLIENT_UP_AND_RUNNING){
        this->state = DO_INITIAL_CLEAR_SCREEN;
        this->initial_clear_screen(this->gd_provider.get_graphics());
    }
}


void mod_vnc::rdp_gdi_up_and_running()
{
    if (this->gd_provider.is_ready_to_draw()
    && this->state == WAIT_CLIENT_UP_AND_RUNNING){
        this->state = DO_INITIAL_CLEAR_SCREEN;
        this->initial_clear_screen(this->gd_provider.get_graphics());
    }
}


void mod_vnc::draw_tile(Rect rect, const uint8_t * raw, gdi::GraphicApi & drawable)
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

void mod_vnc::disconnect()
{
    uint64_t seconds = this->time_base.get_current_time().tv_sec - this->beginning;
    LOG(LOG_INFO, "Client disconnect from VNC module");

    char duration_str[128];
    size_t len = snprintf(duration_str, sizeof(duration_str), "%02d:%02d:%02d",
        int(seconds / 3600),
        int((seconds % 3600) / 60),
        int(seconds % 60));

    this->sesman.log6(LogId::SESSION_DISCONNECTION, {KVLog("duration"_av, {duration_str, len}),});

    LOG_IF(bool(this->verbose & VNCVerbose::basic_trace), LOG_INFO,
        "type=SESSION_DISCONNECTION duration=%s", duration_str);
}
