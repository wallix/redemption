#!/usr/bin/env python3
from typing import Optional, NamedTuple
from collections import OrderedDict
import sys
import os

sys.path.append(os.path.abspath(os.path.dirname(os.path.abspath(__file__)) + '../../../../tools/gen_locale'))
from kbd_parser import KeymapType, KeyLayout, Key, parse_argv


vk_control_masks = {
    '':             0,
    'VK_SHIFT':     0x10000 << 0,
    'VK_CONTROL':   0x10000 << 1,
    'VK_MENU':      0x10000 << 2,
    'VK_NUMLOCK':   0x10000 << 3,
    'VK_CAPITAL':   0x10000 << 4,
    'VK_OEM_8':     0x10000 << 5,
    'VK_KANA':      0x10000 << 6,
    'VK_KANALOCK':  0x10000 << 7,
}

vk_actions = {
    'VK_APPS': ('ContextMenu', 0x15D),
    'VK_BROWSER_BACK': ('BrowserBack', 0x16A),
    'VK_BROWSER_FAVORITES': ('BrowserFavorites', 0x166),
    'VK_BROWSER_FORWARD': ('BrowserForward', 0x169),
    'VK_BROWSER_HOME': ('BrowserHome', 0x132),
    'VK_BROWSER_REFRESH': ('BrowserRefresh', 0x167),
    'VK_BROWSER_SEARCH': ('BrowserSearch', 0x165),
    'VK_BROWSER_STOP': ('BrowserStop', 0x168),
    'VK_CLEAR': ('Clear', 0x59),
    'VK_DBE_NOCODEINPUT': ('CodeInput', 0x62),
    'VK_DELETE': ('Delete', 0x153),
    'VK_DOWN': ('ArrowDown', 0x150),
    'VK_END': ('End', 0x14F),
    'VK_F1': ('F1', 0x3B),
    'VK_F2': ('F2', 0x3C),
    'VK_F3': ('F3', 0x3D),
    'VK_F4': ('F4', 0x3E),
    'VK_F5': ('F5', 0x3F),
    'VK_F6': ('F6', 0x40),
    'VK_F7': ('F7', 0x41),
    'VK_F8': ('F8', 0x42),
    'VK_F9': ('F9', 0x43),
    'VK_F10': ('F10', 0x44),
    'VK_F11': ('F11', 0x57),
    'VK_F12': ('F12', 0x58),
    'VK_F13': ('F13', 0x64),
    'VK_F14': ('F14', 0x65),
    'VK_F15': ('F15', 0x66),
    'VK_F16': ('F16', 0x67),
    'VK_F17': ('F17', 0x68),
    'VK_F18': ('F18', 0x69),
    'VK_F19': ('F19', 0x6A),
    'VK_F20': ('F20', 0x6B),
    'VK_F21': ('F21', 0x6C),
    'VK_F22': ('F22', 0x6D),
    'VK_F23': ('F23', 0x6E),
    'VK_F24': ('F24', 0x76),
    'VK_HELP': ('Help', 0x63),
    'VK_HOME': ('Home', 0x147),
    'VK_INSERT': ('Insert', 0x152),
    'VK_LAUNCH_APP1': ('LaunchApp1', 0x16B),
    'VK_LAUNCH_APP2': ('LaunchApp2', 0x121),
    'VK_LAUNCH_MAIL': ('LaunchMail', 0x16C),
    'VK_LAUNCH_MEDIA_SELECT': ('LaunchMediaPlayer', 0x16D),
    'VK_LEFT': ('ArrowLeft', 0x14B),
    'VK_MEDIA_NEXT_TRACK': ('MediaTrackNext', 0x119),
    'VK_MEDIA_PLAY_PAUSE': ('MediaPlayPause', 0x122),
    'VK_MEDIA_PREV_TRACK': ('MediaTrackPrevious', 0x110),
    'VK_MEDIA_STOP': ('MediaStop', 0x124),
    'VK_NEXT': ('PageDown', 0x151),
    'VK_PRIOR': ('PageUp', 0x149),
    'VK_RIGHT': ('ArrowRight', 0x14D),
    'VK_SCROLL': ('ScrollLock', 0x46),
    'VK_SLEEP': ('Standby', 0x15F),
    'VK_SNAPSHOT': ('PrintScreen', 0x137),
    'VK_UP': ('ArrowUp', 0x148),
    'VK_VOLUME_DOWN': ('AudioVolumeDown', 0x12E),
    'VK_VOLUME_MUTE': ('AudioVolumeMute', 0x120),
    'VK_VOLUME_UP': ('AudioVolumeUp', 0x130),
}

