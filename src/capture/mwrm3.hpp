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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/sugar/bytes_view.hpp"
#include "utils/stream.hpp"
#include "cxx/cxx.hpp"

#include <utility>
#include <chrono>

#include <cstdint>


namespace Mwrm3
{
    enum class Type : uint16_t
    {
        None,
        WrmNew,
        WrmState,
        FdxNew,
        TflNew,

        MwrmHeaderCompatibility = 'v' | '3' << 8,
    };

    template<Type type>
    using integral_type = std::integral_constant<Type, type>;

    inline constexpr auto header_compatibility_packet = "v3\n"_av;

    enum class FileSize : uint64_t;
    enum class FileId : uint64_t;

    enum class Direction : uint8_t
    {
        Unknown,
        ClientToServer,
        ServerToClient,
    };

    enum class TransferedStatus : uint8_t
    {
        Unknown,
        Completed,
        Broken,
    };

    template<class F, class FError>
    auto unserialize_type(bytes_view av, F&& f, FError&& ferror)
    {
        if (av.size() >= 2)
        {
            return f(av.drop_front(2));
        }
        else
        {
            return ferror();
        }
    }

    // TODO 32 -> constante

    struct QuickHash
    {
        explicit QuickHash(bytes_view hash) noexcept
        : hash(hash)
        {
            assert(hash.empty() || hash.size() == 32);
        }

        bytes_view hash;
    };

    struct FullHash
    {
        explicit FullHash(bytes_view hash) noexcept
        : hash(hash)
        {
            assert(hash.empty() || hash.size() == 32);
        }

        bytes_view hash;
    };

    struct Sha256Signature
    {
        explicit Sha256Signature(bytes_view sig) noexcept
        : sig(sig)
        {
            assert(sig.empty() || sig.size() == 32);
        }

        bytes_view sig;
    };

    namespace detail
    {
        namespace types
        {
            struct u8
            {
                static constexpr unsigned size = 1;

                safe_int<uint8_t> x;

                void write(OutStream& stream) noexcept
                {
                    stream.out_uint8(x);
                }
            };

            struct u16
            {
                static constexpr unsigned size = 2;

                safe_int<uint16_t> x;

                void write(OutStream& stream) noexcept
                {
                    stream.out_uint16_le(x);
                }
            };

            struct u8_unsafe
            {
                static constexpr unsigned size = 1;

                checked_int<uint8_t> x;

                void write(OutStream& stream) noexcept
                {
                    stream.out_uint8(x);
                }
            };

            struct u16_unsafe
            {
                static constexpr unsigned size = 2;

                checked_int<uint16_t> x;

                void write(OutStream& stream) noexcept
                {
                    stream.out_uint16_le(x);
                }
            };

            struct u64
            {
                static constexpr unsigned size = 8;

                safe_int<uint64_t> x;

                void write(OutStream& stream) noexcept
                {
                    stream.out_uint64_le(x);
                }
            };

            struct seconds
            {
                static constexpr unsigned size = 8;

                std::chrono::seconds x;

                void write(OutStream& stream) noexcept
                {
                    stream.out_uint64_le(checked_int{x.count()});
                }
            };

            struct u8_bytes_size : u8_unsafe
            {
                u8_bytes_size(bytes_view bytes) noexcept
                : u8_unsafe{bytes.size()}
                {}
            };

            struct u16_bytes_size : u16_unsafe
            {
                u16_bytes_size(bytes_view bytes) noexcept
                : u16_unsafe{bytes.size()}
                {}
            };
        }

        namespace readers
        {
            struct u8
            {
                static constexpr unsigned size = 1;

                uint8_t x;

                bool read(InStream& stream) noexcept
                {
                    x = stream.in_uint8();
                    return true;
                }

                uint8_t value() const noexcept
                {
                    return x;
                }
            };

            struct u16
            {
                static constexpr unsigned size = 2;

