"strict";

// https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/key/Key_Values
// https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/code
// 0xE0 -> 0x0100
// /!\ rotate digit code
// Note: special code and key are same value (ControlLeft, ArrowDown, etc)

/// \return Number | undefined
const keycodeToSingleScancode = function(code) {
    switch (code)
    {
    case "KeyA": return 0x1E;
    case "KeyB": return 0x30;
    case "KeyC": return 0x2E;
    case "KeyD": return 0x20;
    case "KeyE": return 0x12;
    case "KeyF": return 0x21;
    case "KeyG": return 0x22;
    case "KeyH": return 0x23;
    case "KeyI": return 0x17;
    case "KeyJ": return 0x24;
    case "KeyK": return 0x25;
    case "KeyL": return 0x26;
    case "KeyM": return 0x32;
    case "KeyN": return 0x31;
    case "KeyO": return 0x18;
    case "KeyP": return 0x19;
    case "KeyQ": return 0x10;
    case "KeyR": return 0x13;
    case "KeyS": return 0x1F;
    case "KeyT": return 0x14;
    case "KeyU": return 0x16;
    case "KeyV": return 0x2F;
    case "KeyW": return 0x11;
    case "KeyX": return 0x2D;
    case "KeyY": return 0x15;
    case "KeyZ": return 0x2C;

    case "Comma": return 0x33;
    case "Convert": return 0x79;
    case "Slash": return 0x35;
    case "BracketLeft": return 0x1A;
    case "BracketRight": return 0x1B;
    case "Backslash": return 0x2B;
    case "Quote": return 0x28;
    case "Semicolon": return 0x27;
    case "Period": return 0x34;

    case "Space": return 0x39;
    case "Tab": return 0x0F;
    case "PageDown": return 0x151;
    case "PageUp": return 0x149;
    case "Home": return 0x147;
    case "End": return 0x14F;
    case "Delete": return 0x153;
    case "Insert": return 0x152;
    case "Enter": return 0x1C;
    case "Escape": return 0x01;

    case "Backquote": return 0x29;
    case "Digit0": return 0x0B;
    case "Digit1": return 0x02;
    case "Digit2": return 0x03;
    case "Digit3": return 0x04;
    case "Digit4": return 0x05;
    case "Digit5": return 0x06;
    case "Digit6": return 0x07;
    case "Digit7": return 0x08;
    case "Digit8": return 0x09;
    case "Digit9": return 0x0A;
    case "Minus": return 0x0B;
    case "Equal": return 0x0C;
    case "Backspace": return 0x0E;

    case "ArrowDown": return 0x150;
    case "ArrowLeft": return 0x14B;
    case "ArrowRight": return 0x14D;
    case "ArrowUp": return 0x148;

    case "CapsLock": return 0x3A;
    case "ContextMenu": return 0x15D;
    case "ControlLeft": return 0x1D;
    case "ControlRight": return 0x11D;
    case "AltGraph": return 0x138;
    case "AltLeft": return 0x38;
    case "AltRight": return 0x138;
    case "OSLeft": return 0x15B;
    case "OSRight": return 0x15C;
    case "MetaLeft": return 0x15B;
    case "MetaRight": return 0x15C;
    case "ShiftLeft": return 0x2A;
    case "ShiftRight": return 0x36;
    case "KanaMode": return 0x70;

    case "NumLock": return 0x145;
    case "Numpad0": return 0x52;
    case "Numpad1": return 0x4F;
    case "Numpad2": return 0x50;
    case "Numpad3": return 0x51;
    case "Numpad4": return 0x4B;
    case "Numpad5": return 0x4C;
    case "Numpad6": return 0x4D;
    case "Numpad7": return 0x47;
    case "Numpad8": return 0x48;
    case "Numpad9": return 0x49;
    case "NumpadAdd": return 0x4E;
    case "NumpadComma": return 0x7E;
    case "NumpadEnter": return 0x11C;
    case "NumpadEqual": return 0x59;
    case "NumpadDivide": return 0x135;
    case "NumpadDecimal": return 0x53;
    case "NumpadMultiply": return 0x37;
    case "NumpadSubtract": return 0x4A;

    case "F1": return 0x3B;
    case "F2": return 0x3C;
    case "F3": return 0x3D;
    case "F4": return 0x3E;
    case "F5": return 0x3F;
    case "F6": return 0x40;
    case "F7": return 0x41;
    case "F8": return 0x42;
    case "F9": return 0x43;
    case "F10": return 0x44;
    case "F11": return 0x57;
    case "F12": return 0x58;
    case "F13": return 0x64;
    case "F14": return 0x65;
    case "F15": return 0x66;
    case "F16": return 0x67;
    case "F17": return 0x68;
    case "F18": return 0x69;
    case "F19": return 0x6A;
    case "F20": return 0x6B;
    case "F21": return 0x6C;
    case "F22": return 0x6D;
    case "F23": return 0x6E;
    case "F24": return 0x76;

    case "PrintScreen": return 0x37;

    case "ScrollLock": return 0x46;

    case "Paste": return 0x10A;
    case "Copy": return 0x118;
    case "Cut": return 0x117;

    case "AudioVolumeDown": return 0x12E;
    case "AudioVolumeMute": return 0x120;
    case "AudioVolumeUp": return 0x130;
    case "BrowserBack": return 0x16A;
    case "BrowserFavorites": return 0x166;
    case "BrowserForward": return 0x169;
    case "BrowserHome": return 0x132;
    case "BrowserRefresh": return 0x167;
    case "BrowserSearch": return 0x165;
    case "BrowserStop": return 0x168;
    case "LaunchApp1": return 0x16B;
    case "LaunchApp2": return 0x121;
    case "LaunchMail": return 0x16C;
    case "LaunchMediaPlayer": return 0x16D;
    case "MediaPlayPause": return 0x122;
    case "MediaStop": return 0x124;
    case "MediaTrackNext": return 0x119;
    case "MediaTrackPrevious": return 0x110;
    case "VolumeDown": return 0x12E;
    case "VolumeMute": return 0x120;
    case "VolumeUp": return 0x130;

    case "Eject": return 0x12C;
    case "Help": return 0x63;
    case "IntlBackslash": return 0x56;
    case "IntlRo": return 0x73;
    case "IntlYen": return 0x7D;
    // case "Lang1": return 0x1F2; /* key with Korean keyboard layout */
    // case "Lang2": return 0x1F1; /* key with Korean keyboard layout */
    case "Lang1": return 0x72;
    case "Lang2": return 0x71;
    case "NonConvert": return 0x7B;
    case "Power": return 0x15E;
    case "Undo": return 0x108;
    }
};