vk_actions_dup = set((
    ('VK_SNAPSHOT', 0x54),
))

vk_unknowns = set((
    'VK_ABNT_C1', # : ('', 0)
    'VK_ABNT_C2', # : ('', 0x7e)
    'VK_DBE_FLUSHSTRING', # : ('', 0x5D),
    'VK_DBE_KATAKANA', # : ('', 0x),
    'VK_HANJA', # : ('HanjaMode', 0x),
    'VK_IME_OFF', # : ('', 0x1F1),
    'VK_IME_ON', # : ('', 0x1F2),
    'VK_OEM_102', # : ('', 0x56),
    'VK_OEM_1', # : ('', 0x27),
    'VK_OEM_2', # : ('', 0x35),
    'VK_OEM_3', # : ('', 0x29),
    'VK_OEM_4', # : ('', 0x1A),
    'VK_OEM_5', # : ('', 0x2B),
    'VK_OEM_6', # : ('', 0x1B),
    'VK_OEM_7', # : ('', 0x28),
    'VK_OEM_8', # : ('', 0x11D),
    'VK_OEM_AUTO', # : ('Hankaku', 0x5F),
    'VK_OEM_BACKTAB', # : ('Romaji', 0x5E),
    'VK_OEM_COMMA', # : ('', 0x33),
    'VK_OEM_JUMP', # : ('', 0x5C),
    'VK_OEM_MINUS', # : ('', 0x0C),
    'VK_OEM_PA1', # : ('', 0x7B),
    'VK_OEM_PA2', # : ('', 0x7F),
    'VK_OEM_PA3', # : ('', 0x6F),
    'VK_OEM_PERIOD', # : ('', 0x34),
    'VK_OEM_PLUS', # : ('', 0x0D),
    'VK_OEM_RESET', # : ('', 0x71),
    'VK_OEM_WSCTRL', # : ('', 0x5A),

    'VK_LCONTROL',
    'VK_LSHIFT',
    'VK_RSHIFT',
    'VK_LMENU',
    'VK_CAPITAL',
    'VK_NUMLOCK',
    'VK_RCONTROL',
    'VK_RMENU',
    'VK_LWIN',
    'VK_RWIN',
    'VK_KANA',
))

char_to_char_table = {
    '\a': '\\a',
    '\b': '\\b',
    '\t': '\\t',
    '\n': '\\n',
    '\v': '\\v',
    '\f': '\\f',
    '\r': '\\r',
    '\\': '\\\\',
    '\'': '\\\'',
}

def vk_mod_to_mod_flags(mods:str) -> int:
    mod_flags = 0
    for m in mods.split(' '):
        mod_flags |= vk_control_masks[m]
    return mod_flags

def key_to_value(key:Key, mod_flags:int) -> int:
    extended = 0x100 if key.is_extended() else 0
    scancode = key.scancode & 0x7f
    return mod_flags | extended | scancode


layouts:list[KeyLayout] = parse_argv()

error_messages = []
output = ['const layouts = [\n']

