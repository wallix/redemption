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
Copyright (C) Wallix 2018
Author(s): Jonathan Poelen
*/

#pragma once

#include <iomanip>
#include <ostream>
#include <string>
#include <type_traits>

#include <cstdlib>
#include <cstring>

#include "utils/sugar/array_view.hpp"


namespace cli
{
    template<class T, class = void>
    struct has_optional_value : std::false_type
    {};

    template<class T>
    struct has_optional_value<T, decltype(void(T::has_optional_value))> : std::false_type
    {};

    enum class Res
    {
        Ok,
        Exit,
        Help,
        BadOption,
        BadFormat
    };

    struct ParseResult
    {
        int opti;
        int argc;
        char const * const * argv;
        char const * str;
        Res res;
    };

    struct DataOption
    {
        char short_name;
        char const * long_name;
        char const * help;
        bool optional_value;
    };

    constexpr struct Ok_ : std::integral_constant<Res, Res::Ok> {} Ok {};

    template<class Act>
    auto apply_option_impl(ParseResult& pr, Act && act, int)
    -> decltype(act(pr))
    {
        return act(pr);
    }

    template<class Act>
    auto apply_option_impl(ParseResult&, Act && act, char)
    -> decltype(act())
    {
        return act();
    }

    template<class Act>
    Res apply_option(ParseResult& pr, Act && act)
    {
        return apply_option_impl(pr, act, 1), Ok;
    }

    template<class Act>
    struct Option
    {
        DataOption d;
        Act act;

        Option(char short_name, char const * long_name, Act act = Act{})
          : d{short_name, long_name, nullptr, false}
          , act(act)
        {}

        Option(DataOption const & d, Act act)
          : d(d)
          , act(act)
        {}

        Option& optional()
        {
            this->d.optional_value = true;
            return *this;
        }

        Option & help(char const * mess)
        {
            this->d.help = mess;
            return *this;
        }

        template<class NewAct>
        Option<NewAct> action(NewAct act)
        {
            static_assert(std::is_same<void, decltype(
                void(apply_option(std::declval<ParseResult&>(), act))
            )>::value);

            if (has_optional_value<NewAct>::value) {
                this->d.optional_value = true;
            }
            return {this->d, act};
        }
    };

    struct NoAct
    {
        void operator()() const
        {}
    };

    template<class T>
    T operator, (T x, Ok_)
    { return x; }

    template<class Act>
    struct Quit
    {
        Act act;

        Res operator()(ParseResult& pr) const
        {
            apply_option(pr, act);
            return Res::Exit;
        }
    };

    template<class Act>
    Quit<Act> quit(Act act)
    {
        return {act};
    }

    constexpr struct Help
    {
        Res operator()(ParseResult const&) const
        {
            return Res::Help;
        }
    } help {};

    template<class Act>
    Res arg_parse(int*, char const * s, Act act)
    {
        return act(static_cast<int>(std::strtol(s, nullptr, 0))), Ok;
    }

    template<class Act>
    Res arg_parse(unsigned*, char const * s, Act act)
    {
        return act(static_cast<unsigned>(std::strtoul(s, nullptr, 0))), Ok;
    }

    template<class Act>
    Res arg_parse(long*, char const * s, Act act)
    {
        return act(std::strtol(s, nullptr, 0)), Ok;
    }

    template<class Act>
    Res arg_parse(unsigned long*, char const * s, Act act)
    {
        return act(std::strtoul(s, nullptr, 0)), Ok;
    }

    template<class Act>
    Res arg_parse(long long*, char const * s, Act act)
    {
        return act(std::strtoll(s, nullptr, 0)), Ok;
    }

    template<class Act>
    Res arg_parse(unsigned long long*, char const * s, Act act)
    {
        return act(std::strtoull(s, nullptr, 0)), Ok;
    }

    template<class Act>
    Res arg_parse(std::string*, char const * s, Act act)
    {
        return act(s), Ok;
    }

    template<class Act>
    Res arg_parse(char const *, char const * s, Act act)
    {
        return act(s), Ok;
    }