                uint16_t x;

                bool read(InStream& stream) noexcept
                {
                    x = stream.in_uint16_le();
                    return true;
                }

                uint16_t value() const noexcept
                {
                    return x;
                }
            };

            struct u64
            {
                static constexpr unsigned size = 8;

                uint64_t x;

                bool read(InStream& stream) noexcept
                {
                    x = stream.in_uint64_le();
                    return true;
                }

                uint64_t value() const noexcept
                {
                    return x;
                }
            };

            template<class Reader, class T>
            struct to : Reader
            {
                T value() const noexcept
                {
                    return T(Reader::value());
                }
            };

            using seconds = to<u64, std::chrono::seconds>;
            using file_size = to<u64, FileSize>;
            using file_id = to<u64, FileId>;
            using direction = to<u8, Direction>;

            template<std::size_t n>
            struct static_bytes
            {
                static constexpr unsigned size = n;

                bytes_view x;

                bool read(InStream& stream) noexcept
                {
                    if (stream.in_check_rem(n))
                    {
                        x = stream.in_skip_bytes(n);
                        return true;
                    }
                    return false;
                }

                bytes_view value() const noexcept
                {
                    return x;
                }
            };

            // TODO 32 -> constant
            using quick_hash = to<static_bytes<32>, QuickHash>;
            using full_hash = to<static_bytes<32>, FullHash>;
            using sha256_signature = to<static_bytes<32>, Sha256Signature>;

            template<class Bool, class T>
            struct optional_value
            {
                static constexpr unsigned size = 0;

                Bool& b;
                T x {};

                bool read(InStream& stream) noexcept
                {
                    if (b.value())
                    {
                        if constexpr (T::size == 0)
                        {
                            return x.read(stream);
                        }
                        else
                        {
                            if (stream.in_check_rem(T::size))
                            {
                                return x.read(stream);
                            }
                            return false;
                        }
                    }
                    else
                    {
                        return true;
                    }
                }

                auto value() const noexcept
                {
                    return x.value();
                }
            };

            template<class TInt>
            struct mask : TInt
            {
                template<int left_shift>
                struct bit : TInt
                {
                    auto value() const noexcept
                    {
                        return bool(TInt::value() & (1 << left_shift));
                    }
                };

                template<class T, int left_shift>
                auto optional_for() const noexcept
                {
                    return optional_value<bit<left_shift> const, T>{
                        static_cast<bit<left_shift> const&>(static_cast<TInt const&>(*this))
                    };
                }
            };

            using u8mask = mask<u8>;

            template<int m, int left_shift, class T, class ValueType>
            struct bits_from
            {
                static constexpr unsigned size = 0;

                T& x;

                constexpr bool read(InStream& /*in*/)
                {
                    return true;
                }

                auto value() const noexcept
                {
                    if constexpr (left_shift > 0) {
                        return ValueType((x.value() & m) >> left_shift);
                    }
                    else {
                        return ValueType((x.value() & m) << left_shift);
                    }
                }
            };

            struct u8bytes
            {
                static constexpr unsigned size = 1;

                bytes_view x;

                bool read(InStream& stream) noexcept
                {
                    auto n = stream.in_uint8();
                    if (stream.in_check_rem(n))
                    {
                        x = stream.in_skip_bytes(n);
                        return true;
                    }

                    return false;
                }

                bytes_view value() const noexcept
                {
                    return x;
                }
            };

            struct u16bytes_data
            {
                static constexpr unsigned size = 2;

                uint16_t& len;
                bytes_view x;

                bool read(InStream& stream) noexcept
                {
                    if (stream.in_check_rem(len))
                    {
                        x = stream.in_skip_bytes(len);
                        return true;
                    }

                    return false;
                }

                bytes_view value() const noexcept
                {
                    return x;
                }
            };

            struct u16str_data : u16bytes_data
            {
                array_view_const_char value() const noexcept
                {
                    return u16bytes_data::value().as_chars();
                }
            };