/// \return Number | undefined
const numpadCodeToScancode = function(key) {
    switch (key)
    {
    case "NumpadMultiply":  /*case "*": case "Multiply":    */  return 0x37;
    case "Numpad7":         /*case "7": case "Home":        */  return 0x47;
    case "Numpad8":         /*case "8": case "ArrowUp":     */  return 0x48;
    case "Numpad9":         /*case "9": case "PageUp":      */  return 0x49;
    case "NumpadSubtract":  /*case "-": case "Subtract":    */  return 0x4A;
    case "Numpad4":         /*case "4": case "ArrowLeft":   */  return 0x4B;
    case "Numpad5":         /*case "5": case "Unidentified":*/  return 0x4C;
    case "Numpad6":         /*case "6": case "AltRight":    */  return 0x4D;
    case "NumpadAdd":       /*case "+": case "Add":         */  return 0x4E;
    case "Numpad1":         /*case "1": case "End":         */  return 0x4F;
    case "Numpad2":         /*case "2": case "ArrowDown":   */  return 0x50;
    case "Numpad3":         /*case "3": case "PageDown":    */  return 0x51;
    case "Numpad0":         /*case "0": case "Insert":      */  return 0x52;
    case "NumpadDecimal":   /*case ".": case "Delete":      */  return 0x53;
    case "NumpadEqual":     /*case "=":                     */  return 0x59;
    case "NumpadComma":     /*case ",": case "Separator":   */  return 0x7E;
    case "NumpadEnter":     /*case "Enter":                 */  return 0x11C;
    case "NumpadDivide":    /*case "/": case "Divide":      */  return 0x135;
    case "NumLock":         /*                              */  return 0x145;
    }
};

