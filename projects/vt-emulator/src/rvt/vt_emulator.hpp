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

#include <array>
#include <functional> // std::function

#include "rvt/charsets.hpp"
#include "rvt/screen.hpp"

//#define CXX_UNUSED(x) (void)x // [[maybe_unused]]

// #define REDEMPTION_DIAGNOSTIC_IGNORE_CONVERSION REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wconversion")

namespace rvt
{

struct CharCodes
{
    std::array<CharsetId, 4> charset; // coding info
    CharsetId charset_id = CharsetId::Undefined; // actual charset.
    CharsetId sa_charset_id = CharsetId::Undefined; // saved charset.
};


/**
 * Provides an xterm compatible terminal emulation based on the DEC VT102 terminal.
 * A full description of this terminal can be found at http://vt100.net/docs/vt102-ug/
 */
class VtEmulator
{
    enum Mode : uint16_t
    {
        AppScreen       , // Mode #1
        Ansi            , // Use US Ascii for character sets G0-G3 (DECANM)
        Columns132      , // 80 <-> 132 column mode switch (DECCOLM)
        AllowColumns132 , // Allow DECCOLM mode
    };

public:
    /** Constructs a new emulation */
    VtEmulator(int lines, int columns);
    ~VtEmulator();

    // reimplemented from Emulation
    void clearEntireScreen();
    void reset();

    Screen const & getCurrentScreen() const { return *_currentScreen; }
    array_view<ucs4_char const> getWindowTitle() const { return {windowTitle, windowTitleLen}; }

    void setWindowTitle(ucs4_carray_view title);
    void setLogFunction(std::function<void(char const *)> f)
    { this->_logFunction = std::move(f); }

    void receiveChar(ucs4_char cc);
    void setScreenSize(int lines, int columns);

private:
    // reimplemented from Emulation
    void setMode(Mode mode);
    void resetMode(Mode mode);

    using ScreenMode = Screen::Mode;

    void setMode(ScreenMode mode);
    void resetMode(ScreenMode mode);

    ucs4_char applyCharset(ucs4_char  c) const;
    void setCharset(int n, CharsetId cs);
    void useCharset(int n);
    void setAndUseCharset(int n, CharsetId cs);
    void saveCursor();
    void restoreCursor();
    void resetCharset();

    void setScreen(int n);

    void setMargins(int top, int bottom);
    //set margins for all screens back to their defaults
    void setDefaultMargins();

    // returns true if 'mode' is set or false otherwise
    bool getMode(Mode mode);
    // saves the current boolean value of 'mode'
    void saveMode(Mode mode);
    // restores the boolean value of 'mode'
    void restoreMode(Mode mode);
    // returns true if 'mode' is set or false otherwise
    bool getMode(ScreenMode mode);
    // saves the current boolean value of 'mode'
    void saveMode(ScreenMode mode);
    // restores the boolean value of 'mode'
    void restoreMode(ScreenMode mode);
    // resets all modes
    // (except Mode::AllowColumns132)
    void resetModes();

    void resetTokenizer();
    void addToCurrentToken(ucs4_char cc);
    void processWindowAttributeRequest();
    static constexpr int MAX_TOKEN_LENGTH = 256; // Max length of tokens (e.g. window title)
    ucs4_char tokenBuffer[MAX_TOKEN_LENGTH];
    int tokenBufferPos;
    ucs4_char windowTitle[MAX_TOKEN_LENGTH];
    unsigned windowTitleLen = 0;

    static constexpr int MAXARGS = 15;
    void addDigit(int dig);
    void addArgument();
    int argv[MAXARGS];
    int argc;
    void initTokenizer();

    // Set of flags for each of the ASCII characters which indicates
    // what category they fall into (printable character, control, digit etc.)
    // for the purposes of decoding terminal output
    int charClass[256];

    void reportDecodingError();

    void processToken(int code, int32_t p, int q);

    // clears the screen and resizes it to the specified
    // number of columns
    void clearScreenAndSetColumns(int columnCount);

    CharCodes _charsets[2];

    using ModeFlags = Flags<Mode>;

    ModeFlags _currentModes;
    ModeFlags _savedModes;

    Screen _screen0;
    Screen _screen1;
    Screen * _currentScreen = &_screen1;

    std::function<void(char const *)> _logFunction;
};

}
