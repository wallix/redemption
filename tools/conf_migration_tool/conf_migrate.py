#!/usr/bin/python3 -O
##
# Copyright (c) 2023 WALLIX. All rights reserved.
# Licensed computer software. Property of WALLIX.
# Product Name: WALLIX Bastion v9.0
# Author(s): Raphael Zhou, Jonathan Poelen
##

from shutil import copyfile
from enum import IntEnum
from typing import (List, Tuple, Dict, Optional, Union, Iterable, Any,
                    Sequence, NamedTuple, Generator, Callable, TypeVar)

import os
import re
import sys
import itertools

rgx_version = re.compile(r'^(\d+)\.(\d+)\.(\d+)(.*)')


class RedemptionVersionError(Exception):
    pass


class RedemptionVersion:
    def __init__(self, version: str) -> None:
        m = rgx_version.match(version)
        if m is None:
            raise RedemptionVersionError(
                f"Invalid version string format: {version}")

        self.__major = int(m.group(1))
        self.__minor = int(m.group(2))
        self.__build = int(m.group(3))
        self.__revision = m.group(4)

    def __str__(self) -> str:
        return f"{self.__major}.{self.__minor}.{self.__build}{self.__revision}"

    def __lt__(self, other: 'RedemptionVersion') -> bool:
        return self.__part() < other.__part()

    def __le__(self, other: 'RedemptionVersion') -> bool:
        return self.__part() <= other.__part()

    def __eq__(self, other: 'RedemptionVersion') -> bool:
        return self.__part() == other.__part()

    @staticmethod
    def from_file(filename: str) -> 'RedemptionVersion':
        with open(filename) as f:
            line = f.readline()  # read first line
            items = line.split()
            version_string = items[1]
            return RedemptionVersion(version_string)

    def __part(self) -> Tuple[int, int, int, str]:
        return (self.__major, self.__minor, self.__build, self.__revision)


class ConfigKind(IntEnum):
    Unknown = 0
    NewLine = 1
    Comment = 2
    Section = 3
    KeyValue = 4


rgx_ini_parser = re.compile(
    r'(\n)|'
    r'[ \t]*(#)[^\n]*|'
    r'[ \t]*\[[ \t]*(.+?)[ \t]*\][ \t]*|'
    r'[ \t]*([^\s]+)[ \t]*=[ \t]*(.*?)[ \t]*(?=\n|$)|'
    r'[^\n]+')


novalue = ''


class ConfigurationFragment(NamedTuple):
    text: str
    kind: ConfigKind
    value1: str = novalue
    value2: str = novalue


newline_fragment = ConfigurationFragment('\n', ConfigKind.NewLine)


def _to_fragment(m: re.Match) -> ConfigurationFragment:
    # new line
    if m.start(1) != -1:
        return newline_fragment

    # comment
    if m.start(2) != -1:
        return ConfigurationFragment(
            m.group(0), ConfigKind.Comment
        )

    # section
    if m.start(3) != -1:
        return ConfigurationFragment(
            m.group(0), ConfigKind.Section,
            m.group(3)
        )

    # variable
    if m.start(4) != -1:
        return ConfigurationFragment(
            m.group(0), ConfigKind.KeyValue,
            m.group(4),
            m.group(5),
        )

    return ConfigurationFragment(m.group(0), ConfigKind.Unknown)


ConfigurationFragmentListType = List[ConfigurationFragment]


def parse_configuration(content: str) -> ConfigurationFragmentListType:
    return list(map(_to_fragment, rgx_ini_parser.finditer(content)))


def parse_configuration_from_file(filename: str) -> Tuple[str, ConfigurationFragmentListType]:
    with open(filename, encoding='utf-8') as f:
        content = f.read()
        return (content, parse_configuration(content))


