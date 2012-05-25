#if !defined(__KEYMAPSYM__)
#define __KEYMAPSYM__

#include <stdio.h>
#include "constants.hpp"
#include <string.h>
#include <string>

using namespace std;

struct KeymapSym {

    enum {
           KBDFLAGS_EXTENDED = 0x0100
         , KBDFLAGS_DOWN     = 0x4000
         , KBDFLAGS_RELEASE  = 0x8000
         };


    enum {
           SCROLLLOCK  = 0x01
         , NUMLOCK     = 0x02
         , CAPSLOCK    = 0x04
         , FLG_SHIFT   = 0x08
         , FLG_CTRL    = 0x10
         , FLG_ALT     = 0x20
         , FLG_WINDOWS = 0x40
         , FLG_ALTGR   = 0x80
    };

    enum {
           LEFT_SHIFT  = 0x36
         , RIGHT_SHIFT = 0x2A
         , LEFT_CTRL   = 0x1D
         , RIGHT_CTRL  = 0x9D
         , LEFT_ALT    = 0x38
         , RIGHT_ALT   = 0xB8
    };

    // keyboard info
    int keys_down[256];  // key states 0 up 1 down (0..127 plain keys, 128..255 extended keys)

    int key_flags;          // scroll_lock = 1, num_lock = 2, caps_lock = 4,
                            // shift = 8, ctrl = 16, Alt = 32,
                            // Windows = 64, AltGr = 128

    enum {
        SIZE_KEYBUF = 20
    };

    enum {
        SIZE_KEYBUF_SYM = 20
    };

    enum {
        SIZE_KEYBUF_KEVENT = 20
    };

    enum {
        KEVENT_KEY,
        KEVENT_TAB,
        KEVENT_BACKTAB,
        KEVENT_ENTER,
        KEVENT_ESC,
        KEVENT_DELETE,
        KEVENT_BACKSPACE,
        KEVENT_LEFT_ARROW,
        KEVENT_RIGHT_ARROW,
        KEVENT_UP_ARROW,
        KEVENT_DOWN_ARROW,
        KEVENT_HOME,
        KEVENT_END,
        KEVENT_PGUP,
        KEVENT_PGDOWN,
    };

    uint32_t ibuf; // first free position in char buffer
    uint32_t nbuf; // number of char in char buffer
    uint32_t buffer[SIZE_KEYBUF]; // actual char buffer

    uint32_t ibuf_sym; // first free position
    uint32_t nbuf_sym; // number of char in char buffer
    uint32_t buffer_sym[SIZE_KEYBUF_SYM]; // actual char buffer

    uint32_t ibuf_kevent; // first free position
    uint32_t nbuf_kevent; // number of char in char buffer
    uint32_t buffer_kevent[SIZE_KEYBUF_KEVENT]; // actual char buffer

    uint8_t dead_key;

    enum {
        DEADKEY_NONE,
        DEADKEY_CIRC,
        DEADKEY_UML,
        DEADKEY_GRAVE
    };


    uint32_t verbose;
//    uint32_t last_char_key;
//    uint32_t last_char_key_sym;

//    int last_chr_unicode;
    int last_sym;

    typedef int KeyLayout_t[128];

    // keylayout working tables (X11 mode : begins in 8e position.)
    KeyLayout_t keylayout_WORK_noshift_sym;
    KeyLayout_t keylayout_WORK_shift_sym;
    KeyLayout_t keylayout_WORK_altgr_sym;
    KeyLayout_t keylayout_WORK_capslock_sym;
    KeyLayout_t keylayout_WORK_shiftcapslock_sym;

    // constructor
// ################################################################
    KeymapSym(int verbose = 0)
    : ibuf(0)
    , nbuf(0)
    , ibuf_kevent(0)
    , nbuf_kevent(0)
    , dead_key(DEADKEY_NONE)
    , verbose(verbose)
    {
// ################################################################
        memset(this->keys_down, 0, 256 * sizeof(int));

        memset(&this->keylayout_WORK_noshift_sym,       0, 128 * sizeof(int));
        memset(&this->keylayout_WORK_shift_sym,         0, 128 * sizeof(int));
        memset(&this->keylayout_WORK_altgr_sym,         0, 128 * sizeof(int));
        memset(&this->keylayout_WORK_capslock_sym,      0, 128 * sizeof(int));
        memset(&this->keylayout_WORK_shiftcapslock_sym, 0, 128 * sizeof(int));

        this->key_flags = 0;
//        this->last_chr_unicode = 0;
        this->last_sym = 0;
    }

    void synchronize(uint16_t param1){
        this->key_flags = param1 & 0x07;
        // non sticky keys are forced to be UP
        this->keys_down[LEFT_SHIFT] = 0;
        this->keys_down[RIGHT_SHIFT] = 0;
        this->keys_down[LEFT_CTRL] = 0;
        this->keys_down[RIGHT_CTRL] = 0;
        this->keys_down[LEFT_ALT] = 0;
        this->keys_down[RIGHT_ALT] = 0;
    }

// ################################################################

// The TS_KEYBOARD_EVENT structure is a standard T.128 Keyboard Event (see [T128] section
// 8.18.2). RDP keyboard input is restricted to keyboard scancodes, unlike the code-point or virtual
// codes supported in T.128 (a scancode is an 8-bit value specifying a key location on the keyboard).
// The server accepts a scancode value and translates it into the correct character depending on the
// language locale and keyboard layout used in the session.

// keyboardFlags (2 bytes): A 16-bit, unsigned integer. The flags describing the keyboard event.

// +--------------------------+------------------------------------------------+
// | 0x0100 KBDFLAGS_EXTENDED | The keystroke message contains an extended     |
// |                          | scancode. For enhanced 101-key and 102-key     |
// |                          | keyboards, extended keys include the right     |
// |                          | ALT and right CTRL keys on the main section    |
// |                          | of the keyboard; the INS, DEL, HOME, END,      |
// |                          | PAGE UP, PAGE DOWN and ARROW keys in the       |
// |                          | clusters to the left of the numeric keypad;    |
// |                          | and the Divide ("/") and ENTER keys in the     |
// |                          | numeric keypad.                                |
// +--------------------------+------------------------------------------------+
// | 0x4000 KBDFLAGS_DOWN     | Indicates that the key was down prior to this  |
// |                          | event.                                         |
// +--------------------------+------------------------------------------------+
// | 0x8000 KBDFLAGS_RELEASE  | The absence of this flag indicates a key-down  |
// |                          | event, while its presence indicates a          |
// |                          | key-release event.                             |
// +--------------------------+------------------------------------------------+

// keyCode (2 bytes): A 16-bit, unsigned integer. The scancode of the key which
// triggered the event.

