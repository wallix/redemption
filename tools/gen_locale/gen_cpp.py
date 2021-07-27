#!/usr/bin/env python3
from typing import Optional, NamedTuple
from kbd_parser import KeymapType, KeyLayout, parse_argv
from collections import OrderedDict
import re

class Key2(NamedTuple):
    codepoint:int
    is_deadkey:int
    text:str

class Keymap2(NamedTuple):
    mod:str
    keymap:KeymapType
    dkeymap:list[int] # always 128 elements. 0 = no deadkey
    idx:int

class LayoutInfo(NamedTuple):
    layout:KeyLayout
    keymaps:list[Keymap2]

def load_layout_infos(layouts:list[KeyLayout], mods:dict,
                      unique_keymap:dict[Optional[tuple], int],
                      unique_deadkeys:dict[tuple, int]) -> list[LayoutInfo]:
    layouts2:list[LayoutInfo] = []
    for layout in layouts:
        keymaps = layout.keymaps
        keymap_for_layout = []
        for mod in mods:
            keymap = keymaps[mod]
            keys = []
            dkeys = []
            has_dkidx = False
            for key in keymap:
                idk = 0
                if key and (key.codepoint or key.text):
                    if key.deadkeys:
                        deadkeys = [(dk.accent, dk.with_, dk.codepoint)
                                    for dk in key.deadkeys.values()]
                        deadkeys.sort()
                        idk = unique_deadkeys.setdefault((*deadkeys,), len(unique_deadkeys)+1)
                        has_dkidx = True
                    dkeys.append(idk)
                    keys.append(Key2(codepoint=key.codepoint,
                                     is_deadkey=True if idk else False,
                                     text=key.text))
                else:
                    dkeys.append(0)
                    keys.append(None)
            assert (all(i == 0 for i in dkeys[128:]))
            idx = unique_keymap.setdefault((*keys,), len(unique_keymap))
            dkeys = (*dkeys[:128],) if has_dkidx else None
            keymap_for_layout.append(Keymap2(mod=mod, keymap=keymap, idx=idx, dkeymap=dkeys))
        layouts2.append(LayoutInfo(layout=layout, keymaps=keymap_for_layout))
    return layouts2


layouts:list[KeyLayout] = parse_argv()

supported_mods = OrderedDict({
    '': True,
    'VK_SHIFT': True,
    'VK_SHIFT VK_CAPITAL': True,
    'VK_SHIFT VK_CAPITAL VK_NUMLOCK': True,
    'VK_SHIFT VK_CONTROL': True,
    'VK_SHIFT VK_CONTROL VK_MENU VK_CAPITAL': True,
    'VK_SHIFT VK_CONTROL VK_MENU VK_CAPITAL VK_NUMLOCK': True,
    'VK_SHIFT VK_CONTROL VK_MENU VK_NUMLOCK': True,
    'VK_SHIFT VK_CONTROL VK_MENU': True,
    'VK_SHIFT VK_NUMLOCK': True,
    'VK_CAPITAL': True,
    'VK_CAPITAL VK_NUMLOCK': True,
    'VK_CONTROL': True,
    'VK_CONTROL VK_MENU': True,
    'VK_CONTROL VK_MENU VK_NUMLOCK': True,
    'VK_CONTROL VK_MENU VK_CAPITAL': True,
    'VK_CONTROL VK_MENU VK_CAPITAL VK_NUMLOCK': True,
    'VK_NUMLOCK': True,
    'VK_SHIFT VK_OEM_8': True,
    'VK_OEM_8': True,
})

error_messages = []
for layout in layouts:
    for mod,keymap in layout.keymaps.items():
        if mod in supported_mods:
            # check that codepoint <= 0x7fff
            if not all(not key or key.codepoint <= 0x7fff for key in keymap):
                error_messages.append(f'{mod or "NoMod"} for {layout.klid}/{layout.locale_name} have a codepoint greater that 0x7fff')
            # check that there is no deadkeys of deadkeys
            if not all(not key or key.deadkeys or all(d.deadkeys is None for d in key.deadkeys) for key in keymap):
                error_messages.append(f'{mod or "NoMod"} for {layout.klid}/{layout.locale_name} have a deadkeys of deadkeys')
        else:
            # check that unknown mod is empty
            if not all(key is None for key in keymap):
                error_messages.append(f'{mod or "NoMod"} for {layout.klid}/{layout.locale_name} is not null')
if error_messages:
    raise Exception('\n'.join(error_messages))


codepoint_to_char_table = {
    0x7: '\\a',
    0x8: '\\b',
    0x9: '\\t',
    0xa: '\\n',
    0xb: '\\v',
    0xc: '\\f',
    0xd: '\\r',
    0x27: '\\\'',
    0x5C: '\\\\',
}

unique_keymap = {(None,)*256: 0,}
unique_deadkeys = {}
layouts2 = load_layout_infos(layouts, supported_mods, unique_keymap, unique_deadkeys)

strings = [
    '#include "keyboard/keylayouts.hpp"\n\n',
    'constexpr auto DK = KeyLayout::DK;\n\n',
    'using KbdId = KeyLayout::KbdId;\n\n',
]