class UpdateItem(NamedTuple):
    section: Optional[str] = None
    key: Optional[str] = None
    value_transformation: Optional[Callable[[str, Iterable[ConfigurationFragment]], str]] = None
    reason: str = ''
    old_display_name: str = ''
    ini_only: bool = False
    to_ini_only: bool = False

    def update(self, section: str, key: str, value: str,
               fragments: Iterable[ConfigurationFragment]) -> Tuple[str, str, str]:
        if self.section is not None:
            section = self.section
        if self.key is not None:
            key = self.key
        if self.value_transformation is not None:
            value = self.value_transformation(value, fragments)
        return section, key, value


class RemoveItem(NamedTuple):
    reason: str = ''
    old_display_name: str = ''
    ini_only: bool = False


# for information only
class ToIniOnly(NamedTuple):
    reason: str = ''
    old_display_name: str = ''


class MoveSection(NamedTuple):
    name: str
    old_display_name: str = ''


MigrationKeyOrderType = Union[RemoveItem, UpdateItem, ToIniOnly]
MigrationSectionOrderType = Union[RemoveItem,
                                  MoveSection,
                                  Dict[str, MigrationKeyOrderType]]
MigrationDescType = Dict[str, MigrationSectionOrderType]

MigrationType = Tuple[RedemptionVersion, MigrationDescType]


def migration_filter(migration_defs: Iterable[MigrationType],
                     previous_version: RedemptionVersion) -> List[MigrationType]:
    migration_defs = sorted(migration_defs, key=lambda t: t[0])

    for i, t in enumerate(migration_defs):
        if previous_version >= t[0]:
            continue

        return migration_defs[i:]

    return []


def migration_def_to_actions(fragments: Iterable[ConfigurationFragment],
                             migration_def: MigrationDescType
                             ) -> Tuple[
                                 List[Tuple[str, str]],  # renamed_sections
                                 # section, old_key, new_key, new_value
                                 List[Tuple[str, str, str, str]],  # renamed_keys
                                 # old_section, old_key, new_section, new_key, new_value
                                 List[Tuple[str, str, str, str, str]],  # moved_keys
                                 List[str],  # removed_sections
                                 List[Tuple[str, str]],  # removed_keys
                                ]:
    section = ''
    original_section = ''
    migration_key_desc = None
    renamed_sections: List[Tuple[str, str]] = []
    renamed_keys: List[Tuple[str, str, str, str]] = []
    moved_keys: List[Tuple[str, str, str, str, str]] = []
    removed_sections: List[str] = []
    removed_keys: List[Tuple[str, str]] = []

    for fragment in fragments:
        if fragment.kind == ConfigKind.KeyValue:
            if migration_key_desc:
                order = migration_key_desc.get(fragment.value1)
                if order is None:
                    pass
                elif isinstance(order, RemoveItem):
                    removed_keys.append((section, fragment.value1))
                elif isinstance(order, UpdateItem):
                    t = order.update(section, fragment.value1, fragment.value2, fragments)
                    if t[0] == section:
                        renamed_keys.append((original_section, fragment.value1, t[1], t[2]))
                    else:
                        moved_keys.append((original_section, fragment.value1, *t))

        elif fragment.kind == ConfigKind.Section:
            migration_key_desc = None
            section = fragment.value1
            original_section = section
            order = migration_def.get(section)

            if order is None:
                pass
            elif isinstance(order, RemoveItem):
                removed_sections.append(section)
            elif isinstance(order, MoveSection):
                renamed_sections.append((section, order.name))
            elif isinstance(order, dict):
                migration_key_desc = order
            else:
                for order in order:
                    if isinstance(order, MoveSection):
                        renamed_sections.append((section, order.name))
                        section = order.name
                    else:
                        migration_key_desc = order

    return renamed_sections, renamed_keys, moved_keys, removed_sections, removed_keys


def fragments_to_spans_of_sections(fragments: Iterable[ConfigurationFragment]) -> Dict[str, List[Sequence[int]]]:
    start = 0
    section = ''
    section_spans: Dict[str, List[Sequence[int]]] = dict()

    i = 0
    for i, fragment in enumerate(fragments):
        if fragment.kind == ConfigKind.Section:
            if start < i-1:
                section_spans.setdefault(section, []).append(range(start, i-1))
            section = fragment.value1
            start = i
    if start < i-1:
        section_spans.setdefault(section, []).append(range(start, i))

    return section_spans


