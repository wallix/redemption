"strict";

// https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/key/Key_Values
// https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/code
// 0xE0 -> 0x01

const toScancode_Firefox = function(code)
{
    switch (code)
    {
    case "Escape": return 0x0001;
    case "Digit0": return 0x0002;
    case "Digit1": return 0x0003;
    case "Digit2": return 0x0004;
    case "Digit3": return 0x0005;
    case "Digit4": return 0x0006;
    case "Digit5": return 0x0007;
    case "Digit6": return 0x0008;
    case "Digit7": return 0x0009;
    case "Digit8": return 0x000A;
    case "Digit9": return 0x000B;
    case "Minus": return 0x000C;
    case "Equal": return 0x000D;
    case "Backspace": return 0x000E;
    case "Tab": return 0x000F;
    case "KeyQ": return 0x0010;
    case "KeyW": return 0x0011;
    case "KeyE": return 0x0012;
    case "KeyR": return 0x0013;
    case "KeyT": return 0x0014;
    case "KeyY": return 0x0015;
    case "KeyU": return 0x0016;
    case "KeyI": return 0x0017;
    case "KeyO": return 0x0018;
    case "KeyP": return 0x0019;
    case "BracketLeft": return 0x001A;
    case "BracketRight": return 0x001B;
    case "Enter": return 0x001C;
    case "ControlLeft": return 0x001D;
    case "KeyA": return 0x001E;
    case "KeyS": return 0x001F;
    case "KeyD": return 0x0020;
    case "KeyF": return 0x0021;
    case "KeyG": return 0x0022;
    case "KeyH": return 0x0023;
    case "KeyJ": return 0x0024;
    case "KeyK": return 0x0025;
    case "KeyL": return 0x0026;
    case "Semicolon": return 0x0027;
    case "Quote": return 0x0028;
    case "Backquote": return 0x0029;
    case "ShiftLeft": return 0x002A;
    case "Backslash": return 0x002B;
    case "KeyZ": return 0x002C;
    case "KeyX": return 0x002D;
    case "KeyC": return 0x002E;
    case "KeyV": return 0x002F;
    case "KeyB": return 0x0030;
    case "KeyN": return 0x0031;
    case "KeyM": return 0x0032;
    case "Comma": return 0x0033;
    case "Period": return 0x0034;
    case "Slash": return 0x0035;
    case "ShiftRight": return 0x0036;
    case "NumpadMultiply": return 0x0037;
    case "AltLeft": return 0x0038;
    case "Space": return 0x0039;
    case "CapsLock": return 0x003A;
    case "F1": return 0x003B;
    case "F2": return 0x003C;
    case "F3": return 0x003D;
    case "F4": return 0x003E;
    case "F5": return 0x003F;
    case "F6": return 0x0040;
    case "F7": return 0x0041;
    case "F8": return 0x0042;
    case "F9": return 0x0043;
    case "F10": return 0x0044;
    case "Pause": return 0x0045;
    case "ScrollLock": return 0x0046;
    case "Numpad7": return 0x0047;
    case "Numpad8": return 0x0048;
    case "Numpad9": return 0x0049;
    case "NumpadSubtract": return 0x004A;
    case "Numpad4": return 0x004B;
    case "Numpad5": return 0x004C;
    case "Numpad6": return 0x004D;
    case "NumpadAdd": return 0x004E;
    case "Numpad1": return 0x004F;
    case "Numpad2": return 0x0050;
    case "Numpad3": return 0x0051;
    case "Numpad0": return 0x0052;
    case "NumpadDecimal": return 0x0053;
    case "IntlBackslash": return 0x0056;
    case "F11": return 0x0057;
    case "F12": return 0x0058;
    case "NumpadEqual": return 0x0059;
    case "F13": return 0x0064;
    case "F14": return 0x0065;
    case "F15": return 0x0066;
    case "F16": return 0x0067;
    case "F17": return 0x0068;
    case "F18": return 0x0069;
    case "F19": return 0x006A;
    case "F20": return 0x006B;
    case "F21": return 0x006C;
    case "F22": return 0x006D;
    case "F23": return 0x006E;
    case "KanaMode": return 0x0070;
    case "Lang2": return 0x0071;
    // case "Lang2": return 0x1F1; /* key with Korean keyboard layout */
    case "Lang1": return 0x0072;
    // case "Lang1": return 0x1F2; /* key with Korean keyboard layout */
    case "IntlRo": return 0x0073;
    case "F24": return 0x0076;
    case "Convert": return 0x0079;
    case "NonConvert": return 0x007B;
    case "IntlYen": return 0x007D;
    case "NumpadComma": return 0x007E;
    case "Undo": return 0x108;
    case "Paste": return 0x10A;
    case "MediaTrackPrevious": return 0x110;
    case "Cut": return 0x117;
    case "Copy": return 0x118;
    case "MediaTrackNext": return 0x119;
    case "NumpadEnter": return 0x11C;
    case "ControlRight": return 0x11D;
    case "AudioVolumeMute": return 0x120;
    case "VolumeMute": return 0x120;
    case "LaunchApp2": return 0x121;
    case "MediaPlayPause": return 0x122;
    case "MediaStop": return 0x124;
    case "Eject": return 0x12C;
    case "AudioVolumeDown": return 0x12E;
    case "VolumeDown": return 0x12E;
    case "AudioVolumeUp": return 0x130;
    case "VolumeUp": return 0x130;
    case "BrowserHome": return 0x132;
    case "NumpadDivide": return 0x135;
    case "PrintScreen": return 0x137;
    case "AltRight": return 0x138;
    case "Help": return 0x13B;
    case "NumLock": return 0x145;
    case "Home": return 0x147;
    case "ArrowUp": return 0x148;
    case "PageUp": return 0x149;
    case "ArrowLeft": return 0x14B;
    case "ArrowRight": return 0x14D;
    case "End": return 0x14F;
    case "ArrowDown": return 0x150;
    case "PageDown": return 0x151;
    case "Insert": return 0x152;
    case "Delete": return 0x153;
    case "MetaLeft": return 0x15B;
    case "OSLeft": return 0x15B;
    case "MetaRight": return 0x15C;
    case "OSRight": return 0x15C;
    case "ContextMenu": return 0x15D;
    case "Power": return 0x15E;
    case "BrowserSearch": return 0x165;
    case "BrowserFavorites": return 0x166;
    case "BrowserRefresh": return 0x167;
    case "BrowserStop": return 0x168;
    case "BrowserForward": return 0x169;
    case "BrowserBack": return 0x16A;
    case "LaunchApp1": return 0x16B;
    case "LaunchMail": return 0x16C;
    case "LaunchMediaPlayer": return 0x16D;
    }
}

