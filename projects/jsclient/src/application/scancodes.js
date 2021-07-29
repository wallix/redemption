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

    case "PrintScreen": return 0x37;

    case "PageDown": return 0x151;
    case "PageUp": return 0x149;
    case "Home": return 0x147;
    case "End": return 0x14F;
    case "Delete": return 0x153;
    case "Insert": return 0x152;
    case "Enter": return 0x1C;
    case "Escape": return 0x01;
    case "Tab": return 0x0F;
    case "Space": return 0x39;

    case "ArrowDown": return 0x150;
    case "ArrowLeft": return 0x14B;
    case "ArrowRight": return 0x14D;
    case "ArrowUp": return 0x148;

    case "ScrollLock": return 0x46;

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
const numpadKeyToScancode = function(key) {
    switch (key)
    {
    case "Escape":                  return 0x01;
    case "*":                       return 0x37;
    case "7": case "Home":          return 0x47;
    case "8": case "ArrowUp":       return 0x48;
    case "9": case "PageUp":        return 0x49;
    case "-":                       return 0x4A;
    case "4": case "ArrowLeft":     return 0x4B;
    case "5": case "Unidentified":  return 0x4C;
    case "6": case "AltRight":      return 0x4D;
    case "+":                       return 0x4E;
    case "1": case "End":           return 0x4F;
    case "2": case "ArrowDown":     return 0x50;
    case "3": case "PageDown":      return 0x51;
    case "0": case "Insert":        return 0x52;
    case ".": case "Delete":        return 0x53;
    case "=":                       return 0x59;
    case ",":                       return 0x7E;
    case "Enter":                   return 0x11C;
    case "/":                       return 0x135;
    case "NumLock":                 return 0x145;
    }
};

/// \brief convert keycode to scancodes
/// \return Array[Number] | undefined
const codeToScancodes = function(code, flag) {
    const scancode = keycodeToSingleScancode(code);
    if (scancode) return [scancode | flag];
    if (code === "Pause") return [0x21D | flag, 0x45 | flag];
};


// reverse keylayout mask
const NoMod       = 0;
const ShiftMod    = 1 << 0;
const CtrlMod     = 1 << 1;
const AltMod      = 1 << 2;
const NumLockMod  = 1 << 3;
const CapsLockMod = 1 << 4;
const OEM8Mod     = 1 << 5;
const KanaMod     = 1 << 6;
const KanaLockMod = 1 << 7;

// extra flags
const AltGrMod      = 1 << 8;
const RightShiftMod = 1 << 9;
const RightCtrlMod  = 1 << 10;
const BothCtrlMod   = CtrlMod | RightCtrlMod;
const BothShiftMod  = ShiftMod | RightShiftMod;

// move RightCtrlMod/RightShiftMod into CtrlMod/ShiftMod
const modMaskToUndirectionalMod = function(m) {
    return (m & ~0xffff) | ((m & (RightShiftMod | RightCtrlMod)) << 16);
};

// Control scancodes
const ShiftLeftScancode = 0x2A;
const ShiftRightScancode = 0x36;
const CtrlLeftScancode = 0x1D;
const CtrlRightScancode = 0x11D;
const AltScancode = 0x38;
const AltGrScancode = 0x138;

const Release = 0x8000;
const Acquire = 0;


// Control scancodes
const LShiftSC   = 0x2A;
const RShiftSC   = 0x36;
const LCtrlSC    = 0x1D;
const RCtrlSC    = 0x11D;
const AltSC      = 0x38;
const AltGrSC    = 0x138;
const CapsLockSC = 0x3A;
const NumLockSC  = 0x145;

const windowsCtrlFilterMask = AltGrMod | AltMod;
const linuxCtrlFilterMask = 0;

const syncControls = function(scancodeFlag, modMask, accu) {
    if (modMask & RightShiftMod) accu.push(scancodeFlag | ShiftRightScancode);
    if (modMask & RightCtrlMod)  accu.push(scancodeFlag | CtrlRightScancode);
    if (modMask & ShiftMod) accu.push(scancodeFlag | ShiftLeftScancode);
    if (modMask & CtrlMod)  accu.push(scancodeFlag | CtrlLeftScancode);
    if (modMask & AltMod)   accu.push(scancodeFlag | AltScancode);
    if (modMask & AltGrMod) accu.push(scancodeFlag | AltGrScancode);
};

