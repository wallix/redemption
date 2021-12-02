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

    U16Array() = default;

    template<std::size_t N>
    U16Array(uint16_t const (&a)[N]) noexcept
    : a(a)
    {}
};
// for IDE coloration only
bool operator == (emscripten::val const& x, U16Array const&);

using U16CArray = uint16_t[];
#define U16A(...) U16Array(U16CArray{__VA_ARGS__})

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
    (emscripten::val keyboard),
    (() => {
        const mod = require("src/application/scancodes");
        const layouts = require("src/application/reversed_layouts").layouts;
        for (const layout of layouts) {
            if (layout.localeName === "fr-FR") {
                const rkeymap = new mod.ReversedKeymap(layout);
                const driver = ({
                    events: [],
                    sendUnicode: (text, flag) => {
                        this.events.push({type: "unicode", text: text, flag: flag});
                        return true;
                    },
                    sendScancodes(scancodes) => {
                        this.events.push({type: "unicode", scancodes: scancodes});
                        return true;
                    },
                    syncKbdLocks: function(syncFlags) {
                        this.events.push({type: "sync", flags: syncFlags});
                    },
                    compositeLater: function() {
                        this.events.push({type: "composition"});
                    },
                });
                const keyboard = new Keyboard(rkeymap, driver);
                keyboard.events = driver.events;
                return keyboard;
            }
        }
    })()
) {
    auto kevent = [&](
        char const* funcname,
        std::string_view key, std::string_view code,
        std::string_view modifier
    ) {
        auto event = emscripten::val::object();
        event.set("key", std::string(key));
        event.set("code", std::string(code));
        return keyboard.call<bool>(funcname, event);
    };

    auto up = [&](std::string_view key, std::string_view code, std::string_view modifier = {}) {
        kevent("keyup", key, code, modifier);
    };

    auto down = [&](std::string_view key, std::string_view code, std::string_view modifier = {}) {
        kevent("keydown", key, code, modifier);
    };
}
