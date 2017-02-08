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
#include <cstdint>
#include <type_traits>
#include <memory>

#include <vector>
#include <string>
#include <algorithm>

#include <cassert>
#include <cstring> // memset

#include "utils/rect.hpp"
#include "utils/sugar/make_unique.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/underlying_cast.hpp"
#include "utils/sugar/enum_flags_operators.hpp"
#include "cxx/diagnostic.hpp"

#include "rvt/character.hpp"
#include "rvt/charsets.hpp"
#include "rvt/screen.hpp"
#include "rvt/utf8_decoder.hpp"

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
class Vt102Emulation
{
    enum Mode : uint16_t
    {
        AppScreen       , // Mode #1
        Ansi            , // Use US Ascii for character sets G0-G3 (DECANM)
        Columns132      , // 80 <-> 132 column mode switch (DECCOLM)
        AllowColumns132 , // Allow DECCOLM mode
        COUNT_          ,
    };

public:
    /** Constructs a new emulation */
    Vt102Emulation(int lines, int columns);
    ~Vt102Emulation();

    // reimplemented from Emulation
    void clearEntireScreen();
    void reset();

    Screen const & getScreen() const { return *_currentScreen; }

protected:
    // reimplemented from Emulation
    void setMode(Mode mode);
    void resetMode(Mode mode);

    using ScreenMode = Screen::Mode;

    void setMode(ScreenMode mode);
    void resetMode(ScreenMode mode);

public: // TODO protected
    void receiveChar(ucs4_char cc);

private:
    ucs4_char applyCharset(ucs4_char  c) const;
    void setCharset(int n, CharsetId cs);
    void useCharset(int n);
    void setAndUseCharset(int n, CharsetId cs);
    void saveCursor();
    void restoreCursor();
    void resetCharset();

    void setScreen(int n);
    void setScreenSize(int lines, int columns);

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
#define MAX_TOKEN_LENGTH 256 // Max length of tokens (e.g. window title)
    void addToCurrentToken(ucs4_char cc);
    int tokenBuffer[MAX_TOKEN_LENGTH]; //FIXME: overflow?
    int tokenBufferPos;
#define MAXARGS 15
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

    Screen _screens[2];
    Screen * _currentScreen = &_screens[0];
};

Vt102Emulation::Vt102Emulation(int lines, int columns)
: _screens{{lines, columns}, {lines, columns}}
{
    initTokenizer();
    reset();
}

Vt102Emulation::~Vt102Emulation() = default;

void Vt102Emulation::clearEntireScreen()
{
    _currentScreen->clearEntireScreen();
    // bufferedUpdate();
}

void Vt102Emulation::reset()
{
    // Save the current codec so we can set it later.
    // Ideally we would want to use the profile setting
//     const QTextCodec* currentCodec = codec();

    resetTokenizer();
    resetModes();
    resetCharset();
    _currentScreen->reset();

//     if (currentCodec)
//         setCodec(currentCodec);
//     else
//         setCodec(LocaleCodec);
//
//     bufferedUpdate();
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                     Processing the incoming byte stream                   */
/*                                                                           */
/* ------------------------------------------------------------------------- */

/* Incoming Bytes Event pipeline

   This section deals with decoding the incoming character stream.
   Decoding means here, that the stream is first separated into `tokens'
   which are then mapped to a `meaning' provided as operations by the
   `Screen' class or by the emulation class itself.

   The pipeline proceeds as follows:

   - Tokenizing the ESC codes (onReceiveChar)
   - VT100 code page translation of plain characters (applyCharset)
   - Interpretation of ESC codes (processToken)

   The escape codes and their meaning are described in the
   technical reference of this program.
*/

// Tokens ------------------------------------------------------------------ --

/*
   Since the tokens are the central notion if this section, we've put them
   in front. They provide the syntactical elements used to represent the
   terminals operations as byte sequences.

   They are encodes here into a single machine word, so that we can later
   switch over them easily. Depending on the token itself, additional
   argument variables are filled with parameter values.

   The tokens are defined below:

   - CHR        - Printable characters     (32..255 but DEL (=127))
   - CTL        - Control characters       (0..31 but ESC (= 27), DEL)
   - ESC        - Escape codes of the form <ESC><CHR but `[]()+*#'>
   - ESC_DE     - Escape codes of the form <ESC><any of `()+*#%'> C
   - CSI_PN     - Escape codes of the form <ESC>'['     {Pn} ';' {Pn} C
   - CSI_PS     - Escape codes of the form <ESC>'['     {Pn} ';' ...  C
   - CSI_PR     - Escape codes of the form <ESC>'[' '?' {Pn} ';' ...  C
   - CSI_PE     - Escape codes of the form <ESC>'[' '!' {Pn} ';' ...  C
   - VT52       - VT52 escape codes
                  - <ESC><Chr>
                  - <ESC>'Y'{Pc}{Pc}
   - XTE_HA     - Xterm window/terminal attribute commands
                  of the form <ESC>`]' {Pn} `;' {Text} <BEL>
                  (Note that these are handled differently to the other formats)

   The last two forms allow list of arguments. Since the elements of
   the lists are treated individually the same way, they are passed
   as individual tokens to the interpretation. Further, because the
   meaning of the parameters are names (although represented as numbers),
   they are includes within the token ('N').

*/

#define TY_CONSTRUCT(T,A,N) ( \
    ((static_cast<int>(N) & 0xffff) << 16) | \
    ((static_cast<int>(A) & 0xff) << 8) | \
    ( static_cast<int>(T) & 0xff) )

#define TY_CHR(   )     TY_CONSTRUCT(0,0,0)
#define TY_CTL(A  )     TY_CONSTRUCT(1,A,0)
#define TY_ESC(A  )     TY_CONSTRUCT(2,A,0)
#define TY_ESC_CS(A,B)  TY_CONSTRUCT(3,A,B)
#define TY_ESC_DE(A  )  TY_CONSTRUCT(4,A,0)
#define TY_CSI_PS(A,N)  TY_CONSTRUCT(5,A,N)
#define TY_CSI_PN(A  )  TY_CONSTRUCT(6,A,0)
#define TY_CSI_PR(A,N)  TY_CONSTRUCT(7,A,N)

#define TY_VT52(A)    TY_CONSTRUCT(8,A,0)
#define TY_CSI_PG(A)  TY_CONSTRUCT(9,A,0)
#define TY_CSI_PE(A)  TY_CONSTRUCT(10,A,0)

const int MAX_ARGUMENT = 4096;

// Tokenizer --------------------------------------------------------------- --

/* The tokenizer's state

   The state is represented by the buffer (tokenBuffer, tokenBufferPos),
   and accompanied by decoded arguments kept in (argv,argc).
   Note that they are kept internal in the tokenizer.
*/