            class no_value {};

            template<class T>
            struct shadow_ref
            {
                static constexpr unsigned size = T::size;

                T& x;

                bool read(InStream& stream) noexcept
                {
                    return x.read(stream);
                }

                auto value() const noexcept
                {
                    return no_value();
                }
            };

            template<class T>
            shadow_ref(T&) -> shadow_ref<T>;

            struct u16bytes_ref
            {
                u16bytes_ref() = default;
                u16bytes_ref(u16bytes_ref&&) = delete;
                u16bytes_ref(u16bytes_ref const&) = delete;

                shadow_ref<u16> size()
                {
                    return {len};
                }

                u16bytes_data data()
                {
                    return {len.x, {}};
                }

            private:
                u16 len;
            };

            struct u16str_ref
            {
                u16str_ref() = default;
                u16str_ref(u16str_ref&&) = delete;
                u16str_ref(u16str_ref const&) = delete;

                shadow_ref<u16> size()
                {
                    return {len};
                }

                u16str_data data()
                {
                    return {{len.x, {}}};
                }

            private:
                u16 len;
            };

            struct u16bytes
            {
                static constexpr unsigned size = 2;

                bytes_view x;

                bool read(InStream& stream) noexcept
                {
                    auto n = stream.in_uint16_le();
                    if (stream.in_check_rem(n))
                    {
                        x = stream.in_skip_bytes(n);
                        return true;
                    }

                    return false;
                }

                bytes_view value() const noexcept
                {
                    return x;
                }
            };

            struct u16str : u16bytes
            {
                array_view_const_char value() const noexcept
                {
                    return u16bytes::value().as_chars();
                }
            };

            template<unsigned i, class Reader_>
            struct Value
            {
                using Reader = Reader_;
                static const unsigned index = i;
                Reader reader;
            };

            // tuple like
            template<class... Values>
            struct Group : Values...
            {
                static constexpr unsigned size = (... + Values::Reader::size);

                bool read(InStream& stream) noexcept
                {
                    if constexpr (size > 0)
                    {
                        if (!stream.in_check_rem(size))
                        {
                            return false;
                        }
                    }

                    return (... && static_cast<Values&>(*this).reader.read(stream));
                }
            };

            template<class Ints, class... xs>
            struct to_group;

            template<std::size_t... ints, class... xs>
            struct to_group<std::integer_sequence<std::size_t, ints...>, xs...>
            {
                using type = Group<Value<ints, xs>...>;
            };

            template<class... Reader>
            auto group(Reader... reader)
            {
                using g = typename to_group<
                    std::index_sequence_for<Reader...>, Reader...>::type;
                return g{{{reader}}...};
            }
        }

        template<class T> struct wrap_type { using type = T; };
        template<> struct wrap_type<Type> { using type = types::u16; };
        template<> struct wrap_type<FileSize> { using type = types::u64; };
        template<> struct wrap_type<std::chrono::seconds> { using type = types::seconds; };

        template<unsigned n>
        struct Buffer
        {
            uint8_t buf[n];

            template<class... Data>
            Buffer(Data... data) noexcept
            {
                OutStream out(buf);
                (typename wrap_type<Data>::type{data}.write(out), ...);
            }

            bytes_view bytes() const noexcept
            {
                return bytes_view(buf, n);
            }
        };

        template<class... Data>
        Buffer(Data...) -> Buffer<(... + wrap_type<Data>::type::size)>;

        template<int i, Type type, class F>
        auto ignore_no_value_impl(F& f, bytes_view remaining)
        {
            static_assert(i == 0);
            return f(std::integral_constant<Type, type>(), remaining);
        }