const toScancode_Chromium44 = function(code)
{
    switch (code)
    {
    case "Escape": return 0x0001;
    case "Digit0": return 0x0002;
    case "Digit1": return 0x0003;
    case "Digit2": return 0x0004;
    case "Digit3": return 0x0005;
    case "Digit4": return 0x0006;
    case "Digit5": return 0x0007;
    case "Digit6": return 0x0008;
    case "Digit7": return 0x0009;
    case "Digit8": return 0x000A;
    case "Digit9": return 0x000B;
    case "Minus": return 0x000C;
    case "Equal": return 0x000D;
    case "Backspace": return 0x000E;
    case "Tab": return 0x000F;
    case "KeyQ": return 0x0010;
    case "KeyW": return 0x0011;
    case "KeyE": return 0x0012;
    case "KeyR": return 0x0013;
    case "KeyT": return 0x0014;
    case "KeyY": return 0x0015;
    case "KeyU": return 0x0016;
    case "KeyI": return 0x0017;
    case "KeyO": return 0x0018;
    case "KeyP": return 0x0019;
    case "BracketLeft": return 0x001A;
    case "BracketRight": return 0x001B;
    case "Enter": return 0x001C;
    case "ControlLeft": return 0x001D;
    case "KeyA": return 0x001E;
    case "KeyS": return 0x001F;
    case "KeyD": return 0x0020;
    case "KeyF": return 0x0021;
    case "KeyG": return 0x0022;
    case "KeyH": return 0x0023;
    case "KeyJ": return 0x0024;
    case "KeyK": return 0x0025;
    case "KeyL": return 0x0026;
    case "Semicolon": return 0x0027;
    case "Quote": return 0x0028;
    case "Backquote": return 0x0029;
    case "ShiftLeft": return 0x002A;
    case "Backslash": return 0x002B;
    case "KeyZ": return 0x002C;
    case "KeyX": return 0x002D;
    case "KeyC": return 0x002E;
    case "KeyV": return 0x002F;
    case "KeyB": return 0x0030;
    case "KeyN": return 0x0031;
    case "KeyM": return 0x0032;
    case "Comma": return 0x0033;
    case "Period": return 0x0034;
    case "Slash": return 0x0035;
    case "ShiftRight": return 0x0036;
    case "NumpadMultiply": return 0x0037;
    case "AltLeft": return 0x0038;
    case "Space": return 0x0039;
    case "CapsLock": return 0x003A;
    case "F1": return 0x003B;
    case "F2": return 0x003C;
    case "F3": return 0x003D;
    case "F4": return 0x003E;
    case "F5": return 0x003F;
    case "F6": return 0x0040;
    case "F7": return 0x0041;
    case "F8": return 0x0042;
    case "F9": return 0x0043;
    case "F10": return 0x0044;
    case "Pause": return 0x0045;
    case "ScrollLock": return 0x0046;
    case "Numpad7": return 0x0047;
    case "Numpad8": return 0x0048;
    case "Numpad9": return 0x0049;
    case "NumpadSubtract": return 0x004A;
    case "Numpad4": return 0x004B;
    case "Numpad5": return 0x004C;
    case "Numpad6": return 0x004D;
    case "NumpadAdd": return 0x004E;
    case "Numpad1": return 0x004F;
    case "Numpad2": return 0x0050;
    case "Numpad3": return 0x0051;
    case "Numpad0": return 0x0052;
    case "NumpadDecimal": return 0x0053;
    case "IntlBackslash": return 0x0056;
    case "F11": return 0x0057;
    case "F12": return 0x0058;
    case "NumpadEqual": return 0x0059;
    case "F13": return 0x005B;
    case "F14": return 0x005C;
    case "F15": return 0x005D;
    case "F16": return 0x0063;
    case "F17": return 0x0064;
    case "F18": return 0x0065;
    case "F19": return 0x0066;
    case "F20": return 0x0067;
    case "F21": return 0x0068;
    case "F22": return 0x0069;
    case "F23": return 0x006A;
    case "F24": return 0x006B;
    case "KanaMode": return 0x0070;
    case "IntlRo": return 0x0073;
    case "Convert": return 0x0079;
    case "NonConvert": return 0x007B;
    case "IntlYen": return 0x007D;
    case "NumpadComma": return 0x007E;
    case "Undo": return 0x108;
    case "Paste": return 0x10A;
    case "MediaTrackPrevious": return 0x110;
    case "Cut": return 0x117;
    case "Copy": return 0x118;
    case "MediaTrackNext": return 0x119;
    case "NumpadEnter": return 0x11C;
    case "ControlRight": return 0x11D;
    case "LaunchMail": return 0x11E;
    case "AudioVolumeMute": return 0x120;
    case "VolumeMute": return 0x120;
    case "MediaPlayPause": return 0x122;
    case "MediaStop": return 0x124;
    case "Eject": return 0x12C;
    case "AudioVolumeDown": return 0x12E;
    case "VolumeDown": return 0x12E;
    case "AudioVolumeUp": return 0x130;
    case "VolumeUp": return 0x130;
    case "BrowserHome": return 0x132;
    case "NumpadDivide": return 0x135;
    case "PrintScreen": return 0x137;
    case "AltRight": return 0x138;
    case "Help": return 0x13B;
    case "NumLock": return 0x145;
    case "Home": return 0x147;
    case "ArrowUp": return 0x148;
    case "PageUp": return 0x149;
    case "ArrowLeft": return 0x14B;
    case "ArrowRight": return 0x14D;
    case "End": return 0x14F;
    case "ArrowDown": return 0x150;
    case "PageDown": return 0x151;
    case "Insert": return 0x152;
    case "Delete": return 0x153;
    case "OSLeft": return 0x15B;
    case "MetaLeft": return 0x15B;
    case "OSRight": return 0x15C;
    case "MetaRight": return 0x15C;
    case "ContextMenu": return 0x15D;
    case "BrowserSearch": return 0x165;
    case "BrowserFavorites": return 0x166;
    case "BrowserRefresh": return 0x167;
    case "BrowserStop": return 0x168;
    case "BrowserForward": return 0x169;
    case "BrowserBack": return 0x16A;
    }
}

