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
        TflState,
    };

    inline constexpr auto top_header = "v3\n"_av;

    enum class FileSize : uint64_t;

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

    namespace detail
    {
        namespace types
        {
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

                uint8_t value() noexcept
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

                uint16_t value() noexcept
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

                uint64_t value() noexcept
                {
                    return x;
                }
            };

            template<class Reader, class T>
            struct to : Reader
            {
                T value() noexcept
                {
                    return T(Reader::value());
                }
            };

            using seconds = to<u64, std::chrono::seconds>;
            using file_size = to<u64, FileSize>;

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

                bytes_view value() noexcept
                {
                    return x;
                }
            };

            // TODO 32 -> constant
            using quick_hash = to<static_bytes<32>, QuickHash>;
            using full_hash = to<static_bytes<32>, FullHash>;

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

                bytes_view value() noexcept
                {
                    return x;
                }
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

                bytes_view value() noexcept
                {
                    return x;
                }
            };

            template<unsigned, class Reader_>
            struct Value
            {
                using Reader = Reader_;
                Reader reader;
            };

            template<class... Values>
            struct Group : Values...
            {
                // TODO u16bytes or u8bytes should be the last element

                static_assert(
                    (... && (Values::Reader::size > 0))
                 || (... && (Values::Reader::size == 0))
                );
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

        template<class F, class... x0>
        auto unwrapper_group(F& f, readers::Group<x0...>& g0)
        {
            return f(static_cast<x0&>(g0).reader.value()...);
        }

        template<class F, class... x0, class... x1>
        auto unwrapper_group(F& f, readers::Group<x0...>& g0, readers::Group<x1...>& g1)
        {
            return f(
                static_cast<x0&>(g0).reader.value()...,
                static_cast<x1&>(g1).reader.value()...);
        }

        template<class F, class... x0, class... x1, class... x2>
        auto unwrapper_group(F& f,
            readers::Group<x0...>& g0,
            readers::Group<x1...>& g1,
            readers::Group<x2...>& g2)
        {
            return f(
                static_cast<x0&>(g0).reader.value()...,
                static_cast<x1&>(g1).reader.value()...,
                static_cast<x2&>(g2).reader.value()...);
        }

        template<class F, class FError, class... ReaderGroup>
        auto unserialize(bytes_view buf, F&& f, FError&& ferror, ReaderGroup... reader)
        {
            InStream in{buf};

            if ((... && reader.read(in)))
            {
                return unwrapper_group(f, reader...);
            }
            else
            {
                return ferror();
            }
        }

        template<Type type>
        struct serialize_bytes_view
        {
            template<class F>
            auto operator()(bytes_view bytes, F&& f) const noexcept
            {
                return f(type, Buffer{
                    type,
                    detail::types::u16_bytes_size{bytes}
                }.bytes(), bytes);
            }
        };

        template<Type type>
        struct unserialize_bytes_view
        {
            template<class F, class FError>
            auto operator()(bytes_view buf, F&& f, FError&& ferror) const noexcept
            {
                return unserialize(buf, f, ferror, readers::group(readers::u16bytes()));
            }
        };
    }

    // fn_ok(Type, DataSize, bytes_view...)
    // fn_error(Type)

    inline constexpr auto serialize_wrm_new = detail::serialize_bytes_view<Type::WrmNew>{};
    inline constexpr auto unserialize_wrm_new = detail::unserialize_bytes_view<Type::WrmNew>{};

    template<class F>
    auto serialize_wrm_stat(
        FileSize file_size, std::chrono::seconds duration,
        QuickHash quick_hash, FullHash full_hash,
        F&& f)
    {
        return f(Type::WrmState,
            detail::Buffer{
                Type::WrmState,
                file_size,
                duration,
            }.bytes(),
            quick_hash.hash,
            full_hash.hash);
    }

    template<class F, class FError>
    auto unserialize_wrm_stat(bytes_view buf, F&& f, FError&& ferror)
    {
        using namespace detail::readers;
        return detail::unserialize(buf, f, ferror,
            group(file_size(), seconds(), quick_hash(), full_hash())
        );
    }

    inline constexpr auto serialize_fdx_new = detail::serialize_bytes_view<Type::FdxNew>{};
    inline constexpr auto unserialize_fdx_new = detail::unserialize_bytes_view<Type::FdxNew>{};

    template<class F>
    auto serialize_tfl_new(
        uint64_t idx, bytes_view original_filename, bytes_view reference_filename,
        F&& f)
    {
        assert(reference_filename.size() <= 255);
        return f(Type::TflNew,
            detail::Buffer{
                Type::TflNew,
                detail::types::u64{idx},
                detail::types::u16_bytes_size{original_filename},
            }.bytes(),
            original_filename,
            detail::Buffer{
                detail::types::u16_bytes_size{reference_filename}
            }.bytes(),
            reference_filename);
    }

    template<class F, class FError>
    auto unserialize_tfl_new(bytes_view buf, F&& f, FError&& ferror)
    {
        using namespace detail::readers;
        return detail::unserialize(buf, f, ferror,
            group(u64(), u16bytes()), group(u16bytes())
        );
    }

    template<class F>
    auto serialize_tfl_stat(uint64_t idx, FileSize file_size, QuickHash quick_hash, FullHash full_hash, F&& f)
    {
        return f(Type::TflState,
            detail::Buffer{
                Type::TflState,
                detail::types::u64{idx},
                file_size,
            }.bytes(),
            quick_hash.hash,
            full_hash.hash);
    }

    template<class F, class FError>
    auto unserialize_tfl_stat(bytes_view buf, F&& f, FError&& ferror)
    {
        using namespace detail::readers;
        return detail::unserialize(buf, f, ferror,
            group(u64(), file_size(), quick_hash(), full_hash())
        );
    }
}