void Vt102Emulation::resetTokenizer()
{
    tokenBufferPos = 0;
    argc = 0;
    argv[0] = 0;
    argv[1] = 0;
}

void Vt102Emulation::addDigit(int digit)
{
    if (argv[argc] < MAX_ARGUMENT)
        argv[argc] = 10 * argv[argc] + digit;
}

void Vt102Emulation::addArgument()
{
    argc = std::min(argc + 1, MAXARGS - 1);
    argv[argc] = 0;
}

void Vt102Emulation::addToCurrentToken(ucs4_char cc)
{
    tokenBuffer[tokenBufferPos] = cc;
    tokenBufferPos = std::min(tokenBufferPos + 1, MAX_TOKEN_LENGTH - 1);
}

// Character Class flags used while decoding
const int CTL =  1;  // Control character
const int CHR =  2;  // Printable character
const int CPN =  4;  // TODO: Document me
const int DIG =  8;  // Digit
const int SCS = 16;  // Select Character Set
const int GRP = 32;  // TODO: Document me
const int CPS = 64;  // Character which indicates end of window resize

void Vt102Emulation::initTokenizer()
{
    auto as_bytes = [](char const * s) { return reinterpret_cast<uint8_t const *>(s); };

    for (int i = 0; i < 256; ++i)
        charClass[i] = 0;
    for (int i = 0; i < 32; ++i)
        charClass[i] |= CTL;
    for (int i = 32; i < 256; ++i)
        charClass[i] |= CHR;
    for (auto s = as_bytes("@ABCDGHILMPSTXZcdfry"); *s; ++s)
        charClass[*s] |= CPN;
    // resize = \e[8;<row>;<col>t
    for (auto s = as_bytes("t"); *s; ++s)
        charClass[*s] |= CPS;
    for (auto s = as_bytes("0123456789"); *s; ++s)
        charClass[*s] |= DIG;
    for (auto s = as_bytes("()+*%"); *s; ++s)
        charClass[*s] |= SCS;
    for (auto s = as_bytes("()+*#[]%"); *s; ++s)
        charClass[*s] |= GRP;
}

/* Ok, here comes the nasty part of the decoder.

   Instead of keeping an explicit state, we deduce it from the
   token scanned so far. It is then immediately combined with
   the current character to form a scanning decision.

   This is done by the following defines.

   - P is the length of the token scanned so far.
   - L (often P-1) is the position on which contents we base a decision.
   - C is a character or a group of characters (taken from 'charClass').

   - 'cc' is the current character
   - 's' is a pointer to the start of the token buffer
   - 'p' is the current position within the token buffer

   Note that they need to applied in proper order.
*/

#define lec(P,L,C) (p == (P) && s[(L)] == (C))
#define lun(     ) (p ==  1  && cc >= 32 )
#define les(P,L,C) (p == (P) && s[L] < 256 && (charClass[s[(L)]] & (C)) == (C))
#define eec(C)     (p >=  3  && cc == (C))
#define ees(C)     (p >=  3  && cc < 256 && (charClass[cc] & (C)) == (C))
#define eps(C)     (p >=  3  && s[2] != '?' && s[2] != '!' && s[2] != '>' && cc < 256 && (charClass[cc] & (C)) == (C))
#define epp( )     (p >=  3  && s[2] == '?')
#define epe( )     (p >=  3  && s[2] == '!')
#define egt( )     (p >=  3  && s[2] == '>')
#define Xpe        (tokenBufferPos >= 2 && tokenBuffer[1] == ']')
#define Xte        (Xpe      && cc ==  7 )
#define ces(C)     (cc < 256 && (charClass[cc] & (C)) == (C) && !Xte)

#define CNTL(c) ((c)-'@')
const int ESC = 27;
const int DEL = 127;

// process an incoming unicode character
void Vt102Emulation::receiveChar(ucs4_char cc)
{
  if (cc == DEL)
    return; //VT100: ignore.

  if (ces(CTL))
  {
    // DEC HACK ALERT! Control Characters are allowed *within* esc sequences in VT100
    // This means, they do neither a resetTokenizer() nor a pushToToken(). Some of them, do
    // of course. Guess this originates from a weakly layered handling of the X-on
    // X-off protocol, which comes really below this level.
    if (cc == CNTL('X') || cc == CNTL('Z') || cc == ESC)
        resetTokenizer(); //VT100: CAN or SUB
    if (cc != ESC)
    {
        processToken(TY_CTL(cc+'@' ),0,0);
        return;
    }
  }
  // advance the state
  addToCurrentToken(cc);

  int * s = tokenBuffer;
  const int  p = tokenBufferPos;

  if (getMode(Mode::Ansi))
  {
    if (lec(1,0,ESC)) { return; }
    if (lec(1,0,ESC+128)) { s[0] = ESC; receiveChar('['); return; }
    if (les(2,1,GRP)) { return; }
    if (Xte         ) { /* processWindowAttributeRequest();*/ resetTokenizer(); return; }
    if (Xpe         ) { return; }
    if (lec(3,2,'?')) { return; }
    if (lec(3,2,'>')) { return; }
    if (lec(3,2,'!')) { return; }
    if (lun(       )) { processToken( TY_CHR(), applyCharset(cc), 0);   resetTokenizer(); return; }
    if (lec(2,0,ESC)) { processToken( TY_ESC(s[1]), 0, 0);              resetTokenizer(); return; }
    if (les(3,1,SCS)) { processToken( TY_ESC_CS(s[1],s[2]), 0, 0);      resetTokenizer(); return; }
    if (lec(3,1,'#')) { processToken( TY_ESC_DE(s[2]), 0, 0);           resetTokenizer(); return; }
    if (eps(    CPN)) { processToken( TY_CSI_PN(cc), argv[0], argv[1]);  resetTokenizer(); return; }

    // resize = \e[8;<row>;<col>t
    if (eps(CPS))
    {
        processToken( TY_CSI_PS(cc, argv[0]), argv[1], argv[2]);
        resetTokenizer();
        return;
    }

    if (epe(   )) { processToken( TY_CSI_PE(cc), 0, 0); resetTokenizer(); return; }
    if (ees(DIG)) { addDigit(cc-'0'); return; }
    if (eec(';')) { addArgument();    return; }
    for (int i = 0; i <= argc; i++)
    {
        if (epp())
            processToken(TY_CSI_PR(cc,argv[i]), 0, 0);
        else if (egt())
            processToken(TY_CSI_PG(cc), 0, 0); // spec. case for ESC]>0c or ESC]>c
        else if (cc == 'm' && argc - i >= 4 && (argv[i] == 38 || argv[i] == 48) && argv[i+1] == 2)
        {
            // ESC[ ... 48;2;<red>;<green>;<blue> ... m -or- ESC[ ... 38;2;<red>;<green>;<blue> ... m
            i += 2;
            processToken(TY_CSI_PS(cc, argv[i-2]), static_cast<int>(ColorSpace::RGB), (argv[i] << 16) | (argv[i+1] << 8) | argv[i+2]);
            i += 2;
        }
        else if (cc == 'm' && argc - i >= 2 && (argv[i] == 38 || argv[i] == 48) && argv[i+1] == 5)
        {
            // ESC[ ... 48;5;<index> ... m -or- ESC[ ... 38;5;<index> ... m
            i += 2;
            processToken(TY_CSI_PS(cc, argv[i-2]), static_cast<int>(ColorSpace::Index256), argv[i]);
        }
        else
            processToken(TY_CSI_PS(cc,argv[i]), 0, 0);
    }
    resetTokenizer();
  }
  else
  {
    // VT52 Mode
    if (lec(1,0,ESC))
        return;
    if (les(1,0,CHR))
    {
        processToken( TY_CHR(), s[0], 0);
        resetTokenizer();
        return;
    }
    if (lec(2,1,'Y'))
        return;
    if (lec(3,1,'Y'))
        return;
    if (p < 4)
    {
        processToken(TY_VT52(s[1] ), 0, 0);
        resetTokenizer();
        return;
    }
    processToken(TY_VT52(s[1]), s[2], s[3]);
    resetTokenizer();
    return;
  }
}