    void event(const uint16_t keyboardFlags, const uint16_t keyCode)
    {
        // The scancode and its extended nature are merged in a new variable (whose most significant bit indicates the extended nature)
        uint8_t extendedKeyCode = keyCode|((keyboardFlags >> 1)&0x80);
        // The state of that key is updated in the Keyboard status array (1=Make ; 0=Break)
        this->keys_down[extendedKeyCode] = !(keyboardFlags & KBDFLAGS_RELEASE);

printf("\n======\nENTREE - keycode = %#x - extendedKeyCode = %#x\n", keyCode, extendedKeyCode);

        switch (extendedKeyCode){
        //================
        // Lock keys
        //================
            case 0x3A: // capslock
                if (this->keys_down[extendedKeyCode]){
                    this->key_flags ^= CAPSLOCK;
                }
                break;
            case 0x45: // numlock
                if (this->keys_down[extendedKeyCode]){
                    this->key_flags ^= NUMLOCK;
                }
                break;
            case 0x46: // scrolllock
                if (this->keys_down[extendedKeyCode]){
                    this->key_flags ^= SCROLLLOCK;
                }
                break;
        //================
        // Modifier keys
        //================
            case LEFT_SHIFT:   // left shift
            case RIGHT_SHIFT: // right shift
            case LEFT_CTRL:   // left ctrl
            case RIGHT_CTRL:  // right ctrl
            case LEFT_ALT:    // left alt
            case RIGHT_ALT:   // right alt
                break;
        //================
        // All other keys
        //================
            default: // all other codes
                // This table translates the RDP scanodes to X11 scandodes :
                //  - the fist block (0-127) simply applies the +8 Windows to X11 translation and forces some 0 values
                //  - the second block (128-255) give codes for the extended keys that have a meaningful one
                uint8_t map[256] =  {
                    0x00, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, // 0x00 - 0x07
                    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, // 0x08 - 0x0f
                    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, // 0x10 - 0x17
                    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, // 0x18 - 0x1f
                    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, // 0x20 - 0x27
                    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, // 0x28 - 0x2f
                    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, // 0x30 - 0x37
                    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, // 0x38 - 0x3f
                    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, // 0x40 - 0x47
                    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, // 0x48 - 0x4f
                    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, // 0x50 - 0x57
                    0x60, 0x61, 0x62, 0x00, 0x00, 0x00, 0x66, 0x67, // 0x58 - 0x5f
                    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, // 0x60 - 0x67
                    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, // 0x68 - 0x6f
                    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, // 0x70 - 0x77
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x78 - 0x7f

                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80 - 0x87
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x88 - 0x8f
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90 - 0x97
                    0x00, 0x00, 0x00, 0x00, 0x6c, 0x6d, 0x00, 0x00, // 0x98 - 0x9f
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xa0 - 0xa7
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xa8 - 0xaf
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x6f, // 0xb0 - 0xb7
                    0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xb8 - 0xbf
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, // 0xc0 - 0xc7
                    0x62, 0x63, 0x00, 0x64, 0x00, 0x66, 0x00, 0x67, // 0xc8 - 0xcf
                    0x68, 0x69, 0x6a, 0x6b, 0x00, 0x00, 0x00, 0x00, // 0xd0 - 0xd7
                    0x00, 0x00, 0x00, 0x73, 0x74, 0x75, 0x00, 0x00, // 0xd8 - 0xdf
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xe0 - 0xe7
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xe8 - 0xef
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xf0 - 0xf7
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // 0xf8 - 0xff
                } ;

                // if event is a Make
                if (this->keys_down[extendedKeyCode]){
printf("        MAKE- extendedKeyCode = %#x\n",extendedKeyCode);
                    if (this->verbose){
                        LOG(LOG_INFO, "Event is Make for key: %#x", extendedKeyCode);
                    }
                        const KeyLayout_t * layout = &keylayout_WORK_noshift_sym;
//                        this->last_char_key = extendedKeyCode;

                    //=========================================================================
                    // KEYPAD : Keypad keys whose meanings depends on Numlock are handled apart
                    //=========================================================================
                    if ( ( (extendedKeyCode >= 0x47) && (extendedKeyCode <= 0x49) )
                      || ( (extendedKeyCode >= 0x4b) && (extendedKeyCode <= 0x4d) )
                      || ( (extendedKeyCode >= 0x4f) && (extendedKeyCode <= 0x53) )
                       ){
                        if (this->verbose){
                            LOG(LOG_INFO, "Key from keypad: %#x", extendedKeyCode);
                        }
                        // if numlock is activated, keys are printable characters (logical SHIFT mode)
                        if ((this->key_flags & NUMLOCK)) {
                            layout = &this->keylayout_WORK_shift_sym;
                            // Translate the scancode to an unicode char
                            uint8_t sym = map[extendedKeyCode];
                            uint32_t uchar = (*layout)[sym];
//printf("UCHAR = %#x", uchar);
//                           this->push(uchar);
                            uint32_t ksym = (*layout)[sym];
                            this->push_sym(ksym);
                        }
                        // if numlock is not activated, keys are NOT printable characters (logical NO SHIFT mode)
//                        else {
//                            switch (extendedKeyCode){
//                               /* kEYPAD LEFT ARROW */
//                                case 0x4b:
//                                     this->push_kevent(KEVENT_LEFT_ARROW);
//                                    break;
//                                /* kEYPAD UP ARROW */
//                                case 0x48:
//                                    this->push_kevent(KEVENT_UP_ARROW);
//                                    break;
//                                /* kEYPAD RIGHT ARROW */
//                                case 0x4d:
//                                    this->push_kevent(KEVENT_RIGHT_ARROW);
//                                    break;
//                                /* kEYPAD DOWN ARROW */
//                                case 0x50:
//                                    this->push_kevent(KEVENT_DOWN_ARROW);
//                                    break;
//                                /* kEYPAD HOME */
//                                case 0x47:
//                                    this->push_kevent(KEVENT_HOME);
//                                    break;
//                                /* kEYPAD PGUP */
//                                case 0x49:
//                                    this->push_kevent(KEVENT_PGUP);
//                                    break;
//                                /* kEYPAD PGDOWN */
//                                case 0x51:
//                                    this->push_kevent(KEVENT_PGDOWN);
//                                    break;
//                                /* kEYPAD END */
//                                case 0x4F:
//                                    this->push_kevent(KEVENT_END);
//                                    break;
//                                /* kEYPAD DELETE */
//                                case 0x53:
//                                    this->push_kevent(KEVENT_DELETE);
//                                default:
//                                    break;
//                            }
//                        }
                    }

                    //========================================
                    // NOT KEYPAD Specific
                    //========================================
                    else {
                        if (this->verbose){
                            LOG(LOG_INFO, "Key not from keypad: %#x", extendedKeyCode);
                        }
                        if (this->is_ctrl_pressed() && this->is_alt_pressed()){
                            layout = &this->keylayout_WORK_altgr_sym;
                        }
                        else if (this->is_shift_pressed() && this->is_caps_locked()){
                            layout = &this->keylayout_WORK_shiftcapslock_sym;
                        }
                        else if (this->is_shift_pressed()){
                            layout = &this->keylayout_WORK_shift_sym;
                        }
                        else if (this->is_caps_locked()) {
                            layout = &this->keylayout_WORK_capslock_sym;
                        }
                        // Translate the scancode to an unicode char
                        uint8_t sym = map[extendedKeyCode];
                        uint32_t uchar = (*layout)[sym];
                        uint32_t ksym = (*layout)[sym];
                        if (this->verbose){
                            LOG(LOG_INFO, "uchar=%x", uchar);
                        }
                        //==============================================
                        // uchar is in Printable unicode character range.
                        //==============================================
                        // That is :
                        //  * > 0x20 is for ruling out NUL, but also TAB, ESC and BACKSPACE that has unicode values but
                        //           are not actually printable characters and that we don't want to track
                        //  * And not delete (0x7f) nor a dead key (0x5e, 0xa8, 0x60)
                        if (this->verbose){
                            LOG(LOG_INFO, "nbevent in buffer: %u %u\n", this->nbuf, this->nbuf_kevent);
                        }
                        if ((uchar >= 0x20) && (uchar != 0x7F) && (uchar != 0x5E) && (uchar != 0xA8) && (uchar != 0x60)){
                            if (this->verbose){
                                LOG(LOG_INFO, "Printable key : uchar=%x", uchar);
                            }
                            // If previous key was a dead key, push a translated unicode char
                            if (this->dead_key != DEADKEY_NONE){
                                if (this->verbose){
                                    LOG(LOG_INFO, "Dead key : uchar=%x", uchar);
                                }
                                switch (dead_key){
                                case DEADKEY_CIRC:
                                    switch (uchar){
                                    case 'a':
                                        this->push_sym(0xE2); // unicode for â (acirc)
                                        break;
                                    case 'A':
                                        this->push_sym(0xC2); // unicode for Â (Acirc)
                                        break;
                                    case 'e':
                                        this->push_sym(0xEA); // unicode for ê (ecirc)
                                        break;
                                    case 'E':
                                        this->push_sym(0xCA); // unicode for Ê (Ecirc)
                                        break;
                                    case 'i':
                                        this->push_sym(0xEE); // unicode for î (icirc)
                                        break;
                                    case 'I':
                                        this->push_sym(0xCE); // unicode for Î (Icirc)
                                        break;
                                    case 'o':
                                        this->push_sym(0xF4); // unicode for ô (ocirc)
                                        break;
                                    case 'O':
                                        this->push_sym(0xD4); // unicode for Ô (Ocirc)
                                        break;
                                    case 'u':
                                        this->push_sym(0xFB); // unicode for û (ucirc)
                                        break;
                                    case 'U':
                                        this->push_sym(0xDB); // unicode for Û (Ucirc)
                                        break;
                                    case ' ':
                                        this->push_sym(0x5E); // unicode for ^ (caret)
                                        break;
                                    default:
                                        this->push_sym(uchar); // unmodified unicode
                                        break;
                                    }
                                break;
                                case DEADKEY_UML:
                                    switch (uchar){
                                    case 'a':
                                        this->push_sym(0xE4); // unicode for ä (auml)
                                        break;
                                    case 'A':
                                        this->push_sym(0xC4); // unicode for Ä (Auml)
                                        break;
                                    case 'e':
                                        this->push_sym(0xEB); // unicode for ë (euml)
                                        break;
                                    case 'E':
                                        this->push_sym(0xCB); // unicode for Ë (Euml)
                                        break;
                                    case 'i':
                                        this->push_sym(0xEF); // unicode for ï (iuml)
                                        break;
                                    case 'I':
                                        this->push_sym(0xCF); // unicode for Ï (Iuml)
                                        break;
                                    case 'o':
                                        this->push_sym(0xF6); // unicode for ö (ouml)
                                        break;
                                    case 'O':
                                        this->push_sym(0xD6); // unicode for Ö (Ouml)
                                        break;
                                    case 'u':
                                        this->push_sym(0xFC); // unicode for ü (uuml)
                                        break;
                                    case 'U':
                                        this->push_sym(0xDC); // unicode for Ü (Uuml)
                                        break;
                                    case ' ':
                                        this->push_sym(0xA8); // unicode for " (umlaut)
                                        break;
                                    default:
                                        this->push_sym(uchar); // unmodified unicode
                                        break;
                                    }
                                break;
                                case DEADKEY_GRAVE:
                                    switch (uchar){
                                    case 'a':
                                        this->push_sym(0xE0); // unicode for à (agrave)
                                        break;
                                    case 'A':
                                        this->push_sym(0xC0); // unicode for À (Agrave)
                                        break;
                                    case 'e':
                                        this->push_sym(0xE8); // unicode for è (egrave)
                                        break;
                                    case 'E':
                                        this->push_sym(0xC8); // unicode for È (Egrave)
                                        break;
                                    case 'i':
                                        this->push_sym(0xEC); // unicode for ì (igrave)
                                        break;
                                    case 'I':
                                        this->push_sym(0xCC); // unicode for Ì (Igrave)
                                        break;
                                    case 'o':
                                        this->push_sym(0xF2); // unicode for ò (ograve)
                                        break;
                                    case 'O':
                                        this->push_sym(0xD2); // unicode for Ò (Ograve)
                                        break;
                                    case 'u':
                                        this->push_sym(0xF9); // unicode for ù (ugrave)
                                        break;
                                    case 'U':
                                        this->push_sym(0xD9); // unicode for Ù (Ugrave)
                                        break;
                                    case ' ':
                                        this->push_sym(0x60); // unicode for ` (backslash)
                                        break;
                                    default:
                                        this->push_sym(uchar); // unmodified unicode
                                        break;
                                    }
                                break;
                                default:
                                    this->push_sym(uchar); // unmodified unicode
                                break;
                                }
                                this->dead_key = DEADKEY_NONE;
                            }
                            // If previous key wasn't a dead key, simply push
                            else {
                                if (this->verbose){
                                    LOG(LOG_INFO, "not dead key - so pushing char %02x", uchar);
                                }
                                this->push_sym(uchar);
                            }
                        }
                        //=================================================
                        // uchar is NOT in Printable unicode character range
                        //=================================================
                        else {
                            if (this->verbose){
                                LOG(LOG_INFO, "pushing event extendedKeyCode=%x", extendedKeyCode);
                            }
                            switch (extendedKeyCode){
                            case 0x1A:
                                this->is_shift_pressed() ? this->dead_key = DEADKEY_UML : this->dead_key = DEADKEY_CIRC;
                            break;
                            case 0x08:
                                this->dead_key = DEADKEY_GRAVE;
                                break;
//                            /* LEFT ARROW */
//                            case 0xCB:
//                                this->push_kevent(KEVENT_LEFT_ARROW);
//                                break;
//                            /* UP ARROW */
//                            case 0xC8:
//                                this->push_kevent(KEVENT_UP_ARROW);
//                                break;
//                            /* RIGHT ARROW */
//                            case 0xCD:
//                                this->push_kevent(KEVENT_RIGHT_ARROW);
//                                break;
//                            /* DOWN ARROW */
//                            case 0xD0:
//                                this->push_kevent(KEVENT_DOWN_ARROW);
//                                break;
//                            /* HOME */
//                            case 0xC7:
//                                this->push_kevent(KEVENT_HOME);
//                                break;
//                            /* PGUP */
//                            case 0xC9:
//                                this->push_kevent(KEVENT_PGUP);
//                                break;
//                            /* PGDOWN */
//                            case 0xD1:
//                                this->push_kevent(KEVENT_PGDOWN);
//                                break;
//                            /* END */
//                            case 0xCF:
//                                this->push_kevent(KEVENT_END);
//                                break;
//                             /* TAB */
//                            case 0x0F:
//                                if (this->is_shift_pressed()){
//                                    this->push_kevent(KEVENT_BACKTAB);
//                                }
//                                else {
//                                    this->push_kevent(KEVENT_TAB);
//                                }
//                                break;
//                             /* backspace */
//                            case 0x0E:
//                                this->push_kevent(KEVENT_BACKSPACE);
//                                break;
//                            case 0xD3: // delete
//                                this->push_kevent(KEVENT_DELETE);
//                                break;
//                            case 0x53: // numpad delete
//                                this->push_kevent(KEVENT_DELETE);
//                                break;
//                            case 0x1C: // enter
//                                this->push_kevent(KEVENT_ENTER);
//                                break;
//                            case 0x9C: // numpad enter
//                                this->push_kevent(KEVENT_ENTER);
//                                break;
                            default:
                                break;
                            }
                        } // END if PRINTABLE / else
                    } // END if KEYPAD specific / else
                } // END if Keydown
                else {
printf("        BREAK- extendedKeyCode = %#x\n", extendedKeyCode);
                }
            break;

        } // END SWITCH : ExtendedKeyCode

    } // END FUNCT : event

