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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/front/front_wrapper.hpp"
#include "test_only/session_log_test.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/core/font.hpp"

// Comment the code block below to generate testing data.
// Uncomment the code block below to generate testing data.

#include "acl/auth_api.hpp"
#include "acl/license_api.hpp"
#include "configs/config.hpp"
// Uncomment the code block below to generate testing data.
//include "transport/socket_transport.hpp"
#include "core/client_info.hpp"
#include "utils/theme.hpp"
#include "utils/redirection_info.hpp"
#include "utils/sugar/static_array_to_hexadecimal_chars.hpp"
#include "utils/sugar/int_to_chars.hpp"

#include "mod/null/null.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/mod_rdp_factory.hpp"
#include "core/events.hpp"
#include "utils/timebase.hpp"
#include "core/channels_authorizations.hpp"
#include "gdi/osd_api.hpp"
#include "front/front.hpp"
#include "core/guest_ctx.hpp"
#include "capture/cryptofile.hpp"

#include <cstring>


using namespace std::chrono_literals;

namespace dump2008 {
    #include "fixtures/dump_w2008.hpp"
} // namespace dump2008

namespace dump_front {
    #include "fixtures/trace_front_client.hpp"
} // namespace dump_front

namespace {

class MyFront : public FrontWrapper
{
public:
    using FrontWrapper::FrontWrapper;

    void send_to_channel(
        const CHANNELS::ChannelDef & /*channel*/,
        bytes_view /*chunk_data*/,
        std::size_t /*total_length*/,
        uint32_t /*flags*/) override
    {
    }
};

struct FrontTransport : GeneratorTransport
{
    using GeneratorTransport::GeneratorTransport;

    void do_send(const uint8_t * const /*buffer*/, size_t /*len*/) override
    {
        ++counter;
    }

    int counter = 0;
};

ClientInfo make_client_info()
{
    ClientInfo info;
    info.keylayout = KeyLayout::KbdId(0x040C);
    info.console_session = false;
    info.brush_cache_code = 0;
    info.screen_info.bpp = BitsPerPixel{24};
    info.screen_info.width = 800;
    info.screen_info.height = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    utils::strlcpy(info.hostname, "test");
    return info;
}

char* cpy(char* p, chars_view av)
{
    memcpy(p, av.data(), av.size());
    return p + av.size();
}

struct AppendBuffer
{
    char buffer[128];

    void append(std::string& output, char* end)
    {
        if (!output.empty()) {
            output += ", ";
        }
        output.append(buffer, end);
    }
};

void append_mouse(std::string& output, int device_flags, int x, int y)
{
    AppendBuffer buffer;
    char* p = buffer.buffer;
    p = cpy(p, "{flags=0x"_av);
    p = int_to_fixed_hexadecimal_upper_chars(p, static_cast<uint16_t>(device_flags));
    p = cpy(p, ", x="_av);
    p = cpy(p, int_to_decimal_chars(x));
    p = cpy(p, ", y="_av);
    p = cpy(p, int_to_decimal_chars(y));
    p = cpy(p, "}"_av);
    buffer.append(output, p);
}

void append_key_locks(std::string& output, kbdtypes::KeyLocks key_locks)
{
    AppendBuffer buffer;
    char* p = buffer.buffer;
    p = cpy(p, "{KeyLocks=0x"_av);
    p = int_to_fixed_hexadecimal_upper_chars(p, underlying_cast(key_locks));
    p = cpy(p, "}"_av);
    buffer.append(output, p);
}

void append_scancode(std::string& output, kbdtypes::KbdFlags flags, kbdtypes::Scancode scancode)
{
    AppendBuffer buffer;
    char* p = buffer.buffer;
    p = cpy(p, "{KbdFlags=0x"_av);
    p = int_to_fixed_hexadecimal_upper_chars(p, underlying_cast(flags));
    p = cpy(p, ", Scancode=0x"_av);
    p = int_to_fixed_hexadecimal_upper_chars(p, underlying_cast(scancode));
    p = cpy(p, "}"_av);
    buffer.append(output, p);
}

void append_unicode(std::string& output, kbdtypes::KbdFlags flag, uint16_t unicode)
{
    AppendBuffer buffer;
    char* p = buffer.buffer;
    p = cpy(p, "{KbdFlags=0x"_av);
    p = int_to_fixed_hexadecimal_upper_chars(p, underlying_cast(flag));
    p = cpy(p, ", Unicode=0x"_av);
    p = int_to_fixed_hexadecimal_upper_chars(p, unicode);
    p = cpy(p, "}"_av);
    buffer.append(output, p);
}

void append_invalidate(std::string& output, Rect rect)
{
    AppendBuffer buffer;
    char* p = buffer.buffer;
    p = cpy(p, "{Invalidate={"_av);
    p = cpy(p, int_to_decimal_chars(rect.x));
    p = cpy(p, ", "_av);
    p = cpy(p, int_to_decimal_chars(rect.y));
    p = cpy(p, ", "_av);
    p = cpy(p, int_to_decimal_chars(rect.cx));
    p = cpy(p, ", "_av);
    p = cpy(p, int_to_decimal_chars(rect.cy));
    p = cpy(p, "}"_av);
    buffer.append(output, p);
}

} // anonymous namespace