# print keymap (scancodes[256] with DK (mask for deadkey)
for keymap, idx in unique_keymap.items():
    strings.append(f'static constexpr KeyLayout::unicode_t keymap_{idx}[] {{\n')
    for i in range(256//8):
        char_comment = []
        has_char_comment = False
        strings.append(f'/*{i*8:02X}-{i*8+7:02X}*/ ')

        for j in range(i*8, i*8+8):
            if (key := keymap[j]) and (codepoint := key.codepoint):
                if not key.is_deadkey and (0x20 <= codepoint <= 0x7E or 0x7 <= codepoint <= 0xD):
                    c = codepoint_to_char_table.get(codepoint) or chr(codepoint)
                    c = f"'{c}'"
                    strings.append(f"{c:>9}, ")
                    char_comment.append('           ')
                elif key.is_deadkey and 0x20 <= codepoint <= 0x7E:
                    c = codepoint_to_char_table.get(codepoint) or chr(codepoint)
                    c = f"DK|'{c}'"
                    strings.append(f'{c:>9}, ')
                else:
                    strings.append(f'{"DK|" if key.is_deadkey else "   "}0x{codepoint:04x}, ')
                    if 0x20 <= codepoint <= 0x7E:
                        char_comment.append(f'{chr(codepoint):>10} ')
                        has_char_comment = True
                    elif 0x07 <= codepoint <= 0x0D:
                        char_comment.append(f'\\{"abtnvfr"[codepoint - 0x7]:>10} ')
                        has_char_comment = True
                    elif codepoint == 0x1b:
                        char_comment.append('       ESC ')
                        has_char_comment = True
                    elif codepoint > 127:
                        char_comment.append(f'{key.text:>10} ')
                        has_char_comment = True
                    else:
                        char_comment.append('           ')
            else:
                strings.append('        0, ')
                char_comment.append('           ')

        strings.append('\n')
        if has_char_comment:
            strings.append('       //')
            strings += char_comment
            strings.append('\n')

    strings.append('};\n\n')

# print deadkeys map (only when a keymap has at least 1 deadkey)
unique_deadkeys = {v:k for k,v in unique_deadkeys.items()}
for idx,deadkeys in unique_deadkeys.items():
    accent = next(iter(deadkeys))[0]
    strings.append(f'static constexpr KeyLayout::DKeyTable::Data dkeydata_{idx}[] {{\n')
    strings.append(f'    {{.meta={{.size={len(deadkeys)}, .accent=0x{ord(accent):04X} /* {accent} */}}}},\n')
    strings += (f'    {{.dkey={{.second=0x{ord(with_):04X} /* {with_} */, .result=0x{codepoint:04X} /* {chr(codepoint)} */}}}},\n' for accent, with_, codepoint in deadkeys)
    strings.append('};\n\n')

# dkeymap memoization
dktables = {(0,)*128: 0,}
for layout, keymaps in layouts2:
    for mod, keymap, dkeymap, idx in keymaps:
        if dkeymap:
            dktables.setdefault(dkeymap, len(dktables))

# print dkeymap (DKeyTable[])
for deadmap,idx in dktables.items():
    strings.append(f'static constexpr KeyLayout::DKeyTable dkeymap_{idx}[] {{\n')
    for i in range(128//8):
        strings.append('    ')
        strings += (f'{f"{{dkeydata_{deadmap[j]}}},":<16}' if deadmap[j] else '{nullptr},      ' \
                    for j in range(i*8, i*8+8))
        strings.append('\n')
    strings.append('};\n\n')


mods_to_mask = {
    '': 0,
    'VK_SHIFT': 1,
    'VK_CONTROL': 2,
    'VK_MENU': 4,
    'VK_NUMLOCK': 8,
    'VK_CAPITAL': 16,
    'VK_OEM_8': 32,
}

# prepare keymap_mod and dkeymap_mod
unique_layout_keymap = {}
unique_layout_dkeymap = {}
strings2 = ['static constexpr KeyLayout layouts[] {\n']
for layout in layouts2:
    mods_array = [0]*64
    dmods_array = [0]*64
    for mod, keymap, dkeymap, idx in layout.keymaps:
        mask = sum(mods_to_mask[m] for m in mod.split(' '))
        mods_array[mask] = idx
        if dkeymap:
            dmods_array[mask] = dktables[dkeymap]
    k1 = (*mods_array,)
    k2 = (*dmods_array,)
    k1 = unique_layout_keymap.setdefault(k1, len(unique_layout_keymap))
    k2 = unique_layout_dkeymap.setdefault(k2, len(unique_layout_dkeymap))
    layout = layout.layout
    strings2.append(f'    KeyLayout{{KbdId(0x{layout.klid}), KeyLayout::RCtrlIsCtrl({layout.has_right_ctrl_like_oem8 and "false" or "true "}), "{layout.proxy_name or layout.locale_name}"_zv/*, "{layout.display_name}"_zv, "{layout.locale_name}"_zv*/, keymap_mod_{k1}, dkeymap_mod_{k2}}},\n')
strings2.append('};\n')

# print layout
for unique_layout,prefix,atype in (
    (unique_layout_keymap, '', 'KeyLayout::unicode_t, 256'),
    (unique_layout_dkeymap, 'd', 'KeyLayout::DKeyTable, 128')
):
    for k,idx in unique_layout.items():
        strings.append(f'static constexpr sized_array_view<{atype}> {prefix}keymap_mod_{idx}[] {{\n')
        for i in range(64//8):
            strings.append('   ')
            strings += (f' {prefix}keymap_{k[i]},' for i in range(i*8, i*8+8))
            strings.append('\n')
        strings.append('};\n\n')

# print layout
strings2.append('\narray_view<KeyLayout> keylayouts() noexcept\n')
strings2.append('{\n    return layouts;\n}\n\n')
strings2.append('KeyLayout const* find_layout_by_id(KeyLayout::KbdId id) noexcept\n')
strings2.append('{\n    switch (id)\n    {\n')
for i,layout in enumerate(layouts):
    strings2.append(f'    case KbdId{{0x{layout.klid}}}: return &layouts[{i}];\n')
strings2.append('    }\n    return nullptr;\n}\n')

output = f"{''.join(strings)}\n{''.join(strings2)}"
output = re.sub(' +\n', '\n', output)
print(output)