for layout in layouts:
    # text character and action
    rkeymap = {}
    actions_keymap = {}
    for mods,keymap in layout.keymaps.items():
        mod_flags = vk_mod_to_mod_flags(mods)
        for key in keymap:
            if key and not key.deadkeys:
                if key.text:
                    map:list = rkeymap.setdefault((key.text, key.codepoint), [])
                    value = key_to_value(key, mod_flags)
                    map.append(f'0x{value:x}, ')
                elif key_and_scancode := vk_actions.get(key.vk, None):
                    if mod_flags not in (0, 0x80000): # noMod and numLock
                        error_messages.append(f'Action key with control key ({key.vk} + {mods or "noMod"}) in {layout.display_name} (0x{layout.klid})')
                    else:
                        scancode = (key.scancode & 0x7f) | (0x100 if key.scancode & 0x80 else 0)
                        if key_and_scancode[1] != scancode and (key.vk, scancode) not in vk_actions_dup:
                            actions_keymap[key_and_scancode[0]] = scancode
                elif key.vk is not None and key.vk not in vk_unknowns:
                    if not (key.codepoint == 0 and mod_flags in (0, 0x80000, 0x20000)): # noMod numLock ctrl
                        error_messages.append(f'Unknown {key} + {mods or "noMod"} in {layout.display_name} (0x{layout.klid})')

    # dead key and dead key of dead key
    rdeadkeymap = {}
    rdeadkeymap2 = {}
    for mods,keymap in layout.keymaps.items():
        mod_flags = vk_mod_to_mod_flags(mods)
        for key in keymap:
            if key and key.deadkeys:
                value = key_to_value(key, mod_flags)
                for dkey in key.deadkeys.values():
                    if not dkey.deadkeys:
                        map:tuple[set,set] = rdeadkeymap.setdefault(dkey.text, (set(), set()))
                        map[0].add(f'0x{value:x}, ')
                        map[1].add(f"'{char_to_char_table.get(dkey.with_, dkey.with_)}', ")
                    else:
                        m0 = f'0x{value:x}, '
                        m1 = f"'{char_to_char_table.get(dkey.with_, dkey.with_)}', "
                        for dkey2 in dkey.deadkeys.values():
                            assert not dkey2.deadkeys
                            map:tuple[set,set] = rdeadkeymap2.setdefault(dkey2.text, (set(), set(), set()))
                            map[0].add(m0)
                            map[1].add(m1)
                            map[2].add(f"'{char_to_char_table.get(dkey2.with_, dkey2.with_)}', ")

    # remove duplicate dead key
    for k in rdeadkeymap2:
        rdeadkeymap.pop(k, None)


    output.append(f'{{\n  klid: 0x{layout.klid},\n  localeName: "{layout.locale_name}",\n  displayName: "{layout.display_name}",\n  ctrlRightIsOem8: {"true" if layout.has_right_ctrl_like_oem8 else "false"},\n  keymap: {{\n')

    for (text, codepoint), rkeys in rkeymap.items():
        k = char_to_char_table.get(text) or (text if text.isprintable() else f'\\x{codepoint:02x}')
        output.append(f"    '{k}': [{''.join(rkeys)}],\n")

    for name, scancode in actions_keymap.items():
        output.append(f"    '{name}': [0x{scancode|0xff0000:x}],\n")

    output.append('  },\n  deadkeys: {\n')
    output.append('    // text: [accenref, VKs]\n')

    accents = dict()
    push_ref = lambda rkeys1: accents.setdefault(''.join(rkeys1), len(accents))

    for text, (rkeys1, rkeys2) in rdeadkeymap.items():
        text = char_to_char_table.get(text, text)
        output.append(f"    '{text}': [{push_ref(rkeys1)}, [{''.join(rkeys2)}]],\n")

    for text, (rkeys1, rkeys2, rkeys3) in rdeadkeymap2.items():
        text = char_to_char_table.get(text, text)
        output.append(f"    '{text}': [{push_ref(rkeys1)}, [{''.join(rkeys2)}], [{''.join(rkeys3)}]],\n")

    output.append('  },\n  accents: [\n')

    for scancodes in accents:
        output.append(f'    [{scancodes}],\n')

    output.append('  ]\n},\n')

output.append('];\n\n')

output.append('const actionLayout = {')
for key_and_scancode in vk_actions.values():
    output.append(f'  "{key_and_scancode[0]}": 0x{key_and_scancode[1]:x},\n')
output.append('};')

# output.append('\n\nconst layoutsByKlid = {\n')
# for i,layout in enumerate(layouts):
#     output.append(f'  0x{layout.klid}: layouts[{i}],\n')
# output.append('};\n\nconst layoutsByDisplayName = {\n')
# for i,layout in enumerate(layouts):
#     output.append(f'  "{layout.display_name}": layouts[{i}],\n')
# output.append('};')

print(''.join(output))

if error_messages:
    print(f'{len(error_messages)} error(s):', file=sys.stderr)
    print('\n'.join(error_messages), file=sys.stderr)
    exit(1)