RED_AUTO_TEST_CASE(TestFront)
{
    ClientInfo info = make_client_info();

    Inifile ini;
    ini.set<cfg::client::persistent_disk_bitmap_cache>(false);
    ini.set<cfg::client::cache_waiting_list>(true);
    ini.set<cfg::mod_rdp::persistent_disk_bitmap_cache>(false);
    ini.set<cfg::video::png_interval>(std::chrono::seconds{300});
    ini.set<cfg::video::wrm_color_depth_selection_strategy>(ColorDepthSelectionStrategy::depth24);
    ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);
    const uint32_t verbose = 3;

    // Uncomment the code block below to generate testing data.
    //int nodelay = 1;
    //if (-1 == setsockopt( one_shot_server.sck, IPPROTO_TCP, TCP_NODELAY
    //                    , (char *)&nodelay, sizeof(nodelay))) {
    //    LOG(LOG_INFO, "Failed to set socket TCP_NODELAY option on client socket");
    //}
    //SocketTransport front_trans( "RDP Client", one_shot_server.sck, "0.0.0.0", 0
    //                           , verbose, 0);

    // Comment the code block below to generate testing data.
    FrontTransport front_trans(cstr_array_view(dump_front::indata));

    LCGRandom gen1;

    ini.set<cfg::client::tls_support>(false);
    ini.set<cfg::client::tls_fallback_legacy>(true);
    ini.set<cfg::client::bogus_user_id>(false);
    ini.set<cfg::client::rdp_compression>(RdpCompression::none);
    ini.set<cfg::client::fast_path>(false);
    ini.set<cfg::globals::is_rec>(true);
    ini.set<cfg::video::capture_flags>(CaptureFlags::wrm);
    ini.set<cfg::globals::handshake_timeout>(std::chrono::seconds::zero());

    EventManager event_manager;
    auto& events = event_manager.get_events();
    NullSessionLog session_log;

    RED_TEST_PASSPOINT();

    MyFront front(events, session_log, front_trans, gen1, ini);
    null_mod no_mod;

    gdi::NullOsd osd;

    while (!front.is_up_and_running()) {
        front.incoming(no_mod);
        RED_CHECK(event_manager.is_empty());
    }

    // LOG(LOG_INFO, "hostname=%s", front.client_info.hostname);

    // int client_sck = ip_connect("10.10.47.36", 3389, 3, 1000);
    // std::string error_message;
    // SocketTransport t( name
    //                  , client_sck
    //                  , "10.10.47.36"
    //                  , 3389
    //                  , verbose
    //                  , &error_message
    //                  );

    FrontTransport t(cstr_array_view(dump2008::indata));

    Theme theme;

    std::array<uint8_t, 28> server_auto_reconnect_packet {};
    ModRDPParams mod_rdp_params( "administrateur"
                                , "S3cur3!1nux"
                                , "10.10.47.36"
                                , "10.10.43.33"
                                , kbdtypes::KeyLocks::NumLock
                                , global_font()
                                , theme
                                , server_auto_reconnect_packet
                                , ini.get_mutable_ref<cfg::context::close_box_extra_message>()
                                , std::move(ini.get_mutable_ref<cfg::context::redirection_password_or_cookie>())
                                , RDPVerbose(0)
                                );
    mod_rdp_params.device_id                       = "device_id";
    mod_rdp_params.enable_tls                      = false;
    mod_rdp_params.enable_nla                      = false;
    //mod_rdp_params.enable_krb                      = false;
    //mod_rdp_params.enable_clipboard                = true;
    mod_rdp_params.enable_fastpath                 = false;
    mod_rdp_params.disabled_orders                 = TS_NEG_MEM3BLT_INDEX | TS_NEG_DRAWNINEGRID_INDEX | TS_NEG_MULTI_DRAWNINEGRID_INDEX |
                                                     TS_NEG_SAVEBITMAP_INDEX | TS_NEG_MULTIDSTBLT_INDEX | TS_NEG_MULTIPATBLT_INDEX |
                                                     TS_NEG_MULTISCRBLT_INDEX | TS_NEG_MULTIOPAQUERECT_INDEX | TS_NEG_FAST_INDEX_INDEX |
                                                     TS_NEG_POLYGON_SC_INDEX | TS_NEG_POLYGON_CB_INDEX | TS_NEG_POLYLINE_INDEX |
                                                     TS_NEG_FAST_GLYPH_INDEX | TS_NEG_ELLIPSE_SC_INDEX | TS_NEG_ELLIPSE_CB_INDEX;
    mod_rdp_params.enable_new_pointer              = false;
    //mod_rdp_params.rdp_compression                 = 0;
    //mod_rdp_params.error_message                   = nullptr;
    //mod_rdp_params.disconnect_on_logon_user_change = false;
    mod_rdp_params.open_session_timeout            = 5s;
    //mod_rdp_params.certificate_change_action       = 0;
    //mod_rdp_params.extra_orders                    = "";
    mod_rdp_params.verbose = RDPVerbose(verbose);

    // To always get the same client random, in tests
    LCGRandom gen2;

    NullLicenseStore license_store;
    const ChannelsAuthorizations channels_authorizations{};
    ModRdpFactory mod_rdp_factory;

    FileValidatorService * file_validator_service = nullptr;

    TLSClientParams tls_client_params;
    RedirectionInfo redir_info;

    auto mod = new_mod_rdp(
        t, front.gd(), osd, events, session_log,
        front, info, redir_info, gen2, channels_authorizations, mod_rdp_params,
        tls_client_params, license_store, ini, file_validator_service,
        mod_rdp_factory);

    // incoming connexion data
    RED_CHECK_EQUAL(front.screen_info().width, 1024);
    RED_CHECK_EQUAL(front.screen_info().height, 768);

    // Force Front to be up and running after Deactivation-Reactivation
    //  Sequence initiated by mod_rdp.

    RED_TEST_PASSPOINT();

    int count = 0;
    int n = 38;
    detail::ProtectedEventContainer::get_events(events)[0]->alarm.fd = 0;
    event_manager.get_writable_time_base().monotonic_time = MonotonicTimePoint{1s};
    for (; count < n && !event_manager.is_empty(); ++count) {
        event_manager.execute_events([](int){return true;}, false);
    }

    RED_CHECK_EQ(count, n);
    RED_CHECK(mod->is_up_and_running());
    RED_CHECK(!front.is_up_and_running());
