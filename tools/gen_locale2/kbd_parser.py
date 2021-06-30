#!/usr/bin/env python3
from typing import Optional, NamedTuple, Union, Iterator

import xml.etree.ElementTree as ET
from itertools import groupby
from enum import Enum
import sys


class DeadKey(NamedTuple):
    accent:str
    text:str
    result:str
    deadkeys:Optional['DeadKey']


class Key(NamedTuple):
    scancode:int
    codepoint:int
    text:str
    vk:str
    # key = (accent, with_key)
    deadkeys:dict[tuple[str,str], DeadKey]


class KeyLayout(NamedTuple):
    klid:int
    locale_name:str
    display_name:str
    normal_keymaps:dict[str, list[Optional[Key]]]
    # 0xEO
    extended_keymaps:dict[str, list[Optional[Key]]]
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


def _parse_deadkeys(log, result, deadkeys):
    for dead_key_table in result:
        accent, name = _getattribs(log, dead_key_table, 'DeadKeyTable', {'Accent': True, 'Name': False})
        for result in dead_key_table:
            text, with_ = _getattribs(log, result, 'Result', {'Text': False, 'With': True})
            deadkey = DeadKey(accent=accent, text=with_, result=text,
                              deadkeys=None if text else dict())
            k = (accent, with_)
            assert k is not deadkeys
            deadkeys[k] = deadkey
            # double dead keys
            if not text:
                _parse_deadkeys(log, result, deadkey.deadkeys)


mirrormaps = {
    'VK_SHIFT VK_KANA VK_NUMLOCK': 'VK_NUMLOCK',
    'VK_SHIFT VK_CONTROL VK_MENU VK_CAPITAL': 'VK_CONTROL VK_MENU',
    'VK_SHIFT VK_CONTROL VK_MENU VK_NUMLOCK': 'VK_CONTROL VK_MENU VK_NUMLOCK',
    'VK_KANA VK_NUMLOCK': 'VK_NUMLOCK',
    'VK_CONTROL VK_MENU VK_CAPITAL': 'VK_SHIFT VK_CONTROL VK_MENU',
}

def verbose_print(*args):
    print(*args, file=sys.stderr)

def parse_xml_layout(filename, log=verbose_print):
    keymap_mods = {
        0x00: {k:[None for i in range(0, 128)] for k in (
            '',
            'VK_SHIFT',
            'VK_SHIFT VK_CONTROL',
            'VK_SHIFT VK_CAPITAL',
            'VK_SHIFT VK_NUMLOCK',
            'VK_SHIFT VK_KANA',
            'VK_SHIFT VK_OEM_8',
            'VK_SHIFT VK_CONTROL VK_MENU',
            'VK_SHIFT VK_CONTROL VK_KANA',
            'VK_CONTROL',
            'VK_CONTROL VK_MENU',
            'VK_CONTROL VK_KANA',
            'VK_CONTROL VK_MENU VK_NUMLOCK',
            'VK_CAPITAL',
            'VK_NUMLOCK',
            'VK_KANA','VK_OEM_8',
        )},
        # extended
        0xE0: {k:[None for i in range(0, 128)] for k in (
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
    deadtexts:dict[str, int] = {}

    root = ET.parse(filename).getroot()
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
            mirrormap = mirrormaps.get(with_)
            keys = keymaps[with_ or ''] if mirrormap is None else keymaps[mirrormap]
            key = keys[sc]

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

                if mirrormap is None:
                    if key is not None:
                        raise Exception(f'key {sc} ({text}/{codepoint}) already set')
                    assert text is not deadtexts
                    keys[sc] = Key(scancode=sc, codepoint=codepoint, text=text, vk=vk, deadkeys=dict())
                    deadtexts[text] = sc
                else:
                    assert key.scancode == sc
                    assert key.codepoint == codepoint
                    assert key.text == text
                    assert key.vk == vk

            # dead keys
            else:
                assert not vk
                if key is None:
                    key = Key(scancode=sc, codepoint=0, text='', vk=vk, deadkeys=dict())
                    keys[sc] = key
                _parse_deadkeys(log, result, key.deadkeys)

    return KeyLayout(klid, locale_name, display_name,
                     keymap_mods[0], keymap_mods[0xE0], extra_scancodes)

def _accu_scancodes(strings:list[str], map:list[Key]):
    for i,k in enumerate(map):
        if k:
            text = k.text
            if k.codepoint < 32:
                if text == '\n':
                    text = '\\n'
                elif text == '\r':
                    text = '\\r'
                elif k.codepoint:
                    text = ''
            elif k.codepoint == 127:
                text = ''
            strings.append(f"  0x{i:02X} Key(codepoint=0x{k.codepoint:02x}, text='{text}', vk='{k.vk}'\n")
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

    printer


def _all_equal(iterable):
    "Returns True if all the elements are equal to each other"
    g = groupby(iterable)
    return next(g, True) and not next(g, False)


class ModStatus(Enum):
    DISABLED = 0
    ENABLED = 1
    ALL_EQUAL = 2

if __name__ == "__main__":
    if len(sys.argv) == 1:
        print(sys.argv[0], 'layout.xml...')
    else:
        layouts:list[KeyLayout] = []
        for filename in sys.argv[1:]:
            verbose_print('filename:', filename)
            layout = parse_xml_layout(filename)
            layouts.append(layout)
            print_layout(layout)

        mod_infos = {}
        for keymap_type in ('normal_keymaps', 'extended_keymaps'):
            mods = getattr(layouts[0], keymap_type).keys()
            mod_states = {}
            for mod in mods:
                are_same = _all_equal(getattr(layout, keymap_type)[mod] for layout in layouts)
                status = (
                    ModStatus.DISABLED if all(k is None for k in getattr(layouts[0], keymap_type)[mod])
                    else ModStatus.ALL_EQUAL
                ) if are_same else ModStatus.ENABLED
                mod_states[mod] = status
            mod_infos[keymap_type] = mod_states

        are_same = _all_equal(layout.extra_scancodes for layout in layouts)
        status = (
            ModStatus.ALL_EQUAL if layouts[0].extra_scancodes
            else ModStatus.DISABLED
        ) if are_same else ModStatus.ENABLED
        mod_infos['extra_scancodes'] = {'': status}

        print(mod_infos)
