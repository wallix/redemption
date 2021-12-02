"use strict";

// import {ScancodeByMod, SyncFlags, KeyRelease, KeyAcquire, numpadCodeToScancode, codeToScancodes} from './scancodes.js';

const isMods = function(key) {
    switch (key) {
        case 'Control':
        case 'Shift':
        case 'Alt':
        case 'AltGraph':
        case 'NumLock':
        case 'ScrollLock':
        case 'CapsLock':
        case 'OS':
            return true;
    }
    return false;
};

const isComposing = function(key) {
    switch (key) {
        case 'Dead':
        case 'Compose':
        case 'Process':
        case 'Unidentified':
            return true;
    }
    return false;
};

class CompositeKeyState
{
    // driver = class {
    //  compositeLater()
    // }
    constructor(driver) {
        this._isComposing = false;
        this._driver = driver;
    }

    update(evt) {
        if (!this.isComposing) {
            if (isComposing(evt.key)) {
                this.isComposing = true;
                return true;
            }
        }
        else if (evt.isComposing) {
            if (!isMods(evt.key)) {
                return true;
            }
        }
        else {
            this._driver.compositeLater();
            this.isComposing = false;
            return true;
        }

        return false;
    }
}

class Keyboard
{
    // driver = class {
    //  sendScancodes(scancodes:Array[Number]):bool
    //  syncKbdLocks(syncFlags:Number)
    // }
    constructor(keymap, driver) {
        this._driver = driver;
        this._keymap = keymap;
        this._syncState = 0;
        // on window, when left + right shift are pressed, onkeyup is only triggered for the last one
        this._shiftDown = 0;
    }

    copyInternalStateFrom(other) {
        this._syncState = other._syncState;
        this._shiftDown = other._shiftDown;
        this._keymap = other._keymap;
    }

    activeModsSync() {
        this._syncState = 1;
    }

    keyup(evt) {
        if (!this._syncState && evt.key === 'Shift') {
            this._shiftDown &= ~this._codeToShiftMod(evt.code);

            // shift is released but one of the sides is still considered pressed
            if (this._shiftDown && !evt.getModifierState('Shift')) {
                const scancodes = [];
                if (this._shiftDown & SyncFlags.ShiftRight) {
                    scancodes.push(ScancodeByMod.ShiftRight | KeyRelease);
                }
                if (this._shiftDown & SyncFlags.ShiftLeft) {
                    scancodes.push(ScancodeByMod.ShiftLeft | KeyRelease);
                }
                this._shiftDown = 0;
                this._driver.sendScancodes(scancodes);
                return true;
            }
        }
        return this._keyEvent(evt, KeyRelease);
    }

    keydown(evt) {
        if (!this._syncState && evt.key === 'Shift') {
            this._shiftDown |= this._codeToShiftMod(evt.code);
        }
        return this._keyEvent(evt, KeyAcquire);
    }

    sendComposition(text) {
    }

    _keyEvent(evt, flag) {
        if (!this._preprocessKeyEvent(evt, flag)) return true;

        // numpad
        if (evt.location === KeyboardEvent.DOM_KEY_LOCATION_NUMPAD) {
            const scancode = numpadCodeToScancode(evt.code);
            this._driver.sendScancodes([scancode | flag]);
            return true;
        }

        const scancodes = codeToScancodes(evt.key, flag) || codeToScancodes(evt.code, flag);
        if (scancodes) {
            return this._driver.sendScancodes(scancodes);
        }
    }

    /// \return false when key processing should stop
    _preprocessKeyEvent(evt, flag) {
        if (!this._syncState) return true;

        let syncFlags = 0;

        // locks
        if (evt.getModifierState("NumLock")) syncFlags |= SyncFlags.NumLock;
        if (evt.getModifierState("CapsLock")) syncFlags |= SyncFlags.CapsLock;
        if (evt.getModifierState("KanaLock")) syncFlags |= SyncFlags.KanaLock;
        if (evt.getModifierState("ScrollLock")) syncFlags |= SyncFlags.ScrollLock;

        // locks are always properly synchronized and only need to be sent once
        if (this._syncState === 1) {
            this._driver.syncKbdLocks(syncFlags);
            this._syncState = 2;
        }

        if (evt.altKey) syncFlags |= SyncFlags.AltLeft;
        if (evt.getModifierState("AltGraph")) syncFlags |= SyncFlags.AltRight;

        // ambiguously mods
        let leftOrRight = 0;
        if (evt.metaKey) leftOrRight |= (evt.code === 'OSRight' && flag === KeyAcquire) ? SyncFlags.OSRight : SyncFlags.OSLeft;
        if (evt.ctrlKey) leftOrRight |= (evt.code === 'ControlRight' && flag === KeyAcquire) ? SyncFlags.ControlRight : SyncFlags.ControlLeft;
        if (evt.shiftKey) leftOrRight |= (evt.code === 'ShiftRight' && flag === KeyAcquire) ? SyncFlags.ShiftRight : SyncFlags.ShiftLeft;

        syncFlags |= leftOrRight;
        this._driver.sendScancodes(scancodesForSynchronizedMods(syncFlags));

        this._keymap.sync(syncFlags);
        this._shiftDown = leftOrRight & (SyncFlags.ShiftRight | SyncFlags.ShiftLeft);

        // mods are unambiguously synchronized
        if (!leftOrRight) {
            this._syncState = 0;
        }

        switch (evt.code) {
            // mods are already processed, no need to continue
            case "OSLeft":
            case "OSRight":
            case "ShiftLeft":
            case "ShiftRight":
            case "ControlLeft":
            case "ControlRight":
                return false;
        }

        return true;
    }