    // Push all
//    void push(uint32_t sym)
//    {
//        this->push_sym(sym);
//        this->push_kevent(KEVENT_KEY);
//    }

    // Push only sym
    void push_sym(uint32_t sym)
    {
        if (this->nbuf_sym < SIZE_KEYBUF_SYM){
            this->buffer_sym[this->ibuf_sym] = sym;
            this->ibuf_sym++;
            if (this->ibuf_sym >= SIZE_KEYBUF_SYM){
                this->ibuf_sym = 0;
            }
            this->nbuf_sym++;
        }
    }

    uint32_t get_sym()
    {
        if (this->nbuf_sym > 0){
//            // remove top KEY KEVENT if present and any event may have occured before it
//            if (this->nbuf_kevent > 0 && this->top_kevent() == KEVENT_KEY){
//                this->nbuf_kevent--;
//            }
            uint32_t res = this->buffer_sym[(SIZE_KEYBUF_SYM + this->ibuf_sym - this->nbuf_sym) % SIZE_KEYBUF_SYM];

            if (this->nbuf_sym > 0){
                this->nbuf_sym--;
            }
            return res;
        }
        return 0;
    }

    // head of keyboard buffer (or keyboard buffer of size 1)
    uint32_t top_sym() const
    {
        return this->buffer_sym[this->ibuf_sym?this->ibuf_sym-1:SIZE_KEYBUF_SYM-1];
    }

