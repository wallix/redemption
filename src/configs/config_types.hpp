/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_SRC_UTILS_CONFIG_TYPES_TYPES_HPP
#define REDEMPTION_SRC_UTILS_CONFIG_TYPES_TYPES_HPP

#include "parser.hpp"
#include "fileutils.hpp"
#include "basefield.hpp"
#include "underlying_cast.hpp"
#include "dynamic_buffer.hpp"

#include "config_capture_flags.hpp"
#include "config_keyboard_log_flags.hpp"
#include "config_c_str_buf.hpp"

#include <iosfwd>
#include <type_traits>

#include <cstddef>

namespace configs {

using BaseField = FieldObserver::BaseField;
using StringField = FieldObserver::StringField;
using BoolField = FieldObserver::BoolField;
using UnsignedField = FieldObserver::UnsignedField;
using SignedField = FieldObserver::SignedField;

struct IniAccounts {
    char username[255]; // should use string
    char password[255]; // should use string

    IniAccounts()
    {
        this->username[0] = 0;
        this->password[0] = 0;
    }
};

struct null_fill { null_fill() {}; };

struct StringCopier
{
    static void copy(char * dest, char const * src, std::size_t n) {
        const char * e = dest + n - 1;
        for (; dest != e && *src; ++src, ++dest) {
            *dest = *src;
        }
        *dest = 0;
    }
};

template<std::size_t N, class Copier, bool NullableString = false>
class StaticStringBase
{
    struct disable_ctor { };

    StaticStringBase(StaticStringBase const &) = delete;

public:
    explicit StaticStringBase(typename std::conditional<NullableString, null_fill, disable_ctor>::type) noexcept
    : str{}
    {}

    StaticStringBase() noexcept
    { this->str[0] = 0; }

    explicit StaticStringBase(const char * s)
    { *this = s; }

    StaticStringBase& operator=(const char * s) {
        Copier::copy(this->str, s, N);
        return *this;
    }

    const char * c_str() const noexcept {
        return this->str;
    }

    bool empty() const {
        return (this->str[0] == '\0');
    }

    constexpr operator const char * () const noexcept {
        return this->str;
    }

    constexpr std::size_t max_size() const noexcept {
        return N - 1;
    }

    char * data() noexcept {
        return this->str;
    }

protected:
    char str[N];

    // disable deleted constructor
    friend class Inifile;
    StaticStringBase(StaticStringBase &&);
};


template<std::size_t N, class Copier, bool NullableString>
char const * c_str(
    CStrBuf<StaticStringBase<N, Copier, NullableString>>&,
    StaticStringBase<N, Copier, NullableString> const & x
) {
    return x.c_str();
}

template<std::size_t N>
using StaticString = StaticStringBase<N, StringCopier>;

template<std::size_t N>
using StaticNilString = StaticStringBase<N, StringCopier, true>;

template<std::size_t N>
struct StaticKeyString : StaticStringBase<N+1, null_fill, true>
{
    explicit StaticKeyString(const char * s) {
        this->setmem(s);
        this->str[N] = 0;
    }

    void setmem(const char * s, std::size_t n) {
        memcpy(this->str, s, n);
    }

    void setmem(const char * s) {
        this->setmem(s, N);
    }
};

template<std::size_t N>
struct CStrBuf<StaticKeyString<N>> {
    char buf[N*2 + 1];
};

struct Ipv4Copier : StringCopier {};
using StaticIpString = StaticStringBase<16, Ipv4Copier>;

struct PathCopier
{
    static void copy(char * dest, char const * src, std::size_t n) {
        pathncpy(dest, src, n);
    }
};

template<std::size_t N>
using StaticPath = StaticStringBase<N, PathCopier>;


template<class T, T Min, T Max, T Default = Min>
struct Range {
    static_assert(Min < Max, "interval error");
    static_assert(Min <= Default && Default < Max, "interval error");

    explicit Range(T x = Default)
    : x_(x)
    {}