//    front.dump_png("trace_w2008_");
}

namespace TestFrontData
{
namespace
{

struct Mod : null_mod
{
    void rdp_input_mouse(int device_flags, int x, int y) override
    {
        append_mouse(session_log.messages, device_flags, x, y);
    }

    void rdp_input_scancode(
        KbdFlags flags,
        Scancode scancode,
        uint32_t /*time*/,
        Keymap const& /*keymap*/) override
    {
        append_scancode(session_log.messages, flags, scancode);
    }

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override
    {
        append_unicode(session_log.messages, flag, unicode);
    }

    void rdp_input_synchronize(KeyLocks locks) override
    {
        append_key_locks(session_log.messages, locks);
    }

    void rdp_input_invalidate(Rect rect) override
    {
        append_invalidate(session_log.messages, rect);
    }

    std::string events()
    {
        return session_log.events();
    }

    SessionLogTest session_log;
};

struct Gd : gdi::NullGraphic
{
    bool is_slased_circle_cursor = false;

    void cached_pointer(gdi::CachePointerIndex cache_idx) override
    {
        is_slased_circle_cursor = cache_idx.is_predefined_pointer()
                                && cache_idx.as_predefined_pointer() == PredefinedPointer::SlashedCircle;
    }
};

struct FrontCtx
{
    LCGRandom gen1;
    CryptoContext cctx;
    TpduBuffer tpdu_buf;
    FrontTransport trans;
    Mod& mod;
    Front front;

