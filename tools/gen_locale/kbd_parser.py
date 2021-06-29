#!/usr/bin/env python3
from typing import Optional, NamedTuple
from collections import OrderedDict
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

KeymapType = list[Optional[Key]] # always 256 elements
KeymapsType = dict[str, KeymapType]

class KeyLayout(NamedTuple):
    klid:int
    locale_name:str
    display_name:str
    keymaps:KeymapsType
    extra_scancodes:dict[int, Key]
    has_right_ctrl_like_oem8:bool


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
    keymaps = {k:[None]*256 for k in (
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
    )}
    extra_scancodes = {}

    # ascii letter for dead keys
    # deadtexts:dict[str, int] = {}

    root = ElementTree.parse(filename).getroot()
    klid, locale_name, display_name = _getattribs(log, root[0], 'metadata', {'KLID': True,
                                                                             'LocaleName': True,
                                                                             'LayoutDisplayName': True})
    right_ctrl_like_oem8 = False
    has_oem8_key = False
    for pk in root[1]:
        sc, vk, _ = _getattribs(log, pk, 'PK', {'SC': True, 'VK': True, 'Name': False})
        sc = int(sc, 16)

        assert sc & 0xff
        assert (sc & 0xff) <= 0x7f
        assert (sc >> 8) in (0, 0xE0, 0xE1)

        # Pause
        if sc > 0xE100:
            assert sc == 0xE11D
            extra_scancodes[sc] = Key(scancode=sc, codepoint=0, text='', vk=vk, deadkeys=dict())
            continue

        if not pk and vk == 'VK_OEM_8':
            # assume that VK_OEM_8 is set after dead keys
            assert has_oem8_key
            assert sc == 0xE01D # right ctrl
            right_ctrl_like_oem8 = sc

        sc = (sc & 0x7f) | (0x80 if sc >> 8 else 0)

        if not pk:
            keys = keymaps['']
            assert sc not in keys
            keys[sc] = Key(scancode=sc, codepoint=0, text='', vk=vk, deadkeys=dict())
            continue

        for result in pk:
            text, codepoint, vk, with_ = _getattribs(log, result, 'Result', {'Text': False,
                                                                             'TextCodepoints': False,
                                                                             'VK': False,
                                                                             'With': False})
            keys = keymaps[with_ or '']

            if with_ and ('VK_OEM_8' in with_):
                assert not right_ctrl_like_oem8
                has_oem8_key = True

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


    # new keymap: shiftlock + numlock
    for num_mod, caps_mod, final_mod in _merge_numlock_capital:
        assert final_mod not in keymaps
        caps_keymap = keymaps[caps_mod]
        numlock_keymap = keymaps[num_mod]
        keymap = [None] * 256
        for i in range(256):
            k = caps_keymap[i]
            knum = numlock_keymap[i]
            assert not (k and knum)
            keymap[i] = k or knum
        keymaps[final_mod] = keymap

    # merge mod to numlock mod
    for num_mod, merged_mod in _merge_numlock:
        keymap = keymaps[merged_mod]
        numlock_keymap = keymaps[num_mod]
        for i in range(256):
            k = keymap[i]
            knum = numlock_keymap[i]
            assert not (k and knum)
            if k:
                numlock_keymap[i] = k

    return KeyLayout(klid, locale_name, display_name,
                     keymaps, extra_scancodes,
                     right_ctrl_like_oem8)

def _accu_scancodes(strings:list[str], map:list[Key]):
    for i,k in enumerate(map):
        if k:
            text = k.text
            if k.codepoint < 32:
                if 0x07 <= k.codepoint <= 0x0D:
                    text = ('\\a', '\\b', '\\t', '\\n', '\\v', '\\f', '\\r')[k.codepoint - 0x07]
                elif k.codepoint:
                    text = ''
            # del
            elif k.codepoint == 127:
                text = ''
            strings.append(f"  0x{i:02X}: codepoint=0x{k.codepoint:04x} text='{text}' vk='{k.vk}'\n")
            if k.deadkeys:
                strings.append('       DeadKeys: ')
                prefix = ''
                for dk in k.deadkeys.values():
                    strings.append(f'{prefix}{dk.accent} + {dk.text} => {dk.codepoint}\n')
                    prefix = '                 '
        else:
            strings.append(f"  0x{i:02X} -\n")

def print_layout(layout:KeyLayout, printer=sys.stdout.write):
    strings = [f'KLID: {layout.klid}\nLocalName: {layout.locale_name}\nDisplayName: {layout.display_name}\n']

    for mapname,map in layout.keymaps.items():
        strings.append(f'{mapname or "normal"} (0x00)\n')
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

def load_layout_infos(layouts, mods,
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

if __name__ == "__main__":
    if len(sys.argv) == 1:
        print(sys.argv[0], 'layout.xml...')
    else:
        layouts:list[KeyLayout] = []
        log = null_fn
        #log = verbose_print
        for filename in sys.argv[1:]:
            log('filename:', filename)
            layout = parse_xml_layout(filename, log)
            layouts.append(layout)
            if log == verbose_print:
                print_layout(layout)

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
            '#include "cxx/diagnostic.hpp"\n\n',
            'constexpr auto DK = KeyLayout::DK;\n\n',
            'using KbdId = KeyLayout::KbdId;\n\n',
            'REDEMPTION_DIAGNOSTIC_PUSH()\n',
            'REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wc++20-designator")\n\n',
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
            strings2.append(f'    KeyLayout{{KbdId(0x{layout.klid}), KeyLayout::RCtrlIsCtrl({layout.has_right_ctrl_like_oem8 and "false" or "true "}), "{layout.locale_name}"_zv, /*"{layout.display_name}"_zv, */keymap_mod_{k1}, dkeymap_mod_{k2}}},\n')
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
        strings2.append('\nREDEMPTION_DIAGNOSTIC_POP()\n')

        print(''.join(strings))
        print(''.join(strings2))