    Range & operator = (T x) {
        if (x < Min) {
            this->x_ = Min;
        }
        else if (x > Max) {
            this->x_ = Max;
        }
        else {
            this->x_ = x;
        }
        return *this;
    }
    T get() const { return this->x_; }
    operator T () const { return this->x_; }

private:
    T x_;
};

template<class T, T Min, T Max, T Default>
struct CStrBuf<Range<T, Min, Max, Default>>
: CStrBuf<T>
{};


template<std::size_t N>
char const * c_str(CStrBuf<StaticKeyString<N>>& s, StaticKeyString<N> const & key) {
    s.get()[N*2] = 0;
    copy_val(key, s.get(), s.size());
    return s.get();
}

template<class T, T Min, T Max, T Default>
char const * c_str(CStrBuf<Range<T, Min, Max, Default>>& s, Range<T, Min, Max, Default> const & x) {
    return c_str(s, x.get());
}



template<class Enum>
class FlagsField : public BaseField
{
    Enum data;
    char buff[20];

public:
    FlagsField() : data(Enum::none) {
    }

    void set(Enum that) {
        if (this->data != that || this->asked) {
            this->modify();
            this->data = that;
        }
        this->asked = false;
    }

    void set_from_acl(const char * cstr) override {
        this->modify_from_acl();
        this->data = static_cast<Enum>(ulong_from_cstr(cstr)) & Enum::FULL;
        this->asked = false;
    }
    void set_from_cstr(const char * cstr) override {
        this->set(static_cast<Enum>(ulong_from_cstr(cstr)) & Enum::FULL);
    }

    Enum get() const {
        return this->data;
    }

    const char * get_value() override {
        if (this->is_asked()) {
            return "ASK";
        }
        snprintf(buff, sizeof(buff), "%u", static_cast<unsigned>(this->data));
        return buff;
    }
};


template<class Enum, class Traits>
class EnumField : public BaseField
{
protected:
    Enum data;

public:
    EnumField() : data(Traits::get_default()) {
    }

    void set(Enum lang) {
        if (!Traits::valid(lang)) {
            return ;
        }
        if (this->asked || this->data != lang) {
            this->modify();
            this->data = lang;
        }
        this->asked = false;
    }
    void set(const char * cstr) {
        this->set(Traits::cstr_to_enum(cstr));
    }

    void set_from_acl(const char * cstr) override {
        Enum lang = Traits::cstr_to_enum(cstr);
        if (!Traits::valid(lang)) {
            return ;
        }
        this->modify_from_acl();
        this->data = lang;
        this->asked = false;
    }
    void set_from_cstr(const char * cstr) override {
        this->set(Traits::cstr_to_enum(cstr));
    }

    Enum get() const {
        return Traits::valid(this->data) ? this->data : Traits::get_valid();
    }