/// \brief convert keycode to scancodes
/// \return Array[Number] | undefined
const codeToScancodes = function(code, flag) {
    const scancode = keycodeToSingleScancode(code);
    if (scancode) return [scancode | flag];
    if (code === "Pause") return [0x21D | flag, 0x45 | flag];
};


// reverse keylayout mask (see tools/gen_reversed_keymap.py)
const ShiftMod    = 1 << 0;
const AltGrMod    = 1 << 1;
const CapsLockMod = 1 << 2;
const NumLockMod  = 1 << 3;
const CtrlMod     = 1 << 4;
const AltMod      = 1 << 5;
const OEM8Mod     = 1 << 6;
const KanaMod     = 1 << 7;
const KanaLockMod = 1 << 8;

// extra flags
const RightShiftMod = 1 << 9;
const RightCtrlMod  = 1 << 10;

// Control scancodes
const LShiftSC   = 0x2A;
const RShiftSC   = 0x36;
const LCtrlSC    = 0x1D;
const RCtrlSC    = 0x11D;
const AltSC      = 0x38;
const AltGrSC    = 0x138;
const LMetaSC    = 0x15B;
const RMetaSC    = 0x15C;
const CapsLockSC = 0x3A;
const NumLockSC  = 0x145;

// key flags
const KeyRelease = 0x8000;
const KeyAcquire = 0;

const SyncFlags = Object.freeze({
    NoSync:    0,
    // rdp lock flags
    ScrollLock: 0x01,
    NumLock:    0x02,
    CapsLock:   0x04,
    KanaLock:   0x08,
    // control scancodes
    ControlLeft:  0x0010,
    ControlRight: 0x0020,
    ShiftLeft:    0x0040,
    ShiftRight:   0x0080,
    AltLeft:      0x0100,
    AltRight:     0x0200,
    OSLeft:       0x0400,
    OSRight:      0x0800,
    Kana:         0x1000,
});

const ScancodeByMod = Object.freeze({
    ControlLeft:  LCtrlSC,
    ControlRight: RCtrlSC,
    ShiftLeft:    LShiftSC,
    ShiftRight:   RShiftSC,
    AltLeft:      AltSC,
    AltRight:     AltGrSC,
    OSLeft:       0x15B,
    OSRight:      0x15C,
    Kana:         0x70,
});


const emptyReversedLayout = {
    klid: 0,
    localeName: "NoKeyboard",
    displayName: "NoKeyboard",
    ctrlRightIsOem8: false,
    keymap: {},
    actions: {},
    deadkeys: {},
    accents: [],
};

remappingControlLefts = {
    OS:      'OSLeft',
    Meta:    'OSLeft',
    Shift:   'ShiftLeft',
    Control: 'ControlLeft',
};
remappingControlRights = {
    OS:      'OSRight',
    Meta:    'OSRight',
    Shift:   'ShiftRight',
    Control: 'ControlRight',
};
// { code : { key: newcode } }
remappingCodeByKey = {
    OSLeft: remappingControlLefts,
    MetaLeft: remappingControlLefts,
    ShiftLeft: remappingControlLefts,
    ControlLeft: remappingControlLefts,
    OSRight: remappingControlRights,
    MetaRight: remappingControlRights,
    ShiftRight: remappingControlRights,
    ControlRight: remappingControlRights,
};


/// \return scancodes
const scancodesForSynchronizedMods = function(syncFlags) {
    const accu = []
    accu.push(LShiftSC | ((syncFlags & SyncFlags.ShiftLeft)    ? KeyAcquire : KeyRelease));
    accu.push(RShiftSC | ((syncFlags & SyncFlags.ShiftRight)   ? KeyAcquire : KeyRelease));
    accu.push(LCtrlSC  | ((syncFlags & SyncFlags.ControlLeft)  ? KeyAcquire : KeyRelease));
    accu.push(RCtrlSC  | ((syncFlags & SyncFlags.ControlRight) ? KeyAcquire : KeyRelease));
    accu.push(AltSC    | ((syncFlags & SyncFlags.AltLeft)      ? KeyAcquire : KeyRelease));
    accu.push(AltGrSC  | ((syncFlags & SyncFlags.AltRight)     ? KeyAcquire : KeyRelease));
    accu.push(LMetaSC  | ((syncFlags & SyncFlags.OSLeft)       ? KeyAcquire : KeyRelease));
    accu.push(RMetaSC  | ((syncFlags & SyncFlags.OSRight)      ? KeyAcquire : KeyRelease));
    // accu.push(KanaSC   | ((syncFlags & SyncFlags.Kana)         ? KeyAcquire : KeyRelease));

    return accu;
}


