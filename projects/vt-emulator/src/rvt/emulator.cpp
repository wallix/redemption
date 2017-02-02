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
#include "rvt/screen.hpp"

//#define CXX_UNUSED(x) (void)x // [[maybe_unused]]

// #define REDEMPTION_DIAGNOSTIC_IGNORE_CONVERSION REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wconversion")

namespace rvt
{

#define MODES_SCREEN static_cast<int>(Mode::COUNT_)

#define MODE_AppScreen       (MODES_SCREEN+0)   // Mode #1
#define MODE_AppCuKeys       (MODES_SCREEN+1)   // Application cursor keys (DECCKM)
#define MODE_AppKeyPad       (MODES_SCREEN+2)   //
#define MODE_Mouse1000       (MODES_SCREEN+3)   // Send mouse X,Y position on press and release
#define MODE_Mouse1001       (MODES_SCREEN+4)   // Use Hilight mouse tracking
#define MODE_Mouse1002       (MODES_SCREEN+5)   // Use cell motion mouse tracking
#define MODE_Mouse1003       (MODES_SCREEN+6)   // Use all motion mouse tracking
#define MODE_Mouse1005       (MODES_SCREEN+7)   // Xterm-style extended coordinates
#define MODE_Mouse1006       (MODES_SCREEN+8)   // 2nd Xterm-style extended coordinates
#define MODE_Mouse1015       (MODES_SCREEN+9)   // Urxvt-style extended coordinates
#define MODE_Ansi            (MODES_SCREEN+10)   // Use US Ascii for character sets G0-G3 (DECANM)
#define MODE_132Columns      (MODES_SCREEN+11)  // 80 <-> 132 column mode switch (DECCOLM)
#define MODE_Allow132Columns (MODES_SCREEN+12)  // Allow DECCOLM mode
#define MODE_BracketedPaste  (MODES_SCREEN+13)  // Xterm-style bracketed paste mode
#define MODE_total           (MODES_SCREEN+14)

extern unsigned short vt100_graphics[32];

struct CharCodes {
    // coding info
    char charset[4]; //
    int  cu_cs;      // actual charset.
    bool graphic;    // Some VT100 tricks
    bool pound;      // Some VT100 tricks
    bool sa_graphic; // saved graphic
    bool sa_pound;   // saved pound
};


/**
 * Provides an xterm compatible terminal emulation based on the DEC VT102 terminal.
 * A full description of this terminal can be found at http://vt100.net/docs/vt102-ug/
 *
 * In addition, various additional xterm escape sequences are supported to provide
 * features such as mouse input handling.
 * See http://rtfm.etla.org/xterm/ctlseq.html for a description of xterm's escape
 * sequences.
 *
 */
class Vt102Emulation
{
public:
    /** Constructs a new emulation */
    Vt102Emulation(Screen * screen);
    ~Vt102Emulation();

    // reimplemented from Emulation
    void clearEntireScreen();
    void reset();
    char eraseChar() const;

protected:
    // reimplemented from Emulation
    void setMode(int mode);
    void resetMode(int mode);

public: // TODO protected
    void receiveChar(uc_t cc);

private:
    uc_t applyCharset(uc_t  c);
    void setCharset(int n, int cs);
    void useCharset(int n);
    void setAndUseCharset(int n, int cs);
    void saveCursor();
    void restoreCursor();
    void resetCharset(int scrno);

    void setMargins(int top, int bottom);
    //set margins for all screens back to their defaults
    void setDefaultMargins();

    // returns true if 'mode' is set or false otherwise
    bool getMode(int mode);
    // saves the current boolean value of 'mode'
    void saveMode(int mode);
    // restores the boolean value of 'mode'
    void restoreMode(int mode);
    // resets all modes
    // (except MODE_Allow132Columns)
    void resetModes();

    void resetTokenizer();
#define MAX_TOKEN_LENGTH 256 // Max length of tokens (e.g. window title)
    void addToCurrentToken(uc_t cc);
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

    void processToken(int code, int p, int q);

    void reportTerminalType() { /* TODO */ }
    void reportSecondaryAttributes() { /* TODO */ }
    void reportStatus() { /* TODO */ }
    void reportAnswerBack() { /* TODO */ }
    void reportCursorPosition() { /* TODO */ }
    void reportTerminalParms(int p) { (void)p; /* TODO */ }

    // clears the screen and resizes it to the specified
    // number of columns
    void clearScreenAndSetColumns(int columnCount);

    CharCodes _charset[2];

    class TerminalState
    {
    public:
        // Initializes all modes to false
        TerminalState() {
            memset(&mode, false, MODE_total * sizeof(bool));
        }

        bool mode[MODE_total];
    };

    TerminalState _currentModes;
    TerminalState _savedModes;

    bool _reportFocusEvents;

