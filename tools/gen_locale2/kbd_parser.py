#!/usr/bin/env python3
from typing import Optional, NamedTuple

from xml.etree import ElementTree
from itertools import chain
import sys


# key = (accent, with_key)
DeadKeysType = dict[tuple[str,str], 'DeadKey']

class DeadKey(NamedTuple):
    accent:str
    with_:str
    text:str
    codepoint:int
    deadkeys:DeadKeysType

class Key(NamedTuple):
    scancode:int
    codepoint:int
    text:str
    vk:str
    deadkeys:DeadKeysType

KeymapType = list[Optional[Key]] # always 128 elements
KeymapsType = dict[str, KeymapType]

class KeyLayout(NamedTuple):
    klid:int
    locale_name:str
    display_name:str
    normal_keymaps:KeymapsType
    # 0xEO
    extended_keymaps:KeymapsType
    extra_scancodes:dict[int, Key]


def _getattribs(log, node, nodename, attrs):
    if node.tag != nodename:
        raise Exception(f'tag = {node.tag}, but {nodename} expected')

    d = {k:None for k in attrs}

    for k,v in node.attrib.items():
        if k not in d:
            raise Exception(f'{node.tag}: unknown {k}')
        d[k] = v

    # check that all mandatory attributes are extracted
    for k,v in attrs.items():
        if v and d[k] is None:
            raise Exception(f'{node.tag}: {k} is missing')

    log(nodename, d)
    return d.values()


def _parse_deadkeys(log, dead_key_table, deadkeys):
    accent, name = _getattribs(log, dead_key_table, 'DeadKeyTable', {'Accent': True, 'Name': False})
    for result in dead_key_table:
        text, with_ = _getattribs(log, result, 'Result', {'Text': False, 'With': True})
        deadkey = DeadKey(accent=accent, with_=with_, text=text,
                          codepoint=text and ord(text) or 0,
                          deadkeys=None if text else dict())
        k = (accent, with_)
        assert k is not deadkeys
        deadkeys[k] = deadkey
        # double dead keys
        if not text:
            assert len(result) == 1
            _parse_deadkeys(log, result[0], deadkey.deadkeys)
    return accent

def verbose_print(*args):
    print(*args, file=sys.stderr)

_vk_to_codepoint_map = {
    'VK_F1': 0x003B,
    'VK_F2': 0x003C,
    'VK_F3': 0x003D,
    'VK_F4': 0x003E,
    'VK_F5': 0x003F,
    'VK_F6': 0x0040,
    'VK_F7': 0x0041,
    'VK_F8': 0x0042,
    'VK_F9': 0x0043,
    'VK_F10': 0x0044,
    'VK_F11': 0x0057,
    'VK_F12': 0x0058,
    'VK_F13': 0x0064,
    'VK_F14': 0x0065,
    'VK_F15': 0x0066,
    'VK_F16': 0x0067,
    'VK_F17': 0x0068,
    'VK_F18': 0x0069,
    'VK_F19': 0x006A,
    'VK_F20': 0x006B,
    'VK_F21': 0x006C,
    'VK_F22': 0x006D,
    'VK_F23': 0x006E,
}

_merge_numlock = (
    ('VK_SHIFT VK_NUMLOCK', 'VK_SHIFT'),
    ('VK_SHIFT VK_KANA VK_NUMLOCK', 'VK_SHIFT VK_KANA'),
    ('VK_SHIFT VK_CONTROL VK_MENU VK_NUMLOCK', 'VK_SHIFT VK_CONTROL VK_MENU'),
    ('VK_CONTROL VK_MENU VK_NUMLOCK', 'VK_CONTROL VK_MENU'),
    ('VK_NUMLOCK', ''),
    ('VK_KANA VK_NUMLOCK', 'VK_KANA'),
)

_merge_numlock_capital = (
    ('VK_SHIFT VK_NUMLOCK', 'VK_SHIFT VK_CAPITAL', 'VK_SHIFT VK_CAPITAL VK_NUMLOCK'),
    ('VK_SHIFT VK_CONTROL VK_MENU VK_NUMLOCK', 'VK_SHIFT VK_CONTROL VK_MENU VK_CAPITAL',
     'VK_SHIFT VK_CONTROL VK_MENU VK_CAPITAL VK_NUMLOCK'),
    ('VK_CONTROL VK_MENU VK_NUMLOCK', 'VK_CONTROL VK_MENU VK_CAPITAL',
     'VK_CONTROL VK_MENU VK_CAPITAL VK_NUMLOCK'),
    ('VK_NUMLOCK', 'VK_CAPITAL', 'VK_CAPITAL VK_NUMLOCK'),
)