const emulateKey = function(modMask, ...scancodes) {
    // /**/console.log(`mod: 0x${modMask.toString(16)}`)
    // /**/for (const x of [...scancodes]) console.log(`    scancode: 0x${x.toString(16)}`);

    const accu = [];
    syncControls(Release, modMask, accu);
    accu.push(...scancodes);
    syncControls(Acquire, modMask, accu);
    return accu;
};

const searchScancodeWithSameControlMask = function(scancodeMods, controlMask) {
    // /**/let s = ''; for (const x of scancodeMods) s += ` 0x${x.toString(16)},`;
    // /**/console.log(`search: (0x${controlMask.toString(16)}) ${s}`);
    const len = scancodeMods.length;
    for (let i = 0; i < len; ++i) {
        const data = scancodeMods[i];
        if (data & controlMask) {
            // /**/console.log('0x'+(data & 0xff).toString(16))
            return data & 0xffff;
        }
    }
};

function keyToScancodeConverter = function() {
    let rlayout;
    let rkeymap;
    let deadKeymap;
    let accentKeymap;

    let modFlags;
    let virtualModFlags;

    let ctrlIsOem8;

    let isDeadKey = false;

    const scancodeByModsToScancodes = function(scancodeByMods) {
        let scancode = scancodeByMods[modFlags];
        if (scancode) {
            return [scancode | flag];
        }

        // emulate control key up/down

        let expectedModFlags;
        for (expectedModFlags in scancodeByMods) {
            scancode = scancodeByMods[expectedModFlags];
            break;
        }

        const down = flag ^ Release;
        const release = flag & Release;

        const accu = [];

        // ctrl, alt, altgr, oem8
        if ((modFlags ^ expectedModFlags) & (AltMod | CtrlMod | OEM8Mod)) {
            const expectedAltFlags = expectedModFlags & AltMod;
            const expectedCtrlFlags = expectedModFlags & CtrlMod;
            const expectedOem8Flags = expectedModFlags & OEM8Mod;
            switch (expectedAltFlags | expectedCtrlFlags | expectedOem8Flags) {
                case 0:
                    if (virtualModFlags & (RightCtrlMod | OEM8Mod)) accu.push(RCtrlSC | release);
                    if (virtualModFlags & CtrlMod) accu.push(LCtrlSC | release);
                    if (virtualModFlags & AltGrMod) accu.push(AltGrSC | release);
                    if (virtualModFlags & AltMod) accu.push(AltSC | release);
                    break;

                case AltMod:
                    if (!(virtualModFlags & AltMod)) accu.push(AltSC | down);
                    if (virtualModFlags & (RightCtrlMod | OEM8Mod)) accu.push(RCtrlSC | release);
                    if (virtualModFlags & CtrlMod) accu.push(LCtrlSC | release);
                    if (virtualModFlags & AltGrMod) accu.push(AltGrSC | release);
                    break;

                case CtrlMod:
                    if (!(virtualModFlags & (RightCtrlMod | CtrlMod))) accu.push(RCtrlSC | down);
                    if (virtualModFlags & OEM8Mod) accu.push(RCtrlSC | release);
                    if (virtualModFlags & AltGrMod) accu.push(AltGrSC | release);
                    if (virtualModFlags & AltMod) accu.push(AltSC | release);
                    break;

                case OEM8Mod:
                    if (!(virtualModFlags & OEM8Mod)) accu.push(RCtrlSC | down);
                    if (virtualModFlags & CtrlMod) accu.push(LCtrlSC | release);
                    if (virtualModFlags & AltGrMod) accu.push(AltGrSC | release);
                    if (virtualModFlags & AltMod) accu.push(AltSC | release);
                    break;

                case (AltMod | OEM8Mod):
                    if (!(virtualModFlags & OEM8Mod)) accu.push(RCtrlSC | down);
                    if (!(virtualModFlags & AltMod)) accu.push(AltSC | down);
                    if (virtualModFlags & CtrlMod) accu.push(LCtrlSC | release);
                    if (virtualModFlags & AltGrMod) accu.push(AltGrSC | release);
                    break;

                case (CtrlMod | OEM8Mod):
                    if (!(virtualModFlags & CtrlMod)) accu.push(LCtrlSC | down);
                    if (!(virtualModFlags & OEM8Mod)) accu.push(RCtrlSC | down);
                    if (virtualModFlags & AltGrMod) accu.push(AltGrSC | release);
                    if (virtualModFlags & AltMod) accu.push(AltSC | release);
                    break;

                case (AltMod | CtrlMod):
                case (AltMod | CtrlMod | OEM8Mod):
                    if (!(virtualModFlags & AltGrMod) && (virtualModFlags & (AltMod | CtrlMod)) != (AltMod | CtrlMod)) {
                        if (virtualModFlags & (AltMod | CtrlMod)) {
                            if (!(virtualModFlags & (CtrlMod | RightCtrlMod))) accu.push(LCtrlSC | down);
                            if (!(virtualModFlags & AltMod)) accu.push(AltSC | down);
                        }
                        else accu.push(AltGrMod | down);
                    }

                    if (expectedOem8Flags) {
                        if (!(virtualModFlags & OEM8Mod)) accu.push(RCtrlSC | down);
                    }
                    else {
                        if (virtualModFlags & OEM8Mod) accu.push(RCtrlSC | release);
                    }
                    break;
            }
        }

        // left/right shift
        if ((modFlags ^ expectedModFlags) & ShiftMod) {
            if (expectedModFlags & ShiftMod) {
                accu.push(LShiftSC | down);
            }
            else {
                if (virtualModFlags & ShiftMod) accu.push(LShiftSC | release);
                if (virtualModFlags & RightShiftMod) accu.push(RShiftSC | release);
            }
        }

        // CapsLock
        if ((modFlags ^ expectedModFlags) & CapsLockMod) {
            accu.push(CapsLockSC | ((expectedModFlags & CapsLockMod) ? down : release));
        }

        // // KanaMod
        // if ((modFlags ^ expectedModFlags) & KanaMod) {
        //     accu.push(KanaModSC | ((expectedModFlags & KanaMod) ? down : release));
        // }
        //
        // // KanaLockMod
        // if ((modFlags ^ expectedModFlags) & KanaLockMod) {
        //     accu.push(KanaLockSC | ((expectedModFlags & KanaLockMod) ? down : release));
        // }

        const accuLen = accu.length;
        accu.push(scancode);
        for (const i = 0; i < accuLen; ++i) {
            accu.push(accu[i] ^ Release);
        }

        return accu;
    };

    /// @param flag = 0 or 0x8000 (Release)
    const convert = function(key, code, flag) {
        const scancodeByMods = rkeymap[key];
        if (scancodeByMods) {
            return scancodeByModsToScancodes(scancodeByMods);
        }

        const dk = deadKeymap[key];

        // dead key
        if (dk) {
            const scancodeByMods = accentKeymap[dk[0]];
            const accu = scancodeByModsToScancodes(accentKeymap[dk[0]]);
            for (let i = 1; i < dk.length; ++i) {
                accu.push(...scancodeByModsToScancodes(rkeymap[dk[i]]));
            }
            return accu;
        }

        // special
        switch (key) {
            case "CapsLock":
                if (flag === Release) {
                    virtualModFlags ^= CapsLockMod;
                    modFlags ^= CapsLockMod;
                }
                return 0x3A;

            case "NumLock":
                if (flag === Release) {
                    virtualModFlags ^= NumLockMod;
                    modFlags ^= NumLockMod;
                }
                return 0x145;

            case "ControlLeft":
                virtualModFlags ^= CtrlMod;
                modFlags ^= CtrlMod;
                return 0x1D;

            case "ControlRight":
                if (ctrlIsOem8) {
                    virtualModFlags ^= OEM8Mod;
                    modFlags ^= OEM8Mod;
                }
                else {
                    virtualModFlags ^= RightCtrlMod;
                    modFlags ^= CtrlMod;
                }
                return 0x11D;

            case "AltGraph":
            case "AltRight":
                virtualModFlags ^= AltGrMod;
                modFlags ^= AltGrMod;
                return 0x138;

            case "AltLeft":
                virtualModFlags ^= AltMod;
                modFlags ^= AltMod;
                return 0x38;

            case "ShiftLeft":
                virtualModFlags ^= ShiftMod;
                modFlags ^= ShiftMod;
                return 0x2A;

            case "ShiftRight":
                virtualModFlags ^= RightShiftMod;
                modFlags ^= ShiftMod;
                return 0x36;

            default:
                return codeToScancodes(code, flag);
        }
    };

    convert.setLayout = (newLayout) => { layout = newLayout; };
    convert.getLayout = () => layout;

    convert.getModifierScancodes = () => {
        if (virtualModFlags) {
            const accu = [];
            if (virtualModFlags & ShiftMod) accu.push(LShiftSC);
            if (virtualModFlags & RightShiftMod) accu.push(RShiftSC);
            if (virtualModFlags & CtrlMod)  accu.push(LCtrlSC);
            if (virtualModFlags & (RightCtrlMod | OEM8Mod))  accu.push(RCtrlSC);
            if (virtualModFlags & AltMod)   accu.push(AltSC);
            if (virtualModFlags & AltGrMod) accu.push(AltGrSC);
            if (virtualModFlags & CapsLockMod) accu.push(CapsLockSC);
            if (virtualModFlags & NumLockMod) accu.push(NumLockSC);
            return accu;
        }
    }

    // states for test
    convert.getVirtualModFlags = () => virtualModFlags;
    convert.getModFlags = () => modFlags;

    return convert;
}