def _is_empty_line(i: int, fragments: List[ConfigurationFragment]) -> bool:
    return len(fragments) > i and fragments[i].kind == ConfigKind.NewLine


def migrate(fragments: List[ConfigurationFragment],
            migration_def: MigrationDescType) -> Tuple[bool, List[ConfigurationFragment]]:
    renamed_sections, renamed_keys, moved_keys, removed_sections, removed_keys \
        = migration_def_to_actions(fragments, migration_def)

    if not (renamed_sections or renamed_keys or moved_keys or removed_sections or removed_keys):
        return (False, fragments)

    reinject_fragments: Dict[int, Iterable[ConfigurationFragment]] = dict()
    section_spans = fragments_to_spans_of_sections(fragments)

    def iter_from_spans(spans) -> Iterable[int]:
        return (i for span in spans for i in span)

    def iter_key_fragment(section_name: str,
                          kind: ConfigKind,
                          key_name: str
                          ) -> Generator[Tuple[int, ConfigurationFragment], None, None]:
        for i in iter_from_spans(section_spans.get(section_name, ())):
            fragment: ConfigurationFragment = fragments[i]
            if fragment.kind != ConfigKind.KeyValue or key_name != fragment.value1:
                continue
            yield i, fragment

    for section, old_key, new_key, new_value in renamed_keys:
        for i, fragment in iter_key_fragment(section, ConfigKind.KeyValue, old_key):
            reinject_fragments[i] = (
                ConfigurationFragment(f'{new_key}={new_value}', ConfigKind.KeyValue,
                                      new_key, new_value),
            )

    for section in removed_sections:
        for i in iter_from_spans(section_spans.get(section, ())):
            fragment = fragments[i]
            if fragment.kind in (ConfigKind.KeyValue, ConfigKind.Section):
                # if fragment.kind == ConfigKind.Section:
                #     del section_spans[section]
                reinject_fragments[i] = ()
                if _is_empty_line(i+1, fragments):
                    reinject_fragments[i+1] = ()

    for t in itertools.chain(removed_keys, moved_keys):
        for i, fragment in iter_key_fragment(t[0], ConfigKind.KeyValue, t[1]):
            reinject_fragments[i] = ()
            if _is_empty_line(i+1, fragments):
                reinject_fragments[i+1] = ()

    for old_section, new_section in renamed_sections:
        for rng in section_spans.get(old_section, ()):
            istart = rng[0]
            reinject_fragments[istart] = (
                ConfigurationFragment(f'[{new_section}]', ConfigKind.Section, new_section),
            )

    added_keys: Dict[str, List[ConfigurationFragment]] = dict()

    for old_section, old_key, new_section, new_key, new_value in moved_keys:
        for _ in iter_key_fragment(old_section, ConfigKind.KeyValue, old_key):
            added_keys.setdefault(new_section, []).extend((
                newline_fragment,
                ConfigurationFragment(f'{new_key}={new_value}', ConfigKind.KeyValue,
                                      new_key, new_value),
                newline_fragment,
            ))

    result_fragments: List[ConfigurationFragment] = []

    # insert a key in a section
    for i, fragment in enumerate(fragments):
        added_fragments = reinject_fragments.get(i, (fragment,))
        result_fragments.extend(added_fragments)

        for fragment in added_fragments:
            if fragment.kind == ConfigKind.Section:
                section = fragment.value1
                key = added_keys.get(section)
                if key:
                    result_fragments.extend(key)
                    added_keys[section] = []
                break

    # add new section
    for section, keys in added_keys.items():
        if keys:
            result_fragments.extend((
                newline_fragment,
                ConfigurationFragment(f'[{section}]', ConfigKind.Section, section),
            ))
            result_fragments.extend(keys)

    # remove duplicate key and keep the last (i.e. remove moved keys that already exist).
    keys_visited = set()
    def filter_duplicate_key(fragment: ConfigurationFragment) -> bool:
        if fragment.kind == ConfigKind.KeyValue:
            key = fragment.value1
            if key in keys_visited:
                return False
            keys_visited.add(key)
        elif fragment.kind == ConfigKind.Section:
            keys_visited.clear()
        return True
    result_fragments = list(filter(filter_duplicate_key, reversed(result_fragments)))
    result_fragments.reverse()

    return (True, result_fragments)