const toScancode = (typeof InstallTrigger !== 'undefined') ? toScancode_Firefox : toScancode_Chromium44;

const unicodeScancodeTable = {
    Escape: 0x0001,
    Backspace: 0x000E,
    Tab: 0x000F,
    BracketLeft: 0x001A,
    BracketRight: 0x001B,
    Enter: 0x001C,
    ControlLeft: 0x001D,
    ShiftLeft: 0x002A,
    Period: 0x0034,
    ShiftRight: 0x0036,
    NumpadMultiply: 0x0037,
    AltLeft: 0x0038,
    CapsLock: 0x003A,
    F1: 0x003B,
    F2: 0x003C,
    F3: 0x003D,
    F4: 0x003E,
    F5: 0x003F,
    F6: 0x0040,
    F7: 0x0041,
    F8: 0x0042,
    F9: 0x0043,
    F10: 0x0044,
    Pause: 0x0045,
    ScrollLock: 0x0046,
    Numpad7: 0x0047,
    Numpad8: 0x0048,
    Numpad9: 0x0049,
    NumpadSubtract: 0x004A,
    Numpad4: 0x004B,
    Numpad5: 0x004C,
    Numpad6: 0x004D,
    NumpadAdd: 0x004E,
    Numpad1: 0x004F,
    Numpad2: 0x0050,
    Numpad3: 0x0051,
    Numpad0: 0x0052,
    NumpadDecimal: 0x0053,
    IntlBackslash: 0x0056,
    F11: 0x0057,
    F12: 0x0058,
    NumpadEqual: 0x0059,
    F13: 0x0064,
    F14: 0x0065,
    F15: 0x0066,
    F16: 0x0067,
    F17: 0x0068,
    F18: 0x0069,
    F19: 0x006A,
    F20: 0x006B,
    F21: 0x006C,
    F22: 0x006D,
    F23: 0x006E,
    KanaMode: 0x0070,
    Lang2: 0x0071,
    // Lang2: 0x1F1, /* key with Korean keyboard layout */
    Lang1: 0x0072,
    // Lang1: 0x1F2, /* key with Korean keyboard layout */
    IntlRo: 0x0073,
    F24: 0x0076,
    Convert: 0x0079,
    NonConvert: 0x007B,
    IntlYen: 0x007D,
    NumpadComma: 0x007E,
    Undo: 0x108,
    Paste: 0x10A,
    MediaTrackPrevious: 0x110,
    Cut: 0x117,
    Copy: 0x118,
    MediaTrackNext: 0x119,
    NumpadEnter: 0x11C,
    ControlRight: 0x11D,
    AudioVolumeMute: 0x120,
    VolumeMute: 0x120,
    LaunchApp2: 0x121,
    MediaPlayPause: 0x122,
    MediaStop: 0x124,
    Eject: 0x12C,
    AudioVolumeDown: 0x12E,
    VolumeDown: 0x12E,
    AudioVolumeUp: 0x130,
    VolumeUp: 0x130,
    BrowserHome: 0x132,
    NumpadDivide: 0x135,
    PrintScreen: 0x137,
    AltRight: 0x138,
    Help: 0x13B,
    NumLock: 0x145,
    Home: 0x147,
    ArrowUp: 0x148,
    PageUp: 0x149,
    ArrowLeft: 0x14B,
    ArrowRight: 0x14D,
    End: 0x14F,
    ArrowDown: 0x150,
    PageDown: 0x151,
    Insert: 0x152,
    Delete: 0x153,
    MetaLeft: 0x15B,
    OSLeft: 0x15B,
    MetaRight: 0x15C,
    OSRight: 0x15C,
    ContextMenu: 0x15D,
    Power: 0x15E,
    BrowserSearch: 0x165,
    BrowserFavorites: 0x166,
    BrowserRefresh: 0x167,
    BrowserStop: 0x168,
    BrowserForward: 0x169,
    BrowserBack: 0x16A,
    LaunchApp1: 0x16B,
    LaunchMail: 0x16C,
    LaunchMediaPlayer: 0x16D,
};