    FrontCtx(
        EventContainer& events,
        Mod& mod,
        Inifile& ini,
        Front::GuestParameters guest_params = {})
    : trans(FrontTransport(cstr_array_view(dump_front::indata)))
    , mod(mod)
    , front(events, mod.session_log, trans, gen1, ini, cctx, guest_params)
    {
        null_mod no_mod;
        front_process(tpdu_buf, front, trans, no_mod);
    }
};

using KbdFlags = kbdtypes::KbdFlags;
using Scancode = kbdtypes::Scancode;
using KeyLocks = kbdtypes::KeyLocks;
using int_scancode = std::underlying_type_t<Scancode>;

enum class Shortcut : int_scancode
{
    Take = int_scancode(Scancode::F9),
    Give = int_scancode(Scancode::F10),
    Kill = int_scancode(Scancode::F5),
    ToggleGraphics = int_scancode(Scancode::F8),
};

std::string shortcut(FrontCtx& front_ctx, Shortcut scancode)
{
    auto& mod = front_ctx.mod;
    front_ctx.front.input_event_scancode(KbdFlags::NoFlags, Scancode::LCtrl, mod, 0);
    front_ctx.front.input_event_scancode(KbdFlags::NoFlags, Scancode::LShift, mod, 0);
    front_ctx.front.input_event_scancode(KbdFlags::NoFlags, Scancode::LAlt, mod, 0);
    front_ctx.front.input_event_scancode(KbdFlags::NoFlags, Scancode(scancode), mod, 0);
    front_ctx.front.input_event_scancode(KbdFlags::Release, Scancode(scancode), mod, 0);
    front_ctx.front.input_event_scancode(KbdFlags::Release, Scancode::LAlt, mod, 0);
    front_ctx.front.input_event_scancode(KbdFlags::Release, Scancode::LShift, mod, 0);
    front_ctx.front.input_event_scancode(KbdFlags::Release, Scancode::LCtrl, mod, 0);
    return mod.events();
}

std::string keyA(FrontCtx& front_ctx)
{
    auto& mod = front_ctx.mod;
    front_ctx.front.input_event_scancode(KbdFlags::NoFlags, Scancode::A, mod, 0);
    front_ctx.front.input_event_scancode(KbdFlags::Release, Scancode::A, mod, 0);
    return mod.events();
}

inline constexpr auto keyA_av =
    "{KbdFlags=0x0000, Scancode=0x1E}, {KbdFlags=0x8000, Scancode=0x1E}"_av;
inline constexpr auto no_keys =
    ""_av;
inline constexpr auto keys_give =
    "{KbdFlags=0x0000, Scancode=0x1D}, "
    "{KbdFlags=0x0000, Scancode=0x2A}, "
    "{KbdFlags=0x0000, Scancode=0x38}, "
    "{KbdFlags=0x0000, Scancode=0x44}, "
    "{KbdFlags=0x8000, Scancode=0x44}, "
    "{KbdFlags=0x8000, Scancode=0x38}, "
    "{KbdFlags=0x8000, Scancode=0x2A}, "
    "{KbdFlags=0x8000, Scancode=0x1D}"_av;
inline constexpr auto keys_take =
    "{KbdFlags=0x0000, Scancode=0x1D}, "
    "{KbdFlags=0x0000, Scancode=0x2A}, "
    "{KbdFlags=0x0000, Scancode=0x38}, "
    "{KbdFlags=0x0000, Scancode=0x43}, "
    "{KbdFlags=0x8000, Scancode=0x43}, "
    "{KbdFlags=0x8000, Scancode=0x38}, "
    "{KbdFlags=0x8000, Scancode=0x2A}, "
    "{KbdFlags=0x8000, Scancode=0x1D}"_av;
inline constexpr auto keys_take_intercept =
    "{KbdFlags=0x0000, Scancode=0x1D}, "
    "{KbdFlags=0x0000, Scancode=0x2A}, "
    "{KbdFlags=0x0000, Scancode=0x38}, "
    "{KbdFlags=0x8000, Scancode=0x43}, "
    "{KbdFlags=0x8000, Scancode=0x38}, "
    "{KbdFlags=0x8000, Scancode=0x2A}, "
    "{KbdFlags=0x8000, Scancode=0x1D}"_av;


std::string uniA(FrontCtx& front_ctx)
{
    auto& mod = front_ctx.mod;
    front_ctx.front.input_event_unicode(KbdFlags::NoFlags, 'a', mod);
    front_ctx.front.input_event_unicode(KbdFlags::Release, 'a', mod);
    return mod.events();
}

inline constexpr auto uniA_av = "{KbdFlags=0x0000, Unicode=0x0061}, {KbdFlags=0x8000, Unicode=0x0061}"_av;
inline constexpr auto no_unis = ""_av;


std::string locks(FrontCtx& front_ctx, KeyLocks locks)
{
    auto& mod = front_ctx.mod;
    front_ctx.front.input_event_synchronize(locks, mod);
    return mod.events();
}

inline constexpr auto lock_caps = "{KeyLocks=0x04}"_av;
inline constexpr auto lock_scroll = "{KeyLocks=0x01}"_av;
inline constexpr auto no_locks = ""_av;


std::string mouse(FrontCtx& front_ctx)
{
    auto& mod = front_ctx.mod;
    front_ctx.front.input_mouse(1, 1, SlowPath::PTRFLAGS_BUTTON1 | SlowPath::PTRFLAGS_DOWN, mod);
    front_ctx.front.input_mouse(1, 1, SlowPath::PTRFLAGS_BUTTON1, mod);
    return mod.events();
}

inline constexpr auto mouse_pos = "{flags=0x9000, x=1, y=1}, {flags=0x1000, x=1, y=1}"_av;
inline constexpr auto no_mouse = ""_av;


inline constexpr auto user_to_guest_log_event =
    "{KbdFlags=0x0000, Scancode=0x1D}, {KbdFlags=0x0000, Scancode=0x2A}, {KbdFlags=0x0000, Scancode=0x38}, {KeyLocks=0x04}"
    "SESSION_INVITE_CONTROL_OWNERSHIP_CHANGED new_control_owner=\"guest-1\" control_owner=\"user\"\n"_av;
inline constexpr auto user_mask_log_event =
    "{KbdFlags=0x0000, Scancode=0x1D}, {KbdFlags=0x0000, Scancode=0x2A}, {KbdFlags=0x0000, Scancode=0x38}"
    "SESSION_INVITE_GUEST_VIEW_CHANGED state=\"masked\" control_owner=\"user\"\n"
    ", {KbdFlags=0x8000, Scancode=0x42}, "
    "{KbdFlags=0x8000, Scancode=0x38}, {KbdFlags=0x8000, Scancode=0x2A}, {KbdFlags=0x8000, Scancode=0x1D}"_av;
inline constexpr auto guest_to_user_and_mask_log_event =
    "{KeyLocks=0x01}"
    "SESSION_INVITE_CONTROL_OWNERSHIP_CHANGED new_control_owner=\"user\" control_owner=\"guest-1\"\n"
    "SESSION_INVITE_GUEST_VIEW_CHANGED state=\"masked\" control_owner=\"user\"\n"
    ", {KbdFlags=0x8000, Scancode=0x42}, "
    "{KbdFlags=0x8000, Scancode=0x38}, {KbdFlags=0x8000, Scancode=0x2A}, {KbdFlags=0x8000, Scancode=0x1D}"_av;
inline constexpr auto user_unmask_log_event =
    "{KbdFlags=0x0000, Scancode=0x1D}, {KbdFlags=0x0000, Scancode=0x2A}, {KbdFlags=0x0000, Scancode=0x38}, "
    "{Invalidate={0, 0, 1024, 768}"
    "SESSION_INVITE_GUEST_VIEW_CHANGED state=\"unmasked\" control_owner=\"user\"\n"
    ", {KbdFlags=0x8000, Scancode=0x42}, "
    "{KbdFlags=0x8000, Scancode=0x38}, {KbdFlags=0x8000, Scancode=0x2A}, {KbdFlags=0x8000, Scancode=0x1D}"_av;
inline constexpr auto guest_to_user_and_kill_log_event =
    "SESSION_INVITE_GUEST_KILLED name=\"guest-1\" control_owner=\"guest-1\"\n"_av;
inline constexpr auto user_kill_log_event =
    "{KbdFlags=0x0000, Scancode=0x1D}, {KbdFlags=0x0000, Scancode=0x2A}, {KbdFlags=0x0000, Scancode=0x38}"
    "SESSION_INVITE_GUEST_KILLED name=\"guest-1\" control_owner=\"user\"\n, "
    "{KbdFlags=0x8000, Scancode=0x3F}, "
    "{KbdFlags=0x8000, Scancode=0x38}, {KbdFlags=0x8000, Scancode=0x2A}, {KbdFlags=0x8000, Scancode=0x1D}"_av;


int draw(FrontCtx& front_ctx)
{
    front_ctx.front.sync();
    front_ctx.trans.counter = 0;
    front_ctx.front.draw(RDPOpaqueRect({0, 0, 10, 10}, RDPColor()), Rect(0, 0, 10, 10), gdi::ColorCtx::depth24());
    front_ctx.front.sync();
    return front_ctx.trans.counter;
}

void init_ini(Inifile& ini)
{
    ini.set<cfg::client::persistent_disk_bitmap_cache>(false);
    ini.set<cfg::client::cache_waiting_list>(true);
    ini.set<cfg::client::tls_support>(false);
    ini.set<cfg::client::tls_fallback_legacy>(true);
    ini.set<cfg::client::bogus_user_id>(false);
    ini.set<cfg::client::rdp_compression>(RdpCompression::none);
    ini.set<cfg::client::fast_path>(false);
    ini.set<cfg::globals::handshake_timeout>(std::chrono::seconds::zero());
}

template<class F>
void sharing_test(bool enable_shared_control, F f)
{
    Inifile ini;
    init_ini(ini);
    Mod mod;
    SessionLogTest& session_log = mod.session_log;
    EventManager event_manager;
    auto& events = event_manager.get_events();

    FrontCtx user(events, mod, ini);
    RED_CHECK(user.front.is_up_and_running());

    Gd gd;
    user.front.add_graphic(gd);

    bool guest_killed = false;
    FrontCtx guest(events, mod, ini, Front::GuestParameters{
        .is_guest = true,
        .enable_shared_control = enable_shared_control,
        .screen_info = user.front.get_client_info().screen_info,
        .kill_fn = [](void* killed) { *static_cast<bool*>(killed) = true; },
        .fn_ctx = &guest_killed,
    });
    RED_CHECK(guest.front.is_up_and_running());

    RED_CHECK(event_manager.is_empty());

    // start test
    ini.set_acl<cfg::globals::auth_user>("user");

    std::memcpy(const_cast<ClientInfo&>(user.front.get_client_info()).username, "user-original", 5); // NOLINT
    std::memcpy(const_cast<ClientInfo&>(guest.front.get_client_info()).username, "guest-1", 7); // NOLINT

    RED_CHECK(session_log.events() == ""_av);
    RED_CHECK(!gd.is_slased_circle_cursor);

    user.front.add_guest(guest.front, session_log);

    f(user, guest, mod, gd, guest_killed);
}

} // namespace TestFrontData
} // anonymous namespace