    _codeToShiftMod(code) {
        if (code === 'ShiftRight') return SyncFlags.ShiftRight;
        if (code === 'ShiftLeft') return SyncFlags.ShiftLeft;
        return 0;
    }
}

class EmulatedKeyboard extends Keyboard
{
    // driver = class {
    //  sendUnicode(unicode:String, flag:Number):bool
    //  sendScancodes(scancodes:Array[Number]):bool
    //  syncKbdLocks(syncFlags:Number)
    //  compositeLater()
    // }
    constructor(keymap, driver) {
        super(keymap, driver)
        this._hasUnicodeSupport = false;
        this._composite = new CompositeKeyState(driver);
    }

    setUnicodeSupport(enable) {
        this._hasUnicodeSupport = enable;
    }

    sendComposition(text) {
        const scancodes = this._keymap.toScancodesAndFlags(text, text, KeyAcquire);
        if (scancodes) {
            this._driver.sendScancodes(this._unstatedScancodes(scancodes));
        }
        else if (this._hasUnicodeSupport) {
            this._driver.sendUnicode(text, KeyAcquire);
            this._driver.sendUnicode(text, KeyRelease);
        }
    }

    // release pressed keys and press released keys
    _unstatedScancodes(scancodes) {
        const set = new Set();
        for (const scancode of scancodes) {
            if (scancode & KeyRelease)
                set.delete(scancode & ~KeyRelease);
            else
                set.add(scancode);
        }

        const len = scancodes.length;
        for (let i = 0; i < len; ++i) {
            const scancode = scancodes[i];
            if (!(scancode & KeyRelease) && set.has(scancode)) {
                scancodes.push(scancode & KeyRelease);
            }
        }

        return scancodes;
    }

    _keyEvent(evt, flag) {
        if (!this._preprocessKeyEvent(evt, flag)) return true;

        if (this._composite.update(evt)) return false;

        // numpad
        if (evt.location === KeyboardEvent.DOM_KEY_LOCATION_NUMPAD) {
            const scancode = numpadCodeToScancode(evt.code);
            this._driver.sendScancodes([scancode | flag]);
            return true;
        }

        const scancodes = this._keymap.toScancodesAndFlags(evt.key, evt.code, flag);
        if (scancodes) {
            return this._driver.sendScancodes(scancodes);
        }
        else if (this._hasUnicodeSupport) {
            return this._driver.sendUnicode(evt.key, flag);
        }
    }
}

class UnicodeKeyboard extends Keyboard
{
    // driver = class {
    //  sendUnicode(unicode:String, flag:Number):bool
    //  sendScancodes(scancodes:Array[Number]):bool
    //  syncKbdLocks(syncFlags:Number)
    //  compositeLater()
    // }
    constructor(keymap, driver) {
        super(keymap, driver)
        this._composite = new CompositeKeyState(driver);
    }

    sendComposition(text) {
        this._driver.sendUnicode(text, KeyAcquire);
        this._driver.sendUnicode(text, KeyRelease);
    }

    _keyEvent(evt, flag) {
        if (!this._preprocessKeyEvent(evt, flag)) return true;

        if (this._composite.update(evt)) return false;

        if (evt.key.length === 1 || evt.key.charCodeAt(0) > 127) {
            return this._driver.sendUnicode(evt.key, flag);
        }
        else {
            const scancodes = codeToScancodes(evt.key, flag) || codeToScancodes(evt.code, flag);
            if (scancodes) {
                return this._driver.sendScancodes(scancodes);
            }
        }
    }
}

try {
    module.exports.Keyboard = Keyboard;
    module.exports.EmulatedKeyboard = EmulatedKeyboard;
    module.exports.UnicodeKeyboard = UnicodeKeyboard;
}
catch (e) {
    // module not found
}
