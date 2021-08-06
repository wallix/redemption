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
Copyright (C) Wallix 2010-2020
Author(s): Jonathan Poelen
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/compare_collection.hpp"
#include "test_only/js_auto_test_case.hpp"

#include "utils/sugar/overload.hpp"
#include "red_emscripten/val.hpp"

struct U16Array
{
    u16_array_view a;
};
// for IDE coloration only
bool operator == (emscripten::val const& x, U16Array const&);

using U16CArray = uint16_t[];
#define U16A(...) U16Array{make_array_view(U16CArray{__VA_ARGS__})}

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
static ut::assertion_result scancodes_EQ(emscripten::val const& v, U16Array ref)
{
    auto putseq = [&](std::ostream& out, array_view<uint16_t> av){
        out << "{";
        for (auto scancode : av) {
            ut::print_hex(out, scancode, 2);
            out << ", ";
        }
        out << "}";
    };

    if (v.isArray()) {
        const auto vec = ::emscripten::vecFromJSArray<uint16_t>(v);
        return ut::ops::compare_collection_EQ(vec, ref.a, [&](
            std::ostream& out, size_t /*pos*/, char const* op
        ){
            ut::put_data_with_diff(out, vec, op, ref.a, putseq);
        });
    }
    else {
        auto putval = [](std::ostream& out, emscripten::val const& v){
            out << (v.isUndefined() ? "undefined" : "not_array");
        };
        bool is_undefined = v.isUndefined() && ref.a.data() == nullptr;
        return ut::create_assertion_result(is_undefined, v, " != ", ref.a, overload{putseq, putval});
    }
}

RED_TEST_DISPATCH_COMPARISON_EQ((), (::emscripten::val), (::U16Array), ::scancodes_EQ)
#endif