### --dump=json
### @{

def remove_ini_only_type(desc: MigrationDescType) -> MigrationDescType:
    return {section: ({k: v for k, v in values_or_item.items() if type(v) != ToIniOnly}
                      if isinstance(values_or_item, dict) else values_or_item)
            for section, values_or_item in desc.items() if type(values_or_item) != ToIniOnly}


def migrate_file(migration_defs: List[MigrationType],
                 version: RedemptionVersion,
                 ini_filename: str,
                 temporary_ini_filename: str,
                 saved_ini_filename: str,
                 ) -> bool:
    _, fragments = parse_configuration_from_file(ini_filename)

    is_changed = False
    for _, desc in migration_filter(migration_defs, version):
        is_updated, fragments = migrate(fragments, remove_ini_only_type(desc))
        is_changed = is_changed or is_updated

    if is_changed:
        with open(temporary_ini_filename, 'w', encoding='utf-8') as f:
            f.write(''.join(fragment.text for fragment in fragments))

        copyfile(ini_filename, saved_ini_filename)

        os.rename(temporary_ini_filename, ini_filename)

    return is_changed


def dump_json(defs: List[MigrationType]) -> List[Any]:
    """
    format: [
        {
            "version": str,
            "data": {
                <section_name>: RemoveItem | MoveSection | Values
            }
        }
    ]
    RemoveItem = {
        "kind": "remove",
        "reason": optional[str],
        "oldDisplayName": optional[str],
    }
    MoveSection = {
        "kind": "move",
        "reason": optional[str],
        "oldDisplayName": optional[str],
    }
    Values = {
        "kind": "values",
        "values": {
            <value_name>: RemoveItem | UpdateItem,
        }
    }
    UpdateItem = {
        "kind": "update",
        "newSection": optional[str],
        "newKey": optional[str],
        "reason": optional[str],
        "oldDisplayName": optional[str],
        "iniOnly": optional[bool],
        "toIniOnly": optional[bool],
    }
    """
    def update_if(d, k, v):
        if v:
            d[k] = v

    def remove_to_dict(item: RemoveItem):
        d = {'kind': 'remove'}
        update_if(d, 'reason', item.reason)
        update_if(d, 'oldDisplayName', item.old_display_name)
        update_if(d, 'iniOnly', item.ini_only)
        return d

    def ini_only_to_dict(item: ToIniOnly):
        d = {'kind': 'update', 'toIniOnly': True}
        update_if(d, 'reason', item.reason)
        update_if(d, 'oldDisplayName', item.old_display_name)
        return d

    def move_to_dict(item: MoveSection):
        d = {'kind': 'move', 'newName': item.name}
        update_if(d, 'oldDisplayName', item.old_display_name)
        return d

    def update_to_dict(item: UpdateItem) -> Optional[Dict[str, str]]:
        if not (item.section or item.key or item.to_ini_only):
            return None
        d = {'kind': 'update'}
        update_if(d, 'newSection', item.section)
        update_if(d, 'newKey', item.key)
        update_if(d, 'reason', item.reason)
        update_if(d, 'oldDisplayName', item.old_display_name)
        update_if(d, 'iniOnly', item.ini_only)
        update_if(d, 'toIniOnly', item.to_ini_only)
        return d

    def dict_to_dict(values: Dict[str, MigrationKeyOrderType]):
        data = {}
        for k, item in values.items():
            obj = visitor[type(item)](item)
            if obj:
                data[k] = obj
        return {'kind': 'values', 'values': data}

    visitor = {
        RemoveItem: remove_to_dict,
        ToIniOnly: ini_only_to_dict,
        MoveSection: move_to_dict,
        UpdateItem: update_to_dict,
        dict: dict_to_dict,
    }

    l = []
    for version, desc in defs:
        data = {}
        for section, values_or_item in desc.items():
            obj = visitor[type(values_or_item)](values_or_item)
            if obj:
                data[section] = obj
        if data:
            l.append({'version': str(version), 'data': data})

    return l