RED_AUTO_TEST_CASE(TestViewAndControlSharingFront)
{
    using namespace TestFrontData;

sharing_test(true, [](FrontCtx& user, FrontCtx& guest, Mod& mod, Gd& gd, bool& guest_killed){
    RED_CHECK(mod.session_log.events() == "SESSION_INVITE_GUEST_CONNECTION guest=\"guest-1\" mode=\"view-control\"\n"_av);

    RED_TEST_CONTEXT("user control") {
        RED_CHECK(!gd.is_slased_circle_cursor);
        RED_CHECK(keyA(guest) == no_keys);
        RED_CHECK(keyA(user) == keyA_av);
        RED_CHECK(uniA(guest) == no_unis);
        RED_CHECK(uniA(user) == uniA_av);
        RED_CHECK(locks(guest, KeyLocks::CapsLock) == no_locks);
        RED_CHECK(locks(user, KeyLocks::ScrollLock) == lock_scroll);
        RED_CHECK(mouse(guest) == no_mouse);
        RED_CHECK(mouse(user) == mouse_pos);

        RED_TEST_CONTEXT("guest send Take") {
            RED_CHECK(shortcut(guest, Shortcut::Take) == no_keys);
            RED_CHECK(!gd.is_slased_circle_cursor);
            RED_CHECK(keyA(guest) == no_keys);
            RED_CHECK(keyA(user) == keyA_av);
            RED_CHECK(uniA(guest) == no_unis);
            RED_CHECK(uniA(user) == uniA_av);
            RED_CHECK(locks(guest, KeyLocks::CapsLock) == no_locks);
            RED_CHECK(locks(user, KeyLocks::ScrollLock) == lock_scroll);
            RED_CHECK(mouse(guest) == no_mouse);
            RED_CHECK(mouse(user) == mouse_pos);
        }

        RED_TEST_CONTEXT("guest send Give") {
            RED_CHECK(shortcut(guest, Shortcut::Give) == no_keys);
            RED_CHECK(!gd.is_slased_circle_cursor);
            RED_CHECK(keyA(guest) == no_keys);
            RED_CHECK(keyA(user) == keyA_av);
            RED_CHECK(uniA(guest) == no_unis);
            RED_CHECK(uniA(user) == uniA_av);
            RED_CHECK(locks(guest, KeyLocks::CapsLock) == no_locks);
            RED_CHECK(locks(user, KeyLocks::ScrollLock) == lock_scroll);
            RED_CHECK(mouse(guest) == no_mouse);
            RED_CHECK(mouse(user) == mouse_pos);
        }

        RED_TEST_CONTEXT("user send Take") {
            // shortcut scancode is skipped
            RED_CHECK(shortcut(user, Shortcut::Take) == keys_take_intercept);
            RED_CHECK(!gd.is_slased_circle_cursor);
            RED_CHECK(keyA(guest) == no_keys);
            RED_CHECK(keyA(user) == keyA_av);
            RED_CHECK(uniA(guest) == no_unis);
            RED_CHECK(uniA(user) == uniA_av);
            RED_CHECK(locks(guest, KeyLocks::CapsLock) == no_locks);
            RED_CHECK(locks(user, KeyLocks::ScrollLock) == lock_scroll);
            RED_CHECK(mouse(guest) == no_mouse);
            RED_CHECK(mouse(user) == mouse_pos);
        }
    }

    RED_TEST_CONTEXT("guest control") {
        RED_CHECK(shortcut(user, Shortcut::Give) == user_to_guest_log_event);
        RED_CHECK(gd.is_slased_circle_cursor);
        RED_CHECK(keyA(guest) == keyA_av);
        RED_CHECK(keyA(user) == no_keys);
        RED_CHECK(uniA(guest) == uniA_av);
        RED_CHECK(uniA(user) == no_unis);
        RED_CHECK(locks(guest, KeyLocks::CapsLock) == lock_caps);
        RED_CHECK(locks(user, KeyLocks::ScrollLock) == no_locks);
        RED_CHECK(mouse(guest) == mouse_pos);
        RED_CHECK(mouse(user) == no_mouse);

        RED_TEST_CONTEXT("guest send Take") {
            RED_CHECK(shortcut(guest, Shortcut::Take) == keys_take);
            RED_CHECK(gd.is_slased_circle_cursor);
            RED_CHECK(keyA(guest) == keyA_av);
            RED_CHECK(keyA(user) == no_keys);
            RED_CHECK(uniA(guest) == uniA_av);
            RED_CHECK(uniA(user) == no_unis);
            RED_CHECK(locks(guest, KeyLocks::CapsLock) == lock_caps);
            RED_CHECK(locks(user, KeyLocks::ScrollLock) == no_locks);
            RED_CHECK(mouse(guest) == mouse_pos);
            RED_CHECK(mouse(user) == no_mouse);
        }

        RED_TEST_CONTEXT("guest send Give") {
            RED_CHECK(shortcut(guest, Shortcut::Give) == keys_give);
            RED_CHECK(gd.is_slased_circle_cursor);
            RED_CHECK(keyA(guest) == keyA_av);
            RED_CHECK(keyA(user) == no_keys);
            RED_CHECK(uniA(guest) == uniA_av);
            RED_CHECK(uniA(user) == no_unis);
            RED_CHECK(locks(guest, KeyLocks::CapsLock) == lock_caps);
            RED_CHECK(locks(user, KeyLocks::ScrollLock) == no_locks);
            RED_CHECK(mouse(guest) == mouse_pos);
            RED_CHECK(mouse(user) == no_mouse);
        }

        RED_TEST_CONTEXT("user send Give") {
            RED_CHECK(shortcut(user, Shortcut::Give) == no_keys);
            RED_CHECK(gd.is_slased_circle_cursor);
            RED_CHECK(keyA(guest) == keyA_av);
            RED_CHECK(keyA(user) == no_keys);
            RED_CHECK(uniA(guest) == uniA_av);
            RED_CHECK(uniA(user) == no_unis);
            RED_CHECK(locks(guest, KeyLocks::CapsLock) == lock_caps);
            RED_CHECK(locks(user, KeyLocks::ScrollLock) == no_locks);
            RED_CHECK(mouse(guest) == mouse_pos);
            RED_CHECK(mouse(user) == no_mouse);
        }
    }

    RED_CHECK(draw(guest) == 1);
    RED_CHECK(draw(user) == 1);

    RED_TEST_CONTEXT("toggle graphics") {
        RED_TEST_CONTEXT("disable graphics") {
            RED_CHECK(shortcut(user, Shortcut::ToggleGraphics) == guest_to_user_and_mask_log_event);
            RED_CHECK(!gd.is_slased_circle_cursor);
            RED_CHECK(keyA(guest) == no_keys);
            RED_CHECK(keyA(user) == keyA_av);
            RED_CHECK(draw(guest) == 0);
            RED_CHECK(draw(user) == 1);

            RED_TEST_CONTEXT("give control (disabled)") {
                RED_CHECK(shortcut(user, Shortcut::Give) == keys_give);
                RED_CHECK(!gd.is_slased_circle_cursor);
                RED_CHECK(keyA(guest) == no_keys);
                RED_CHECK(keyA(user) == keyA_av);
                RED_CHECK(draw(guest) == 0);
                RED_CHECK(draw(user) == 1);
            }

            RED_TEST_CONTEXT("restore graphics") {
                RED_CHECK(shortcut(user, Shortcut::ToggleGraphics) == user_unmask_log_event);
                RED_CHECK(!gd.is_slased_circle_cursor);
                RED_CHECK(keyA(guest) == no_keys);
                RED_CHECK(keyA(user) == keyA_av);
                RED_CHECK(draw(guest) == 1);
                RED_CHECK(draw(user) == 1);
            }
        }

        RED_TEST_CONTEXT("disable graphics when guest") {
            RED_CHECK(shortcut(user, Shortcut::Give) == user_to_guest_log_event);
            RED_CHECK(gd.is_slased_circle_cursor);
            RED_CHECK(keyA(guest) == keyA_av);
            RED_CHECK(keyA(user) == no_keys);
            RED_CHECK(shortcut(user, Shortcut::ToggleGraphics) == guest_to_user_and_mask_log_event);
            RED_CHECK(!gd.is_slased_circle_cursor);
            RED_CHECK(keyA(guest) == no_keys);
            RED_CHECK(keyA(user) == keyA_av);
            RED_CHECK(draw(guest) == 0);
            RED_CHECK(draw(user) == 1);

            RED_TEST_CONTEXT("restore graphics") {
                RED_CHECK(shortcut(user, Shortcut::ToggleGraphics) == user_unmask_log_event);
                RED_CHECK(!gd.is_slased_circle_cursor);
                RED_CHECK(keyA(guest) == no_keys);
                RED_CHECK(keyA(user) == keyA_av);
                RED_CHECK(draw(guest) == 1);
                RED_CHECK(draw(user) == 1);
            }
        }
    }

    RED_TEST_CONTEXT("killed guest control") {
        RED_CHECK(shortcut(user, Shortcut::Give) == user_to_guest_log_event);
        RED_CHECK(gd.is_slased_circle_cursor);
        RED_CHECK(!guest_killed);
        RED_CHECK(shortcut(user, Shortcut::Kill) == guest_to_user_and_kill_log_event);
        RED_CHECK(guest_killed);
        user.front.remove_guest(guest.front);
        RED_CHECK(!gd.is_slased_circle_cursor);
        RED_CHECK(mod.session_log.events() ==
            "SESSION_INVITE_GUEST_DISCONNECTION guest=\"guest-1\" duration=\"00:00:00\"\n"_av);
        RED_CHECK(keyA(user) == keyA_av);
        RED_CHECK(uniA(user) == uniA_av);
        RED_CHECK(locks(user, KeyLocks::ScrollLock) == lock_scroll);
        RED_CHECK(mouse(user) == mouse_pos);
    }

});
}

