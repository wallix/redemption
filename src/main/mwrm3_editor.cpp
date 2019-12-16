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

#include "capture/mwrm3.hpp"
#include "transport/crypto_transport.hpp"
#include "utils/genfstat.hpp"
#include "utils/sugar/scope_exit.hpp"

#include <iostream>

#if REDEMPTION_HAS_INCLUDE(<cxxabi.h>)
#  include <cxxabi.h>
#endif

namespace
{

// TODO from ScytaleMwrm3ReaderData::next
struct Mwrm3FileReader
{
    Mwrm3FileReader(InCryptoTransport& crypto_transport)
    : crypto_transport(crypto_transport)
    {}

    enum class [[nodiscard]] NextResult
    {
        Ok,
        DataToLarge,
        Eof,
    };

    NextResult next_data()
    {
        if (this->remaining_data.size() == this->buffer.size())
        {
            return NextResult::DataToLarge;
        }

        memmove(this->buffer.data(), this->remaining_data.data(), this->remaining_data.size());

        const auto free_buffer_len = this->buffer.size() - this->remaining_data.size();

        size_t len = crypto_transport.partial_read(
            this->buffer.data() + this->remaining_data.size(),
            free_buffer_len);

        if (len == 0)
        {
            return NextResult::Eof;
        }

        this->remaining_data = array_view(this->buffer)
            .first(len + this->remaining_data.size());

        return NextResult::Ok;
    }

    bytes_view get_bytes() const noexcept
    {
        return this->remaining_data;
    }

private:
    std::array<uint8_t, 1024*16> buffer;
    bytes_view remaining_data {"", 0};
    InCryptoTransport& crypto_transport;
};


struct TypeName : std::string_view {};

template<class T>
constexpr TypeName get_type_name()
{
    std::string_view s = __PRETTY_FUNCTION__;
    s.remove_suffix(1);
#ifdef __clang__
    s.remove_prefix(76);
    return {s};
#elif defined(__GNUG__)
    s.remove_prefix(71);
    return {s};
#endif
}

template<class E, E e>
constexpr TypeName get_enum_name()
{
#if defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 9)
    std::string_view s = __PRETTY_FUNCTION__;
#ifdef __clang__
    s.remove_prefix(76 + get_type_name<E>().size() + 6);
    if ('0' <= s[0] && s[0] <= '9')
    {
        return {};
    }
#elif defined(__GNUG__)
    s.remove_prefix(71 + get_type_name<E>().size() + 8);
    if (s[0] == '(')
    {
        return {};
    }
#endif
    s.remove_prefix(get_type_name<E>().size() + 2);
    s.remove_suffix(1);
    return {s};
#else
    return {};
#endif
}


constexpr inline char const* hexadecimal_string = "0123456789ABCDEF";

void print_value(array_view_const_char chars)
{
    std::cout.write(chars.data(), chars.size());
}

void print_value(bytes_view bytes)
{
    for (auto byte : bytes)
    {
        if (' ' <= byte && byte <= '~')
        {
            if (byte == '\\')
            {
                std::cout << '\\';
            }

            std::cout << byte;
        }
        else
        {
            std::cout << "\\x";
            std::cout << hexadecimal_string[byte >> 4];
            std::cout << hexadecimal_string[byte & 0xf];
        }
    }
}

char const* duration_suffix(std::chrono::seconds const&) { return "s"; }
// char const* duration_suffix(std::chrono::milliseconds const&) { return "ms"; }
// char const* duration_suffix(std::chrono::nanoseconds const&) { return "ns"; }

template<class Rep, class Period>
void print_value(std::chrono::duration<Rep, Period> duration)
{
    // C++20: std::cout << duration;
    std::cout << duration.count() << duration_suffix(duration);
}

template<class T, class = void>
struct print_value_impl;

template<class E>
class enum_names
{
    constexpr static std::size_t _len(std::index_sequence<>)
    {
        return 0;
    }