/// \return a function (key, flag) => (Array[Number] | undefined)
/// and add properties follow:
/// - getLayout()
/// - setLayout(layout)
/// - getOSBehavior()
/// - setOSBehavior(osType)
/// - getModifierScancodes() : Array[Number] | undefined
const createUnicodeToScancodeConverter = function(layout)
{
    // on window, alt+ctrl = altgr
    let osType = 'windows';
    let ctrlFilterMask = windowsCtrlFilterMask;
    let controlMaskMap = windowsControlMaskMap;

    // current with combination mask (ShiftAltGrMod)
    let modMask = 0;
    // virtual mask without combination ((CtrlMod | ShiftMod | AltGrMod) or NoMod)
    // Note: CtrlMod+AltMod => AltGrMod
    let controlMask = NoMod;
    let isDeadKey = false;


    const convert = function(key, flag, isRightKey) {
        if (key === 'Space') {
            key = ' ';
        }

        if (isDeadKey) {
            // ignore released dead key
            if (flag) return;

            isDeadKey = false;
            const codes = layout.deadmap[key];
            if (codes) {
                const scancode = searchScancodeWithSameControlMask(codes[1], controlMask);
                if (scancode) {
                    return [codes[0], scancode];
                }
            }
            return ;
        }

        if (key.length === 1) {
            const datas = layout.keymap[key];
            if (datas) {
                const scancode = searchScancodeWithSameControlMask(datas, controlMask);
                if (scancode) {
                    return [scancode | flag];
                }

                const data = datas[0];
                if (data & NoMod) {
                    const mask = (modMask & ctrlFilterMask) ? modMask : (modMask & ~BothCtrlMod);
                    return emulateKey(mask, data & 0xff | flag);
                }
                else if (data & ShiftMod) {
                    const mask = (modMask & ctrlFilterMask) ? (modMask | ~BothShiftMod) : (modMask & ~(BothCtrlMod | BothShiftMod));
                    if (controlMask & ShiftMod) {
                        return emulateKey(mask, data & 0xff | flag);
                    }
                    else {
                        return emulateKey(mask,
                                          ShiftLeftScancode,
                                          data & 0xff | flag,
                                          ShiftLeftScancode | Release,
                        );
                    }
                }
                else if (data & AltGrMod) {
                    const mask = modMask & ~(AltGrMod | BothCtrlMod);
                    if (controlMask & AltGrMod) {
                        return emulateKey(mask, data & 0xff | flag);
                    }
                    else {
                        return emulateKey(mask,
                                          AltGrScancode,
                                          data & 0xff | flag,
                                          AltGrScancode | Release,
                        );
                    }
                }
                else if (data & ShiftAltGrMod) {
                    const mask = modMask & ~(AltGrMod | BothShiftMod | BothCtrlMod);
                    if ((controlMask & (AltGrMod | ShiftMod)) == (AltGrMod | ShiftMod)) {
                        return emulateKey(mask, data & 0xff | flag);
                    }
                    else if (controlMask & AltGrMod) {
                        return emulateKey(mask,
                                          ShiftLeftScancode,
                                          data & 0xff | flag,
                                          ShiftLeftScancode | Release,
                        );
                    }
                    else if (controlMask & ShiftMod) {
                        return emulateKey(mask,
                                          AltGrScancode,
                                          data & 0xff | flag,
                                          AltGrScancode | Release,
                        );
                    }
                    else {
                        return emulateKey(mask,
                                          ShiftLeftScancode,
                                          AltGrScancode,
                                          data & 0xff | flag,
                                          AltGrScancode | Release,
                                          ShiftLeftScancode | Release,
                        );
                    }
                }
            }
        }
        else {
            let scancode;
            switch (key) {
                case 'Dead': {
                    isDeadKey = true;
                    return ;
                }

                case 'Shift': {
                    const mod = isRightKey ? RightShiftMod : ShiftMod;
                    if (flag) {
                        modMask &= ~mod;
                    }
                    else {
                        modMask |= mod;
                    }
                    scancode = isRightKey ? ShiftRightScancode : ShiftLeftScancode;
                    break;
                }

                case 'Control': {
                    const mod = isRightKey ? RightCtrlMod : CtrlMod;
                    if (flag) {
                        modMask &= ~mod;
                    }
                    else {
                        modMask |= mod;
                    }
                    scancode = isRightKey ? CtrlRightScancode : CtrlLeftScancode;
                    break;
                }

                case 'Alt':
                    if (flag) {
                        modMask &= ~AltMod;
                    }
                    else {
                        modMask |= AltMod;
                    }
                    scancode = AltScancode;
                    break;

                case 'AltGraph':
                    if (flag) {
                        modMask &= ~AltGrMod;
                    }
                    else {
                        modMask |= AltGrMod;
                    }
                    scancode = AltGrScancode;
                    break;

                case 'OS': return [flag | (isRightKey ? 0x15C : 0x15B)];
                case 'CapsLock': return ;
                default: return codeToScancodes(key, flag);
            }

            controlMask = controlMaskMap[modMaskToUndirectionalMod(modMask)] || NoMod;

            return [scancode | flag];
        }
    };

    convert.setLayout = (newLayout) => { layout = newLayout; };
    convert.getLayout = () => layout;
    convert.setOSBehavior = function(os) {
        osType = os.toLowerCase();
        if (osType === 'windows') {
            let ctrlFilterMask = windowsCtrlFilterMask;
            let controlMaskMap = windowsControlMaskMap;
        }
        else {
            let ctrlFilterMask = linuxCtrlFilterMask;
            let controlMaskMap = linuxControlMaskMap;
        }
    };
    convert.getOSBehavior = () => osType;

    convert.getModifierScancodes = () => {
        if (modMask) {
            const accu = [];
            syncControls(0, modMask, accu);
            return accu;
        }
    }

    // states for test
    convert.getControlMask = () => controlMask;
    convert.getModMask = () => modMask;
    convert.isDeadKey = () => isDeadKey;

    return convert;
};

try {
    module.exports.keycodeToSingleScancode = keycodeToSingleScancode;
    module.exports.keycodeToMultiScancodes = keycodeToMultiScancodes;
    module.exports.codeToScancodes = codeToScancodes;
    module.exports.createUnicodeToScancodeConverter = createUnicodeToScancodeConverter;
    module.exports.numpadKeyToScancode = numpadKeyToScancode;
}
catch (e) {
    // module not found
}