        template<int i, Type type, class F, class T, class... Ts>
        auto ignore_no_value_impl(F& f, bytes_view remaining, T x, Ts... xs)
        {
            if constexpr (i == 0)
            {
                return f(std::integral_constant<Type, type>(), remaining, x, xs...);
            }
            else if constexpr (std::is_same_v<T, readers::no_value>)
            {
                (void)x;
                return ignore_no_value_impl<i, type>(f, remaining, xs...);
            }
            else
            {
                return ignore_no_value_impl<i-1, type>(f, remaining, xs..., x);
            }
        }

        template<Type type, class F, class... Ts>
        auto ignore_no_value(F& f, bytes_view remaining, Ts... xs)
        {
            constexpr int count_no_value = (0 + ... + std::is_same_v<Ts, readers::no_value>);
            if constexpr (count_no_value == 0)
            {
                return f(std::integral_constant<Type, type>(), remaining, xs...);
            }
            else
            {
                return ignore_no_value_impl<sizeof...(xs) - count_no_value, type>(f, remaining, xs...);
            }
        }

        template<Type type, class F, class... x0>
        auto unwrapper_group(F& f, bytes_view remaining, readers::Group<x0...>& g0)
        {
            return ignore_no_value<type>(f, remaining,
                static_cast<x0&>(g0).reader.value()...);
        }

        template<Type type, class F, class... x0, class... x1>
        auto unwrapper_group(F& f, bytes_view remaining,
            readers::Group<x0...>& g0,
            readers::Group<x1...>& g1)
        {
            return ignore_no_value<type>(f, remaining,
                static_cast<x0&>(g0).reader.value()...,
                static_cast<x1&>(g1).reader.value()...);
        }

        template<Type type, class F, class... x0, class... x1, class... x2>
        auto unwrapper_group(F& f, bytes_view remaining,
            readers::Group<x0...>& g0,
            readers::Group<x1...>& g1,
            readers::Group<x2...>& g2)
        {
            return ignore_no_value<type>(f, remaining,
                static_cast<x0&>(g0).reader.value()...,
                static_cast<x1&>(g1).reader.value()...,
                static_cast<x2&>(g2).reader.value()...);
        }

        template<Type type, class F, class... x0, class... x1, class... x2, class... x3>
        auto unwrapper_group(F& f, bytes_view remaining,
            readers::Group<x0...>& g0,
            readers::Group<x1...>& g1,
            readers::Group<x2...>& g2,
            readers::Group<x3...>& g3)
        {
            return ignore_no_value<type>(f, remaining,
                static_cast<x0&>(g0).reader.value()...,
                static_cast<x1&>(g1).reader.value()...,
                static_cast<x2&>(g2).reader.value()...,
                static_cast<x3&>(g3).reader.value()...);
        }

        template<Type type, class F, class... x0, class... x1, class... x2, class... x3, class... x4>
        auto unwrapper_group(F& f, bytes_view remaining,
            readers::Group<x0...>& g0,
            readers::Group<x1...>& g1,
            readers::Group<x2...>& g2,
            readers::Group<x3...>& g3,
            readers::Group<x4...>& g4)
        {
            return ignore_no_value<type>(f, remaining,
                static_cast<x0&>(g0).reader.value()...,
                static_cast<x1&>(g1).reader.value()...,
                static_cast<x2&>(g2).reader.value()...,
                static_cast<x3&>(g3).reader.value()...,
                static_cast<x4&>(g4).reader.value()...);
        }

        template<Type type, class F, class FError, class... ReaderGroup>
        auto unserialize(bytes_view buf, F&& f, FError&& ferror, ReaderGroup... reader)
        {
            InStream in{buf};

            return ((... && reader.read(in)))
                // TODO should be static_cast<Value&>(static_cast<Group&>(flat_tuple)...)...
                ? unwrapper_group<type>(f, in.remaining_bytes(), reader...)
                : ferror();
        }
    }

    // for serialize_*
    // fn_ok(integral_type<Type>, serialized_bytes_view...)

    // for unserialize_*
    // fn_ok(integral_type<Type>, remaining_bytes, values...)
    // fn_error()