### @}


def main(migration_defs: List[MigrationType], argv: List[str]) -> int:
    if len(argv) != 4 or argv[1] not in ('-s', '-f'):
        if len(argv) == 2 and argv[1] == '--dump=json':
            import json
            print(json.dumps(dump_json(migration_defs)))
            return 0

        print(f'{argv[0]} {{-s|-f}} old_version ini_filename\n'
              '  -s   <version> is a output format of redemption --version\n'
              '  -f   <version> is a version of redemption from file\n\n'
              f'{argv[0]} --dump=json\n',
              file=sys.stderr)
        return 1

    ini_filename = argv[3]
    if argv[1] == '-f':
        old_version = RedemptionVersion.from_file(argv[2])
    else:
        old_version = RedemptionVersion(argv[2])

    print(f"PreviousRedemptionVersion={old_version}")

    if migrate_file(migration_defs,
                    old_version,
                    ini_filename=ini_filename,
                    temporary_ini_filename=f'{ini_filename}.work',
                    saved_ini_filename=f'{ini_filename}.{old_version}'):
        print("Configuration file updated")
    return 0


def _to_bool(value: str) -> bool:
    return value.strip().lower() in ('1', 'yes', 'true', 'on')


def _to_int(value: str) -> int:
    try:
        if value.startswith('0x'):
            return int(value, 16)
        return int(value)
    except ValueError:
        return 0


_IniValue = TypeVar("_IniValue")

def _get_values(fragments: Iterable[ConfigurationFragment],
                desc: Sequence[Tuple[str, str, _IniValue, Callable[[str], _IniValue]]]
                ) -> List[_IniValue]:
    values = [default_value for section, name, default_value, converter in desc]

    tree: Dict[str, Dict[str, int]] = {}
    for i, t in enumerate(desc):
        tree.setdefault(t[0], {})[t[1]] = i

    section = None
    for fragment in fragments:
        if fragment.kind == ConfigKind.KeyValue:
            if section is not None:
                i = section.get(fragment.value1)
                if i is not None:
                    values[i] = desc[i][3](fragment.value2)
        elif fragment.kind == ConfigKind.Section:
            section = tree.get(fragment.value1)

    return values


def _merge_session_log_format_10_5_31(value: str, fragments: Iterable[ConfigurationFragment]) -> str:
    enable_session_log, enable_arcsight_log = _get_values(
        fragments, (('session_log', 'enable_session_log', True, _to_bool),
                    ('session_log', 'enable_arcsight_log', False, _to_bool)))

    return str(
        (1 if enable_session_log else 0) |
        (2 if enable_arcsight_log else 0)
    )


PerformanceFlagParts = Tuple[str, str, str, str, str, str, str]

def _performance_flags_to_string(flags: int, enable: bool) -> PerformanceFlagParts:
    signs = ('-', '+')
    return (
        signs[enable] + 'wallpaper' if flags & 0x1 else '',
        signs[enable] + 'menu_animations' if flags & 0x4 else '',
        signs[enable] + 'theme' if flags & 0x8 else '',
        signs[enable] + 'mouse_cursor_shadows' if flags & 0x20 else '',
        signs[enable] + 'cursor_blinking' if flags & 0x40 else '',
        signs[not enable] + 'font_smoothing' if flags & 0x80 else '',
        signs[not enable] + 'desktop_composition' if flags & 0x100 else '',
    )