    Screen* _currentScreen;
};

unsigned short vt100_graphics[32] = {
    // 0/8     1/9    2/10    3/11    4/12    5/13    6/14    7/15
    0x0020, 0x25C6, 0x2592, 0x2409, 0x240c, 0x240d, 0x240a, 0x00b0,
    0x00b1, 0x2424, 0x240b, 0x2518, 0x2510, 0x250c, 0x2514, 0x253c,
    0xF800, 0xF801, 0x2500, 0xF803, 0xF804, 0x251c, 0x2524, 0x2534,
    0x252c, 0x2502, 0x2264, 0x2265, 0x03C0, 0x2260, 0x00A3, 0x00b7
};

Vt102Emulation::Vt102Emulation(Screen * screen)
: _reportFocusEvents(false)
, _currentScreen(screen)
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
    resetCharset(0);
//     _screen[0]->reset();
    resetCharset(1);
//     _screen[1]->reset();
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

void Vt102Emulation::addToCurrentToken(uc_t cc)
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

    resetTokenizer();
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
void Vt102Emulation::receiveChar(uc_t cc)
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

  if (getMode(MODE_Ansi))
  {
    if (lec(1,0,ESC)) { return; }
    if (lec(1,0,ESC+128)) { s[0] = ESC; receiveChar('['); return; }
    if (les(2,1,GRP)) { return; }
    if (Xte         ) { /* TODO processWindowAttributeRequest();*/ resetTokenizer(); return; }
    if (Xpe         ) { return; }
    if (lec(3,2,'?')) { return; }
    if (lec(3,2,'>')) { return; }
    if (lec(3,2,'!')) { return; }
    if (lun(       )) { processToken( TY_CHR(), applyCharset(cc), 0);   resetTokenizer(); return; }
    if (lec(2,0,ESC)) { processToken( TY_ESC(s[1]), 0, 0);              resetTokenizer(); return; }
    if (les(3,1,SCS)) { processToken( TY_ESC_CS(s[1],s[2]), 0, 0);      resetTokenizer(); return; }
    if (lec(3,1,'#')) { processToken( TY_ESC_DE(s[2]), 0, 0);           resetTokenizer(); return; }
    if (eps(    CPN)) { processToken( TY_CSI_PN(cc), argv[0],argv[1]);  resetTokenizer(); return; }

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

void Vt102Emulation::processToken(int token, int p, int q)
{
  switch (token)
  {
    case TY_CHR(         ) : _currentScreen->displayCharacter     (static_cast<unsigned short>(p) /* TODO */ ); break; //UTF16

    //             127 DEL    : ignored on input

    case TY_CTL('@'      ) : /* NUL: ignored                      */ break;
    case TY_CTL('A'      ) : /* SOH: ignored                      */ break;
    case TY_CTL('B'      ) : /* STX: ignored                      */ break;
    case TY_CTL('C'      ) : /* ETX: ignored                      */ break;
    case TY_CTL('D'      ) : /* EOT: ignored                      */ break;
    case TY_CTL('E'      ) :      reportAnswerBack     (          ); break; //VT100
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
    case TY_ESC('Z'      ) :      reportTerminalType   (          ); break;
    case TY_ESC('c'      ) :      reset                (          ); break;

    case TY_ESC('n'      ) :      useCharset           (         2); break;
    case TY_ESC('o'      ) :      useCharset           (         3); break;
    case TY_ESC('7'      ) :      saveCursor           (          ); break;
    case TY_ESC('8'      ) :      restoreCursor        (          ); break;

    case TY_ESC('='      ) :          setMode      (MODE_AppKeyPad); break;
    case TY_ESC('>'      ) :        resetMode      (MODE_AppKeyPad); break;
    case TY_ESC('<'      ) :          setMode      (MODE_Ansi     ); break; //VT100

    case TY_ESC_CS('(', '0') :      setCharset           (0,    '0'); break; //VT100
    case TY_ESC_CS('(', 'A') :      setCharset           (0,    'A'); break; //VT100
    case TY_ESC_CS('(', 'B') :      setCharset           (0,    'B'); break; //VT100

    case TY_ESC_CS(')', '0') :      setCharset           (1,    '0'); break; //VT100
    case TY_ESC_CS(')', 'A') :      setCharset           (1,    'A'); break; //VT100
    case TY_ESC_CS(')', 'B') :      setCharset           (1,    'B'); break; //VT100

    case TY_ESC_CS('*', '0') :      setCharset           (2,    '0'); break; //VT100
    case TY_ESC_CS('*', 'A') :      setCharset           (2,    'A'); break; //VT100
    case TY_ESC_CS('*', 'B') :      setCharset           (2,    'B'); break; //VT100

    case TY_ESC_CS('+', '0') :      setCharset           (3,    '0'); break; //VT100
    case TY_ESC_CS('+', 'A') :      setCharset           (3,    'A'); break; //VT100
    case TY_ESC_CS('+', 'B') :      setCharset           (3,    'B'); break; //VT100

    case TY_ESC_CS('%', 'G') :      /* TODO setCodec             (Utf8Codec   );*/ break; //LINUX
    case TY_ESC_CS('%', '@') :      /* TODO setCodec             (LocaleCodec );*/ break; //LINUX

    case TY_ESC_DE('3'      ) : /* Double height line, top half    */
                                _currentScreen->setLineProperty( LineProperty::DoubleWidth , true );
                                _currentScreen->setLineProperty( LineProperty::DoubleHeight , true );
                                    break;
    case TY_ESC_DE('4'      ) : /* Double height line, bottom half */
                                _currentScreen->setLineProperty( LineProperty::DoubleWidth , true );
                                _currentScreen->setLineProperty( LineProperty::DoubleHeight , true );
                                    break;
    case TY_ESC_DE('5'      ) : /* Single width, single height line*/
                                _currentScreen->setLineProperty( LineProperty::DoubleWidth , false);
                                _currentScreen->setLineProperty( LineProperty::DoubleHeight , false);
                                break;
    case TY_ESC_DE('6'      ) : /* Double width, single height line*/
                                _currentScreen->setLineProperty( LineProperty::DoubleWidth , true);
                                _currentScreen->setLineProperty( LineProperty::DoubleHeight , false);
                                break;
    case TY_ESC_DE('8'      ) : _currentScreen->helpAlign            (          ); break;

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
    case TY_CSI_PS('J',      3) : /* TODO clearHistory();*/                            break;
    case TY_CSI_PS('g',   0) : _currentScreen->changeTabStop        (false     ); break; //VT100
    case TY_CSI_PS('g',   3) : _currentScreen->clearTabStops        (          ); break; //VT100
    case TY_CSI_PS('h',   4) : _currentScreen->    setMode      (Mode::Insert   ); break;
    case TY_CSI_PS('h',  20) :          setMode      (Mode::NewLine  ); break;
    case TY_CSI_PS('i',   0) : /* IGNORE: attached printer          */ break; //VT100
    case TY_CSI_PS('l',   4) : _currentScreen->  resetMode      (Mode::Insert   ); break;
    case TY_CSI_PS('l',  20) :        resetMode      (Mode::NewLine  ); break;
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

    case TY_CSI_PS('n',   5) :      reportStatus         (          ); break;
    case TY_CSI_PS('n',   6) :      reportCursorPosition (          ); break;
    case TY_CSI_PS('q',   0) : /* IGNORED: LEDs off                 */ break; //VT100
    case TY_CSI_PS('q',   1) : /* IGNORED: LED1 on                  */ break; //VT100
    case TY_CSI_PS('q',   2) : /* IGNORED: LED2 on                  */ break; //VT100
    case TY_CSI_PS('q',   3) : /* IGNORED: LED3 on                  */ break; //VT100
    case TY_CSI_PS('q',   4) : /* IGNORED: LED4 on                  */ break; //VT100
    case TY_CSI_PS('x',   0) :      reportTerminalParms  (         2); break; //VT100
    case TY_CSI_PS('x',   1) :      reportTerminalParms  (         3); break; //VT100

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
    case TY_CSI_PN('c'      ) :      reportTerminalType   (          ); break; //VT100
    case TY_CSI_PN('d'      ) : _currentScreen->setCursorY           (p         ); break; //LINUX
    case TY_CSI_PN('f'      ) : _currentScreen->setCursorYX          (p,      q); break; //VT100
    case TY_CSI_PN('r'      ) :      setMargins           (p,      q); break; //VT100
    case TY_CSI_PN('y'      ) : /* IGNORED: Confidence test          */ break; //VT100

    case TY_CSI_PR('h',   1) :          setMode      (MODE_AppCuKeys); break; //VT100
    case TY_CSI_PR('l',   1) :        resetMode      (MODE_AppCuKeys); break; //VT100
    case TY_CSI_PR('s',   1) :         saveMode      (MODE_AppCuKeys); break; //FIXME
    case TY_CSI_PR('r',   1) :      restoreMode      (MODE_AppCuKeys); break; //FIXME

    case TY_CSI_PR('l',   2) :        resetMode      (MODE_Ansi     ); break; //VT100

    case TY_CSI_PR('h',   3) :          setMode      (MODE_132Columns); break; //VT100
    case TY_CSI_PR('l',   3) :        resetMode      (MODE_132Columns); break; //VT100

    case TY_CSI_PR('h',   4) : /* IGNORED: soft scrolling           */ break; //VT100
    case TY_CSI_PR('l',   4) : /* IGNORED: soft scrolling           */ break; //VT100

    case TY_CSI_PR('h',   5) : _currentScreen->    setMode      (Mode::Screen   ); break; //VT100
    case TY_CSI_PR('l',   5) : _currentScreen->  resetMode      (Mode::Screen   ); break; //VT100

    case TY_CSI_PR('h',   6) : _currentScreen->    setMode      (Mode::Origin   ); break; //VT100
    case TY_CSI_PR('l',   6) : _currentScreen->  resetMode      (Mode::Origin   ); break; //VT100
    case TY_CSI_PR('s',   6) : _currentScreen->   saveMode      (Mode::Origin   ); break; //FIXME
    case TY_CSI_PR('r',   6) : _currentScreen->restoreMode      (Mode::Origin   ); break; //FIXME

    case TY_CSI_PR('h',   7) : _currentScreen->    setMode      (Mode::Wrap     ); break; //VT100
    case TY_CSI_PR('l',   7) : _currentScreen->  resetMode      (Mode::Wrap     ); break; //VT100
    case TY_CSI_PR('s',   7) : _currentScreen->   saveMode      (Mode::Wrap     ); break; //FIXME
    case TY_CSI_PR('r',   7) : _currentScreen->restoreMode      (Mode::Wrap     ); break; //FIXME

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

    case TY_CSI_PR('h',  25) :          setMode      (Mode::Cursor   ); break; //VT100
    case TY_CSI_PR('l',  25) :        resetMode      (Mode::Cursor   ); break; //VT100
    case TY_CSI_PR('s',  25) :         saveMode      (Mode::Cursor   ); break; //VT100
    case TY_CSI_PR('r',  25) :      restoreMode      (Mode::Cursor   ); break; //VT100

    case TY_CSI_PR('h',  40) :         setMode(MODE_Allow132Columns ); break; // XTERM
    case TY_CSI_PR('l',  40) :       resetMode(MODE_Allow132Columns ); break; // XTERM

    case TY_CSI_PR('h',  41) : /* IGNORED: obsolete more(1) fix     */ break; //XTERM
    case TY_CSI_PR('l',  41) : /* IGNORED: obsolete more(1) fix     */ break; //XTERM
    case TY_CSI_PR('s',  41) : /* IGNORED: obsolete more(1) fix     */ break; //XTERM
    case TY_CSI_PR('r',  41) : /* IGNORED: obsolete more(1) fix     */ break; //XTERM

    case TY_CSI_PR('h',  47) :          setMode      (MODE_AppScreen); break; //VT100
    case TY_CSI_PR('l',  47) :        resetMode      (MODE_AppScreen); break; //VT100
    case TY_CSI_PR('s',  47) :         saveMode      (MODE_AppScreen); break; //XTERM
    case TY_CSI_PR('r',  47) :      restoreMode      (MODE_AppScreen); break; //XTERM

    case TY_CSI_PR('h',  67) : /* IGNORED: DECBKM                   */ break; //XTERM
    case TY_CSI_PR('l',  67) : /* IGNORED: DECBKM                   */ break; //XTERM
    case TY_CSI_PR('s',  67) : /* IGNORED: DECBKM                   */ break; //XTERM
    case TY_CSI_PR('r',  67) : /* IGNORED: DECBKM                   */ break; //XTERM

    // XTerm defines the following modes:
    // SET_VT200_MOUSE             1000
    // SET_VT200_HIGHLIGHT_MOUSE   1001
    // SET_BTN_EVENT_MOUSE         1002
    // SET_ANY_EVENT_MOUSE         1003
    //

    //Note about mouse modes:
    //There are four mouse modes which xterm-compatible terminals can support - 1000,1001,1002,1003
    //Konsole currently supports mode 1000 (basic mouse press and release) and mode 1002 (dragging the mouse).
    //TODO:  Implementation of mouse modes 1001 (something called hilight tracking) and
    //1003 (a slight variation on dragging the mouse)
    //

    case TY_CSI_PR('h', 1000) :          setMode      (MODE_Mouse1000); break; //XTERM
    case TY_CSI_PR('l', 1000) :        resetMode      (MODE_Mouse1000); break; //XTERM
    case TY_CSI_PR('s', 1000) :         saveMode      (MODE_Mouse1000); break; //XTERM
    case TY_CSI_PR('r', 1000) :      restoreMode      (MODE_Mouse1000); break; //XTERM

    case TY_CSI_PR('h', 1001) : /* IGNORED: hilite mouse tracking    */ break; //XTERM
    case TY_CSI_PR('l', 1001) :        resetMode      (MODE_Mouse1001); break; //XTERM
    case TY_CSI_PR('s', 1001) : /* IGNORED: hilite mouse tracking    */ break; //XTERM
    case TY_CSI_PR('r', 1001) : /* IGNORED: hilite mouse tracking    */ break; //XTERM

    case TY_CSI_PR('h', 1002) :          setMode      (MODE_Mouse1002); break; //XTERM
    case TY_CSI_PR('l', 1002) :        resetMode      (MODE_Mouse1002); break; //XTERM
    case TY_CSI_PR('s', 1002) :         saveMode      (MODE_Mouse1002); break; //XTERM
    case TY_CSI_PR('r', 1002) :      restoreMode      (MODE_Mouse1002); break; //XTERM

    case TY_CSI_PR('h', 1003) :          setMode      (MODE_Mouse1003); break; //XTERM
    case TY_CSI_PR('l', 1003) :        resetMode      (MODE_Mouse1003); break; //XTERM
    case TY_CSI_PR('s', 1003) :         saveMode      (MODE_Mouse1003); break; //XTERM
    case TY_CSI_PR('r', 1003) :      restoreMode      (MODE_Mouse1003); break; //XTERM

    case TY_CSI_PR('h',  1004) : _reportFocusEvents = true; break;
    case TY_CSI_PR('l',  1004) : _reportFocusEvents = false; break;

    case TY_CSI_PR('h', 1005) :          setMode      (MODE_Mouse1005); break; //XTERM
    case TY_CSI_PR('l', 1005) :        resetMode      (MODE_Mouse1005); break; //XTERM
    case TY_CSI_PR('s', 1005) :         saveMode      (MODE_Mouse1005); break; //XTERM
    case TY_CSI_PR('r', 1005) :      restoreMode      (MODE_Mouse1005); break; //XTERM

    case TY_CSI_PR('h', 1006) :          setMode      (MODE_Mouse1006); break; //XTERM
    case TY_CSI_PR('l', 1006) :        resetMode      (MODE_Mouse1006); break; //XTERM
    case TY_CSI_PR('s', 1006) :         saveMode      (MODE_Mouse1006); break; //XTERM
    case TY_CSI_PR('r', 1006) :      restoreMode      (MODE_Mouse1006); break; //XTERM

    case TY_CSI_PR('h', 1015) :          setMode      (MODE_Mouse1015); break; //URXVT
    case TY_CSI_PR('l', 1015) :        resetMode      (MODE_Mouse1015); break; //URXVT
    case TY_CSI_PR('s', 1015) :         saveMode      (MODE_Mouse1015); break; //URXVT
    case TY_CSI_PR('r', 1015) :      restoreMode      (MODE_Mouse1015); break; //URXVT

    case TY_CSI_PR('h', 1034) : /* IGNORED: 8bitinput activation     */ break; //XTERM

    case TY_CSI_PR('h', 1047) :          setMode      (MODE_AppScreen); break; //XTERM
    case TY_CSI_PR('l', 1047) : /*TODO _screen[1]*/_currentScreen->clearEntireScreen(); resetMode(MODE_AppScreen); break; //XTERM
    case TY_CSI_PR('s', 1047) :         saveMode      (MODE_AppScreen); break; //XTERM
    case TY_CSI_PR('r', 1047) :      restoreMode      (MODE_AppScreen); break; //XTERM

    //FIXME: Unitoken: save translations
    case TY_CSI_PR('h', 1048) :      saveCursor           (          ); break; //XTERM
    case TY_CSI_PR('l', 1048) :      restoreCursor        (          ); break; //XTERM
    case TY_CSI_PR('s', 1048) :      saveCursor           (          ); break; //XTERM
    case TY_CSI_PR('r', 1048) :      restoreCursor        (          ); break; //XTERM

    //FIXME: every once new sequences like this pop up in xterm.
    //       Here's a guess of what they could mean.
    case TY_CSI_PR('h', 1049) : saveCursor(); /*TODO _screen[1]*/_currentScreen->clearEntireScreen(); setMode(MODE_AppScreen); break; //XTERM
    case TY_CSI_PR('l', 1049) : resetMode(MODE_AppScreen); restoreCursor(); break; //XTERM

    case TY_CSI_PR('h', 2004) :          setMode      (MODE_BracketedPaste); break; //XTERM
    case TY_CSI_PR('l', 2004) :        resetMode      (MODE_BracketedPaste); break; //XTERM
    case TY_CSI_PR('s', 2004) :         saveMode      (MODE_BracketedPaste); break; //XTERM
    case TY_CSI_PR('r', 2004) :      restoreMode      (MODE_BracketedPaste); break; //XTERM

    //FIXME: weird DEC reset sequence
    case TY_CSI_PE('p'      ) : /* IGNORED: reset         (        ) */ break;

    //FIXME: when changing between vt52 and ansi mode evtl do some resetting.
    case TY_VT52('A'      ) : _currentScreen->cursorUp             (         1); break; //VT52
    case TY_VT52('B'      ) : _currentScreen->cursorDown           (         1); break; //VT52
    case TY_VT52('C'      ) : _currentScreen->cursorRight          (         1); break; //VT52
    case TY_VT52('D'      ) : _currentScreen->cursorLeft           (         1); break; //VT52

    case TY_VT52('F'      ) :      setAndUseCharset     (0,    '0'); break; //VT52
    case TY_VT52('G'      ) :      setAndUseCharset     (0,    'B'); break; //VT52

    case TY_VT52('H'      ) : _currentScreen->setCursorYX          (1,1       ); break; //VT52
    case TY_VT52('I'      ) : _currentScreen->reverseIndex         (          ); break; //VT52
    case TY_VT52('J'      ) : _currentScreen->clearToEndOfScreen   (          ); break; //VT52
    case TY_VT52('K'      ) : _currentScreen->clearToEndOfLine     (          ); break; //VT52
    case TY_VT52('Y'      ) : _currentScreen->setCursorYX          (p-31,q-31 ); break; //VT52
    case TY_VT52('Z'      ) :      reportTerminalType   (           ); break; //VT52
    case TY_VT52('<'      ) :          setMode      (MODE_Ansi     ); break; //VT52
    case TY_VT52('='      ) :          setMode      (MODE_AppKeyPad); break; //VT52
    case TY_VT52('>'      ) :        resetMode      (MODE_AppKeyPad); break; //VT52

    case TY_CSI_PG('c'      ) :  reportSecondaryAttributes(          ); break; //VT100

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

//#define CHARSET _charset[_currentScreen==_screen[1]]

// Apply current character map.

uc_t Vt102Emulation::applyCharset(uc_t  c)
{
    return c;
    // TODO
    //if (CHARSET.graphic && 0x5f <= c && c <= 0x7e) return vt100_graphics[c - 0x5f];
    //if (CHARSET.pound && c == '#') return 0xa3;  //This mode is obsolete
    //return c;
}

/*
   "Charset" related part of the emulation state.
   This configures the VT100 charset filter.

   While most operation work on the current _screen,
   the following two are different.
*/

void Vt102Emulation::resetCharset(int scrno)
{
    _charset[scrno].cu_cs = 0;
    strncpy(_charset[scrno].charset, "BBBB", 4);
    _charset[scrno].sa_graphic = false;
    _charset[scrno].sa_pound = false;
    _charset[scrno].graphic = false;
    _charset[scrno].pound = false;
}

void Vt102Emulation::setCharset(int n, int cs) // on both screens.
{
    _charset[0].charset[n & 3] = static_cast<char>(cs); useCharset(_charset[0].cu_cs);
    _charset[1].charset[n & 3] = static_cast<char>(cs); useCharset(_charset[1].cu_cs);
}

void Vt102Emulation::setAndUseCharset(int n, int /*cs*/)
{
    // TODO CHARSET.charset[n & 3] = cs;
    useCharset(n & 3);
}

void Vt102Emulation::useCharset(int /*n*/)
{
    // TODO CHARSET.cu_cs   = n & 3;
    // TODO CHARSET.graphic = (CHARSET.charset[n & 3] == '0');
    // TODO CHARSET.pound   = (CHARSET.charset[n & 3] == 'A'); //This mode is obsolete
}

void Vt102Emulation::setDefaultMargins()
{
    // TODO _screen[0]->setDefaultMargins();
    // TODO _screen[1]->setDefaultMargins();
    _currentScreen->setDefaultMargins();
}

void Vt102Emulation::setMargins(int t, int b)
{
    // TODO _screen[0]->setMargins(t, b);
    // TODO _screen[1]->setMargins(t, b);
    _currentScreen->setMargins(t, b);
}

void Vt102Emulation::saveCursor()
{
    // TODO CHARSET.sa_graphic = CHARSET.graphic;
    // TODO CHARSET.sa_pound   = CHARSET.pound; //This mode is obsolete
    // we are not clear about these
    //sa_charset = charsets[cScreen->_charset];
    //sa_charset_num = cScreen->_charset;
    _currentScreen->saveCursor();
}

void Vt102Emulation::restoreCursor()
{
    // TODO CHARSET.graphic = CHARSET.sa_graphic;
    // TODO CHARSET.pound   = CHARSET.sa_pound; //This mode is obsolete
    _currentScreen->restoreCursor();
}

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
    // MODE_Allow132Columns is not reset here
    // to match Xterm's behavior (see Xterm's VTReset() function)

    resetMode(MODE_132Columns); saveMode(MODE_132Columns);
    resetMode(MODE_Mouse1000);  saveMode(MODE_Mouse1000);
    resetMode(MODE_Mouse1001);  saveMode(MODE_Mouse1001);
    resetMode(MODE_Mouse1002);  saveMode(MODE_Mouse1002);
    resetMode(MODE_Mouse1003);  saveMode(MODE_Mouse1003);
    resetMode(MODE_Mouse1005);  saveMode(MODE_Mouse1005);
    resetMode(MODE_Mouse1006);  saveMode(MODE_Mouse1006);
    resetMode(MODE_Mouse1015);  saveMode(MODE_Mouse1015);
    resetMode(MODE_BracketedPaste);  saveMode(MODE_BracketedPaste);

    resetMode(MODE_AppScreen);  saveMode(MODE_AppScreen);
    resetMode(MODE_AppCuKeys);  saveMode(MODE_AppCuKeys);
    resetMode(MODE_AppKeyPad);  saveMode(MODE_AppKeyPad);
    resetMode(Mode::NewLine);
    setMode(MODE_Ansi);
}

void Vt102Emulation::setMode(int m)
{
    _currentModes.mode[m] = true;
    switch (m) {
    case MODE_132Columns:
        if (getMode(MODE_Allow132Columns))
            clearScreenAndSetColumns(132);
        else
            _currentModes.mode[m] = false;
        break;
    case MODE_Mouse1000:
    case MODE_Mouse1001:
    case MODE_Mouse1002:
    case MODE_Mouse1003:
        // TODO emit programUsesMouseChanged(false);
        break;

    case MODE_BracketedPaste:
        // TODO emit programBracketedPasteModeChanged(true);
        break;

    case MODE_AppScreen :
        // TODO _screen[1]->clearSelection();
        // TODO setScreen(1);
        break;
    }
    // FIXME: Currently this has a redundant condition as MODES_SCREEN is 6
    // and MODE_NewLine is 5
    if (m < MODES_SCREEN || static_cast<Mode::enum_t>(m) == Mode::NewLine) {
        // TODO _screen[0]->setMode(m);
        // TODO _screen[1]->setMode(m);
        _currentScreen->setMode(m);
    }
}

void Vt102Emulation::resetMode(int m)
{
    _currentModes.mode[m] = false;
    switch (m) {
    case MODE_132Columns:
        if (getMode(MODE_Allow132Columns))
            clearScreenAndSetColumns(80);
        break;
    case MODE_Mouse1000 :
    case MODE_Mouse1001 :
    case MODE_Mouse1002 :
    case MODE_Mouse1003 :
        // TODO emit programUsesMouseChanged(true);
        break;

    case MODE_BracketedPaste:
        // TODO emit programBracketedPasteModeChanged(false);
        break;

    case MODE_AppScreen :
        // TODO _screen[0]->clearSelection();
        // TODO setScreen(0);
        break;
    }
    // FIXME: Currently this has a redundant condition as MODES_SCREEN is 6
    // and MODE_NewLine is 5
    if (m < MODES_SCREEN || static_cast<Mode::enum_t>(m) == Mode::NewLine) {
        // TODO _screen[0]->resetMode(m);
        // TODO _screen[1]->resetMode(m);
        _currentScreen->resetMode(m);
    }
}

void Vt102Emulation::saveMode(int m)
{
    _savedModes.mode[m] = _currentModes.mode[m];
}

void Vt102Emulation::restoreMode(int m)
{
    if (_savedModes.mode[m])
        setMode(m);
    else
        resetMode(m);
}

bool Vt102Emulation::getMode(int m)
{
    return _currentModes.mode[m];
}

char Vt102Emulation::eraseChar() const
{
    // TODO
//     KeyboardTranslator::Entry entry = _keyTranslator->findEntry(
//                                           Qt::Key_Backspace,
//                                           0,
//                                           0);
//     if (entry.text().count() > 0)
//         return entry.text()[0];
//     else
//         return '\b';
    return '\0';
}

#if 0
// print contents of the scan buffer
static void hexdump(int* s, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        if (s[i] == '\\')
            printf("\\\\");
        else if ((s[i]) > 32 && s[i] < 127)
            printf("%c", s[i]);
        else
            printf("\\%04x(hex)", s[i]);
    }
}
#endif

// return contents of the scan buffer
static std::string hexdump2(int* s, int len)
{
    int i;
    char dump[128];
    std::string returnDump;

    for (i = 0; i < len; i++) {
        if (s[i] == '\\')
            snprintf(dump, sizeof(dump), "%s", "\\\\");
        else if ((s[i]) > 32 && s[i] < 127)
            snprintf(dump, sizeof(dump), "%c", s[i]);
        else
            snprintf(dump, sizeof(dump), "\\%04x(hex)", s[i]);
        returnDump.append(std::string(dump));
    }
    return returnDump;
}

void Vt102Emulation::reportDecodingError()
{
    if (tokenBufferPos == 0 || (tokenBufferPos == 1 && (tokenBuffer[0] & 0xff) >= 32))
        return;

//    printf("Undecodable sequence: ");
//    hexdump(tokenBuffer, tokenBufferPos);
//    printf("\n");

    std::string outputError("Undecodable sequence: ");
    outputError.append(hexdump2(tokenBuffer, tokenBufferPos));
    //qDebug() << outputError;
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

#ifndef IN_IDE_PARSER
# define BOOST_EQUAL_CHECK(a, b) Checker{__FILE__, __LINE__, #a " == " #b}.check_equal(a, b)
#else
# define BOOST_EQUAL_CHECK
#endif

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

template<int> struct i_{};

int main()
{
    rvt::Screen screen(40, 40);
    screen.cursorDown(1);
    BOOST_EQUAL_CHECK(screen.cursor_x(), 0);
    BOOST_EQUAL_CHECK(screen.cursor_y(), 1);
    screen.cursorDown(1);
    BOOST_EQUAL_CHECK(screen.cursor_x(), 0);
    BOOST_EQUAL_CHECK(screen.cursor_y(), 2);
    screen.cursorDown(2);
    BOOST_EQUAL_CHECK(screen.cursor_x(), 0);
    BOOST_EQUAL_CHECK(screen.cursor_y(), 4);
    screen.cursorDown(40);
    BOOST_EQUAL_CHECK(screen.cursor_x(), 0);
    BOOST_EQUAL_CHECK(screen.cursor_y(), 39);
    screen.cursorUp(10);
    BOOST_EQUAL_CHECK(screen.cursor_x(), 0);
    BOOST_EQUAL_CHECK(screen.cursor_y(), 29);
    screen.cursorUp(40);
    BOOST_EQUAL_CHECK(screen.cursor_x(), 0);
    BOOST_EQUAL_CHECK(screen.cursor_y(), 0);
    screen.cursorRight(2);
    BOOST_EQUAL_CHECK(screen.cursor_x(), 2);
    BOOST_EQUAL_CHECK(screen.cursor_y(), 0);
    screen.cursorRight(200);
    BOOST_EQUAL_CHECK(screen.cursor_x(), 39);
    BOOST_EQUAL_CHECK(screen.cursor_y(), 0);
    screen.cursorLeft(3);
    BOOST_EQUAL_CHECK(screen.cursor_x(), 36);
    BOOST_EQUAL_CHECK(screen.cursor_y(), 0);
    screen.cursorLeft(40);
    BOOST_EQUAL_CHECK(screen.cursor_x(), 0);
    BOOST_EQUAL_CHECK(screen.cursor_y(), 0);
    screen.cursorRight(1);
    BOOST_EQUAL_CHECK(screen.cursor_x(), 1);
    BOOST_EQUAL_CHECK(screen.cursor_y(), 0);
    screen.cursorLeft(2);
    BOOST_EQUAL_CHECK(screen.cursor_x(), 0);
    BOOST_EQUAL_CHECK(screen.cursor_y(), 0);

    rvt::Vt102Emulation emulator(&screen);
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
    emulator.receiveChar(touc("é"));
    emulator.receiveChar('e');
    emulator.receiveChar(0xcc82);
    int i = 0;
    for (auto const & line : screen.getScreenLines()) {
        std::cout << std::setw(4) << ++i << " ";
        for (rvt::Character const & ch : line) {
            std::cout << char(ch.character ? ch.character : '?');
        }
        std::cout << '\n';
    }

    const rvt::Color color_table[] = {
        rvt::Color(0x00, 0x00, 0x00), // Dfore
        rvt::Color(0xFF, 0xFF, 0xFF), // Dback
        rvt::Color(0x00, 0x00, 0x00), // Black
        rvt::Color(0xB2, 0x18, 0x18), // Red
        rvt::Color(0x18, 0xB2, 0x18), // Green
        rvt::Color(0xB2, 0x68, 0x18), // Yellow
        rvt::Color(0x18, 0x18, 0xB2), // Blue
        rvt::Color(0xB2, 0x18, 0xB2), // Magenta
        rvt::Color(0x18, 0xB2, 0xB2), // Cyan
        rvt::Color(0xB2, 0xB2, 0xB2), // White
        // intensive versions
        rvt::Color(0x00, 0x00, 0x00),
        rvt::Color(0xFF, 0xFF, 0xFF),
        rvt::Color(0x68, 0x68, 0x68),
        rvt::Color(0xFF, 0x54, 0x54),
        rvt::Color(0x54, 0xFF, 0x54),
        rvt::Color(0xFF, 0xFF, 0x54),
        rvt::Color(0x54, 0x54, 0xFF),
        rvt::Color(0xFF, 0x54, 0xFF),
        rvt::Color(0x54, 0xFF, 0xFF),
        rvt::Color(0xFF, 0xFF, 0xFF),
    };
    static_assert(sizeof(color_table) / sizeof(color_table[0]) == rvt::TABLE_COLORS, "");

    auto print_color = [&color_table](char const * cmd, rvt::CharacterColor const & ch_color) {
        auto color = ch_color.color(color_table);
        std::cout << cmd << (color.red()+0) << ";" << (color.green()+0) << ";" << (color.blue()+0);
    };
    auto print_mode = [](char const * cmd, rvt::Character const & ch, rvt::Rendition r) {
        if (bool(ch.rendition & r)) {
            std::cout << cmd;
        }
    };
    auto print_uc = [](rvt::uc_t uc) {
        if (uc & 0xff00) {
            std::cout << uchar(uc >> 8);
        }
        std::cout << uchar(uc);
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
            if (ch.is_extended()) {
                for (rvt::uc_t uc : screen.extendedCharTable()[ch.character]) {
                    print_uc(uc);
                }
            }
            else {
                print_uc(ch.character);
            }
        }
        std::cout << '\n';
    }
}