    namespace Mwrm3Serial
    {
        struct MwrmHeaderCompatibility
        {
            static constexpr Type type = Type::MwrmHeaderCompatibility;

            template<class F>
            static auto serialize(F&& f)
            {
                return f(integral_type<type>(), bytes_view(header_compatibility_packet));
            }

            template<class F, class FError>
            static decltype(auto) unserialize(bytes_view buf, F&& f, FError&& ferror)
            {
                using namespace detail::readers;
                /* ignore newline compatibility character */
                return (buf.size() > 0) ? f(integral_type<type>(), buf.drop_front(1)) : ferror();
            }
        };
    }

    inline constexpr Mwrm3Serial::MwrmHeaderCompatibility mwrm_header_compatibility {};

    namespace Mwrm3Serial
    {
        struct WrmNew
        {
            static constexpr Type type = Type::WrmNew;

            template<class F>
            static auto serialize(array_view_const_char filename, F&& f)
            {
                return f(integral_type<type>(), detail::Buffer{
                    type,
                    detail::types::u16_bytes_size{filename}
                }.bytes(), bytes_view(filename));
            }

            template<class F, class FError>
            static decltype(auto) unserialize(bytes_view buf, F&& f, FError&& ferror)
            {
                using namespace detail::readers;
                return detail::unserialize<type>(buf, f, ferror, group(u16str()));
            }
        };
    }

    inline constexpr Mwrm3Serial::WrmNew wrm_new {};

    namespace Mwrm3Serial
    {
        struct WrmState
        {
            static constexpr Type type = Type::WrmState;

            template<class F>
            static auto serialize(
                FileSize file_size, std::chrono::seconds duration,
                QuickHash quick_hash, FullHash full_hash,
                F&& f)
            {
                assert(quick_hash.hash.size() == full_hash.hash.size());
                return f(integral_type<type>(),
                    detail::Buffer{
                        type,
                        file_size,
                        duration,
                        detail::types::u8_unsafe{
                            ((quick_hash.hash.empty() ? 0 : 1) << 0)
                          | ((full_hash.hash.empty() ? 0 : 1) << 1)
                        },
                    }.bytes(),
                    quick_hash.hash,
                    full_hash.hash);
            }

            template<class F, class FError>
            static decltype(auto) unserialize(bytes_view buf, F&& f, FError&& ferror)
            {
                using namespace detail::readers;
                u8mask m;
                return detail::unserialize<Type::WrmState>(buf, f, ferror,
                    group(file_size(), seconds(), shadow_ref{m}),
                    group(m.optional_for<quick_hash, 0>()),
                    group(m.optional_for<full_hash, 1>())
                );
            }
        };
    }

    inline constexpr Mwrm3Serial::WrmState wrm_state {};

    namespace Mwrm3Serial
    {
        struct FdxNew
        {
            static constexpr Type type = Type::FdxNew;

            template<class F>
            static auto serialize(array_view_const_char filename, F&& f)
            {
                return f(integral_type<type>(), detail::Buffer{
                    type,
                    detail::types::u16_bytes_size{filename}
                }.bytes(), bytes_view(filename));
            }

            template<class F, class FError>
            static decltype(auto) unserialize(bytes_view buf, F&& f, FError&& ferror)
            {
                using namespace detail::readers;
                return detail::unserialize<type>(buf, f, ferror, group(u16str()));
            }
        };
    }

    inline constexpr Mwrm3Serial::FdxNew fdx_new {};

    namespace Mwrm3Serial
    {
        struct TflNew
        {
            static constexpr Type type = Type::TflNew;