def parse_xml_layout(filename, log=verbose_print):
    # VK_MENU = Alt
    # VK_CAPITAL = CapsLock
    # VK_CONTROL + VK_MENU = AltGr
    keymap_mods = {
        0x00: {k:[None]*128 for k in (
            '',
            'VK_SHIFT',
            'VK_SHIFT VK_CONTROL',
            'VK_SHIFT VK_CAPITAL',
            'VK_SHIFT VK_NUMLOCK',
            'VK_SHIFT VK_KANA',
            'VK_SHIFT VK_OEM_8',
            'VK_SHIFT VK_CONTROL VK_MENU',
            'VK_SHIFT VK_CONTROL VK_KANA',
            'VK_SHIFT VK_KANA VK_NUMLOCK',
            'VK_SHIFT VK_CONTROL VK_MENU VK_CAPITAL',
            'VK_SHIFT VK_CONTROL VK_MENU VK_NUMLOCK',
            'VK_CONTROL',
            'VK_CONTROL VK_MENU',
            'VK_CONTROL VK_KANA',
            'VK_CONTROL VK_MENU VK_NUMLOCK',
            'VK_CONTROL VK_MENU VK_CAPITAL',
            'VK_CAPITAL',
            'VK_NUMLOCK',
            'VK_OEM_8',
            'VK_KANA',
            'VK_KANA VK_NUMLOCK',
        )},
        # extended
        0xE0: {k:[None]*128 for k in (
            '',
            'VK_SHIFT',
            'VK_CONTROL',
            'VK_KANA',
            'VK_SHIFT VK_KANA',
            'VK_CONTROL VK_KANA',
        )},
    }
    extra_scancodes = {}

    # ascii letter for dead keys
    # deadtexts:dict[str, int] = {}

    root = ElementTree.parse(filename).getroot()
    klid, locale_name, display_name = _getattribs(log, root[0], 'metadata', {'KLID': True,
                                                                             'LocaleName': True,
                                                                             'LayoutDisplayName': True})
    for pk in root[1]:
        sc, vk, _ = _getattribs(log, pk, 'PK', {'SC': True, 'VK': True, 'Name': False})
        sc = int(sc, 16)

        # Pause
        if sc > 0xE07F:
            extra_scancodes[sc] = Key(scancode=sc, codepoint=0, text='', vk=vk, deadkeys=dict())
            continue

        keymaps = keymap_mods[sc >> 8]
        sc = sc & 0xff
        assert sc

        if not pk:
            keys = keymaps['']
            assert sc not in keys
            keys[sc] = Key(scancode=sc, codepoint=0, text='', vk=vk, deadkeys=dict())
            continue

        assert sc < 128

        for result in pk:
            text, codepoint, vk, with_ = _getattribs(log, result, 'Result', {'Text': False,
                                                                             'TextCodepoints': False,
                                                                             'VK': False,
                                                                             'With': False})
            keys = keymaps[with_ or '']

            if sc in keys:
                raise Exception(f'key {sc} ({text}/{codepoint}) already set')

            if text or codepoint:
                if codepoint:
                    assert not text
                    codepoint = int(codepoint, 16)
                    text = codepoint.to_bytes((codepoint.bit_length() + 7) // 8, byteorder='big'
                                              ).decode('utf8')
                elif len(text) == 1:
                    codepoint = ord(text)
                else:
                    # multi char
                    codepoint = 0

                keys[sc] = Key(scancode=sc, codepoint=codepoint, text=text, vk=vk, deadkeys=dict())

            # dead keys
            elif len(result):
                assert not vk
                assert len(result) == 1
                deadkeys = dict()
                accent = _parse_deadkeys(log, result[0], deadkeys)
                assert len(accent) == 1
                assert deadkeys
                keys[sc] = Key(scancode=sc, codepoint=ord(accent), text=accent, vk=vk, deadkeys=deadkeys)

    normal_keymaps_mods = keymap_mods[0]

    # new keymap: shiftlock + numlock
    for num_mod, caps_mod, final_mod in _merge_numlock_capital:
        assert final_mod not in normal_keymaps_mods
        caps_keymap = normal_keymaps_mods[caps_mod]
        numlock_keymap = normal_keymaps_mods[num_mod]
        keymap = [None] * 128
        for i in range(128):
            k = caps_keymap[i]
            knum = numlock_keymap[i]
            assert not (k and knum)
            keymap[i] = k or knum
        normal_keymaps_mods[final_mod] = keymap

    # merge mod to numlock mod
    for num_mod, merged_mod in _merge_numlock:
        keymap = normal_keymaps_mods[merged_mod]
        numlock_keymap = normal_keymaps_mods[num_mod]
        for i in range(128):
            k = keymap[i]
            knum = numlock_keymap[i]
            assert not (k and knum)
            if k:
                numlock_keymap[i] = k

    return KeyLayout(klid, locale_name, display_name,
                     normal_keymaps_mods, keymap_mods[0xE0], extra_scancodes)