// Interpreting Codes ---------------------------------------------------------

constexpr inline CharsetId char_to_charset_id(char c)
{
    return('0' == c) ? CharsetId::VT100Graphics
        : ('A' == c) ? CharsetId::IBMPC
        : ('B' == c) ? CharsetId::Latin1
        : ('U' == c) ? CharsetId::IBMPC
        : ('K' == c) ? CharsetId::UserDefined
        : CharsetId::Undefined;
}

/*
   Now that the incoming character stream is properly tokenized,
   meaning is assigned to them. These are either operations of
   the current _screen, or of the emulation class itself.

   The token to be interpreted comes in as a machine word
   possibly accompanied by two parameters.

   Likewise, the operations assigned to, come with up to two
   arguments. One could consider to make up a proper table
   from the function below.

   The technical reference manual provides more information
   about this mapping.
*/

void Vt102Emulation::processToken(int token, int32_t p, int q)
{
  switch (token)
  {
    case TY_CHR(         ) : _currentScreen->displayCharacter     (static_cast<ucs4_char>(p)); break; //UTF16

    //             127 DEL    : ignored on input

    case TY_CTL('@'      ) : /* NUL: ignored                      */ break;
    case TY_CTL('A'      ) : /* SOH: ignored                      */ break;
    case TY_CTL('B'      ) : /* STX: ignored                      */ break;
    case TY_CTL('C'      ) : /* ETX: ignored                      */ break;
    case TY_CTL('D'      ) : /* EOT: ignored                      */ break;
    case TY_CTL('F'      ) : /* ACK: ignored                      */ break;
    case TY_CTL('G'      ) : /* TODO emit stateSet(NOTIFYBELL);*/         break; //VT100
    case TY_CTL('H'      ) : _currentScreen->backspace            (          ); break; //VT100
    case TY_CTL('I'      ) : _currentScreen->tab                  (          ); break; //VT100
    case TY_CTL('J'      ) : _currentScreen->newLine              (          ); break; //VT100
    case TY_CTL('K'      ) : _currentScreen->newLine              (          ); break; //VT100
    case TY_CTL('L'      ) : _currentScreen->newLine              (          ); break; //VT100
    case TY_CTL('M'      ) : _currentScreen->toStartOfLine        (          ); break; //VT100

    case TY_CTL('N'      ) :      useCharset           (         1); break; //VT100
    case TY_CTL('O'      ) :      useCharset           (         0); break; //VT100

    case TY_CTL('P'      ) : /* DLE: ignored                      */ break;
    case TY_CTL('Q'      ) : /* DC1: XON continue                 */ break; //VT100
    case TY_CTL('R'      ) : /* DC2: ignored                      */ break;
    case TY_CTL('S'      ) : /* DC3: XOFF halt                    */ break; //VT100
    case TY_CTL('T'      ) : /* DC4: ignored                      */ break;
    case TY_CTL('U'      ) : /* NAK: ignored                      */ break;
    case TY_CTL('V'      ) : /* SYN: ignored                      */ break;
    case TY_CTL('W'      ) : /* ETB: ignored                      */ break;
    case TY_CTL('X'      ) : _currentScreen->displayCharacter     (    0x2592); break; //VT100
    case TY_CTL('Y'      ) : /* EM : ignored                      */ break;
    case TY_CTL('Z'      ) : _currentScreen->displayCharacter     (    0x2592); break; //VT100
    case TY_CTL('['      ) : /* ESC: cannot be seen here.         */ break;
    case TY_CTL('\\'     ) : /* FS : ignored                      */ break;
    case TY_CTL(']'      ) : /* GS : ignored                      */ break;
    case TY_CTL('^'      ) : /* RS : ignored                      */ break;
    case TY_CTL('_'      ) : /* US : ignored                      */ break;

    case TY_ESC('D'      ) : _currentScreen->index                (          ); break; //VT100
    case TY_ESC('E'      ) : _currentScreen->nextLine             (          ); break; //VT100
    case TY_ESC('H'      ) : _currentScreen->changeTabStop        (true      ); break; //VT100
    case TY_ESC('M'      ) : _currentScreen->reverseIndex         (          ); break; //VT100
    case TY_ESC('c'      ) :      reset                (          ); break;

    case TY_ESC('n'      ) :      useCharset           (         2); break;
    case TY_ESC('o'      ) :      useCharset           (         3); break;
    case TY_ESC('7'      ) :      saveCursor           (          ); break;
    case TY_ESC('8'      ) :      restoreCursor        (          ); break;

    case TY_ESC('<'      ) :          setMode      (Mode::Ansi     ); break; //VT100

    case TY_ESC_CS('(', '0') :      setCharset           (0, char_to_charset_id('0')); break; //VT100
    case TY_ESC_CS('(', 'A') :      setCharset           (0, char_to_charset_id('A')); break; //VT100
    case TY_ESC_CS('(', 'B') :      setCharset           (0, char_to_charset_id('B')); break; //VT100
    case TY_ESC_CS('(', 'U') :      setCharset           (0, char_to_charset_id('U')); break; //Linux
    case TY_ESC_CS('(', 'K') :      setCharset           (0, char_to_charset_id('K')); break; //Linux

    case TY_ESC_CS(')', '0') :      setCharset           (1, char_to_charset_id('0')); break; //VT100
    case TY_ESC_CS(')', 'A') :      setCharset           (1, char_to_charset_id('A')); break; //VT100
    case TY_ESC_CS(')', 'B') :      setCharset           (1, char_to_charset_id('B')); break; //VT100
    case TY_ESC_CS(')', 'U') :      setCharset           (1, char_to_charset_id('U')); break; //Linux
    case TY_ESC_CS(')', 'K') :      setCharset           (1, char_to_charset_id('K')); break; //Linux

    case TY_ESC_CS('*', '0') :      setCharset           (2, char_to_charset_id('0')); break; //VT100
    case TY_ESC_CS('*', 'A') :      setCharset           (2, char_to_charset_id('A')); break; //VT100
    case TY_ESC_CS('*', 'B') :      setCharset           (2, char_to_charset_id('B')); break; //VT100
    case TY_ESC_CS('*', 'U') :      setCharset           (2, char_to_charset_id('U')); break; //Linux
    case TY_ESC_CS('*', 'K') :      setCharset           (2, char_to_charset_id('K')); break; //Linux

    case TY_ESC_CS('+', '0') :      setCharset           (3, char_to_charset_id('0')); break; //VT100
    case TY_ESC_CS('+', 'A') :      setCharset           (3, char_to_charset_id('A')); break; //VT100
    case TY_ESC_CS('+', 'B') :      setCharset           (3, char_to_charset_id('B')); break; //VT100
    case TY_ESC_CS('+', 'U') :      setCharset           (3, char_to_charset_id('U')); break; //Linux
    case TY_ESC_CS('+', 'K') :      setCharset           (3, char_to_charset_id('K')); break; //Linux

    case TY_ESC_CS('%', 'G') :      /* TODO setCodec             (Utf8Codec   );*/ break; //LINUX
    case TY_ESC_CS('%', '@') :      /* TODO setCodec             (LocaleCodec );*/ break; //LINUX

    case TY_ESC_DE('3'     ) : /* Double height line, top half    */
                               _currentScreen->setLineProperty( LineProperty::DoubleWidth , true );
                               _currentScreen->setLineProperty( LineProperty::DoubleHeight , true );
                                   break;
    case TY_ESC_DE('4'     ) : /* Double height line, bottom half */
                               _currentScreen->setLineProperty( LineProperty::DoubleWidth , true );
                               _currentScreen->setLineProperty( LineProperty::DoubleHeight , true );
                                   break;
    case TY_ESC_DE('5'     ) : /* Single width, single height line*/
                               _currentScreen->setLineProperty( LineProperty::DoubleWidth , false);
                               _currentScreen->setLineProperty( LineProperty::DoubleHeight , false);
                               break;
    case TY_ESC_DE('6'     ) : /* Double width, single height line*/
                               _currentScreen->setLineProperty( LineProperty::DoubleWidth , true);
                               _currentScreen->setLineProperty( LineProperty::DoubleHeight , false);
                               break;
    case TY_ESC_DE('8'     ) : _currentScreen->helpAlign            (          ); break;

// resize = \e[8;<row>;<col>t
    case TY_CSI_PS('t',   8) : // TODO setImageSize( p /*lines */, q /* columns */ );
                               // TODO emit imageResizeRequest(QSize(q, p));
                               break;

// change tab text color : \e[28;<color>t  color: 0-16,777,215
    case TY_CSI_PS('t',   28) : /* TODO emit changeTabTextColorRequest      ( p        );*/          break;

    case TY_CSI_PS('K',   0) : _currentScreen->clearToEndOfLine     (          ); break;
    case TY_CSI_PS('K',   1) : _currentScreen->clearToBeginOfLine   (          ); break;
    case TY_CSI_PS('K',   2) : _currentScreen->clearEntireLine      (          ); break;
    case TY_CSI_PS('J',   0) : _currentScreen->clearToEndOfScreen   (          ); break;
    case TY_CSI_PS('J',   1) : _currentScreen->clearToBeginOfScreen (          ); break;
    case TY_CSI_PS('J',   2) : _currentScreen->clearEntireScreen    (          ); break;
    case TY_CSI_PS('J',   3) : /* clearHistory();*/                               break;
    case TY_CSI_PS('g',   0) : _currentScreen->changeTabStop        (false     ); break; //VT100
    case TY_CSI_PS('g',   3) : _currentScreen->clearTabStops        (          ); break; //VT100
    case TY_CSI_PS('h',   4) : _currentScreen->    setMode      (ScreenMode::Insert   ); break;
    case TY_CSI_PS('h',  20) :          setMode      (ScreenMode::NewLine  ); break;
    case TY_CSI_PS('i',   0) : /* IGNORE: attached printer          */ break; //VT100
    case TY_CSI_PS('l',   4) : _currentScreen->  resetMode      (ScreenMode::Insert   ); break;
    case TY_CSI_PS('l',  20) :        resetMode      (ScreenMode::NewLine  ); break;
    case TY_CSI_PS('s',   0) :      saveCursor           (          ); break;
    case TY_CSI_PS('u',   0) :      restoreCursor        (          ); break;

    case TY_CSI_PS('m',   0) : _currentScreen->setDefaultRendition  (          ); break;
    case TY_CSI_PS('m',   1) : _currentScreen->  setRendition     (Rendition::Bold     ); break; //VT100
    case TY_CSI_PS('m',   3) : _currentScreen->  setRendition     (Rendition::Italic   ); break; //VT100
    case TY_CSI_PS('m',   4) : _currentScreen->  setRendition     (Rendition::Underline); break; //VT100
    case TY_CSI_PS('m',   5) : _currentScreen->  setRendition     (Rendition::Blink    ); break; //VT100
    case TY_CSI_PS('m',   7) : _currentScreen->  setRendition     (Rendition::Reverse  ); break;
    case TY_CSI_PS('m',  10) : /* IGNORED: mapping related          */ break; //LINUX
    case TY_CSI_PS('m',  11) : /* IGNORED: mapping related          */ break; //LINUX
    case TY_CSI_PS('m',  12) : /* IGNORED: mapping related          */ break; //LINUX
    case TY_CSI_PS('m',  22) : _currentScreen->resetRendition     (Rendition::Bold     ); break;
    case TY_CSI_PS('m',  23) : _currentScreen->resetRendition     (Rendition::Italic   ); break; //VT100
    case TY_CSI_PS('m',  24) : _currentScreen->resetRendition     (Rendition::Underline); break;
    case TY_CSI_PS('m',  25) : _currentScreen->resetRendition     (Rendition::Blink    ); break;
    case TY_CSI_PS('m',  27) : _currentScreen->resetRendition     (Rendition::Reverse  ); break;

    case TY_CSI_PS('m',   30) : _currentScreen->setForeColor         (ColorSpace::System,  0); break;
    case TY_CSI_PS('m',   31) : _currentScreen->setForeColor         (ColorSpace::System,  1); break;
    case TY_CSI_PS('m',   32) : _currentScreen->setForeColor         (ColorSpace::System,  2); break;
    case TY_CSI_PS('m',   33) : _currentScreen->setForeColor         (ColorSpace::System,  3); break;
    case TY_CSI_PS('m',   34) : _currentScreen->setForeColor         (ColorSpace::System,  4); break;
    case TY_CSI_PS('m',   35) : _currentScreen->setForeColor         (ColorSpace::System,  5); break;
    case TY_CSI_PS('m',   36) : _currentScreen->setForeColor         (ColorSpace::System,  6); break;
    case TY_CSI_PS('m',   37) : _currentScreen->setForeColor         (ColorSpace::System,  7); break;

    case TY_CSI_PS('m',   38) : _currentScreen->setForeColor         (ColorSpace(p),       q); break;

    case TY_CSI_PS('m',   39) : _currentScreen->setForeColor         (ColorSpace::Default,  0); break;

    case TY_CSI_PS('m',   40) : _currentScreen->setBackColor         (ColorSpace::System,  0); break;
    case TY_CSI_PS('m',   41) : _currentScreen->setBackColor         (ColorSpace::System,  1); break;
    case TY_CSI_PS('m',   42) : _currentScreen->setBackColor         (ColorSpace::System,  2); break;
    case TY_CSI_PS('m',   43) : _currentScreen->setBackColor         (ColorSpace::System,  3); break;
    case TY_CSI_PS('m',   44) : _currentScreen->setBackColor         (ColorSpace::System,  4); break;
    case TY_CSI_PS('m',   45) : _currentScreen->setBackColor         (ColorSpace::System,  5); break;
    case TY_CSI_PS('m',   46) : _currentScreen->setBackColor         (ColorSpace::System,  6); break;
    case TY_CSI_PS('m',   47) : _currentScreen->setBackColor         (ColorSpace::System,  7); break;

    case TY_CSI_PS('m',   48) : _currentScreen->setBackColor         (ColorSpace(p),       q); break;

    case TY_CSI_PS('m',   49) : _currentScreen->setBackColor         (ColorSpace::Default,  1); break;

    case TY_CSI_PS('m',   90) : _currentScreen->setForeColor         (ColorSpace::System,  8); break;
    case TY_CSI_PS('m',   91) : _currentScreen->setForeColor         (ColorSpace::System,  9); break;
    case TY_CSI_PS('m',   92) : _currentScreen->setForeColor         (ColorSpace::System, 10); break;
    case TY_CSI_PS('m',   93) : _currentScreen->setForeColor         (ColorSpace::System, 11); break;
    case TY_CSI_PS('m',   94) : _currentScreen->setForeColor         (ColorSpace::System, 12); break;
    case TY_CSI_PS('m',   95) : _currentScreen->setForeColor         (ColorSpace::System, 13); break;
    case TY_CSI_PS('m',   96) : _currentScreen->setForeColor         (ColorSpace::System, 14); break;
    case TY_CSI_PS('m',   97) : _currentScreen->setForeColor         (ColorSpace::System, 15); break;

    case TY_CSI_PS('m',  100) : _currentScreen->setBackColor         (ColorSpace::System,  8); break;
    case TY_CSI_PS('m',  101) : _currentScreen->setBackColor         (ColorSpace::System,  9); break;
    case TY_CSI_PS('m',  102) : _currentScreen->setBackColor         (ColorSpace::System, 10); break;
    case TY_CSI_PS('m',  103) : _currentScreen->setBackColor         (ColorSpace::System, 11); break;
    case TY_CSI_PS('m',  104) : _currentScreen->setBackColor         (ColorSpace::System, 12); break;
    case TY_CSI_PS('m',  105) : _currentScreen->setBackColor         (ColorSpace::System, 13); break;
    case TY_CSI_PS('m',  106) : _currentScreen->setBackColor         (ColorSpace::System, 14); break;
    case TY_CSI_PS('m',  107) : _currentScreen->setBackColor         (ColorSpace::System, 15); break;

    case TY_CSI_PS('q',   0) : /* IGNORED: LEDs off                 */ break; //VT100
    case TY_CSI_PS('q',   1) : /* IGNORED: LED1 on                  */ break; //VT100
    case TY_CSI_PS('q',   2) : /* IGNORED: LED2 on                  */ break; //VT100
    case TY_CSI_PS('q',   3) : /* IGNORED: LED3 on                  */ break; //VT100
    case TY_CSI_PS('q',   4) : /* IGNORED: LED4 on                  */ break; //VT100

    case TY_CSI_PN('@'      ) : _currentScreen->insertChars          (p         ); break;
    case TY_CSI_PN('A'      ) : _currentScreen->cursorUp             (p         ); break; //VT100
    case TY_CSI_PN('B'      ) : _currentScreen->cursorDown           (p         ); break; //VT100
    case TY_CSI_PN('C'      ) : _currentScreen->cursorRight          (p         ); break; //VT100
    case TY_CSI_PN('D'      ) : _currentScreen->cursorLeft           (p         ); break; //VT100
    case TY_CSI_PN('E'      ) : /* Not implemented: cursor next p lines */         break; //VT100
    case TY_CSI_PN('F'      ) : /* Not implemented: cursor preceding p lines */    break; //VT100
    case TY_CSI_PN('G'      ) : _currentScreen->setCursorX           (p         ); break; //LINUX
    case TY_CSI_PN('H'      ) : _currentScreen->setCursorYX          (p,      q); break; //VT100
    case TY_CSI_PN('I'      ) : _currentScreen->tab                  (p         ); break;
    case TY_CSI_PN('L'      ) : _currentScreen->insertLines          (p         ); break;
    case TY_CSI_PN('M'      ) : _currentScreen->deleteLines          (p         ); break;
    case TY_CSI_PN('P'      ) : _currentScreen->deleteChars          (p         ); break;
    case TY_CSI_PN('S'      ) : _currentScreen->scrollUp             (p         ); break;
    case TY_CSI_PN('T'      ) : _currentScreen->scrollDown           (p         ); break;
    case TY_CSI_PN('X'      ) : _currentScreen->eraseChars           (p         ); break;
    case TY_CSI_PN('Z'      ) : _currentScreen->backtab              (p         ); break;
    case TY_CSI_PN('d'      ) : _currentScreen->setCursorY           (p         ); break; //LINUX
    case TY_CSI_PN('f'      ) : _currentScreen->setCursorYX          (p,      q); break; //VT100
    case TY_CSI_PN('r'      ) :      setMargins           (p,      q); break; //VT100
    case TY_CSI_PN('y'      ) : /* IGNORED: Confidence test          */ break; //VT100

    case TY_CSI_PR('l',   2) :        resetMode      (Mode::Ansi     ); break; //VT100

    case TY_CSI_PR('h',   3) :          setMode      (Mode::Columns132); break; //VT100
    case TY_CSI_PR('l',   3) :        resetMode      (Mode::Columns132); break; //VT100

    case TY_CSI_PR('h',   4) : /* IGNORED: soft scrolling           */ break; //VT100
    case TY_CSI_PR('l',   4) : /* IGNORED: soft scrolling           */ break; //VT100

    case TY_CSI_PR('h',   5) : _currentScreen->    setMode      (ScreenMode::Screen   ); break; //VT100
    case TY_CSI_PR('l',   5) : _currentScreen->  resetMode      (ScreenMode::Screen   ); break; //VT100

    case TY_CSI_PR('h',   6) : _currentScreen->    setMode      (ScreenMode::Origin   ); break; //VT100
    case TY_CSI_PR('l',   6) : _currentScreen->  resetMode      (ScreenMode::Origin   ); break; //VT100
    case TY_CSI_PR('s',   6) : _currentScreen->   saveMode      (ScreenMode::Origin   ); break; //FIXME
    case TY_CSI_PR('r',   6) : _currentScreen->restoreMode      (ScreenMode::Origin   ); break; //FIXME

    case TY_CSI_PR('h',   7) : _currentScreen->    setMode      (ScreenMode::Wrap     ); break; //VT100
    case TY_CSI_PR('l',   7) : _currentScreen->  resetMode      (ScreenMode::Wrap     ); break; //VT100
    case TY_CSI_PR('s',   7) : _currentScreen->   saveMode      (ScreenMode::Wrap     ); break; //FIXME
    case TY_CSI_PR('r',   7) : _currentScreen->restoreMode      (ScreenMode::Wrap     ); break; //FIXME

    case TY_CSI_PR('h',   8) : /* IGNORED: autorepeat on            */ break; //VT100
    case TY_CSI_PR('l',   8) : /* IGNORED: autorepeat off           */ break; //VT100
    case TY_CSI_PR('s',   8) : /* IGNORED: autorepeat on            */ break; //VT100
    case TY_CSI_PR('r',   8) : /* IGNORED: autorepeat off           */ break; //VT100

    case TY_CSI_PR('h',   9) : /* IGNORED: interlace                */ break; //VT100
    case TY_CSI_PR('l',   9) : /* IGNORED: interlace                */ break; //VT100
    case TY_CSI_PR('s',   9) : /* IGNORED: interlace                */ break; //VT100
    case TY_CSI_PR('r',   9) : /* IGNORED: interlace                */ break; //VT100

    case TY_CSI_PR('h',  12) : /* IGNORED: Cursor blink             */ break; //att610
    case TY_CSI_PR('l',  12) : /* IGNORED: Cursor blink             */ break; //att610
    case TY_CSI_PR('s',  12) : /* IGNORED: Cursor blink             */ break; //att610
    case TY_CSI_PR('r',  12) : /* IGNORED: Cursor blink             */ break; //att610

    case TY_CSI_PR('h',  25) :          setMode      (ScreenMode::Cursor   ); break; //VT100
    case TY_CSI_PR('l',  25) :        resetMode      (ScreenMode::Cursor   ); break; //VT100
    case TY_CSI_PR('s',  25) :         saveMode      (ScreenMode::Cursor   ); break; //VT100
    case TY_CSI_PR('r',  25) :      restoreMode      (ScreenMode::Cursor   ); break; //VT100

    case TY_CSI_PR('h',  40) :         setMode(Mode::AllowColumns132 ); break; // XTERM
    case TY_CSI_PR('l',  40) :       resetMode(Mode::AllowColumns132 ); break; // XTERM

    case TY_CSI_PR('h',  41) : /* IGNORED: obsolete more(1) fix     */ break; //XTERM
    case TY_CSI_PR('l',  41) : /* IGNORED: obsolete more(1) fix     */ break; //XTERM
    case TY_CSI_PR('s',  41) : /* IGNORED: obsolete more(1) fix     */ break; //XTERM
    case TY_CSI_PR('r',  41) : /* IGNORED: obsolete more(1) fix     */ break; //XTERM

    case TY_CSI_PR('h',  47) :          setMode      (Mode::AppScreen); break; //VT100
    case TY_CSI_PR('l',  47) :        resetMode      (Mode::AppScreen); break; //VT100
    case TY_CSI_PR('s',  47) :         saveMode      (Mode::AppScreen); break; //XTERM
    case TY_CSI_PR('r',  47) :      restoreMode      (Mode::AppScreen); break; //XTERM

    case TY_CSI_PR('h',  67) : /* IGNORED: DECBKM                   */ break; //XTERM
    case TY_CSI_PR('l',  67) : /* IGNORED: DECBKM                   */ break; //XTERM
    case TY_CSI_PR('s',  67) : /* IGNORED: DECBKM                   */ break; //XTERM
    case TY_CSI_PR('r',  67) : /* IGNORED: DECBKM                   */ break; //XTERM

    case TY_CSI_PR('h', 1034) : /* IGNORED: 8bitinput activation     */ break; //XTERM

    case TY_CSI_PR('h', 1047) :          setMode      (Mode::AppScreen); break; //XTERM
    case TY_CSI_PR('l', 1047) : _currentScreen->clearEntireScreen(); resetMode(Mode::AppScreen); break; //XTERM
    case TY_CSI_PR('s', 1047) :         saveMode      (Mode::AppScreen); break; //XTERM
    case TY_CSI_PR('r', 1047) :      restoreMode      (Mode::AppScreen); break; //XTERM

    //FIXME: Unitoken: save translations
    case TY_CSI_PR('h', 1048) :      saveCursor           (          ); break; //XTERM
    case TY_CSI_PR('l', 1048) :      restoreCursor        (          ); break; //XTERM
    case TY_CSI_PR('s', 1048) :      saveCursor           (          ); break; //XTERM
    case TY_CSI_PR('r', 1048) :      restoreCursor        (          ); break; //XTERM

    //FIXME: every once new sequences like this pop up in xterm.
    //       Here's a guess of what they could mean.
    case TY_CSI_PR('h', 1049) : saveCursor(); _currentScreen->clearEntireScreen(); setMode(Mode::AppScreen); break; //XTERM
    case TY_CSI_PR('l', 1049) : resetMode(Mode::AppScreen); restoreCursor(); break; //XTERM

    //FIXME: weird DEC reset sequence
    case TY_CSI_PE('p'      ) : /* IGNORED: reset         (        ) */ break;

    //FIXME: when changing between vt52 and ansi mode evtl do some resetting.
    case TY_VT52('A'      ) : _currentScreen->cursorUp             (         1); break; //VT52
    case TY_VT52('B'      ) : _currentScreen->cursorDown           (         1); break; //VT52
    case TY_VT52('C'      ) : _currentScreen->cursorRight          (         1); break; //VT52
    case TY_VT52('D'      ) : _currentScreen->cursorLeft           (         1); break; //VT52

    // FIXME The special graphics characters in the VT100 are different from those in the VT52.
    case TY_VT52('F'      ) :      setAndUseCharset     (0, char_to_charset_id('0')); break; //VT52
    case TY_VT52('G'      ) :      setAndUseCharset     (0, char_to_charset_id('B')); break; //VT52

    case TY_VT52('H'      ) : _currentScreen->setCursorYX          (1,1       ); break; //VT52
    case TY_VT52('I'      ) : _currentScreen->reverseIndex         (          ); break; //VT52
    case TY_VT52('J'      ) : _currentScreen->clearToEndOfScreen   (          ); break; //VT52
    case TY_VT52('K'      ) : _currentScreen->clearToEndOfLine     (          ); break; //VT52
    case TY_VT52('Y'      ) : _currentScreen->setCursorYX          (p-31,q-31 ); break; //VT52
    case TY_VT52('<'      ) :          setMode      (Mode::Ansi     ); break; //VT52

    default:
        reportDecodingError();
        break;
  };
}

