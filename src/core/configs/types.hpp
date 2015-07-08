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

template<std::size_t N>
using StaticString = StaticStringBase<N, StringCopier>;

template<std::size_t N>
using StaticNilString = StaticStringBase<N, StringCopier, true>;

template<std::size_t N>
struct StaticKeyString : StaticStringBase<N, null_fill, true>
{
    explicit StaticKeyString(const char * s) {
        this->setmem(s);
    }

    void setmem(const char * s, std::size_t n) {
        memcpy(this->str, s, n);
    }

    void setmem(const char * s) {
        this->setmem(s, N);
    }

    constexpr std::size_t max_size() const noexcept {
        return N;
    }
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
        return Traits::valid(this->data) ? this->data : Traits::get_default();
    }

    const char * get_value() override {
        if (this->is_asked()) {
            return "ASK";
        }
        return Traits::to_string(this->data);
    }
};


#define MK_ENUM_IO(E)                                                                 \
    template<class Ch, class Tr>                                                      \
    std::basic_ostream<Ch, Tr> & operator << (std::basic_ostream<Ch, Tr> & os, E e) { \
        return os << underlying_cast(e);                                              \
    }


#define MK_ENUM_FLAG_FN(E)                                                     \
    MK_ENUM_IO(E)                                                              \
                                                                               \
    inline E operator | (E x, E y) {                                           \
        return static_cast<E>(underlying_cast(x) | underlying_cast(y));        \
    }                                                                          \
                                                                               \
    inline E operator & (E x, E y) {                                           \
        return static_cast<E>(underlying_cast(x) & underlying_cast(y));        \
    }                                                                          \
                                                                               \
    inline E & operator |= (E & x, E y) {                                      \
        return reinterpret_cast<E&>(underlying_cast(x) |= underlying_cast(y)); \
    }                                                                          \
                                                                               \
    inline E & operator &= (E & x, E y) {                                      \
        return reinterpret_cast<E&>(underlying_cast(x) &= underlying_cast(y)); \
    }



enum class Level : unsigned { low, medium, high, NB };

MK_ENUM_IO(Level)

inline Level level_from_cstr(char const * value) {
    return
        0 == strcasecmp("medium", value) ? Level::medium
      : 0 == strcasecmp("high",value) ? Level::high
      : Level::low
    ;
}

struct LevelFieldTraits {
    static Level get_default() { return Level::low; }
    static bool valid(Level) { return true; }
    static Level cstr_to_enum(char const * cstr) { return level_from_cstr(cstr); }
    static char const * to_string(Level e) {
        switch (e) {
            case Level::medium: return "1";
            case Level::high: return "2";
            default: return "0";
        }
    }
};

using LevelField = EnumField<Level, LevelFieldTraits>;


enum class Language : unsigned { en, fr, NB };

MK_ENUM_IO(Language)

struct LanguageFieldTraits {
    static Language get_default() { return Language::NB; }
    static bool valid(Language lang) { return underlying_cast(lang) < underlying_cast(Language::NB); }
    static Language cstr_to_enum(char const * cstr) {
        if (0 == strcmp(cstr, "en")) {
            return Language::en;
        }
        if (0 == strcmp(cstr, "fr")) {
            return Language::fr;
        }
        return Language::NB;
    }
    static char const * to_string(Language lang) { return Language::fr == lang ? "fr" : "en"; }
};

using LanguageField = EnumField<Language, LanguageFieldTraits>;


enum class CaptureFlags : unsigned {
    none,
    png = 1 << 0,
    wrm = 1 << 1,
    flv = 1 << 2,
    ocr = 1 << 3,
    ocr2 = 1 << 4,
    FULL = ((1 << 5) - 1)
};
MK_ENUM_FLAG_FN(CaptureFlags)


enum class KeyboardLogFlags : unsigned {
    none,
    syslog = 1 << 0,
    wrm = 1 << 1,
    ocr = 1 << 2,
    FULL = ((1 << 3) - 1)
};
MK_ENUM_FLAG_FN(KeyboardLogFlags)

using KeyboardLogFlagsField = FlagsField<KeyboardLogFlags>;


enum class ClipboardLogFlags : unsigned {
    none,
    syslog = 1 << 0,
    FULL = ((1 << 1) - 1)
};
MK_ENUM_FLAG_FN(ClipboardLogFlags)

using ClipboardLogFlagsField = FlagsField<ClipboardLogFlags>;


enum class ColorDepth : unsigned {
    unspecified,
    depth8 = 8,
    depth15 = 15,
    depth16 = 16,
    depth24 = 24,
    //depth32,
};

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


#undef MK_ENUM_IO
#undef MK_ENUM_FLAG_FN


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

using configs::level_from_cstr;

#endif