def _accu_scancodes(strings:list[str], map:list[Key]):
    for i,k in enumerate(map):
        if k:
            text = k.text
            if k.codepoint < 32:
                if 0x07 <= k.codepoint <= 0x0D:
                    text = ('\\a', '\\b', '\\t', '\\n', '\\v', '\\f', '\\r')[k.codepoint - 0x07]
                elif k.codepoint:
                    text = ''
            elif k.codepoint == 127:
                text = ''
            strings.append(f"  0x{i:02X}: codepoint=0x{k.codepoint:04x} text='{text}' vk='{k.vk}'\n")
            if k.deadkeys:
                strings.append('       DeadKeys: ')
                prefix = ''
                for dk in k.deadkeys.values():
                    strings.append(f'{prefix}{dk.accent} + {dk.text} => {dk.result}\n')
                    prefix = '                 '
        else:
            strings.append(f"  0x{i:02X} -\n")

def print_layout(layout:KeyLayout, printer=sys.stdout.write):
    strings = [f'KLID: {layout.klid}\nLocalName: {layout.locale_name}\nDisplayName: {layout.display_name}\n']

    for mapname,map in layout.normal_keymaps.items():
        strings.append(f'{mapname or "normal"} (0x00)\n')
        _accu_scancodes(strings, map)

    for mapname,map in layout.extended_keymaps.items():
        strings.append(f'{mapname or "normal"} (0xE0)\n')
        _accu_scancodes(strings, map)

    strings.append('extra:\n')
    for i,k in layout.extra_scancodes.items():
        strings.append(f"  0x{i:04X} Key(vk='{k.vk}')\n")

    printer(''.join(strings))


def null_fn(*args):
    pass


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

def load_layout_infos(layouts, attr, mods,
                      unique_keymap:dict[Optional[tuple], int],
                      unique_deadkeys:dict[tuple, int]) -> list[LayoutInfo]:
    layouts2:list[LayoutInfo] = []
    for layout in layouts:
        keymaps = getattr(layout, attr)
        keymap_for_layout = []
        for mod in mods:
            keymap = keymaps[mod]
            keys = []
            dkeys = []
            old_dkidx = len(unique_deadkeys)
            for key in keymap:
                idk = 0
                if key and (key.codepoint or key.text):
                    if key.deadkeys:
                        deadkeys = [(dk.accent, dk.with_, dk.codepoint)
                                    for dk in key.deadkeys.values()]
                        deadkeys.sort()
                        idk = unique_deadkeys.setdefault((*deadkeys,), len(unique_deadkeys)+1)
                    dkeys.append(idk)
                    keys.append(Key2(codepoint=key.codepoint,
                                     is_deadkey=True if idk else False,
                                     text=key.text))
                else:
                    dkeys.append(0)
                    keys.append(None)
            idx = unique_keymap.setdefault((*keys,), len(unique_keymap))
            dkeys = None if old_dkidx == len(unique_deadkeys) else (*dkeys,)
            keymap_for_layout.append(Keymap2(mod=mod, keymap=keymap, idx=idx, dkeymap=dkeys))
        layouts2.append(LayoutInfo(layout=layout, keymaps=keymap_for_layout))
    return layouts2