RED_AUTO_TEST_CASE(TestViewOnlySharingFront)
{
    using namespace TestFrontData;

sharing_test(false, [](FrontCtx& user, FrontCtx& guest, Mod& mod, Gd& gd, bool& guest_killed){
    RED_CHECK(mod.session_log.events() == "SESSION_INVITE_GUEST_CONNECTION guest=\"guest-1\" mode=\"view-only\"\n"_av);

    RED_TEST_CONTEXT("user control") {
        RED_CHECK(!gd.is_slased_circle_cursor);
        RED_CHECK(keyA(guest) == no_keys);
        RED_CHECK(keyA(user) == keyA_av);
        RED_CHECK(uniA(guest) == no_unis);
        RED_CHECK(uniA(user) == uniA_av);
        RED_CHECK(locks(guest, KeyLocks::CapsLock) == no_locks);
        RED_CHECK(locks(user, KeyLocks::ScrollLock) == lock_scroll);
        RED_CHECK(mouse(guest) == no_mouse);
        RED_CHECK(mouse(user) == mouse_pos);

        RED_TEST_CONTEXT("guest send Take") {
            RED_CHECK(shortcut(guest, Shortcut::Take) == no_keys);
            RED_CHECK(!gd.is_slased_circle_cursor);
            RED_CHECK(keyA(guest) == no_keys);
            RED_CHECK(keyA(user) == keyA_av);
            RED_CHECK(uniA(guest) == no_unis);
            RED_CHECK(uniA(user) == uniA_av);
            RED_CHECK(locks(guest, KeyLocks::CapsLock) == no_locks);
            RED_CHECK(locks(user, KeyLocks::ScrollLock) == lock_scroll);
            RED_CHECK(mouse(guest) == no_mouse);
            RED_CHECK(mouse(user) == mouse_pos);
        }

        RED_TEST_CONTEXT("guest send Give") {
            RED_CHECK(shortcut(guest, Shortcut::Give) == no_keys);
            RED_CHECK(!gd.is_slased_circle_cursor);
            RED_CHECK(keyA(guest) == no_keys);
            RED_CHECK(keyA(user) == keyA_av);
            RED_CHECK(uniA(guest) == no_unis);
            RED_CHECK(uniA(user) == uniA_av);
            RED_CHECK(locks(guest, KeyLocks::CapsLock) == no_locks);
            RED_CHECK(locks(user, KeyLocks::ScrollLock) == lock_scroll);
            RED_CHECK(mouse(guest) == no_mouse);
            RED_CHECK(mouse(user) == mouse_pos);
        }

        RED_TEST_CONTEXT("user send Take") {
            // shortcut scancode is skipped
            RED_CHECK(shortcut(user, Shortcut::Take) == keys_take);
            RED_CHECK(!gd.is_slased_circle_cursor);
            RED_CHECK(keyA(guest) == no_keys);
            RED_CHECK(keyA(user) == keyA_av);
            RED_CHECK(uniA(guest) == no_unis);
            RED_CHECK(uniA(user) == uniA_av);
            RED_CHECK(locks(guest, KeyLocks::CapsLock) == no_locks);
            RED_CHECK(locks(user, KeyLocks::ScrollLock) == lock_scroll);
            RED_CHECK(mouse(guest) == no_mouse);
            RED_CHECK(mouse(user) == mouse_pos);
        }
    }

    RED_TEST_CONTEXT("guest control (disabled)") {
        RED_CHECK(shortcut(user, Shortcut::Give) == keys_give);
        RED_CHECK(!gd.is_slased_circle_cursor);
        RED_CHECK(keyA(guest) == no_keys);
        RED_CHECK(keyA(user) == keyA_av);
        RED_CHECK(uniA(guest) == no_unis);
        RED_CHECK(uniA(user) == uniA_av);
        RED_CHECK(locks(guest, KeyLocks::CapsLock) == no_locks);
        RED_CHECK(locks(user, KeyLocks::ScrollLock) == lock_scroll);
        RED_CHECK(mouse(guest) == no_mouse);
        RED_CHECK(mouse(user) == mouse_pos);
    }

    RED_CHECK(draw(guest) == 1);
    RED_CHECK(draw(user) == 1);

    RED_TEST_CONTEXT("toggle graphics") {
        RED_TEST_CONTEXT("disable graphics") {
            RED_CHECK(shortcut(user, Shortcut::ToggleGraphics) == user_mask_log_event);
            RED_CHECK(!gd.is_slased_circle_cursor);
            RED_CHECK(keyA(guest) == no_keys);
            RED_CHECK(keyA(user) == keyA_av);
            RED_CHECK(draw(guest) == 0);
            RED_CHECK(draw(user) == 1);

            RED_TEST_CONTEXT("give control (disabled)") {
                RED_CHECK(shortcut(user, Shortcut::Give) == keys_give);
                RED_CHECK(!gd.is_slased_circle_cursor);
                RED_CHECK(keyA(guest) == no_keys);
                RED_CHECK(keyA(user) == keyA_av);
                RED_CHECK(draw(guest) == 0);
                RED_CHECK(draw(user) == 1);
            }

            RED_TEST_CONTEXT("restore graphics") {
                RED_CHECK(shortcut(user, Shortcut::ToggleGraphics) == user_unmask_log_event);
                RED_CHECK(!gd.is_slased_circle_cursor);
                RED_CHECK(keyA(guest) == no_keys);
                RED_CHECK(keyA(user) == keyA_av);
                RED_CHECK(draw(guest) == 1);
                RED_CHECK(draw(user) == 1);
            }
        }

        RED_TEST_CONTEXT("disable graphics when guest") {
            RED_CHECK(shortcut(user, Shortcut::ToggleGraphics) == user_mask_log_event);
            RED_CHECK(!gd.is_slased_circle_cursor);
            RED_CHECK(keyA(guest) == no_keys);
            RED_CHECK(keyA(user) == keyA_av);
            RED_CHECK(draw(guest) == 0);
            RED_CHECK(draw(user) == 1);

            RED_TEST_CONTEXT("restore graphics") {
                RED_CHECK(shortcut(user, Shortcut::ToggleGraphics) == user_unmask_log_event);
                RED_CHECK(!gd.is_slased_circle_cursor);
                RED_CHECK(keyA(guest) == no_keys);
                RED_CHECK(keyA(user) == keyA_av);
                RED_CHECK(draw(guest) == 1);
                RED_CHECK(draw(user) == 1);
            }
        }
    }

    RED_TEST_CONTEXT("killed guest") {
        RED_CHECK(!gd.is_slased_circle_cursor);
        RED_CHECK(!guest_killed);
        RED_CHECK(shortcut(user, Shortcut::Kill) == user_kill_log_event);
        RED_CHECK(guest_killed);
        user.front.remove_guest(guest.front);
        RED_CHECK(!gd.is_slased_circle_cursor);
        RED_CHECK(mod.session_log.events() ==
            "SESSION_INVITE_GUEST_DISCONNECTION guest=\"guest-1\" duration=\"00:00:00\"\n"_av);
        RED_CHECK(keyA(user) == keyA_av);
        RED_CHECK(uniA(user) == uniA_av);
        RED_CHECK(locks(user, KeyLocks::ScrollLock) == lock_scroll);
        RED_CHECK(mouse(user) == mouse_pos);
    }

});

}