void Vt102Emulation::clearScreenAndSetColumns(int /*columnCount*/)
{
    // TODO setImageSize(_currentScreen->getLines(),columnCount);
    clearEntireScreen();
    setDefaultMargins();
    _currentScreen->setCursorYX(0,0);
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                                VT100 Charsets                             */
/*                                                                           */
/* ------------------------------------------------------------------------- */

// Character Set Conversion ------------------------------------------------ --

/*
   The processing contains a VT100 specific code translation layer.
   It's still in use and mainly responsible for the line drawing graphics.

   These and some other glyphs are assigned to codes (0x5f-0xfe)
   normally occupied by the latin letters. Since this codes also
   appear within control sequences, the extra code conversion
   does not permute with the tokenizer and is placed behind it
   in the pipeline. It only applies to tokens, which represent
   plain characters.

   This conversion it eventually continued in TerminalDisplay.C, since
   it might involve VT100 enhanced fonts, which have these
   particular glyphs allocated in (0x00-0x1f) in their code page.
*/

// Apply current character map.

#define CHARSET _charsets[_currentScreen == &_screens[1]]

ucs4_char Vt102Emulation::applyCharset(ucs4_char c) const
{
    auto const charset_index = underlying_cast(CHARSET.charset_id);
    if (charset_index < underlying_cast(CharsetId::MAX_) && c < charset_map_size) {
        return charset_maps[charset_index][c];
    }
    return c;
}

/*
   "Charset" related part of the emulation state.
   This configures the VT100 charset filter.

   While most operation work on the current _screen,
   the following two are different.
*/

void Vt102Emulation::resetCharset()
{
    _charsets[0].charset.fill(CharsetId::Latin1);
    _charsets[0].charset_id = CharsetId::Latin1;
    _charsets[0].sa_charset_id = CharsetId::Latin1;

    _charsets[1].charset.fill(CharsetId::Latin1);
    _charsets[1].charset_id = CharsetId::Latin1;
    _charsets[1].sa_charset_id = CharsetId::Latin1;
}

void Vt102Emulation::setCharset(int n, CharsetId cs) // on both screens.
{
    _charsets[0].charset[n & 3] = cs;
    _charsets[1].charset[n & 3] = cs;
    this->useCharset(n);
}

void Vt102Emulation::setAndUseCharset(int n, CharsetId cs)
{
    CHARSET.charset[n & 3] = cs;
    useCharset(n);
}

void Vt102Emulation::useCharset(int n)
{
    CHARSET.charset_id = CHARSET.charset[n & 3];
}

void Vt102Emulation::setDefaultMargins()
{
    _currentScreen->setDefaultMargins();
}

void Vt102Emulation::setScreen(int n)
{
    _currentScreen = &_screens[n & 1];
}

void Vt102Emulation::setScreenSize(int lines, int columns)
{
    if (lines < 1 || columns < 1) {
        return;
    }

    _screens[0].resizeImage(lines, columns);
    _screens[1].resizeImage(lines, columns);
}

void Vt102Emulation::setMargins(int t, int b)
{
    _currentScreen->setMargins(t, b);
}

void Vt102Emulation::saveCursor()
{
    CHARSET.sa_charset_id = CHARSET.charset_id;
    // we are not clear about these
    //sa_charset = charsets[cScreen->_charset];
    //sa_charset_num = cScreen->_charset;
    _currentScreen->saveCursor();
}

void Vt102Emulation::restoreCursor()
{
    CHARSET.charset_id = CHARSET.sa_charset_id;
    _currentScreen->restoreCursor();
}

// #undef CHARSET

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                                Mode Operations                            */
/*                                                                           */
/* ------------------------------------------------------------------------- */

/*
   Some of the emulations state is either added to the state of the screens.

   This causes some scoping problems, since different emulations choose to
   located the mode either to the current _screen or to both.

   For strange reasons, the extend of the rendition attributes ranges over
   all screens and not over the actual _screen.

   We decided on the precise precise extend, somehow.
*/

// "Mode" related part of the state. These are all booleans.

void Vt102Emulation::resetModes()
{
    // Mode::AllowColumns132 is not reset here
    // to match Xterm's behavior (see Xterm's VTReset() function)

    resetMode(Mode::Columns132); saveMode(Mode::Columns132);
    resetMode(Mode::AppScreen);  saveMode(Mode::AppScreen);
    resetMode(ScreenMode::NewLine);
    setMode(Mode::Ansi);
}

void Vt102Emulation::setMode(Mode m)
{
    _currentModes.set(m);
    switch (m) {
    case Mode::Columns132:
        if (getMode(Mode::AllowColumns132))
            clearScreenAndSetColumns(132);
        else
            _currentModes.reset(m);
        break;

    case Mode::AppScreen :
        setScreen(1);
        break;
    }
}

void Vt102Emulation::setMode(ScreenMode m)
{
    _screens[0].setMode(m);
    _screens[1].setMode(m);
}

void Vt102Emulation::resetMode(Mode m)
{
    _currentModes.reset(m);
    switch (m) {
    case Mode::Columns132:
        if (getMode(Mode::AllowColumns132))
            clearScreenAndSetColumns(80);
        break;

    case Mode::AppScreen :
        setScreen(0);
        break;
    }
}

void Vt102Emulation::resetMode(ScreenMode m)
{
    _screens[0].resetMode(m);
    _screens[1].resetMode(m);
}

void Vt102Emulation::saveMode(Mode m)
{
    _savedModes.copy_of(m, _currentModes);
}

void Vt102Emulation::restoreMode(Mode m)
{
    _currentModes.copy_of(m, _savedModes);
}

bool Vt102Emulation::getMode(Mode m)
{
    return _currentModes.has(m);
}

void Vt102Emulation::saveMode(ScreenMode m)
{
    // TODO saveMode(static_cast<Screen>(m))
    _currentScreen->saveMode(m);
}

void Vt102Emulation::restoreMode(ScreenMode m)
{
    // TODO restoreMode(static_cast<Screen>(m))
    _currentScreen->resetMode(m);
}

bool Vt102Emulation::getMode(ScreenMode m)
{
    // TODO getMode(static_cast<Screen>(m))
    return _currentScreen->getMode(m);
}

// return contents of the scan buffer
static std::string hexdump2(int* s, int len)
{
    int i;
    char dump[128];
    std::string returnDump;

    for (i = 0; i < len; i++) {
        if (s[i] == '\\')
            std::snprintf(dump, sizeof(dump), "%s", "\\\\");
        else if ((s[i]) > 32 && s[i] < 127)
            std::snprintf(dump, sizeof(dump), "%c", s[i]);
        else
            std::snprintf(dump, sizeof(dump), "\\%04x(hex)", s[i]);
        returnDump.append(dump);
    }
    return returnDump;
}

void Vt102Emulation::reportDecodingError()
{
    if (tokenBufferPos == 0 || (tokenBufferPos == 1 && (tokenBuffer[0] & 0xff) >= 32))
        return;

    std::string outputError("Undecodable sequence: ");
    outputError.append(hexdump2(tokenBuffer, tokenBufferPos));
    LOG(LOG_INFO, "%s", outputError.c_str());
    // std::cerr << outputError << std::endl;
}

}