    uint32_t nb_sym_available() const
    {
        return this->nbuf_sym;
    }

//    void push_kevent(uint32_t uevent)
//    {
//        if (this->nbuf_kevent < SIZE_KEYBUF_KEVENT){
//            this->buffer_kevent[this->ibuf_kevent] = uevent;
//            this->ibuf_kevent++;
//            if (this->ibuf_kevent >= SIZE_KEYBUF_KEVENT){
//                this->ibuf_kevent = 0;
//            }
//            this->nbuf_kevent++;
//        }
//    }

//    uint32_t get_kevent()
//    {
//        uint32_t res = this->buffer_kevent[(SIZE_KEYBUF_KEVENT + this->ibuf_kevent - this->nbuf_kevent) % SIZE_KEYBUF_KEVENT];

//        if (this->nbuf_kevent > 0){
//            if (res == KEVENT_KEY && this->nbuf > 0){
//                    this->nbuf--;
//            }
//            this->nbuf_kevent--;
//        }
//        return res;
//    }

//    // head of keyboard buffer (or keyboard buffer of size 1)
//    uint32_t top_kevent() const
//    {
//        return this->buffer_kevent[this->ibuf_kevent?this->ibuf_kevent-1:SIZE_KEYBUF_KEVENT-1];
//    }

//    uint32_t nb_kevent_available() const
//    {
//        return this->nbuf_kevent;
//    }