    const char * get_value() override {
        if (this->is_asked()) {
            return "ASK";
        }
        return Traits::to_string(this->data);
    }
};


#include "mk_enum_def.hpp"

template<class E, class = void>
struct enum_option
{ using type = std::false_type; };

#ifdef IN_IDE_PARSER
# define ENUM_OPTION(Enum, X, ...)
#else
# define ENUM_OPTION(Enum, X, ...)                       \
    template<class T> struct enum_option<Enum, T> {      \
        static constexpr const std::initializer_list<    \
            std::decay<decltype(X)>::type                \
        > value {X, __VA_ARGS__};                        \
        using type = std::true_type;                     \
    };                                                   \
    template<class T> constexpr const                    \
    std::initializer_list<std::decay<decltype(X)>::type> \
    enum_option<Enum, T>::value
#endif

#define MK_ENUM_FIELD(Enum, ...)                               \
    MK_ENUM_IO(Enum)                                           \
    ENUM_OPTION(Enum, __VA_ARGS__);                            \
    inline void parse(Enum & e, char const * cstr)  {          \
        unsigned i = 0;                                        \
        auto l = {__VA_ARGS__};                                \
        for (auto s : l) {                                     \
            if (0 == strcmp(cstr, s)) {                        \
                e = static_cast<Enum>(i);                      \
            }                                                  \
            ++i;                                               \
        }                                                      \
        /*e = get_valid();*/                                   \
    }                                                          \
    inline int copy_val(Enum e, char * buff, std::size_t n)  { \
        char const * cstr;                                     \
        auto l{__VA_ARGS__};                                   \
        if (underlying_cast(e) >= underlying_cast(Enum::NB)) { \
            cstr = *l.begin();                                 \
        }                                                      \
        cstr = *(l.begin() + underlying_cast(e));              \
        return snprintf(buff, n, "%s", cstr);                  \
    }                                                          \
    inline char const * c_str(CStrBuf<Enum>& , Enum e) {       \
        auto l{__VA_ARGS__};                                   \
        if (underlying_cast(e) >= underlying_cast(Enum::NB)) { \
            return *l.begin();                                 \
        }                                                      \
        return *(l.begin() + underlying_cast(e));              \
    }



template<class E>
E enum_to_option(E e) {
    return e;
}

enum class Level : unsigned { low, medium, high, NB };
ENUM_OPTION(Level, "low", "medium", "high");
MK_ENUM_IO(Level)

inline Level level_from_cstr(char const * value) {
    return
        0 == strcasecmp("medium", value) ? Level::medium
      : 0 == strcasecmp("high",value) ? Level::high
      : Level::low
    ;
}

inline char const * cstr_from_level(Level lvl) {
    return lvl == Level::high
        ? "high"
        : lvl == Level::medium
        ? "medium"
        : "low";
}

struct LevelFieldTraits {
    static Level get_default() { return Level::low; }
    static Level get_valid() { return Level::low; }
    static bool valid(Level) { return true; }
    static Level cstr_to_enum(char const * cstr) { return level_from_cstr(cstr); }
    static char const * to_string(Level e) {
        switch (e) {
            case Level::medium: return "medium";
            case Level::high: return "high";
            default: return "low";
        }
    }
};

inline char const * enum_to_option(Level e) {
    return LevelFieldTraits::to_string(e);
}

using LevelField = EnumField<Level, LevelFieldTraits>;


enum class Language : unsigned { en, fr, NB };
MK_ENUM_FIELD(Language, "en", "fr")


enum class ClipboardEncodingType : unsigned { utf8, latin1, NB };
MK_ENUM_FIELD(ClipboardEncodingType, "utf-8", "latin1")

enum class ClipboardLogFlags : unsigned {
    none,
    syslog = 1 << 0,
    FULL = ((1 << 1) - 1)
};
MK_ENUM_FLAG_FN(ClipboardLogFlags)
MK_PARSER_ENUM_FLAGS(ClipboardLogFlags)


enum class ColorDepth : unsigned {
    unspecified,
    depth8 = 8,
    depth15 = 15,
    depth16 = 16,
    depth24 = 24,
    //depth32,
};
ENUM_OPTION(ColorDepth, 8, 15, 16, 24);
MK_ENUM_IO(ColorDepth)

inline ColorDepth color_depth_from_cstr(char const * value) {
    switch (ulong_from_cstr(value)) {
        case 8: return ColorDepth::depth8;
        case 15: return ColorDepth::depth15;
        case 16: return ColorDepth::depth16;
        default:
        case 24: return ColorDepth::depth24;
        //case 32: x = ColorDepth::depth32;
    }
}

inline char const * cstr_from_color_depth(ColorDepth c) {
    switch (c) {
        case ColorDepth::depth8: return "8";
        case ColorDepth::depth15: return "15";
        case ColorDepth::depth16: return "16";
        default:
        case ColorDepth::depth24: return "24";
        //case 32: x = ColorDepth::depth32;
    }
}


#include "mk_enum_undef.hpp"
#undef MK_ENUM_FIELD
#undef ENUM_OPTION


class ReadOnlyStringField : public BaseField {
protected:
    std::string data;
public:
    ReadOnlyStringField() = default;

    void set(std::string const & string) {
        this->set_from_cstr(string.c_str());
    }
    void set_empty() {
        if (!this->data.empty()){
            this->modify();
            this->data.clear();
        }
    }
    void set_from_acl(const char * cstr) override {
        this->modify_from_acl();
        this->data = cstr;
        this->asked = false;
    }
    void set_from_cstr(const char * cstr) override {
        this->data = cstr;
    }
    bool is_empty(){
        return this->data.empty();
    }
    const std::string & get() const {
        return this->data;
    }
    std::string & get() {
        return this->data;
    }

    const char * get_cstr() const {
        return this->get().c_str();
    }

    const char * get_value() override {
        if (this->is_asked()) {
            return "ASK";
        }
        return this->get().c_str();
    }
};

}

using configs::Level;
using configs::Language;
using configs::ColorDepth;
using configs::CaptureFlags;
using configs::KeyboardLogFlags;
using configs::ClipboardLogFlags;
using configs::ClipboardEncodingType;

using configs::level_from_cstr;

#endif