#include <iostream>

template<class T>
auto print_value(std::ostream & os, T const & value)
-> decltype(void(os << value))
{ os << value; }

// void print_value(std::ostream & os, vt100::Cursor const & cursor)
// { os << "{" << cursor.x << " " << cursor.y << "}"; }


struct Checker
{
    char const * file;
    int line;
    const char * expr;

    template<class T, class U>
    void check_equal(T const & a, U const & b)
    {
        if (!(a == b)) {
            std::cerr << file << ":" << line << ": " << expr << " -- [";
            print_value(std::cerr, a);
            std::cerr << " == ";
            print_value(std::cerr, b);
            std::cerr << "] failed\n";
            std::abort();
        }
    }
};


#include <iomanip>

using uchar = unsigned char;

template<std::size_t n>
int touc(char const (&s)[n])
{
    int r = 0;
    for (std::size_t i = 1; i < n; ++i) {
        r <<= 8;
        r |= uchar(s[i-1]);
    }
    return r;
}

// template<int> struct i_{};

int main()
{
    rvt::Vt102Emulation emulator(40, 40);

    for (int i = 0; i < 20; ++i) {
        emulator.receiveChar('a');
        emulator.receiveChar('b');
        emulator.receiveChar('c');
    }
    emulator.receiveChar('\033');
    emulator.receiveChar('[');
    emulator.receiveChar('0');
    emulator.receiveChar('B');
    emulator.receiveChar('\033');
    emulator.receiveChar('[');
    emulator.receiveChar('3');
    emulator.receiveChar('1');
    emulator.receiveChar('m');
    for (int i = 0; i < 20; ++i) {
        emulator.receiveChar('a');
        emulator.receiveChar('b');
        emulator.receiveChar('c');
    }
    emulator.receiveChar('\033');
    emulator.receiveChar('[');
    emulator.receiveChar('4');
    emulator.receiveChar('4');
    emulator.receiveChar('m');
    emulator.receiveChar(UTF8toUnicodeIterator("é").code());
    emulator.receiveChar('e');
    emulator.receiveChar(0x311);
    emulator.receiveChar(0xac00);


    rvt::Screen const & screen = emulator.getScreen();

    auto print_uc = [](rvt::ucs4_char uc) {
        char utf8_ch[4];
        std::size_t const n = ucs4_to_utf8(uc, utf8_ch);
        std::cout.write(utf8_ch, static_cast<std::streamsize>(n));
    };

    auto print_ch = [&screen, print_uc](rvt::Character const & ch) {
        if (ch.isRealCharacter) {
            if (ch.is_extended()) {
                for (rvt::ucs4_char uc : screen.extendedCharTable()[ch.character]) {
                    print_uc(uc);
                }
            }
            else {
                print_uc(ch.character);
            }
        }
    };

    int i = 0;
    for (auto const & line : screen.getScreenLines()) {
        std::cout << std::setw(4) << ++i << " ";
        for (rvt::Character const & ch : line) {
            print_ch(ch);
        }
        std::cout << '\n';
    }

    auto print_color = [](char const * cmd, rvt::CharacterColor const & ch_color) {
        auto color = ch_color.color(rvt::color_table);
        std::cout << cmd << (color.red()+0) << ";" << (color.green()+0) << ";" << (color.blue()+0);
    };
    auto print_mode = [](char const * cmd, rvt::Character const & ch, rvt::Rendition r) {
        if (bool(ch.rendition & r)) {
            std::cout << cmd;
        }
    };

    // Format
    //@{
    rvt::Rendition previous_rendition = rvt::Rendition::Default;
    rvt::CharacterColor previous_fg(rvt::ColorSpace::Default, 0);
    rvt::CharacterColor previous_bg(rvt::ColorSpace::Default, 1);
    //@}
    std::cout << "\033[0";
    print_color(";38;2;", previous_fg);
    print_color(";48;2;", previous_bg);
    std::cout << "m";
    for (auto const & line : screen.getScreenLines()) {
        for (rvt::Character const & ch : line) {
            if (!ch.isRealCharacter) {
                continue;
            }
            if (ch.backgroundColor != previous_bg
             || ch.foregroundColor != previous_fg
             || ch.rendition != previous_rendition
            ) {
                std::cout << "\033[0";
                print_mode(";1", ch, rvt::Rendition::Bold);
                print_mode(";3", ch, rvt::Rendition::Italic);
                print_mode(";4", ch, rvt::Rendition::Underline);
                print_mode(";5", ch, rvt::Rendition::Blink);
                print_mode(";7", ch, rvt::Rendition::Reverse);
                print_color(";38;2;", ch.foregroundColor);
                print_color(";48;2;", ch.backgroundColor);
                std::cout << "m";
                previous_bg = ch.backgroundColor;
                previous_fg = ch.foregroundColor;
                previous_rendition = ch.rendition;
            }
            print_ch(ch);
        }
        std::cout << '\n';
    }
}
