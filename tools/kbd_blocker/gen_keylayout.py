#!/usr/bin/env python3
from kbd_parser import KeymapType, KeyLayout, Key, parse_argv


vk_control_masks = {
    '':             0,
    'VK_SHIFT':     1 << 0,
    'VK_NUMLOCK':   1 << 1,
    'VK_OEM_8':     1 << 2,
    'altgr':        1 << 3,
    'VK_KANA':      1 << 4,
    'VK_CAPITAL':   1 << 5,
    'VK_MENU':      1 << 6,
    'VK_CONTROL':   1 << 7,
    'VK_KANALOCK':  1 << 8,
}
nomod = 0
numlock = vk_control_masks['VK_NUMLOCK']
capslock = vk_control_masks['VK_CAPITAL']
kanalock = vk_control_masks['VK_KANALOCK']
kana = vk_control_masks['VK_KANA']
ctrl = vk_control_masks['VK_CONTROL']
alt = vk_control_masks['VK_MENU']
altgr = vk_control_masks['altgr']
shift = vk_control_masks['VK_SHIFT']
oem8 = vk_control_masks['VK_OEM_8']
ctrl_alt = ctrl | alt
ignored_mods = capslock | kanalock | ctrl | alt | kana | altgr | oem8


def vk_mod_to_mod_flags(mods:str) -> int:
    mod_flags = 0
    for m in mods.split(' '):
        mod_flags |= vk_control_masks[m]
    # ctrl+alt to altgr
    if (mod_flags & ctrl_alt) == ctrl_alt:
        mod_flags = mod_flags & ~ctrl_alt | altgr
    # remove ctrl
    return mod_flags & ~ctrl

def key_to_compressed_scancode(key:Key) -> int:
    extended = 0x80 if key.is_extended() else 0
    scancode = key.scancode & 0x7f
    return extended | scancode

def is_lockable_numpad(scancode:int) -> bool:
    return 0x47 <= scancode <= 0x53 and scancode not in (0x4A, 0x4E)

layouts:list[KeyLayout] = parse_argv()

rkeymaps = []

for layout in layouts:
    rkeymap = dict()
    normal_keymap = layout.keymaps['']
    for mods,keymap in layout.keymaps.items():
        mod_flags = vk_mod_to_mod_flags(mods)
        if mod_flags & ignored_mods:
            continue

        for key in keymap:
            if key and key.codepoint:
                # ignore numlock with unlockable key
                if mod_flags & numlock and not is_lockable_numpad(key.scancode):
                    continue
                # ignore shifted character
                if mod_flags & shift and key.text and key.text.lower()[0] == normal_keymap[key.scancode].text:
                    continue
                l = rkeymap.setdefault(key.codepoint, set())
                l.add((key_to_compressed_scancode(key), mod_flags))

    rkeymaps.append(rkeymap)


def insert_sublist_and_return_index(l, subl):
    sublen = len(subl)
    llen = len(l)
    if sublen <= llen:
        # search sublist
        for i in range(llen - sublen + 1):
            if l[i:i+sublen] == subl:
                return i

    i = 0
    # search overlap
    for i in range(llen):
        if l[llen-i-1:] != subl[:i]:
            break
    l += subl[i:]
    return llen-i

all_scancode_mods = []
all_codepoint_iscmods = []
all_layout = []
all_ilayout = dict()

irlayouts = []

import sys

for rkeymap in rkeymaps:
    rlayout = []
    for codepoint,keys in rkeymap.items():
        keys = sorted(keys)
        isc = insert_sublist_and_return_index(all_scancode_mods, keys)
        assert all_scancode_mods[isc:isc+len(keys)] == keys
        cpiscmods = [codepoint, len(keys), isc]
        ik = insert_sublist_and_return_index(all_codepoint_iscmods, cpiscmods)
        assert all_codepoint_iscmods[ik:ik+3] == cpiscmods
        rlayout.append(ik)
    rlayout.sort(key=lambda ik: all_codepoint_iscmods[ik:ik+3])
    ilayout = all_ilayout.setdefault((*rlayout,), len(all_ilayout))
    irlayouts.append(ilayout)


def divide_chunks(l, n):
    for i in range(0, len(l), n):
        yield l[i:i + n]

output = [
    '#include "keyboard/keyboard_shortcut_layout.hpp"\n\n',
    'namespace\n{\n\n'
    'using KSMods = KeyboardShortcutLayout::KSMods;\n\n'
]

for name, l, n, nchars, comm in (
    ('KeyboardShortcutLayout::ScancodeAndMods all_keycode_and_mods',
        [f'{{{scancode:>3},KSMods({mods:>2})}}' for scancode,mods in all_scancode_mods], 4, 0, ''),
    ('uint16_t all_codepoint_and_iscmods', all_codepoint_iscmods, 15, 4, '// codepoint, len, iscm\n'),
    *((f'uint16_t rlayout_{i}', rlayout, 15, 4, '')
      for i,rlayout in sorted((ikb,rlayout) for rlayout,ikb in all_ilayout.items())),
):
    output.append(comm)
    output.append(f'constexpr {name}[] = {{')
    for l in divide_chunks(l, n):
        output.append('\n  ')
        output.append(''.join(map(lambda x: f'{x:>{nchars}},', l)))
    output.append('\n};\n\n')

output.append('} // anonymous namespace\n\n')

output.append('''KeyboardShortcutLayout KeyboardShortcutLayout::find_layout_by_id(KeyLayout::KbdId kid) noexcept
{
    KeyboardShortcutLayout layout{};
    switch (underlying_cast(kid))
    {
''')
for ilayout, irlayout in enumerate(irlayouts):
    output.append(f'    case 0x{layouts[ilayout].klid}: layout.cp_and_iscmods_indexes = rlayout_{irlayout}; break;\n')
output.append('''
    }

  return layout;
}

array_view<KeyboardShortcutLayout::ScancodeAndMods>
KeyboardShortcutLayout::find_unicode(uint16_t unicode) const noexcept
{
    auto it = std::lower_bound(
        cp_and_iscmods_indexes.begin(), cp_and_iscmods_indexes.end(),
        unicode, [](auto i, auto unicode) noexcept {
            return all_codepoint_and_iscmods[i] < unicode;
        }
    );
    if (it != cp_and_iscmods_indexes.end() && all_codepoint_and_iscmods[*it] == unicode) {
        const auto len = all_codepoint_and_iscmods[*it+1];
        const auto iscmods = all_codepoint_and_iscmods[*it+2];
        return {all_keycode_and_mods + iscmods, len};
    }
    return {};
}
''')

print(''.join(output))
