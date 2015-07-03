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
#include "config_types/authid.hpp"

#include <cstddef>
#include <type_traits>

namespace config_types {

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
    StaticStringBase(typename std::conditional<NullableString, null_fill, disable_ctor>::type) noexcept
    : str{}
    {}

    StaticStringBase() noexcept
    { this->str[0] = 0; }

    StaticStringBase(const char * s)
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
    StaticKeyString(const char * s) {
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

    Range(T x = Default)
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

enum class Level : unsigned { low, medium, high, NB };

/// Field which contains an Level type
class LevelField : public BaseField {
    Level data;
    char buff[20];
public:
    LevelField() : data(Level::low) {
    }

    void set(Level that) {
        if (this->data != that || this->asked) {
            this->modify();
            this->data = that;
        }
        this->asked = false;
    }

    virtual void set_from_acl(const char * cstr) {
        this->modify_from_acl();
        this->data = static_cast<Level>(ulong_from_cstr(cstr));
        this->asked = false;
    }
    virtual void set_from_cstr(const char * cstr) {
        this->set(static_cast<Level>(ulong_from_cstr(cstr)));
    }

    Level get() const {
        return this->data;
    }

    virtual const char * get_value() {
        if (this->is_asked()) {
            return "ASK";
        }
        snprintf(buff, sizeof(buff), "%u", this->data);
        return buff;
    }
};

enum class Language : unsigned { en, fr, NB };

/// Field which contains a Language type
class LanguageField : public BaseField {
protected:
    Language data;
public:
    LanguageField() : data(Language::NB) {
    }

    void set(Language lang) {
        if (static_cast<unsigned>(lang) < static_cast<unsigned>(Language::NB)) {
            return ;
        }
        if (this->asked || this->data != lang) {
            this->modify();
            this->data = lang;
        }
        this->asked = false;
    }

    virtual void set_from_acl(const char * cstr) {
        Language lang = this->to_language(cstr);
        if (lang == Language::NB) {
            return ;
        }
        this->modify_from_acl();
        this->data = lang;
        this->asked = false;
    }
    virtual void set_from_cstr(const char * cstr) {
        Language lang = this->to_language(cstr);
        if (lang == Language::NB) {
            return ;
        }
        if (this->asked || this->data != lang) {
            this->modify();
            this->data = lang;
        }
        this->asked = false;
    }

    Language get() const {
        return static_cast<unsigned>(this->data) < static_cast<unsigned>(Language::NB)
            ? this->data
            : Language::en;
    }

    virtual const char * get_value() {
        if (this->is_asked()) {
            return "ASK";
        }
        return Language::fr == this->data ? "fr" : "en";
    }

private:
    Language to_language(char const * cstr) const {
        if (0 == strcmp(cstr, "en")) {
            return Language::en;
        }
        if (0 == strcmp(cstr, "fr")) {
            return Language::fr;
        }
        Language::NB;
    }
};

enum class CaptureFlags : unsigned {
    none,
    png = 1 << 1,
    wrm = 1 << 2,
    flv = 1 << 3,
    ocr = 1 << 4,
    ocr2 = 1 << 5,
    FULL = ((1 << 6) - 1)
};

CaptureFlags operator | (CaptureFlags x, CaptureFlags y) {
    return static_cast<CaptureFlags>(static_cast<unsigned>(x) | static_cast<unsigned>(y));
}

CaptureFlags operator & (CaptureFlags x, CaptureFlags y) {
    return static_cast<CaptureFlags>(static_cast<unsigned>(x) & static_cast<unsigned>(y));
}

enum class KeyboardLogFlags : unsigned {
    none,
    syslog = 1 << 1,
    wrm = 1 << 2,
    ocr = 1 << 3,
    FULL = ((1 << 4) - 1)
};

KeyboardLogFlags operator | (KeyboardLogFlags x, KeyboardLogFlags y) {
    return static_cast<KeyboardLogFlags>(static_cast<unsigned>(x) | static_cast<unsigned>(y));
}

KeyboardLogFlags operator & (KeyboardLogFlags x, KeyboardLogFlags y) {
    return static_cast<KeyboardLogFlags>(static_cast<unsigned>(x) & static_cast<unsigned>(y));
}

/// Field which contains an KeyboardLogFlags type
class KeyboardLogFlagsField : public BaseField {
    KeyboardLogFlags data;
    char buff[20];
public:
    KeyboardLogFlagsField() : data(KeyboardLogFlags::none) {
    }

    void set(KeyboardLogFlags that) {
        if (this->data != that || this->asked) {
            this->modify();
            this->data = that;
        }
        this->asked = false;
    }

    virtual void set_from_acl(const char * cstr) {
        this->modify_from_acl();
        this->data = static_cast<KeyboardLogFlags>(ulong_from_cstr(cstr)) & KeyboardLogFlags::FULL;
        this->asked = false;
    }
    virtual void set_from_cstr(const char * cstr) {
        this->set(static_cast<KeyboardLogFlags>(ulong_from_cstr(cstr)) & KeyboardLogFlags::FULL);
    }

    KeyboardLogFlags get() const {
        return this->data;
    }

    virtual const char * get_value() {
        if (this->is_asked()) {
            return "ASK";
        }
        snprintf(buff, sizeof(buff), "%u", static_cast<unsigned>(this->data));
        return buff;
    }
};

enum class ColorDepth { depth8, depth15, depth16, depth24/*, depth32*/, NB };

}

#endif