RED_AUTO_TEST_CASE_WD(TestGuestCtx, wd)
{
    GuestCtx guest_ctx;
    EventManager event_manager;
    auto& events = event_manager.get_events();

    using namespace TestFrontData;

    Mod mod;
    Inifile ini;
    init_ini(ini);
    FrontCtx front_ctx(events, mod, ini);

    std::string sck_filename = "front2_{SID}_eFiuUrCubBc=_1.sck";

    // change current directory for unix socket name limitation
    // path is limited to 108 bytes (92 for portability)
    char dir[2024] {};
    RED_REQUIRE((getcwd(dir, sizeof(dir)) != nullptr));

    auto sck_path = wd.add_file(sck_filename);

    RED_REQUIRE_MESSAGE(chdir(wd.dirname()) == 0, strerror(errno));

    auto result = guest_ctx.start(
        "."_av, "{SID}"_av,
        events, front_ctx.front, front_ctx.mod,
        mod.session_log,
        100ms,
        front_ctx.gen1, ini,
        true
    );

    RED_CHECK(result.errnum == 0);
    RED_CHECK(result.errmsg == nullptr);
    RED_CHECK(guest_ctx.sck_path() == "./front2_{SID}_1.sck");
    RED_CHECK(guest_ctx.sck_password() == "eFiuUrCubBdoWhO9oFN681gSe86QjqitSECHMYAfRbY="_av);

    auto fd = local_connect(guest_ctx.sck_path(), 100ms, true);
    RED_REQUIRE_MESSAGE(fd.is_open(), strerror(errno));

    RED_REQUIRE(detail::ProtectedEventContainer::get_events(events).size() == 1);

    front_ctx.gen1.set_seed(0);

    event_manager.execute_events([](int /*fd*/){ return true; }, false);
    RED_REQUIRE(guest_ctx.has_front());

    RED_REQUIRE_MESSAGE(chdir(dir) == 0, strerror(errno));

    wd.remove_file(sck_filename);
    RED_CHECK_WORKSPACE(wd);

    RED_REQUIRE(fcntl(fd.fd(), F_SETFL, fcntl(fd.fd(), F_GETFL) & ~O_NONBLOCK) == 0);

    auto input = cstr_array_view(dump_front::indata);
    RED_REQUIRE(write(fd.fd(), input.data(), input.size()) == static_cast<ssize_t>(input.size()));
    event_manager.execute_events([](int /*fd*/){ return true; }, false);

    RED_CHECK(!guest_ctx.has_front());
    RED_CHECK(mod.session_log.events() ==
        "SESSION_INVITE_GUEST_CONNECTION_REJECTED name=\"guest-1\" reason=\"bad password\"\n"_av);
}