// Keyboard behavior
//
// Windows: ctrl+alt = altgr
//
//                                | Windows | Linux
//  Numpad8 + Verrnum off         |    Up   |   Up
//  Numpad8 + Verrnum off + Shift |    Up   |   8
//  Numpad8 + Verrnum on          |    8    |   8
//  Numpad8 + Verrnum on  + Shift |    Up   |   Up
//
//  Digit1 + CapsLock off         |    &    |   &   (fr)
//  Digit1 + CapsLock on          |    1    |   &   (fr)
//  Digit1 + CapsLock off + Shift |    1    |   1   (fr)
//  Digit1 + CapsLock on  + Shift |    &    |   1   (fr)


class ReversedKeymap
{
    // _virtualModFlags:
    //   - left controls
    //   - ctrl+alt = altgr when _altGrIsCtrlAndAlt
    //   - eom8 when _rctrlIsOem8 && (_modFlags & RightCtrlMod)
    // _modFlags: all controls

    constructor(reversedLayout) {
        this._modFlags = 0;
        this._virtualModFlags = 0;
        this._altGrIsCtrlAndAlt = true;
        this.layout = reversedLayout || emptyReversedLayout;
    }

    // states for test
    getModFlags() { return this._modFlags; }
    getVirtualModFlags() { return this._virtualModFlags; }

    get altGrIsCtrlAndAlt() {
        return this._altGrIsCtrlAndAlt;
    }

    set altGrIsCtrlAndAlt(altGrIsCtrlAndAlt) {
        this._altGrIsCtrlAndAlt = altGrIsCtrlAndAlt;
        this._syncAltGrFlags();
    }

    get layout() {
        return this._layout;
    }

    set layout(reversedLayout) {
        this._layout = reversedLayout;
        this._keymap = reversedLayout.keymap;
        this._deadKeymap = reversedLayout.deadkeys;
        this._accentKeymap = reversedLayout.accents;
        this._rctrlIsOem8 = reversedLayout.ctrlRightIsOem8;
        this._syncOEM8();
    }

    sync(syncFlags) {
        let virtualModFlags = 0;
        let modFlags = 0;

        const updateMods = (syncf, modf, vmodf) => {
            if (syncFlags & syncf) {
                virtualModFlags |= modf;
                modFlags |= vmodf;
            }
        }

        updateMods(SyncFlags.ShiftLeft, ShiftMod, ShiftMod);
        updateMods(SyncFlags.ShiftRight, ShiftMod, RightShiftMod);
        updateMods(SyncFlags.ControlLeft, CtrlMod, CtrlMod);
        updateMods(SyncFlags.ControlRight, CtrlMod, RightCtrlMod);
        updateMods(SyncFlags.AltLeft, AltMod, AltMod);
        updateMods(SyncFlags.AltRight, AltGrMod, AltGrMod);
        updateMods(SyncFlags.CapsLock, CapsLockMod, CapsLockMod);
        updateMods(SyncFlags.NumLock, NumLockMod, NumLockMod);
        // updateMods(SyncFlags.KanaLock, KanaLockMod, KanaLockMod);
        // updateMods(SyncFlags.Kana, KanaMod, KanaMod);
        // ignore ScrollLock value

        this._virtualModFlags = virtualModFlags;
        this._modFlags = modFlags;

        this._syncOEM8();
        this._syncAltGrFlags();
    }

    keyUp(key, code) {
        return this.toScancodesAndFlags(key, code, KeyRelease);
    }

    keyDown(key, code) {
        return this.toScancodesAndFlags(key, code, KeyAcquire);
    }