    bool is_caps_locked() const
    {
        return this->key_flags & CAPSLOCK;
    }

    bool is_scroll_locked() const
    {
        return this->key_flags & SCROLLLOCK;
    }

    bool is_num_locked() const
    {
        return this->key_flags & NUMLOCK;
    }

    bool is_left_shift_pressed() const
    {
        return this->keys_down[LEFT_SHIFT];
    }

    bool is_right_shift_pressed() const
    {
        return this->keys_down[RIGHT_SHIFT];
    }

    bool is_shift_pressed() const
    {
        return this->is_left_shift_pressed() || this->is_right_shift_pressed();
    }

    bool is_left_ctrl_pressed() const
    {
        return this->keys_down[LEFT_CTRL];
    }

    bool is_right_ctrl_pressed() const
    {
        return this->keys_down[RIGHT_CTRL];
    }

    bool is_ctrl_pressed() const
    {
        return is_right_ctrl_pressed() || is_left_ctrl_pressed();
    }

    bool is_left_alt_pressed() const
    {
        return this->keys_down[LEFT_ALT];
    }

    bool is_right_alt_pressed() const // altgr
    {
        return this->keys_down[RIGHT_ALT];
    }

    bool is_alt_pressed() const
    {
        return is_right_alt_pressed() || is_left_alt_pressed();
    }


// ################################################################
    void init_layout_sym(int keyb)
    {
        // %s/^[^"]*"[^"]*"[^"]*"\([^:]*\):.*$/\1/

        // DEFAULT KEYMAP
        const KeyLayout_t DEFAULT_noshift_sym = {
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                0x37,    0x38,    0x39,    0x30,    0xdf,  0xfe51,  0xff08,  0xff09,
                0x71,    0x77,    0x65,    0x72,    0x74,    0x7a,    0x75,    0x69,
                0x6f,    0x70,    0xfc,    0x2b,  0xff0d,  0xffe3,    0x61,    0x73,
                0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0xf6,
                0xe4,  0xfe52,  0xffe1,    0x23,    0x79,    0x78,    0x63,    0x76,
                0x62,    0x6e,    0x6d,    0x2c,    0x2e,    0x2d,  0xffe2,  0xffaa,
              0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
              0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
              0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
              0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x3c,  0xffc8,
              0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
              0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
              0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
//            0, 0, 0, 0, 0, 0, 0, 0,
//            0, 27, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 223, 180, 0, 0, 113, 119, 101, 114,
//            116, 122, 117, 105, 111, 112, 252, 43, 13, 0, 97, 115, 100, 102, 103, 104, 106, 107, 108, 246,
//            228, 94, 0, 35, 121, 120, 99, 118, 98, 110, 109, 44, 46, 45, 0, 42, 0, 32, 0, 0,
//            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45, 0, 0, 0, 43, 0,
//            0, 0, 0, 0, 0, 0, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//            13, 0, 0, 0, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 61, 0,
        } ;

        const KeyLayout_t DEFAULT_shift_sym = {
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,  0xff1b,    0x21,    0x22,    0xa7,    0x24,    0x25,    0x26,
                0x2f,    0x28,    0x29,    0x3d,    0x3f,  0xfe50,  0xff08,  0xfe20,
                0x51,    0x57,    0x45,    0x52,    0x54,    0x5a,    0x55,    0x49,
                0x4f,    0x50,    0xdc,    0x2a,  0xff0d,  0xffe3,    0x41,    0x53,
                0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0xd6,
                0xc4,    0xb0,  0xffe1,    0x27,    0x59,    0x58,    0x43,    0x56,
                0x42,    0x4e,    0x4d,    0x3b,    0x3a,    0x5f,  0xffe2,  0xffaa,
              0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
              0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
              0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
              0xffb2,  0xffb3,  0xffb0,  0xffac,     0x0,  0xff7e,    0x3e,  0xffc8,
              0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
              0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
              0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
//              0xffed,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
//            0, 0, 0, 0, 0, 0, 0, 0,
//            0, 27, 33, 34, 167, 36, 37, 38, 47, 40, 41, 61, 63, 96, 0, 0, 81, 87, 69, 82,
//            84, 90, 85, 73, 79, 80, 220, 42, 13, 0, 65, 83, 68, 70, 71, 72, 74, 75, 76, 214,
//            196, 176, 0, 39, 89, 88, 67, 86, 66, 78, 77, 59, 58, 95, 0, 42, 0, 32, 0, 0,
//            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55, 56, 57, 45, 52, 53, 54, 43, 49,
//            50, 51, 48, 44, 0, 0, 62, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//            13, 0, 0, 0, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 61, 0,
        } ;

        const KeyLayout_t DEFAULT_altgr_sym = {
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,  0xff1b,    0xb9,    0xb2,    0xb3,    0xbc,    0xbd,    0xac,
                0x7b,    0x5b,    0x5d,    0x7d,    0x5c,  0xfe5b,  0xff08,  0xff09,
                0x40,   0x1b3,  0x20ac,    0xb6,   0x3bc,   0x8fb,   0x8fe,   0x8fd,
                0xf8,    0xfe,  0xfe57,  0xfe53,  0xff0d,  0xffe3,    0xe6,    0xdf,
                0xf0,   0x1f0,   0x3bf,   0x2b1,    0x6a,   0x3a2,   0x1b3,  0xfe59,
              0xfe52,    0xac,  0xffe1,  0xfe50,    0xab,    0xbb,    0xa2,   0xad2,
               0xad3,    0x6e,    0xb5,   0x8a3,    0xb7,  0xfe60,  0xffe2,  0xffaa,
              0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
              0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
              0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
              0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x7c,  0xffc8,
              0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
              0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,     0x0,
              0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
//            0, 0, 0, 0, 0, 0, 0, 0,
//            0, 27, 185, 178, 179, 188, 189, 172, 123, 91, 93, 125, 92, 184, 0, 0, 64, 322, 8364, 182,
//            359, 8592, 8595, 8594, 248, 254, 168, 126, 13, 0, 230, 223, 240, 273, 331, 295, 106, 312, 322, 733,
//            94, 172, 0, 96, 171, 187, 162, 8220, 8221, 110, 181, 0, 183, 0, 0, 42, 0, 32, 0, 0,
//            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45, 0, 0, 0, 43, 0,
//            0, 0, 0, 0, 0, 0, 124, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//            13, 0, 0, 0, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 61, 0,
        } ;

        const KeyLayout_t DEFAULT_capslock_sym = {
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                0x37,    0x38,    0x39,    0x30,    0xdf,  0xfe51,  0xff08,  0xff09,
                0x51,    0x57,    0x45,    0x52,    0x54,    0x5a,    0x55,    0x49,
                0x4f,    0x50,    0xdc,    0x2b,  0xff0d,  0xffe3,    0x41,    0x53,
                0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0xd6,
                0xc4,  0xfe52,  0xffe1,    0x23,    0x59,    0x58,    0x43,    0x56,
                0x42,    0x4e,    0x4d,    0x2c,    0x2e,    0x2d,  0xffe2,  0xffaa,
              0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
              0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
              0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
              0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x3c,  0xffc8,
              0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
              0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
              0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
//            0, 0, 0, 0, 0, 0, 0, 0,
//            0, 27, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 223, 180, 0, 0, 81, 87, 69, 82,
//            84, 90, 85, 73, 79, 80, 220, 43, 13, 0, 65, 83, 68, 70, 71, 72, 74, 75, 76, 214,
//            196, 94, 0, 35, 89, 88, 67, 86, 66, 78, 77, 44, 46, 45, 0, 42, 0, 32, 0, 0,
//            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45, 0, 0, 0, 43, 0,
//            0, 0, 0, 0, 0, 0, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//            13, 0, 0, 0, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 61, 0,
        } ;

        const KeyLayout_t DEFAULT_shiftcapslock_sym = {
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
//                0, 0, 0, 0, 0, 0, 0, 0,
//                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        } ;

        // Intialize the WORK tables
        for(int i=0 ; i < 128 ; i++) {
            keylayout_WORK_noshift_sym[i] = DEFAULT_noshift_sym[i] ;
            keylayout_WORK_shift_sym[i] = DEFAULT_shift_sym[i] ;
            keylayout_WORK_altgr_sym[i] = DEFAULT_altgr_sym[i] ;
            keylayout_WORK_capslock_sym[i] = DEFAULT_capslock_sym[i] ;
            keylayout_WORK_shiftcapslock_sym[i] = DEFAULT_shiftcapslock_sym[i] ;
        }

        switch (keyb){

            case 0x040c: // French
            {
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +----+----+-------+
// | 01 |  | 3B | 3C | 3D | 3E |  | 3F | 40 | 41 | 42 |  | 43 | 44 | 57 | 58 |  | 37 | 46 | 1D+45 |
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +----+----+-------+
//                                     ***  keycodes suffixed by 'x' are extended ***
// +----+----+----+----+----+----+----+----+----+----+----+----+----+--------+  +----+----+----+  +--------------------+
// | 29 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 0A | 0B | 0C | 0D |   0E   |  | 52x| 47x| 49x|  | 45 | 35x| 37 | 4A  |
// +-------------------------------------------------------------------------+  +----+----+----+  +----+----+----+-----+
// |  0F  | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 1A | 1B |      |  | 53x| 4Fx| 51x|  | 47 | 48 | 49 |     |
// +------------------------------------------------------------------+  1C  |  +----+----+----+  +----+----+----| 4E  |
// |  3A   | 1E | 1F | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 2B |     |                    | 4B | 4C | 4D |     |
// +-------------------------------------------------------------------------+       +----+       +----+----+----+-----+
// |  2A | 56 | 2C | 2D | 2E | 2F | 30 | 31 | 32 | 33 | 34 | 35 |    36      |       | 48x|       | 4F | 50 | 51 |     |
// +-------------------------------------------------------------------------+  +----+----+----+  +---------+----| 1Cx |
// |  1D  |  5Bx | 38 |           39           |  38x  |  5Cx |  5Dx |  1Dx  |  | 4Bx| 50x| 4Dx|  |    52   | 53 |     |
// +------+------+----+------------------------+-------+------+------+-------+  +----+----+----+  +---------+----+-----+

                    const KeyLayout_t x040c_noshift_sym = {
                                     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                     0x0,  0xff1b,    0x26,    0xe9,    0x22,    0x27,    0x28,    0x2d,
                                    0xe8,    0x5f,    0xe7,    0xe0,    0x29,    0x3d,  0xff08,  0xff09,
                                    0x61,    0x7a,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                                    0x6f,    0x70,  0xfe52,    0x24,  0xff0d,  0xffe3,    0x71,    0x73,
                                    0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0x6d,
                                    0xf9,    0xb2,  0xffe1,    0x2a,    0x77,    0x78,    0x63,    0x76,
                                    0x62,    0x6e,    0x2c,    0x3b,    0x3a,    0x21,  0xffe2,  0xffaa,
                                  0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                                  0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                                  0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                                  0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x3c,  0xffc8,
                                  0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                                  0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                                  0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                     0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
//                                     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
//                        /*   0 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /*   8 */     0x0,   0x1b,   0x26,   0xe9,   0x22,   0x27,   0x28,   0x2d,
//                        /*  16 */    0xe8,   0x5f,   0xe7,   0xe0,   0x29,   0x3d,    0x8,
//                        /*  23 */     0x9,    'a',    'z',    'e',    'r',    't',    'y',    'u',    'i',   'o',   'p',   0x5e,   0x24,   0xd,
//                        /*  37 */     0x0,    'q',    's',    'd',    'f',    'g',    'h',    'j',   'k',    'l',   'm',   0xf9,
//                        /*  49 */    0xb2,    0x0,   0x2a,    'w',    'x',    'c',    'v',    'b',   'n',   0x2c,  0x3b,   0x3a,   0x21,   0x0,
//                        /*  63 */    0x2a,    0x0,    ' ',    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /*  72 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /*  80 */     0x0,    0x0,   0x2d,    0x0,    0x0,    0x0,   0x2b,    0x0,
//                        /*  88 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x3c,    0x0,
//                        /*  96 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /* 104 */     0x0,    0x0,    0x0,    0x0,    0xd,    0x0,    0x0,    0x0,
//                        /* 112 */    0x2f,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
                    };
                    const KeyLayout_t x040c_shift_sym = {
                                     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                     0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                                    0x37,    0x38,    0x39,    0x30,    0xb0,    0x2b,  0xff08,  0xfe20,
                                    0x41,    0x5a,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                                    0x4f,    0x50,  0xfe57,    0xa3,  0xff0d,  0xffe3,    0x51,    0x53,
                                    0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0x4d,
                                    0x25,    0x7e,  0xffe1,    0xb5,    0x57,    0x58,    0x43,    0x56,
                                    0x42,    0x4e,    0x3f,    0x2e,    0x2f,    0xa7,  0xffe2,  0xffaa,
                                  0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                                  0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                                  0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                                  0xffb2,  0xffb3,  0xffb0,  0xffae,     0x0,  0xff7e,    0x3e,  0xffc8,
                                  0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                                  0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                                  0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                     0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
//                                  0xffed,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
//                        /*   0 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /*   8 */     0x0,   0x1b,   0x31,   0x32,   0x33,   0x34,   0x35,   0x36,
//                        /*  16 */    0x37,   0x38,   0x39,   0x30,   0xb0,   0x2b,    0x8,    0x0,
//                        /*  24 */    0x41,   0x5a,   0x45,   0x52,   0x54,   0x59,   0x55,   0x49,
//                        /*  32 */    0x4f,   0x50,   0xa8,   0xa3,    0xd,    0x0,   0x51,   0x53,
//                        /*  40 */    0x44,   0x46,   0x47,   0x48,   0x4a,   0x4b,   0x4c,   0x4d,
//                        /*  48 */    0x25,   0x7e,    0x0,   0xb5,   0x57,   0x58,   0x43,   0x56,
//                        /*  56 */    0x42,   0x4e,   0x3f,   0x2e,   0x2f,   0xa7,    0x0,   0x2a,
//                        /*  64 */     0x0,   0x20,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /*  72 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x37,
//                        /*  80 */    0x38,   0x39,   0x2d,   0x34,   0x35,   0x36,   0x2b,   0x31,
//                        /*  88 */    0x32,   0x33,   0x30,   0x2e,    0x0,    0x0,   0x3e,    0x0,
//                        /*  96 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /* 104 */     0x0,    0x0,    0x0,    0x0,    0xd,    0x0,    0x0,    0x0,
//                        /* 112 */    0x2f,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
                    };
                    const KeyLayout_t x040c_capslock_sym = {
                                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                 0x0,  0xff1b,    0x26,    0xc9,    0x22,    0x27,    0x28,    0x2d,
                                0xc8,    0x5f,    0xc7,    0xc0,    0x29,    0x3d,  0xff08,  0xff09,
                                0x41,    0x5a,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                                0x4f,    0x50,  0xfe52,    0x24,  0xff0d,  0xffe3,    0x51,    0x53,
                                0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0x4d,
                                0xd9,    0xb2,  0xffe1,    0x2a,    0x57,    0x58,    0x43,    0x56,
                                0x42,    0x4e,    0x2c,    0x3b,    0x3a,    0x21,  0xffe2,  0xffaa,
                              0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                              0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                              0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                              0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x3c,  0xffc8,
                              0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                              0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                              0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                 0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
//                                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
//                        /*   0 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /*   8 */     0x0,   0x1b,   0x31,   0x32,   0x33,   0x34,   0x35,   0x36,
//                        /*  16 */    0x37,   0x38,   0x39,   0x30,   0xb0,   0x2b,    0x8,    0x0,
//                        /*  24 */    0x41,   0x5a,   0x45,   0x52,   0x54,   0x59,   0x55,   0x49,
//                        /*  32 */    0x4f,   0x50,   0xa8,   0xa3,    0xd,    0x0,   0x51,   0x53,
//                        /*  40 */    0x44,   0x46,   0x47,   0x48,   0x4a,   0x4b,   0x4c,   0x4d,
//                        /*  48 */    0x25,   0x7e,    0x0,   0xb5,   0x57,   0x58,   0x43,   0x56,
//                        /*  56 */    0x42,   0x4e,   0x3f,   0x2e,   0x2f,   0xa7,    0x0,   0x2a,
//                        /*  64 */     0x0,   0x20,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /*  72 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x37,
//                        /*  80 */    0x38,   0x39,   0x2d,   0x34,   0x35,   0x36,   0x2b,   0x31,
//                        /*  88 */    0x32,   0x33,   0x30,   0x2e,    0x0,    0x0,   0x3e,    0x0,
//                        /*  96 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /* 104 */     0x0,    0x0,    0x0,    0x0,    0xd,    0x0,    0x0,    0x0,
//                        /* 112 */    0x2f,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
                    };
                    const KeyLayout_t x040c_altgr_sym = {
                                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                 0x0,  0xff1b,    0xb9,    0x7e,    0x23,    0x7b,    0x5b,    0x7c,
                                0x60,    0x5c,    0x5e,    0x40,    0x5d,    0x7d,  0xff08,  0xff09,
                                0xe6,    0xab,  0x20ac,    0xb6,   0x3bc,   0x8fb,   0x8fe,   0x8fd,
                                0xf8,    0xfe,  0xfe57,    0xa4,  0xff0d,  0xffe3,    0x40,    0xdf,
                                0xf0,   0x1f0,   0x3bf,   0x2b1,    0x6a,   0x3a2,   0x1b3,    0xb5,
                              0xfe52,    0xac,  0xffe1,  0xfe50,   0x1b3,    0xbb,    0xa2,   0xad2,
                               0xad3,    0x6e,  0xfe51,   0x8a3,    0xb7,  0xfe60,  0xffe2,  0xffaa,
                              0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                              0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                              0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                              0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x7c,  0xffc8,
                              0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                              0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,     0x0,
                              0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                 0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
//                                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
//                        /*   0 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /*   8 */     0x0,   0x1b,   0xb9,   0x7e,   0x23,   0x7b,   0x5b,   0x7c,
//                        /*  16 */    0x60,   0x5c,   0x5e,   0x40,   0x5d,   0x7d,    0x8,    0x9,
//                        /*  24 */    0xe6,   0xab, 0x20ac,   0xb6,  0x167, 0x2190, 0x2193, 0x2192,
//                        /*  32 */    0xf8,   0xfe,   0xa8,   0xa4,    0xd,    0x0,   0x40,   0xdf,
//                        /*  40 */    0xf0,  0x111,  0x14b,  0x127,   0x6a,  0x138,  0x142,   0xb5,
//                        /*  48 */    0x5e,   0xac,    0x0,   0x60,  0x142,   0xbb,   0xa2, 0x201c,
//                        /*  56 */  0x201d,   0x6e,   0xb4,    0x0,   0xb7,    0x0,    0x0,   0x2a,
//                        /*  64 */     0x0,   0x20,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /*  72 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /*  80 */     0x0,    0x0,   0x2d,    0x0,    0x0,    0x0,   0x2b,    0x0,
//                        /*  88 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x7c,    0x0,
//                        /*  96 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /* 104 */     0x0,    0x0,    0x0,    0x0,    0xd,    0x0,    0x0,    0x0,
//                        /* 112 */    0x2f,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
                    };
                    const KeyLayout_t x040c_shiftcapslock_sym = {
                                 0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                 0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                                0x37,    0x38,    0x39,    0x30,    0xb0,    0x2b,  0xff08,  0xfe20,
                                0x61,    0x7a,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                                0x6f,    0x70,  0xfe57,    0xa3,  0xff0d,  0xffe3,    0x71,    0x73,
                                0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0x6d,
                                0x25,    0x7e,  0xffe1,   0x39c,    0x77,    0x78,    0x63,    0x76,
                                0x62,    0x6e,    0x3f,    0x2e,    0x2f,    0xa7,  0xffe2,  0xffaa,
                              0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                              0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                              0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                              0xffb2,  0xffb3,  0xffb0,  0xffae,     0x0,  0xff7e,    0x3e,  0xffc8,
                              0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                              0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                              0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                 0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
//                              0xffed,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
//                        /*   0 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /*   8 */     0x0,   0x1b,   0x26,   0xe9,   0x22,   0x27,   0x28,   0x2d,
//                        /*  16 */    0xe8,   0x5f,   0xe7,   0xe0,   0x29,   0x3d,    0x8,    0x9,
//                        /*  24 */    0x61,   0x7a,   0x65,   0x72,   0x74,   0x79,   0x75,   0x69,
//                        /*  32 */    0x6f,   0x70,   0x5e,   0x24,    0xd,    0x0,   0x71,   0x73,
//                        /*  40 */    0x64,   0x66,   0x67,   0x68,   0x6a,   0x6b,   0x6c,   0x6d,
//                        /*  48 */    0xf9,   0xb2,    0x0,   0x2a,   0x77,   0x78,   0x63,   0x76,
//                        /*  56 */    0x62,   0x6e,   0x2c,   0x3b,   0x3a,   0x21,    0x0,   0x2a,
//                        /*  64 */     0x0,   0x20,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /*  72 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /*  80 */     0x0,    0x0,   0x2d,    0x0,    0x0,    0x0,   0x2b,    0x0,
//                        /*  88 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x3c,    0x0,
//                        /*  96 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//                        /* 104 */     0x0,    0x0,    0x0,    0x0,    0xd,    0x0,    0x0,    0x0,
//                        /* 112 */    0x2f,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
                    };

                    for(size_t i = 0 ; i < 128 ; i++) {
                        if (x040c_noshift_sym[i]){
                            keylayout_WORK_noshift_sym[i] = x040c_noshift_sym[i] ;
                        }
                        if (x040c_shift_sym[i]){
                            keylayout_WORK_shift_sym[i] = x040c_shift_sym[i] ;
                        }
                        if (x040c_altgr_sym[i]){
                            keylayout_WORK_altgr_sym[i] = x040c_altgr_sym[i] ;
                        }
                        if (x040c_capslock_sym[i]){
                            keylayout_WORK_capslock_sym[i] = x040c_capslock_sym[i] ;
                        }
                        if (x040c_shiftcapslock_sym[i]){
                            keylayout_WORK_shiftcapslock_sym[i] = x040c_shiftcapslock_sym[i] ;
                        }
                    }
                }
                break;

            default:
                break;
        }

    } // KeymapSym::init_layout

};


#endif
