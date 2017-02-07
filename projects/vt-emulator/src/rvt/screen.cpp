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

#include "rvt/screen.hpp"

#include <algorithm>
#include <cassert>

namespace rvt
{

//FIXME: this is emulation specific. Use false for xterm, true for ANSI.
//FIXME: see if we can get this from terminfo.
const bool BS_CLEARS = false;

//Macro to convert x,y position on screen to position within an image.
//
//Originally the image was stored as one large contiguous block of
//memory, so a position within the image could be represented as an
//offset from the beginning of the block.  For efficiency reasons this
//is no longer the case.
//Many internal parts of this class still use this representation for parameters and so on,
//notably moveImage() and clearImage().
//This macro converts from an X,Y position into an image offset.

const Character Screen::DefaultChar = Character(' ',
                                      CharacterColor(ColorSpace::Default, DEFAULT_FORE_COLOR),
                                      CharacterColor(ColorSpace::Default, DEFAULT_BACK_COLOR),
                                      Rendition::Default,
                                      false);

Screen::Screen(int lines, int columns):
    _lines(lines),
    _columns(columns),
    _screenLines(_lines + 1),
    _screenLinesSize(_lines),
    _scrolledLines(0),
    _cuX(0),
    _cuY(0),
    _currentRendition(Rendition::Default),
    _topMargin(0),
    _bottomMargin(0),
    _effectiveForeground(CharacterColor()),
    _effectiveBackground(CharacterColor()),
    _effectiveRendition(Rendition::Default),
    _lastPos(-1)
{
    _lineProperties.resize(_lines + 1, LineProperty::Default);

    initTabStops();
    reset();
}

Screen::~Screen() = default;

void Screen::cursorUp(int n)
//=CUU
{
    if (n == 0) n = 1; // Default
    const int stop = _cuY < _topMargin ? 0 : _topMargin;
    _cuX = std::min(_columns - 1, _cuX); // nowrap!
    _cuY = std::max(stop, _cuY - n);
}

void Screen::cursorDown(int n)
//=CUD
{
    if (n == 0) n = 1; // Default
    const int stop = _cuY > _bottomMargin ? _lines - 1 : _bottomMargin;
    _cuX = std::min(_columns - 1, _cuX); // nowrap!
    _cuY = std::min(stop, _cuY + n);
}

void Screen::cursorLeft(int n)
//=CUB
{
    if (n == 0) n = 1; // Default
    _cuX = std::min(_columns - 1, _cuX); // nowrap!
    _cuX = std::max(0, _cuX - n);
}

void Screen::cursorRight(int n)
//=CUF
{
    if (n == 0) n = 1; // Default
    _cuX = std::min(_columns - 1, _cuX + n);
}

void Screen::setMargins(int top, int bot)
//=STBM
{
    if (top == 0) top = 1;      // Default
    if (bot == 0) bot = _lines;  // Default
    top = top - 1;              // Adjust to internal lineno
    bot = bot - 1;              // Adjust to internal lineno
    if (!(0 <= top && top < bot && bot < _lines)) {
        //Debug()<<" setRegion("<<top<<","<<bot<<") : bad range.";
        return;                   // Default error action: ignore
    }
    _topMargin = top;
    _bottomMargin = bot;
    _cuX = 0;
    _cuY = getMode(Mode::Origin) ? top : 0;
}

int Screen::topMargin() const
{
    return _topMargin;
}
int Screen::bottomMargin() const
{
    return _bottomMargin;
}

void Screen::index()
//=IND
{
    if (_cuY == _bottomMargin)
        scrollUp(1);
    else if (_cuY < _lines - 1)
        _cuY += 1;
}

void Screen::reverseIndex()
//=RI
{
    if (_cuY == _topMargin)
        scrollDown(_topMargin, 1);
    else if (_cuY > 0)
        _cuY -= 1;
}

void Screen::nextLine()
//=NEL
{
    toStartOfLine();
    index();
}

void Screen::eraseChars(int n)
{
    if (n == 0) n = 1; // Default
    const int p = std::max(0, std::min(_cuX + n - 1, _columns - 1));
    clearImage(loc(_cuX, _cuY), loc(p, _cuY), ' ');
}

void Screen::deleteChars(int n)
{
    assert(n >= 0);

    // always delete at least one char
    if (n == 0)
        n = 1;

    // if cursor is beyond the end of the line there is nothing to do
    if (_cuX >= int(_screenLines[_cuY].size()))
        return;

    if (_cuX + n > int(_screenLines[_cuY].size()))
        n = int(_screenLines[_cuY].size() - _cuX);

    assert(n >= 0);
    assert(_cuX + n <= int(_screenLines[_cuY].size()));

    auto pos = _screenLines[_cuY].begin() + _cuX;
    _screenLines[_cuY].erase(pos, pos + n);

    // Append space(s) with current attributes
    Character spaceWithCurrentAttrs(' ', _effectiveForeground,
                                    _effectiveBackground,
                                    _effectiveRendition, false);

    for (int i = 0; i < n; i++)
        _screenLines[_cuY].emplace_back(spaceWithCurrentAttrs);
}

void Screen::insertChars(int n)
{
    if (n == 0) n = 1; // Default

    if (int(_screenLines[_cuY].size()) < _cuX)
        _screenLines[_cuY].resize(_cuX);

    _screenLines[_cuY].insert(_screenLines[_cuY].begin() + _cuX, n, Character(' '));

    if (int(_screenLines[_cuY].size()) > _columns)
        _screenLines[_cuY].resize(_columns);
}

void Screen::deleteLines(int n)
{
    if (n == 0) n = 1; // Default
    scrollUp(_cuY, n);
}

void Screen::insertLines(int n)
{
    if (n == 0) n = 1; // Default
    scrollDown(_cuY, n);
}

void Screen::setMode(Mode m)
{
    _currentModes.set(m);
    switch (m) {
    case Mode::Origin:
        _cuX = 0;
        _cuY = _topMargin;
        break; //FIXME: home
    default:
        break;
    }
}

void Screen::resetMode(Mode m)
{
    _currentModes.reset(m);
    switch (m) {
    case Mode::Origin:
        _cuX = 0;
        _cuY = 0;
        break; //FIXME: home
    default:
        break;
    }
}

void Screen::saveMode(Mode m)
{
    _savedModes.copy_of(m, _currentModes);
}

void Screen::restoreMode(Mode m)
{
    _currentModes.copy_of(m, _savedModes);
}

bool Screen::getMode(Mode m) const
{
    return _currentModes.has(m);
}

void Screen::saveCursor()
{
    _savedState.cursorColumn = _cuX;
    _savedState.cursorLine  = _cuY;
    _savedState.rendition = _currentRendition;
    _savedState.foreground = _currentForeground;
    _savedState.background = _currentBackground;
}

void Screen::restoreCursor()
{
    _cuX     = std::min(_savedState.cursorColumn, _columns - 1);
    _cuY     = std::min(_savedState.cursorLine, _lines - 1);
    _currentRendition   = _savedState.rendition;
    _currentForeground   = _savedState.foreground;
    _currentBackground   = _savedState.background;
    updateEffectiveRendition();
}

void Screen::resizeImage(int new_lines, int new_columns)
{
    if ((new_lines == _lines) && (new_columns == _columns)) return;

    if (_cuY > new_lines - 1) {
        // attempt to preserve focus and _lines
        _bottomMargin = _lines - 1; //FIXME: margin lost
        for (int i = 0; i < _cuY - (new_lines - 1); i++) {
            scrollUp(0, 1);
        }
    }

    // create new screen _lines and copy from old to new

    auto newScreenLines = std::vector<ImageLine>(new_lines + 1);
    for (int i = 0; i < std::min(_lines, new_lines + 1) ; i++)
        newScreenLines[i] = _screenLines[i];
    for (int i = _lines; (i > 0) && (i < new_lines + 1); i++)
        newScreenLines[i].resize(new_columns); // TODO + max konsole_wcwidth - 1

    _lineProperties.resize(new_lines + 1, LineProperty::Default);

    _screenLines = std::move(newScreenLines);
    _screenLinesSize = new_lines;

    _lines = new_lines;
    _columns = new_columns;
    _cuX = std::min(_cuX, _columns - 1);
    _cuY = std::min(_cuY, _lines - 1);

    // FIXME: try to keep values, evtl.
    _topMargin = 0;
    _bottomMargin = _lines - 1;
    initTabStops();
}

void Screen::setDefaultMargins()
{
    _topMargin = 0;
    _bottomMargin = _lines - 1;
}

/*
   Clarifying rendition here and in the display.

   currently, the display's color table is
   0       1       2 .. 9    10 .. 17
   dft_fg, dft_bg, dim 0..7, intensive 0..7

   _currentForeground, _currentBackground contain values 0..8;
   - 0    = default color
   - 1..8 = ansi specified color

   re_fg, re_bg contain values 0..17
   due to the TerminalDisplay's color table

   rendition attributes are

   attr           widget screen
   -------------- ------ ------
   Rendition::Underline     XX     XX    affects foreground only
   Rendition::Blink         XX     XX    affects foreground only
   Rendition::Bold          XX     XX    affects foreground only
   Rendition::Reverse       --     XX
   RE_TRANSPARENT   XX     --    affects background only
   RE_INTENSIVE     XX     --    affects foreground only

   Note that Rendition::Bold is used in both widget
   and screen rendition. Since xterm/vt102
   is to poor to distinguish between bold
   (which is a font attribute) and intensive
   (which is a color attribute), we translate
   this and Rendition::Bold in falls eventually apart
   into Rendition::Bold and RE_INTENSIVE.
   */

void Screen::reverseRendition(Character& p) const
{
    CharacterColor f = p.foregroundColor;
    CharacterColor b = p.backgroundColor;

    p.foregroundColor = b;
    p.backgroundColor = f; //p->r &= ~RE_TRANSPARENT;
}

void Screen::updateEffectiveRendition()
{
    _effectiveRendition = _currentRendition;
    if (bool(_currentRendition & Rendition::Reverse)) {
        _effectiveForeground = _currentBackground;
        _effectiveBackground = _currentForeground;
    } else {
        _effectiveForeground = _currentForeground;
        _effectiveBackground = _currentBackground;
    }

    if (bool(_currentRendition & Rendition::Bold))
        _effectiveForeground.setIntensive();
}

void Screen::reset(bool clearScreen)
{
    setMode(Mode::Wrap);
    saveMode(Mode::Wrap);      // wrap at end of margin

    resetMode(Mode::Origin);
    saveMode(Mode::Origin);  // position refer to [1,1]

    resetMode(Mode::Insert);
    saveMode(Mode::Insert);  // overstroke

    setMode(Mode::Cursor);                         // cursor visible
    resetMode(Mode::Screen);                         // screen not inverse
    resetMode(Mode::NewLine);

    _topMargin = 0;
    _bottomMargin = _lines - 1;

    setDefaultRendition();
    saveCursor();

    if (clearScreen)
        clear();
}

void Screen::clear()
{
    clearEntireScreen();
    home();
}

void Screen::backspace()
{
    _cuX = std::min(_columns - 1, _cuX); // nowrap!
    _cuX = std::max(0, _cuX - 1);

    if (int(_screenLines[_cuY].size()) < _cuX + 1)
        _screenLines[_cuY].resize(_cuX + 1);

    if (BS_CLEARS) {
        _screenLines[_cuY][_cuX].character = ' ';
        _screenLines[_cuY][_cuX].rendition &= ~Rendition::ExtendedChar;
    }
}

void Screen::tab(int n)
{
    // note that TAB is a format effector (does not write ' ');
    if (n == 0) n = 1;
    while ((n > 0) && (_cuX < _columns - 1)) {
        cursorRight(1);
        while ((_cuX < _columns - 1) && !_tabStops[_cuX])
            cursorRight(1);
        n--;
    }
}

void Screen::backtab(int n)
{
    // note that TAB is a format effector (does not write ' ');
    if (n == 0) n = 1;
    while ((n > 0) && (_cuX > 0)) {
        cursorLeft(1);
        while ((_cuX > 0) && !_tabStops[_cuX]) {
            cursorLeft(1);
        }
        n--;
    }
}

void Screen::clearTabStops()
{
    for (int i = 0; i < _columns; i++)
        _tabStops[i] = false;
}

void Screen::changeTabStop(bool set)
{
    if (_cuX >= _columns)
        return;

    _tabStops[_cuX] = set;
}

void Screen::initTabStops()
{
    _tabStops.resize(_columns);

    // The 1st tabstop has to be one longer than the other.
    // i.e. the kids start counting from 0 instead of 1.
    // Other programs might behave correctly. Be aware.
    for (int i = 0; i < _columns; i++)
        _tabStops[i] = (i % 8 == 0 && i != 0);
}

void Screen::newLine()
{
    if (getMode(Mode::NewLine))
        toStartOfLine();

    index();
}


struct interval {
    unsigned long first;
    unsigned long last;
};

/* auxiliary function for binary search in interval table */
static bool bisearch(ucs4_char ucs, const interval * table, int max)
{
    int min = 0;
    int mid;

    if (ucs < table[0].first || ucs > table[max].last)
        return false;
    while (max >= min) {
        mid = (min + max) / 2;
        if (ucs > table[mid].last)
            min = mid + 1;
        else if (ucs < table[mid].first)
            max = mid - 1;
        else
            return true;
    }

    return false;
}


/* The following functions define the column width of an ISO 10646
 * character as follows:
 *
 *    - The null character (U+0000) has a column width of 0.
 *
 *    - Other C0/C1 control characters and DEL will lead to a return
 *      value of -1.
 *
 *    - Non-spacing and enclosing combining characters (general
 *      category code Mn or Me in the Unicode database) have a
 *      column width of 0.
 *
 *    - Other format characters (general category code Cf in the Unicode
 *      database) and ZERO WIDTH SPACE (U+200B) have a column width of 0.
 *
 *    - Hangul Jamo medial vowels and final consonants (U+1160-U+11FF)
 *      have a column width of 0.
 *
 *    - Spacing characters in the East Asian Wide (W) or East Asian
 *      FullWidth (F) category as defined in Unicode Technical
 *      Report #11 have a column width of 2.
 *
 *    - All remaining characters (including all printable
 *      ISO 8859-1 and WGL4 characters, Unicode control characters,
 *      etc.) have a column width of 1.
 *
 * This implementation assumes that quint16 characters are encoded
 * in ISO 10646.
 */
inline int konsole_wcwidth(ucs4_char ucs)
{
    /* sorted list of non-overlapping intervals of non-spacing characters */
    /* generated by "uniset +cat=Me +cat=Mn +cat=Cf -00AD +1160-11FF +200B c" */
    static const struct interval combining[] = {
        { 0x0300, 0x036F }, { 0x0483, 0x0486 }, { 0x0488, 0x0489 },
        { 0x0591, 0x05BD }, { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 },
        { 0x05C4, 0x05C5 }, { 0x05C7, 0x05C7 }, { 0x0600, 0x0603 },
        { 0x0610, 0x0615 }, { 0x064B, 0x065E }, { 0x0670, 0x0670 },
        { 0x06D6, 0x06E4 }, { 0x06E7, 0x06E8 }, { 0x06EA, 0x06ED },
        { 0x070F, 0x070F }, { 0x0711, 0x0711 }, { 0x0730, 0x074A },
        { 0x07A6, 0x07B0 }, { 0x07EB, 0x07F3 }, { 0x0901, 0x0902 },
        { 0x093C, 0x093C }, { 0x0941, 0x0948 }, { 0x094D, 0x094D },
        { 0x0951, 0x0954 }, { 0x0962, 0x0963 }, { 0x0981, 0x0981 },
        { 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 }, { 0x09CD, 0x09CD },
        { 0x09E2, 0x09E3 }, { 0x0A01, 0x0A02 }, { 0x0A3C, 0x0A3C },
        { 0x0A41, 0x0A42 }, { 0x0A47, 0x0A48 }, { 0x0A4B, 0x0A4D },
        { 0x0A70, 0x0A71 }, { 0x0A81, 0x0A82 }, { 0x0ABC, 0x0ABC },
        { 0x0AC1, 0x0AC5 }, { 0x0AC7, 0x0AC8 }, { 0x0ACD, 0x0ACD },
        { 0x0AE2, 0x0AE3 }, { 0x0B01, 0x0B01 }, { 0x0B3C, 0x0B3C },
        { 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B43 }, { 0x0B4D, 0x0B4D },
        { 0x0B56, 0x0B56 }, { 0x0B82, 0x0B82 }, { 0x0BC0, 0x0BC0 },
        { 0x0BCD, 0x0BCD }, { 0x0C3E, 0x0C40 }, { 0x0C46, 0x0C48 },
        { 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 }, { 0x0CBC, 0x0CBC },
        { 0x0CBF, 0x0CBF }, { 0x0CC6, 0x0CC6 }, { 0x0CCC, 0x0CCD },
        { 0x0CE2, 0x0CE3 }, { 0x0D41, 0x0D43 }, { 0x0D4D, 0x0D4D },
        { 0x0DCA, 0x0DCA }, { 0x0DD2, 0x0DD4 }, { 0x0DD6, 0x0DD6 },
        { 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A }, { 0x0E47, 0x0E4E },
        { 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 }, { 0x0EBB, 0x0EBC },
        { 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 }, { 0x0F35, 0x0F35 },
        { 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 }, { 0x0F71, 0x0F7E },
        { 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 }, { 0x0F90, 0x0F97 },
        { 0x0F99, 0x0FBC }, { 0x0FC6, 0x0FC6 }, { 0x102D, 0x1030 },
        { 0x1032, 0x1032 }, { 0x1036, 0x1037 }, { 0x1039, 0x1039 },
        { 0x1058, 0x1059 }, { 0x1160, 0x11FF }, { 0x135F, 0x135F },
        { 0x1712, 0x1714 }, { 0x1732, 0x1734 }, { 0x1752, 0x1753 },
        { 0x1772, 0x1773 }, { 0x17B4, 0x17B5 }, { 0x17B7, 0x17BD },
        { 0x17C6, 0x17C6 }, { 0x17C9, 0x17D3 }, { 0x17DD, 0x17DD },
        { 0x180B, 0x180D }, { 0x18A9, 0x18A9 }, { 0x1920, 0x1922 },
        { 0x1927, 0x1928 }, { 0x1932, 0x1932 }, { 0x1939, 0x193B },
        { 0x1A17, 0x1A18 }, { 0x1B00, 0x1B03 }, { 0x1B34, 0x1B34 },
        { 0x1B36, 0x1B3A }, { 0x1B3C, 0x1B3C }, { 0x1B42, 0x1B42 },
        { 0x1B6B, 0x1B73 }, { 0x1DC0, 0x1DCA }, { 0x1DFE, 0x1DFF },
        { 0x200B, 0x200F }, { 0x202A, 0x202E }, { 0x2060, 0x2063 },
        { 0x206A, 0x206F }, { 0x20D0, 0x20EF }, { 0x302A, 0x302F },
        { 0x3099, 0x309A }, { 0xA806, 0xA806 }, { 0xA80B, 0xA80B },
        { 0xA825, 0xA826 }, { 0xFB1E, 0xFB1E }, { 0xFE00, 0xFE0F },
        { 0xFE20, 0xFE23 }, { 0xFEFF, 0xFEFF }, { 0xFFF9, 0xFFFB },
        { 0x10A01, 0x10A03 }, { 0x10A05, 0x10A06 }, { 0x10A0C, 0x10A0F },
        { 0x10A38, 0x10A3A }, { 0x10A3F, 0x10A3F }, { 0x1D167, 0x1D169 },
        { 0x1D173, 0x1D182 }, { 0x1D185, 0x1D18B }, { 0x1D1AA, 0x1D1AD },
        { 0x1D242, 0x1D244 }, { 0xE0001, 0xE0001 }, { 0xE0020, 0xE007F },
        { 0xE0100, 0xE01EF }
    };

    /* test for 8-bit control characters */
    if (ucs == 0)
        return 0;
    if (ucs < 32 || (ucs >= 0x7f && ucs < 0xa0))
        return -1;

    /* binary search in table of non-spacing characters */
    if (bisearch(ucs, combining, sizeof(combining) / sizeof(struct interval) - 1))
        return 0;

    /* if we arrive here, ucs is not a combining or C0/C1 control character */

    return 1 +
           (ucs >= 0x1100 &&
            (ucs <= 0x115f ||                    /* Hangul Jamo init. consonants */
             ucs == 0x2329 || ucs == 0x232a ||
             (ucs >= 0x2e80 && ucs <= 0xa4cf &&
              ucs != 0x303f) ||                  /* CJK ... Yi */
             (ucs >= 0xac00 && ucs <= 0xd7a3) || /* Hangul Syllables */
             (ucs >= 0xf900 && ucs <= 0xfaff) || /* CJK Compatibility Ideographs */
             (ucs >= 0xfe10 && ucs <= 0xfe19) || /* Vertical forms */
             (ucs >= 0xfe30 && ucs <= 0xfe6f) || /* CJK Compatibility Forms */
             (ucs >= 0xff00 && ucs <= 0xff60) || /* Fullwidth Forms */
             (ucs >= 0xffe0 && ucs <= 0xffe6) ||
             (ucs >= 0x20000 && ucs <= 0x2fffd) ||
             (ucs >= 0x30000 && ucs <= 0x3fffd)));
}


void Screen::displayCharacter(ucs4_char c)
{
    // Note that VT100 does wrapping BEFORE putting the character.
    // This has impact on the assumption of valid cursor positions.
    // We indicate the fact that a newline has to be triggered by
    // putting the cursor one right to the last column of the screen.

    int w = konsole_wcwidth(c);
    if (w < 0)
        return;
    else if (w == 0) {
        // TODO if (QChar(c).category() != QChar::Mark_NonSpacing)
        if (c == ' ' || c == '\t') // TODO NonSpacing
            return;
        int charToCombineWithX = -1;
        int charToCombineWithY = -1;
        if (_cuX == 0) {
            // We are at the beginning of a line, check
            // if previous line has a character at the end we can combine with
            if (_cuY > 0 && _columns == int(_screenLines[_cuY - 1].size())) {
                charToCombineWithX = _columns - 1;
                charToCombineWithY = _cuY - 1;
            } else {
                // There is nothing to combine with
                // TODO Seems gnome-terminal shows the characters alone
                // might be worth investigating how to do that
                return;
            }
        } else {
            charToCombineWithX = _cuX - 1;
            charToCombineWithY = _cuY;
        }

        // Prevent "cat"ing binary files from causing crashes.
        if (charToCombineWithX >= int(_screenLines[charToCombineWithY].size())) {
            return;
        }

        Character & currentChar = _screenLines[charToCombineWithY][charToCombineWithX];
        _extendedCharTable.growChar(currentChar, c);
        return;
    }

    if (_cuX + w > _columns) {
        if (getMode(Mode::Wrap)) {
            _lineProperties[_cuY] |= LineProperty::Wrapped;
            nextLine();
        } else {
            _cuX = _columns - w;
        }
    }

    // ensure current line vector has enough elements
    if (int(_screenLines[_cuY].size()) < _cuX + w) {
        _screenLines[_cuY].resize(_cuX + w);
    }

    if (getMode(Mode::Insert)) insertChars(w);

    _lastPos = loc(_cuX, _cuY);

    // check if selection is still valid.
    //checkSelection(_lastPos, _lastPos);

    Character& currentChar = _screenLines[_cuY][_cuX];

    currentChar.character = c;
    currentChar.foregroundColor = _effectiveForeground;
    currentChar.backgroundColor = _effectiveBackground;
    currentChar.rendition = _effectiveRendition;
    currentChar.isRealCharacter = true;

    int i = 0;
    const int newCursorX = _cuX + w--;
    while (w) {
        i++;

        Character& ch = _screenLines[_cuY][_cuX + i];
        ch.character = 0;
        ch.foregroundColor = _effectiveForeground;
        ch.backgroundColor = _effectiveBackground;
        ch.rendition = _effectiveRendition;
        ch.isRealCharacter = false;

        w--;
    }
    _cuX = newCursorX;
}

int Screen::scrolledLines() const
{
    return _scrolledLines;
}
void Screen::resetScrolledLines()
{
    _scrolledLines = 0;
}

void Screen::scrollUp(int n)
{
    if (n == 0) n = 1; // Default
    scrollUp(_topMargin, n);
}

Rect Screen::lastScrolledRegion() const
{
    return _lastScrolledRegion;
}

void Screen::scrollUp(int from, int n)
{
    if (n <= 0 || from + n > _bottomMargin) return;

    _scrolledLines -= n;
    _lastScrolledRegion = Rect(
        0, static_cast<int16_t>(_topMargin),
        static_cast<uint16_t>(_columns - 1), static_cast<uint16_t>(_bottomMargin - _topMargin)
    );

    //FIXME: make sure `topMargin', `bottomMargin', `from', `n' is in bounds.
    moveImage(loc(0, from), loc(0, from + n), loc(_columns - 1, _bottomMargin));
    clearImage(loc(0, _bottomMargin - n + 1), loc(_columns - 1, _bottomMargin), ' ');
}

void Screen::scrollDown(int n)
{
    if (n == 0) n = 1; // Default
    scrollDown(_topMargin, n);
}

void Screen::scrollDown(int from, int n)
{
    _scrolledLines += n;

    //FIXME: make sure `topMargin', `bottomMargin', `from', `n' is in bounds.
    if (n <= 0)
        return;
    if (from > _bottomMargin)
        return;
    if (from + n > _bottomMargin)
        n = _bottomMargin - from;
    moveImage(loc(0, from + n), loc(0, from), loc(_columns - 1, _bottomMargin - n));
    clearImage(loc(0, from), loc(_columns - 1, from + n - 1), ' ');
}

void Screen::setCursorYX(int y, int x)
{
    setCursorY(y);
    setCursorX(x);
}

void Screen::setCursorX(int x)
{
    if (x == 0) x = 1; // Default
    x -= 1; // Adjust
    _cuX = std::max(0, std::min(_columns - 1, x));
}

void Screen::setCursorY(int y)
{
    if (y == 0) y = 1; // Default
    y -= 1; // Adjust
    _cuY = std::max(0, std::min(_lines  - 1, y + (getMode(Mode::Origin) ? _topMargin : 0)));
}

void Screen::home()
{
    _cuX = 0;
    _cuY = 0;
}

void Screen::toStartOfLine()
{
    _cuX = 0;
}

int Screen::getCursorX() const
{
    return _cuX;
}

int Screen::getCursorY() const
{
    return _cuY;
}

void Screen::clearImage(int loca, int loce, char c)
{
    //FIXME: check positions

    const int topLine = loca / _columns;
    const int bottomLine = loce / _columns;

    Character clearCh(c, _currentForeground, _currentBackground, Rendition::Default, false);

    //if the character being used to clear the area is the same as the
    //default character, the affected _lines can simply be shrunk.
    const bool isDefaultCh = (clearCh == Screen::DefaultChar);

    for (int y = topLine; y <= bottomLine; y++) {
        _lineProperties[y] = LineProperty::Default;

        const int endCol = (y == bottomLine) ? loce % _columns : _columns - 1;
        const int startCol = (y == topLine) ? loca % _columns : 0;

        std::vector<Character>& line = _screenLines[y];

        if (isDefaultCh && endCol == _columns - 1) {
            line.resize(startCol);
        } else {
            if (line.size() < std::size_t(endCol + 1))
                line.resize(endCol + 1);

            Character* data = line.data();
            for (int i = startCol; i <= endCol; i++)
                data[i] = clearCh;
        }
    }
}

void Screen::moveImage(int dest, int sourceBegin, int sourceEnd)
{
    assert(sourceBegin <= sourceEnd);

    const int lines = (sourceEnd - sourceBegin) / _columns;

    //move screen image and line properties:
    //the source and destination areas of the image may overlap,
    //so it matters that we do the copy in the right order -
    //forwards if dest < sourceBegin or backwards otherwise.
    //(search the web for 'memmove implementation' for details)
    if (dest < sourceBegin) {
        for (int i = 0; i <= lines; i++) {
            _screenLines[(dest / _columns) + i ] = _screenLines[(sourceBegin / _columns) + i ];
            _lineProperties[(dest / _columns) + i] = _lineProperties[(sourceBegin / _columns) + i];
        }
    } else {
        for (int i = lines; i >= 0; i--) {
            _screenLines[(dest / _columns) + i ] = _screenLines[(sourceBegin / _columns) + i ];
            _lineProperties[(dest / _columns) + i] = _lineProperties[(sourceBegin / _columns) + i];
        }
    }

    if (_lastPos != -1) {
        const int diff = dest - sourceBegin; // Scroll by this amount
        _lastPos += diff;
        if ((_lastPos < 0) || (_lastPos >= (lines * _columns)))
            _lastPos = -1;
    }
}

void Screen::clearToEndOfScreen()
{
    clearImage(loc(_cuX, _cuY), loc(_columns - 1, _lines - 1), ' ');
}

void Screen::clearToBeginOfScreen()
{
    clearImage(loc(0, 0), loc(_cuX, _cuY), ' ');
}

void Screen::clearEntireScreen()
{
    // Add entire screen to history
    for (int i = 0; i < (_lines - 1); i++) {
        scrollUp(0, 1);
    }

    clearImage(loc(0, 0), loc(_columns - 1, _lines - 1), ' ');
}

/*! fill screen with 'E'
  This is to aid screen alignment
  */

void Screen::helpAlign()
{
    clearImage(loc(0, 0), loc(_columns - 1, _lines - 1), 'E');
}

void Screen::clearToEndOfLine()
{
    clearImage(loc(_cuX, _cuY), loc(_columns - 1, _cuY), ' ');
}

void Screen::clearToBeginOfLine()
{
    clearImage(loc(0, _cuY), loc(_cuX, _cuY), ' ');
}

void Screen::clearEntireLine()
{
    clearImage(loc(0, _cuY), loc(_columns - 1, _cuY), ' ');
}

void Screen::setRendition(Rendition rendention)
{
    _currentRendition |= rendention;
    updateEffectiveRendition();
}

void Screen::resetRendition(Rendition rendention)
{
    _currentRendition &= ~rendention;
    updateEffectiveRendition();
}

void Screen::setDefaultRendition()
{
    setForeColor(ColorSpace::Default, DEFAULT_FORE_COLOR);
    setBackColor(ColorSpace::Default, DEFAULT_BACK_COLOR);
    _currentRendition   = Rendition::Default;
    updateEffectiveRendition();
}

void Screen::setForeColor(ColorSpace space, int color)
{
    _currentForeground = CharacterColor(space, color);

    if (_currentForeground.isValid())
        updateEffectiveRendition();
    else
        setForeColor(ColorSpace::Default, DEFAULT_FORE_COLOR);
}

void Screen::setBackColor(ColorSpace space, int color)
{
    _currentBackground = CharacterColor(space, color);

    if (_currentBackground.isValid())
        updateEffectiveRendition();
    else
        setBackColor(ColorSpace::Default, DEFAULT_BACK_COLOR);
}

void Screen::setLineProperty(LineProperty property , bool enable)
{
    if (enable)
        _lineProperties[_cuY] |= property;
    else
        _lineProperties[_cuY] &= ~property;
}
void Screen::fillWithDefaultChar(Character* dest, int count)
{
    for (int i = 0; i < count; i++)
        dest[i] = Screen::DefaultChar;
}

}