    /// @param flag = 0 (KeyAcquire) or 0x8000 (KeyRelease)
    toScancodesAndFlags(key, code, flag) {
        const scancodeByMods = this._keymap[key];
        if (scancodeByMods) {
            return this._scancodeByModsToScancodes(scancodeByMods, flag);
        }

        const dk = this._deadKeymap[key];

        // dead key
        if (dk) {
            const accu = this._scancodeByModsToScancodes(this._accentKeymap[dk[0]], flag);
            for (let i = 1; i < dk.length; ++i) {
                accu.push(...this._scancodeByModsToScancodes(this._keymap[dk[i]], flag));
            }
            return accu;
        }

        // special key (ex: alt is ctrl left)
        const codeByKey = remappingCodeByKey[code];
        if (codeByKey) {
            key = codeByKey[key] || key;
        }
        // use key for special configuration such as Esc <-> CapsLock
        return this._codeToScancodes(key, flag) || this._codeToScancodes(code, flag);
    }

    _scancodeByModsToScancodes(scancodeByMods, flag) {
        let scancode = scancodeByMods[this._virtualModFlags];
        if (scancode) {
            return [scancode | flag];
        }

        // emulate control key up/down

        let expectedModFlags;
        for (expectedModFlags in scancodeByMods) {
            scancode = scancodeByMods[expectedModFlags];
            if (flag === KeyRelease) {
                return [scancode | flag];
            }
            break;
        }

        // if scancodeByMods
        if (!scancode) {
            return;
        }

        // const release = flag ^ KeyRelease;
        // const down = flag & KeyRelease;
        const release = KeyRelease;
        const down = 0;

        const accu = [];

        // shift
        if ((this._virtualModFlags ^ expectedModFlags) & ShiftMod) {
            if (expectedModFlags & ShiftMod) {
                accu.push(LShiftSC | down);
            }
            else {
                if (this._modFlags & ShiftMod) accu.push(LShiftSC | release);
                if (this._modFlags & RightShiftMod) accu.push(RShiftSC | release);
            }
        }

        // ctrl
        if ((this._virtualModFlags ^ expectedModFlags) & CtrlMod) {
            if (expectedModFlags & CtrlMod) {
                accu.push(LCtrlSC | down);
            }
            else {
                if (this._modFlags & CtrlMod) accu.push(LCtrlSC | release);
                if ((this._modFlags & RightCtrlMod) && !this._rctrlIsOem8) accu.push(RCtrlSC | release);
            }
        }

        // oem8
        if ((this._virtualModFlags ^ expectedModFlags) & OEM8Mod) {
            accu.push(RCtrlSC | ((expectedModFlags & OEM8Mod) ? down : release));
        }

        // alt
        if ((this._virtualModFlags ^ expectedModFlags) & AltMod) {
            accu.push(AltSC | ((expectedModFlags & AltMod) ? down : release));
        }

        // altGr
        if ((this._virtualModFlags ^ expectedModFlags) & AltGrMod) {
            accu.push(AltGrSC | ((expectedModFlags & AltGrMod) ? down : release));
        }

        // CapsLock
        if ((this._virtualModFlags ^ expectedModFlags) & CapsLockMod) {
            accu.push(CapsLockSC | ((expectedModFlags & CapsLockMod) ? down : release));
        }

        // // KanaMod
        // if ((this._virtualModFlags ^ expectedModFlags) & KanaMod) {
        //     accu.push(KanaModSC | ((expectedModFlags & KanaMod) ? down : release));
        // }
        //
        // // KanaLockMod
        // if ((this._virtualModFlags ^ expectedModFlags) & KanaLockMod) {
        //     accu.push(KanaLockSC | ((expectedModFlags & KanaLockMod) ? down : release));
        // }

        const accuLen = accu.length;
        accu.push(scancode);
        // reset emulated keys
        for (let i = 0; i < accuLen; ++i) {
            accu.push(accu[i] ^ KeyRelease);
        }

        return accu;
    }