            template<class F>
            static auto serialize(
                FileId idx, FileSize file_size,
                Direction direction, TransferedStatus transfered_status,
                bytes_view original_filename, bytes_view tfl_filename,
                QuickHash quick_hash, FullHash full_hash,
                Sha256Signature signature,
                F&& f)
            {
                assert(quick_hash.hash.size() == full_hash.hash.size());
                return f(integral_type<type>(),
                    detail::Buffer{
                        type,
                        detail::types::u64{idx},
                        file_size,
                        detail::types::u8_unsafe{
                            ((quick_hash.hash.empty() ? 0 : 1) << 0)
                          | ((full_hash.hash.empty() ? 0 : 1) << 1)
                          | ((signature.sig.empty() ? 0 : 1) << 2)
                          | ((safe_cast<uint8_t>(direction) & 0b11) << 3)
                          | ((safe_cast<uint8_t>(transfered_status) & 0b11) << 5)
                        },
                        detail::types::u16_bytes_size{original_filename},
                        detail::types::u16_bytes_size{tfl_filename},
                    }.bytes(),
                    original_filename,
                    tfl_filename,
                    quick_hash.hash,
                    full_hash.hash,
                    signature.sig);
            }

            template<class F, class FError>
            static decltype(auto) unserialize(bytes_view buf, F&& f, FError&& ferror)
            {
                using namespace detail::readers;
                u16str_ref original_filename;
                u16str_ref tfl_filename;
                u8mask m;
                return detail::unserialize<type>(buf, f, ferror,
                    group(
                        file_id(),
                        file_size(),
                        shadow_ref{m},
                        bits_from<0b11'000, 3, u8mask, Direction>{m},
                        bits_from<0b11'00000, 5, u8mask, TransferedStatus>{m},
                        original_filename.size(),
                        tfl_filename.size(),
                        original_filename.data()
                    ),
                    group(tfl_filename.data()),
                    group(m.optional_for<quick_hash, 0>()),
                    group(m.optional_for<full_hash, 1>()),
                    group(m.optional_for<sha256_signature, 2>())
                );
            }
        };
    }

    inline constexpr Mwrm3Serial::TflNew tfl_new {};


    template<class F, class FError>
    decltype(auto) serial_for_type(Mwrm3::Type type, F&& f_ok, FError&& f_error)
    {
#define X_MACRO(f) \
    f(wrm_new)     \
    f(wrm_state)   \
    f(fdx_new)     \
    f(tfl_new)     \
    f(mwrm_header_compatibility)

#define TYPE(S) decltype(f_ok(S)),
#define CASE(S) case S.type: return Result(f_ok(S));

        using none_type = integral_type<Type::None>;
        using Result = std::common_type_t<X_MACRO(TYPE) decltype(f_error(none_type()))>;

        switch (type)
        {
            X_MACRO(CASE)
            case Type::None:;
        }

        return Result(f_error(none_type()));

#undef TYPE
#undef CASE
#undef X_MACRO
    }

    template<class F, class FError>
    decltype(auto) unserialize_packet(Mwrm3::Type type, bytes_view av, F&& f_ok, FError&& f_error)
    {
        return serial_for_type(type, [&](auto serial){
            auto serial_error = [&]{ return f_error(integral_type<serial.type>()); };
            return serial.unserialize(av, f_ok, serial_error);
        }, f_error);
    }

    enum class ParserResult
    {
        Ok,
        UnknownType,
        NeedMoreData,
    };

    template<class F>
    ParserResult parse_packet(bytes_view av, F&& f)
    {
        if (av.size() >= 2)
        {
            using namespace Mwrm3;

            auto f_ok = [&](auto type, bytes_view remaining, auto... xs){
                f(type, remaining, xs...);
                return ParserResult::Ok;
            };
            auto f_error = [](Type type){
                return type == Type::None
                    ? ParserResult::UnknownType
                    : ParserResult::NeedMoreData;
            };

            InStream in(av);
            auto type = Type(in.in_uint16_le());
            return unserialize_packet(type, in.remaining_bytes(), f_ok, f_error);
        }

        return ParserResult::NeedMoreData;
    }
}
