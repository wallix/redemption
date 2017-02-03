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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen;
*
*   Based on Konsole, an X terminal
*/

#pragma once

#include "rvt/character_color.hpp"

#include "cxx/attributes.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/enum_flags_operators.hpp"

#include <array>
#include <vector>
#include <memory>

#include <cstdint>
#include <cstring> // memcpy


namespace rvt
{

enum class Rendition : uint8_t
{
    Default       = 0,
    Bold          = (1 << 0),
    Blink         = (1 << 1),
    Underline     = (1 << 2),
    Reverse       = (1 << 3),
    Italic        = (1 << 5),
    Cursor        = (1 << 6),
    ExtendedChar  = (1 << 7),
};

}

template<> struct is_enum_flags<rvt::Rendition> : std::true_type {};


namespace rvt
{

using uc_t = uint32_t;


struct ExtendedCharacter
{
    array_view<uc_t const> as_array() const noexcept
    { return {this->chars.get(), this->len}; }

    void append(uc_t uc);

    uint16_t len;
    uint16_t capacity;
    std::unique_ptr<uc_t[]> chars;
};


/**
 * A single character in the terminal which consists of a unicode character
 * value, foreground and background colors and a set of rendition attributes
 * which specify how it should be drawn.
 */
class Character
{
public:
    /**
     * Constructs a new character.
     *
     * @param _c The unicode character value of this character.
     * @param _f The foreground color used to draw the character.
     * @param _b The color used to draw the character's background.
     * @param _r A set of rendition flags which specify how this character
     *           is to be drawn.
     * @param _real Indicate whether this character really exists, or exists
     *              simply as place holder.
     */
    explicit inline Character(uc_t _c = ' ',
                              CharacterColor  _f = CharacterColor(ColorSpace::Default, DEFAULT_FORE_COLOR),
                              CharacterColor  _b = CharacterColor(ColorSpace::Default, DEFAULT_BACK_COLOR),
                              Rendition  _r = Rendition::Default,
                              bool _real = true)
    : character(_c)
    , rendition(_r)
    , foregroundColor(_f)
    , backgroundColor(_b)
    , isRealCharacter(_real)
    { }

    inline bool is_extended() const noexcept
    { return bool(this->rendition & Rendition::ExtendedChar); }

    /** The unicode character value for this character.
     *
     * if Rendition::ExtendedChar is set, character is unicode sequence point.
     */
    uc_t character;

    /** A combination of RENDITION flags which specify options for drawing the character. */
    Rendition rendition;

    /** The foreground color used to draw this character. */
    CharacterColor  foregroundColor;

    /** The color used to draw this character's background. */
    CharacterColor  backgroundColor;

    /** Indicate whether this character really exists, or exists simply as place holder.
     *
     *  TODO: this boolean filed can be further improved to become a enum filed, which
     *  indicates different roles:
     *
     *    RealCharacter: a character which really exists
     *    PlaceHolderCharacter: a character which exists as place holder
     *    TabStopCharacter: a special place holder for HT("\t")
     */
    bool isRealCharacter;

    /**
     * returns true if the format (color, rendition flag) of the compared characters is equal
     */
    bool equalsFormat(const Character& other) const;

    /**
     * Compares two characters and returns true if they have the same unicode character value,
     * rendition and colors.
     */
    friend bool operator == (const Character& a, const Character& b);

    /**
     * Compares two characters and returns true if they have different unicode character values,
     * renditions or colors.
     */
    friend bool operator != (const Character& a, const Character& b);
};

/**
 * A table which stores sequences of unicode characters, referenced
 * by hash keys.  The hash key itself is the same size as a unicode
 * character ( ushort ) so that it can occupy the same space in
 * a structure.
 */
class ExtendedCharTable
{
public:
    ExtendedCharTable() = default;

    REDEMPTION_CXX_NODISCARD
    bool growChar(Character & character, uc_t uc);

    void clear();

    inline array_view<uc_t const> operator[](std::size_t i) const noexcept
    { return this->extendedCharTable[i].as_array(); }

private:
    std::vector<ExtendedCharacter> extendedCharTable;
};

inline bool operator == (const Character& a, const Character& b)
{
    return a.character == b.character && a.equalsFormat(b);
}

inline bool operator != (const Character& a, const Character& b)
{
    return !operator==(a, b);
}

inline bool Character::equalsFormat(const Character& other) const
{
    return backgroundColor == other.backgroundColor &&
           foregroundColor == other.foregroundColor &&
           rendition == other.rendition;
}


inline void ExtendedCharacter::append(uc_t uc)
{
    if (this->len == this->capacity) {
        if (this->len != (1u << (8 * sizeof(this->len) - 1))) {
            std::unique_ptr<uc_t[]> u(new uc_t[this->capacity * 2u]);
            memcpy(u.get(), this->chars.get(), this->len);
            this->chars = std::move(u);
            this->capacity *= 2u;
            this->chars[this->len] = uc;
            this->len++;
        }
    }
    else {
        this->chars[this->len] = uc;
        this->len++;
    }
}


inline REDEMPTION_CXX_NODISCARD bool ExtendedCharTable::growChar(Character & character, uc_t uc)
{
    if (character.is_extended()) {
        this->extendedCharTable[character.character].append(uc);
    }
    else {
        uint16_t capacity = 4;
        this->extendedCharTable.emplace_back(ExtendedCharacter{
            2, capacity, std::unique_ptr<uc_t[]>{new uc_t[capacity]{character.character, uc}}
        });
        character.character = this->extendedCharTable.size() - 1;
        character.rendition |= Rendition::ExtendedChar;
    }
    return true;
}

inline void ExtendedCharTable::clear()
{
    this->extendedCharTable.clear();
}

}
