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

    enum class DataSize : uint16_t;

    constexpr uint16_t data_size_max = ~uint16_t{};
    constexpr unsigned header_line_size = 4u /*u16, u16*/;
    constexpr unsigned line_size_max = data_size_max + header_line_size;

    inline constexpr auto top_header = "v3\n"_av;

    /// \return empty view if buffer is too short, otherwise the serialized header in buffer
    [[nodiscard]]
    inline std::pair<writable_bytes_view, writable_bytes_view>
    serialize_header_line(writable_bytes_view buffer, Type type, DataSize data_size) noexcept
    {
        const auto data_size_ = underlying_cast(data_size);
        const auto need_size = data_size_ + header_line_size;

        if (REDEMPTION_UNLIKELY(buffer.size() < need_size))
        {
            return {};
        }

        OutStream out{buffer};

        out.out_uint16_le(safe_int(type));
        out.out_uint16_le(data_size_);

        return {out.get_bytes(), out.get_tail()};
    }

    /// \return result.first if Type(0) if buffer is too short
    [[nodiscard]]
    inline std::pair<Type, DataSize> parse_header_line(bytes_view buffer) noexcept
    {
        if (REDEMPTION_UNLIKELY(buffer.size() < header_line_size))
        {
            return {};
        }

        InStream in{buffer};

        Type type = safe_int{in.in_uint16_le()};
        DataSize data_size = safe_int{in.in_uint16_le()};

        return {type, data_size};
    }

    enum class FileSize : uint64_t;

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
                static constexpr unsigned size = 2;

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
        }

        template<class T> struct wrap_type { using type = T; };
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

        template<class F, class FError, class... Bytes>
        auto serialize(Type type, F&& f, FError&& ferror, Bytes... bytes)
        {
            const auto sz = (0 + ... + bytes.size());
            if (REDEMPTION_UNLIKELY(data_size_max < sz))
            {
                return ferror(type);
            }
            return f(type, DataSize(sz), bytes...);
        }

        template<class F, class... Bytes>
        auto unsafe_serialize(Type type, F&& f, Bytes... bytes)
        {
            const auto sz = (0 + ... + bytes.size());
            return f(type, DataSize(checked_int{sz}), bytes...);
        }

        template<Type type>
        struct serialize_new_file
        {
            template<class F, class FError>
            auto operator()(bytes_view filename, F&& f, FError&& ferror) const noexcept
            {
                return serialize(type, f, ferror, filename);
            }
        };
    }

    // fn_ok(Type, DataSize, bytes_view...)
    // fn_error(Type)

    inline constexpr detail::serialize_new_file<Type::WrmNew> serialize_wrm_new {};

    template<class F>
    auto serialize_wrm_stat(
        FileSize file_size, std::chrono::seconds duration,
        QuickHash quick_hash, FullHash full_hash,
        F&& f)
    {
        return detail::unsafe_serialize(Type::WrmState, f,
            detail::Buffer{file_size, duration}.bytes(), quick_hash.hash, full_hash.hash);
    }

    inline constexpr detail::serialize_new_file<Type::FdxNew> serialize_fdx_new {};

    template<class F, class FError>
    auto serialize_tfl_new(
        uint64_t idx, bytes_view original_filename, bytes_view reference_filename,
        F&& f, FError&& ferror)
    {
        assert(reference_filename.size() <= 255);
        return detail::serialize(Type::TflNew, f, ferror,
            detail::Buffer{
                detail::types::u64{idx},
                detail::types::u16_unsafe{original_filename.size()},
            }.bytes(),
            original_filename, reference_filename);
    }

    template<class F>
    auto serialize_tfl_stat(uint64_t idx, FileSize file_size, QuickHash quick_hash, FullHash full_hash, F&& f)
    {
        return detail::unsafe_serialize(Type::TflState, f,
            detail::Buffer{
                detail::types::u64{idx},
                file_size,
            }.bytes(), quick_hash.hash, full_hash.hash);
    }
}