    template<class T, class Act>
    Res arg_parse(array_view<T>*, char const * s, Act act)
    {
        return act({s, strlen(s)}), Ok;
    }

    template<class T, class Act>
    struct Arg
    {
        char const * name;
        Act act;

        static_assert(std::is_same<void, decltype(
            void(std::declval<Act const&>()(std::declval<T>()))
        )>::value);

        Res operator()(ParseResult& pr) const
        {
            auto r = arg_parse(static_cast<T*>(nullptr), pr.str, act);
            if (r == Res::Ok) {
                ++pr.opti;
            }
            return r;
        }
    };

    template<class T, class Act>
    Arg<T, Act> arg(char const * name, Act act)
    {
        return {name, act};
    }

    template<class T, class Act>
    Arg<T, Act> arg(Act act)
    {
        return {nullptr, act};
    }

    template<class Mem>
    struct ParamTypeMem;

    template<class R, class M, class T>
    struct ParamTypeMem<R (M::*)(T) const>
    {
        using type = typename std::decay<T>::type;
    };

    template<class F>
    using ParamType = typename ParamTypeMem<decltype(&F::operator())>::type;

    template<class Act>
    Arg<ParamType<Act>, Act> arg(char const * name, Act act)
    {
        return {name, act};
    }

    template<class Act>
    Arg<ParamType<Act>, Act> arg(Act act)
    {
        return {nullptr, act};
    }

    template<class Act>
    struct OnOff
    {
        static const bool has_optional_value = true;

        Act act;

        static_assert(std::is_same<void, decltype(
            void(std::declval<Act const&>()(bool{}))
        )>::value);

        Res operator()(ParseResult& pr) const
        {
            Res r = (this->act(!*pr.str || !strcmp(pr.str, "on")), Ok);
            if (r == Res::Ok) {
                ++pr.opti;
            }
            return r;
        }
    };

    template<class Act>
    OnOff<Act> on_off(Act act)
    {
        return {act};
    }

    inline auto on_off_location(bool& x)
    {
        return on_off([&](bool state){ x = state; });
    }

    template<auto X, class T>
    inline auto on_off_bit_location(T& value)
    {
        return on_off([&](bool state){
            if (state) {
                value |= X;
            }
            else {
                value &= ~X;
            }
        });
    }

    template<class T>
    struct SetStr
    {
        T& value;

        static_assert(std::is_same<void, decltype(
            void(std::declval<T&>() = std::declval<char const*>())
        )>::value);

        Res operator()(ParseResult& pr) const
        {
            value = pr.str;
            return Ok;
        }
    };

    template<class T>
    inline auto arg_location(char const* name, T& x)
    {
        return arg<T>(name, [&x](auto&& value) { x = std::forward<decltype(value)>(value); });
    }

    template<class T>
    inline auto arg_location(T& x)
    {
        return arg_location(nullptr, x);
    }

    inline Option<NoAct> option(char short_name, char const * long_name)
    {
        return {short_name, long_name};
    }

    inline Option<NoAct> option(char short_name)
    {
        return {short_name, ""};
    }

    inline Option<NoAct> option(char const * long_name)
    {
        return {0, long_name};
    }

    struct Helper
    {
        char const * s;
    };

    inline Helper helper(char const * s)
    {
        return {s};
    }

    inline Res parse_long_option(char const *, ParseResult const&)
    {
        return Res::BadOption;
    }

    template<class... Opts>
    Res parse_long_option(char const * s, ParseResult& pr, Helper const&, Opts const&... opts)
    {
        return parse_long_option(s, pr, opts...);
    }

    template<class Opt, class... Opts>
    Res parse_long_option(char const * s, ParseResult& pr, Opt const& opt, Opts const&... opts)
    {
        auto* s1 = opt.d.long_name;
        auto* s2 = s;
        for (; *s1 == *s2 && *s1; ++s1, ++s2) {
        }
        if (!*s1) {
            if (!*s2) {
                if (opt.d.optional_value) {
                    pr.str = "";
                }
                else {
                    ++pr.opti;
                    pr.str = (pr.opti < pr.argc) ? pr.argv[pr.opti] : "";
                }
                return apply_option(pr, opt.act);
            }
            else if (*s2 == '=') {
                pr.str = s2+1;
                return apply_option(pr, opt.act);
            }
        }
        return parse_long_option(s, pr, opts...);
    }