    template<std::size_t... ints>
    constexpr static std::size_t _len(std::index_sequence<ints...>)
    {
        std::string_view names[]{get_enum_name<E, E(ints)>()...};
        for (auto& name : names)
        {
            if (name.empty())
            {
                return &name - names;
            }
        }
        return 0;
    }

    template<std::size_t... ints>
    constexpr static auto _names(std::index_sequence<ints...>)
    {
        return std::array<std::string_view, sizeof...(ints)>{
            get_enum_name<E, E(ints)>()...
        };
    }

    template<std::size_t... ints>
    constexpr static auto _name(E e, std::index_sequence<ints...>)
    {
        auto v = +underlying_cast(e);
        std::string_view sv;
        ((v == ints ? void(sv = get_enum_name<E, E(ints)>()) : void()), ...);
        return sv;
    }

public:
    static constexpr unsigned char size
        = _len(std::make_index_sequence<(sizeof(E) <= sizeof(uint16_t) ? 126 : 0)>());

    static std::string_view name(E e)
    {
        if constexpr (size > 0)
        {
            return _name(e, std::make_index_sequence<size>());
        }
        else
        {
            (void)e;
            return {};
        }
    }
};

template<class T>
void print_enum_value(T x, std::string_view name)
{
    if (not name.empty())
    {
        std::cout << name << "(" << x << ")";
    }
    else
    {
        std::cout << x;
    }
}

template<class E, E e>
void print_enum()
{
    print_enum_value(+underlying_cast(e), get_enum_name<Mwrm3::Type, e>());
}

template<class E>
void print_enum(E e)
{
    print_enum_value(+underlying_cast(e), enum_names<E>::name(e));
}

template<class T>
void print_value(T const& x)
{
    if constexpr (std::is_enum_v<T>)
    {
        print_enum(x);
    }
    else if constexpr (std::is_integral_v<T>)
    {
        std::cout << x;
    }
    else
    {
        print_value_impl<T>::print(x);
    }
}

template<class T>
struct print_value_impl<T, decltype(void(std::declval<T&>().bytes))>
{
    static void print(T const& x)
    {
        print_value(x.bytes);
    }
};

template<class T>
struct print_value_impl<T, decltype(void(std::declval<T&>().str))>
{
    static void print(T const& x)
    {
        print_value(x.str);
    }
};

int mwrm_text_viewer(Mwrm3FileReader& file)
{
    auto remaining_data = file.get_bytes();
    int nb_packet = 0;

    auto print_values = [&](auto type, bytes_view next_data, auto... xs){
        ++nb_packet;
        remaining_data = next_data;

        std::cout << "#" << nb_packet << "\nMwrm3::Type: "; print_enum<Mwrm3::Type, type.value>();
        ((
            std::cout << "\n" << get_type_name<decltype(xs)>() << ": ",
            print_value(xs)
        ), ...);
        std::cout << "\n\n";
    };

    for (;;) switch (Mwrm3::parse_packet(remaining_data, print_values))
    {
        case Mwrm3::ParserResult::Ok:
            break;

        case Mwrm3::ParserResult::UnknownType: {
            auto int_type = InStream(remaining_data).in_uint16_le();
            std::cerr << "Unknown type: 0x" << std::hex << int_type << "\n";
            return 1;
        }

        case Mwrm3::ParserResult::NeedMoreData: {
            switch (file.next_data())
            {
                case Mwrm3FileReader::NextResult::Ok:
                    remaining_data = file.get_bytes();
                    break;

                case Mwrm3FileReader::NextResult::Eof:
                    return 0;

                case Mwrm3FileReader::NextResult::DataToLarge:
                    std::cerr << "Data too large\n";
                    return 2;
            }
        }
    }
}

}

int main(int ac, char** av)
{
    if (ac != 2)
    {
        std::cerr << "Usage: mwrm3_editor file.mwrm3\n";
        return 255;
    }

    CryptoContext cctx;
    Fstat fstat;
    InCryptoTransport infile(cctx, InCryptoTransport::EncryptionMode::Auto, fstat);
    infile.open(av[1]);

    Mwrm3FileReader reader{infile};
    return mwrm_text_viewer(reader);
}
