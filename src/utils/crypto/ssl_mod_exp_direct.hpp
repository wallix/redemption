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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

*/

#pragma once

#include <algorithm>
#include <memory>
#include <cassert>
#include <cstddef>


/**
 * \pre  \a out_len >= \a modulus_size
 * \return  the length of the big-endian number placed at out. ~size_t{} if error
 */
static inline size_t mod_exp_direct(
    uint8_t * out, size_t out_len,
    const uint8_t * inr, size_t in_len,
    const uint8_t * modulus, size_t modulus_size,
    const uint8_t * exponent, size_t /*TODO exponent_size*/
) {
    assert(out_len >= modulus_size);

    using elem_t = uint32_t;
    using product_t = uint64_t;
    static constexpr elem_t base = ~elem_t{};

    class Bignum_one {};
    struct Bignum
    {
        // [little -------------------- big]
        //  u8 [big - little][big - little]
        size_t n;
        size_t n_max;
        std::unique_ptr<uint8_t[]> a;

        Bignum(Bignum &&) = default;
        Bignum & operator = (Bignum &&) = default;

        Bignum(Bignum_one /*unused*/, size_t max_size)
        : n{1}
        , n_max{max_size}
        , a{new uint8_t[n_max]{0}}
        {
            a[0] = 1;
        }

        Bignum(uint8_t const * s, size_t n, size_t max_size)
        : n{n}
        , n_max{max_size}
        , a{new uint8_t[n_max]}
        {
            assert(n <= max_size);
            std::reverse_copy(s, s + n, a.get());
        }

        void multiply(Bignum const & rhs, Bignum & result) const
        {
            product_t carry = 0;
            size_t digit = 0;
            for (;; ++digit) {
                //result.a[digit] = elem_t(carry % base);
                //carry /= base;

                product_t oldcarry = carry;
                carry /= base;
                result.a[digit] = elem_t(oldcarry - carry * base);

                bool found = false;

                for (size_t i = std::max<size_t>(digit < rhs.n ? 0 : digit - rhs.n + 1, this->n_max);
                    i < n && i <= digit; ++i
                ) {
                    product_t pval = result.a[digit] + a[i] * product_t(rhs.a[digit - i]);

                    //carry += pval / base;
                    //pval %= base;
                    product_t quot = pval / base;
                    carry += quot;
                    pval -= quot * base;

                    result.a[digit] = elem_t(pval);
                    found = true;
                }

                if (!found) {
                    break;
                }
            }

            for (; carry > 0; ++digit) {
                result.a[digit] = elem_t(carry % base);
                carry /= base;
            }

            //result.remove_leading_zeros();
        }

        void modulus(Bignum const & rhs, Bignum & D) const
        {
            D = Bignum(rhs.a.get(), rhs.n, rhs.n_max);
            Bignum N(a.get(), n, n_max);
            for (int i = int(N.n) - 1; i >= 0; --i)
            {
                a[n-1] = N.a[i];
                std::rotate(a.get(), a.get() + n - 1, a.get() + n);
                //this->remove_leading_zeros();
                //*this -= D * d_in_r(*this, D);
            }
            //this->remove_leading_zeros();
        }
    };

    size_t max_size = std::max(in_len, modulus_size);
    Bignum b{inr, in_len, max_size};
    Bignum const m{modulus, modulus_size, max_size};
    //Bignum const exp{exponent, exponent_size, max_size};
    unsigned exp = exponent[0];

    Bignum result{Bignum_one{}, max_size};
    Bignum ctx{Bignum_one{}, max_size};

    while (exp > 0) {
        if (exp & 1) {
            // result = (result * b) % m;
            result.multiply(b, ctx);
            std::swap(result, ctx);
            result.modulus(m, ctx);
            std::swap(result, ctx);
        }
        exp >>= 1;
        // b = (b * b) % m;
        b.multiply(b, ctx);
        std::swap(b, ctx);
        b.modulus(m, ctx);
        std::swap(b, ctx);
    }

    size_t const count_bytes = std::max(out_len, result.n);
    std::reverse_copy(result.a.get(), result.a.get() + count_bytes, out);

    return count_bytes; //result.copy_unsafe(out/*, out_len*/);

//
//     auto b256_to_bigint = [](uint8_t const * s, size_t n, char * ctx) {
//         static constexpr char t[]{
//             '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
//             'A', 'B', 'C', 'D', 'E', 'F',
//         };
//         for (auto e = s+n; s != e; ++s) {
//             *ctx++ = t[(*s & 0xf0) >> 4];
//             *ctx++ = t[(*s & 0x0f)];
//         }
//         *ctx = 0;
//     };
//
//     auto u = std::make_unique<char[]>(std::max({in_len, modulus_size, exponent_size})*2 + 2 + 1);
//     auto p = u.get();
//     p[0] = '0';
//     p[1] = 'x';
//
//     using int_type = boost::multiprecision::cpp_int;
//
//     b256_to_bigint(inr, in_len, p+2);
//     int_type base(p);
//     b256_to_bigint(exponent, exponent_size, p+2);
//     int_type exp(p);
//     b256_to_bigint(modulus, modulus_size, p+2);
//     int_type m(p);
//
//     int_type r = boost::multiprecision::powm(base, exp, m);
//
//     auto bigint_to_b256 = [](char const * s, size_t n, uint8_t * out) {
//         auto hex_to_byte = [](char c) -> unsigned {
//             return
//                 ('0' <= c && c <= '9') ? c - '0'
//               : ('A' <= c && c <= 'F') ? c - 'A' + 10
//               : c - 'a' + 10;
//         };
//         for (auto e = s + (n - (n&1)); s != e; s += 2) {
//             *out++ = static_cast<uint8_t>(hex_to_byte(*s) << 4 | hex_to_byte(*(s+1)));
//         }
//         if (n&1) {
//             *out++ = static_cast<uint8_t>(hex_to_byte(*s));
//         }
//         return n/2 + (n&1);
//     };
//
//     std::ostringstream oss;
//     oss << std::hex << r;
//     auto const str_hex = oss.str();
//     return str_hex[0] == '0' ? 0 : bigint_to_b256(str_hex.c_str(), str_hex.size(), out);
}