def _merge_performance_flags_10_5_31(value: str, fragments: Iterable[ConfigurationFragment]) -> str:
    force_present, force_not_present = _get_values(
        fragments, (('client', 'performance_flags_force_present', 0x28, _to_int),
                    ('client', 'performance_flags_force_not_present', 0, _to_int)))

    force_not_present_elements = _performance_flags_to_string(force_not_present, True)
    force_present_elements = _performance_flags_to_string(force_present, False)

    return ','.join(not_present or present
                    for not_present, present in zip(force_not_present_elements,
                                                    force_present_elements)
                    if not_present or present)


migration_defs: List[MigrationType] = [
    (RedemptionVersion('9.1.39'), {
        'globals': {
            'session_timeout': UpdateItem(key='base_inactivity_timeout'),
        },
    }),
    (RedemptionVersion("9.1.71"), {
        'video': {
            'replay_path': UpdateItem(section='mod_replay'),
        },
        'mod_rdp': {
            'session_probe_exe_or_file': UpdateItem(section='session_probe', key='exe_or_file'),
            'session_probe_arguments': UpdateItem(section='session_probe', key='arguments'),
            'session_probe_customize_executable_name': UpdateItem(section='session_probe',
                                                                  key='customize_executable_name'),
            'session_probe_allow_multiple_handshake': UpdateItem(section='session_probe',
                                                                 key='allow_multiple_handshake'),
            'session_probe_at_end_of_session_freeze_connection_and_wait':
                UpdateItem(section='session_probe',
                           key='at_end_of_session_freeze_connection_and_wait'),
            'session_probe_enable_cleaner': UpdateItem(section='session_probe', key='enable_cleaner'),
            'session_probe_clipboard_based_launcher_reset_keyboard_status':
                UpdateItem(section='session_probe',
                           key='clipboard_based_launcher_reset_keyboard_status'),
            'session_probe_bestsafe_integration': UpdateItem(section='session_probe',
                                                             key='enable_bestsafe_interaction'),
        },
    }),
    (RedemptionVersion("9.1.76"), {
        'all_target_mod': {
            'connection_retry_count': RemoveItem(),
        },
    }),
    (RedemptionVersion("10.2.8"), {
        'video': {
            'capture_groupid': RemoveItem(
                reason='Old mechanism for the apache server to access the file.'),
        },
    }),
    (RedemptionVersion("10.3.3"), {
        'metrics': RemoveItem(reason='Abandoned project.'),
    }),
    (RedemptionVersion("10.5.27"), {
        'mod_rdp': {
            'proxy_managed_drives': ToIniOnly(),
        },
        'globals': {
            'glyph_cache': RemoveItem(reason='Is configurable with "Disabled Orders".'),
            'authfile': ToIniOnly(),
            'trace_type': ToIniOnly(reason='Has no effect because it is overwritten by other configurations at Bastion level.'),
        },
        'client': {
            'bogus_user_id': RemoveItem(
                reason='A malformed packet containing UserId is now always supported.'),
            'bogus_sc_net_size': RemoveItem(
                reason='A malformed SCNet packet length is now always supported.'),
            'bogus_number_of_fastpath_input_event': RemoveItem(
                reason='Workaround for old version of freerdp client.'),
            'bogus_neg_request': RemoveItem(
                reason='Workaround for old version of jrdp client.'),
            'keyboard_layout_proposals': UpdateItem(section='internal_mod'),
        },
        'video': {
            'bogus_vlc_frame_rate': RemoveItem(reason='Workaround for old version of VLC.'),
        },
        'session_log': {
            'hide_non_printable_kbd_input': RemoveItem(
                reason='No longer used and replaced by "Keyboard input masking level".'),
        },
        'mod_replay': {
            'replay_path': ToIniOnly(reason='Useless in Bastion.'),
        },
    }),
    (RedemptionVersion("10.5.31"), {
        'mod_rdp': {
            'allow_channels': UpdateItem(key='allowed_channels', ini_only=True),
            'deny_channels': UpdateItem(key='denied_channels', ini_only=True),
            'accept_monitor_layout_change_if_capture_is_not_started': RemoveItem(
                reason='Is now always accepted.'),
            'experimental_fix_too_long_cookie': RemoveItem(
                reason='Is now always supported.'),
        },
        'globals': {
            'experimental_support_resize_session_during_recording': RemoveItem(
                reason='Is now always supported.'),
            'support_connection_redirection_during_recording': RemoveItem(
                reason='Is now always supported.'),
            'new_pointer_update_support': RemoveItem(
                reason='Is now always supported.'),
            'encryptionLevel': UpdateItem(
                section='client', key='encryption_level',
                value_transformation=lambda *_: 'high',
                to_ini_only=True,
                reason='Is now always high. Legacy encryption when External Security Protocol (TLS, CredSSP, etc) is disable.'),
            'certificate_password': ToIniOnly(),
            'experimental_enable_serializer_data_block_size_limit': ToIniOnly(),
            'enable_bitmap_update': ToIniOnly(reason='Is now always supported.'),
            'listen_address': ToIniOnly(),
            'mod_recv_timeout': ToIniOnly(reason='Unused'),  # soon :D
            'unicode_keyboard_event_support': ToIniOnly(reason='Is now always supported.'),
        },
        'client': {
            'disable_tsk_switch_shortcuts': RemoveItem(
                reason='Has no effect because it is overwritten by other configurations at Bastion level.'),
            'performance_flags_default': RemoveItem(
                reason='Redundancy with options that add or remove flags.'),
            'performance_flags_force_present': UpdateItem(
                key='force_performance_flags',
                value_transformation=_merge_performance_flags_10_5_31,
                reason='Merging of "Performance Flags Force Present" and "Performance Flags Not Force Present".'),
            'performance_flags_force_not_present': UpdateItem(
                key='force_performance_flags',
                value_transformation=_merge_performance_flags_10_5_31,
                reason='Merging of "Performance Flags Force Present" and "Performance Flags Not Force Present".'),
            'cache_waiting_list': ToIniOnly(),
            'enable_nla': ToIniOnly(reason='Is now always supported.'),
            'recv_timeout': ToIniOnly(reason='Unused'),  # soon :D
        },
        'session_log': {
            'enable_session_log': UpdateItem(
                key='syslog_format',
                value_transformation=_merge_session_log_format_10_5_31,
                reason='Merging of "Enable Session Log" and "Enable Arcsight Log".'),
            'enable_arcsight_log': UpdateItem(
                key='syslog_format',
                value_transformation=_merge_session_log_format_10_5_31,
                reason='Merging of "Enable Session Log" and "Enable Arcsight Log".'),
        },
        'video': {
            'disable_keyboard_log': UpdateItem(
                key='enable_keyboard_log',
                # has meta (4) -> False
                value_transformation=lambda value, _: f'{(int(value) & 4) == 0}'),
            'disable_clipboard_log': UpdateItem(
                value_transformation=lambda value, _: f'{(int(value) >> 1)}'),
            'disable_file_system_log': UpdateItem(
                value_transformation=lambda value, _: f'{(int(value) >> 1)}'),
            'png_interval': UpdateItem(
                value_transformation=lambda value, _: f'{(int(value) * 100)}',
                to_ini_only=True),
            'png_limit': ToIniOnly(reason='Old mechanism before Redis.'),
        }
    }),
    (RedemptionVersion("10.5.35"), {
        'globals': {
            'enable_close_box': UpdateItem(section='internal_mod'),
            'close_timeout': UpdateItem(section='internal_mod', key='close_box_timeout'),
            'enable_osd': UpdateItem(key='enable_end_time_warning_osd'),
            'allow_using_multiple_monitors': UpdateItem(section='client'),
            'allow_scale_factor': UpdateItem(section='client'),
            'unicode_keyboard_event_support': UpdateItem(section='client'),
            'bogus_refresh_rect': UpdateItem(section='mod_rdp'),
        },
        'client': {
            'force_performance_flags': UpdateItem(section='mod_rdp'),
            'auto_adjust_performance_flags': UpdateItem(section='mod_rdp'),
            'show_target_user_in_f12_message': UpdateItem(section='globals'),
        },
    }),
]

if __name__ == '__main__':
    exit(main(migration_defs, sys.argv))
