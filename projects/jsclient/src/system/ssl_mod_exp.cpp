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

#include "system/ssl_mod_exp.hpp"
#include "red_emscripten/em_js.hpp"

#include <memory>
#include <string>

namespace
{
    RED_EM_JS(unsigned, mod_exp_js, (
        char const* base_str, char const* modulus_str, char const* exponent_str,
        char* out, unsigned out_len
    ), {
        let base = BigInt(UTF8ToString(base_str));
        const modulus = BigInt(UTF8ToString(modulus_str));
        let exponent = BigInt(UTF8ToString(exponent_str));

        const int2 = 2n;
        const int1 = 1n;

        let result = 1n;
        while (exponent) {
            if ((exponent % int2) == int1) {
                result = (result * base) % modulus;
            }
            base = (base * base) % modulus;
            exponent = exponent / int2;
        }

        if (!result) {
            return 0;
        }

        const hex = result.toString(16);

        if (hex.length > out_len * 2) {
            return 0;
        }

        out = HEAPU8.subarray(out, out + out_len);
        let ihex = 0;
        let iout = 0;

        const htoi = (i) => i <= 57 ? i - 48 : i - 97 + 10;

        if (hex.length & 1) {
            out[iout++] = htoi(hex.charCodeAt(ihex++));
        }

        for (const endhex = hex.length; ihex < endhex; ihex += 2) {
            out[iout++] = (htoi(hex.charCodeAt(ihex)) << 4) | htoi(hex.charCodeAt(ihex+1));
        }

        return iout;
    })
}

writable_bytes_view mod_exp(
    writable_bytes_view out,
    bytes_view inr,
    bytes_view modulus,
    bytes_view exponent
)
{
    if (inr.empty() || modulus.empty() || exponent.empty()) {
        return {};
    }

    auto hexbuffer = std::make_unique<uint8_t[]>(
        (inr.size() + modulus.size() + exponent.size()) * 2
        + 9 /* ("0x" + "\0") * 3 */
    );
    auto* hexstr_p = hexbuffer.get();

    auto to_hexa_string = [&](bytes_view av){
        char const* hextable = "0123456789ABCDEF";
        auto p = hexstr_p;
        *hexstr_p++ = '0';
        *hexstr_p++ = 'x';
        for (uint8_t c : av) {
            *hexstr_p++ = hextable[c >> 4];
            *hexstr_p++ = hextable[c & 0xFu];
        }
        *hexstr_p++ = '\0';
        return byte_ptr(p).as_charp();
    };

    unsigned n = mod_exp_js(
        to_hexa_string(inr),
        to_hexa_string(modulus),
        to_hexa_string(exponent),
        out.as_charp(), out.size());
    return out.first(n);
}