if __name__ == "__main__":
    if len(sys.argv) == 1:
        print(sys.argv[0], 'layout.xml...')
    else:
        layouts:list[KeyLayout] = []
        log = null_fn
        # log = verbose_print
        for filename in sys.argv[1:]:
            log('filename:', filename)
            layout = parse_xml_layout(filename, log)
            layouts.append(layout)
            if log == verbose_print:
                print_layout(layout)

        normal_mod_supported = set((
            '',
            'VK_SHIFT',
            'VK_SHIFT VK_CAPITAL',
            'VK_SHIFT VK_CAPITAL VK_NUMLOCK',
            'VK_SHIFT VK_CONTROL',
            'VK_SHIFT VK_CONTROL VK_MENU VK_CAPITAL',
            'VK_SHIFT VK_CONTROL VK_MENU VK_CAPITAL VK_NUMLOCK',
            'VK_SHIFT VK_CONTROL VK_MENU VK_NUMLOCK',
            'VK_SHIFT VK_CONTROL VK_MENU',
            'VK_SHIFT VK_NUMLOCK',
            'VK_CAPITAL',
            'VK_CAPITAL VK_NUMLOCK',
            'VK_CONTROL',
            'VK_CONTROL VK_MENU',
            'VK_CONTROL VK_MENU VK_NUMLOCK',
            'VK_CONTROL VK_MENU VK_CAPITAL',
            'VK_CONTROL VK_MENU VK_CAPITAL VK_NUMLOCK',
            'VK_NUMLOCK',
            'VK_SHIFT VK_OEM_8',
            'VK_OEM_8',
        ))

        extended_mod_supported = set((
            '',
            'VK_SHIFT',
            'VK_CONTROL'
        ))

        error_messages = []
        for attr,seq in (('normal_keymaps', normal_mod_supported),
                         ('extended_keymaps', extended_mod_supported)):
            for layout in layouts:
                for mod,keymap in getattr(layout, attr).items():
                    if mod in seq:
                        # check that codepoint <= 0x7fff
                        if not all(not key or key.codepoint <= 0x7fff for key in keymap):
                            error_messages.append(f'{attr}: {mod or "NoMod"} for {layout.klid}/{layout.locale_name} have a codepoint greater that 0x7fff')
                        # check that there is no deadkeys of deadkeys
                        if not all(not key or key.deadkeys or all(d.deadkeys is None for d in key.deadkeys) for key in keymap):
                            error_messages.append(f'{attr}: {mod or "NoMod"} for {layout.klid}/{layout.locale_name} have a deadkeys of deadkeys')
                    else:
                        # check that unknown mod is empty
                        if not all(key is None for key in keymap):
                            error_messages.append(f'{attr}: {mod or "NoMod"} for {layout.klid}/{layout.locale_name} is not null')
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

        mods_to_names_table = {
            '': 'no_mod',
            'VK_SHIFT': 'shift',
            'VK_SHIFT VK_CAPITAL': 'capslock_shift',
            'VK_SHIFT VK_CAPITAL VK_NUMLOCK': 'capslock_numlock_shift',
            'VK_SHIFT VK_CONTROL': 'shift_ctrl',
            'VK_SHIFT VK_CONTROL VK_MENU VK_CAPITAL': 'capslock_shift_altgr',
            'VK_SHIFT VK_CONTROL VK_MENU VK_CAPITAL VK_NUMLOCK': 'capslock_numlock_shift_altgr',
            'VK_SHIFT VK_CONTROL VK_MENU VK_NUMLOCK': 'numlock_shift_altgr',
            'VK_SHIFT VK_CONTROL VK_MENU': 'shift_altgr',
            'VK_SHIFT VK_NUMLOCK': 'numlock_shift',
            'VK_SHIFT VK_OEM_8': 'shift_oem8',
            'VK_CAPITAL': 'capslock',
            'VK_CAPITAL VK_NUMLOCK': 'numlock_capslock',
            'VK_CONTROL': 'ctrl',
            'VK_CONTROL VK_MENU': 'altgr',
            'VK_CONTROL VK_MENU VK_NUMLOCK': 'numlock_altgr',
            'VK_CONTROL VK_MENU VK_CAPITAL': 'capslock_altgr',
            'VK_CONTROL VK_MENU VK_CAPITAL VK_NUMLOCK': 'capslock_numlock_altgr',
            'VK_NUMLOCK': 'numlock',
            'VK_OEM_8': 'oem8',
        }

        unique_keymap = {(None,)*128: 0,}
        unique_deadkeys = dict()
        normal_layouts = load_layout_infos(layouts, 'normal_keymaps', normal_mod_supported,
                                           unique_keymap, unique_deadkeys)
        extended_layouts = load_layout_infos(layouts, 'extended_keymaps', extended_mod_supported,
                                             unique_keymap, unique_deadkeys)

        strings = [
            '#include "keyboard/keylayout2.hpp"\n\n',
            'constexpr auto DK = KeyLayout2::DK;\n\n',
        ]

        # print keymap (scancodes[128] with DK (0x8000) mask for deadkey)
        for keymap, idx in unique_keymap.items():
            strings.append(f'static constexpr KeyLayout2::unicode_t keymap_{idx}[128] {{\n')
            for i in range(128//8):
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
            strings.append(f'static constexpr DKeyTable::Data dkeydata_{idx}[] {{\n')
            strings.append(f'    {{.size={len(deadkeys)}}},\n')
            strings.append(f'    {{.accent=0x{ord(accent):04X} /* {accent} */}},\n')
            strings += (f'    {{.dkey={{0x{ord(with_):04X} /* {with_} */, 0x{codepoint:04X} /* {chr(codepoint)} */}}}},\n' for accent, with_, codepoint in deadkeys)
            strings.append('};\n\n')

        # dkeymap memoization
        dktables = {(0,)*128: 0,}
        for layout, keymaps in chain(normal_layouts, extended_layouts):
            for mod, keymap, dkeymap, idx in keymaps:
                if dkeymap:
                    dktables.setdefault(dkeymap, len(dktables))

        # print dkeymap (DKeyTable[])
        for deadmap,idx in dktables.items():
            strings.append(f'static constexpr DKeyTable dkeymap_{idx}[] {{\n')
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
        for normal_layout,extended_layout in zip(normal_layouts, extended_layouts):
            mods_array = [0]*64
            dmods_array = [0]*64
            for mod, keymap, dkeymap, idx in normal_layout.keymaps:
                mask = sum(mods_to_mask[m] for m in mod.split(' '))
                mods_array[mask] = idx
                if dkeymap:
                    dmods_array[mask] = dktables[dkeymap]
            k1 = (*mods_array,)
            k2 = (*dmods_array,)
            k1 = unique_layout_keymap.setdefault(k1, len(unique_layout_keymap))
            k2 = unique_layout_dkeymap.setdefault(k2, len(unique_layout_dkeymap))
            layout = normal_layout.layout
            strings2.append(f'    KeyLayout2{{KeyLayout2::KbdId(0x{layout.klid}), "{layout.locale_name}", /*"{layout.display_name}", */keymap_mod_{k1}, dkeymap_mod_{k2}, ')

            mods_array = [0]*64
            for mod, keymap, dkeymap, idx in extended_layout.keymaps:
                mask = sum(mods_to_mask[m] for m in mod.split(' '))
                mods_array[mask] = idx
                assert not dkeymap
            k1 = (*mods_array,)
            k1 = unique_layout_keymap.setdefault(k1, len(unique_layout_keymap))
            strings2.append(f'keymap_mod_{k1}}},\n')
        strings2.append('};\n')

        # print layout
        for unique_layout,prefix,atype in (
            (unique_layout_keymap, '', 'KeyLayout2::unicode_t'),
            (unique_layout_dkeymap, 'd', 'DKeyTable')
        ):
            for k,idx in unique_layout.items():
                strings.append(f'static constexpr sized_array_view<{atype}, 128> {prefix}keymap_mod_{idx}[] {{\n')
                for i in range(64//8):
                    strings.append('   ')
                    strings += (f' {prefix}keymap_{k[i]},' for i in range(i*8, i*8+8))
                    strings.append('\n')
                strings.append('};\n\n')

        # print layout
        strings2.append('\narray_view<KeyLayout> KeyLayout2::keylayouts() noexcept\n')
        strings2.append('{\n    return layouts;\n}\n\n')
        strings2.append('KeyLayout const* KeyLayout2::find_layout_by_id(KbdId id) noexcept\n')
        strings2.append('{\n    switch (id)\n    {\n')
        for i,(layout,keymaps) in enumerate(normal_layouts):
            strings2.append(f'    case 0x{layout.klid}: return &layouts[{i}];\n')
        strings2.append('    }\n    return nullptr;\n}\n')

        print(''.join(strings))
        print(''.join(strings2))
