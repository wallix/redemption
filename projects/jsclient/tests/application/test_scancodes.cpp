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
#include "test_only/test_framework/hex.hpp"
#include "test_only/js_auto_test_case.hpp"

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
    const bool is_array = v.isArray();
    const auto vec = is_array ? ::emscripten::vecFromJSArray<uint16_t>(v) : std::vector<uint16_t>{};

    return ut::ops::compare_collection_EQ(vec, ref.a, [&](
        std::ostream& out, size_t /*pos*/, char const* op
    ){
        std::stringstream oss;
        auto putseq = [&](array_view<uint16_t> av){
            oss << "{";
            for (auto scancode : av) {
                ut::boost_test_print_type(oss, ut::minimal_hex{scancode});
                oss << ", ";
            }
            oss << "}";
        };

        if (is_array) {
            putseq(vec);
        }
        else {
            oss << (v.isUndefined() ? "undefined" : "not_array");
        }

        auto s1 = oss.str();
        oss.str({});

        putseq(ref.a);

        ut::put_message_with_diff(out, s1, op, oss.str());
    });
}

RED_TEST_DISPATCH_COMPARISON_EQ((), (::emscripten::val), (::U16Array), ::scancodes_EQ)
#endif

RED_JS_AUTO_TEST_CASE(
    TestUnicodeToScancode,
    (emscripten::val unicodeToScancode),
    (() => {
        const converter = require("src/application/scancodes").createUnicodeToScancodeConverter;
        const layout = {
            name: "fr-FR", lcid: 0x0000040c, keymap: {
                /*  A */ "\u0041": [
                    0x00120010 /*   | S |   |     | L |     |     |       |   */,
                ],
                /*  a */ "\u0061": [
                    0x00110010 /* N |   |   |     |   | L+S |     |       |   */,
                ],
                /*  b */ "\u0062": [
                    0x00210030 /* N |   |   |     |   | L+S |     |       |   */,
                ],
                /*  o */ "\u006f": [
                    0x00210018 /* N |   |   |     |   | L+S |     |       |   */,
                ],
            }, deadmap: {
                //                    ~
                /*  õ */ "\u00f5": [0x03, [
                    //  o
                    0x00210018 /* N |   |   |     |   | L+S |     |       |   */,
                ]],
            }
        };

        return converter(layout);
    })()
) {
    auto convert = [&unicodeToScancode](char const* key, unsigned state, bool is_right = false){
        return unicodeToScancode(std::string(key), state, is_right);
    };

    auto getControlMask = [&unicodeToScancode]{
        return ut::hex32{unicodeToScancode.call<uint32_t>("getControlMask")};
    };

    auto getModMask = [&unicodeToScancode]{
        return ut::hex32{unicodeToScancode.call<uint32_t>("getModMask")};
    };

    auto isDeadKey = [&unicodeToScancode]{
        return unicodeToScancode.call<bool>("isDeadKey");
    };

    const unsigned keyAcquire = 0;
    const unsigned keyRelease = 0x8000;

    const ut::hex32 Zero {0};
    const ut::hex32 NoMod   {  0x1'0000};
    const ut::hex32 ShiftMod{  0x2'0000};
    const ut::hex32 AltGrMod{  0x4'0000};
    const ut::hex32 CtrlMod {0x100'0000};
    const ut::hex32 AltMod  {0x200'0000};
    const ut::hex32 ShiftRightMod {0x2};

    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);

    RED_CHECK(convert("y", keyAcquire).isUndefined());
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);

    // b
    //@{
    RED_CHECK(convert("b", keyAcquire) == U16A(0x30));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);

    RED_CHECK(convert("b", keyRelease) == U16A(0x8030));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);
    //@}

    // A
    //@{
    RED_CHECK(convert("Shift", keyAcquire) == U16A(0x2A));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("A", keyAcquire) == U16A(0x10));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("A", keyRelease) == U16A(0x8010));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("Shift", keyRelease) == U16A(0x802a));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);
    //@}

    // A'
    //@{
    RED_CHECK(convert("Shift", keyAcquire) == U16A(0x2A));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("A", keyAcquire) == U16A(0x10));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("Shift", keyRelease) == U16A(0x802a));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);

    RED_CHECK(convert("a", keyRelease) == U16A(0x8010));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);
    //@}

    // A shift right
    //@{
    RED_CHECK(convert("Shift", keyAcquire, true) == U16A(0x36));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftRightMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("A", keyAcquire) == U16A(0x10));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftRightMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("A", keyRelease) == U16A(0x8010));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftRightMod);
    RED_CHECK(getControlMask() == ShiftMod);

    // right = false -> unchanged state
    RED_CHECK(convert("Shift", keyRelease) == U16A(0x802a));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftRightMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("Shift", keyRelease, true) == U16A(0x8036));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);
    //@}

    // A shift right + left
    //@{
    RED_CHECK(convert("Shift", keyAcquire, true) == U16A(0x36));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftRightMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("Shift", keyAcquire) == U16A(0x2A));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == (ShiftMod | ShiftRightMod));
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("A", keyAcquire) == U16A(0x10));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == (ShiftMod | ShiftRightMod));
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("A", keyRelease) == U16A(0x8010));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == (ShiftMod | ShiftRightMod));
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("Shift", keyRelease, true) == U16A(0x8036));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("Shift", keyRelease) == U16A(0x802a));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);
    //@}

    // a with Shift
    //@{
    RED_CHECK(convert("Shift", keyAcquire) == U16A(0x2A));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("a", keyAcquire) == U16A(0x802A, 0x10, 0x2A));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("a", keyRelease) == U16A(0x802A, 0x8010, 0x2A));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("Shift", keyRelease) == U16A(0x802a));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);
    //@}

    // a' with Shift
    //@{
    RED_CHECK(convert("Shift", keyAcquire) == U16A(0x2A));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("a", keyAcquire) == U16A(0x802A, 0x10, 0x2A));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == ShiftMod);
    RED_CHECK(getControlMask() == ShiftMod);

    RED_CHECK(convert("Shift", keyRelease) == U16A(0x802a));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);

    RED_CHECK(convert("a", keyRelease) == U16A(0x8010));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);
    //@}

    // õ
    //@{
    RED_CHECK(convert("Dead", keyAcquire).isUndefined());
    RED_CHECK(isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);

    RED_CHECK(convert("Dead", keyRelease).isUndefined());
    RED_CHECK(isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);

    RED_CHECK(convert("õ", keyAcquire) == U16A(0x03, 0x18));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);

    RED_CHECK(convert("o", keyRelease) == U16A(0x8018));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);
    //@}

    // Ctrl, Alt, AltGr
    //@{
    RED_CHECK(convert("Control", keyAcquire) == U16A(0x1D));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == CtrlMod);
    RED_CHECK(getControlMask() == CtrlMod);

    RED_CHECK(convert("Alt", keyAcquire) == U16A(0x38));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == (CtrlMod | AltMod));
    RED_CHECK(getControlMask() == AltGrMod);

    RED_CHECK(convert("Control", keyRelease) == U16A(0x801D));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == AltMod);
    RED_CHECK(getControlMask() == AltMod);

    RED_CHECK(convert("AltGraph", keyAcquire) == U16A(0x138));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == (AltMod | AltGrMod));
    RED_CHECK(getControlMask() == AltGrMod);

    RED_CHECK(convert("Alt", keyRelease) == U16A(0x8038));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == AltGrMod);
    RED_CHECK(getControlMask() == AltGrMod);

    RED_CHECK(convert("AltGraph", keyRelease) == U16A(0x8138));
    RED_CHECK(!isDeadKey());
    RED_CHECK(getModMask() == Zero);
    RED_CHECK(getControlMask() == NoMod);
    //@}
}
