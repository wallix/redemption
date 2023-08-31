#!/usr/bin/env python3
import unittest
import os

from typing import List, Union, Tuple, NamedTuple
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
                          ConfigurationFragment)


class TestMigration(unittest.TestCase):
    maxDiff = None

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
            'sec1': [(1, 3)],
            'sec2': [(4, 6), (10, 15)],
            'sec3': [(7, 9)],
            'sec4': [(16, 20)],
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

        migrate_def = {
            'sec1': {
                'moved_key': UpdateItem(key='moved_key_to_a'),
                'updated_value': UpdateItem(values={'old_b_value': 'new_b'}),
                'removed_key': RemoveItem(),
            },
            'moved_section': (MoveSection('new_moved_section'), {
                'moved_key_to_removed_section': UpdateItem(section='removed_section'),
                'aa': UpdateItem(section='removed_section', key='new_aa',
                                  values={'old_b_value': 'new_value'}),
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

        self.assertEqual(migrate(fragments, migrate_def), (True, [
            ConfigurationFragment('#[moved_section]', kind=ConfigKind.Comment),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('[new_moved_section]', kind=ConfigKind.Section, value1='new_moved_section'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('#moved_key_to_removed_section=va', kind=ConfigKind.Comment),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('#moved_key_to_new_section=vb', kind=ConfigKind.Comment),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('#moved_key_to_new_section_and_renamed_key_to_cc=vc', kind=ConfigKind.Comment),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('#moved_key_to_new_section5=vd', kind=ConfigKind.Comment),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('[sec1]', kind=ConfigKind.Section, value1='sec1'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('cc=vc', kind=ConfigKind.KeyValue, value1='cc', value2='vc'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('#moved_key=vva', kind=ConfigKind.Comment),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('moved_key_to_a=vva', kind=ConfigKind.KeyValue,
                                  value1='moved_key_to_a', value2='vva'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('#updated_value=old_b_value', kind=ConfigKind.Comment),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('updated_value=new_b', kind=ConfigKind.KeyValue,
                                  value1='updated_value', value2='new_b'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('#removed_key=vvc', kind=ConfigKind.Comment),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('d=vvd', kind=ConfigKind.KeyValue, value1='d', value2='vvd'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('e=vve', kind=ConfigKind.KeyValue, value1='e', value2='vve'),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('#[removed_section]', kind=ConfigKind.Comment),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('#a=vvva', kind=ConfigKind.Comment),
            ConfigurationFragment('\n', kind=ConfigKind.NewLine),
            ConfigurationFragment('#b=vvvb', kind=ConfigKind.Comment),
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
            ConfigurationFragment(text='#key = val', kind=ConfigKind.Comment, value1='', value2=''),
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
            version,
            ini_filename=ini_filename,
            temporary_ini_filename=f'{ini_filename}.work',
            saved_ini_filename=f'{ini_filename}.{version}'))

        with open(ini_filename, encoding='utf-8') as f:
            self.assertEqual(f.read(),
                '[globals]\n'
                '#session_timeout = 1000\n'
                'base_inactivity_timeout=1000\n'
                '\n'
                '[mod_rdp]\n'
                '#session_probe_exe_or_file=notepad\n'
                'depth=15\n'
                '\n'
                '[video]\n'
                '#replay_path=/tmp/\n'
                '\n'
                '[all_target_mod]\n'
                '#connection_retry_count=3\n'
                '\n'
                '[mod_replay]\n'
                'replay_path=/tmp/\n'
                '\n'
                '[session_probe]\n'
                'exe_or_file=notepad\n')

        os.remove(ini_filename)
        os.remove(f'{ini_filename}.{version}')
