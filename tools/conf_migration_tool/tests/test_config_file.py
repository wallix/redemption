#!/usr/bin/env python3
import unittest
import os

from typing import Tuple
from conf_migrate import (parse_configuration,
                          migration_filter,
                          migration_def_to_actions,
                          fragments_to_spans_of_sections,
                          migrate,
                          migrate_file,
                          RedemptionVersion,
                          UpdateItem,
                          RemoveItem,
                          MoveSection,
                          ConfigKind,
                          ConfigurationFragment,
                          MigrationDescType,
                          migration_defs,
                          _merge_session_log_format_10_5_31,
                          _merge_performance_flags_10_5_31)


def process_migrate(migrate_def: MigrationDescType, ini: str) -> Tuple[bool, str]:
    r = migrate(parse_configuration(ini), migrate_def)
    return (r[0], ''.join(fragment.text for fragment in r[1]))


class TestMigration(unittest.TestCase):
    maxDiff = None

    def test_unknown_section(self):
        sections = {
            "globals",

            "client",

            "all_target_mod",
            "remote_program",
            "mod_rdp",
            "protocol",
            "session_probe",
            "server_cert",
            "mod_vnc",

            "session_log",
            "ocr",
            "capture",
            "video",
            "audit",

            "file_verification",
            "file_storage",
            "icap_server_down",
            "icap_server_up",

            "crypto",
            "websocket",

            "vnc_over_ssh",

            "context",

            "internal_mod",
            "mod_replay",
            "translation",
            "theme",

            "debug",

            # old sections

            "metrics",
        }
        for _, d in migration_defs:
            for k, v in d.items():
                self.assertNotEqual(k, 'rdp', 'should be mod_rdp')
                self.assertNotEqual(k, 'vnc', 'should be mod_vnc')
                self.assertIn(k, sections)
                if type(v) == UpdateItem and v.section:
                    self.assertIn(v.section, sections)


    def test_parse_configuration(self):
        self.assertEqual(parse_configuration('[section]\n# bla bla\nkey = value'), [
            ConfigurationFragment('[section]', ConfigKind.Section, 'section'),
            ConfigurationFragment('\n', ConfigKind.NewLine),
            ConfigurationFragment('# bla bla', ConfigKind.Comment),
            ConfigurationFragment('\n', ConfigKind.NewLine),
            ConfigurationFragment('key = value', ConfigKind.KeyValue, 'key', 'value'),
        ])
        self.assertEqual(parse_configuration('  [section]  \n  # bla bla\n  key  =  value  '), [
            ConfigurationFragment('  [section]  ', ConfigKind.Section, 'section'),
            ConfigurationFragment('\n', ConfigKind.NewLine),
            ConfigurationFragment('  # bla bla', ConfigKind.Comment),
            ConfigurationFragment('\n', ConfigKind.NewLine),
            ConfigurationFragment('  key  =  value  ', ConfigKind.KeyValue, 'key', 'value'),
        ])
        self.assertEqual(parse_configuration('[section]  key  =  value \nline error\nkey2=va lu e2 '), [
            ConfigurationFragment('[section]  ', ConfigKind.Section, 'section'),
            ConfigurationFragment('key  =  value ', ConfigKind.KeyValue, 'key', 'value'),
            ConfigurationFragment('\n', ConfigKind.NewLine),
            ConfigurationFragment('line error', ConfigKind.Unknown),
            ConfigurationFragment('\n', ConfigKind.NewLine),
            ConfigurationFragment('key2=va lu e2 ', ConfigKind.KeyValue, 'key2', 'va lu e2'),
        ])

    def test_migration_filter(self):
        c1 = (RedemptionVersion('9.1.30'), dict())
        c2 = (RedemptionVersion('9.1.32'), dict())
        c3 = (RedemptionVersion('9.1.34'), dict())
        desc = (c1, c2, c3)
        self.assertEqual(migration_filter(desc, RedemptionVersion('9.1.29')), [c1,c2,c3])
        self.assertEqual(migration_filter(desc, RedemptionVersion('9.1.30')), [c2,c3])
        self.assertEqual(migration_filter(desc, RedemptionVersion('9.1.31')), [c2,c3])
        self.assertEqual(migration_filter(desc, RedemptionVersion('9.1.32')), [c3])
        self.assertEqual(migration_filter(desc, RedemptionVersion('9.1.33')), [c3])
        self.assertEqual(migration_filter(desc, RedemptionVersion('9.1.34')), [])
        self.assertEqual(migration_filter(desc, RedemptionVersion('9.1.35')), [])

    def test_fragments_to_spans_of_sections(self):
        ini = '''
        [sec1] a=a
        [sec2] a=a
        [sec3] a=a
        [sec2]
        b=b
        c=c
        [sec4]
        d=d
        '''

        fragments = parse_configuration(ini)

        self.assertEqual(list(fragment.text for fragment in fragments), [
            '\n',
            '        [sec1] ',
            'a=a',
            '\n',
            '        [sec2] ',
            'a=a',
            '\n',
            '        [sec3] ',
            'a=a',
            '\n',
            '        [sec2]',
            '\n',
            '        b=b',
            '\n',
            '        c=c',
            '\n',
            '        [sec4]',
            '\n',
            '        d=d',
            '\n',
            '        ',
        ])

        self.assertEqual(fragments_to_spans_of_sections(fragments), {
            'sec1': [range(1, 3)],
            'sec2': [range(4, 6), range(10, 15)],
            'sec3': [range(7, 9)],
            'sec4': [range(16, 20)],
        })

    def test_migrate(self):
        ini = (
            '[moved_section]\n'
            'moved_key_to_removed_section=va\n'
            'moved_key_to_new_section=vb\n'
            'moved_key_to_new_section_and_renamed_key_to_cc=vc\n'
            'moved_key_to_new_section5=vd\n'
            '\n'
            '[sec1]\n'
            'moved_key=vva\n'
            'updated_value=old_b_value\n'
            'removed_key=vvc\n'
            'd=vvd\n'
            'e=vve\n'
            '\n'
            '[removed_section]\n'
            'a=vvva\n'
            'b=vvvb\n'
            '\n'
            '[sec4]\n'
            'x=y\n'
        )

        def remap(d):
            return lambda value,_: d.get(value, value)

        migrate_def = {
            'sec1': {
                'moved_key': UpdateItem(key='moved_key_to_a'),
                'updated_value': UpdateItem(value_transformation=remap({'old_b_value': 'new_b'})),
                'removed_key': RemoveItem(),
            },
            'moved_section': (MoveSection('new_moved_section'), {
                'moved_key_to_removed_section': UpdateItem(section='removed_section'),
                'aa': UpdateItem(section='removed_section', key='new_aa',
                                 value_transformation=remap({'old_b_value': 'new_value'})),
                'moved_key_to_new_section': UpdateItem(section='sec4'),
                'moved_key_to_new_section_and_renamed_key_to_cc': UpdateItem(section='sec1', key='cc'),
                'moved_key_to_new_section5': UpdateItem(section='sec5'),
            }),
            'removed_section': RemoveItem(),
        }

        fragments = parse_configuration(ini)

        self.assertEqual(migration_def_to_actions(fragments, migrate_def), (
            # renamed_sections
            [('moved_section', 'new_moved_section')],
            # renamed_keys
            [
                ('sec1', 'moved_key', 'moved_key_to_a', 'vva'),
                ('sec1', 'updated_value', 'updated_value', 'new_b')
            ],
            # moved_keys
            [
                ('moved_section', 'moved_key_to_removed_section',
                 'removed_section', 'moved_key_to_removed_section', 'va'),
                ('moved_section', 'moved_key_to_new_section',
                 'sec4', 'moved_key_to_new_section', 'vb'),
                ('moved_section', 'moved_key_to_new_section_and_renamed_key_to_cc',
                 'sec1', 'cc', 'vc'),
                ('moved_section', 'moved_key_to_new_section5',
                 'sec5', 'moved_key_to_new_section5', 'vd'),
            ],
            # removed_sections
            ['removed_section'],
            # removed_keys
            [('sec1', 'removed_key')]
        ))

        self.assertEqual(migrate(fragments, {}), (False, fragments))

        self.assertEqual(migrate(fragments, {'video': {}}), (False, fragments))

        self.assertEqual(migrate(fragments, migrate_def), (True, [
            ConfigurationFragment('[new_moved_section]', kind=ConfigKind.Section, value1='new_moved_section'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('[sec1]', kind=ConfigKind.Section, value1='sec1'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('cc=vc', kind=ConfigKind.KeyValue, value1='cc', value2='vc'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('moved_key_to_a=vva', kind=ConfigKind.KeyValue,
                                  value1='moved_key_to_a', value2='vva'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('updated_value=new_b', kind=ConfigKind.KeyValue,
                                  value1='updated_value', value2='new_b'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('d=vvd', kind=ConfigKind.KeyValue, value1='d', value2='vvd'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('e=vve', kind=ConfigKind.KeyValue, value1='e', value2='vve'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('[sec4]', kind=ConfigKind.Section, value1='sec4'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('moved_key_to_new_section=vb', kind=ConfigKind.KeyValue,
                                  value1='moved_key_to_new_section', value2='vb'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('x=y', kind=ConfigKind.KeyValue, value1='x', value2='y'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('[removed_section]', kind=ConfigKind.Section, value1='removed_section'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('moved_key_to_removed_section=va', kind=ConfigKind.KeyValue,
                                  value1='moved_key_to_removed_section', value2='va'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('[sec5]', kind=ConfigKind.Section, value1='sec5'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('moved_key_to_new_section5=vd', kind=ConfigKind.KeyValue,
                                  value1='moved_key_to_new_section5', value2='vd'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
        ]))

    def test_migrate_value_to_an_already_existing_value(self):
        ini = (
            '[sec1]\n'
            'key = val\n'  # this key will be moved in sec2 then removed
            '[sec2]\n'
            'key = other value\n'
        )

        migrate_def = {
            'sec1': {
                'key': UpdateItem(section='sec2'),
            }
        }

        fragments = parse_configuration(ini)

        self.assertEqual(migration_def_to_actions(fragments, migrate_def), (
            # renamed_sections
            [],
            # renamed_keys
            [],
            # moved_keys
            [
                ('sec1', 'key',
                 'sec2', 'key', 'val'),
            ],
            # removed_sections
            [],
            # removed_keys
            []
        ))

        self.assertEqual(migrate(fragments, {}), (False, fragments))

        self.assertEqual(migrate(fragments, migrate_def), (True, [
            ConfigurationFragment(text='[sec1]', kind=ConfigKind.Section, value1='sec1', value2=''),
            ConfigurationFragment(text='\n', kind=ConfigKind.NewLine, value1='', value2=''),
            ConfigurationFragment(text='[sec2]', kind=ConfigKind.Section, value1='sec2', value2=''),
            ConfigurationFragment(text='\n', kind=ConfigKind.NewLine, value1='', value2=''),
            ConfigurationFragment(text='\n', kind=ConfigKind.NewLine, value1='', value2=''),
            ConfigurationFragment(text='\n', kind=ConfigKind.NewLine, value1='', value2=''),
            ConfigurationFragment(text='key = other value', kind=ConfigKind.KeyValue,
                                  value1='key', value2='other value'),
            ConfigurationFragment(text='\n', kind=ConfigKind.NewLine, value1='', value2='')
        ]))

    def test_migrate_file(self):
        ini_filename = '/tmp/test_config_file.ini'

        with open(ini_filename, 'w', encoding='utf-8') as f:
            f.write(
                '[globals]\n'
                'session_timeout = 1000\n'
                '\n'
                '[mod_rdp]\n'
                'session_probe_exe_or_file=notepad\n'
                'depth=15\n'
                '\n'
                '[video]\n'
                'replay_path=/tmp/\n'
                '\n'
                '[all_target_mod]\n'
                'connection_retry_count=3\n')

        version = RedemptionVersion('9.1.38')

        self.assertEqual(True, migrate_file(
            migration_defs,
            version,
            ini_filename=ini_filename,
            temporary_ini_filename=f'{ini_filename}.work',
            saved_ini_filename=f'{ini_filename}.{version}'))

        with open(ini_filename, encoding='utf-8') as f:
            self.assertEqual(f.read(),
                '[globals]\n'
                'base_inactivity_timeout=1000\n'
                '\n'
                '[mod_rdp]\n'
                'depth=15\n'
                '\n'
                '[video]\n'
                '\n'
                '[all_target_mod]\n'
                '\n'
                '[session_probe]\n'
                'exe_or_file=notepad\n'
                '\n'
                '[mod_replay]\n'
                'replay_path=/tmp/\n')

        os.remove(ini_filename)
        os.remove(f'{ini_filename}.{version}')

    def test_merge_session_log_format_10_5_31(self):
        migrate_def = {
            'session_log': {
                'enable_session_log': UpdateItem(
                    key='syslog_format', value_transformation=_merge_session_log_format_10_5_31),
                'enable_arcsight_log': UpdateItem(
                    key='syslog_format', value_transformation=_merge_session_log_format_10_5_31),
            },
        }

        for ini in (
            '[sec1]\nvalue1=xy\n',
            '[session_log]\nvalue1=xy\n',
        ):
            self.assertEqual(process_migrate(migrate_def, ini), (False, ini), ini)

        for d, r in (
            ('enable_session_log=True', 1),
            ('enable_session_log=False', 0),
            ('enable_arcsight_log=True', 3),
            ('enable_arcsight_log=False', 1),
        ):
            ini = f'[session_log]\n{d}\n'
            self.assertEqual(process_migrate(migrate_def, ini),
                             (True, f'[session_log]\nsyslog_format={r}\n'),
                             f'ini: {ini}')

        for d, r in (
            ('enable_session_log=True\nenable_arcsight_log=True', 3),
            ('enable_session_log=True\nenable_arcsight_log=False', 1),
            ('enable_session_log=False\nenable_arcsight_log=True', 2),
            ('enable_session_log=False\nenable_arcsight_log=False', 0),
        ):
            ini = f'[session_log]\n{d}\n'
            self.assertEqual(process_migrate(migrate_def, ini),
                             (True, f'[session_log]\n\nsyslog_format={r}\n'),
                             f'ini: {ini}')


    def test_merge_performance_flags_10_5_31(self):
        migrate_def = {
            'client': {
                'performance_flags_force_present': UpdateItem(
                    key='force_performance_flags',
                    value_transformation=_merge_performance_flags_10_5_31),
                'performance_flags_force_not_present': UpdateItem(
                    key='force_performance_flags',
                    value_transformation=_merge_performance_flags_10_5_31),
            },
        }

        for ini in (
            '[sec1]\nvalue1=xy\n',
            '[client]\nvalue1=xy\n',
        ):
            self.assertEqual(process_migrate(migrate_def, ini), (False, ini), f'ini: {ini}')

        for d, r in (
            ('performance_flags_force_present=0x28', '-theme,-mouse_cursor_shadows'),
            ('performance_flags_force_present=0x104', '-menu_animations,+desktop_composition'),
            ('performance_flags_force_not_present=0x28', '+theme,+mouse_cursor_shadows'),
            ('performance_flags_force_not_present=0x104', '+menu_animations,-theme,-mouse_cursor_shadows,-desktop_composition'),
        ):
            ini = f'[client]\n{d}\n'
            self.assertEqual(process_migrate(migrate_def, ini),
                             (True, f'[client]\nforce_performance_flags={r}\n'),
                             f'ini: {ini}')

        for d, r in (
            ('performance_flags_force_not_present=0x104\nperformance_flags_force_present=0',
             '+menu_animations,-desktop_composition'),
            ('performance_flags_force_not_present=0x185\nperformance_flags_force_present=0x104',
             '+wallpaper,+menu_animations,-font_smoothing,-desktop_composition'),
            ('performance_flags_force_not_present=0x104\nperformance_flags_force_present=0x185',
             '-wallpaper,+menu_animations,+font_smoothing,-desktop_composition'),
        ):
            ini = f'[client]\n{d}\n'
            self.assertEqual(process_migrate(migrate_def, ini),
                             (True, f'[client]\n\nforce_performance_flags={r}\n'),
                             f'ini: {ini}')


    def test_migrate_10_5_31(self):
        migrate_def = next(migrate_def for version, migrate_def in migration_defs
                           if RedemptionVersion("10.5.31") == version)

        self.assertEqual(process_migrate(migrate_def, '[video]\ndisable_keyboard_log=3\n'),
                         (True, '[video]\nenable_keyboard_log=True\n'))

        self.assertEqual(process_migrate(migrate_def, '[video]\ndisable_keyboard_log=0\n'),
                         (True, '[video]\nenable_keyboard_log=True\n'))

        self.assertEqual(process_migrate(migrate_def, '[video]\ndisable_keyboard_log=4\n'),
                         (True, '[video]\nenable_keyboard_log=False\n'))

        self.assertEqual(process_migrate(migrate_def, '[video]\ndisable_keyboard_log=5\n'),
                         (True, '[video]\nenable_keyboard_log=False\n'))

        self.assertEqual(process_migrate(migrate_def, '[video]\ndisable_file_system_log=0\n'),
                         (True, '[video]\ndisable_file_system_log=0\n'))

        self.assertEqual(process_migrate(migrate_def, '[video]\ndisable_file_system_log=1\n'),
                         (True, '[video]\ndisable_file_system_log=0\n'))

        self.assertEqual(process_migrate(migrate_def, '[video]\ndisable_file_system_log=4\n'),
                         (True, '[video]\ndisable_file_system_log=2\n'))