RED_JS_AUTO_TEST_CASE(
    TestUnicodeToScancode,
    (emscripten::val reversedKeymap),
    (() => {
        const ReversedKeymap = require("src/application/scancodes").ReversedKeymap;
        const layouts = require("src/application/reversed_layouts").layouts;
        for (const layout of layouts) {
            if (layout.localeName === "fr-FR") {
                return new ReversedKeymap(layout);
            }
        }
    })()
) {
    auto toScancodes2 = [&](std::string_view key, std::string_view code, uint16_t state){
        return reversedKeymap.call<emscripten::val>("toScancodesAndFlags", std::string(key), std::string(code), state);
    };

    auto toScancodes = [&](std::string_view key, uint16_t state){
        return toScancodes2(key, key, state);
    };

    auto getVirtualModFlags = [&reversedKeymap]{
        return reversedKeymap.call<uint32_t>("getVirtualModFlags");
    };

    auto getModFlags = [&reversedKeymap]{
        return reversedKeymap.call<uint32_t>("getModFlags");
    };

    const unsigned keyAcquire = 0;
    const unsigned keyRelease = 0x8000;

    const uint32_t NoMod       = 0;
    const uint32_t ShiftMod    = 1 << 0;
    const uint32_t AltGrMod    = 1 << 1;
    const uint32_t CapsLockMod = 1 << 2;
    const uint32_t NumLockMod  = 1 << 3;
    const uint32_t CtrlMod     = 1 << 4;
    const uint32_t AltMod      = 1 << 5;
    // const uint32_t OEM8Mod     = 1 << 6;
    // const uint32_t KanaMod     = 1 << 7;
    // const uint32_t KanaLockMod = 1 << 8;
    const uint32_t RightShiftMod = 1 << 9;
    const uint32_t RightCtrlMod  = 1 << 10;

    auto const h32 = ut::hex_int::u32();

    #define TEST_HEX32(...) BOOST_TEST(__VA_ARGS__, h32)

    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);

    // unknown character
    //@{
    RED_CHECK(toScancodes("ß", keyAcquire) == U16Array());
    TEST_HEX32(getModFlags() == 0);
    TEST_HEX32(getVirtualModFlags() == 0);
    //@}

    // b
    //@{
    RED_CHECK(toScancodes("b", keyAcquire) == U16A(0x30));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);

    RED_CHECK(toScancodes("b", keyRelease) == U16A(0x8030));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);
    //@}

    // A
    //@{
    RED_CHECK(toScancodes2("Shift", "ShiftLeft", keyAcquire) == U16A(0x2A));
    TEST_HEX32(getModFlags() == ShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes("A", keyAcquire) == U16A(0x10));
    TEST_HEX32(getModFlags() == ShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes("A", keyRelease) == U16A(0x8010));
    TEST_HEX32(getModFlags() == ShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes2("Shift", "ShiftLeft", keyRelease) == U16A(0x802a));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);
    //@}

    // A'
    //@{
    RED_CHECK(toScancodes2("Shift", "ShiftLeft", keyAcquire) == U16A(0x2A));
    TEST_HEX32(getModFlags() == ShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes("A", keyAcquire) == U16A(0x10));
    TEST_HEX32(getModFlags() == ShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes2("Shift", "ShiftLeft", keyRelease) == U16A(0x802a));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);

    RED_CHECK(toScancodes("a", keyRelease) == U16A(0x8010));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);
    //@}

    // A shift right
    //@{
    RED_CHECK(toScancodes2("Shift", "ShiftRight", keyAcquire) == U16A(0x36));
    TEST_HEX32(getModFlags() == RightShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes("A", keyAcquire) == U16A(0x10));
    TEST_HEX32(getModFlags() == RightShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes("A", keyRelease) == U16A(0x8010));
    TEST_HEX32(getModFlags() == RightShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    // ShiftLeft is already released -> unchanged state
    RED_CHECK(toScancodes2("Shift", "ShiftLeft", keyRelease) == U16A(0x802a));
    TEST_HEX32(getModFlags() == RightShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes2("Shift", "ShiftRight", keyRelease) == U16A(0x8036));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);
    //@}

    // A shift right + left
    //@{
    RED_CHECK(toScancodes2("Shift", "ShiftRight", keyAcquire) == U16A(0x36));
    TEST_HEX32(getModFlags() == RightShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes2("Shift", "ShiftLeft", keyAcquire) == U16A(0x2A));
    TEST_HEX32(getModFlags() == (ShiftMod | RightShiftMod));
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes("A", keyAcquire) == U16A(0x10));
    TEST_HEX32(getModFlags() == (ShiftMod | RightShiftMod));
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes("A", keyRelease) == U16A(0x8010));
    TEST_HEX32(getModFlags() == (ShiftMod | RightShiftMod));
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes2("Shift", "ShiftRight", keyRelease) == U16A(0x8036));
    TEST_HEX32(getModFlags() == ShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes2("Shift", "ShiftLeft", keyRelease) == U16A(0x802a));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);
    //@}

    // a with Shift
    //@{
    RED_CHECK(toScancodes2("Shift", "ShiftLeft", keyAcquire) == U16A(0x2A));
    TEST_HEX32(getModFlags() == ShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes("a", keyAcquire) == U16A(0x802A, 0x10, 0x2A));
    TEST_HEX32(getModFlags() == ShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes("a", keyRelease) == U16A(0x8010));
    TEST_HEX32(getModFlags() == ShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes2("Shift", "ShiftLeft", keyRelease) == U16A(0x802a));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);
    //@}

    // a' with Shift
    //@{
    RED_CHECK(toScancodes2("Shift", "ShiftLeft", keyAcquire) == U16A(0x2A));
    TEST_HEX32(getModFlags() == ShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes("a", keyAcquire) == U16A(0x802A, 0x10, 0x2A));
    TEST_HEX32(getModFlags() == ShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes2("Shift", "ShiftLeft", keyRelease) == U16A(0x802a));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);

    RED_CHECK(toScancodes("a", keyRelease) == U16A(0x8010));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);
    //@}

    // õ
    //@{
    RED_CHECK(toScancodes("Dead", keyAcquire) == U16Array());
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);

    RED_CHECK(toScancodes("Dead", keyRelease) == U16Array());
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);

    RED_CHECK(toScancodes("õ", keyAcquire) == U16A(0x138, 0x03, 0x8138, 0x18));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);

    RED_CHECK(toScancodes("o", keyRelease) == U16A(0x8018));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);
    //@}

    // Ctrl, Alt, AltGr
    //@{
    RED_CHECK(toScancodes2("Control", "ControlLeft", keyAcquire) == U16A(0x1D));
    TEST_HEX32(getModFlags() == CtrlMod);
    TEST_HEX32(getVirtualModFlags() == CtrlMod);
    reversedKeymap.set("altGrIsCtrlAndAlt", false);
    TEST_HEX32(getModFlags() == CtrlMod);
    TEST_HEX32(getVirtualModFlags() == CtrlMod);
    reversedKeymap.set("altGrIsCtrlAndAlt", true);
    TEST_HEX32(getModFlags() == CtrlMod);
    TEST_HEX32(getVirtualModFlags() == CtrlMod);

    RED_CHECK(toScancodes2("Alt", "AltLeft", keyAcquire) == U16A(0x38));
    TEST_HEX32(getModFlags() == (CtrlMod | AltMod));
    TEST_HEX32(getVirtualModFlags() == AltGrMod);
    reversedKeymap.set("altGrIsCtrlAndAlt", false);
    TEST_HEX32(getModFlags() == (CtrlMod | AltMod));
    TEST_HEX32(getVirtualModFlags() == (CtrlMod | AltMod));
    reversedKeymap.set("altGrIsCtrlAndAlt", true);
    TEST_HEX32(getModFlags() == (CtrlMod | AltMod));
    TEST_HEX32(getVirtualModFlags() == AltGrMod);

    RED_CHECK(toScancodes2("Control", "ControlLeft", keyRelease) == U16A(0x801D));
    TEST_HEX32(getModFlags() == AltMod);
    TEST_HEX32(getVirtualModFlags() == AltMod);
    reversedKeymap.set("altGrIsCtrlAndAlt", false);
    TEST_HEX32(getModFlags() == AltMod);
    TEST_HEX32(getVirtualModFlags() == AltMod);
    reversedKeymap.set("altGrIsCtrlAndAlt", true);
    TEST_HEX32(getModFlags() == AltMod);
    TEST_HEX32(getVirtualModFlags() == AltMod);

    RED_CHECK(toScancodes2("AltGraph", "AltRight", keyAcquire) == U16A(0x138));
    TEST_HEX32(getModFlags() == (AltMod | AltGrMod));
    TEST_HEX32(getVirtualModFlags() == AltGrMod);
    reversedKeymap.set("altGrIsCtrlAndAlt", false);
    TEST_HEX32(getModFlags() == (AltMod | AltGrMod));
    TEST_HEX32(getVirtualModFlags() == (AltMod | AltGrMod));
    reversedKeymap.set("altGrIsCtrlAndAlt", true);
    TEST_HEX32(getModFlags() == (AltMod | AltGrMod));
    TEST_HEX32(getVirtualModFlags() == AltGrMod);

    RED_CHECK(toScancodes2("Alt", "AltLeft", keyRelease) == U16A(0x8038));
    TEST_HEX32(getModFlags() == AltGrMod);
    TEST_HEX32(getVirtualModFlags() == AltGrMod);
    reversedKeymap.set("altGrIsCtrlAndAlt", false);
    TEST_HEX32(getModFlags() == AltGrMod);
    TEST_HEX32(getVirtualModFlags() == AltGrMod);
    reversedKeymap.set("altGrIsCtrlAndAlt", true);
    TEST_HEX32(getModFlags() == AltGrMod);
    TEST_HEX32(getVirtualModFlags() == AltGrMod);

    RED_CHECK(toScancodes2("AltGraph", "AltRight", keyRelease) == U16A(0x8138));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);

    RED_CHECK(toScancodes2("Alt", "AltLeft", keyAcquire) == U16A(0x38));
    TEST_HEX32(getModFlags() == AltMod);
    TEST_HEX32(getVirtualModFlags() == AltMod);
    reversedKeymap.set("altGrIsCtrlAndAlt", false);
    TEST_HEX32(getModFlags() == AltMod);
    TEST_HEX32(getVirtualModFlags() == AltMod);
    reversedKeymap.set("altGrIsCtrlAndAlt", true);
    TEST_HEX32(getModFlags() == AltMod);
    TEST_HEX32(getVirtualModFlags() == AltMod);

    RED_CHECK(toScancodes2("Control", "ControlRight", keyAcquire) == U16A(0x11D));
    TEST_HEX32(getModFlags() == (RightCtrlMod | AltMod));
    TEST_HEX32(getVirtualModFlags() == AltGrMod);
    reversedKeymap.set("altGrIsCtrlAndAlt", false);
    TEST_HEX32(getModFlags() == (RightCtrlMod | AltMod));
    TEST_HEX32(getVirtualModFlags() == (CtrlMod | AltMod));
    reversedKeymap.set("altGrIsCtrlAndAlt", true);
    TEST_HEX32(getModFlags() == (RightCtrlMod | AltMod));
    TEST_HEX32(getVirtualModFlags() == AltGrMod);

    RED_CHECK(toScancodes2("Control", "ControlRight", keyRelease) == U16A(0x811D));
    TEST_HEX32(getModFlags() == AltMod);
    TEST_HEX32(getVirtualModFlags() == AltMod);
    reversedKeymap.set("altGrIsCtrlAndAlt", false);
    TEST_HEX32(getModFlags() == AltMod);
    TEST_HEX32(getVirtualModFlags() == AltMod);
    reversedKeymap.set("altGrIsCtrlAndAlt", true);
    TEST_HEX32(getModFlags() == AltMod);
    TEST_HEX32(getVirtualModFlags() == AltMod);

    RED_CHECK(toScancodes2("Alt", "AltLeft", keyRelease) == U16A(0x8038));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);
    //@}

    // Shift
    //@{
    RED_CHECK(toScancodes2("Shift", "ShiftLeft", keyAcquire) == U16A(0x2a));
    TEST_HEX32(getModFlags() == ShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes2("Shift", "ShiftRight", keyAcquire) == U16A(0x36));
    TEST_HEX32(getModFlags() == (ShiftMod | RightShiftMod));
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes2("Shift", "ShiftLeft", keyRelease) == U16A(0x802a));
    TEST_HEX32(getModFlags() == RightShiftMod);
    TEST_HEX32(getVirtualModFlags() == ShiftMod);

    RED_CHECK(toScancodes2("Shift", "ShiftRight", keyRelease) == U16A(0x8036));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);
    //@}

    // OS
    //@{
    RED_CHECK(toScancodes2("OS", "OSLeft", keyAcquire) == U16A(0x15b));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);

    RED_CHECK(toScancodes2("OS", "OSRight", keyAcquire) == U16A(0x15c));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);

    RED_CHECK(toScancodes2("OS", "OSLeft", keyRelease) == U16A(0x815b));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);

    RED_CHECK(toScancodes2("OS", "OSRight", keyRelease) == U16A(0x815c));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);
    //@}

    // sync
    //@{
    // const uint32_t ScrollLock   = 0x01;
    const uint32_t NumLock      = 0x02;
    // const uint32_t CapsLock     = 0x04;
    // const uint32_t KanaLock     = 0x08;
    // const uint32_t ControlLeft  = 0x0010;
    const uint32_t ControlRight = 0x0020;
    // const uint32_t ShiftLeft    = 0x0040;
    // const uint32_t ShiftRight   = 0x0080;
    const uint32_t AltLeft      = 0x0100;
    // const uint32_t AltRight     = 0x0200;
    // const uint32_t OSLeft       = 0x0400;
    // const uint32_t OSRight      = 0x0800;
    // const uint32_t Kana         = 0x1000;

    auto sync = [&reversedKeymap](uint32_t syncFlags){
        return reversedKeymap.call<emscripten::val>("sync", syncFlags);
    };

    //                              SL      SR     CL     CR      AL     AR      OSL     OSR
    RED_CHECK(sync(0xffff) == U16A(0x2A,   0x36,  0x1D,  0x11D,  0x38,  0x138,  0x15B,  0x15C));
    TEST_HEX32(getModFlags() == (ShiftMod | CtrlMod | AltMod | AltGrMod | CapsLockMod | NumLockMod | RightShiftMod | RightCtrlMod));
    TEST_HEX32(getVirtualModFlags() == (ShiftMod | AltGrMod | CapsLockMod | NumLockMod));

    RED_CHECK(sync(0)    == U16A(0x802A, 0x8036, 0x801D, 0x811D, 0x8038, 0x8138, 0x815B, 0x815C));
    TEST_HEX32(getModFlags() == NoMod);
    TEST_HEX32(getVirtualModFlags() == NoMod);

    RED_CHECK(sync(NumLock | AltLeft | ControlRight)
                         == U16A(0x802A, 0x8036, 0x801D,  0x11D, 0x38,   0x8138, 0x815B, 0x815C));
    TEST_HEX32(getModFlags() == (AltMod | NumLockMod | RightCtrlMod));
    TEST_HEX32(getVirtualModFlags() == (AltGrMod | NumLockMod));
    //@}
}