    _codeToScancodes(code, flag) {
        switch (code) {
            case "CapsLock":
                if (flag === KeyAcquire) {
                    this._modFlags ^= CapsLockMod;
                    this._virtualModFlags ^= CapsLockMod;
                }
                return [CapsLockSC | flag];

            case "NumLock":
                if (flag === KeyAcquire) {
                    this._modFlags ^= NumLockMod;
                    this._virtualModFlags ^= NumLockMod;
                }
                return [NumLockSC | flag];

            case "ControlLeft":
                this._updateFlags(CtrlMod, CtrlMod, CtrlMod | RightCtrlMod, flag);
                this._syncAltGrFlags();
                return [LCtrlSC | flag];

            case "ControlRight":
                this._updateFlags(CtrlMod, RightCtrlMod, CtrlMod | RightCtrlMod, flag);
                this._syncOEM8();
                this._syncAltGrFlags();
                return [RCtrlSC | flag];

            case "AltGraph":
            case "AltRight":
                this._updateFlags(AltGrMod, AltGrMod, AltGrMod, flag);
                this._syncAltGrFlags();
                return [AltGrSC | flag];

            case "Alt":
            case "AltLeft":
                this._updateFlags(AltMod, AltMod, AltMod, flag);
                this._syncAltGrFlags();
                return [AltSC | flag];

            case "ShiftLeft":
                this._updateFlags(ShiftMod, ShiftMod, ShiftMod | RightShiftMod, flag);
                return [LShiftSC | flag];

            case "ShiftRight":
                this._updateFlags(ShiftMod, RightShiftMod, ShiftMod | RightShiftMod, flag);
                return [RShiftSC | flag];

            // case "Dead":
            // case "Compose":
            //     return;

            default:
                return codeToScancodes(code, flag);
        }
    }

    _updateFlags(modf, vmodf, vmodfMask, flag) {
        if (flag === KeyRelease) {
            this._modFlags &= ~vmodf;
            this._virtualModFlags &= ~modf;
            this._virtualModFlags |= (this._modFlags & vmodfMask) ? modf : 0;
        }
        else {
            this._modFlags |= vmodf;
            this._virtualModFlags |= modf;
        }
    }

    _updateLock(lockFlags, lockMod, mod) {
        if (lockFlags & lockMod) {
            this._modFlags |= mod;
            this._virtualModFlags |= mod;
        }
        else {
            this._modFlags &= ~mod;
            this._virtualModFlags &= ~mod;
        }
    }

    _syncAltGrFlags() {
        const ctrl = this._rctrlIsOem8 ? CtrlMod : (CtrlMod | RightCtrlMod);
        const hasCtrl = this._modFlags & ctrl;
        const hasAlt = this._modFlags & AltMod;
        const hasAltGr = this._modFlags & AltGrMod;
        this._virtualModFlags &= ~(CtrlMod | AltMod | AltGrMod);
        if (this._altGrIsCtrlAndAlt && ((hasCtrl && hasAlt) || hasAltGr)) {
            this._virtualModFlags |= AltGrMod;
        }
        else {
            this._virtualModFlags |= hasCtrl ? CtrlMod : 0;
            this._virtualModFlags |= hasAlt | hasAltGr;
        }
    }

    _syncOEM8() {
        this._virtualModFlags &= ~OEM8Mod;
        if (this._rctrlIsOem8 && (this._modFlags & RightCtrlMod)) {
            this._virtualModFlags &= ~CtrlMod;
            this._virtualModFlags |= OEM8Mod | (this._modFlags & CtrlMod);
        }
    }
};

const scancodeFromscancodeAndFlags = function(scancodeAndFlags) { return scancodeAndFlags & 0xff; };
const flagsFromScancodeAndFlags = function(scancodeAndFlags) { return scancodeAndFlags & 0xff00; };

try {
    module.exports.scancodesForSynchronizedMods = scancodesForSynchronizedMods;
    module.exports.scancodeFromscancodeAndFlags = scancodeFromscancodeAndFlags;
    module.exports.flagsFromScancodeAndFlags = flagsFromScancodeAndFlags;
    module.exports.keycodeToSingleScancode = keycodeToSingleScancode;
    module.exports.numpadCodeToScancode = numpadCodeToScancode;
    module.exports.codeToScancodes = codeToScancodes;
    module.exports.ReversedKeymap = ReversedKeymap;
    module.exports.ScancodeByMod = ScancodeByMod;
    module.exports.NullLayout = NullLayout;
    module.exports.KeyRelease = KeyRelease;
    module.exports.KeyAcquire = KeyAcquire;
    module.exports.SyncFlags = SyncFlags;
}
catch (e) {
    // module not found
}