    inline Res parse_short_option(char const *, ParseResult const&)
    {
        return Res::BadOption;
    }

    template<class... Opts>
    Res parse_short_option(char const * s, ParseResult& pr, Helper const&, Opts const&... opts)
    {
        return parse_short_option(s, pr, opts...);
    }

    template<class Opt, class... Opts>
    Res parse_short_option(char const * s, ParseResult& pr, Opt const& opt, Opts const&... opts)
    {
        if (opt.d.short_name == s[0]) {
            if (s[1]) {
                pr.str = s + 1;
            }
            else {
                ++pr.opti;
                pr.str = (pr.opti < pr.argc) ? pr.argv[pr.opti] : "";
            }
            return apply_option(pr, opt.act);
        }
        return parse_short_option(s, pr, opts...);
    }

    template<class Output, class Opt, class T, class Act>
    void print_action(Output&& out, Opt const& opt, Arg<T, Act> const& arg)
    {
        out << " [";
        if (arg.name) {
            out << arg.name;
        }
        else if (*opt.d.long_name) {
            out << opt.d.long_name;
        }
        else {
            out << opt.d.short_name;
        }
        out << ']';
    }

    template<class Output, class Opt, class Act>
    void print_action(Output&& out, Opt const&, OnOff<Act> const&)
    {
        out << "[={on|off}]";
    }

    template<class Output, class Opt, class Act>
    void print_action(Output&&, Opt const&, Act const&)
    {}

    template<class Opt>
    void print_help(std::ostream& out, Opt const& opt)
    {
        constexpr int minlen = 32;
        int n = 0;

        if (opt.d.short_name) {
            n += 2;
            out << '-' << opt.d.short_name;
            if (*opt.d.long_name) {
                n += 4;
                n += int(strlen(opt.d.long_name));
                out << ", --" << opt.d.long_name;
            }
        }
        else if (*opt.d.long_name) {
            n += 2;
            n += int(strlen(opt.d.long_name));
            out << "--" << opt.d.long_name;
        }

        struct Output
        {
            std::ostream& out;
            int& n;
            Output& operator<<(char const* s)
            {
                n += int(strlen(s));
                out << s;
                return *this;
            }

            Output& operator<<(char c)
            {
                n += 1;
                out << c;
                return *this;
            }
        };
        print_action(Output{out, n}, opt, opt.act);

        out << std::setw(std::max(minlen - n, 0)) << "  " << opt.d.help << "\n";
    }

    inline void print_help(std::ostream & out, Helper h)
    {
        out << "\n" << h.s << "\n\n";
    }

    template<class... Opts>
    auto options(Opts... opts)
    {
        return [opts...](auto && f){
            return f(opts...);
        };
    }

    template<class Tuple>
    ParseResult parse(Tuple const& t, int const ac, char const * const * const av)
    {
        ParseResult r;
        r.argc = ac;
        r.argv = av;
        r.opti = 1;
        r.str = nullptr;
        r.res = t([&r](auto... opts) {
            while (r.opti < r.argc) {
                auto * s = r.argv[r.opti];
                Res res = Res::BadFormat;
                if (s[0] == '-' && s[1]) {
                    if (s[1] == '-') {
                        if (s[2]) {
                            res = parse_long_option(s+2, r, opts...);
                        }
                    }
                    else {
                        res = parse_short_option(s+1, r, opts...);
                    }
                }
                if (res != Res::Ok) {
                    return res;
                }
            }
            return Res::Ok;
        });
        return r;
    }

    template<class Tuple>
    void print_help(Tuple const& t, std::ostream & out)
    {
        t([&out](auto... opts) {
            (void)std::initializer_list<int>{
                (print_help(out, opts), 0)...
            };
        });
    }
}
